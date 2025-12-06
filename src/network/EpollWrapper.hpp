#ifndef EPOLLWRAPPER_HPP
#define EPOLLWRAPPER_HPP

#include <sys/epoll.h>
#include <vector>

/**
 * EpollWrapper: Wrapper para la API de epoll de Linux
 * 
 * ¿Qué es epoll?
 * - Es un mecanismo de I/O no bloqueante muy eficiente en Linux
 * - Permite monitorear múltiples file descriptors (sockets) simultáneamente
 * - En lugar de hacer polling (preguntar constantemente), el kernel te avisa
 *   cuando hay eventos (datos listos para leer, conexiones nuevas, etc.)
 * 
 * ¿Por qué es mejor que select/poll?
 * - Escala mejor con muchos sockets (O(1) vs O(n))
 * - Solo te notifica de los sockets que realmente tienen eventos
 * - Más eficiente en memoria y CPU
 */
class EpollWrapper {
private:
	// No copy constructor or assignment operator (C++98 style)
	EpollWrapper(const EpollWrapper&);
	EpollWrapper& operator=(const EpollWrapper&);

	//Es el file descriptor que representa la instancia de epoll en el kernel.
	int epollFd_;  // El file descriptor de epoll (creado por epoll_create1)
	//Todas las instancias comparten el mismo valor.
	static const int MAX_EVENTS = 64;  // Máximo de eventos que podemos recibir en una llamada

public:
	EpollWrapper();
	~EpollWrapper();

	/**
	 * Agrega un file descriptor (socket) a epoll para monitorearlo
	 * @param fd: El file descriptor a monitorear (ej: socket de escucha o cliente)
	 * @param events: Qué eventos queremos monitorear:
	 *   - EPOLLIN: Hay datos listos para leer
	 *   - EPOLLOUT: El socket está listo para escribir
	 *   - EPOLLET: Modo "edge-triggered" (solo notifica cuando cambia el estado)
	 *   - EPOLLRDHUP: El cliente cerró la conexión
	 */
	void addFd(int fd, uint32_t events);
	
	/**
	 * Modifica los eventos que estamos monitoreando para un fd existente
	 * Útil cuando quieres cambiar de solo leer a leer+escribir, por ejemplo
	 */
	void modifyFd(int fd, uint32_t events);
	
	/**
	 * Elimina un file descriptor de epoll
	 * Se llama cuando un cliente se desconecta o ya no necesitamos monitorearlo
	 */
	void removeFd(int fd);
	
	/**
	 * Espera a que ocurran eventos en los sockets monitoreados
	 * 
	 * @param events: Vector donde se guardarán los eventos que ocurrieron
	 * @param timeout: Tiempo máximo de espera en milisegundos
	 *                 -1 = esperar indefinidamente hasta que haya un evento
	 *                 0 = no esperar, retornar inmediatamente
	 *                 >0 = esperar ese tiempo máximo
	 * @return: Número de eventos que ocurrieron (0 si timeout, -1 si error)
	 * 
	 * IMPORTANTE: Esta función BLOQUEA hasta que haya eventos o timeout
	 * Pero como los sockets están en modo no bloqueante, después de recibir
	 * el evento, las operaciones (read/accept) no bloquean.
	 */
	int wait(std::vector<struct epoll_event>& events, int timeout = -1);
	
	// Get the epoll file descriptor
	int getEpollFd() const
	{
		return epollFd_;
	}
};

#endif // EPOLLWRAPPER_HPP

