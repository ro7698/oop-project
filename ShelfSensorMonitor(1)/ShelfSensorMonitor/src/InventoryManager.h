#pragma once
#include "Shelf.h"
#include "Logger.h"
#include "RestockRequest.h"
#include "AlertManager.h"
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

// All stock mutations go through here — never modify stock directly (NF-08).
class InventoryManager {
private:
    std::vector<std::shared_ptr<Shelf>>& shelves;
    Logger&        logger;
    AlertManager&  alertManager;
    std::vector<RestockRequest> restockRequests;
    int requestCounter;

    std::string ts() const {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&t);
        std::ostringstream o;
        o << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
        return o.str();
    }

    std::shared_ptr<Shelf> findShelf(const std::string& barcode) {
        for (auto& s : shelves)
            if (s->getStockCount(barcode) >= 0) return s;
        return nullptr;
    }

public:
    InventoryManager(std::vector<std::shared_ptr<Shelf>>& sh, Logger& log, AlertManager& am)
        : shelves(sh), logger(log), alertManager(am), requestCounter(0) {}

    bool processSale(const std::string& barcode, int qty, const std::string& userId) {
        auto s = findShelf(barcode);
        if (!s) { logger.log("SALE_FAILED",barcode,qty,userId,"Product not found"); return false; }
        bool ok = s->updateStock(barcode, -qty);
        if (ok) logger.log("SALE",barcode,qty,userId,"Sale on shelf "+s->getShelfId());
        return ok;
    }

    bool processRestock(const std::string& barcode, int qty,
                        const std::string& userId, const std::string& requestId = "") {
        auto s = findShelf(barcode);
        if (!s) { logger.log("RESTOCK_FAILED",barcode,qty,userId,"Product not found"); return false; }
        bool ok = s->updateStock(barcode, qty);
        if (ok) {
            logger.log("RESTOCK",barcode,qty,userId,"Restock on shelf "+s->getShelfId()+". ReqId="+requestId);
            if (!requestId.empty()) acknowledgeRestock(requestId, userId);
        }
        return ok;
    }

    bool processExpiredRemoval(const std::string& barcode, int qty, const std::string& userId) {
        auto s = findShelf(barcode);
        if (!s) return false;
        bool ok = s->updateStock(barcode, -qty);
        if (ok) logger.log("EXPIRY_REMOVAL",barcode,qty,userId,"Removed from "+s->getShelfId());
        return ok;
    }

    std::string createRestockRequest(const std::string& shelfId, const std::string& barcode,
                                     const std::string& companyId, const std::string& supermarketId,
                                     const std::string& alertId) {
        std::ostringstream o;
        o << "RR" << std::setw(3) << std::setfill('0') << ++requestCounter;
        std::string id = o.str();
        restockRequests.emplace_back(id, shelfId, barcode, companyId, supermarketId, ts(), alertId);
        logger.log("RESTOCK_REQUEST_CREATED",barcode,0,"SYSTEM","Request "+id+" for shelf "+shelfId);
        return id;
    }

    bool dispatchRestock(const std::string& requestId, int qty,
                         const std::string& date, const std::string& userId) {
        for (auto& r : restockRequests) {
            if (r.requestId == requestId) {
                bool ok = r.dispatch(qty, date);
                if (ok) logger.log("RESTOCK_DISPATCH",r.productBarcode,qty,userId,
                                   "Dispatched "+requestId+". ETA: "+date);
                return ok;
            }
        }
        return false;
    }

    bool acknowledgeRestock(const std::string& requestId, const std::string& userId) {
        for (auto& r : restockRequests) {
            if (r.requestId == requestId) {
                bool ok = r.acknowledge(userId, ts());
                if (ok) logger.log("RESTOCK_ACK",r.productBarcode,r.quantityDispatched,
                                   userId,"Acknowledged "+requestId);
                return ok;
            }
        }
        return false;
    }

    const std::vector<RestockRequest>& getRestockRequests() const { return restockRequests; }
};
