#include "main.h"

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

    config conf;
    redisContext *c;
    redisReply *reply;
    redisReply* streamReply;
    redisReply* entriesReply;
    redisReply* entryReply;
    redisReply* fieldsReply;



    // Connessione a Redis
    c = redisConnect(REDIS_SERVER, REDIS_PORT);

    // Wait configurations from user-interaction-service
    conf = getConf(c);

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
        reply = RedisCommand(c, com.c_str());
        if (reply == nullptr) {
            cerr << "Errore nell'esecuzione del comando XREAD" << endl;
            break;
        } else {
            for (int i = 0; i < reply->elements; ++i) {

                streamReply = reply->element[i];
                string nomeStream = streamReply->element[0]->str;
                entriesReply = streamReply->element[1];

                for (int j = 0; j < entriesReply->elements; ++j) {

                    entryReply = entriesReply->element[j];
                    string entryID = entryReply->element[0]->str;
                    fieldsReply = entryReply->element[1];

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
