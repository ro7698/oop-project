#pragma once
#include <string>

class Sensor {
protected:
    std::string sensorId;
    std::string shelfId;
    bool healthy;
public:
    Sensor(const std::string& id, const std::string& shelf)
        : sensorId(id), shelfId(shelf), healthy(true) {}
    virtual ~Sensor() = default;
    virtual float readValue() = 0;
    virtual bool  isHealthy() = 0;
    std::string getSensorId() const { return sensorId; }
    std::string getShelfId()  const { return shelfId; }
};
