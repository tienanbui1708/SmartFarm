#pragma once

#ifndef __AP_H_
#define __AP_H_

/* Includes ------------------------------------------------------------------*/

/* Define --------------------------------------------------------------------*/
#define AP_TIMEOUT (3 * 60 * 1000)  // Set timeout to 10 minutes

/* Variables -----------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
void ap_init(void);
void ap_process(void);
void ap_stop(void);
void ap_request_exit_to_normal();  // add

#endif