#include <OneWire.h>
#include <DallasTemperature.h>

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
void setup() {
  Serial.begin(115200);
  DS18B20.begin();      // initializando el sensor DS18B20
  pinMode(Rele,OUTPUT);
  pinMode(Foco,OUTPUT);
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
