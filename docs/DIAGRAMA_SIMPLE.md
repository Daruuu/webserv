# Diagrama Simple del Flujo del Servidor

##  Vista Rápida

```
┌─────────────────────────────────────────────────────────────┐
│                         INICIO                                │
│                    (main.cpp línea 15)                       │
└───────────────────────────┬─────────────────────────────────┘
                            │
                            ▼
            ┌───────────────────────────────┐
            │  1. Configurar señales        │
            │     signal(SIGPIPE, SIG_IGN)  │
            └───────────────┬───────────────┘
                            │
                            ▼
            ┌───────────────────────────────┐
            │  2. Crear ServerManager       │
            │     ServerManager server;     │
            │     ├─> TcpListener          │
            │     └─> EpollWrapper         │
            └───────────────┬───────────────┘
                            │
                            ▼
            ┌───────────────────────────────┐
            │  3. Iniciar servidor          │
            │     server.start("127.0.0.1", 8080) │
            │     ├─> Crear socket          │
            │     ├─> bind() a puerto       │
            │     ├─> listen()              │
            │     └─> Registrar en epoll    │
            └───────────────┬───────────────┘
                            │
                            ▼
            ┌───────────────────────────────┐
            │  4. BUCLE PRINCIPAL           │
            │     server.run()              │
            │                              │
            │     ┌────────────────────┐   │
            │     │  A) ESPERAR        │   │
            │     │  epoll_wait()      │◄──┼──┐
            │     │  (BLOQUEA AQUÍ)    │   │  │
            │     └─────────┬──────────┘   │  │
            │               │              │  │
            │               ▼              │  │
            │     ┌────────────────────┐  │  │
            │     │  B) PROCESAR        │  │  │
            │     │  eventos recibidos  │  │  │
            │     │                     │  │  │
            │     │  ¿Nueva conexión?   │  │  │
            │     │  └─> accept()       │  │  │
            │     │                     │  │  │
            │     │  ¿Datos recibidos?  │  │  │
            │     │  └─> recv()          │  │  │
            │     │                     │  │  │
            │     │  ¿Desconexión?      │  │  │
            │     │  └─> close()         │  │  │
            │     └─────────┬────────────┘  │  │
            │               │              │  │
            │               └──────────────┼──┘
            │                              │
            └──────────────────────────────┘
```

---

## Ciclo de Vida de una Conexión

```
CLIENTE                    SERVIDOR
   │                          │
   │─── connect() ───────────>│
   │                          │
   │                          │ epoll detecta EPOLLIN
   │                          │ en socket de escucha
   │                          │
   │                          │ handleNewConnection()
   │                          │ ├─> accept() → clientFd
   │                          │ ├─> epoll_.addFd(clientFd)
   │                          │ └─> clientFds_[clientFd] = true
   │                          │
   │<─── Conexión aceptada ───│
   │                          │
   │─── "GET / HTTP/1.1" ────>│
   │                          │
   │                          │ epoll detecta EPOLLIN
   │                          │ en clientFd
   │                          │
   │                          │ handleClientData(clientFd)
   │                          │ ├─> recv() → datos
   │                          │ └─> Imprimir datos
   │                          │
   │<─── (procesar request) ───│
   │                          │
   │─── close() ─────────────>│
   │                          │
   │                          │ epoll detecta EPOLLRDHUP
   │                          │
   │                          │ handleClientDisconnect(clientFd)
   │                          │ ├─> epoll_.removeFd(clientFd)
   │                          │ ├─> close(clientFd)
   │                          │ └─> clientFds_.erase(clientFd)
   │                          │
```

---

##  Estructura de Archivos y Responsabilidades

```
src/
├── main.cpp
│   └─> Punto de entrada, configuración inicial
│
└── network/
    ├── ServerManager.hpp/cpp
    │   └─> Orquesta todo: bucle principal, manejo de eventos
    │
    ├── TcpListener.hpp/cpp
    │   └─> Socket de escucha: socket(), bind(), listen(), accept()
    │
    └── EpollWrapper.hpp/cpp
        └─> Sistema de eventos: epoll_create1(), epoll_ctl(), epoll_wait()
```

---

##  Secuencia de Llamadas (Números de Línea)

### Inicialización
1. `main.cpp:15` → `main()` inicia
2. `main.cpp:31` → `signal(SIGPIPE, SIG_IGN)`
3. `main.cpp:40` → `ServerManager server;`
   - `ServerManager.cpp:10` → Constructor
   - `TcpListener.cpp:11` → Constructor TcpListener
   - `EpollWrapper.cpp:17` → Constructor EpollWrapper
4. `main.cpp:53` → `server.start("127.0.0.1", 8080)`
   - `ServerManager.cpp:36` → `start()`
   - `ServerManager.cpp:38` → `listener_.bindAndListen()`
     - `TcpListener.cpp:107` → `bindAndListen()`
     - `TcpListener.cpp:112` → `createSocket()`
     - `TcpListener.cpp:30` → `socket()`
     - `TcpListener.cpp:128` → `bind()`
     - `TcpListener.cpp:136` → `listen()`
   - `ServerManager.cpp:43` → `epoll_.addFd()`
     - `EpollWrapper.cpp:45` → `addFd()`
5. `main.cpp:70` → `server.run()`

### Bucle Principal
6. `ServerManager.cpp:80` → `run()` - bucle while(true)
7. `ServerManager.cpp:90` → `epoll_.wait()` ⏸️ **BLOQUEA AQUÍ**
   - `EpollWrapper.cpp:108` → `wait()`
   - `EpollWrapper.cpp:115` → `epoll_wait()` ⏸️
8. `ServerManager.cpp:98` → Procesar eventos
9. `ServerManager.cpp:102` → Si es listener → `handleNewConnection()`
   - `ServerManager.cpp:135` → `handleNewConnection()`
   - `ServerManager.cpp:138` → `listener_.acceptConnection()`
     - `TcpListener.cpp:166` → `acceptConnection()`
     - `TcpListener.cpp:179` → `accept()`
   - `ServerManager.cpp:150` → `epoll_.addFd(clientFd)`
10. `ServerManager.cpp:106` → Si es cliente → `handleClientData()`
    - `ServerManager.cpp:181` → `handleClientData()`
    - `ServerManager.cpp:189` → `recv()`
11. `ServerManager.cpp:110` → Si hay error → `handleClientDisconnect()`
    - `ServerManager.cpp:246` → `handleClientDisconnect()`
    - `ServerManager.cpp:250` → `epoll_.removeFd()`
    - `ServerManager.cpp:253` → `close()`
12. Vuelve al paso 7 (epoll_wait)

---

##  Conceptos en 30 Segundos

1. **Epoll**: Sistema del kernel que nos avisa cuando hay eventos (conexiones, datos)
2. **No Bloqueante**: Las operaciones retornan inmediatamente, no esperan
3. **Edge-Triggered**: Solo notifica cuando cambia el estado (más eficiente)
4. **Event Loop**: Bucle que espera eventos y los procesa
5. **File Descriptor (fd)**: Número que identifica un socket/conexión

---

## Cómo Empezar a Leer el Código

### Opción 1: De Arriba Hacia Abajo (Recomendado)
1. `main.cpp` - Entiende el inicio
2. `ServerManager.hpp` - Ve la estructura
3. `ServerManager.cpp` - Ve la implementación
4. `TcpListener.cpp` - Entiende los sockets
5. `EpollWrapper.cpp` - Entiende epoll

### Opción 2: Siguiendo el Flujo
1. `main.cpp:15` - Inicio
2. `main.cpp:40` - Creación
3. `main.cpp:53` - Inicio del servidor
4. `ServerManager.cpp:36` - start()
5. `ServerManager.cpp:80` - run()
6. `ServerManager.cpp:90` - epoll_wait()
7. `ServerManager.cpp:98` - Procesar eventos

---

##  Tips para Debugging

- **Punto de entrada**: `main.cpp:15`
- **Bucle principal**: `ServerManager.cpp:86`
- **Único bloqueo**: `EpollWrapper.cpp:115` (epoll_wait)
- **Nueva conexión**: `ServerManager.cpp:135`
- **Datos recibidos**: `ServerManager.cpp:181`
- **Desconexión**: `ServerManager.cpp:246`

---


