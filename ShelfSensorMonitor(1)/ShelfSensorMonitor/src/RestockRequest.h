#pragma once
#include <string>

enum class RestockStatus { Pending, Dispatched, Acknowledged };

struct RestockRequest {
    std::string   requestId;
    std::string   shelfId;
    std::string   productBarcode;
    std::string   companyId;
    std::string   supermarketId;
    RestockStatus status;
    int           quantityDispatched;
    std::string   dispatchDate;
    std::string   acknowledgedBy;
    std::string   acknowledgedAt;
    std::string   createdAt;
    std::string   alertId;

    RestockRequest(const std::string& id, const std::string& shelf,
                   const std::string& barcode, const std::string& company,
                   const std::string& supermarket, const std::string& created,
                   const std::string& alert)
        : requestId(id), shelfId(shelf), productBarcode(barcode),
          companyId(company), supermarketId(supermarket),
          status(RestockStatus::Pending), quantityDispatched(0),
          createdAt(created), alertId(alert) {}

    bool dispatch(int qty, const std::string& date) {
        if (status != RestockStatus::Pending) return false;
        quantityDispatched = qty;
        dispatchDate = date;
        status = RestockStatus::Dispatched;
        return true;
    }

    bool acknowledge(const std::string& userId, const std::string& timestamp) {
        if (status != RestockStatus::Dispatched) return false;
        acknowledgedBy = userId;
        acknowledgedAt = timestamp;
        status = RestockStatus::Acknowledged;
        return true;
    }

    static std::string statusToString(RestockStatus s) {
        switch (s) {
            case RestockStatus::Pending:      return "Pending";
            case RestockStatus::Dispatched:   return "Dispatched";
            case RestockStatus::Acknowledged: return "Acknowledged";
        }
        return "Unknown";
    }
};
