/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

/* Variables -----------------------------------------------------------------*/
bool rtc_state = false;

uint16_t rtc_year  = 0;
uint8_t  rtc_month = 0, rtc_day = 0;
uint8_t  rtc_hour = 0, rtc_minute = 0, rtc_second = 0;

/* Internal config -----------------------------------------------------------*/
static const char* NTP1           = "pool.ntp.org";
static const char* NTP2           = "time.google.com";
static const char* NTP3           = "time.cloudflare.com";
static const long  GMT_OFFSET_SEC = 7 * 3600;  // VN: UTC+7
static const long  DST_OFFSET_SEC = 0;

/* Helpers -------------------------------------------------------------------*/
static bool time_is_valid()
{
  time_t now = time(nullptr);
  return (now > 1514764800);
}

static void load_localtime()
{
  struct tm t;
  time_t    now = time(nullptr);
  localtime_r(&now, &t);

  rtc_year   = (uint16_t)(t.tm_year + 1900);
  rtc_month  = (uint8_t)(t.tm_mon + 1);
  rtc_day    = (uint8_t)(t.tm_mday);
  rtc_hour   = (uint8_t)(t.tm_hour);
  rtc_minute = (uint8_t)(t.tm_min);
  rtc_second = (uint8_t)(t.tm_sec);
}

/* Task ----------------------------------------------------------------------*/
static void rtc_task(void* pv)
{
  configTime(GMT_OFFSET_SEC, DST_OFFSET_SEC, NTP1, NTP2, NTP3);

  uint32_t next_print_ms = 0;

  while (1)
  {
    if (WiFi.isConnected() && !time_is_valid())
    {
      for (int i = 0; i < 20 && !time_is_valid(); ++i)
      {
        vTaskDelay(pdMS_TO_TICKS(500));
      }
      rtc_state = time_is_valid();
    }

    if (rtc_state)
    {
      load_localtime();
    }

    uint32_t now_ms = millis();
    if (now_ms >= next_print_ms)
    {
      next_print_ms = now_ms + 60000UL;
      Serial.print("rtc:\t ");
      Serial.print(rtc_day);
      Serial.print("/");
      Serial.print(rtc_month);
      Serial.print("/");
      Serial.print((int)rtc_year);
      Serial.print(" ");
      Serial.print(rtc_hour);
      Serial.print(":");
      Serial.print(rtc_minute);
      Serial.print(":");
      Serial.println(rtc_second);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/* API giữ nguyên ------------------------------------------------------------*/
bool is_leap_year(uint8_t year)
{
  if (year % 4 != 0)
    return false;
  return true;
}

uint8_t days_in_month(uint8_t year, uint8_t month)
{
  static const uint8_t days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (month == 2 && is_leap_year(year))
    return 29;
  return days[month - 1];
}

void rtc_init(void)
{
  xTaskCreate(rtc_task, "RTC Task", 1024 * 2, nullptr, 1, nullptr);

  Serial.println("rtc:\t[init]");
}
