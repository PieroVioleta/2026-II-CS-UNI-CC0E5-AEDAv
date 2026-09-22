# 03 · foreach y ApplyFunction

> **Ramas cubiertas:** `foreach` (0.431→0.432), `03-Foreach3`, `04-foreach4`
> **Estado:** contenido inicial — pendiente de revisar con lo visto en clase

## 📋 Ficha

| Rama | Commit(s) | Archivos tocados | Versión |
|------|-----------|------------------|---------|
| `foreach` ⚠ | `44abac0` "Vector o.431" | `containers/vector.h` (+2 comentarios) | 0.431 |
| `foreach` ⚠ | `39d42a3` "Vector 0.432" | `containers/vector.h` (1±) | 0.432 |
| `03-Foreach3` | `9989bf5` "Variadic Template" | `containers/vector.h` (30±), `Demos.cpp` (43±) | — |
| `03-Foreach3` | `a73d619` "Foreach Level 3 added" | `foreach.h` (nuevo, 34 l) | — |
| `04-foreach4` | `5ca72aa` "ApplyFunction Level 4 updated" | `foreach.h` (7±) | — |

⚠ Rama borrada en upstream, sigue disponible como rama local.

## 🎯 En qué estábamos

El TODO dejado en 0.43 mandaba: *«aplicarle una función a cada elemento, ej.
sumarle un valor x»*. Esta etapa lo implementa — y de paso sube 4 peldaños de
abstracción. Ojo: la rama lateral `foreach` **no tiene foreach.h**; es solo un
retoque estético de `vector.h` (y hereda el `push_back` roto de 0.43).

## 🧩 La implementación

> 🔬 Esta etapa tiene un **deep-dive aparte** con la escalera de niveles
> explicada peldaño a peldaño y compilaciones verificadas:
> **[03a-escalera-applyfunction.md](03a-escalera-applyfunction.md)**

### Antes (`02-Vector` / `04-operator`)

Para operar sobre cada elemento había que escribir el loop a mano cada vez, o
modificar `DemoVector()` para cada operación nueva.

### Después (esta etapa)

**Nivel #0 — método miembro con índices** (`9989bf5`, dentro de `Vector`):

```cpp
template <typename Func, typename... Args>   // ← variadic template
void ApplyFunction(Func func, Args... args) {
    for (size_t i = 0; i < size(); ++i)
        func(m_data[i], args...);            // ← expande el pack
}
```

**Mismo commit, bonus:** `print()` se renombra a **`write()`**, aparece
`read(istream&)` (¡con implementación vacía!) y `operator>>`; y `Vector` gana
**`begin()/end()` con raw pointers** — lo mínimo para ser iterable:

```cpp
T* begin() { return m_data; }
T* end()   { return m_data + m_size; }
```

**Niveles 1→3 en `foreach.h`** (`a73d619`) — la progresión de abstracción,
conservando los niveles previos comentados como documentación:

```cpp
// Nivel #1 (comentado): libre con índices → requiere conocer el Container
// Nivel #2 (comentado): libre con container.begin()/end()

// Nivel #3a (activo): el motor, sobre un PAR DE ITERADORES
template <typename Iterator, typename Func, typename... Args>
void ApplyFunction(Iterator begin, Iterator end, Func func, Args... args) {
    for (auto iter = begin; iter != end; ++iter)
        func(*iter, args...);
}

// Nivel #3b (activo): conveniencia sobre un Container
template <typename Container, typename Func, typename... Args>
void ApplyFunction(Container &container, Func func, Args... args) {
    ::ApplyFunction(container.begin(), container.end(), func, args...);
    //  ↑ :: para llamar la sobrecarga global, no la miembro
}
```

**Nivel #4 — range-based for** (`5ca72aa`): reemplaza al 3b:

```cpp
for (auto iter : container)   // ⚠ 'iter' es un ELEMENTO (T), no un iterador
    func(*iter, args...);     // ← desreferencia un int... ¿compila?
```

**Demos muestra el poder real** — funciones, funciones template y tipos
distintos sobre el mismo contenedor:

```cpp
::ApplyFunction(vec, AddOne);            // sumar 1
::ApplyFunction(vec, AddX<TX>, 5);       // sumar x (función template)
::ApplyFunction(vec, Square);            // elevar al cuadrado

Vector<string> strVec;                   // ← el template ya es genérico
::ApplyFunction(strVec, AddX<string>, "!");
```

## 📚 Conceptos nuevos

- **Variadic templates**: `typename... Args` acepta cualquier número de
  argumentos; `args...` expande el pack en la llamada. Es el "..." que permite
  pasar parámetros extra a la función aplicada.
- **Callback como parámetro template**: `Func` puede ser función, función
  template instanciada (`AddX<TX>`), functor o lambda — todo sin costo
  (inlining en compile-time, no function pointers).
- **Raw pointers como iteradores**: `begin()/end()` devolviendo `T*` ya
  cumplen el protocolo de iteradores (`++`, `!=`, `*`). Es exactamente como
  funciona `std::vector` por dentro.
- **La escalera de abstracción**: índices (#0/#1) → iteradores del
  contenedor (#2/#3) → par de iteradores puro (#3a, el estándar STL) →
  range-for (#4).
- **`::ApplyFunction`**: el scope resolution fuerza la función libre cuando
  podría haber ambigüedad con métodos del propio contenedor.

## 🐛 Problemas detectados en esta etapa

1. **Nivel #4 no compila**: `for (auto iter : container)` hace que `iter` sea
   un **elemento** (`T`), no un iterador — `*iter` intenta desreferenciar un
   `int`. Será corregido cuando existan iteradores de verdad (nota 04).
   Lo coherente sería: `for (auto &elem : container) func(elem, args...);`
2. **`read()` quedó vacío**: `operator>>` existe pero la "Implementation for
   reading vector from stream" es un comentario — deuda abierta.
3. El comentario en Demos dice *"after applying lambda function"* pero
   `AddOne`/`Square` son funciones y `AddX` es una función template — no hay
   lambdas (aún).
4. La rama lateral `foreach` (0.431/0.432) sigue arrastrando el `push_back`
   roto heredado de 0.43.
5. `write()` mantiene el `endl` interno y el `operator<<` sin `const` (notas 02).

## 👨‍🏫 Lo que explicó el profe

> ⚠ *Por completar con lo dicho en clase.*

- ¿Por qué el nivel #3a (par de iteradores) es "el nivel STL" y los demás no?
- ¿Cuándo conviene `typename... Args` vs `std::function`?

## ✍️ Mi práctica

- [ ] Reproducir los 4 niveles y verificar cuáles compilan con `TX=int`
- [ ] Aplicar una lambda real: `::ApplyFunction(vec, [](TX &v){ v *= 2; });`
- [ ] Implementar `read()` de verdad y probar `cin >> vec`
- [ ] Romper a propósito el `begin()/end()` y observar el error del nivel #3

## 🔗 Referencias

- `git show upstream/03-Foreach3:foreach.h`
- `git diff 'operator<<' upstream/04-foreach4`
- [cppreference — parameter pack](https://en.cppreference.com/w/cpp/language/parameter_pack)
- [cppreference — range-based for](https://en.cppreference.com/w/cpp/language/range-for)
