#include <iostream>
#include <cstdlib>
#include "../utils/configuration.h"
#include "../con2redis/con2redis.h"
#include <string>

#define REDIS_SERVER "localhost"
#define REDIS_PORT 6379

const std::string PURP_GRASS = "\033[1;35m";
const std::string YELL_GRASS = "\033[1;33m";
const std::string BLUE_GRASS = "\033[1;34m";

const double DEFAULT_W = 25.0;
const double DEFAULT_SENS = 3;


void printCovAnomaly(redisReply* reply) {
    // Processa i messaggi dallo stream covAnomaly
    for (size_t i = 0; i < reply->element[0]->element[1]->elements; ++i) {
        std::string field = reply->element[0]->element[1]->element[i]->str;
        std::string value = reply->element[0]->element[1]->element[++i]->str;

        if (field == "first_sensor") {
            cout << "first_sensor: " << value << endl;
        } else if (field == "second_sensor") {
            cout << "second_sensor: " << value << endl;
        } else if (field == "anomalValue") {
            cout << "anomalValue: " << value << endl;
        }
    }
}

void printAvgAnomaly(redisReply* reply) {
    // Processa i messaggi dallo stream avgAnomaly
    for (size_t i = 0; i < reply->element[0]->element[1]->elements; ++i) {
        std::string field = reply->element[0]->element[1]->element[i]->str;
        std::string value = reply->element[0]->element[1]->element[++i]->str;

        if (field == "sensor") {
            cout << "sensor: " << value << endl;
        } else if (field == "anomalValue") {
            cout << "anomalValue: " << value << endl;
        }
    }
}