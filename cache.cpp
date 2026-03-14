#include <cstdlib>
#include <iostream>
#include <ostream>
#include <random>
#include <vector>

const int CACHE_LINES = 16;
const int BLOCK_SIZE = 8;
const unsigned long int MEMORY_ADDR_TOTAL =
    65536; 

class Line {
public:
  int tag;
  bool valid_bit;
  std::vector<int> data;

  Line() {
    tag = 0;
    valid_bit = false;
    data.resize(BLOCK_SIZE, 0);
  }
  Line &operator=(const Line &line) {
    if (this != &line) {
      this->tag = line.tag;
      this->valid_bit = line.valid_bit;
      this->data = line.data;
    }
    return *this;
  }
};

class Cache {
public:
  std::vector<Line> lines;
  int num_MISS;
  int num_HIT;
  Cache() {
    lines.resize(CACHE_LINES);
    num_MISS = 0;
    num_HIT = 0;
  }

  void printCache() {
    std::cout << "CACHE LINE\n" << std::endl;
    for (size_t line = 0; line < CACHE_LINES; line++) {
      std::cout << "Cache Line " << line << ": ";
      std::cout << "Tag = " << this->lines[line].tag
                << ", Valid = " << (lines[line].valid_bit ? "True" : "False")
                << ", Data = [";
      for (size_t j = 0; j < BLOCK_SIZE; ++j) {
        std::cout << lines[line].data[j] << (j == BLOCK_SIZE - 1 ? "" : ", ");
      }
      std::cout << "]" << std::endl;
    }

    std::cout << "\nMISSES: " << this->num_MISS << std::endl;
    std::cout << "HITS: " << this->num_HIT << std::endl;
  }

  void access(int tag, std::vector<unsigned long> &memory) {
    size_t index = tag % CACHE_LINES;
    if (this->lines[index].valid_bit && (this->lines[index].tag <= tag) &&
        (tag < (this->lines[index].tag + 8))) {
      std::cout << "HIT for address " << tag << std::endl;
      this->num_HIT++;
    } else {
      std::cout << "MISS for address " << tag << std::endl;
      this->lines[index].tag = tag;
      this->lines[index].valid_bit = true;
      this->num_MISS++;
      for (size_t i = 0; i < BLOCK_SIZE; i++) {
        if (tag + i < MEMORY_ADDR_TOTAL) {
          this->lines[index].data[i] = memory[tag + i];
        }
      }
    }
  }
};

std::vector<unsigned long> init_memory() {
  std::vector<unsigned long> memory;
  memory.resize(MEMORY_ADDR_TOTAL); // 2 ^16 addresses
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<std::mt19937::result_type> random(
      0, MEMORY_ADDR_TOTAL);
  for (size_t i = 0; i < memory.size(); i++) {
    memory[i] = random(rng);
  }
  return memory;
}

int main() {
  auto memory = init_memory();
  Cache cache;
  cache.printCache();
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<std::mt19937::result_type> random(
      0, MEMORY_ADDR_TOTAL);

  for (size_t addr = 0; addr < MEMORY_ADDR_TOTAL; addr++) {
    cache.access(random(rng) % MEMORY_ADDR_TOTAL,
                 memory); // Usar random(rng) correctamente
  }

  cache.printCache();
  return 0;
}