# sqlite

```sql

CREATE TABLE IF NOT EXISTS finance
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,

  name           TEXT,
  code           TEXT,
  description    TEXT,
  note           TEXT,
  node_rule      BOOLEAN    DEFAULT 0,        -- DDCI = 1, DICD = 0
  branch         BOOLEAN    DEFAULT FALSE,
  unit           INTEGER,
  removed        BOOLEAN    DEFAULT FALSE
);

CREATE TABLE IF NOT EXISTS finance_path
(
  ancestor      INTEGER    CHECK (ancestor   >= 1),
  descendant    INTEGER    CHECK (descendant >= 1),
  distance      INTEGER    CHECK (distance   >= 0)

  -- PRIMARY KEY (descendant, distance, ancestor)
);

-- CREATE INDEX finance_path_idx ON finance_path (ancestor, distance);

CREATE TABLE IF NOT EXISTS finance_transaction
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,

  post_date      DATE,
  code           TEXT,
  lhs_node       INTEGER,
  lhs_rate       REAL       DEFAULT 1.0       CHECK (lhs_rate > 0),
  description    TEXT,
  rhs_node       INTEGER,
  rhs_rate       REAL       DEFAULT 1.0       CHECK (rhs_rate > 0),
  state          BOOLEAN    DEFAULT FALSE,
  document       TEXT,
  debit          NUMERIC                      CHECK (debit    >=0),
  credit         NUMERIC                      CHECK (credit   >=0),
  removed        BOOLEAN    DEFAULT FALSE
);

CREATE TABLE IF NOT EXISTS product
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,

  name           TEXT,
  code           TEXT,
  description    TEXT,
  note           TEXT,
  node_rule      BOOLEAN    DEFAULT 0,        -- DDCI = 1, DICD = 0
  branch         BOOLEAN    DEFAULT FALSE,
  unit           INTEGER,
  removed        BOOLEAN    DEFAULT FALSE
);

CREATE TABLE IF NOT EXISTS product_path
(
  ancestor      INTEGER    CHECK (ancestor   >= 1),
  descendant    INTEGER    CHECK (descendant >= 1),
  distance      INTEGER    CHECK (distance   >= 0)

  -- PRIMARY KEY (descendant, distance, ancestor)
);

-- CREATE INDEX product_path_idx ON product_path (ancestor, distance);

CREATE TABLE IF NOT EXISTS product_transaction
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,

  post_date      DATE,
  code           TEXT,
  lhs_node       INTEGER,
  lhs_rate       REAL       DEFAULT 1.0       CHECK (lhs_rate > 0),
  description    TEXT,
  rhs_node       INTEGER,
  rhs_rate       REAL       DEFAULT 1.0       CHECK (rhs_rate > 0),
  state          BOOLEAN    DEFAULT FALSE,
  document       TEXT,
  debit          NUMERIC                      CHECK (debit    >=0),
  credit         NUMERIC                      CHECK (credit   >=0),
  removed        BOOLEAN    DEFAULT FALSE
);

CREATE TABLE IF NOT EXISTS network
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,

  name           TEXT,
  code           TEXT,
  description    TEXT,
  note           TEXT,
  node_rule      BOOLEAN    DEFAULT 0,        -- DDCI = 1, DICD = 0
  branch         BOOLEAN    DEFAULT FALSE,
  unit           INTEGER,
  removed        BOOLEAN    DEFAULT FALSE
);

CREATE TABLE IF NOT EXISTS network_path
(
  ancestor      INTEGER    CHECK (ancestor   >= 1),
  descendant    INTEGER    CHECK (descendant >= 1),
  distance      INTEGER    CHECK (distance   >= 0)

  -- PRIMARY KEY (descendant, distance, ancestor)
);

-- CREATE INDEX network_path_idx ON network_path (ancestor, distance);

CREATE TABLE IF NOT EXISTS network_transaction
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,

  post_date      DATE,
  code           TEXT,
  lhs_node       INTEGER,
  lhs_rate       REAL       DEFAULT 1.0       CHECK (lhs_rate > 0),
  description    TEXT,
  rhs_node       INTEGER,
  rhs_rate       REAL       DEFAULT 1.0       CHECK (rhs_rate > 0),
  state          BOOLEAN    DEFAULT FALSE,
  document       TEXT,
  debit          NUMERIC                      CHECK (debit    >=0),
  credit         NUMERIC                      CHECK (credit   >=0),
  removed        BOOLEAN    DEFAULT FALSE
);

CREATE TABLE IF NOT EXISTS task
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,

  name           TEXT,
  code           TEXT,
  description    TEXT,
  note           TEXT,
  node_rule      BOOLEAN    DEFAULT 0,        -- DDCI = 1, DICD = 0
  branch         BOOLEAN    DEFAULT FALSE,
  unit           INTEGER,
  removed        BOOLEAN    DEFAULT FALSE
);

CREATE TABLE IF NOT EXISTS task_path
(
  ancestor      INTEGER    CHECK (ancestor   >= 1),
  descendant    INTEGER    CHECK (descendant >= 1),
  distance      INTEGER    CHECK (distance   >= 0)

  -- PRIMARY KEY (descendant, distance, ancestor)
);

-- CREATE INDEX task_path_idx ON task_path (ancestor, distance);

CREATE TABLE IF NOT EXISTS task_transaction
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,

  post_date      DATE,
  code           TEXT,
  lhs_node       INTEGER,
  lhs_rate       REAL       DEFAULT 1.0       CHECK (lhs_rate > 0),
  description    TEXT,
  rhs_node       INTEGER,
  rhs_rate       REAL       DEFAULT 1.0       CHECK (rhs_rate > 0),
  state          BOOLEAN    DEFAULT FALSE,
  document       TEXT,
  debit          NUMERIC                      CHECK (debit    >=0),
  credit         NUMERIC                      CHECK (credit   >=0),
  removed        BOOLEAN    DEFAULT FALSE
);
CREATE TABLE IF NOT EXISTS section_rule
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  -- 1-finance,  2-sales, 3-task, 4-network, 5-product, 6-purchase
  static_label       TEXT,
  static_node        INTEGER,
  dynamic_label      TEXT,
  dynamic_node_lhs   INTEGER,
  operation          TEXT,
  dynamic_node_rhs   INTEGER,
  hide_time          BOOLEAN    DEFAULT TRUE,
  base_unit          INTEGER,
  document_dir       TEXT,
  value_decimal      INTEGER    DEFAULT 2,
  rate_decimal       INTEGER    DEFAULT 4
);

INSERT INTO section_rule (static_node) VALUES (0);
INSERT INTO section_rule (static_node) VALUES (0);
INSERT INTO section_rule (static_node) VALUES (0);
INSERT INTO section_rule (static_node) VALUES (0);
INSERT INTO section_rule (static_node) VALUES (0);
INSERT INTO section_rule (static_node) VALUES (0);

```
