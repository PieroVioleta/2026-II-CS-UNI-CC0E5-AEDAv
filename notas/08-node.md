# 08 · Node (value, ref)

> **Ramas cubiertas:** `11-Node`
> **Estado:** contenido inicial — pendiente de revisar con lo visto en clase

## 📋 Ficha

| Rama | Commit(s) | Archivos tocados | Versión |
|------|-----------|------------------|---------|
| `11-Node` | `791e75c` "Adapt Vector to store Node (value, ref) pairs" | `containers/vector.h`, `types.h`, `Demos.cpp` | — |

*Verificado para estas notas: el árbol compila (con el `#include <utility>` de
la nota 04) y corre — output `[(0,10),(1,11),...,(9,19)]` exacto.*

## 🎯 En qué estábamos

El `Vector` guardaba valores sueltos. Ahora cada elemento pasa a ser una
**pareja (valor, referencia)** — el profesor introduce la abstracción `Node`,
base conceptual de las listas enlazadas que vienen después. La pregunta de
diseño: ¿cuánto del contenedor hay que tocar? La respuesta: gracias al
**traits** (nota 04), mucho menos de lo que parece.

## 🧩 La implementación

### Antes (`10-Concurrency`)

`m_data` era `value_type*` (T), `push_back(value)`, el iterador recorría `T`.

### Después (esta etapa)

**El nuevo tipo, en `vector.h`:**

```cpp
template <typename T>
struct GeneralNode {
private:
    T   m_value;
    Ref m_ref;                       // "Reference to the value" (metadata del usuario)
public:
    GeneralNode() = default;         // requerido por resize(): new Node[new_cap]
    GeneralNode(const T& value, Ref ref) : m_value(value), m_ref(ref) {}
    T    getValue() const { return m_value; }
    Ref  getRef()   const { return m_ref;   }
    T&   value()          { return m_value; }  // acceso mutable para ApplyFunction

    friend ostream &operator<<(ostream &os, const GeneralNode<T> &node) {
        os << "(" << node.getValue() << "," << node.getRef() << ")";
        return os;
    }
};
```

Con el nuevo alias en `types.h`:

```cpp
using TX  = int;
using Ref = long long;
```

**El traits absorbe el cambio** — y el CRTP cobra su deuda sin moverse:

```cpp
template <typename T>
struct VectorAscTraits {
    using value_type      = T;
    using Node            = GeneralNode<T>;
    using ForwardIterator = VectorForwardIterator<Node>;  // ← itera Node, no T
};
```

`GeneralIterator` (CRTP) quedó **intacto** — el iterador es genérico y ahora
parametriza sobre `Node`. En `Vector`, el cambio es mecánico:
`m_data` pasa a `Node*`, `operator[]`/`at()` devuelven `Node&`, y:

```cpp
void push_back(const value_type& value, Ref ref) {   // ← firma nueva
    lock_guard<mutex> lock(m_mutex);
    ...
    m_data[m_size] = Node(value, ref);
    ++m_size;
}
```

**El formato de impresión cambia**: `[(0,10),(1,11),...]` (comas, sin endl
final). Los callbacks de foreach se adaptan a la nueva unidad:

```cpp
void AddOne(GeneralNode<TX> &node) { node.value() += 1; }      // solo el valor
void AddX(GeneralNode<T> &node, T x) { node.value() += x; }    // el ref NO se toca
```

Y el demo genera las parejas: `{{0, 10}, {1, 11}, ..., {9, 19}}` — la "ref"
aquí es un dato de ejemplo (valor+10); en `DemoRaceCondition` llega como
`values[i].first/.second` (pares STL).

**Output verificado:**

```
Container after ApplyFunction: [(1,10),(2,11),...,(10,19)]   ← value+1, ref intacto
```

## 📚 Conceptos nuevos

- **`Node` como unidad de almacenamiento**: valor + metadata — la semilla de
  las listas enlazadas (donde el ref será un puntero al siguiente).
- **Traits como tabla de indirección**: cambiar *qué* guarda el contenedor
  (T → Node) se hizo en un solo lugar (`VectorAscTraits`); `Vector` y el
  iterador CRTP se adaptaron sin rediseño.
- **`sizeof` y padding** (verificado): `sizeof(GeneralNode<int>) = 16` —
  `int` (4B) + padding (4B) + `long long` (8B); el compilador alinea el
  `long long` a 8. Guardar `int`+`long long` cuesta el doble que los datos.
- **`GeneralNode() = default`**: necesario porque `new Node[new_cap]`
  exige default-constructible — la limitación clásica del array dinámico
  (el STL real usa memoria cruda + placement new, nota 01 problema 6).
- **Separación de accesos**: `getValue()/getRef()` (lectura) vs `value()`
  (mutación) — hace explícito qué puede cambiar el foreach.

## 🐛 Problemas detectados en esta etapa

1. **El bug del vector vacío sigue vivo** (verificado aquí): el guard
   `if (size() > 0)` de la nota 02 nunca se arregló — `write()` sobre un
   `Vector` vacío da `Segmentation fault` a día de hoy (sobrevive desde
   0.4 pasando por 5 ramas).
2. **El foreach pierde transparencia**: el nivel #3a prometía
   `func(*iter, args...)` sin conocer el contenedor — pero ahora `func` debe
   conocer `GeneralNode`. La abstracción de foreach se mantiene en el código,
   se rompe en el contrato: los callbacks ya no son "operaciones sobre T".
3. **`read()` sigue vacío** (warning `-Wreturn-type` persiste, verificado).
4. **Aritmética de Node**: 16 bytes por elemento donde los datos ocupan 12 —
   con 200 000 elementos son ~800 KB de padding. (En curso está bien; en
   producción, `#pragma pack` o reordenar campos.)
5. Sigue el `#include <utility>` faltante (verificado: sin él no compila).

## 👨‍🏫 Lo que explicó el profe

> ⚠ *Por completar con lo dicho en clase.*

- ¿Para qué sirve en la vida real guardar una ref junto al valor? *(completar)*
- Conexión anticipada: `GeneralNode` → lista enlazada → árboles.

## ✍️ Mi práctica

- [ ] Reproducir el output exacto: `[(0,10)...(9,19)]` → ApplyFunction → `(1,10)...`
- [ ] Imprimir `sizeof` del Node con `T=char` vs `T=double` y explicar el padding
- [ ] Probar `write()` sobre vector vacío → segfault; arreglarlo UNA vez y
      portarlo a la rama que corresponda
- [ ] (Reto) Hacer que un callback solo reciba `T&` (no Node) usando el traits

## 🔗 Referencias

- `git diff upstream/10-Concurrency upstream/11-Node`
- [cppreference — data structure alignment/padding](https://en.cppreference.com/w/cpp/language/data_members)
- **Nota 02** (el bug del `size()-1` que persiste)
