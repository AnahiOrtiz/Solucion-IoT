#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <DHT.h>
#include <Stepper.h>
#include <DHT_U.h>      
#include <ESP32Servo.h> 

WiFiClient esp32Client;
PubSubClient mqttClient(esp32Client);

const char* ssid = "WiCAT_OrtizCruz";
const char* password = "16021239";

char *server = "200.79.179.167";
int port = 1884;

//DHT11
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//Fotoresistencia
const int potPin = 32;
const int ledPin = 26;
int var = 0;
const int stepsPerRevolution = 2048; 
int fotoval = 0;
char datos[40];
String resultS = "";
int temperatura = 0;
int humedad = 0;
int potValue = 0;
String cadena = "";
int PinServo = 15;

#define IN1 5 
#define IN2 18
#define IN3 19
#define IN4 21
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

//servomotor
Servo myservo;

void setup() {
  Serial.begin(115200);
  delay(10);
  dht.begin();
  pinMode(potPin, INPUT);
  pinMode(26,OUTPUT);
  pinMode(IN1,OUTPUT);               
  pinMode(IN2,OUTPUT);               
  pinMode(IN3,OUTPUT);               
  pinMode(IN4,OUTPUT);               
  myStepper.setSpeed(10);
  myservo.attach(PinServo);
  myservo.write(0);
  WiFiInit();
  mqttClient.setServer(server, port);
  mqttClient.setCallback(callback);
}

void loop() {
  if(!mqttClient.connected()){
    reconnect();
  }
  mqttClient.loop();

  Temperatura();
  delay(1000);
  Humedad();
  delay(1000);
  Luminosidad();
  delay(1000);
 
  mqttClient.publish("AppMovil", datos);
  delay(5000);
}

void WiFiInit(){
  Serial.println("Conectandose a: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("Conectado a WiFi");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length){
  Serial.println("Mensaje recibido en el topico: ");
  Serial.println(topic);
  Serial.println("Mensaje; ");
  String messageTemp;

  for(int i = 0; i < length; i++){
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  if(messageTemp == "s90"){
    myservo.write(90);
  }
  if(messageTemp == "s0"){
    myservo.write(0);
  }
  if(messageTemp == "L1"){
    digitalWrite(ledPin, HIGH);
  }
  if(messageTemp == "L0"){
    digitalWrite(ledPin, LOW);
  }
  if(messageTemp == "gira"){
    myStepper.step(stepsPerRevolution); 
  }
}

void reconnect(){
  while(!mqttClient.connected()){
    Serial.print("Intentando conectarse a MQTT...");
    if(mqttClient.connect("")){
      Serial.println("Conectando");
      mqttClient.subscribe("Tarjeta");
    }else{
      Serial.print("Fallo, rc = ");
      Serial.print(mqttClient.state());
      Serial.println("Intentar de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

void Temperatura(){
  temperatura = dht.readTemperature();

  sprintf(datos, "Temperatura (°C): %d ", temperatura);
  mqttClient.publish("AppMovil", datos);
  Serial.print("Temperatura: ");
  Serial.println(temperatura);
 }

void Humedad(){
  humedad = dht.readHumidity();
  sprintf(datos, "Humedad: %d ", humedad);
  mqttClient.publish("AppMovil",datos);
  Serial.print("Humedad: ");
  Serial.println(humedad);
 }

 void Luminosidad(){
  potValue = analogRead(potPin);
  sprintf(datos, "Luminosidad: %d ", potValue);
  mqttClient.publish("AppMovil",datos);
  Serial.print("Luminosidad: ");
  Serial.println(potValue);
 }
