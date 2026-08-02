#pragma once
#ifndef __BUTTON_H_
#define __BUTTON_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Define --------------------------------------------------------------------*/
#define BUTTON_PIN 14 // 0, 14

/* Variables -----------------------------------------------------------------*/
extern uint32_t key_code;
extern uint32_t key_code_before_releasing;

/* Functions -----------------------------------------------------------------*/
void button_init();
void button_process();             // Process button state

#endif