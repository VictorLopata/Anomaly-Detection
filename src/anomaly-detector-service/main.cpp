
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "main.h"
#include "../con2redis/con2redis.h"


using namespace std;


int binomialCoeff(int n, int k) {
    int ans = 1;
    if (k > n - k) {
        k = n - k;
    }
    for (int i = 0; i < k; i++) {
        ans = ans * (n - i);
        ans = ans / (i + 1);
    }
    return ans;
}

int main() {
    // Connessione a Redis
    redisContext *c = redisConnect(REDIS_SERVER, REDIS_PORT);


    // Aspetta la configurazione sulla stream conf
    config conf = getConf(c);
    cout << "n_stream: " << conf.num_streams << " Threshold: "<< conf.threshold << " W: "<< conf.W <<endl;
    int numStreamCov = binomialCoeff(conf.num_streams, 2);
    int numTotStream = numStreamCov + conf.num_streams;

    auto* streams = new string[numTotStream];
    auto* avgCurr = new double[conf.num_streams];
    auto* covCurr = new double[numStreamCov];

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
    map<string, string> lastID;
    for (int i = 0; i < numTotStream; i++)
    {
        lastID[streams[i]] = "$";
    }

    while (1) {
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
                    cout << "Stream: " << nomeStream << ", ID: " << entryID << endl;
                    for (size_t k = 0; k < fieldsReply->elements; k += 2)
                    {
                        // TODO: Confronta con media corrente, se supera threshold, allora anomalia. Salva nel database.
                        string campo = fieldsReply->element[k]->str;
                        string valore = fieldsReply->element[k+1]->str;
                        cout << "  " << campo << ": " << valore << endl;
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
