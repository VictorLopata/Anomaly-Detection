#ifndef AVERAGE_H
#define AVERAGE_H

#include <iostream>
#include "../con2redis/con2redis.h"
#include <map>

using namespace std;

class Average {
    public:
        // Constructor for connecting to Redis
        Average(const string& redis_host, const string& redis_port, int n_sensors, int windowSize);

        // Destructor for cleaning the connection
        ~Average();

        // Listen to the streams Redis
        void listenStreams(const vector<string>& streams);

        // Store the value from a stream (sensor) in the array
        void getValueStream(const string& stream, float value);

        // This method calculates average from the streams (Sensors in our project)
        double calculate_average(const vector<double>& values);

    private:
        redisContext* c; // Connection to Redis
        int n_sensors; // Number of streams (sensors)
        int windowSize; // Time W after which we compute the average
        map<string, vector<float>> values; // This contains values from each sensor in a time W
};


#endif //AVERAGE_H
