/*--------------------------------------------------------------------------
  --------------------------Toque quando entra no modo configuração: ------------------------*/
char* musica[] = {"La", "Re", "Fa", "Sol", "La", "Re", "Fa", "Sol", "Mi", "Pausa", "Sol", "Do", "Fa", "Mi", "Sol", "Do", "Fa", "Mi", "Re", "Fim"}; //Game of Thrones
int duracao[] = {700, 500, 300, 250, 250, 300, 200, 200, 700, 200, 500, 500, 200, 200, 200, 500, 200, 200, 500};
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
/*--------------------------------------------------------------------------
  --------------------------Configuracao modo ------------------------*/
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
byte modo; // false (0) - normal true (1) configuraÃ§ao
#define botao D2
#define buzzer D1
/*--------------------------------------------------------------------------
  --------------------------Configuracao NTP Server ------------------------*/
#include <NTPClient.h>//Biblioteca do NTP.
#include <WiFiUdp.h>//Biblioteca do UDP.
WiFiUDP udp;//Cria um objeto "UDP".
NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000);//Cria um objeto "NTP" com as configuraÃ§Ãµes.
String hora;//VÃ¡riavel que armazenara o horario do NTP.
/*--------------------------------------------------------------------------
  --------------------------Configuracao Sensor DS18B20 ------------------------*/
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS D4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
char temperaturaString[6];
float temperatura;
int controleTemperatura = 0;
int diferencaTemperatura = 3;
/*--------------------------------------------------------------------------
  --------------------------Configuracao ServerMQTT ------------------------*/
#include <PubSubClient.h>
char mqttServer[40] = "m14.cloudmqtt.com";
char mqttPort[10] = "15015";
char mqttUser[40] = "nyggmttv";
char mqttPassword[40] = "hMmfyHEX9rIr";
char mqttTopic[40] = "TemperaturaAtual";
char mqttName[40] = "Micros";
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

void writeEEPROM(int startAdr, int laenge, char* writeString) {
  EEPROM.begin(512); //Max bytes of eeprom to use
  yield();
  Serial.println();
  Serial.print("writing EEPROM: ");
  //write to eeprom
  for (int i = 0; i < laenge; i++)
  {
    EEPROM.write(startAdr + i, writeString[i]);
    Serial.print(writeString[i]);
  }
  EEPROM.commit();
  EEPROM.end();
}

void readEEPROM(int startAdr, int maxLength, char* dest) {
  EEPROM.begin(512);
  delay(10);
  for (int i = 0; i < maxLength; i++)
  {
    dest[i] = char(EEPROM.read(startAdr + i));
  }
  EEPROM.end();
  Serial.print("ready reading EEPROM:");
  Serial.println(dest);
}

float getTemperatura() {
  float temp;
  do {
    DS18B20.requestTemperatures();
    temp = DS18B20.getTempCByIndex(0);
    delay(100);
  } while (temp == 85.0 || temp == (-127.0));
  return temp;
}

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

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void rotinaModo() {
  // faz a leitura do pino D2 (no nosso caso, o botão está ligado nesse pino)
  modo = digitalRead(botao);
  // checa se o botão está pressionado
  if (modo == HIGH) {
    tocar(musica, duracao);
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

    Serial. println ( " conectado ... yeey :) " );
  }
  else {
    WiFiManager wifiManager;
    wifiManager.autoConnect("MicrosWifiAP", "janela129");
    //SPIFFS.format();
    createFile();
    lerJson();
  }
}

void tocar(char* mus[], int tempo[]) {
  int tom = 0;
  for (int i = 0; mus[i] != "Fim"; i++) {
    if (mus[i] == "Do") tom = 262;
    if (mus[i] == "Re") tom = 294;
    if (mus[i] == "Mi") tom = 330;
    if (mus[i] == "Fa") tom = 349;
    if (mus[i] == "Sol") tom = 392;
    if (mus[i] == "La") tom = 440;
    if (mus[i] == "Si") tom = 494;
    if (mus[i] == "Do#") tom = 528;
    if (mus[i] == "Re#") tom = 622;
    if (mus[i] == "Fa#") tom = 370;
    if (mus[i] == "Sol#") tom = 415;
    if (mus[i] == "La#") tom = 466;
    if (mus[i] == "Pausa") tom = 0;
    tone(buzzer, tom, tempo[i]);
    delay(tempo[i]);
  }
  noTone(buzzer);
}

void beep() {
  tone(buzzer, 1500);
  delay(200);
  noTone(buzzer);
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
void gravarJson() {
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

    File configFile = SPIFFS.open("/config.json", "r+");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
}
void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(botao, INPUT);
  Serial.begin(115200);
  rotinaModo();
  DS18B20.begin();
  initMQTT();

  temperatura = getTemperatura();
  dtostrf(temperatura, 2, 2, temperaturaString);
  client.subscribe("TemperaturaAtual");

  ntp.begin();//Inicia o NTP.
  ntp.forceUpdate();//ForÃ§a o Update.


}

void callback(char* topic, byte* payload, unsigned int length) {

  String message;
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    message += c;
  }
  Serial.println(String(topic) + "=" + String(message));
  Serial.flush();
}

void loop() {

  client.loop();
  temperatura = getTemperatura();
  dtostrf(temperatura, 2, 2, temperaturaString);
  Serial.println(temperaturaString);
  client.publish("TemperaturaAtual", temperaturaString );
  hora = ntp.getFormattedTime();//Armazena na vÃ¡riavel HORA, o horario atual.
  Serial.println(hora);//Printa a hora jÃ¡ formatada no monitor.
  //  client.publish("TemperaturaAtual", hora );
  delay(7000);
}


