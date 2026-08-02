/* _wifi.cpp -----------------------------------------------------------------*/
#include "_wifi.h"
#include "led.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <string.h>

#include <ElegantOTA.h>
#include <WebServer.h>
#include <WiFiClient.h>

#include "main_process.h"
#include "rtc.h"
#include "sensor.h"
#include "variable.h"

/* Config --------------------------------------------------------------------*/
#define CHECK_INTERNET_CONNECTION_PERIOD 500   // ms
#define NETCHECK_INTERVAL_MS             3000  // ms
#define WIFI_RETRY_COOLDOWN_MS           3000  // ms
#define MQTT_BACKOFF_MAX_S               30

/* FSM -----------------------------------------------------------------------*/
typedef enum
{
  WIFI_INIT,
  CONNECT_WIFI,
  CONNECT_INTERNET,
  CONNECT_MQTT,
  MQTT_LOOP,
  HTTP_LOOP
} E_WIFI_PROCESS_STATE;

/* State/IO ------------------------------------------------------------------*/
uint8_t is_wifi_connected     = 0;
uint8_t is_internet_connected = 0;
uint8_t is_mqtt_connected     = 0;

WiFiClient   wifi_client;
HTTPClient   http_client;
PubSubClient mqtt_client(wifi_client);

WebServer ota_server(80);

static bool wifi_started = false;

/* Throttle & backoff --------------------------------------------------------*/
static uint32_t next_wifi_try_ms = 0;
static uint32_t next_mqtt_try_ms = 0;
static uint32_t next_netcheck_ms = 0;
static uint8_t  mqtt_backoff_s   = 2;

/* Protos --------------------------------------------------------------------*/
static bool check_internet_connection(void);
static void mqtt_publish_process(void);
static void mqtt_callback(String& topic, String& payload);

/* PubSubClient payload adapter -> String ------------------------------------*/
static void mqtt_callback_adapter(char* topic, uint8_t* payload, unsigned int len)
{
  String t(topic), p;
  p.reserve(len);
  for (unsigned int i = 0; i < len; ++i)
    p += (char)payload[i];
  mqtt_callback(t, p);
}

/* Internet probe ------------------------------------------------------------*/
static bool check_internet_connection(void)
{
  http_client.begin(wifi_client, "http://www.google.com/generate_204");
  http_client.setTimeout(CHECK_INTERNET_CONNECTION_PERIOD);
  int code = http_client.GET();

  if (code == 204)
  {
    is_internet_connected = 1;
    Serial.println("wifi:\tInternet OK");
    http_client.end();
    return true;
  }

  is_internet_connected = 0;
  if (code > 0)
  {
    Serial.printf("wifi:\tHTTP code=%d -> Internet NOT OK\n", code);
  }
  else
  {
    Serial.printf("wifi:\tHTTP error: %s\n", http_client.errorToString(code).c_str());
  }
  http_client.end();
  return false;
}

/* Bring-up STA cleanly ------------------------------------------------------*/
void wifi_init(void)
{
  if (wifi_started)
  {
    Serial.println("wifi:\t[init skipped]");
    return;
  }
  wifi_started = true;

  Serial.printf("wifi:\t[init] %s (%d) - %s (%d)\n", wifi_ssid, wifi_ssid_length, wifi_password, wifi_password_length);

  WiFi.softAPdisconnect(true);
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_MODE_NULL);
  delay(150);

  WiFi.persistent(false);
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(true);

  WiFi.mode(WIFI_MODE_STA);
  delay(50);
  WiFi.begin(wifi_ssid, wifi_password);

  Serial.println("wifi:\tconnect to wifi");
}

/* Periodic flags ------------------------------------------------------------*/
void wifi_handler(void)
{
  static uint32_t prev = 0;
  uint32_t        now  = millis();
  if (now - prev < 1000)
    return;
  prev = now;

  prev              = now;
  is_wifi_connected = WiFi.isConnected() ? 1 : 0;
  is_mqtt_connected = mqtt_client.connected() ? 1 : 0;

  if (!is_wifi_connected)
  {
    led_toggle_1s();
    is_internet_connected = 0;
    is_mqtt_connected     = 0;
  }
  else
  {
    led_on();
  }
}

void wifi_stop_for_ap(void)
{
  if (mqtt_client.connected())
  {
    mqtt_client.disconnect();
  }
  http_client.end();
}

/* FSM helpers ---------------------------------------------------------------*/
void mqtt_process_reset_state(E_WIFI_PROCESS_STATE state)
{
  switch (state)
  {
    case CONNECT_WIFI:
      if (!WiFi.isConnected() && millis() >= next_wifi_try_ms)
      {
        WiFi.softAPdisconnect(true);
        WiFi.disconnect(true);
        WiFi.mode(WIFI_MODE_STA);
        WiFi.begin(wifi_ssid, wifi_password);
        next_wifi_try_ms = millis() + WIFI_RETRY_COOLDOWN_MS;
        Serial.println("wifi:\tconnect to wifi");
      }
      break;

    case CONNECT_INTERNET:
      next_netcheck_ms = millis();  // cho phép check ngay lần đầu
      Serial.println("wifi:\tconnect to internet");
      break;

    case CONNECT_MQTT:
      mqtt_client.setBufferSize(1024 * 2);  // hoặc 2048 nếu bạn có heap dư (~>100KB)
      mqtt_client.setServer(mqtt_host, mqtt_port);
      Serial.printf("wifi:\tprepare mqtt %s : %d\n", mqtt_host, mqtt_port);
      break;

    case MQTT_LOOP:
      mqtt_client.setCallback(mqtt_callback_adapter);
      mqtt_client.subscribe(String(mqtt_topic_sub).c_str());
      Serial.println("wifi:\tmqtt loop");
      break;

    default:
      break;
  }
}

/* FSM main ------------------------------------------------------------------*/
void mqtt_process(void)
{
  static E_WIFI_PROCESS_STATE wifi_state = WIFI_INIT;

  static uint32_t prev = 0;
  uint32_t        now  = millis();
  if (now - prev < 50)
    return;
  prev = now;

  switch (wifi_state)
  {
    case WIFI_INIT:
      Serial.println("wifi:\t[init]");
      wifi_state = CONNECT_WIFI;
      mqtt_process_reset_state(CONNECT_WIFI);
      break;

    case CONNECT_WIFI:
      if (WiFi.isConnected())
      {
        Serial.print("wifi:\twifi connected ");
        Serial.println(WiFi.localIP());
        wifi_state = CONNECT_INTERNET;
        mqtt_process_reset_state(CONNECT_INTERNET);
      }
      break;

    case CONNECT_INTERNET:
      if (millis() >= next_netcheck_ms)
      {
        if (check_internet_connection())
        {
          Serial.println("wifi:\tinternet connected");
          mqtt_backoff_s   = 2;
          next_mqtt_try_ms = millis();

          ElegantOTA.begin(&ota_server);
          ota_server.begin();

          Serial.print("OTA ready: http://");
          Serial.println(WiFi.localIP());

          wifi_state = CONNECT_MQTT;
          mqtt_process_reset_state(CONNECT_MQTT);
        }
        else
        {
          Serial.println("wifi:\tinternet NOT connected");
          next_netcheck_ms = millis() + NETCHECK_INTERVAL_MS;
        }
      }
      break;

    case CONNECT_MQTT:
      if (!WiFi.isConnected())
      {
        Serial.println("wifi:\twifi disconnected");
        wifi_state = CONNECT_WIFI;
        mqtt_process_reset_state(CONNECT_WIFI);
        break;
      }

      if (!mqtt_client.connected() && millis() >= next_mqtt_try_ms)
      {
        Serial.printf("wifi:\tconnect to mqtt %s : %d\n", mqtt_host, mqtt_port);
        bool ok = mqtt_client.connect(_id, mqtt_username, mqtt_password);
        if (!ok)
        {
          int8_t rc = mqtt_client.state();
          Serial.printf("wifi:\tmqtt connect failed, state=%d\n", rc);
          if (mqtt_backoff_s < MQTT_BACKOFF_MAX_S)
          {
            mqtt_backoff_s *= 2;
            if (mqtt_backoff_s > MQTT_BACKOFF_MAX_S)
              mqtt_backoff_s = MQTT_BACKOFF_MAX_S;
          }
          next_mqtt_try_ms = millis() + (uint32_t)mqtt_backoff_s * 1000UL;
        }
        else
        {
          mqtt_backoff_s   = 2;
          next_mqtt_try_ms = millis() + 30000UL;
        }
      }

      if (mqtt_client.connected())
      {
        Serial.printf("wifi:\tmqtt connected (%s)\n", mqtt_username);
        wifi_state = MQTT_LOOP;
        mqtt_process_reset_state(MQTT_LOOP);
      }
      break;

    case MQTT_LOOP:
      if (!WiFi.isConnected())
      {
        Serial.println("wifi:\twifi disconnected");
        wifi_state = CONNECT_WIFI;
        mqtt_process_reset_state(CONNECT_WIFI);
        break;
      }
      if (!mqtt_client.connected())
      {
        Serial.println("wifi:\tmqtt is not connected");
        wifi_state = CONNECT_MQTT;  // quay lại nhánh reconnect theo backoff
        break;
      }

      mqtt_publish_process();
      mqtt_client.loop();

      ota_server.handleClient();
      ElegantOTA.loop();
      break;

    default:
      wifi_state = WIFI_INIT;
      Serial.println("wifi:\tunknown state");
      break;
  }
}

/* MQTT callbacks/publish ----------------------------------------------------*/
static void mqtt_callback(String& topic, String& payload)
{
  Serial.println("mqtt:\t[subscribed] " + topic + ":\n" + payload);
}

void mqtt_pub_test()
{
  JsonDocument json_doc;
  JsonObject   json_root = json_doc.to<JsonObject>();
  String       json_string;

  json_root["reset"] = reset_count;
  json_root["id"]    = String(_id);

  // --- RTC block ---
  JsonObject rtc = json_root["rtc"].to<JsonObject>();
  char       buf[32];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d", (int)rtc_year, (int)rtc_month, (int)rtc_day, (int)rtc_hour, (int)rtc_minute, (int)rtc_second);
  json_root["rtc"] = buf;

  // --- DHT13 block ---
  JsonObject dht       = json_root["dht"].to<JsonObject>();
  dht["valid"]         = g_dht13.valid ? true : false;
  dht["temperature_c"] = g_dht13.temperature_c;
  dht["humidity_pct"]  = g_dht13.humidity_pct;

  // --- Light block ---
  JsonObject light   = json_root["light"].to<JsonObject>();
  light["valid"]     = g_light.valid ? true : false;
  light["intensity"] = g_light.intensity;

  // --- Rain block ---
  JsonObject rain     = json_root["rain"].to<JsonObject>();
  rain["valid"]       = g_rain.valid ? true : false;
  rain["status"]      = g_rain.status;
  rain["lower_limit"] = g_rain.lower_limit;
  rain["hysteresis"]  = g_rain.hysteresis;
  rain["reset_delay"] = g_rain.reset_delay;
  rain["sensitivity"] = g_rain.sensitivity;

  // --- TDS block ---
  JsonObject tds       = json_root["tds"].to<JsonObject>();
  tds["valid"]         = g_tds.valid ? true : false;
  tds["temperature_c"] = g_tds.temperature_c;
  tds["ec"]            = g_tds.ec;
  tds["salinity"]      = g_tds.salinity;
  tds["tds"]           = g_tds.tds;
  tds["ec_raw"]        = g_tds.ec_raw;

  // serialize & publish
  serializeJson(json_doc, json_string);

  if (mqtt_client.connected())
  {
    bool ok = mqtt_client.publish(mqtt_topic_pub, json_string.c_str());
    Serial.printf("mqtt:\t[publish] result=%d\n", ok);
  }
  else
  {
    Serial.println("mqtt:\t[publish fail] not connected");
  }
}

static void mqtt_publish_process(void)
{
  if (!mqtt_client.connected())
    return;

  static unsigned long last_pub = 0;
  unsigned long        now      = millis();

  if (now - last_pub >= 10000)
  {
    last_pub = now;
    mqtt_pub_test();
  }
}