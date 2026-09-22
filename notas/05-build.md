# 05 · Makefile

> **Ramas cubiertas:** `08-Makefile`
> **Estado:** contenido inicial — pendiente de revisar con lo visto en clase

## 📋 Ficha

| Rama | Commit(s) | Archivos tocados | Versión |
|------|-----------|------------------|---------|
| `08-Makefile` | `dc783fe` "Makefile running" | `Makefile` (nuevo), `main.cpp`, `.gitignore`, `GeneralIterator.h` (+1 comentario `// CRTP`) | — |

## 🎯 En qué estábamos

Hasta acá, compilar era tipear (o copiar) la línea mágica del comentario en
`main.cpp`:

```bash
g++ -std=c++23 main.cpp Demos.cpp -o main
```

Etapa corta: se automatiza el build con un `Makefile` mínimo.

## 🧩 La implementación

### Antes (`07-Vector2.0`)

Sin Makefile: un comentario en `main.cpp` era la "documentación" del build
(incluso con el typo de la nota 01: `Demos.cpp-o main`).

### Después (esta etapa)

```makefile
CXX      := g++
CXXFLAGS := -std=c++23
TARGET   := main
SRCS     := main.cpp Demos.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
```

Y en `main.cpp` los comentarios de compilación se reemplazan por:

```cpp
// Compilar asi: make
// Ejecutar asi: ./main
```

**Detalles del mismo commit:**

- `.gitignore`: se **elimina la línea `Makefile`** — el gitignore stock la
  traía porque en proyectos CMake el Makefile es un archivo generado; acá el
  Makefile es **código fuente escrito a mano** y debe versionarse. Lección:
  no arrastrar gitignores ajenos sin revisar.
- `GeneralIterator.h`: solo se le agrega el comentario `// CRTP` (etiquetar
  la técnica).

## 📚 Conceptos nuevos

- **Targets, prerequisitos y recetas**: `$(TARGET): $(SRCS)` + tab + comando.
  Make re-ejecuta si los prerequisitos son más nuevos que el target.
- **`.PHONY`**: declara que `all`/`run`/`clean` no son archivos — sin esto, si
  existiera un archivo llamado `clean`, `make clean` diría "up to date".
- **Variables** (`CXX`, `CXXFLAGS`): un solo lugar para cambiar compilador o
  flags.
- **Target por defecto**: el primero que aparece (`all`) es el que corre con
  `make` a secas.
- **Dependencia en cadena**: `run: all` ejecuta `all` (→ compilación) antes de
  `./main`.

## 🐛 Problemas detectados en esta etapa

1. **"Makefile running" no significaba que compila** (verificado): `make`
   ejecuta la receta correctamente, pero la build falla por el
   `std::exchange`/`<utility>` heredado de la nota 04 — el propio GCC lo
   sugiere:
   ```
   note: 'std::exchange' is defined in header '<utility>';
         did you forget to '#include <utility>'?
   ```
   El Makefile es la pieza que sí funciona de ese commit.
2. **Recompila todo siempre**: `$(TARGET): $(SRCS)` dispara la compilación
   completa si cambia *cualquier* .cpp (o el Makefile). No hay .o separados.
3. **No hay tracking de headers**: tocar `containers/vector.h` o
   `GeneralIterator.h` NO dispara recompilación — el binario queda viejo
   silenciosamente. Falta algo tipo `g++ -MMD` / `$(wildcard *.h)`.
4. **`clean` no limpia `vector.txt`** que produce el demo.

## 👨‍🏫 Lo que explicó el profe

> ⚠ *Por completar con lo dicho en clase.*

- ¿Por qué tabs y no espacios en las recetas? *(el clásico `missing separator`)*
- ¿Cuándo vale la pena saltar a CMake?

## ✍️ Mi práctica

- [ ] `make`, `make run`, `make clean` en el árbol de `08-Makefile`
- [ ] Agregar el `#include <utility>` y verificar que `make run` completa
- [ ] `touch containers/vector.h && make` → observar que NO recompila (bug 3)
- [ ] Mejora: agregar `CXXFLAGS += -Wall -Wextra` y leer los warnings
- [ ] (Reto) Reglas por-objeto: `main.o: main.cpp` con `$(CXX) -c`

## 🔗 Referencias

- `git show upstream/08-Makefile:Makefile`
- [Manual de GNU Make](https://www.gnu.org/software/make/manual/make.html)
- [cppreference — std::exchange (header \<utility\>)](https://en.cppreference.com/w/cpp/utility/exchange)
