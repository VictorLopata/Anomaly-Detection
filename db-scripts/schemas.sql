\c :dbname

CREATE DOMAIN WindowID as integer check ( VALUE >= 0 );
CREATE DOMAIN SensorID as integer check ( VALUE >= 0 );
CREATE TYPE AnomalyType AS ENUM ('Covariance Anomaly', 'Average Anomaly');

CREATE TABLE IF NOT EXISTS Averages(
    value FLOAT,
    window WindowID NOT NULL,
    sensor SensorID NOT NULL,
    time timestamp,

    PRIMARY KEY (window, sensor, time)
);

CREATE TABLE IF NOT EXISTS Covariances(
    id SERIAL PRIMARY KEY,
    sensor1 SensorID NOT NULL,
    sensor2 SensorID NOT NULL,
    window WindowID NOT NULL,
    value FLOAT,
    time timestamp,

    FOREIGN KEY (sensor1) REFERENCES Sensors(id),
    FOREIGN KEY (sensor2) REFERENCES Sensors(id)
);

CREATE TABLE IF NOT EXISTS Sensors(
    id SERIAL PRIMARY KEY
);

CREATE TABLE IF NOT EXISTS Anomaly(
    windowA WindowID NOT NULL ,
    anomaly AnomalyType,
    sensor1 SensorID NOT NULL,
    sensor2 SensorID,
    time timestamp NOT NULL,

    PRIMARY KEY (anomaly, sensor1),
    FOREIGN KEY (sensor1) REFERENCES Sensors(id),
    FOREIGN KEY (sensor2) REFERENCES Sensors(id)
)