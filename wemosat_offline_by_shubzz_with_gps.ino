
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <TinyGPS++.h>


//NEO-6M GPS Module:
//VCC: Connect to 5V on ESP8266 (if your NEO-6M works with 3.3V, connect it to 3.3V on ESP8266).
//GND: Connect to GND on ESP8266.
//TXD: Connect to RX (Receive) pin on ESP8266 (e.g., GPIO2 or any other digital pin you prefer).
//RXD: Connect to TX (Transmit) pin on ESP8266 (e.g., GPIO3 or any other digital pin you prefer).
//BMP280 Sensor:
//VCC: Connect to 3.3V on ESP8266.
//GND: Connect to GND on ESP8266.
//SDA: Connect to SDA (Data) pin on ESP8266 (e.g., GPIO4).
//SCL: Connect to SCL (Clock) pin on ESP8266 (e.g., GPIO5).


AsyncWebServer server(80);

const char* wifi_ssid = "iset5";
const char* wifi_pass = "iset@1234";
const int my_ip = 141;

Adafruit_BMP280 bmp;
#define NEO6M_SERIAL Serial1  // Assuming you connect NEO-6M to Serial1 (RX=D7, TX=D8)
TinyGPSPlus gps;

float bmpAlt, bmpTemp, bmpPressure, ref, Alt= 0;
float neoAlt;

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void handleADC(AsyncWebServerRequest *request) {
  String data = "{\"BMP_Altitude\":\"" + String(bmpAlt) + "\", \"BMP_Temperature\":\"" + String(bmpTemp) + "\", \"BMP_Pressure\":\"" + String(bmpPressure) + "\", \"NEO_Altitude\":\"" + String(neoAlt) + "\", \"Latitude\":\"" + String(gps.location.lat(), 6) + "\", \"Longitude\":\"" + String(gps.location.lng(), 6) + "\", \"Speed\":\"" + String(gps.speed.kmph()) + "\"}";

  request->send(200, "text/plain", data);
}

void setup() {
  Serial.begin(115200);
  NEO6M_SERIAL.begin(9600);  // Initialize NEO-6M GPS module

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


if (gps.location.isValid() && gps.satellites.value() > 0) {
    neoAlt = gps.altitude.meters();
    Serial.println("Valid GPS data: " + String(neoAlt));
} else {
    Serial.println("Invalid GPS data");
}


  // Read data from NEO-6M
//  while (NEO6M_SERIAL.available() > 0) {
//    if (gps.encode(NEO6M_SERIAL.read())) {
//      if (gps.altitude.isValid()) {
//        neoAlt = gps.altitude.meters();
//      }
//    }
//  }
   Serial.println("NEO-6M Altitude: " + String(neoAlt));

  
  delay(50);
}
