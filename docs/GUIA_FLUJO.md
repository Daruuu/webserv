# 🗺️ Guía de Flujo del Servidor Web

Esta guía te ayudará a entender cómo funciona el servidor web paso a paso, desde que se inicia hasta que maneja conexiones.

---

## 📋 Índice

1. [Arquitectura General](#arquitectura-general)
2. [Flujo de Inicialización](#flujo-de-inicialización)
3. [Flujo del Bucle Principal](#flujo-del-bucle-principal)
4. [Flujo de una Conexión Nueva](#flujo-de-una-conexión-nueva)
5. [Flujo de Recepción de Datos](#flujo-de-recepción-de-datos)
6. [Flujo de Desconexión](#flujo-de-desconexión)
7. [Orden de Lectura Recomendado](#orden-de-lectura-recomendado)

---

## 🏗️ Arquitectura General

```
┌─────────────────────────────────────────────────────────────┐
│                        main.cpp                              │
│  - Punto de entrada                                          │
│  - Configura señales                                         │
│  - Crea ServerManager                                        │
│  - Inicia servidor                                           │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                    ServerManager                             │
│  ┌──────────────┐         ┌──────────────┐                  │
│  │ TcpListener  │         │ EpollWrapper │                  │
│  │              │         │              │                  │
│  │ - Socket     │         │ - epoll_fd   │                  │
│  │   de escucha │         │ - Monitorea  │                  │
│  │ - bind()     │         │   eventos    │                  │
│  │ - listen()   │         │ - wait()     │                  │
│  │ - accept()   │         │              │                  │
│  └──────────────┘         └──────────────┘                  │
│                                                               │
│  - Bucle principal (run())                                   │
│  - Maneja conexiones                                         │
│  - Maneja datos recibidos                                    │
│  - Maneja desconexiones                                      │
└─────────────────────────────────────────────────────────────┘
```

---

## 🚀 Flujo de Inicialización

### Paso 1: main() - Configuración Inicial

**Archivo:** `src/main.cpp` (líneas 15-31)

```
1. signal(SIGPIPE, SIG_IGN)
   └─> Ignora señales SIGPIPE para evitar crashes

2. ServerManager server;
   └─> Crea el objeto ServerManager
       ├─> Constructor de ServerManager (línea 10)
       │   └─> Inicializa (vacío por ahora)
       ├─> Constructor de TcpListener (línea 11 en TcpListener.cpp)
       │   └─> listenFd_ = -1, isBound_ = false
       └─> Constructor de EpollWrapper (línea 17 en EpollWrapper.cpp)
           └─> epoll_create1(0) → crea epoll_fd
```

### Paso 2: server.start() - Configuración del Servidor

**Archivo:** `src/network/ServerManager.cpp` (líneas 36-46)

```
server.start("127.0.0.1", 8080)
│
├─> listener_.bindAndListen("127.0.0.1", 8080)
│   │
│   ├─> createSocket() (línea 30 en TcpListener.cpp)
│   │   ├─> socket(AF_INET, SOCK_STREAM, 0) → listenFd_
│   │   ├─> setSocketOptions() → SO_REUSEADDR
│   │   └─> setNonBlocking() → O_NONBLOCK
│   │
│   ├─> Preparar sockaddr_in (línea 115)
│   │   ├─> sin_family = AF_INET
│   │   ├─> sin_port = htons(8080)
│   │   └─> sin_addr = inet_pton("127.0.0.1")
│   │
│   ├─> bind(listenFd_, &addr, sizeof(addr)) (línea 128)
│   │   └─> Enlaza socket a 127.0.0.1:8080
│   │
│   └─> listen(listenFd_, SOMAXCONN) (línea 136)
│       └─> Pone socket en modo escucha
│
└─> epoll_.addFd(listener_.getFd(), EPOLLIN | EPOLLET) (línea 43)
    │
    └─> EpollWrapper::addFd() (línea 45 en EpollWrapper.cpp)
        ├─> Crea epoll_event
        ├─> ev.events = EPOLLIN | EPOLLET
        ├─> ev.data.fd = listenFd_
        └─> epoll_ctl(epollFd_, EPOLL_CTL_ADD, listenFd_, &ev)
            └─> Registra socket de escucha en epoll
```

**Resultado:** El servidor está listo y escuchando en `127.0.0.1:8080`

---

## 🔄 Flujo del Bucle Principal

### Paso 3: server.run() - El Corazón del Servidor

**Archivo:** `src/network/ServerManager.cpp` (líneas 80-117)

```
┌─────────────────────────────────────────────────────────────┐
│                    BUCLE INFINITO                            │
│                                                               │
│  while (true) {                                              │
│                                                               │
│    ┌─────────────────────────────────────────────────────┐  │
│    │  PASO 1: ESPERAR EVENTOS (BLOQUEA AQUÍ)              │  │
│    │                                                       │  │
│    │  numEvents = epoll_.wait(events, -1)                 │  │
│    │    │                                                  │  │
│    │    └─> EpollWrapper::wait() (línea 108)              │  │
│    │         │                                             │  │
│    │         ├─> events.resize(MAX_EVENTS)                │  │
│    │         │                                             │  │
│    │         └─> epoll_wait(epollFd_, &events[0], ...)    │  │
│    │              │                                        │  │
│    │              └─> ⏸️ BLOQUEA hasta que haya eventos   │  │
│    │                     o timeout                        │  │
│    │                                                       │  │
│    │  ⚠️ ESTA ES LA ÚNICA LÍNEA QUE BLOQUEA               │  │
│    └─────────────────────────────────────────────────────┘  │
│                                                               │
│    ┌─────────────────────────────────────────────────────┐  │
│    │  PASO 2: PROCESAR EVENTOS                            │  │
│    │                                                       │  │
│    │  for (int i = 0; i < numEvents; ++i) {              │  │
│    │    fd = events[i].data.fd                             │  │
│    │                                                       │  │
│    │    ┌─────────────────────────────────────────────┐  │  │
│    │    │ ¿Es el socket de escucha?                   │  │  │
│    │    │ if (fd == listener_.getFd())                │  │  │
│    │    │   └─> handleNewConnection()                 │  │  │
│    │    └─────────────────────────────────────────────┘  │  │
│    │                                                       │  │
│    │    ┌─────────────────────────────────────────────┐  │  │
│    │    │ ¿Es un cliente con datos?                   │  │  │
│    │    │ if (events[i].events & EPOLLIN)             │  │  │
│    │    │   └─> handleClientData(fd)                   │  │  │
│    │    └─────────────────────────────────────────────┘  │  │
│    │                                                       │  │
│    │    ┌─────────────────────────────────────────────┐  │  │
│    │    │ ¿Hubo error o desconexión?                   │  │  │
│    │    │ if (events[i].events & (EPOLLERR|EPOLLHUP))  │  │  │
│    │    │   └─> handleClientDisconnect(fd)             │  │  │
│    │    └─────────────────────────────────────────────┘  │  │
│    │  }                                                   │  │
│    └─────────────────────────────────────────────────────┘  │
│                                                               │
│    ┌─────────────────────────────────────────────────────┐  │
│    │  PASO 3: REPETIR                                    │  │
│    │                                                       │  │
│    │  Vuelve al inicio del while (línea 86)               │  │
│    └─────────────────────────────────────────────────────┘  │
│                                                               │
│  }                                                             │
└─────────────────────────────────────────────────────────────┘
```

**Puntos clave:**
- ⏸️ **Solo bloquea en `epoll_wait()`** cuando no hay eventos
- ⚡ **Todo lo demás es no bloqueante** (accept, read, write)
- 🔄 **El bucle se repite indefinidamente** hasta que se termine el proceso

---

## 🔌 Flujo de una Conexión Nueva

### Cuando un Cliente se Conecta

**Archivo:** `src/network/ServerManager.cpp` (líneas 135-155)

```
Cliente hace: telnet 127.0.0.1 8080
│
└─> Kernel recibe conexión TCP
    │
    └─> epoll detecta EPOLLIN en listenFd_
        │
        └─> epoll_wait() retorna con evento
            │
            └─> ServerManager::run() detecta:
                if (fd == listener_.getFd())
                │
                └─> handleNewConnection()
                    │
                    ┌──────────────────────────────────────┐
                    │  BUCLE: Aceptar TODAS las conexiones  │
                    │  pendientes (importante en EPOLLET)   │
                    │                                        │
                    │  while (true) {                       │
                    │    clientFd = listener_.acceptConnection() │
                    │      │                                 │
                    │      └─> TcpListener::acceptConnection() │
                    │           (línea 166 en TcpListener.cpp) │
                    │           │                            │
                    │           ├─> accept(listenFd_, ...)  │
                    │           │   └─> Retorna nuevo fd    │
                    │           │                            │
                    │           ├─> fcntl(clientFd, ...)    │
                    │           │   └─> Pone en no bloqueante│
                    │           │                            │
                    │           └─> return clientFd          │
                    │                                        │
                    │    if (clientFd == -1)                │
                    │      break;  // No hay más conexiones │
                    │                                        │
                    │    epoll_.addFd(clientFd, ...)        │
                    │      │                                 │
                    │      └─> Registra cliente en epoll    │
                    │          con EPOLLIN | EPOLLET |      │
                    │          EPOLLRDHUP                    │
                    │                                        │
                    │    clientFds_[clientFd] = true;        │
                    │      └─> Guarda fd en mapa            │
                    │  }                                     │
                    └──────────────────────────────────────┘
```

**Puntos clave:**
- 🔄 **Bucle while**: En modo edge-triggered, debemos aceptar TODAS las conexiones pendientes
- ⚡ **No bloqueante**: `accept()` retorna inmediatamente (EAGAIN si no hay más)
- 📝 **Registro**: Cada nuevo cliente se registra en epoll para monitorear sus datos

---

## 📨 Flujo de Recepción de Datos

### Cuando un Cliente Envía Datos

**Archivo:** `src/network/ServerManager.cpp` (líneas 181-226)

```
Cliente envía: "GET / HTTP/1.1\r\n..."
│
└─> Kernel recibe datos en el socket del cliente
    │
    └─> epoll detecta EPOLLIN en clientFd
        │
        └─> epoll_wait() retorna con evento
            │
            └─> ServerManager::run() detecta:
                if (events[i].events & EPOLLIN)
                │
                └─> handleClientData(clientFd)
                    │
                    ┌──────────────────────────────────────┐
                    │  BUCLE: Leer TODOS los datos         │
                    │  disponibles (crucial en EPOLLET)    │
                    │                                        │
                    │  while (true) {                       │
                    │    bytesRead = recv(clientFd, ...)   │
                    │      │                                 │
                    │      ├─> bytesRead > 0                │
                    │      │   └─> Datos recibidos          │
                    │      │       └─> Imprimir datos       │
                    │      │                                 │
                    │      ├─> bytesRead == 0               │
                    │      │   └─> Cliente cerró conexión   │
                    │      │       └─> handleClientDisconnect() │
                    │      │                                 │
                    │      └─> bytesRead == -1              │
                    │          ├─> errno == EAGAIN          │
                    │          │   └─> No hay más datos     │
                    │          │       └─> break            │
                    │          │                            │
                    │          └─> Otro error               │
                    │              └─> handleClientDisconnect() │
                    │  }                                     │
                    └──────────────────────────────────────┘
```

**Puntos clave:**
- 🔄 **Bucle while**: En modo edge-triggered, debemos leer TODOS los datos disponibles
- ⚡ **No bloqueante**: `recv()` retorna inmediatamente
- 📊 **EAGAIN**: Significa "no hay más datos ahora", es normal, no es error
- 📝 **bytesRead == 0**: El cliente cerró la conexión

---

## 🔌 Flujo de Desconexión

### Cuando un Cliente se Desconecta

**Archivo:** `src/network/ServerManager.cpp` (líneas 246-257)

```
Cliente cierra conexión (Ctrl+C en telnet, cierra navegador, etc.)
│
└─> Kernel detecta cierre
    │
    ├─> Opción 1: EPOLLRDHUP en epoll_wait()
    │   └─> ServerManager::run() detecta:
    │       if (events[i].events & EPOLLRDHUP)
    │       └─> handleClientDisconnect(fd)
    │
    ├─> Opción 2: EPOLLERR en epoll_wait()
    │   └─> ServerManager::run() detecta:
    │       if (events[i].events & EPOLLERR)
    │       └─> handleClientDisconnect(fd)
    │
    └─> Opción 3: recv() retorna 0
        └─> handleClientData() detecta:
            if (bytesRead == 0)
            └─> handleClientDisconnect(fd)
                │
                └─> handleClientDisconnect(clientFd)
                    │
                    ├─> epoll_.removeFd(clientFd)
                    │   └─> Remueve de epoll (línea 80)
                    │
                    ├─> close(clientFd)
                    │   └─> Cierra el file descriptor
                    │
                    └─> clientFds_.erase(clientFd)
                        └─> Limpia el mapa
```

**Puntos clave:**
- 🧹 **Limpieza completa**: Remover de epoll, cerrar fd, limpiar mapa
- ⚠️ **Importante**: Si no limpiamos, se agotan los file descriptors

---

## 📚 Orden de Lectura Recomendado

Para entender el código en orden, sigue esta secuencia:

### 1. **Inicio: main.cpp**
   - Líneas 15-79
   - Entiende cómo se inicia todo

### 2. **Arquitectura: ServerManager.hpp**
   - Líneas 1-97
   - Ve la estructura general y las responsabilidades

### 3. **Inicialización: ServerManager.cpp**
   - Líneas 10-46 (constructor y start)
   - Cómo se configura el servidor

### 4. **Socket de Escucha: TcpListener**
   - `TcpListener.hpp` completo
   - `TcpListener.cpp` líneas 11-144
   - Cómo se crea y configura el socket

### 5. **Sistema de Eventos: EpollWrapper**
   - `EpollWrapper.hpp` completo
   - `EpollWrapper.cpp` completo
   - Cómo funciona epoll

### 6. **Bucle Principal: ServerManager::run()**
   - `ServerManager.cpp` líneas 80-117
   - El corazón del servidor

### 7. **Manejo de Conexiones: handleNewConnection()**
   - `ServerManager.cpp` líneas 135-155
   - `TcpListener.cpp` líneas 166-204 (acceptConnection)

### 8. **Manejo de Datos: handleClientData()**
   - `ServerManager.cpp` líneas 181-226
   - Cómo se leen los datos

### 9. **Limpieza: handleClientDisconnect()**
   - `ServerManager.cpp` líneas 246-257
   - Cómo se limpian las conexiones

---

## 🎯 Diagrama de Flujo Completo

```
                    INICIO
                      │
                      ▼
              ┌───────────────┐
              │   main()      │
              │               │
              │ 1. signal()   │
              │ 2. ServerManager │
              │ 3. server.start()│
              └───────┬───────┘
                      │
                      ▼
         ┌────────────────────────┐
         │  ServerManager::start()│
         │                        │
         │ 1. listener_.bindAndListen() │
         │    ├─> socket()        │
         │    ├─> bind()          │
         │    └─> listen()        │
         │                        │
         │ 2. epoll_.addFd()      │
         └───────────┬────────────┘
                     │
                     ▼
         ┌────────────────────────┐
         │  ServerManager::run()   │
         │                        │
         │  ┌──────────────────┐  │
         │  │ while (true) {   │  │
         │  │                 │  │
         │  │ 1. epoll_wait() │◄─┼──┐
         │  │    (BLOQUEA)     │  │  │
         │  │                 │  │  │
         │  │ 2. Procesar     │  │  │
         │  │    eventos      │  │  │
         │  │                 │  │  │
         │  │    ┌──────────┐ │  │  │
         │  │    │ ¿Listener?│ │  │  │
         │  │    │ └─> handleNewConnection() │
         │  │    │           │ │  │  │
         │  │    │ ¿Cliente? │ │  │  │
         │  │    │ └─> handleClientData() │
         │  │    │           │ │  │  │
         │  │    │ ¿Error?   │ │  │  │
         │  │    │ └─> handleClientDisconnect() │
         │  │    └──────────┘ │  │  │
         │  │                 │  │  │
         │  │ 3. Repetir      │  │  │
         │  │ }               │──┘  │
         │  └──────────────────┘    │
         └───────────────────────────┘
```

---

## 🔑 Conceptos Clave

### 1. **I/O No Bloqueante**
- Todos los sockets están en modo `O_NONBLOCK`
- `accept()`, `read()`, `write()` retornan inmediatamente
- Si no pueden completarse, retornan `EAGAIN` (no es error)

### 2. **Edge-Triggered (EPOLLET)**
- Epoll solo notifica cuando el estado **cambia**
- Debemos leer/aceptar **TODOS** los datos/conexiones disponibles
- Si no, perdemos datos y epoll no vuelve a notificar

### 3. **Event Loop (Bucle de Eventos)**
- Solo hay **UNA** llamada bloqueante: `epoll_wait()`
- Todo lo demás es no bloqueante
- Puede manejar miles de clientes con un solo hilo

### 4. **File Descriptors**
- Cada conexión tiene un `fd` único
- Debemos limpiarlos cuando se desconectan
- Si no, se agotan (límite típico: 1024)

---

## 🐛 Puntos de Debug Recomendados

1. **Después de `epoll_wait()`**: Ver qué eventos llegaron
2. **En `handleNewConnection()`**: Ver qué clientes se conectan
3. **En `handleClientData()`**: Ver qué datos se reciben
4. **En `handleClientDisconnect()`**: Ver qué clientes se desconectan

---

