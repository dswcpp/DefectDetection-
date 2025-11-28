-- DefectDetection SQLite schema (based on docs/数据库表结构设计.md)
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS users (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    username        TEXT NOT NULL UNIQUE,
    password_hash   TEXT NOT NULL,
    role            TEXT NOT NULL,               -- admin/operator/engineer
    is_active       INTEGER DEFAULT 1,
    created_at      DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS batches (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    batch_no        TEXT NOT NULL UNIQUE,        -- BATCH-20250115-A
    product_type    TEXT NOT NULL,
    start_time      DATETIME NOT NULL,
    end_time        DATETIME,
    target_count    INTEGER DEFAULT 0,
    actual_count    INTEGER DEFAULT 0,
    ok_count        INTEGER DEFAULT 0,
    ng_count        INTEGER DEFAULT 0,
    yield_rate      REAL,
    status          TEXT DEFAULT 'running',      -- running/completed/paused
    operator_id     INTEGER,
    shift           TEXT,                        -- A/B/C
    remark          TEXT,
    created_at      DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at      DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (operator_id) REFERENCES users(id)
);
CREATE INDEX IF NOT EXISTS idx_batches_batch_no ON batches(batch_no);
CREATE INDEX IF NOT EXISTS idx_batches_status ON batches(status);

CREATE TABLE IF NOT EXISTS stations (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    station_code    TEXT NOT NULL UNIQUE,
    station_name    TEXT,
    status          TEXT DEFAULT 'online'
);

CREATE TABLE IF NOT EXISTS products (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    product_id      TEXT NOT NULL UNIQUE,        -- 序列号
    batch_id        INTEGER NOT NULL,
    barcode         TEXT,
    created_at      DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (batch_id) REFERENCES batches(id)
);
CREATE INDEX IF NOT EXISTS idx_products_batch_id ON products(batch_id);

CREATE TABLE IF NOT EXISTS inspections (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    product_id      INTEGER NOT NULL,
    station_id      INTEGER,
    inspect_time    DATETIME NOT NULL,
    result          TEXT NOT NULL,               -- OK/NG/ERROR
    defect_count    INTEGER DEFAULT 0,
    max_severity    REAL DEFAULT 0,
    severity_level  TEXT,
    cycle_time_ms   INTEGER,
    acquire_time_ms INTEGER,
    process_time_ms INTEGER,
    image_path      TEXT,
    annotated_path  TEXT,
    thumbnail_path  TEXT,
    exposure_us     INTEGER,
    gain_db         REAL,
    brightness_avg  REAL,
    temperature_c   REAL,
    model_version   TEXT,
    FOREIGN KEY (product_id) REFERENCES products(id),
    FOREIGN KEY (station_id) REFERENCES stations(id)
);
CREATE INDEX IF NOT EXISTS idx_inspections_product_id ON inspections(product_id);
CREATE INDEX IF NOT EXISTS idx_inspections_time ON inspections(inspect_time);
CREATE INDEX IF NOT EXISTS idx_inspections_result ON inspections(result);

CREATE TABLE IF NOT EXISTS defects (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    inspection_id   INTEGER NOT NULL,
    defect_type     TEXT NOT NULL,               -- scratch/crack/foreign/dimension
    confidence      REAL DEFAULT 0,
    severity_score  REAL DEFAULT 0,
    severity_level  TEXT,
    region_x        INTEGER,
    region_y        INTEGER,
    region_width    INTEGER,
    region_height   INTEGER,
    features        TEXT,                        -- JSON 字符串
    created_at      DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (inspection_id) REFERENCES inspections(id)
);
CREATE INDEX IF NOT EXISTS idx_defects_inspection_id ON defects(inspection_id);
CREATE INDEX IF NOT EXISTS idx_defects_type ON defects(defect_type);

CREATE TABLE IF NOT EXISTS audit_logs (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp       DATETIME DEFAULT CURRENT_TIMESTAMP,
    user_id         INTEGER,
    action          TEXT,
    category        TEXT,
    target          TEXT,
    old_value       TEXT,
    new_value       TEXT,
    result          TEXT,
    FOREIGN KEY (user_id) REFERENCES users(id)
);
CREATE INDEX IF NOT EXISTS idx_audit_logs_user ON audit_logs(user_id);

CREATE TABLE IF NOT EXISTS spc_samples (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    inspection_id   INTEGER NOT NULL,
    metric          TEXT NOT NULL,               -- 尺寸/严重度等
    value           REAL NOT NULL,
    created_at      DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (inspection_id) REFERENCES inspections(id)
);
CREATE INDEX IF NOT EXISTS idx_spc_metric ON spc_samples(metric);

CREATE TABLE IF NOT EXISTS alarms (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    level           TEXT NOT NULL,               -- info/warn/error/critical
    message         TEXT NOT NULL,
    module          TEXT,
    created_at      DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS golden_samples (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    sample_code     TEXT NOT NULL UNIQUE,
    sample_type     TEXT NOT NULL,               -- ok/scratch/crack/foreign/dimension
    expected_result TEXT NOT NULL,
    reference_image TEXT,
    remark          TEXT,
    created_at      DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS golden_test_results (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    golden_id       INTEGER NOT NULL,
    test_time       DATETIME DEFAULT CURRENT_TIMESTAMP,
    result          TEXT NOT NULL,               -- OK/NG
    severity_score  REAL,
    FOREIGN KEY (golden_id) REFERENCES golden_samples(id)
);

CREATE TABLE IF NOT EXISTS model_versions (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    version         TEXT NOT NULL UNIQUE,
    model_path      TEXT NOT NULL,
    is_active       INTEGER DEFAULT 0,
    created_at      DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS config_history (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    version         TEXT NOT NULL,
    path            TEXT NOT NULL,
    applied_by      INTEGER,
    applied_at      DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (applied_by) REFERENCES users(id)
);

-- 默认索引用于查询效率，可按需扩展
