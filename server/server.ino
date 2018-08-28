#include <FS.h>

#include <NTPClient.h>//Biblioteca do NTP.
#include <WiFiUdp.h>//Biblioteca do UDP.
WiFiUDP udp;//Cria um objeto "UDP".
NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000);//Cria um objeto "NTP" com as configurações.
String hora;//Váriavel que armazenara o horario do NTP.
/*------------------------------------------------------------------------*/
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
  --------------------------Configuracao Wi-FI ------------------------*/
#include <ESP8266WiFi.h>
const char* ssid = "WifiCasa";
const char* password =  "84432320";
/*--------------------------------------------------------------------------
  --------------------------Configuracao Wi-FI Server ------------------------*/
#include <WiFiServer.h>//Biblioteca que gerencia o uso do TCP.
WiFiServer servidor(80);//Cria um objeto "servidor" na porta 80 (http).
WiFiClient cliente;//Cria um objeto "cliente".
String html;//String que armazena o corpo do site.
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
float getTemperatura() {
  float temp;
  do {
    DS18B20.requestTemperatures();
    temp = DS18B20.getTempCByIndex(0);
    delay(100);
  } while (temp == 85.0 || temp == (-127.0));
  return temp;
}

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
 
    if (client.connect("Micros", mqttUser, mqttPassword )) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
      
    }
  }
}

void setup() {    
  Serial.begin(9600);
  DS18B20.begin();
  initWifi(); 
  initMQTT();
  
  Serial.println(WiFi.localIP());//Printa o IP que foi consebido ao ESP8266 (este ip que voce ira acessar).
  servidor.begin();//Inicia o Servidor.
   
  ntp.begin();//Inicia o NTP.
  ntp.forceUpdate();//Força o Update.

  temperatura = getTemperatura();
  dtostrf(temperatura, 2, 2, temperaturaString);
 client.subscribe("TemperaturaAtual");
  
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
  http();
  temperatura = getTemperatura();
  dtostrf(temperatura, 2, 2, temperaturaString);

  delay(5000);  
  Serial.println(temperaturaString);
  client.publish("TemperaturaAtual", temperaturaString );
  hora = ntp.getFormattedTime();//Armazena na váriavel HORA, o horario atual.
  Serial.println(hora);//Printa a hora já formatada no monitor.
//  client.publish("TemperaturaAtual", hora );  
}
void http()//Sub rotina que verifica novos clientes e se sim, envia o HTML.
{
   cliente = servidor.available();//Diz ao cliente que há um servidor disponivel.
 
   if (cliente == true)//Se houver clientes conectados, ira enviar o HTML.
   {
      String req = cliente.readStringUntil('\r');//Faz a leitura do Cliente.
      Serial.println(req);//Printa o pedido no Serial monitor.
 
      if (req.indexOf("/LED") > -1)//Caso o pedido houver led, inverter o seu estado.
      {
         digitalWrite(D4, !digitalRead(D4));//Inverte o estado do led.
      }
 
      html = "";//Reseta a string.
      html += "HTTP/1.1 Content-Type: text/html\n\n";//Identificaçao do HTML.
      html += "<!DOCTYPE html><html><head><title>ESP8266 WEB</title>";//Identificaçao e Titulo.
      html += "<meta name='viewport' content='user-scalable=no'>";//Desabilita o Zoom.
      html += "<style>h1{font-size:2vw;color:black;}</style></head>";//Cria uma nova fonte de tamanho e cor X.
      html += "<body bgcolor='ffffff'><center><h1>";//Cor do Background
 
      //Estas linhas acima sao parte essencial do codigo, só altere se souber o que esta fazendo!
 
      html += "<form action='/LED' method='get'>";//Cria um botao GET para o link /LED
      html += "<input type='submit' value='LED' id='frm1_submit'/></form>";
 
      html += "</h1></center></body></html>";//Termino e fechamento de TAG`s do HTML. Nao altere nada sem saber!
      cliente.print(html);//Finalmente, enviamos o HTML para o cliente.
      cliente.stop();//Encerra a conexao.
   }
}
