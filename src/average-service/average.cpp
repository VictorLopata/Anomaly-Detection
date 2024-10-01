#include "average.h"
#include <iostream>

Average::Average(const string& redis_host, const string& redis_port) {
  c = redisConnect(redis_host.c_str(), redis_port);
  if (c == NULL || c->err) {
    cerr << "Error connecting to redis at " << redis_host << ":" << redis_port << endl;
    exit(1);
  }
}

Average::~Average() {
  if(c != nullptr){
    redisFree(c);
  }
}

void Average::listenStreams(const vector<string>& streams) {
  for (const string& stream : streams) {

  }
}