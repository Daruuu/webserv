#include "TcpListener.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <cstring>

TcpListener::TcpListener() : listenFd_(-1), isBound_(false) {
}

TcpListener::~TcpListener() {
	if (listenFd_ != -1) {
		close(listenFd_);
	}
}

/**
 * Crea el socket TCP y lo configura
 * 
 * socket(AF_INET, SOCK_STREAM, 0):
 * - AF_INET: IPv4
 * - SOCK_STREAM: TCP (conexión confiable, ordenada)
 * - 0: Protocolo por defecto (TCP para SOCK_STREAM)
 * 
 * Retorna un file descriptor que usaremos para todas las operaciones
 */
void TcpListener::createSocket() {
	listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (listenFd_ == -1) {
		throw std::runtime_error("Failed to create socket");
	}
	
	setSocketOptions();
	setNonBlocking();
}

/**
 * Configura opciones del socket
 * 
 * SO_REUSEADDR: Permite reutilizar la dirección aunque esté en TIME_WAIT
 * 
 * ¿Por qué es necesario?
 * - Cuando cierras un servidor, el socket entra en estado TIME_WAIT (~30 seg)
 * - Sin SO_REUSEADDR, no puedes reabrir el servidor inmediatamente
 * - Con SO_REUSEADDR, puedes reutilizar la dirección inmediatamente
 * 
 * Esto es especialmente útil durante desarrollo cuando reinicias el servidor
 */
void TcpListener::setSocketOptions() {
	int opt = 1;
	// Allow address reuse to avoid "Address already in use" errors
	if (setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		close(listenFd_);
		listenFd_ = -1;
		throw std::runtime_error("Failed to set socket options");
	}
}

/**
 * Configura el socket en modo NO BLOQUEANTE usando fcntl
 * 
 * ¿Cómo funciona fcntl?
 * 1. F_GETFL: Obtiene los flags actuales del fd
 * 2. Agregamos O_NONBLOCK a esos flags
 * 3. F_SETFL: Aplica los nuevos flags
 * 
 * O_NONBLOCK: Hace que todas las operaciones I/O sean no bloqueantes
 * - accept() retorna inmediatamente (EAGAIN si no hay conexiones)
 * - read() retorna inmediatamente (EAGAIN si no hay datos)
 * - write() retorna inmediatamente (EAGAIN si el buffer está lleno)
 */
void TcpListener::setNonBlocking() {
	// Obtener flags actuales
	int flags = fcntl(listenFd_, F_GETFL, 0);
	if (flags == -1) {
		close(listenFd_);
		listenFd_ = -1;
		throw std::runtime_error("Failed to get socket flags");
	}
	
	// Agregar flag O_NONBLOCK y aplicar
	if (fcntl(listenFd_, F_SETFL, flags | O_NONBLOCK) == -1) {
		close(listenFd_);
		listenFd_ = -1;
		throw std::runtime_error("Failed to set socket to non-blocking");
	}
}

/**
 * Enlaza el socket a una dirección IP:puerto y comienza a escuchar
 * 
 * Proceso completo:
 * 1. Crea el socket (createSocket)
 * 2. Prepara la estructura sockaddr_in con la IP y puerto
 * 3. bind(): Enlaza el socket a esa dirección
 *    - Le dice al kernel: "Este socket escuchará en esta IP:puerto"
 * 4. listen(): Pone el socket en modo "escucha"
 *    - Le dice al kernel: "Acepta conexiones entrantes en este socket"
 *    - SOMAXCONN: Máximo número de conexiones en cola (típicamente 128)
 * 
 * Después de esto, el socket está listo para aceptar conexiones
 * epoll nos avisará cuando llegue una conexión nueva
 */
void TcpListener::bindAndListen(const std::string& host, int port)
{
	//Estás intentando inicializar algo que ya está inicializado
	if (isBound_) {
		throw std::runtime_error("Socket already bound");
	}
	
	createSocket();
	
	// ============================================================
	// OPCIÓN 1: Usando getaddrinfo
	// ============================================================
	// getaddrinfo es una función que convierte una dirección (IP o nombre)
	// a una estructura que el socket puede usar.
	// 
	// Ventajas:
	// - Puede resolver nombres como "localhost" a "127.0.0.1"
	// - Puede trabajar con direcciones IP directamente como "127.0.0.1"
	// - Es más flexible y portable
	
	// Paso 1: Preparar las "pistas" (hints) para getaddrinfo
	// Le decimos qué tipo de dirección queremos
	struct addrinfo hints;
	struct addrinfo *result = NULL;  // Aquí guardará el resultado
	
	// Limpiar la estructura hints (poner todo en cero)
	std::memset(&hints, 0, sizeof(hints));
	
	// Configurar qué queremos:
	hints.ai_family = AF_INET;        // Solo IPv4 (no IPv6)
	hints.ai_socktype = SOCK_STREAM;  // TCP (no UDP)
	hints.ai_flags = AI_PASSIVE;      // Para servidor
	
	// Paso 2: Convertir el puerto de número a string
	// getaddrinfo necesita el puerto como string, no como número
	std::string portStr = std::to_string(port);
	
	// Paso 3: Preparar el hostname
	// Si host está vacío, usamos NULL (significa "todas las interfaces")
	// Si host tiene valor, usamos ese valor
	const char *hostname = NULL;
	if (!host.empty()) {
		hostname = host.c_str();
	}
	
	// Paso 4: Llamar a getaddrinfo
	// Esta función hace la "magia": convierte el hostname/IP a una estructura
	// que el socket puede usar
	int status = getaddrinfo(hostname, portStr.c_str(), &hints, &result);
	
	// Paso 5: Verificar si funcionó
	// Si status != 0, hubo un error
	if (status != 0) {
		close(listenFd_);
		listenFd_ = -1;
		// gai_strerror convierte el código de error a un mensaje legible
		std::string errorMsg = "getaddrinfo failed: ";
		errorMsg += gai_strerror(status);
		throw std::runtime_error(errorMsg);
	}
	
	// Paso 6: Usar el resultado para hacer bind
	// getaddrinfo nos da una estructura con la dirección lista para usar
	// Solo necesitamos pasarla a bind()
	if (bind(listenFd_, result->ai_addr, result->ai_addrlen) == -1) {
		// Si falla, liberar la memoria ANTES de lanzar el error
		freeaddrinfo(result);
		close(listenFd_);
		listenFd_ = -1;
		throw std::runtime_error("Failed to bind socket");
	}
	
	// Paso 7: Liberar la memoria que getaddrinfo asignó
	// IMPORTANTE: Siempre hay que liberar con freeaddrinfo
	freeaddrinfo(result);
	
	// ============================================================
	// OPCIÓN 2: Usando inet_pton (CÓDIGO COMENTADO - NO SE USA)
	// ============================================================
	// Esta es una forma MÁS SIMPLE pero MENOS FLEXIBLE
	// 
	// Ventajas:
	// - Código más corto y directo
	// - Más fácil de entender para principiantes
	// 
	// Desventajas:
	// - Solo funciona con direcciones IP directas (ej: "127.0.0.1")
	// - NO puede resolver nombres como "localhost"
	// - Si le pasas "localhost", fallará
	//
	// Si quieres usar esta opción, descomenta el código de abajo
	// y comenta la OPCIÓN 1 (getaddrinfo)
	/*
	// Paso 1: Crear una estructura para guardar la dirección
	struct sockaddr_in addr;
	
	// Paso 2: Limpiar la estructura (poner todo en cero)
	std::memset(&addr, 0, sizeof(addr));
	
	// Paso 3: Configurar el tipo de dirección (IPv4)
	addr.sin_family = AF_INET;
	
	// Paso 4: Configurar el puerto
	// htons() convierte el número del puerto al formato que usa la red
	addr.sin_port = htons(port);
	
	// Paso 5: Convertir la IP de string a número
	// inet_pton() convierte "127.0.0.1" a un número que el socket entiende
	// Retorna 1 si éxito, 0 si formato inválido, -1 si error
	if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
		close(listenFd_);
		listenFd_ = -1;
		throw std::runtime_error("Invalid host address");
	}
	
	// Paso 6: Hacer bind con la dirección preparada
	if (bind(listenFd_, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		close(listenFd_);
		listenFd_ = -1;
		throw std::runtime_error("Failed to bind socket");
	}
	*/
	
	// LISTEN: Poner el socket en modo escucha
	// SOMAXCONN: Máximo de conexiones pendientes en la cola
	if (listen(listenFd_, SOMAXCONN) == -1) {
		close(listenFd_);
		listenFd_ = -1;
		throw std::runtime_error("Failed to listen on socket");
	}
	
	isBound_ = true;
	std::cout << "Server listening on " << host << ":" << port << std::endl;
}

/**
 * Acepta una nueva conexión de cliente
 * 
 * ¿Cómo funciona accept()?
 * - Toma una conexión de la cola de conexiones pendientes
 * - Crea un NUEVO socket para comunicarse con ese cliente
 * - Retorna el file descriptor de ese nuevo socket
 * 
 * ¿Por qué retorna -1 en modo no bloqueante?
 * - Si no hay conexiones en la cola, accept() no puede esperar (no bloquea)
 * - Retorna -1 y errno = EAGAIN/EWOULDBLOCK
 * - Esto es NORMAL, no es un error
 * 
 * ¿Cuándo hay conexiones disponibles?
 * - Cuando epoll nos notifica EPOLLIN en el socket de escucha
 * - Significa que hay al menos una conexión esperando ser aceptada
 * 
 * IMPORTANTE: El nuevo socket del cliente también se pone en modo no bloqueante
 * para que read()/write() no bloqueen
 */
int TcpListener::acceptConnection() {
	if (!isBound_) {
		return -1;
	}
	
	// Estructura para guardar la dirección del cliente
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	
	// ACCEPT: Acepta una conexión pendiente
	// En modo no bloqueante:
	// - Si hay conexión: retorna el fd del cliente
	// - Si NO hay conexión: retorna -1, errno = EAGAIN
	int clientFd = accept(listenFd_, (struct sockaddr*)&clientAddr, &clientLen);
	
	if (clientFd == -1) {
		// EAGAIN or EWOULDBLOCK means no connection available (non-blocking)
		// Esto es NORMAL, no es un error
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return -1;
		}
		// Other errors are real problems
		return -1;
	}
	
	// CRUCIAL: El nuevo socket del cliente también debe ser no bloqueante
	// Si no, read()/write() en ese socket bloquearían el servidor
	int flags = fcntl(clientFd, F_GETFL, 0);
	if (flags != -1) {
		fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);
	}
	
	// Mostrar información del cliente (opcional, para debugging)
	char clientIp[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
	std::cout << "New client connected from " << clientIp << std::endl;
	
	return clientFd;
}

