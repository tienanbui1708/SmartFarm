/* Includes ------------------------------------------------------------------*/
#include "variable.h"
#include "memory.h"

#include <Arduino.h>
#include <string.h>
#include <cstring>

/* Define --------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
// address
uint16_t address_version     = 0;
uint16_t address_id          = 0;
uint16_t address_reset_count = 0;

uint16_t address_station_code = 0;

uint16_t address_wifi_ssid     = 0;
uint16_t address_wifi_password = 0;

uint16_t address_ap_ssid     = 0;
uint16_t address_ap_password = 0;

uint16_t address_mqtt_host      = 0;
uint16_t address_mqtt_username  = 0;
uint16_t address_mqtt_password  = 0;
uint16_t address_mqtt_topic_sub = 0;
uint16_t address_mqtt_topic_pub = 0;
uint16_t address_mqtt_port      = 0;

// variables
char     _id[64]     = ID_DEFAULT;
uint16_t _version    = VERSION_DEFAULT;
uint16_t reset_count = 0;

char station_code[17] = STATION_CODE;

uint8_t wifi_ssid_length     = strlen(WIFI_SSID_DEFAULT);
uint8_t wifi_password_length = strlen(WIFI_PASSWORD_DEFAULT);
char    wifi_ssid[33]        = WIFI_SSID_DEFAULT;
char    wifi_password[65]    = WIFI_PASSWORD_DEFAULT;

char ap_ssid[33]     = AP_SSID_DEFAULT;
char ap_password[65] = AP_PASSWORD_DEFAULT;

char     mqtt_host[65]      = MQTT_HOST;
char     mqtt_username[65]  = MQTT_USERNAME;
char     mqtt_password[65]  = MQTT_PASSWORD;
char     mqtt_topic_sub[65] = MQTT_TOPIC_SUB;
char     mqtt_topic_pub[65] = MQTT_TOPIC_PUB;
uint32_t mqtt_port          = MQTT_PORT;

/* Functions -----------------------------------------------------------------*/
void set_address(uint16_t* startAddr, uint16_t* sourceAddr, uint16_t numOfAddr, uint16_t sizeOfPara)
{
  uint8_t  _i;
  uint16_t _addr = *startAddr;

  for (_i = 0; _i < numOfAddr; _i++)
  {
    *(sourceAddr + _i) = _addr;
    _addr              = _addr + sizeOfPara;
  }
  *startAddr = _addr;
}

void variable_init(void)
{
  uint16_t _addr = 0x0000;

  set_address(&_addr, &address_version, 1, sizeof(_version));
  set_address(&_addr, &address_id, 1, sizeof(_id));

  set_address(&_addr, &address_reset_count, 1, sizeof(reset_count));

  set_address(&_addr, &address_station_code, 1, sizeof(station_code));

  set_address(&_addr, &address_wifi_ssid, 1, sizeof(wifi_ssid));
  set_address(&_addr, &address_wifi_password, 1, sizeof(wifi_password));

  set_address(&_addr, &address_ap_ssid, 1, sizeof(ap_ssid));
  set_address(&_addr, &address_ap_password, 1, sizeof(ap_password));

  set_address(&_addr, &address_mqtt_host, 1, sizeof(mqtt_host));
  set_address(&_addr, &address_mqtt_username, 1, sizeof(mqtt_username));
  set_address(&_addr, &address_mqtt_password, 1, sizeof(mqtt_password));
  set_address(&_addr, &address_mqtt_topic_sub, 1, sizeof(mqtt_topic_sub));
  set_address(&_addr, &address_mqtt_topic_pub, 1, sizeof(mqtt_topic_pub));
  set_address(&_addr, &address_mqtt_port, 1, sizeof(mqtt_port));
}

void save_wifi(void)
{
  EepromWriteString(address_wifi_ssid, wifi_ssid, sizeof(wifi_ssid));
  EepromWriteString(address_wifi_password, wifi_password, sizeof(wifi_password));
}

void save_ap(void)
{
  EepromWriteString(address_ap_password, ap_password, sizeof(ap_password));
}

void save_mqtt(void)
{
  EepromWriteString(address_mqtt_host, mqtt_host, sizeof(mqtt_host));
  EepromWriteString(address_mqtt_username, mqtt_username, sizeof(mqtt_username));
  EepromWriteString(address_mqtt_password, mqtt_password, sizeof(mqtt_password));
  EepromWriteString(address_mqtt_topic_sub, mqtt_topic_sub, sizeof(mqtt_topic_sub));
  EepromWriteString(address_mqtt_topic_pub, mqtt_topic_pub, sizeof(mqtt_topic_pub));
  EepromWrite32b(address_mqtt_port, mqtt_port);
}

void save_id(void)
{
  EepromWriteString(address_id, _id, sizeof(_id));
}

void save_station_code(void)
{
  EepromWriteString(address_station_code, station_code, sizeof(station_code));
}

void save_reset_count(void)
{
  EepromWrite16b(address_reset_count, reset_count);
}

void update_version(void)
{
  EepromWrite16b(address_version, _version);
}

void restore_wifi(void)
{
  char read_wifi_ssid[32];
  EepromReadString(address_wifi_ssid, read_wifi_ssid, 33);

  char read_wifi_password[64];
  EepromReadString(address_wifi_password, read_wifi_password, 65);

  strcpy(wifi_ssid, read_wifi_ssid);
  strcpy(wifi_password, read_wifi_password);
  wifi_ssid_length     = strlen(wifi_ssid);
  wifi_password_length = strlen(wifi_password);

  Serial.print("res: \t [wifi] ");
  Serial.print(wifi_ssid);
  Serial.print(" (");
  Serial.print(wifi_ssid_length);
  Serial.print(") ");

  Serial.print("- ");
  Serial.print(wifi_password);
  Serial.print(" (");
  Serial.print(wifi_password_length);
  Serial.println(")");
}

void restore_mqtt(void)
{
  char read_mqtt_host[65];
  EepromReadString(address_mqtt_host, read_mqtt_host, 65);

  char read_mqtt_username[65];
  EepromReadString(address_mqtt_username, read_mqtt_username, 65);

  char read_mqtt_password[65];
  EepromReadString(address_mqtt_password, read_mqtt_password, 65);

  char read_mqtt_topic_sub[65];
  EepromReadString(address_mqtt_topic_sub, read_mqtt_topic_sub, 65);

  char read_mqtt_topic_pub[65];
  EepromReadString(address_mqtt_topic_pub, read_mqtt_topic_pub, 65);

  strcpy(mqtt_host, read_mqtt_host);
  strcpy(mqtt_username, read_mqtt_username);
  strcpy(mqtt_password, read_mqtt_password);
  strcpy(mqtt_topic_sub, read_mqtt_topic_sub);
  strcpy(mqtt_topic_pub, read_mqtt_topic_pub);
  mqtt_port = EepromRead32b(address_mqtt_port);

  Serial.print("res: \t [mqtt] ");
  Serial.print(mqtt_host);
  Serial.print(" - ");
  Serial.print(mqtt_username);
  Serial.print(" - ");
  Serial.print(mqtt_password);
  Serial.print(" - ");
  Serial.print(mqtt_topic_sub);
  Serial.print(" - ");
  Serial.print(mqtt_topic_pub);
  Serial.print(" - ");
  Serial.println(mqtt_port);
}

void restore_id(void)
{
  char read_id[sizeof(_id)];
  EepromReadString(address_id, read_id, sizeof(read_id));

  strcpy(_id, read_id);

  Serial.print("res: \t [id] ");
  Serial.print(_id);
  Serial.print(" (");
  Serial.print(strlen(_id));
  Serial.println(")");
}

void restore_reset_count(void)
{
  reset_count = EepromRead16b(address_reset_count);
  Serial.print("res: \t [reset] ");
  Serial.println(reset_count);
}

void restore_station_code(void)
{
  char read_station_code[17];
  EepromReadString(address_station_code, read_station_code, 17);

  strcpy(station_code, read_station_code);

  Serial.print("res: \t [station code] ");
  Serial.print(station_code);
  Serial.print(" (");
  Serial.print(strlen(station_code));
  Serial.println(")");
}

void restore_ap(void)
{
  char read_ap_password[65];
  EepromReadString(address_ap_password, read_ap_password, 65);

  strcpy(ap_password, read_ap_password);

  Serial.print("res: \t [ap password] ");
  Serial.println(ap_password);
}
