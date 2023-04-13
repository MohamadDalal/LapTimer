//Inkluder relevante biblioteker
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

//Faste værdier for ssid og password til AP.
const char *ssidAP = "Laptime";
const char *passwordAP = "aauracing01";


/*
   - SSID og password
     kan ændres ovenfor.

   - Der kan være max 40 lap tider.

   - For at ændre HTML siderne skal man opdatere
     laptime.html i data mappen som ligger i samme
     mappe som denne .ino fil.

   - Der bruges SPIFFS og ESPAsyncsWebServer som
     biblioteker. En guide til at få skidtet installeret
     kan findes på følgende links:
     https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
     https://randomnerdtutorials.com/esp32-web-server-spiffs-spi-flash-file-system/

   - ESP32 køres som access point for at have
     mere stabil forbindelse.
*/

enum RaceTypes{
      //AutoCross = 1,
      Endurance = 1,
      SkidPad = 2
} RaceType;
float debounceTime = 5000;

String htmlFileName = "/endurance.html";

/*
   Vi bruger en asynkron server opsat som
   access point for at det bliver mere stabilt.
   Asynkrone servere fungerer godt med mange
   klienter forbundet, fordi serveren kan køre
   opgaver for forskellige klienter uafhængigt
   af hinanden.
*/
AsyncWebServer serverAP(80);
AsyncEventSource events("/events");
const char* PARAM_MESSAGE = "message";

//SSID og password variable
String inputSSID, inputPass;

//Globale til lapTimer
int currentLap = 0;
int skidLapIndex = 0; // Ignore lap 1,3 and lap 2 is right lap and lap 4 is left lap. Reset to 0 when lap 5 comes
int currentSkidLap = 0;
//int currentLeftLap = 0;
//int currentRightLap = 0;
float lapTimes[80];
float leftLapTimes[80];
float rightLapTimes[80];
float lastTime = millis();
bool isStarted = true;
bool reset = false;
float lapTimeStart = 0;
float lastTimeDiff = 0;


float next = millis() + 5000;
// Rigtig værdier
int PHOTO_SENSOR_THRESHOLD = 3000;
//#define photoSensorPin 32
// Værdier brugt til at teste med en joystick på pin 34
#define photoSensorPin 34
//const int PHOTO_SENSOR_THRESHOLD = 2500;

void lapSimulator() {
  if (currentLap == 0) {
    lastTime = millis();
    currentLap++;

    events.send("reload", "reload", millis());
  }
  if (millis() > next) {
    lapTimes[currentLap - 1] = millis() - lastTime;

    //Send noget random for at tjekke connection (ikke nødvendigt)
    events.send("ping", NULL, millis());

    /*
       Send "reload" event som opdaterer siden for alle
       klienter. Når den opdaterer bliver "processor"
       funktionen kørt, og dermed bliver alle placeholder
       værdier i koden opdateret. Vi beder kun klienterne
       om at opdatere når der er kommet et nyt lap. Kæft
       det er smart!
    */

    //Printer seneste laptime
    Serial.println("Lap " + String(currentLap) + ": " + lapTimes[currentLap - 1] / 1000.0);

    //Nulstiller laptiden.
    lastTime = millis();

    next += 5000;

    //Hvis der bliver over 40 laptimes så overskriv de første
    if (currentLap + 1 > 80) {
      currentLap = 1;
    }
    else {
      currentLap++;
    }
    events.send("reload", "reload", millis());
    delay(3000);
  }
}

// ----- LAPTIMERS -----
void lapTimerEndurance() {
  int photoSensorValue = analogRead(photoSensorPin);//Læs foto sensor
  if (currentLap == 0) {
    if (photoSensorValue > PHOTO_SENSOR_THRESHOLD) {
      lastTime = millis();
      currentLap++;

      events.send("reload", "reload", millis());
    }
  }
  else if ((photoSensorValue > PHOTO_SENSOR_THRESHOLD) && (millis() > (lastTime + debounceTime))) {  
    //Tilføj laptiden til array
    lapTimes[currentLap - 1] = millis() - lastTime;

    //Send noget random for at tjekke connection (ikke nødvendigt)
    events.send("ping", NULL, millis());

    /*
       Send "reload" event som opdaterer siden for alle
       klienter. Når den opdaterer bliver "processor"
       funktionen kørt, og dermed bliver alle placeholder
       værdier i koden opdateret. Vi beder kun klienterne
       om at opdatere når der er kommet et nyt lap. Kæft
       det er smart!
    */


    //Printer seneste laptime
    Serial.println("Lap " + String(currentLap) + ": " + String(lapTimes[currentLap - 1]));

    //Nulstiller laptiden.
    lastTime = millis();

    //Hvis der bliver over 40 laptimes så overskriv de første
    if (currentLap + 1 > 80) {
      currentLap = 1;
    }
    else {
      currentLap++;
    }
    events.send("reload", "reload", millis());
  }
}


void lapTimerSkidPad() {
  int photoSensorValue = analogRead(photoSensorPin);//Læs foto sensor
  /*if (currentSkidLap == 0) {
    if (photoSensorValue > PHOTO_SENSOR_THRESHOLD) {
      lastTime = millis();
      currentSkidLap++;
      events.send("reload", "reload", millis());
    }
  }*/
  /*else if (skidLapIndex == 0) {
    if (photoSensorValue > PHOTO_SENSOR_THRESHOLD) {
      lastTime = millis();
      if (currentLap + 1 > 80) {
          currentSkidLap = 1;
        }
        else {
          currentSkidLap++;
        }
      skidLapIndex++;
      events.send("reload", "reload", millis());
    }
  }*/
  if ((photoSensorValue > PHOTO_SENSOR_THRESHOLD) && (millis() > (lastTime + debounceTime))) { 
    Serial.print("Skid Lap Index is: "); Serial.println(skidLapIndex); 
    switch(skidLapIndex){
      case 0:
        lastTime = millis();
        if (currentLap + 1 > 80) {
            currentSkidLap = 1;
          }
          else {
            currentSkidLap++;
          }
        //skidLapIndex++;
        events.send("reload", "reload", millis());
      case 1:
        lastTime = millis();
        break;
      case 2:
        rightLapTimes[currentSkidLap - 1] = millis() - lastTime;
        lastTime = millis();
        //currentRightLap++;
        //events.send("reload", "reload", millis());
        break;
      case 3:
        lastTime = millis();
        break;
      case 4:
        leftLapTimes[currentSkidLap - 1] = millis() - lastTime;
        lastTime = millis();
        //currentLeftLap++;
        //Hvis der bliver over 40 laptimes så overskriv de første
        /*if (currentLap + 1 > 80) {
          currentSkidLap = 1;
        }
        else {
          currentSkidLap++;
        }*/
        //events.send("reload", "reload", millis());
        break;
      default:
        Serial.println("Reached default in LapTimerSkidPad switch case. This should not be possible");
    }
    skidLapIndex = (skidLapIndex + 1)%5;
    //Send noget random for at tjekke connection (ikke nødvendigt)
    events.send("ping", NULL, millis());
  }
}



/*
   Hjemmesiden vil konstant spørge om nogle placeholder
   værdier. Denne funktion returnerer værdierne som de
   skal have.
*/
String processor(const String& var) {
  //Serial.println(var);
  if (var == "STATUS") {
    if (isStarted) {
      return "ON";
    }
    else {
      return "OFF";
    }
  }
  if (var == "SENSORREADING"){
    return String(analogRead(photoSensorPin));
  }
  if (var == "SENSORTHRESH"){
    return String(PHOTO_SENSOR_THRESHOLD);
  }
  if (var == "LAPNUMBER") {
    if (RaceType == Endurance){
      return String(currentLap);
    }
    else if (RaceType == SkidPad){
      return String(currentSkidLap);
    }
  }
  if (var == "FASTESTTIME") {
    return findFastestTime();
  }
  if (var == "DELTATIME") {
    return findDeltaTime();
  }
  if (var == "AVERAGETIME") {
    return findAverageTime();
  }
  if (var == "SLOWESTTIME") {
    return findSlowestTime();
  }


  for (int i = 1; i < 81; i++) {
    String s = var;
    if (s == String(i)) {
      if(RaceType == Endurance){
        if (s.toInt() > currentLap) {
          return "";
        }
        else if (s == "1") {
          return ("Lap " + String(currentLap - i + 1) + ": " + String((millis() - lastTime) / 1000.0) + "...");
        }
        else if (s.toInt() <= currentLap) {
          return ("Lap " + String(currentLap - i + 1) + ": " + String(lapTimes[currentLap - i] / 1000.0));
        }
        else {
          return "";
        }
      }
      else if (RaceType == SkidPad){
        if (s.toInt() > currentSkidLap) {
          return "";
        }
        /*else if (s == "1") {
          if (skidLapIndex == 1){
            Serial.println("Lap " + String(currentSkidLap) + ": Right " + String((millis() - lastTime) / 1000.0) + "... Left ...");
            return ("Lap " + String(currentSkidLap) + ": Right " + String((millis() - lastTime) / 1000.0) + "... Left ...");
          }
          else if (skidLapIndex == 3){
            Serial.println("Lap " + String(currentSkidLap) + ": Right " + rightLapTimes[currentLap] + " Left " + String((millis() - lastTime) / 1000.0) + "...");
            return ("Lap " + String(currentSkidLap) + ": Right " + rightLapTimes[currentLap] + " Left " + String((millis() - lastTime) / 1000.0) + "...");
          }
        }*/
        else if (s.toInt() <= currentSkidLap) {
          Serial.println("Lap " + String(currentSkidLap - i + 1) + ": Right " + String(rightLapTimes[currentLap - i - 1] / 1000.0) + " Left " + String(leftLapTimes[currentLap - i - 1] / 1000.0));
          return ("Lap " + String(currentSkidLap - i + 1) + ": Right " + String(rightLapTimes[currentLap - i - 1] / 1000.0) + " Left " + String(leftLapTimes[currentLap - i - 1] / 1000.0));
        }
        else {
          return "";
        }
      }
    }
  }
  return "";
}

String findFastestTime() {
  float minValue = 100000.0;
  if (currentLap < 2) {
    return "none";
  }
  for (int i = 0; i < 80; i++) {
    if (lapTimes[i] != -1.0) {
      //Serial.println("FindFastest " + String(i+1) + ": " + String(lapTimes[i]));
      if (lapTimes[i] < minValue) {
        minValue = lapTimes[i];
      }
    }
  }
  return String(minValue / 1000.0);
}

String findSlowestTime() {
  float maxValue = 0.0;
  if (currentLap < 2) {
    return "none";
  }
  for (int i = 0; i < 80; i++) {
    if (lapTimes[i] != -1.0) {
      //Serial.println("FindSlowest " + String(i+1) + ": " + String(lapTimes[i]));
      if (lapTimes[i] > maxValue) {
        maxValue = lapTimes[i];
      }
    }
  }
  return String(maxValue / 1000.0);
}

String findAverageTime() {
  if (currentLap < 2) {
    return "none";
  }
  int totalTimes = 0;
  float addedTimes = 0.0;
  for (int i = 0; i < 80; i++) {
    if (lapTimes[i] != -1.0) {
      addedTimes += lapTimes[i];
      totalTimes++;
      //Serial.println("findAverage: currentLap: " + String(currentLap) + ", i: " + String(i) + ", Found time: " + String(lapTimes[i]));
    }
  }
  //Serial.println("AddedTimes: " + String(addedTimes) + " | TotalTimes: " + String(totalTimes));
  float averageTime = addedTimes / totalTimes;
  return String(averageTime / 1000.0);
}

String findDeltaTime() {
  if (currentLap < 3) {
    return "none";
  }
  int totalTimes = 0;
  for (int i = 0; i < 80; i++) {
    if (lapTimes[i] != -1.0) {
      totalTimes++;
    }
  }
  float deltaTime = (lapTimes[totalTimes - 1] - lapTimes[totalTimes - 2]) / 1000.0;
  Serial.println("totalTimes: " + String(totalTimes) + " | lastLap=" + String(lapTimes[totalTimes - 1]) + " | lap before that=" + String(lapTimes[totalTimes - 2]));
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



// ----- MAIN WEBSERVER -----
/*
   Server som access point til laptimer siden
*/
void lapTimePage() {
  Serial.println();
  Serial.println("Starting AP");

  //Sæt WiFi mode til AP
  WiFi.mode(WIFI_AP);

  //Initialiser AP
  WiFi.softAP(ssidAP, passwordAP);

  //Få IP-addressen til forbundne netværk og print den til serial.
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // ___________________SERVER RESPONSES___________________
  /*
     Følgende håndterer hvordan serveren skal svare når den får
     forskellige HTML requests.
  */

  //Standard request
  serverAP.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, htmlFileName.c_str(), String(), false, processor);
  });

  //Link .css til HTML siden
  serverAP.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  serverAP.on("/endurance", HTTP_GET, [](AsyncWebServerRequest * request) {
    lastTime = millis();
    next = millis() + 5000;
    RaceType = Endurance;
    htmlFileName = "/endurance.html";
    debounceTime = 5000;
    request->send(SPIFFS, htmlFileName.c_str(), String(), false, processor);
    events.send("reload", "reload", millis());
  });

  serverAP.on("/skidpad", HTTP_GET, [](AsyncWebServerRequest * request) {
    lastTime = millis();
    next = millis() + 5000;
    RaceType = SkidPad;
    htmlFileName = "/skidpad.html";
    debounceTime = 1000;
    request->send(SPIFFS, htmlFileName.c_str(), String(), false, processor);
    events.send("reload", "reload", millis());
  });
  
  serverAP.on("/start", HTTP_GET, [](AsyncWebServerRequest * request) {
    isStarted = true;
    lastTime = millis();
    next = millis() + 5000;
    request->send(SPIFFS, htmlFileName.c_str(), String(), false, processor);
    events.send("reload", "reload", millis());
    //events.send("STATUS: ON", "status", millis());
  });

  serverAP.on("/stop", HTTP_GET, [](AsyncWebServerRequest * request) {
    isStarted = false;
    lastTimeDiff = millis() - lastTime;
    request->send(SPIFFS, htmlFileName.c_str(), String(), false, processor);
    events.send("reload", "reload", millis());
    events.send("STATUS: OFF", "status", millis());
  });

  serverAP.on("/reset", HTTP_GET, [](AsyncWebServerRequest * request) {
    reset = true;
    isStarted = false;
    currentLap = 0;
    lastTimeDiff = 5000;
    request->send(SPIFFS, htmlFileName.c_str(), String(), false, processor);
    events.send("reload", "reload", millis());
  });

  serverAP.on("/getCurrentLapTime", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (isStarted) {
      if (RaceType == Endurance){
        request->send_P(200, "text/plain", ("Lap " + String(currentLap) + ": " + String((millis() - lastTime) / 1000.0) + " ...").c_str());
      }
      else if (RaceType == SkidPad){
        switch (skidLapIndex){
          case 0:
            //request->send_P(200, "text/plain", ("Lap " + String(currentSkidLap) + ": Right ... Left ...";
            request->send_P(200, "text/plain", "");
            break;
          case 1:
            request->send_P(200, "text/plain", ("Lap " + String(currentSkidLap) + ": Right ... Left ...").c_str());
            break;
          case 2:
            request->send_P(200, "text/plain", ("Lap " + String(currentSkidLap) + ": Right " + String((millis() - lastTime) / 1000.0) + "... Left ...").c_str());
            break;
          case 3:
            request->send_P(200, "text/plain", ("Lap " + String(currentSkidLap) + ": Right " + String(rightLapTimes[currentLap] / 1000.0) + " Left ...").c_str());
            break;
          case 4:
            request->send_P(200, "text/plain", ("Lap " + String(currentSkidLap) + ": Right " + String(rightLapTimes[currentLap] /1000.0) + " Left " + String((millis() - lastTime) / 1000.0) + "...").c_str());
            break;
          default:
            request->send_P(200, "text/plain", "Error: Not supposed to reach default value");
            break;
          
        }
        /*if (skidLapIndex == 1){
            //Serial.println("Lap " + String(currentSkidLap) + ": Right " + String((millis() - lastTime) / 1000.0) + "... Left ...");
            request->send_P(200, "text/plain", ("Lap " + String(currentSkidLap) + ": Right " + String((millis() - lastTime) / 1000.0) + "... Left ...").c_str());
            //return ("Lap " + String(currentSkidLap) + ": Right " + String((millis() - lastTime) / 1000.0) + "... Left ...");
        }
        else if (skidLapIndex == 3){
          //Serial.println("Lap " + String(currentSkidLap) + ": Right " + rightLapTimes[currentLap] + " Left " + String((millis() - lastTime) / 1000.0) + "...");
          request->send_P(200, "text/plain", ("Lap " + String(currentSkidLap) + ": Right " + rightLapTimes[currentLap] + " Left " + String((millis() - lastTime) / 1000.0) + "...").c_str());
          //return ("Lap " + String(currentSkidLap) + ": Right " + rightLapTimes[currentLap] + " Left " + String((millis() - lastTime) / 1000.0) + "...");
        }*/
      }
    }
    else {
      request->send_P(200, "text/plain", "");
    }
  });

  // Send hvad photo sensoren læser til hjemmesiden
  serverAP.on("/getSensorReading", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", String(analogRead(photoSensorPin)).c_str());
  });
  
  // Possible messages of PUT request are: Positive numbers, negative numbers, decimal numbers and exponents "Ex:5e3"
  // Plan is to check that string only has numbers and then return a message according to that
  // If string has something other than number then return "Only integers allowed"
  // If number is too big or too small then return a message saying it is
  // If successful return "Threshold updated succesfully"
  // Returned message is a JSON object with status and message to be displayed
  // "Threshold changed successfully" message does not really show on the website, because a reload event is initiated
  serverAP.on("/setSensorThreshold", HTTP_PUT, [](AsyncWebServerRequest * request) {
    String message;
    String responseMessage = "{\"message\":\"No Message\"}";
    if (request->hasParam(PARAM_MESSAGE, true)) {
        message = request->getParam(PARAM_MESSAGE, true)->value();
    } else {
        message = "No message sent";
    }
    Serial.print("Got set threshold request with message: "); Serial.println(message);
    // Input tinget i hjemmesiden vil ikke give værdier der er ikke tal
    // Tjekke om beskeden har minus, decimal eller eksponent
    if((message.indexOf("-") + message.indexOf("e") + message.indexOf(".")) == -3){
      // Integer was given
      int newThresh = message.toInt();
      //Serial.println(newThresh);
      if(newThresh > 4095){
        // Too high
        responseMessage = "{\"status\":-1,\"message\":\"Threshold too high\"}";
      }
      else{
        // Success
        PHOTO_SENSOR_THRESHOLD = newThresh;
        Serial.print("Set the new threshold to: "); Serial.println(newThresh);
        responseMessage = "{\"status\":0,\"message\":\"Threshold changed successfully\"}";
        events.send("reload", "reload", millis());
      }
    }
    else{
      // Non-integer was given
      responseMessage = "{\"status\":-1,\"message\":\"Threshold is not an integer\"}";
    }
    request->send(200, "application/json", responseMessage);
    Serial.println("Finished handling the set threshold request");
  });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient * client) {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });

  //Initialize event listener
  serverAP.addHandler(&events);

  //Start AP
  serverAP.begin();
  Serial.println(" AP Server started");
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(photoSensorPin, INPUT);

  //Check om SPIFFS kan køre
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occured with spiffs");
    return;
  }

  resetArr();

  //Start access point
  lapTimePage();
}

void resetArr() {
  for (int i = 0; i < 80; i++) {
    lapTimes[i] = -1.0;
    rightLapTimes[i] = -1.0;
    leftLapTimes[i] = -1.0;
  }
  reset = false;
}




void loop() {
  if (isStarted) {
    if(RaceType == Endurance){
      //debounceTime = 5000;
      //Serial.println("Endurance");
      lapTimerEndurance();
    }
    else if(RaceType == SkidPad){
      //debounceTime = 1000;
      //Serial.println("SkidPad");
      lapTimerSkidPad();
    }
    //lapTimer();       //Normal laptimer
    //lapSimulator();     //Laptimer simulator
  }
  if (reset) {
    resetArr();
  }

  delay(10);
}


/*
  // ----- FILEHANDLING -----
  //Læs .txt fil
  String readSPIFFS(const char * path) {
  Serial.printf("READSPIFFS: Reading file: %s\r\n", path);
  File file = SPIFFS.open(path, "r"); //Åbner filen
  if (!file || file.isDirectory()) { //Tjekker om filen blev åbnet
    Serial.println("READSPIFFS: Could not find the specified file");
    return String();
  }
  //Serial.println("File found. reading: ");
  String contents;

  //Mens filen er åben: læs indeholdet én char ad gangen og indsæt i string
  while (file.available()) {
    contents += String((char)file.read());
  }
  Serial.println("READSPIFFS: " + contents);
  return contents; //returner texten som string.
  }

  //Skriv .txt fil
  void writeSPIFFS(const char * path, const char * contents) {
  Serial.printf("WRITESPIFFS: Writing file: %s\r\n", path);
  File file = SPIFFS.open(path, "w"); //Åben filen
  //Tjekker om filen kan åbnes og skrives til
  if (!file) { //Tjek om filen kunne åbnes
    Serial.println("WRITESPIFFS: Failed to write the file");
    return;
  }

  //Tjekker om der blev skrevet noget til filen
  //samtidig med at den skriver.
  if (file.print(contents) && (String(contents) != "")) { //
    Serial.println("WRITESPIFFS: File was written");
  }
  else if (String(contents) == "") {
    Serial.println("WRITESPIFFS: File wrote empty string");
  }
  else {
    Serial.println("WRITESPIFFS: File write failed");
  }
  }

  //Tjekker om en fil eksisterer (SKAL OMSKRIVES)
  String fileChecker(String path, String content) {
  Serial.println("FILECHECKER: Checking: " + path);
  String contents;
  File file = SPIFFS.open(path.c_str(), "r"); //Åben filen
  if (!file || file.isDirectory()) { //Tjek om filen blev åbnet og om den findes i mappen.
    //Lav og skriv filen med standardværdien.
    writeSPIFFS(path.c_str(), content.c_str());
    return (path + " was not found");
  }
  else {
    while (file.available()) { //hvis filen eksisterer: tjek hvad filen indeholder.
      contents += String((char)file.read());
    }
    if (contents == "") { //Hvis filen ikke indeholder noget: skriv standardværdien.
      writeSPIFFS(path.c_str(), content.c_str());
      return String(path + " was empty");
    }
    else { //Hvis filen indeholder noget: print hvad den indeholder og udfør ikke noget.
      return String(path + " contained: " + contents);
    }
  }
  }
*/
