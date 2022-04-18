#include <ESPFlashCounter.h>
#include <ESPFlash.h>
#include <ESPFlashString.h>

#include <sdios.h>
#include <MinimumSerial.h>
#include <BufferedPrint.h>
#include <FreeStack.h>

/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/telegram-control-esp32-esp8266-nodemcu-outputs/
  
  Project created using Brian Lough's Universal Telegram Bot Library: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
  Example based on the Universal Arduino Telegram Bot Library: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/blob/master/examples/ESP8266/FlashLED/FlashLED.ino
*/

#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "Deco-Gabi";
const char* password = "poliana90";

String StrLocal;
String StrAcender;
String StrApagar;
String StrEstado;

// Initialize Telegram BOT
#define BOTtoken "1979733101:AAEz583XmK_jXO5vozCRcYVcbOoy1Wlm7VE"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID_DANILO "1337388095"
#define CHAT_ID_GABRIELE "1163829049"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "password";
const char* PARAM_INPUT_3 = "bottoken";
const char* PARAM_INPUT_4 = "chatid1";
const char* PARAM_INPUT_5 = "chatid2";
const char* PARAM_INPUT_6 = "local";
const char* PARAM_INPUT_7 = "command";

//<meta name="viewport" content="width=device-width, initial-scale=1">
// HTML web page to handle 3 input fields (input1, input2, input3)
/*const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
  <html>
  <head>
  <title>TELEGRAM WebServer</title>  
  </head>
  <body>
  <h1>TELEGRAM WebServer Configuration</h1>
<form action="/action_page.php">
  <label for="ssid">SSID: </label>
  <input type="text" id="ssid" name="ssid"><br><br>
  <label for="password">Password: </label>
  <input type="text" id="password" name="password"><br><br>
  <label for="bottoken">BotToken: </label>
  <input type="text" id="bottoken" name="bottoken"><br><br>
  <label for="chatid1">CHAT ID1: </label>
  <input type="text" id="chatid1" name="chatid1"><br><br>
  <label for="chatid2">CHAT ID2: </label>
  <input type="text" id="chatid2" name="chatid2"><br><br> 
  <label for="local">LOCAL:</label>
  <input type="text" id="local" name="local"><br><br>   
  <br><br>
  <input type="submit" value="Submit">
</form>  
</body></html>)rawliteral";*/

WiFiClientSecure client;
UniversalTelegramBot *bot;
AsyncWebServer server(80);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan = 0;

const int ledPin = 2;
bool ledState = LOW;

const int relayPin = 5;
bool relayState = LOW;

//String strLightState;
String strSSID;
String strPASSW;
String strBOTTOKEN;
String strCHATID1;
String strCHATID2;
int last_message_received;
#define CONFIG_SSID                  "Deco-Gabi"
#define CONFIG_SSID_FILEPATH         "/config_ssid"
#define CONFIG_PASSW                 "poliana90"
#define CONFIG_PASSW_FILEPATH        "config_passw"
#define CONFIG_BOTTOKEN              "1979733101:AAEz583XmK_jXO5vozCRcYVcbOoy1Wlm7VE"
#define CONFIG_BOTTOKEN_FILEPATH     "/config_bottoken"
#define CONFIG_CHATID1              "1337388095"
#define CONFIG_CHATID1_FILEPATH     "/config_chatid1"
#define CONFIG_CHATID2              "1163829049"
#define CONFIG_CHATID2_FILEPATH     "/config_chatid2"

#define CONFIG_ACENDER_FILEPATH     "/acender"
#define CONFIG_APAGAR_FILEPATH      "/apagar"
#define CONFIG_ESTADO_FILEPATH      "/estado"
#define CONFIG_LOCAL_FILEPATH       "/local"

ESPFlashString ConfigSSID(CONFIG_SSID_FILEPATH, CONFIG_SSID);
ESPFlashString ConfigPASSW(CONFIG_PASSW_FILEPATH, CONFIG_PASSW);
ESPFlashString ConfigBOTTOKEN(CONFIG_BOTTOKEN_FILEPATH, CONFIG_BOTTOKEN);
ESPFlashString ConfigCHATID1(CONFIG_CHATID1_FILEPATH, CONFIG_CHATID1);
ESPFlashString ConfigCHATID2(CONFIG_CHATID2_FILEPATH, CONFIG_CHATID2);

ESPFlashString ConfigACENDER(CONFIG_ACENDER_FILEPATH, "");
ESPFlashString ConfigAPAGAR(CONFIG_APAGAR_FILEPATH, "");
ESPFlashString ConfigESTADO(CONFIG_ESTADO_FILEPATH, "");
ESPFlashString ConfigLOCAL(CONFIG_LOCAL_FILEPATH, "");

bool request_serial_output = false;

void(* resetFunc) (void) = 0;  // declare reset fuction at address 0

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot->messages[i].chat_id);
    if ((chat_id != strCHATID1) && (chat_id != strCHATID2)){
      bot->sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot->messages[i].text;
    Serial.print("Text:");
    Serial.println(text);
  
    String from_name = bot->messages[i].from_name;

    if (text == StrAcender) {
      String welcome = "Ola, " + from_name + ".\n";
      welcome += "A luz da "+ StrLocal +" será acesa!\n\n";
      bot->sendMessage(chat_id, welcome, "");
      relayState = HIGH;
      digitalWrite(relayPin, relayState);     
      ConfigESTADO.set(StrAcender);   

      int numNewMessages = bot->getUpdates(bot->last_message_received+1);            
    }    
    if (text == StrApagar) {
      String welcome = "Ola, " + from_name + ".\n";
      welcome += "A luz da "+StrLocal+" será apagada!\n\n";
      bot->sendMessage(chat_id, welcome, "");
      relayState = LOW;
      digitalWrite(relayPin, relayState);      
      ConfigESTADO.set(StrApagar);
      int numNewMessages = bot->getUpdates(bot->last_message_received+1);                  
    }

    StrEstado = ConfigESTADO.get();
    Serial.print("StrEstado:");
    Serial.println(StrEstado);    
  }
}


char cWebpage[1024];
char cCommand[1024];
char cHtml[256];
char cArray[64];
void setup() {
  Serial.begin(115200);

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, relayState);
  
  strSSID =  ConfigSSID.get();
  Serial.println("SSID:"+ strSSID);      
  strPASSW = ConfigPASSW.get();
  Serial.println("PASSWORD:"+ strPASSW);      
  strBOTTOKEN = ConfigBOTTOKEN.get();  
  Serial.println("BOT TOKEN:"+ strBOTTOKEN);      
  strCHATID1 = ConfigCHATID1.get();
  Serial.println("CHATID1:"+ strCHATID1);      
  strCHATID2 = ConfigCHATID2.get();
  Serial.println("CHATID2:"+ strCHATID2);  

  StrAcender = ConfigACENDER.get();
  StrApagar = ConfigAPAGAR.get();
  
        
  StrEstado = ConfigESTADO.get();
  Serial.println("Estado:"+ StrEstado);    

  StrLocal = ConfigLOCAL.get();
  Serial.println("LOCAL:"+ StrLocal);    
  
  

  bot = new UniversalTelegramBot(strBOTTOKEN, client);
    
  if (StrEstado == StrAcender) {
    relayState = HIGH;
    digitalWrite(relayPin, relayState);     
    ConfigESTADO.set(StrAcender);   
  }

  if (StrEstado == StrApagar) {
    relayState = LOW;
    digitalWrite(relayPin, relayState);      
    ConfigESTADO.set(StrApagar);
  }


strcpy(cWebpage,"<!DOCTYPE html>\r\n \
<html>\r\n \
<head>\r\n  \
<title>TELEGRAM WebServer</title>\r\n  \  
</head>\r\n  \
<body>\r\n  \
<h1>TELEGRAM WebServer Configuration</h1>\r\n  \
<form action=\"/action_page.php\">\r\n\
  <label for=\"ssid\">SSID: </label>\r\n");
  memset(cHtml,0,sizeof(cHtml));  
  sprintf(cHtml,"<input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"%s\"><br><br>\n",strSSID);
  strcat(cWebpage,cHtml);   
  
  strcat(cWebpage,"<label for=\"password\">Password: </label>");
  memset(cHtml,0,sizeof(cHtml));  
  sprintf(cHtml,"<input type=\"text\" id=\"password\" name=\"password\" value=\"%s\"><br><br>\n  ",strPASSW);
  strcat(cWebpage,cHtml);   

  strBOTTOKEN.toCharArray(cArray,sizeof(cArray));
  strcat(cWebpage,"<label for=\"bottoken\">BotToken: </label>");  
  memset(cHtml,0,sizeof(cHtml));  
  sprintf(cHtml,"<input type=\"text\" id=\"bottoken\" name=\"bottoken\" value=\"%s\" size=\"64\"><br><br>\n",cArray);
  strcat(cWebpage,cHtml);   

  
  strcat(cWebpage,"<label for=\"chatid1\">CHAT ID1: </label>");
  memset(cHtml,0,sizeof(cHtml));    
  sprintf(cHtml,"<input type=\"text\" id=\"chatid1\" name=\"chatid1\" value=\"%s\"><br><br>\n",strCHATID1);
  strcat(cWebpage,cHtml);   


  strcat(cWebpage,"<label for=\"chatid2\">CHAT ID2: </label>");
  memset(cHtml,0,sizeof(cHtml));    
  sprintf(cHtml,"<input type=\"text\" id=\"chatid2\" name=\"chatid2\" value=\"%s\"><br><br>\n",strCHATID2);
  strcat(cWebpage,cHtml);   

  Serial.print("StrLocal:");
  Serial.println(StrLocal);
  
  memset(cArray,0,sizeof(cArray));
  StrLocal.toCharArray(cArray,sizeof(cArray));
  
  strcat(cWebpage,"<label for=\"local\">LOCAL: </label>");
  memset(cHtml,0,sizeof(cHtml));    
  sprintf(cHtml,"<input type=\"text\" id=\"local\" name=\"local\" value=\"%s\"><br><br>\n",cArray);
  strcat(cWebpage,cHtml);   

  strcat(cWebpage,"<br>\r\n\
  <input type=\"submit\" value=\"Submit\">\r\n\
</form>\r\n\  
</body></html>");
  
  Serial.print("cWebpage:");
  Serial.println(cWebpage);
  

strcpy(cCommand,"<!DOCTYPE html>\r\n \
<html>\r\n \
<head>\r\n  \
<title>TELEGRAM WebServer</title>\r\n  \  
</head>\r\n  \
<body>\r\n  \
<h1>TELEGRAM WebServer Command</h1>\r\n  \
<form action=\"/command.php\">\r\n\
  <label for=\"command\">COMMAND: </label>\r\n");
  memset(cHtml,0,sizeof(cHtml)); 
  memset(cArray,0,sizeof(cArray));
  StrEstado.toCharArray(cArray,sizeof(cArray));   
  sprintf(cHtml,"<input type=\"text\" id=\"command\" name=\"command\" value=\"%s\"><br><br>\n",cArray);
  strcat(cCommand,cHtml);   
  
  strcat(cCommand,"<br>\r\n\
  <input type=\"submit\" value=\"Submit\">\r\n\
</form>\r\n\  
</body></html>");



  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(/*ssid, password*/strSSID,strPASSW);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  
  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", cWebpage);
  });

// Send web page with input fields to client
  server.on("/command.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", cCommand);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/action_page.php", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    boolean boParam = false;
    // Send request to main loop
    request_serial_output = true;    
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      strSSID = request->getParam(PARAM_INPUT_1)->value();
      if(strSSID.length() > 0)
      {
        boParam = true;
        inputMessage = PARAM_INPUT_1;
        inputParam = strSSID;
        Serial.println(inputMessage);
        Serial.println(strSSID);
        /*request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                         + strSSID + ") with value: " + inputMessage +
                                         "<br><a href=\"/\">Return to Home Page</a>"); */
        ConfigSSID.set(strSSID);                                            
      }        
    }
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2)) {
      strPASSW = request->getParam(PARAM_INPUT_2)->value();
      if(strPASSW.length() > 0)
      {
        boParam = true;
        if(inputMessage.length() > 0)
        {
          inputMessage += String(",") + PARAM_INPUT_2;                        
        }
        else{
          inputMessage =PARAM_INPUT_2;                                   
        }

        if(inputParam.length() > 0)
        {
          inputParam += String(",") + strPASSW;                        
        }
        else{
          inputParam =strPASSW;                                   
        }

        Serial.println(inputMessage);
        Serial.println(strPASSW);
        /*request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                         + strPASSW + ") with value: " + inputMessage +
                                         "<br><a href=\"/\">Return to Home Page</a>");    */  
        ConfigPASSW.set(strPASSW);        
      }
    }
    // GET input3 value on <ESP_IP>/get?input3=<inputMessage>
    if (request->hasParam(PARAM_INPUT_3)) {
      strBOTTOKEN = request->getParam(PARAM_INPUT_3)->value();
      if(strBOTTOKEN.length() > 0)
      {
        boParam = true;
        if(inputMessage.length() > 0)
        {
          inputMessage += String(",") + PARAM_INPUT_3;                        
        }
        else{
          inputMessage =PARAM_INPUT_3;                                   
        }

        if(inputParam.length() > 0)
        {
          inputParam += String(",") + strBOTTOKEN;                        
        }
        else{
          inputParam =strBOTTOKEN;                                   
        }

        Serial.println(inputMessage);
        Serial.println(strBOTTOKEN);
        /*request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                         + strBOTTOKEN + ") with value: " + inputMessage +
                                         "<br><a href=\"/\">Return to Home Page</a>");   */         
        ConfigBOTTOKEN.set(strBOTTOKEN);                                               
      }
    }
    // GET input4 value on <ESP_IP>/get?input4=<inputMessage>
    if (request->hasParam(PARAM_INPUT_4)) {
      strCHATID1 = request->getParam(PARAM_INPUT_4)->value();
      if(strCHATID1.length() > 0)
      {
        boParam = true;
        if(inputMessage.length() > 0)
        {
          inputMessage += String(",") + PARAM_INPUT_4;                        
        }
        else{
          inputMessage =PARAM_INPUT_4;                                   
        }

        if(inputParam.length() > 0)
        {
          inputParam += String(",") + strCHATID1;                        
        }
        else{
          inputParam =strCHATID1;                                   
        }
        Serial.println(inputMessage);
        Serial.println(strCHATID1);
        /*request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                         + strCHATID1 + ") with value: " + inputMessage +
                                         "<br><a href=\"/\">Return to Home Page</a>");    */        
        ConfigCHATID1.set(strCHATID1);                                             
      }
            
    }
    // GET input5 value on <ESP_IP>/get?input5=<inputMessage>
    if (request->hasParam(PARAM_INPUT_5)) {
      strCHATID2 = request->getParam(PARAM_INPUT_5)->value();
      if(strCHATID2.length() > 0)
      {
        boParam = true;

        if(inputMessage.length() > 0)
        {
          inputMessage += String(",") + PARAM_INPUT_5;                        
        }
        else{
          inputMessage =PARAM_INPUT_5;                                   
        }        
        if(inputParam.length() > 0)
        {
          inputParam += String(",") + strCHATID2;                        
        }
        else{
          inputParam =strCHATID2;                                   
        }
        Serial.println(inputMessage);
        Serial.println(strCHATID2);
        /*request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                         + strCHATID2 + ") with value: " + inputMessage +
                                         "<br><a href=\"/\">Return to Home Page</a>");*/            
        ConfigCHATID2.set(strCHATID2);                                               
      }      
    }  

    if (request->hasParam(PARAM_INPUT_6)) {
      StrLocal = request->getParam(PARAM_INPUT_6)->value();
      if(StrLocal.length() > 0)
      {
        boParam = true;

        if(inputMessage.length() > 0)
        {
          inputMessage += String(",") + PARAM_INPUT_6;                        
        }
        else{
          inputMessage =PARAM_INPUT_6;                                   
        }        

        if(inputParam.length() > 0)
        {
          inputParam += String(",") + StrLocal;                        
        }
        else{
          inputParam = StrLocal;                                   
        }

        Serial.println(inputMessage);
        Serial.println(StrLocal);
        /*request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                         + strSSID + ") with value: " + inputMessage +
                                         "<br><a href=\"/\">Return to Home Page</a>"); */
        /*ConfigSSID.set(cStrLocal);    */ 
       
        StrAcender = String("/acender")+ String("_") + StrLocal;
        StrApagar = String("/apagar")+ String("_") + StrLocal;
        Serial.println(StrAcender);
        Serial.println(StrApagar);


        ConfigLOCAL.set(StrLocal);
        ConfigACENDER.set(StrAcender);
        ConfigAPAGAR.set(StrApagar);


      }        
    }
    
      
    if (boParam == false)
    {
      inputMessage = "No message sent";
      inputParam = "none";
      Serial.println(inputMessage);
      Serial.println(inputParam);
      request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                       + inputParam + ") with value: " + inputMessage +
                                       "<br><a href=\"/\">Return to Home Page</a>");      
    }                                       
    else
    {
      Serial.println(inputMessage);
      Serial.println(inputParam );
      request->send(200, "text/html", "HTTP GET request sent to your ESP on input field ("+ 
                                        inputMessage +") with value: " +  inputParam +
                                       "<br><a href=\"/\">Return to Home Page</a>");  
      
    }
   

  if (boParam == false)
  {
    inputMessage = "No message sent";
    inputParam = "none";
    Serial.println(inputMessage);
    Serial.println(inputParam);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");      
  }                                       
  else
  {
    Serial.println(inputMessage);
    Serial.println(inputParam );
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field ("+ 
                                      inputMessage +") with value: " +  inputParam +
                                     "<br><a href=\"/\">Return to Home Page</a>");      
  }

});  


// Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/command.php", HTTP_GET, [] (AsyncWebServerRequest *request) 
  {
      String inputMessage;
      String inputParam;
      boolean boParam = false;
      /* Send request to main loop*/
      request_serial_output = true;    
      // GET input7 value on <ESP_IP>/get?input7=<inputMessage>
    if (request->hasParam(PARAM_INPUT_7)) 
    {
        StrEstado = request->getParam(PARAM_INPUT_7)->value();
        if(StrEstado.length() > 0)
        {
            boParam = true;
            inputMessage = PARAM_INPUT_7;
            inputParam = StrEstado;
            Serial.println(inputMessage);
            Serial.println(StrEstado);
            ConfigESTADO.set(StrEstado);                                            
        }        
    }
    
  if (boParam == false)
  {
    inputMessage = "No message sent";
    inputParam = "none";
    Serial.println(inputMessage);
    Serial.println(inputParam);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");      
  }                                       
  else
  {
    Serial.println(inputMessage);
    Serial.println(inputParam );
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field ("+ 
                                      inputMessage +") with value: " +  inputParam +
                                     "<br><a href=\"/\">Return to Home Page</a>");      
  }  
});       
 


  server.onNotFound(notFound);
  server.begin();  
}


void loop() {

  strcpy(cCommand,"<!DOCTYPE html>\r\n \
  <html>\r\n \
  <head>\r\n  \
  <title>TELEGRAM WebServer</title>\r\n  \  
  </head>\r\n  \
  <body>\r\n  \
  <h1>TELEGRAM WebServer Command</h1>\r\n  \
  <form action=\"/command.php\">\r\n\
    <label for=\"command\">COMMAND: </label>\r\n");
    memset(cHtml,0,sizeof(cHtml)); 
    memset(cArray,0,sizeof(cArray));
    StrEstado.toCharArray(cArray,sizeof(cArray));   
    sprintf(cHtml,"<input type=\"text\" id=\"command\" name=\"command\" value=\"%s\"><br><br>\n",cArray);
    strcat(cCommand,cHtml);   
    
    strcat(cCommand,"<br>\r\n\
    <input type=\"submit\" value=\"Submit\">\r\n\
  </form>\r\n\  
  </body></html>");

  if(request_serial_output) {
    request_serial_output = false; // Clear flag
    // Output serial
    Serial.println("Preparing to Reset");
    // Wait 1s. You can't do that in the webserver handler!
    delay(500);
    resetFunc();                                               
  }
  else
  {
    if (millis() > lastTimeBotRan + botRequestDelay)  
    {
      lastTimeBotRan = millis(); 
      last_message_received = bot->last_message_received;
      Serial.print("last_message_received:");
      Serial.println(bot->last_message_received);
      bot->last_message_received = 0;
      int numNewMessages = bot->getUpdates(bot->last_message_received);

      Serial.print("numNewMessages:");
      Serial.println(numNewMessages);
      
      if(numNewMessages > 0)
      {
        Serial.println("got response");
        handleNewMessages(numNewMessages);        
      }
    }    
  }
}
