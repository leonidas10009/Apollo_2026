/**
 * @file src/backends/linux/uinput_backend.cpp
 * @brief Linux uinput backend for simple virtual gamepads.
 */
#include "apollo/input/backend.hpp"

#include "backends/backend_factory.hpp"

#ifdef __linux__

  #include <linux/input.h>
  #include <linux/uinput.h>

  #include <fcntl.h>
  #include <sys/ioctl.h>
  #include <unistd.h>

  #include <cerrno>
  #include <cstdint>
  #include <cstdio>
  #include <cstring>
  #include <mutex>
  #include <string>
  #include <utility>

namespace apollo::input::detail {
  namespace {
    /**
     * @brief Write one input event.
     *
     * @param fd uinput file descriptor.
     * @param type Event type.
     * @param code Event code.
     * @param value Event value.
     * @return True on success.
     */
    bool emit_event(int fd, std::uint16_t type, std::uint16_t code, std::int32_t value) {
      input_event event {};
      event.type = type;
      event.code = code;
      event.value = value;
      return ::write(fd, &event, sizeof(event)) == static_cast<ssize_t>(sizeof(event));
    }

    /**
     * @brief One virtual gamepad backed by Linux uinput.
     */
    class UinputDevice final: public Device {
    public:
      UinputDevice(Profile profile, std::string name):
          info_ {profile, {}, std::move(name)} {
        fd_ = ::open("/dev/uinput", O_WRONLY | O_NONBLOCK);
        if (fd_ < 0) {
          return;
        }

        ioctl(fd_, UI_SET_EVBIT, EV_KEY);
        ioctl(fd_, UI_SET_EVBIT, EV_ABS);
        for (const auto code : {
               BTN_A, BTN_B, BTN_X, BTN_Y,
               BTN_TL, BTN_TR, BTN_START, BTN_SELECT, BTN_MODE,
               BTN_THUMBL, BTN_THUMBR,
               BTN_DPAD_UP, BTN_DPAD_DOWN, BTN_DPAD_LEFT, BTN_DPAD_RIGHT}) {
          ioctl(fd_, UI_SET_KEYBIT, code);
        }

        for (const auto code : {ABS_X, ABS_Y, ABS_RX, ABS_RY, ABS_Z, ABS_RZ, ABS_HAT0X, ABS_HAT0Y}) {
          ioctl(fd_, UI_SET_ABSBIT, code);
        }

        uinput_user_dev device {};
        std::snprintf(device.name, sizeof(device.name), "%s", info_.name.c_str());
        device.id.bustype = BUS_USB;
        device.id.vendor = 0x045E;
        device.id.product = 0x028E;
        device.id.version = 0x0114;

        set_abs(device, ABS_X, -32768, 32767);
        set_abs(device, ABS_Y, -32768, 32767);
        set_abs(device, ABS_RX, -32768, 32767);
        set_abs(device, ABS_RY, -32768, 32767);
        set_abs(device, ABS_Z, 0, 255);
        set_abs(device, ABS_RZ, 0, 255);
        set_abs(device, ABS_HAT0X, -1, 1);
        set_abs(device, ABS_HAT0Y, -1, 1);

        if (::write(fd_, &device, sizeof(device)) != static_cast<ssize_t>(sizeof(device))) {
          ::close(fd_);
          fd_ = -1;
          return;
        }
        if (ioctl(fd_, UI_DEV_CREATE) < 0) {
          ::close(fd_);
          fd_ = -1;
          return;
        }

        connected_ = true;
        sync_();
      }

      ~UinputDevice() override {
        disconnect();
      }

      const DeviceInfo &info() const override {
        return info_;
      }

      bool connected() const override {
        return connected_;
      }

      bool update(const GamepadState &state) override {
        std::scoped_lock lock(mutex_);
        if (!connected_) {
          return false;
        }

        const auto previous = state_;
        state_ = state;

        emit_buttons_(previous.buttons, state.buttons);
        emit_abs_(ABS_X, previous.left_x, state.left_x);
        emit_abs_(ABS_Y, previous.left_y, state.left_y);
        emit_abs_(ABS_RX, previous.right_x, state.right_x);
        emit_abs_(ABS_RY, previous.right_y, state.right_y);
        emit_abs_(ABS_Z, previous.left_trigger, state.left_trigger);
        emit_abs_(ABS_RZ, previous.right_trigger, state.right_trigger);

        const int hat_x = (state.pressed(Button::dpad_right) ? 1 : 0) - (state.pressed(Button::dpad_left) ? 1 : 0);
        const int hat_y = (state.pressed(Button::dpad_down) ? 1 : 0) - (state.pressed(Button::dpad_up) ? 1 : 0);
        emit_abs_(ABS_HAT0X, last_hat_x_, hat_x);
        emit_abs_(ABS_HAT0Y, last_hat_y_, hat_y);
        last_hat_x_ = hat_x;
        last_hat_y_ = hat_y;

        return sync_();
      }

      void disconnect() override {
        std::scoped_lock lock(mutex_);
        if (!connected_) {
          return;
        }
        connected_ = false;
        ioctl(fd_, UI_DEV_DESTROY);
        ::close(fd_);
        fd_ = -1;
      }

    private:
      /**
       * @brief Configure one absolute axis.
       */
      static void set_abs(uinput_user_dev &device, std::uint16_t code, int minimum, int maximum) {
        device.absmin[code] = minimum;
        device.absmax[code] = maximum;
      }

      /**
       * @brief Emit changed digital buttons.
       */
      void emit_buttons_(std::uint32_t previous, std::uint32_t current) {
        struct mapping_t {
          Button button;
          std::uint16_t code;
        };
        static constexpr mapping_t mappings[] {
          {Button::a, BTN_A},
          {Button::b, BTN_B},
          {Button::x, BTN_X},
          {Button::y, BTN_Y},
          {Button::left_shoulder, BTN_TL},
          {Button::right_shoulder, BTN_TR},
          {Button::start, BTN_START},
          {Button::back, BTN_SELECT},
          {Button::guide, BTN_MODE},
          {Button::left_thumb, BTN_THUMBL},
          {Button::right_thumb, BTN_THUMBR},
          {Button::dpad_up, BTN_DPAD_UP},
          {Button::dpad_down, BTN_DPAD_DOWN},
          {Button::dpad_left, BTN_DPAD_LEFT},
          {Button::dpad_right, BTN_DPAD_RIGHT},
        };

        for (const auto &mapping : mappings) {
          const bool was_pressed = (previous & button_bit(mapping.button)) != 0;
          const bool is_pressed = (current & button_bit(mapping.button)) != 0;
          if (was_pressed != is_pressed) {
            emit_event(fd_, EV_KEY, mapping.code, is_pressed ? 1 : 0);
          }
        }
      }

      /**
       * @brief Emit an absolute axis when it changes.
       */
      void emit_abs_(std::uint16_t code, std::int32_t previous, std::int32_t current) {
        if (previous != current) {
          emit_event(fd_, EV_ABS, code, current);
        }
      }

      /**
       * @brief Flush the report.
       */
      bool sync_() {
        return emit_event(fd_, EV_SYN, SYN_REPORT, 0);
      }

      int fd_ = -1;
      DeviceInfo info_;
      GamepadState state_;
      int last_hat_x_ = 0;
      int last_hat_y_ = 0;
      std::mutex mutex_;
      bool connected_ = false;
    };

    /**
     * @brief Linux uinput backend factory.
     */
    class UinputBackend final: public Backend {
    public:
      const char *name() const override {
        return "uinput";
      }

      bool available(std::string *error) const override {
        if (::access("/dev/uinput", W_OK) == 0) {
          if (error) {
            error->clear();
          }
          return true;
        }
        if (error) {
          *error = std::string {"/dev/uinput is not writable: "} + std::strerror(errno);
        }
        return false;
      }

      std::unique_ptr<Device> create(Profile profile, const std::string &name, std::string *error) override {
        auto device = std::make_unique<UinputDevice>(profile, name);
        if (!device->connected()) {
          if (error) {
            *error = "Unable to create a uinput device (check /dev/uinput permissions)";
          }
          return nullptr;
        }
        if (error) {
          error->clear();
        }
        return device;
      }

      std::vector<Profile> supported_profiles() const override {
        return {Profile::xbox_360, Profile::dualshock4, Profile::generic};
      }
    };
  }  // namespace

  std::unique_ptr<Backend> create_uinput_backend() {
    return std::make_unique<UinputBackend>();
  }
}  // namespace apollo::input::detail

#endif
