#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>

struct LogEvent {
    std::string timestamp;
    std::string event;
    std::string barcode;
    int         quantity;
    std::string userId;
    std::string note;

    LogEvent(const std::string& ts, const std::string& ev, const std::string& bc,
             int qty, const std::string& uid, const std::string& n)
        : timestamp(ts), event(ev), barcode(bc), quantity(qty), userId(uid), note(n) {}
};

class Logger {
private:
    std::string logFilePath;
    std::vector<LogEvent> events;

    std::string ts() const {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&t);
        std::ostringstream o;
        o << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
        return o.str();
    }

public:
    explicit Logger(const std::string& path = "../shelf_log.txt") : logFilePath(path) {}

    void log(const std::string& event, const std::string& barcode, int quantity,
             const std::string& userId, const std::string& note) {
        std::string t = ts();
        events.emplace_back(t, event, barcode, quantity, userId, note);
        std::ofstream f(logFilePath, std::ios::app);
        if (f.is_open())
            f << "[" << t << "] " << event << " | bc=" << barcode
              << " | qty=" << quantity << " | user=" << userId << " | " << note << "\n";
    }

    const std::vector<LogEvent>& getEvents() const { return events; }
};
