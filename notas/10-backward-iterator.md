# 10 · Iterador hacia atrás y test de recorrido

> **Ramas cubiertas:** `13-BackwardIterator`, `14-TestTraversal` — **etapa final del curso**
> **Estado:** contenido inicial — pendiente de revisar con lo visto en clase

## 📋 Ficha

| Rama | Commit(s) | Archivos tocados | Tema |
|------|-----------|------------------|------|
| `13-BackwardIterator` | `2e2a3fa` "Add VectorBackwardIterator and rbegin()/rend()" | `containers/vector.h` (+18) | Iterador backward |
| `14-TestTraversal` | `1181739` "Recorridos ok" | `foreach.h` (forwarding), `Demos.cpp` (TestTraversal), `vector.h` (TODOs) | Test de recorridos |
| `14-TestTraversal` | `7734aab` "TODOs updated" | `containers/vector.h`, `Demos.cpp`, `foreach.h` | Limpieza de TODOs |

*Verificado para estas notas (árbol `14` + `#include <utility>`): compila con
0 errores y corre — `Forward traversal: [(0,10)...(9,19)]`,
`Backward traversal: [(9,19)...(0,10)]`.*

## 🎯 En qué estábamos

Cierre del curso: el `Vector` es template, con `Node`, mutex, initializer_list
y iterador CRTP. Falta recorrerlo **al revés** — y acá el diseño de las notas
04/08 paga todo lo que sembró: el mismo `ApplyFunction` genérico recorre
adelante y atrás sin tocar una línea.

## 🧩 La implementación

### El iterador backward (`2e2a3fa`)

Misma maquinaria CRTP, una sola diferencia — el `++` que decrece:

```cpp
template <typename T>
class VectorBackwardIterator : public GeneralIterator<VectorBackwardIterator<T>, T> {
public:
    using MySelf = VectorBackwardIterator<T>;
    using Parent = GeneralIterator<MySelf, T>;
    using Parent::Parent;

    VectorBackwardIterator& operator++() { --Parent::m_ptr; return *this; }
    //                                     ↑ todo lo demás igual; ++ avanza "lógicamente"
};
```

Se cuelga del traits y del `Vector` con la convención de nombres del STL:

```cpp
struct VectorAscTraits {
    using value_type        = T;
    using Node              = GeneralNode<T>;
    using ForwardIterator   = VectorForwardIterator<Node>;
    using BackwardIterator  = VectorBackwardIterator<Node>;   // nuevo
};

BackwardIterator rbegin() { return BackwardIterator(m_data + m_size - 1); }  // último
BackwardIterator rend()   { return BackwardIterator(m_data - 1); }           // uno antes del primero
```

**El payoff (`1181739`)** — `TestTraversal` recorre en ambos sentidos con el
mismo algoritmo genérico de la nota 03:

```cpp
template <typename Node>
void PrintNode(Node &node, ostream &os) { os << node << " "; }

template <typename Container>
void TestTraversal(Container &container) {
    using Node = typename Container::Node;
    cout << "Forward traversal:  [";
    ::ApplyFunction(container.begin(), container.end(), PrintNode<Node>, cout);
    cout << "]" << endl;
    cout << "Backward traversal: [";
    ::ApplyFunction(container.rbegin(), container.rend(), PrintNode<Node>, cout);
    cout << "]" << endl;
}
```

Output verificado:

```
Forward traversal:  [(0,10) (1,11) (2,12) ... (9,19)]
Backward traversal: [(9,19) (8,18) (7,17) ... (0,10)]
```

**El regalo técnico de `14`: perfect forwarding en `ApplyFunction`:**

```cpp
// args se reenvia en cada iteracion del for: si Args deduce un rvalue real
// (no una referencia), forward lo moveria repetidamente en cada llamada a
// func, dejandolo invalido despues de la primera. Usar args solo para
// referencias/constantes compartidas entre elementos (streams, valores a
// sumar, etc.), no para recursos que func deba consumir/mover.
template <typename Iterator, typename Func, typename... Args>
void ApplyFunction(Iterator begin, Iterator end, Func func, Args&&... args) {
    for (auto iter = begin; iter != end; ++iter)
        func(*iter, std::forward<Args>(args)...);
}
```

Y en `vector.h`, los TODOs actualizados apuntan a consolidar:
`write()` → *"convertirla en una linea que usa la funcion ApplyFunction
generica"*; el `ApplyFunction` miembro gana `lock_guard` y el TODO
*"reutilizar la funcion ApplyFunction generica de foreach.h"* — una sola
fuente de verdad pendiente.

## 📚 Conceptos nuevos

- **Iteradores reversos**: misma interfaz, semántica invertida — `++` retrocede
  físicamente. La convención STL (`rbegin`/`rend`) permite que el algoritmo no
  se entere de la dirección.
- **One-past-the-end en reversa**: `rend() = m_data - 1` — el análogo invertido
  del `end()`. (El STL real lo resuelve distinto: `reverse_iterator` guarda el
  puntero +1 y desreferencia con offset — ver problemas.)
- **Perfect forwarding**: `Args&&...` (referencias de reenvío, deducen lvalue o
  rvalue) + `std::forward<Args>(args)...` preservan la categoría del argumento
  en cada llamada al callback. El comentario del código enseña el riesgo: en un
  loop, mover el mismo pack N veces invalida los recursos desde la 2ª llamada.
- **Single source of truth** (aspiración de los TODOs): un solo `ApplyFunction`
  para member/free/write — la deuda que el curso deja abierta a propósito.

## 🐛 Problemas detectados en esta etapa

1. **`rbegin()` sobre vector vacío = segfault** (verificado con mini-test):
   `m_data + m_size - 1` con `m_size == 0` apunta a `m_data - 1` y
   desreferenciarlo crashea — misma familia que el bug del `write()` vacío
   (notas 02/08): nunca hubo guard para vacíos.
2. **`rend() = m_data - 1` es formalmente UB**: el estándar solo garantiza la
   validez del puntero one-*past*-the-end; el one-*before*-first no existe en
   C++. `std::reverse_iterator` lo esquiva guardando `m_data + m_size`
   internamente. Funciona en la práctica, pero es un detalle fino de estándar.
3. **Solo hay `rbegin()/rend()` no-const** — falta el par `crbegin()/crend()`
   y las versiones const para recorrer vectores constantes.
4. **La copia del algoritmo persiste**: el TODO de "reutilizar la función
   genérica" reconoce que hay 3 versiones vivas de ApplyFunction (member,
   container, iterator-pair) — la consolidación quedó pendiente.
5. El `#include <utility>` faltante sigue (el forwarding usa `std::forward`,
   que sí está incluido en `foreach.h` — pero el `std::exchange` de vector.h
   necesita el include que verificamos en la nota 04).

## 👨‍🏫 Lo que explicó el profe

> ⚠ *Por completar con lo dicho en clase.*

- ¿Por qué `operator++` y no `operator--` en el backward? *(convención STL)*
- Cuándo copiar el patrón rbegin/rend en la lista enlazada del proyecto final.

## ✍️ Mi práctica

- [ ] Verificar el output forward/backward con el fix de `<utility>`
- [ ] Provocar el segfault del `rbegin()` vacío y agregar el guard
- [ ] Reescribir `rend()` al estilo `std::reverse_iterator` (guardar `m_data + m_size`)
- [ ] (Reto) Consolidar los TODOs: `write()` y el miembro `ApplyFunction`
      delegando en el genérico de `foreach.h`

## 🔗 Referencias

- `git diff upstream/12-InitializerList upstream/14-TestTraversal`
- [cppreference — reverse_iterator](https://en.cppreference.com/w/cpp/iterator/reverse_iterator)
- [cppreference — std::forward](https://en.cppreference.com/w/cpp/utility/forward)

---

## 🏁 Cierre del arco del curso

De `02-Vector` (0.1: copia de IA con `unsigned long long` y sin guard) a
`14-TestTraversal`: **template → operator<< → foreach (4 niveles) → iterador
CRTP → traits → Makefile → testing → mutex → Node → initializer_list →
backward + forwarding**. El hilo conductor que repiten todas las notas:
*el código de IA da un punto de partida, no un destino* — cada etapa encontró
un límite, lo nombró con un concepto (CRTP, forwarding, RAII...) y lo versionó
roto-para-arreglarlo. Quedan abiertas como tarea: los guards para vacíos, el
`<utility>`, la consolidación de ApplyFunction y el `read()` — el repo sigue
ahí para practicar.
