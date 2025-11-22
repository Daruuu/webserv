# Git Workflow - Guía del Equipo

Esta guía establece las reglas y buenas prácticas para la gestión de Git en nuestro proyecto webserv.

## 📋 Tabla de Contenidos

- [Estrategia de Branching](#estrategia-de-branching)
- [Workflow Recomendado](#workflow-recomendado)
- [Convenciones de Commits](#convenciones-de-commits)
- [Reglas del Equipo](#reglas-del-equipo)
- [Ejemplo de Workflow Completo](#ejemplo-de-workflow-completo)
- [Resolución de Conflictos](#resolución-de-conflictos)
- [Comandos Útiles](#comandos-útiles)

---

## 🌳 Estrategia de Branching

Utilizamos **Git Flow simplificado**, ideal para equipos pequeños:

### Ramas Principales

- **`main`**: Código en producción, siempre estable
  - Solo se actualiza mediante merge desde `develop`
  - Cada merge representa una versión funcional del proyecto
  
- **`develop`**: Rama de desarrollo principal
  - Aquí se integra todo el trabajo del equipo
  - Debe compilar y pasar tests básicos

### Ramas de Trabajo

- **`feature/*`**: Para nuevas funcionalidades
  - Ejemplo: `feature/http-parser`, `feature/config-file`, `feature/cgi-handler`
  - Se crean desde `develop` y se mergean de vuelta a `develop`
  
- **`fix/*`**: Para correcciones de bugs
  - Ejemplo: `fix/memory-leak`, `fix/timeout-handling`
  
- **`hotfix/*`**: Para correcciones urgentes en producción
  - Se crean desde `main` y se mergean a `main` y `develop`

---

## 🔄 Workflow Recomendado

### 1. Crear Rama para cada Tarea

```bash
# Asegurarse de tener develop actualizado
git checkout develop
git pull origin develop

# Crear nueva rama feature
git checkout -b feature/nombre-funcionalidad
```

### 2. Trabajar y Hacer Commits Frecuentes

```bash
# Añadir cambios
git add .

# O añadir archivos específicos
git add src/parser.cpp include/parser.hpp

# Commit con mensaje descriptivo
git commit -m "feat(parser): implementar parsing de request line"
```

### 3. Mantener la Rama Actualizada

```bash
# Actualizar con los últimos cambios de develop
git fetch origin
git rebase origin/develop

# Si hay conflictos, resolverlos y continuar
git add .
git rebase --continue
```

### 4. Push y Crear Pull Request

```bash
# Subir rama al repositorio remoto
git push origin feature/nombre-funcionalidad

# Ir a GitHub/GitLab y crear Pull Request hacia develop
```

### 5. Code Review y Merge

- Al menos **1 miembro del equipo** debe revisar el PR
- Resolver comentarios y sugerencias
- Una vez aprobado, hacer **squash and merge** o **rebase and merge**

### 6. Limpieza Post-Merge

```bash
# Volver a develop y actualizar
git checkout develop
git pull origin develop

# Borrar rama local
git branch -d feature/nombre-funcionalidad

# Borrar rama remota (si no se borró automáticamente)
git push origin --delete feature/nombre-funcionalidad
```

---

## 📝 Convenciones de Commits

Usamos **Conventional Commits** para mantener un historial limpio y legible.

### Formato

```
tipo(scope): descripción breve en minúsculas

[cuerpo opcional con más detalles]

[footer opcional: referencias a issues]
```

### Tipos de Commits

| Tipo | Descripción | Ejemplo |
|------|-------------|---------|
| `feat` | Nueva funcionalidad | `feat(server): implementar manejo de peticiones GET` |
| `fix` | Corrección de bug | `fix(parser): corregir parsing de headers HTTP` |
| `docs` | Documentación | `docs(readme): añadir instrucciones de compilación` |
| `refactor` | Refactorización sin cambiar funcionalidad | `refactor(config): extraer parsing a función separada` |
| `test` | Añadir o modificar tests | `test(server): añadir tests para timeout` |
| `style` | Formato, espacios, punto y coma | `style(parser): formatear código según norminette` |
| `chore` | Tareas de mantenimiento | `chore(makefile): actualizar flags de compilación` |
| `perf` | Mejoras de rendimiento | `perf(server): optimizar búsqueda de rutas` |

### Scope (Alcance)

El scope indica qué parte del proyecto se ve afectada:

- `server`: Servidor HTTP principal
- `parser`: Parser de HTTP
- `config`: Configuración
- `cgi`: Manejo de CGI
- `response`: Generación de respuestas
- `request`: Manejo de peticiones
- `utils`: Utilidades generales
- `makefile`: Sistema de build

### Ejemplos de Buenos Commits

```bash
feat(server): implementar multiplexing con select/poll/epoll
fix(parser): corregir buffer overflow en lectura de headers
docs(config): documentar formato del archivo de configuración
refactor(response): separar generación de headers en función propia
test(cgi): añadir tests para timeout de scripts CGI
style(server): aplicar norminette a server.cpp
chore(makefile): añadir regla para tests
perf(parser): usar string reserve para evitar realocaciones
```

### Ejemplos de Malos Commits ❌

```bash
# Muy vago
git commit -m "fix bug"
git commit -m "update"
git commit -m "changes"

# Sin tipo
git commit -m "añadir parser"

# Demasiado largo en la primera línea
git commit -m "feat(server): implementar todo el sistema de manejo de peticiones HTTP con soporte para GET, POST, DELETE y manejo de errores"

# En mayúsculas (debe ser minúsculas)
git commit -m "Feat(Server): Add HTTP handling"
```

---

## ⚠️ Reglas del Equipo

### Reglas Obligatorias

1. **NUNCA hacer push directo a `main` o `develop`**
   - Todo cambio debe pasar por Pull Request
   
2. **Siempre trabajar en ramas feature/fix**
   - No hacer commits directamente en develop
   
3. **Pull Requests requieren al menos 1 revisión**
   - Otro miembro del equipo debe aprobar
   - El autor NO puede aprobar su propio PR
   
4. **Hacer rebase antes de crear PR**
   - Mantiene el historial limpio y lineal
   
5. **Resolver conflictos localmente**
   - No crear PRs con conflictos
   
6. **Borrar ramas feature después de mergear**
   - Mantiene el repositorio limpio

### Buenas Prácticas

- ✅ Commits pequeños y frecuentes
- ✅ Mensajes de commit descriptivos
- ✅ Probar el código antes de hacer push
- ✅ Actualizar la rama con develop regularmente
- ✅ Comentar el código complejo
- ✅ Actualizar documentación cuando sea necesario
- ✅ Referenciar issues en commits: `fix(parser): corregir timeout (#23)`

---

## 🔧 Ejemplo de Workflow Completo

```bash
# ========================================
# 1. INICIO: Actualizar develop
# ========================================
git checkout develop
git pull origin develop

# ========================================
# 2. CREAR RAMA: Nueva funcionalidad
# ========================================
git checkout -b feature/http-parser

# ========================================
# 3. TRABAJAR: Implementar y commitear
# ========================================
# Implementar parsing básico
vim src/parser.cpp
git add src/parser.cpp include/parser.hpp
git commit -m "feat(parser): implementar parsing de request line"

# Añadir tests
vim tests/parser_test.cpp
git add tests/parser_test.cpp
git commit -m "test(parser): añadir tests para request line"

# Refactorizar
vim src/parser.cpp
git add src/parser.cpp
git commit -m "refactor(parser): extraer validación a función separada"

# ========================================
# 4. ACTUALIZAR: Rebase con develop
# ========================================
git fetch origin
git rebase origin/develop

# Si hay conflictos:
# - Resolver conflictos en los archivos
# - git add <archivos-resueltos>
# - git rebase --continue

# ========================================
# 5. PUSH: Subir cambios
# ========================================
git push origin feature/http-parser

# Si ya hiciste push antes del rebase:
git push origin feature/http-parser --force-with-lease

# ========================================
# 6. PULL REQUEST: Crear en GitHub/GitLab
# ========================================
# - Ir a la web del repositorio
# - Crear Pull Request de feature/http-parser → develop
# - Añadir descripción clara
# - Asignar reviewers
# - Esperar aprobación

# ========================================
# 7. LIMPIEZA: Después del merge
# ========================================
git checkout develop
git pull origin develop
git branch -d feature/http-parser
```

---

## 🔥 Resolución de Conflictos

### Durante Rebase

```bash
# 1. Iniciar rebase
git rebase origin/develop

# 2. Si hay conflictos, Git te lo indicará
# CONFLICT (content): Merge conflict in src/server.cpp

# 3. Abrir archivos con conflictos y resolverlos
vim src/server.cpp

# Buscar marcadores de conflicto:
# <<<<<<< HEAD
# Tu código
# =======
# Código de develop
# >>>>>>> origin/develop

# 4. Después de resolver, añadir archivos
git add src/server.cpp

# 5. Continuar rebase
git rebase --continue

# Si quieres abortar el rebase:
git rebase --abort
```

### Durante Merge

```bash
# 1. Si hay conflictos durante merge
git merge develop

# 2. Resolver conflictos en archivos
vim src/server.cpp

# 3. Añadir archivos resueltos
git add src/server.cpp

# 4. Completar merge
git commit

# Para abortar merge:
git merge --abort
```

---

## 🛠️ Comandos Útiles

### Ver Estado del Repositorio

```bash
# Ver estado de archivos
git status

# Ver historial de commits
git log --oneline --graph --all

# Ver cambios no commiteados
git diff

# Ver cambios en staging
git diff --staged
```

### Gestión de Ramas

```bash
# Listar ramas locales
git branch

# Listar ramas remotas
git branch -r

# Listar todas las ramas
git branch -a

# Borrar rama local
git branch -d nombre-rama

# Borrar rama remota
git push origin --delete nombre-rama

# Cambiar de rama
git checkout nombre-rama

# Crear y cambiar a nueva rama
git checkout -b nueva-rama
```

### Deshacer Cambios

```bash
# Descartar cambios en archivo (no commiteado)
git checkout -- archivo.cpp

# Descartar todos los cambios no commiteados
git reset --hard

# Quitar archivo de staging
git reset HEAD archivo.cpp

# Modificar último commit (antes de push)
git commit --amend

# Volver a commit anterior (cuidado!)
git reset --hard HEAD~1
```

### Actualizar y Sincronizar

```bash
# Traer cambios sin mergear
git fetch origin

# Traer y mergear cambios
git pull origin develop

# Subir cambios
git push origin nombre-rama

# Subir forzado (después de rebase)
git push origin nombre-rama --force-with-lease
```

### Stash (Guardar Trabajo Temporal)

```bash
# Guardar cambios temporalmente
git stash

# Ver lista de stashes
git stash list

# Aplicar último stash
git stash apply

# Aplicar y borrar último stash
git stash pop

# Borrar todos los stashes
git stash clear
```

---

## 📚 Recursos Adicionales

- [Conventional Commits](https://www.conventionalcommits.org/)
- [Git Flow](https://nvie.com/posts/a-successful-git-branching-model/)
- [GitHub Flow](https://guides.github.com/introduction/flow/)
- [Atlassian Git Tutorials](https://www.atlassian.com/git/tutorials)

---

## 👥 Contacto y Dudas

Si tienes dudas sobre el workflow o necesitas ayuda:
1. Pregunta en el grupo del equipo
2. Revisa esta documentación
3. Consulta con otros miembros del equipo

**Recuerda**: Es mejor preguntar antes de hacer un push que romper el repositorio 😊

---

*Última actualización: 2025-11-22*
