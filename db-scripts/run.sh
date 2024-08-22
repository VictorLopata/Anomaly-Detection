#!/bin/sh

sudo -u postgres psql postgres -f parameters.sql -f setup.sql -f schemas.sql -f triggers.sql