#include "ServerManager.hpp"
#include <unistd.h>
#include <iostream>
#include <vector>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <cstring>
#include <errno.h>

ServerManager::ServerManager() {
}

ServerManager::~ServerManager() {
	// Cerrar todas las conexiones de clientes al destruir el servidor
	//std::map<int, Client> clientFds_;  // ← En el futuro
	//boolActualmente siempre true, no se usa realmente
	for (std::map<int, bool>::iterator it = clientFds_.begin(); it != clientFds_.end(); ++it) {
		close(it->first);
	}
	clientFds_.clear();
}

/**
 * Inicia el servidor: crea el socket de escucha y lo registra en epoll
 * 
 * Proceso:
 * 1. listener_.bindAndListen(): Crea y configura el socket de escucha
 * 2. epoll_.addFd(): Registra el socket en epoll para monitorear conexiones nuevas
 * 
 * EPOLLIN: Queremos ser notificados cuando haya datos para leer
 *          En el socket de escucha, esto significa "hay una conexión nueva"
 * 
 * EPOLLET: Edge-triggered mode (modo disparado por borde)
 *          - Solo notifica cuando el estado CAMBIA (de "no conexión" a "hay conexión")
 *          - Más eficiente que level-triggered (que notifica mientras haya conexiones)
 *          - Requiere leer TODOS los datos disponibles en una pasada
 */
void ServerManager::start(const std::string& host, int port) {
	// Bind and listen
	listener_.bindAndListen(host, port);
	
	// Agregar el socket de escucha a epoll
	// EPOLLIN: Notificar cuando haya conexiones nuevas
	// EPOLLET: Edge-triggered (solo notifica cuando cambia el estado)
	epoll_.addFd(listener_.getFd(), EPOLLIN | EPOLLET);
	
	std::cout << "Server started and ready to accept connections" << std::endl;
}

/**
 * BUCLE PRINCIPAL DEL SERVIDOR - El corazón de todo
 * 
 * Este es el bucle de eventos que hace que el servidor funcione.
 * Es la implementación del patrón "event loop" o "reactor pattern".
 * 
 * ¿Cómo funciona?
 * 
 * 1. ESPERAR EVENTOS (epoll_wait):
 *    - Bloquea aquí hasta que ocurra algo (conexión nueva, datos recibidos, etc.)
 *    - El kernel despierta nuestro proceso cuando hay eventos
 *    - Retorna una lista de TODOS los eventos que ocurrieron
 * 
 * 2. PROCESAR EVENTOS:
 *    - Iteramos sobre cada evento recibido
 *    - Identificamos qué socket es (listener o cliente)
 *    - Llamamos a la función apropiada para manejar ese evento
 * 
 * 3. REPETIR:
 *    - Volvemos a esperar más eventos
 *    - El ciclo continúa indefinidamente
 * 
 * ¿Por qué es eficiente?
 * - Solo bloquea cuando NO hay nada que hacer (ahorra CPU)
 * - Cuando hay eventos, los procesa rápidamente (todo no bloqueante)
 * - Puede manejar miles de clientes con un solo hilo
 * 
 * ¿Qué tipos de eventos procesamos?
 * - EPOLLIN en listener: Nueva conexión entrante
 * - EPOLLIN en cliente: Datos recibidos del cliente
 * - EPOLLERR/EPOLLHUP: Error o cliente desconectado
 */
void ServerManager::run() {
	std::vector<struct epoll_event> events;
	
	std::cout << "Entering event loop..." << std::endl;
	
	// BUCLE INFINITO - El servidor corre hasta que se termine el proceso
	while (true) {
		// PASO 1: ESPERAR EVENTOS
		// Esta es la ÚNICA línea que bloquea en todo el servidor
		// timeout = -1 significa "esperar indefinidamente hasta que haya eventos"
		int numEvents = epoll_.wait(events, -1);
		
		if (numEvents == 0) {
			continue; // Timeout (no debería pasar con timeout=-1, pero por si acaso)
		}
		
		// PASO 2: PROCESAR CADA EVENTO
		// Puede haber múltiples eventos listos al mismo tiempo
		for (int i = 0; i < numEvents; ++i) {
			int fd = events[i].data.fd;  // El fd que generó este evento
			
			// ¿Es el socket de escucha? → Nueva conexión
			if (fd == listener_.getFd()) {
				handleNewConnection();
			}
			// ¿Es un cliente con datos para leer? → Leer datos
			else if (events[i].events & EPOLLIN) {
				handleClientData(fd);
			}
			// ¿Hubo error o el cliente cerró? → Limpiar conexión
			if (events[i].events & (EPOLLERR | EPOLLHUP)) {
				handleClientDisconnect(fd);
			}
		}
		
		// PASO 3: Volver a esperar más eventos (vuelta al inicio del while)
	}
}

/**
 * Maneja nuevas conexiones entrantes
 * 
 * Se llama cuando epoll detecta EPOLLIN en el socket de escucha.
 * Esto significa que hay al menos una conexión nueva esperando ser aceptada.
 * 
 * ¿Por qué el bucle while?
 * - En modo edge-triggered (EPOLLET), epoll solo notifica UNA VEZ cuando cambia el estado
 * - Pero pueden llegar MÚLTIPLES conexiones entre notificaciones
 * - Debemos aceptar TODAS las conexiones pendientes en una pasada
 * - Si no, algunas conexiones quedarían esperando hasta la próxima notificación
 * 
 * ¿Cuándo termina el bucle?
 * - Cuando accept() retorna -1 (EAGAIN)
 * - Significa que ya no hay más conexiones pendientes
 */
void ServerManager::handleNewConnection() {
	// Aceptar TODAS las conexiones pendientes (importante en edge-triggered)
	while (true) {
		int clientFd = listener_.acceptConnection();
		
		if (clientFd == -1) {
			// No hay más conexiones disponibles (EAGAIN)
			// Esto es normal, significa que ya aceptamos todas las pendientes
			break;
		}
		
		// Registrar el nuevo cliente en epoll para monitorear sus datos
		// EPOLLIN: Notificar cuando el cliente envíe datos
		// EPOLLET: Edge-triggered (solo notifica cuando llegan nuevos datos)
		// EPOLLRDHUP: Notificar cuando el cliente cierre la conexión
		epoll_.addFd(clientFd, EPOLLIN | EPOLLET | EPOLLRDHUP);
		
		// Guardar el fd del cliente para poder rastrearlo
		clientFds_[clientFd] = true;
	}
}

/**
 * Maneja datos recibidos de un cliente
 * 
 * Se llama cuando epoll detecta EPOLLIN en un socket de cliente.
 * Esto significa que el cliente envió datos y están listos para leer.
 * 
 * ¿Por qué el bucle while?
 * - En modo edge-triggered, epoll solo notifica cuando LLEGAN nuevos datos
 * - Pero puede haber MÁS datos en el buffer del socket que no caben en un solo read()
 * - Debemos leer TODOS los datos disponibles hasta que read() retorne EAGAIN
 * - Si no, perderíamos datos y epoll no volvería a notificar (edge-triggered)
 * 
 * ¿Qué hace recv()?
 * - Lee datos del socket del cliente
 * - En modo no bloqueante:
 *   - bytesRead > 0: Leyó datos (puede ser menos que el tamaño del buffer)
 *   - bytesRead == 0: Cliente cerró la conexión
 *   - bytesRead == -1: Error o EAGAIN (no hay más datos)
 * 
 * ¿Por qué EAGAIN es normal?
 * - Significa "no hay más datos disponibles en este momento"
 * - En modo no bloqueante, esto es esperado cuando leemos todo
 * - No es un error, simplemente ya leímos todos los datos disponibles
 */
void ServerManager::handleClientData(int clientFd) {
	char buffer[4096];  // Buffer para leer datos (4KB a la vez)
	ssize_t bytesRead;
	
	// Leer TODOS los datos disponibles (crucial en edge-triggered)
	while (true) {
		// recv() lee datos del socket
		// En modo no bloqueante, retorna inmediatamente
		bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
		
		if (bytesRead > 0) {
			// ¡Datos recibidos! Imprimirlos todos
			// Null-terminate para seguridad (aunque usamos write para imprimir todos los bytes)
			buffer[bytesRead] = '\0';
			
			// Imprimir todos los bytes recibidos
			std::cout << "=== Received " << bytesRead << " bytes from client " << clientFd << " ===" << std::endl;
			std::cout.write(buffer, bytesRead);  // write() imprime exactamente 'bytesRead' bytes
			std::cout << std::endl;
			std::cout << "=== End of data ===" << std::endl;
			
			// Continuar el bucle para leer más datos si hay
		}
		else if (bytesRead == 0) {
			// bytesRead == 0 significa que el cliente cerró la conexión
			// (el cliente hizo close() o cerró su programa)
			handleClientDisconnect(clientFd);
			break;
		}
		else {
			// bytesRead == -1: Error o no hay más datos
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				// EAGAIN: No hay más datos disponibles (normal en modo no bloqueante)
				// Ya leímos todos los datos que había en el buffer
				// Salir del bucle, epoll nos notificará cuando lleguen más datos
				break;
			}
			else {
				// Error real (no EAGAIN)
				std::cerr << "Error reading from client " << clientFd << ": " << strerror(errno) << std::endl;
				handleClientDisconnect(clientFd);
				break;
			}
		}
	}
}

/**
 * Maneja la desconexión de un cliente
 * 
 * Se llama cuando:
 * - El cliente cierra la conexión (read() retorna 0)
 * - Hay un error en el socket (EPOLLERR)
 * - El cliente cierra su extremo (EPOLLRDHUP)
 * 
 * Proceso de limpieza:
 * 1. Remover el socket de epoll (ya no queremos monitorearlo)
 * 2. Cerrar el file descriptor (liberar recursos del kernel)
 * 3. Remover del mapa de clientes (limpiar nuestra estructura de datos)
 * 
 * ¿Por qué es importante limpiar?
 * - Los file descriptors son recursos limitados del sistema
 * - Si no los cerramos, se agotan (límite típico: 1024 por proceso)
 * - Si no los removemos de epoll, epoll seguirá monitoreando sockets muertos
 */
void ServerManager::handleClientDisconnect(int clientFd) {
	std::cout << "Client " << clientFd << " disconnected" << std::endl;
	
	// Remover de epoll (ya no queremos monitorear este socket)
	epoll_.removeFd(clientFd);
	
	// Cerrar el socket (liberar el file descriptor)
	close(clientFd);
	
	// Remover del mapa de clientes (limpiar nuestra estructura de datos)
	clientFds_.erase(clientFd);
}

