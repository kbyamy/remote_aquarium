#include <M5StickC.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
//#include <base64.h>
 
// WiFi //////////////////////////////////////////////////////////////
const char* ssid = "WiFiのSSID";
const char* password = "WiFiのパスワード";

// AWS IoT ///////////////////////////////////////////////////////////
const char* awsEndpoint = "xxx.xxx.xxx.amazonaws.com";
const int awsPort = 8883;
const char* pubTopic = "pub01";
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
PubSubClient mqttClient(httpsClient);

typedef struct {
  uint32_t length;
  uint8_t *buf;
} jpeg_data_t;

jpeg_data_t jpeg_data;

static const int RX_BUF_SIZE = 100000;
 
static const uint8_t packet_begin[3] = { 0xFF, 0xD8, 0xEA };

long messageSentAt = 0;
int dummyValue = 0;
char pubMessage[128];

void setup_wifi(){
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // For WiFi Error
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
    // Create a random client ID
    String clientId = "ESP32-device";
    //clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
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
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.print("\n");
}

void setup() {
  M5.begin();
  M5.Axp.ScreenBreath(0);

  Serial.begin(115200);
  Serial2.begin(1152000, SERIAL_8N1, 32, 33); // Grove

  // jpeg image data buffer
  jpeg_data.buf = (uint8_t *) malloc(sizeof(uint8_t) * RX_BUF_SIZE);
  
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

    if (Serial2.available()) {
      digitalWrite(10, LOW);
      uint8_t rx_buffer[10];
      int rx_size = Serial2.readBytes(rx_buffer, 10);
      if (rx_size == 10) {
        if ((rx_buffer[0] == packet_begin[0]) && (rx_buffer[1] == packet_begin[1]) && (rx_buffer[2] == packet_begin[2])) {
          jpeg_data.length = (uint32_t)(rx_buffer[4] << 16) | (rx_buffer[5] << 8) | rx_buffer[6];
          int rx_size = Serial2.readBytes(jpeg_data.buf, jpeg_data.length);
          Serial.print("rx_size is ");
          Serial.println(rx_size);
          // TODO:jpeg_data.bufをbase64エンコードする
          // client.write(jpeg_data.buf, jpeg_data.length);
          sprintf(pubMessage, "{\"message\": \"%d\"}", dummyValue++);
          Serial.print("Publishing message to topic ");
          Serial.println(pubTopic);
          Serial.println(pubMessage);
          mqttClient.publish(pubTopic, pubMessage);
          Serial.println("Published.");
        }
      }
    }
  }
  delay(5);
}
