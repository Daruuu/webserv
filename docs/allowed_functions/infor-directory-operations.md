# Explicación de `dirent.h`

`dirent.h` es una cabecera de la biblioteca estándar de C (parte del estándar POSIX) que se utiliza para realizar operaciones con directorios, como abrir carpetas, leer su contenido y cerrarlas. Es fundamental cuando necesitas listar archivos o recorrer estructuras de directorios.

## 1. Tipos de Datos Principales

*   **`DIR *`**: Es un puntero a un "flujo de directorio" (directory stream). Funciona de manera muy similar a `FILE *` para archivos. Mantiene el estado de en qué punto de la lectura del directorio te encuentras.
*   **`struct dirent`**: Es una estructura que representa una **entrada** dentro del directorio (un archivo, una subcarpeta, etc.).
    *   El campo más importante es `d_name`, que contiene el nombre del archivo (char array).
    *   Otro campo común es `d_type` (en sistemas BSD/Linux), que te dice si es un archivo, directorio, enlace simbólico, etc.

## 2. Funciones Principales

Las tres funciones más importantes son:

1.  **`opendir(const char *name)`**:
    *   Abre un directorio para lectura.
    *   Devuelve un puntero `DIR *` si tiene éxito, o `NULL` si falla (por ejemplo, si la ruta no existe o no tienes permisos).
    *   *Ejemplo:* `DIR *dir = opendir(path.c_str());`

2.  **`readdir(DIR *dirp)`**:
    *   Lee la **siguiente** entrada del directorio.
    *   Devuelve un puntero a `struct dirent` con la información del archivo actual.
    *   Devuelve `NULL` cuando ya no quedan más archivos por leer o si hay un error.
    *   **Importante**: `readdir` devuelve también las entradas `.` (directorio actual) y `..` (directorio padre), por lo que a menudo tendrás que filtrarlas con un `if`.
    *   *Ejemplo:* `while ((entry = readdir(dir)) != NULL)`

3.  **`closedir(DIR *dirp)`**:
    *   Cierra el flujo del directorio y libera los recursos asociados. Es importante llamarlo siempre al terminar para evitar fugas de memoria o de descriptores de archivo.
    *   *Ejemplo:* `closedir(dir);`

## Ejemplo visual de `struct dirent`

Aunque la estructura exacta varía según el sistema operativo, generalmente se ve así:

```c
struct dirent {
    ino_t          d_ino;       // Número de inodo
    off_t          d_off;       // Offset a la siguiente entrada
    unsigned short d_reclen;    // Longitud de este registro
    unsigned char  d_type;      // Tipo de archivo (DT_DIR, DT_REG, etc.)
    char           d_name[256]; // Nombre del archivo (terminado en nulo)
};
```

En el contexto de un servidor web (como `webserv`), esta librería es esencial para funcionalidades como el **autoindexing** (mostrar un listado de archivos cuando el usuario accede a una carpeta que no tiene un `index.html`).
