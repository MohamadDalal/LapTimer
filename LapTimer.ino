//Inkluder relevante biblioteker
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "SkidPad.h"
#include "Endurance.h"

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

enum RaceTypeIndices{
      //AutoCross = 0,
      endurance = 0,
      skidPad = 1
} RaceTypeIndex;
RaceType* RaceTypes[3];
RaceType* currentRaceType;


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


float next = millis() + 5000;
// Rigtig værdier
int PHOTO_SENSOR_THRESHOLD = 3000;
//#define photoSensorPin 32
// Værdier brugt til at teste med en joystick på pin 34
#define photoSensorPin 34
//const int PHOTO_SENSOR_THRESHOLD = 2500;



/*
   Hjemmesiden vil konstant spørge om nogle placeholder
   værdier. Denne funktion returnerer værdierne som de
   skal have.
*/
String processor(const String& var) {
  //Serial.print("Starting processor with var: "); Serial.println(var);
  return currentRaceType->processor(var);
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
    request->send(SPIFFS, currentRaceType->getHtmlFileName().c_str(), String(), false, processor);
  });

  //Link .css til HTML siden
  serverAP.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  serverAP.on("/endurance", HTTP_GET, [](AsyncWebServerRequest * request) {
    currentRaceType = RaceTypes[endurance];
    currentRaceType->modeSwitch();
    request->send(SPIFFS, currentRaceType->getHtmlFileName().c_str(), String(), false, processor);
    events.send("reload", "reload", millis());
  });

  serverAP.on("/skidpad", HTTP_GET, [](AsyncWebServerRequest * request) {
    currentRaceType = RaceTypes[skidPad];
    currentRaceType->modeSwitch();
    request->send(SPIFFS, currentRaceType->getHtmlFileName().c_str(), String(), false, processor);
    events.send("reload", "reload", millis());
  });
  
  serverAP.on("/start", HTTP_GET, [](AsyncWebServerRequest * request) {
    currentRaceType->start();
    request->send(SPIFFS, currentRaceType->getHtmlFileName().c_str(), String(), false, processor);
    events.send("reload", "reload", millis());
    //events.send("STATUS: ON", "status", millis());
  });

  serverAP.on("/stop", HTTP_GET, [](AsyncWebServerRequest * request) {
    currentRaceType->stop();
    request->send(SPIFFS, currentRaceType->getHtmlFileName().c_str(), String(), false, processor);
    events.send("reload", "reload", millis());
    events.send("STATUS: OFF", "status", millis());
  });

  serverAP.on("/reset", HTTP_GET, [](AsyncWebServerRequest * request) {
    //reset = true;
    //isStarted = false;
    currentRaceType->reset();
    request->send(SPIFFS, currentRaceType->getHtmlFileName().c_str(), String(), false, processor);
    events.send("reload", "reload", millis());
  });

  serverAP.on("/getCurrentLapTime", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (currentRaceType->isStarted()) {
        String reply = currentRaceType->getCurrentLapTime();
        request->send_P(200, "text/plain", reply.c_str());
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

  RaceTypes[endurance] = new Endurance(photoSensorPin, &PHOTO_SENSOR_THRESHOLD, 5000, &events, "/endurance.html");
  RaceTypes[skidPad] = new SkidPad(photoSensorPin, &PHOTO_SENSOR_THRESHOLD, 1000, &events, "/skidpad.html");
  currentRaceType = RaceTypes[endurance];

  //Start access point
  lapTimePage();
}





void loop() {
  if (currentRaceType->isStarted()){
    //Serial.print("Starting lap timer");
    currentRaceType->lapTimer();
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
