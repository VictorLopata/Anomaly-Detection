-- create the database :dbname
DROP DATABASE IF EXISTS :dbname ;
CREATE DATABASE :dbname ;

\c :dbname postgres

CREATE USER :admin WITH ENCRYPTED PASSWORD 'admin';
