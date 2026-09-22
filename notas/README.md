# Notas de repaso — Curso AEDAv (2026-II, CS-UNI)

> Repaso de la progresión del curso: el profesor partió de una implementación de un
> `Vector` copiada de IA (rama `02-Vector`) y, commit a commit y rama a rama, fue
> mostrando cómo mejorar ese código. Estas notas documentan esa evolución.
>
> Repo del profe (upstream): `ecuadros/2026-II-CS-UNI-CC0E5-AEDAv`

## Índice de notas

| # | Nota | Ramas cubiertas | Tema |
|---|------|-----------------|------|
| 00 | [Setup y punto de partida](00-setup.md) | `00-Students`, `01-Main` | Repo base, roster vía PRs, primer `main.cpp`, C++23 |
| 01 | [El vector copiado de IA](01-vector-copiado.md) | `02-Vector` | `containers/vector.h`, críticas iniciales |
| 02 | [Impresión del vector](02-impresion.md) | `operator<<`, `04-operator` | `operator<<`, Vector 0.42→0.43 |
| 03 | [foreach y ApplyFunction](03-foreach.md) | `foreach`, `03-Foreach3`, `04-foreach4` | Recorridos, `foreach.h`, `types.h` |
| 04 | [Iteradores y CRTP](04-iteradores.md) | `05-iterator`, `06-CRTP`, `07-Vector2.0` | `GeneralIterator.h`, CRTP |
| 05 | [Makefile](05-build.md) | `08-Makefile` | Build con Makefile |
| 06 | [Testing del contenedor](06-testing.md) | `09-TestContainer` | Iterator con errores, TODOs |
| 07 | [Concurrencia](07-concurrencia.md) | `10-Concurrency` | Mutex en `push_back` |
| 08 | [Node (value, ref)](08-node.md) | `11-Node` | `Vector` almacena pares |
| 09 | [Initializer list](09-initializer-list.md) | `12-InitializerList` | Constructor con `initializer_list` |
| 10 | [Iterador hacia atrás](10-backward-iterator.md) | `13-BackwardIterator`, `14-TestTraversal` | `rbegin()`/`rend()`, test de recorrido |

## Mapa de ramas del profe

Orden cronológico por fecha del último commit:

| Rama | Fecha | Último commit |
|------|-------|---------------|
| `00-Students` | 2026-09-05 | Merge PR #9 (datos de estudiantes) |
| `01-Main` | 2026-09-05 | C++23 activado |
| `02-Vector` | 2026-09-08 | Vector 0.3 |
| `operator<<` ⚠ | 2026-09-08 | Vector 0.42 |
| `04-operator` | 2026-09-08 | Vector 0.43 |
| `foreach` ⚠ | 2026-09-08 | Vector 0.432 |
| `03-Foreach3` | 2026-09-12 | Foreach nivel 3 |
| `04-foreach4` | 2026-09-12 | ApplyFunction nivel 4 |
| `05-iterator` | 2026-09-13 | GeneralIterator por arreglar |
| `06-CRTP` | 2026-09-15 | GeneralIterator creado |
| `07-Vector2.0` | 2026-09-17 | GeneralIterator fixed |
| `08-Makefile` | 2026-09-17 | Makefile funcionando |
| `09-TestContainer` | 2026-09-17 | General Iterator con errores |
| `10-Concurrency` | 2026-09-19 | Mutex en `push_back` |
| `11-Node` | 2026-09-19 | Vector almacena `Node` (value, ref) |
| `12-InitializerList` | 2026-09-19 | Constructor con `initializer_list` |
| `13-BackwardIterator` | 2026-09-19 | `VectorBackwardIterator`, `rbegin()`/`rend()` |
| `14-TestTraversal` | 2026-09-19 | TODOs actualizados |

⚠ `operator<<` y `foreach` fueron **borradas en upstream**, pero siguen disponibles como ramas locales de este repo.

## Cómo revisar una etapa

```bash
# ver los archivos de una etapa sin cambiar de rama
git ls-tree -r upstream/02-Vector --name-only

# ver el diff entre dos etapas consecutivas
git diff upstream/02-Vector upstream/03-Foreach3 -- containers/

# comparar tu implementación contra la del profe
git diff upstream/05-iterator -- containers/vector.h
```
