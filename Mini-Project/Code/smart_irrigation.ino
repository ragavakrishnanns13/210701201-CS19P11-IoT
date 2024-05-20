#include "DHT.h"
#include "ESP8266WiFi.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Pin and sensor type definitions
#define DHTPIN D3
#define DHTTYPE DHT11
#define SOILMOISTURE 13
#define WLAN_SSID "@j@"
#define WLAN_PASS "87654321"
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "projecttopic56"
#define AIO_KEY "aio_GuHM497sfloweJzOh0HCcbqJhZAU"

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish photocell1 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");
Adafruit_MQTT_Publish photocell2 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humy");
Adafruit_MQTT_Publish photocell3 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/msg2");
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/relay1");

char arr1[] = "000.00";
char arr2[] = "100";
char arr3[] = "YES";
long temperature2;
char humidity;
char soilmoiture_flag;
char cnt = 0;

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println(F("DHTxx test!"));
  dht.begin();
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  pinMode(SOILMOISTURE, INPUT);
  Serial.println(F("Adafruit MQTT demo"));
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  mqtt.subscribe(&onoffbutton);
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("% temperature2: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  temperature2 = (long)(t * 100);
  updateArr1();
  humidity = (char)h;
  updateArr2();
  updateSoilMoistureFlag();

  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(3000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
      if (strcmp((char *)onoffbutton.lastread, "OFF") == 0) digitalWrite(2, HIGH);
      if (strcmp((char *)onoffbutton.lastread, "ON") == 0) digitalWrite(2, LOW);
    }
  }

  cnt = (cnt % 3) + 1;
  if (cnt == 1) photocell1.publish(arr1);
  else if (cnt == 2) photocell2.publish(arr2);
  else if (cnt == 3) photocell3.publish(arr3);
}

void updateArr1() {
  if (temperature2 < 10) {
    snprintf(arr1, sizeof(arr1), " 0.0%d", temperature2 % 10);
  } else if (temperature2 < 100) {
    snprintf(arr1, sizeof(arr1), " 0.%02ld", temperature2);
  } else if (temperature2 < 1000) {
    snprintf(arr1, sizeof(arr1), " %ld.%02ld", temperature2 / 100, temperature2 % 100);
  } else if (temperature2 < 10000) {
    snprintf(arr1, sizeof(arr1), "%ld.%02ld", temperature2 / 100, temperature2 % 100);
  } else {
    snprintf(arr1, sizeof(arr1), "%ld.%02ld", temperature2 / 100, temperature2 % 100);
  }
}

void updateArr2() {
  if (humidity < 10) {
    snprintf(arr2, sizeof(arr2), "  0%d", humidity % 10);
  } else if (humidity < 100) {
    snprintf(arr2, sizeof(arr2), " %d", humidity);
  } else {
    snprintf(arr2, sizeof(arr2), "%d", humidity);
  }
}

void updateSoilMoistureFlag() {
  soilmoiture_flag = digitalRead(SOILMOISTURE) == LOW ? 0 : 1;
  if (soilmoiture_flag == 0) {
    strcpy(arr3, "YES");
  } else {
    strcpy(arr3, "NO ");
  }
}

void MQTT_connect() {
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0) {
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}
