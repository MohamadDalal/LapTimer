#ifndef MY_RACETYPE_H
#define MY_RACETYPE_H

#include <Arduino.h>
#include "ESPAsyncWebServer.h"

class RaceType{

  protected:
    bool initialized = false;
    int currentLap = 0;
    float lastTime = millis();
    bool started = true;
    //bool reset = false;
    int *PHOTO_SENSOR_THRESHOLD;
    int photoSensorPin;
    String htmlFileName;
    float debounceTime;
    float next;
    AsyncEventSource* events;
    
  public:
    RaceType();
    RaceType(int sensorPin, int* sensorThreshold, float debounceTime, AsyncEventSource* events, String htmlFileName);
    virtual void init(int sensorPin, int* sensorThreshold, float debounceTime, AsyncEventSource* events, String htmlFileName);
    bool isInitialized();
    bool isStarted();
    String getHtmlFileName();
    virtual void lapTimer();
    virtual String processor(const String& var);
    String findFastestTime(float timesArray[], int currentLap);
    String findAverageTime(float timesArray[], int currentLap);
    String findSlowestTime(float timesArray[], int currentLap);
    String findDeltaTime(float timesArray[], int currentLap);
    virtual String getCurrentLapTime();
    virtual void start();
    virtual void stop();
    virtual void reset();
    virtual void failLap();
    virtual void modeSwitch();
    
  
};

#endif
