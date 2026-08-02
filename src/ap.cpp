/* Includes ------------------------------------------------------------------*/
#include "ap.h"

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <string.h>

#include "_wifi.h"
#include "html/html.h"
#include "main_process.h"
#include "variable.h"

/* Variables -----------------------------------------------------------------*/
AsyncWebServer server(80);

static char          ap_ssid_temp[32]     = "";
static bool          ap_logged_in         = false;
static unsigned long ap_logged_in_timeout = 0;
static unsigned long ap_timeout           = 0;

static bool     ap_exit_pending = false;
static uint32_t ap_exit_at      = 0;

/* Helpers -------------------------------------------------------------------*/
static void handleScanWifi(AsyncWebServerRequest* request)
{
  int8_t n = WiFi.scanComplete();

  if (n == WIFI_SCAN_FAILED)
  {
    WiFi.scanNetworks(true, false);
    request->send(202, "application/json", "[]");
    return;
  }
  if (n == WIFI_SCAN_RUNNING)
  {
    request->send(202, "application/json", "[]");
    return;
  }

  String json = "[";
  for (int i = 0; i < n; ++i)
  {
    if (i)
      json += ",";
    json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
  }
  json += "]";
  request->send(200, "application/json", json);
  WiFi.scanDelete();
}

/* Functions -----------------------------------------------------------------*/
void ap_init(void)
{
  ap_timeout   = millis() + AP_TIMEOUT;
  ap_logged_in = false;

  WiFi.disconnect(true);
  WiFi.mode(WIFI_MODE_NULL);
  vTaskDelay(pdMS_TO_TICKS(10));  // <--- yield

  esp_wifi_set_ps(WIFI_PS_NONE);

  snprintf(ap_ssid, sizeof(ap_ssid), "%s", _id);
  snprintf(ap_ssid_temp, sizeof(ap_ssid_temp), "%s %s", DEVICE_NAME, _id);

  WiFi.mode(WIFI_MODE_APSTA);
  vTaskDelay(pdMS_TO_TICKS(10));  // <--- yield

  WiFi.softAP(ap_ssid, ap_password);
  vTaskDelay(pdMS_TO_TICKS(10));  // <--- yield

  WiFi.enableSTA(true);
  vTaskDelay(pdMS_TO_TICKS(10));  // <--- yield

  IPAddress IP = WiFi.softAPIP();
  Serial.print("ap:\t[init] ");
  Serial.println(IP);

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest* r) { r->send(404); });

  server.on("/", HTTP_GET,
            [](AsyncWebServerRequest* r)
            {
              r->send_P(200, "text/html", html_monitor);
              ap_timeout = millis() + AP_TIMEOUT;
            });
  server.on("/", HTTP_POST,
            [](AsyncWebServerRequest* r)
            {
              r->send_P(200, "text/html", html_monitor);
              ap_timeout = millis() + AP_TIMEOUT;
            });

  server.on("/wifi", HTTP_GET,
            [](AsyncWebServerRequest* r)
            {
              r->send_P(200, "text/html", html_setting);
              ap_timeout = millis() + AP_TIMEOUT;
            });
  server.on("/wifi", HTTP_POST,
            [](AsyncWebServerRequest* r)
            {
              r->send_P(200, "text/html", html_setting);
              ap_timeout = millis() + AP_TIMEOUT;
            });

  server.on("/scan_wifi", HTTP_GET, [](AsyncWebServerRequest* r) { handleScanWifi(r); });

  server.on("/login", HTTP_GET,
            [](AsyncWebServerRequest* r)
            {
              r->send_P(200, "text/html", html_login);
              ap_timeout = millis() + AP_TIMEOUT;
            });
  server.on("/login", HTTP_POST,
            [](AsyncWebServerRequest* r)
            {
              r->send_P(200, "text/html", html_login);
              ap_timeout = millis() + AP_TIMEOUT;
            });

  server.on("/admin", HTTP_GET,
            [](AsyncWebServerRequest* r)
            {
              if (ap_logged_in)
                r->send_P(200, "text/html", html_admin);
              else
                r->send_P(200, "text/html", html_login);
              ap_timeout = millis() + AP_TIMEOUT;
            });

  server.on("/normal", HTTP_POST,
            [](AsyncWebServerRequest* r)
            {
              Serial.println("ap:\t[change to normal mode]");
              r->send_P(200, "text/html", html_normal);
              ap_exit_pending = true;
              ap_exit_at      = millis() + 250;
            });

  server.on("/data", HTTP_GET,
            [](AsyncWebServerRequest* r)
            {
              String data = String(wifi_ssid) + "," + String(ap_ssid_temp);
              r->send_P(200, "text/plain", data.c_str());
              ap_timeout = millis() + AP_TIMEOUT;
            });

  server.on("/getAdmin", HTTP_GET,
            [](AsyncWebServerRequest* r)
            {
              String data = String(_id) + "," + String(station_code);
              r->send_P(200, "text/plain", data.c_str());
              ap_timeout = millis() + AP_TIMEOUT;
            });

  server.on("/updateWiFi", HTTP_POST,
            [](AsyncWebServerRequest* r)
            {
              if (r->hasParam("new_ssid", true) && r->hasParam("new_password", true))
              {
                String newSSID     = r->getParam("new_ssid", true)->value();
                String newPassword = r->getParam("new_password", true)->value();

                strncpy(wifi_ssid, newSSID.c_str(), sizeof(wifi_ssid) - 1);
                wifi_ssid[sizeof(wifi_ssid) - 1] = '\0';
                strncpy(wifi_password, newPassword.c_str(), sizeof(wifi_password) - 1);
                wifi_password[sizeof(wifi_password) - 1] = '\0';

                wifi_ssid_length     = strlen(wifi_ssid);
                wifi_password_length = strlen(wifi_password);
                save_wifi();

                Serial.printf("ap:\t[new wifi] ssid=%s (%d); password=%s (%d)\n", wifi_ssid, wifi_ssid_length, wifi_password, wifi_password_length);
                r->send(200, "text/plain", "Update wifi successful");
              }
              ap_timeout = millis() + AP_TIMEOUT;
            });

  server.on("/checkLogin", HTTP_POST,
            [](AsyncWebServerRequest* r)
            {
              if (r->hasParam("username", true) && r->hasParam("password", true))
              {
                String username = r->getParam("username", true)->value();
                String password = r->getParam("password", true)->value();

                if (username == AP_ADMIN_USERNAME && password == AP_ADMIN_PASSWORD)
                {
                  Serial.println("ap:\t[check login] OK");
                  ap_logged_in         = true;
                  ap_logged_in_timeout = millis() + (30UL * 60UL * 1000UL);
                  r->send(200, "text/plain", "Login successful");
                }
                else
                {
                  Serial.printf("ap:\t[check login] Fail: username=%s (%d); password=%s (%d)\n", username.c_str(), (int)username.length(), password.c_str(),
                                (int)password.length());
                  r->send(401, "text/plain", "Login failed");
                }
                ap_timeout = millis() + AP_TIMEOUT;
              }
            });

  server.on("/updateAdmin", HTTP_POST,
            [](AsyncWebServerRequest* r)
            {
              if (!ap_logged_in)
              {
                Serial.println("ap:\t[updateAdmin] have not logged in");
                r->send(401, "text/plain", "Have not logged in");
                return;
              }
              if (r->hasParam("id", true) && r->hasParam("station_code", true))
              {
                String new_id           = r->getParam("id", true)->value();
                String new_station_code = r->getParam("station_code", true)->value();

                if (new_id != "NULL" && new_id.length() == 10)
                {
                  strncpy(_id, new_id.c_str(), sizeof(_id) - 1);
                  _id[sizeof(_id) - 1] = '\0';
                  Serial.print("ap:\t[id] ");
                  Serial.println(_id);
                  save_id();
                }
                if (new_station_code != "NULL")
                {
                  strncpy(station_code, new_station_code.c_str(), sizeof(station_code) - 1);
                  station_code[sizeof(station_code) - 1] = '\0';
                  Serial.print("ap:\t[station_code] ");
                  Serial.println(station_code);
                  save_station_code();
                }
                r->send(200, "text/plain", "Update admin successful");
              }
              ap_timeout = millis() + AP_TIMEOUT;
            });

  vTaskDelay(pdMS_TO_TICKS(10));
  server.begin();
  vTaskDelay(pdMS_TO_TICKS(10));
}

void ap_stop(void)
{
  server.end();
  if (WiFi.scanComplete() == WIFI_SCAN_RUNNING)
    WiFi.scanDelete();
  WiFi.softAPdisconnect(true);
  delay(120);
}

void ap_request_exit_to_normal()
{
  ap_exit_pending = true;
  ap_exit_at      = millis() + 250;
}

void ap_process(void)
{
  static uint32_t prev = 0;
  uint32_t        now  = millis();
  if (now - prev < 50)
    return;
  prev = now;

  if (ap_exit_pending && (int32_t)(millis() - ap_exit_at) >= 0)
  {
    ap_exit_pending = false;
    ap_stop();
    main_process_state = NORMAL;
    main_process_reset_state(NORMAL);
    return;
  }

  if (ap_logged_in && millis() > ap_logged_in_timeout)
    ap_logged_in = false;

  if (millis() > ap_timeout)
  {
    Serial.println("ap:\t[timeout] Back to Normal mode");
    main_process_state = NORMAL;
    main_process_reset_state(NORMAL);
  }
}
