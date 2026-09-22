# 04 · Iteradores y CRTP

> **Ramas cubiertas:** `05-iterator`, `06-CRTP`, `07-Vector2.0`
> **Estado:** contenido inicial — pendiente de revisar con lo visto en clase

## 📋 Ficha

| Rama | Commit(s) | Archivos tocados | Diseño del iterador |
|------|-----------|------------------|---------------------|
| `05-iterator` | `5a020f2`, `568df9a` "Iterator V1.0", `abe4243`, `f570c76` (ambos "to be fixed") | `containers/vector.h` (56±), `Demos.cpp` (10±), `foreach.h` (4±) | v1: herencia naive, dentro de vector.h |
| `06-CRTP` | `096d64e` "CRTP", `49c01f0` "GeneralIterator created" | `containers/GeneralIterator.h` (nuevo, 27 l), `containers/vector.h` (30±) | v2: estilo F-bound + traits |
| `07-Vector2.0` | `289e3e1` "Vector 2.0", `77fd402` "with errors", `6abf3ae` "Generalterator fixed" | `containers/GeneralIterator.h` (9±), `containers/vector.h` (79±) | v3: CRTP verdadero + Vector por Traits |

> 🔬 La evolución del diseño tiene **deep-dive aparte**, con las 3 versiones
> lado a lado, la matriz de compilación verificada y la saga del include
> faltante: **[04a-general-iterator-crtp.md](04a-general-iterator-crtp.md)**

## 🎯 En qué estábamos

Después de foreach (nota 03) el contenedor ya era iterable con raw pointers.
Esta etapa construye un **iterador de verdad** como clase — y de paso
`Vector` se convierte en `Vector<Traits>` (Vector 2.0).

## 🧩 La implementación

### Antes (`04-foreach4`)

- Iteradores = raw pointers `T*` de `begin()/end()`.
- El nivel #4 de foreach estaba roto (`*iter` sobre el elemento, nota 03).
- El fix llegó aquí: `for (auto &v : container) func(v, args...);`

### Después (esta etapa — resumen; detalle en el deep-dive)

**v1 — dentro de `vector.h` (`05-iterator`):** `GeneralIterator<T>` con
`m_ptr` protegido, `operator*`, `friend operator!=`... pero con el
`operator*` duplicado y `VectorForwardIterator` usado antes de declararse.

**v2 — extracción a `GeneralIterator.h` (`06-CRTP`):** la base se templaliza
con el hijo: `class VectorForwardIterator : public GeneralIterator<VectorForwardIterator<T>>`
+ nacen los traits (`VectorAscTraits<T>`) y `Vector` pasa a ser
`template <typename Traits>`.

**v3 — CRTP verdadero (`07-Vector2.0`):**

```cpp
template <typename Derived, typename T>
class GeneralIterator {
    ...
    friend bool operator== (const Derived& a, const Derived& b);
    friend bool operator!= (const Derived& a, const Derived& b);
};

template <typename T>
class VectorForwardIterator : public GeneralIterator<VectorForwardIterator<T>, T> {
    using Parent = GeneralIterator<MySelf, T>;
    using Parent::Parent;                       // hereda ctor
    VectorForwardIterator& operator++() { ++Parent::m_ptr; return *this; }
};

template <typename Traits>
class Vector {
    ForwardIterator begin() { return ForwardIterator(m_data); }
    ForwardIterator end()   { return ForwardIterator(m_data + m_size); }
};
```

Uso real en Demos: `Vector<VectorAscTraits<TX>> vec;`

**Vector 2.0 también reescribió el contenedor:** move por `std::exchange`
en vez de swap, member initializers (`= nullptr`, `= 0`), `push_back(const T&)`
en vez de por valor, `virtual ~Vector`, y `reserve` renombrado a `resize`.

## 📚 Conceptos nuevos

- **Iterador como clase**: encapsula el puntero y expone `*`, `++`, `==`/`!=` —
  el protocolo que `ApplyFunction` ya consumía sin saberlo.
- **CRTP** (*Curiously Recurring Template Pattern*): la base recibe a la clase
  derivada como parámetro. Permite que la base conozca el tipo concreto del
  hijo **en compile-time** (polimorfismo estático, sin vptr).
- **`using Parent::Parent`**: herencia de constructores (C++11) — evita
  redeclarar el ctor del puntero.
- **`Parent::m_ptr`**: para entrar a miembros de una base dependiente de un
  template hace falta calificarlos (two-phase lookup).
- **Traits**: `VectorAscTraits<T>` agrupa `value_type` + `ForwardIterator` —
  `Vector` queda parametrizado por Traits y puede cambiar su iterador sin
  tocarlo (semilla del iterador backward, nota 10).
- **`std::exchange(a, v)`**: asigna `v` a `a` y devuelve el valor viejo — move
  members sin variable temporal (vive en `<utility>`).

## 🐛 Problemas detectados en esta etapa

1. **v1 no compila**: dos `operator*() const` con la misma firma
   (`reference` y `value_type&` eran lo mismo) → "cannot be overloaded"; y el
   `friend operator!=` usa `VectorForwardIterator` antes de su declaración →
   "does not name a type".
2. **v2 no compila**: la base extrae `value_type` del hijo incompleto
   (`typename Iterator::value_type` con `Iterator = VectorForwardIterator<T>`)
   → "invalid use of incomplete type" — dependencia circular.
3. **v3 no compila por UN include**: `std::exchange` vive en `<utility>`, que
   nunca se incluyó. Verificado: ese único `#include` roto persiste en
   `08-Makefile`, `10-Concurrency` (12 errores, todos el mismo) y hasta
   `14-TestTraversal`. Agregarlo hace compilar y correr todo el curso.
4. **"Generalterator fixed" nunca compiló**: lección — el mensaje del commit
   no es verificación.
5. **Renombre cuestionable**: `reserve` → `resize` — en `std::vector` son cosas
   distintas (`resize` cambia `size()`, `reserve` solo `capacity()`). Aquí la
   función solo crece la capacidad.
6. **`virtual ~Vector`**: destructor polimórfico en un contenedor de valores —
   agrega vptr a cada instancia; el STL mantiene `~vector` no virtual.
7. **`push_back(const T&)`** mejora el costo de copia, pero pierde la ruta de
   move que tenía la versión por valor (el STL tiene ambas sobrecargas).

## 👨‍🏫 Lo que explicó el profe

> ⚠ *Por completar con lo dicho en clase.*

- ¿Por qué CRTP y no polimorfismo clásico (`unique_ptr<IteratorBase>`)? *(completar)*
- El rol de los traits para el iterador backward.

## ✍️ Mi práctica

- [ ] Reproducir los 3 estados rotos: extraer cada rama y compilar (ver deep-dive)
- [ ] Aplicar el fix de `<utility>` y correr el demo completo hasta el final
- [ ] Explicar en voz alta qué resuelve `typename Derived` en la base
- [ ] (Reto) Agregar `operator--` bidireccional vía CRTP sin duplicar código

## 🔗 Referencias

- `git show upstream/06-CRTP:containers/GeneralIterator.h` (v2)
- `git show upstream/07-Vector2.0:containers/GeneralIterator.h` (v3)
- [cppreference — CRTP](https://en.cppreference.com/w/cpp/language/crtp)
- [cppreference — std::exchange](https://en.cppreference.com/w/cpp/utility/exchange)
