#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <FirebaseESP32.h>
#include <DHT11.h>

#define FIREBASE_HOST "smart-es-8a1fc-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "VaSzhyQl0B8yqQgCmnDozZ8W0j1BBjXSlxW2lv8k"

#define DHT_PIN 2
#define SOIL_MOISTURE_PIN A0

#define DRY_THRESHOLD 700    
#define MOIST_THRESHOLD 400  

DHT11 dht11(DHT_PIN);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const int BUTTON_PIN = 13;
const int LED_BUILTIN = 2;
String content;
int Temperature = 0, Humidity = 0, moistureValue = 0;
String soilCondition = "";
String firebaseTime = "0";

void setup() {
    Serial.begin(115200);
    EEPROM.begin(96);  // EEPROM size for SSID & Password
    delay(10);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.println("\n🚀 Startup...");

    // ✅ Read stored WiFi credentials from EEPROM
    String storedSSID = readEEPROM(0, 32);
    String storedPASS = readEEPROM(32, 64);

    Serial.print("🔹 Stored SSID: "); Serial.println(storedSSID);
    Serial.print("🔹 Stored PASS: "); Serial.println(storedPASS);

    if (storedSSID.length() > 0 && storedPASS.length() > 0) {
        WiFi.begin(storedSSID.c_str(), storedPASS.c_str());
        if (testWifi()) {
            Serial.println("✅ Connected to WiFi!");
        } else {
            Serial.println("❌ WiFi Failed! Starting AP mode...");
            setupAP();
        }
    } else {
        Serial.println("❌ No WiFi credentials found! Starting AP mode...");
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

void loop() {
    server.handleClient();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("✅ WiFi Connected!");
        timeClient.update();
        
        int currentHour = timeClient.getHours();
        int currentMinute = timeClient.getMinutes();
        int currentSecond = timeClient.getSeconds();
        Serial.printf("⏰ Current Time: %02d:%02d:%02d\n", currentHour, currentMinute, currentSecond);

        if (Firebase.ready() && Firebase.getInt(fbdo, "/devices/time")) {
            firebaseTime = fbdo.stringData();
            Serial.print("🔥 Firebase Time: ");
            Serial.println(firebaseTime);
        } else {
            Serial.print("❌ Failed to read Firebase time! Error: ");
            Serial.println(fbdo.errorReason());
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

// ✅ Reads EEPROM for SSID & Password
String readEEPROM(int start, int len) {
    String value = "";
    for (int i = start; i < start + len; i++) {
        char c = EEPROM.read(i);
        if (c != 0 && c != 255) value += c;
    }
    return value;
}

// ✅ Writes SSID & Password to EEPROM
void writeEEPROM(int start, int len, String value) {
    for (int i = 0; i < len; i++) {
        if (i < value.length()) EEPROM.write(start + i, value[i]);
        else EEPROM.write(start + i, 0);
    }
    EEPROM.commit();
}

// ✅ WiFi AP & Web Server Configuration
void setupAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32_AP", "");
    Serial.println("🌍 WiFi AP Started!");
}

// ✅ Creates Web Server for WiFi Credentials
void createWebServer() {
    server.on("/", []() {
        IPAddress ip = WiFi.softAPIP();
        content = "<html><h1>ESP32 WiFi Setup</h1>";
        content += "<form method='get' action='setting'>";
        content += "<label>SSID:</label><input name='ssid'><br>";
        content += "<label>Password:</label><input name='pass'><br>";
        content += "<input type='submit'></form>";
        content += "</html>";
        server.send(200, "text/html", content);
    });

    server.on("/setting", []() {
        String qsid = server.arg("ssid");
        String qpass = server.arg("pass");

        if (qsid.length() > 0 && qpass.length() > 0) {
            Serial.println("💾 Saving WiFi credentials...");
            writeEEPROM(0, 32, qsid);
            writeEEPROM(32, 64, qpass);
            Serial.println("✅ WiFi Credentials Saved! Restarting ESP...");
            server.send(200, "text/html", "Saved! Restarting ESP32...");
            delay(2000);
            ESP.restart();
        } else {
            server.send(400, "text/html", "❌ Invalid Input");
        }
    });
}

// ✅ Tests WiFi Connection
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

// ✅ Reads Soil Moisture
void readSoilMoisture() {
    moistureValue = analogRead(SOIL_MOISTURE_PIN);
    soilCondition = (moistureValue > DRY_THRESHOLD) ? "Dry Soil ⚠️" :
                    (moistureValue > MOIST_THRESHOLD) ? "Moist Soil ✅" : "Wet Soil 💧";
    Serial.printf("💧 Soil Moisture: %d → %s\n", moistureValue, soilCondition.c_str());
}

// ✅ Reads Temperature & Humidity
void readDHT() {
    int temp, hum;
    int result = dht11.readTemperatureHumidity(temp, hum);
    if (result == 0) {
        Temperature = temp;
        Humidity = hum;
        Serial.printf("🌡️ Temp: %d°C  💧 Humidity: %d%%\n", Temperature, Humidity);
    } else {
        Serial.println(DHT11::getErrorString(result));
    }
}

// ✅ Uploads Data to Firebase
void uploadDataToFirebase() {
    if (Firebase.ready()) {
        Serial.println("🔥 Uploading to Firebase...");
        Firebase.setInt(fbdo, "/devices/Temperature", Temperature);
        Firebase.setInt(fbdo, "/devices/Humidity", Humidity);
        Firebase.setInt(fbdo, "/devices/moistureValue", moistureValue);
        Firebase.setString(fbdo, "/devices/soilCondition", soilCondition);
        Serial.println("✅ Data Uploaded!");
    } else {
        Serial.println("⚠️ Firebase NOT Ready!");
    }
}
