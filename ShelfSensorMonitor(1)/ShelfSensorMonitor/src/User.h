#pragma once
#include <string>
#include <vector>

class User {
protected:
    std::string userId;
    std::string name;
    std::string email;
    std::string role;
    std::string tenantId;
public:
    User(const std::string& id, const std::string& n, const std::string& e,
         const std::string& r, const std::string& tid)
        : userId(id), name(n), email(e), role(r), tenantId(tid) {}
    virtual ~User() = default;
    virtual bool login(const std::string& credential) = 0;
    virtual std::vector<std::string> getPermissions() = 0;

    std::string getUserId()   const { return userId; }
    std::string getName()     const { return name; }
    std::string getEmail()    const { return email; }
    std::string getRole()     const { return role; }
    std::string getTenantId() const { return tenantId; }
};
