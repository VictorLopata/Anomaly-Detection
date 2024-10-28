#include <string>
using namespace std;


struct window_stream {
    double val = 0;
    int count = 0;
};

string get_last_message_id(redisContext* context, const string& stream_name) {
    // Eseguiamo il comando XINFO STREAM <stream_name>
    redisReply* reply = (redisReply*) redisCommand(context, "XINFO STREAM %s", stream_name.c_str());

    // Troviamo l'ID dell'ultimo messaggio ("last-entry")
    string last_message_id = "";
    for (size_t i = 0; i < reply->elements; i += 2) {
        std::string key(reply->element[i]->str);
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
