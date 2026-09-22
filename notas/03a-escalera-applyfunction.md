# 🔬 Deep-dive · La escalera de ApplyFunction (niveles 0→4)

> **Complemento de:** [03-foreach.md](03-foreach.md)
> **Evidencia:** compilaciones reales ejecutadas con `g++ -std=c++23` sobre los
> árboles de las ramas `foreach`, `03-Foreach3` y `04-foreach4`.
> **Estado:** contenido inicial — pendiente de revisar con lo visto en clase

## ¿Por qué una "escalera"?

La tarea es siempre la misma: **aplicar una función `F` a cada elemento**.
Cada peldaño resuelve la limitación del anterior subiendo un nivel de
abstracción. Al final, el algoritmo ya no conoce ni al contenedor ni al tipo
de sus elementos — esa es la idea que sostiene todo el STL.

| Nivel | Firma | Requiere del contenedor | ¿Compilaba? |
|-------|-------|--------------------------|-------------|
| #0 | miembro `vec.ApplyFunction(func, args...)` | nada (es parte de la clase) | ✅ desde `9989bf5` |
| #1 | libre `ApplyFunction(container, func, args...)` con índices | `operator[]` + `size()` | comentado |
| #2 | libre con `container.begin()/end()` | protocolo iterador | comentado |
| #3a | libre con **par de iteradores** `ApplyFunction(b, e, func, args...)` | nada — solo iteradores | ✅ desde `a73d619` |
| #3b | libre con `Container&` que delega en #3a | `begin()/end()` | ✅ hasta `5ca72aa` |
| #4 | libre con `Container&` usando range-for | `begin()/end()` (o adl) | ❌ roto en `5ca72aa`, fix en `05-iterator` |

## Nivel #0 — método miembro con índices

```cpp
// dentro de class Vector { ... };
template <typename Func, typename... Args>
void ApplyFunction(Func func, Args... args) {
    for (size_t i = 0; i < size(); ++i)
        func(m_data[i], args...);
}
```

Funciona, pero el algoritmo vive **dentro** del contenedor: cada contenedor
nuevo (lista, stack...) tendría que reimplementar ApplyFunction. El algoritmo
y la estructura quedan casados.

## Nivel #1 — función libre con índices (comentado)

```cpp
template <typename Container, typename Func, typename... Args>
void ApplyFunction(Container &container, Func func, Args... args) {
    for (size_t i = 0; i < container.size(); ++i)
        func(container[i], args...);
}
```

Progreso: la función libre ya no es parte de la clase. Pero exige
`operator[]` **con acceso aleatorio O(1)** — una lista enlazada no puede
cumplirlo. El algoritmo sigue sabiendo demasiado.

## Nivel #2 — función libre con begin()/end() (comentado)

```cpp
template <typename Container, typename Func, typename... Args>
void ApplyFunction(Container &container, Func func, Args... args) {
    for (auto iter = container.begin(); iter != container.end(); ++iter)
        func(*iter, args...);
}
```

Introduce el **protocolo iterador** (`++`, `!=`, `*`). Solo exige que el
contenedor sepa entregarte sus extremos. Este nivel quedó comentado porque su
idea se absorbe en el #3.

## Nivel #3a — par de iteradores: el nivel STL ⭐

```cpp
template <typename Iterator, typename Func, typename... Args>
void ApplyFunction(Iterator begin, Iterator end, Func func, Args... args) {
    for (auto iter = begin; iter != end; ++iter)
        func(*iter, args...);
}
```

**Ya no aparece la palabra "container"**. Solo dos iteradores. Es la interfaz
canónica del STL (`std::sort(v.begin(), v.end(), f)` — misma forma).

Evidencia — el mismo algoritmo compila sin cambios con un array C crudo y con
`std::vector`:

```cpp
int arr[5] = {10, 20, 30, 40, 50};
::ApplyFunction(arr, arr + 5, AddOne);        // arr+5 ES un iterador

std::vector<int> v{1, 2, 3};
::ApplyFunction(v.begin(), v.end(), AddOne);
// 11 21 31 41 51 | 2 3 4   ← verificado con g++ -std=c++23
```

**Por qué funciona con `Vector` del curso:** desde `9989bf5`, `begin()/end()`
devuelven raw pointers — y un puntero ya es un iterador válido (`++`, `!=`, `*`):

```cpp
T* begin() { return m_data; }
T* end()   { return m_data + m_size; }   // ← one-past-the-end, como el STL
```

## Nivel #3b — overload de conveniencia (y el truco `::`)

```cpp
template <typename Container, typename Func, typename... Args>
void ApplyFunction(Container &container, Func func, Args... args) {
    ::ApplyFunction(container.begin(), container.end(), func, args...);
}
```

Azúcar para escribir `ApplyFunction(vec, f)` en vez de
`ApplyFunction(vec.begin(), vec.end(), f)`. El `::` inicial disambigua: sin él,
el nombre `ApplyFunction` dentro de esta misma función podría resolverse contra
una versión miembro del contenedor (la del nivel #0) — `::` garantiza la función
libre global.

## Nivel #4 — range-based for (y el bug que versionó el profe)

```cpp
for (auto iter : container)   // ⚠ 'iter' es un ELEMENTO, no un iterador
    func(*iter, args...);
```

La intención era azúcar sintáctica, pero `auto` captura el **elemento** (`T`),
no un iterador. Con `TX = int`, `*iter` intenta desreferenciar un `int`.
Error real capturado compilando `04-foreach4`:

```
foreach.h:38:14: error: invalid type argument of unary ‘*’ (have ‘int’)
    required from: ApplyFunction(Vector<int>&, void (*)(int&), ...)   // Demos.cpp:38
    required from: ApplyFunction(Vector<string>&, ..., {const char*}) // Demos.cpp:62
```

**El fix llegó en `05-iterator`** — el nombre correcto y la referencia:

```cpp
for (auto &v : container)   // 'v' es un elemento, por referencia
    func(v, args...);       // sin '*'
```

Lección de estilo: el nombre de la variable **importa**. `iter` llamó al error;
`v` lo delata.

## Matriz de compilación verificada

| Árbol | Compila | Evidencia |
|-------|---------|-----------|
| rama `foreach` (0.432) | ❌ | `error: expected unqualified-id before 'if'` — el `push_back` sin firma heredado de 0.43 |
| `03-Foreach3` | ✅ (corre todo) | warning único: `read()` sin return (`-Wreturn-type`) |
| `04-foreach4` | ❌ | `invalid type argument of unary '*' (have 'int')` en foreach.h:38 |
| `05-iterator` | ✅ | nivel #4 corregido con `auto &v` |

## Variadic templates: el `typename... Args`

- En la **firma**: `typename... Args` declara un *parameter pack* — cero, uno o
  N tipos adicionales.
- En la **llamada**: `args...` expande el pack en orden, separado por comas:

```cpp
::ApplyFunction(vec, AddX<TX>, 5);        // Args = {int},        args = (5)
::ApplyFunction(strVec, AddX<string>, "!"); // Args = {const char*}, args = ("!")
```

- El compilador genera una instancia por cada combinación — el error de
  `04-foreach4` lo muestra textualmente: `[with Args = {int}]` y
  `[with Args = {const char*}]`.

Sin variadics, habría que escribir `ApplyFunction1`, `ApplyFunction2`...
(conombres por aridad). Con ellos, una sola función.

## Los callbacks que acepta `Func`

```cpp
void AddOne(TX &v) { v += 1; }                    // función
template <typename T> void AddX(T &v, T x) {...}  // función template → AddX<TX>
::ApplyFunction(vec, [](TX &v){ v *= 2; });       // lambda (closure anónima)
struct Times2 { void operator()(TX &v) { v *= 2; } }; // functor (objeto invocable)
```

Todos inlinan en compile-time: `Func` es un parámetro de tipo, no un puntero a
función — cero indirección en runtime.

## Lecciones de esta escalera

1. **De adentro hacia afuera**: primero funciona acoplado (#0), luego se
   desacopla por capas hasta el nivel STL (#3a).
2. **El código comentado como documentación viva**: el profe conservó los
   niveles 1 y 2 comentados — la historia de la abstracción queda en el archivo.
3. **Los niveles "roto-arreglado" son parte del método**: 04-foreach4 se
   versionó sin compilar, y el fix llegó en la rama siguiente.
4. **La abstracción correcta elimina conocimiento**: #3a no sabe qué es un
   contenedor, y por eso sirve para todos.
