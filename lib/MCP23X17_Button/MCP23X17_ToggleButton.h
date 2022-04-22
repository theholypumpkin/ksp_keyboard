// Arduino ToogleButton Library
// https://github.com/JChristensen/JC_Button
// Copyright (C) 2018 by Jack Christensen and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html

#ifndef JC_MCP23X17_TOGGLEBUTTON_H
#define JC_MCP23X17_TOGGLEBUTTON_H

#include <Arduino.h>
#include "MCP23X17_Button.h"

// a derived class for a "push-on, push-off" (toggle) type button.
// initial state can be given, default is off (false).
class MCP23X17_ToggleButton : public MCP23X17_Button
{
public:
    // constructor is similar to MCP_Button, but includes the initial state for the toggle.
    MCP23X17_ToggleButton(Adafruit_MCP23X17& mcp,
                    uint8_t mcp_pin,
                    uint8_t keyboard_key,
                    bool mcp_initialState = false,
                    uint32_t mcp_dbTime = 25,
                    uint8_t mcp_puEnable = true,
                    uint8_t mcp_invert = true)
        : MCP23X17_Button(mcp, mcp_pin, keyboard_key, mcp_dbTime, 
        mcp_puEnable, mcp_invert), m_toggleState(mcp_initialState) {}

    // read the button and return its state.
    // should be called frequently.
    bool read(uint16_t all_pins);

    // has the state changed?
    bool changed();

    // return the current state
    bool toggleState();

private:
    bool m_toggleState;
    bool m_changed;
};
#endif
