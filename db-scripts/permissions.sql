\c :dbname postgres

-- user already exists
GRANT ALL PRIVILEGES ON DATABASE :dbname to :admin;

ALTER TABLE Average OWNER TO :admin;
ALTER TABLE Covariance OWNER TO :admin;


GRANT ALL ON SCHEMA public TO :admin;
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO :admin;

-- grant usage and select permission to autoincrements

GRANT USAGE, SELECT ON ALL SEQUENCES IN SCHEMA public TO :admin;
