#include "EpollWrapper.hpp"
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <errno.h>

/**
 * Constructor: Crea una instancia de epoll
 * 
 * epoll_create1(0) crea un nuevo objeto epoll en el kernel
 * Retorna un file descriptor que usaremos para todas las operaciones de epoll
 * 
 * ¿Por qué es necesario?
 * - El kernel necesita un lugar donde guardar la lista de sockets a monitorear
 * - Este fd es como un "registro" donde apuntamos qué sockets queremos vigilar
 */
EpollWrapper::EpollWrapper()
 {
	//defecto: 0, sin flags especiales para crear el objeto epoll.
	epollFd_ = epoll_create1(0);
	if (epollFd_ == -1) {
		throw std::runtime_error("Failed to create epoll instance");
	}
}

/**
 * Destructor: Cierra el file descriptor de epoll
 * Esto libera los recursos del kernel
 */
EpollWrapper::~EpollWrapper() {
	if (epollFd_ != -1) {
		close(epollFd_);
	}
}

/**
 * Agrega un socket a la lista de monitoreo de epoll
 * 
 * Proceso:
 * 1. Creamos una estructura epoll_event que describe:
 *    - Qué eventos queremos monitorear (EPOLLIN, EPOLLOUT, etc.)
 *    - Qué información guardar (en este caso, el fd del socket)
 * 2. Llamamos a epoll_ctl con EPOLL_CTL_ADD para registrar el socket
 * 
 * Después de esto, epoll_wait() nos notificará cuando ocurran esos eventos
 */
void EpollWrapper::addFd(int fd, uint32_t events) {
	struct epoll_event ev;
	ev.events = events;      // Qué eventos queremos (leer, escribir, etc.)
	ev.data.fd = fd;        // Guardamos el fd para saber qué socket es cuando recibamos el evento
	
	if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
		throw std::runtime_error("Failed to add fd to epoll");
	}
}

/**
 * Modifica los eventos que estamos monitoreando para un socket
 * 
 * Ejemplo de uso: Un socket estaba solo para leer (EPOLLIN),
 * ahora también queremos escribir (EPOLLIN | EPOLLOUT)
 */
void EpollWrapper::modifyFd(int fd, uint32_t events) {
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	
	if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
		throw std::runtime_error("Failed to modify fd in epoll");
	}
}

/**
 * Elimina un socket de la lista de monitoreo
 * 
 * Se llama cuando:
 * - Un cliente se desconecta
 * - Ya no necesitamos monitorear ese socket
 * 
 * No lanzamos excepción si falla porque el fd podría ya estar cerrado
 */
void EpollWrapper::removeFd(int fd) {
	if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, NULL) == -1) {
		// Don't throw on error, fd might already be closed
		// Just log it if needed
	}
}

/**
 * Espera a que ocurran eventos en los sockets monitoreados
 * 
 * Esta es la función MÁS IMPORTANTE de epoll. Es el corazón del servidor.
 * 
 * ¿Cómo funciona?
 * 1. El kernel revisa todos los sockets que registramos con addFd()
 * 2. Si alguno tiene datos listos, conexiones nuevas, etc., los marca
 * 3. Retorna una lista de TODOS los eventos que ocurrieron
 * 4. Nosotros procesamos cada evento (leer datos, aceptar conexiones, etc.)
 * 
 * ¿Por qué bloquea?
 * - Si no hay eventos, el kernel pone nuestro proceso a dormir
 * - Cuando hay eventos, el kernel nos despierta
 * - Esto es eficiente: no consumimos CPU preguntando constantemente
 * 
 * ¿Por qué no bloquea después?
 * - Los sockets están en modo NO BLOQUEANTE
 * - Cuando epoll nos dice "hay datos", hacemos read() que retorna inmediatamente
 * - Si no hay más datos, read() retorna EAGAIN (no bloquea)
 */
int EpollWrapper::wait(std::vector<struct epoll_event>& events, int timeout) {
	// Preparamos el vector con espacio para MAX_EVENTS eventos
	// epoll_wait necesita un array de tamaño fijo
	events.resize(MAX_EVENTS);
	
	// ESPERA aquí hasta que haya eventos o timeout
	// Esta es la única línea que bloquea en todo el servidor
	int numEvents = epoll_wait(epollFd_, &events[0], MAX_EVENTS, timeout);
	
	if (numEvents == -1) {
		// EINTR significa que una señal interrumpió la espera (normal, no es error)
		if (errno != EINTR) {
			throw std::runtime_error("epoll_wait failed");
		}
		return 0; // Interrupted by signal, but not an error
	}
	
	// Redimensionamos el vector al número real de eventos recibidos
	// (puede ser menos que MAX_EVENTS)
	events.resize(numEvents);
	return numEvents;
}

