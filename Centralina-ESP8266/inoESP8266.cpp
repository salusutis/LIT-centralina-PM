//#include <DHT.h>
#include <SDS011.h>
#include "base64.hpp"
#include "RunningAverage.h"
SDS011 sds;

float p10, p25;
int error;
RunningAverage pm25Stats(10); 
RunningAverage pm10Stats(10); 
RunningAverage temperatureStats(10); 
const int sleepTime = 60 * 1;

#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <ESP8266WiFiType.h>
#include <ESP8266WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiSTA.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "DHT.h"
#define DHTTYPE DHT22   // DHT 22

#include <ArduinoJson.h>

WiFiClient client;
// La connessione di WeMos D1 mini al un network WiFi viene
// realizzata usando un personal encryption WEP e WPA2
// Per poter sapere a quale rete connettersi bisogna
// effettuare un broadcast dell'SSID (nome del network)

// definizione di due array di caratteri
// in cui memorizzare nome della rete WiFi e password

//const char* ssid = "iPhone di tonio";
//const char* pass = "postpost";

const char* ssid = "iPhone di tonio";
const char* pass = "postpost";

String writeAPIKey = "DNXMT8MOFXL1YDW2"; // write API key for your ThingSpeak Channel
const char* server = "api.thingspeak.com"; //"52.1.229.129" 

const char* host = "192.168.1.4";
const int httpPort = 80;

// Sensore DHT
const int pinDHT = D7; //D4

unsigned int Pm25 = 0;
unsigned int Pm10 = 0;
// Inizializzazione del sensore DHT
DHT dht(pinDHT, DHTTYPE);

// Variabili temporanee
static char tempCelsius[7];
static char tempUmidita[7];

#define TIMEOUT  5000   

  String getResponse(){
      String response;
      long startTime = millis();
    
      delay( 200 );
      while ( client.available() < 1 && (( millis() - startTime ) < TIMEOUT ) ){
            delay( 5 );
      }
      
      if( client.available() > 0 ){ // Get response from server
         char charIn;
         do {
             charIn = client.read(); // Read a char from the buffer.
             response += charIn;     // Append the char to the string response.
            } while ( client.available() > 0 );
        }
      client.stop();
            
      return response;
    }



void reconnect() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  sds.begin(D1, D2);
  delay(10);                // inizializzazione Serial Monitor
  dht.begin();              // inizializzazione del DHT


  // Connessione alla rete WiFi

  Serial.println();
  Serial.println();
  Serial.println("------------- Avvio connessione ------------");
  Serial.print("Tentativo di connessione alla rete: ");
  Serial.println(ssid);

  /*
      Viene impostata l'impostazione station (differente da AP o AP_STA)
     La modalità STA consente all'ESP8266 di connettersi a una rete Wi-Fi
     (ad esempio quella creata dal router wireless), mentre la modalità AP
     consente di creare una propria rete e di collegarsi
     ad altri dispositivi (ad esempio il telefono).
  */

  WiFi.mode(WIFI_STA);

  /*
      Inizializza le impostazioni di rete della libreria WiFi e fornisce lo stato corrente della rete,
      nel caso in esempio ha come parametri ha il nome della rete e la password.
      Restituisce come valori:

      WL_CONNECTED quando connesso al network
      WL_IDLE_STATUS quando non connesso al network, ma il dispositivo è alimentato
  */
  WiFi.begin(ssid, pass);

  /*
      fino a quando lo non si è connessi alla WiFi
      compariranno a distanza di 250 ms dei puntini che
      evidenziano lo stato di avanzamento della connessione
  */
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }

  // se connesso alla WiFi stampa sulla serial monitor
  // nome della rete e stato di connessione
  Serial.println("");
  Serial.print("Sei connesso ora alla rete: ");
  Serial.println(ssid);
  Serial.println("WiFi connessa");

 
}

void loop() {
  pm25Stats.clear();
  pm10Stats.clear();
  temperatureStats.clear();
  reconnect();

  sds.wakeup();
  
  Serial.println("Calibrating SDS011 (15 sec)");
  delay(15000);
    for (int i = 0; i < 10; i++)
  {
    error = sds.read(&p25, &p10);

    if (!error && p25 < 999 && p10 < 1999) {
      pm25Stats.addValue(p25); 
      pm10Stats.addValue(p10);
    }

      Serial.println("Average PM10: " + String(pm10Stats.getAverage()) + ", PM2.5: "+ String(pm25Stats.getAverage()));
    delay(1500);
  }
 


 
  // La lettura da parte del sensore può richiedere anche più di 2 secondi

  // lettura dell'umidità
  float h = dht.readHumidity();

  // Lettura temperatura in gradi Celsius
  float t = dht.readTemperature();

  // Verifica se la lettura è riuscita altrimenti si ripete la rilevazione
  if (isnan(h) || isnan(t)) {
    Serial.println("Impossibile leggere i dati dal sensore DHT!");
    strcpy(tempCelsius, "Fallito");
    strcpy(tempUmidita, "Fallito");
  }
  else {
    // Calcola i valori di temperatura in Celsius e Umidità

    float hic = dht.computeHeatIndex(t, h, false);

    dtostrf(hic, 6, 2, tempCelsius);

    dtostrf(h, 6, 2, tempUmidita);

    Serial.print("Temperatura: ");
    Serial.print(t);
    Serial.print(" *C ");
    
    Serial.print("Umidita': ");
    Serial.print(h);
    Serial.println(" %\t");
 Serial.flush();
float pm25n=normalizePM25(float(pm25Stats.getAverage()),h);
float pm10n=normalizePM10(float(pm10Stats.getAverage()),h);

    String body = "field1=";
           body += String(pm10n);
           body += "&field2=";
           body += String(pm25n);
           body += "&field3=";
           body += String(t);
           body += "&field4=";
           body += String(h);


 
 WiFiClient client;
  if (!client.connect(server, httpPort)) {
    Serial.println("connection failed");
    return;
  }


   // POST data via HTTP
   //Serial.println( "Connecting to ThingSpeak for update..." );
   //Serial.println();
    
   //client.println( "POST /update HTTP/1.1" );
   //client.println( "Host: api.thingspeak.com" );
   //client.println( "Connection: close" );
   //client.println( "Content-Type: application/x-www-form-urlencoded" );
   //client.println( "Content-Length: " + String( body.length() ) );
   //client.println();
   //client.println( body );
   // 
   //Serial.println( body );
   // 
   //String answer=getResponse();
   //Serial.println( answer );
  
  Serial.println(body);
  Serial.println("\nconnected to server");     
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + writeAPIKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(body.length());
    client.print("\n\n");
    client.print(body);
    client.print("\n\n");

  }

  
  Serial.println("Sleep(" + String(sleepTime) + ")");
  sds.sleep();
  delay(sleepTime * 1000);

}
float normalizePM25(float pm25, float humidity){
  return pm25/(1.0+0.48756*pow((humidity/100.0), 8.60068));
}

float normalizePM10(float pm10, float humidity){
  return pm10/(1.0+0.81559*pow((humidity/100.0), 5.83411));
}

void ProcessSerialData()
{
  uint8_t mData = 0;
  uint8_t i = 0;
  uint8_t mPkt[10] = {0};
  uint8_t mCheck = 0;
  while (Serial1.available() > 0)
  {
    // from www.inovafitness.com
    // packet format: AA C0 PM25_Low PM25_High PM10_Low PM10_High 0 0 CRC AB
    mData = Serial1.read();     delay(2);//wait until packet is received
    //Serial.println(mData);
    //Serial.println("*");
    if (mData == 0xAA) //head1 ok
    {
      mPkt[0] =  mData;
      mData = Serial1.read();
      if (mData == 0xc0) //head2 ok
      {
        mPkt[1] =  mData;
        mCheck = 0;
        for (i = 0; i < 6; i++) //data recv and crc calc
        {
          mPkt[i + 2] = Serial1.read();
          delay(2);
          mCheck += mPkt[i + 2];
        }
        mPkt[8] = Serial1.read();
        delay(1);
        mPkt[9] = Serial1.read();
        if (mCheck == mPkt[8]) //crc ok
        {
          //Serial.flush();
          //Serial.write(mPkt,10);

          Pm25 = (uint16_t)mPkt[2] | (uint16_t)(mPkt[3] << 8);
          Pm10 = (uint16_t)mPkt[4] | (uint16_t)(mPkt[5] << 8);
          if (Pm25 > 9999)
            Pm25 = 9999;
          if (Pm10 > 9999)
            Pm10 = 9999;
              //      Serial.println('Pm25'+Pm25);
              //      Serial.println('Pm10'+Pm10);
          //get one good packet
          return;
        }
      }
    }
  }
}
