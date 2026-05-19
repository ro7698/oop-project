#pragma once
#include "Shelf.h"
#include "AlertManager.h"
#include "Logger.h"
#include <vector>
#include <memory>
#include <string>

struct LowStockDetection {
    std::string shelfId;
    std::string productBarcode;
    std::string companyId;
    int         currentStock;
    int         threshold;
};

// Polls all shelves and fuses weight + IR data.
// Alert fires when stock <= threshold AND at least one sensor confirms (F-01, F-02).
// Holds vector<Sensor*> indirectly through Shelf — polymorphism via virtual readValue().
class StockMonitor {
private:
    std::vector<std::shared_ptr<Shelf>> shelves;
    AlertManager& alertManager;
    Logger&        logger;
    static constexpr float IR_SPARSE_CM = 25.0f;

public:
    StockMonitor(AlertManager& am, Logger& log) : alertManager(am), logger(log) {}

    void addShelf(std::shared_ptr<Shelf> s) { shelves.push_back(s); }

    std::vector<LowStockDetection> poll() {
        std::vector<LowStockDetection> detections;
        for (const auto& shelf : shelves) {
            if (!shelf->areSensorsHealthy()) {
                logger.log("SENSOR_FAULT","",0,"SYSTEM","Fault on shelf "+shelf->getShelfId());
                continue;
            }
            bool irConfirms = shelf->getIRReading() > IR_SPARSE_CM;
            const auto& prods  = shelf->getProducts();
            const auto& stocks = shelf->getStockCounts();
            for (size_t i = 0; i < prods.size(); i++) {
                if (stocks[i] > prods[i].getThreshold()) continue;
                if (!irConfirms) continue; // need at least IR to confirm
                std::string msg = "Low stock: " + prods[i].getName()
                    + " (" + std::to_string(stocks[i]) + " units remaining on "
                    + shelf->getName() + ")";
                alertManager.fireAlert("low_stock", shelf->getShelfId(),
                                       prods[i].getBarcode(), prods[i].getCompanyId(), msg);
                logger.log("ALERT_FIRED", prods[i].getBarcode(), stocks[i], "SYSTEM", msg);
                detections.push_back({shelf->getShelfId(), prods[i].getBarcode(),
                                      prods[i].getCompanyId(), stocks[i], prods[i].getThreshold()});
            }
        }
        return detections;
    }
};
