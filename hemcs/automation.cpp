#include "automation.h"

// Global variables (simulated inputs)
int variables[4] = {0, 1, 2, 3}; // Array to hold variables

// Task function type
using TaskFunction = void (*)(int);

// Profile struct
struct Profile {
    int sensorVariable; // Index of the variable to compare to
    int operation;     // 0 for equal, 1 for less than, and 2 for greater than
    int value;         // The value it would compare
    int taskNum;       // Number of the task
};

// Array to hold profiles
const int MAX_PROFILES = 20;
Profile profiles[MAX_PROFILES];
int numProfiles = 0;

// Function prototypes
void addProfile(int sensorVariable, int operation, int value, int taskNum);
void taskWatcher();
void task(int taskNum);

// int main() {
//     // Example usage
//     addProfile(0, 0, 10, 1); // If var0 == 10, run task 1
//     addProfile(1, 1, 20, 2); // If var1 < 20, run task 2

//     // Simulate changing variable values
//     variables[0] = 10;
//     variables[1] = 10;

//     // Check if any profiles are matched and run corresponding tasks
//     taskWatcher();

//     return 0;
// }

/*
  sensorVariable:
    1 = Energy
    2 = power
    3 = voltage
    4 = current

  operation:
    1 = equals
    2 = between
    3 = less than
    4 = greater than

  value:
    the value to be compared with the sensor val
*/

void addProfile(int sensorVariable, int operation, int value, int taskNum) {
    if (numProfiles < MAX_PROFILES) {
        profiles[numProfiles].sensorVariable = sensorVariable;
        profiles[numProfiles].operation = operation;
        profiles[numProfiles].value = value;
        profiles[numProfiles].taskNum = taskNum;
        numProfiles++;
    } else {
        Serial.println("Max profiles reached.");
    }
}

void taskWatcher() {
    for (int i = 0; i < numProfiles; i++) {
        int variableValue = 0; // Initialize variableValue
        if (profiles[i].sensorVariable >= 0 && profiles[i].sensorVariable < 4) {
            variableValue = variables[profiles[i].sensorVariable];
        } else {
            Serial.println("Invalid variable index in profile.");
            continue; // Skip this profile
        }

        bool conditionMet = false;
        switch (profiles[i].operation) {
            case 0: // Equal
                conditionMet = (variableValue == profiles[i].value);
                break;
            case 1: // Less than
                conditionMet = (variableValue < profiles[i].value);
                break;
            case 2: // Greater than
                conditionMet = (variableValue > profiles[i].value);
                break;
            default:
                Serial.println("Invalid operation in profile.");
                continue; // Skip this profile
        }

        if (conditionMet) {
            // Run the corresponding task
            task(profiles[i].taskNum);
        }
    }
}

void task(int taskNum) {
    switch (taskNum) {
        case 1:
            Serial.println("Running Task 1");
            // Implement task 1
            break;
        case 2:
            Serial.println("Running Task 2");
            // Implement task 2
            break;
        // Add more cases for additional tasks if needed
        default:
            Serial.println("Invalid task number.");
            break;
    }
}
