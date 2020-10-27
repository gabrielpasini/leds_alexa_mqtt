#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "REDE_WIFI";
const char* pass = "SENHA";
const char* brokerUser = "USUARIO_MQTT_SERVER";
const char* brokerPass = "SENHA_MQTT_SERVER";
const char* broker = "ENDERECO_SERVER_EX:postman.cloudmqtt.com";

WiFiClient espClient;
PubSubClient client(espClient);
char mensagem[16];
char mensagemTratada[16];
char sinalBuffer[16];
unsigned int lengthMsg = 0;
unsigned int velRainbow = 5;
bool ligaRainbow = false;
int red = 0;
int green = 0;
int blue = 0;
uint8_t redPin = 12;
uint8_t greenPin = 13;
uint8_t bluePin = 14; 
const int freq = 5000;
const int resolution = 8;

void setupWifi() {
  delay(100);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print("-");
  }
  Serial.print("\nConnected to ");
  Serial.println(ssid);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String topico = topic;
  lengthMsg = length;
  for (int i = 0; i < lengthMsg; i++) {
    mensagem[i] = (char)payload[i];
  }
  Serial.print(topic);
  Serial.print(" ");
  Serial.print(mensagem);
  Serial.println();
  if (topico == "/led_lamp/rainbow") {
    if (mensagem[0] == '1') {
      ligaRainbow = true;
    } else if (mensagem[0] == '0') {
      ligaRainbow = false;
      ledcWrite(1, 0);
      ledcWrite(2, 0);
      ledcWrite(3, 0);
    }
    client.publish("/led_lamp/corFixa", "#000000");
  }
  if (topico == "/led_lamp/corFixa") {
    for (int i = 0; i < lengthMsg; i++) {
      mensagemTratada[i] = mensagem[i + 1];
    }
    String hexstring = mensagemTratada;
    if (hexstring != "000000") {
      ligaRainbow = false;
      int number = (int) strtol( &hexstring[1], NULL, 16);
      red = number >> 16;
      green = number >> 8 & 0xFF;
      blue = number & 0xFF;
      ledcWrite(1, red);
      ledcWrite(2, green);
      ledcWrite(3, blue);
    }
  }
  if (topico == "/led_lamp/velocidade") {
    velRainbow = (int)mensagem[0] - 48;
  }

  //limpa mensagem
  for (int i = 0; i < lengthMsg; i++) {
    mensagem[i] = '\0';
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("\nConnecting to ");
    Serial.println(broker);
    if (client.connect("/led_lamp", brokerUser, brokerPass)) {
      Serial.print("\nConnected to ");
      Serial.println(broker);
      client.subscribe("/led_lamp/corFixa");
      client.subscribe("/led_lamp/velocidade");
      client.subscribe("/led_lamp/rainbow");
    } else {
      Serial.println("\nTrying connect again");
      delay(5000);
    }
  }
}

void setup() {
  ledcSetup(1, freq, resolution);
  ledcSetup(2, freq, resolution);
  ledcSetup(3, freq, resolution);
  ledcAttachPin(redPin, 1);
  ledcAttachPin(greenPin, 2);
  ledcAttachPin(bluePin, 3);
  Serial.begin(115200);
  setupWifi();
  client.setServer(broker, 10978);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if (ligaRainbow) {
    int r, g, b;
    ledcWrite(3, 255);
    for(r = 0; r <= 255; r++){
      ledcWrite(1, r);
      delay(velRainbow);
    }
    for(b = 255; b >= 0; b--){
      ledcWrite(3, b);
      delay(velRainbow);
    }
    for(g = 0; g <= 255; g++){
      ledcWrite(2, g);
      delay(velRainbow);
    }
    for(r = 255; r >= 0; r--){
      ledcWrite(1, r);
      delay(velRainbow);
    }
    for(b = 0; b <= 255; b++){
      ledcWrite(3, b);
      delay(velRainbow);
    }
    for(g = 255; g >= 0; g--){
      ledcWrite(2, g);
      delay(velRainbow);
    }
  }
  client.loop();
}
