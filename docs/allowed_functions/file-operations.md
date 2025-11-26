# Operaciones de Archivos: `stat` y `fcntl`

En el desarrollo de servidores web y programación de sistemas en C++98, a menudo necesitamos más control e información sobre los archivos y descriptores de archivo que la que ofrecen las funciones básicas de E/S (`open`, `read`, `write`). Aquí es donde entran `stat` y `fcntl`.

## 1. `stat`: Obteniendo información del archivo

La función `stat` nos permite obtener información detallada sobre un archivo (metadatos) sin necesidad de abrirlo.

### Firma
```c
#include <sys/stat.h>

int stat(const char *path, struct stat *buf);
```
- **path**: Ruta al archivo.
- **buf**: Puntero a una estructura `struct stat` donde se guardará la información.
- **Retorno**: 0 si tiene éxito, -1 si falla (y `errno` se establece).

### La estructura `struct stat`
Esta estructura contiene información vital. Los campos más comunes son:

```c
struct stat {
    dev_t     st_dev;     // ID del dispositivo que contiene el archivo
    ino_t     st_ino;     // Número de inodo
    mode_t    st_mode;    // Protección y tipo de archivo (¡Muy importante!)
    nlink_t   st_nlink;   // Número de enlaces físicos (hard links)
    uid_t     st_uid;     // ID del usuario propietario
    gid_t     st_gid;     // ID del grupo propietario
    dev_t     st_rdev;    // ID del dispositivo (si es un archivo especial)
    off_t     st_size;    // Tamaño total en bytes
    time_t    st_atime;   // Hora del último acceso
    time_t    st_mtime;   // Hora de la última modificación
    time_t    st_ctime;   // Hora del último cambio de estado
};
```

### Macros para `st_mode`
El campo `st_mode` es un mapa de bits. Para verificar el tipo de archivo, usamos macros estándar:

- **`S_ISREG(st_mode)`**: ¿Es un archivo regular?
- **`S_ISDIR(st_mode)`**: ¿Es un directorio?
- **`S_ISLNK(st_mode)`**: ¿Es un enlace simbólico? (Nota: para enlaces usa `lstat` en lugar de `stat`).

### Ejemplo en C++98

```cpp
#include <iostream>
#include <sys/stat.h>
#include <errno.h>

void check_file(const char *path) {
    struct stat fileInfo;

    if (stat(path, &fileInfo) != 0) {
        std::cerr << "Error al obtener info del archivo" << std::endl;
        return;
    }

    std::cout << "Tamaño: " << fileInfo.st_size << " bytes" << std::endl;

    if (S_ISDIR(fileInfo.st_mode)) {
        std::cout << "Es un directorio." << std::endl;
    } else if (S_ISREG(fileInfo.st_mode)) {
        std::cout << "Es un archivo regular." << std::endl;
    }
}
```

---

## 2. `fcntl`: Control de Archivos

`fcntl` (File Control) es una navaja suiza para manipular descriptores de archivo. En el contexto de un servidor web (`webserv`), su uso más crítico es **hacer que las operaciones de E/S sean no bloqueantes**.

### Firma
```c
#include <fcntl.h>

int fcntl(int fd, int cmd, ... /* arg */ );
```
- **fd**: El descriptor de archivo a modificar.
- **cmd**: La operación a realizar (ej. `F_GETFL`, `F_SETFL`).
- **arg**: Argumento opcional dependiendo del comando.

### Uso: E/S No Bloqueante (`O_NONBLOCK`)
Por defecto, `read` y `write` bloquean la ejecución si no hay datos disponibles o si no se pueden escribir inmediatamente. En un servidor que maneja múltiples conexiones, esto es fatal. Usamos `fcntl` para activar `O_NONBLOCK`.

### Pasos para activar `O_NONBLOCK`
1.  Obtener las banderas actuales con `F_GETFL`.
2.  Añadir `O_NONBLOCK` con una operación OR a nivel de bits (`|`).
3.  Establecer las nuevas banderas con `F_SETFL`.

### Ejemplo en C++98

```cpp
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

int set_nonblocking(int fd) {
    // 1. Obtener banderas actuales
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "Error al obtener flags" << std::endl;
        return -1;
    }

    // 2. Modificar banderas
    // Mantenemos las banderas existentes y añadimos O_NONBLOCK
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "Error al establecer O_NONBLOCK" << std::endl;
        return -1;
    }
    
    return 0;
}
```

Al usar `O_NONBLOCK`, si intentas leer y no hay datos, `read` devolverá `-1` y `errno` será `EAGAIN` (o `EWOULDBLOCK`). Tu código debe estar preparado para manejar este caso (simplemente intentarlo de nuevo más tarde).
