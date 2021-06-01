//Declaracion de librerias
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include "EEPROM.h"
#include <LEDMatrixDriver.hpp>
#include <FirebaseESP32.h>

//Headers
#include "mainpage.h"
#include "font.h"
#include "mat_functions.h"
#include "Wifi.h"
#include <WebServer.h>

//Conexion a Firebase
#define URL  "esp32-c771a-default-rtdb.firebaseio.com"
#define secreto "bfiO0KAe3FFSGFVxsUD0hdtkyJSAdw36ylSlEHbn"


FirebaseData myFireBaseData;


//provide your own WiFi SSID and password
const char* ssid = "Totalplay-3F9F";
const char* password = "3F9F6A2FP8mmzqK9";


//Create WebServer instance
WebServer server(80);

//Initialize message to display
String message = "Hola";


void setup() {
  //This uses EEPROM to store previous message
  //Initialize EEPROM
  if (!EEPROM.begin(1000)) {
    Serial.println("Failed to initialise EEPROM");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }
  //Initialize the display
  lmd.setEnabled(true);
  lmd.setIntensity(2);   //Brilo de los leds 0 = Bajo, 10 = Alto
   //For debugging
  Serial.begin(9600);
  //Use ESP32 as WiFi Station
  WiFi.mode(WIFI_STA);
  //Initiate WiFi Connection
  WiFi.begin(ssid, password);
  Serial.println("");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("--");
  }

  Firebase.begin(URL, secreto);
  Firebase.reconnectWiFi(true);
  Serial.println("Conectado con exito");
  
  Serial.println("");
  Serial.print("Conectado a:");
  //Print your WiFi's SSID (might be insecure)
  Serial.println(ssid);
  Serial.print("Servidor: ");
  //Print your local IP address (needed for browsing the app)
  Serial.println(WiFi.localIP());
  //Home page. Contents of 'page' is in mainpage.h
  server.on("/", []() {
   server.send(200, "text/html", page);
  });
   //POST data handler
  server.on("/dataHandler",HTTP_POST,dataHandler);
  //start web server
  server.begin();
  //Just stating things
  Serial.println("*Servidor HTTP Inicado*");
  //At first start, read previous message from EEPROM
  message = EEPROM.readString(0);
  int len = message.length();
  //Print stored message to serial port for checking
  Serial.print("Historial de mensajes: ");
  Serial.println(message);
}

//function for handling POST data
void dataHandler(){
  String msg = server.arg("message");   //message from POST data
  Serial.print("Recibido: ");          //print to serial terminal
  Serial.println(msg);
  msg.toUpperCase();                  //all incoming string is converted to uppercase since no font for small cases
  message = msg;
  EEPROM.writeString(0,message);      //store received message to EEPROM
  EEPROM.commit();                    //commit the save
  server.sendHeader("Location","/");  //redirect client to home page
  server.send(303);                   //redirect http code
}

void loop() {
  
  server.handleClient();              //make the ESP32 respond to web clients
  int len = message.length();         //get message length
  if(len > 100) return;               //limit messages to 100 characters
  writeToMatrix(message,len);         //write to LED Matrix. This function is on mat_functions.h
   // Toggle display of the new framebuffer
  lmd.display();
  // Wait to let the human read the display
  delay(ANIM_DELAY);
  // Advance to next coordinate
  if( --x < len * -8 ) {
    x = LEDMATRIX_WIDTH;
  }
  delay(100);                         //adjust this for faster/slower scrolling

  Firebase.set(myFireBaseData, "message:" ,message);
  message = myFireBaseData.stringData();

  
}
