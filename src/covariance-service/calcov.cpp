#include <iostream>
#include <vector>
#include <unordered_map>
#include <thread>
#include <numeric>
#include <chrono>
#include <hiredis/hiredis.h>

#define REDIS_SERVER "localhost"
#define REDIS_PORT 6379

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

// Ottiengo l'unità di tempo w da Redis
int get_time_window(redisContext* c) {
    redisReply* reply = (redisReply*)redisCommand(c, "GET time_window");
    int w = (reply && reply->type == REDIS_REPLY_STRING) ? std::stoi(reply->str) : 5; // Predefinito 5 secondi
    freeReplyObject(reply);
    return w;
}

// Ottiengo il numero di stream di dati n da Redis
int get_dataset_size(redisContext* c) {
    redisReply* reply = (redisReply*)redisCommand(c, "GET dataset_size");
    int n = (reply && reply->type == REDIS_REPLY_STRING) ? std::stoi(reply->str) : 2; // Predefinito 2 stream
    freeReplyObject(reply);
    return n;
}

// Legge i dati da Redis Stream e archivia in ciascuna coda del stream di dati
bool get_stream_data(redisContext* c, int n, std::unordered_map<int, std::vector<double>>& streams_data) {
    redisReply* reply = (redisReply*)redisCommand(c, "XREAD BLOCK 5000 STREAMS dataset_stream $ COUNT %d", n);

    if (reply->type == REDIS_REPLY_ARRAY) {
        for (size_t i = 0; i < reply->elements; ++i) {
            redisReply* stream = reply->element[i];
            if (stream->type == REDIS_REPLY_ARRAY) {
                redisReply* entries = stream->element[1];

                for (size_t j = 0; j < entries->elements; ++j) {
                    redisReply* entry = entries->element[j];

                    int stream_id = std::stoi(entry->element[1]->element[0]->str);  // Supponiamo che il primo campo sia l'ID del stream di dati
                    double data_value = std::stod(entry->element[1]->element[1]->str);  // Supponiamo che il secondo campo sia il valore dei dati

                    // Aggiunge al stream di dati solo se il valore dei dati non è nullo
                    if (!entry->element[1]->element[1]->str.empty()) {
                        streams_data[stream_id].push_back(data_value);
                    }
                }
            }
        }
    }

    freeReplyObject(reply);
    return true;
}

// Controlla se due stram di dati hanno la stessa dimensione
bool same_size(const std::vector<double>& x, const std::vector<double>& y) {
    return x.size() == y.size();
}

int main() {
    // Connessione al server Redis
    redisContext* c = redisConnect(REDIS_SERVER, REDIS_PORT);
    if (c == NULL || c->err) {
        std::cerr << "Redis connection error: " << (c ? c->errstr : "NULL") << std::endl;
        return 1;
    }

    int batch_count = 0; // Registrare il batch attualmente calcolato

    while (true) {
        // Ottieni l'unità di tempo w e il numero di stream di dati n da Redis
        int w = get_time_window(c);
        int n = get_dataset_size(c);

        std::cout << "Time window (seconds): " << w << " | Data stream n : " << n << std::endl;

        std::unordered_map<int, std::vector<double>> streams_data;

        // Ottieni dati validi dal stream di dati
        get_stream_data(c, n, streams_data);

        // Attraversa lo stream di dati ed esegue il calcolo di covarianza
        for (auto it1 = streams_data.begin(); it1 != streams_data.end(); ++it1) {
            for (auto it2 = std::next(it1); it2 != streams_data.end(); ++it2) {
                // Calcola la covarianza solo per stream di dati della stessa dimensione
                if (same_size(it1->second, it2->second)) {
                    // calcola la covarianza
                    double cov = covariance(it1->second, it2->second);
                    std::cout << "Covariance between stream " << it1->first << " and stream " << it2->first << ": " << cov << std::endl;

                    // Invia i risultati al canale Redis
                    std::string channel = "c#" + std::to_string(batch_count);
                    redisReply* push_reply = (redisReply*)redisCommand(c, "XADD %s * cov %f stream1 %d stream2 %d", channel.c_str(), cov, it1->first, it2->first);
                    freeReplyObject(push_reply);
                } else {
                    std::cout << "Stream " << it1->first << " and Stream " << it2->first << " do not have the same size, skipping covariance calculation." << std::endl;
                }
            }
        }

        // Attende w secondi prima di eseguire il successivo ciclo di calcoli
        std::this_thread::sleep_for(std::chrono::seconds(w));
        batch_count++;  // Aggiorna il numero di batch
    }

    // Chiude la connessione Redis
    redisFree(c);

    return 0;
}
