/* Crawler Link
by E. Aaltonen 2023
 Device B: ESP32C3  
 for controlling onboard hardware, incorporated in the RC truck
 linked with Device A through 1-way ESP-NOW (as respondent) 
 and with Device C through I2C (as slave)
 Programming environment: PlatformIO
 */
 
// Include Libraries
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

//Pins
#define LED_VAS_VILKKU D10
#define LED_OIK_VILKKU D9
#define LED_AJO D8
#define LED_VAS_TAKA D7
#define LED_OIK_TAKA D6
#define LED_PERUUTUS D3

//Älä käytä näitä ledeille tms:
/*I2C SDA: D4
  I2C SCL: D5*/

// Vastaanotettava data
typedef struct espnow_msg {
  int a;
  float b;
} espnow_msg;

espnow_msg rxData;

// PWM-vakiot
const int PWMparkki = 32; // parkkien oletuskirkkaus
const int PWMtaka = 32; // takavalojen oletuskirkkaus
const int PWMajo = 32; // lähivalon PWM-perusarvo

int vvas=0; // vasen vilkku
int voik=0; // oikea vilkku
int vtemp = 0; // vilkkujen tilapäismuuttuja
int valo=0; // ajovalojen tila
long vt = 0; // vilkun sykli
int tRefr = 0; // ledien päivitysaika

int jarru = 0; // jarruvalon tarve
int peruutus = 0; // peruutusvalon tarve
int vperus = 0; // PWM-perusarvo
int tperus = 0; // PWM-perusarvo


float akkuV = 0.0; // akku 1
float txV = 0.0; // lähettimen paristo


void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len); // ESP-NOW
void requestEvent();  // I2C, datan lähetys
void receiveEvent(int a); //I2C, datan vastaanotto

int vilkunKirkkaus(long aika, int b);
void laitaValot(int k);

void setup() {
  // Set up Serial Monitor
  Serial.begin(115200);
  pinMode(LED_VAS_VILKKU, OUTPUT);
  pinMode(LED_OIK_VILKKU, OUTPUT);
  pinMode(LED_VAS_TAKA, OUTPUT);
  pinMode(LED_OIK_TAKA, OUTPUT);
  pinMode(LED_VAS_TAKA, OUTPUT);
  pinMode(LED_AJO, OUTPUT);
  pinMode(LED_PERUUTUS, OUTPUT);

  delay(1000);
  Serial.println("LED-yksikkö");
  
  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  
  Wire.begin(8);  //I2C, osoite 8
  Wire.onRequest(requestEvent); 
  Wire.onReceive(receiveEvent);

  // Initilize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register callback function
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Valot päälle");
  digitalWrite(LED_VAS_VILKKU, HIGH);
  digitalWrite(LED_OIK_VILKKU, HIGH);
  digitalWrite(LED_VAS_TAKA, HIGH);
  digitalWrite(LED_OIK_TAKA, HIGH);
  digitalWrite(LED_VAS_TAKA, HIGH);
  digitalWrite(LED_AJO, HIGH);
  digitalWrite(LED_PERUUTUS, HIGH);
  delay(1000);
  digitalWrite(LED_VAS_VILKKU, LOW);
  digitalWrite(LED_OIK_VILKKU, LOW);
  digitalWrite(LED_VAS_TAKA, LOW);
  digitalWrite(LED_OIK_TAKA, LOW);
  digitalWrite(LED_VAS_TAKA, LOW);
  digitalWrite(LED_AJO, LOW);
  digitalWrite(LED_PERUUTUS, LOW);
  Serial.println("Valot pois");

  tRefr = millis();
  vt = millis();

}
 
void loop() {

    //Vilkkusykli
    if(millis()-tRefr > 50){
      if((millis()-vt) > 1200){
        vt = millis();
      }

      if(vvas){
        analogWrite(LED_VAS_VILKKU, vilkunKirkkaus((millis()-vt), vperus)); // vas. etuvilkku
        analogWrite(LED_VAS_TAKA, vilkunKirkkaus((millis()-vt), tperus)); // vas. etuvalo
      }
      if(voik){
        analogWrite(LED_OIK_VILKKU, vilkunKirkkaus((millis()-vt), vperus));// oik. etuvilkku
        analogWrite(LED_OIK_TAKA, vilkunKirkkaus((millis()-vt), tperus));// oik. takavalo
      }
      
      tRefr = millis();
  }

}

// Callback function executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rxData, incomingData, sizeof(rxData));
  /*Serial.print("Data received: ");
  Serial.println(len);
  Serial.print("k-luku: ");
  Serial.println(rxData.a);
  Serial.print("Pariston jännite: ");
  Serial.println(rxData.b);*/

  //kByt = rxData.a;
  laitaValot(rxData.a);

}

int vilkunKirkkaus(long aika, int b)
{
  int kirk;
  
  if(aika < 600){
    kirk = aika*2.5;
    if(kirk > 255){
      kirk = 255;
    }
  }
  
  if (aika >= 600){
    kirk = 255-(aika-600)*2;
  }

  if(kirk < b){
      kirk = b;
    }
  return kirk;
}

void laitaValot(int k)
{
  //vähennetään valoja
  if(bitRead(k, 2)==1){
    valo--;
    if(valo<0){
      valo = 0;
    }
  }

  //lisätään valoja
  if(bitRead(k, 3)==1){
    valo++;
    if(valo>3){
      valo = 3;
    }
  }


  //säädetään vilkkujen peruskirkkaus valojen mukaan
  if(valo == 0){
    vperus = 0;
    tperus = 0;    
  }

  //sytytellään valoja
  if(valo>0){
    vperus = PWMparkki;
    tperus = PWMtaka;

    switch (valo){
      case(1):
        analogWrite(LED_AJO, 0);
        //Serial.print("Ajovalot pois");
        break;
      case(2):
        analogWrite(LED_AJO, PWMajo);
        //Serial.print("Lähivalot");
        break;
      case(3):
        analogWrite(LED_AJO, 4095);
        //Serial.print("Kaukovalot");
        break;
    }

  }
  // Päivitetään etuvilkkujen kirkkaus
  analogWrite(LED_VAS_VILKKU, vperus);
  analogWrite(LED_OIK_VILKKU, vperus);

  //viedään komennot vilkkujen ja takavalojen muuttujiin
  vvas ^= bitRead(k, 0);
  voik ^= bitRead(k, 1);
  
  //asetetaan vilkkujen oletusmuuttuja
  if(vtemp)
  {
    if(!(vvas || voik))
      vtemp = 0;
  }
  else
  {
      if(vvas || voik)
      vtemp = 1;
  }
  
  peruutus ^= bitRead(k, 5);

  analogWrite(LED_PERUUTUS, (peruutus*4095)); //Peruutusvalo kytketään suoraan komennosta
  
  //tarkistetaan jarru
  jarru ^= bitRead(k, 4);
  
  if(jarru) // jos jarrutus alkaa tai jatkuu
  {
    if(!vvas)
    {
      analogWrite(LED_VAS_TAKA, 4095);
    }
    if(!voik)
    {
      analogWrite(LED_OIK_TAKA, 4095);
    }
  }

  if(!jarru) // jos ei jarruteta
    {
      analogWrite(LED_VAS_TAKA, tperus);
      analogWrite(LED_OIK_TAKA, tperus);
    }


  //käynnistetään vilkkuviive, jos jompikumpi vilkku laitettiin nyt päälle
  if(!vtemp && (vvas | voik))
  { 
    vt = millis();
    
  }
 

}

void requestEvent() {
  byte send = 0;
  if(vvas)
    send |= 1;
  
  if(voik)
    send |= 2;
  
  int v = valo << 2;
  send |= v;
  
  if(jarru)
    send |= 16;
  
  if(peruutus)
    send |= 32;
  
  Wire.write(send); 
}


void receiveEvent(int n)
{
    int y = Wire.read();
    //Serial.print("Saatu komento: ");
    //Serial.print(y); 

    laitaValot(y);
}
