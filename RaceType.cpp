#include "RaceType.h"

bool RaceType::isInitialized(){return this->initialized;};

String RaceType::findFastestTime(float timesArray[], int currentLap) {
  float minValue = 100000.0;
  if (currentLap < 2) {
    return "none";
  }
  for (int i = 0; i < 80; i++) {
    if (timesArray[i] != -1.0) {
      //Serial.println("FindFastest " + String(i+1) + ": " + String(lapTimes[i]));
      if (timesArray[i] < minValue) {
        minValue = timesArray[i];
      }
    }
  }
  return String(minValue / 1000.0);
}

String RaceType::findAverageTime(float timesArray[], int currentLap) {
  if (currentLap < 2) {
    return "none";
  }
  int totalTimes = 0;
  float addedTimes = 0.0;
  for (int i = 0; i < 80; i++) {
    if (timesArray[i] != -1.0) {
      addedTimes += timesArray[i];
      totalTimes++;
      //Serial.println("findAverage: currentLap: " + String(currentLap) + ", i: " + String(i) + ", Found time: " + String(lapTimes[i]));
    }
  }
  //Serial.println("AddedTimes: " + String(addedTimes) + " | TotalTimes: " + String(totalTimes));
  float averageTime = addedTimes / totalTimes;
  return String(averageTime / 1000.0);
}

String RaceType::findSlowestTime(float timesArray[], int currentLap) {
  float maxValue = 0.0;
  if (currentLap < 2) {
    return "none";
  }
  for (int i = 0; i < 80; i++) {
    if (timesArray[i] != -1.0) {
      //Serial.println("FindSlowest " + String(i+1) + ": " + String(lapTimes[i]));
      if (timesArray[i] > maxValue) {
        maxValue = timesArray[i];
      }
    }
  }
  return String(maxValue / 1000.0);
}

String RaceType::findDeltaTime(float timesArray[], int currentLap) {
  if (currentLap < 2) {
    return "none";
  }
  int totalTimes = 0;
  for (int i = 0; i < 80; i++) {
    if (timesArray[i] != -1.0) {
      totalTimes++;
    }
  }
  float deltaTime = (timesArray[totalTimes - 1] - timesArray[totalTimes - 2]) / 1000.0;
  Serial.println("totalTimes: " + String(totalTimes) + " | lastLap=" + String(timesArray[totalTimes - 1]) + " | lap before that=" + String(timesArray[totalTimes - 2]));
  if (deltaTime > 0.0) {
    return ("+" + String(abs(deltaTime)));
  }
  else if (deltaTime < 0.0) {
    return ("-" + String(abs(deltaTime)));
  }
  else {
    return String(abs(deltaTime));
  }
}
