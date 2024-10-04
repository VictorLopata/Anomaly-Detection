#include "main.h"
#include "average.h"
#include "../con2redis/con2redis.h"
#include "../utils/configuration.h"

#define REDIS_SERVER "localhost"
#define REDIS_PORT 6379

int main() {

	//Initial config with window, threshold and number of streams
	config configuration = getConf();

	// Create object Average
	Average avg = Average(REDIS_SERVER, REDIS_PORT, configuration.num_streams, configuration.W);

	// Allocate the array with all the streams
	vector<string> streams(configuration.W);
	avg.listenStreams(streams);
}
