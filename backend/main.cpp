#include "simulator.hpp"
#include <iostream>
#include <random>
#include <string>

int main(int argc, char **argv) {
  if (argc != 5) {
    std::cout << "Usage: ./test_simulator <cacheType> <policy> <pattern> "
                 "<numAccesses>\n";
    std::cout << "cacheType: DM | FA | 2W\n";
    std::cout << "policy: LRU | FIFO | Random\n";
    std::cout << "pattern: Random | Sequential | FourStrike | ConflictDM | "
                 "WorkingSet64 | StrideConflict\n";
    return 1;
  }

  std::string cacheType = argv[1];
  std::string policyStr = argv[2];
  std::string patternStr = argv[3];
  int numAccesses = std::stoi(argv[4]);

  Policy policy = Policy::LRU;
  if (policyStr == "FIFO")
    policy = Policy::FIFO;
  if (policyStr == "Random")
    policy = Policy::Random;

  AccessPattern pattern = AccessPattern::Random;
  if (patternStr == "Sequential")
    pattern = AccessPattern::Sequential;
  if (patternStr == "FourStrike")
    pattern = AccessPattern::FourStrike;
  if (patternStr == "ConflictDM")
    pattern = AccessPattern::ConflictDM;
  if (patternStr == "WorkingSet64")
    pattern = AccessPattern::WorkingSet64;
  if (patternStr == "StrideConflict")
    pattern = AccessPattern::StrideConflict;

  Cache *cache = nullptr;

  if (cacheType == "DM") {
    cache = new CacheDM();
  } else if (cacheType == "FA") {
    cache = new CacheFullyAssoc(policy);
  } else if (cacheType == "2W") {
    cache = new Cache2SetWayAssociative(policy);
  } else {
    std::cout << "Invalid cache type\n";
    return 1;
  }

  std::mt19937 rng(12345);
  std::uniform_int_distribution<unsigned long> dist(0, MEMORY_SIZE - 1);

  for (int i = 0; i < numAccesses; i++) {
    unsigned long addr = gen_address(pattern, i, rng, dist);
    cache->access(addr);
  }

  std::cout << "Hits: " << cache->hits << "\n";
  std::cout << "Misses: " << cache->misses << "\n";
  std::cout << "HitRate: " << (100.0 * cache->hits / cache->accesses) << "\n";
  std::cout << "Replacements: " << cache->getReplacements() << "\n";

  delete cache;
  return 0;
}
