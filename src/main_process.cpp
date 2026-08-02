/* Includes ------------------------------------------------------------------*/
#include "main_process.h"
#include <Arduino.h>

#include "_wifi.h"
#include "ap.h"
#include "button.h"
#include "led.h"
#include "memory.h"
#include "rtc.h"
#include "sensor.h"
#include "variable.h"

/* Variables -----------------------------------------------------------------*/
main_processState main_process_state = INIT;

static main_processState pending_state          = INIT;
static bool              in_transition          = false;
static uint32_t          transition_deadline_ms = 0;

/* RTOS ----------------------------------------------------------------------*/
static void main_process_task(void*);

/* Helper Functions ----------------------------------------------------------*/
static void request_transition(main_processState target, uint32_t cooldown_ms = 250)
{
  if (in_transition || main_process_state == target)
    return;
  in_transition          = true;
  pending_state          = target;
  transition_deadline_ms = millis() + cooldown_ms;
  Serial.printf("main:\t[transition -> %s] cooling down...\n", (target == AP) ? "AP" : (target == NORMAL) ? "NORMAL" : "UNKNOWN");
}

static void process_transition_tick()
{
  if (!in_transition)
    return;
  if ((int32_t)(millis() - transition_deadline_ms) < 0)
    return;

  in_transition      = false;
  main_process_state = pending_state;
  main_process_reset_state(main_process_state);
}

/* Functions -----------------------------------------------------------------*/
void first_program(void)
{
  if (EepromRead16b(address_version) == _version)
  {
    Serial.println("Restoring...");
    restore_id();
    restore_station_code();
    restore_reset_count();
    reset_count = (reset_count + 1) % 65535;
    save_reset_count();
    restore_wifi();
  }
  else
  {
    Serial.println("First program...");
    strcpy(_id, ID_DEFAULT);
    _version = VERSION_DEFAULT;

    wifi_ssid_length     = strlen(WIFI_SSID_DEFAULT);
    wifi_password_length = strlen(WIFI_PASSWORD_DEFAULT);
    strcpy(wifi_ssid, WIFI_SSID_DEFAULT);
    strcpy(wifi_password, WIFI_PASSWORD_DEFAULT);

    update_version();
    save_id();
    save_station_code();
    save_reset_count();
    save_wifi();
  }
}

void init_system(void)
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("-----------");
  Serial.println("INIT SYSTEM");
  Serial.println("-----------");

  variable_init();
  eeprom_init();
  first_program();

  led_init();
  button_init();
  rtc_init();

  sensor_init();

  main_process_init();

  Serial.println("\n");
  Serial.println("-------------");
  Serial.println("START PROGRAM");
  Serial.println("-------------");
}

void main_process_reset_state(main_processState state)
{
  switch (state)
  {
    case AP:
      Serial.println("main:\t[ap init]");

      wifi_stop_for_ap();
      vTaskDelay(pdMS_TO_TICKS(200));

      ap_init();
      led_off();
      break;

    case NORMAL:
      Serial.println("main:\t[normal init]");

      vTaskDelay(pdMS_TO_TICKS(200));

      wifi_init();

      led_toggle_1s();
      break;

    default:
      break;
  }
}

void main_process(void)
{
  process_transition_tick();

  switch (main_process_state)
  {
    case INIT:
      Serial.println("main:\t[init]");
      main_process_state = NORMAL;
      main_process_reset_state(NORMAL);
      break;

    case AP:
      ap_process();

      if (key_code == 20)
      {
        request_transition(NORMAL, 300);
      }
      break;

    case NORMAL:
      wifi_handler();
      mqtt_process();

      if (key_code == 20)
      {
        request_transition(AP, 300);
      }
      break;

    default:
      Serial.println("main:\t[invalid state]");
      break;
  }
}

/* RTOS entry ----------------------------------------------------------------*/
static void main_process_task(void* pv)
{
  (void)pv;
  while (1)
  {
    main_process();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void main_process_init(void)
{
  xTaskCreate(main_process_task, "main_process", 4096 * 2, nullptr, 1, nullptr);
}
