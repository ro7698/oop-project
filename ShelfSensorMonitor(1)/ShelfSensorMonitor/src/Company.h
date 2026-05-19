#pragma once
#include "Tenant.h"
#include <vector>
#include <algorithm>

class Company : public Tenant {
private:
    std::string contactEmail;
    std::vector<std::string> productBarcodes;
public:
    Company(const std::string& id, const std::string& n, const std::string& email)
        : Tenant(id, n), contactEmail(email) {}

    std::string getTenantType() const override { return "company"; }

    void registerProduct(const std::string& barcode) { productBarcodes.push_back(barcode); }

    bool ownsProduct(const std::string& barcode) const {
        return std::find(productBarcodes.begin(), productBarcodes.end(), barcode)
               != productBarcodes.end();
    }

    const std::vector<std::string>& getProductBarcodes() const { return productBarcodes; }
    std::string getContactEmail() const { return contactEmail; }
};
