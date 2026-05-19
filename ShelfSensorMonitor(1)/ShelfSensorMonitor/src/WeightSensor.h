#pragma once
#include "Sensor.h"
#include <cstdlib>

// Simulates a load cell + HX711 amplifier.
// Reports total shelf weight in grams with ±1g noise.
class WeightSensor : public Sensor {
private:
    float currentWeight;
    float maxCapacity;
public:
    WeightSensor(const std::string& id, const std::string& shelf, float maxCap)
        : Sensor(id, shelf), currentWeight(maxCap), maxCapacity(maxCap) {}

    void setWeight(float w) { currentWeight = (w < 0) ? 0 : w; }

    float readValue() override {
        float noise = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        return currentWeight + noise;
    }
    bool isHealthy() override { return healthy && currentWeight >= 0; }

    float getMaxCapacity()   const { return maxCapacity; }
    float getCurrentWeight() const { return currentWeight; }
};
