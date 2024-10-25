#include "main.h"

int micro_sleep(long usec)
{
    struct timespec ts;
    int res;

    if (usec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = usec / 1000000;
    ts.tv_nsec = (usec % 1000000) * 1000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

int main() {
    Con2DB db = Con2DB(POSTGRESQL_SERVER, POSTGRESQL_PORT, POSTGRESQL_USER, POSTGRESQL_PSW, POSTGRESQL_DBNAME);
    PGresult *query_res;

    char query[QUERY_LEN];

    while(1) {
        sprintf(query, "SELECT EXTRACT(EPOCH FROM AVG(calculate_at - end_timestamp)) * 1000 as avg FROM Average");

        query_res = db.RunQuery(query, true);

        if ((PQresultStatus(query_res) != PGRES_COMMAND_OK && PQresultStatus(query_res) != PGRES_TUPLES_OK) || PQntuples(query_res) <= 0) {
            printf("DB_ERROR\n");
            continue;
        }

        char* average = PQgetvalue(query_res, 0, PQfnumber(query_res, "avg"));

        if(strlen(average)==0){
            sprintf(average, "0");
        }
        cout << "Average avg: " << average << endl;

        char response_status[8];

        if (atof(average) <= MAX_CONNECTION_TIME_AVG) {
            sprintf(response_status, "SUCCESS");
        } else {
            sprintf(response_status, "ERROR");
        }

        sprintf(query, "SELECT EXTRACT(EPOCH FROM AVG(calculate_at - end_timestamp)) * 1000 as avg FROM Covariance");

        query_res = db.RunQuery(query, true);

        if ((PQresultStatus(query_res) != PGRES_COMMAND_OK && PQresultStatus(query_res) != PGRES_TUPLES_OK) || PQntuples(query_res) <= 0) {
            printf("DB_ERROR\n");
            continue;
        }

        average = PQgetvalue(query_res, 0, PQfnumber(query_res, "avg"));

        if(strlen(average) == 0){
            sprintf(average, "0");
        }
        cout << "Average cov: " << average << endl;

        if (atof(average) <= MAX_RESPONSE_TIME_AVG) {
            sprintf(response_status, "SUCCESS");
        } else {
            sprintf(response_status, "ERROR");
        }

        micro_sleep(60000000);
    }

    db.finish();

    return 0;
}