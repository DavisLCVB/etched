#ifndef ETCHED_SYMBOL_TABLE_HPP
#define ETCHED_SYMBOL_TABLE_HPP

#include <array>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <utility>

#include "contracts.hpp"
#include "strings.hpp"
#include "types.hpp"

namespace etched::detail {

template <IsHashConfig Config>
struct Hash {
  static constexpr uint32_t initHash = Config::initHash;
  static constexpr uint32_t prime = Config::prime;
  static constexpr uint32_t saltMultiplier = Config::saltMultiplier;
  static constexpr uint32_t avalancheConstant01 = 0x85ebca6b;
  static constexpr uint32_t avalancheConstant02 = 0xc2b2ae35;
  static constexpr uint32_t avalancheShift1 = 16;
  static constexpr uint32_t avalancheShift2 = 13;

  [[nodiscard]] static constexpr auto salted(std::string_view str,
                                             uint32_t salt) -> uint32_t {
    uint32_t h = initHash ^ (salt * saltMultiplier);
    for (char c : str) {
      h ^= static_cast<uint32_t>(c);
      h *= prime;
    }
    h ^= h >> avalancheShift1;
    h *= avalancheConstant01;
    h ^= h >> avalancheShift2;
    h *= avalancheConstant02;
    h ^= h >> avalancheShift1;
    return h;
  }
};

template <size_t Capacity, IsSymbolTableConfig Config>
struct PerfectSymbolTable {
  struct Entry {
    BoundedString<Config::maxStringSize> key;  // NOLINT
    EntryMetadata metadata{};
    bool occupied{false};
  };

  using HashType = Hash<typename Config::hashConfig>;

  std::array<Entry, Capacity> table{};
  uint32_t salt = 0;
  bool saltFound = false;
  Optional<size_t> positionalIndex_;  // NOLINT

  [[nodiscard]] constexpr auto positionalIndex() const
      -> Optional<size_t> {  // NOLINT
    return positionalIndex_;
  }

  [[nodiscard]] constexpr auto find(std::string_view key) const
      -> Optional<EntryMetadata> {
    if constexpr (Capacity == 0) {
      return none<EntryMetadata>();
    }
    uint32_t h = HashType::salted(key, salt);
    const auto& entry = table[h % Capacity];

    if (entry.occupied && entry.key == key) {
      return some(entry.metadata);
    }
    return none<EntryMetadata>();
  }

  consteval auto insert(std::string_view key, EntryMetadata metadata) -> bool {
    uint32_t h = HashType::salted(key, salt);
    size_t idx = h % Capacity;
    if (table[idx].occupied) {  // NOLINT
      return false;
    }
    table[idx] = {BoundedString<Config::maxStringSize>(key), metadata,
                  true};  // NOLINT
    return true;
  }

  template <IsArgument Option>
  [[nodiscard]] consteval static auto getEntryType() -> EntryType {
    if constexpr (IsPositionalOption<Option>) {
      return EntryType::POSITIONAL;
    } else if constexpr (IsOptionFlag<Option>) {
      return EntryType::FLAG;
    } else if constexpr (IsOptionValue<Option>) {
      return EntryType::OPTION;
    } else if constexpr (IsCommand<Option>) {
      return EntryType::COMMAND;
    } else {
      return EntryType::UNKNOWN;
    }
  }

  template <IsArgument... Options>
  [[nodiscard]] consteval static auto create(  // NOLINT
      const std::tuple<Options...>& options)   // NOLINT
      -> PerfectSymbolTable<Capacity, Config> {
    PerfectSymbolTable<Capacity, Config> st{};

    struct KeyWithMetadata {
      BoundedString<Config::maxStringSize> key;
      EntryMetadata metadata{};
    };

    constexpr size_t maxKeysLimit = Config::maxKeysLimit;
    constexpr uint32_t maxSaltAttempts = Config::maxSaltAttempts;

    KeyWithMetadata allKeys[maxKeysLimit]{};  // NOLINT
    size_t keyCount = 0;
    using BoundedStringSized = BoundedString<Config::maxStringSize>;

    auto collect =
        [&]<size_t... Is>(std::index_sequence<Is...>) -> auto {  // NOLINT
      auto processOption = [&](auto i, const auto& opt) -> auto {
        using OptType = std::decay_t<decltype(opt)>;
        auto addKey = [&](std::string_view k, EntryType type) -> auto {
          if (keyCount < maxKeysLimit) {
            allKeys[keyCount++] = {BoundedStringSized(k), {size_t(i), type}};
          }
        };

        if constexpr (IsPositionalOption<OptType>) {
          if (st.positionalIndex_.hasValue()) {
            throw CompileError(
                "Only one positional argument collector allowed");
          }
          st.positionalIndex_ = some(size_t(i));
        } else if constexpr (IsOption<OptType>) {
          bool isFlag = IsOptionFlag<OptType>;
          EntryType entryType = isFlag ? EntryType::FLAG : EntryType::OPTION;
          if (opt.shortName != '\0') {
            addKey(std::string_view(&opt.shortName, 1), entryType);
          }
          if (!opt.longName.empty()) {
            addKey(opt.longName, entryType);
          }
        } else if constexpr (IsCommand<OptType>) {
          if (!opt.name.empty()) {
            addKey(opt.name, EntryType::COMMAND);
          }
        }
      };
      (processOption(Is, std::get<Is>(options)), ...);
    };
    collect(std::make_index_sequence<sizeof...(Options)>{});

    if (keyCount == 0) {
      st.saltFound = true;
      return st;
    }

    bool found = false;
    for (uint32_t s = 0; s < maxSaltAttempts; ++s) {
      bool collision = false;
      bool occupied[Capacity]{};  // NOLINT

      for (size_t i = 0; i < keyCount; ++i) {
        uint32_t h = HashType::salted(allKeys[i].key.view(), s) % Capacity;
        if (occupied[h]) {
          collision = true;
          break;
        }
        occupied[h] = true;
      }
      if (!collision) {
        st.salt = s;
        found = true;
        break;
      }
    }

    if (!found) {
      throw CompileError(  // NOLINT
          "Could not find a perfect hash salt. Try increasing Capacity.");
    }

    for (size_t i = 0; i < keyCount; ++i) {
      uint32_t h = HashType::salted(allKeys[i].key.view(), st.salt) % Capacity;
      st.table[h] = {allKeys[i].key, allKeys[i].metadata, true};
    }
    st.saltFound = true;
    return st;
  }
};

constexpr size_t minimunSimbolTableCapacity = 64;
constexpr size_t symbolTableExtraCapacity = 16;

template <size_t Capacity, IsSymbolTableConfig Config>
using DefaultSymbolTable =
    PerfectSymbolTable<(Capacity < minimunSimbolTableCapacity)
                           ? minimunSimbolTableCapacity
                           : (size_t(Capacity* Config::factor) +
                              symbolTableExtraCapacity),
                       Config>;

}  // namespace etched::detail

#endif  // ETCHED_SYMBOL_TABLE_HPP
