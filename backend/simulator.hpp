#pragma once
#include <cstddef>
#include <random>
#include <vector>

constexpr size_t CACHE_LINES = 16;
constexpr size_t BLOCK_SIZE = 8;
constexpr size_t MEMORY_SIZE = 1024;
constexpr size_t TWO_WAY_SET = 2;
constexpr size_t NUM_SETS = CACHE_LINES / TWO_WAY_SET;

enum class Policy { LRU, FIFO, Random };

enum class AccessPattern {
  Random,
  Sequential,
  FourStrike,
  ConflictDM,
  WorkingSet64,
  StrideConflict
};

class Cache {
public:
  unsigned long hits = 0;
  unsigned long misses = 0;
  unsigned long accesses = 0;

  virtual ~Cache() = default;
  virtual void access(unsigned long address) = 0;
  virtual void clean() { hits = misses = accesses = 0; }

  virtual unsigned long getReplacements() const { return 0; }
};

struct Line {
  unsigned long tag = 0;
  bool valid = false;
};

class CacheDM : public Cache {
public:
  std::vector<Line> lines;
  unsigned long replacements = 0;

  CacheDM();
  void access(unsigned long address) override;
  void clean() override;
  unsigned long getReplacements() const override { return replacements; }
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
  Policy policy;
  std::mt19937 rng;

  CacheFullyAssoc(Policy p);
  void access(unsigned long address) override;
  void clean() override;
  unsigned long getReplacements() const override { return replacements; }
};

struct TwoWayLine {
  unsigned long tag = 0;
  bool valid = false;
  unsigned long age = 0;
};

class Cache2SetWayAssociative : public Cache {
public:
  std::vector<std::vector<TwoWayLine>> sets;
  unsigned long replacements = 0;
  Policy policy;
  std::mt19937 rng;

  Cache2SetWayAssociative(Policy p);
  void access(unsigned long address) override;
  void clean() override;
  unsigned long getReplacements() const override { return replacements; }
};

unsigned long gen_address(AccessPattern pattern, int i, std::mt19937 &rng,
                          std::uniform_int_distribution<unsigned long> &dist);
