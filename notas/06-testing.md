# 06 · Testing del contenedor

> **Ramas cubiertas:** `09-TestContainer`
> **Estado:** contenido inicial — pendiente de revisar con lo visto en clase

## 📋 Ficha

| Rama | Commit(s) | Archivos tocados | Versión |
|------|-----------|------------------|---------|
| `09-TestContainer` | `77fd402` "General Iterator with errors" — **snapshot, sin commits propios** | `containers/GeneralIterator.h` (2±), `containers/vector.h` (9±) | — |

## 🎯 En qué estábamos

Dato revelador: `09-TestContainer` **apunta exactamente al mismo commit**
(`77fd402`) que ya vive dentro de la cadena de `07-Vector2.0` — es una **rama
de captura** que congela el estado "con errores" justo *antes* del fix
(`6abf3ae` "Generalterator fixed", en la otra rama). Su función: dejar
etiquetado el momento "esto está roto" para poder enseñarlo/después corregirlo.
Por eso no tiene Makefile (ese llegó después en `08-Makefile`).

## 🧩 La implementación

### Qué contiene "General Iterator with errors" (compilación verificada)

El commit revirtió el iterador del diseño CRTP v3 hacia la forma F-bound v2,
con efectos medibles — 3 familias de errores conviviendo:

```
1. incomplete type de vuelta (la reversión reintrodujo el ciclo de la v2):
   GeneralIterator.h:13: error: invalid use of incomplete type
     'class VectorForwardIterator<int>'

2. el ctor deja de calzar:
   vector.h:108: error: no matching function for call to
     'VectorForwardIterator<int>::VectorForwardIterator(value_type*&)'

3. el siempre-presente <utility> faltante:
   vector.h:59: error: 'exchange' is not a member of 'std'
```

Y en `Demos.cpp`, los **labels de los "tests" quedaron desincronizados** de lo
que hacen:

```cpp
::ApplyFunction(vec, AddX<TX>, 7);    // suma 7...
cout << "Vector after applying lambda function (Add 5): ";  // ...el label dice 5

::ApplyFunction(strVec, AddX<string>, "!-X");  // agrega "!-X"
cout << "... (Add !): ";                        // ...el label dice "!"
```

## 📚 Conceptos nuevos

- **El "test" que había hasta acá**: `DemoVector()` es un *smoke test manual* —
  ejecutar, mirar el output y confiar. Sin `assert`, sin framework, sin CI.
- **Smoke test vs test automatizado**: el demo solo dice "no crashea"; no
  verifica valores esperados. Un label desincronizado (7 vs "Add 5") pasa
  inadvertido para un humano que "ve que imprime algo".
- **`assert` / `static_assert`**: la forma mínima de volver un demo en test —
  falla con línea y valor si la expectativa no se cumple, sin que nadie mire.
- **Frameworks de test** (mención): GoogleTest, Catch2, doctest — `TEST`,
  `REQUIRE`, reporte de fallos, aislamiento por caso.
- **Ramas snapshot**: congelar un estado roto con nombre (`09-TestContainer`)
  para poder volver a él — el "rojo" guardado antes del "verde".

## 🐛 Problemas detectados en esta etapa

1. **"Corre" ≠ "está bien"**: el criterio de testeo era visual. El label
   "(Add 5)" con un `7` de verdad es exactamente el bug que un assert habría
   atrapado.
2. **Los labels son documentación que se pudre**: se editó el argumento y no
   el texto — nada lo obliga a estar sincronizado sin un test.
3. **El snapshot congeló un árbol que no compila** (3 familias de error a la
   vez) — no hay forma de "correr los tests" en este estado.
4. **Contraste honesto**: acá el commit SÍ dice "with errors" (correcto);
   `6abf3ae` decía "fixed" y mentía (deep-dive 04a). El nombre del commit
   debería derivar de la evidencia, no al revés.
5. El README seguía siendo solo el título — sin instrucciones de cómo
   verificar nada.

## 👨‍🏫 Lo que explicó el profe

> ⚠ *Por completar con lo dicho en clase.*

- ¿Por qué se congeló esta rama antes de arreglar? *(completar)*
- Qué test mínimos le pediría a un `Vector` (capacidad, límites, copia, move).

## ✍️ Mi práctica

- [ ] Verificar el estado roto: `git archive upstream/09-TestContainer | tar -x -C /tmp/t09 && g++ -std=c++23 -fsyntax-only /tmp/t09/Demos.cpp`
- [ ] Convertir el demo en test mínimo: `assert((vec.size() == 10));` y
      verificar valores post-`AddOne` con un loop de asserts
- [ ] Poner en verde: restaurar v3 (`06-CRTP`'s shape) + `#include <utility>`
      y ver pasar los asserts
- [ ] Corregir los labels o mejor: eliminar los labels y dejar que el assert
      hable
- [ ] (Reto) Escribir el primer test que falle a propósito y leer el reporte

## 🔗 Referencias

- `git diff 289e3e1 77fd402` (lo que introdujo "with errors")
- `git log --oneline upstream/09-TestContainer -3` (la rama = snapshot)
- [cppreference — assert](https://en.cppreference.com/w/c/error/assert)
- [GoogleTest primer test](https://google.github.io/googletest/quickstart-cmake.html)
