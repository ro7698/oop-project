#pragma once
#include <string>

class Tenant {
protected:
    std::string tenantId;
    std::string name;
public:
    Tenant(const std::string& id, const std::string& n) : tenantId(id), name(n) {}
    virtual ~Tenant() = default;
    virtual std::string getTenantType() const = 0;
    std::string getTenantId() const { return tenantId; }
    std::string getName()     const { return name; }
};
