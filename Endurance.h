#ifndef MY_ENDURANCE_H
#define MY_ENDURANCE_H

#include "RaceType.h"

class Endurance: public RaceType{

  private:
    float lapTimes[80];
    bool startedFirstLap = false;

  public:
    Endurance();
    Endurance(int sensorPin, int* sensorThreshold, float debounceTime, AsyncEventSource* events, String htmlFileName);
    void init(int sensorPin, int* sensorThreshold, float debounceTime, AsyncEventSource* events, String htmlFileName);
    void lapTimer();
    String processor(const String& var);
    //String findFastestTime(float timesArray[], int currentLap);
    //String findAverageTime(float timesArray[], int currentLap);
    //String findSlowestTime(float timesArray[], int currentLap);
    //String findDeltaTime();
    String getCurrentLapTime();
    void reset();
};

#endif
