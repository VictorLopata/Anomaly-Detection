#include "average.h"
#include <iostream>


#define REDIS_SERVER "localhost"
#define REDIS_PORT 6379

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
  values.resize(n_sensors, 0.0);

  // This initializes the stream names which we are listening
  for (int i = 0; i < n_sensors; i++) {
    streams.push_back("stream #" + to_string(i));
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
            values[sensorIdx] += value;
          }

          lastIDs[streamName] = entryID; // Update the last ID
          count++; // Increment the counter after reading a new value
        }
      }
    }

    freeReplyObject(reply);

    auto now = steady_clock::now();
    duration<double> elapsed = now - start;

    if (elapsed.count() >= windowSize) {
        cout << "Numero di secondi: " << elapsed.count() << endl;
        if (this->count != 0) {
            calculate_averages();
            cleanVectors();
            count = 0;  // Reset the counter to start counting new values during the new window
        }
      start = steady_clock::now();  // Reset the timer for the next window
    }
  }
}

void Average::calculate_averages() {

  for (int i = 0; i < n_sensors; i++) {
    double avg =  values[i] / this->count;
    cout << "Count: " << this->count << endl;
    string streamName = "avgS" + to_string(i);
    cout << "avg" << i << ": " << avg << endl;
    /**
    string comm = "XADD " + streamName + "* avg " + to_string(avg);
    redisReply* reply = (redisReply *)redisCommand(c, comm.c_str());

    if (reply == nullptr) {
      cerr << "Error sending the average to Redis for sensor" << i << endl;
    }

    freeReplyObject(reply);
     **/
  }
}

void Average::cleanVectors() {
  for (int i = 0; i < values.size(); ++i) {
    values[i] = 0.0;
  }
}