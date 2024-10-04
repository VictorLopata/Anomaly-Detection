#include "average.h"
#include <iostream>

Average::Average(const string& redis_host, const string& redis_port, int n_sensors, int windowSize) {
  c = redisConnect(redis_host.c_str(), redis_port);
  if (c == nullptr || c->err) {
    cerr << "Error connecting to redis at " << redis_host << ":" << redis_port << endl;
    exit(1);
  }
  this->n_sensors = n_sensors;
  this->windowSize = windowSize;

  for (int i = 0; i < n_sensors; i++) {
    string streamName = "stream_" + to_string(i+1);
    values[streamName] = vector<float>();
  }
}

Average::~Average() {
  if(c != nullptr){
    redisFree(c);
  }
}

void Average::getValueStream(const string& streamName, float value) {
  values[streamName].push_back(value);
}

double Average::calculate_average(const vector<double> &values) {

}


void Average::listenStreams(const vector<string>& streams) {
  for (const string& stream : streams) {
    // Read the values from the N streams of redis
    redisReply* reply = (redisReply*)redisCommand(c, "XREAD COUNT 1 STREAMS %s 0", count, stream);
  }
}