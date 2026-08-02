#pragma once

#ifndef __RTC_H_
#define __RTC_H_

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/* Define --------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
extern bool     rtc_state;

extern uint16_t rtc_year;
extern uint8_t  rtc_month, rtc_day;
extern uint8_t  rtc_hour, rtc_minute, rtc_second;

/* Functions -----------------------------------------------------------------*/
void rtc_init(void);

#endif