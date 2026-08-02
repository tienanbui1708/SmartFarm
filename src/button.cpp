#include "button.h"
#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Define --------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
uint32_t key_code                  = 0;
uint32_t key_code_before_releasing = 0;

/* Task handles */
TaskHandle_t buttonTaskHandle = NULL;

/* Functions -----------------------------------------------------------------*/
void button_task(void* pvParameters)
{
  unsigned long previousMillis = 0;

  while (1)
  {
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      key_code_before_releasing = 0;
      key_code++;

      if (key_code == 1000)
      {
        key_code = 1000;
      }
    }
    else
    {
      key_code_before_releasing = key_code;
      key_code                  = 0;
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void button_init()
{
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  key_code                  = 0;
  key_code_before_releasing = 0;

  // Create the button task
  xTaskCreate(button_task, "Button Task", 1024, NULL, 2, &buttonTaskHandle);

  Serial.println("but: \t [init]");
}