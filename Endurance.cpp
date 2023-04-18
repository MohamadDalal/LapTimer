#include "Endurance.h"

Endurance::Endurance(){}

Endurance::Endurance(int sensorPin, int* sensorThreshold, float debounceTime, AsyncEventSource* events, String htmlFileName){
  this->init(sensorPin, sensorThreshold, debounceTime, events, htmlFileName);
}

void Endurance::init(int sensorPin, int* sensorThreshold, float debounceTime, AsyncEventSource* events, String htmlFileName){
  this->photoSensorPin = sensorPin;
  this->PHOTO_SENSOR_THRESHOLD = sensorThreshold;
  this->events = events;
  this->debounceTime = debounceTime;
  this->htmlFileName = htmlFileName;
  this->reset();
  this->initialized = true;
}

void Endurance::lapTimer(){
  int photoSensorValue = analogRead(this->photoSensorPin);//Læs foto sensor
  //Serial.print("Read following sensor value:"); Serial.println(photoSensorValue);
  if ((photoSensorValue > *this->PHOTO_SENSOR_THRESHOLD) && (!this->startedFirstLap)){
    Serial.println("Starting first lap");
    this->startedFirstLap = true;
    this->lastTime = millis();
  }
  else if ((photoSensorValue > *this->PHOTO_SENSOR_THRESHOLD) && (millis() > (this->lastTime + debounceTime)) && this->startedFirstLap) {  
    //Tilføj laptiden til array
    this->lapTimes[this->currentLap] = millis() - lastTime;

    //Send noget random for at tjekke connection (ikke nødvendigt)
    this->events->send("ping", NULL, millis());

    /*
       Send "reload" event som opdaterer siden for alle
       klienter. Når den opdaterer bliver "processor"
       funktionen kørt, og dermed bliver alle placeholder
       værdier i koden opdateret. Vi beder kun klienterne
       om at opdatere når der er kommet et nyt lap. Kæft
       det er smart!
    */


    //Printer seneste laptime
    Serial.println("Lap " + String(this->currentLap) + ": " + String(this->lapTimes[this->currentLap - 1]));

    //Nulstiller laptiden.
    this->lastTime = millis();

    //Hvis der bliver over 40 laptimes så overskriv de første
    this->currentLap = (this->currentLap + 1) % 80;
    this->events->send("reload", "reload", millis());
  }
}

String Endurance::processor(const String& var){
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

  if (var == "FASTESTTIME") {return findFastestTime(this->lapTimes, this->currentLap);}
  if (var == "AVERAGETIME") {return findAverageTime(this->lapTimes, this->currentLap);}
  if (var == "SLOWESTTIME") {return findSlowestTime(this->lapTimes, this->currentLap);}
  if (var == "DELTATIME") {return findDeltaTime(this->lapTimes, this->currentLap);}

  for (int i = 1; i < 81; i++) {
    String s = var;
    if (s == String(i)) {
      if (s.toInt() > this->currentLap) {
        return "";
      }
      else if (s == "0") {return "";}
      else if (s.toInt() <= this->currentLap) {
        //Serial.println("Lap " + String(this->currentLap - i + 1) + ": " + String(this->lapTimes[this->currentLap - i] / 1000.0));
        return ("Lap " + String(this->currentLap - i + 1) + ": " + String(this->lapTimes[this->currentLap - i] / 1000.0));
      }
      else {
        return "";
      }
    }
  }
  return "";
}

String Endurance::getCurrentLapTime(){
  //Serial.println("Getting current lap time in Endurance");
  if(this->startedFirstLap){
    return "Lap " + String(this->currentLap + 1) + ": " + String((millis() - this->lastTime) / 1000.0) + " ...";
  }
  else{
    return "";
  }
}

void Endurance::reset(){
  this->currentLap = 0;
  this->startedFirstLap = false;
  for (int i = 0; i < 80; i++) {
    this->lapTimes[i] = -1.0;
  }
}

/*
 * String Endurance::findFastestTime(float timesArray[], int currentLap) {
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

String Endurance::findAverageTime(float timesArray[], int currentLap) {
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

String Endurance::findSlowestTime(float timesArray[], int currentLap) {
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

String Endurance::findDeltaTime(float timesArray[], int currentLap) {
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
