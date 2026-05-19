#pragma once
#include "User.h"

class SupermarketManager : public User {
public:
    SupermarketManager(const std::string& id, const std::string& name,
                       const std::string& email, const std::string& supermarketId)
        : User(id, name, email, "SupermarketManager", supermarketId) {}

    bool login(const std::string& credential) override { return !credential.empty(); }

    std::vector<std::string> getPermissions() override {
        return {"view_all_shelves","view_all_alerts","acknowledge_restock",
                "remove_expired","view_event_log"};
    }
};
