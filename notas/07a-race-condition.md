# 🔬 Deep-dive · La race condition, reproducida de verdad

> **Complemento de:** [07-concurrencia.md](07-concurrencia.md)
> **Evidencia:** compilación y ejecución reales del árbol `10-Concurrency`
> (`g++ -std=c++23`), con y sin las líneas `lock_guard<mutex>`.
> **Estado:** contenido inicial — pendiente de revisar con lo visto en clase

## El experimento

Se extrajo el árbol de `10-Concurrency`, se aplicó el fix de `<utility>`
(nota 04) y se compiló dos veces: **con mutex** (estado `5575be8`) y **sin
mutex** (simulando el estado rojo de `49ceccf`, borrando las dos líneas
`lock_guard`).

| Versión | Resultado (5 corridas) |
|---------|------------------------|
| **CON mutex** | ✅ `se esperaban 200000, quedaron 200000 · suma 19999900000 == 19999900000` — perfecta las corridas |
| **SIN mutex** | 💥 **crashea el 100%**: `Aborted (core dumped)` y `Segmentation fault` alternados, *antes* de siquiera imprimir el reporte |
| SIN mutex, stderr | `double free or corruption (!prev)` — el allocator de glibc abortando |

## ¿Qué está pasando dentro de la carrera?

### 1. Pérdida de inserciones (lost update)

Dos threads llegan con el mismo `m_size`:

```
Thread A: lee m_size = 7          Thread B: lee m_size = 7
          escribe m_data[7] = a             escribe m_data[7] = b   ← pisa a 'a'
          ++m_size → 8                      ++m_size → 8            ← contó 2, insertó 1
```

`push_back` no es atómico: son **lectura + escritura + incremento** que el
scheduler puede entrelazar como quiera. Con `N=10` la ventana es minúscula;
con `N=200 000` y 5 threads, es estadísticamente inevitable.

### 2. El crash real: `resize()` concurrente

```cpp
void resize(size_t new_cap) {
    value_type* new_data = new value_type[new_cap];   // dos threads a la vez
    for (size_t i = 0; i < m_size; ++i)
        new_data[i] = m_data[i];                      // ← leen el MISMO m_data
    delete[] m_data;                                  // ← liberan el MISMO buffer
    m_data = new_data;
}
```

Dos threads dentro de `resize` simultáneamente → doble `delete[]` del mismo
puntero y datos escritos sobre un buffer ya liberado. El mensaje de glibc lo
dice textualmente: **`double free or corruption (!prev)`** — el allocator
detecta la corrupción y aborta (`SIGABRT`); si no la detecta, el acceso a
memoria muerta da `SIGSEGV`. Por eso las corridas alternan entre ambos.

### 3. El orden, también no determinista (verificado)

Aun **con** el mutex funcionando perfecto, el output alterna:

```
[Hello World]   ← corrida 1
[World Hello]   ← corrida 2
```

El mutex evita **corrupción**, no **orden**: quien toma el lock primero, inserta
primero, y eso depende del scheduler. Si el orden importara, haría falta
comunicación explícita (cola, entrega por chunks) — no basta con sincronizar.

## Por qué `lock_guard<mutex>` es el fix correcto (y suficiente aquí)

```cpp
{
    lock_guard<mutex> lock(m_mutex);   // ctor → lock()
    ... sección crítica ...            // excepciones o return: igual pasa por acá
}                                      // dtor → unlock()
```

- **RAII**: el recurso (el lock) se adquiere en el constructor y se libera en
  el destructor — la liberación está atada al scope, no a la disciplina del
  programador.
- Alternativa manual (`m_mutex.lock() ... m_mutex.unlock()`) se rompe con un
  `throw` intermedio: el unlock nunca corre → deadlock para el resto de threads.
- Es un **mutex grueso** (toda la operación): correcto y simple; el costo se
  paga en contención (los 5 threads se turnan). Fino/lock-free es otro curso.

## Cómo se detecta *sin* esperar al crash: ThreadSanitizer

```bash
g++ -std=c++23 -fsanitize=thread -g main.cpp Demos.cpp -o main && ./main
```

TSan instrumenta cada acceso a memoria y reporta la carrera aunque la corrida
termine "bien" (con mutex, ni reporta). El mensaje del propio demo admite el
límite: *"la race sigue ahí: vuelve a correr... o compila con -fsanitize=thread"*.
Regla para estas notas: **un test que pasa no prueba que no haya race — el
detector sí**.

## Lecciones

1. **El volumen agranda la ventana**: 10 elementos esconden la carrera,
   200 000 con 5 threads la hacen explotar. Diseñar el test para la ventana.
2. **Corregir contra invariantes, no contra output**: suma esperada vs suma
   obtenida — la propiedad matemática no se pudre como un label.
3. **El crash no siempre es donde crees**: el fallo no estaba en el contador
   sino en el `new[]`/`delete[]` de `resize` — dos modos de fallo (SIGABRT,
   SIGSEGV) para una sola carrera.
4. **Sincronizar ≠ ordenar**: mutex para integridad; para orden, diseño
   explícito.
5. **La sección crítica incluye TODO lo que toca estado compartido** — el fix
   del curso cubre `push_back`/`pop_back`, pero el resto de la API sigue
   siendo terreno de carrera.
