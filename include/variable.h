#pragma once
#ifndef __VARIABLE_H_
#define __VARIABLE_H_

/* Includes ------------------------------------------------------------------*/
#include <Arduino.h>

/* Define --------------------------------------------------------------------*/
// information
#define PRODUCT_NAME     "Smart Farm"
#define PRODUCT_MODEL    "SF_20250915"
#define FIRMWARE_VERSION "1"
#define BUILD_NUMBER     "D20240404.0404.1-D"

// device setting
#define VERSION_DEFAULT 36

#define DEVICE_NAME  "Smart Farm"
#define ID_DEFAULT   "SF_TOMOCHAN_0001"
#define STATION_CODE "SF1"

// wifi setting
#define WIFI_SSID_DEFAULT     "393B_Home_1"
#define WIFI_PASSWORD_DEFAULT "1234567890"

// mqtt setting
#define MQTT_HOST      "mqtt.abcsolutions.com.vn"
#define MQTT_USERNAME  "abcsolution"
#define MQTT_PASSWORD  "CseLAbC5c6"
#define MQTT_TOPIC_SUB "young/smartFarm/subscriber_test"
#define MQTT_TOPIC_PUB "young/smartFarm/publish_test"
#define MQTT_PORT      1883

// ap setting
#define AP_SSID_DEFAULT     "AP-ESP32"
#define AP_PASSWORD_DEFAULT "12345678"
#define AP_ADMIN_USERNAME   "admin"
#define AP_ADMIN_PASSWORD   "adminisme"

/* Variables -----------------------------------------------------------------*/
// address
extern uint16_t address_version;
extern uint16_t address_id;
extern uint16_t address_reset_count;

extern uint16_t address_station_code;

extern uint16_t address_wifi_ssid;
extern uint16_t address_wifi_password;

extern uint16_t address_ap_id;
extern uint16_t address_ap_password;

extern uint16_t address_mqtt_host;
extern uint16_t address_mqtt_username;
extern uint16_t address_mqtt_password;
extern uint16_t address_mqtt_topic_sub;
extern uint16_t address_mqtt_topic_pub;
extern uint16_t address_mqtt_port;

// variables
extern char     _id[64];  // xx:xx:xx:xxxx
extern uint16_t _version;
extern uint16_t reset_count;

extern char station_code[17];

extern uint8_t wifi_ssid_length;
extern uint8_t wifi_password_length;
extern char    wifi_ssid[33];
extern char    wifi_password[65];

extern char ap_ssid[33];
extern char ap_password[65];

extern char     mqtt_host[65];
extern char     mqtt_username[65];
extern char     mqtt_password[65];
extern char     mqtt_topic_sub[65];
extern char     mqtt_topic_pub[65];
extern uint32_t mqtt_port;

/* Functions -----------------------------------------------------------------*/
void variable_init(void);

void save_wifi(void);
void save_mqtt(void);
void save_id(void);
void save_station_code(void);
void save_reset_count(void);
void save_ap(void);

void restore_wifi(void);
void restore_mqtt(void);
void restore_id(void);
void restore_station_code(void);
void restore_reset_count(void);
void restore_ap(void);

void update_version(void);

#endif
