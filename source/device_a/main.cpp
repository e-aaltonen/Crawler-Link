/* Crawler Link
by E. Aaltonen 2023
 Device A: ESP32C3  
 for physical user interface, incorporated in the RC transmitter
 linked with Device B through 1-way ESP-NOW (as initiator) 
 Programming environment: PlatformIO
 */
 
// Include Libraries
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// Variables for test data

//Pins
#define NAPPI_VALOMIINUS D10
#define NAPPI_VALOPLUS D5
#define NAPPI_VAS_VILKKU D8
#define NAPPI_OIK_VILKKU D7
#define NAPPI_JARRU D4
#define NAPPI_PERUUTUS D3
#define PARISTO_VOLT A0

int kBit = 0;
int anaLuku = 0;
bool jonossa = 0;
float txVolt = 0;

// MAC Address of responder - edit as required
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Replace with MAC on device b, as necessary

// Lähetettävä data
typedef struct espnow_msg {
  int a;
  float b;
} espnow_msg;

espnow_msg txData;

// Peer info
esp_now_peer_info_t peerInfo;


// Callback function called when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  
  // Set up Serial Monitor
  Serial.begin(115200);
  delay(1000);
  Serial.println("Lähetysyksikkö");

  pinMode(NAPPI_VALOMIINUS, INPUT_PULLDOWN);
  pinMode(NAPPI_VALOPLUS, INPUT_PULLDOWN);
  pinMode(NAPPI_VAS_VILKKU, INPUT_PULLDOWN);
  pinMode(NAPPI_OIK_VILKKU, INPUT_PULLDOWN);
  pinMode(NAPPI_JARRU, INPUT_PULLDOWN);
  pinMode(NAPPI_PERUUTUS, INPUT_PULLDOWN);
  pinMode(PARISTO_VOLT, INPUT);

  delay(1000);
 
  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initilize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the send callback
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  
  // Create test data
  
  if(digitalRead(NAPPI_VALOMIINUS)){
    kBit = kBit^4;
    jonossa = true;
  }

  if(digitalRead(NAPPI_VALOPLUS)){
    kBit = kBit^8;
    jonossa = true;
  }

  if(digitalRead(NAPPI_VAS_VILKKU)){
    kBit = kBit^1;
    jonossa = true;
  }

  if(digitalRead(NAPPI_OIK_VILKKU)){
    kBit = kBit^2;
    jonossa = true;
  }  

  if(digitalRead(NAPPI_JARRU)){
    kBit = kBit^16;
    jonossa = true;
  }  

  if(digitalRead(NAPPI_PERUUTUS)){
    kBit = kBit^32;
    jonossa = true;
  }  

  if(jonossa){
    
    Serial.println("--");

    anaLuku = analogRead(PARISTO_VOLT);
    txVolt = map(anaLuku, 0, 4095, 0, 6000)/1000.0;

    txData.a = kBit;
    txData.b = txVolt;

    Serial.println(txData.a);
    Serial.println(txData.b);
      
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &txData, sizeof(txData));
    
    if (result == ESP_OK) {
      Serial.println("Sending confirmed");
    }
    else {
      Serial.println("Sending error");
    }
    delay(500);

    jonossa = false;
    kBit = 0;
  }

  // Format structured data
}
