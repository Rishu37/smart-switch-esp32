/*
 * relay_config.h - the ONLY file you edit to change the product variant.
 *
 * 1. Set RELAY_VARIANT to 2, 4 or 8.
 * 2. Set RELAY_ON_LEVEL: 1 for active-high relay boards, 0 for active-low.
 * 3. Edit the names / GPIO lists below if you want different ones.
 *
 * GPIO notes (ESP32-WROOM-32):
 *  - Do NOT use GPIO 0, 2, 12, 15 (boot pins), 6-11 (flash), 34-39 (input only).
 *  - GPIO 19 is avoided because the stock example driver may use it.
 *  - GPIO 16/17 are fine on WROOM-32, but are used by PSRAM on WROVER modules.
 */
#pragma once

#include <driver/gpio.h>

#ifndef RELAY_VARIANT
#define RELAY_VARIANT 4          /* 2, 4 or 8 */
#endif

#define RELAY_ON_LEVEL 1         /* 1 = relay ON when GPIO is HIGH, 0 = active-low board */
#define RESTORE_LAST_STATE 1     /* 1 = remember relay states across power cuts, 0 = always start OFF */

#if RELAY_VARIANT == 2
  #define NUM_RELAYS 2
  #define RELAY_GPIO_LIST { GPIO_NUM_16, GPIO_NUM_17 }
  #define RELAY_NAME_LIST { "Light", "Fan" }

#elif RELAY_VARIANT == 4
  #define NUM_RELAYS 4
  #define RELAY_GPIO_LIST { GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_18, GPIO_NUM_21 }
  #define RELAY_NAME_LIST { "Switch 1", "Switch 2", "Fan", "Light" }

#elif RELAY_VARIANT == 8
  #define NUM_RELAYS 8
  #define RELAY_GPIO_LIST { GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_18, GPIO_NUM_21, \
                            GPIO_NUM_22, GPIO_NUM_23, GPIO_NUM_25, GPIO_NUM_26 }
  #define RELAY_NAME_LIST { "Switch 1", "Switch 2", "Switch 3", "Switch 4", \
                            "Switch 5", "Switch 6", "Fan", "Light" }

#else
  #error "RELAY_VARIANT must be 2, 4 or 8"
#endif
