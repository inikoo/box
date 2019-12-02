 /**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


// globale veriables 

ESP8266WiFiMulti WiFiMulti;

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
String new_ssid="";
String new_pass="";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;
void(* resetFunc) (void) = 0; 

void setup() {
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Helio","12341234");
 
  Serial.println("5645");
   
  //Serial.setDebugOutput(true);
    // scan("Helio","12341234"); 

  if ( WiFiMulti.run() == WL_CONNECTED) {
  
    Serial.println("3322");
 
  }
  else{
     Serial.println("6464");
    // WiFi.begin(ssid,password);
  }
  
  
}

void loop() {


    if(Serial.available())
    {

           

            int state = Serial.parseInt();

              if (state == 1982){
                    
              String tag_id;
              String url; 
              tag_id= Serial.readStringUntil('x');
              url=Serial.readStringUntil('#');
              Serial.println(tag_id);
              tag_id.replace(" ","");
              tag_id.replace("\n","\0");
              url.replace(" ","");
              tag_id.replace("\n","\0");
              
              tagSend(tag_id,url);
    
            } 


            if(state==5555){

               Serial.println("Reset Wifi Card");
               WiFi.disconnect(true);
               WiFi.setAutoConnect(false); 
               delay(300);
               ESP.eraseConfig();
               delay(300);
               
               ESP.reset();
               delay(300);
               
               ESP.restart();
              
            }

            

            if (state == 9999){
          
              new_ssid= Serial.readStringUntil('%');
              new_pass=Serial.readStringUntil('%');
              new_ssid.trim();
              new_pass.trim();
              scan(new_ssid,new_pass);
              delay(1000);
              if ((WiFiMulti.run() == WL_CONNECTED)){
                 Serial.println(1100);
                //Serial.println("connect to"+new_ssid);
                //Serial.println(send_cmd());
              }
            }

            if (state == 1984){
          
              String  cmd= Serial.readStringUntil('%');

               //Serial.println(cmd);
               cmd.trim();
              if ((WiFiMulti.run() == WL_CONNECTED)){
                
                Serial.println(send_cmd(cmd));
              }
            }


             if (state == 1983){
          
              
              if ((WiFiMulti.run() == WL_CONNECTED)){
                // if conected 
                Serial.println(2277);
              }
              else{

                 Serial.println(3388);
              }
            }


            if (state == 1986){
          
              String  cmd_ntp= Serial.readStringUntil('%');

               //Serial.println(cmd);
               cmd_ntp.trim();
              if ((WiFiMulti.run() == WL_CONNECTED)){
                
                Serial.println(ntp(cmd_ntp));
              }
              else{

                 Serial.println(3388);
              }
            }

    }


}

void tagSend(String tag_id ,String urlcmd){
 
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;
  
    Serial.println("Hello6" +tag_id +"hello");
    
   //String urlcmd="http://etronicsolutions.com/rfid/index.php?tag_id=";
     // String urlcmd="https://aw.aurora.systems/login.php";

    String cmd=urlcmd +tag_id;
    Serial.println(cmd);
    
    if (http.begin(client,cmd)) {  // HTTP
     // Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }

  delay(1000);
}

void pass_set(String ssid,String pass){

ssid=(String)ssid;
pass=(String)pass;

ssid.trim();
pass.trim();
#ifndef STASSID
#define STASSID ssid.c_str();
#define STAPSK   pass.c_str();
#endif
//
  const char* ssid_z   = STASSID;
  const char* password = STAPSK;
  //WiFi.printDiag(Serial);
  
  WiFi.begin(ssid_z,password);
 int p=0;

   while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    p++;
    if(p>150){
      Serial.println("5454");
    }
    else{
      Serial.println("4545");
    }

    if(p>160){

     resetFunc();
    }
    
  }

  
  WiFiMulti.addAP(ssid_z,password);
  Serial.println("2171");
  
 
}

String send_cmd(String url){

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();

    HTTPClient https;

    String rtn_str="e01";

    if (https.begin(*client, url)) {  // HTTPS
     // Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
       // Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          //Serial.println(payload);
          rtn_str=payload;
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  
  delay(1000);
 return rtn_str;

}

String ntp(String offset){
   timeClient.begin();
   delay(100);
  String time_stmp="error_ntp";
 int off_set=offset.toInt();
 timeClient.setTimeOffset(off_set);


 
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  //Serial.println(formattedDate);

  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  //Serial.print("DATE: ");
  //Serial.println(dayStamp);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  //Serial.print("HOUR: ");
  //Serial.println(timeStamp);

  time_stmp= dayStamp+"   "+timeStamp;

  return time_stmp; 
  
}


void scan(String ssid_new,String pass_new){

  WiFi.disconnect(true);
  WiFi.setAutoConnect(false);
  WiFi.setPhyMode(WIFI_PHY_MODE_11G);
  delay(300);
  Serial.println(7777);
  Serial.println("Scan start ... ");
  int n = WiFi.scanNetworks();
  Serial.print(n);
   
  for (int i = 0; i < n; i++)
  {
    Serial.println(WiFi.SSID(i));

    if(ssid_new==WiFi.SSID(i)){

     Serial.println("8888");
      pass_set(ssid_new,pass_new);
      
      
      break;
    }

    delay(2000);
  }
  Serial.println(1122);


 
}


 
 
