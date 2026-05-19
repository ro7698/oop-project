#pragma once
#include <string>

class Product {
private:
    std::string barcode;
    std::string sku;
    std::string name;
    int         threshold;
    std::string expiryDate;    // "YYYY-MM-DD" or "" if non-perishable
    std::string companyId;
    bool        perishable;
    float       weightPerUnit; // grams
public:
    Product(const std::string& bc, const std::string& s, const std::string& n,
            int thresh, const std::string& expiry, const std::string& cid,
            bool perish, float wpUnit)
        : barcode(bc), sku(s), name(n), threshold(thresh), expiryDate(expiry),
          companyId(cid), perishable(perish), weightPerUnit(wpUnit) {}

    std::string getBarcode()       const { return barcode; }
    std::string getSku()           const { return sku; }
    std::string getName()          const { return name; }
    int         getThreshold()     const { return threshold; }
    std::string getExpiryDate()    const { return expiryDate; }
    std::string getCompanyId()     const { return companyId; }
    bool        isPerishable()     const { return perishable; }
    float       getWeightPerUnit() const { return weightPerUnit; }

    void setThreshold(int t)                 { threshold   = t; }
    void setExpiryDate(const std::string& d) { expiryDate  = d; }
};
