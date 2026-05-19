#pragma once
#include <string>
#include <vector>

// Simulates a USB HID EAN-13 barcode scanner.
// simulateScan() cycles through a preset list of barcodes.
class BarcodeScanner {
private:
    std::string scannerId;
    std::vector<std::string> presetBarcodes;
    int scanIndex;
public:
    explicit BarcodeScanner(const std::string& id) : scannerId(id), scanIndex(0) {}

    void loadBarcodes(const std::vector<std::string>& barcodes) {
        presetBarcodes = barcodes;
        scanIndex = 0;
    }

    std::string simulateScan() {
        if (presetBarcodes.empty()) return "";
        return presetBarcodes[(scanIndex++) % (int)presetBarcodes.size()];
    }

    void resetIndex()          { scanIndex = 0; }
    std::string getScannerId() const { return scannerId; }
    int         getScanCount() const { return scanIndex; }
};
