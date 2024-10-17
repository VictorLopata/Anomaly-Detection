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
        streams.push_back("stream#" + to_string(i));
    }

    for (const auto &stream : streams) {
        lastIDs[stream] = get_last_message_id(c, stream);
    }



    while (true) {
        string startTimestamp = getCurrentTimestamp();
        string command = "XREAD BLOCK 0 STREAMS";

        for (const auto &stream : streams) {
            command += " " + stream;
        }
        for (const auto &stream : streams) {
            command += " " + lastIDs[stream];
        }

        // std::cout << "Time window (seconds): " << w << " | Data stream n : " << n << std::endl;


        std::this_thread::sleep_for(seconds(int(conf.W)));

        std::unordered_map<int, std::vector<double> > streams_data;
        for (const auto &stream : streams) {
            //cout <<  stream << ": " + lastIDs[stream] << endl;
        }

        cout << "[" << getCurrentTimestamp() << "]" << " Get stream data..." << endl;
        get_stream_data(c, command, streams_data, lastIDs);



        string endTimestamp = getCurrentTimestamp();

        replace(startTimestamp.begin(), startTimestamp.end(), ' ', '_');
        replace(endTimestamp.begin(), endTimestamp.end(), ' ', '_');
        cout << "CALCULATING COVARIANCE..." << endl;
        calculate_cov(c, streams_data, startTimestamp, endTimestamp, conf.num_streams);


    }

    // Chiude la connessione Redis
    redisFree(c);

    return 0;
}


