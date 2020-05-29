/*
 * Board: Node32 Lite (ESP32 Dev Module)
 * https://www.cytron.io/p-node32-lite-wifi-and-amp;-bluetooth-development-kit
 */


#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SimpleDHT.h>


#define APIKEY  "type your favoriot api key here"
#define DEVICE_DEV_ID "device id here"        //change id device

const char ssid[] = "your id for wifi/hotspot";             //change SSID wifi
const char password[] = "password for wifi/hotspot";     //change password wifi

#define SensorPin 34          // the pH meter Analog output is connected with the Arduino’s Analog
unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;
long previousMillis = 0;
int interval = 5000; // 5 seconds



void setup()
{
  pinMode(13,OUTPUT); 
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

}

void loop()
{
  for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    buf[i]=analogRead(SensorPin);
    delay(10);
  }
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=buf[i];
  float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue=3.5*phValue;                      //convert the millivolt into pH value
  Serial.print("    pH:");  
  Serial.print(phValue,2);
  Serial.println(" ");
  
  digitalWrite(13, HIGH);       
  delay(800);
  digitalWrite(13, LOW); 
 
   StaticJsonDocument<200> doc;
  
    JsonObject root = doc.to<JsonObject>(); // Json Object refer to { }
    root["device_developer_id"] = DEVICE_DEV_ID;
  
    JsonObject data = root.createNestedObject("data");
    data["phValue"] = (float)phValue;
    
  
    String body;
    serializeJson(root, body);
    Serial.println(body);
  
    HTTPClient http;
  
    http.begin("http://apiv2.favoriot.com/v2/streams");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Apikey", APIKEY);
  
    int httpCode = http.POST(body);
    if (httpCode > 0) {
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    }
    else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();

    previousMillis = millis();
  
}
