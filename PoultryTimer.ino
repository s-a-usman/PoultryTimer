#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Servo.h>

// EEPROM Address Definitions
const int TIMER_HOURS_ADDR = 0;
const int TIMER_MINUTES_ADDR = 1;
const int REMAINING_HOURS_ADDR = 2;
const int REMAINING_MINUTES_ADDR = 3;
const int REMAINING_SECONDS_ADDR = 4;
const int TIMER_RUNNING_ADDR = 5;

// Pin Definitions
const int POTENTIOMETER_PIN = A0;
const int SERVO_PIN = 9;
const int START_BUTTON_PIN = 3;
const int LED_PIN = 5;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Timer Variables
unsigned long startTime = 0;
unsigned long timerDuration = 0;
unsigned long alarmStartTime = 0;
unsigned long buttonPressStartTime = 0;
bool timerRunning = false;
bool alarmActive = false;
bool settingTimer = true;
bool initialStartup = true;
bool backToSetting = false;
int selectedHours = 0;
int selectedMinutes = 0;
int previousHours = -1;
int previousMinutes = -1;
int previousSeconds = -1;

// Button Debounce Variables
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 50;

Servo myServo;  // Servo object

bool isButtonPressed() {
  if (digitalRead(START_BUTTON_PIN) == LOW) {
    if (millis() - lastButtonPress > debounceDelay) {
      lastButtonPress = millis();
      return true;
    }
  }
  return false;
}

void displayRemainingTime(int hours, int minutes, int seconds) {
  lcd.setCursor(0, 0);
  lcd.print("   Time Left:");

  lcd.setCursor(0, 1);
  lcd.print("    ");
  if (hours < 10) lcd.print("0");
  lcd.print(hours);
  lcd.print(":");
  if (minutes < 10) lcd.print("0");
  lcd.print(minutes);
  lcd.print(":");
  if (seconds < 10) lcd.print("0");
  lcd.print(seconds);
}

void triggerAlarm() {
  alarmActive = true;
  timerRunning = false;
  alarmStartTime = millis();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TIME'S UP!");
  myServo.write(90);  // Move servo to 0 degrees
}

void displayTimerSetting() {
  lcd.setCursor(0, 0);
  lcd.print("   Set Timer:");

  lcd.setCursor(0, 1);
  lcd.print("    ");
  if (selectedHours < 10) lcd.print("0");
  lcd.print(selectedHours);
  lcd.print(":");
  if (selectedMinutes < 10) lcd.print("0");
  lcd.print(selectedMinutes);
  lcd.print(":00    ");
}

void setup() {
  pinMode(POTENTIOMETER_PIN, INPUT);
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  myServo.attach(SERVO_PIN);
  myServo.write(0);  // Servo at 0 degrees initially

  lcd.init();
  lcd.backlight();

  // Check if timer was running before power loss
  checkPreviousTimerState();
}

void checkPreviousTimerState() {
  byte storedRunningState = EEPROM.read(TIMER_RUNNING_ADDR);

  if (storedRunningState == 1) {
    selectedHours = EEPROM.read(REMAINING_HOURS_ADDR);
    selectedMinutes = EEPROM.read(REMAINING_MINUTES_ADDR);
    int remainingSeconds = EEPROM.read(REMAINING_SECONDS_ADDR);

    timerDuration = (selectedHours * 3600L + selectedMinutes * 60L + remainingSeconds) * 1000L;
    startTime = millis() - (timerDuration - (selectedHours * 3600L + selectedMinutes * 60L + remainingSeconds) * 1000L);

    timerRunning = true;
    settingTimer = false;
    initialStartup = false;
  } else {
    displayTimerSetting();
  }
}

void resetTimer() {
  settingTimer = false;
  alarmActive = false;
  timerRunning = true;
  backToSetting = false;

  int potValue = analogRead(POTENTIOMETER_PIN);
  int totalMinutes = map(potValue, 0, 1023, 0, 120);

  selectedHours = totalMinutes / 60;
  selectedMinutes = totalMinutes % 60;

  timerDuration = (selectedHours * 3600L + selectedMinutes * 60L) * 1000L;

  EEPROM.write(TIMER_HOURS_ADDR, selectedHours);
  EEPROM.write(TIMER_MINUTES_ADDR, selectedMinutes);
  EEPROM.write(TIMER_RUNNING_ADDR, 1);

  startTime = millis();

  myServo.write(0);  // Reset servo to 0 degrees

  previousHours = -1;
  previousMinutes = -1;
  previousSeconds = -1;
  buttonPressStartTime = 0;
  
  // for (int i = 0 ; i < EEPROM.length() ; i++) {
  //   EEPROM.write(i, 0);
  // }

  displayRemainingTime(selectedHours, selectedMinutes, 0);
}

void loop() {
  if (settingTimer) {
    int potValue = analogRead(POTENTIOMETER_PIN);
    int totalMinutes = map(potValue, 0, 1023, 0, 120);

    selectedHours = totalMinutes / 60;
    selectedMinutes = totalMinutes % 60;

    // Blink LED only during setting mode
    digitalWrite(LED_PIN, millis() % 600 < 300 ? HIGH : LOW);

    // Update display only if values change
    if (initialStartup && (selectedHours != previousHours || selectedMinutes != previousMinutes)) {
      previousHours = selectedHours;
      previousMinutes = selectedMinutes;
      displayTimerSetting();
    }

    // Handle short button press to start the timer
    if (isButtonPressed()) {
      timerDuration = (selectedHours * 3600L + selectedMinutes * 60L) * 1000L;

      EEPROM.write(TIMER_HOURS_ADDR, selectedHours);
      EEPROM.write(TIMER_MINUTES_ADDR, selectedMinutes);
      EEPROM.write(TIMER_RUNNING_ADDR, 1);

      startTime = millis();
      timerRunning = true;
      settingTimer = false;
      initialStartup = false;
      digitalWrite(LED_PIN, LOW);  // Turn off LED when exiting setting mode
    }
  } else if (timerRunning) {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - startTime;
    unsigned long remainingTime = timerDuration - elapsedTime;

    int hoursLeft = remainingTime / (3600L * 1000L);
    int minutesLeft = (remainingTime % (3600L * 1000L)) / (60L * 1000L);
    int secondsLeft = (remainingTime % (60L * 1000L)) / 1000L;

    // Handle long press to return to setup mode
    if (digitalRead(START_BUTTON_PIN) == LOW) {
      if (buttonPressStartTime == 0) {
        buttonPressStartTime = currentTime;
      }

      if (currentTime - buttonPressStartTime >= 3000) {
        backToSetting = true;
        settingTimer = true;
        initialStartup = true;
        digitalWrite(LED_PIN, HIGH);  // Start LED blinking when back to setting
        displayTimerSetting();
        return;
      }
    } else {
      buttonPressStartTime = 0;  // Reset button press time when released
    }

    EEPROM.write(REMAINING_HOURS_ADDR, hoursLeft);
    EEPROM.write(REMAINING_MINUTES_ADDR, minutesLeft);
    EEPROM.write(REMAINING_SECONDS_ADDR, secondsLeft);

    if (hoursLeft != previousHours || minutesLeft != previousMinutes || secondsLeft != previousSeconds) {
      previousHours = hoursLeft;
      previousMinutes = minutesLeft;
      previousSeconds = secondsLeft;
      displayRemainingTime(hoursLeft, minutesLeft, secondsLeft);
    }

    if (elapsedTime >= timerDuration) {
      triggerAlarm();
    }
  } else if (alarmActive) {
    if (millis() - alarmStartTime >= 3000) {
      resetTimer();
    }
  }
}
