/*
  LCD2004 + ESP8266 + Botón + TempSensor + Telegram
  Frank Ramirez
  Creation: 2023-03-16: V01
  Modification: 2024-05-11: V02
  Comments: 
  Solve Bugs, add D6 as input button, add pinmode(output 0 output 8)
  2024-05-11: WeMos D1 R2 & mini

-Telegram Arduino code downloaded from
https://microcontrollerslab.com/telegram-esp32-esp8266-nodemcu-control-gpios-leds/

-WebServer Arduino code downloaded from
_ I cannot find the place anymore, if you know the link please share it I will post it here to give
- the corresponding credit.
****************************************************************************************************************
private_data.h

#ifndef PRIVATE_DATA_h
#define PRIVATE_DATA_h
const char* _ssid     = "SSID";
const char* _password = "SSID-PASSWORD";
// Initialize Telegram BOT
#define _BOTtoken "1234567890:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"  // replace this with your bot token
#define _CHAT_ID "01234567"  //replace with your telegram user ID
#endif

****************************************************************************************************************
  
*/

#include <SPI.h>
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   
#include <ArduinoJson.h>

/*
 * Código de usuario
 * LC2004 make use of Wemo D1 Mini SPI {D5:SPI_Clk; D7:SPI_MOSI;D7; D1:RS; D2:RW; D3:Ena}
 * D0:Relay_ElectroValve
 * D4:Manual_Relay_WaterPump_ON/OFF
 * D6:Manual_Relay_REGAR_CESPED/PLANTAS
 * D8:Relay_WaterPump
 */
 
#include "LCD_2004.h"
#include "private_data.h"

/*
 * Definición Variables
 */
uint8_t rCursorPos = 0;
char sGenMsg[] = "********************";
char tempCbuffer[10] = "";
 
/*
 * Configuración Sensor Temp 
 */
#define ONE_WIRE_BUS 1 // 1-Wire bus data pin (Tx Pin)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
/*
 * Fin Configuración Sensor Temp 
 */

/*
 * Configuración WiFi
 * Replace with your network credentials (credentials stored in "private_data.h")
 */

const char* ssid     = _ssid; 
const char* password = _password;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output0State = "off";  //ELECTRO_VÁLVULA
String output8State = "off";  //BOMBA_AGUA
String sTempC = "00.00";

// Assign output variables to GPIO pins
const int output0 = 16;     //GPIO16  ->  DO  -> RelayEletroValve
const int output8 = 15;     //GPIO15  ->  D8  -> RelayWaterPump
const int input4 = 2;       //GPIO2   ->  D4  -> Manual_RelayWaterPump
const int input6 = 12;      //GPIO12  ->  D6  -> Manual_Relay_REGAR_CESPED/PLANTAS

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
/*
 * Fin Configuración WiFi
 */

/*
 * Confguración Telegram 
 * Replace with your Telegram credentials (credentials stored in "private_data.h")
 */
// Initialize Telegram BOT
#define BOTtoken _BOTtoken
#define CHAT_ID _CHAT_ID

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int bot_delay = 1000;
unsigned long lastTimeBotRan;
int numNewMessages = 0;

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("Handling New Message");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String user_text = bot.messages[i].text;
    Serial.println(user_text);

    String your_name = bot.messages[i].from_name;

    if (user_text == "/start") {
      String welcome = "Bienvenido, " + your_name + ".\n";
      welcome += "Use los siguientes comandos para controlar la bomba de agua.\n\n";
      welcome += "Envíe /ba_on para encender la bomba de agua GPIO8 ON \n";
      welcome += "Envíe /ba_off para apagar la bomba de agua GPIO8 OFF \n";
      welcome += "Envíe /obtener_ba_estado para saber el estado de la bomba de agua. Estado GPIO8\n";
      welcome += "Envíe /rp_on para regar el césped GPIO0 ON \n";
      welcome += "Envíe /rc_on para regar las plantas GPIO0 OFF \n";
      welcome += "Envíe /obtener_rprc_estado para saber qué está regando. Estado GPIO0\n";   
      welcome += "Envíe /obtener_temp para saber la temperatura.\n";     
      bot.sendMessage(chat_id, welcome, "");
    }

    if (user_text == "/ba_on") {
      bot.sendMessage(chat_id, "Bomba de agua encendida ON", "");
      //ledState = HIGH;
      digitalWrite(output8, HIGH);
      output8State="on";
    }
    
    if (user_text == "/ba_off") {
      bot.sendMessage(chat_id, "Bomba de agua apagada OFF", "");
      //ledState = LOW;
      digitalWrite(output8, LOW);
      output8State="off";
    }
    
    if (user_text == "/obtener_ba_estado") {
      if (digitalRead(output8)){
        bot.sendMessage(chat_id, "Bomba de agua encendida ON", "");
      }
      else{
        bot.sendMessage(chat_id, "Bomba de agua apagada OFF", "");
      }
    }

    if (user_text == "/rp_on") {
      bot.sendMessage(chat_id, "Selección Regar Césped", "");
      //ledState = HIGH;
      digitalWrite(output0, HIGH);
      output0State="on";
    }
    
    if (user_text == "/rc_on") {
      bot.sendMessage(chat_id, "Selección Regar Plantas", "");
      //ledState = LOW;
      digitalWrite(output0, LOW);
      output0State="off";
    }
    
    if (user_text == "/obtener_rprc_estado") {
      if (digitalRead(output0)){
        bot.sendMessage(chat_id, "Selección Regar Césped ON", "");
      }
      else{
        bot.sendMessage(chat_id, "Selección Regar Plantas ON", "");
      }
    }
    if (user_text == "/obtener_temp") {
      bot.sendMessage(chat_id, sTempC, "");
    }
  }
}

/*
 * Fin Confguración Telegram 
 */

// the setup function runs once when you press reset or power the board
void setup() {
  /* 
   * Initialize Telegram. 
   */
  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif   
  /* 
   * Fin Initialize Telegram. 
   */
   
  /* 
   * Initialize LCD_2004. 
   */
  //pinMode(LED_BUILTIN, OUTPUT);  
  pinMode(LCD_Data, OUTPUT);
  pinMode(LCD_Clk, OUTPUT);
  pinMode(LCD_Ena, OUTPUT);
  pinMode(LCD_RW, OUTPUT);
  pinMode(LCD_RS, OUTPUT);

  SPI.begin();
  SPI.setBitOrder(LSBFIRST);
  delay(10);

  digitalWrite(LCD_Ena,HIGH);  
  digitalWrite(LCD_RW,LOW);
  digitalWrite(LCD_RS,LOW);
  delay(10);

  LCD2004_Init();
  LCD2004_WelcomeScreen();

  delay(2000);  
  /*
   * FIN_LCD
   */

  /* 
   * Generic I/O 
   */
  pinMode(output0, OUTPUT);  //
  pinMode(output8, OUTPUT);  //
  pinMode(input4, INPUT);  //
  pinMode(input6, INPUT);  // 
  /* 
   * FIN Generic I/O 
   */


  /*
   * Wifi Conection 
   */
  strcpy(sGenMsg, "Conectando WiFi     ");   
  LCD2004_WriteMainLabel((uint8_t *)sGenMsg);
  delay(10);
  rCursorPos = 56; 
  LCD2004_SetCursor(rCursorPos);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (rCursorPos>58){
      LCD2004_WriteMainLabel((uint8_t *)sGenMsg);
      rCursorPos = 56; 
      LCD2004_SetCursor(rCursorPos);
    }else{
      LCD2004_WriteChar('.');  
      rCursorPos++;        
    }
  }
  //LCD2004_ClearScreen();

  //String(WiFi.localIP()[0]).toCharArray(sGenMsg,20);
  (String(WiFi.localIP()[0]) + String(".") +\
  String(WiFi.localIP()[1]) + String(".") +\
  String(WiFi.localIP()[2]) + String(".") +\
  String(WiFi.localIP()[3])).toCharArray(sGenMsg,20);

  //Write IP address in LCD2004
  LCD2004_WriteMainLabel((uint8_t *)sGenMsg);

  if (digitalRead(input4)!=HIGH){
    strcpy(sGenMsg, "ERROR: INTERR=ON   1");
    LCD2004_WriteSubLabel((uint8_t *)&sGenMsg);
    strcpy(sGenMsg, "CIERRE SW CONTINUAR ");
    LCD2004_WriteMessage((uint8_t *)&sGenMsg);
    while (digitalRead(input4)!=HIGH){
      delay(1000);
    }             
  }


  strcpy(sGenMsg, "BOMBA AGUA       OFF");
  LCD2004_WriteSubLabel((uint8_t *)&sGenMsg);
  digitalWrite(output8, LOW);

  strcpy(sGenMsg, "RIEGO PLANTAS     ON");
  LCD2004_WriteMessage((uint8_t *)&sGenMsg);
  digitalWrite(output0, LOW);

  server.begin();
/*
 * Setup Temp Sensor
 */
  // start serial port
  //Serial.begin(9600);

  // Start up the DallasTemperature library
  sensors.begin();

  // Set up the resolution for our sensors
  sensors.setResolution(12);
   
}

// the loop function runs over and over again forever
void loop() {
  /*
   * Telegram Check
   */

  if (millis() > lastTimeBotRan + bot_delay)  {
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      //Serial.println("Got Response!");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      //Serial.println("MERCY..");
    }
    lastTimeBotRan = millis();
    //Serial.println("FINISHIIM..");
  }
  /*
   * Fin Telegram Check
   */

  /*
   * Check botones
   */
  
  //WaterPump ON/OFF
  if (digitalRead(input4)!=HIGH) {    
    while (digitalRead(input4)!=HIGH){
        delay(50);
    }
    if (output8State == "off"){
      output8State = "on";
      strcpy(sGenMsg, "BOMBA AGUA        ON");
      LCD2004_WriteSubLabel((uint8_t *)&sGenMsg);
      digitalWrite(output8, HIGH);
    } 
    else if (output8State == "on"){
      output8State = "off";
      strcpy(sGenMsg, "BOMBA AGUA       OFF"); 
      LCD2004_WriteSubLabel((uint8_t *)&sGenMsg);             
      digitalWrite(output8, LOW);        
    }
  }

  //Regar Césped/Plantas
  if (digitalRead(input6)!=HIGH) {    
    while (digitalRead(input6)!=HIGH){
        delay(50);
    }
    if (output0State == "off"){
      output0State = "on";
      strcpy(sGenMsg, "RIEGO CESPED      ON");
      LCD2004_WriteSubLabel((uint8_t *)&sGenMsg);
      digitalWrite(output0, HIGH);
    } 
    else if (output0State == "on"){
      output0State = "off";
      strcpy(sGenMsg, "RIEGO PLANTAS     ON"); 
      LCD2004_WriteSubLabel((uint8_t *)&sGenMsg);             
      digitalWrite(output0, LOW);        
    }
  }

/*
 * Check Temp sensor
 */

  sensors.requestTemperatures(); // Send the command to start the temperature conversion

  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  float tempC = sensors.getTempCByIndex(0);
  sTempC = String(tempC,5).substring(0,5);
  
  // Check if reading was successful
  //if(tempC != DEVICE_DISCONNECTED_C) {
  //  Serial.println(tempC); // Print temperature of device 1 (index 0)
  //}else{
  //  Serial.println("Error: Could not read temperature data");
  // }
  
  //digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)  
  //delay(1000);                       // wait for a second

  //digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  //delay(1000); 

  /*digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)  
  delay(1000);                       // wait for a second

  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000); 
  */
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    //Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;

    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
       
        //Serial.write(c);                    // print it out the serial monitor
        
    
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /0/on") >= 0) {
              //Serial.println("GPIO 4 on");
              output0State = "on";
              strcpy(sGenMsg, "RIEGO CESPED      ON");
              LCD2004_WriteMessage((uint8_t *)&sGenMsg);
              digitalWrite(output0, HIGH);
              //digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)              
            } else if (header.indexOf("GET /0/off") >= 0) {
              //Serial.println("GPIO 4 off");
              output0State = "off";
              strcpy(sGenMsg, "RIEGO PLANTAS     ON");
              LCD2004_WriteMessage((uint8_t *)&sGenMsg);              
              digitalWrite(output0, LOW);
              //digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
            } else if (header.indexOf("GET /8/on") >= 0) {
              //Serial.println("GPIO 4 on");
              output8State = "on";
              strcpy(sGenMsg, "BOMBA AGUA        ON");
              //LCD2004_WriteMessage((uint8_t *)&sGenMsg);
              LCD2004_WriteSubLabel((uint8_t *)&sGenMsg);
              digitalWrite(output8, HIGH);
              //digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
            } else if (header.indexOf("GET /8/off") >= 0) {
              //Serial.println("GPIO 4 off");
              output8State = "off";
              //LCD2004_WriteMessage((uint8_t *)&sGenMsg);
              strcpy(sGenMsg, "BOMBA AGUA       OFF"); 
              LCD2004_WriteSubLabel((uint8_t *)&sGenMsg);             
              digitalWrite(output8, LOW);
              //digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
            }
             //Serial.println("GPIO 4 on");

            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta charset=\"utf-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #667386; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #79A892;}");
            client.println(".button3 {background-color: #E31B23;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server</h1>");
            client.println("<p>Temperatura ambiente:"+sTempC+"ºC</p>");  
            // Display current state, and ON/OFF buttons for GPIO 4  
            client.println("<p>GPIO 0 - ELECTRO-VÁLVULA - State " + output0State + "</p>");
            // If the output4State is off, it displays the ON button       
            if (output0State=="off") {
              //client.println("<p><a href=\"/0/on\"><button class=\"button\">ON</button></a></p>");
              client.println("<p><a href=\"/0/on\"><button class=\"button\">REGAR PLANTAS</button></a></p>");
            } else {
              //client.println("<p><a href=\"/0/off\"><button class=\"button button2\">OFF</button></a></p>");
              client.println("<p><a href=\"/0/off\"><button class=\"button button2\">REGAR CÉSPED</button></a></p>");
            }

            // Display current state, and ON/OFF buttons for GPIO 4  
            client.println("<p>GPIO 8 - BOMBA AGUA - State " + output8State + "</p>");
            // If the output4State is off, it displays the ON button       
            if (output8State=="off") {
              client.println("<p><a href=\"/8/on\"><button class=\"button\">BOMBA DE AGUA APAGADA</button></a></p>");
            } else {
              client.println("<p><a href=\"/8/off\"><button class=\"button button3\">BOMBA DE AGUA ENCENDIDA</button></a></p>");
            }
            
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    //Serial.println("Client disconnected.");
    //Serial.println("");
  }
 
}
