#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>  
#include <TinyGPS++.h>
#include <SoftwareSerial.h>


#define FIREBASE_HOST "DATABASE_URL"
#define FIREBASE_AUTH "xxxxxxxxxxxxxxx" //database secrey key
#define WIFI_SSID "xxxxxxx"
#define WIFI_PASSWORD "xxxxxxxx"

// GPS configuration
static const int gpsRxPin = D5; // Connect GPS TX to D5 on ESP8266
static const int gpsTxPin = D6; // Connect GPS RX to D6 on ESP8266
static const uint32_t GPSBaud = 9600;

// Declare the Firebase Data object in the global scope
FirebaseData firebaseData;

// Declare global variables to store GPS data
double latitude = 0;
double longitude = 0;
double speed = 0.0;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial gpsSerial(gpsRxPin, gpsTxPin);
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    lcd.print("Connecting to WiFi..");
    delay(500);
    Serial.print(".");
  }

  
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.print("Connected to WiFi");
  delay(3000);

  // Initialize GPS
  gpsSerial.begin(GPSBaud);

  lcd.clear();
  lcd.print("Connecting GPS...");

  while (!gps.location.isValid()) {
    while (gpsSerial.available() > 0) {
      gps.encode(gpsSerial.read());
      Serial.println("waiting for valid location...");
      delay(10);
    }
  }

  // Connect to Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  delay(1000);
}

void loop() {
  // Read GPS data
  while (gpsSerial.available() > 0) {
    if(gps.encode(gpsSerial.read())) {
      lcd.clear();
      lcd.print("GPS STATUS: OK");
      lcd.setCursor(0, 2);
      lcd.print("Sending data to");
      lcd.setCursor(0, 3);
      lcd.print("Database...");
      
      latitude = gps.location.lat();
      longitude = gps.location.lng();
      speed = gps.speed.kmph(); // Speed in kilometers per hour
      // latitude += 0.0001; 
      // longitude += 0.0001; 
      // speed += 1.0;

      displayInfo();
      
    }
  }

  // Update Firebase
  if (Firebase.setDouble(firebaseData, "/busses/bus2/latitude", latitude) &&
      Firebase.setDouble(firebaseData, "/busses/bus2/longitude", longitude) &&
      Firebase.setDouble(firebaseData, "/busses/bus2/speed", speed)) {
    Serial.println("Data uploaded successfully");
    Serial.print("Latitude: ");
    Serial.println(latitude, 6);
    Serial.print("Longitude: ");
    Serial.println(longitude, 6);
    Serial.print("Speed: ");
    Serial.println(speed);
    Serial.println();
    Serial.println();
  } else {
    Serial.println(firebaseData.errorReason());
    lcd.clear();
    lcd.print("Error sending data");
    delay(500);
  }

  delay(1000);
}

void displayInfo()
{
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}
