# Índices en SQL (PostgreSQL)

Ejemplos simples de los 3 tipos de índice. Ejecutar con:

```
psql -d tu_base -f sql/01_btree.sql
```

| Archivo | Índice | Sirve para |
|---|---|---|
| `01_btree.sql` | B-tree | igualdad, rangos, `ORDER BY` (el default) |
| `02_hash.sql` | Hash | solo igualdad (`=`) |
| `03_gist.sql` | GiST | datos no escalares: puntos, rangos, texto |
