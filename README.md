## LEDS controlados pela Alexa ou pela internet via MQTT utilizando a placa `ESP32`

GPIO da placa: [ESP32 GPIO](https://cdn.discordapp.com/attachments/663479366918995985/770466205538189332/ESP32-Pinout.png)

Faça download da [IDE do Arduino](https://www.arduino.cc/en/Main/Software#)

Cadastre os dispositivos no [Sinric PRO](https://portal.sinric.pro/dashboard)

### `led_pc.ino`

###RESPONSÁVEL PELAS FUNÇÕES DA ALEXA

Fita utilizada:
- Led WS2812 5V (Endereçada)

Efeitos disponíveis:
- Rainbow Chroma RGB;
- Escolha de velocidade;
- Escolha de largura;
- Cores HEX fixas;

### `led_lamp.ino`
Fita utilizada:
- Led 5050 12V (Não endereçada)

Efeitos disponíveis:
- Rainbow RGB;
- Escolha de velocidade;
- Cores HEX fixas;
