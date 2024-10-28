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

    PGresult *query_res;
    Con2DB db(POSTGRESQL_SERVER, POSTGRESQL_PORT, POSTGRESQL_USER, POSTGRESQL_PSW, POSTGRESQL_DBNAME);

    // Connessione a Redis
    c = redisConnect(REDIS_SERVER, REDIS_PORT);

    // Wait configurations from user-interaction-service
    conf = getConf(c);

    int numStreamCov = binomialCoeff(conf.num_streams, 2);
    int numTotStream = numStreamCov + conf.num_streams;

    vector<string> streams(numTotStream);
    // Remove currStreams and firstTime
    vector<int> n(numTotStream, 0); // tiene traccia del numero di valori ricevuti per ogni stream.
    vector<double> mean(numTotStream, 0.0); // rappresenta la media corrente dei valori ricevuti per ogni stream.
    vector<double> M2(numTotStream, 0.0); // accumula il quadrato della distanza di ogni valore dalla media; viene usata per calcolare la varianza.

    size_t pos;
    string numberPart;
    int ind;
    bool isAvg;
    bool anomaly;
    char query[QUERY_LEN];
    double k_value = conf.sens; // Number of standard deviations for anomaly detection

    // Inserimento nomi stream medie e delle covarianze
    for (int i = 0; i < numTotStream; i++){
        string strName;
        if (i < conf.num_streams) {
            strName = "a#" + to_string(i);
        } else {
            strName = "c#" + to_string(i);
        }
        streams[i] = strName;
    }

    // Mappa per tenere traccia degli ultimi ID letti per ogni stream
    std::map<std::string, std::string> lastID;
    for (const auto& streamName : streams) {
        lastID[streamName] = get_last_message_id(c, streamName);
    }

    cout << "Ora ascolto le stream ....." << endl;
    string comGen = "XREAD BLOCK 0 STREAMS";

    for (int i = 0; i < numTotStream; i++) {
        comGen += " " + streams[i];
    }
    string com;

    while (true) {
        // Composizione del comando XREAD
        com = comGen;
        for (int i = 0; i < numTotStream; i++) {
            com += " " + lastID[streams[i]];
        }

        // Esecuzione comando
        reply = RedisCommand(c, com.c_str());

        if (reply == nullptr) {
            cerr << "Errore nell'esecuzione del comando XREAD" << endl;
            break;
        } else {
            for (size_t i = 0; i < reply->elements; ++i) {

                streamReply = reply->element[i];
                string nomeStream = streamReply->element[0]->str;
                entriesReply = streamReply->element[1];

                if ('a' == nomeStream[0]) {
                    isAvg = true;
                } else {
                    isAvg = false;
                }
                cout << "------ " << nomeStream << " ------" << endl;

                pos = nomeStream.find('#');
                numberPart = nomeStream.substr(pos + 1);
                ind = stoi(numberPart);

                if (!isAvg) {
                    ind = conf.num_streams + ind;
                }

                for (size_t j = 0; j < entriesReply->elements; ++j) {

                    entryReply = entriesReply->element[j];
                    string entryID = entryReply->element[0]->str;
                    lastID[nomeStream] = entryID;
                    fieldsReply = entryReply->element[1];

                    double valore;
                    string startTimestamp;
                    string endTimestamp;

                    for (size_t k = 0; k < fieldsReply->elements; k += 2)
                    {
                        string campo = fieldsReply->element[k]->str;

                        if (campo == "val") {
                            valore = stod(fieldsReply->element[k+1]->str);

                            // delta è la differenza tra il nuovo valore e la media mean[ind], ovvero quanto il nuovo valore si discosta dalla media attuale.
                            // La media viene aggiornata aggiungendo una piccola frazione del delta, in modo che so avvicini al nuovo valore senza cambiare drasticamente.
                            // M2[ind] accumula il quadrato della differenza tra il nuovo valore e la media. Serve per il calcolo della varianza, necessaria per calcolare la deviazione standard.

                            double delta, delta2, variance, std_dev;

                            if (n[ind] == 0) {
                                // First value initialization
                                n[ind] = 1;
                                mean[ind] = valore;
                                M2[ind] = 0.0;
                                anomaly = false;
                            } else {
                                n[ind] += 1;
                                delta = valore - mean[ind];
                                mean[ind] += delta / n[ind];
                                delta2 = valore - mean[ind];
                                M2[ind] += delta * delta2;

                                if (n[ind] > 1) {
                                    variance = M2[ind] / (n[ind] - 1);
                                    std_dev = sqrt(variance);

                                    if (std_dev == 0) {
                                        anomaly = false;
                                    } else {
                                        anomaly = abs(valore - mean[ind]) > k_value * std_dev;
                                    }
                                } else {
                                    anomaly = false;
                                }

                                if (anomaly) {
                                    sendAnomaly(c, reply, valore, isAvg, stoi(numberPart), numStreamCov);
                                }
                            }

                        } else if (campo == "startTimestamp") {
                            startTimestamp = fieldsReply->element[k+1]->str;
                            replace(startTimestamp.begin(), startTimestamp.end(), '_', ' ');
                        } else if (campo == "endTimestamp") {
                            endTimestamp = fieldsReply->element[k+1]->str;
                            replace(endTimestamp.begin(), endTimestamp.end(), '_', ' ');
                        }
                    }

                    // Mando al Database.
                    if (isAvg) {
                        cout << "RICEVUTA MEDIA DEL SENSORE " << stoi(numberPart) << ". AVG = " << valore << endl;
                        sprintf(query, "INSERT INTO average (sensor_id, start_timestamp, end_timestamp, value, is_anomaly) VALUES (%s,\'%s\',\'%s\',%s, %s);",
                                numberPart.c_str(), startTimestamp.c_str(), endTimestamp.c_str(), to_string(valore).c_str(), string(anomaly ? "TRUE" : "FALSE").c_str());
                    } else {
                        pair<int, int> p = indexToPair(stoi(numberPart), conf.num_streams);
                        cout << "RICEVUTA COVARIANZA TRA SENSORE " << p.first << " E SENSORE " << p.second << " (" << stoi(numberPart) << ")" << ". COV: " << valore <<endl;
                        sprintf(query, "INSERT INTO covariance (sensor1_id, sensor2_id, start_timestamp, end_timestamp, value, is_anomaly) VALUES (%d, %d, \'%s\', \'%s\',%f, %s);",
                                p.first,p.second, startTimestamp.c_str(), endTimestamp.c_str(), valore, string(anomaly ? "TRUE" : "FALSE").c_str());
                    }
                    query_res = db.RunQuery(query, false);

                    if (PQresultStatus(query_res) != PGRES_COMMAND_OK) {
                        cout << "Errore durante DB: " << PQresultStatus(query_res) << endl;
                        continue;
                    }

                }
            }
        }
        freeReplyObject(reply);
    }
    db.finish();
    redisFree(c);
    return 0;
}

