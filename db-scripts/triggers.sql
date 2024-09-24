
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



-- TRIGGERS

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