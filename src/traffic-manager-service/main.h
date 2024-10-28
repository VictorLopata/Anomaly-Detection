#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <thread>
#include <numeric>
#include "../con2redis/con2redis.h"
#include <iomanip>
#include <sstream>
#include <utils/utils.h>
#include <utils/stream.h>

using namespace std;
using namespace chrono;


#ifndef ANOMALY_DETECTION_MAIN_H
#define ANOMALY_DETECTION_MAIN_H

#endif //ANOMALY_DETECTION_MAIN_H


void send_streams(redisContext *c, unordered_map<int, std::vector<double> >& streams_data, string startTimestamp, string endTimestamp, int n) {
    for (auto it1 = streams_data.begin(); it1 != streams_data.end(); ++it1) {
        string strName = "str#" + to_string(it1->first);

        for (const auto& val : it1->second) {

            string comm = "XADD " + strName + " * val " + to_string(val) + " startTimestamp " + startTimestamp + " endTimestamp " + endTimestamp;
            cout << comm << endl;
            redisReply* reply = (redisReply *)redisCommand(c, comm.c_str());
            freeReplyObject(reply);
        }


    }
    string comm = "XADD end * val ok";
    cout << comm << endl;
    redisReply* reply = (redisReply *)redisCommand(c, comm.c_str());
    freeReplyObject(reply);
}