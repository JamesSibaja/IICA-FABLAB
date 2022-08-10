#include <WiFi101.h>
#include <OneWire.h>                
#include <DallasTemperature.h>
#include <SD.h>
#include <SPI.h>

OneWire ourWire(2);

DallasTemperature sensors(&ourWire);

DeviceAddress address1 = {0x28, 0x1C, 0x8F, 0x79, 0xA2, 0x0, 0x3, 0xFC};
DeviceAddress address2 = {0x28, 0x89, 0xC5, 0x79, 0xA2, 0x0, 0x3, 0xF2};
DeviceAddress address3 = {0x28, 0xD7, 0x47, 0x79, 0xA2, 0x0, 0x3, 0x87};

float EA = A0; 
float humedad; 
float calculo;
float porcentaje;

const char WEBSITE[] = "api.pushingbox.com"; //pushingbox API server
const String devid = "v00DB1FF6420FBAE"; // ID del dispositivo en Pushingbox
const char* MY_SSID = "IICAFABLAB"; // nombre del dispositivo de internet
const char* MY_PWD =  "1234567890"; // contraseña del dispositivo de internet

int status = WL_IDLE_STATUS;
// si no se quiere usar DNS
// es mejor usar el IP numerico en vez del nombre del servidor:
// IPAddress server(X,X,X,X);  // IP numerico de Google (no DNS)

void setup() {
  Serial.begin(9600);
  Serial.print("(SSID: ");
  sensors.begin();
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
}

void printWifiStatus() {

  Serial.print("(SSID: ");
  Serial.print(WiFi.SSID());
  Serial.print(" / ");

  IPAddress ip = WiFi.localIP();
  Serial.print("Dirección IP: ");
  Serial.print(ip);
  Serial.print(" / ");

  long rssi = WiFi.RSSI();
  Serial.print("Fuerza de la señal (RSSI): ");
  Serial.print(rssi);
  Serial.println("dBm)");
}

void loop() {
  // Establecer la conexion a internet:
  
  Serial.println("Conectandose al dispositivo: ");
  Serial.print(MY_SSID);
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  status = WiFi.begin(MY_SSID, MY_PWD);
  delay(5000);
  Serial.println("\nConectado!!!\n");
  printWifiStatus();

  Serial.println("\n"); 
  humedad = analogRead(EA);
  calculo = (humedad-505)*100L/(952-505);
  porcentaje = 100-calculo;
  sensors.requestTemperatures();
  float temp1= sensors.getTempC(address1);
  float temp2= sensors.getTempC(address2);
  float temp3= sensors.getTempC(address3);
  Serial.print("Temperatura Sensor 1: ");
  Serial.print(temp1);
  Serial.println(" °C");
  Serial.print("Temperatura Sensor 2: ");
  Serial.print(temp2);
  Serial.println(" °C");
  Serial.print("Temperatura Sensor 3: ");
  Serial.print(temp3);
  Serial.println(" °C");
  Serial.print("Humedad: ");
  if (porcentaje > 100) {
    Serial.println("100%");
  } else if (porcentaje < 0) {
    Serial.println("0%");
  } else if (porcentaje > 0 && porcentaje < 100) {
    Serial.print(porcentaje);
    Serial.println("%");
  }
  WiFiClient client;
    if (client.connect(WEBSITE, 80)){ 
         client.print("GET /pushingbox?devid=" + devid
       + "&TemperaturaS1="   + (String) temp1
       + "&TemperaturaS2="   + (String) temp2
       + "&TemperaturaS3="   + (String) temp3
       + "&Humedad="         + (String) porcentaje
         );
      client.println(" HTTP/1.1"); 
      client.print("Host: ");
      client.println(WEBSITE);
      client.println("User-Agent: MKR1000/1.0");
      client.println();
      Serial.println("Datos Enviados!!!\n");
      pinMode(5, OUTPUT);
      digitalWrite(5, HIGH);
      delay(5000);
      client.stop();
      digitalWrite(5, LOW);
      delay(890000);
      }
}
