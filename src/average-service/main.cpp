#include "main.h"

#include "average.h"
#include "../con2redis/con2redis.h"
#include "../utils/configuration.h"

int main() {

	//Initial config with window, threshold and number of streams
	config configuration = getConf();

	// Create object Average
	Average avg = Average(REDIS_SERVER, REDIS_PORT);

	// Allocate the array with all the streams
	vector<string> streams(configuration.W);
	avg.listenStreams(streams);
}
