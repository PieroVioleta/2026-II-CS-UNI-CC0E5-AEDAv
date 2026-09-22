# 01 · El vector copiado de IA

> **Ramas cubiertas:** `02-Vector` (Vector 0.1 → 0.3)
> **Estado:** contenido inicial — pendiente de revisar con lo visto en clase

## 📋 Ficha

| Rama | Commit(s) | Archivos tocados | Versión |
|------|-----------|------------------|---------|
| `02-Vector` | `f4ff2db` "Vector 0.1 created" | `containers/vector.h` (101 l), `Demos.cpp/h`, `main.cpp` | 0.1 |
| `02-Vector` | `584265b` "Vector basico 0.2" | `containers/vector.h` (20±) | 0.2 |
| `02-Vector` | `b54034e` "Vector 0.3" | `containers/vector.h` (38±) | 0.3 |

## 🎯 En qué estábamos

Tras el setup (nota 00), el profesor trajo una implementación de un `Vector`
**copiada de IA** como punto de partida, y sobre ella empezamos a hacer
ingeniería inversa: entenderla, ejecutarla y encontrarle problemas.

## 🧩 La implementación

### Antes (`01-Main`)

Solo el Hello World — no existía `containers/`.

### Después (esta etapa)

**Estructura del proyecto:**

- `containers/vector.h` — el contenedor (header-only)
- `Demos.cpp/h` — `DemoVector()`: pushea 0..9 y los imprime con `operator[]`
- `main.cpp` — ahora llama `DemoVector()` en vez del Hello World

**La evolución en 3 commits:**

| Commit | Qué cambió |
|--------|-----------|
| 0.1 | Vector de `unsigned long long`, campo `cap`, sin include guard, sin newline final |
| 0.2 | Tipo → `int`, renombre `cap` → `m_capacity` (convención `m_`) |
| 0.3 | Include guard `__VECTOR_H__`, alias `using T = int;`, loops con `size_t`, `T` en toda la API |

**La API final (0.3):**

```cpp
Vector()                          // ctor por defecto (capacidad 0)
~Vector()                         // delete[] data
Vector(const Vector&)             // copia: reserve + loop de asignación
operator=(Vector other)           // copy&swap (paso por valor)
Vector(Vector&&) noexcept         // move: swap
operator=(Vector&&) noexcept      // move: swap
push_back(T) / pop_back()
operator[](int) / at(int)         // sin chequeo / con excepción
size() / capacity() / empty() / clear()
// privado: reserve(new_cap) con new[] + copia + delete[]
```

**El corazón del contenedor — crecimiento amortizado:**

```cpp
void push_back(T value) {
    if (m_size == m_capacity) {
        int new_cap = (m_capacity == 0) ? 1 : m_capacity * 2;  // duplica
        reserve(new_cap);                                       // new[] + copiar + delete[]
    }
    data[m_size] = value;
    ++m_size;
}
```

## 📚 Conceptos nuevos

- **Array dinámico / capacidad vs tamaño**: `m_size` ≠ `m_capacity`; el
  crecimiento duplica la capacidad → `push_back` amortizado O(1).
- **Rule of five**: ctor copia, `operator=` copia, ctor move, `operator=` move,
  destructor. Aquí via **copy&swap**: `operator=` recibe por valor y hace swap.
- **`noexcept` en move**: necesario para que `std::vector` (y algoritmos STL)
  elijan move sobre copy al realocar.
- **`operator[]` vs `at()`**: acceso sin chequeo (rápido) vs con
  `std::out_of_range` (seguro).

## 🐛 Problemas del código copiado de IA

1. **No es template.** `using T = int;` es solo un alias — este `Vector` solo
   guarda `int`. Un vector real necesita `template<typename T>`.
2. **Destructores comentados** en `pop_back()` y `clear()`:
   ```cpp
   --m_size;
   // data[m_size].~T();   ← la IA lo dejó como "TODO" comentado
   ```
   Con `int` funciona (trivial), con un tipo que maneja recursos sería una fuga.
3. **Include guard con identificador reservado**: `__VECTOR_H__` — los
   identificadores con doble guion bajo inicial son del compilador. Debería ser
   `VECTOR_H` o `#pragma once`.
4. **Declaración múltiple confusa**: `T * data, m_size, m_capacity;` mezcla un
   puntero y dos ints en una línea — el `*` no se propaga y se presta a errores.
5. **`size()`/`capacity()` retornan `T`** (int con signo): inconsistente con
   `std::size_t`; genera warnings al mezclar con loops unsigned (ver Demos:
   `unsigned long long i` para contar hasta 10).
6. **`reserve` usa `new T[...]`**: exige tipo default-constructible; un
   `std::vector` real usa memoria cruda + placement new (nivel avanzado).
7. **Typo en las instrucciones de compilación** (`main.cpp`):
   `g++ -std=c++23 main.cpp Demos.cpp-o main` — falta el espacio; el comentario
   de la IA ni siquiera compila.
8. **Lo que falta y motiva las siguientes etapas**: impresión (`operator<<`,
   nota 02), recorrido genérico (`foreach`, nota 03), iteradores (nota 04).

## 👨‍🏫 Lo que explicó el profe

> ⚠ *Por completar con lo dicho en clase.*

- ¿Por qué empezamos con código de IA? *(completar)*
- Cómo detectar los problemas 1-8 en revisión de código.

## ✍️ Mi práctica

- [ ] Reproducir `02-Vector` local: `git diff upstream/01-Main upstream/02-Vector`
- [ ] Compilar: `g++ -std=c++23 main.cpp Demos.cpp -o main`
- [ ] Provocar el bug del `at()` fuera de rango y leer la excepción
- [ ] Imprimir `size()` vs `capacity()` tras varios `push_back` (ver el x2)
- [ ] (Reto) Convertir el alias `using T = int` en un `template<typename T>`

## 🔗 Referencias

- `git show upstream/02-Vector:containers/vector.h`
- Evolución: `git diff f4ff2db 584265b` y `git diff 584265b b54034e`
- [cppreference — Rule of five](https://en.cppreference.com/w/cpp/language/rule_of_three)
- [cppreference — copy and swap idiom](https://en.cppreference.com/w/cpp/language/copy_assignment)
