/* sensor.cpp ----------------------------------------------------------------
   Read Modbus sensors (ESP32, MAX485)
   - Uses ModbusMaster library
   - Creates RTOS task sensor_task() to read 4 sensors every 2 s
   - Adjust DE/RX/TX pins at top if wiring different
------------------------------------------------------------------------------*/

#include "sensor.h"

#include <Arduino.h>
#include <ModbusMaster.h>

/* -------------------------
   RS485 / UART pin config
   chỉnh lại nếu wiring khác
   -------------------------*/
#define DE_PIN 18  // RE/DE của MAX485 (driver enable)
#define TX_PIN 5   // ESP32 TX -> DI
#define RX_PIN 19  // ESP32 RX <- RO

#define MODBUS_SERIAL_BAUD 4800
#define MODBUS_CONFIG      SERIAL_8N1

/* -------------------------
   Sensor Modbus defines (bạn đã cung cấp)
   -------------------------*/
// DHT13 – Temperature & Humidity Sensor (ID = 2)
#define MODBUS_ID_DHT13       2
#define REG_DHT13_HUMIDITY    0x0000
#define REG_DHT13_TEMPERATURE 0x0001
#define LEN_DHT13             2

// Rain Sensor (ID = 5)
#define MODBUS_ID_RAIN  5
#define REG_RAIN_STATUS 0x0000
#define REG_RAIN_LL     0x0031

// Light Sensor (ID = 4)
#define MODBUS_ID_LIGHT       4
#define REG_LIGHT_INTENSITY_L 0x0003
#define REG_LIGHT_INTENSITY_H 0x0004
#define LEN_LIGHT_INTENSITY   2

// TDS/EC Sensor (ID = 6)
#define MODBUS_ID_TDS      6
#define REG_TDS_TEMP       0x0000
#define LEN_TDS_TEMP       1
#define REG_TDS_EC_SAL_TDS 0x0002
#define LEN_TDS_EC_SAL_TDS 3
#define REG_TDS_ECRAWAD    0x0008
#define LEN_TDS_ECRAWAD    1
#define REG_TDS_TEMP_COMP  0x0020
#define REG_TDS_COEFFS     0x0022
#define LEN_TDS_COEFFS     4

/* -------------------------
   Modbus master instance
   -------------------------*/
static HardwareSerial ModbusSerial(2);
static ModbusMaster   node;

DHT13_Data_t g_dht13 = {0};
Rain_Data_t  g_rain  = {0};
Light_Data_t g_light = {0};
TDS_Data_t   g_tds   = {0};

/* Forward declarations */
static void preTransmission();
static void postTransmission();
static void sensor_task(void* pv);

/* -------------------------
   Helpers
   -------------------------*/
static inline uint32_t make_u32(uint16_t hi, uint16_t lo)
{
  return (((uint32_t)hi) << 16) | (uint32_t)lo;
}

/* -------------------------
   Sensor read functions
   Returns true on success, false on failure
   -------------------------*/
static bool read_dht13(float& humidity_pct, float& temperature_c)
{
  node.begin(MODBUS_ID_DHT13, ModbusSerial);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  uint8_t res = node.readInputRegisters(REG_DHT13_HUMIDITY, LEN_DHT13);
  if (res != node.ku8MBSuccess)
    return false;

  uint16_t raw_h = node.getResponseBuffer(0);
  uint16_t raw_t = node.getResponseBuffer(1);

  // Giả định: raw scaled by 100 (ví dụ 2760 -> 27.60°C)
  humidity_pct  = (float)raw_h / 100.0f;
  temperature_c = (float)raw_t / 100.0f;
  return true;
}

static bool read_light(uint32_t& light_intensity)
{
  node.begin(MODBUS_ID_LIGHT, ModbusSerial);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  uint8_t res = node.readInputRegisters(REG_LIGHT_INTENSITY_L, LEN_LIGHT_INTENSITY);
  if (res != node.ku8MBSuccess)
    return false;

  // Giả định: reg0 = high word, reg1 = low word
  uint16_t w0     = node.getResponseBuffer(0);
  uint16_t w1     = node.getResponseBuffer(1);
  light_intensity = make_u32(w0, w1);
  return true;
}

static bool read_rain(uint16_t& rain_status, uint16_t& lower_limit_temp, uint16_t& temp_hysteresis, uint16_t& reset_delay, uint16_t& sensitivity)
{
  node.begin(MODBUS_ID_RAIN, ModbusSerial);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  uint8_t res = node.readInputRegisters(REG_RAIN_STATUS, 1);
  if (res != node.ku8MBSuccess)
    return false;
  rain_status = node.getResponseBuffer(0);

  // đọc các tham số mở rộng (nếu có)
  res = node.readInputRegisters(REG_RAIN_LL, 4);
  if (res == node.ku8MBSuccess)
  {
    lower_limit_temp = node.getResponseBuffer(0);
    temp_hysteresis  = node.getResponseBuffer(1);
    reset_delay      = node.getResponseBuffer(2);
    sensitivity      = node.getResponseBuffer(3);
  }
  else
  {
    lower_limit_temp = temp_hysteresis = reset_delay = sensitivity = 0xFFFF;
  }
  return true;
}

static bool read_tds_ec(float& temperature_c, float& ec_us_cm, float& salinity, float& tds_mg_l, uint16_t& ec_raw)
{
  node.begin(MODBUS_ID_TDS, ModbusSerial);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  // Read temperature
  uint8_t res = node.readInputRegisters(REG_TDS_TEMP, LEN_TDS_TEMP);
  if (res != node.ku8MBSuccess)
    return false;
  uint16_t t_raw = node.getResponseBuffer(0);
  temperature_c  = (float)t_raw / 100.0f;  // giả định /100

  vTaskDelay(pdMS_TO_TICKS(20));  // gap nhỏ giữa requests

  // Read EC/Salinity/TDS
  res = node.readInputRegisters(REG_TDS_EC_SAL_TDS, LEN_TDS_EC_SAL_TDS);
  if (res != node.ku8MBSuccess)
  {
    ec_us_cm = salinity = tds_mg_l = NAN;
  }
  else
  {
    // Giả định mỗi giá trị 16-bit scaled /100
    ec_us_cm = (float)node.getResponseBuffer(0) / 100.0f;
    salinity = (float)node.getResponseBuffer(1) / 100.0f;
    tds_mg_l = (float)node.getResponseBuffer(2) / 100.0f;
  }

  vTaskDelay(pdMS_TO_TICKS(20));
  res = node.readInputRegisters(REG_TDS_ECRAWAD, LEN_TDS_ECRAWAD);
  if (res == node.ku8MBSuccess)
    ec_raw = node.getResponseBuffer(0);
  else
    ec_raw = 0xFFFF;

  return true;
}

/* -------------------------
   Pre / Post transmission (DE control)
   -------------------------*/
static void preTransmission()
{
  digitalWrite(DE_PIN, HIGH);  // enable driver (TX)
  delayMicroseconds(50);       // settle
}

static void postTransmission()
{
  delayMicroseconds(50);
  digitalWrite(DE_PIN, LOW);  // disable driver (RX)
}

/* -------------------------
   Sensor task (RTOS)
   -------------------------*/
static void sensor_task(void* pv)
{
  (void)pv;

  pinMode(DE_PIN, OUTPUT);
  digitalWrite(DE_PIN, LOW);
  ModbusSerial.begin(MODBUS_SERIAL_BAUD, MODBUS_CONFIG, RX_PIN, TX_PIN);

  vTaskDelay(pdMS_TO_TICKS(100));

  while (1)
  {
    // DHT13
    float hum, temp;
    if (read_dht13(hum, temp))
    {
      g_dht13.temperature_c = temp * 10;
      g_dht13.humidity_pct  = hum * 10;
      g_dht13.valid         = true;
    }
    else
    {
      g_dht13.valid = false;
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    // Light
    uint32_t light;
    if (read_light(light))
    {
      g_light.intensity = light;
      g_light.valid     = true;
    }
    else
    {
      g_light.valid = false;
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    // Rain
    uint16_t rs, ll, hyst, rd, sens;
    if (read_rain(rs, ll, hyst, rd, sens))
    {
      g_rain.status      = rs;
      g_rain.lower_limit = ll;
      g_rain.hysteresis  = hyst;
      g_rain.reset_delay = rd;
      g_rain.sensitivity = sens;
      g_rain.valid       = true;
    }
    else
    {
      g_rain.valid = false;
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    // TDS/EC
    float    tt, ec, sal, tds;
    uint16_t raw;
    if (read_tds_ec(tt, ec, sal, tds, raw))
    {
      g_tds.temperature_c = tt;
      g_tds.ec            = ec;
      g_tds.salinity      = sal;
      g_tds.tds           = tds;
      g_tds.ec_raw        = raw;
      g_tds.valid         = true;
    }
    else
    {
      g_tds.valid = false;
    }

    Serial.printf("DHT13[T=%.2f°C H=%.2f%% %s] | LIGHT[%u %s] | RAIN[st=%u ll=%u hy=%u rd=%u sens=%u %s] | TDS[T=%.2f°C EC=%.2f Sal=%.2f TDS=%.2f Raw=%u %s]\n",
                  g_dht13.temperature_c, g_dht13.humidity_pct, g_dht13.valid ? "OK" : "ERR", g_light.intensity, g_light.valid ? "OK" : "ERR", g_rain.status,
                  g_rain.lower_limit, g_rain.hysteresis, g_rain.reset_delay, g_rain.sensitivity, g_rain.valid ? "OK" : "ERR", g_tds.temperature_c, g_tds.ec,
                  g_tds.salinity, g_tds.tds, g_tds.ec_raw, g_tds.valid ? "OK" : "ERR");

    vTaskDelay(pdMS_TO_TICKS(3000 - 3 * 100));
  }
}

/* -------------------------
   Public init
   -------------------------*/
void sensor_init(void)
{
  xTaskCreatePinnedToCore(sensor_task, "sensor_task", 4096, NULL, 1, NULL, 1);
}
