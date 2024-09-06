#include "main.h"
#include "stm32f1xx_hal.h" // Include the main HAL header file which includes stm32f1xx_hal_gpio.h
#include "stdint.h" // Include the necessary header file for uint16_t

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stdio.h"

typedef enum{
    HC04_Idle_State,
    HC04_Wait_rising_State,
    HC04_Wait_falling_State,
    HC04_Complete_State,
}HC04_State;

HC04_State HC04_Current_State = HC04_Idle_State; // Initial state

// Function to start the HC04 sensor
void HC04_start()
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
    hal_delay(1);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
    HC04_Current_State = HC04_Wait_rising_State;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch(HC04_Current_State)
    {
        case HC04_Wait_rising_State:
            if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == 1)
            {
                htim1.Instance -> CNT = 0;
                HC04_Current_State = HC04_Wait_falling_State;
                Hal_tim_base_start(&htim1);
            }
            else
            {
                HC04_Current_State = HC04_Idle_State;
            }
            break;
        case HC04_Wait_falling_State:
            if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == 0)
            {
                Hal_tim_base_stop(&htim1);
                HC04_Current_State = HC04_Complete_State;
            }
            else
            {
                HC04_Current_State = HC04_Idle_State;
            }
            break;
        default:
            break;
    }
}
void HC04_Handle()
{
    if(HC04_Current_State == HC04_Complete_State)
    {
        uint32_t time = htim1.Instance -> CNT;
        float distance = time * 0.034 / 2;
        HC04_Complete_Callback(distance);
        HC04_Current_State = HC04_Idle_State;
    }
}
void HC04_Complete_Callback(float distance)
{
    char buffer[50];
    sprintf(buffer, "Distance: %f cm\n", distance);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 1000);
}

int main(){

    while(1){
        uint32_t t_get_distance;
        if(HAL_GetTick() - t_get_distance >= 300)
        {
            HC04_start();
           // t_get_distance = HAL_GetTick();
        }
        HC04_Handle();
    }

    return 0;
}