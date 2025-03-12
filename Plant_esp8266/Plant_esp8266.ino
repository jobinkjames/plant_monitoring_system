#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <FirebaseESP8266.h>

#define FIREBASE_HOST "smart-es-8a1fc-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "VaSzhyQl0B8yqQgCmnDozZ8W0j1BBjXSlxW2lv8k" 

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

const int BUTTON_PIN = D7;
const int SHORT_PRESS_TIME = 2000;
int lastState = HIGH;
int currentState;
unsigned long pressedTime = 0;
unsigned long releasedTime = 0;
long pressDuration = 0;

int status = 0;
float voltage1 = 0;
float percentage = 0;
float temperature = 0;
float humidity = 0;
int moisture = 0;
int status1 = 0;
int status2 = 0;
int ontime1 = 0;
int ontime2 = 0;
int offtime1 = 0;
int offtime2 = 0;

const char* ssid = "your_wifi_ssid";
const char* passphrase = "your_wifi_password";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    Serial.begin(115200);
    EEPROM.begin(512);
    delay(10);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(D0, OUTPUT);
    
    Serial.println("Starting...");
    WiFi.begin(ssid, passphrase);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nWiFi Connected!");

    // Firebase Initialization
    config.host = FIREBASE_HOST;
    config.signer.tokens.legacy_token = FIREBASE_AUTH;
    
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

void loop() {
    apsetbutton();
    
    if (WiFi.status() == WL_CONNECTED) {
        Firebase.setInt(fbdo, "/devices/voltage", voltage1);
        Firebase.setInt(fbdo, "/devices/percentage", percentage);
        Firebase.setInt(fbdo, "/devices/status", status);
        Firebase.setFloat(fbdo, "/devices/temperature", temperature);
        Firebase.setFloat(fbdo, "/devices/humidity", humidity);
        Firebase.setInt(fbdo, "/devices/moisture", moisture);
        Firebase.setInt(fbdo, "/devices/status1", status1);
        Firebase.setInt(fbdo, "/devices/status2", status2);
        Firebase.setInt(fbdo, "/devices/ontime1", ontime1);
        Firebase.setInt(fbdo, "/devices/ontime2", ontime2);
        Firebase.setInt(fbdo, "/devices/offtime1", offtime1);
        Firebase.setInt(fbdo, "/devices/offtime2", offtime2);
        Serial.println("Data uploaded to Firebase");
    }
    delay(5000); // Upload every 5 seconds
}

void apsetbutton()
 {
  currentState = digitalRead(BUTTON_PIN);
  if(lastState == HIGH && currentState == LOW)        // button is pressed
    pressedTime = millis();
  else if(lastState == LOW && currentState == HIGH) 
  { // button is released
    releasedTime = millis();

     pressDuration = releasedTime - pressedTime;

    if(pressDuration > SHORT_PRESS_TIME )
    {
      Serial.println("Turning the HotSpot On");
      launchWeb();
       setupAP();
         while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print(".");
    delay(100);
    server.handleClient();
  }
    }
  }

  // save the the last state
  lastState = currentState;
 }
