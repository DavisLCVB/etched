#pragma once
#ifndef ETCHED_ARGUMENT_PARSER_HPP
#define ETCHED_ARGUMENT_PARSER_HPP

#include "config.hpp"
#include "contracts.hpp"
#include "orchestrator.hpp"
#include "strings.hpp"

namespace etched {

//TODO Document inner components

template <detail::IsOrchestratorConfig Conf = DefaultConfig,
          detail::IsArgument... Options>
class ArgumentParser {
  static_assert(sizeof...(Options) <= Conf::maxArgs,
                "Number of options exceeds maximum configured arguments");

  using OrchestratorType = DefaultOrchestrator<Conf, Options...>;

 public:
  consteval ArgumentParser(std::string_view appName,
                           std::string_view appDescription,  // NOLINT
                           Options... opts)
      : name_(appName),
        description_(appDescription),
        orchestrator_(appName, appDescription, opts...) {}

  [[nodiscard]] auto parse(const int argc, const char* argv[])  //NOLINT
      -> Result<Output> {                                       // NOLINT
    if (argc > static_cast<int>(Conf::maxArgs)) {
      return err<Output>("Exceeded maximum number of arguments");
    }
    return orchestrator_.parse(argc, argv);
  }

  [[nodiscard]] auto name() const -> std::string_view { return name_; }

  [[nodiscard]] auto description() const -> std::string_view {
    return description_;
  }

  [[nodiscard]] auto help() const -> std::string_view {
    return orchestrator_.helpText();
  }

  template <detail::String Tag>
  [[nodiscard]] auto getOption() const -> const auto& {
    return orchestrator_.template getOption<Tag>();
  }

  template <detail::String Tag>
  [[nodiscard]] auto get() const -> decltype(auto) {
    return orchestrator_.template get<Tag>();
  }

  template <detail::String Tag>
  [[nodiscard]] auto has() const -> bool {
    return orchestrator_.template has<Tag>();
  }

  [[nodiscard]] auto getOptions() { return orchestrator_.getOptions(); }

 private:
  std::string_view name_;
  std::string_view description_;
  OrchestratorType orchestrator_;
};

}  // namespace etched

#endif  // ETCHED_ARGUMENT_PARSER_HPP
