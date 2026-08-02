#pragma once
#ifndef __MEMORY_H_
#define __MEMORY_H_

/* Includes ------------------------------------------------------------------*/
#include <Arduino.h>
 
/* Define --------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
void eeprom_init(void);

uint8_t EepromRead8b(uint16_t _address);
uint16_t EepromRead16b(uint16_t _address);
uint32_t EepromRead32b(uint16_t _address);
float EepromReadFloat(uint16_t _address);
void EepromReadString(uint16_t _addr, char* _dest, uint16_t _maxSize);

void EepromWrite8b(uint16_t _address, uint8_t _data);
void EepromWrite16b(uint16_t _address, uint16_t _data);
void EepromWrite32b(uint16_t _address, uint32_t _data);
void EepromWriteFloat(uint16_t _address, float _data);
void EepromWriteString(uint16_t _addr, char* _source, uint16_t _maxSize);
void EepromCommit(void);
uint8_t CompareStringInEeprom(uint16_t _addr, char* _term);

#endif
