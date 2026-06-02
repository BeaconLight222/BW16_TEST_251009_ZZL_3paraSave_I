
#include <Arduino.h>
#include <GTimer.h>
#include "ButtonHandler.h"

// Button debounce timer and interrupt handling 

#define BEACON_OS_BUTTON_DEBOUNCE_TIMER (2)  // Use id2 for button debouncing

// Button debounce timer will run this often when debouncing the button is active
#define BEACON_OS_BUTTON_DEBOUNCE_TIMER_POLLING_MS (5) 

// Forward declaration
void button_debounce_timer_handler(void);
void button_event_handler(bool state);

static void beacon_os_button_debounce_timer_callback(uint32_t data) {
	(void)data;
	button_debounce_timer_handler();
}

void beacon_os_start_button_debounce_timer(void) {
	// GTimer expects interval in microseconds, so convert from milliseconds
	GTimer.begin(BEACON_OS_BUTTON_DEBOUNCE_TIMER, BEACON_OS_BUTTON_DEBOUNCE_TIMER_POLLING_MS * 1000, beacon_os_button_debounce_timer_callback);
}

void beacon_os_stop_button_debounce_timer(void) {
	GTimer.stop(BEACON_OS_BUTTON_DEBOUNCE_TIMER);
}

static void beacon_os_button_isr_handler(uint32_t id, uint32_t event) {
	(void)id;
	(void)event;
	button_event_handler(digitalRead(PA15) == LOW);
}

int beacon_os_button_init(void) {
	return 0;
}

bool beacon_os_get_button_state(void) {
	return (digitalRead(PA15) == LOW);
}

int beacon_os_enable_button_interrupt(void) {
	// Interrupt detects FALLING edge.  We only care about the press the release will be detected via the debounce logic
	pinMode(PA15, INPUT_IRQ_FALL);
	digitalSetIrqHandler(PA15, beacon_os_button_isr_handler);
    //2.4V, change from 5D00, disable PAD_BIT_SDIO_H3L1 change voltage from 1.6V to 2.4V. Maybe still some conflict but OK for now.
    //This also ensures the pin is pulled up
	PINMUX->PADCTR[_PA_15] = 0x1D00;
	return 0;
}

int beacon_os_disable_button_interrupt(void) {
	pinMode(PA15, INPUT_PULLUP);
	return 0;
}

class ButtonHandler {
public:
    enum State {
        UNPRESSED,
        DEBOUNCING_PRESS,
        PRESSED,
        DEBOUNCING_RELEASE
    };

    ButtonHandler() : currentState(UNPRESSED), debounceCount(0), pressDurationTicks(0), lastDetectedPressType(NO_PRESS) {}

    void begin() {
        beacon_os_button_init();
    }

    void enableButtonInterrupt() {
        beacon_os_enable_button_interrupt();
    }

    void disableButtonInterrupt() {
        beacon_os_disable_button_interrupt();
    }

    void emit_event() {
        // 4 seconds = 4000ms. 4000ms / 5ms per tick = 800 ticks.
        if (pressDurationTicks >= 800) {
            lastDetectedPressType = LONG_PRESS;
        } else {
            lastDetectedPressType = SHORT_PRESS;
        }
    }

    void handleInterrupt(bool state) {
        //  Reset the button tracking class instance to track a new button press
        currentState = DEBOUNCING_PRESS;
        lastButtonState = state;
        debounceCount = 0;
        pressDurationTicks = 0;

        // Disable the button interrupt
        beacon_os_disable_button_interrupt();
        
        //  Enable the button debounce timer
        beacon_os_start_button_debounce_timer();
    }

    void handleTimer() {
        if (currentState == UNPRESSED) {
          // Timer should not be running in this state, but if it is, reset and return
          beacon_os_stop_button_debounce_timer();
          beacon_os_enable_button_interrupt();
          return;
        } 

        bool buttonState = beacon_os_get_button_state();

        if (currentState == DEBOUNCING_PRESS) {

          if(buttonState == lastButtonState) {
            // Button state has not changed, increment debounce count
            debounceCount++;
            if(debounceCount > 2) {
              currentState = PRESSED;
              debounceCount = 0; // Reset for release debouncing
            }
          } else {
            // Button state has changed before debounce threshold, reset to UNPRESSED
            beacon_os_stop_button_debounce_timer();
            beacon_os_enable_button_interrupt();
            currentState = UNPRESSED;
            return;
          }

        } else if (currentState == PRESSED) {

          // We have debounced and are pressed, start tracking for how long
          pressDurationTicks++;

          // Now debounce the release
          if(buttonState != lastButtonState) {
            // Button state has changed, increment release debounce count
            debounceCount++;
            if(debounceCount >= 2) {
              // Release is debounced, stop timer and re-enable interrupt, and send event
              beacon_os_stop_button_debounce_timer();
              beacon_os_enable_button_interrupt();
              currentState = UNPRESSED; // Update state here for safety

              emit_event();
              return;
            }
          } else {
            // Button state is still pressed, reset release debounce count
            debounceCount = 0;  
          }
        }
    }

    ButtonPressType getLastDetectedPressTypeAndReset(void) {
        ButtonPressType detectedPressType = lastDetectedPressType;
        if(detectedPressType != NO_PRESS) {
            lastDetectedPressType = NO_PRESS; // Reset after reading
        }
        return detectedPressType;
    }

private:
    State currentState;
    int lastButtonState;
    int debounceCount;
    int pressDurationTicks;
    ButtonPressType lastDetectedPressType;
};

// Instantiate a global ButtonHandler object that will be used in the interrupt and timer handlers
//  Does not need any kind of public access though, once initialized this module only produces
//  events
static ButtonHandler g_button_handler;

int button_handler_init() {
    g_button_handler.begin();
    return 0;
}

int button_handler_enable_interrupt() {
    g_button_handler.enableButtonInterrupt();
    return 0;
}

int button_handler_disable_interrupt() {
    g_button_handler.disableButtonInterrupt();
    return 0;
}

ButtonPressType button_handler_check_activity_and_reset() {
    return g_button_handler.getLastDetectedPressTypeAndReset();
}

// Callback from beacon_os button debounce timer
void button_debounce_timer_handler(void) {
    g_button_handler.handleTimer();
}

// Callback from beacon_os button interrupt handler
void button_event_handler(bool state) {
    g_button_handler.handleInterrupt(state);
}