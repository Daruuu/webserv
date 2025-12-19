## Cuerpo (Body) de la Petición HTTP

### 1. ¿Qué es el body en HTTP?

El **body** es la parte opcional del mensaje HTTP que va **después** de los headers y la línea en blanco que los separa.

- **Función principal**  
  - Transportar **datos de la aplicación**: contenido de formularios, JSON, ficheros subidos, etc.

- **Relación con el método**  
  - Métodos como **GET** normalmente no llevan body (aunque el estándar no lo prohíbe, muchos servidores lo ignoran).  
  - Métodos como **POST**, **PUT**, **PATCH** suelen usar el body para enviar datos al servidor.

- **Relación con los headers**  
  - `Content-Length` y/o `Transfer-Encoding` indican **cómo** leer el body.  
  - `Content-Type` indica **qué formato** tienen los datos del body (formulario, JSON, fichero binario, etc.).

---

### 2. Flujo general para manejar el body

Una vez parseada la start line y los headers, el servidor decide **si debe leer un body y cómo**.

- **Paso 1 – Decidir si la petición puede/debe tener body**  
  - Según el método y la semántica de tu servidor.  
  - Por ejemplo, en un **POST** a `/upload` probablemente esperas un body con datos de archivo.

- **Paso 2 – Mirar los headers relevantes**  
  - `Content-Length`: número de bytes exactos del body.  
  - `Transfer-Encoding: chunked`: body enviado en trozos (chunks).  
  - `Content-Type`: formato del contenido (formulario, JSON, etc.).

- **Paso 3 – Elegir estrategia de lectura**  
  - Si hay `Transfer-Encoding: chunked` → usar lógica de **chunks**.  
  - En caso contrario, si hay `Content-Length` → leer un número fijo de bytes.  
  - Si no hay ninguno:
    - Según el protocolo y el contexto, el body puede considerarse vacío, o puede implicar cerrar la conexión para marcar el final (menos común en HTTP/1.1 bien formado).

- **Paso 4 – Leer los datos desde el socket**  
  - Usando el **buffer temporal** y el **acumulador** (como se describió en los otros apuntes).  
  - Asegurar que se lee exactamente lo que indican `Content-Length` o los chunks.

- **Paso 5 – Entregar el body bruto a capas superiores**  
  - Al final del parseo “de nivel HTTP” tienes el body como:  
    - Una secuencia de bytes (por ejemplo, en un `std::vector<char>` o similar).  
  - Sobre ese body bruto, otra capa puede interpretar:
    - Formularios.  
    - JSON.  
    - Archivos binarios, etc.

---

### 3. Body con Content-Length

Cuando existe `Content-Length`, el servidor sabe **exactamente** cuántos bytes del body esperar.

- **Pasos lógicos**  
  - Obtener el valor de `Content-Length` y validarlo:  
    - Debe ser un entero no negativo.  
  - Leer desde el socket, usando el buffer y el acumulador, hasta reunir **exactamente** esa cantidad de bytes.  
  - Si ya había parte del body en el acumulador (porque vino junto con los headers), contarlo como parte de esos bytes.

- **Ventajas**  
  - Proceso de lectura sencillo: se sabe desde el principio cuántos bytes leer.  
  - Fácil de validar y limitar (por ejemplo, rechazar cuerpos demasiado grandes).

- **Errores típicos**  
  - `Content-Length` no numérico o negativo.  
  - El cuerpo se corta antes de llegar a esa longitud (conexión cerrada antes de tiempo).  
  - El cliente envía más datos de los declarados.

---

### 4. Body con Transfer-Encoding: chunked

Con **chunked**, el body llega en **fragmentos**; no se conoce la longitud total de antemano.

- **Estructura general**  
  - Cada chunk:  
    - Una línea con el tamaño del chunk en hexadecimal.  
    - Ese número de bytes de datos.  
    - Un salto de línea.  
  - Último chunk: tamaño `0` en hexadecimal + posible bloque final (trailers) + línea vacía.

- **Flujo de lectura**  
  - Repetir:  
    - Leer línea de tamaño.  
    - Convertir el tamaño hexadecimal a entero.  
    - Si el tamaño es **0** → fin del cuerpo.  
    - Leer exactamente ese número de bytes de datos.  
    - Leer el salto de línea que sigue.  
  - Concatenar los datos de todos los chunks en un **acumulador de body**.

- **Resultado final**  
  - A capas superiores les interesa el body **ya reconstruido**, sin los metadatos de chunked.  
  - Es decir, el servidor HTTP monta un body “continuo” con todos los chunks concatenados.

---

### 5. Tipos habituales de body según Content-Type

El **parsing semántico** del body depende mucho del `Content-Type`.

- **`application/x-www-form-urlencoded`**  
  - Usado típicamente en formularios simples (método POST).  
  - El body contiene pares `clave=valor` separados por `&`, similar a una query string.  
  - Necesita:  
    - Separar por `&`.  
    - Separar cada par por `=`.  
    - Decodificar caracteres especiales (URL-encoding).

- **`multipart/form-data`**  
  - Usado para formularios con subida de archivos.  
  - El body está dividido en **partes** separadas por un “boundary”.  
  - Cada parte tiene sus propios headers (por ejemplo, `Content-Disposition`) y su propio mini-body.  
  - Es más complejo de parsear:  
    - Hay que detectar los límites.  
    - Identificar campos de formulario y archivos.  
    - Extraer nombres de campos, nombres de archivo, tipos MIME, etc.

- **`application/json`**  
  - El body contiene un objeto/array JSON en texto.  
  - El servidor HTTP como tal solo necesita entregar el string completo;  
  - Otra capa (lógica de aplicación) parseará el JSON.

- **Contenido binario genérico**  
  - Por ejemplo, `application/octet-stream`.  
  - Los datos pueden ser cualquier cosa (ficheros binarios, blobs, etc.).  
  - El servidor HTTP solo transporta bytes; la interpretación la hace la aplicación.

---

### 6. Manejo de errores relacionados con el body

El parser del body debe ser estricto para evitar comportamientos inesperados.

- **Errores de tamaño**  
  - `Content-Length` inconsistente con los bytes realmente leídos.  
  - Cuerpo chunked mal formado (tamaños incorrectos, chunk final ausente).

- **Límites de tamaño**  
  - Para evitar abusos (DoS, uploads enormes), el servidor puede:  
    - Definir un tamaño máximo de body aceptado.  
    - Cortar la lectura y devolver un error si se supera.

- **Tiempo de lectura**  
  - Un cliente podría enviar el body muy lentamente.  
  - El servidor suele tener timeouts:  
    - Si no recibe datos en un tiempo razonable, aborta la petición.

- **Errores de formato del contenido**  
  - Inconsistencias con el `Content-Type`:  
    - Por ejemplo, `multipart/form-data` sin boundary correcto.  
    - `application/x-www-form-urlencoded` con pares mal formados.  
  - Estos errores suelen tratarse en la capa de aplicación, pero pueden reflejarse al cliente como errores 4xx.

---

### 7. Resumen

- El **body** es la parte de datos de la petición o respuesta, opcional y controlada por headers.  
- `Content-Length` y `Transfer-Encoding` indican **cómo** leer el body; `Content-Type` indica **qué es** el contenido.  
- Con `Content-Length` lees un número fijo de bytes; con chunked reconstruyes el body a partir de múltiples chunks.  
- El parser HTTP ideal entrega a la capa de aplicación un body ya reconstruido (secuencia de bytes) y metadatos suficientes (tipo, longitud, etc.).  
- Un buen manejo de errores (tamaño, tiempo, formato) es esencial para la seguridad y la robustez del servidor.


