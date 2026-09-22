# 🔬 Deep-dive · GeneralIterator: de herencia naive a CRTP (3 versiones)

> **Complemento de:** [04-iteradores.md](04-iteradores.md)
> **Evidencia:** compilaciones reales con `g++ -std=c++23` sobre los árboles de
> `05-iterator`, `06-CRTP`, `07-Vector2.0`, `10-Concurrency` y `14-TestTraversal`.
> **Estado:** contenido inicial — pendiente de revisar con lo visto en clase

## La pregunta de diseño

¿Cómo escribir **una** clase iterador genérica que sirva para cualquier
contenedor, sin repetir `operator*`, `operator++`, `operator==`... en cada una?

El curso la respondió **3 veces**, y cada versión enseña algo distinto.

## Matriz de compilación verificada

| Etapa | Diseño | Compila | Error real |
|-------|--------|---------|------------|
| `05-iterator` | v1: naive, en vector.h | ❌ | `operator*` duplicado; forward-decl faltante |
| `06-CRTP` | v2: F-bound + traits | ❌ | `invalid use of incomplete type` |
| `07-Vector2.0` | v3: CRTP + traits | ❌ | `'exchange' is not a member of 'std'` (×12) |
| `10-Concurrency` | v3 + mutex | ❌ | los 12 errores = el mismo `std::exchange` |
| `14-TestTraversal` | v3 + Node + mutex | ❌ | idem |
| `14` **+ 1 línea** | — | ✅ | `#include <utility>` y todo el curso corre |

## Versión 1 — herencia naive (`05-iterator`)

Nace dentro de `containers/vector.h`:

```cpp
template <typename T>
class GeneralIterator {
public:
    using value_type        = T;
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using pointer           = value_type *;
    using reference         = value_type&;
protected:
    pointer m_ptr;
public:
    GeneralIterator(pointer ptr) : m_ptr(ptr) {}
    reference   operator*() const { return *m_ptr; }   // ┐ MISMA FIRMA:
    value_type& operator*() const { return *m_ptr; }   // ┘ cannot be overloaded
    // friend operator!= usa VectorForwardIterator... que aún no existe:
    friend bool operator!= (const VectorForwardIterator& a, ...);
};

template <typename T>
class VectorForwardIterator : public GeneralIterator<T> { ... };
```

**Dos errores del compilador (reales):**

1. `cannot be overloaded with` — cuando `reference` es alias de `value_type&`,
   los dos `operator*` son la misma firma. El error de la IA: declarar alias
   redundantes y luego "abstraer" dos veces lo mismo.
2. `VectorForwardIterator does not name a type` — la base referencia al hijo
   declarado después. En C++ el orden importa.

Además la herencia es ingenua: `GeneralIterator<T>` no sabe que su hijo es
`VectorForwardIterator<T>` — no puede hacer `friend` de él.

## Versión 2 — F-bound: la base recibe al hijo (`06-CRTP`)

Se extrae a `containers/GeneralIterator.h` y aparece el primer intento de CRTP:

```cpp
template <typename Iterator>                       // ← el parámetro es el HIJO
class GeneralIterator {
public:
    using value_type = typename Iterator::value_type;  // ← lee del hijo
    ...
    friend bool operator!= (const Iterator& a, const Iterator& b) {
        return a.m_ptr != b.m_ptr;                 // m_ptr del hijo ✓
    }
};

template <typename T>
class VectorForwardIterator : public GeneralIterator<VectorForwardIterator<T>> {
    using MySelf = VectorForwardIterator<T>;       // se pasa a sí mismo
    using Parent = GeneralIterator<MySelf>;
    using GeneralIterator<MySelf>::GeneralIterator;   // hereda ctor
    VectorForwardIterator& operator++() { ++Parent::m_ptr; return *this; }
};
```

Esto es **F-bounded polymorphism**: la base se templaliza con su derivada.
El `friend operator!=` ya recibe el tipo correcto. Pero el compilador dice:

```
error: invalid use of incomplete type 'class VectorForwardIterator<int>'
```

**La dependencia circular**: `GeneralIterator<VectorForwardIterator<T>>` necesita
`VectorForwardIterator::value_type` — pero esa clase está **siendo definida**
en ese momento. El hijo es incompleto dentro de su propia declaración de base.
Lección: una base nunca puede leer tipos definidos *dentro* del hijo.

## Versión 3 — CRTP con dos parámetros (`07-Vector2.0`) ⭐

El fix es separar las fuentes de información: `Derived` para la identidad, `T`
para los datos:

```cpp
template <typename Derived, typename T>
class GeneralIterator {
public:
    using value_type = T;              // ← del parámetro, no del hijo
    ...
protected:
    pointer m_ptr;
public:
    GeneralIterator(pointer ptr) : m_ptr(ptr) {}
    value_type& operator*() const { return *m_ptr; }
    friend bool operator== (const Derived& a, const Derived& b) {
        return a.m_ptr == b.m_ptr;     // ahora también operator==!
    }
    friend bool operator!= (const Derived& a, const Derived& b) {
        return a.m_ptr != b.m_ptr;
    }
};

template <typename T>
class VectorForwardIterator : public GeneralIterator<VectorForwardIterator<T>, T> {
public:
    using MySelf = VectorForwardIterator<T>;
    using Parent = GeneralIterator<MySelf, T>;
    using Parent::Parent;              // ← using Parent::Parent
    VectorForwardIterator& operator++() { ++Parent::m_ptr; return *this; }
};
```

**Lo que resuelve cada pieza:**

- `typename Derived, typename T` — identidad del hijo y tipo del elemento por
  vías independientes: no hay ciclo de tipos.
- `friend bool operator==(const Derived&...)` — el friend se genera para el
  tipo concreto; dos iteradores de tipos distintos no se pueden comparar entre
  sí (seguridad de tipos gratis).
- `using Parent::Parent` — hereda el ctor del puntero sin redeclararlo.
- `Parent::m_ptr` — miembros de bases dependientes exigen calificación
  (two-phase name lookup); `++m_ptr` a secas no compila.

**El traits completa el diseño:**

```cpp
template <typename T>
struct VectorAscTraits {
    using value_type      = T;
    using ForwardIterator = VectorForwardIterator<T>;
};

template <typename Traits>
class Vector {
    using value_type      = Traits::value_type;
    using ForwardIterator = Traits::ForwardIterator;
    ...
    ForwardIterator begin() { return ForwardIterator(m_data); }
    ForwardIterator end()   { return ForwardIterator(m_data + m_size); }
};

// Uso en Demos:
Vector<VectorAscTraits<TX>> vec;
```

`Vector` ya no menciona a su iterador: lo pide al traits. Cambiar la dirección
del recorrido (nota 10) = otro traits, cero cambios en `Vector`.

## Vector 2.0: lo que más cambió en el contenedor

| Antes | Después (`289e3e1`) | Comentario |
|-------|--------------------|------------|
| swap en move ctor/asig | `std::exchange(other.m_data, nullptr)` | asigna y devuelve el viejo, sin temporal |
| `T *m_data;` sin init | `value_type *m_data = nullptr;` | member initializers |
| `push_back(T value)` | `push_back(const value_type&)` | evita copia en objetos grandes (pierde ruta move) |
| `reserve()` | `resize()` ⚠ | renombre que colisiona con la semántica del STL |
| `~Vector()` | `virtual ~Vector()` ⚠ | vptr por instancia; el STL no lo hace |
| `template <typename T> class Vector` | `template <typename Traits> class Vector` | Vector 2.0 |

## La saga de `std::exchange` (y cómo terminó)

`std::exchange` vive en **`<utility>`** — que nunca se incluyó. El estado
"fixed" de `6abf3ae` y **todas** las ramas siguientes quedaron rotos por esa
única línea (verificado: los 12 errores de `10-Concurrency` son idénticos).

Fix de una línea sobre `14-TestTraversal`:

```diff
 #include <mutex>
+#include <utility>
```

Resultado: **0 errores**, y el demo final corre completo:

```
Container using write(): [(0,10),(1,11),...,(9,19)]
Forward traversal:  [(0,10) (1,11) ... (9,19)]
Backward traversal: [(9,19) (8,18) ... (0,10)]
```

(El output ya muestra los pares `Node` del curso completo — nota 08.)

## Lecciones

1. **El CRTP resuelve la pregunta correcta**: "¿cómo comparto implementación
   entre iteradores sin costos en runtime?" — la respuesta no es herencia
   virtual, es templates + `Derived`.
2. **La base no puede leer al hijo incompleto**: v2 vs v3 es la lección en
   acción (`typename Iterator::value_type` ❌ vs `typename T` ✓).
3. **El mensaje del commit no compila código**: "Generalterator fixed" tenía
   12 errores esperando. Única verdad: `g++` sin errores.
4. **Un include faltante puede ocultar todo lo demás**: hasta que no se arregla
   el primero, no sabes si el resto está bien.
5. **Los traits desacoplan el contenedor de su recorrido** — la semilla del
   iterador backward (nota 10).
