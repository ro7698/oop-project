#pragma once
#include "Product.h"
#include "WeightSensor.h"
#include "IRSensor.h"
#include <string>
#include <vector>
#include <memory>

// Owns one WeightSensor, one IRSensor, and a list of Products with stock counts.
// All stock mutations go through updateStock() to keep sensor readings in sync.
class Shelf {
private:
    std::string shelfId;
    std::string name;
    std::string supermarketId;
    std::vector<Product> products;
    std::vector<int>     stockCounts;
    std::unique_ptr<WeightSensor> weightSensor;
    std::unique_ptr<IRSensor>     irSensor;

    void syncWeight() {
        if (!weightSensor) return;
        float total = 0;
        for (size_t i = 0; i < products.size(); i++)
            total += stockCounts[i] * products[i].getWeightPerUnit();
        weightSensor->setWeight(total);
    }

public:
    Shelf(const std::string& id, const std::string& n, const std::string& smId)
        : shelfId(id), name(n), supermarketId(smId) {}

    void attachWeightSensor(std::unique_ptr<WeightSensor> ws) { weightSensor = std::move(ws); }
    void attachIRSensor    (std::unique_ptr<IRSensor>     ir) { irSensor     = std::move(ir); }

    void addProduct(const Product& p, int initialStock) {
        products.push_back(p);
        stockCounts.push_back(initialStock >= 0 ? initialStock : 0);
        syncWeight();
    }

    int getStockCount(const std::string& barcode) const {
        for (size_t i = 0; i < products.size(); i++)
            if (products[i].getBarcode() == barcode) return stockCounts[i];
        return -1;
    }

    bool updateStock(const std::string& barcode, int delta) {
        for (size_t i = 0; i < products.size(); i++) {
            if (products[i].getBarcode() == barcode) {
                stockCounts[i] = std::max(0, stockCounts[i] + delta);
                syncWeight();
                return true;
            }
        }
        return false;
    }

    float getWeightReading()  const { return weightSensor ? weightSensor->readValue() : 0; }
    float getIRReading()      const { return irSensor     ? irSensor->readValue()     : 0; }
    bool  areSensorsHealthy() const {
        return (weightSensor && weightSensor->isHealthy()) &&
               (irSensor     && irSensor->isHealthy());
    }

    std::string getShelfId()       const { return shelfId; }
    std::string getName()          const { return name; }
    std::string getSupermarketId() const { return supermarketId; }
    const std::vector<Product>& getProducts()    const { return products; }
    const std::vector<int>&     getStockCounts() const { return stockCounts; }
};
