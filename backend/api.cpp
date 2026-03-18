#include "httplib.h"
#include "simulator.hpp"
#include <iostream>
#include <memory>
#include <string>

using namespace httplib;

int main() {
  Server svr;

  std::cout << "Servidor en http://localhost:8080\n";

  svr.Get("/simulate", [](const Request &req, Response &res) {
    try {
      // Permitir CORS
      res.set_header("Access-Control-Allow-Origin", "*");
      res.set_header("Access-Control-Allow-Headers", "Content-Type");

      std::string cacheType = req.get_param_value("cacheType");
      std::string policyStr = req.get_param_value("policy");
      std::string patternStr = req.get_param_value("pattern");
      int numAccesses = std::stoi(req.get_param_value("numAccesses"));

      Policy policy = (policyStr == "LRU")    ? Policy::LRU
                      : (policyStr == "FIFO") ? Policy::FIFO
                                              : Policy::Random;

      std::unique_ptr<Cache> cache;

      if (cacheType == "DM") {
        cache = std::make_unique<CacheDM>();
      } else if (cacheType == "FA") {
        cache = std::make_unique<CacheFullyAssoc>(policy);
      } else {
        cache = std::make_unique<Cache2SetWayAssociative>(policy);
      }

      AccessPattern pattern =
          (patternStr == "Random")         ? AccessPattern::Random
          : (patternStr == "Sequential")   ? AccessPattern::Sequential
          : (patternStr == "4Strike")      ? AccessPattern::FourStrike
          : (patternStr == "ConflictDM")   ? AccessPattern::ConflictDM
          : (patternStr == "WorkingSet64") ? AccessPattern::WorkingSet64
                                           : AccessPattern::StrideConflict;

      std::mt19937 rng(std::random_device{}());
      std::uniform_int_distribution<unsigned long> dist(0, MEMORY_SIZE - 1);

      for (int i = 0; i < numAccesses; ++i) {
        unsigned long addr = gen_address(pattern, i, rng, dist);
        cache->access(addr);
      }

      double hitRate =
          cache->accesses ? (double)cache->hits / cache->accesses * 100.0 : 0.0;

      std::string json = "{";
      json += "\"hits\":" + std::to_string(cache->hits) + ",";
      json += "\"misses\":" + std::to_string(cache->misses) + ",";
      json += "\"hitRate\":" + std::to_string(hitRate) + ",";
      json += "\"replacements\":" + std::to_string(cache->getReplacements());
      json += "}";

      res.set_content(json, "application/json");

    } catch (...) {
      res.status = 400;
      res.set_header("Access-Control-Allow-Origin", "*");
      res.set_content("{\"error\":\"bad request\"}", "application/json");
    }
  });

  svr.listen("0.0.0.0", 8080);
}
