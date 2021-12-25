#ifndef MCP23X17_BUTTON_H
#define MCP23X17_BUTTON_H

#include <Arduino.h>
#include <Adafruit_MCP23X17.h>

class MCP23X17_Button {
  public:
    // MCP_Button(pin, dbTime, puEnable, invert) instantiates a button object.
    //
    // Required parameter:
    // pin      The Arduino pin the button is connected to
    //
    // Optional parameters:
    // dbTime   Debounce time in milliseconds (default 25ms)
    // puEnable true to enable the AVR internal pullup resistor (default true)
    // invert   true to interpret a low logic level as pressed (default true)
    MCP23X17_Button(Adafruit_MCP23X17& mcp,
                    uint8_t mcp_pin,
                    uint32_t mcp_dbTime = 25,
                    uint8_t mcp_puEnable = true,
                    uint8_t mcp_invert = true
                   )
      : mcp_register(mcp),
        m_pin(mcp_pin),
        m_dbTime(mcp_dbTime),
        m_puEnable(mcp_puEnable),
        m_invert(mcp_invert){}

    // Initialize a MCP_Button object and the pin it's connected to
    void begin();

    // Returns the current debounced button state, true for pressed,
    // false for released. Call this function frequently to ensure
    // the sketch is responsive to user input.
    bool read(uint16_t all_pins);

    // Returns true if the button state was pressed at the last call to read().
    // Does not cause the button to be read.
    bool isPressed();

    // Returns true if the button state was released at the last call to read().
    // Does not cause the button to be read.
    bool isReleased();

    // Returns true if the button state at the last call to read() was pressed,
    // and this was a change since the previous read.
    bool wasPressed();

    // Returns true if the button state at the last call to read() was released,
    // and this was a change since the previous read.
    bool wasReleased();

    // Returns true if the button state at the last call to read() was pressed,
    // and has been in that state for at least the given number of milliseconds.
    bool pressedFor(uint32_t ms);

    // Returns true if the button state at the last call to read() was released,
    // and has been in that state for at least the given number of milliseconds.
    bool releasedFor(uint32_t ms);

    // Returns the time in milliseconds (from millis) that the button last
    // changed state.
    uint32_t lastChange();

  private:
    Adafruit_MCP23X17& mcp_register;

    uint8_t m_pin;         // arduino pin number connected to button
    uint32_t m_dbTime;     // debounce time (ms)
    bool m_puEnable;       // internal pullup resistor enabled
    bool m_invert;         // if true, interpret logic low as pressed, else interpret logic high as pressed
    bool m_state;          // current button state, true=pressed
    bool m_lastState;      // previous button state
    bool m_changed;        // state changed since last read
    uint32_t m_time;       // time of current state (ms from millis)
    uint32_t m_lastChange; // time of last state change (ms)
};
#endif
