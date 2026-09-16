#ifndef CONFIG_CYD_H
#define CONFIG_CYD_H

// ST7789 CYD: use the full 80 MHz display speed.
// Enable FRAMESKIP only if the display is unstable at 80 MHz.
// #define FRAMESKIP

#include "src/ControllerTypes.h"

// Touchscreen input is handled by the custom CYD touch layer.
// The XPT2046 controller is used by the common ESP32 CYD boards.
#define CYD_TOUCH_ENABLE
#define CYD_TOUCH_CS   33
#define CYD_TOUCH_IRQ  36
#define CYD_TOUCH_CLK  25
#define CYD_TOUCH_MISO 39
#define CYD_TOUCH_MOSI 32

// The custom touchscreen provides the game controls.
#define CONTROLLER_TYPE CT_NC

// Screen Configuration
#define TFT_BACKLIGHT_ENABLE
#define TFT_BACKLIGHT_PIN 21
#define SCREEN_ROTATION   1
#define SCREEN_SWAP_BYTES

// MicroSD card module Pins
#define SD_FREQ                  80000000
#define SD_MOSI_PIN              23
#define SD_MISO_PIN              19
#define SD_SCLK_PIN              18
#define SD_CS_PIN                5
#define SD_SPI_PORT              VSPI

// NES controller pins (available as an optional external controller)
#define CONTROLLER_NES_CLK       22
#define CONTROLLER_NES_LATCH     27
#define CONTROLLER_NES_DATA      35

// Unused button pins on CYD
#define A_BUTTON                 -1
#define B_BUTTON                 -1
#define LEFT_BUTTON              -1
#define RIGHT_BUTTON             -1
#define UP_BUTTON                -1
#define DOWN_BUTTON              -1
#define START_BUTTON             -1
#define SELECT_BUTTON            -1

// Unused SNES controller pins
#define CONTROLLER_SNES_CLK      -1
#define CONTROLLER_SNES_LATCH    -1
#define CONTROLLER_SNES_DATA     -1

// Unused PS1/PS2 controller pins
#define CONTROLLER_PSX_DATA      -1
#define CONTROLLER_PSX_COMMAND   -1
#define CONTROLLER_PSX_ATTENTION -1
#define CONTROLLER_PSX_CLK       -1

// UART controller pins remain available for an external adapter.
#define CONTROLLER_UART_TX       27
#define CONTROLLER_UART_RX       22

// CYD onboard speaker/audio output
#define DAC_PIN                  1

// #define DEBUG
// #define ENABLE_PROFILING

#endif
