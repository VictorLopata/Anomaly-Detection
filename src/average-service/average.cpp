#include "average.h"


string getCurrentTimestamp() {
    auto now = system_clock::now();
    auto in_time_t = system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S"); // Formattato come stringa leggibile
    return ss.str();
}

Average::Average() {
  c = redisConnect(REDIS_SERVER, REDIS_PORT);
  if (c == nullptr || c->err) {
    cerr << "Error connecting to redis at " << REDIS_SERVER << endl;
    cerr << "Error: " << c->errstr << endl;
    exit(1);
  }

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
    streams.push_back("stream#" + to_string(i));
  }

}

Average::~Average() {
  if(c != nullptr){
    redisFree(c);
  }
}

void Average::listenStreams() {
  for (const auto &stream : streams) {
    lastIDs[stream] = "$";
  }

  auto start = steady_clock::now(); //start the timer for W window
  string startTimestamp = getCurrentTimestamp();

  cout << "Ascoltando le stream..." << endl;
  while (true) {
    string command = "XREAD COUNT 1 BLOCK 0 STREAMS";

    for (const auto &stream : streams) {
      command += " " + stream;
    }
    for (const auto &stream : streams) {
      command += " " + lastIDs[stream];
    }


    // Execute the command
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
            double value = stof(fieldsReply->element[k + 1]->str);
            int sensorIdx = distance(streams.begin(), find(streams.begin(), streams.end(), streamName));

            // Add value to the corresponding index of the stream
            // cout << "RICEVUTO VALORE DA SensorID: " << sensorIdx << " Value: " << value << endl;
            str_info[sensorIdx].val += value;
            str_info[sensorIdx].count += 1;
          }

          lastIDs[streamName] = entryID; // Update the last ID
        }
      }
    }

    freeReplyObject(reply);

    auto now = steady_clock::now();
    duration<double> elapsed = now - start;

    if (elapsed.count() >= windowSize) {
        cout << "Numero di secondi: " << elapsed.count() << endl;
        string endTimestamp = getCurrentTimestamp();
        cout << "START TIMESTAMP: " << startTimestamp << " END TIMESTAMP: " << endTimestamp << endl;
        calculate_averages(startTimestamp, endTimestamp);

        start = steady_clock::now();
        startTimestamp = getCurrentTimestamp(); // Reset the timer for the next window
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

      replace(startTimestamp.begin(), startTimestamp.end(), ' ', '_');
      replace(endTimestamp.begin(), endTimestamp.end(), ' ', '_');
      string comm = "XADD " + streamName + " * val " + to_string(avg) + " startTimestamp " + startTimestamp + " endTimestamp " + endTimestamp;
      redisReply* reply = (redisReply *)redisCommand(this->c, comm.c_str());
      cout << avg << " mandato su stream " << streamName << endl;

      if (reply == nullptr) {
          cerr << "Error sending the average to Redis for sensor" << i << endl;
      }

      freeReplyObject(reply);

  }
}
