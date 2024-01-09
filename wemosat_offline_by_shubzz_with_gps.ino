#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

TinyGPSPlus gps;
SoftwareSerial SerialGPS(D6, D7); // Connect Neo-6M GPS module to GPIO 12 (RX) and GPIO 13 (TX)
// D6=TXneo    D7=RXneo

AsyncWebServer server(80);

const char* wifi_ssid = "Spider";
const char* wifi_pass = "Shubzzz@8788";
const int my_ip = 141;

Adafruit_BMP280 bmp;
//#define NEO6M_SERIAL Serial1  // Assuming you connect NEO-6M to Serial1 (RX=D7, TX=D8)


float bmpAlt, bmpTemp, bmpPressure, ref, Alt= 0;
float neoAlt, Latitude, Longitude, Speed;
float referenceAltitude = 0.0;

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}


void handleADC(AsyncWebServerRequest *request) {
  String data = "{\"BMP_Altitude\":\"" + String(bmpAlt) + "\", \"BMP_Temperature\":\"" + String(bmpTemp) + "\", \"BMP_Pressure\":\"" + String(bmpPressure) + "\", \"NEO_Altitude\":\"" + String(neoAlt) + "\", \"Latitude\":\"" + String(Latitude, 6) + "\", \"Longitude\":\"" + String(Longitude, 6) + "\", \"Speed\":\"" + String(Speed) + "\"}";

  request->send(200, "text/plain", data);
}

void setup() {
  Serial.begin(115200);
  SerialGPS.begin(9600);  // Initialize NEO-6M GPS module

  WiFi.begin(wifi_ssid, wifi_pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    IPAddress gateway = WiFi.gatewayIP();
    IPAddress staticIP(gateway[0], gateway[1], gateway[2], my_ip);
    WiFi.config(staticIP, gateway, IPAddress(255, 255, 255, 0));

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(wifi_ssid);
    Serial.print("Static IP address: ");
    Serial.println(staticIP);
    Serial.print("Gateway: ");
    Serial.println(gateway);
  }


  
  if (!bmp.begin(0x76)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
  }

 ref = bmp.readPressure() / 100;

//    referenceAltitude = gps.altitude.meters();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  // Serve gzipped Chart.min.js file at "/Chart.min.js"
  server.on("/Chart.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/Chart.min.js.gz", "text/javascript");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/readADC", HTTP_GET, handleADC);
  server.onNotFound(notFound);

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount filesystem!\n");
  }

  server.begin();
}

void loop() {
  // Read data from BMP280
  bmpAlt = bmp.readAltitude(ref);

  bmpTemp = bmp.readTemperature();
  bmpPressure = bmp.readPressure() / 1000.0F;


//if (gps.location.isValid() && gps.satellites.value() > 0) {
//    neoAlt = gps.altitude.meters();
//    Serial.println("Valid GPS data: " + String(neoAlt));
//} else {
//    Serial.println("Invalid GPS data");
//}


  // Read data from NEO-6M
//  while (NEO6M_SERIAL.available() > 0) {
//    if (gps.encode(NEO6M_SERIAL.read())) {
//      if (gps.altitude.isValid()) {
//        neoAlt = gps.altitude.meters();
//      }
//    }
//  }


while (SerialGPS.available() > 0)
    if (gps.encode(SerialGPS.read())) {
      if (gps.location.isValid()) {
        Latitude = gps.location.lat();
        Longitude = gps.location.lng();
        float neoAlt1 = gps.altitude.meters();
        Speed = gps.speed.kmph();


        neoAlt = neoAlt1 - referenceAltitude;

//        Serial.print("Latitude: ");
//        Serial.println(LatitudeString);
//        Serial.print("Longitude: ");
//        Serial.println(LongitudeString);
          Serial.print("neoAlt1: ");
          Serial.println(neoAlt);
      } 

  
  delay(50);
}
}
