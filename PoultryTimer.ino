#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// EEPROM Address Definitions
const int TIMER_HOURS_ADDR = 0;
const int TIMER_MINUTES_ADDR = 1;
const int REMAINING_HOURS_ADDR = 2;
const int REMAINING_MINUTES_ADDR = 3;
const int REMAINING_SECONDS_ADDR = 4;
const int TIMER_RUNNING_ADDR = 5;

// Pin Definitions
const int POTENTIOMETER_PIN = A0;
const int BUZZER_PIN = 9;
const int START_BUTTON_PIN = 2;

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
int selectedHours = 0;
int selectedMinutes = 0;
int previousHours = -1;
int previousMinutes = -1;
int previousSeconds = -1;

// Button Debounce Variables
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 50;

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
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time Left:");
  
  lcd.setCursor(0, 1);
  
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
  tone(BUZZER_PIN, 1000);
}

void displayTimerSetting() {
  if (initialStartup) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set Timer:");
    
    lcd.setCursor(0, 1);
    // Hours with leading zero
    if (selectedHours < 10) lcd.print("0");
    lcd.print(selectedHours);
    lcd.print(":");
    
    // Minutes with leading zero
    if (selectedMinutes < 10) lcd.print("0");
    lcd.print(selectedMinutes);
    lcd.print(":00    ");
  }
}

void setup() {
  pinMode(POTENTIOMETER_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  
  lcd.init();
  lcd.backlight();
  
  // Check if timer was running before power loss
  checkPreviousTimerState();
}

void checkPreviousTimerState() {
  byte storedRunningState = EEPROM.read(TIMER_RUNNING_ADDR);
  
  if (storedRunningState == 1) {
    // Recover previous timer settings
    selectedHours = EEPROM.read(REMAINING_HOURS_ADDR);
    selectedMinutes = EEPROM.read(REMAINING_MINUTES_ADDR);
    int remainingSeconds = EEPROM.read(REMAINING_SECONDS_ADDR);
    
    // Recalculate timer duration
    timerDuration = (selectedHours * 3600L + selectedMinutes * 60L + remainingSeconds) * 1000L;
    
    // Start timer from where it left off
    startTime = millis() - (timerDuration - (selectedHours * 3600L + selectedMinutes * 60L + remainingSeconds) * 1000L);
    
    timerRunning = true;
    settingTimer = false;
    initialStartup = false;
  } else {
    // Normal startup - display timer setting
    displayTimerSetting();
  }
}

void resetTimer() {
  settingTimer = false;
  alarmActive = false;
  timerRunning = true;

  // Read the potentiometer value to set a new timer
  int potValue = analogRead(POTENTIOMETER_PIN);
  int totalMinutes = map(potValue, 0, 1023, 0, 120);

  selectedHours = totalMinutes / 60;
  selectedMinutes = totalMinutes % 60;

  // Recalculate timer duration with the new potentiometer position
  timerDuration = (selectedHours * 3600L + selectedMinutes * 60L) * 1000L;

  // Store the new timer settings in EEPROM
  EEPROM.write(TIMER_HOURS_ADDR, selectedHours);
  EEPROM.write(TIMER_MINUTES_ADDR, selectedMinutes);
  EEPROM.write(TIMER_RUNNING_ADDR, 1);

  // Reset start time
  startTime = millis();

  noTone(BUZZER_PIN);

  // Clear previous tracking variables
  previousHours = -1;
  previousMinutes = -1;
  previousSeconds = -1;
  buttonPressStartTime = 0;

  // Display the updated timer settings
  displayRemainingTime(selectedHours, selectedMinutes, 0);
}

void loop() {
  if (settingTimer) {
    int potValue = analogRead(POTENTIOMETER_PIN);
    
    // Map potentiometer to 0-120 minutes (2 hours max)
    int totalMinutes = map(potValue, 0, 1023, 0, 120);
    
    selectedHours = totalMinutes / 60;
    selectedMinutes = totalMinutes % 60;
    
    if (initialStartup && (selectedHours != previousHours || selectedMinutes != previousMinutes)) {
      previousHours = selectedHours;
      previousMinutes = selectedMinutes;
      displayTimerSetting();
    }
    
    if (isButtonPressed()) {
      timerDuration = (selectedHours * 3600L + selectedMinutes * 60L) * 1000L;
      
      // Store initial timer settings
      EEPROM.write(TIMER_HOURS_ADDR, selectedHours);
      EEPROM.write(TIMER_MINUTES_ADDR, selectedMinutes);
      EEPROM.write(TIMER_RUNNING_ADDR, 1);
      
      startTime = millis();
      timerRunning = true;
      settingTimer = false;
      initialStartup = false;
    }
  } 
  else if (timerRunning) {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - startTime;
    unsigned long remainingTime = timerDuration - elapsedTime;
    
    int hoursLeft = remainingTime / (3600L * 1000L);
    int minutesLeft = (remainingTime % (3600L * 1000L)) / (60L * 1000L);
    int secondsLeft = (remainingTime % (60L * 1000L)) / 1000L;
    
    // Check for manual reset (button held for 5 seconds)
    if (digitalRead(START_BUTTON_PIN) == LOW) {
      if (buttonPressStartTime == 0) {
        buttonPressStartTime = currentTime;
      }
      
      if (currentTime - buttonPressStartTime >= 5000) {
        resetTimer();
        return;
      }
    } else {
      buttonPressStartTime = 0;
    }
    
    // Store remaining time periodically
    EEPROM.write(REMAINING_HOURS_ADDR, hoursLeft);
    EEPROM.write(REMAINING_MINUTES_ADDR, minutesLeft);
    EEPROM.write(REMAINING_SECONDS_ADDR, secondsLeft);
    
    if (hoursLeft != previousHours || 
        minutesLeft != previousMinutes || 
        secondsLeft != previousSeconds) {
      previousHours = hoursLeft;
      previousMinutes = minutesLeft;
      previousSeconds = secondsLeft;
      displayRemainingTime(hoursLeft, minutesLeft, secondsLeft);
    }
    
    if (elapsedTime >= timerDuration) {
      triggerAlarm();
    }
  } 
  else if (alarmActive) {
    // Automatically reset after 3 seconds
    if (millis() - alarmStartTime >= 3000) {
      resetTimer();
    }
  }
}