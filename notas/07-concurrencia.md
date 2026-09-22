# 07 · Concurrencia

> **Ramas cubiertas:** `10-Concurrency`
> **Estado:** contenido inicial — pendiente de revisar con lo visto en clase

## 📋 Ficha

| Rama | Commit(s) | Archivos tocados | Versión |
|------|-----------|------------------|---------|
| `10-Concurrency` | `49ceccf` "Concurrency still with race conditions" | `Demos.cpp/h` (demo de carrera, **sin** sincronizar) | — |
| `10-Concurrency` | `5575be8` "Add mutex to synchronize Vector against concurrent push_back" | `containers/vector.h` (mutex), `Demos.cpp` | — |

> 🔬 La carrera se **reprodujo de verdad** para estas notas (con y sin mutex).
> El experimento completo: **[07a-race-condition.md](07a-race-condition.md)**

## 🎯 En qué estábamos

Hasta acá, todo el código era single-thread. Esta etapa mete **5 threads
pusheando al mismo `Vector`** — primero se *muestra* la desastre (rojo), luego
se *arregla* con un mutex (verde). El método TDD en dos commits:
"still with race conditions" → "Add mutex".

## 🧩 La implementación

### El fix (commit `5575be8`)

```cpp
#include <mutex>          // nuevo include

template <typename Traits>
class Vector {
    ...
    mutex m_mutex;        // nuevo miembro

    void push_back(const value_type& value) {
        lock_guard<mutex> lock(m_mutex);   // ← RAII: lock al construir
        if (m_size == m_capacity) {
            size_t new_cap = (m_capacity == 0) ? 10 : m_capacity * 2;
            resize(new_cap);
        }
        m_data[m_size] = value;
        ++m_size;
    }                                      // ← unlock automático al salir

    void pop_back() {
        lock_guard<mutex> lock(m_mutex);
        if (m_size > 0)
            --m_size;
    }
```

Detalle: la capacidad inicial pasó de `1` a `10` (menos realocaciones tempranas
— con threads, cada `resize` es una ventana de riesgo).

### El demo que detecta la carrera (`Demos.cpp`)

```cpp
const int NThreads = 5;

// 5 workers, índices entrelazados: t, t+N, t+2N, ...
template <typename Container>
void ConcurrentInsert(Container &container,
                      const vector<typename Container::value_type> &values) {
    size_t n = values.size();
    vector<thread> workers;
    for (int t = 0; t < NThreads; ++t)
        workers.emplace_back([&container, &values, n, t]() {
            for (size_t i = t; i < n; i += NThreads)
                container.push_back(values[i]);
        });
    for (auto &worker : workers)
        worker.join();
}

void DemoRaceCondition() {
    const size_t N = 200000;                    // muchos elementos: ventana grande
    vector<TX> values(N);                       // generados en loop, no a mano
    ...
    ConcurrentInsert(vec, values);

    long long expectedSum = 0;
    for (TX v : values) expectedSum += v;       // lo que DEBERÍA dar
    long long actualSum = 0;
    for (size_t i = 0; i < vec.size(); ++i) actualSum += vec[i];

    if (vec.size() != N || actualSum != expectedSum)
        cout << "*** Race condition detectada: se perdieron inserciones ***";
    else
        cout << "No se perdio ningun elemento EN ESTA CORRIDA (la race sigue ahi)";
}
```

**Tres ideas de diseño de test concurrente, todas en el demo:**

1. **Volumen**: 200 000 elementos (vs 10 del demo normal) — la ventana de
   carrera se agranda hasta ser visible.
2. **Invariantes, no labels**: se compara contra una propiedad matemática
   (suma esperada + count), no contra texto impreso.
3. **No-determinismo confeso**: "No se perdió... EN ESTA CORRIDA" — pasar no
   prueba que no haya race; sugieren `-fsanitize=thread`.

También `TestContainer` se generalizó (insert concurrente + write + archivo en
modo append + ApplyFunction) y los archivos de salida se truncan al inicio de
`DemoVector`.

## 📚 Conceptos nuevos

- **Data race**: dos threads acceden la misma memoria, al menos uno escribe,
  sin sincronización → comportamiento indefinido.
- **`std::mutex` + `std::lock_guard`**: el lock se toma al construir y se
  libera al destruir (RAII) — imposible olvidar el unlock, incluso con excepciones.
- **`std::thread` / `join()`**: crear workers y esperarlos; la lambda captura
  el contenedor **por referencia** (`&`) — todos comparten el mismo objeto.
- **Interleaving de índices** (`i = t; i += NThreads`): reparte el trabajo sin
  bloqueos, pero el **orden** final de inserción queda a merced del scheduler.
- **Sección crítica**: el bloque protegido por el mutex — aquí todo el cuerpo
  de `push_back` (el `resize` incluido).
- **ThreadSanitizer** (`-fsanitize=thread`): detecta la race aunque la
  corrida "pase" — el detector va al código, no al ojo humano.

## 🐛 Problemas detectados en esta etapa

1. **El estado sigue sin compilar** (verificado): los 12 errores son el mismo
   `std::exchange`/`<utility>` faltante — el demo de la carrera no podía
   ejecutarse tal como está en el repo. Con el include, sí (ver deep-dive).
2. **Sincronización parcial**: solo `push_back`/`pop_back` están protegidos.
   `resize`, `operator[]`, `at()`, `size()`, `begin()/end()`, iteradores...
   siguen desprotegidos — un lector concurrente con un escritor sigue siendo
   race (el STL no es thread-safe ni con el mutex de otro).
3. **El orden también es una race** (verificado): aun con mutex, el output
   alterna `[Hello World]` / `[World Hello]` — sincronizar evita corrupción,
   no garantiza orden.
4. **"EN ESTA CORRIDA"**: el mensaje del propio demo lo admite — un test
   concurrente que pasa no es prueba de ausencia de race. Falta TSan en el
   flujo normal.

## 👨‍🏫 Lo que explicó el profe

> ⚠ *Por completar con lo dicho en clase.*

- ¿Por qué 5 threads y no 2? *(completar)*
- Alternativas al mutex grueso: fine-grained, lock-free, `std::atomic`.

## ✍️ Mi práctica

- [ ] Reproducir el experimento del deep-dive: con mutex ✅, sin mutex 💥
- [ ] Compilar con `-fsanitize=thread -g` y leer el reporte de la carrera
- [ ] Medir: ¿el mutex hace más lento el `ConcurrentInsert` de 200 000? ¿cuánto?
- [ ] (Reto) Hacer thread-safe `operator[]` sin bloquear más de lo necesario

## 🔗 Referencias

- `git diff 49ceccf 5575be8` (la adición del mutex, línea a línea)
- [cppreference — lock_guard](https://en.cppreference.com/w/cpp/thread/lock_guard)
- [ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html)
- **[07a-race-condition.md](07a-race-condition.md)** — el experimento verificado
