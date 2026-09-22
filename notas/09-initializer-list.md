# 09 · Initializer list

> **Ramas cubiertas:** `12-InitializerList`
> **Estado:** contenido inicial — pendiente de revisar con lo visto en clase

## 📋 Ficha

| Rama | Commit(s) | Archivos tocados | Versión |
|------|-----------|------------------|---------|
| `12-InitializerList` | `3e9d58a` "Add Vector(initializer_list\<pair\<value_type, Ref\>\>) constructor" | `containers/vector.h` (+7), `Demos.cpp` (4±) | — |

*Verificado para estas notas: compila (con el fix `<utility>` de la nota 04)
y corre; output final idéntico al de la nota 08.*

## 🎯 En qué estábamos

Después del `Node` (nota 08), construir un `Vector` era declararlo vacío y
pushear de a uno:

```cpp
Vector<VectorAscTraits<TX>> vec;
TestContainer(vec, {{0, 10}, {1, 11}, ...}, ...);   // el TestContainer pushea
```

Esta etapa agrega el constructor que el STL tiene desde C++11: **armar el
vector de una vez con sintaxis de llaves**.

## 🧩 La implementación

### Antes (`11-Node`)

No existía forma de inicializar con valores directamente — solo `Vector() {}`
y crecer con `push_back`.

### Después (esta etapa)

```cpp
#include <initializer_list>   // nuevo include

// Cada elemento de la lista es una pareja (valor, ref) para un Node
Vector(initializer_list<pair<value_type, Ref>> values) {
    for (const auto &v : values)
        push_back(v.first, v.second);
}
```

Y el demo lo usa — el `Vector` nace con 5 elementos y el resto entra por el
camino de siempre:

```cpp
Vector<VectorAscTraits<TX>> vec({{0, 10}, {1, 11}, {2, 12}, {3, 13}, {4, 14}});
TestContainer(vec, {{5, 15}, {6, 16}, {7, 17}, {8, 18}, {9, 19}}, "vector.txt", AddOne);
```

**Verificaciones hechas para estas notas:**

```
Vector<VectorAscTraits<TX>> v = {{1, 100}, {2, 200}};   // ✅ compila:
// conversión implícita — el ctor NO es explicit
Vector<VectorAscTraits<TX>> w({{3, 300}});               // ✅ forma con paréntesis
// output: direct: [(1,100),(2,200)] · parens: [(3,300)]
```

## 📚 Conceptos nuevos

- **`std::initializer_list<T>`**: objeto liviano (puntero + tamaño) sobre un
  arreglo temporal implícito que el compilador crea para `{...}`. Solo lectura.
  Es lo que permite que `{1, 2, 3}` llegue a un constructor como una unidad.
- **Brace-init anidada**: `{{0, 10}, {1, 11}}` — cada `{a, b}` interno se
  descompone en un `pair<value_type, Ref>`; el externo es el
  `initializer_list` de pares. La estructura de llaves refleja la estructura
  del tipo.
- **Conversión implícita vs `explicit`**: sin `explicit`, un
  `Vector<T> v = {{...}}` (copy-initialization) compila. Con `explicit`,
  exigiría `Vector<T> v({...})`. ¿Cuándo conviene cada uno? (El STL hace
  implícito a propósito.)
- **Delegación en `push_back`**: el ctor no duplica la lógica (mutex, growth,
  `Node(value, ref)`) — reutiliza la ruta crítica ya sincronizada.
  Alternativa incorrecta: copiar el cuerpo de `push_back` adentro.

## 🐛 Problemas detectados en esta etapa

1. **No reserva de una vez**: cada `push_back` del init-list puede pasar por
   `resize` — con la lista de 10 del demo, crece 0→10→(20). El
   `std::vector` real reserva el tamaño exacto y construye una vez. Mejora
   obvia: `reserve(values.size())` al inicio del ctor.
2. **Copia vs move innecesaria**: `push_back(const value_type&, Ref)` copia
   el valor de cada par; para strings grandes sería una copia evitable
   (`push_back(v.first, v.second)` podría mover). Menor, pero real.
3. Persisten los heredados: el bug del `write()` vacío (nota 02/08, segfault
   verificado), `read()` vacío, `#include <utility>` faltante (verificado
   otra vez acá).

## 👨‍🏫 Lo que explicó el profe

> ⚠ *Por completar con lo dicho en clase.*

- ¿Por qué el STL eligió llaves (`{}`) para el init-list y no otra sintaxis?
- ¿Qué pasa con `Vector v = {}` (lista vacía) — qué ctor gana?

## ✍️ Mi práctica

- [ ] Verificar el output del demo (idéntico a la nota 08, pero naciendo con 5)
- [ ] Agregar `reserve(values.size())` al ctor y contar las realocaciones
      (hint: imprimir dentro de `resize`)
- [ ] Agregar `explicit` al ctor y ver qué deja de compilar — discutir
- [ ] (Reto) Constructor análogo: `Vector(initializer_list<T>)` que llene
      refs en 0 (¿o mejor no existe? ¿se mezclaría con el de pares?)

## 🔗 Referencias

- `git diff upstream/11-Node upstream/12-InitializerList`
- [cppreference — initializer_list](https://en.cppreference.com/w/cpp/utility/initializer_list)
- [C++ Core Guidelines — explicit constructors](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-explicit)
