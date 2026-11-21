-- ============================================================
-- Indice B-tree: el indice por defecto de PostgreSQL
-- Sirve para: =, <, >, <=, >=, BETWEEN, ORDER BY, LIKE 'abc%'
-- ============================================================

CREATE TABLE empleados (
    id      SERIAL PRIMARY KEY,
    nombre  TEXT,
    salario NUMERIC
);

-- forma explicita (btree es el default, las dos lineas son equivalentes)
CREATE INDEX idx_empleados_salario        ON empleados (salario);
CREATE INDEX idx_empleados_salario_btree  ON empleados USING btree (salario);

-- consultas que aprovechan el indice
SELECT * FROM empleados WHERE salario = 3500;
SELECT * FROM empleados WHERE salario BETWEEN 2000 AND 5000;
SELECT * FROM empleados ORDER BY salario;

-- ver que el plan de ejecucion usa el indice (buscar "Index Scan")
EXPLAIN SELECT * FROM empleados WHERE salario > 3000;
