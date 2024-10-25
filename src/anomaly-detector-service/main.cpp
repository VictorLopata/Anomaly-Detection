#include "main.h"

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

    // Initialization of streams, vectors and data to be used
    int numStreamCov = binomialCoeff(conf.num_streams, 2);
    int numTotStream = numStreamCov + conf.num_streams;

    vector<string> streams(numTotStream);
    vector<double> avgCurr(conf.num_streams, 0.0);
    vector<double> covCurr(numStreamCov, 0.0);
    vector<double> currStreams(numTotStream);
    vector<bool> firstTime(numTotStream, true);

    size_t pos;
    string numberPart;
    int ind;
    bool isAvg;
    bool anomaly;
    char query[QUERY_LEN];

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

                            if (firstTime[ind]) {

                                firstTime[ind] = false;
                                currStreams[ind] = valore;
                                anomaly = false;

                            } else {

                                // Controlla se è presente l'anomalia.
                                anomaly = abs(valore - currStreams[ind]) > conf.threshold;
                                // cout << "Valore precedente: " << currStreams[ind] << "    Valore attuale: " << valore << "    Anomaly: " << anomaly << endl;

                                if (anomaly) {
                                    sendAnomaly(c, reply, valore, isAvg,stoi(numberPart), numStreamCov);
                                } else {
                                    currStreams[ind] = valore;
                                    currStreams[ind] /= 2;
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
                    //cout << "Campo: " << campo << " Valore: " << valore << endl;
                    // Mando al Database.
                    if (isAvg) {
                        cout << "RICEVUTA MEDIA DEL SENSORE " << stoi(numberPart) << ". AVG = " << valore << endl;
                        sprintf(query, "INSERT INTO average (sensor_id, start_timestamp, end_timestamp, value, is_anomaly) VALUES (%s,\'%s\',\'%s\',%s, %s);",
                                numberPart.c_str(), startTimestamp.c_str(), endTimestamp.c_str(), to_string(valore).c_str(), string(anomaly ? "TRUE" : "FALSE").c_str());
                    } else {
                        // pair<int, int> p = findPair(stoi(numberPart), numStreamCov);
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
