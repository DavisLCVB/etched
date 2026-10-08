#pragma once

#include <cstdint>

#ifndef ETCHED_OPTIONS_TRAITS_HPP
#define ETCHED_OPTIONS_TRAITS_HPP

namespace etched::options {

struct OptionsConfig {
  bool allowNegativeNumbers{true};
  bool allowShortJoinedValue{true};
  bool allowLongJoinedValue{true};
  bool allowFlagClustering{true};
  bool allowUnorderedGlobalOption{false};
};

enum class OptionType : uint8_t {
  OPTION = 0,
  FLAG = 1 << 0,
  POSITIONAL = 1 << 1
};

template <OptionType Type>
struct RequiredValueSelector {
  static constexpr bool requiresValue = true;
};

template <>
struct RequiredValueSelector<OptionType::FLAG> {
  static constexpr bool requiresValue = false;
};

// Holds only compile-time properties. It must not declare data members:
// members of a base class cannot be set with designated initializers.
template <OptionType Type>
struct BasicTraits {
  static constexpr bool requiresValue{
      RequiredValueSelector<Type>::requiresValue};
};

enum class RepetitionPolicy : uint8_t {
  THROW_ERROR = 0,
  TAKE_FIRST = 1 << 0,
  TAKE_LAST = 1 << 1,
  COUNT = 1 << 2,
  // Appends each occurrence to the value. Only valid when the value type is a
  // container; each element is deserialized on its own.
  ACCUMULATE = 1 << 3
};

struct OptionTraits : public BasicTraits<OptionType::OPTION> {
  bool isRequired{false};
  RepetitionPolicy repetitionPolicy{RepetitionPolicy::THROW_ERROR};
};

struct FlagTraits : public BasicTraits<OptionType::FLAG> {
  bool isRequired{false};
  bool hasNegativeForm{false};
  RepetitionPolicy repetitionPolicy{RepetitionPolicy::THROW_ERROR};
};

enum class PositionalPolicy : uint8_t {
  SINGLE = 0,
  ZERO_OR_MORE = 1 << 0,
  ONE_OR_MORE = 1 << 1
};

struct PositionalTraits : public BasicTraits<OptionType::POSITIONAL> {
  bool isRequired{false};
  // Error if more than one variadic positional argument is provided (zero or more, one or more)
  PositionalPolicy positionalPolicy{PositionalPolicy::SINGLE};
};

template <OptionType T>
struct TraitSelector {
  using Traits = BasicTraits<T>;
};

template <>
struct TraitSelector<OptionType::OPTION> {
  using Traits = OptionTraits;
};

template <>
struct TraitSelector<OptionType::FLAG> {
  using Traits = FlagTraits;
};

template <>
struct TraitSelector<OptionType::POSITIONAL> {
  using Traits = PositionalTraits;
};

template <OptionType Type>
class Option : public TraitSelector<Type>::Traits {
 public:
  using Traits = typename TraitSelector<Type>::Traits;

  constexpr Option() = default;

  constexpr Option(const Traits& traits) : Traits(traits) {}
};

}  // namespace etched::options

#endif  // ETCHED_OPTIONS_TRAITS_HPP
