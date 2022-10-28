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
const int Foco = 21;
const int Vent = 18;

//SetPoint
int setPoint = 22;
int Sup;
int Inf;

String pwmValue;

AsyncWebServer server(80);

int estadoFoco = 1;
int estadoVentilador = 1;

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
    request->send(SPIFFS, "/index.html",String(), false);
  });

  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String(), false);
  });
  
  server.on("/sensor.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/sensor.html", String(), false);
  });

  server.on("/reloj.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/reloj.html", String(), false);
  });

  server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.css", "text/css");
  });
  
  server.on("/IP", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.localIP()).c_str());
  });
  server.on("/HOSTNAME", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.getHostname()).c_str());
  });
  server.on("/STATUS", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.status()).c_str());
  });
  server.on("/SSID", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.SSID()).c_str());
  });
  server.on("/PSK", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.psk()).c_str());
  });
  server.on("/BSSID", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.BSSIDstr()).c_str());
  });
  server.on("/RSSI", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.RSSI()).c_str());
  });

  //Encendido y apagado foco
  server.on("/FocoEstado0", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Estado foco: apagado\t");
    digitalWrite(Foco, LOW);
    estadoFoco = 0;
  });
  server.on("/FocoEstado1", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Estado foco: encendido\t");
    digitalWrite(Foco, HIGH);
    estadoFoco = 1;
  });
  //Encendido y apagado foco
  server.on("/VentiladorEstado0", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Estado ventilador: apagado\t");
    digitalWrite(Vent, LOW);
    estadoVentilador = 0;
  });
  server.on("/VentiladorEstado1", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Estado ventilador: encendido\t");
    digitalWrite(Vent, HIGH);
    estadoVentilador = 1;
  });

  //Segunda pestaña
  server.on("/SLIDER", HTTP_POST, [](AsyncWebServerRequest *request){
    pwmValue = request->arg("pwmValue");
    Serial.print("Set_point:\t");
    Serial.println(pwmValue);
    setPoint = pwmValue.toInt();
    if (T < setPoint) {
      digitalWrite(Foco, HIGH);
      digitalWrite(Vent, LOW);
      estadoFoco = 1;
      estadoVentilador = 0;
    }
    else {
      digitalWrite(Foco, LOW);
      digitalWrite(Vent, HIGH);
      estadoFoco = 0;
      estadoVentilador = 1;
    }
    request->send(SPIFFS, "/sensor.html", String(), false);
  });

  server.on("/SensorLM35", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(T).c_str());
  });
  server.on("/GraficaFoco", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(estadoFoco).c_str());
  });
  server.on("/GraficaVentilador", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(estadoVentilador).c_str());
  });
  
  server.begin();
}

void loop() {
  DS18B20.requestTemperatures();       // enviando comando de lectura
  T = DS18B20.getTempCByIndex(0);  // lectura de temperatura en °C
  Serial.print("Temperature: ");
  Serial.println(T);                // imprimiendo temperatura en °C
  delay(500);
}
