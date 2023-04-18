#include "SkidPad.h"

SkidPad::SkidPad(){}

SkidPad::SkidPad(int sensorPin, int* sensorThreshold, float debounceTime, AsyncEventSource* events, String htmlFileName){
  this->init(sensorPin, sensorThreshold, debounceTime, events, htmlFileName);
}

void SkidPad::init(int sensorPin, int* sensorThreshold, float debounceTime, AsyncEventSource* events, String htmlFileName){
  this->photoSensorPin = sensorPin;
  this->PHOTO_SENSOR_THRESHOLD = sensorThreshold;
  this->events = events;
  this->debounceTime = debounceTime;
  this->htmlFileName = htmlFileName;
  this->reset();
  this->initialized = true;
}

void SkidPad::lapTimer(){
  int photoSensorValue = analogRead(this->photoSensorPin);//Læs foto sensor
  if ((photoSensorValue > *this->PHOTO_SENSOR_THRESHOLD) && (millis() > (this->lastTime + debounceTime))) { 
    Serial.print("Skid Lap Index is: "); Serial.println(this->skidLapIndex); 
    switch(this->skidLapIndex){
      case 0:
        this->lastTime = millis();                // Needed for debouncing
        break;
      case 1:
        this->lastTime = millis();
        break;
      case 2:
        this->rightLapTimes[this->currentLap] = millis() - lastTime;
        this->lastTime = millis();
        break;
      case 3:
        this->lastTime = millis();                // Needed for debouncing
        break;
      case 4:
        this->leftLapTimes[this->currentLap] = millis() - lastTime;
        this->lastTime = millis();
        this->currentLap = (this->currentLap + 1) % 80;
        this->events->send("reload", "reload", millis());
        break;
      default:
        Serial.println("Reached default in LapTimerSkidPad switch case. This should not be possible");
    }
    skidLapIndex = (skidLapIndex + 1)%5;
    //Send noget random for at tjekke connection (ikke nødvendigt)
    events->send("ping", NULL, millis());
  }
}

String SkidPad::processor(const String& var){
  //Serial.println(var);
  /*switch(var){
    case "STATUS":
      if (this->isStarted) {
      return "ON";
      }
      else {
        return "OFF";
      }
      break;
    case "SENSORREADING": return String(analogRead(this->photoSensorPin)); break;
    case "SENSORTHRESH": return String(*this->PHOTO_SENSOR_THRESHOLD); break;
    case "LEFTFASTESTTIME": return this->findFastestTime(this->leftLapTimes, this->currentLap); break;
    case "LEFTAVERAGETIME": return this->findAverageTime(this->leftLapTimes, this->currentLap); break;
    case "LEFTSLOWESTTIME": return this->findSlowestTime(this->leftLapTimes, this->currentLap); break;
    case "LEFTDELTATIME": return this->findLeftDeltaTime(); break;
    case "RIGHTFASTESTTIME": return this->findFastestTime(this->rightLapTimes, this->currentLap); break;
    case "RIGHTAVERAGETIME": return this->findAverageTime(this->rightLapTimes, this->currentLap); break;
    case "RIGHTSLOWESTTIME": return this->findSlowestTime(this->rightLapTimes, this->currentLap); break;
    case "RIGHTDELTATIME": return this->findLeftDeltaTime(); break;
    default:
      for (int i = 1; i < 81; i++) {
        String s = var;
        if (s == String(i)) {
          if (s.toInt() > this->currentLap) {
            return "";
          }
          else if (s == "0") {return "";}
          else if (s.toInt() <= this->currentLap) {
            Serial.println("Lap " + String(this->currentLap - i + 1) + ": Right " + String(this->rightLapTimes[this->currentLap - i] / 1000.0) + " Left " + String(this->leftLapTimes[this->currentLap - i] / 1000.0));
            return ("Lap " + String(this->currentLap - i + 1) + ": Right " + String(this->rightLapTimes[this->currentLap - i] / 1000.0) + " Left " + String(this->leftLapTimes[this->currentLap - i] / 1000.0));
          }
          else {
            return "";
          }
        }
      }
  }*/
  //Serial.println(var);
  if (var == "STATUS") {
    if (this->isStarted()) {
      return "ON";
    }
    else {
      return "OFF";
    }
  }
  if (var == "SENSORREADING"){return String(analogRead(this->photoSensorPin));}
  if (var == "SENSORTHRESH"){return String(*this->PHOTO_SENSOR_THRESHOLD);}
  if (var == "LAPNUMBER") {return String(this->currentLap);}

  if (var == "LEFTFASTESTTIME") {return findFastestTime(this->leftLapTimes, this->currentLap);}
  if (var == "LEFTAVERAGETIME") {return findAverageTime(this->leftLapTimes, this->currentLap);}
  if (var == "LEFTSLOWESTTIME") {return findSlowestTime(this->leftLapTimes, this->currentLap);}
  if (var == "LEFTDELTATIME") {return findDeltaTime(this->leftLapTimes, this->currentLap);}
  //if (var == "LeftDELTATIME") {return findleftDeltaTime();}

  if (var == "RIGHTFASTESTTIME") {return findFastestTime(this->rightLapTimes, this->currentLap);}
  if (var == "RIGHTAVERAGETIME") {return findAverageTime(this->rightLapTimes, this->currentLap);}
  if (var == "RIGHTSLOWESTTIME") {return findSlowestTime(this->rightLapTimes, this->currentLap);}
  if (var == "RIGHTDELTATIME") {return findDeltaTime(this->rightLapTimes, this->currentLap);}
  //if (var == "RIGHTDELTATIME") {return findRightDeltaTime();}


  for (int i = 1; i < 81; i++) {
    String s = var;
    if (s == String(i)) {
      if (s.toInt() > this->currentLap) {
        return "";
      }
      else if (s == "0") {return "";}
      else if (s.toInt() <= this->currentLap) {
        Serial.println("Lap " + String(this->currentLap - i + 1) + ": Right " + String(this->rightLapTimes[this->currentLap - i] / 1000.0) + " Left " + String(this->leftLapTimes[this->currentLap - i] / 1000.0));
        return ("Lap " + String(this->currentLap - i + 1) + ": Right " + String(this->rightLapTimes[this->currentLap - i] / 1000.0) + " Left " + String(this->leftLapTimes[this->currentLap - i] / 1000.0));
      }
      else {
        return "";
      }
    }
  }
  return "";
}

String SkidPad::getCurrentLapTime(){
  String returnString = "";
  switch (skidLapIndex){
    case 0:
      returnString = "";
      break;
    case 1:
      returnString = "Lap " + String(this->currentLap+1) + ": Left ... Right ...";
      break;
    case 2:
      returnString = "Lap " + String(this->currentLap+1) + ": Left ..." + " Right " + String((millis() - this->lastTime) / 1000.0) + "...";
      break;
    case 3:
      returnString = "Lap " + String(this->currentLap+1) + ": Left ..." + " Right " + String(this->rightLapTimes[this->currentLap] / 1000.0);
      break;
    case 4:
      returnString = "Lap " + String(this->currentLap+1) + ": Left " + String((millis() - lastTime) / 1000.0) + "..." + " Right " + String(this->rightLapTimes[this->currentLap] /1000.0);
      break;
    default:
      returnString = "Error: Not supposed to reach default value";
      break;
  }
  return returnString;
}

void SkidPad::reset(){
  this->currentLap = 0;
  this->skidLapIndex = 0;
  for (int i = 0; i < 80; i++) {
    this->rightLapTimes[i] = -1.0;
    this->leftLapTimes[i] = -1.0;
  }
}
/*
String SkidPad::findFastestTime(float timesArray[], int currentLap) {
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

String SkidPad::findAverageTime(float timesArray[], int currentLap) {
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

String SkidPad::findSlowestTime(float timesArray[], int currentLap) {
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

String SkidPad::findDeltaTime(float timesArray[], int currentLap) {
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
*/
/*
String SkidPad::findLeftDeltaTime() {
  if (this->currentLap < 2) {
    return "none";
  }
  int totalTimes = 0;
  for (int i = 0; i < 80; i++) {
    if (this->leftLapTimes[i] != -1.0) {
      totalTimes++;
    }
  }
  float deltaTime = (this->leftLapTimes[totalTimes - 1] - this->leftLapTimes[totalTimes - 2]) / 1000.0;
  Serial.println("LeftLap totalTimes: " + String(totalTimes) + " | lastLap=" + String(this->leftLapTimes[totalTimes - 1]) + " | lap before that=" + String(this->leftLapTimes[totalTimes - 2]));
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

String SkidPad::findRightDeltaTime() {
  if (this->currentLap < 2) {
    return "none";
  }
  int totalTimes = 0;
  for (int i = 0; i < 80; i++) {
    if (this->rightLapTimes[i] != -1.0) {
      totalTimes++;
    }
  }
  float deltaTime = (this->rightLapTimes[totalTimes - 1] - this->rightLapTimes[totalTimes - 2]) / 1000.0;
  Serial.println("RightLap totalTimes: " + String(totalTimes) + " | lastLap=" + String(this->rightLapTimes[totalTimes - 1]) + " | lap before that=" + String(this->rightLapTimes[totalTimes - 2]));
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
*/
