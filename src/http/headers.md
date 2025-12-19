## Headers HTTP, Content-Length, Chunked y Manejo de Errores

### 0. Lectura desde el socket: buffer temporal y acumulador

Antes de parsear start line, headers y cuerpo, el servidor necesita **recibir los datos crudos** del socket de forma segura y ordenada. Para eso es útil separar dos ideas: **buffer temporal** y **acumulador**.

- **Buffer temporal**  
  - Es un bloque de memoria de tamaño fijo (por ejemplo, unos cuantos KB).  
  - Se usa en cada llamada a la función de lectura del socket.  
  - Cada lectura:  
    - Llena el buffer con “un trozo” de la petición.  
    - Puede traer solo parte de la start line, parte de los headers o mezclar de todo.

- **Acumulador (por ejemplo `std::string` o `std::vector<char>`)**  
  - Es una estructura donde se **van concatenando** todos los datos leídos con el buffer temporal.  
  - Objetivo: tener en un solo sitio el mensaje (o al menos la parte que necesitamos para parsear).  
  - Flujo típico:  
    - Leer N bytes al buffer temporal.  
    - Añadir esos N bytes al final del acumulador.  
    - Repetir hasta tener, como mínimo:  
      - Toda la start line.  
      - Todos los headers (hasta la doble secuencia de salto de línea).  
      - Y, si es posible, el cuerpo completo (según `Content-Length` o chunked).

- **Ventaja de este enfoque**  
  - Separa dos problemas:  
    - **Recepción** de datos del socket (que puede llegar en trozos arbitrarios).  
    - **Parseo** (que necesita operar sobre una secuencia continua de bytes).  
  - Permite que el parser trabaje sobre una representación cómoda:  
    - Por ejemplo, una `std::string` en la que puedes buscar `\r\n\r\n`, separar líneas, etc.

- **Relación con el parseo**  
  - Primero, se llena el **acumulador** leyendo varias veces del socket con el buffer temporal.  
  - Después, sobre el contenido acumulado se ejecutan:  
    - Parseo de start line.  
    - Parseo de headers.  
    - Cálculo de cuánto cuerpo falta por leer (por `Content-Length` o chunks).  
  - Si falta cuerpo, se continúa leyendo del socket (mismo esquema de buffer + acumulador) hasta tenerlo completo.

- **En el caso de chunked**  
  - El acumulador también es útil:  
    - Se van leyendo trozos del socket.  
    - Se van acumulando.  
    - El parser va consumiendo del acumulador:  
      - Tamaño de chunk en texto.  
      - Datos del chunk.  
      - Se repite hasta encontrar el chunk de tamaño 0.

---

### 1. ¿Qué son los headers HTTP?

Los **headers** son líneas clave:valor que van después de la start line y antes del cuerpo.  
Sirven para enviar **metadatos** sobre la petición o la respuesta.

- **Ejemplos de lo que expresan**:  
  - Quién envía la petición (`User-Agent`).  
  - A qué host se dirige (`Host`).  
  - Tipo de contenido del cuerpo (`Content-Type`).  
  - Longitud del cuerpo (`Content-Length`).  
  - Cómo se transmite el cuerpo (`Transfer-Encoding: chunked`).

- **Formato general de cada header**  
  - `Nombre-Header: valor`.  
  - Un conjunto de headers termina con una **línea vacía**.

---

### 2. Pasos lógicos para parsear los headers

Imagina que ya has parseado la start line y te quedas con el “bloque” de headers como texto.

- **Paso 1 – Leer línea por línea hasta encontrar una línea vacía**  
  - Cada línea no vacía es un header.  
  - La primera línea vacía indica el **fin de los headers**.

- **Paso 2 – Verificar formato de cada header**  
  - Debe contener al menos un `:` que separe nombre y valor.  
  - Si una línea no cumple este formato → header mal formado (posible 400 Bad Request).

- **Paso 3 – Separar nombre y valor**  
  - Todo lo que va antes de `:` → **nombre del header**.  
  - Todo lo que va después → **valor bruto del header**.  
  - Normalizar:  
    - Eliminar espacios en blanco innecesarios alrededor del nombre y del valor.  
    - Tratar el nombre de forma case-insensitive internamente.

- **Paso 4 – Almacenar los headers en una estructura interna**  
  - Un mapa/diccionario: nombre → valor (o lista de valores si se repite).  
  - Ten en cuenta:  
    - Algunos headers pueden aparecer varias veces (por ejemplo, `Cookie`).  
    - Debes decidir cómo manejar duplicados (concatenar, sobrescribir, mantener lista).

- **Paso 5 – Validaciones básicas tras el parseo**  
  - Comprobar que headers obligatorios estén presentes (por ejemplo, `Host` en HTTP/1.1).  
  - Ver si hay combinaciones inválidas (por ejemplo, `Content-Length` y `Transfer-Encoding: chunked` juntos).

---

### 3. Content-Length

El header **`Content-Length`** indica el número exacto de bytes del cuerpo del mensaje.

- **Cuándo es importante**  
  - En peticiones con cuerpo (POST, PUT, etc.).  
  - En respuestas con cuerpo de longitud conocida.

- **Uso en el parseo**  
  - Si existe, te dice **cuántos bytes** debes leer después de los headers para completar el cuerpo.  
  - Pasos:  
    - Comprobar que el valor es un número entero no negativo.  
    - Convertirlo a un valor numérico interno.  
    - Leer exactamente esa cantidad de bytes del socket (usando el buffer y el acumulador).

- **Errores típicos relacionados**  
  - Valor no numérico → petición mal formada.  
  - Valor negativo → petición mal formada.  
  - Cuerpo recibido más corto que `Content-Length` → cuerpo incompleto (error de lectura o conexión).  
  - Cuerpo más largo que `Content-Length` → datos sobrantes inesperados (error de protocolo).

---

### 4. Transfer-Encoding: chunked (chunks)

Cuando se usa **`Transfer-Encoding: chunked`**, el cuerpo no se envía como un bloque continuo con longitud conocida, sino en **trozos (chunks)**.

- **Cuándo se usa**  
  - Cuando el emisor no conoce la longitud total del cuerpo al inicio.  
  - En respuestas generadas sobre la marcha (streaming, generación dinámica, etc.).

- **Estructura conceptual del cuerpo en chunked**  
  - Repetición de:  
    - Línea con el tamaño del chunk en hexadecimal.  
    - Ese número de bytes de datos.  
    - Salto de línea.  
  - Último chunk:  
    - Tamaño `0` en hexadecimal.  
    - Opcionalmente, algunos headers de “trailer”.  
    - Línea vacía final.

- **Pasos lógicos para parsear un cuerpo chunked**  
  - Paso 1: Leer la línea del tamaño del chunk (en texto, desde el acumulador).  
  - Paso 2: Interpretar ese número como hexadecimal → tamaño del chunk en bytes.  
  - Paso 3: Si el tamaño es 0 → fin del cuerpo.  
  - Paso 4: Leer exactamente ese número de bytes como datos del chunk.  
  - Paso 5: Leer el salto de línea que sigue al chunk.  
  - Paso 6: Repetir el ciclo hasta encontrar el chunk de tamaño 0.

- **Relación con Content-Length**  
  - Si `Transfer-Encoding: chunked` está presente, normalmente **no se usa `Content-Length`**.  
  - El final del cuerpo se detecta por el chunk de tamaño cero, no por una longitud fija.

---

### 5. Manejo de errores en el parseo de headers y cuerpo

Un servidor robusto debe detectar errores y responder con el código HTTP adecuado.

#### 5.1 Errores en el parseo de headers

- **Sintaxis de línea incorrecta**  
  - Falta el `:` en un header.  
  - Caracteres no válidos en el nombre del header.  
  - Acción típica: responder con **400 Bad Request**.

- **Headers obligatorios ausentes**  
  - Por ejemplo, `Host` en HTTP/1.1.  
  - Dependiendo del protocolo → **400 Bad Request**.

- **Combinaciones contradictorias**  
  - `Content-Length` y `Transfer-Encoding: chunked` a la vez.  
  - Múltiples `Content-Length` con valores distintos.  
  - Acción típica: tratar la petición como inválida → **400 Bad Request**.

#### 5.2 Errores en Content-Length

- **Valor no numérico o negativo**  
  - Error de formato → **400 Bad Request**.

- **Longitud declarada vs cuerpo real**  
  - Cuerpo recibido más corto:  
    - Podría indicar que el cliente cerró la conexión antes de terminar.  
    - Acción: considerar la petición incompleta y no procesarla.  
  - Cuerpo más largo:  
    - Datos extra, posible error de protocolo → descartar petición.

#### 5.3 Errores en chunked

- **Tamaño de chunk no válido (no hexadecimal, negativo, etc.)**  
  - Cuerpo mal formado → **400 Bad Request**.

- **Chunk incompleto**  
  - No se reciben tantos bytes como el tamaño declarado.  
  - Posible corte de conexión o ataque.  
  - Acción: abortar la lectura y no procesar la petición.

- **Falta chunk de tamaño 0 o final incorrecto**  
  - No se detecta correctamente el final del cuerpo.  
  - Tratado como mensaje mal formado.

---

### 6. Resumen conceptual

- **Headers**: metadatos clave:valor que describen la petición/respuesta; se parsean línea a línea hasta una línea vacía.  
- **Content-Length**: indica longitud exacta del cuerpo; permite saber cuántos bytes leer.  
- **Transfer-Encoding: chunked**: el cuerpo se envía en chunks; cada chunk lleva su tamaño en hexadecimal, y el último tamaño es 0.  
- **Buffer temporal + acumulador**: permiten recibir datos del socket en trozos y dar al parser una vista continua del mensaje.  
- **Manejo de errores**:  
  - Validar formato de cada header y coherencia global.  
  - Validar el valor de `Content-Length` y que coincida con los datos.  
  - Validar la estructura completa de los chunks.


