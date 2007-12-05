/**
 * @file exchange_monitor.cpp
 * @brief Implementation of C++ real-time telemetry and subscriber monitor.
 * @author Yousef Khaled (Telecom Systems Engineering - 2004)
 */

#include "exchange_monitor.hpp"
#include <iomanip>

namespace telecom {

ExchangeMonitor::ExchangeMonitor(const std::string& exchange_id, uint32_t capacity)
    : exchange_id_(exchange_id), max_capacity_(capacity) {}

bool ExchangeMonitor::register_subscriber(const std::string& msisdn, const std::string& imsi, uint32_t lac, uint32_t cell) {
    if (sessions_.size() >= max_capacity_) return false;

    SubscriberSession session;
    session.msisdn = msisdn;
    session.imsi = imsi;
    session.current_lac = lac;
    session.cell_id = cell;
    session.is_in_call = false;
    session.last_activity = std::chrono::system_clock::now();

    sessions_[msisdn] = session;
    return true;
}

bool ExchangeMonitor::update_call_state(const std::string& msisdn, bool in_call) {
    auto it = sessions_.find(msisdn);
    if (it == sessions_.end()) return false;

    it->second.is_in_call = in_call;
    it->second.last_activity = std::chrono::system_clock::now();
    return true;
}

void ExchangeMonitor::print_active_subscribers() const {
    std::cout << "\n--- Active Subscriber Sessions (" << exchange_id_ << ") ---\n";
    std::cout << std::left << std::setw(18) << "MSISDN" 
              << std::setw(18) << "IMSI" 
              << std::setw(10) << "LAC" 
              << std::setw(10) << "CELL" 
              << "CALL STATE\n";
    std::cout << std::string(65, '-') << "\n";

    for (const auto& [msisdn, s] : sessions_) {
        std::cout << std::left << std::setw(18) << s.msisdn
                  << std::setw(18) << s.imsi
                  << std::setw(10) << s.current_lac
                  << std::setw(10) << s.cell_id
                  << (s.is_in_call ? "IN_CALL" : "STANDBY") << "\n";
    }
    std::cout << "-----------------------------------------------------------------\n";
}

} // namespace telecom
