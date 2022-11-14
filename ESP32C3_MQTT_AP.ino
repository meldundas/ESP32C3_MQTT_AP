
#include <string>

#define APNAME "ESP32C3"  //name for esp32c3 access point

#define MQTT_TOPIC_IN "esp32c3/led"
#define MQTT_TOPIC_OUT "esp32c3/button"

#define BUTTON 9
#define APBUTTON 19 //acces point request button

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Freenove_WS2812_Lib_for_ESP32.h"


#include <HTTPClient.h>
#include <WebServer.h>
#include <Preferences.h>


#define LEDS_COUNT  1
#define LEDS_PIN  8
#define CHANNEL   0

//LEDs
Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);

//MQTT
WiFiClient espClient;
PubSubClient client(espClient);



//Establishing Local server at port 80
  WebServer server(80);

  Preferences preferences;

class AccessPoint
{

  
private:
  /* data */
  String st;
  String content;
  int statusCode;
  String psid;
  String ppass = "";
  String pbroker;
  
public:
  AccessPoint(/* args */);
  ~AccessPoint();


bool testWifi(void)
{
  int c = 0;
  //Serial.println("Waiting for Wifi to connect");
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
  Serial.print("Broker: ");
  Serial.println(pbroker);
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
      //Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
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
    //st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP(APNAME, "");
  Serial.println("Initializing_softap_for_wifi credentials_modification");
  launchWeb();
  Serial.println("over");
}

void createWebServer()
{
  {
    server.on("/", [&]() {

      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      preferences.begin("credentials", false);
      String pbroker = preferences.getString("broker", "");
      preferences.end();

      this->content = "<!DOCTYPE HTML>\r\n";
      this->content += "<html>";
      this->content += "<head>";
      this->content += "<style>";
      this->content += "html {height: 100%;}";
      this->content += "body {background-image: linear-gradient(to bottom right, #33475b, #0033CC, #FF77CC, rgb(255, 122,89));";
      this->content += "color: #92a8d1;}";
      this->content += "h1 {color: white;}";
      this->content += "p {color: white;}";
      this->content += "</style>";
      this->content += "</head>";
      this->content += "<body>";
      this->content += "<h1>Welcome to Wifi Credentials Update page</h1>";
      this->content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      this->content += "<p>";
      this->content += "Current IP: ";
      this->content += ipStr;
      this->content += st;
      this->content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64>";
      this->content += "<p>";
      this->content += "Current MQTT Broker:</p><p>";
      this->content += pbroker;
      this->content += "</p><p><label for='brokers'>Choose a broker:  </label>";
      this->content += "<select name='broker' id='broker'>";
      this->content += "<option value='test.mosquitto.org'>Mosquitto</option>";
      this->content += "<option value='broker.hivemq.com'>HiveMQ</option>";
      this->content += "<option value='mqtt.eclipse.org'>Eclipse</option>";
      this->content += "<option value='broker.emqx.io'>EMQX</option>";
      this->content += "</select><br><br><input type='submit'></form></p>";
      this->content += "</body>";
      this->content += "</html>";
      server.send(200, "text/html", this->content);
    });
    server.on("/scan", [&]() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      this->content = "<!DOCTYPE HTML>\r\n";
      this->content += "<html>";
      this->content += "<head>";
      this->content += "<style>";
      this->content += "html {height: 100%;}";
      this->content += "body {background-image: linear-gradient(to bottom right, #33475b, #0033CC, #FF77CC, rgb(255, 122,89));";
      this->content += "color: #92a8d1;}";
      this->content += "h1 {color: white;}";
      this->content += "p {color: white;}";
      this->content += "</style>";
      this->content += "</head>";
      this->content += "<body>";
      this->content += "<h1>Rescanning...go back</h1>";
      server.send(200, "text/html", this->content);
    });

    server.on("/setting", [&]() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      String qbroker = server.arg("broker");

      Serial.print(qsid);
        Serial.print("   ");
        Serial.print(qpass);
        Serial.print("   ");
        Serial.print(qbroker);
        Serial.println("");

      if (qsid.length() > 0 && qpass.length() > 0 && qbroker.length() > 0) {
        //if new values entered - store new values

        // Serial.print(qsid);
        // Serial.print("   ");
        // Serial.print(qpass);
        // Serial.print("   ");
        // Serial.print(qbroker);
        // Serial.println("");

        preferences.begin("credentials", false);

        Serial.println("storing ssid:");

        preferences.putString("ssid", qsid);
        
        Serial.println("storing pass:");
        
        preferences.putString("pass", qpass);

        Serial.println("storing broker:");
        
        preferences.putString("broker", qbroker);
        // Close the Preferences
        preferences.end();

        this->content = "{\"Success\":\"saved preferences... reset to boot into new wifi\"}"; //FIXME: does not show this
        this->statusCode = 200;
        
      } else {
        this->content = "{\"Error\":\"404 not found\"}";
        this->statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      //server.send(this->statusCode, "application/json", this->content);
      server.send(this->statusCode, "text/html", this->content);

      Serial.println("sent update settings status");

      delay(5);

      ESP.restart();

    });
  }
}

};

AccessPoint::AccessPoint(/* args */)
{
}

AccessPoint::~AccessPoint()
{
}


unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (256)
char msg[MSG_BUFFER_SIZE];
int value = 0;


void mqtt_reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32-C3";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(MQTT_TOPIC_IN);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  StaticJsonDocument<0> filter;
  filter.set(true);
  StaticJsonDocument<64> doc;

  DeserializationError error = deserializeJson(doc, payload, length, DeserializationOption::Filter(filter));

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  
  
  Serial.println((uint8_t)doc["bright"]);
  Serial.println((uint8_t)doc["red"]);
  Serial.println((uint8_t)doc["green"]);
  Serial.println((uint8_t)doc["blue"]);

  strip.setBrightness(doc["bright"]);

  strip.setLedColorData(0, doc["red"], doc["green"], doc["blue"]);
  strip.show();
}


void mqttSendButtonState(bool state){
  char buffer[256];
  StaticJsonDocument<32> doc;
  doc["buttonStatus"] = (bool)state;
  size_t n = serializeJson(doc, buffer);
  client.publish(MQTT_TOPIC_OUT, (const uint8_t*)buffer, n,true);

}

//Start of Program

//Variables
int i = 0;

//Array of pointers to public MQTT brokers
// const char MQTT_SERVER[] = "broker.hivemq.com";
//const char* MQTT_SERVER = "broker.hivemq.com";
const char *Mosquitto = "test.mosquitto.org";
const char *HiveMQ = "broker.hivemq.com";
const char *Eclipse = "mqtt.eclipse.org";
const char *EMQX = "broker.emqx.io";
const char *MQTT_SERVER[] = {Mosquitto, HiveMQ, Eclipse, EMQX};


static volatile bool buttonStatus=false;

//Access Point
AccessPoint ap;


void setup() {
  Serial.begin(115200);

  strip.begin();
  strip.setBrightness(5);

  strip.setLedColorData(0, 0, 0, 255); //intensity rgb
  strip.show();

  //Next is to setup push button
  pinMode(BUTTON, INPUT_PULLUP);  

  pinMode(APBUTTON, INPUT_PULLUP);
 

  Serial.println("Disconnecting current wifi connection");
  WiFi.disconnect();
  
  // Open Preferences with my-app namespace. Each application module, library, etc
  // has to use a namespace name to prevent key name collisions. We will open storage in
  // RW-mode (second parameter has to be false).
  // Note: Namespace name is limited to 15 chars.
  //credentials {
  //ssid: "your_ssid"
  //pass: "your_pass"
  //}

  preferences.begin("credentials", false);
  
  delay(10);
  Serial.println();
  Serial.println();
  Serial.println("Startup");

  // Read preferences for ssid and pass
  Serial.println("Reading preferences");

  // Get the ssid value, if the key does not exist, return a default value of ""
  // Note: Key name is limited to 15 chars.
    
  String psid = preferences.getString("ssid", "");
  
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(psid);
  //Serial.println("Reading Preferences pass");

  String ppass = preferences.getString("pass", "");
  Serial.print("PASS: ");
  Serial.println(ppass);


  WiFi.begin(psid.c_str(), ppass.c_str());

  String pbroker = preferences.getString("broker", "");
  Serial.print("BROKER: ");
  Serial.println(pbroker);
  
  preferences.end();

  //const char* pbrokerc = pbroker.c_str();
  //Serial.println(pbrokerc);

  if(!pbroker.compareTo(MQTT_SERVER[0]))  //returns 0 if equal
      client.setServer(MQTT_SERVER[0], 1883);

  if(!pbroker.compareTo(MQTT_SERVER[1]))
      client.setServer(MQTT_SERVER[1], 1883);

  if(!pbroker.compareTo(MQTT_SERVER[2]))
      client.setServer(MQTT_SERVER[2], 1883);

  if(!pbroker.compareTo(MQTT_SERVER[3]))
      client.setServer(MQTT_SERVER[3], 1883);

  //client.setServer(MQTT_SERVER[1], 1883);  //TODO nw - pbroker.c_str() - w MQTT_SERVER
  client.setCallback(callback);
  
  mqttSendButtonState(buttonStatus);

}

void loop() {
  static bool last_state=false;


  if ((WiFi.status() == WL_CONNECTED))
  {

  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    mqtt_reconnect();     //TODO: - can get stuck here 
  } 
  client.loop();

  //digitalRead(BUTTON) ? buttonStatus=true : buttonStatus=false;
  digitalRead(BUTTON) ? buttonStatus=false : buttonStatus=true; //active lo
 
  //If we have a new state we need to change update and push the new state
  if(last_state!=buttonStatus){
    last_state=buttonStatus;
    mqttSendButtonState(buttonStatus);
    buttonStatus ? Serial.println("TRUE") : Serial.println("FALSE");
  }

  }
  else
  {
  }

    if (ap.testWifi() && (digitalRead(APBUTTON))) //button not pressed
  {
    //Serial.println(" connection status positive");
    return;
  }
  else
  {
    Serial.println("Connection Status Negative / APBUTTON LOW");
    Serial.println("Turning the HotSpot On");
    ap.launchWeb();
    ap.setupAP();// Setup HotSpot
  }

  Serial.println();
  Serial.println("Waiting.");

  while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print(".");
    strip.setLedColorData(0, 255, 0, 0); //not connected to wifi  flash red-green 
    strip.show();
    delay(100);
    strip.setLedColorData(0, 0, 255, 0);
    strip.show();
    delay(100);
    server.handleClient();
  }





  
   
}
