#include <iostream>
#include <deque>
#include <thread>
#include <numeric>
#include <chrono>
#include <hiredis/hiredis.h>

// 计算协方差
double covariance(const std::deque<double>& x, const std::deque<double>& y) {
    double mean_x = std::accumulate(x.begin(), x.end(), 0.0) / x.size();
    double mean_y = std::accumulate(y.begin(), y.end(), 0.0) / y.size();
    double cov = 0.0;

    for (size_t i = 0; i < x.size(); ++i) {
        cov += (x[i] - mean_x) * (y[i] - mean_y);
    }

    return cov / x.size();
}

// 从 Redis 获取时间单位 w
int get_time_window(redisContext* c) {
    redisReply* reply = (redisReply*)redisCommand(c, "GET time_window");
    int w = (reply && reply->type == REDIS_REPLY_STRING) ? std::stoi(reply->str) : 5; // 默认 5 秒
    freeReplyObject(reply);
    return w;
}

// 从 Redis 获取数据集数量 n
int get_dataset_size(redisContext* c) {
    redisReply* reply = (redisReply*)redisCommand(c, "GET dataset_size");
    int n = (reply && reply->type == REDIS_REPLY_STRING) ? std::stoi(reply->str) : 2; // 默认 2 个数据集
    freeReplyObject(reply);
    return n;
}

int main() {
    // 连接 Redis 服务器
    redisContext* c = redisConnect("127.0.0.1", 6379);
    if (c == NULL || c->err) {
        std::cerr << "Redis connection error: " << (c ? c->errstr : "NULL") << std::endl;
        return 1;
    }

    int batch_count = 0; // 记录当前计算的批次

    while (true) {
        // 从 Redis 获取时间单位 w 和数据集大小 n
        int w = get_time_window(c);
        int n = get_dataset_size(c);
        std::cout << "Time window (seconds): " << w << " | Dataset size: " << n << std::endl;

        // 从 Redis Stream 中获取 n 个数据集
        redisReply* reply = (redisReply*)redisCommand(c, "XREAD BLOCK 5000 STREAMS dataset_stream $ COUNT %d", n);
        if (reply->type == REDIS_REPLY_ARRAY) {
            std::deque<double> window_x;
            std::deque<double> window_y;

            for (size_t i = 0; i < reply->elements; ++i) {
                redisReply* stream = reply->element[i];
                if (stream->type == REDIS_REPLY_ARRAY) {
                    redisReply* entries = stream->element[1];
                    for (size_t j = 0; j < entries->elements; ++j) {
                        redisReply* entry = entries->element[j];
                        double data_x = std::stod(entry->element[1]->element[1]->str);
                        double data_y = std::stod(entry->element[1]->element[3]->str);

                        window_x.push_back(data_x);
                        window_y.push_back(data_y);

                        // 保持窗口大小为 n
                        if (window_x.size() == n && window_y.size() == n) {
                            // 计算协方差
                            double cov = covariance(window_x, window_y);
                            std::cout << "Covariance: " << cov << std::endl;

                            // 发送结果到 Redis 通道
                            std::string channel = "c#" + std::to_string(batch_count);
                            redisReply* push_reply = (redisReply*)redisCommand(c, "XADD %s * cov %f", channel.c_str(), cov);
                            freeReplyObject(push_reply);

                            batch_count++;  // 更新批次编号

                            // 清空窗口，开始下一批次计算
                            window_x.clear();
                            window_y.clear();
                        }
                    }
                }
            }
        }

        freeReplyObject(reply);

        // 等待 w 秒后进行下一轮计算
        std::this_thread::sleep_for(std::chrono::seconds(w));
    }

    // 关闭 Redis 连接
    redisFree(c);

    return 0;
}