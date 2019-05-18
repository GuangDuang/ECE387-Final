
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

boolean locked= false; 
int message;
int inPin = D8;
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">
    <link rel=\"icon\" href=\"data:,\">
    <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
    body {background-color: powderblue;}
    .button { background-color: Tomato; border: none; color: white; width: 100px; height: 50;border-radius: 8px;
    text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}
    .button2 {background-color: Gray;}</style></head>
    
<body>
<h1>ECE 387 Group Project</h1>
<h1>Locker</h1>
<h1 style = color:GREEN>EMPTY</h1>
<h1>Set Pin to lock</h1>
 
<form action="/action_page">
<br>
  <input type="password" minlength="4" maxlength="4" size="4" name="pws">
  <br>
   <input type="submit" value="Submit">
</form> 
 
</body>
</html>
)=====";

const char MAIN_page1[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">
    <link rel=\"icon\" href=\"data:,\">
    <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
    body {background-color: powderblue;}
    .button { background-color: Tomato; border: none; color: white; width: 100px; height: 50;border-radius: 8px;
    text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}
    .button2 {background-color: Gray;}</style></head>
    
<body>
<h1>ECE 387 Group Project</h1>
<h1>Locker</h1>
<h1 style = color:RED>Locked</h1>
<h1>Enter Pin to unlock</h1>
 
<form action="/action_page">
  <br>
  <input type="password" minlength="4" maxlength="4" size="4"name="pws">
  <br>
  <input type="submit" value="Submit">
</form> 
 
</body>
</html>
)=====";
 

const char* ssid = "PORSCHE";
//"2.4-1707 Sassafras";
const char* password ="12345678";
//"CXNK0030C4DB";
 
ESP8266WebServer server(80); 

void handleRoot() {
  String s = "";
  if(digitalRead(inPin) > 0){
    s = MAIN_page1; //Read HTML contents
  }else{
    s = MAIN_page; //Read HTML contents
  }
  server.send(200, "text/html", s); //Send web page
 
}
//===============================================================
// This routine is executed when you press submit
//===============================================================
void handleForm() {
 String pws = server.arg("pws");
 message = pws.toInt();
 Serial.write((int)(message/10));
 Serial.write(message%10);
 String s = "";
 if(!locked){
   delay(1000);
   if(digitalRead(inPin) > 0){
      s = "<a href='/'> Locked! Go Back </a>";
   }else{
      s = "<a href='/'> Retry or Check ID</a>";
   } 
 }else{
   delay(1000);
   locked = digitalRead(inPin);
   if(digitalRead(inPin) > 0){
    s = "<a href='/'> Wrong Password Please try again </a>";
  }else{
    s = "<a href='/'> Unlocked Go Back </a>";
  }
  
 }
 server.send(200, "text/html", s); //Send web page
}
//==============================================================
//                  SETUP
//==============================================================
void setup(void){
  Serial.begin(9600);
  pinMode(inPin, INPUT);
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println("WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
 
  server.on("/", handleRoot);      //Which routine to handle at root location
  server.on("/action_page", handleForm); //form action is handled here
 
  server.begin();                  //Start server
  Serial.println("HTTP server started");
}
//==============================================================
//                     LOOP
//==============================================================
void loop(void){
  server.handleClient();          //Handle client requests
}
