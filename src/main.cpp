#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include<Servo.h>
#include<DNSServer.h>
#include<ESP8266WebServer.h>
#include<WiFiManager.h>
#include<ArduinoJson.h>


#ifndef STASSID
#define STASSID "WLAN1-4221G5"
#define STAPSK  "HutMountainHouse"
#endif


Servo myservo;
int threshold = 60;
int isOn;
int led = 4;
boolean ledison = false;

void PressButton() {
  if (analogRead(A0) > threshold) {
    //if the heating is on then it will usually take 4 button presses to turn it off, check that it has been off twice before stopping
    bool last = true;
    for (int i = 0; i < 4; i++) {
      myservo.write(0);
      delay(500);
      myservo.write(90);
      delay(500);

      if (analogRead(A0) < threshold && !last)
        break;
      last = analogRead(A0) > threshold;
    }

  } else {
    for (int i = 0; i < 2; i++) {
      myservo.write(0);
      delay(500);
      myservo.write(90);
      delay(900);
      if (analogRead(A0) > threshold)
        break;
    }

  }

}

void toggleLed() {
  if (ledison)
    digitalWrite(led, LOW);
  else
    digitalWrite(led, HIGH);
  ledison = !ledison;
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

bool pollHTTP(bool isOn){
  int toggle;
    if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, "http://192.168.8.148:8000/heating/usage"); //HTTP
    
    http.addHeader("Content-Type", "application/json");
    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header and body
    
    DynamicJsonDocument doc(1024);

    doc["is_on"] = isOn;
    String json;
    serializeJson(doc, json);
    int httpCode = http.POST(json);   
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        toggle = payload == "1";
        
        Serial.println("received payload!!:\n<<");
        Serial.println(payload);
        Serial.println(isOn);
        Serial.println(toggle);
        return toggle;
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    
  }
  return toggle;
}

void activateHeating(){
  
  isOn = (analogRead(A0) > threshold ? 1 : 0);
  Serial.println(isOn);
  if(pollHTTP(isOn)){
    Serial.println("TOGGLING!");
    PressButton();
  }
}



void setup() {
  myservo.attach(5);
  myservo.write(90);
  Serial.begin(115200);



  WiFiManager WiFiManager;
  WiFiManager.autoConnect("Heating Controller");

  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  // wait for WiFi connection

  activateHeating();
  delay(20000);
}