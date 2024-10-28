#include "average.h"

std::string get_last_message_id(redisContext* context, const std::string& stream_name) {
    // Eseguiamo il comando XINFO STREAM <stream_name>
    redisReply* reply = (redisReply*) redisCommand(context, "XINFO STREAM %s", stream_name.c_str());



    // Troviamo l'ID dell'ultimo messaggio ("last-entry")
    std::string last_message_id = "";
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


// This will return us the current time (for the average or covariance calculated)
string getCurrentTimestamp() {
    auto now = system_clock::now();
    auto in_time_t = system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&in_time_t), "%Y-%m-%d %H:%M:%S"); // Formattato come stringa leggibile
    return ss.str();
}

Average::Average() {
  c = redisConnect(REDIS_SERVER, REDIS_PORT);
  if (c == nullptr || c->err) {
    cerr << "Error connecting to redis at " << REDIS_SERVER << endl;
    cerr << "Error: " << c->errstr << endl;
    exit(1);
  }

  // This gives us the input data about the streams and thw window
  cout << "Aspettando configurazione..." << endl;
  config configuration = getConf(c);

  this->n_sensors = configuration.num_streams;
  this->windowSize = configuration.W;

  cout << "Configurazione ricvuta: W(" << this->windowSize << ") e n_sensor(" << this->n_sensors << ")" << endl;


  // This initializes the starting vector of values to 0
  //values.resize(n_sensors, 0.0);
  str_info.resize(n_sensors);



  // This initializes the stream names which we are listening
  for (int i = 0; i < n_sensors; i++) {
    streams.push_back("str#" + to_string(i));
  }
  streams.push_back("end");

}

Average::~Average() {
  if(c != nullptr){
    redisFree(c);
  }
}

void Average::listenStreams() {
  for (const auto &stream : streams) {
    lastIDs[stream] = get_last_message_id(c, stream);
  }

  auto start = steady_clock::now(); //start the timer for W window
  string startTimestamp;
  string endTimestamp;

  bool end = false;
  cout << "Ascoltando le stream..." << endl;
  while (true) {
    string command = "XREAD BLOCK 0 STREAMS";

    // Build the command for Redis
    for (const auto &stream : streams) {
      command += " " + stream;
    }
    for (const auto &stream : streams) {
      command += " " + lastIDs[stream];
    }





    // Execute the command
    redisReply* reply = (redisReply *)redisCommand(c, command.c_str());
    cout << "RICEVUTO BLOCCO "<< endl;
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
                str_info[sensorIdx].val += stof(value);
                str_info[sensorIdx].count += 1;
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

    freeReplyObject(reply);
    if (end) {
        calculate_averages(startTimestamp, endTimestamp);
        end = false;
    }
  }
}

void Average::calculate_averages(string startTimestamp, string endTimestamp) {

  for (int i = 0; i < n_sensors; i++) {
      if (str_info[i].count == 0) {
          // cout << "La stream " << streams[i] << " non ha ricevuto nessun valore per il momento..." << endl;
          continue;
      }
      double avg =  str_info[i].val / str_info[i].count;
      str_info[i].val = 0;
      str_info[i].count = 0;
      string streamName = "a#" + to_string(i);

      string comm = "XADD " + streamName + " * val " + to_string(avg) + " startTimestamp " + startTimestamp + " endTimestamp " + endTimestamp;
      redisReply* reply = (redisReply *)redisCommand(this->c, comm.c_str());
      cout << avg << " mandato su stream " << streamName << endl;

      if (reply == nullptr) {
          cerr << "Error sending the average to Redis for sensor" << i << endl;
      }

      freeReplyObject(reply);

  }
}
