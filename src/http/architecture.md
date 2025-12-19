## Arquitectura básica del manejo HTTP en `webserv`

### Diagrama de clases y flujo (vista simple)

```text
             +--------------------+
             |    ServerManager   |
             |  (usa Epoll, etc.) |
             +----------+---------+
                        |
                        | crea/gestiona
                        v
                 +------+------+
                 |   Client    |   (1 por conexión TCP)
                 +------+------+
                        |
        lee/escribe     | usa
       bytes socket     v
                 +------+------+
                 |  HttpParser |
                 +------+------+
                        |
          construye     v
                 +------+------+
                 | HttpRequest |
                 +------+------+
                        |
                        | pasa la request a
                        v
               +--------+----------+
               |  RequestProcessor |
               +--------+----------+
                        |
       decide estático / CGI / error
        usa config y CGI handler
        +-------------------+------------------+
        |                                      |
        v                                      v
+---------------+                    +----------------+
|  ServerConfig |                    |   CgiHandler   |
+---------------+                    +----------------+

                        |
                        | crea
                        v
                 +------+------+
                 | HttpResponse|
                 +------+------+
                        |
                        | se serializa a texto
                        v
                 +------+------+
                 |   Client    |  (mete bytes en buffer de salida y los
                 +-------------+   manda por el socket)
```

### Resumen rápido de responsabilidades

- **ServerManager**: acepta conexiones, integra Epoll y crea/gestiona instancias de `Client`.
- **Client**: representa una conexión TCP; acumula bytes de entrada/salida, usa `HttpParser` y dispara `RequestProcessor`.
- **HttpParser**: máquina de estados que consume bytes y construye un `HttpRequest` (start line, headers, body).
- **HttpRequest**: datos ya parseados de la petición (método, path, query, headers, body).
- **RequestProcessor**: recibe `HttpRequest` + `ServerConfig`, decide estático/CGI/error y construye `HttpResponse`.
- **ServerConfig**: configuración de puertos, roots, rutas estáticas/CGI, límites, etc.
- **CgiHandler**: ejecuta programas/scripts CGI y devuelve su salida para formar la respuesta.
- **HttpResponse**: datos de la respuesta (status, headers, body), que luego se serializa y se envía al cliente.


