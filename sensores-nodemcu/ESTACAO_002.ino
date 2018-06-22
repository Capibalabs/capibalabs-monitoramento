/*
 CapibaLabs
 ================================================================================================
 Autor    : Michel Nascimento
 E-mail   : mhsn@a.recife.ifpe.edu.br
 Data     : 14/05/2017
 Titulo   : Comunicacao NodeMCU-Servidor UV-CO-Luminosidade
 ================================================================================================
 Autor    : Arthur Castro
 E-mail   : alc@a.recife.ifpe.edu.br
 Data     : 13/09/2017
 Titulo   : Construção do hardware, Leitura de Sensores e multiplexação da entrada analógica
 ================================================================================================
 */

// Bibliotecas
#include <ESP8266WiFi.h>

#define pinC D4
#define pinB D3
#define pinA D2

// Variável armazena saída analógica para os sensores analógicos
#define pinco 0
#define pinuv 1
#define pinldr 2

// Valores de proporções do map-mux
#define prop1 1
#define prop2 988

// Variavel para indicar status defuncionamento
#define errorPin 16

const int sleep_time = 20;  

// Cria o client para atribuições wifi
WiFiClient client;

// Variaveis que armazenam SSID e password da rede wifi
const char* ssid = "AP_RNP";
const char* password = "rnp@1fp3";

// SETUP
void setup()
{
  pinMode(pinC,OUTPUT);
  pinMode(pinB,OUTPUT);
  pinMode(pinA,OUTPUT);
 

// Configura o led de status 
  pinMode(errorPin, OUTPUT);

// Flash do led para indicar inicio da operação
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(errorPin , HIGH);
    delay(200);
    digitalWrite(errorPin , LOW);
    delay(200);
  }

  // Inicia comunicação serial
  Serial.begin(115200);

  // Delay apenas para início seguro do sensor sht
  delay(10);

  // Mensagens para aviso de conexão com a rede
  Serial.println("");
  Serial.println("");
  Serial.print("Conectando à rede ");
  Serial.println(ssid);

  // Método para escanear redes wifi próximas
  int n = WiFi.scanNetworks();

  Serial.println("escaneamento realizado");
  if (n == 0)
  {
    Serial.println("Nenhuma rede foi encontrada");
    Serial.println("Sleep");
  }

  // Caso haja rede, conecta com a wifi
  WiFi.begin(ssid, password);

  // Conexão realizada
  Serial.println("");
  Serial.println("Wi-Fi connected");
}

// Fim do setup

/* Variáveis para guardar valor da porta analógica
 e Percentual de calibração do ldr */
  int lum;
  float lumP;

/* Variáveis reservadas para medições de CO */
  int co_analog;
  // Resistência de carga
  int mqR = 10000;
  // Resistência de superfície
  float rS;
  /*
     * r0 - Parâmetro definido no esquema de calibração do sensor, 
     * Uso de método de calibração adequado
     * às condições
     */ 
    float r0 = 1211.39;

/* Variáveis do sensor de UV*/
float uv_analog;
String UVIndex;
// Atribuição do ip do servidor
char server[] = {"192.168.127.14"};
int port = 80;

// Limite de intervalo entre conexões
unsigned long lastConnectionTime = 0;
const unsigned long postingInterval = 10L * 1000L;
 int uv, ldr;
 float co;
// Função para transmissão dos dados do sensor
void transmitSensorData()
{
  // Leitura dos dados do client
  while (client.available()) 
  {
    char c = client.read();
    Serial.write(c);
  }

  if (millis() - lastConnectionTime > postingInterval) 
  {
    // Para a conexão com o client
    client.stop();

    // Variáveis a serem passadas por parãmetro no método GET
    String requestData = "co=" + String(co);
    String requestData1 = "uv=" + UVIndex;
    String requestData2 = "luminosidade=" + String(lumP);   
        
    // Preparação para envio dos dados via GET
    if (client.connect(server, port)) 
    {
      // Se ocorreu a coexão com o servidor
      Serial.println("[INFO] Servidor conectado - GET iniciado");
       
      // Fazendo HTTP request:

      // Passagem do GET direcionado ao arquivo de tratamento
      client.println("GET /node-server/estacao2.php?" + requestData + "&" + requestData1 + "&" + requestData2 + " HTTP/1.1");
      client.println("Host: " + String(server));
      client.println("Connection: close");
      client.println();    

      lastConnectionTime = millis();
    
      Serial.println("[INFO] GET Realizado");
    } 
    else 
    {
      // Conexão com o servidor falhou
      Serial.println("[ERROR] Conexão falhou.....");
    }    
  }

  Serial.println("......................");  
}

void loop()
{
 
        
                    
  /* Luminosidade */
    ldr = analogReadMUX(pinldr); 
    lumP = map(ldr,prop1,prop2,0,100);

  /* Gás CO */
    co_analog = analogReadMUX(pinco); 
    // Cálculo do RS
    rS = ((987.0 * mqR) / co_analog) - mqR;
    
    // Função transformação, definida após processo calibração
    co = exp(log((rS/r0)/20.48)/-0.4687);
    
    // UV
    
    int analog_uv = analogReadMUX(pinuv); 
    float voltage = (analog_uv * (3.3 / 1023.0))*1000;
    if(voltage<=150)
      {
        UVIndex = "0";
      }else if (voltage>150 && voltage<=227)
      {
        UVIndex = "0";
      }else if (voltage>227 && voltage<=318)
      {
        UVIndex = "1";
      }
      else if (voltage>318 && voltage<=408)
      {
        UVIndex = "2";
      }else if (voltage>408 && voltage<=503)
      {
        UVIndex = "3";
      }
      else if (voltage>503 && voltage<=606)
      {
        UVIndex = "4";
      }else if (voltage>606 && voltage<=696)
      {
        UVIndex = "5";
      }else if (voltage>696 && voltage<=795)
      {
        UVIndex = "6";
      }else if (voltage>795 && voltage<=881)
      {
        UVIndex = "7";
      }
      else if (voltage>881 && voltage<=976)
      {
        UVIndex = "8";
      }
      else if (voltage>976 && voltage<=1079)
      {
        UVIndex = "9";
      }
      else if (voltage>1079 && voltage<=1170)
      {
        UVIndex = "10";
      }else if (voltage>1170)
      {
        UVIndex = "11";
      }
    // Chamada do método de transmissão dos dados
    transmitSensorData();  
    
  

    
  // Tempo de espera para cada transmissão
  delay(1000*60*10); 
}

 int analogReadMUX(int AX){
  #define pinMUX A0
  pinMode(pinMUX, INPUT);
  unsigned int leitura;                         
  
// ---------- Chaveamento do Pino do MUX ----------
  switch(AX){
    case 0:
      digitalWrite(pinC, 0);
      digitalWrite(pinB, 0);
      digitalWrite(pinA, 0);
      break;
    case 1:
      digitalWrite(pinC, 0);
      digitalWrite(pinB, 0);
      digitalWrite(pinA, 1);
      break;
    case 2:
      digitalWrite(pinC, 0);
      digitalWrite(pinB, 1);
      digitalWrite(pinA, 0);
      break;
  }
  leitura = analogRead(pinMUX);
  return leitura;
  }         
