/**
 * @file tests/core_tests.cpp
 * @brief Dependency-free unit tests for the apollo-input core.
 */
#include "apollo/input/input.hpp"

#include <cstdint>
#include <iostream>
#include <string>

namespace {
  int failures = 0;

  void check(bool condition, const char *expression, const char *file, int line) {
    if (!condition) {
      std::cerr << file << ':' << line << ": check failed: " << expression << '\n';
      ++failures;
    }
  }

  #define CHECK(expression) check((expression), #expression, __FILE__, __LINE__)

  void test_profiles() {
    apollo::input::Profile profile = apollo::input::Profile::generic;
    CHECK(std::string {apollo::input::to_string(apollo::input::Profile::xbox_360)} == "xbox_360");
    CHECK(std::string {apollo::input::to_string(apollo::input::Profile::dualsense)} == "dualsense");
    CHECK(apollo::input::profile_from_string("switch_pro", profile));
    CHECK(profile == apollo::input::Profile::switch_pro);
    CHECK(!apollo::input::profile_from_string("not-a-profile", profile));
  }

  void test_buttons() {
    apollo::input::GamepadState state;
    CHECK(!state.pressed(apollo::input::Button::a));
    state.set(apollo::input::Button::a, true);
    CHECK(state.pressed(apollo::input::Button::a));
    CHECK(!state.pressed(apollo::input::Button::b));
    state.set(apollo::input::Button::a, false);
    CHECK(!state.pressed(apollo::input::Button::a));

    state.left_trigger = 200;
    state.right_trigger = 100;
    state.left_x = 1234;
    CHECK(state.left_trigger == 200);
    CHECK(state.right_trigger == 100);
    CHECK(state.left_x == 1234);
    state.clear();
    CHECK(state.buttons == 0);
    CHECK(state.left_x == 0);
    CHECK(state.left_trigger == 0);
  }

  void test_null_backend() {
    apollo::input::BackendKind kind = apollo::input::BackendKind::automatic;
    CHECK(apollo::input::backend_from_string("null", kind));
    CHECK(kind == apollo::input::BackendKind::null_backend);
    CHECK(std::string {apollo::input::to_string(apollo::input::BackendKind::vigem)} == "vigem");

    auto backend = apollo::input::create_backend(apollo::input::BackendKind::null_backend);
    CHECK(backend != nullptr);
    CHECK(std::string {backend->name()} == "null");

    std::string error;
    CHECK(backend->available(&error));
    CHECK(error.empty());

    auto device = backend->create(apollo::input::Profile::xbox_360, "Test Pad", &error);
    CHECK(device != nullptr);
    CHECK(error.empty());
    CHECK(device->connected());
    CHECK(device->info().profile == apollo::input::Profile::xbox_360);
    CHECK(device->info().name == "Test Pad");

    apollo::input::GamepadState state;
    state.set(apollo::input::Button::a, true);
    CHECK(device->update(state));
    CHECK(!device->update_imu({}));

    device->disconnect();
    CHECK(!device->connected());
    CHECK(!device->update(state));
  }

  void test_unknown_backend_falls_back_to_null() {
    auto backend = apollo::input::create_backend(static_cast<apollo::input::BackendKind>(999));
    CHECK(backend != nullptr);
    CHECK(std::string {backend->name()} == "null");
  }
}  // namespace

int main() {
  test_profiles();
  test_buttons();
  test_null_backend();
  test_unknown_backend_falls_back_to_null();

  if (failures != 0) {
    std::cerr << failures << " check(s) failed\n";
    return 1;
  }

  std::cout << "apollo-input core tests passed\n";
  return 0;
}
