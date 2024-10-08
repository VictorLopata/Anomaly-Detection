#ifndef AVERAGE_H
#define AVERAGE_H

#include <iostream>
#include "../con2redis/con2redis.h"
#include <map>

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
        void calculate_averages();


    private:
        redisContext* c; // Connection to Redis
        vector<string> streams; // Stream names of the stream I am listening
        map<string, string> lastIDs; // Last ID stored for XREAD

        int n_sensors; // Number of streams (sensors)
        int windowSize; // Time W after which we compute the average
        vector<double> values; // This contains all the averages for each stream
        int count = 0; //  This count the number of values which I accept during the window W


        // This method flush the previous values of the stream in order to accept the new ones
        void cleanVectors();

};


#endif //AVERAGE_H
