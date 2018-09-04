/*--------------------------------------------------------------------------
  --------------------------VariÃ¡veis globais ajustaveis: ------------------------*/
char intervaloTempo[10] = "10"; // em milesegundos, para atualizar a temperatura lida
char ipAddr[16] = "172.024.001.001";//Pi Access Point IP-Adr.
/*--------------------------------------------------------------------------
  --------------------------Configuracao Wi-FI ------------------------*/
#include <ESP8266WiFi.h>
/*--------------------------------------------------------------------------
  --------------------------Configuracao EEPROM ------------------------*/
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <EEPROM.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
//flag for saving data
bool shouldSaveConfig = false;
/*--------------------------------------------------------------------------
  --------------------------Configuracao modo ------------------------*/
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
byte modo; // false (0) - normal true (1) configuraÃ§ao
#define botao D2
#define led D1
/*--------------------------------------------------------------------------/*--------------------------------------------------------------------------
  --------------------------Configuracao SENSOR IR ------------------------*/
#include <IRremoteESP8266.h>
#include <IRsend.h>
IRsend irsend(14); //FUNÇÃO RESPONSÁVEL PELO MÉTODO DE ENVIO DO SINAL IR / UTILIZA O GPIO14(D5)
int tamanho = 259; //TAMANHO DA LINHA RAW(68 BLOCOS)
int frequencia = 32; //FREQUÊNCIA DO SINAL IR(32KHz)
// BOTÃO LIGA
uint16_t liga[] = {4600, 4150, 800, 1350, 800, 300, 750, 1400, 750, 1400, 750, 300, 800, 300, 750, 1400, 750, 350, 750, 300, 750, 1450, 700, 350, 700, 400, 700, 1450, 700, 1450, 700, 400, 650, 1500, 650, 400, 700, 400, 650, 450, 650, 1500, 650, 1500, 650, 1500, 650, 1500, 650, 1500, 650, 1550, 600, 1500, 650, 1550, 600, 450, 600, 500, 600, 450, 650, 450, 650, 450, 600, 450, 650, 450, 600, 1550, 600, 500, 600, 450, 650, 1500, 600, 500, 600, 500, 600, 1550, 600, 1550, 600, 500, 600, 1500, 650, 1550, 600, 450, 600, 1550, 600, 1550, 600}; // BOTÃO DESLIGA
uint16_t desliga[] = {4500, 4200, 650, 1450, 700, 400, 650, 1450, 650, 1500, 650, 400, 650, 450, 650, 1500, 600, 450, 650, 450, 600, 1500, 650, 450, 600, 500, 600, 1500, 600, 1550, 600, 450, 600, 1550, 600, 450, 600, 1550, 600, 1550, 600, 1550, 600, 1550, 600, 450, 600, 1550, 600, 1500, 600, 1550, 600, 450, 600, 500, 600, 450, 600, 500, 550, 1550, 600, 500, 550, 500, 600, 1550, 600, 1500, 600, 1550, 600, 500, 550, 500, 600, 450, 600, 500, 600, 450, 600, 500, 600, 450, 600, 450, 600, 1550, 600, 1550, 600, 1550, 550, 1600, 550, 1550, 550}; //COLE A LINHA RAW CORRESPONDENTE DENTRO DAS CHAVES
/*--------------------------------------------------------------------------
  /*--------------------------------------------------------------------------
  --------------------------Configuracao ServerMQTT ------------------------*/
#include <PubSubClient.h>
char mqttServer[40] = "m14.cloudmqtt.com";
char mqttPort[10] = "15015";
char mqttUser[40] = "nyggmttv";
char mqttPassword[40] = "hMmfyHEX9rIr";
char mqttTopic[40] = "TemperaturaAtual";
char mqttName[40] = "Micros1";
WiFiClient espClient;
PubSubClient client(espClient);
WiFiManagerParameter custom_mqtt_server("server", "mqttServer", mqttServer, 40);
WiFiManagerParameter custom_mqtt_port("port", "mqttPort", mqttPort, 10);
WiFiManagerParameter custom_mqtt_user("user", "mqttUser", mqttUser, 40);
WiFiManagerParameter custom_mqtt_password("password", "mqttPassword", mqttPassword, 40);
WiFiManagerParameter custom_mqtt_topic("topic", "mqttTopic", mqttTopic, 40);
WiFiManagerParameter custom_mqtt_name("name", "mqttName", mqttName, 40);
WiFiManagerParameter custom_intervalo_tempo("intervaloTempo", "intervaloTempo", intervaloTempo, 10);
/*--------------------------------------------------------------------------
  --------------------------------------------------------------------------*/
void initMQTT() {
  int port = parseInt(mqttPort);
  client.setServer(mqttServer, 15015);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect(mqttName, mqttUser, mqttPassword )) {

      Serial.println("connected");

    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);

    }
  }
}

void piscaLed() {
  for (int i = 0; i < 15; i++) {
    digitalWrite(led, HIGH);
    delay(150);
    digitalWrite(led, LOW);
    delay(150);
  }
}
void lerJson() {
  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(mqttServer, json["mqttServer"]);
          strcpy(mqttPort, json["mqttPort"]);
          strcpy(mqttUser, json["mqttUser"]);
          strcpy(mqttPassword, json["mqttPassword"]);
          strcpy(mqttTopic, json["mqttTopic"]);
          strcpy(mqttName, json["mqttName"]);
          strcpy(intervaloTempo, json["intervaloTempo"]);

        } else {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
}

void lerParametros() {
  strcpy(mqttServer, custom_mqtt_server.getValue());
  strcpy(mqttPort, custom_mqtt_port.getValue());
  strcpy(mqttUser, custom_mqtt_user.getValue());
  strcpy(mqttPassword, custom_mqtt_password.getValue());
  strcpy(mqttTopic, custom_mqtt_topic.getValue());
  strcpy(mqttName, custom_mqtt_name.getValue());
  strcpy(intervaloTempo, custom_intervalo_tempo.getValue());
}

void createFile(void) {
  //Abre o sistema de arquivos
  if (!SPIFFS.begin()) {
    Serial.println("Erro ao abrir o sistema de arquivos");
  } else {
    Serial.println("Sistema de arquivos aberto com sucesso!");
  }
  File wFile;

  //Cria o arquivo se ele não existir
  if (SPIFFS.exists("/config.json")) {
    Serial.println("Arquivo ja existe!");
  } else {
    Serial.println("Criando o arquivo...");
    wFile = SPIFFS.open("/config.json", "w+");
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();

    json["mqttServer"] = mqttServer;
    json["mqttPort"] = mqttPort;
    json["mqttUser"] = mqttUser;
    json["mqttPassword"] = mqttPassword;
    json["mqttTopic"] = mqttTopic;
    json["mqttName"] = mqttName;
    json["intervaloTempo"] = intervaloTempo;

    json.printTo(Serial);
    json.printTo(wFile);
    //Verifica a criação do arquivo
    if (!wFile) {
      Serial.println("Erro ao criar arquivo!");
    } else {
      Serial.println("Arquivo criado com sucesso!");
    }
  }
  wFile.close();
}

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void gravarJson() {
  if (SPIFFS.begin()) {
    //save the custom parameters to FS
    if (shouldSaveConfig)    {
      Serial.println("saving config");
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.createObject();

      json["mqttServer"] = mqttServer;
      json["mqttPort"] = mqttPort;
      json["mqttUser"] = mqttUser;
      json["mqttPassword"] = mqttPassword;
      json["mqttTopic"] = mqttTopic;
      json["mqttName"] = mqttName;
      json["intervaloTempo"] = intervaloTempo;

      File configFile = SPIFFS.open("/config.json", "w");
      if (!configFile) {
        Serial.println("failed to open config file for writing");
      }

      json.printTo(Serial);
      json.printTo(configFile);
      configFile.close();
      //end save
    }
  }
}
void rotinaModo() {
  // faz a leitura do pino D2 (no nosso caso, o botão está ligado nesse pino)
  modo = digitalRead(botao);
  // checa se o botão está pressionado
  if (modo == HIGH) {
    piscaLed();
    WiFiManager wifiManager;
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    //add all your parameters here
    WiFiManagerParameter text_config("<p><b>Configurations MQTT</b></p>");
    wifiManager.addParameter(&text_config);
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_user);
    wifiManager.addParameter(&custom_mqtt_password);
    wifiManager.addParameter(&custom_mqtt_topic);
    wifiManager.addParameter(&custom_mqtt_name);
    WiFiManagerParameter text_tempo("<p><b>Intervalo de tempo verificar temperatura</b></p>");
    wifiManager.addParameter(&text_tempo);
    wifiManager.addParameter(&custom_intervalo_tempo);
    wifiManager.startConfigPortal ( "MicrosWifiAP", "janela129"  );

    lerParametros();
    gravarJson();

  }
  else {
    WiFiManager wifiManager;
    wifiManager.autoConnect("MicrosWifiAP", "janela129");
    //SPIFFS.format();
    createFile();
    lerJson();
  }
}


int parseInt(char* chars)
{
  int sum = 0;
  int len = strlen(chars);
  for (int x = 0; x < len; x++)
  {
    int n = chars[len - (x + 1)] - '0';
    sum = sum + pow(n, x);
  }
  return sum;
}

int powInt(int x, int y)
{
  for (int i = 0; i < y; i++)
  {
    x *= 10;
  }
  return x;
}

void setup() {
  Serial.begin(115200);
  SPIFFS.begin();
  lerJson();
  pinMode(led, OUTPUT);
  pinMode(botao, INPUT);
  rotinaModo();
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
  boolean arLigado = false;
  if (message.toInt() >= 24 && arLigado == false ) {
    irsend.sendRaw(liga, tamanho, frequencia); // PARÂMETROS NECESSÁRIOS PARA ENVIO DO SINAL IR
    Serial.println("Comando enviado: Liga");
    delay(50); // TEMPO(EM MILISEGUNDOS) DE INTERVALO ENTRE UM COMANDO E OUTRO;
    arLigado == true;
  }
  else if (message.toInt() <= 17 && arLigado == true) {
    irsend.sendRaw(desliga, tamanho, frequencia); // PARÂMETROS NECESSÁRIOS PARA ENVIO DO SINAL IR
    Serial.println("Comando enviado: Desliga");
    delay(50); // TEMPO(EM MILISEGUNDOS) DE INTERVALO ENTRE UM COMANDO E OUTRO;
    arLigado == false;
  }
  Serial.flush();
}

void loop() {
  client.loop();
  char c = Serial.read(); //VARIÁVEL RESPONSÁVEL POR RECEBER O CARACTER DIGITADO NA JANELA SERIAL

  if (c == 'l') { //SE CARACTER DIGITADO FOR IGUAL A "a", FAZ
    irsend.sendRaw(liga, tamanho, frequencia); // PARÂMETROS NECESSÁRIOS PARA ENVIO DO SINAL IR
    Serial.println("Comando enviado: Liga");
    delay(50); // TEMPO(EM MILISEGUNDOS) DE INTERVALO ENTRE UM COMANDO E OUTRO
  }
  if (c == 'd') { //SE CARACTER DIGITADO FOR IGUAL A "a", FAZ
    irsend.sendRaw(desliga, tamanho, frequencia); // PARÂMETROS NECESSÁRIOS PARA ENVIO DO SINAL IR
    Serial.println("Comando enviado: Desliga");
    delay(50); // TEMPO(EM MILISEGUNDOS) DE INTERVALO ENTRE UM COMANDO E OUTRO
  }

}
