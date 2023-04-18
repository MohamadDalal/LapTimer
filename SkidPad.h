#ifndef MY_SKIDPAD_H
#define MY_SKIDPAD_H

#include "RaceType.h"

class SkidPad: public RaceType{

  private:
    int skidLapIndex = 0;
    float leftLapTimes[80];
    float rightLapTimes[80];
    bool startedFirstSkid = false;

  public:
    SkidPad();
    SkidPad(int sensorPin, int* sensorThreshold, float debounceTime, AsyncEventSource* events, String htmlFileType);
    void init(int sensorPin, int* sensorThreshold, float debounceTime, AsyncEventSource* events, String htmlFileType);
    void lapTimer();
    String processor(const String& var);
    //String findFastestTime(float timesArray[], int currentLap);
    //String findAverageTime(float timesArray[], int currentLap);
    //String findSlowestTime(float timesArray[], int currentLap);
    //String findDeltaTime();
    //String findLeftDeltaTime();
    //String findRightDeltaTime();
    String getCurrentLapTime();
    void reset();
  
};


#endif
