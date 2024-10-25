\c :dbname

CREATE DOMAIN IntGZ AS integer CHECK (VALUE > 0);
CREATE DOMAIN IntGEZ AS integer CHECK (VALUE >= 0);
CREATE DOMAIN StringS AS VARCHAR(50);
CREATE TYPE logType AS ENUM ('Average', 'Covariance');

CREATE TABLE IF NOT EXISTS Average(
    id SERIAL PRIMARY KEY,
    sensor_id IntGEZ NOT NULL,
    start_timestamp timestamp  NOT NULL,
    end_timestamp timestamp NOT NULL,
    value float,
    is_anomaly BOOLEAN NOT NULL DEFAULT FALSE,
    calculate_at timestamp DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT start_gz_end CHECK (end_timestamp > start_timestamp),
    CONSTRAINT unique_sensor_av UNIQUE (sensor_id, start_timestamp)
);

CREATE TABLE IF NOT EXISTS Covariance(
    id SERIAL PRIMARY KEY,
    sensor1_id IntGEZ NOT NULL,
    sensor2_id IntGEZ NOT NULL,
    start_timestamp timestamp NOT NULL,
    end_timestamp timestamp NOT NULL,
    value float,
    is_anomaly BOOLEAN NOT NULL DEFAULT FALSE,
    calculate_at timestamp DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT start_gz_end CHECK (end_timestamp > start_timestamp),
    CONSTRAINT unique_sensor_cov UNIQUE (sensor1_id , sensor2_id , start_timestamp),
    CONSTRAINT diff_sens CHECK (sensor1_id != sensor2_id)
);

CREATE TABLE IF NOT EXISTS SessionLogs(
    id SERIAL PRIMARY KEY,
    timestamp timestamp DEFAULT CURRENT_TIMESTAMP,
    type logType,
    value float,
    status StringS
);
