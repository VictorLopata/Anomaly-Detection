#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "../con2redis/con2redis.h"
#include "../average-service/average.h"
#include "../con2db/pgsql.h"

#define REDIS_SERVER "localhost"
#define REDIS_PORT 6379
#define QUERY_LEN 1000

#define POSTGRESQL_SERVER "localhost"
#define POSTGRESQL_PORT "5432"
#define POSTGRESQL_USER "admin"
#define POSTGRESQL_PSW "admin"
#define POSTGRESQL_DBNAME "anomalyseeker"

pair<int, int> findPair(int j, int n) {
    int a = 0;
    int somma = 0;

    // Trova il valore di a
    while (somma + (n - a - 1) <= j) {
        somma += (n - a - 1);
        a++;
    }

    // Calcola b
    int b = a + 1 + (j - somma);

    return make_pair(a,b);
}

std::pair<int, int> indexToPair(int idx, int N) {
    int low = 0, high = N - 1;
    int i = -1;
    while (low <= high) {
        int mid = (low + high) / 2;
        int s_mid = mid * (2 * N - mid - 1) / 2;
        if (s_mid <= idx) {
            i = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    int s_i = i * (2 * N - i - 1) / 2;
    int j = i + (idx - s_i) + 1;
    return make_pair(i,j);
}

void sendAnomaly(redisContext *c, redisReply *reply, double anomalValue, bool isAvg, int j, int n) {

    string com;
    if (!isAvg) {
        pair<int, int> p = findPair(j, n);
        reply = RedisCommand(c, "XADD covAnomaly * first_sensor %s second_sensor %s anomalValue %s", (to_string(p.first)).c_str(), (to_string(p.second)).c_str(), (to_string(anomalValue)).c_str());
    } else {
        reply = RedisCommand(c, "XADD avgAnomaly * sensor %s anomalValue %s", (to_string(j)).c_str(), (to_string(anomalValue)).c_str());
    }

}

void save2db(char* query) {
    cout << "Saved in db..." << endl;
}

std::string get_last_message_id(redisContext* context, const std::string& stream_name) {
    // Eseguiamo il comando XINFO STREAM <stream_name>
    redisReply* reply = (redisReply*) redisCommand(context, "XINFO STREAM %s", stream_name.c_str());



    // Troviamo l'ID dell'ultimo messaggio ("last-entry")
    std::string last_message_id = "";
    for (size_t i = 0; i < reply->elements; i += 2) {
        std::string key(reply->element[i]->str);
        if (key == "last-entry") {
            if (reply->element[i + 1]->type == REDIS_REPLY_ARRAY && reply->element[i + 1]->elements > 0) {
                last_message_id = reply->element[i + 1]->element[0]->str;
            }
            break;
        }
    }

    // Libera la memoria allocata per la risposta
    freeReplyObject(reply);
    if (last_message_id == "") {
        last_message_id = "0";
    }

    return last_message_id;
}

