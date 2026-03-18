#include "simulator.hpp"

CacheDM::CacheDM() : lines(CACHE_LINES) {}
void CacheDM::access(unsigned long address) {
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

void CacheDM::clean() {
  Cache::clean();
  replacements = 0;
  for (auto &line : lines) {
    line.valid = false;
    line.tag = 0;
  }
}

CacheFullyAssoc::CacheFullyAssoc(Policy p)
    : lines(CACHE_LINES), replacements(0), policy(p),
      rng(std::random_device{}()) {}

void CacheFullyAssoc::access(unsigned long address) {
  accesses++;
  unsigned long block_addr = address / BLOCK_SIZE;
  unsigned long tag = block_addr;

  if (policy == Policy::LRU || policy == Policy::FIFO) {
    for (auto &line : lines)
      if (line.valid)
        line.age++;
  }

  for (auto &line : lines) {
    if (line.valid && line.tag == tag) {
      hits++;
      if (policy == Policy::LRU)
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
  size_t victim = 0;

  if (policy == Policy::Random) {
    std::uniform_int_distribution<size_t> dist(0, lines.size() - 1);
    victim = dist(rng);
  } else {
    for (size_t i = 1; i < lines.size(); ++i)
      if (lines[i].age > lines[victim].age)
        victim = i;
  }

  lines[victim].tag = tag;
  lines[victim].valid = true;
  lines[victim].age = 0;
}

void CacheFullyAssoc::clean() {
  Cache::clean();
  replacements = 0;
  for (auto &line : lines) {
    line.valid = false;
    line.age = 0;
    line.tag = 0;
  }
}

Cache2SetWayAssociative::Cache2SetWayAssociative(Policy p)
    : sets(NUM_SETS, std::vector<TwoWayLine>(TWO_WAY_SET)), replacements(0),
      policy(p), rng(std::random_device{}()) {}

void Cache2SetWayAssociative::clean() {
  Cache::clean();
  replacements = 0;
  for (auto &set : sets)
    for (auto &line : set)
      line = {};
}

void Cache2SetWayAssociative::access(unsigned long address) {
  accesses++;
  unsigned long block_addr = address / BLOCK_SIZE;
  size_t set_idx = block_addr % NUM_SETS;
  unsigned long tag = block_addr / NUM_SETS;

  auto &set = sets[set_idx];

  if (policy == Policy::LRU || policy == Policy::FIFO) {
    for (auto &line : set)
      if (line.valid)
        line.age++;
  }

  for (auto &line : set) {
    if (line.valid && line.tag == tag) {
      hits++;
      if (policy == Policy::LRU)
        line.age = 0;
      return;
    }
  }

  misses++;

  for (auto &line : set) {
    if (!line.valid) {
      line.valid = true;
      line.tag = tag;
      line.age = 0;
      return;
    }
  }

  replacements++;
  size_t victim = 0;

  if (policy == Policy::Random) {
    std::uniform_int_distribution<size_t> dist(0, set.size() - 1);
    victim = dist(rng);
  } else {
    for (size_t i = 1; i < set.size(); ++i)
      if (set[i].age > set[victim].age)
        victim = i;
  }

  set[victim].tag = tag;
  set[victim].valid = true;
  set[victim].age = 0;
}

unsigned long gen_address(AccessPattern pattern, int i, std::mt19937 &rng,
                          std::uniform_int_distribution<unsigned long> &dist) {
  switch (pattern) {
  case AccessPattern::Random:
    return dist(rng);

  case AccessPattern::Sequential:
    return i % MEMORY_SIZE;

  case AccessPattern::FourStrike: {
    // 20 bloques distintos en ciclo → más que las 16 líneas de FA
    unsigned long block = i % 20;
    return block * BLOCK_SIZE;
  }

  case AccessPattern::ConflictDM: {
    // 3 bloques distintos que caen SIEMPRE en el MISMO índice
    unsigned long blockA = 0;
    unsigned long blockB = CACHE_LINES;     // 16
    unsigned long blockC = CACHE_LINES * 2; // 32

    unsigned long block;
    if (i % 3 == 0)
      block = blockA;
    else if (i % 3 == 1)
      block = blockB;
    else
      block = blockC;

    return block * BLOCK_SIZE;
  }

  case AccessPattern::WorkingSet64: {
    unsigned long block = (i / 8) % 64;
    return block * BLOCK_SIZE;
  }

  case AccessPattern::StrideConflict:
    return (i * NUM_SETS * BLOCK_SIZE) % MEMORY_SIZE;
  }

  return 0;
}
