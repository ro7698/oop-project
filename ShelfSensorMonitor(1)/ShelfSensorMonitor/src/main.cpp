// =============================================================================
//  Supermarket Shelf Sensor Monitor
//  Compile:  g++ -std=c++17 -o ShelfSensorMonitor main.cpp
//  Run:      ./ShelfSensorMonitor        writes ../dashboard/data.json
// =============================================================================
#include "Sensor.h"
#include "WeightSensor.h"
#include "IRSensor.h"
#include "BarcodeScanner.h"
#include "Product.h"
#include "Shelf.h"
#include "Tenant.h"
#include "Supermarket.h"
#include "Company.h"
#include "User.h"
#include "SupermarketManager.h"
#include "CompanyRep.h"
#include "AlertManager.h"
#include "Logger.h"
#include "RestockRequest.h"
#include "StockMonitor.h"
#include "InventoryManager.h"
#include "ExpiryMonitor.h"

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cstdlib>

// ── Helpers ──────────────────────────────────────────────────────────────────

std::string nowISO() {
    auto tp = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&t);
    std::ostringstream o;
    o << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return o.str();
}

std::string jstr(const std::string& s) {
    std::string r;
    for (char c : s) {
        if      (c == '"')  r += "\\\"";
        else if (c == '\\') r += "\\\\";
        else if (c == '\n') r += "\\n";
        else                r += c;
    }
    return r;
}

int daysUntil(const std::string& d) {
    if (d.empty()) return 999;
    std::tm exp = {};
    std::istringstream ss(d);
    ss >> std::get_time(&exp, "%Y-%m-%d");
    exp.tm_hour = 0; exp.tm_min = 0; exp.tm_sec = 0;
    auto now = std::chrono::system_clock::now();
    std::time_t nowT = std::chrono::system_clock::to_time_t(now);
    std::tm nowTm = *std::localtime(&nowT);
    nowTm.tm_hour = 0; nowTm.tm_min = 0; nowTm.tm_sec = 0;
    return (int)(std::difftime(std::mktime(&exp), std::mktime(&nowTm)) / 86400.0);
}

std::string stockStatus(int s, int t) {
    if (s == 0)    return "critical";
    if (s <= t)    return "low";
    return "ok";
}

std::string expiryStatus(const std::string& expiry, bool perishable) {
    if (!perishable || expiry.empty()) return "ok";
    int d = daysUntil(expiry);
    if (d < 0)  return "critical";
    if (d == 0) return "urgent";
    if (d <= 3) return "warning";
    return "ok";
}

// ── JSON Export ──────────────────────────────────────────────────────────────

void exportJSON(const std::string& genAt,
                const Supermarket& sm,
                const std::vector<Company>& companies,
                const std::vector<std::shared_ptr<Shelf>>& shelves,
                const AlertManager& am,
                const InventoryManager& inv,
                const Logger& log,
                const std::string& path)
{
    std::ofstream f(path);
    if (!f) { std::cerr << "ERROR: cannot write " << path << "\n"; return; }

    f << "{\n";
    f << "  \"generatedAt\": \"" << genAt << "\",\n";

    // Supermarket
    f << "  \"supermarket\": {\n"
      << "    \"id\": \""       << sm.getTenantId()   << "\",\n"
      << "    \"name\": \""     << jstr(sm.getName()) << "\",\n"
      << "    \"location\": \"" << jstr(sm.getLocation()) << "\"\n"
      << "  },\n";

    // Companies
    f << "  \"companies\": [\n";
    for (size_t i = 0; i < companies.size(); i++) {
        f << "    {\"id\":\"" << companies[i].getTenantId() << "\","
          << "\"name\":\""    << jstr(companies[i].getName()) << "\","
          << "\"email\":\""   << jstr(companies[i].getContactEmail()) << "\"}";
        if (i+1 < companies.size()) f << ",";
        f << "\n";
    }
    f << "  ],\n";

    // Shelves
    f << "  \"shelves\": [\n";
    for (size_t si = 0; si < shelves.size(); si++) {
        const auto& s = shelves[si];
        f << "    {\n"
          << "      \"id\": \""           << s->getShelfId()   << "\",\n"
          << "      \"name\": \""         << jstr(s->getName()) << "\",\n"
          << "      \"weightReading\": "
              << std::fixed << std::setprecision(1) << s->getWeightReading() << ",\n"
          << "      \"irReading\": "
              << std::fixed << std::setprecision(1) << s->getIRReading() << ",\n"
          << "      \"sensorsHealthy\": " << (s->areSensorsHealthy()?"true":"false") << ",\n"
          << "      \"products\": [\n";

        const auto& prods  = s->getProducts();
        const auto& stocks = s->getStockCounts();
        for (size_t pi = 0; pi < prods.size(); pi++) {
            const auto& p = prods[pi];
            f << "        {\n"
              << "          \"barcode\": \""     << p.getBarcode()    << "\",\n"
              << "          \"sku\": \""          << p.getSku()        << "\",\n"
              << "          \"name\": \""         << jstr(p.getName()) << "\",\n"
              << "          \"companyId\": \""    << p.getCompanyId() << "\",\n"
              << "          \"currentStock\": "   << stocks[pi]       << ",\n"
              << "          \"threshold\": "      << p.getThreshold() << ",\n"
              << "          \"isPerishable\": "   << (p.isPerishable()?"true":"false") << ",\n"
              << "          \"expiryDate\": \""   << p.getExpiryDate() << "\",\n"
              << "          \"stockStatus\": \""  << stockStatus(stocks[pi],p.getThreshold()) << "\",\n"
              << "          \"expiryStatus\": \"" << expiryStatus(p.getExpiryDate(),p.isPerishable()) << "\"\n"
              << "        }";
            if (pi+1 < prods.size()) f << ",";
            f << "\n";
        }
        f << "      ]\n    }";
        if (si+1 < shelves.size()) f << ",";
        f << "\n";
    }
    f << "  ],\n";

    // Alerts
    f << "  \"alerts\": [\n";
    const auto& alerts = am.getAlerts();
    for (size_t i = 0; i < alerts.size(); i++) {
        const auto& a = alerts[i];
        f << "    {\n"
          << "      \"id\": \""             << a.id             << "\",\n"
          << "      \"type\": \""           << a.type           << "\",\n"
          << "      \"shelfId\": \""        << a.shelfId        << "\",\n"
          << "      \"productBarcode\": \"" << a.productBarcode << "\",\n"
          << "      \"companyId\": \""      << a.companyId      << "\",\n"
          << "      \"message\": \""        << jstr(a.message)  << "\",\n"
          << "      \"timestamp\": \""      << a.timestamp      << "\",\n"
          << "      \"acknowledgedBySupermarket\": " << (a.acknowledgedBySupermarket?"true":"false") << ",\n"
          << "      \"acknowledgedByCompany\": "     << (a.acknowledgedByCompany?"true":"false")     << "\n"
          << "    }";
        if (i+1 < alerts.size()) f << ",";
        f << "\n";
    }
    f << "  ],\n";

    // Restock Requests
    f << "  \"restockRequests\": [\n";
    const auto& reqs = inv.getRestockRequests();
    for (size_t i = 0; i < reqs.size(); i++) {
        const auto& r = reqs[i];
        f << "    {\n"
          << "      \"id\": \""               << r.requestId      << "\",\n"
          << "      \"shelfId\": \""          << r.shelfId        << "\",\n"
          << "      \"productBarcode\": \""   << r.productBarcode << "\",\n"
          << "      \"companyId\": \""        << r.companyId      << "\",\n"
          << "      \"supermarketId\": \""    << r.supermarketId  << "\",\n"
          << "      \"status\": \""           << RestockRequest::statusToString(r.status) << "\",\n"
          << "      \"quantityDispatched\": " << r.quantityDispatched << ",\n"
          << "      \"dispatchDate\": \""     << r.dispatchDate   << "\",\n"
          << "      \"acknowledgedBy\": \""   << r.acknowledgedBy << "\",\n"
          << "      \"acknowledgedAt\": \""   << r.acknowledgedAt << "\",\n"
          << "      \"createdAt\": \""        << r.createdAt      << "\",\n"
          << "      \"alertId\": \""          << r.alertId        << "\"\n"
          << "    }";
        if (i+1 < reqs.size()) f << ",";
        f << "\n";
    }
    f << "  ],\n";

    // Event Log
    f << "  \"eventLog\": [\n";
    const auto& evs = log.getEvents();
    for (size_t i = 0; i < evs.size(); i++) {
        const auto& e = evs[i];
        f << "    {\n"
          << "      \"timestamp\": \"" << e.timestamp << "\",\n"
          << "      \"event\": \""     << e.event     << "\",\n"
          << "      \"barcode\": \""   << e.barcode   << "\",\n"
          << "      \"quantity\": "    << e.quantity  << ",\n"
          << "      \"userId\": \""    << e.userId    << "\",\n"
          << "      \"note\": \""      << jstr(e.note) << "\"\n"
          << "    }";
        if (i+1 < evs.size()) f << ",";
        f << "\n";
    }
    f << "  ]\n}\n";

    std::cout << "✅  data.json written to: " << path << "\n";
}

// ── main ─────────────────────────────────────────────────────────────────────

int main() {
    srand((unsigned)time(nullptr));
    std::string startTime = nowISO();
    std::cout << "=== Supermarket Shelf Sensor Monitor ===\n";
    std::cout << "Started: " << startTime << "\n\n";

    Logger       logger("../shelf_log.txt");
    AlertManager alertManager;

    // Companies
    std::vector<Company> companies = {
        Company("C001","Brookside Dairy Ltd",  "rep@brookside.co.ke"),
        Company("C002","Unga Group Ltd",        "rep@unga.co.ke")
    };

    // Products
    Product milk   ("5901234123457","BRK-MILK-1L",  "Brookside Full Cream Milk 1L",   10,"2026-05-14","C001",true, 1040.0f);
    Product yogurt ("5901234123464","BRK-YOG-500",  "Brookside Strawberry Yogurt 500ml",8,"2026-05-13","C001",true,  530.0f);
    Product butter ("5901234123495","BRK-BTR-250",  "Brookside Butter 250g",            6,"2026-05-16","C001",true,  270.0f);
    Product maize  ("5901234123471","UNG-MAIZE-2K", "Unga Jogoo Maize Flour 2kg",       5,"",          "C002",false,2100.0f);
    Product wheat  ("5901234123488","UNG-WHEAT-1K", "Unga Ngano Wheat Flour 1kg",       5,"",          "C002",false,1050.0f);
    Product mandazi("5901234123501","UNG-MND-500",  "Unga Mandazi Mix 500g",            4,"",          "C002",false, 520.0f);

    companies[0].registerProduct(milk.getBarcode());
    companies[0].registerProduct(yogurt.getBarcode());
    companies[0].registerProduct(butter.getBarcode());
    companies[1].registerProduct(maize.getBarcode());
    companies[1].registerProduct(wheat.getBarcode());
    companies[1].registerProduct(mandazi.getBarcode());

    // Shelves
    auto shelf1 = std::make_shared<Shelf>("SH001","Dairy Shelf A",   "SM001");
    auto shelf2 = std::make_shared<Shelf>("SH002","Cereals Shelf B", "SM001");
    auto shelf3 = std::make_shared<Shelf>("SH003","Mixed Shelf C",   "SM001");

    { auto ws=std::make_unique<WeightSensor>("WS001","SH001",25000.f);
      auto ir=std::make_unique<IRSensor>    ("IR001","SH001",25.f);
      ir->setDistance(35.f); // sparse — confirms low stock
      shelf1->attachWeightSensor(std::move(ws)); shelf1->attachIRSensor(std::move(ir)); }

    { auto ws=std::make_unique<WeightSensor>("WS002","SH002",40000.f);
      auto ir=std::make_unique<IRSensor>    ("IR002","SH002",25.f);
      ir->setDistance(28.f); // slightly sparse
      shelf2->attachWeightSensor(std::move(ws)); shelf2->attachIRSensor(std::move(ir)); }

    { auto ws=std::make_unique<WeightSensor>("WS003","SH003",10000.f);
      auto ir=std::make_unique<IRSensor>    ("IR003","SH003",25.f);
      ir->setDistance(8.f);  // well stocked
      shelf3->attachWeightSensor(std::move(ws)); shelf3->attachIRSensor(std::move(ir)); }

    shelf1->addProduct(milk,    3);  // LOW  threshold=10
    shelf1->addProduct(yogurt, 12);  // OK   expires today
    shelf1->addProduct(butter,  2);  // LOW  threshold=6
    shelf2->addProduct(maize,   4);  // LOW  threshold=5
    shelf2->addProduct(wheat,   8);  // OK
    shelf3->addProduct(mandazi, 7);  // OK

    Supermarket supermarket("SM001","FreshMart Nairobi","Westlands, Nairobi");
    supermarket.addShelf(shelf1);
    supermarket.addShelf(shelf2);
    supermarket.addShelf(shelf3);

    std::vector<std::shared_ptr<Shelf>> allShelves = {shelf1,shelf2,shelf3};

    SupermarketManager manager    ("MGR001","Alice Wanjiku", "alice@freshmart.co.ke","SM001");
    CompanyRep         brooksideRep("REP001","James Mwangi", "james@brookside.co.ke","C001");
    CompanyRep         ungaRep    ("REP002","Susan Otieno",  "susan@unga.co.ke",     "C002");

    InventoryManager inventoryMgr(allShelves, logger, alertManager);
    StockMonitor     stockMonitor(alertManager, logger);
    ExpiryMonitor    expiryMonitor(alertManager, logger);
    for (const auto& s : allShelves) stockMonitor.addShelf(s);

    // ── Simulation ────────────────────────────────────────────────────────────
    std::cout << "Step 1: Expiry check\n";
    expiryMonitor.runCheck(allShelves);
    std::cout << "  Alerts so far: " << alertManager.getAlerts().size() << "\n";

    std::cout << "Step 2: Stock monitor poll\n";
    auto detections = stockMonitor.poll();
    std::cout << "  Low-stock detections: " << detections.size() << "\n";

    std::cout << "Step 3: Create restock requests\n";
    for (const auto& d : detections) {
        std::string alertId;
        for (const auto& a : alertManager.getAlerts())
            if (a.type=="low_stock" && a.shelfId==d.shelfId && a.productBarcode==d.productBarcode)
                alertId = a.id;
        std::string id = inventoryMgr.createRestockRequest(
            d.shelfId, d.productBarcode, d.companyId, "SM001", alertId);
        std::cout << "  Created " << id << " for " << d.productBarcode << "\n";
    }

    std::cout << "Step 4: Brookside rep dispatches milk restock\n";
    for (const auto& r : inventoryMgr.getRestockRequests()) {
        if (r.productBarcode == milk.getBarcode()) {
            if (inventoryMgr.dispatchRestock(r.requestId,24,"2026-05-14",brooksideRep.getUserId()))
                std::cout << "  Dispatched " << r.requestId << " — 24 units ETA 2026-05-14\n";
            break;
        }
    }

    std::cout << "Step 5: Simulate sales\n";
    inventoryMgr.processSale(wheat.getBarcode(), 2, manager.getUserId());

    BarcodeScanner scanner("SC001");
    scanner.loadBarcodes({wheat.getBarcode(), wheat.getBarcode(), wheat.getBarcode()});
    for (int i = 0; i < 3; i++)
        inventoryMgr.processSale(scanner.simulateScan(), 1, manager.getUserId());
    std::cout << "  5x Wheat Flour sold (2 manual + 3 via scanner)\n";

    std::cout << "\nExporting data.json...\n";
    exportJSON(startTime, supermarket, companies, allShelves,
               alertManager, inventoryMgr, logger, "../dashboard/data.json");

    std::cout << "\n=== Done ===\n";
    std::cout << "Alerts: "           << alertManager.getAlerts().size()           << "\n";
    std::cout << "Restock requests: " << inventoryMgr.getRestockRequests().size()  << "\n";
    std::cout << "Log events: "       << logger.getEvents().size()                 << "\n";
    return 0;
}
