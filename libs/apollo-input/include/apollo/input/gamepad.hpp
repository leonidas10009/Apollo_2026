/**
 * @file include/apollo/input/gamepad.hpp
 * @brief Portable gamepad state, profiles, and capability definitions.
 */
#pragma once

#include <cstdint>
#include <string>

namespace apollo::input {

  /**
   * @brief Virtual controller profile exposed by a backend.
   */
  enum class Profile {
    xbox_360,
    xbox_one,
    xbox_series,
    dualshock4,
    dualsense,
    switch_pro,
    generic,
  };

  /**
   * @brief Digital gamepad buttons shared by all backends.
   */
  enum class Button : std::uint32_t {
    a = 0,
    b,
    x,
    y,
    dpad_up,
    dpad_down,
    dpad_left,
    dpad_right,
    start,
    back,
    guide,
    left_thumb,
    right_thumb,
    left_shoulder,
    right_shoulder,
    touchpad,
    touchpad_click,
    share,
    options,
    paddle1,
    paddle2,
    paddle3,
    paddle4,
  };

  /**
   * @brief Bit mask for one digital button.
   *
   * @param button Button to encode.
   * @return Single-bit mask.
   */
  constexpr std::uint32_t button_bit(Button button) {
    return std::uint32_t {1} << static_cast<std::uint32_t>(button);
  }

  /**
   * @brief One complete gamepad input report.
   */
  struct GamepadState {
    std::uint32_t buttons = 0;  ///< Bit mask of Button values.
    std::uint8_t left_trigger = 0;  ///< 0-255.
    std::uint8_t right_trigger = 0;  ///< 0-255.
    std::int16_t left_x = 0;  ///< -32768 to 32767.
    std::int16_t left_y = 0;  ///< -32768 to 32767.
    std::int16_t right_x = 0;  ///< -32768 to 32767.
    std::int16_t right_y = 0;  ///< -32768 to 32767.

    /**
     * @brief Check whether a button is pressed.
     *
     * @param button Button to query.
     * @return True when the button bit is set.
     */
    bool pressed(Button button) const {
      return (buttons & button_bit(button)) != 0;
    }

    /**
     * @brief Set or clear a button bit.
     *
     * @param button Button to update.
     * @param pressed New pressed state.
     */
    void set(Button button, bool pressed) {
      if (pressed) {
        buttons |= button_bit(button);
      } else {
        buttons &= ~button_bit(button);
      }
    }

    /**
     * @brief Reset all buttons, triggers, and sticks.
     */
    void clear() {
      *this = {};
    }
  };

  /**
   * @brief Optional touchpad report.
   */
  struct TouchpadState {
    bool active = false;  ///< Whether a finger is touching the pad.
    std::uint16_t x = 0;  ///< Touch X coordinate.
    std::uint16_t y = 0;  ///< Touch Y coordinate.
  };

  /**
   * @brief Optional inertial measurement unit sample.
   */
  struct ImuSample {
    float gyro_x = 0.0F;  ///< Radians per second.
    float gyro_y = 0.0F;  ///< Radians per second.
    float gyro_z = 0.0F;  ///< Radians per second.
    float accel_x = 0.0F;  ///< Meters per second squared.
    float accel_y = 0.0F;  ///< Meters per second squared.
    float accel_z = 0.0F;  ///< Meters per second squared.
    std::uint64_t timestamp_us = 0;  ///< Host timestamp in microseconds.
  };

  /**
   * @brief Features a backend can expose for a profile.
   */
  struct Capabilities {
    bool rumble = false;  ///< Rumble feedback.
    bool leds = false;  ///< LED or lightbar feedback.
    bool imu = false;  ///< Gyroscope and accelerometer.
    bool touchpad = false;  ///< Touchpad input.
    bool adaptive_triggers = false;  ///< Adaptive trigger feedback.
    bool raw_keyboard = false;  ///< Raw Input-visible keyboard.
    bool raw_mouse = false;  ///< Raw Input-visible mouse.
  };

  /**
   * @brief Description of a created virtual device.
   */
  struct DeviceInfo {
    Profile profile = Profile::generic;  ///< Active profile.
    Capabilities capabilities;  ///< Features exposed by the backend.
    std::string name;  ///< Device name shown by the operating system.
  };

  /**
   * @brief Convert a profile to a stable short name.
   *
   * @param profile Profile to convert.
   * @return Static string such as "xbox_360".
   */
  const char *to_string(Profile profile);

  /**
   * @brief Parse a profile from its short name.
   *
   * @param name Profile name.
   * @param profile Receives the parsed profile.
   * @return True when the name is valid.
   */
  bool profile_from_string(const std::string &name, Profile &profile);

}  // namespace apollo::input
