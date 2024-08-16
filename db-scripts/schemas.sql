\c :dbname

CREATE DOMAIN IntGZ AS integer CHECK (VALUE > 0);
CREATE DOMAIN IntGEZ AS integer CHECK (VALUE >= 0);

CREATE TABLE IF NOT EXISTS Average(
    id SERIAL PRIMARY KEY,
    sensor_id IntGEZ NOT NULL,
    start_timestamp IntGEZ NOT NULL,
    end_timestamp IntGEZ NOT NULL,
    value float,
    is_anomaly BOOLEAN NOT NULL DEFAULT FALSE,
    CONSTRAINT start_gz_end CHECK (end_timestamp > start_timestamp),
    CONSTRAINT unique_sensor UNIQUE (sensor_id, start_timestamp)
);

CREATE TABLE IF NOT EXISTS Covariance(
    id SERIAL PRIMARY KEY,
    sensor1_id IntGEZ NOT NULL,
    sensor2_id IntGEZ NOT NULL,
    start_timestamp IntGEZ NOT NULL,
    end_timestamp IntGEZ NOT NULL,
    value float,
    is_anomaly BOOLEAN NOT NULL DEFAULT FALSE,
    CONSTRAINT start_gz_end CHECK (end_timestamp > start_timestamp),
    CONSTRAINT unique_sensor UNIQUE (sensor1_id , sensor2_id , start_timestamp)
)
