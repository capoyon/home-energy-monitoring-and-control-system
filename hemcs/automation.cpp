#include "automation.h"
// Task function type
void executeTask(int taskNum) ;
//using TaskFunction = void (*)(int);

// Profile struct
struct Profile {
    uint8_t targetSensor; // Index of the variable to compare to 0 = time, 1 = energy, 2 = power, 3 = voltage, 4 = current
    uint8_t operation;     // 0 for equal, 1 for less than, and 2 for greater than, 3 between
    
    float wantedVal1;
    float wantedVal2; // for between
    
    // for time
    uint8_t month = 0;
    uint8_t monthDay = 0;
    uint8_t weekDay = 0;
    uint8_t hour = 0;
    uint8_t minute = 0;  

    uint8_t taskNum; // task to perform
};

// Array to hold profiles
const int MAX_PROFILES = 20;
Profile profiles[MAX_PROFILES];
int numProfiles = 0;

// Function prototypes
void addProfile(int targetSensor, int operation, int value, int taskNum);
void taskWatcher();
void task(int taskNum);

/*
  targetSensor:
    0 = Time
    1 = Energy
    2 = power
    3 = voltage
    4 = current

  operation:
    0 = equals
    1 = less than
    2 = greater than
    3 = between
/settings
  epoch: In case target sensor is time


*/

void addProfile(uint8_t targetSensor = 0 , uint8_t operation = 0, uint8_t month = 0,
                uint8_t monthDay = 0, uint8_t weekDay = 0, uint8_t hour = 0, uint8_t minute = 0,
                float wantedVal1 = 0, float wantedVal2 = 0, uint8_t taskNum = 0) {
    if (numProfiles < MAX_PROFILES) {
        profiles[numProfiles].targetSensor = targetSensor;
        profiles[numProfiles].operation = operation;
        profiles[numProfiles].month = month;
        profiles[numProfiles].monthDay = monthDay;
        profiles[numProfiles].weekDay = weekDay;
        profiles[numProfiles].hour = hour;
        profiles[numProfiles].minute = minute;
        profiles[numProfiles].wantedVal1 = wantedVal1;
        profiles[numProfiles].wantedVal2 = wantedVal2;
        profiles[numProfiles].taskNum = taskNum;
        numProfiles++;
    } else {
        Serial.printf("Max profiles reached.\n");
    }
}

void taskWatcher() {
  for (int i = 0; i < numProfiles; i++) {
    bool conditionMet = false;
    float targetSensorVal = 0.0f;

    if (profiles[i].targetSensor == 0 ) {
      // Time-based condition check
      DateTime now = rtc.now();
      uint8_t month = now.month();
      uint8_t monthDay = now.day();
      uint8_t weekDay = now.dayOfTheWeek();
      uint8_t hour = now.hour();
      uint8_t minute = now.minute();
      if ((profiles[i].month == 0 || profiles[i].month == month) &&
          (profiles[i].monthDay == 0 || profiles[i].monthDay == monthDay) &&
          (profiles[i].weekDay == 0 || profiles[i].weekDay == weekDay) &&
          (profiles[i].hour == hour) &&
          (profiles[i].minute == minute)) {
        conditionMet = true;
      }
    }
    else {
      // Sensor-based condition check
      switch (profiles[i].targetSensor) {
        case 1:
          targetSensorVal = datahandler.energy;
          break;
        case 2:
          targetSensorVal = datahandler.power;
          break; // Add break statement
        case 3:
          targetSensorVal = datahandler.voltage;
          break;
        case 4:
          targetSensorVal = datahandler.current;
          break;
        default:
          Serial.printf("Invalid target sensor in profile.\n");
          continue; // Skip this profile
      }
      
      // Operation check
      switch (profiles[i].operation) {
        case 0: // Equal
          conditionMet = (round(targetSensorVal * 100) == round(profiles[i].wantedVal1 * 100));
          break;
        case 1: // Less than
          conditionMet = (round(targetSensorVal * 100) < round(profiles[i].wantedVal1 * 100));
          break;
        case 2: // Greater than
          conditionMet = (round(targetSensorVal * 100) > round(profiles[i].wantedVal1 * 100));
          break;
        case 3: // Range
          conditionMet = (round(targetSensorVal * 100) > round(profiles[i].wantedVal1 * 100) && round(targetSensorVal * 100) < round(profiles[i].wantedVal2 * 100));
          break;
        default:
          Serial.printf("Invalid operation in profile.\n");
          continue; // Skip this profile
      }

    }
    
    if (conditionMet) {
      // Run the corresponding task
      executeTask(profiles[i].taskNum);
    }
  }
}


void executeTask(int taskNum) {
  switch (taskNum) {
      case 1:
          printf("Running Task 1\n");
          // Implement task 1
          break;
      case 2:
          printf("Running Task 2\n");
          // Implement task 2
          break;
      // Add more cases for additional tasks if needed
      default:
          printf("Invalid task number.\n");
          break;
  }
}

void removeProfile(int indexToRemove) {
  if (indexToRemove < 0 || indexToRemove >= numProfiles) {
      // Invalid index
      return;
  }

  // Shift elements after the indexToRemove one position to the left
  for (int i = indexToRemove; i < numProfiles - 1; ++i) {
      profiles[i] = profiles[i + 1];
  }

  // Decrement the count of profiles
  --numProfiles;
}