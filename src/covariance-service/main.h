#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <thread>
#include <numeric>
#include <chrono>
#include "../con2redis/con2redis.h"
#include <iomanip>
#include <sstream>

using namespace std;
using namespace chrono;



#define REDIS_SERVER "localhost"
#define REDIS_PORT 6379

string get_last_message_id(redisContext* context, const string& stream_name) {
    // Eseguiamo il comando XINFO STREAM <stream_name>
    redisReply* reply = (redisReply*) redisCommand(context, "XINFO STREAM %s", stream_name.c_str());



    // Troviamo l'ID dell'ultimo messaggio ("last-entry")
    string last_message_id = "";
    for (size_t i = 0; i < reply->elements; i += 2) {
        string key(reply->element[i]->str);
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

// Controlla se due stram di dati hanno la stessa dimensione
bool same_size(const vector<double>& x, const vector<double>& y) {
    return x.size() == y.size();
}
// Funzione che calcola la covarianza
double covariance(const std::vector<double>& x, const std::vector<double>& y) {
    double mean_x = std::accumulate(x.begin(), x.end(), 0.0) / x.size();
    double mean_y = std::accumulate(y.begin(), y.end(), 0.0) / y.size();
    double cov = 0.0;

    for (size_t i = 0; i < x.size(); ++i) {
        cov += (x[i] - mean_x) * (y[i] - mean_y);
    }

    return cov / x.size();
}

int getIndex(int N, int i, int j) {
    if (i >= j) {
        // Gestione dell'input non valido
        int tmp = i;
        i = j;
        j = tmp;
    }
    int index = i * (2 * N - i - 1) / 2 + (j - i - 1);
    return index;
}

void calculate_cov(redisContext *c, unordered_map<int, vector<double> >& streams_data, string startTimestamp, string endTimestamp, int n) {

    // Attraversa lo stream di dati ed esegue il calcolo di covarianza
    for (auto it1 = streams_data.begin(); it1 != streams_data.end(); ++it1) {
        for (auto it2 = next(it1); it2 != streams_data.end(); ++it2) {
            cout << "entriii" << endl;
            // Calcola la covarianza solo per stream di dati della stessa dimensione
            if (same_size(it1->second, it2->second)) {
                // calcola la covarianza
                double cov = covariance(it1->second, it2->second);
                cout << "Covariance between stream " << it1->first << " and stream " << it2->first << ": " << cov << endl;

                int indice = getIndex(n, it1->first, it2->first);
                cout << "INDICE: " << indice << endl;
                // Invia i risultati al canale Redis
                string channel = "c#" + to_string(indice);

                string comm = "XADD " + channel + " * val " + to_string(cov) + " startTimestamp " + startTimestamp + " endTimestamp " + endTimestamp;
                cout << comm << endl;
                redisReply* reply = (redisReply *)redisCommand(c, comm.c_str());

                freeReplyObject(reply);
            } else {
                cout << "Stream " << it1->first << " and Stream " << it2->first << " do not have the same size, skipping covariance calculation." << endl;
            }
        }
    }
}

string getCurrentTimestamp() {
    auto now = system_clock::now();
    auto in_time_t = system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&in_time_t), "%Y-%m-%d %H:%M:%S"); // Formattato come stringa leggibile
    return ss.str();
}



// Ottengo l'unità di tempo w da Redis
int get_time_window(redisContext* c) {
    redisReply* reply = (redisReply*)redisCommand(c, "GET time_window");
    int w = (reply && reply->type == REDIS_REPLY_STRING) ? stoi(reply->str) : 5; // Predefinito 5 secondi
    freeReplyObject(reply);
    return w;
}

// Ottengo il numero di stream di dati n da Redis
int get_dataset_size(redisContext* c) {
    redisReply* reply = (redisReply*)redisCommand(c, "GET dataset_size");
    int n = (reply && reply->type == REDIS_REPLY_STRING) ? stoi(reply->str) : 2; // Predefinito 2 stream
    freeReplyObject(reply);
    return n;
}

// Legge i dati da Redis Stream e archivia in ciascuna coda del stream di dati
bool get_stream_data(redisContext* c, string command, std::unordered_map<int, std::vector<double> >& streams_data, map<string, string>& lastIDs) {
    string prefix = "stream#";
    redisReply* reply = (redisReply *)redisCommand(c, command.c_str());
    if (reply && reply->type == REDIS_REPLY_ARRAY) {
        for (size_t i = 0; i < reply->elements; ++i) {

            redisReply* streamReply = reply->element[i];
            string streamName = streamReply->element[0]->str;
            redisReply* entriesReply = streamReply->element[1];

            size_t pos = streamName.find(prefix);
            string number_part = streamName.substr(prefix.length());
            int sensorIdx = std::stoi(number_part);

            for (size_t j = 0; j < entriesReply->elements; ++j) {
                redisReply* entryReply = entriesReply->element[j];
                string entryID = entryReply->element[0]->str;
                redisReply* fieldsReply = entryReply->element[1];

                for (size_t k = 0; k < fieldsReply->elements; k += 2) {

                    string field = fieldsReply->element[k]->str;
                    double value = stof(fieldsReply->element[k + 1]->str);
                    cout << "RICEVUTO VALORE DA SensorID: " << sensorIdx << " Value: " << value << endl;
                    streams_data[sensorIdx].push_back(value);

                }

                lastIDs[streamName] = entryID; // Update the last ID
            }
        }
    }

    freeReplyObject(reply);
    return true;
}


