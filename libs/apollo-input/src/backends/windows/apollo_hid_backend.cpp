/**
 * @file src/backends/windows/apollo_hid_backend.cpp
 * @brief Apollo HID (UMDF2 + VHF) backend for Windows virtual controllers.
 *
 * This backend talks to the Apollo HID driver through apollo-hid-client. Each
 * virtual device owns its own control handle, so controllers are isolated per
 * file object in the driver and closing one device never removes another.
 */
#include "apollo/input/backend.hpp"

#include "backends/backend_factory.hpp"

#ifdef _WIN32

  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif

  #include <windows.h>

  #include <apollo/hid/client.hpp>
  #include <apollo/hid/driver_version.hpp>
  #include <apollo/hid/profile.hpp>
  #include <apollo/hid/report.hpp>
  #include <apollo/hid/slots.hpp>

  #include <algorithm>
  #include <array>
  #include <atomic>
  #include <chrono>
  #include <cmath>
  #include <condition_variable>
  #include <cstdint>
  #include <cstdio>
  #include <memory>
  #include <mutex>
  #include <optional>
  #include <span>
  #include <string>
  #include <thread>
  #include <utility>
  #include <vector>

namespace apollo::input::detail {
  namespace {

    namespace hid = apollo::hid;

    /// Conversion factor from radians per second to DS4 gyro units.
    constexpr float DS4_GYRO_SCALE = 16.0F * 57.2957795F;

    /// Conversion factor from meters per second squared to DS4 accelerometer units.
    constexpr float DS4_ACCEL_SCALE = 8192.0F / 9.80665F;

    /// Feedback polling interval in milliseconds.
    constexpr int FEEDBACK_POLL_INTERVAL_MS = 8;

    /**
     * @brief Reject an old or unreadable driver before opening a control handle.
     *
     * The version is read from the PnP device property, not from the control
     * handle, so probing driver 0.1.0.5 cannot trigger its global cleanup.
     *
     * @param error Optional human-readable reason.
     * @return True when the installed driver is new enough.
     */
    bool ensure_supported_driver(std::string *error) {
      const auto version = hid::installed_driver_version();
      if (!version) {
        if (error) {
          *error = "Apollo HID driver is not installed or its version cannot be read; install 0.1.0.6 or newer";
        }
        return false;
      }
      if (!hid::supports_current_protocol(*version)) {
        if (error) {
          *error = "Apollo HID driver " + hid::format_driver_version(*version) +
                   " is too old; install 0.1.0.6 or newer";
        }
        return false;
      }
      return true;
    }

    /**
     * @brief Map an ApolloInput profile to an Apollo HID profile.
     *
     * Only profiles with a native HID descriptor are mapped. Other profiles
     * intentionally fail so Backend::create_compatible() can walk the
     * compatibility chain.
     *
     * @param profile ApolloInput profile.
     * @return Apollo HID profile, or std::nullopt when not implemented.
     */
    std::optional<hid::Profile> to_hid_profile(Profile profile) {
      switch (profile) {
        case Profile::generic:
          return hid::Profile::generic;
        case Profile::dualshock4:
          return hid::Profile::dualshock4;
        case Profile::dualsense:
          return hid::Profile::dualsense;
        case Profile::dualsense_edge:
          return hid::Profile::dualsense_edge;
        case Profile::xbox_one:
          return hid::Profile::xbox_one;
        case Profile::xbox_series:
          return hid::Profile::xbox_series;
        case Profile::xbox_elite:
          return hid::Profile::xbox_elite;
        case Profile::switch_pro:
          return hid::Profile::switch_pro;
        case Profile::steam_controller:
          return hid::Profile::steam_controller;
        case Profile::steam_deck:
          return hid::Profile::steam_deck;
        default:
          return std::nullopt;
      }
    }

    /**
     * @brief Map an Apollo HID profile back to an ApolloInput profile.
     *
     * @param profile Apollo HID profile.
     * @return ApolloInput profile, or std::nullopt when unknown.
     */
    std::optional<Profile> to_input_profile(hid::Profile profile) {
      switch (profile) {
        case hid::Profile::generic:
          return Profile::generic;
        case hid::Profile::dualshock4:
          return Profile::dualshock4;
        case hid::Profile::xbox_360:
          return Profile::xbox_360;
        case hid::Profile::xbox_one:
          return Profile::xbox_one;
        case hid::Profile::xbox_series:
          return Profile::xbox_series;
        case hid::Profile::xbox_elite:
          return Profile::xbox_elite;
        case hid::Profile::dualsense:
          return Profile::dualsense;
        case hid::Profile::dualsense_edge:
          return Profile::dualsense_edge;
        case hid::Profile::switch_pro:
          return Profile::switch_pro;
        case hid::Profile::steam_controller:
          return Profile::steam_controller;
        case hid::Profile::steam_deck:
          return Profile::steam_deck;
      }
      return std::nullopt;
    }

    /**
     * @brief Translate the ApolloInput button mask to the Apollo HID mask.
     *
     * @param buttons ApolloInput button bit mask.
     * @return Apollo HID button bit mask.
     */
    std::uint32_t to_hid_buttons(std::uint32_t buttons) {
      std::uint32_t result = 0;
      const auto set = [buttons, &result](Button source, hid::Button target) {
        if ((buttons & button_bit(source)) != 0) {
          result |= static_cast<std::uint32_t>(target);
        }
      };

      set(Button::a, hid::Button::a);
      set(Button::b, hid::Button::b);
      set(Button::x, hid::Button::x);
      set(Button::y, hid::Button::y);
      set(Button::dpad_up, hid::Button::dpad_up);
      set(Button::dpad_down, hid::Button::dpad_down);
      set(Button::dpad_left, hid::Button::dpad_left);
      set(Button::dpad_right, hid::Button::dpad_right);
      set(Button::start, hid::Button::start);
      set(Button::options, hid::Button::start);
      set(Button::back, hid::Button::back);
      set(Button::share, hid::Button::back);
      set(Button::guide, hid::Button::guide);
      set(Button::left_thumb, hid::Button::left_stick);
      set(Button::right_thumb, hid::Button::right_stick);
      set(Button::left_shoulder, hid::Button::left_shoulder);
      set(Button::right_shoulder, hid::Button::right_shoulder);
      set(Button::touchpad, hid::Button::misc1);
      set(Button::touchpad_click, hid::Button::misc1);
      set(Button::paddle1, hid::Button::paddle1);
      set(Button::paddle2, hid::Button::paddle2);
      set(Button::paddle3, hid::Button::paddle3);
      set(Button::paddle4, hid::Button::paddle4);
      set(Button::misc2, hid::Button::misc2);
      return result;
    }

    /**
     * @brief Convert an ApolloInput gamepad state to the HID state.
     *
     * @param state ApolloInput state.
     * @return Apollo HID state.
     */
    hid::GamepadState to_hid_state(const GamepadState &state) {
      hid::GamepadState result {};
      result.buttons = to_hid_buttons(state.buttons);
      result.left_x = state.left_x;
      result.left_y = state.left_y;
      result.right_x = state.right_x;
      result.right_y = state.right_y;
      result.left_trigger = state.left_trigger;
      result.right_trigger = state.right_trigger;
      result.hat = 8;
      result.battery_level = 10;
      result.battery_cable = true;
      return result;
    }

    /**
     * @brief Map driver capability bits to the ApolloInput capability model.
     *
     * The profile matrix defines the maximum feature set; the device info from
     * the driver defines what this build actually exposes.
     *
     * @param info Driver device information.
     * @param profile Actual ApolloInput profile.
     * @return Effective capabilities.
     */
    Capabilities to_capabilities(const hid::DeviceInfo &info, Profile profile) {
      auto capabilities = profile_capabilities(profile);
      const auto bits = hid::capabilities_from_bits(info.capabilities);

      capabilities.rumble = hid::has_capability(bits, hid::Capability::rumble);
      capabilities.leds = hid::has_capability(bits, hid::Capability::rgb_led);
      capabilities.imu = hid::has_capability(bits, hid::Capability::imu);
      capabilities.touchpad = hid::has_capability(bits, hid::Capability::touchpad);
      capabilities.adaptive_triggers = hid::has_capability(bits, hid::Capability::adaptive_triggers);
      capabilities.trigger_rumble = hid::has_capability(bits, hid::Capability::trigger_rumble);
      capabilities.player_leds = hid::has_capability(bits, hid::Capability::player_leds);
      capabilities.battery = hid::has_capability(bits, hid::Capability::battery);
      capabilities.paddles = hid::has_capability(bits, hid::Capability::paddles);
      return capabilities;
    }

    /**
     * @brief Generate a process-unique 16-byte controller serial.
     *
     * @return Zero-padded serial.
     */
    std::array<std::uint8_t, 16> next_serial() {
      static std::atomic<std::uint32_t> counter {0};
      const auto sequence = counter.fetch_add(1, std::memory_order_relaxed) + 1;
      const auto value = static_cast<std::uint32_t>(GetCurrentProcessId()) ^ (sequence * 2654435761U);

      std::array<std::uint8_t, 16> serial {};
      std::snprintf(reinterpret_cast<char *>(serial.data()), serial.size(),
                    "APLH-%08X", static_cast<unsigned>(value));
      return serial;
    }

    /**
     * @brief Scale a floating-point sensor value to a clamped DS4 raw value.
     *
     * @param value Sensor value.
     * @param scale Conversion factor.
     * @return Clamped signed 16-bit value.
     */
    std::int16_t sensor_value(float value, float scale) {
      const auto scaled = std::clamp(value * scale, -32768.0F, 32767.0F);
      return static_cast<std::int16_t>(scaled);
    }

    /**
     * @brief One virtual controller backed by the Apollo HID driver.
     */
    class ApolloHidDevice final: public Device {
    public:
      /**
       * @brief Construct a virtual device.
       *
       * @param client Open control handle dedicated to this device.
       * @param slot Slot returned by the driver.
       * @param info Device information exposed by the backend.
       */
      ApolloHidDevice(hid::Client client, std::uint32_t slot, DeviceInfo info):
          client_(std::move(client)),
          slot_(slot),
          info_(std::move(info)) {
        if (info_.capabilities.rumble || info_.capabilities.leds ||
            info_.capabilities.player_leds || info_.capabilities.adaptive_triggers) {
          feedback_thread_ = std::thread(&ApolloHidDevice::feedback_loop, this);
        }
      }

      ~ApolloHidDevice() override {
        disconnect();
      }

      ApolloHidDevice(const ApolloHidDevice &) = delete;
      ApolloHidDevice &operator=(const ApolloHidDevice &) = delete;

      const DeviceInfo &info() const override {
        return info_;
      }

      bool connected() const override {
        return connected_.load(std::memory_order_relaxed);
      }

      bool update(const GamepadState &state) override {
        if (!connected()) {
          return false;
        }

        auto sanitized = state;
        AnalogDeadzone deadzone;
        {
          std::lock_guard lock(state_mutex_);
          deadzone = deadzone_;
        }
        sanitize_gamepad_state(sanitized, deadzone);

        hid::GamepadState report;
        {
          std::lock_guard lock(state_mutex_);
          auto next = to_hid_state(sanitized);
          next.imu = state_.imu;
          next.touchpad = state_.touchpad;
          next.timestamp = ++report_timestamp_;
          state_ = next;
          report = state_;
        }
        return submit(report);
      }

      void set_deadzone(const AnalogDeadzone &deadzone) override {
        std::lock_guard lock(state_mutex_);
        deadzone_ = deadzone;
      }

      bool update_touchpad(const TouchpadState &state) override {
        if (!connected() || !info_.capabilities.touchpad) {
          return false;
        }

        hid::GamepadState report;
        {
          std::lock_guard lock(state_mutex_);
          const auto timestamp = ++report_timestamp_;
          auto &touchpad = state_.touchpad;
          touchpad.points[0].active = state.active;
          touchpad.points[0].id = 0;
          touchpad.points[0].x = state.x;
          touchpad.points[0].y = state.y;
          touchpad.points[1] = {};
          touchpad.packet_counter = static_cast<std::uint8_t>(++touch_packet_);
          touchpad.timestamp = static_cast<std::uint8_t>(timestamp);
          state_.timestamp = timestamp;
          report = state_;
        }
        return submit(report);
      }

      bool update_imu(const ImuSample &sample) override {
        if (!connected() || !info_.capabilities.imu) {
          return false;
        }

        hid::GamepadState report;
        {
          std::lock_guard lock(state_mutex_);
          state_.imu.gyro_x = sensor_value(sample.gyro_x, DS4_GYRO_SCALE);
          state_.imu.gyro_y = sensor_value(sample.gyro_y, DS4_GYRO_SCALE);
          state_.imu.gyro_z = sensor_value(sample.gyro_z, DS4_GYRO_SCALE);
          state_.imu.accel_x = sensor_value(sample.accel_x, DS4_ACCEL_SCALE);
          state_.imu.accel_y = sensor_value(sample.accel_y, DS4_ACCEL_SCALE);
          state_.imu.accel_z = sensor_value(sample.accel_z, DS4_ACCEL_SCALE);
          state_.timestamp = ++report_timestamp_;
          report = state_;
        }
        return submit(report);
      }

      void set_feedback_callback(FeedbackCallback callback) override {
        {
          std::lock_guard lock(feedback_mutex_);
          feedback_ = std::move(callback);
        }
        feedback_cv_.notify_all();
      }

      void disconnect() override {
        if (!connected_.exchange(false)) {
          return;
        }

        {
          std::lock_guard lock(feedback_mutex_);
          feedback_running_ = false;
        }
        feedback_cv_.notify_all();
        if (feedback_thread_.joinable()) {
          feedback_thread_.join();
        }

        std::lock_guard io_lock(io_mutex_);
        hid::Error error = hid::Error::none;
        client_.destroy(slot_, &error);
        client_.close();
      }

    private:
      /**
       * @brief Encode and submit one complete HID report.
       *
       * @param state HID state.
       * @return True when the driver accepted the report.
       */
      bool submit(const hid::GamepadState &state) {
        if (!connected()) {
          return false;
        }

        const auto hid_profile = to_hid_profile(info_.profile);
        if (!hid_profile) {
          return false;
        }
        const auto report = hid::encode_report(*hid_profile, state);

        std::lock_guard io_lock(io_mutex_);
        hid::Error error = hid::Error::none;
        return client_.submit(slot_, report, &error);
      }

      /**
       * @brief Poll driver output reports and translate them to feedback.
       */
      void feedback_loop() {
        std::unique_lock lock(feedback_mutex_);
        while (feedback_running_) {
          if (!feedback_) {
            feedback_cv_.wait(lock, [this] {
              return !feedback_running_ || static_cast<bool>(feedback_);
            });
            continue;
          }

          auto callback = feedback_;
          lock.unlock();
          poll_feedback(callback);
          lock.lock();
          feedback_cv_.wait_for(lock, std::chrono::milliseconds(FEEDBACK_POLL_INTERVAL_MS), [this] {
            return !feedback_running_;
          });
        }
      }

      /**
       * @brief Read one pending output report and invoke the callback.
       *
       * @param callback Callback copied by the feedback loop.
       */
      void poll_feedback(const FeedbackCallback &callback) {
        hid::Error error = hid::Error::none;
        std::optional<hid::OutputEvent> output;
        {
          std::lock_guard io_lock(io_mutex_);
          output = client_.read_output(slot_, &error);
        }
        if (!output) {
          return;
        }

        const auto hid_profile = to_hid_profile(info_.profile);
        if (!hid_profile) {
          return;
        }
        const auto parsed = hid::parse_feedback(
          *hid_profile, std::span<const std::uint8_t>(output->report, output->report_size));
        if (!parsed) {
          return;
        }

        if (parsed->low_frequency_rumble != 0 || parsed->high_frequency_rumble != 0 ||
            parsed->left_trigger_rumble != 0 || parsed->right_trigger_rumble != 0) {
          Feedback rumble;
          rumble.type = FeedbackType::rumble;
          rumble.low_frequency = static_cast<std::uint16_t>(parsed->low_frequency_rumble) * 257U;
          rumble.high_frequency = static_cast<std::uint16_t>(parsed->high_frequency_rumble) * 257U;
          rumble.left_trigger_rumble =
            static_cast<std::uint16_t>(parsed->left_trigger_rumble) * 257U;
          rumble.right_trigger_rumble =
            static_cast<std::uint16_t>(parsed->right_trigger_rumble) * 257U;
          callback(rumble);
        }

        if (parsed->lightbar || parsed->player_leds != 0) {
          Feedback led;
          led.type = FeedbackType::led;
          led.led_red = parsed->red;
          led.led_green = parsed->green;
          led.led_blue = parsed->blue;
          led.player_leds = parsed->player_leds;
          callback(led);
        }

        if (parsed->trigger_mode != hid::AdaptiveTriggerMode::none) {
          Feedback adaptive;
          adaptive.type = FeedbackType::adaptive_trigger;
          adaptive.trigger_mode = static_cast<std::uint8_t>(parsed->trigger_mode);
          adaptive.trigger = 0;
          callback(adaptive);
        }
      }

      hid::Client client_;  ///< Dedicated control handle.
      std::uint32_t slot_ {};  ///< Driver slot.
      DeviceInfo info_;  ///< Public device information.
      std::atomic<bool> connected_ {true};  ///< Lifecycle state.
      std::mutex io_mutex_;  ///< Serializes submit/read/destroy on the handle.
      std::mutex state_mutex_;  ///< Protects state_, deadzone_ and counters.
      hid::GamepadState state_ {};  ///< Latest merged HID state.
      AnalogDeadzone deadzone_ {};  ///< Analog filter configuration.
      std::uint16_t report_timestamp_ {};  ///< DS4 report timestamp.
      std::uint8_t touch_packet_ {};  ///< DS4 touch packet counter.
      std::mutex feedback_mutex_;  ///< Protects callback and worker flags.
      std::condition_variable feedback_cv_;  ///< Wakes the feedback worker.
      FeedbackCallback feedback_;  ///< User callback.
      bool feedback_running_ {true};  ///< Worker lifecycle flag.
      std::thread feedback_thread_;  ///< Output polling thread.
    };

    /**
     * @brief Apollo HID backend factory.
     */
    class ApolloHidBackend final: public Backend {
    public:
      const char *name() const override {
        return "apollo_hid";
      }

      bool available(std::string *error) const override {
        if (!ensure_supported_driver(error)) {
          return false;
        }

        hid::Error open_error = hid::Error::none;
        auto client = hid::Client::open(&open_error);
        if (!client) {
          if (error) {
            *error = std::string {hid::error_message(open_error)};
          }
          return false;
        }

        const auto version = client->version(&open_error);
        if (!version) {
          if (error) {
            *error = std::string {hid::error_message(open_error)};
          }
          return false;
        }
        if (!hid::is_compatible_version(*version)) {
          if (error) {
            *error = "Apollo HID protocol version mismatch: driver " +
                     std::to_string(version->protocol_version) + ", client " +
                     std::to_string(hid::PROTOCOL_VERSION);
          }
          return false;
        }
        if (version->max_report_size < hid::DUALSHOCK4_REPORT_SIZE) {
          if (error) {
            *error = "Apollo HID driver report size is too small";
          }
          return false;
        }

        if (error) {
          error->clear();
        }
        return true;
      }

      std::unique_ptr<Device> create(Profile profile, const std::string &name, std::string *error) override {
        const auto hid_profile = to_hid_profile(profile);
        if (!hid_profile) {
          if (error) {
            *error = std::string {"Apollo HID does not implement the "} + to_string(profile) +
                     " profile; use create_compatible() to fall back";
          }
          return nullptr;
        }

        if (!ensure_supported_driver(error)) {
          return nullptr;
        }

        hid::Error hid_error = hid::Error::none;
        auto client = hid::Client::open(&hid_error);
        if (!client) {
          if (error) {
            *error = std::string {hid::error_message(hid_error)};
          }
          return nullptr;
        }

        const auto version = client->version(&hid_error);
        if (!version || !hid::is_compatible_version(*version) ||
            version->max_report_size < hid::DUALSHOCK4_REPORT_SIZE) {
          if (error) {
            *error = !version
              ? std::string {hid::error_message(hid_error)}
              : "Apollo HID protocol or report size is incompatible";
          }
          return nullptr;
        }

        const auto serial = next_serial();
        const auto created = client->create_pad(*hid_profile, hid::AUTO_SLOT, serial, &hid_error);
        if (!created) {
          if (error) {
            *error = std::string {hid::error_message(hid_error)};
          }
          return nullptr;
        }

        const auto driver_info = client->device_info(created->slot, &hid_error);
        if (!driver_info) {
          if (error) {
            *error = std::string {hid::error_message(hid_error)};
          }
          client->destroy(created->slot);
          return nullptr;
        }

        const auto actual_hid_profile = static_cast<hid::Profile>(driver_info->profile);
        const auto actual_profile = to_input_profile(actual_hid_profile).value_or(Profile::generic);

        DeviceInfo info;
        info.profile = actual_profile;
        info.capabilities = to_capabilities(*driver_info, actual_profile);
        info.name = name;

        if (error) {
          error->clear();
        }
        return std::make_unique<ApolloHidDevice>(std::move(*client), created->slot, std::move(info));
      }

      std::vector<Profile> supported_profiles() const override {
        std::vector<Profile> profiles;
        for (const auto &profile : hid::supported_profiles()) {
          if (const auto mapped = to_input_profile(profile.profile)) {
            profiles.push_back(*mapped);
          }
        }
        return profiles;
      }
    };
  }  // namespace

  /**
   * @brief Create the Apollo HID backend.
   *
   * @return Apollo HID backend instance.
   */
  std::unique_ptr<Backend> create_apollo_hid_backend() {
    return std::make_unique<ApolloHidBackend>();
  }
}  // namespace apollo::input::detail

#endif
