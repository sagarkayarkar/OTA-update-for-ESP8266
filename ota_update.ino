
//Key Features 1.OTA Updates from internet
//             2.Set WIFI crediatials run time 
//*************This Program Has Been Written BY Sagar Kayarkar ******

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <HardwareSerial.h>

/* >>>>>>>>>>>>>update varaible */

/*const char* ssid = "Home2";
const char* password = "helloworld"; */
const String FirmwareVer={"3.2"}; 

//you can use differnet server or data base i am using github

#define URL_fw_Version "https://raw.githubusercontent.com/sagarkayarkar/update/master/version.txt"
#define URL_fw_Bin "https://raw.githubusercontent.com/sagarkayarkar/update/master/ota_update.ino.nodemcu.bin"

HTTPClient http;
String a;
int OTA_FLAG = 0;

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/* >>>>>>>>>>>>auth variable */

//Variables
int i = 0;
int hotspot_disable = 1;
int statusCode;
const char* ssid = "text";
const char* passphrase = "text";
String st;
String content;
String inputString ;

//Function Decalration
bool testWifi(void);
void launchWeb(void);
void setupAP(void);

//Establishing Local server at port 80 whenever required
ESP8266WebServer server(80);
// <<<<<<<<<<<<<<<<<<<<<<


void FirmwareUpdate()
{
  http.begin(URL_fw_Version,"CC AA 48 48 66 46 0E 91 53 2C 9C 7C 23 2A B1 74 4D 29 9D 33");     // check version URL
  delay(100);
  int httpCode = http.GET();            // get data from version file
  delay(100);
  String payload;
  if (httpCode == HTTP_CODE_OK)         // if version received
  {
      payload = http.getString();  // save received version
      Serial.println(payload );
  }
  else
  {
      Serial.print("error in downloading version file:");
      Serial.println(httpCode);
  }
  http.end();
  if (httpCode == HTTP_CODE_OK)         // if version received
  {
  if(payload.equals(FirmwareVer) )
  {   
     Serial.println("Device already on latest firmware version"); 
  }
  else
  {
      /* >>>>>>>>>>> you can slip this part upto left arraow for auto update */
     Serial.println("New firmware detected");
     Serial.println("Do you Wants to update the latest firmware");
     Serial.println("Type Yes To Update");
     Serial.println("Type No to Skip");
     UP :
     while(Serial.available()) 
     {
          a= Serial.readString();// read the incoming data as string
          Serial.println(a);
     }
     if(a == "")
        goto UP ;
     if(a == "no") 
     {
         OTA_FLAG = 1;
         Serial.println("Now Nodemcu will check update only after restart call");
         return ; 
     }
     else if(a == "yes" )
     {
        Serial.println("proceed with flash");
     }
     else 
     {
        Serial.println("please enter right entry");
        goto UP ;
     }
     /* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<< skip this part for auto update */
     WiFiClient client;
     ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
     t_httpUpdate_return ret = ESPhttpUpdate.update(URL_fw_Bin,"","CC AA 48 48 66 46 0E 91 53 2C 9C 7C 23 2A B1 74 4D 29 9D 33");
      switch (ret) {
          case HTTP_UPDATE_FAILED:
          Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          break;

          case HTTP_UPDATE_NO_UPDATES:
          Serial.println("HTTP_UPDATE_NO_UPDATES");
          break;

          case HTTP_UPDATE_OK:
          Serial.println("HTTP_UPDATE_OK");
          break;
       } 
  }
 }  
}

unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 30000;

 void repeatedCall(){
    unsigned long currentMillis = millis();
    if ((currentMillis - previousMillis) >= interval) 
    {
        // save the last time you blinked the LED
        previousMillis = currentMillis;
        if(OTA_FLAG == 0)
        {
            FirmwareUpdate();
        }
    }
 }

void setup()
{
  Serial.begin(115200); //Initialising if(DEBUG)Serial Monitor
  Serial.println();
  Serial.println("Disconnecting previously connected WiFi");
  WiFi.disconnect();
  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println();
  Serial.println();
  Serial.println("Startup");

  //---------------------------------------- Read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");

  String esid;
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");

  String epass = "";
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);

  WiFi.begin(esid.c_str(), epass.c_str());
  if (testWifi())
  {
    Serial.println("Succesfully Connected!!!");
    return;
  }
  else
  {
    Setnewnetwork();
  }

  Serial.println();
  Serial.println("Waiting.");
  Serial.println("version 3.2");
}

void Setnewnetwork()
{
  Serial.println("Turning the HotSpot On");
  hotspot_disable =0;
  launchWeb();
  setupAP();// Setup HotSpot
  while ((WiFi.status() != WL_CONNECTED))
  {
    delay(100);
    server.handleClient();
  }
}
void loop() {
  if ((WiFi.status() == WL_CONNECTED))
  {

//you can use button input to change the WIFI Network

 //>>>>>>>>>>>>>>>
  if(Serial.available()) {
  inputString= Serial.readString();
  Serial.println(inputString);
  if( inputString == "change" )
  {
    //<<<<<<<<<<<<<<
    //can add diff check to call this function i am getting input from serial console
    Setnewnetwork();
  }
 }
 repeatedCall();    
//>>>>>>>>>>>>>>>>>>>
// Add Your Code Here //
///////////////////////
//////////////////////
/////////////////////
//<<<<<<<<<<<<<<<<<<<<
 
  }
  else
  {       
        if(hotspot_disable)
        {
        Serial.println("wifi not connected");
        Serial.println("enter ""change \" ""to change the network");
        Setnewnetwork();
        }
  }
}


//-------- Fuctions used for WiFi credentials saving and connecting to it which you do not need to change 
bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void launchWeb()
{
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
  Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10); 
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);

    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP("DIY_sagar", "");
  Serial.println("softap");
  launchWeb();
  Serial.println("over");
}

void createWebServer()
{
 {
    server.on("/", []() {

      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
      content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      content += ipStr;
      content += "<p>";
      content += st;
      content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", content);
    });

    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 96; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");

        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        EEPROM.commit();

        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.reset();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);

    });
  } 
}
