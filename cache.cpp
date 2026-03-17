#include <cstddef>
#include <iostream>
#include <iterator>
#include <random>
#include <utility>
#include <vector>

const int CACHE_LINES = 16;
const int BLOCK_SIZE = 8;
const unsigned long MEMORY_SIZE = 65536;

class Cache {
public:
  unsigned long hits;
  unsigned long misses;
  unsigned long accesses;
  Cache() {
    misses = 0;
    hits = 0;
    accesses = 0;
  }
  virtual void clean() {
    misses = 0;
    hits = 0;
    accesses = 0;
  }
  virtual void printStats() { std::cout << "Statistics" << std::endl; };
};
class Line {
public:
  unsigned long tag;
  bool valid;

  Line() : tag(0), valid(false) {}
};

// Direct mapping
class CacheDM : public Cache {
public:
  std::vector<Line> lines;

  CacheDM() {
    lines.resize(CACHE_LINES);
    hits = 0;
    misses = 0;
    accesses = 0;
  }

  void clean() {
    for (size_t i = 0; i < CACHE_LINES; i++) {
      lines[i].tag = 0;
      lines[i].valid = false;
    }
  }

  void access_direct(unsigned long address);
  void printStats() override {
    std::cout << "\nDirect Mapping Cache Statistics\n";
    std::cout << "Accesses: " << accesses << "\n";
    std::cout << "Hits: " << hits << "\n";
    std::cout << "Misses: " << misses << "\n";

    double hit_rate = accesses ? (double)hits / accesses * 100.0 : 0.0;
    std::cout << "Hit Rate: " << hit_rate << "%\n";
  };
};

void CacheDM::access_direct(unsigned long address) {
  accesses++;

  unsigned long block_addr = address / BLOCK_SIZE;
  unsigned long index = block_addr % CACHE_LINES;
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

// N-set associative
class CacheNSetAssoc : public Cache {
public:
  void access_2_way_set();
};

void CacheNSetAssoc::access_2_way_set() {}

class FullyLine {
public:
  bool valid;
  unsigned long counter;
  unsigned long tag;
  FullyLine() {
    tag = 0;
    counter = 0;
    valid = false;
  }
};

// fully associative
class CacheFullyAssoc : public Cache {
public:
  std::vector<FullyLine> lines;
  int replace;

  CacheFullyAssoc() {
    lines.resize(CACHE_LINES);
    replace = 0;
  }
  void access_fully(unsigned long tag);
  std::pair<bool, int> check_no_full();
  bool in_cache(unsigned long tag);
  std::pair<int, unsigned long> less_used();
  void clean() override {
    misses = 0;
    hits = 0;
    accesses = 0;
    replace = 0;
  }
  void printStats() override {
    std::cout << "\nFully Associative Cache Statistics\n";
    std::cout << "Accesses: " << accesses << "\n";
    std::cout << "Hits: " << hits << "\n";
    std::cout << "Misses: " << misses << "\n";
    std::cout << "Replacements: " << replace << "\n";
    double hit_rate = accesses ? (double)hits / accesses * 100.0 : 0.0;
    std::cout << "Hit Rate: " << hit_rate << "%\n";
  }
};

std::pair<bool, int> CacheFullyAssoc::check_no_full() {
  for (auto it = lines.begin(); it != lines.end(); ++it) {
    if (!it->valid) {
      return {true, std::distance(lines.begin(), it)};
    }
  }
  return {false, -1};
}
bool CacheFullyAssoc::in_cache(unsigned long tag) {

  for (auto &line : lines) {
    if (line.tag == tag and line.valid) {
      return true;
    }
  }

  return false;
}

std::pair<int, unsigned long> CacheFullyAssoc::less_used() {

  std::pair<int, unsigned long> min; 
  min.first = -1;
  min.second = lines[0].counter;
  for (auto it = lines.begin() + 1; it < lines.end(); it++) {
    if (it->counter < min.second) {
      min.second = it->counter;
      min.first = std::distance(lines.begin(), it);
    }
  }

  return min;
}

void CacheFullyAssoc::access_fully(unsigned long address) {
  accesses++;

  unsigned long block_addr = address / BLOCK_SIZE;
  unsigned long tag = block_addr;

  for (auto &line : lines) {
    if (line.valid)
      line.counter++;
  }

  for (auto &line : lines) {
    if (line.valid && line.tag == tag) {
      hits++;
      line.counter = 0;
      return;
    }
  }

  misses++;
  for (auto &line : lines) {
    if (!line.valid) {
      line.valid = true;
      line.tag = tag;
      line.counter = 0;
      return;
    }
  }

  int lru_idx = 0;
  for (size_t  i = 1; i < lines.size(); i++) {
    if (lines[i].counter > lines[lru_idx].counter) {
      lru_idx = i;
    }
  }

  replace++;
  lines[lru_idx].tag = tag;
  lines[lru_idx].counter = 0;
}

int main() {

  CacheDM cache_dm;
  CacheFullyAssoc cache_fully_assoc;
  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_int_distribution<unsigned long> dist(0, MEMORY_SIZE - 1);

  const int NUM_ACCESSES = 200000;

  std::cout << "\n\n+-------------+\n| RANDOM ADDR |\n+-------------+";

  for (int i = 0; i < NUM_ACCESSES; i++) {
    unsigned long addr = dist(rng);
    cache_dm.access_direct(addr);
    cache_fully_assoc.access_fully(addr);
  }

  cache_dm.printStats();
  cache_fully_assoc.printStats();
  cache_dm.clean();
  cache_fully_assoc.clean();

  std::cout
      << "\n\n+-----------------+\n| SEQUENTIAL ADDR |\n+-----------------+";

  for (int i = 0; i < NUM_ACCESSES; i++) {
    unsigned long addr = i % MEMORY_SIZE;
    cache_dm.access_direct(addr);
    cache_fully_assoc.access_fully(addr);
  }

  cache_dm.printStats();
  cache_fully_assoc.printStats();

  return 0;
}
