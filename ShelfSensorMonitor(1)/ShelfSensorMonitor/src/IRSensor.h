#pragma once
#include "Sensor.h"

// Simulates a Sharp GP2Y0A IR proximity sensor (10–80 cm).
// Low distance = items present. High distance = shelf sparse.
class IRSensor : public Sensor {
private:
    float distance;
    float emptyThreshold;
public:
    IRSensor(const std::string& id, const std::string& shelf, float emptyThresh = 25.0f)
        : Sensor(id, shelf), distance(5.0f), emptyThreshold(emptyThresh) {}

    void  setDistance(float d)    { distance = (d < 0) ? 0 : d; }
    float readValue()  override   { return distance; }
    bool  isHealthy()  override   { return healthy; }
    bool  isEmpty()    const      { return distance > emptyThreshold; }
    float getEmptyThreshold()const{ return emptyThreshold; }
};
