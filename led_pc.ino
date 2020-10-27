#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <FastLED.h>

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
bool statusLeds = false;
bool ligaRainbow = false;
bool ligaRitmo = false;
unsigned int velRainbow = 220;
unsigned int larguraRainbow = 10;
unsigned int sensi = 500;
int sinal = 0;
int loopRitmo = 0;
int r = 0;
int g = 0;
int b = 0;
#define NUM_LEDS 120
#define DATA_PIN 13
#define READ_PIN 34

CRGB leds[NUM_LEDS];

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

void desligaLeds() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
    FastLED.show();
    delay(10);
  }
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
  if (topico == "/led_pc/rainbow") {
    if (mensagem[0] == '1') {
      ligaRainbow = true;
    } else if (mensagem[0] == '0') {
      ligaRainbow = false;
      desligaLeds();
    }
    client.publish("/led_pc/corFixa", "#000000");
  }
  if (topico == "/led_pc/ritmo") {
    loopRitmo = 0;
    if (mensagem[0] == '1') {
      ligaRitmo = true;
    } else if (mensagem[0] == '0') {
      ligaRitmo = false;
    }
  }
  if (topico == "/led_pc/sensi") {
    int milhar = 0;
    int centena = 0;
    int dezena = 0;
    int unidade = 0;
    if (lengthMsg == 4) {
      milhar = ((int)mensagem[0] - 48) * 1000;
      centena = ((int)mensagem[1] - 48) * 100;
      dezena = ((int)mensagem[2] - 48) * 10;
      unidade = (int)mensagem[3] - 48;
    } else if (lengthMsg == 3) {
      milhar = 0;
      centena = ((int)mensagem[0] - 48) * 100;
      dezena = ((int)mensagem[1] - 48) * 10;
      unidade = (int)mensagem[2] - 48;
    } else if (lengthMsg == 2) {
      milhar = 0;
      centena = 0;
      dezena = ((int)mensagem[0] - 48) * 10;
      unidade = (int)mensagem[1] - 48;
    } else if (lengthMsg == 1) {
      milhar = 0;
      centena = 0;
      dezena = 0;
      unidade = (int)mensagem[0] - 48;
    }
    sensi = milhar + centena + dezena + unidade;
  }
  if (topico == "/led_pc/corFixa") {
    for (int i = 0; i < lengthMsg; i++) {
      mensagemTratada[i] = mensagem[i + 1];
    }
    String hexstring = mensagemTratada;
    if (hexstring != "000000") {
      ligaRainbow = false;
      int number = (int) strtol( &hexstring[1], NULL, 16);
      r = number >> 16;
      g = number >> 8 & 0xFF;
      b = number & 0xFF;
      CRGB rgbval(r, g, b);
      fill_solid(leds, NUM_LEDS, rgbval);
      FastLED.setBrightness(255);
      FastLED.show();
    }
  }
  if (topico == "/led_pc/velocidade") {
    int centena = 0;
    int dezena = 0;
    int unidade = 0;
    if (lengthMsg == 3) {
      centena = ((int)mensagem[0] - 48) * 100;
      dezena = ((int)mensagem[1] - 48) * 10;
      unidade = (int)mensagem[2] - 48;
    } else if (lengthMsg == 2) {
      centena = 0;
      dezena = ((int)mensagem[0] - 48) * 10;
      unidade = (int)mensagem[1] - 48;
    } else if (lengthMsg == 1) {
      centena = 0;
      dezena = 0;
      unidade = (int)mensagem[0] - 48;
    }
    velRainbow = centena + dezena + unidade;
  }
  if (topico == "/led_pc/largura") {
    int dezena = 0;
    int unidade = 0;
    if (lengthMsg == 2) {
      dezena = ((int)mensagem[0] - 48) * 10;
      unidade = (int)mensagem[1] - 48;
    } else {
      dezena = 0;
      unidade = (int)mensagem[0] - 48;
    }
    larguraRainbow = dezena + unidade;
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
    if (client.connect("/led_pc", brokerUser, brokerPass)) {
      Serial.print("\nConnected to ");
      Serial.println(broker);
      client.subscribe("/led_pc/corFixa");
      client.subscribe("/led_pc/velocidade");
      client.subscribe("/led_pc/largura");
      client.subscribe("/led_pc/rainbow");
      client.subscribe("/led_pc/ritmo");
      client.subscribe("/led_pc/sensi");
    } else {
      Serial.println("\nTrying connect again");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(READ_PIN, INPUT);
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setCorrection(0xFFA0FF);
  FastLED.setTemperature(0xFF9329);
  Serial.begin(115200);
  setupWifi();
  client.setServer(broker, 10978);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if (ligaRitmo) {
    sinal = analogRead(READ_PIN);
    if (loopRitmo == 1000) {
      loopRitmo = 0;
      sprintf(sinalBuffer,"%d",sinal);
      char* sinalStr = sinalBuffer;
      client.publish("/led_pc/monitor", sinalStr);
    }
    int sensiMedia = sensi - sensi / 4;
    int sensiBaixa = sensi / 2;
    if (sinal >= sensi) {
      FastLED.setBrightness(255);
    } else if ((sinal >= sensiMedia) && (sinal < sensi)) {
      FastLED.setBrightness(32);
    } else if ((sinal >= sensiBaixa) && (sinal < sensiMedia)) {
      FastLED.setBrightness(8);
    } else {
      FastLED.setBrightness(0);
    }
    if (ligaRainbow) {
      fill_rainbow(leds, NUM_LEDS, (millis() * (255 - velRainbow) / 255), larguraRainbow);
      FastLED.show();
    } else {
      CRGB rgbval(r, g, b);
      fill_solid(leds, NUM_LEDS, rgbval);
      FastLED.show();
    }
    loopRitmo++;
  }
  if (ligaRainbow && !ligaRitmo) {
    fill_rainbow(leds, NUM_LEDS, (millis() * (255 - velRainbow) / 255), larguraRainbow);
    FastLED.setBrightness(255);
    FastLED.show();
  }
  client.loop();
}
