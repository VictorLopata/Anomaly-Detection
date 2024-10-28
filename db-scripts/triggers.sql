\c :dbname
-- PROCEDURES

CREATE FUNCTION check_avg_disjoint()
    RETURNS TRIGGER
    LANGUAGE plpgsql
AS $$
BEGIN
    IF EXISTS (
        SELECT 1 FROM Average
        WHERE Average.sensor_id = NEW.sensor_id
          AND (NEW.start_timestamp, NEW.end_timestamp) OVERLAPS (Average.start_timestamp, Average.end_timestamp)
    ) THEN
        RAISE EXCEPTION 'invalid avg time-stamp';
END IF;

RETURN NEW;
END;
$$;


CREATE FUNCTION check_cov_disjoint()
    RETURNS TRIGGER
    LANGUAGE plpgsql
AS $$
BEGIN
    IF EXISTS (
        SELECT 1 FROM Covariance
        WHERE Covariance.sensor1_id = NEW.sensor1_id
          AND Covariance.sensor2_id = NEW.sensor2_id
          AND (NEW.start_timestamp, NEW.end_timestamp) OVERLAPS (Covariance.start_timestamp, Covariance.end_timestamp)
    ) THEN
        RAISE EXCEPTION 'invalid cov time-stamp';
END IF;

RETURN NEW;
END;
$$;


CREATE OR REPLACE FUNCTION time_overlap() RETURNS TRIGGER AS $$
BEGIN
    IF EXISTS (
        SELECT 1 FROM Average
        WHERE sensor_id = NEW.sensor_id
          AND (NEW.start_timestamp, NEW.end_timestamp) OVERLAPS (start_timestamp, end_timestamp)
          AND id <> NEW.id
    ) THEN
        RAISE EXCEPTION 'Time intervals for the same sensor cannot overlap.';
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION check_WindowTime()
    RETURNS TRIGGER AS $$
DECLARE
    timeWindow INTERVAL := '10 seconds';
BEGIN
    IF NEW.end_timestamp - NEW.start_timestamp != timeWindow THEN
        RAISE EXCEPTION 'Window Time not correct.';
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;



-- TRIGGERS
CREATE TRIGGER check_windowAVG
    BEFORE INSERT OR UPDATE ON Average
    FOR EACH ROW
    EXECUTE FUNCTION check_WindowTime();

CREATE TRIGGER check_windowCOV
    BEFORE INSERT OR UPDATE ON Covariance
    FOR EACH ROW
    EXECUTE FUNCTION check_WindowTime();


CREATE TRIGGER check_time_overlap
    BEFORE INSERT OR UPDATE ON Average
    FOR EACH ROW
EXECUTE FUNCTION time_overlap();


-- [Vincolo Average Disjoint]
CREATE TRIGGER average_disjoint_time
    BEFORE INSERT
    ON Average
    FOR EACH ROW
    EXECUTE PROCEDURE check_avg_disjoint();

-- [Vincolo Covariance Disjoint]
CREATE TRIGGER covariance_disjoint_time
    BEFORE INSERT
    ON Covariance
    FOR EACH ROW
    EXECUTE PROCEDURE check_cov_disjoint();

