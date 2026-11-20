/**
 * @file src/core/backend.cpp
 * @brief Backend selection and name helpers.
 */
#include "apollo/input/backend.hpp"

#include "backends/backend_factory.hpp"

#include <array>
#include <string_view>

namespace apollo::input {
  namespace {
    struct backend_name_t {
      BackendKind kind;
      std::string_view name;
    };

    constexpr std::array BACKEND_NAMES {
      backend_name_t {BackendKind::automatic, "automatic"},
      backend_name_t {BackendKind::vigem, "vigem"},
      backend_name_t {BackendKind::uinput, "uinput"},
      backend_name_t {BackendKind::null_backend, "null"},
    };
  }  // namespace

  const char *to_string(BackendKind kind) {
    for (const auto &entry : BACKEND_NAMES) {
      if (entry.kind == kind) {
        return entry.name.data();
      }
    }
    return "unknown";
  }

  bool backend_from_string(const std::string &name, BackendKind &kind) {
    for (const auto &entry : BACKEND_NAMES) {
      if (entry.name == name) {
        kind = entry.kind;
        return true;
      }
    }
    return false;
  }

  std::unique_ptr<Backend> create_backend(BackendKind kind) {
    if (kind == BackendKind::null_backend) {
      return detail::create_null_backend();
    }

#if defined(_WIN32) && defined(APOLLO_INPUT_WITH_VIGEM)
    if (kind == BackendKind::automatic || kind == BackendKind::vigem) {
      return detail::create_vigem_backend();
    }
#elif defined(__linux__)
    if (kind == BackendKind::automatic || kind == BackendKind::uinput) {
      return detail::create_uinput_backend();
    }
#endif

    return detail::create_null_backend();
  }
}  // namespace apollo::input
