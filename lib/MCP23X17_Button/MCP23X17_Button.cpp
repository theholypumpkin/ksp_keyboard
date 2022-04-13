#include "MCP23X17_Button.h"

/*----------------------------------------------------------------------*
  / initialize a MCP_Button object and the pin it's connected to.             *
  /-----------------------------------------------------------------------*/
void MCP23X17_Button::begin()
{
  mcp_register.pinMode(m_pin, m_puEnable ? INPUT_PULLUP : INPUT);
  m_state = mcp_register.digitalRead(m_pin);
  if (m_invert)
    m_state = !m_state;
  m_time = millis();
  m_lastState = m_state;
  m_changed = false;
  m_lastChange = m_time;
}

/*----------------------------------------------------------------------*
  / returns the state of the button, true if pressed, false if released.  *
  / does debouncing, captures and maintains times, previous state, etc.   *
  /-----------------------------------------------------------------------*/
bool MCP23X17_Button::read(uint16_t all_pins)
{
  uint32_t ms = millis();
  uint16_t bitmap = 1; // HEX: 0x0001 BIN: 0b 0000 0000 0000 0001
  /* shift the mask bit to the right pin position  and than logical AND it with all_pins when it
   * returns 1 we the button is not pressed so we assign true to pinVal, when it returns 0 we
   * assign false to pinVal meaning the button is pressed.
   * This is later inverted by the debouncer i.e True becomes false and false becomes true.
   * After some debouncing the values is assigned to m_state wich we can read with isPressed();
   */
  bool pinVal = (all_pins & bitmap << m_pin); 
  if (m_invert) // Default = true
    pinVal = !pinVal; // Here we invert the boolean.
  if (ms - m_lastChange < m_dbTime)
    m_changed = false;
  else
  {
    m_lastState = m_state; //Even if m_state is 1, last state stays because the object instace is not saved inside the map. i.e the object gets reset every time.
    m_state = pinVal;
    m_changed = (m_state != m_lastState);
    if (m_changed)
      m_lastChange = ms;
  }
  m_time = ms;
  return m_state;
}

/*----------------------------------------------------------------------*
   isPressed() and isReleased() check the button state when it was last
   read, and return false (0) or true (!=0) accordingly.
   These functions do not cause the button to be read.
  ----------------------------------------------------------------------*/
bool MCP23X17_Button::isPressed()
{
  return m_state;
}

bool MCP23X17_Button::isReleased()
{
  return !m_state;
}

/*----------------------------------------------------------------------*
   wasPressed() and wasReleased() check the button state to see if it
   changed between the last two reads and return false (0) or
   true (!=0) accordingly.
   These functions do not cause the button to be read.
  ----------------------------------------------------------------------*/
bool MCP23X17_Button::wasPressed()
{
  return m_state && m_changed;
}

bool MCP23X17_Button::wasReleased()
{
  return !m_state && m_changed;
}

/*----------------------------------------------------------------------*
   pressedFor(ms) and releasedFor(ms) check to see if the button is
   pressed (or released), and has been in that state for the specified
   time in milliseconds. Returns false (0) or true (!=0) accordingly.
   These functions do not cause the button to be read.
  ----------------------------------------------------------------------*/
bool MCP23X17_Button::pressedFor(uint32_t ms)
{
  return m_state && m_time - m_lastChange >= ms;
}

bool MCP23X17_Button::releasedFor(uint32_t ms)
{
  return !m_state && m_time - m_lastChange >= ms;
}

/*----------------------------------------------------------------------*
   lastChange() returns the time the button last changed state,
   in milliseconds.
  ----------------------------------------------------------------------*/
uint32_t MCP23X17_Button::lastChange()
{
  return m_lastChange;
}
