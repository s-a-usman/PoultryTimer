# Timer with Power-Loss Recovery and Alarm System

## Overview
This project is a timer system with power-loss recovery, user-friendly controls, and a built-in alarm feature. It is designed to operate as a countdown timer with the ability to store and recover its state in the event of a power interruption. The timer is equipped with an intuitive interface for setting the duration and visual feedback on the remaining time, making it suitable for applications like cooking, study sessions, or general task management.

---

## Features
1. **Power-Loss Recovery**:
   - Automatically saves the timer's state (hours, minutes, seconds, and running status) into the EEPROM.
   - Recovers and resumes the timer upon power restoration.

2. **User-Friendly Interface**:
   - Uses an LCD screen to display timer settings and remaining time.
   - Allows users to set the timer duration with a potentiometer and start it with a button.

3. **Visual and Audible Feedback**:
   - Displays remaining time in HH:MM:SS format.
   - Triggers a buzzer alarm when the timer reaches zero.

4. **Manual Timer Reset**:
   - The timer can be manually reset by holding the start button for 5 seconds.

5. **Automatic Alarm Reset**:
   - The alarm resets itself after 3 seconds and restores the timer to the last set duration.

---

## Components Required
- **Microcontroller**: Arduino Nano
- **LCD**: 16x2 LiquidCrystal_I2C
- **Buzzer**: Active buzzer module
- **Potentiometer**: 10kΩ
- **Push Button**: Momentary push button
- **EEPROM**: Built-in Arduino EEPROM
- **Miscellaneous**: Resistors, wires, and breadboard for prototyping

---

## Circuit Diagram
1. **LCD**: Connected via I2C protocol to Arduino Nano.
2. **Potentiometer**: Analog input connected to `A0` pin for timer adjustment.
3. **Buzzer**: Connected to digital pin `9`.
4. **Start Button**: Connected to digital pin `2` with a pull-up resistor.

---

## Installation and Setup
1. **Hardware Assembly**:
   - Connect the components as per the circuit diagram.
   - Ensure proper pull-up resistors for the push button and correct polarity for the buzzer.

2. **Software Upload**:
   - Install the required Arduino libraries:
     - `Wire.h`
     - `LiquidCrystal_I2C.h`
     - `EEPROM.h`
   - Upload the provided code to the Arduino Nano using the Arduino IDE.

3. **Power Supply**:
   - Connect a 5V power source to the Arduino Nano for operation.

---

## How to Use
### Setting the Timer
1. Adjust the potentiometer to set the desired time (up to 2 hours).
2. Press the start button to save the time and begin the countdown.
   - The timer's state is saved to the EEPROM.

### Monitoring the Timer
- The LCD displays the remaining time in `HH:MM:SS` format.

### Alarm Notification
- When the timer reaches zero, the buzzer sounds an alarm, and the screen displays "TIME'S UP!"

### Resetting the Timer
- Hold the start button for 5 seconds to reset the timer to the previously saved duration.

### Power-Loss Recovery
- If power is interrupted, the timer resumes from where it left off upon power restoration.

---

## Customization
- **Timer Range**: Modify the `map()` function in the `loop()` function to extend or reduce the timer's maximum duration.
- **Alarm Duration**: Adjust the `3000` millisecond delay in the `triggerAlarm()` function to change how long the alarm sounds.

---

## Example Usage Scenarios
- **Cooking Timer**: Set precise durations for baking, boiling, or frying.
- **Study Sessions**: Use it as a Pomodoro timer for productivity.
- **Exercise Timer**: Track workout durations effectively.

---

## Troubleshooting
1. **No LCD Display**:
   - Verify the I2C address (default is `0x27`) and reinitialize the LCD.
2. **Button Not Responding**:
   - Check the pull-up resistor connection and debounce logic.
3. **EEPROM Errors**:
   - Ensure the EEPROM memory locations are not overwritten by other functions.

---

## Future Enhancements
- Add an option for dynamic alarm tones.
- Incorporate a real-time clock (RTC) for precise timing.
- Introduce a touchscreen interface for enhanced usability.

---

This README provides all necessary details to set up and operate the timer system. Feel free to modify or enhance the project as per your requirements!