#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <FastLED.h>
#include "SinricPro.h"
#include "SinricProLight.h"

#define BAUD_RATE 115200
#define WIFI_SSID "REDE_WIFI"    
#define WIFI_PASS "SENHA_WIFI"
#define APP_KEY "KEY_SINRICPRO"
#define APP_SECRET "SENHA_SINRICPRO"
#define BROKER_USER "USUARIO_BROKER_MQTT"
#define BROKER_PASS "SENHA_BROKER_MQTT"
#define BROKER_URL "ENDERECO_SERVER_MQTT"

#define LED_SETUP_ID "ID_DISPOSITIVO_SINRICPRO"
#define LED_LAMP_ID "ID_DISPOSITIVO_SINRICPRO"

WiFiClient espClient;
PubSubClient client(espClient);

bool powerState;        
int globalBrightness = 100;

char mensagem[16];
char mensagemTratada[16];
char sinalBuffer[16];
unsigned int lengthMsg = 0;
bool statusLeds = false;
bool ligaRainbow = false;
unsigned int velRainbow = 220;
unsigned int larguraRainbow = 10;
int r = 0;
int g = 0;
int b = 0;
#define NUM_LEDS 60
#define DATA_PIN 13

CRGB leds[NUM_LEDS];

bool ledSetupOnPower(const String &deviceId, bool &state) {
  powerState = state;
  if (state) {
    Serial.println("liga led_pc");
    client.publish("/led_pc/rainbow", "1");
  } else {
    Serial.println("desliga led_pc");
    client.publish("/led_pc/rainbow", "0");
  }
  return true; // request handled properly
}

bool ledLampOnPower(const String &deviceId, bool &state) {
  powerState = state;
  if (state) {
    Serial.println("liga led_lamp");
    client.publish("/led_lamp/rainbow", "1");
  } else {
    Serial.println("desliga led_lamp");
    client.publish("/led_lamp/rainbow", "0");
  }
  return true; // request handled properly
}

bool ledSetupOnBrightness(const String &deviceId, int &brightness) {
  globalBrightness = brightness;
  //FastLED.setBrightness(map(brightness, 0, 100, 0, 255));
  //FastLED.show();
  return true;
}

bool ledSetupOnAdjustBrightness(const String &deviceId, int brightnessDelta) {
  globalBrightness += brightnessDelta;
  brightnessDelta = globalBrightness;
  //FastLED.setBrightness(map(globalBrightness, 0, 100, 0, 255));
  //FastLED.show();
  return true;
}

bool ledSetupOnColor(const String &deviceId, byte &red, byte &green, byte &blue) {
  char hexCode[8];
  sprintf(hexCode,"#%02x%02x%02x",red,green,blue);
  Serial.println(hexCode);
  client.publish("/led_pc/corFixa", hexCode);
  return true;
}

bool ledLampOnColor(const String &deviceId, byte &red, byte &green, byte &blue) {
  char hexCode[8];
  sprintf(hexCode,"#%02x%02x%02x",red,green,blue);
  Serial.println(hexCode);
  client.publish("/led_lamp/corFixa", hexCode);
  return true;
}

void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  IPAddress localIP = WiFi.localIP();
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", localIP.toString().c_str());
}

void setupSinricPro() {
  SinricProLight &ledSetup = SinricPro[LED_SETUP_ID];
  ledSetup.onPowerState(ledSetupOnPower);
  ledSetup.onBrightness(ledSetupOnBrightness);
  ledSetup.onAdjustBrightness(ledSetupOnAdjustBrightness);
  ledSetup.onColor(ledSetupOnColor);

  SinricProLight &ledLamp = SinricPro[LED_LAMP_ID];
  ledLamp.onPowerState(ledLampOnPower);
  //ledLamp.onBrightness(onBrightness);
  //ledLamp.onAdjustBrightness(onAdjustBrightness);
  ledLamp.onColor(ledLampOnColor);

  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.restoreDeviceStates(true);
  SinricPro.begin(APP_KEY, APP_SECRET);
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
      Serial.print(r);
      Serial.print(g);
      Serial.print(b);
      Serial.print("\n");
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
    Serial.println(BROKER_URL);
    if (client.connect("/led_pc", BROKER_USER, BROKER_PASS)) {
      Serial.print("\nConnected to ");
      Serial.println(BROKER_URL);
      client.subscribe("/led_pc/corFixa");
      client.subscribe("/led_pc/velocidade");
      client.subscribe("/led_pc/largura");
      client.subscribe("/led_pc/rainbow");
    } else {
      Serial.println("\nTrying connect again");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(BAUD_RATE); Serial.printf("\r\n\r\n");
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setCorrection(0xFFA0FF);
  FastLED.setTemperature(0xFF9329);
  setupWiFi();
  setupSinricPro();
  client.setServer(BROKER_URL, 10978);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if (ligaRainbow) {
    fill_rainbow(leds, NUM_LEDS, (millis() * (255 - velRainbow) / 255), larguraRainbow);
    FastLED.setBrightness(255);
    FastLED.show();
  }
  client.loop();
  SinricPro.handle();
}
