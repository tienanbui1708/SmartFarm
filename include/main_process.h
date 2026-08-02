#pragma once

#ifndef __MAINPROCESS_H_
#define __MAINPROCESS_H_

/* Includes ------------------------------------------------------------------*/

/* Define --------------------------------------------------------------------*/
typedef enum MAIN_PROCESS_STATE
{
  INIT,
  AP,
  NORMAL,
} main_processState;

/* Variables -----------------------------------------------------------------*/
extern main_processState main_process_state;

/* Functions -----------------------------------------------------------------*/
void init_system(void);
void my_main(void);
void main_process_reset_state(main_processState state);
void main_process_init(void);

#endif