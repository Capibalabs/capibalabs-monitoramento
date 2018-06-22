/*
 CapibaLabs
 ============================================================================
 Autor    : Michel Nascimento
 E-mail   : mhsn@a.recife.ifpe.edu.br
 Data     : 09/2017
 Titulo   : Comunicacao NodeMCU-Servidor Temperatura-Umidade-Luminosidade
 ============================================================================
 Autor    : Arthur Castro
 E-mail   : alc@a.recife.ifpe.edu.br
 Data     : 09/2017
 Titulo   : Construção do hardware e Leitura de sensores
 ============================================================================
 */
 
#include <DHT.h>
#include <ESP8266WiFi.h>

#define ldr A0

#define errorPin 16
#define DHTPIN D3
#define DHTTYPE DHT22

// Instantiates and initializes the dht object
DHT dht(DHTPIN, DHTTYPE);

// Define and initialize constants and variables that we'll use later in the code
const int sleep_time = 20;  // Time to sleep (in seconds) between posts to Ubidots
WiFiClient client;

// After creating an account on Ubidots, you'll be able to setup variables where you
// will store the data. In order to post the measurements to the Ubidots variables,
// we need their "IDs", which are given on the website
//String variable_id1 = "592c4679762542443134bce6";
//String variable_id2 = "592c46857625424434d99c85";
//
//// In addition, we'll need the API token, which is what prevents other users
//// Ubidots to publish their data to one of your variables
//String token = "1J2aKdHwdYG1VtButf7mlcG2jvwtYx";

// We'll also initialize the values for our Wi-Fi network
const char* ssid = "Sua Rede";
const char* password = "Senha da Rede";

//////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
void ubiSave_value(String, String);

// The setup function is executed once by the ESP8266 when it's powered up or reset
void setup()
{
  pinMode(ldr,INPUT);

  pinMode(errorPin, OUTPUT); // sets pin as an output to drive an LED for status indication
  // The following loop flashes the LED four times to indicate we're inside the setup function
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(errorPin , HIGH);
    delay(200);
    digitalWrite(errorPin , LOW);
    delay(200);
  }

  // Initialize Serial (USB) communication, which will be used for sending debugging messages
  // to the computer
  Serial.begin(115200);

  // Start the communication with the DHT sensor by callibg the begin method of the dht object
  dht.begin();
  // Manual delay while the communication with the sensor starts
  delay(10);

  // Debug messsages to indicate we're about to connect to the network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Use the scanNetworks method inside the Wi-Fi class to scan for any available Wi-Fi networks
  // nearby. If none are found, go to sleep
  int n = WiFi.scanNetworks();

  Serial.println("scan done");
  if (n == 0)
  {
    Serial.println("no networks found");
    Serial.println("Going into sleep");
    // ESP.deepSleep(sleep_time * 1000000);
  }

  // If networks are found, attempt to connect to our Wi-Fi network
  WiFi.begin(ssid, password);

  // While the connection is not established, IDLE inside this while loop
  //while (WiFi.status() != WL_CONNECTED)
  //{
  // delay(500);
  //Serial.print(".");
  //}

  // Once the connection to our Wi-Fi netowrk is successful, print some debug messages
  Serial.println("");
  Serial.println("Wi-Fi connected");
}
int lum;
float lumP;

float temp;
float hum;
char server[] = {"192.168.0.0"};
int port = 80;

unsigned long lastConnectionTime = 0;
const unsigned long postingInterval = 10L * 1000L;

void transmitSensorData()
{
  // Read all incoming data (if any)
  while (client.available()) 
  {
    char c = client.read();
    Serial.write(c);
  }

  if (millis() - lastConnectionTime > postingInterval) 
  {
    client.stop();
    
    String requestData = "temperatura=" + String(temp);
    String requestData1 = "umidade=" + String(hum);
    String requestData2 = "luminosidade=" + String(lumP);
    Serial.println(String(temp));
    Serial.println("[INFO] Connecting to Server");   
        
    // Prepare data or parameters that need to be posted to server
    if (client.connect(server, port)) 
    {
      Serial.println("[INFO] Server Connected - HTTP GET Started");
       
      // Make a HTTP request:
      client.println("GET /node-server/add.php?" + requestData + "&" + requestData1 + "&" + requestData2 + " HTTP/1.1");
      client.println("Host: " + String(server));
      client.println("Connection: close");
      client.println();    

      lastConnectionTime = millis();
    
      Serial.println("[INFO] HTTP GET Completed");
    } 
    else 
    {
      // Connection to server:port failed
      Serial.println("[ERROR] Connection Failed");
    }    
  }

  Serial.println("-----------------------------------------------");  
}

void loop()
{
  lum = analogRead(ldr);
  lumP = map(lum,0,1023,0,100);

  // Read the current temperature and humidity measured by the sensor
  temp = dht.readTemperature(); //true retorna valor em °F
  hum = dht.readHumidity();
  // Call our user-defined function ubiSave_value (defined below), and pass it both the
  // measurements as well as the corresponding Ubidots variable IDs
//  ubiSave_value(String(variable_id1), String(temp));
//  ubiSave_value(String(variable_id2), String(hum));

  // Send some debug messages over USB
//  Serial.println("Ubidots data");
//  Serial.println("temperature: " + String(temp));
//  Serial.println("humidity: " + String(hum));
//  Serial.println(" Going to Sleep for a while !" );

  // deepSleep time is defined in microseconds. Multiply seconds by 1e6
  //ESP.deepSleep(sleep_time * 1000000);//one or other

  // Wait a few seconds before publishing additional data to avoid saturating the system
  //delay(sleep_time * 1000);

  transmitSensorData();  

  // Delay
  delay(1000*60*10); 
}
