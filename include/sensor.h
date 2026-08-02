#pragma once

#ifndef __SENSOR_H_
#define __SENSOR_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Define --------------------------------------------------------------------*/
typedef struct
{
  float temperature_c;  // °C
  float humidity_pct;   // %RH
  bool  valid;          // true nếu đọc thành công
} DHT13_Data_t;

// Rain sensor
typedef struct
{
  uint16_t status;       // 0 = không mưa, 1 = có mưa
  uint16_t lower_limit;  // ngưỡng dưới (°C)
  uint16_t hysteresis;   // độ trễ (°C)
  uint16_t reset_delay;  // thời gian reset (s)
  uint16_t sensitivity;  // độ nhạy
  bool     valid;
} Rain_Data_t;

// Light sensor
typedef struct
{
  uint32_t intensity;  // 0 – 4,294,967,295
  bool     valid;
} Light_Data_t;

// TDS/EC sensor
typedef struct
{
  float    temperature_c;  // °C
  float    ec;             // µS/cm
  float    salinity;       // ppt
  float    tds;            // mg/L
  uint16_t ec_raw;         // raw ADC
  bool     valid;
} TDS_Data_t;

/* Variables -----------------------------------------------------------------*/
extern DHT13_Data_t g_dht13;
extern Rain_Data_t  g_rain;
extern Light_Data_t g_light;
extern TDS_Data_t   g_tds;

/* Functions -----------------------------------------------------------------*/
void sensor_init(void);

#endif