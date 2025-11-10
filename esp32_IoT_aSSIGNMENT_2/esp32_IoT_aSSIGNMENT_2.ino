#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Adafruit_NeoPixel.h>


#define WIFI_SSID "EE-983HCJ"
#define WIFI_PASSWORD "HnXrd6pqCNJ4H7pM"


#define MQTT_SERVER "test.mosquitto.org"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "ESP32_EnvMonitor_01" 
#define MQTT_TOPIC_PUB "smartenv/data"       

// DHT Sensor Settings
#define DHTPIN 14
#define DHTTYPE DHT11  
#define NEOPIXEL_PIN 5
#define NUMPIXELS 1

#define TEMP_WARNING_THRESHOLD 26.0
#define TEMP_CRITICAL_THRESHOLD 28.0
#define PUBLISH_INTERVAL_MS 5000  

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

long lastMsg = 0;



void setup_wifi() {
  delay(10);
  Serial.print("\nConnecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n WiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(MQTT_CLIENT_ID)) {
      Serial.println("Connected to Mosquitto broker");
    } else {
      Serial.print("Failed to connect");
      Serial.print(client.state());
      Serial.println("Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pixels.begin();
  pixels.show(); 

  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > PUBLISH_INTERVAL_MS) {
    lastMsg = now;

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      pixels.setPixelColor(0, pixels.Color(255, 0, 255)); 
      pixels.show();
      return;
    }

  
    if (temperature >= TEMP_CRITICAL_THRESHOLD) {
      pixels.setPixelColor(0, pixels.Color(255, 0, 0)); 
    } else if (temperature >= TEMP_WARNING_THRESHOLD) {
      pixels.setPixelColor(0, pixels.Color(255, 255, 0)); 
    } else {
      pixels.setPixelColor(0, pixels.Color(0, 255, 0)); 
    }
    pixels.show();

   
    String payload = "{\"temperature\": " + String(temperature, 2) +
                     ", \"humidity\": " + String(humidity, 2) + "}";

    Serial.print("Publishing to ");
    Serial.print(MQTT_TOPIC_PUB);
    Serial.print(": ");
    Serial.println(payload);

    client.publish(MQTT_TOPIC_PUB, payload.c_str());
  }
}