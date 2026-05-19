#pragma once
#include "Tenant.h"
#include "Shelf.h"
#include <vector>
#include <memory>

class Supermarket : public Tenant {
private:
    std::string location;
    std::vector<std::shared_ptr<Shelf>> shelves;
public:
    Supermarket(const std::string& id, const std::string& n, const std::string& loc)
        : Tenant(id, n), location(loc) {}

    std::string getTenantType() const override { return "supermarket"; }
    void addShelf(std::shared_ptr<Shelf> s)    { shelves.push_back(s); }

    const std::vector<std::shared_ptr<Shelf>>& getShelves() const { return shelves; }
    std::string getLocation() const { return location; }
};
