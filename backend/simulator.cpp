#include "simulator.hpp"

CacheDM::CacheDM() : lines(CACHE_LINES), replacements(0) {}

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
    if (line.valid) {
      replacements++;
    }
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
      rng(std::random_device{}()), timer(0) {
        
      }

void CacheFullyAssoc::access(unsigned long address) {
  accesses++;
  unsigned long block_addr = address / BLOCK_SIZE;
  unsigned long tag = block_addr;

  for (auto &line : lines) {
    if (line.valid && line.tag == tag) {
      hits++;
      if (policy == Policy::LRU) {
        line.age = timer++;
      }
      return;
    }
  }

  misses++;

  for (auto &line : lines) {
    if (!line.valid) {
      line.valid = true;
      line.tag = tag;
      line.age = timer++;
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
      if (lines[i].age < lines[victim].age)
        victim = i;
  }

  lines[victim].tag = tag;
  lines[victim].valid = true;
  lines[victim].age = timer++;
}

void CacheFullyAssoc::clean() {
  Cache::clean();
  replacements = 0;
  timer = 0;
  for (auto &line : lines) {
    line.valid = false;
    line.age = 0;
    line.tag = 0;
  }
}

Cache2SetWayAssociative::Cache2SetWayAssociative(Policy p)
    : sets(NUM_SETS, std::vector<TwoWayLine>(TWO_WAY_SET)),
      replacements(0), policy(p),
      rng(std::random_device{}()), timer(0) {}

void Cache2SetWayAssociative::clean() {
  Cache::clean();
  replacements = 0;
  timer = 0;
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

  for (auto &line : set) {
    if (line.valid && line.tag == tag) {
      hits++;
      if (policy == Policy::LRU) {
        line.age = timer++;
      }
      return;
    }
  }

  misses++;

  for (auto &line : set) {
    if (!line.valid) {
      line.valid = true;
      line.tag = tag;
      line.age = timer++;
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
      if (set[i].age < set[victim].age)
        victim = i;
  }

  set[victim].tag = tag;
  set[victim].valid = true;
  set[victim].age = timer++;
}

unsigned long gen_address(AccessPattern pattern, int i, std::mt19937 &rng,
                          std::uniform_int_distribution<unsigned long> &dist) {
  switch (pattern) {
  case AccessPattern::Random:
    return dist(rng);

  case AccessPattern::Sequential:
    return i % MEMORY_SIZE;

  case AccessPattern::FourStrike: {
    unsigned long block = i % 20;
    return block * BLOCK_SIZE;
  }

  case AccessPattern::ConflictDM: {
    unsigned long blockA = 0;
    unsigned long blockB = CACHE_LINES;
    unsigned long blockC = CACHE_LINES * 2;

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

TimingStats calculate_timing_stats(const Cache &cache,
                                   const TimingConfig &config) {
  TimingStats stats;
  if (cache.accesses == 0) {
    return stats;
  }

  stats.totalCycles =
      (cache.accesses * config.hitLatencyCycles) +
      (cache.misses * config.missPenaltyCycles);
  stats.amatCycles = stats.totalCycles / cache.accesses;

  if (config.cpuFrequencyGHz > 0.0) {
    stats.estimatedTimeNs = stats.totalCycles / config.cpuFrequencyGHz;
  }

  return stats;
}
