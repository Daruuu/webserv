# Allowed Functions - C++98

Esta es la lista completa de funciones del sistema permitidas para el proyecto webserv. Todas deben ser implementadas usando C++98.

---

## 📋 Índice por Categoría

- [Process Management](#process-management)
- [File Operations](#file-operations)
- [Directory Operations](#directory-operations)
- [Networking - Sockets](#networking---sockets)
- [Networking - Utils](#networking---utils)
- [I/O Multiplexing](#io-multiplexing)
- [IPC (Inter-Process Communication)](#ipc-inter-process-communication)
- [Error Handling](#error-handling)

---

## 🔄 Process Management

Funciones para gestión de procesos y señales.

| Función | Descripción | Header |
|---------|-------------|--------|
| `execve` | Ejecuta un programa | `<unistd.h>` |
| `fork` | Crea un proceso hijo | `<unistd.h>` |
| `waitpid` | Espera a que un proceso hijo termine | `<sys/wait.h>` |
| `kill` | Envía señal a un proceso | `<signal.h>` |
| `signal` | Maneja señales | `<signal.h>` |

**Uso típico**: Ejecución de scripts CGI, manejo de procesos hijos.

---

## 📄 File Operations

Funciones para operaciones básicas con archivos.

| Función | Descripción | Header |
|---------|-------------|--------|
| `open` | Abre un archivo | `<fcntl.h>` |
| `close` | Cierra un descriptor de archivo | `<unistd.h>` |
| `read` | Lee datos de un archivo | `<unistd.h>` |
| `write` | Escribe datos en un archivo | `<unistd.h>` |
| `access` | Verifica permisos de acceso | `<unistd.h>` |
| `stat` | Obtiene información de archivo | `<sys/stat.h>` |
| `fcntl` | Manipula descriptor de archivo | `<fcntl.h>` |

**Uso típico**: Lectura de archivos estáticos, configuración, logs.

---

## 📁 Directory Operations

Funciones para trabajar con directorios.

| Función | Descripción | Header |
|---------|-------------|--------|
| `opendir` | Abre un directorio | `<dirent.h>` |
| `readdir` | Lee entradas de directorio | `<dirent.h>` |
| `closedir` | Cierra un directorio | `<dirent.h>` |
| `chdir` | Cambia el directorio actual | `<unistd.h>` |

**Uso típico**: Listado de directorios, autoindex, navegación de rutas.

---

## 🌐 Networking - Sockets

Funciones principales para comunicación de red mediante sockets.

| Función | Descripción | Header |
|---------|-------------|--------|
| `socket` | Crea un socket | `<sys/socket.h>` |
| `bind` | Asocia socket a dirección | `<sys/socket.h>` |
| `listen` | Escucha conexiones entrantes | `<sys/socket.h>` |
| `accept` | Acepta una conexión | `<sys/socket.h>` |
| `connect` | Conecta a un socket remoto | `<sys/socket.h>` |
| `send` | Envía datos por socket | `<sys/socket.h>` |
| `recv` | Recibe datos de socket | `<sys/socket.h>` |

**Uso típico**: Servidor HTTP, manejo de conexiones cliente-servidor.

---

## 🔧 Networking - Utils

Funciones auxiliares para networking.

| Función | Descripción | Header |
|---------|-------------|--------|
| `htons` | Host to Network Short (conversión endianness) | `<arpa/inet.h>` |
| `htonl` | Host to Network Long | `<arpa/inet.h>` |
| `ntohs` | Network to Host Short | `<arpa/inet.h>` |
| `ntohl` | Network to Host Long | `<arpa/inet.h>` |
| `getaddrinfo` | Obtiene información de dirección | `<netdb.h>` |
| `freeaddrinfo` | Libera memoria de getaddrinfo | `<netdb.h>` |
| `setsockopt` | Configura opciones de socket | `<sys/socket.h>` |
| `getsockname` | Obtiene dirección local de socket | `<sys/socket.h>` |
| `getprotobyname` | Obtiene protocolo por nombre | `<netdb.h>` |

**Uso típico**: Configuración de sockets, resolución de direcciones, conversión de datos de red.

---

## ⚡ I/O Multiplexing

Funciones para manejar múltiples conexiones simultáneas (elegir UNA de estas familias).

### select (POSIX - Portable)

| Función | Descripción | Header |
|---------|-------------|--------|
| `select` | Monitorea múltiples file descriptors | `<sys/select.h>` |

### poll (POSIX - Más eficiente que select)

| Función | Descripción | Header |
|---------|-------------|--------|
| `poll` | Monitorea múltiples file descriptors | `<poll.h>` |

### epoll (Linux - Muy eficiente)

| Función | Descripción | Header |
|---------|-------------|--------|
| `epoll_create` | Crea instancia de epoll | `<sys/epoll.h>` |
| `epoll_ctl` | Controla interfaz de epoll | `<sys/epoll.h>` |
| `epoll_wait` | Espera eventos en epoll | `<sys/epoll.h>` |

### kqueue (BSD/macOS - Muy eficiente)

| Función | Descripción | Header |
|---------|-------------|--------|
| `kqueue` | Crea cola de eventos | `<sys/event.h>` |
| `kevent` | Registra/obtiene eventos | `<sys/event.h>` |

**Uso típico**: Servidor no bloqueante, manejo de múltiples clientes simultáneos.

> [!IMPORTANT]
> Debes elegir **UNA** de estas técnicas de multiplexing. Se recomienda implementar soporte para al menos `select` (portable) y opcionalmente `epoll` (Linux) o `kqueue` (macOS).

---

## 🔗 IPC (Inter-Process Communication)

Funciones para comunicación entre procesos.

| Función | Descripción | Header |
|---------|-------------|--------|
| `pipe` | Crea un pipe unidireccional | `<unistd.h>` |
| `socketpair` | Crea par de sockets conectados | `<sys/socket.h>` |
| `dup` | Duplica descriptor de archivo | `<unistd.h>` |
| `dup2` | Duplica descriptor a número específico | `<unistd.h>` |

**Uso típico**: Comunicación con procesos CGI, redirección de stdin/stdout.

---

## ⚠️ Error Handling

Funciones y variables para manejo de errores.

| Función/Variable | Descripción | Header |
|------------------|-------------|--------|
| `errno` | Variable global con código de error | `<errno.h>` |
| `strerror` | Convierte errno a string | `<string.h>` |
| `gai_strerror` | Convierte error de getaddrinfo a string | `<netdb.h>` |

**Uso típico**: Diagnóstico de errores, logging, mensajes de error al cliente.

---

## 📚 Recursos Adicionales

### Man Pages
Para consultar la documentación de cualquier función:
```bash
man <función>
# Ejemplo:
man socket
man select
man epoll_create
```

### Secciones del Manual
- Sección 2: System calls
- Sección 3: Library functions

```bash
man 2 open    # System call
man 3 strerror # Library function
```

### Links Útiles
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [The Linux Programming Interface](http://man7.org/tlpi/)
- [POSIX Specifications](https://pubs.opengroup.org/onlinepubs/9699919799/)

---

## ✅ Checklist de Implementación

- [ ] Implementar manejo básico de sockets (socket, bind, listen, accept)
- [ ] Elegir e implementar sistema de I/O multiplexing (select/poll/epoll/kqueue)
- [ ] Implementar lectura/escritura de archivos
- [ ] Implementar manejo de directorios (autoindex)
- [ ] Implementar ejecución de CGI (fork, execve, pipe)
- [ ] Implementar manejo de errores robusto
- [ ] Configurar sockets como no bloqueantes (fcntl)
- [ ] Implementar manejo de señales

---

*Última actualización: 2025-11-22*
