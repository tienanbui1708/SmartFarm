#pragma once

#ifndef __WIFI_H_
#define __WIFI_H_

/* Includes ------------------------------------------------------------------*/
#include <WString.h>
#include <stdint.h>
#include <deque>

/* Define --------------------------------------------------------------------*/
#define WIFI_PERIOD  (2000 / 50)
#define WIFI_CONNECT (10000 / 50)

/* Enum ----------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
extern String mqtt_topic_raw;

extern uint8_t is_wifi_connected;
extern uint8_t is_internet_connected;
extern uint8_t is_mqtt_connected;

/* Functions -----------------------------------------------------------------*/
void wifi_init(void);
void wifi_handler(void);

void wifi_process(void);
void mqtt_process(void);

void wifi_stop_for_ap(void);
#endif