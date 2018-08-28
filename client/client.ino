/*--------------------------------------------------------------------------
  --------------------------Configuracao SENSOR IR ------------------------*/
#include <IRremoteESP8266.h>
#include <IRsend.h>
IRsend irsend(14); //FUNÇÃO RESPONSÁVEL PELO MÉTODO DE ENVIO DO SINAL IR / UTILIZA O GPIO14(D5)
int tamanho = 68; //TAMANHO DA LINHA RAW(68 BLOCOS)
int frequencia = 32; //FREQUÊNCIA DO SINAL IR(32KHz)
// BOTÃO LIGA
uint16_t liga[] = {4600, 4150, 800, 1350, 800, 300, 750, 1400, 750, 1400, 750, 300, 800, 300, 750, 1400, 750, 350, 750, 300, 750, 1450, 700, 350, 700, 400, 700, 1450, 700, 1450, 700, 400, 650, 1500, 650, 400, 700, 400, 650, 450, 650, 1500, 650, 1500, 650, 1500, 650, 1500, 650, 1500, 650, 1550, 600, 1500, 650, 1550, 600, 450, 600, 500, 600, 450, 650, 450, 650, 450, 600, 450, 650, 450, 600, 1550, 600, 500, 600, 450, 650, 1500, 600, 500, 600, 500, 600, 1550, 600, 1550, 600, 500, 600, 1500, 650, 1550, 600, 450, 600, 1550, 600, 1550, 600}; //COLE A LINHA RAW CORRESPONDENTE DENTRO DAS CHAVES
// BOTÃO DESLIGA
uint16_t desliga[] = {4500, 4200, 650, 1450, 700, 400, 650, 1450, 650, 1500, 650, 400, 650, 450, 650, 1500, 600, 450, 650, 450, 600, 1500, 650, 450, 600, 500, 600, 1500, 600, 1550, 600, 450, 600, 1550, 600, 450, 600, 1550, 600, 1550, 600, 1550, 600, 1550, 600, 450, 600, 1550, 600, 1500, 600, 1550, 600, 450, 600, 500, 600, 450, 600, 500, 550, 1550, 600, 500, 550, 500, 600, 1550, 600, 1500, 600, 1550, 600, 500, 550, 500, 600, 450, 600, 500, 600, 450, 600, 500, 600, 450, 600, 450, 600, 1550, 600, 1550, 600, 1550, 550, 1600, 550, 1550, 550}; //COLE A LINHA RAW CORRESPONDENTE DENTRO DAS CHAVES
/*--------------------------------------------------------------------------
  --------------------------Configuracao Wi-FI ------------------------*/
#include <ESP8266WiFi.h>
const char* ssid = "WifiCasa";
const char* password =  "84432320";
/*--------------------------------------------------------------------------
  --------------------------Configuracao ServerMQTT ------------------------*/
#include <PubSubClient.h>
const char* mqttServer = "m14.cloudmqtt.com";
const int mqttPort = 15015;
const char* mqttUser = "nyggmttv";
const char* mqttPassword = "hMmfyHEX9rIr";
WiFiClient espClient;
PubSubClient client(espClient);
/*--------------------------------------------------------------------------
  --------------------------------------------------------------------------*/
void initWifi(){
   WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
}

void initMQTT(){
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("Micros2", mqttUser, mqttPassword )) {
      Serial.println("connected");  
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);    
    }
  }
}
void setup() {
 
  Serial.begin(115200);
  initWifi();
  initMQTT();
  irsend.begin(); //INICIALIZA A FUNÇÃO

 client.subscribe("TemperaturaAtual");
  
}
 
void callback(char* topic, byte* payload, unsigned int length) {

  String message;
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    message += c;
  }
  Serial.println(String(topic) + "=" + String(message));
   
 if (message == "4") {
  
//    digitalWrite(D5, 1);

Serial.println("Deu certo");
 }
 //else {  
//    digitalWrite(D5, 0);
//  }
  Serial.flush();
}
 
void loop() {
  client.loop();
  char c = Serial.read(); //VARIÁVEL RESPONSÁVEL POR RECEBER O CARACTER DIGITADO NA JANELA SERIAL
    
    if (c == 'l'){ //SE CARACTER DIGITADO FOR IGUAL A "a", FAZ
        irsend.sendRaw(liga,tamanho,frequencia);  // PARÂMETROS NECESSÁRIOS PARA ENVIO DO SINAL IR
        Serial.println("Comando enviado: Liga");
        delay(50); // TEMPO(EM MILISEGUNDOS) DE INTERVALO ENTRE UM COMANDO E OUTRO
    } 
    if (c == 'd'){ //SE CARACTER DIGITADO FOR IGUAL A "a", FAZ
        irsend.sendRaw(desliga,tamanho,frequencia);  // PARÂMETROS NECESSÁRIOS PARA ENVIO DO SINAL IR
        Serial.println("Comando enviado: Desliga");
        delay(50); // TEMPO(EM MILISEGUNDOS) DE INTERVALO ENTRE UM COMANDO E OUTRO
    }
  
}
