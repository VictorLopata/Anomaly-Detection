#include "main.h"



int main() {

    config conf;
    redisContext *c;
    redisReply *reply;
    redisReply* streamReply;
    redisReply* entriesReply;
    redisReply* entryReply;
    redisReply* fieldsReply;


    // Connessione al server Redis
    c = redisConnect(REDIS_SERVER, REDIS_PORT);
    if (c == nullptr || c->err) {
        cerr << "Error connecting to redis at " << REDIS_SERVER << endl;
        cerr << "Error: " << c->errstr << endl;
        exit(1);
    }

    cout << "Aspettando configurazione..." << endl;
    conf = getConf(c);



    vector<string> streams;
    map<string, string> lastIDs;

    // Inserimento nomi stream medie e delle covarianze
    for (int i = 0; i < conf.num_streams; i++) {
        streams.push_back("str#" + to_string(i));
    }
    streams.push_back("end");

    for (const auto &stream : streams) {
        lastIDs[stream] = get_last_message_id(c, stream);
    }


    string startTimestamp;
    string endTimestamp;
    bool end = false;
    std::unordered_map<int, std::vector<double> > streams_data;
    while (true) {
        string command = "XREAD BLOCK 0 STREAMS";

        for (const auto &stream : streams) {
            command += " " + stream;
        }
        for (const auto &stream : streams) {
            command += " " + lastIDs[stream];
        }

        // std::cout << "Time window (seconds): " << w << " | Data stream n : " << n << std::endl;




        for (const auto &stream : streams) {
            //cout <<  stream << ": " + lastIDs[stream] << endl;
        }

        cout << "[" << getCurrentTimestamp() << "]" << " Get stream data..." << endl;
        // get_stream_data(c, command, streams_data, lastIDs);

        redisReply* reply = (redisReply *)redisCommand(c, command.c_str());
        if (reply && reply->type == REDIS_REPLY_ARRAY) {
            for (size_t i = 0; i < reply->elements; ++i) {

                redisReply* streamReply = reply->element[i];
                string streamName = streamReply->element[0]->str;
                redisReply* entriesReply = streamReply->element[1];

                for (size_t j = 0; j < entriesReply->elements; ++j) {
                    redisReply* entryReply = entriesReply->element[j];
                    string entryID = entryReply->element[0]->str;
                    redisReply* fieldsReply = entryReply->element[1];

                    for (size_t k = 0; k < fieldsReply->elements; k += 2) {
                        string field = fieldsReply->element[k]->str;
                        string value = fieldsReply->element[k + 1]->str;
                        int sensorIdx = distance(streams.begin(), find(streams.begin(), streams.end(), streamName));

                        // Add value to the corresponding index of the stream
                        if (streamName == "end" && value == "ok") {
                            end = true;

                        }
                        else if (field == "val") {
                            cout << "RICEVUTO VALORE DA SensorID: " << sensorIdx << " Value: " << value << endl;
                            streams_data[sensorIdx].push_back(stof(value));
                        } else if (field == "startTimestamp") {
                            startTimestamp = value;
                        } else if (field == "endTimestamp") {
                            endTimestamp = value;
                        }

                    }

                    lastIDs[streamName] = entryID; // Update the last ID
                }
            }
        }


        if (end) {
            cout << "CALCULATING COVARIANCE..." << endl;
            calculate_cov(c, streams_data, startTimestamp, endTimestamp, conf.num_streams);
            streams_data.clear();
            end = false;
        }

    }

    // Chiude la connessione Redis
    redisFree(c);

    return 0;
}


