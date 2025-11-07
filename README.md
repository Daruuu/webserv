# 🚀 Webserv: Nuestro Propio Servidor HTTP

> **"This is when you finally understand why URLs start with HTTP"**

---

## 🎯 Resumen del Proyecto

Este proyecto tiene como objetivo escribir nuestro propio **servidor HTTP desde cero en C++98**.
El servidor debe ser **compatible con navegadores web estándar** y debe implementar la **lógica subyacente del protocolo HTTP**.

La función principal de un servidor web es **almacenar, procesar y entregar páginas web** a los clientes a través del protocolo **HTTP**.

---

## ⚙️ Requisitos Mandatorios

El proyecto debe implementarse bajo el estándar **C++98** y **sin ninguna biblioteca externa**.

### 1. Requisitos de Código y Compilación

* **Lenguaje:** C++98
* **Compilador:** `c++` con los flags `-Wall -Wextra -Werror`
* **Makefile:** Debe contener las reglas `$(NAME)`, `all`, `clean`, `fclean` y `re`
* **Robustez:** El programa **no debe fallar ni terminar inesperadamente**
* **Funciones Externas Permitidas:** Solo se puede usar el conjunto de funciones C/Unix especificadas (`socket`, `select`, `poll`, `execve`, `read`, `write`, etc.)

---

### 2. Arquitectura de Red (I/O No Bloqueante)

La gestión eficiente y no bloqueante de múltiples clientes es crucial.

* **Non-Blocking I/O:** El servidor debe permanecer no bloqueante en todo momento.
* **I/O Multiplexing Único:** Solo se permite **una única llamada** (`poll()`, `select()`, `kqueue()` o `epoll()`) para todas las operaciones de I/O.
* **Doble Monitoreo:** El multiplexor debe monitorear lectura y escritura simultáneamente.
* **Control Estricto:** Nunca realizar `read` o `write` sin una notificación previa de disponibilidad.

---

### 3. Funcionalidad HTTP y Protocolo

El servidor debe ser compatible con navegadores estándar y simular el comportamiento de un servidor como **NGINX**.

* **Métodos Obligatorios:** `GET`, `POST` y `DELETE`
* **Contenido:** Capaz de servir un sitio web totalmente estático
* **Respuestas:** Códigos de estado HTTP precisos y páginas de error personalizadas
* **Archivos:** Los clientes deben poder **subir archivos**
* **CGI (Common Gateway Interface):** Soporte para ejecución de scripts (`.php`, Python, etc.) según la extensión del archivo

  > *Nota: `fork()` solo se puede usar para CGI.*

---

### 4. Archivo de Configuración

El programa debe aceptar un archivo de configuración como argumento al ejecutarse.
El formato debe estar **inspirado en la sección `server` de NGINX**.

Debe permitir configurar:

* **Puertos:** Múltiples pares `interface:port` en los que escuchar
* **Páginas de Error:** Personalizadas por código HTTP
* **Límite de Cuerpo:** Tamaño máximo permitido para los cuerpos de las peticiones (`client_max_body_size`)
* **Reglas por Ruta (location):**

  * Lista de **métodos HTTP aceptados**
  * **Redirecciones HTTP**
  * **Ruta raíz** del directorio solicitado
  * **Listado de directorios** activado/desactivado
  * **Archivo por defecto** al acceder a un directorio
  * **Ruta de almacenamiento** para los archivos subidos

---

## 🧠 Recomendaciones y Buenas Prácticas

* **Lectura Previa:** Releer los **RFCs** del protocolo HTTP (RFC 2616 y 7230)
* **Pruebas:** Usar herramientas como `telnet`, `curl`, `Postman` y `NGINX` como referencia
* **Resiliencia:** El servidor debe mantenerse operativo ante múltiples clientes y peticiones simultáneas
* **Tests Automatizados:** Escribir tests en Python o Go para verificar el comportamiento del servidor
* **Uso de IA:** Permitido para automatizar o documentar, pero **debes comprender todo el código generado**

---

## 📁 Estructura Recomendada del Proyecto

```bash
webserv/
├── src/
│   ├── main.cpp
│   ├── Server.cpp
│   ├── Request.cpp
│   ├── Response.cpp
│   ├── ConfigParser.cpp
│   └── ...
├── include/
│   ├── Server.hpp
│   ├── Request.hpp
│   ├── Response.hpp
│   ├── ConfigParser.hpp
│   └── ...
├── config/
│   └── default.conf
├── Makefile
└── README.md
```

---

## 🧩 Ejecución

```bash
make
./webserv config/default.conf
```

Luego abre en tu navegador:
👉 `http://localhost:8080`

---

## 👥 Créditos

Proyecto realizado en la **Academia 42** como parte del cursus de C++:

* Ana Medina Burgos
* Darunny Salazar
* Carles Pujades

---

## 📚 Referencias

* [RFC 2616 – HTTP/1.1 Specification](https://www.rfc-editor.org/rfc/rfc2616)
* [RFC 7230 – Message Syntax and Routing](https://www.rfc-editor.org/rfc/rfc7230)
* [NGINX Configuration Guide](https://nginx.org/en/docs/)
* [Beej’s Guide to Network Programming](https://beej.us/guide/bgnet/)
