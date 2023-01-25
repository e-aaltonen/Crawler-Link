/* Crawler Link
by E. Aaltonen 2023
 Device C: ESP8266  
 for WiFi Access Point
 linked with Device B via I2C (as master)
 and with the client divice WiFi (as an AP)
 Programming environment: Aruidno IDE
 */
 

#include <ESP8266WiFi.h>
#include "Wire.h"

// WiFi AP
const char WiFiAPPSK[] = "replace_with_your_password"; // Replace password as necessary


// AD-tulot
const int AD = AD; // akun jännitteen luku


//--
int byt = 0;

int vvas=0; // vasen vilkku
int voik=0; // oikea vilkku
int valo=0; // ajovalojen tila

int jarru = 0; // jarruvalo
int peruutus = 0;

float akkuV = 0.0; // akku 1
float txV = 0.0; // lähettimen paristo

WiFiServer server(80);

int val = -1; 

byte i2cdata; // i2c-väylästä 1 tavu

void setup() 
{
  setupWiFi();
  server.begin();
  
  delay(1000);
  Wire.begin(5, 4);        // SCL: D2 (4), SDA: D1 (5)
 
  Serial.begin(115200);     
  Serial.print("Käynnistyy ");
  for(int i = 0; i<3; i++){
    Serial.print(i);
    delay(1000);
  }

}

void loop() 
{
  //////
  // I2C-kysely
  //////
  Wire.requestFrom(8, 1);    // request 1 bytes from peripheral device #8

  while (Wire.available()) { // 
    i2cdata = Wire.read(); // vastaanotettu tavu
    
    vvas = bitRead(i2cdata, 0);
    voik = bitRead(i2cdata, 1);
    jarru = bitRead(i2cdata, 4);
    peruutus = bitRead(i2cdata, 5);
    int v = (i2cdata >> 2) & 3;
    valo = v;
    
  }
  delay(10);
  
  //////
  // Wifi-yhteys
  /////
  
  WiFiClient client = server.available();

  if(client) {
  /////
  // Selaimelta tulevan request-rivin lukeminen
  /////
  
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  //////
  // Syötteen käsittely
  //////

  val = -1;
  int n;
  String arvo;
  
  if (req.indexOf("/k/") != -1)
  {
    n = req.indexOf("/k/");
    arvo = req.substring(n+3,n+6); 

    if (req.indexOf("pois") != -1) {
      byt = 0;
      val = 0;
      Serial.println("Sammutetaan kaikki");
      
     }
    else
    {
      val = arvo.toInt();
      Serial.print("val: ");
      Serial.println(val);  
    }
    
      
  }// loppu


  if (val >= 0)
  {
    Serial.print("byt: ");
    Serial.println(byt, BIN);
    
    Serial.print("val: ");
    Serial.println(val, BIN);
    
    byt = (val ^ byt);
    Serial.print("byt: ");
    Serial.println(byt, BIN);
    
    
    vvas = bitRead(byt, 0);
    voik = bitRead(byt, 1);
    jarru = bitRead(byt, 4);
    peruutus = bitRead(byt, 5);


      Serial.print("Vvas: ");
      Serial.print(vvas);
      Serial.print(" voik: ");
      Serial.print(voik);
      
      Serial.print(" valo: ");
      Serial.println(valo);


    // Välitetään tiedot valoyksikölle
      Wire.beginTransmission(8); 
      Wire.write(val);  
      Wire.endTransmission(); 


  }// loppu
  
  
  client.flush();
  
  //////
  // Rakennetaan HTML-sivu
  //////
  
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  s += "<head> \r\n";
  s += "<link rel=\"icon\" href=\"data:,\"> \r\n"; // Estetään turha favicon-pyyntö
  s += "</head> \r\n";
    
  s += "<style>body {font-size: 50px; background-color: navy;}\r\n";
  s += "div {text-align: center; padding-top: 200;}\r\n";
  s += "button {font-size: 50px; width: 150px; height: 150px; padding: 20; border-radius: 20px; background: silver; box-shadow: 10px 10px;}\r\n";
  s += "button.valittu {box-shadow: 5px 5px; position: relative; left: 5px; top: 5px; opacity: 0.7;}\r\n";
  s += "table{background: navy; padding: 20px; box-shadow: 3px 5px 10px 10px #101030; }\r\n";
  s += "td {text-align: center; vertical-align: top; color: white; font-size: 20px; font-family: Arial; padding: 10px;}\r\n";
  s += "span { color: #202010; font-size: 40px; }\r\n";
  s += ".palaa {color: orange; text-shadow: 0 0 20px orange, 0 0 40px yellow;}\r\n";
  s += ".P-on {color: yellow; text-shadow: 0 0 20px white, 0 0 40px yellow;}\r\n";
  s += ".LO-on {color: green; text-shadow: 0 0 20px white, 0 0 40px lightgreen;}\r\n";
  s += ".HI-on {color: blue; text-shadow: 0 0 20px white, 0 0 40px lightblue;}\r\n";
  s += ".J-on {color: red; text-shadow: 0 0 20px orange, 0 0 40px red;}\r\n";
  s += ".Per-on {color: white; text-shadow: 0 0 20px yellow, 0 0 40px lightblue;}\r\n";
  s += "</style>\r\n";
  s += "<body onLoad=\"setTimeout('vilkkuviive()',600);setTimeout('alkutila()',12000);\">\r\n";


        s += "<p>\n";

        //if (val >= 0)
  //{
        s += "<div>\n";
        s += "<table width='100%'><tr>\n";
        s += "<td>Akku</td>\n";
        s += "<td>Tx</td></tr>\n";
        s += "<tr><td>" + String(akkuV, 2) +" V</td>\n";        
        s += "<td>" + String(txV, 2) +" V</td></tr></table>\n";
        s += "</div>\n";
        s += "<div>\n";
        
        s += "<table align='center'><tr><td>\n";
        
        s += "<a href='4'><button type='button'>&#9686; -</button></a>\n";
        s += "</td><td></td><td>\n";
        s += "<a href='8'><button type='button'>&#9686;+</button></a>\n";
        s += "<p>\n";
        s += "</td></tr><tr><td>\n";
                s += "<span \n";
                if (valo>0){     
          s += " class='P-on'";}
          s += ">&#9679;</span><p> P\n";
        s += "</td><td>\n";
                s += "<span \n";
                if (valo>1){     
          s += " class='LO-on'";}
          s += ">&#9679;</span><p> LO\n";
        s += "</td><td>\n";
                s += "<span \n";
                if (valo>2){     
          s += " class='HI-on'";}
          s += ">&#9679;</span><p> HI\n";
        s += "</td></tr><tr><td>       \n";
        
        s += "<a href='1'><button type='button'";
        if (vvas==1){     
          s += " class='valittu'";}
        s += ">&#8656;</button></a>\n";
        s += "<p>\n";
        s += "<span";

        if (vvas==1){     
          s += " class='eipala'";}
        s += ">&#9679;</span>\n";
        s += "</td><td>\n";
        s += "<a href='3'><button type='button'";
        if (vvas==1 && voik==1){     
          s += " class='valittu'";}
        s += ">&#8660;</button></a>\n";
        s += "</td><td>\n";
        s += "<a href='2'><button type='button'";
        if (voik==1){     
          s += " class='valittu'";}
        s += ">&#8658;</button></a>\n";
        s += "<p>\n";
        s += "<span";
        if (voik==1){     
          s += " class='eipala'";}
        s += ">&#9679;</span>\n";
        s += "</td></tr><tr><td>\n";
        
        s += "<a href='16'><button type='button'";
        if (jarru){     
          s += " class='valittu'";}
        s += ">J</button></a>\n";
        s += "</td><td></td><td>\n";
        s += "<a href='32'><button type='button'";
        if (peruutus){     
          s += " class='valittu'";}
        s += ">P</button></a>\n";
        s += "<p>\n";
        s += "</td></tr><tr><td>\n";
                s += "<span \n";
                if (jarru){     
          s += " class='J-on'";}
          s += ">&#9679;</span><p> J\n";
        s += "</td><td>\n";
        s += "</td><td>\n";
                s += "<span \n";
                if (peruutus){     
          s += " class='Per-on'";}
          s += ">&#9679;</span><p> P\n";
          s += "</td></tr><tr><td rowspan=3>\n";
        s += "<a href='0'><button type='button' width='100%'>&#9842;</button></a>\n";
        s += "</td></tr></table>\n";

        s += "</div>\n";

        s += "<script>\n";
        s += "var t=0;\n";
        s += "function vilkku() {\n";
            s += "t = t^1;\n";
           s += "switch(t) {\n";
            s += "case 0: \n";
                s += "document.querySelectorAll('.palaa').forEach(e => e.classList.replace('palaa', 'eipala'));\n";
            
                s += "break;\n";
            s += "case 1: \n";
                s += "document.querySelectorAll('.eipala').forEach(e => e.classList.replace('eipala', 'palaa'));\n";
            
            s += "}}\n";
        s += "function vilkkuviive() {\n";
        s += "    vilkku();\n";

        s += "    setTimeout('vilkkuviive()',600);}\n";
        s += "function alkutila() {\n";
        s += "    window.open('0', '_self');\n";
        s += "    }\n";
        
        s += "</script>\n";
        //}// loppu

  //////
  // Sivun lähetys
  //////
  
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // Yhteys katkaistaan ja client-olio tuhotaan
  }// if client: loppu
}

void setupWiFi()
{
  WiFi.mode(WIFI_AP);

  //////
  // SSID muodostetaan lisäämällä loppuun MAC-osoitteen 2 viimeistä tavua
  //////
  
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "ESP8266 Thing " + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WiFiAPPSK);
}
