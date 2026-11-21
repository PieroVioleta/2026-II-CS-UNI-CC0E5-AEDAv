-- ============================================================
-- Indice Hash: SOLO igualdad (=)
-- No sirve para rangos, ni < >, ni ORDER BY (para eso: btree)
-- ============================================================

CREATE TABLE usuarios (
    id    SERIAL PRIMARY KEY,
    email TEXT
);

CREATE INDEX idx_usuarios_email_hash ON usuarios USING hash (email);

-- consulta que lo aprovecha: igualdad exacta
SELECT * FROM usuarios WHERE email = 'ana@uni.pe';

-- ver el plan (buscar "Hash" o "Index Scan" usando el indice)
EXPLAIN SELECT * FROM usuarios WHERE email = 'ana@uni.pe';

-- esta NO aprovecha el hash (es un rango): el plan hara un Seq Scan
EXPLAIN SELECT * FROM usuarios WHERE id > 100;
