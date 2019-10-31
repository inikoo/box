 /**
Aw Firmware file date 26/9/19 
*/
#include <ArduinoJson.h>
#include <SPI.h>
#include <RFID.h>
#include <Ethernet.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SD.h>
#include "RTClib.h"
#include <EthernetClient.h>
#include <HttpClient.h>
#include "StringSplitter.h"



String mySerial="test0004";
bool fact_reset=false;


//config file ------------------------------

struct Config {
              char hostname[255];
              char boxname[255];
              int http_port;
              int https_port;
              char key_suffix[16];
              bool new_box;
              char wifi_ssid[255];
              char wifi_pass[255];
              bool  tcpip_conn;
              bool  wifi_conn;
              char box_serial[24];
              long offset;
              String key;
              char box_model[24];
          };

const char *filename = "config.txt";  // <- SD library uses 8.3 filenames
Config config;                         // <- global configuration object




//-------------Sd------------------------------------
const int chipSelect = 4;
//--------------------------------------
#define SS_PIN 53
#define RST_PIN 48
void(* resetFunc) (void) = 0;
RFID rfid(SS_PIN, RST_PIN); 
//-------------Lcd-----------------

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example




//-------------------Globale variable---------------------------
EthernetClient client;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

const int kNetworkTimeout = 30*1000;
const int kNetworkDelay = 1000;
int err =0;
int serNum0;
int serNum1;
int serNum2;
int serNum3;
int serNum4;
int buzz = 5; 
int btn = 2; 
int i =0;
String tagID;
RTC_DS1307 rtc;
bool wifi_conn=false;
bool tcp_conn=false;
bool off_line=true;
int wifi_rst_btn=7;
bool box_register=false;

//==========================================================
void setup()
{ 

  beep();
  
    fact_reset=digitalRead(btn);
  // Decalre config Hard code (not save in SD)

   config.key="jwUFBkWWbuilHBTEHxqWuScj80WIpVMH";
  
   strcpy(config.hostname,"box3.inikoo.com");
   
 

//---------------------------------------------------

   bool result=true;
   
   Serial.begin(115200);
   Serial2.begin(115200);
  
   //initial variables 
   tagID="";
   pinMode(buzz,OUTPUT);
    pinMode(buzz,INPUT);
   pinMode(wifi_rst_btn, OUTPUT);
   String boxName=(String)config.boxname;
   digitalWrite(wifi_rst_btn,HIGH);
   //------------------------------------------------------------------- Display-----
   
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    setup();
  }
 ///----------------------------------- Read Configeration-----------------------------

   SPI.begin(); 
   
   ReadConfig();
   SPI.end(); 
   delay(3000);
   do{

      conn();
    
   }while(off_line);
   
  Serial.println("Ready");



 if (check_box_states()){
        
          display.clearDisplay();
          display.setTextSize(1);             // Normal 1:1 pixel scale
          display.setTextColor(WHITE);        // Draw white text
          display.setCursor(0,0);             // Start at top-left corner
          display.println(F("Box Registerd"));
          display.setCursor(0,10); 
          display.println(config.boxname);
          display.setCursor(0,20); 
          display.println(config.box_serial);
          display.display();
          delay(2000);
          
    
}else{

     register_box();   
}
     
 
             
//-----------------------------------------------------------------------------------   
 
if(box_register){
 
 Serial.println("Start ");

 //================normal mode==================
 SPI.begin(); 
 rfid.init();
  
}
  


 
}


void loop()
{ 
  fact_reset=digitalRead(btn); 
  if(fact_reset){

    fac_reset();
    
  }
  i++;

  if(off_line){

   setup();
}

if(!box_register){

  setup();
   
 }

////================== Time======================= 

 mYrtc();
rfid_get();
//
 if(i>5000){

     display.clearDisplay();
        display.setTextSize(2);             // Normal 1:1 pixel scale
        display.setTextColor(WHITE);        // Draw white text
        display.setCursor(0,0);             // Start at top-left corner
        display.println(F("SYNC...."));
         
        display.display();
        delay(2000);
  sync();
 i=0;
}
 
}


void ReadConfig(){

   if (!SD.begin(chipSelect)) {
    Serial.println(F("Failed to initialize SD library"));
    delay(1000);
    display.clearDisplay();
    display.setTextSize(1);             
    display.setTextColor(WHITE);         
    display.setCursor(0,0);             
    display.println(F("Re-Insert Card..."));
    display.display();
    
  }

    Serial.println(F("Loading configuration..."));
   
    fac_reset();
    loadConfiguration(filename, config);
    
    display.clearDisplay();
    display.setTextSize(1);             
    display.setTextColor(WHITE);         
    display.setCursor(0,0);             
    display.println(F("Readding Config"));
  // Create configuration file
 
  // Dump config file
  Serial.println(F("Print config file..."));
  printFile(filename);
    for(int i=0;i<100;i++){
       display.setCursor(i,10);
       display.setTextSize(2); 
       display.println(F("."));
       display.display();
       delay(5);
    }

    
   
}



void oledTime(String tag){


   display.clearDisplay();            // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.setTextSize(1);
  display.println(config.boxname);
  display.setCursor(0,12); 
  display.setTextSize(2);
 display.println(tag);
 display.display();
  delay(100);
  
}

void dataLog(){
 

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");

      display.clearDisplay();

      display.setTextSize(1);             // Normal 1:1 pixel scale
      display.setTextColor(WHITE);        // Draw white text
      display.setCursor(0,0);             // Start at top-left corner
      display.println(F("SD Card Not Present"));
      display.display();
      delay(5000);
        
  }
  Serial.println("card initialized.");

  // make a string for assembling the data to log:
  String dataString = "";

  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 3; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 2) {
      dataString += ",";
    }
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("log.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  
}

void mYrtc(){
  rtc.begin();
    if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
      //rtc.adjust(DateTime(2019, 9, 18, 04, 33, 0));
  }
  //rtc.adjust(DateTime(2019, 9, 18, 16, 50, 0));

  DateTime time = rtc.now();

  
  
  String welcome=String(time.timestamp(DateTime::TIMESTAMP_TIME));
  oledTime(welcome); 
}


void loadConfiguration(const char *filename, Config &config) {
  // Open file for reading
  File file = SD.open(filename);

  StaticJsonDocument<512> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error){
    Serial.println(F("Failed to read file, using default configuration"));

        
    display.clearDisplay();
    display.setTextSize(1);             
    display.setTextColor(WHITE);         
    display.setCursor(0,0);             
    display.println(F("SD Card Fail"));
    display.display();   
    delay(500);
    saveConfiguration(filename, config,config.wifi_ssid,config.wifi_pass,config.boxname,config.offset,true,true);
    delay(5000);
    setup();

  }
  // Copy values from the JsonDocument to the Config
     config.http_port = doc["http_port"];
     config.offset=doc["offset"];
     config.tcpip_conn=doc["tcpip_conn"];
     config.wifi_conn=doc["wifi_conn"];
     
     strcpy(config.hostname,doc["hostname"]);
     strcpy(config.box_serial,doc["box_serial"]);
     strcpy(config.wifi_ssid,doc["wifi_ssid"]);
     strcpy(config.wifi_pass,doc["wifi_pass"]);
     strcpy(config.boxname,doc["boxname"]);
    
  file.close();
}

// Saves the configuration to a file
void saveConfiguration(const char *filename, const Config &config,String ssid,String pass,String boxName,long offset, bool tcpip_conn,bool wifi_conn) {

  SD.remove(filename);

  // Open file for writing
  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println(F("Failed to create file"));

     display.clearDisplay();
    display.setTextSize(1);             
    display.setTextColor(WHITE);         
    display.setCursor(0,0);             
    display.println(F("Failed to create file"));
    display.display();   
    delay(5000);
    setup();
    return;
  }

  StaticJsonDocument<256> doc;

  // Set the values in the document
  doc["hostname"] = (String)config.hostname;
  doc["boxname"]= boxName;
  doc["offset"]=offset;
  doc["box_serial"]=mySerial;
  doc["wifi_ssid"]=ssid;
  doc["wifi_pass"] =pass;
  doc["tcpip_conn"]=tcpip_conn;
  doc["wifi_conn"]=wifi_conn;
  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  file.close();
}

// Prints the content of a file to the Serial
void printFile(const char *filename) {
  // Open file for reading
  File file = SD.open(filename);
  if (!file) {
    Serial.println(F("Failed to read file"));
       display.clearDisplay();
    display.setTextSize(1);             
    display.setTextColor(WHITE);         
    display.setCursor(0,0);             
    display.println(F("Failed to read file"));
    display.display();   
    delay(5000);
    setup();
  }

  // Extract each characters by one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();

  // Close the file
  file.close();
}

void rfid_get(){

  if (rfid.isCard()) {
        if (rfid.readCardSerial()) {

          Serial.println(" ");
                Serial.println("Card found");
                serNum0 = rfid.serNum[0];
                serNum1 = rfid.serNum[1];
                serNum2 = rfid.serNum[2];
                serNum3 = rfid.serNum[3];
                serNum4 = rfid.serNum[4];
                String tagid= String(rfid.serNum[0],DEC)+String(rfid.serNum[1],DEC)+String(rfid.serNum[2],DEC)+String(rfid.serNum[3],DEC)+String(rfid.serNum[4],DEC);
              
                //Serial.println(" ");
                Serial.println("Cardnumber:"+tagid);
                 tag_beep();
                   display.clearDisplay();
                  display.setTextSize(1);             // Normal 1:1 pixel scale
                  display.setTextColor(WHITE);        // Draw white text
                  display.setCursor(0,0);             // Start at top-left corner
                  display.println(F("Card Detected"));
                  display.setCursor(0,10); 
                  display.println(F("Tag Id: "));
                  display.setCursor(0,20); 
                  display.println(tagid);
                  display.display();
                   DateTime time =rtc.now();
                   
                 // Send to Server
                 String cmd=get_server_responce("send_tag_id="+tagid+"&timestamp="+time.unixtime());
                 Serial.println(cmd);

                String state=to_json(cmd,"state");

                
                      display.clearDisplay();
                      display.setTextSize(1);             // Normal 1:1 pixel scale
                      display.setTextColor(WHITE);        // Draw white text
                      display.setCursor(0,0);             // Start at top-left corner
                      display.println(F("Tag ID: "));
                      display.setCursor(0,10); 
                      display.println(tagid);
                      display.setCursor(0,20); 
                      display.println(state);
                      display.display();
                      delay(5000);
                  
        }
  }

  
}

void tcp_ip_send(String tag){
   
 if (client.connect(config.hostname, 80)) 
  {
      Serial.println("connected");
       client.print("GET box.inikoo.com/");
      // client.print("Helio&AUTH_KEY=67655523.jwUFBkWWbuilHBTEHxqWuScj80WIpVMH");
       client.println();
         client.println();
       client.println("HTTP/1.0");
        client.println();

   // client.print("GET http://etronicsolutions.com/rfid/index.php?tag_id=");
   // client.print(tag);
   // client.println();
   // client.println("HTTP/1.0");
     
      delay(10);
  }
  
  else 
  {
    Serial.println("Tcp/IP connection failed");
     display.clearDisplay();
    display.setTextSize(1);             
    display.setTextColor(WHITE);         
    display.setCursor(0,0);             
    display.println(F("TCP/IP Fail"));
    display.display();   
    delay(500);
 
  
      
               
  }
  
    while (client.connected())
   {
              if (client.available()) {
              // char c = client.read();
              //Serial.print(c);

               String line = client.readString();
                 Serial.print(line);
               //char status[32] = {0};
              //client.readBytesUntil('\r', status, sizeof(status));
              // Serial.println(status);
              }
              if (!client.connected()) {
                Serial.println();
                Serial.println("disconnecting.");
                client.stop();
                 delay(100);
             
              }
              
              
   }
  
  
}

String  send_cmd(String val){
  String temp="";
 //const char kPath[] = "/api/?register=Helio&AUTH_KEY=test0001.jwUFBkWWbuilHBTEHxqWuScj80WIpVMH";

  const char* kPath=  val.c_str();

   
  EthernetClient c;
  HttpClient http(c);
  
  err = http.get(config.hostname, kPath);
  if (err == 0)
  {
    //Serial.println("startedRequest ok");

    err = http.responseStatusCode();
    if (err >= 0)
    {
       

      err = http.skipResponseHeaders();
      if (err >= 0)
      {
        int bodyLen = http.contentLength();
      
        // Now we've got to the body, so we can print it out
        unsigned long timeoutStart = millis();
        char c;
        // Whilst we haven't timed out & haven't reached the end of the body
        while ( (http.connected() || http.available()) &&
               ((millis() - timeoutStart) < kNetworkTimeout) )
        {
            if (http.available())
            {
                c = http.read();
                temp=temp+c;
                // Print out this character
                //Serial.print(c);
               
                bodyLen--;
                // We read something, reset the timeout counter
                timeoutStart = millis();
            }
            else
            {
                // We haven't got any data, so let's pause to allow some to
                // arrive
                delay(kNetworkDelay);
            }
        }
      }
      else
      {
        Serial.print("Failed to skip response headers: ");
        Serial.println(err);
      }
    }
    else
    {    
      Serial.print("Getting response failed: ");
      Serial.println(err);
    }
  }
  else
  {
    Serial.print("Connect failed: ");
    Serial.println(err);
    delay(5000);
    setup();
  }
  http.stop();

  // And just stop, now that we've tried a download

  StringSplitter *splitter = new StringSplitter(temp, '\n', 3);
 

  String temp2=splitter->getItemAtIndex(1);

   return temp2;
}


String to_json(String val,String par){

 //Serial.println(val);

  StringSplitter *splitter = new StringSplitter(val, '\n', 3);
 

  String sub=splitter->getItemAtIndex(1);
   
   DynamicJsonDocument doc(1024);
   deserializeJson(doc, val);
  JsonObject obj = doc.as<JsonObject>();

  String msg=obj[par];
 
  return msg;
}




void welcome(){


  
//welcome sscreen ============================================

    

                      display.clearDisplay();
                      display.setTextSize(1);             // Normal 1:1 pixel scale
                      display.setTextColor(WHITE);        // Draw white text
                      display.setCursor(0,0);             // Start at top-left corner
                      display.println(F("Please Insert the cable"));
                       display.setCursor(0,17); 
                      display.println(F("connecting"));
                      display.display();
                      delay(2000);
   
}

void register_box(){
  
 
 String state="Wating";
 String cmd2="";
 int i=0;
 delay(1000);
   
do{
                       

  
                           i++;

                         cmd2=get_server_responce("register=Helio");
                        if(cmd2.length()>5){

                         state=to_json(cmd2,"state");
                         state.trim();
                         Serial.println(i);
                         Serial.println(state);
                         
                         if(state=="Waiting"){
                           box_register=false;
                           i=0;
                          }
                          
                        }
                         

                          if(state=="Registered"){
                           box_register=true;
                            break;
                          }

                          
                                    
                          display.clearDisplay();
                          display.setTextSize(2);             // Normal 1:1 pixel scale
                          display.setTextColor(WHITE);        // Draw white text
                          display.setCursor(0,0);             // Start at top-left cor
                          display.println(config.box_serial);
                          display.display();
                          delay(2000);
                          Serial2.flush();
                          Serial2.flush();

                          if(i>4){
                          Serial.println(cmd2.length());
                            if(wifi_conn){
                                                           
                                 reset_wifi_card(); 
                            }
                           

                           saveConfiguration(filename, config,config.wifi_ssid,config.wifi_pass,config.boxname,config.offset,config.tcpip_conn,config.wifi_conn);
                           resetFunc();
                            
                          }

                        delay(10000);
                       

   }while(!(state=="Registered"));
if(box_register){

   reconfig_val(cmd2);
}

  
}
bool wifi_setup(){

   Serial2.flush();
   delay(200);
   Serial2.flush();
   delay(200);

  bool state=true;

            String cmd2="";
            String cmd3="conected";
            String cmd4="not_conected";
            bool cdk=1;
            String result="";

              String ssid=(String)config.wifi_ssid;
              ssid=ssid+"%";
              String pass=(String)config.wifi_pass;
               pass=pass+"%";

              String cmd="9999   "+ssid+"  "+pass;

  
               display.clearDisplay();
               display.setTextSize(1);             // Normal 1:1 pixel scale
               display.setTextColor(WHITE);        // Draw white text
               display.setCursor(0,0);             // Start at top-left corner
               display.println(F("Set up WiFI Hostpot"));
               display.setCursor(0,12); 
               display.print(F("SSID:"));
               display.print(config.wifi_ssid);
               display.setCursor(0,22); 
               display.print(F("Pwd:"));
               display.print(config.wifi_pass);
               display.display();
               delay(1000);

 
                  i=0;
                  cmd2=wifi_check(i);
                 
                 if(wifi_conn){

                      Serial.println("connected to wifi");
               
                      wifi_conn=true;
                      state=false;
                  
                      
                 }


                 if(!wifi_conn){

                 
                  delay(300);
                 
                     Serial2.println(cmd);
                     state=wifi_scan();
                     delay(3000);
                 }

                 if(cmd2==""){
                    wifi_setup();
                    //                  display.clearDisplay();
                    //                  display.setTextSize(1);             // Normal 1:1 pixel scale
                    //                  display.setTextColor(WHITE);        // Draw white text
                    //                  display.setCursor(0,0);             // Start at top-left corner
                    //                  display.println(F("Wifi Cad Not Detected"));
                    //                  display.display();
                    //                  delay(1000);
                  }

           delay(2000);
   


return state;
  
}


bool tcpip_setup(){

  bool state=true;
  String chack_ip="0.0.0.0";
  String ip="";

 
            display.clearDisplay();
            display.setTextSize(1);             // Normal 1:1 pixel scale
            display.setTextColor(WHITE);        // Draw white text
            display.setCursor(0,0);             // Start at top-left corner
            display.println(F("connecting..."));
            display.setCursor(0,10); 
            display.println(F("Network cable")); 
            display.display();


 if(Ethernet.begin(mac) == 0) {
             display.clearDisplay();
            display.setTextSize(1);             // Normal 1:1 pixel scale
            display.setTextColor(WHITE);        // Draw white text
            display.setCursor(0,0);             // Start at top-left corner
            display.println(F("Check cable"));
            display.setCursor(0,10); 
            display.println(F("tcp/ip fail")); 
            display.display();
            Serial.println("Failed to configure Ethernet using DHCP");
  }
  // print your local IP address:
  //Serial.println(Ethernet.localIP());
  ip=(String)Ethernet.localIP();
  ip.trim();
 // Serial.println(ip);
  
  if(0<ip.toInt()){

        display.clearDisplay();
        display.setTextSize(1);             // Normal 1:1 pixel scale
        display.setTextColor(WHITE);        // Draw white text
        display.setCursor(0,0);             // Start at top-left corner
        display.println(F("Coneected"));
        display.setCursor(0,20); 
        display.println(Ethernet.localIP()); 
        display.display();
        tcp_conn=true;
        state=false;                        
       
   
    
  }else{

            display.clearDisplay();
            display.setTextSize(1);             // Normal 1:1 pixel scale
            display.setTextColor(WHITE);        // Draw white text
            display.setCursor(0,0);             // Start at top-left corner
            display.println(F("Check cable"));
            display.setCursor(0,10); 
            display.println(F("tcp/ip fail")); 
            display.display();
            delay(1000);  
            state=true;
            tcp_conn=false;
           
    
  }



 return state;
  
}



bool wifi_scan(){
bool state=true;
 String result;
 Serial.println("No connection avilable");

 for( i=0;60>i;i++ ) {
           result = Serial2.readStringUntil('\n');
                         Serial.println(result);
                         result.trim();

                    
                        if(8888==result.toInt()){
         
              
                         
                                wifi_conn= serial_wait(4545);
                                
                                if (!wifi_conn){
                                 state=true;
                                 display.clearDisplay();
                                 display.setTextSize(1);             // Normal 1:1 pixel scale
                                 display.setTextColor(WHITE);        // Draw white text
                                 display.setCursor(0,0);             // Start at top-left corner
                                 display.println(F("Wrong wifi Setting"));
                                 display.setCursor(0,10);
                                 display.println(F("Switch off/On")); 
                              
                                 display.display();

                                 delay(5000);
                                 fac_reset();
                                  break; 
                                }else{

                                  state=false;
                                  break;
                                }
   
                              break;                           
                            }
                            else{
                                 display.clearDisplay();
                                 display.setTextSize(1);             // Normal 1:1 pixel scale
                                 display.setTextColor(WHITE);        // Draw white text
                                 display.setCursor(0,0);             // Start at top-left corner
                                 display.println(F("Scaning...."));
                                 display.setCursor(0,10); 
                                 display.println(result); 
                                 display.display();
                                

                          if(1122==result.toInt()){
                            
                                                                  
                                display.clearDisplay();
                                display.setTextSize(1);             // Normal 1:1 pixel scale
                                display.setTextColor(WHITE);        // Draw white text
                                display.setCursor(0,0);             // Start at top-left corner
                                display.println(F("No Wifi Netwrok found"));
                                display.setCursor(0,10); 
                                display.println(F("Setup Wifi.."));
                                display.display();
                                wifi_conn=false;
                                state=true;
                                break;

                          }

                       }
                          
                         
                      }

     return state;

 
}
bool serial_wait( int val){

  bool check=true;

  int i=0;
  int j=0;
String reply="";
  do{
    delay(100);
    i++;
      reply = Serial2.readStringUntil('\n');
      Serial.println(reply);
        reply.trim();

        if(reply.toInt()==1100){
          break;
          check=true;
        }

        if(reply.toInt()==2171){
          break;
          check=true;
        }

        if(reply.toInt()==5454){

             display.clearDisplay();
                 display.setTextSize(1);             // Normal 1:1 pixel scale
                 display.setTextColor(WHITE);        // Draw white text
                 display.setCursor(0,0);             // Start at top-left corner
                 display.println(F("Wrong wifi Setting"));
                 display.setCursor(0,10);
                 display.println(F("Switch off/On")); 
              
                 display.display();
          
                 delay(2000);
                check=false;
                fact_reset=true;
                fac_reset();

          break;
          check=false;
        }


         display.clearDisplay();
          display.setTextSize(1);             // Normal 1:1 pixel scale
          display.setTextColor(WHITE);        // Draw white text
          display.setCursor(0,0);             // Start at top-left corner
          display.println(F("Conecting To wifi"));
          display.setCursor(0,10); 
          display.println(config.wifi_ssid);
          display.display();
          delay(10);
         display.setCursor(i,20); 
         display.println(F("*"));
         display.display();
               if(i>100){
                i=0;
                j++;
               }

               if(j>2){
                 display.clearDisplay();
                 display.setTextSize(1);             // Normal 1:1 pixel scale
                 display.setTextColor(WHITE);        // Draw white text
                 display.setCursor(0,0);             // Start at top-left corner
                 display.println(F("Wrong wifi Setting"));
                 display.setCursor(0,10);
                 display.println(F("Switch off/On")); 
              
                 display.display();
          
                 delay(2000);
                check=false;
                fact_reset=true;
                fac_reset();
                break; 
               }
        
  }while(reply.toInt()!=1100);


  return check;
}
void  getTimeandDate(String offSet){

  if(wifi_conn)
  {

                               Serial2.println("1986 "+offSet+"%");
                          
                               String a="";
                                        for(i=0; i<100; i++) 
                                           {
                                                  while(Serial2.available()) {
                                                    a+=Serial2.readString();
                                                  
                                                 }
                                                  delay(10);
                                            }
                          
                                            Serial.println(a);
                          
                                  a.trim();
                                    rtc.begin();
                                      if (! rtc.isrunning()) {
                                      Serial.println("RTC is NOT running!");
                                       
                                    }
                                  
                                      StringSplitter *splitter = new StringSplitter(a, '  ', 2);
                                   
                                    String sub1=splitter->getItemAtIndex(0);
                                    String sub2=splitter->getItemAtIndex(1);
                                  
                                     
                                  
                                   // Serial.println(sub1);
                                   // Serial.println(sub2);
                                  
                                  
                                    StringSplitter *dateStr = new StringSplitter(sub1, '-', 3);
                                  
                                    String yyyy=dateStr->getItemAtIndex(0);
                                    String mm=dateStr->getItemAtIndex(1);
                            String dd=dateStr->getItemAtIndex(2);
                            //Serial.println(yyyy);
                            //Serial.println(mm);
                           // Serial.println(dd);
                          
                          
                             StringSplitter *timeStr = new StringSplitter(sub2, ':', 3);
                              String h=timeStr->getItemAtIndex(0);
                              String m=timeStr->getItemAtIndex(1);
                              String s=timeStr->getItemAtIndex(2);
                          
                           // Serial.println(h);
                          //  Serial.println(m);
                           // Serial.println(s);
                            int iyy=yyyy.toInt();
                            int imm=mm.toInt();
                            int idd=dd.toInt();
                          
                            int ih=h.toInt();
                            int im=m.toInt();
                            int is=s.toInt();
                            
                            rtc.adjust(DateTime(iyy, imm, idd, ih, im,is));

  }else if(tcp_conn){

                          display.clearDisplay();
                          display.setTextSize(1);             // Normal 1:1 pixel scale
                          display.setTextColor(WHITE);        // Draw white text
                          display.setCursor(0,0);             // Start at top-left corner
                          display.println(F("Connect Wifi to time Sync"));
                          display.setCursor(0,10); 
                          display.println(config.wifi_ssid);
                          display.display(); 
                          delay(3000);

    
  }else{
           off_line=true;
       }
  

}


bool conn()
{
     if(config.tcpip_conn){  
      tcp_conn = !tcpip_setup();
     }

     else {
       tcp_conn=false;
     }

     if(config.wifi_conn){

       if(tcp_conn){

       wifi_conn=!wifi_setup();
        
      }else{

          do{

            wifi_conn=!wifi_setup();

            
          }while(! wifi_conn);

       }


      
     }

     config.wifi_conn=wifi_conn;
      config.tcpip_conn=tcp_conn;
      saveConfiguration(filename, config,config.wifi_ssid,config.wifi_pass,config.boxname,config.offset,tcp_conn,wifi_conn);
     
 if(tcp_conn || wifi_conn ){
         Serial.println("On Line");
         display.clearDisplay();
        display.setTextSize(2);             // Normal 1:1 pixel scale
        display.setTextColor(WHITE);        // Draw white text
        display.setCursor(0,0);             // Start at top-left corner
        display.println(F("On Line"));
         
        display.display();
        delay(2000);
        off_line=false;
        return false;
      }else{
         Serial.println("Off line");
         config.wifi_conn=true;
      config.tcpip_conn=true;
        off_line=true;
        return true;
      }

      
}

void reconfig_val(String cmd2)
{

 Serial.println("Reconfig....");
 String timeOffset=to_json(cmd2,"time_offset" );
 String myName=to_json(cmd2,"name");
 String wifi_ssid=to_json(cmd2,"SSID");
 String wifi_pass=to_json(cmd2,"wifi_pwd");
 



if(myName.indexOf("@") > 0){
 strcpy(config.wifi_ssid,wifi_ssid.c_str());
 strcpy(config.wifi_pass,wifi_ssid.c_str());
 strcpy(config.boxname,myName.c_str());
 

    saveConfiguration(filename, config,wifi_ssid,wifi_pass,myName,timeOffset.toInt(),config.tcpip_conn,true);

 if(timeOffset.toInt()>0){

   getTimeandDate(timeOffset);
}

 Serial.println("Reconfig Ssucess...");
  
 }else{

  if(wifi_conn){
     reset_wifi_card();
     delay(5000);
      resetFunc();
  }
 }



 
                        
//                        if(((String)config.offset).toInt()!=timeOffset.toInt())
//                         {
//                            config.offset=timeOffset.toInt();
//                            getTimeandDate(timeOffset);
//                               
//                                  display.clearDisplay();
//                                  display.setTextSize(1);             // Normal 1:1 pixel scale
//                                  display.setTextColor(WHITE);        // Draw white text
//                                  display.setCursor(0,0);             // Start at top-left corner
//                                  display.println(F("Time Zone offset:"));
//                                  display.setCursor(0,10); 
//                                  display.println(config.offset);
//                                  display.display();
//                                  delay(3000);
//                         }

//
//                    if((String)config.wifi_ssid !=wifi_ssid || (String)config.wifi_pass !=wifi_pass )
//                     {
//                              saveConfiguration(filename, config,wifi_ssid,wifi_pass,myName,timeOffset.toInt());
//                              display.clearDisplay();
//                              display.setTextSize(1);             // Normal 1:1 pixel scale
//                              display.setTextColor(WHITE);        // Draw white text
//                              display.setCursor(0,0);             // Start at top-left corner
//                              display.println(F("Wifi Reset"));
//                              display.setCursor(0,10); 
//                              display.println(config.wifi_ssid);
//                              display.display();
//                              delay(3000);
//                    
//                             //  setup();
//                             
//                     
//                     
//                     }
                            

}
bool check_box_states(){

  if(off_line){

    setup();
  }

  
 display.clearDisplay();

                       display.setTextSize(1);             // Normal 1:1 pixel scale
                     display.setTextColor(WHITE);        // Draw white text
                         display.setCursor(0,0);             // Start at top-left corner
                         display.println(F("Waiting For"));
                         display.setCursor(0,10); 
                         display.println(F("Server Responce.."));
                         display.display();

                
  
   String cmd2=get_server_responce("register=Helio");
   Serial.println(cmd2);
   if(cmd2.length()>5){
   String state=to_json(cmd2,"state" );
  state.trim();
 
   if(state=="Registered"){
                             
                             box_register=true;
                              
                             reconfig_val(cmd2);
                             return true;
                             
                           }else if(state=="Waiting"){
                                         
                                         return false;
                                      }
                                        else{
                                               check_box_states();
                                                                                
                                        }   

    
   }else{
       
       check_box_states();
   }
                              
  
}

void sync()
{ 
   Serial.println("sync....");

   String cmd=get_server_responce("register=Helio");
   Serial.println(cmd);
   reconfig_val(cmd);
}


String get_server_responce(String action){


  
   String box_serial=(String)config.box_serial;
   
   String cmd="";
   String wifi_cmd="1984  https://"+(String)config.hostname+"/api/?"+action+"&AUTH_KEY="+box_serial+"."+(String)config.key+"%";
   String tcp_cmd="/api/?"+action+"&AUTH_KEY="+box_serial+"."+(String)config.key;
   String state="";
   Serial.println(wifi_cmd);
   if(tcp_conn){
                            
                       cmd=send_cmd(tcp_cmd);

                       cmd.trim();     
                            

                 
                  }else if(wifi_conn ){

                        while( Serial2.available() > 0 ) {
                           Serial.println(Serial2.read());
                        
                              }
                                
                                Serial.println("conect Sever -wifi- mode");
                                
                                  delay(100);
                                  Serial2.println(wifi_cmd);
        
                            for(i=0;i<20;i++){  
                               cmd= Serial2.readStringUntil('\n');
                                 cmd.trim();
                                   //Serial.println(cmd);
                                    if (cmd.length()>5){
                  
                                       break;
                                  }
                                 delay(10);
                                }
                         
                        

                  }else{

                     setup();
                  }
 cmd.trim(); 
 return cmd;

}

void fac_reset(){
   if(fact_reset){
     Serial2.println(5555);

     delay(1000);
      saveConfiguration(filename, config,"Helio","12341234", "Helio",0,true,true);

          delay(1000);

                               
       display.clearDisplay();
       display.setTextSize(1);             // Normal 1:1 pixel scale
       display.setTextColor(WHITE);        // Draw white text
       display.setCursor(0,0);             // Start at top-left corner
       display.println(F("Factroy Reset"));
       display.setCursor(0,10);
       display.println(F("Switch off/On")); 
    
       display.display();

       delay(5000);

       

      fact_reset=false;

      resetFunc();

                                 

    }
}

String wifi_check(int val){

    val++;  
    if(val>3){
    saveConfiguration(filename, config,config.wifi_ssid,config.wifi_pass,config.boxname,config.offset,true,false);
    reset_wifi_card();
    setup();
      
    }           
                     while( Serial2.available() > 0 ) {
                         Serial.println(Serial2.read());
                        
                      }

                  Serial2.flush();
                    delay(5000);
   
  String cmd2="";
                
               Serial2.println("1983");
                Serial.println("wifi check"); 

                
              cmd2 = Serial2.readStringUntil('\n');
                Serial.println(cmd2);         
                   

                        
                 if(2277==cmd2.toInt()){

               
                      wifi_conn=true;
                      Serial.println("wifi conected");
                      
                 }else if(3388==cmd2.toInt()){

                      wifi_conn=false;
                      Serial.println("wifi fail");

                      
                  
                 }else{
                       
                     while( Serial2.available() > 0 ) {
                         Serial.println(Serial2.read());
                        
                      }

                  Serial2.flush();
                    delay(5000);
                      
                    wifi_check(val); 

                    
                 }

                  delay(100);

                  


   return cmd2;
}


void reset_wifi_card(){

   digitalWrite(wifi_rst_btn,LOW);
                                   delay(300);
                                   digitalWrite(wifi_rst_btn,HIGH);
                                   delay(10000);

  
}

void beep(){

   
   tone(buzz,1000,1350);
   delay(100);
   tone(buzz,1000,1450);
   delay(100);



 
  
}

void tag_beep(){

  tone(buzz,500,850);

   
}
 
