# 02 · Impresión del vector (operator<<)

> **Ramas cubiertas:** `operator<<` (Vector 0.4→0.42), `04-operator` (Vector 0.43)
> **Estado:** contenido inicial — pendiente de revisar con lo visto en clase

## 📋 Ficha

| Rama | Commit(s) | Archivos tocados | Versión |
|------|-----------|------------------|---------|
| `operator<<` ⚠ | `a1faacd` "Vector 0.4" | `containers/vector.h` (70±), `Demos.cpp/h`, `types.h` (nuevo), `main.cpp`, `vector.txt` (nuevo) | 0.4 |
| `operator<<` ⚠ | `ffe56f4` "Vector 0.41" | `Demos.cpp` (1 l) | 0.41 |
| `operator<<` ⚠ | `5f7d48e` "Vector 0.42" | `containers/vector.h` (1 l) | 0.42 |
| `04-operator` | `66d3014` "Vector 0.43" | `containers/vector.h` (4 l) | 0.43 |

⚠ Rama borrada en upstream, sigue disponible como rama local.

## 🎯 En qué estábamos

Con el `Vector` copiado de IA funcionando (nota 01), esta etapa responde dos
cosas: **cómo imprimir el vector sin repetir el loop** y, de paso, **cómo dejar
de estar casados con `int`**.

## 🧩 La implementación

### Antes (`02-Vector`)

Imprimir era copiar el loop a mano en cada lugar:

```cpp
cout << "Vector contents: ";
for (int i = 0; i < vec.size(); ++i)
    cout << vec[i] << " ";
```

Y el tipo estaba fijado por un alias dentro del header: `using T = int;`.

### Después (esta etapa)

**El commit grande es 0.4 (`a1faacd`) — `Vector` se vuelve template:**

```cpp
- using T = int;
+ template <typename T>
  class Vector {
```

Consecuencias en cascada del cambio: `data` → `m_data` (convención `m_` ahora
completa), índices y tamaños pasan de `int` a `size_t`, y **`operator[]` ahora
también chequea límites**:

```cpp
T& operator[](size_t index) {
    if (index >= m_size) throw std::out_of_range("Indice fuera de rango");
    return m_data[index];
}
```

**La impresión** — método miembro + `operator<<` libre que delega:

```cpp
ostream &print(ostream &os) {
    os << "[";
    for (size_t i = 0; i < size()-1; ++i)
        os << m_data[i] << " ";
    if (size() > 0)
        os << m_data[size()-1];
    return os << "]" << endl;
}

template <typename T>
ostream& operator<<(ostream &os, Vector<T> &vec) {
    return vec.print(os);
}
```

**El alias del tipo emigra a `types.h`** (nuevo archivo):

```cpp
// types.h
using TX = int;   // Demos ahora escribe Vector<TX>
```

**Demos muestra 3 formas de salida** — `cout`, `ofstream` (archivo
`vector.txt`), y el operador:

```cpp
cout << "Vector using print: ";
vec.print(cout);

ofstream of("vector.txt");
vec.print(of);            // ¡el mismo print() sirve para el archivo!
of.close();

cout << "Vector using cout directly: ";
cout << vec << endl;
```

**Los commits pequeños:**

| Commit | Qué cambió |
|--------|-----------|
| 0.41 | Typo en Demos: "Vector using **count** directly" → "cout" |
| 0.42 | `operator<<` pierde el `const`: `const Vector<T>&` → `Vector<T>&` ⚠ regresión |
| 0.43 | Agrega TODO + **rompe `push_back`** (ver abajo) |

**El TODO que siembra la próxima etapa** (0.43):

```cpp
// TODO: aplicarle una funcion a cada elemento.
//       ej. sumarle un valor x
```

## 📚 Conceptos nuevos

- **Templates vs alias de tipo**: `using T = int` solo renombra (un `Vector`
  de enteros); `template <typename T>` parametriza la clase (un `Vector` de
  lo que sea). La IA original confundía las dos cosas.
- **Abstracción de streams**: `std::ostream` es la interfaz común de `cout`,
  `ofstream`, `stringstream`... por eso `print(ostream&)` sirve para pantalla
  y archivo con el mismo código.
- **`operator<<` como función libre**: no puede ser miembro (`os << vec` tiene
  al stream a la izquierda); patrón: delega en un método de la clase.
- **`size_t`**: tipos sin signo para tamaños/índices — evita mezclar
  signed/unsigned en comparaciones.
- **const-correctness**: imprimir no modifica → parámetros y métodos deberían
  ser `const`.

## 🐛 Problemas detectados en esta etapa

1. **Bug del vector vacío en `print()`**: cuando `size() == 0`,
   `size()-1` con `size_t` hace *underflow* → `SIZE_MAX`, el loop corre y lee
   `m_data[0]` sobre un `nullptr` → crash. El guard `if (size() > 0)` está
   **después** del loop, no lo protege:
   ```cpp
   for (size_t i = 0; i < size()-1; ++i)   // ← con size()==0: i < SIZE_MAX
   ```
2. **Regresión 0.42**: `operator<<` cambió a `Vector<T>&` sin `const` → ya no
   se puede imprimir un vector const. Causa raíz: `print()` no es método
   `const`. Lo correcto: `void print(ostream&) const` + `operator<<` con
   `const Vector<T>&`.
3. **`endl` dentro de `print()`**: fuerza flush y asume que el que imprime
   quiere newline (convención STL: no agregarlo).
4. **`using namespace std;` en un header**: contamina el namespace global de
   todo el que incluya `vector.h`; además deja `ostream` sin calificar.
5. **`operator[]` ahora lanza**: se aleja de la convención de `std::vector`
   (op[] sin chequeo, `at()` con chequeo). Diseño discutible — comentar en clase.
6. **0.43 rompe `push_back`**: el commit borra la línea `void push_back(T value) {`
   y queda el cuerpo huérfano:
   ```cpp
       if (m_size == m_capacity) {   // ← código que no compila, versionado igual
   ```
   Lección: **no commitear sin compilar**.
7. Sigue el destructor comentado (`~T()`) de la nota 01.

## 👨‍🏫 Lo que explicó el profe

> ⚠ *Por completar con lo dicho en clase.*

- ¿Por qué el TODO de 0.43 apunta a foreach y no a otra cosa?
- Convención `operator[]` con o sin chequeo: qué hace el estándar y por qué.

## ✍️ Mi práctica

- [ ] Reproducir el bug del vector vacío: `Vector<int> v; cout << v;` → crash
- [ ] Arreglar: `print()` const + guard al inicio + `operator<<` const
- [ ] Verificar que `cout << vec` deja de compilar sin el `const` de 0.42
- [ ] Escribir en `vector.txt` con `ofstream` y con `ostringstream` (mismo print)
- [ ] Reconstruir `push_back` roto de 0.43 y compilar

## 🔗 Referencias

- `git diff upstream/02-Vector 'operator<<'` (0.4 completo)
- `git diff 'operator<<' upstream/04-operator` (0.43, código roto)
- [cppreference — operator<< para contenedores](https://en.cppreference.com/w/cpp/language/operators)
- [C++ Core Guidelines — const correctness](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#con-const-and-immutability)
