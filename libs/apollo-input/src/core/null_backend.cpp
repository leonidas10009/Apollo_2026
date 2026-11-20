/**
 * @file src/core/null_backend.cpp
 * @brief No-op backend for tests, previews, and unsupported platforms.
 */
#include "apollo/input/backend.hpp"

#include "backends/backend_factory.hpp"

#include <array>
#include <utility>

namespace apollo::input::detail {
  namespace {
    class NullDevice final: public Device {
    public:
      NullDevice(Profile profile, std::string name):
          info_ {profile, {}, std::move(name)} {
      }

      const DeviceInfo &info() const override {
        return info_;
      }

      bool connected() const override {
        return connected_;
      }

      bool update(const GamepadState &state) override {
        state_ = state;
        return connected_;
      }

      void disconnect() override {
        connected_ = false;
      }

    private:
      DeviceInfo info_;
      GamepadState state_ {};
      bool connected_ = true;
    };

    class NullBackend final: public Backend {
    public:
      const char *name() const override {
        return "null";
      }

      bool available(std::string *error) const override {
        if (error) {
          error->clear();
        }
        return true;
      }

      std::unique_ptr<Device> create(Profile profile, const std::string &name, std::string *error) override {
        if (error) {
          error->clear();
        }
        return std::make_unique<NullDevice>(profile, name);
      }

      std::vector<Profile> supported_profiles() const override {
        return {
          Profile::xbox_360,
          Profile::xbox_one,
          Profile::xbox_series,
          Profile::dualshock4,
          Profile::dualsense,
          Profile::switch_pro,
          Profile::generic,
        };
      }
    };
  }  // namespace

  std::unique_ptr<Backend> create_null_backend() {
    return std::make_unique<NullBackend>();
  }
}  // namespace apollo::input::detail
