#include "httplib.h"
#include "simulator.hpp"
#include <iostream>
#include <memory>
#include <string>

using namespace httplib;

namespace {
bool is_valid_cache_type(const std::string &cacheType) {
  return cacheType == "DM" || cacheType == "FA" || cacheType == "2W";
}

bool is_valid_policy(const std::string &policyStr) {
  return policyStr == "LRU" || policyStr == "FIFO" || policyStr == "Random";
}

bool is_valid_pattern(const std::string &patternStr) {
  return patternStr == "Random" || patternStr == "Sequential" ||
         patternStr == "FourStrike" || patternStr == "4Strike" ||
         patternStr == "ConflictDM" || patternStr == "WorkingSet64" ||
         patternStr == "StrideConflict";
}

double parse_positive_param(const Request &req, const char *name,
                            double defaultValue, bool allowZero = false) {
  if (!req.has_param(name)) {
    return defaultValue;
  }

  double value = std::stod(req.get_param_value(name));
  if (value < 0.0 || (!allowZero && value == 0.0)) {
    throw std::invalid_argument("invalid numeric parameter");
  }
  return value;
}
} // namespace

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
      TimingConfig timingConfig;
      timingConfig.hitLatencyCycles =
          parse_positive_param(req, "hitLatency", timingConfig.hitLatencyCycles);
      timingConfig.missPenaltyCycles = parse_positive_param(
          req, "missPenalty", timingConfig.missPenaltyCycles, true);
      timingConfig.cpuFrequencyGHz = parse_positive_param(
          req, "cpuFrequencyGHz", timingConfig.cpuFrequencyGHz);

      if (!is_valid_cache_type(cacheType) || !is_valid_policy(policyStr) ||
          !is_valid_pattern(patternStr) || numAccesses < 0) {
        throw std::invalid_argument("invalid simulation parameters");
      }

      Policy policy = (policyStr == "LRU")    ? Policy::LRU
                      : (policyStr == "FIFO") ? Policy::FIFO
                                              : Policy::Random;

      std::unique_ptr<Cache> cache;

      if (cacheType == "DM") {
        cache = std::make_unique<CacheDM>();
      } else if (cacheType == "FA") {
        cache = std::make_unique<CacheFullyAssoc>(policy);
      } else if (cacheType == "2W") {
        cache = std::make_unique<Cache2SetWayAssociative>(policy);
      } else {
        throw std::invalid_argument("invalid cache type");
      }

      AccessPattern pattern =
          (patternStr == "Random")         ? AccessPattern::Random
          : (patternStr == "Sequential")   ? AccessPattern::Sequential
          : (patternStr == "FourStrike" || patternStr == "4Strike")
              ? AccessPattern::FourStrike
          : (patternStr == "ConflictDM")   ? AccessPattern::ConflictDM
          : (patternStr == "WorkingSet64") ? AccessPattern::WorkingSet64
                                           : AccessPattern::StrideConflict;

      std::mt19937 rng(std::random_device{}());
      std::uniform_int_distribution<unsigned long> dist(0, MEMORY_SIZE - 1);

      cache->clean();
      for (int i = 0; i < numAccesses; ++i) {
        unsigned long addr = gen_address(pattern, i, rng, dist);
        cache->access(addr);
      }

      double hitRate =
          cache->accesses ? (double)cache->hits / cache->accesses * 100.0 : 0.0;
      TimingStats timingStats = calculate_timing_stats(*cache, timingConfig);

      std::string json = "{";
      json += "\"hits\":" + std::to_string(cache->hits) + ",";
      json += "\"misses\":" + std::to_string(cache->misses) + ",";
      json += "\"hitRate\":" + std::to_string(hitRate) + ",";
      json += "\"replacements\":" + std::to_string(cache->getReplacements()) +
              ",";
      json += "\"hitLatency\":" +
              std::to_string(timingConfig.hitLatencyCycles) + ",";
      json += "\"missPenalty\":" +
              std::to_string(timingConfig.missPenaltyCycles) + ",";
      json += "\"cpuFrequencyGHz\":" +
              std::to_string(timingConfig.cpuFrequencyGHz) + ",";
      json += "\"totalCycles\":" + std::to_string(timingStats.totalCycles) +
              ",";
      json += "\"amat\":" + std::to_string(timingStats.amatCycles) + ",";
      json += "\"estimatedTimeNs\":" +
              std::to_string(timingStats.estimatedTimeNs);
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
