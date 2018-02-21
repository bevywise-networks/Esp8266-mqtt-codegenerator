#include<instrumentation.h>

bool shouldSaveConfig = false;
int i=0;
WiFiClient espClient;
PubSubClient client(espClient);


void saveConfigCallback () 
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
  
}

int stringToNumber(String thisString) 
{
      int i, value, length;
      length = thisString.length();
      char blah[(length + 1)];
      for (i = 0; i < length; i++) 
      {
        blah[i] = thisString.charAt(i);
      }
      blah[i] = 0;
      value = atoi(blah);
      return value;
      
    }



void reconnect(char* data,const char* CLIENT_ID,const char* user_name,const char* pass_word,const char* topic) 
{
if (!client.connected()) 
{
Serial.print("Attempting MQTT connection...");
Serial.println(user_name);
Serial.println(pass_word);
if (client.connect(CLIENT_ID,user_name,pass_word)) 
{
Serial.println("connected");
client.subscribe(topic);
client.publish(topic,data);
} 
else 
{

Serial.print("failed, rc=");
Serial.print(client.state());
Serial.println(" try again in 5 seconds");
delay(5000);
}
}
}


void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
   {
    Serial.print((char)payload[i]);
   }
   Serial.println();
}

void setdata(char* data,int buttonpin ,const char* WIFI_SSID,const char* WIFI_PASSWORD,const char* mqtt_server,const char* mqtt_port,const char* user_name,const char* pass_word,const char* CLIENT_ID,const char* topic) 
{

if(digitalRead(buttonpin) == LOW)
{ 
  
  Serial.println("mounting FS");
  if (SPIFFS.begin()) 
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) 
    {
      
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) 
      {
        Serial.println("opened config file");
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) 
        {
          Serial.println("\nparsed json");
          user_name=json["user_name"];
          pass_word=json["pass_word"];
          mqtt_server=json["mqtt_server"];
          mqtt_port=json["mqtt_port"];
         } 
        else 
        {
          Serial.println("failed to load json config");
        }
      }
    }
  } 
  else 
  {
    Serial.println("failed to mount FS");
  }
 WiFiManagerParameter custom_user_name("user_name", "user name", user_name, 40);
 WiFiManagerParameter custom_pass_word("pass_word", "pass word", pass_word, 40);  
 WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
 WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_user_name);
  wifiManager.addParameter(&custom_pass_word);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  Serial.begin(115200);
  Serial.println("\n Starting");
  WiFi.printDiag(Serial);  
  if (!wifiManager.startConfigPortal()) 
 {
    Serial.println("failed to connect and hit timeout");
    
  }

  Serial.println("connected... :)");
  user_name=custom_user_name.getValue();
  pass_word=custom_pass_word.getValue();
  mqtt_server=custom_mqtt_server.getValue();
  mqtt_port=custom_mqtt_port.getValue();
  if (shouldSaveConfig) 
  {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["user_name"]= user_name;
    json["pass_word"]= pass_word;
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) 
    {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  
  
  

  
  client.setServer(mqtt_server, 1883);
  reconnect(data,CLIENT_ID,user_name,pass_word,topic);
  client.setCallback(callback);

  
 
}

  if(WiFi.SSID()!="")
  {
    delay(10000);
     
    WiFi.mode(WIFI_STA);
     
    Serial.print(WiFi.SSID());
    
    unsigned long startedAt = millis();
     Serial.print("After waiting ");
    int connRes = WiFi.waitForConnectResult();
    float waited = (millis()- startedAt);
    Serial.print(waited/1000);
    Serial.print(" secs in setup() connection result is ");
    Serial.println(connRes);
  
  pinMode(buttonpin, INPUT_PULLUP);
  
  if (WiFi.status()!=WL_CONNECTED)
  {
    Serial.println("failed to connect, finishing setup anyway");
  } 
  else
  {
        
    Serial.print("local ip: ");
    Serial.println(WiFi.localIP());
    if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) 
      {
        Serial.println("opened config file");
        size_t size = configFile.size();
        
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) 
        {
          Serial.println("\nparsed json");
         user_name=json["user_name"];
          pass_word=json["pass_word"];
          mqtt_server=json["mqtt_server"];
          mqtt_port, json["mqtt_port"];
    
  }
  
  client.setServer(mqtt_server, 1883);
  reconnect(data,CLIENT_ID,user_name,pass_word,topic);
  client.setCallback(callback);

         
         
          
      }
      }
    }
    }
  }

   
 if(WiFi.SSID()=="") 
{
Serial.println("mounting FS...");
  if (SPIFFS.begin()) 
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) 
    {
      
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) 
      {
        Serial.println("opened config file");
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) 
        {
          Serial.println("\nparsed json");
          user_name=json["user_name"];
          pass_word=json["pass_word"];
          mqtt_server=json["mqtt_server"];
          mqtt_port=json["mqtt_port"];
          
        } 
        else 
        {
          Serial.println("failed to load json config");
        }
      }
    }
  } 
  else 
  {
    Serial.println("failed to mount FS");
  }
  
 WiFiManagerParameter custom_user_name("user_name", "user name", user_name, 40);
 WiFiManagerParameter custom_pass_word("pass_word", "pass word", pass_word, 40);
 WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
 WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
 WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  wifiManager.addParameter(&custom_user_name);
  wifiManager.addParameter(&custom_pass_word);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  
  Serial.begin(115200);
  Serial.println("\n Starting");
  WiFi.printDiag(Serial); 
  
  if (!wifiManager.startConfigPortal()) 
 {
    Serial.println("failed to connect and hit timeout");
    
  }

  
  Serial.println("connected)");
  user_name=custom_user_name.getValue();
  pass_word=custom_pass_word.getValue();
  mqtt_server=custom_mqtt_server.getValue();
  mqtt_port=custom_mqtt_port.getValue();
  if (shouldSaveConfig) 
  {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["user_name"]= user_name;
    json["pass_word"]=pass_word;
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) 
    {
      Serial.println("failed to open config file for writing");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, 1883);
  reconnect(data,CLIENT_ID,user_name,pass_word,topic);
  client.setCallback(callback);
  }
  
  
  
  
  
  

}


  



void senddata(char* data,const char* mqtt_server,const char* mqtt_port,const char* user_name,const char* pass_word,const char* CLIENT_ID,const char* topic) 
{
  
if (!client.connected())
    {
      
     
      reconnect(data,CLIENT_ID,user_name,pass_word,topic);
      
    } 

    for(i=0;i<1000;i++)
      {
        client.loop();
      }

   delay(5000);
 
client.disconnect();
  
}
