#pragma once
#include "Shelf.h"
#include "AlertManager.h"
#include "Logger.h"
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

// Tiered expiry alerts (F-05):
//   past expiry → critical | today → urgent | 1-3 days → warning | 3+ → none
class ExpiryMonitor {
private:
    AlertManager& alertManager;
    Logger&        logger;

    int daysUntil(const std::string& dateStr) const {
        if (dateStr.empty()) return 999;
        std::tm exp = {};
        std::istringstream ss(dateStr);
        ss >> std::get_time(&exp, "%Y-%m-%d");
        exp.tm_hour = 0; exp.tm_min = 0; exp.tm_sec = 0;
        auto now = std::chrono::system_clock::now();
        std::time_t nowT = std::chrono::system_clock::to_time_t(now);
        std::tm nowTm = *std::localtime(&nowT);
        nowTm.tm_hour = 0; nowTm.tm_min = 0; nowTm.tm_sec = 0;
        return (int)(std::difftime(std::mktime(&exp), std::mktime(&nowTm)) / 86400.0);
    }

public:
    ExpiryMonitor(AlertManager& am, Logger& log) : alertManager(am), logger(log) {}

    void runCheck(const std::vector<std::shared_ptr<Shelf>>& shelves) {
        for (const auto& shelf : shelves) {
            for (const auto& p : shelf->getProducts()) {
                if (!p.isPerishable()) continue;
                int d = daysUntil(p.getExpiryDate());
                std::string bc = p.getBarcode(), cid = p.getCompanyId(), sid = shelf->getShelfId();
                if (d < 0) {
                    std::string m = "CRITICAL — "+p.getName()+" has EXPIRED. Remove immediately.";
                    alertManager.fireAlert("expiry_critical",sid,bc,cid,m);
                    logger.log("EXPIRY_CRITICAL",bc,0,"SYSTEM",m);
                } else if (d == 0) {
                    std::string m = "URGENT — "+p.getName()+" expires TODAY. Remove or act now.";
                    alertManager.fireAlert("expiry_urgent",sid,bc,cid,m);
                    logger.log("EXPIRY_URGENT",bc,0,"SYSTEM",m);
                } else if (d <= 3) {
                    std::string m = "WARNING — "+p.getName()+" expires in "+std::to_string(d)+" day(s). Consider promotion.";
                    alertManager.fireAlert("expiry_warning",sid,bc,cid,m);
                    logger.log("EXPIRY_WARNING",bc,d,"SYSTEM",m);
                }
            }
        }
    }
};
