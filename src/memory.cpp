/* Includes ------------------------------------------------------------------*/
#include "memory.h"
#include <EEPROM.h>

/* Define --------------------------------------------------------------------*/
#define EPROM_MEMORY_SIZE 4096

/* Variables -----------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
union FloatUIntConvertor
{
  float    _float;
  uint32_t _uint;
};

void eeprom_init(void)
{
  EEPROM.begin(EPROM_MEMORY_SIZE);
}

uint8_t EepromRead8b(uint16_t _address)
{
  uint8_t _data;

  _data = ~EEPROM.read(_address);
  return _data;
}

uint16_t EepromRead16b(uint16_t _address)
{
  uint16_t _data;

  _data = EepromRead8b(_address);
  _data = (_data << 8) | EepromRead8b(_address + 1);
  return _data;
}

uint32_t EepromRead32b(uint16_t _address)
{
  uint32_t _data;

  _data = EepromRead16b(_address);
  _data = (_data << 16) | EepromRead16b(_address + 2);
  return _data;
}

float EepromReadFloat(uint16_t _address)
{
  union FloatUIntConvertor _fic;
  uint32_t                 _data;

  _data      = EepromRead16b(_address);
  _data      = (_data << 16) | EepromRead16b(_address + 2);
  _fic._uint = _data;
  return _fic._float;
}

/*
_maxSize is maximun of bytes to read from EEPROM
*/
void EepromReadString(uint16_t _addr, char* _dest, uint16_t _maxSize)
{
  uint16_t _i;

  if (_maxSize == 0)
    return;

  for (_i = 0; _i < _maxSize; _i++)
  {
    _dest[_i] = EepromRead8b(_addr + _i);
    if (_dest[_i] == '\0')
      break;  // Break loop when end of string
  }
  // End of string
  if (_i == _maxSize)
    _dest[_i - 1] = '\0';
}

void EepromWrite8b(uint16_t _address, uint8_t _data)
{
  EEPROM.write(_address, ~_data);
  EepromCommit();
}

void EepromWrite16b(uint16_t _address, uint16_t _data)
{
  _data = ~_data;
  // Send MSB
  EEPROM.write(_address, (_data & 0xFF00) >> 8);
  // Send LSB
  EEPROM.write(_address + 1, _data & 0x00FF);

  EepromCommit();
}

void EepromWrite32b(uint16_t _address, uint32_t _data)
{
  _data = ~_data;

  // Send MSB
  EEPROM.write(_address, (_data & 0xFF000000) >> 24);

  EEPROM.write(_address + 1, (_data & 0x00FF0000) >> 16);
  EEPROM.write(_address + 2, (_data & 0x0000FF00) >> 8);

  // Send LSB
  EEPROM.write(_address + 3, _data & 0x000000FF);

  EepromCommit();
}

void EepromWriteFloat(uint16_t _address, float _data)
{
  union FloatUIntConvertor _fic;

  _fic._float = _data;
  EepromWrite32b(_address, _fic._uint);
}

void EepromCommit(void)
{
  EEPROM.commit();
}

/*
_maxSize is maximun of bytes to write into EEPROM
*/
void EepromWriteString(uint16_t _addr, char* _source, uint16_t _maxSize)
{
  uint16_t _i;

  if (_maxSize == 0)
    return;

  for (_i = 0; _i < _maxSize; _i++)
  {
    EEPROM.write(_addr + _i, ~_source[_i]);
    if (_source[_i] == '\0')
      break;  // Break loop when end of string
  }
  // End of string
  if (_i == _maxSize)
    EEPROM.write(_addr + _i - 1, ~'\0');

  EEPROM.commit();
}

uint8_t CompareStringInEeprom(uint16_t _addr, char* _term)
{
  char _cont[100];

  EepromReadString(_addr, _cont, sizeof(_cont));
  if (strcmp(_cont, _term) == 0)
    return 1;

  return 0;
}
