#include "main.h"
#include "average.h"
#include "../con2redis/con2redis.h"
#include "../utils/configuration.h"

#define REDIS_SERVER "localhost"
#define REDIS_PORT 6379

int main() {

	//Initial config with window, threshold and number of streams

	// Create object Average
	Average avg = Average();

	avg.listenStreams();

	return 0;
}
