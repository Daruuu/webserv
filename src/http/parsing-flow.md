## Diagramas de flujo: Parseo de Start Line, Headers y Body

Este archivo resume en forma de **diagramas de flujo textuales** cómo debería funcionar el parser HTTP en tres etapas:

- Start line (request line).  
- Headers.  
- Body (Content-Length y chunked).

---

### 1) Diagrama de flujo: parseo de Start Line

```text
[Datos en acumulador]
        |
        v
¿Hay "\r\n" disponible?
        |
   +----+----+
   |         |
  NO        SÍ
   |         |
   |   Extraer texto hasta "\r\n"
   |         |
   |    ¿Línea vacía?
   |         |
   |    +----+----+
   |    |         |
   |   SÍ        NO
   |    |         |
   |  ERROR   (posible respuesta anterior,
   |          no debería ser vacía aquí)
   |         
   v
(esperar más datos
 del socket)
```

```text
[Tenemos la start line completa]
        |
        v
Dividir por espacios -> [método, URI, versión]
        |
        v
¿Hay exactamente 3 partes?
        |
   +----+----+
   |         |
  NO        SÍ
   |         |
   |       v
   |   Validar método soportado
   |       |
   |   ¿Método válido?
   |       |
   |   +---+---+
   |   |       |
   |  NO      SÍ
   |   |       |
   |  ERROR    v
   |        Separar URI
   |        en:
   |        - path
   |        - query string (opcional, tras '?')
   |             |
   |             v
   |        Validar versión HTTP
   |             |
   |        ¿Versión soportada?
   |             |
   |        +----+----+
   |        |         |
   |       NO        SÍ
   |        |         |
   |      ERROR       v
   v              Guardar:
 ERROR           - método
                 - path
                 - query
                 - versión
                      |
                      v
             Estado -> "Start line OK"
```

---

### 2) Diagrama de flujo: parseo de Headers

```text
Estado actual: "Start line OK"
        |
        v
Entrar en modo lectura de headers
        |
        v
¿Hay "\r\n\r\n" en el acumulador?
        |
   +----+----+
   |         |
  NO        SÍ
   |         |
   |     Separar:
   |     - bloque de headers (hasta "\r\n\r\n")
   |     - resto (posible comienzo del body)
   |         |
   v         v
(esperar más  Procesar bloque de headers
 datos del           línea a línea
  socket)
```

```text
[Procesar cada línea de header]
        |
        v
Tomar siguiente línea del bloque
        |
        v
¿Línea vacía?
        |
   +----+----+
   |         |
  SÍ        NO
   |         |
Fin headers  v
             ¿La línea contiene ":"?
                      |
                +-----+-----+
                |           |
               NO          SÍ
                |           |
             ERROR      Separar en:
                        - nombre
                        - valor
                          |
                          v
                   Normalizar nombre
                   (case-insensitive)
                   y recortar espacios
                          |
                          v
                   Guardar en mapa:
                   headers[nombre] = valor
                          |
                          v
                  Volver a siguiente línea
```

```text
[Después de leer todos los headers]
        |
        v
Validar requisitos:
- ¿Header Host presente (HTTP/1.1)?
- ¿No hay combinaciones prohibidas?
    * Content-Length + Transfer-Encoding: chunked
    * Múltiples Content-Length distintos
        |
        v
¿Validación OK?
        |
   +----+----+
   |         |
  NO        SÍ
   |         |
  ERROR      v
        Estado -> "Headers OK"
```

---

### 3) Diagrama de flujo: parseo de Body

#### 3.1 Selección de estrategia (sin entrar aún en Content-Type)

```text
Estado actual: "Headers OK"
        |
        v
¿Método requiere o puede tener body?
        |
   +----+----+
   |         |
  NO        SÍ
   |         |
   |     Mirar headers:
   |     - Content-Length
   |     - Transfer-Encoding
   |          |
   |          v
   |    ¿Transfer-Encoding: chunked?
   |          |
   |     +----+----+
   |     |         |
   |    SÍ        NO
   |     |         |
   |     |    ¿Hay Content-Length?
   |     |         |
   |     |    +----+----+
   |     |    |         |
   |     |   SÍ        NO
   |     |    |         |
   v     v    v         v
Body vacío  Modo    Modo    Body vacío
 (pasar      chunked  fixed   (según
  directo)   (chunks) length  política)
```

---

#### 3.2 Modo Content-Length (longitud fija)

```text
[Modo fixed length]
        |
        v
Obtener N = Content-Length (validado)
        |
        v
¿Ya hay parte del body en el acumulador?
        |
        v
Restar lo ya presente de N
        |
        v
Mientras (faltan bytes por leer):
    leer del socket -> buffer temporal
    añadir al acumulador/body
        |
        v
¿Se han leído exactamente N bytes?
        |
   +----+----+
   |         |
  NO        SÍ
   |         |
  ERROR      v
     (corte      Estado -> "Body OK"
   inesperado    Body listo para
    u otro)      capa de aplicación
```

---

#### 3.3 Modo chunked

```text
[Modo chunked]
        |
        v
Inicializar body reconstruido vacío
        |
        v
BUCLE:
  1) Leer línea de tamaño de chunk
     (hasta "\r\n" en el acumulador)
        |
        v
  2) Convertir tamaño (hex) -> M
        |
        v
  ¿Conversión válida?
        |
   +----+----+
   |         |
  NO        SÍ
   |         |
  ERROR      v
        ¿M == 0?
        |
   +----+----+
   |         |
  SÍ        NO
   |         |
   |     3) Asegurar que hay
   |        al menos M bytes
   |        de datos + "\r\n"
   |           |
   |           v
   |      ¿Datos completos?
   |           |
   |      +----+----+
   |      |         |
   |     NO        SÍ
   |      |         |
   |   Esperar   4) Copiar M bytes
   |   más datos    al body reconstruido
   |                y saltar "\r\n"
   |                     |
   |                     v
   |               Volver al inicio del bucle
   |
   v
Fin de chunks (M == 0)
        |
        v
Opcional: procesar trailers (headers finales)
        |
        v
Estado -> "Body OK"
Body reconstruido listo
para capa de aplicación
```

---

### 4) Resumen visual

- **Start line**:  
  - Buscar primera línea → validar 3 partes → validar método, URI (path + query) y versión.

- **Headers**:  
  - Leer hasta `\r\n\r\n` → procesar cada línea `nombre: valor` → validar coherencia.

- **Body**:  
  - Decidir si hace falta body.  
  - Elegir entre `Content-Length` (longitud fija) o `chunked`.  
  - Leer, reconstruir y validar hasta dejar un body “plano” listo para la lógica de aplicación.




