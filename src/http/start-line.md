## Start Line HTTP, URI, Query String y Recursos Estáticos/Dinámicos

### 1. Start line de una petición HTTP

La **start line** es la primera línea de la petición HTTP. Resume qué quiere el cliente, sobre qué recurso y usando qué versión del protocolo.

- **Componentes**:  
  - **Método** (GET, POST, DELETE, etc.).  
  - **URI de la petición** (incluye ruta y, opcionalmente, **query string**).  
  - **Versión HTTP** (por ejemplo, HTTP/1.1).

- **Importancia**:  
  - Define el tipo de operación, el recurso objetivo y las reglas del protocolo.  
  - Un fallo al parsear la start line hace imposible procesar correctamente la petición.

---

### 2. Pasos lógicos para parsear la start line

Piensa en un flujo de **leer → separar → validar → guardar**.

- **Paso 1 – Leer la primera línea**  
  - Del flujo del socket, tomar la primera línea completa hasta el salto de línea.

- **Paso 2 – Comprobar estructura básica**  
  - Verificar que se pueda dividir en:
    - Método.  
    - URI (request-target, que puede incluir **query string**).  
    - Versión.  
  - Si falta algo → petición mal formada.

- **Paso 3 – Extraer y validar el método**  
  - Primera “palabra” de la línea.  
  - Comprobar si es un método soportado (GET, POST, DELETE, etc.).  
  - Si no lo es → método no permitido.

- **Paso 4 – Extraer el URI completo (ruta + query string)**  
  - Segunda “palabra” de la línea.  
  - Este valor puede contener:
    - **Ruta**: lo que va antes del `?` (por ejemplo, `/products`).  
    - **Query string**: lo que va después del `?` (por ejemplo, `?page=2&sort=asc`).  
  - A nivel de parseo:
    - Localizar el carácter `?`.  
    - Separar:
      - Parte izquierda → **ruta**.  
      - Parte derecha → **query string** (opcional; puede no existir).

- **Paso 5 – Extraer y validar la versión HTTP**  
  - Tercera “palabra” de la línea.  
  - Verificar formato y compatibilidad (por ejemplo, HTTP/1.0, HTTP/1.1).

- **Paso 6 – Validar coherencia**  
  - Método compatible con la ruta y con la configuración.  
  - Versión compatible con las normas del servidor (por ejemplo, obligación de `Host` en HTTP/1.1).

- **Paso 7 – Guardar el resultado del parseo**  
  - La estructura interna debería contener como mínimo:
    - Método.  
    - Ruta normalizada.  
    - **Query string separada en bruto** (y, opcionalmente, ya dividida en pares clave‑valor).  
    - Versión HTTP.  
  - Esta información alimenta el resto del procesamiento: resolución de rutas, controladores, CGI, permisos, etc.

---

### 3. URI, ruta y query string

El **URI** es el identificador del recurso que se está pidiendo. Para un servidor web, lo más relevante es:

- **Ruta**  
  - Parte del URI que identifica el recurso base en el servidor.  
  - Ejemplos: `/`, `/index.html`, `/users/123`, `/static/css/style.css`.  
  - Suele mapearse a:
    - Un archivo estático.  
    - Un endpoint de API o recurso dinámico.

- **Query string**  
  - Es la parte del URI que va tras el `?`.  
  - Está formada (típicamente) por pares `clave=valor` separados por `&`.  
  - Ejemplo conceptual:  
    - Ruta: `/products`  
    - Query string: `?page=2&sort=price_desc`  
  - **Propósito principal**:
    - Enviar parámetros adicionales sin modificar la ruta base.  
    - Filtrar, paginar, ordenar o aplicar opciones a un recurso.  
  - **Importancia para el servidor**:
    - Puede cambiar totalmente la respuesta aunque la ruta sea la misma.  
    - En recursos dinámicos, la lógica de negocio suele depender de estos parámetros.  
    - Se debe parsear con cuidado:
      - Separar por `&`.  
      - Separar cada par en `clave` y `valor`.  
      - Decodificar caracteres especiales (URL‑encoding).

- **Normalización y seguridad**  
  - Normalizar la ruta (eliminar `..`, `/./`, etc.) antes de mapear a disco.  
  - Tratar la query string como datos de entrada no confiables:
    - Pueden usarse en ataques (inyección en backends, por ejemplo).  
    - Nunca deben usarse directamente sin validación.

---

### 4. Archivos estáticos

- **Definición**  
  - Contenido servido tal cual está almacenado en el sistema de archivos.  
  - Ejemplos: HTML, CSS, JS, imágenes, fuentes.

- **Relación con ruta y query string**  
  - En la mayoría de configuraciones, la **ruta** se utiliza para localizar el archivo físico.  
  - La **query string**:
    - Normalmente **no cambia el archivo** que se lee del disco.  
    - Puede usarse para:
      - Técnicas de cache busting (`/style.css?v=123`).  
      - Parámetros de tracking o analítica.  
    - El servidor estático típico:
      - Ignora la query string para la búsqueda de archivo.  
      - Pero los clientes/proxies pueden tratarla como parte de la clave de caché.

- **Características**  
  - Mismo contenido para todos los usuarios (a menos que el archivo cambie en disco).  
  - Muy eficientes de servir y muy cacheables.

---

### 5. Recursos dinámicos

- **Definición**  
  - Respuestas generadas en el momento (por un script, una aplicación, un CGI, etc.).  
  - El contenido puede variar según el usuario, el tiempo, la base de datos, etc.

- **Relación con ruta y query string**  
  - La **ruta** indica qué script/handler o endpoint se ejecuta.  
  - La **query string**:
    - Proporciona parámetros que afectan a la lógica.  
    - Ejemplos:
      - Filtros: `?category=books&price_min=10`.  
      - Paginación: `?page=3&limit=20`.  
      - Parámetros de búsqueda: `?q=http+server`.  
    - En un entorno CGI o similar, esos parámetros suelen entregarse al programa de forma estándar (por ejemplo, en una variable de entorno con la query string bruta, que luego se parsea).

- **Características**  
  - El mismo URI base (misma ruta) puede devolver **respuestas distintas** según la query string.  
  - Mayor costo de CPU/IO que un archivo estático.  
  - Requiere validación y saneamiento de todos los parámetros (incluidos los de la query string).

---

### 6. Conexión entre start line, URI, query string y tipo de recurso

Flujo conceptual típico en un servidor:

- **1. Parsear start line**  
  - Extraer método, URI completo y versión.

- **2. Dividir el URI en ruta y query string**  
  - Ruta = parte antes de `?`.  
  - Query string = parte después de `?` (si existe).  
  - Guardar ruta normalizada y query string por separado.

- **3. Determinar el tipo de recurso**  
  - Usar la ruta para decidir:
    - Si se trata de un archivo estático (mapear a disco).  
    - Si se trata de un recurso dinámico (mapear a un handler/CGI).

- **4. Usar la query string según el tipo**  
  - **Recurso estático**:
    - La ruta determina el archivo.  
    - La query suele no cambiar el archivo, pero puede afectar a caché y analítica.  
  - **Recurso dinámico**:
    - Ruta decide qué programa/controlador se ejecuta.  
    - Query string alimenta la lógica de negocio (filtros, paginación, búsquedas, opciones).

- **5. Generar y enviar la respuesta**  
  - Basada en:
    - Método.  
    - Ruta.  
    - Query string.  
    - Versión HTTP.  
    - Configuración del servidor.

---

### 7. Resumen

- **Start line**: primera línea de la petición; contiene método, URI y versión.  
- **URI**: incluye ruta y, opcionalmente, **query string**; es la clave para identificar el recurso.  
- **Query string**: parte tras el `?` en el URI; transmite parámetros adicionales y es fundamental en recursos dinámicos.  
- **Archivos estáticos**: se resuelven principalmente con la ruta; la query string suele no cambiar el archivo.  
- **Recursos dinámicos**: la ruta selecciona el handler/script; la query string aporta datos que modifican la lógica y el contenido de la respuesta.


