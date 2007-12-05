/**
 * @file exchange_monitor.hpp
 * @brief C++ real-time telemetry and subscriber monitor for telecom exchange.
 * @author Yousef Khaled (Telecom Systems Engineering - 2004)
 */

#ifndef TELECOM_EXCHANGE_MONITOR_HPP
#define TELECOM_EXCHANGE_MONITOR_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <memory>
#include <iostream>

namespace telecom {

struct SubscriberSession {
    std::string msisdn;
    std::string imsi;
    uint32_t current_lac;
    uint32_t cell_id;
    bool is_in_call;
    std::chrono::system_clock::time_point last_activity;
};

class ExchangeMonitor {
public:
    ExchangeMonitor(const std::string& exchange_id, uint32_t capacity);
    ~ExchangeMonitor() = default;

    bool register_subscriber(const std::string& msisdn, const std::string& imsi, uint32_t lac, uint32_t cell);
    bool update_call_state(const std::string& msisdn, bool in_call);
    void print_active_subscribers() const;
    size_t get_active_count() const { return sessions_.size(); }

private:
    std::string exchange_id_;
    uint32_t max_capacity_;
    std::unordered_map<std::string, SubscriberSession> sessions_;
};

} // namespace telecom

#endif // TELECOM_EXCHANGE_MONITOR_HPP
