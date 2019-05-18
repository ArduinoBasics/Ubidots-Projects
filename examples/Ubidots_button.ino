/*==========================================================================
 * Project: Ubidots button
 * Author:  Scott C
 * Date created: 06 May 2019
 * Arduino IDE version: 1.8.5
 * Operating System: Windows 10 Pro
 * Tutorial Link: TBA
 * 
 * Description: 
 * Create a button on a Ubidots dashboard and interface with an ESP8266 based module (Maker Display 2) via MQTT subscription.
 *  
 * Acknowledgements:
 * A number of different tutorials on the internet contributed the knowledge-base required to build this project.
 * https://help.ubidots.com/connect-your-devices/connect-an-esp32-devkitc-to-ubidots-over-mqtt
 * https://help.ubidots.com/user-guides/how-to-adjust-the-device-name-and-variable-name
 * https://help.ubidots.com/user-guides/find-your-token-from-your-ubidots-account)
 * https://help.ubidots.com/how-to-with-ubidots/controlling-devices-using-mqtt
 * https://help.ubidots.com/user-guides/making-your-own-mqtt-library
 * https://pubsubclient.knolleary.net/api.html
 * 
============================================================================ */

 /* -------------------------------------------------------------------------   
 * Libraries           
 * -------------------------------------------------------------------------- 
 * ESP8266WiFi.h  : https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi  and  https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html  and
 * PubSubClient.h : https://github.com/knolleary/pubsubclient
 * 
 * ESP8266WiFi.h  : This library is required for the WiFi connection to the internet.
 * PubSubClient.h : Is used to create an MQTT Client to handle the communication between the Ubidots MQTT broker and the Maker Display (ESP8266).
 ============================================================================ */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>


/*----------------------------------------------------------------------------
 * Global Variables
 * ---------------------------------------------------------------------------
 *         SSID_NAME : The SSID of your WiFi Router
 *         SSID_PASS : The PASSWORD of your WiFI Router
 *             TOKEN : Your specific Ubidots TOKEN 
 *  MQTT_CLIENT_NAME : Unique MQTT Client Name - the name can be anything, but must be unique.
 *      DEVICE_LABEL : The "Device API Label" = maker-display
 *   VARIABLE_LABEL1 : The "Button1 API Label" for the button = button1
 *        mqttBroker : The link to the Ubidots MQTT broker
 *       buttonTopic : This char array will hold the path to the button on the Ubidots dashboard. Eg:  /v1.6/devices/maker-display/button1/lv  
 *      makerDisplay : create a WiFiClient
 *            client : create a PubSubClient from the WiFi Client - for MQTT communication.
 *            
 ============================================================================ */

const char* SSID_NAME = "WiFi_Name"; 
const char* SSID_PASS = "WiFi_password"; 
const char* TOKEN = "xxxxxxx-Ubidots_Token-xxxxxxxx"; 
const char* MQTT_CLIENT_NAME = "makerDisplayWhite";                                 
const char* DEVICE_LABEL = "maker-display";        
const char* VARIABLE_LABEL1 = "button1";  
char mqttBroker[]  = "industrial.api.ubidots.com";
char buttonTopic[100];

WiFiClient makerDisplay;
PubSubClient client(makerDisplay);



/*----------------------------------------------------------------------------
 * setup()
 * ---------------------------------------------------------------------------
 * Begin Serial communication at a Baud Rate of 9600
 *  - Wait for the COM port to start Serial communication. Show the progress with a "."
 * Establish a WiFi connection to the router using your SSID NAME and PASSWORD.
 *  - Give it time to connect. Once the WiFi.status() changes to WL_CONNECTED. Then continue...
 * Set the module to reconnect to an access point in case it is disconnects.
 * Configure the MQTT client to connect to the Ubidots mqttBroker ("industrial.api.ubidots.com") on port 1883 upon request.
 * If the broker sends information back to WiFi module, automatically call the "callback()" function.            
 ============================================================================ */

void setup(){
  Serial.begin(9600);
  while (!Serial) {
    delay(10); 
  }
  
  WiFi.begin(SSID_NAME, SSID_PASS);                  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");                               
  }
  Serial.println();
  Serial.println(F("WiFi connected"));
  WiFi.setAutoReconnect(true); 
                        
  client.setServer(mqttBroker, 1883);                 
  client.setCallback(callback);                       
}


/*----------------------------------------------------------------------------
 * loop()
 * ---------------------------------------------------------------------------
 * If the MQTT client is NOT connected, then call the MQTTconnect function to establish the connection.           
 * The client.loop() function allows the program to monitor the communication link with the MQTT broker.
 ============================================================================ */

void loop(){
   if (!client.connected()) {
    MQTTconnect();
  }
  client.loop();
}




/*----------------------------------------------------------------------------
 * callback()
 * ---------------------------------------------------------------------------
 * The function first ensures that the topic received from the MQTT broker is the correct one for the Button.
 * The strcmp function returns a zero (0) if "topic" and "buttonTopic" variables are equal  i.e. /v1.6/devices/maker-display/button1/lv
 * The value of the button (0 or 1) will be captured by the payload variable.
 * If payload[0]='1' then the button is ON. If zero '0', then it is OFF.
 * Print the state of the button to the Serial monitor.
 ============================================================================ */

void callback(char* topic, byte* payload, unsigned int length) {
  if(strcmp(topic,buttonTopic)==0){
    if(payload[0]=='1'){
      Serial.println("BUTTON ON");
    } else {
      Serial.println("BUTTON OFF");
    }
  }
}



/*----------------------------------------------------------------------------
 * MQTTconnect()
 * ---------------------------------------------------------------------------
 * While the MQTT client is NOT connected,
 *  - try to connect to the MQTT broker, using the unique MQTT_CLIENT_NAME, and TOKEN.  
 *  - remember that the MQTT broker was configured in setup: "industrial.api.ubidots.com" on port 1883 
 *  
 *  Display a message upon successful connection.
 *  Using the sprintf() function, buttonTopic will equal: /v1.6/devices/maker-display/button1/lv
 *  Subscribe to receive messages related to the button on the Ubidots dashboard (defined by buttonTopic).
 *  
 *  If the connection FAILS - print a "Failed connection" message. 
 *  Then wait 2 seconds before trying again.
 *  
 ============================================================================ */
 
void MQTTconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {       
      Serial.print("Successful Connection to ");               
      Serial.print(mqttBroker);
      Serial.println(" on port 1883");
      
      sprintf(buttonTopic, "/v1.6/devices/%s/%s/lv", DEVICE_LABEL, VARIABLE_LABEL1);
      client.subscribe(buttonTopic); 
                         
    } else {
      Serial.print("Failed connection, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}
