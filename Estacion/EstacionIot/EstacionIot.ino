// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID           "TMPLIbSFxpmn"
#define BLYNK_DEVICE_NAME           "Quickstart Template"
#define BLYNK_AUTH_TOKEN            "UMQbO19gF4rwQirGz8genlL2u3zItMIv"


// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <WiFiClientSecure.h>
#include <DHT.h>

// Variables para los pines de entrada/salida
const byte HYTS = 4; 
const byte WSPD = 5;
//const byte WDIR = GPIO15;
DHT DHTsensor(HYTS, DHT22);

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "FABLAB";
char pass[] = "#$IICA2019$#";
const char* myDomain = "https://script.google.com";
String script = "/macros/s/AKfycbycl-dvb9SMjsGlKuCM0XqxUj18sFplkmIBezRqYA-MP3WYCfh7GYwOEDMmXD4736Tkjw/exec";

long ultimoReporte=0;
long periodoReportes = 5 * 1000; 

//int randomNumber;
//String windD;
//float wind;
float hum;
float temp;
long distancia;
//float windSpeed;
long deltaTime =1;
//String dir[8] = {"N","NE","E","SE","S","SO","O","NO"};

// Variables auxiliares para la medición del viento
//volatile long lastWindIRQ = 0;
//volatile byte windClicks = 0;
//long lastWindCheck = 0;

// This function is called every time the Virtual Pin 0 state changes
//BLYNK_WRITE(V0)
//{
//  // Set incoming value from pin V0 to a variable
//  int value = param.asInt();
//
//  // Update state
//  Blynk.virtualWrite(V1, value);
//}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

void setup()
{
  // Debug console
  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass);
  
  DHTsensor.begin();
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
  //pinMode(WSPD, INPUT_PULLUP);
  pinMode(HYTS, INPUT);
  ultimoReporte = millis();
  // attach external interrupt pins to IRQ functions
  attachInterrupt(digitalPinToInterrupt(WSPD), wspeedIRQ, FALLING);

  // turn on interrupts
  interrupts();
  analogReadResolution(10);
}

void loop()
{
  Blynk.run();
  
  wind = get_wind_speed();
  windD  = get_wind_direction();
  if ( isnan(DHTsensor.readHumidity()) != 1 )
        hum = DHTsensor.readHumidity();  
  if ( isnan(DHTsensor.readTemperature()) != 1 )
        temp = DHTsensor.readTemperature(); 
//  randomNumber = random(200,600);
//  hum = float(randomNumber)/10;
//  randomNumber = random(200,350);
//  temp = float(randomNumber)/10;
  if ((millis() - ultimoReporte) > periodoReportes) {
    
    ultimoReporte = millis();
    int valor = wind;
    Serial.println(valor);
    enviarDatosCloud(valor);
  }
  
  Blynk.virtualWrite(V2, wind);
  Blynk.virtualWrite(V0, windD);
  Blynk.virtualWrite(V1, hum);
  Blynk.virtualWrite(V4, temp);
  //Escribimos el numero aleatorio por el puerto serie
//  Serial.print("El numero aleatorio es = ");
//  Serial.println(wind);
 
  //Esperamos 1 segundo para repetir
  delay(1000);
  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
}

float get_wind_speed() {
          
  if (millis() - lastWindIRQ > 3000){//velocidad mínima del viento de 0.8 km/h
    windSpeed = 0;
  }else{
    windSpeed = 1 / ((float)deltaTime);// Calculamos la cantidad de clicks por ms
    windSpeed *= 2401.14; // Conversión click por ms a km/h
  }
  //Serial.println(windSpeed);
  return (windSpeed);
}

void wspeedIRQ() {

  // Ignoramos los errores del interruptor, por rebotes menores a 10 ms (velocidad máxima del viento de 228 km/h)
  if (millis() - lastWindIRQ > 10) {    
    lastWindIRQ = millis(); //Grab the current time
    deltaTime = millis() - lastWindCheck;
    lastWindCheck = millis();
    //Serial.print("click");
  }
}

String get_wind_direction() {

  int adc;

  adc = analogRead(35); // get the current reading from the sensor

  // Los valores de la veleta se muestran comparan en orden ascendente
  // Cada límite se definió 5 unidades de resistencia por sobre el valor del sensor
  //Serial.println(adc);
  Serial.println(adc); 
  if (adc < (271+10) ) return ("E");
  if (adc < (383+10) ) return ("SE");
  if (adc < (493+10) ) return ("S");
  if (adc < (650+10) ) return ("NE");
  if (adc < (778+10) ) return ("SO");
  if (adc < (912+10) ) return ("N");
  if (adc < (1005+10) ) return ("NO");
  if (adc < (1023+10) ) return ("O");
  return ("ERROR"); // error, disconnected?
}

void enviarDatosCloud(int valor) {
  WiFiClientSecure clientSecure;
  if (clientSecure.connect(myDomain, 443)) {
    Serial.println("Connection successful");  

    String Data = "data="+String(valor);
    clientSecure.println("POST " + script + " HTTP/1.1");
    clientSecure.println("Host: " + String(myDomain));
    clientSecure.println("Content-Length: " + String(Data.length()));
    clientSecure.println("Content-Type: application/x-www-form-urlencoded");
    clientSecure.println();
    
    clientSecure.print(Data);    
    Serial.println("Waiting for response.");
    long int StartTime=millis();
    while (!clientSecure.available()) {
      Serial.print(".");
      delay(100);
      if ((StartTime+60000) < millis()) {
        Serial.println();
        Serial.println("No response.");
        break;
      }
    }
    Serial.println(); 
    
    while (clientSecure.available()) {
      Serial.print(char(clientSecure.read()));
    }  
    
  } else {
    Serial.println("Connected to " + String(myDomain) + " failed.");
  }
  clientSecure.stop();
}
