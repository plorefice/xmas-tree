#include <stdint.h>
#include <stdbool.h>
#include "stm32l0xx_hal.h"

#include "button.h"

#define DEBOUNCE_DURATION 100
#define POLL_INTERVAL     50

static bool previous_value = false;
static uint32_t debounce = 0;

static bool button_is_pressed(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  uint32_t counter = 0;

  if (HAL_GetTick() - debounce < DEBOUNCE_DURATION)
    return previous_value;

  GPIO_InitStruct.Pin = USER_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_BTN_GPIO_Port, &GPIO_InitStruct);

  HAL_GPIO_WritePin(USER_BTN_GPIO_Port, USER_BTN_Pin, GPIO_PIN_SET);

  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  HAL_GPIO_Init(USER_BTN_GPIO_Port, &GPIO_InitStruct);

  while (HAL_GPIO_ReadPin(USER_BTN_GPIO_Port, USER_BTN_Pin) == GPIO_PIN_SET)
    counter++;

  previous_value = counter > 15;
  debounce = HAL_GetTick();

  return previous_value;
}

int button_detect_press(void)
{
  /* Prevent spurious activations from previous pression */
  while (button_is_pressed())
    HAL_Delay(POLL_INTERVAL);

  while (!button_is_pressed())
    HAL_Delay(POLL_INTERVAL);

  uint32_t start = HAL_GetTick();

  while (button_is_pressed() && HAL_GetTick() - start < BUTTON_LONG_PRESS)
    HAL_Delay(POLL_INTERVAL);

  return HAL_GetTick() - start;
}
