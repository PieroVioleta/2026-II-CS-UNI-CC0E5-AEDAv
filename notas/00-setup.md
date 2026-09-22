# 00 · Setup y punto de partida

> **Ramas cubiertas:** `00-Students`, `01-Main`
> **Estado:** contenido inicial — pendiente de revisar con lo visto en clase

## 📋 Ficha

| Rama | Commit(s) | Archivos | Qué hizo |
|------|-----------|----------|----------|
| `00-Students` | `13bea41` → `bc4e482` (8 commits) | `README.md`, `Students.txt` | Repo base del curso + registro de estudiantes vía PRs (#5, #9) |
| `01-Main` | `64331cb` "main added" | `main.cpp`, `.gitignore` | Primer programa C++ + gitignore estándar de C++ |
| `01-Main` | `7c4ff30` "C++23 activated" | `main.cpp` | Documenta compilación con `-std=c++23` |

## 🎯 En qué estábamos

Punto cero del curso: el profesor creó el repo institucional y cada estudiante
se registró mediante un PR. Todavía **no hay código C++** — esta etapa es de
logística y convenciones.

## 🧩 La implementación

### Antes (initial commit `13bea41`)

Solo dos archivos:

- `README.md` — únicamente el título del curso (`# 2026-II-CS-UNI-CC0E5-AEDAv`)
- `Students.txt` — el roster: una fila por estudiante con formato `código UNI<TAB>Nombre`

### Después (esta etapa)

**1. El roster se llena con PRs.** Los estudiantes hicieron fork y agregaron su
fila. Detalle curioso del PR #9 (`bc4e482`): dos estudiantes se habían agregado
con formato de mailmap (`"Nombre" <email>`), y se corrigieron al formato del
curso `código + nombre`:

```diff
-"Luna Bueno Aldo" <alunab@uni.pe>
+20170325B	Luna Bueno Aldo
```

Lección: **consistencia de formato** en archivos compartidos (aquí el código UNI
es el identificador, no el email).

**2. Primer `main.cpp`** (`64331cb`) — 9 líneas, sin newline final:

```cpp
#include <iostream>

using namespace std;

// g++ main.cpp -o main
int main() {
    cout << "Hello, World! CS-UNI!" << endl;
    return 0;
}
```

**3. `.gitignore` estándar de C++** (mismo commit): ~70 líneas cubriendo
objetos (`*.o`), libs, headers precompilados, directorios de build, CMake, y al
final la línea `main` — que ignora **el ejecutable** compilado.

**4. "C++23 activated"** (`7c4ff30`) — solo agrega 2 líneas, ambas comentarios:

```diff
 // g++ main.cpp -o main
+// para ejecutar: ./main
+// g++ -std=c++23 main.cpp -o main
```

## 📚 Conceptos nuevos

- **Flujo fork + PR** como mecánica del curso: cada estudiante trabaja en su
  fork y propone cambios al repo del profe vía PR.
- **`-std=c++23`**: el curso exige el estándar C++23. A partir de `02-Vector`
  el código lo va a necesitar (p. ej. `std::print`/features modernos).
- **`.gitignore` de C++**: nunca versionar binarios. El ejecutable se llama
  `main` (sin extensión) y esa línea exacta es la que lo ignora.

## 👨‍🏫 Lo que explicó el profe

> ⚠ *Por completar con lo dicho en clase.*

- Convención: el binario compilado se nombra `main` (por eso el gitignore).
- ¿Por qué C++23 y no C++20/17? *(completar)*
- Reglas del registro en `Students.txt`: formato `código<TAB>Nombre`.

## ✍️ Mi práctica

- [ ] Verificar versión del compilador: `g++ --version` (¿soporta C++23?)
- [ ] Compilar y correr el Hello World con las 2 formas de los comentarios
- [ ] (Opcional) Compilar con `-std=c++23 -Wall -Wextra` y comparar

## 🔗 Referencias

- Rama `upstream/00-Students`: `git log --oneline upstream/00-Students`
- Rama `upstream/01-Main`: `git diff upstream/00-Students upstream/01-Main`
- [cppreference — C++23](https://en.cppreference.com/w/cpp/23)
