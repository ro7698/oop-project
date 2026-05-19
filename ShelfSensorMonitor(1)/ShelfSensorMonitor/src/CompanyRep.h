#pragma once
#include "User.h"

class CompanyRep : public User {
public:
    CompanyRep(const std::string& id, const std::string& name,
               const std::string& email, const std::string& companyId)
        : User(id, name, email, "CompanyRep", companyId) {}

    bool login(const std::string& credential) override { return !credential.empty(); }

    std::vector<std::string> getPermissions() override {
        return {"view_own_products","view_own_alerts","dispatch_restock","set_threshold"};
    }
};
