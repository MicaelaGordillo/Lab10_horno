// Importamos las librerías
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"

#include "config.h"

#define SENSOR_PIN  19 // ESP32 pin GIOP21 conectado al pin DQ del DS18B20

OneWire oneWire(SENSOR_PIN);  // asignando el pin de lectura
DallasTemperature DS18B20(&oneWire);

float T; // temperatura en Celsius

//reles
const int Foco = 18;
const int Vent = 21;

//SetPoint
int setPoint = 22;
int Sup;
int Inf;

int page = 0;
AsyncWebServer server(80);

String getRSSI(){
  return String(WiFi.RSSI());
}
void setup() {
  Serial.begin(115200);
  DS18B20.begin();      // initializando el sensor DS18B20
  pinMode(Vent,OUTPUT);
  pinMode(Foco,OUTPUT);

  // Iniciamos  SPIFFS
  if (!SPIFFS.begin())
  { Serial.println("ha ocurrido un error al montar SPIFFS");
    return;
  }
  // conectamos al Wi-Fi
  WiFi.begin(ssid, password);
  // Mientras no se conecte, mantenemos un bucle con reintentos sucesivos
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    // Esperamos un segundo
    Serial.println("Conectando a la red WiFi..");
  }
  Serial.println();
  Serial.println(WiFi.SSID());
  Serial.print("Direccion IP:\t");
  // Imprimimos la ip que le ha dado nuestro router
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html",String(), false, datos);
  });

  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    page = 0;
    request->send(SPIFFS, "/index.html", String(), false);
  });
  
  server.on("/serial.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    page = 1;
    request->send(SPIFFS, "/serial.html", String(), false, datos);
  });

  server.on("/reloj.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    page = 2;
    request->send(SPIFFS, "/reloj.html", String(), false, datos);
  });

  server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.css", "text/css");
  });
  
  server.on("/RSSI", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain",getRSSI().c_str());
  });

  //Encendido y apagado foco
  server.on("/FocoEstado0", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Estado foco: apagado\t");
    digitalWrite(Foco, LOW);
  });
  server.on("/FocoEstado1", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Estado foco: encendido\t");
    digitalWrite(Foco, HIGH);
  });
  //Encendido y apagado foco
  server.on("/VentiladorEstado0", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Estado ventilador: apagado\t");
    digitalWrite(Vent, LOW);
  });
  server.on("/VentiladorEstado1", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Estado ventilador: encendido\t");
    digitalWrite(Vent, HIGH);
  });
  
  server.begin();
}

String datos(const String& var) {
  if (var == "IP") {
    return WiFi.localIP().toString();
  }else if (var == "HOSTNAME") {
    return String(WiFi.SSID());
  }else if (var == "STATUS") {
    return String(WiFi.status());
  }else if (var == "PSK") {
    return String(WiFi.psk());
  }else if (var == "RSSI") {
    return String(WiFi.RSSI());
  }
  return var;
}

void loop() {
  DS18B20.requestTemperatures();       // enviando comando de lectura
  T = DS18B20.getTempCByIndex(0);  // lectura de temperatura en °C
  Serial.print("Temperature: ");
  Serial.println(T);                // imprimiendo temperatura en °C

  if (T < setPoint)
  {
    digitalWrite(Foco, HIGH);
    digitalWrite(Vent, LOW);
  }
  else
  {
    digitalWrite(Foco, LOW);
    digitalWrite(Vent, HIGH);
  }

  delay(500);
}
