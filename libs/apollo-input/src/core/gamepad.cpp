/**
 * @file src/core/gamepad.cpp
 * @brief Profile names and parsing helpers.
 */
#include "apollo/input/gamepad.hpp"

#include <array>
#include <string_view>

namespace apollo::input {
  namespace {
    struct profile_name_t {
      Profile profile;
      std::string_view name;
    };

    constexpr std::array PROFILE_NAMES {
      profile_name_t {Profile::xbox_360, "xbox_360"},
      profile_name_t {Profile::xbox_one, "xbox_one"},
      profile_name_t {Profile::xbox_series, "xbox_series"},
      profile_name_t {Profile::dualshock4, "dualshock4"},
      profile_name_t {Profile::dualsense, "dualsense"},
      profile_name_t {Profile::switch_pro, "switch_pro"},
      profile_name_t {Profile::generic, "generic"},
    };
  }  // namespace

  const char *to_string(Profile profile) {
    for (const auto &entry : PROFILE_NAMES) {
      if (entry.profile == profile) {
        return entry.name.data();
      }
    }
    return "generic";
  }

  bool profile_from_string(const std::string &name, Profile &profile) {
    for (const auto &entry : PROFILE_NAMES) {
      if (entry.name == name) {
        profile = entry.profile;
        return true;
      }
    }
    return false;
  }
}  // namespace apollo::input
