#ifndef AVERAGE_H
#define AVERAGE_H

#include <iostream>
#include "../con2redis/con2redis.h"

using namespace std;

class Average {
    public:
        // Constructor for connecting to Redis
        Average(const string& redis_host, const string& redis_port);

        // Destructor for cleaning the connection
        ~Average();

        void listenStreams(const vector<string>& streams);

        // This method calculates average from the streams (Sensors in our project)
        double calculate_average(const vector<double>& values);

    private:
        RedisContext* c; // Connection to Redis
        vector<double> values; //This contains the values i store from a window W
};


#endif //AVERAGE_H
