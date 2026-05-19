#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

struct Alert {
    std::string id;
    std::string type;           // low_stock | expiry_warning | expiry_urgent | expiry_critical
    std::string shelfId;
    std::string productBarcode;
    std::string companyId;
    std::string message;
    std::string timestamp;
    bool acknowledgedBySupermarket;
    bool acknowledgedByCompany;

    Alert(const std::string& i, const std::string& t, const std::string& sh,
          const std::string& pb, const std::string& cid,
          const std::string& msg, const std::string& ts)
        : id(i), type(t), shelfId(sh), productBarcode(pb), companyId(cid),
          message(msg), timestamp(ts),
          acknowledgedBySupermarket(false), acknowledgedByCompany(false) {}
};

class AlertManager {
private:
    std::vector<Alert> alerts;
    int alertCounter;

    std::string ts() const {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&t);
        std::ostringstream o;
        o << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
        return o.str();
    }

public:
    AlertManager() : alertCounter(0) {}

    std::string fireAlert(const std::string& type, const std::string& shelfId,
                          const std::string& barcode, const std::string& companyId,
                          const std::string& message) {
        std::string id = "ALT" + std::to_string(++alertCounter);
        alerts.emplace_back(id, type, shelfId, barcode, companyId, message, ts());
        return id;
    }

    const std::vector<Alert>& getAlerts() const { return alerts; }

    std::vector<Alert> getAlertsForCompany(const std::string& companyId) const {
        std::vector<Alert> r;
        for (const auto& a : alerts)
            if (a.companyId == companyId) r.push_back(a);
        return r;
    }

    bool acknowledgeAlert(const std::string& alertId, bool bySupermarket) {
        for (auto& a : alerts) {
            if (a.id == alertId) {
                if (bySupermarket) a.acknowledgedBySupermarket = true;
                else               a.acknowledgedByCompany     = true;
                return true;
            }
        }
        return false;
    }
};
