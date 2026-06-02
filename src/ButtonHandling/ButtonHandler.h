
#pragma once

enum ButtonPressType {
    NO_PRESS,
    SHORT_PRESS,
    LONG_PRESS
};

/**
 * @brief Initialize button handling for input detection
 */
int button_handler_init(void);

/**
 * @brief Enable hardware interrupt for button presses
 *
 * @return 0 on success, or -1 on failure.
 */
int button_handler_enable_interrupt(void);

/**
 * @brief Disable hardware interrupt for button presses
 *
 * @return 0 on success, or -1 on failure.
 */
int button_handler_disable_interrupt(void);

/**
 * @brief Check the button activity and return the detected press type, then reset the state
 *
 * This function should be called in the main loop to check if a button press has been detected.
 * It returns the type of button press (short or long) that was detected since the last call,
 * and then resets the internal state to wait for the next button press.
 *
 * @return An integer representing the detected button press type:
 *         - 0: No press detected
 *         - 1: Short press detected
 *         - 2: Long press detected
 */
ButtonPressType button_handler_check_activity_and_reset(void);