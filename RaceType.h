#ifndef MY_RACETYPE_H
#define MY_RACETYPE_H

#include <Arduino.h>

class RaceType{

  protected:
    //int currentLap = 0;
    //float lapTimes[80];
    float lastTime = millis();
    bool isStarted = true;
    bool reset = false;
    //float lapTimeStart = 0; // Not used
    //float lastTimeDiff = 0; // Not used
    int PHOTO_SENSOR_THRESHOLD;
    int photoSensorPin;
    float debounceTime;
    float next;
    
  public:
    RaceType();
    void lapTimer();
    void resetArr();
    void processor(const String& var);
  
};

#endif
