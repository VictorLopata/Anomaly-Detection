#!/bin/bash

# Configurazione delle variabili
DB_NAME="anom1"
DB_USER="postgres"


echo "Creazione del database $DB_NAME..."
createdb -U $DB_USER $DB_NAME

# Verifica se la creazione del database ha avuto successo
if [ $? -ne 0 ]; then
    echo "Errore nella fase di creazione del database; potrebbe già esistere!"
    exit 1
fi

echo "Esecuzione dello script SQL..."
psql -U $DB_USER -d $DB_NAME -f schemas.sql -f triggers.sql

if [ $? -ne 0 ]; then
    echo "Errore nell'esecuzione dello script SQL."
    exit 1
fi
echo "Setup completato con successo."
