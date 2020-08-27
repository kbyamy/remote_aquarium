#include <M5StickC.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
 
// WiFi //////////////////////////////////////////////////////////////
const char* ssid     = "WiFiのSSID";
const char* password = "WiFiのパスワード";

// AWS IoT ///////////////////////////////////////////////////////////
const char* awsEndpoint = "xxx.xxx.xxx.amazonaws.com";
const int   awsPort     = 8883;
const char* pubTopic    = "topic01/fromDevice";
const char* subTopic    = "topic01/fromCloud";

const char* rootCA = "-----BEGIN CERTIFICATE-----\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"-----END CERTIFICATE-----\n";

const char* certificate = "-----BEGIN CERTIFICATE-----\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"-----END CERTIFICATE-----\n";

const char* privateKey = "-----BEGIN RSA PRIVATE KEY-----\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
"-----END RSA PRIVATE KEY-----\n";


WiFiClientSecure httpsClient;
PubSubClient     mqttClient(httpsClient);

static const int     RX_BUF_SIZE = 100000;

long messageSentAt = 0;
int  dummyValue = 0;
char pubMessage[128];

void setup_wifi(){
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.disconnect(true);
  delay(1000);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_awsiot(){
  httpsClient.setCACert(rootCA);
  httpsClient.setCertificate(certificate);
  httpsClient.setPrivateKey(privateKey);
  mqttClient.setServer(awsEndpoint, awsPort);
  mqttClient.setCallback(mqttCallback);
}

void connect_awsiot() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client IDG
    String clientId = "m5stickc";
    //clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      mqttClient.subscribe(subTopic, 0); // QOS=0
      Serial.println("subscribed");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying＿
      delay(5000);
    }
  }
}

void mqttCallback (char* topic, byte* payload, unsigned int length) {
    Serial.print("Received. topic=");
    Serial.println(topic);

    char message[length];
    for (int i = 0; i < length; i++) {
      message[i] = (char)payload[i];
    }
    Serial.println(message);

    M5.Lcd.setCursor(5, 50);
    M5.Lcd.printf("Received: %s", message);
}

void setup() {
  M5.begin();
  M5.Axp.ScreenBreath(0);

  Serial.begin(115200);
  
  delay(10000) ;
  
  setup_wifi();
  setup_awsiot();
}

void loop() {
  
  if (!mqttClient.connected()) {
    connect_awsiot();
  }

  mqttClient.loop();

  long now = millis();
  if (now - messageSentAt > 5000) {
    messageSentAt = now;
    sprintf(pubMessage, "{\"message\": \"%d\"}", dummyValue++);
    Serial.print("Publishing message to topic ");
    Serial.println(pubTopic);
    Serial.println(pubMessage);
    mqttClient.publish(pubTopic, pubMessage);
    Serial.println("Published.");
  }
  delay(5);
}
