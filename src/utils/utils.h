#include <chrono>

#ifndef UTILS_H
#define UTILS_H
#define REDIS_SERVER "localhost"
#define REDIS_PORT 6379

#endif //UTILS_H


string getCurrentTimestamp() {
	auto now = system_clock::now();
	auto in_time_t = system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S"); // Formattato come stringa leggibile
	return ss.str();
}

