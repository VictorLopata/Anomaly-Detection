#ifndef AVERAGE_H
#define AVERAGE_H

#include <iostream>
#include "../con2redis/con2redis.h"
#include <map>
#include "../utils/stream.h"
#include <chrono>
#include <iomanip>
#include <sstream>



#define REDIS_SERVER "localhost"
#define REDIS_PORT 6379

using namespace std;
using namespace chrono;


class Average {
    public:
        // Constructor for connecting to Redis
        Average();

        // Destructor for cleaning the connection
        ~Average();

        // Listen to the streams Redis
        void listenStreams();

        // This method calculates average from the streams (Sensors in our project)
        void calculate_averages(string startTimestamp, string endTimestamp);


    private:
        redisContext* c; // Connection to Redis
        vector<string> streams; // Stream names of the stream I am listening
        map<string, string> lastIDs; // Last ID stored for XREAD

        int n_sensors; // Number of streams (sensors)
        int windowSize; // Time W after which we compute the average
        vector<window_stream> str_info; // This contains all the averages for each stream



};


#endif //AVERAGE_H
