#include <cstddef>
#include <fstream>
#include <iostream>
#include <ostream>
#include <random>
#include <vector>
constexpr size_t CACHE_LINES = 12;
constexpr size_t BLOCK_SIZE = 8;
constexpr size_t MEMORY_SIZE = 1024;
constexpr size_t TWO_WAY_SET = 2;

class Cache {
public:
  unsigned long hits = 0;
  unsigned long misses = 0;
  unsigned long accesses = 0;

  virtual ~Cache() = default;
  virtual void access(unsigned long address) = 0;
  virtual void clean() { hits = misses = accesses = 0; }
  virtual void printStats() const {
    std::cout << "Accesses: " << accesses << "\n";
    std::cout << "Hits: " << hits << "\n";
    std::cout << "Misses: " << misses << "\n";

    double hit_rate = accesses ? (double)hits / accesses * 100.0 : 0.0;
    std::cout << "\033[31m"
              << "Hit Rate: " << hit_rate << "%"
              << "\033[0m\n";
  }
};

struct Line {
  unsigned long tag = 0;
  bool valid = false;
};

class CacheDM : public Cache {
public:
  std::vector<Line> lines;

public:
  CacheDM() : lines(CACHE_LINES) {}

  void access(unsigned long address) override {
    accesses++;

    unsigned long block_addr = address / BLOCK_SIZE;
    size_t index = block_addr % CACHE_LINES;
    unsigned long tag = block_addr / CACHE_LINES;

    Line &line = lines[index];

    if (line.valid && line.tag == tag) {
      hits++;
    } else {
      misses++;
      line.valid = true;
      line.tag = tag;
    }
  }

  void clean() override {
    Cache::clean();
    for (auto &line : lines) {
      line.valid = false;
      line.tag = 0;
    }
  }

  void printStats() const override {
    std::cout << "\n[Direct Mapped Cache]\n";
    Cache::printStats();
  }
};

struct FullyLine {
  bool valid = false;
  unsigned long tag = 0;
  unsigned long age = 0;
};

class CacheFullyAssoc : public Cache {
public:
  std::vector<FullyLine> lines;
  unsigned long replacements = 0;

public:
  CacheFullyAssoc() : lines(CACHE_LINES) {}

  void access(unsigned long address) override {
    accesses++;

    unsigned long block_addr = address / BLOCK_SIZE;
    unsigned long tag = block_addr;

    for (auto &line : lines)
      if (line.valid)
        line.age++;

    for (auto &line : lines) {
      if (line.valid && line.tag == tag) {
        hits++;
        line.age = 0;
        return;
      }
    }

    misses++;

    for (auto &line : lines) {
      if (!line.valid) {
        line.valid = true;
        line.tag = tag;
        line.age = 0;
        return;
      }
    }

    replacements++;
    size_t lru_idx = 0;
    for (size_t i = 1; i < lines.size(); i++)
      if (lines[i].age > lines[lru_idx].age)
        lru_idx = i;

    lines[lru_idx].tag = tag;
    lines[lru_idx].age = 0;
  }

  void clean() override {
    Cache::clean();
    replacements = 0;
    for (auto &line : lines) {
      line.valid = false;
      line.age = 0;
      line.tag = 0;
    }
  }

  void printStats() const override {
    std::cout << "\n[Fully Associative Cache]\n";
    Cache::printStats();
    std::cout << "Replacements: " << replacements << "\n";
  }
};

struct TwoWayLine {
  unsigned long tag = 0;
  bool valid = 0;
  unsigned long age = 0;
};

class Cache2SetWayAssociative : public Cache {
public:
  std::vector<std::vector<TwoWayLine>> sets;
  unsigned long replacements;

  Cache2SetWayAssociative() {
    size_t set_size = CACHE_LINES / TWO_WAY_SET;
    sets.resize(set_size, std::vector<TwoWayLine>(TWO_WAY_SET));
    replacements = 0;
  }

  void clean() override {
    Cache::clean();
    replacements = 0;
    for (auto &set : sets) {
      for (auto &line : set) {
        line.tag = 0;
        line.valid = false;
        line.age = 0;
      }
    }
  }

  void access(unsigned long address) override {
    accesses++;

    unsigned long block_addr = address / BLOCK_SIZE;
    size_t set = block_addr % (CACHE_LINES / TWO_WAY_SET);
    unsigned long tag = block_addr / (CACHE_LINES / TWO_WAY_SET);

    for (auto &line : sets[set])
      if (line.valid)
        line.age++;

    for (auto &line : sets[set]) {
      if (line.valid && line.tag == tag) {
        hits++;
        line.age = 0;
        return;
      }
    }

    misses++;

    for (auto &line : sets[set]) {
      if (!line.valid) {
        line.valid = true;
        line.tag = tag;
        line.age = 0;
        return;
      }
    }

    replacements++;
    size_t lru = 0;
    for (size_t i = 1; i < sets[set].size(); i++)
      if (sets[set][i].age > sets[set][lru].age)
        lru = i;

    sets[set][lru].tag = tag;
    sets[set][lru].valid = true;
    sets[set][lru].age = 0;
  }

  void printStats() const override {
    std::cout << "\n[2-Way Set Associative Cache]\n";
    Cache::printStats();
    std::cout << "Replacements: " << replacements << "\n";
  }
};

int main() {
  CacheDM cache_dm;
  CacheFullyAssoc cache_fully;
  Cache2SetWayAssociative cache_2_way_set;

  std::ofstream myfile("cache_stats.csv");
  myfile << "AccessType,CacheType,Hits,Misses,HitRate,Replacements\n";

  std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<unsigned long> dist(0, MEMORY_SIZE - 1);
  const int NUM_ACCESSES = 200000;

  for (int i = 0; i < NUM_ACCESSES; i++) {
    auto addr = dist(rng);
    cache_dm.access(addr);
    cache_fully.access(addr);
    cache_2_way_set.access(addr);
  }

  myfile << "Random,DirectMapping," << cache_dm.hits << "," << cache_dm.misses << ","
         << (double)cache_dm.hits / cache_dm.accesses * 100 << ",0\n";
  myfile << "Random,FullyAssociative," << cache_fully.hits << ","
         << cache_fully.misses << ","
         << (double)cache_fully.hits / cache_fully.accesses * 100 << ","
         << cache_fully.replacements << "\n";
  myfile << "Random,2WaySet," << cache_2_way_set.hits << ","
         << cache_2_way_set.misses << ","
         << (double)cache_2_way_set.hits / cache_2_way_set.accesses * 100 << ","
         << cache_2_way_set.replacements << "\n";

  cache_dm.clean();
  cache_fully.clean();
  cache_2_way_set.clean();

  for (int i = 0; i < NUM_ACCESSES; i++) {
    auto addr = i % MEMORY_SIZE;
    cache_dm.access(addr);
    cache_fully.access(addr);
    cache_2_way_set.access(addr);
  }

  myfile << "Sequential,DirectMapping," << cache_dm.hits << "," << cache_dm.misses
         << "," << (double)cache_dm.hits / cache_dm.accesses * 100 << ",0\n";
  myfile << "Sequential,FullyAssociative," << cache_fully.hits << ","
         << cache_fully.misses << ","
         << (double)cache_fully.hits / cache_fully.accesses * 100 << ","
         << cache_fully.replacements << "\n";
  myfile << "Sequential,2WaySet," << cache_2_way_set.hits << ","
         << cache_2_way_set.misses << ","
         << (double)cache_2_way_set.hits / cache_2_way_set.accesses * 100 << ","
         << cache_2_way_set.replacements << "\n";

  myfile.close();
  return 0;
}