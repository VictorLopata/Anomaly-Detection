#include <iostream>
#include <deque>
#include <thread>
#include <numeric>
#include <chrono>
#include <hiredis/hiredis.h>

// 计算窗口内的协方差
double covariance(const std::deque<double>& x, const std::deque<double>& y) {
    double mean_x = std::accumulate(x.begin(), x.end(), 0.0) / x.size();
    double mean_y = std::accumulate(y.begin(), y.end(), 0.0) / y.size();
    double cov = 0.0;
    
    for (size_t i = 0; i < x.size(); ++i) {
        cov += (x[i] - mean_x) * (y[i] - mean_y);
    }
    
    return cov / x.size();
}

// 从 Redis 获取时间窗口 w
int get_time_window(redisContext* c) {
    redisReply* reply = (redisReply*)redisCommand(c, "GET time_window");
    int w = (reply && reply->type == REDIS_REPLY_STRING) ? std::stoi(reply->str) : 5; // 默认 5 秒
    freeReplyObject(reply);
    return w;
}

int main() {
    // 连接 Redis 服务器
    redisContext* c = redisConnect("127.0.0.1", 6379);
    if (c == NULL || c->err) {
        std::cerr << "Redis connection error: " << (c ? c->errstr : "NULL") << std::endl;
        return 1;
    }

    // 从 Redis 获取时间窗口 w
    int w = get_time_window(c);
    std::cout << "Time window (seconds): " << w << std::endl;

    std::deque<double> window_x;
    std::deque<double> window_y;

    while (true) {
        // 从 Redis Stream 中获取数据集
        redisReply* reply = (redisReply*)redisCommand(c, "XREAD BLOCK 5000 STREAMS dataset_stream $");
        if (reply->type == REDIS_REPLY_ARRAY) {
            for (size_t i = 0; i < reply->elements; ++i) {
                redisReply* stream = reply->element[i];
                if (stream->type == REDIS_REPLY_ARRAY) {
                    redisReply* entries = stream->element[1];
                    for (size_t j = 0; j < entries->elements; ++j) {
                        redisReply* entry = entries->element[j];
                        double data_x = std::stod(entry->element[1]->element[1]->str);
                        double data_y = std::stod(entry->element[1]->element[3]->str);

                        // 添加数据到窗口
                        window_x.push_back(data_x);
                        window_y.push_back(data_y);

                        if (window_x.size() > w) {
                            window_x.pop_front();
                            window_y.pop_front();
                        }

                        if (window_x.size() == w) {
                            // 计算协方差
                            double cov = covariance(window_x, window_y);
                            std::cout << "Covariance: " << cov << std::endl;

                            // 将协方差推送到 Redis Stream
                            redisReply* push_reply = (redisReply*)redisCommand(c, "XADD covariance_stream * cov %f", cov);
                            freeReplyObject(push_reply);
                        }
                    }
                }
            }
        }

        freeReplyObject(reply);

        // 按时间单位 w 等待
        std::this_thread::sleep_for(std::chrono::seconds(w));
        w = get_time_window(c);  // 动态获取更新的时间窗口
    }

    // 关闭 Redis 连接
    redisFree(c);

    return 0;
}