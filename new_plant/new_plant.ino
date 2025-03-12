#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <FirebaseESP32.h>
#include <DHT11.h>

// ✅ Firebase Credentials
#define FIREBASE_HOST "smart-es-8a1fc-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "VaSzhyQl0B8yqQgCmnDozZ8W0j1BBjXSlxW2lv8k"

// ✅ Sensor Pins
#define DHT_PIN 2
#define SOIL_MOISTURE_PIN A0

// ✅ Soil Moisture Thresholds
#define DRY_THRESHOLD 700    
#define MOIST_THRESHOLD 400  

DHT11 dht11(DHT_PIN);

const char* ssid = "text";
const char* passphrase = "text";

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const int BUTTON_PIN = 13;
const int LED_BUILTIN = 2;
int lastState = HIGH;
int currentState;
unsigned long pressedTime = 0;
unsigned long releasedTime = 0;
long pressDuration = 0;
String content;

// ✅ Global Variables
int Temperature = 0;
int Humidity = 0;
int moistureValue = 0;
String soilCondition = "";  // Holds condition ("Dry", "Moist", "Wet")
int firebaseTime = 0;

void setup() {
    Serial.begin(115200);
    EEPROM.begin(512);
    delay(10);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.println("\n🚀 Startup...");

    WiFi.begin(ssid, passphrase);

    if (testWifi()) {
        Serial.println("✅ Connected to WiFi!");
    } else {
        Serial.println("❌ WiFi Failed! Starting AP mode...");
        setupAP();
    }

    config.host = FIREBASE_HOST;
    config.signer.tokens.legacy_token = FIREBASE_AUTH;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    timeClient.begin();
    timeClient.setTimeOffset(19800);

    dht11.setDelay(1000);
    createWebServer();
    server.begin();
}

// ✅ Function to Read Soil Moisture and Check Condition
void readSoilMoisture() {
    moistureValue = analogRead(SOIL_MOISTURE_PIN);
    
    Serial.print("💧 Soil Moisture Level: ");
    Serial.print(moistureValue);

    if (moistureValue > DRY_THRESHOLD) {
        soilCondition = "Dry Soil! ⚠️ Water Needed!";
    } 
    else if (moistureValue > MOIST_THRESHOLD && moistureValue <= DRY_THRESHOLD) {
        soilCondition = "Moist Soil ✅ Good Condition";
    } 
    else {
        soilCondition = "Wet Soil 💧 No Water Needed";
    }

    Serial.print(" → Condition: ");
    Serial.println(soilCondition);
}

// ✅ Function to Read Temperature & Humidity
void readDHT() {
    int temp, hum;
    int result = dht11.readTemperatureHumidity(temp, hum);

    if (result == 0) {
        Temperature = temp;
        Humidity = hum;
        Serial.print("🌡️ Temperature: ");
        Serial.print(Temperature);
        Serial.print(" °C  💧 Humidity: ");
        Serial.print(Humidity);
        Serial.println(" %");
    } else {
        Serial.println(DHT11::getErrorString(result));
    }
}

// ✅ Function to Upload Data to Firebase
void uploadDataToFirebase() {
    if (Firebase.ready()) {
        Serial.println("🔥 Uploading Data to Firebase...");
        
        Firebase.setInt(fbdo, "/devices/Temperature", Temperature);
        Firebase.setInt(fbdo, "/devices/Humidity", Humidity);
        Firebase.setInt(fbdo, "/devices/moistureValue", moistureValue);
        Firebase.setString(fbdo, "/devices/soilCondition", soilCondition);

        Serial.println("✅ Data Uploaded Successfully!");
    } else {
        Serial.println("⚠️ Firebase NOT Ready! Skipping upload.");
    }
}

void loop() {
    server.handleClient();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("✅ WiFi Connected!");

        if (Firebase.ready()) {
            Serial.println("🔥 Firebase Connected!");

            if (Firebase.getInt(fbdo, "/devices/time")) {
                firebaseTime = fbdo.intData();
                Serial.print("⏰ Firebase Time: ");
                Serial.println(firebaseTime);
            } else {
                Serial.print("❌ Failed to read Firebase time! Error: ");
                Serial.println(fbdo.errorReason());
            }
        } else {
            Serial.println("⚠️ Firebase NOT Ready! Retrying...");
        }

        readDHT();
        readSoilMoisture();
        uploadDataToFirebase();
    } else {
        Serial.println("❌ WiFi Disconnected! Reconnecting...");
        WiFi.reconnect();
    }

    delay(5000);
}

// ✅ Web Server Routes
void createWebServer() {
    server.on("/", []() {
        IPAddress ip = WiFi.softAPIP();
        content = "<html><h1>ESP32 Web Server</h1>";
        content += "<p>ESP32 Time from Firebase: " + String(firebaseTime) + "</p>";
        content += "<p>Soil Moisture Level: " + String(moistureValue) + "</p>";
        content += "<p>Soil Condition: " + soilCondition + "</p>";
        content += "<p>Use this page to configure WiFi.</p>";
        content += "<form method='get' action='setting'>";
        content += "<label>SSID:</label><input name='ssid'><label>Password:</label><input name='pass'><input type='submit'></form>";
        content += "</html>";
        server.send(200, "text/html", content);
    });

    server.on("/setting", []() {
        String qsid = server.arg("ssid");
        String qpass = server.arg("pass");

        if (qsid.length() > 0 && qpass.length() > 0) {
            Serial.println("Saving credentials to EEPROM...");
            for (int i = 0; i < 96; ++i) EEPROM.write(i, 0);
            for (int i = 0; i < qsid.length(); ++i) EEPROM.write(i, qsid[i]);
            for (int i = 0; i < qpass.length(); ++i) EEPROM.write(32 + i, qpass[i]);
            EEPROM.commit();

            content = "{\"Success\":\"Saved. Restart ESP32 to connect.\"}";
            server.send(200, "application/json", content);
            delay(1000);
            ESP.restart();
        } else {
            server.send(400, "application/json", "{\"Error\":\"Invalid Input\"}");
        }
    });
}

// ✅ WiFi Connection Test
bool testWifi() {
    int attempts = 0;
    Serial.println("🔗 Connecting to WiFi...");
    while (attempts < 20) {
        if (WiFi.status() == WL_CONNECTED) return true;
        delay(500);
        Serial.print("*");
        attempts++;
    }
    Serial.println("\nWiFi Timeout! Switching to AP Mode.");
    return false;
}

// ✅ Web Server Setup
void setupAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32_AP", "");
    Serial.println("🌍 WiFi AP Started!");
}
