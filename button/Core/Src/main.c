/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// ------------------------ button------------------
uint8_t btn_current = 1;
uint8_t btn_last = 1;
uint8_t btn_filter = 1;
uint8_t is_debouncing;
uint32_t time_deboune;
uint8_t is_press_timout;
uint8_t time_start_press;

// ---------------- var led ---------------
typedef enum{
  LED_OFF = 0,
  LED1_BLINK_1HZ = 1,
  LED2_BLINK_5HZ = 2,
}LedStatus;

LedStatus led_status = LED2_BLINK_5HZ;

uint32_t is_press_timeout;

void btn_pressing_callback(){
  switch (led_status)
  {
    case LED_OFF:
      led_status = LED1_BLINK_1HZ;
    break;
    case LED1_BLINK_1HZ:
      led_status = LED2_BLINK_5HZ;
    break;
    case LED2_BLINK_5HZ:
      led_status = LED1_BLINK_1HZ;
    break;

  }
}

void btn_release_callback(){

}

void btn_press_timeout_callback(){

}

void btn_press_short_callback(){

}
void Button_handler (){
	uint32_t sta = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
	
	if(sta != btn_filter)
		{
			btn_filter = sta;
      is_debouncing = 1;
      time_deboune = HAL_GetTick();
		}

    if(is_debouncing && (HAL_GetTick() - time_deboune >= 15)){
      btn_current = btn_filter;
      is_debouncing = 0;
    }
    if(btn_current != btn_last){
      if(btn_current == 0) // nhan xuong
      {
        is_press_timeout = 1;
        btn_pressing_callback();
        time_start_press = HAL_GetTick();
      }
      else{
        if(HAL_GetTick() - time_start_press <= 1000){
         btn_press_short_callback();
      }
      btn_release_callback();
    }
    btn_last = btn_current;
    }
    // ----------------------- xu ly-----------------
    if(is_press_timeout&& (HAL_GetTick() - time_start_press >= 3000)){
      btn_press_timeout_callback();
      is_press_timeout = 0;
    }
}

void led1_Blink_1Hz(){
  static uint32_t tled1_blink;
  if(HAL_GetTick() - tled1_blink >= 500){
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
    tled1_blink = HAL_GetTick();
  }
}

void led1_Blink_5Hz(){
  static uint32_t tled1_blink;
  if(HAL_GetTick() - tled1_blink >= 100){
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
    tled1_blink = HAL_GetTick();
  }
}

void led_off(){
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, 1);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, 0);
}


void led_handle(){
   switch (led_status)
  {
    case LED_OFF:
     
    break;
    case LED1_BLINK_1HZ:
      
    break;
    case LED2_BLINK_5HZ:
      
    break;

  }
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  Button_handler ();
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  led_status = LED_OFF;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_Delay(1000);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14,GPIO_PIN_SET);
		HAL_Delay(1000);
		
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PD13 PD14 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
