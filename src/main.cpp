#include <Adafruit_MCP23X17.h>
#include <Keyboard.h>
#include <MCP23X17_Button.h>
#include <MCP23X17_ToggleButton.h>
#include <AsyncTimer.h>
#include <math.h>
/*==================================================================================================
 * Rocket/Plane Control Defintions directly connected to the MCU
 */
#define W_S_ADC_PIN A1 // Pitch
#define A_D_ADC_PIN A2 // Yaw
#define Q_E_ADC_PIN A3 // Roll
#define UNUSED_ADC_PLACEHOLDER_PIN A4 // Maybe fwd bwd andere RCS Modus
#define THROTTLE_UP_DOWN_ADC_PIN A5
/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -
 * array bound for global moving avarge loop
 */
#define GBL_AVG_ARR_BND 8
/*__________________________________________________________________________________________________
 * The pin definitions for the first of two shift registers
 * Action Group Button Definitions
 */
#define ABORT_AG_PIN 7 // You have to press one button and turn a key to trigger the abort - SR2
#define AG_1_PIN 6 // SR2
#define AG_2_PIN 5 // SR2
#define AG_3_PIN 4 // SR2
#define AG_4_PIN 3 // SR2
#define AG_5_PIN 2 // SR2

#define AG_6_PIN 7 // SR1
#define AG_7_PIN 6 // SR1
#define AG_8_PIN 5 // SR1
#define AG_9_PIN 4 // SR1
#define AG_10_PIN 3 // SR1
/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -
 * Rocket/Plane Control Buttons
 */
#define SAS_TOGGLE_PIN 12      // T - SR2
#define INVERT_SAS_HOLD_PIN 13 // F - SR2
#define GEAR_TOGGLE_PIN 14  // G - SR2
#define BREAK_HOLD_PIN 15 // B - SR2

#define STAGE_PIN 11 // Space - SR1 - Has two button connected
/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -
 RCS */
#define RCS_TOGGLE_PIN 0 // R, H, N, I, J, K, L - SR1
#define RCS_FWD_PIN 1 // Forward, Backward, Downward, Left, Upward, Right - SR 1
#define RCS_BWD_PIN 2 // SR1
#define RCS_DWD_PIN 15 // SR1
#define RCS_LWD_PIN 14 // SR1
#define RCS_UPD_PIN 13 // SR1
#define RCS_RWD_PIN 12 // SR1
/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -
 WASD QE */
#define W_PITCH_PIN 8 // W - SR2
#define S_PITCH_PIN 9 // S - SR2
#define A_YAW_PIN 10  // A - SR2
#define D_YAW_PIN 11 // D - SR2
#define Q_ROLL_PIN 1 // Q - SR2
#define E_ROLL_PIN 0 // E - SR2

#define LIGHTS_TOGGLE_PIN 10 // U - SR1
#define MAP_TOGGLE_PIN 9 // M - SR1

//NOTE ONE PIN IS STILL LEFT FREE

/*================================================================================================== 
 * The CS Pins for both Shiftregisters
 */
#define SR_1_CS_PIN 6
#define SR_2_CS_PIN 7
/*==================================================================================================
 * Create two Shift Register Objects
 */
Adafruit_MCP23X17 sr1;
Adafruit_MCP23X17 sr2;
/*__________________________________________________________________________________________________
 * Buttons
 */
MCP23X17_Button 
  abortActionGroupButtonSR2(sr2, ABORT_AG_PIN), 
  actionGroup1ButtonSR2(sr2, AG_1_PIN),
  actionGroup2ButtonSR2(sr2, AG_2_PIN),
  actionGroup3ButtonSR2(sr2, AG_3_PIN),
  actionGroup4ButtonSR2(sr2, AG_4_PIN),
  actionGroup5ButtonSR2(sr2, AG_5_PIN),
  actionGroup6ButtonSR1(sr1, AG_6_PIN),
  actionGroup7ButtonSR1(sr1, AG_7_PIN),
  actionGroup8ButtonSR1(sr1, AG_8_PIN),
  actionGroup9ButtonSR1(sr1, AG_9_PIN),
  actionGroup10ButtonSR1(sr1, AG_10_PIN),

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  invertSASHoldButtonSR2(sr2, INVERT_SAS_HOLD_PIN),

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  breakHoldbuttonSR2(sr2, BREAK_HOLD_PIN),

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  wButtonSR2(sr2, W_PITCH_PIN),
  aButtonSR2(sr2, A_YAW_PIN),
  sButtonSR2(sr2, S_PITCH_PIN),
  dButtonSR2(sr2, D_YAW_PIN),
  qButtonSR2(sr2, Q_ROLL_PIN),
  eButtonSR2(sr2, E_ROLL_PIN),

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  stageButtonSR1(sr1, STAGE_PIN),

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  RCSTranslateForwardButtonSR1(sr1, RCS_FWD_PIN),
  RCSTranslateBackwardButtonSR1(sr1, RCS_BWD_PIN),
  RCSTranslateDownButtonSR1(sr1, RCS_DWD_PIN),
  RCSTranslateUpButtonSR1(sr1, RCS_UPD_PIN),
  RCSTranslateLeftButtonSR1(sr1, RCS_LWD_PIN),
  RCSTranslateRightButtonSR1(sr1, RCS_RWD_PIN);

/*__________________________________________________________________________________________________
 * Toggle Buttons
 */
MCP23X17_ToggleButton
  SASToggleButtonSR2(sr2, SAS_TOGGLE_PIN),

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  gearToggleButtonSR2(sr2, GEAR_TOGGLE_PIN),

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  RCSToggleButtonSR1(sr1, RCS_TOGGLE_PIN),

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  lightToggleButtonSR1(sr1, LIGHTS_TOGGLE_PIN),

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  mapToggleButtonSR1(sr1, MAP_TOGGLE_PIN);

/*__________________________________________________________________________________________________
 * Create out Async Soft timer
 */
AsyncTimer timer;

/*==================================================================================================
 * Create global variables for the joystick inputs
 */
uint16_t a_d_mean = 0, a_d_old_mean = 0, a_d_neutral_pt = 0; // x-axis
uint16_t w_s_mean = 0, w_s_old_mean = 0, w_s_neutral_pt = 0; // y-axis
uint16_t q_e_mean = 0, q_e_old_mean = 0, q_e_neutral_pt = 0; // z-axis
uint16_t th_mean = 0, th_old_mean = 0; // Throttle

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -
 * throttle (shift/ctrl)
 */
uint16_t trottle_up_dwn_mean = 0, trottle_up_dwn_old_mean = 0, trottle_up_dwn_neutral_pt = 0; 

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -
 * Arrays to store the reading fro mean calculation
 */
uint16_t a_d_values_arr[GBL_AVG_ARR_BND]; // x-axis
uint16_t w_s_values_arr[GBL_AVG_ARR_BND]; // y-axis
uint16_t q_e_values_arr[GBL_AVG_ARR_BND]; // z-axis
uint16_t trottle_up_dwn_values_arr[GBL_AVG_ARR_BND]; // throttle (shift/ctrl)

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -
 * the global array intex
 */
uint8_t gbl_arr_ptr = 0;

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -
 * Keep the if statements blocked as long as a async timer is running.
 */
bool a_blocked = false, d_blocked = false, w_blocked = false, s_blocked = false, q_blocked = false, 
e_blocked = false, th_blocked;

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -
 * variables for the async timers
 */
uint16_t a_frequency = 0, d_frequency = 0, w_frequency = 0, s_frequency = 0, q_frequency = 0, 
e_frequency = 0, th_timeout = 0;
uint16_t a_interval_id = 0, d_interval_id = 0, w_interval_id = 0,  s_interval_id = 0, 
q_interval_id = 0, e_interval_id = 0;

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -
 * Variables for the linear function which converts the joystick tilt into repeated button presses
 * which variable frequency between erach press
 */
double m = 0.0;

//NOTE CHANGE MULTIPLYER FOR JOYSTICK SENSITEVITY
const double B = 358.0, RANGE = 11.0, MULTIPLYER = 2.0;

/*__________________________________________________________________________________________________
 * Variables wher each bit repesents if the pin on the shift register is high or low
 */
uint16_t sr1_all_pins = 0, sr2_all_pins = 0;

/*==================================================================================================
 * Constantly calculate the mean of the analog sticks to remove occiolations
 */
uint16_t mean(uint16_t* arr){
  /*4095*8 would be the highest value ever to be possile. 
   * This is still less than 2^16 so we are fine staying within 2 bytes.
   */
  uint16_t mean = 0;
  for(uint8_t i = 0; i < GBL_AVG_ARR_BND; ++i)
  {
    mean += arr[i]; // add the i-th value from the list and add it to local variable mean.
  }
  // To avoid a expensive devision (chip has no FPU) we shift the value by 3 to the right
  // it will yield the same result
  return mean >> 3; 
}

/*================================================================================================*/
void setup() {
  Serial.begin(9600); //TODO ONLY FOR DEBUG REMOVE LATER

  /*________________________________________________________________________________________________
   *Start the Async Soft Timer
   */
  timer.setup();
  /*________________________________________________________________________________________________
   *Start the Keyboard Library
   */
  Keyboard.begin();
  /*________________________________________________________________________________________________
   * Begin booth Shiftregisters
   */
  if (!sr1.begin_SPI(SR_1_CS_PIN)) {
    Serial.println("Error.");
    while (1);
  }
  if (!sr2.begin_SPI(SR_2_CS_PIN)) {
    Serial.println("Error.");
    while (1);
  }
  /*________________________________________________________________________________________________
   * Begin all the Buttons //TODO Maybe delay 1 ms between each begin to not overload the spi bus
   */
  abortActionGroupButtonSR2.begin();
  actionGroup1ButtonSR2.begin();
  actionGroup2ButtonSR2.begin();
  actionGroup3ButtonSR2.begin();
  actionGroup4ButtonSR2.begin();
  actionGroup5ButtonSR2.begin();
  actionGroup6ButtonSR1.begin();
  actionGroup7ButtonSR1.begin();
  actionGroup8ButtonSR1.begin();
  actionGroup9ButtonSR1.begin();
  actionGroup10ButtonSR1.begin();

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  invertSASHoldButtonSR2.begin();

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  breakHoldbuttonSR2.begin();

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  wButtonSR2.begin();
  aButtonSR2.begin();
  sButtonSR2.begin();
  dButtonSR2.begin();
  qButtonSR2.begin();
  eButtonSR2.begin();

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  stageButtonSR1.begin();

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  RCSTranslateBackwardButtonSR1.begin();
  RCSTranslateForwardButtonSR1.begin();
  RCSTranslateLeftButtonSR1.begin();
  RCSTranslateRightButtonSR1.begin();
  RCSTranslateUpButtonSR1.begin();
  RCSTranslateDownButtonSR1.begin();

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  RCSToggleButtonSR1.begin();

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  SASToggleButtonSR2.begin();

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  gearToggleButtonSR2.begin();

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  lightToggleButtonSR1.begin();

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  mapToggleButtonSR1.begin();
  
  /*________________________________________________________________________________________________
   * Fill up the avarage Array buffer
   */
  analogReadResolution(12);

  for(gbl_arr_ptr = 0; gbl_arr_ptr < GBL_AVG_ARR_BND; ++gbl_arr_ptr)
  {
    a_d_values_arr[gbl_arr_ptr] = analogRead(A_D_ADC_PIN);
    w_s_values_arr[gbl_arr_ptr] = analogRead(W_S_ADC_PIN);
    q_e_values_arr[gbl_arr_ptr] = analogRead(Q_E_ADC_PIN);
    trottle_up_dwn_values_arr[gbl_arr_ptr] = analogRead(THROTTLE_UP_DOWN_ADC_PIN);
  }
  // get the neutral point (basically calibration)
  a_d_neutral_pt = mean(a_d_values_arr);
  w_s_neutral_pt = mean(w_s_values_arr);
  q_e_neutral_pt = mean(q_e_values_arr);
  trottle_up_dwn_old_mean = mean(trottle_up_dwn_values_arr);
  
  a_d_old_mean = a_d_neutral_pt;
  w_s_old_mean = w_s_neutral_pt;
  q_e_old_mean = q_e_neutral_pt;
  /*________________________________________________________________________________________________
   * calculate `m` slope of linear function.
   */
  m = 48.0/22.0;

  /*________________________________________________________________________________________________
   * reset the global array pointer
   */
  gbl_arr_ptr = 0; 
}

/*================================================================================================*/
void loop() {
  timer.handle(); //Update Async soft timer
  /*________________________________________________________________________________________________
   * Read all pin values from both shiftregisters
   */
  sr1_all_pins = sr1.readGPIOAB();
  sr2_all_pins = sr2.readGPIOAB();

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   
   * and now update the button values using the read regiister
   */
  abortActionGroupButtonSR2.read(sr2_all_pins);
  actionGroup1ButtonSR2.read(sr2_all_pins);
  actionGroup2ButtonSR2.read(sr2_all_pins);
  actionGroup3ButtonSR2.read(sr2_all_pins);
  actionGroup4ButtonSR2.read(sr2_all_pins);
  actionGroup5ButtonSR2.read(sr2_all_pins);
  actionGroup6ButtonSR1.read(sr1_all_pins);
  actionGroup7ButtonSR1.read(sr1_all_pins);
  actionGroup8ButtonSR1.read(sr1_all_pins);
  actionGroup9ButtonSR1.read(sr1_all_pins);
  actionGroup10ButtonSR1.read(sr1_all_pins);

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  invertSASHoldButtonSR2.read(sr2_all_pins);

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  breakHoldbuttonSR2.read(sr2_all_pins);

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  wButtonSR2.read(sr2_all_pins);
  aButtonSR2.read(sr2_all_pins);
  sButtonSR2.read(sr2_all_pins);
  dButtonSR2.read(sr2_all_pins);
  qButtonSR2.read(sr2_all_pins);
  eButtonSR2.read(sr2_all_pins);

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  stageButtonSR1.read(sr1_all_pins);

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  RCSTranslateBackwardButtonSR1.read(sr1_all_pins);
  RCSTranslateForwardButtonSR1.read(sr1_all_pins);
  RCSTranslateLeftButtonSR1.read(sr1_all_pins);
  RCSTranslateRightButtonSR1.read(sr1_all_pins);
  RCSTranslateUpButtonSR1.read(sr1_all_pins);
  RCSTranslateDownButtonSR1.read(sr1_all_pins);

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  RCSToggleButtonSR1.read(sr1_all_pins);

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  SASToggleButtonSR2.read(sr2_all_pins);

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  gearToggleButtonSR2.read(sr2_all_pins);

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  lightToggleButtonSR1.read(sr1_all_pins);

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  mapToggleButtonSR1.read(sr1_all_pins);
  /*________________________________________________________________________________________________
   * Check if each Button was Pressed or not.
   */
  if(abortActionGroupButtonSR2.wasPressed()){
    Keyboard.press(KEY_BACKSPACE);
    delay(1);
    Keyboard.release(KEY_BACKSPACE);
  }

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(actionGroup1ButtonSR2.wasPressed()){
    Keyboard.press('1');
    delay(1);
    Keyboard.release('1');
  }

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(actionGroup2ButtonSR2.wasPressed()){
    Keyboard.press('2');
    delay(1);
    Keyboard.release('2');
  }

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(actionGroup3ButtonSR2.wasPressed()){
    Keyboard.press('3');
    delay(1);
    Keyboard.release('3');
  }

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(actionGroup4ButtonSR2.wasPressed()){
    Keyboard.press('4');
    delay(1);
    Keyboard.release('4');
  }

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(actionGroup5ButtonSR2.wasPressed()){
    Keyboard.press('5');
    delay(1);
    Keyboard.release('5');
  }

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(actionGroup6ButtonSR1.wasPressed()){
    Keyboard.press('6');
    delay(1);
    Keyboard.release('6');
  }

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(actionGroup7ButtonSR1.wasPressed()){
    Keyboard.press('7');
    delay(1);
    Keyboard.release('7');
  }

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(actionGroup8ButtonSR1.wasPressed()){
    Keyboard.press('8');
    delay(1);
    Keyboard.release('8');
  }

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(actionGroup9ButtonSR1.wasPressed()){
    Keyboard.press('9');
    delay(1);
    Keyboard.release('9');
  }

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(actionGroup10ButtonSR1.wasPressed()){
    Keyboard.press('0');
    delay(1);
    Keyboard.release('0');
  }

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(invertSASHoldButtonSR2.isPressed())
    Keyboard.press('f');
  else if(invertSASHoldButtonSR2.wasReleased())
    Keyboard.release('f');

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(breakHoldbuttonSR2.isPressed())
    Keyboard.press('b');
  else if(breakHoldbuttonSR2.wasReleased())
    Keyboard.release('b');

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(wButtonSR2.isPressed())
    Keyboard.press('w');
  else if(wButtonSR2.wasReleased())
    Keyboard.release('w');

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(aButtonSR2.isPressed())
    Keyboard.press('a');
  else if(aButtonSR2.wasReleased())
    Keyboard.release('a');

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(sButtonSR2.isPressed())
    Keyboard.press('s');
  else if(sButtonSR2.wasReleased())
    Keyboard.release('s');

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(dButtonSR2.isPressed())
    Keyboard.press('d');
  else if(dButtonSR2.wasReleased())
    Keyboard.release('d');

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(qButtonSR2.isPressed())
    Keyboard.press('q');
  else if(qButtonSR2.wasReleased())
    Keyboard.release('q');

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(eButtonSR2.isPressed())
    Keyboard.press('e');
  else if(eButtonSR2.wasReleased())
    Keyboard.release('e');

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(stageButtonSR1.wasPressed()){
    Keyboard.press(' '); //BUG this might not press a space character
    delay(1);
    Keyboard.release(' ');
  }

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(RCSTranslateBackwardButtonSR1.isPressed())
    Keyboard.press('n');
  else if(RCSTranslateBackwardButtonSR1.wasReleased())
    Keyboard.release('n');

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(RCSTranslateForwardButtonSR1.isPressed())
    Keyboard.press('h');
  else if(RCSTranslateForwardButtonSR1.wasReleased())
    Keyboard.release('h');

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(RCSTranslateDownButtonSR1.isPressed())
    Keyboard.press('i');
  else if(RCSTranslateDownButtonSR1.wasReleased())
    Keyboard.release('i');

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(RCSTranslateUpButtonSR1.isPressed())
    Keyboard.press('k');
  else if(RCSTranslateUpButtonSR1.wasReleased())
    Keyboard.release('k');

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(RCSTranslateLeftButtonSR1.isPressed())
    Keyboard.press('j');
  else if(RCSTranslateLeftButtonSR1.wasReleased())
    Keyboard.release('j');

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(RCSTranslateRightButtonSR1.isPressed())
    Keyboard.press('l');
  else if(RCSTranslateBackwardButtonSR1.wasReleased())
    Keyboard.release('l');

  /*________________________________________________________________________________________________
   * Check if each Toggle Button was has changed or not.
   */
  if(SASToggleButtonSR2.changed()){
      Keyboard.press('t');
      delay(1);
      Keyboard.release('t');
  }

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(RCSToggleButtonSR1.changed()){
      Keyboard.press('r');
      delay(1);
      Keyboard.release('r');
  }

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(gearToggleButtonSR2.changed()){
      Keyboard.press('g');
      delay(1);
      Keyboard.release('g');
  }

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(mapToggleButtonSR1.changed()){
      Keyboard.press('m');
      delay(1);
      Keyboard.release('m');
  }

  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if(lightToggleButtonSR1.changed()){
      Keyboard.press('u');
      delay(1);
      Keyboard.release('u');
  }

  /*________________________________________________________________________________________________
   * Update all the means by replacing one reading in the array
   */
  a_d_values_arr[gbl_arr_ptr] = analogRead(A_D_ADC_PIN); // Overwrite value at the gbl_arr_ptr place
  w_s_values_arr[gbl_arr_ptr] = analogRead(W_S_ADC_PIN);
  q_e_values_arr[gbl_arr_ptr] = analogRead(Q_E_ADC_PIN);
  trottle_up_dwn_values_arr[gbl_arr_ptr] = analogRead(THROTTLE_UP_DOWN_ADC_PIN);
  
  a_d_mean = mean(a_d_values_arr); // calculate the current mean
  w_s_mean = mean(w_s_values_arr);
  q_e_mean = mean(q_e_values_arr);
  trottle_up_dwn_mean = mean(trottle_up_dwn_values_arr);

  /*________________________________________________________________________________________________
  * Proccess the Joysticks
  * I'm aware it violates DRY but I'm new to lambada
  */
  if ((a_d_mean < a_d_neutral_pt - 10) && (!a_blocked))
  {
    a_blocked = true; //block the codeblock frm future execution
    a_d_old_mean = a_d_mean; // store mean
    a_frequency = round(((m*(a_d_mean/RANGE))- B)*MULTIPLYER);

    a_interval_id =  timer.setInterval([]() // lambada function
    {
      Keyboard.press('a');
      delay(1);
      Keyboard.release('a');
      if((a_d_mean > a_d_old_mean + 10) || (a_d_mean < a_d_old_mean - 10))
        {
          a_blocked = false; // Unblock the a-key
          timer.cancel(a_interval_id); // Cancel the interval
        }
    }, a_frequency);
  }
  
  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if ((a_d_mean > a_d_neutral_pt + 10) && (!d_blocked))
  {
    d_blocked = true; //block the codeblock frm future execution
    a_d_old_mean = a_d_mean; // store mean
    d_frequency = round((-1*(m*(a_d_mean/RANGE))- B)*MULTIPLYER);

    d_interval_id =  timer.setInterval([]() // lambada function
    {
      Keyboard.press('d');
      delay(1);
      Keyboard.release('d');
      if((a_d_mean > a_d_old_mean + 10) || (a_d_mean < a_d_old_mean - 10))
        {
          d_blocked = false; // Unblock the a-key
          timer.cancel(d_interval_id); // Cancel the interval
        }
    }, d_frequency);
  }
  
  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if ((w_s_mean < w_s_neutral_pt - 10) && (!s_blocked))
  {
    s_blocked = true; //block the codeblock frm future execution
    w_s_old_mean = w_s_mean; // store mean
    s_frequency = round((-1*(m*(w_s_mean/RANGE))- B)*MULTIPLYER);

    s_interval_id =  timer.setInterval([]() // lambada function
    {
      Keyboard.press('d');
      delay(1);
      Keyboard.release('d');
      if((w_s_mean > w_s_old_mean + 10) || (w_s_mean < w_s_old_mean - 10))
        {
          s_blocked = false; // Unblock the a-key
          timer.cancel(s_interval_id); // Cancel the interval
        }
    }, s_frequency);
  }
  
  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if ((w_s_mean > w_s_neutral_pt + 10) && (!w_blocked))
  {
    w_blocked = true; //block the codeblock frm future execution
    w_s_old_mean = w_s_mean; // store mean
    w_frequency = round(((m*(w_s_mean/RANGE))- B)*MULTIPLYER);

    w_interval_id =  timer.setInterval([]() // lambada function
    {
      Keyboard.press('w');
      delay(1);
      Keyboard.release('w');
      if((w_s_mean > w_s_old_mean + 10) || (w_s_mean < w_s_old_mean - 10))
        {
          w_blocked = false; // Unblock the a-key
          timer.cancel(w_interval_id); // Cancel the interval
        }
    }, w_frequency);
  }
  
  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if ((q_e_mean < q_e_neutral_pt - 10) && (!q_blocked))
  {
    q_blocked = true; //block the codeblock frm future execution
    q_e_old_mean = q_e_mean; // store mean
    q_frequency = round((-1*(m*(q_e_mean/RANGE))- B)*MULTIPLYER);
    
    q_interval_id =  timer.setInterval([]() // lambada function
    {
      Keyboard.press('d');
      delay(1);
      Keyboard.release('d');
      if((q_e_mean > q_e_old_mean + 10) || (q_e_mean < q_e_old_mean - 10))
        {
          q_blocked = false; // Unblock the a-key
          timer.cancel(q_interval_id); // Cancel the interval
        }
    }, q_frequency);
  }
  
  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if ((q_e_mean > q_e_neutral_pt + 10) && (!e_blocked))
  {
    e_blocked = true; //block the codeblock frm future execution
    q_e_old_mean = q_e_mean; // store mean
    e_frequency = round(((m*(q_e_mean/RANGE))- B)*MULTIPLYER);

    e_interval_id =  timer.setInterval([]() // lambada function
    {
      Keyboard.press('w');
      delay(1);
      Keyboard.release('w');
      if((q_e_mean > q_e_old_mean + 10) || (q_e_mean < q_e_old_mean - 10))
        {
          e_blocked = false; // Unblock the a-key
          timer.cancel(e_interval_id); // Cancel the interval
        }
    }, e_frequency);
  }

  /*________________________________________________________________________________________________
   * Process the Throttle
   * Range from 4070 to 2570 make a range of 1500 points. We throttle 0.75 points per ms, 15 points 
   * are 1% . I have to push the button for 15 ms
   * */
  if((th_old_mean +5 <= th_mean)&&(!th_blocked)){
    th_blocked = true;
    // claculate the difference to know how long the throttle key has to be pressed
    th_timeout = th_mean - th_old_mean; 
    th_old_mean = th_mean; // set the old mean to the current mean
    
    Keyboard.press(KEY_LEFT_SHIFT);
    timer.setTimeout([]()
    {
      th_blocked = false;
      Keyboard.release(KEY_LEFT_SHIFT);
    }, th_timeout); // Always 5 MS
  }
  
  /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
  if((th_old_mean -5 >= th_mean)&&(!th_blocked)){
    th_blocked = true;
    // claculate the difference to know how long the throttle key has to be pressed
    th_timeout = th_mean - th_old_mean;
    th_old_mean = th_mean;

    Keyboard.press(KEY_LEFT_CTRL);
    timer.setTimeout([]()
    {
      th_blocked = false;
      Keyboard.release(KEY_LEFT_CTRL);
    }, th_timeout); // Always 5 MS
  }
  
  /*______________________________________________________________________________________________*/
  gbl_arr_ptr < GBL_AVG_ARR_BND-1 ? gbl_arr_ptr++ : 0; // increment ptr if less than 7 else set 0
}