
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "main.h"
#include "../con2redis/con2redis.h"
#include "../average-service/average.h"


using namespace std;


// Funzione per calcolare il coefficiente binomiale
int binomialCoeff(int n, int k) {
    int ans = 1;
    if (k > n - k) {
        k = n - k;
    }
    for (int i = 0; i < k; ++i) {
        ans *= (n - i);
        ans /= (i + 1);
    }
    return ans;
}

int main() {
    // Connessione a Redis
    redisContext *c = redisConnect(REDIS_SERVER, REDIS_PORT);

    // Wait configurations from user-interaction-service
    config conf = getConf(c);
    cout << "n_stream: " << conf.num_streams << " Threshold: "<< conf.threshold << " W: "<< conf.W <<endl;
    int numStreamCov = binomialCoeff(conf.num_streams, 2);
    int numTotStream = numStreamCov + conf.num_streams;

    vector<string> streams(numTotStream);
    vector<double> avgCurr(conf.num_streams, 0.0);
    vector<double> covCurr(numStreamCov, 0.0);

    // Inserimento nomi stream medie e delle covarianze
    for (int i = 0; i < numTotStream; i++){
        string strName;
        if (i < conf.num_streams) {
            strName = "avgS" + to_string(i);
        } else {
            strName = "covN" + to_string(i);
        }
        streams[i] = strName;
    }

    // Ascolto le stream su Redis.

    // Mappa per tenere traccia degli ultimi ID letti per ogni stream
    std::map<std::string, std::string> lastID;
    for (const auto& streamName : streams) {
        lastID[streamName] = "$";
    }


    while (true) {
        // Composizione del comando XREAD
        string com = "XREAD COUNT 1 BLOCK 0 STREAMS";
        for (int i = 0; i < numTotStream; i++) {
            com += " " + streams[i];
        }

        for (int i = 0; i < numTotStream; i++) {
            com += " " + lastID[streams[i]];
        }
        // Esecuzione comando
        redisReply* reply = (redisReply*)redisCommand(c, com.c_str());
        if (reply == nullptr) {
            cerr << "Errore nell'esecuzione del comando XREAD" << endl;
            break;
        } else {
            for (int i = 0; i < reply->elements; ++i) {

                redisReply* streamReply = reply->element[i];
                string nomeStream = streamReply->element[0]->str;
                redisReply* entriesReply = streamReply->element[1];

                for (int j = 0; j < entriesReply->elements; ++j) {

                    redisReply* entryReply = entriesReply->element[j];
                    string entryID = entryReply->element[0]->str;
                    redisReply* fieldsReply = entryReply->element[1];

                    for (size_t k = 0; k < fieldsReply->elements; k += 2)
                    {
                        string campo = fieldsReply->element[k]->str;
                        string valore = fieldsReply->element[k+1]->str;

                    }

                    lastID[nomeStream] = entryID;
                }
            }
        }
        freeReplyObject(reply);
    }

    redisFree(c);
    return 0;
}
