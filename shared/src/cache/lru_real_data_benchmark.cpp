#include <benchmark/benchmark.h>
#include <userver/cache/lru_map.hpp>
#include <userver/cache/lfu_map.hpp>
#include <userver/cache/impl/slru.hpp>

#include <userver/fs/blocking/read.hpp>

USERVER_NAMESPACE_BEGIN

namespace {

using Lru = cache::LruMap<std::string, unsigned>;
using Lfu = LfuBase<std::string, unsigned>;
using Slru = cache::LruMap<std::string, unsigned, std::hash<std::string>, std::equal_to<std::string>, cache::CachePolicy::kSLRU>;
using TinyLfu = cache::LruMap<std::string, unsigned, std::hash<std::string>, std::equal_to<std::string>, cache::CachePolicy::kTinyLFU>;

constexpr unsigned kElementsCount = 5000;

  std::vector<std::string> LoadData(std::string file) {
    auto rows = userver::fs::blocking::ReadFileContents(file);
    std::vector<std::string> res;

    std::istringstream is(rows);
    std::string key;
    while (is >> key) {
      res.push_back(key);
    }
    return res;
  }
} // namespace

template <const char* file, typename CachePolicyContainer>
void OnRealData(benchmark::State& state) {
  auto data = LoadData(file);
  auto lru = CachePolicyContainer(kElementsCount);
  for (auto _ : state) {
    for (auto& key : data) {
      if (!lru.Get(key)) {
        lru.Put(key, 1);
      }
    }
  }
}
static const char phoenix[] = "/home/st/c/userver/test_data/phoenix";
static const char goblet[] = "/home/st/c/userver/test_data/goblet";
BENCHMARK(OnRealData<phoenix, Lru>);
BENCHMARK(OnRealData<goblet, Lru>);
BENCHMARK(OnRealData<phoenix, Lfu>);
BENCHMARK(OnRealData<goblet, Lfu>);
BENCHMARK(OnRealData<phoenix, Slru>);
BENCHMARK(OnRealData<goblet, Slru>);
BENCHMARK(OnRealData<phoenix, TinyLfu>);
BENCHMARK(OnRealData<goblet, TinyLfu>);

USERVER_NAMESPACE_END