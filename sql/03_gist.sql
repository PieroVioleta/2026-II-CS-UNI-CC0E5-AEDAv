-- ============================================================
-- Indice GiST: generalizado para datos NO escalares
-- puntos/geometria, rangos, texto, arreglos...
-- ============================================================

CREATE TABLE lugares (
    id     SERIAL PRIMARY KEY,
    nombre TEXT,
    punto  POINT
);

CREATE INDEX idx_lugares_punto ON lugares USING gist (punto);

-- contencion: lugares dentro del recuadro ((0,0),(10,10))
SELECT * FROM lugares WHERE punto <@ box '((0,0),(10,10))';

-- los 3 lugares mas cercanos a un punto (orden por distancia)
SELECT * FROM lugares ORDER BY punto <-> point '(5,5)' LIMIT 3;

EXPLAIN SELECT * FROM lugares WHERE punto <@ box '((0,0),(10,10))';

-- otro uso tipico de gist: rangos que se solapan
CREATE TABLE reservas (
    id      SERIAL PRIMARY KEY,
    sala    TEXT,
    horario INT4RANGE
);

CREATE INDEX idx_reservas_horario ON reservas USING gist (horario);

-- reservas que se solapan con las 10:00-12:00
SELECT * FROM reservas WHERE horario && int4range(10, 12);
