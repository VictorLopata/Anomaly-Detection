#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>
#include <deque>
#include <hiredis/hiredis.h>

// 模拟从 Redis 中获取数据
std::pair<double, double> fetch_data_from_redis(redisContext* c) {
    double x_value = (double)(rand() % 100);  // 模拟从 Redis 获取的数据
    double y_value = (double)(rand() % 100);  // 模拟从 Redis 获取的数据
    return {x_value, y_value};
}

// 从 Redis 获取时间单位
int get_time_unit_from_redis(redisContext* c) {
    redisReply* reply = (redisReply*)redisCommand(c, "GET window_time_unit");
    if (reply->type == REDIS_REPLY_STRING) {
        int time_unit = std::stoi(reply->str);  // 转换为 int
        freeReplyObject(reply);
        return time_unit;
    } else {
        freeReplyObject(reply);
        throw std::runtime_error("Failed to retrieve time unit from Redis");
    }
}

// 计算窗口内的均值
double mean(const std::deque<double>& data) {
    double sum = 0.0;
    for (const auto& value : data) {
        sum += value;
    }
    return sum / data.size();
}

// 计算窗口内的协方差
double covariance(const std::deque<double>& x, const std::deque<double>& y) {
    double mean_x = mean(x);
    double mean_y = mean(y);
    double cov = 0.0;
    
    for (size_t i = 0; i < x.size(); ++i) {
        cov += (x[i] - mean_x) * (y[i] - mean_y);
    }
    
    return cov / x.size();
}

// 从 Redis 获取阈值
double get_threshold_from_redis(redisContext* c) {
    redisReply* reply = (redisReply*)redisCommand(c, "GET covariance_threshold");
    if (reply->type == REDIS_REPLY_STRING) {
        double threshold = std::stod(reply->str);  // 转换为 double
        freeReplyObject(reply);
        return threshold;
    } else {
        freeReplyObject(reply);
        throw std::runtime_error("Failed to retrieve threshold from Redis");
    }
}

// 使用历史均值和标准差来检测异常
bool is_covariance_anomalous_zscore(double cov, double historical_mean, double historical_std_dev, double zscore_threshold) {
    double z_score = (cov - historical_mean) / historical_std_dev;
    return std::abs(z_score) > zscore_threshold;
}

int main() {
    // 连接 Redis 服务器
    redisContext* c = redisConnect("127.0.0.1", 6379);
    if (c == NULL || c->err) {
        std::cerr << "Redis connection error: " << (c ? c->errstr : "NULL") << std::endl;
        return 1;
    }

    // 假设的历史均值和标准差 (实际应用中应基于历史数据计算)
    double historical_mean = 50.0;  // 假设历史协方差均值
    double historical_std_dev = 10.0;  // 假设历史协方差标准差

    // 模拟连续的数据流
    for (int t = 0; t < 100; ++t) {  // 假设处理 100 秒的数据流
        // 每秒从 Redis 获取数据
        std::pair<double, double> new_data = fetch_data_from_redis(c);

        // 获取当前窗口时间单位 w
        int window_time_unit = get_time_unit_from_redis(c);

        // 创建滑动窗口
        static std::deque<double> window_x, window_y;
        window_x.push_back(new_data.first);
        window_y.push_back(new_data.second);

        // 如果窗口大小超过 w 秒，移除最早的数据
        if (window_x.size() > window_time_unit) {
            window_x.pop_front();
            window_y.pop_front();
        }

        // 每 w 秒计算协方差
        if (t % window_time_unit == 0 && window_x.size() == window_time_unit) {
            double cov = covariance(window_x, window_y);
            std::cout << "Covariance at time " << t << ": " << cov << std::endl;

            // 从 Redis 获取当前阈值
            double zscore_threshold = get_threshold_from_redis(c);

            // 检测是否存在异常
            if (is_covariance_anomalous_zscore(cov, historical_mean, historical_std_dev, zscore_threshold)) {
                std::cout << "Anomaly detected at time " << t << "! Covariance: " << cov << " (Threshold: " << zscore_threshold << ")" << std::endl;
            } else {
                std::cout << "Covariance is normal at time " << t << std::endl;
            }
        }

        // 模拟 1 秒的数据传输延迟
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // 关闭 Redis 连接
    redisFree(c);

    return 0;
}