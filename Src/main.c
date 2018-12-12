#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "main.h"
#include "stm32l0xx_hal.h"
#include "animation.h"
#include "jingles.h"
#include "button.h"

struct led {
  uint8_t index;

  uint16_t brightness;
  uint16_t time_on;

  struct animation anim;
};

struct led_pin_mapping {
  GPIO_TypeDef *port;
  uint16_t      pin;
};

static const struct led_pin_mapping led_map[] = {
  [0]  = { .port = LED_0_GPIO_Port,  .pin = LED_0_Pin  },
  [1]  = { .port = LED_1_GPIO_Port,  .pin = LED_1_Pin  },
  [2]  = { .port = LED_2_GPIO_Port,  .pin = LED_2_Pin  },
  [3]  = { .port = LED_3_GPIO_Port,  .pin = LED_3_Pin  },
  [4]  = { .port = LED_4_GPIO_Port,  .pin = LED_4_Pin  },
  [5]  = { .port = LED_5_GPIO_Port,  .pin = LED_5_Pin  },
  [6]  = { .port = LED_6_GPIO_Port,  .pin = LED_6_Pin  },
  [7]  = { .port = LED_7_GPIO_Port,  .pin = LED_7_Pin  },
  [8]  = { .port = LED_8_GPIO_Port,  .pin = LED_8_Pin  },
  [9]  = { .port = LED_9_GPIO_Port,  .pin = LED_9_Pin  },
  [10] = { .port = LED_10_GPIO_Port, .pin = LED_10_Pin },
  [11] = { .port = LED_11_GPIO_Port, .pin = LED_11_Pin },
  [12] = { .port = LED_12_GPIO_Port, .pin = LED_12_Pin },
  [13] = { .port = LED_13_GPIO_Port, .pin = LED_13_Pin },
  [14] = { .port = LED_14_GPIO_Port, .pin = LED_14_Pin },
  [15] = { .port = LED_15_GPIO_Port, .pin = LED_15_Pin },
  [16] = { .port = LED_16_GPIO_Port, .pin = LED_16_Pin },
  [17] = { .port = LED_17_GPIO_Port, .pin = LED_17_Pin },
  [18] = { .port = LED_18_GPIO_Port, .pin = LED_18_Pin },
  [19] = { .port = LED_19_GPIO_Port, .pin = LED_19_Pin },
};

static struct led leds[NUM_LEDS];

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim21;
UART_HandleTypeDef huart2;

void SystemClock_Config(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM21_Init(void);
static void MX_USART2_UART_Init(void);

int main(void)
{
  enum animations current_anim = ANIM_PULSE;

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* TODO: properly generate random seed */
  srand(0);

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_TIM21_Init();
  MX_USART2_UART_Init();

  /* Initialize all leds to random brightnesses */
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].index = i;
    leds[i].brightness = rand() % (MAX_BRIGHTNESS / 2);
    leds[i].time_on = 0;

    animation_switch_to(&leds[i].anim, current_anim);
  }

  /* Start LED timer */
  HAL_TIM_Base_Start_IT(&htim21);

  /* Start buzzer PWM */
  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
  
  while (1) {
    int duration = button_detect_press();

    if (duration >= BUTTON_LONG_PRESS) {
      if (jingle_is_playing())
        jingle_stop();
      else
        jingle_start(JINGLE_BELLS);
    } else if (duration >= BUTTON_SHORT_PRESS) {
      current_anim = (current_anim + 1) % NUM_ANIMATIONS;

      for (int i = 0; i < NUM_LEDS; i++)
        animation_switch_to(&leds[i].anim, current_anim);
    }
  }
}

void HAL_SYSTICK_Callback(void)
{
  jingle_update();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  static int iter = 0;
  int leds_on = 0;

  if (htim->Instance == TIM21) {
    for (int i = 0; i < NUM_LEDS; i++) {
      struct led *led = &leds[i];

      if (led->time_on < led->brightness && leds_on < MAX_LEDS_ON) {
        led->time_on++;
        leds_on++;

        HAL_GPIO_WritePin(led_map[i].port, led_map[i].pin, GPIO_PIN_SET);
      } else {
        HAL_GPIO_WritePin(led_map[i].port, led_map[i].pin, GPIO_PIN_RESET);
      }
    }
    
    if ((iter = (iter + 1) % MAX_BRIGHTNESS) == 0)
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i].brightness = leds[i].anim.at(HAL_GetTick(), &leds[i].anim.data);
        leds[i].time_on = 0;
      }
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  /* Configure the main internal regulator output voltage */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Initializes the CPU, AHB and APB busses clocks */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    _Error_Handler(__FILE__, __LINE__);

  /* Initializes the CPU, AHB and APB busses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
    _Error_Handler(__FILE__, __LINE__);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    _Error_Handler(__FILE__, __LINE__);

  /* Configure the Systick */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* TIM2 init function */
static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 31;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    _Error_Handler(__FILE__, __LINE__);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
    _Error_Handler(__FILE__, __LINE__);

  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
    _Error_Handler(__FILE__, __LINE__);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    _Error_Handler(__FILE__, __LINE__);

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
    _Error_Handler(__FILE__, __LINE__);

  HAL_TIM_MspPostInit(&htim2);
}

/* TIM21 init function */
static void MX_TIM21_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim21.Instance = TIM21;
  htim21.Init.Prescaler = 31;
  htim21.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim21.Init.Period = 207;
  htim21.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim21) != HAL_OK)
    _Error_Handler(__FILE__, __LINE__);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim21, &sClockSourceConfig) != HAL_OK)
    _Error_Handler(__FILE__, __LINE__);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim21, &sMasterConfig) != HAL_OK)
    _Error_Handler(__FILE__, __LINE__);
}

/* USART2 init function */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
    _Error_Handler(__FILE__, __LINE__);
}

/* Configure pins */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, LED_0_Pin|LED_1_Pin|LED_2_Pin|LED_3_Pin 
                          |LED_4_Pin|LED_5_Pin|LED_6_Pin|LED_7_Pin 
                          |LED_8_Pin|LED_9_Pin|LED_10_Pin|LED_11_Pin 
                          |LED_12_Pin|LED_15_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOB, LED_19_Pin|LED_13_Pin|LED_16_Pin|LED_17_Pin 
                          |LED_18_Pin|LED_14_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = USER_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_BTN_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LED_0_Pin|LED_1_Pin|LED_2_Pin|LED_3_Pin 
                          |LED_4_Pin|LED_5_Pin|LED_6_Pin|LED_7_Pin 
                          |LED_8_Pin|LED_9_Pin|LED_10_Pin|LED_11_Pin 
                          |LED_12_Pin|LED_15_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LED_19_Pin|LED_13_Pin|LED_16_Pin|LED_17_Pin 
                          |LED_18_Pin|LED_14_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  while(1)
    ;
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
