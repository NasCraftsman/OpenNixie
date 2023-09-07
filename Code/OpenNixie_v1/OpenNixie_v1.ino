//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  OPEN NIXIE FANTASY PROJECT
//
//  Modes: (press fast two times the center button to change the mode)
//  * Mode0-dice
//  * Mode1-cronometer 
//  * Mode2-universal clock 
//  * Mode3-work break counter
//
//  The circuit:
//  * This code is made to be used with the NixeFantasy Controller V2 and the NixieFantasy Square Base V2
//
//  The License: 
//  * Created in 2023 by nas.craftsman https://github.com/NasCraftsman
//  * This example code is in the public domain.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                       NixieFantasy Square Base V1
//
//    Nixie 1:N100
//    o o
//    o x
//    clk 1-10
//  
//    Nixie 2:N101
//    o o
//    x o
//    clk 11-20
//  
//    Nixie 3:N101
//    x o
//    o o
//    clk 21-30
//    Nixie 4:N100
//    o x
//    o o
//    clk 31-40
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include "time.h"

const char* ssid     = "prue";
const char* password = "ojetecalor";
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

#define SET       15      // General purpose button
#define SCK       2       // SPI sck pin
#define MOSI      4       // SPI MOSI pin
#define MISO      27       // SPI MISO pin
#define SS        34       // SPI MISO pin
#define SDA       18      // I2C SDA pin
#define SCL       19      // I2C SCL pin
#define R         22      // R LEDs pin
#define G         21      // G LEDs pin
#define B         23      // B LEDs pin
#define LE        25      // Nixies serial-parallel enable pin
#define ACC_INT   26      // Accelerometer interrupt pin 
#define EN_170    32      // 170V Enable pin 
#define EN_12     33      // 12V Enable pin 

#define LEDC_TIMER_13_BIT  13   // Use 13 bit precission for LEDC timer
#define LEDC_BASE_FREQ     5000 // Use 5000 Hz as a LEDC base frequency 
#define LEDC_CHANNEL_0     0    // Use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_1     1
#define LEDC_CHANNEL_2     2

//-----------------------AP PARAMETERS------------------------------------//
/* Put your SSID & Password */
const char* ssid_ap = "NIXIES";  // Enter SSID here
const char* password_ap = "12345678";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

//--------------------WEBSERVER PARAMETERS-------------------------------//
AsyncWebServer  server(80);

const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
String inputMessage1;
String inputMessage2;
String inputParam1;
String inputParam2;

//--------------------HTTP CLIENT PARAMETERS-----------------------------//
const char* serverName = "http://192.168.0.14:8000/";

//--------------------GLOBAL VARIABLES-----------------------------------//
String states[4]={"DICES MODE","CRONOMETER MODE","UNIVERSAL CLOCK MODE","TAKE A REST MODE"};
byte hour=0;
byte minute=0;
byte connection_counter=0;
byte one_time_2=0;
byte number_of_dices=2;
byte faces_of_dices=6;
int button_state=0;
int lastTriggerMillis=0;
int lastTriggerMillis_function=0;
int s=0;
int s10=0;
int m=0;
int m10=0;
int rest_period=120;
int clock_period=1;
int dice_values[4]={0,0,0,0};
bool dices_rolled=0;
bool parameters_received=0;
bool acc_flag=0;
bool button_flag=0; 
bool one_time=0;
bool button_released=0;
bool is_connected=false;
byte pinArray[40]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//---------------------------------------------------------------------------------------------------ANALOG CONTROL FOR LEDS------------------------------------------------------------------------------------------
// Arduino like analogWrite
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  uint32_t duty = (8191 / valueMax) * min(value, valueMax);  // calculate duty, 8191 from 2 ^ 13 - 1
  ledcWrite(channel, duty);  // write duty to LEDC
}
//---------------------------------------------------------------------------------------------------CONTROL LEDS------------------------------------------------------------------------------------------------------

void LEDS(int Re, int Gr, int Bl){
  ledcAnalogWrite(LEDC_CHANNEL_0, Re);//100
  ledcAnalogWrite(LEDC_CHANNEL_1, Gr);//100
  ledcAnalogWrite(LEDC_CHANNEL_2, Bl);//20
}
//---------------------------------------------------------------------------------------------------TURN ON NIXIES---------------------------------------------------------------------------------------------------

void turn_on(){
  digitalWrite(EN_12, HIGH);
  delay(10); 
  digitalWrite(EN_170, LOW);
  delay(10);
  digitalWrite(LE, HIGH);
}
//---------------------------------------------------------------------------------------------------TURN OFF NIXIES--------------------------------------------------------------------------------------------------
void turn_off(){
  digitalWrite(LE, LOW);
  digitalWrite(EN_170, HIGH);
  delay(10);
  digitalWrite(EN_12, LOW);
  delay(10); 
}
//---------------------------------------------------------------------------------------------------RESET NIXIES------------------------------------------------------------------------------------------------------
void reset(){
  int a_r=0;
  digitalWrite(EN_12, HIGH);
  delay(10);
  digitalWrite(MOSI, LOW);
  delay(1);
  while(a_r<40){ 
      digitalWrite(SCK,HIGH);
      delay(1);
      digitalWrite(SCK,LOW);
      delay(1);
      a_r++;
  }
  a_r=0;
  digitalWrite(EN_12, LOW);
  delay(10);
}
//---------------------------------------------------------------------------------------------------CREATE A FLUSH ANIMATION----------------------------------------------------------------------------------------------
void flus(int vel){
  turn_off();
  turn_on(); 
  digitalWrite(MOSI, HIGH);
  delay(1);
  for(int i=0;i<=40;i++){ 
      digitalWrite(SCK,HIGH);
      delay(vel);
      digitalWrite(SCK,LOW);
      delay(vel);
      digitalWrite(MOSI, LOW);
      delay(1); 
  }
  turn_off(); 
}
//---------------------------------------------------------------------------------------------------CONTROL ONE NIXIE------------------------------------------------------------------------------------------------------
void set_number( int n_nixie, int number ){
  if(number>9){
    number=9;
  }
  if(n_nixie>4){
    n_nixie=4;
  }
  if(n_nixie==1){
    number=number+0;
  }
  else if(n_nixie==2){
    number=number+10;
  }
  else if(n_nixie==3){
    number=number+20;
  }
  else{
    number=number+30;
  }
  turn_off();
  reset();
  digitalWrite(EN_12, HIGH);
  delay(10); 
  digitalWrite(MOSI, HIGH);
  delay(1);
  for(int i=0;i<=number;i++){ 
      digitalWrite(SCK,HIGH);
      delay(1);
      digitalWrite(SCK,LOW);
      delay(1);
      digitalWrite(MOSI, LOW);
      delay(1);
  }
  turn_on(); 
}
//---------------------------------------------------------------------------------------------------CONTROL 4 NIXIES------------------------------------------------------------------------------------------------------
void set_four_numbers(int n1, int n2, int n3, int n4){
  long long nixie1=1<<10-n1-1;
  long long nixie2=1<<10-n2-1;
  long long nixie3=1<<10-n3-1;
  long long nixie4=1<<10-n4-1;
  long long final_nixie=(nixie1<<30)+(nixie2<<20)+(nixie3<<10)+(nixie4<<0);
  byte pin=0;
  for(int i=0;i<=40;i++){
    pin=(final_nixie>>(i))&1;
    pinArray[40-i]=pin;
  } 
  turn_off();
  reset();
  digitalWrite(EN_12, HIGH);
  delay(10);
  digitalWrite(SCK,LOW);
  delay(1);
  digitalWrite(MOSI, LOW);
  delay(1);
  for(int i=0;i<40;i++){ 
        if(pinArray[40-i]){
          digitalWrite(MOSI, HIGH);
          delay(1);
        }
        digitalWrite(SCK,HIGH);
        delay(1);
        digitalWrite(SCK,LOW);
        delay(1);
        digitalWrite(MOSI, LOW);
        delay(1); 
    }
  turn_on(); 
}
//---------------------------------------------------------------------------------------------------PRINT TIME FROM SERVER------------------------------------------------------------------------------------------------------
byte  printLocalTime(){
      double m=0;
      double h=0; 
      byte h1=0;
      byte h2=0;
      byte m1=0;
      byte m2=0;
      
      struct tm timeinfo;
      
      while(!getLocalTime(&timeinfo)){
          delay(500);
          Serial.println("Failed to obtain time");
          connection_counter++;
            if(connection_counter>=5){
              Serial.println("Unable to obtain time");
              Serial.println("________________________________");
              return 0;
            }
      }
        
        hour = timeinfo.tm_hour;
        minute = timeinfo.tm_min;
        Serial.print("Hour:");
        Serial.println(hour);
        Serial.print("Minute:");
        Serial.println(minute);
        if(hour>9){
          h2=hour/10;
          h=hour;
          h=((h/10)-h2)*100;
          h1=h/10; 
        }
        else{
          h1=hour;
          h2=0;
        }
        if(minute>9){
          m2=minute/10;
          m=minute;
          m=((m/10)-m2)*100;
          m1=m/10;
        }
        else{
          m1=minute;
          m2=0;
        }
        set_four_numbers(m1,m2,h2,h1);
        Serial.println("________________________________");
        return 1;

}
//---------------------------------------------------------------------------------------------------DICE FUNCTION------------------------------------------------------------------------------------------------------
void dice(byte n_dices, byte f_dices){
  byte place=0;
  byte place_played[4]={5,5,5,5};
  byte played_flag=1;
  
  if(!digitalRead(SET)){
    button_released=0;
    if(!one_time){ 
      Serial.println("Sacking....");
      lastTriggerMillis_function=millis();
      one_time=1;
    }
    LEDS(100,100,20);
    flus(10);
  }
  else if(button_released){   
        if(one_time){
            Serial.println("Rolling dice...");
            for(int i=0;i<n_dices;i++){
                    while(played_flag==1){
                          place=random(0,4);
                          played_flag=0;
                          for(int n=0;n<n_dices;n++){
                                if(place_played[n]==place){
                                  played_flag=1;
                                }
                          }
                    }
                    played_flag=1;
                    place_played[i]=place;
                    dice_values[place]=random(1,f_dices+1);
                    Serial.print("Position of dice ");
                    Serial.print(i);
                    Serial.print(": ");
                    Serial.println(place_played[i]);
                    Serial.print("Value of dice ");
                    Serial.print(i);
                    Serial.print(": ");
                    Serial.println(dice_values[place]);
             }
            Serial.println("________________________________");
            set_four_numbers(dice_values[2],dice_values[0],dice_values[3],dice_values[1]);
            lastTriggerMillis_function=millis();
            dices_rolled=1;
        }
    one_time=0;
  }
  if(dices_rolled){
    for(int i=0;i<4;i++){
          dice_values[i]=0;
          place_played[i]=5;
      }
    if((millis()-lastTriggerMillis_function)>3000){
      flus(5);
      turn_off();
      dices_rolled=0;
    }
  }
}
//---------------------------------------------------------------------------------------------------CRONOMETER FUNCTION------------------------------------------------------------------------------------------------------
void cronometer(){
    if(!digitalRead(SET)){
        button_released=0;
        if(!one_time){
         
          Serial.println("Are you ready!?");
          lastTriggerMillis_function=millis();
          one_time=1;
        }
        if((millis()-lastTriggerMillis_function)>300){
          LEDS(100,100,20);
          flus(10);
        }
  }
  else if(button_released){
    if((millis()-lastTriggerMillis_function)>1000){
        if(one_time){
          lastTriggerMillis_function=millis();
          Serial.println("Go!");
          s=0;
          s10=0;
          m=0;
          m10=0;
          while(digitalRead(SET)&&!button_flag){
               LEDS(100,100,20);
               if((millis()-lastTriggerMillis_function)>=1000){
                  s++;
                  if(s>9){
                      s=0;
                      s10++;
                  }
                  if(s10>=6){
                      s10=0;
                      m++;
                  }
                  if(m>9){
                      m=0;
                      m10++;
                  }
                  if(m10>=6){
                      m10=0;
                  }
                  lastTriggerMillis_function=millis(); 
                  set_four_numbers(s,s10,m10,m);
                 }
          }
          LEDS(0,0,0);
        }
    }
    one_time=0;
  }
}
//---------------------------------------------------------------------------------------------------CLOCK FUNCTION-----------------------------------------------------------------------------------------------
void universalclock(int period, bool modes){
   if(one_time_2==0){
            Serial.println("Setting time");
            configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); 
            Serial.println("Printing time");
            printLocalTime();
            lastTriggerMillis_function=millis();
            one_time_2++;
            Serial.println("________________________________");
   }
   if(millis()-lastTriggerMillis_function>60000*period){
              Serial.println("Setting time");
              configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); 
              Serial.println("Printing time");
              printLocalTime();
              lastTriggerMillis_function=millis();
              one_time_2=1;
              Serial.println("________________________________");
   }
   if(!modes){
               if(millis()-lastTriggerMillis_function>2000){
                      if(one_time_2==1){
                        flus(5);
                        turn_off();
                        one_time_2++;
                      }
                }
   }
  else{
                if(millis()-lastTriggerMillis_function>2000){
                      if(one_time_2==1){
                          for(int i=0;i<5;i++){
                                flus(5);
                          }
                          turn_off();
                          one_time_2++;
                       }  
                }          
 }
}
//---------------------------------------------------------------------------------------------------ACCELEROMETER INTERRUPT--------------------------------------------------------------------------------------
void acc(){
 acc_flag=1;
}
//---------------------------------------------------------------------------------------------------BUTTON INTERRUT-----------------------------------------------------------------------------------------------
void but(){
    if((millis() - lastTriggerMillis)<200){
      button_state++;
      if(button_state>3){
        button_state=0;
      } 
      button_flag++;
  }
  button_released=1;
  lastTriggerMillis=millis();
}
// HTML web page to handle 3 input fields (input1, input2, input3)
//---------------------------------------------------------------------------------------------------WEB SERVER------------------------------------------------------------------------------------------------------
const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <title>ESP Input Form</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style> body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; text-align:center; }</style>
    </head>
    
    <body>
    <h3>Enter your WiFi credentials</h3>
    <form action="/get">
        USR: <input type="text" name="input1"><br>
        PASS: <input type="password" name="input2"><br><br>
        <input type="submit" value="Submit">
    </form><br>
    </body>
  </html>
)rawliteral";


//---------------------------------------------------------------------------------------------------404 ERROR------------------------------------------------------------------------------------------------------

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

//---------------------------------------------------------------------------------------------------WEB SERVER CONFIG------------------------------------------------------------------------------------------------------
void WEB_SERVER_config(){

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {

    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputParam1 = PARAM_INPUT_1;
       parameters_received++;
    }
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;
       parameters_received++;
    }
   
    else {
      inputMessage1 = "No message sent";
      inputParam1 = "none";
      inputMessage2 = "No message sent";
      inputParam2 = "none";
       parameters_received=0;
    }
    Serial.println("Parameters Received From Client:");
    Serial.print(inputParam1);
    Serial.print(":");
    Serial.println(inputMessage1);
    Serial.print(inputParam2);
    Serial.print(":");
    Serial.println(inputMessage2);
    Serial.println("________________________________");
    
    request->send(200, "text/html", "PARAMETERS SENT CORRECTLY"
                                     "<br><br><a href=\"/\">Return to Home Page</a>");
  });

  server.onNotFound(notFound);
  server.begin();
  Serial.println("HTTP server started");
  
}

//---------------------------------------------------------------------------------------------------MANAGE INFO FROM SERVER------------------------------------------------------------------------------------------------
void manage_info(){
 if( parameters_received!=0){
    WiFi.softAPdisconnect();
    Serial.println("Disconnecting AP");
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to:");
    Serial.println(inputMessage1.c_str());
    Serial.print("With PASS:");
    Serial.println(inputMessage2.c_str());
    WiFi.begin(inputMessage1.c_str(), inputMessage2.c_str());
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("WiFi Failed!");
      Serial.println("Configuring local AP Server...");
      WiFi.softAP(ssid_ap, password_ap);
      WiFi.softAPConfig(local_ip, gateway, subnet);
      delay(100);
      Serial.println("Local AP Server Configured!");
      parameters_received=0; 
      is_connected=false;
      return;
    }
    //ENCENDER TODOS LOS LEDS EN VERDE 
    Serial.println("Connected to Wifi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    parameters_received=0;
    is_connected=true;
    Serial.println("________________________________");
  }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {

  Serial.begin(115200);

  pinMode(SS, OUTPUT);
  pinMode(SET, INPUT);
  pinMode(SCK, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(SDA, OUTPUT);
  pinMode(SCL, OUTPUT);
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(LE, OUTPUT);
  pinMode(ACC_INT, INPUT);
  pinMode(EN_170, OUTPUT);
  pinMode(EN_12, OUTPUT);

  digitalWrite(R, LOW);
  digitalWrite(G, LOW);
  digitalWrite(B, LOW);  

  attachInterrupt(ACC_INT, acc, FALLING);
  attachInterrupt(SET, but, RISING);
  
  ledcSetup(LEDC_CHANNEL_1, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(R, LEDC_CHANNEL_0);

  ledcSetup(LEDC_CHANNEL_2, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(G, LEDC_CHANNEL_1);
  
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(B , LEDC_CHANNEL_2);

  reset();
  turn_off();

  set_four_numbers(0,0,0,0);

  Serial.println("---------------------SETTING UP----------------------");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        connection_counter++;
        if(connection_counter>=5){
          connection_counter=0;
          break;
        }
  }
  if(WiFi.status() == WL_CONNECTED){
      set_four_numbers(1,1,1,1);
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      is_connected=true;
      Serial.print("Connection:");
      Serial.println(is_connected);
      delay(500);
  }
  else{
      Serial.print("ESP32 SDK: ");
      Serial.println(ESP.getSdkVersion());
      Serial.println("Configuring local AP Server...");
      //WiFi.onEvent(WiFiEvent);
      WiFi.softAP(ssid_ap, password_ap);
      WiFi.softAPConfig(local_ip, gateway, subnet);
      delay(100);
      Serial.println("Local AP Server Configured!");
      WEB_SERVER_config();
      Serial.println("Web Server Configured!");
      is_connected=false;
      Serial.print("Connection:");
      Serial.println(is_connected);
  }
  flus(3);
  turn_off();
  Serial.println("-----------------------------------------------------");
}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void loop() {
  
      manage_info();
      
      if(button_flag){
        set_four_numbers(button_state,button_state,button_state,button_state);
        LEDS(100,100,20);
        delay(200);
        flus(3);
        turn_off();
        LEDS(0,0,0);
        button_flag=0;
        one_time=0;
        one_time_2=0;
        Serial.println("________________________________");
        Serial.println(states[button_state]);
        Serial.print("Connection:");
        Serial.println(is_connected);
        Serial.println("________________________________");
       
      }
      
      if(button_state==0){
        dice(number_of_dices,faces_of_dices);
      }
      else if(button_state==1){
        cronometer();
      }
      else if(button_state==2 && is_connected){
        universalclock(clock_period,0);
      }
      else if(button_state==3 && is_connected){
        universalclock(rest_period,1); 
      }

}
