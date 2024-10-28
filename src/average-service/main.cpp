#include "main.h"
#include "average.h"
#include "../con2redis/con2redis.h"
#include "../utils/configuration.h"

#define REDIS_SERVER "localhost"
#define REDIS_PORT 6379

int main() {

	// Create object Average
	Average avg = Average();

    // Listen to stream from the average class
	avg.listenStreams();

	return 0;
}
