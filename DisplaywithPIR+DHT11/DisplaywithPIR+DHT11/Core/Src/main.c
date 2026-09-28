/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

#include "fonts.h"
#include "ssd1306.h"
#include <stdio.h>
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
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define DHT11_PORT GPIOA
#define DHT11_PIN  GPIO_PIN_2

static void DHT11_Pin_Output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

static void DHT11_Pin_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;

    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/* Microsecond delay using Cortex-M3 DWT */
static void DHT11_Delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (HAL_RCC_GetHCLKFreq() / 1000000U);

    while ((DWT->CYCCNT - start) < ticks)
    {
    }
}

static void DHT11_DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0;
}

static uint8_t DHT11_WaitForState(GPIO_PinState state,
                                  uint32_t timeout_us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks =
        timeout_us * (HAL_RCC_GetHCLKFreq() / 1000000U);

    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) != state)
    {
        if ((DWT->CYCCNT - start) > ticks)
        {
            return 0;
        }
    }

    return 1;
}

static uint8_t DHT11_Read(uint8_t *temperature,
                          uint8_t *humidity)
{
    uint8_t data[5] = {0};

    /* Send start signal */
    DHT11_Pin_Output();

    HAL_GPIO_WritePin(DHT11_PORT,
                      DHT11_PIN,
                      GPIO_PIN_RESET);

    HAL_Delay(20);

    HAL_GPIO_WritePin(DHT11_PORT,
                      DHT11_PIN,
                      GPIO_PIN_SET);

    DHT11_Delay_us(30);

    /* Release data line */
    DHT11_Pin_Input();

    /* DHT11 response */
    if (!DHT11_WaitForState(GPIO_PIN_RESET, 100))
        return 0;

    if (!DHT11_WaitForState(GPIO_PIN_SET, 100))
        return 0;

    if (!DHT11_WaitForState(GPIO_PIN_RESET, 100))
        return 0;

    /* Read 40 bits */
    for (uint8_t i = 0; i < 40; i++)
    {
        /* Wait for HIGH pulse */
        if (!DHT11_WaitForState(GPIO_PIN_SET, 100))
            return 0;

        uint32_t start = DWT->CYCCNT;

        uint32_t timeout_ticks =
            120U * (HAL_RCC_GetHCLKFreq() / 1000000U);

        while (HAL_GPIO_ReadPin(DHT11_PORT,
                                DHT11_PIN) == GPIO_PIN_SET)
        {
            if ((DWT->CYCCNT - start) > timeout_ticks)
            {
                return 0;
            }
        }

        uint32_t high_ticks = DWT->CYCCNT - start;

        uint32_t high_us =
            high_ticks /
            (HAL_RCC_GetHCLKFreq() / 1000000U);

        data[i / 8] <<= 1;

        /*
         * DHT11:
         * ~26-28 us HIGH = 0
         * ~70 us HIGH    = 1
         */
        if (high_us > 40)
        {
            data[i / 8] |= 1;
        }
    }

    /* Verify checksum */
    if ((uint8_t)(data[0] +
                  data[1] +
                  data[2] +
                  data[3]) != data[4])
    {
        return 0;
    }

    *humidity = data[0];
    *temperature = data[2];

    return 1;
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

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  DHT11_DWT_Init();

  SSD1306_Init();

  SSD1306_Clear();

  SSD1306_GotoXY(0, 0);
  SSD1306_Puts("MOTION", &Font_11x18, 1);

  SSD1306_GotoXY(0, 30);
  SSD1306_Puts("DHT11", &Font_11x18, 1);

  SSD1306_UpdateScreen();

  HAL_Delay(1000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  static uint8_t temperature = 0;
	    static uint8_t humidity = 0;
	    static uint8_t dht11_ok = 0;
	    static uint32_t last_dht_read = 0;

	    /* Read DHT11 every 2 seconds */
	    if ((HAL_GetTick() - last_dht_read) >= 2000)
	    {
	        last_dht_read = HAL_GetTick();

	        dht11_ok = DHT11_Read(&temperature, &humidity);
	    }

	    /* Read PIR */
	    uint8_t motion =
	        (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET);

	    /* Control external LED */
	    if (motion)
	    {
	        HAL_GPIO_WritePin(GPIOA,
	                          GPIO_PIN_1,
	                          GPIO_PIN_SET);
	    }
	    else
	    {
	        HAL_GPIO_WritePin(GPIOA,
	                          GPIO_PIN_1,
	                          GPIO_PIN_RESET);
	    }

	    /* OLED */
	    char line[22];

	    SSD1306_Clear();

	    /* Temperature and humidity */
	    if (dht11_ok)
	    {
	        sprintf(line, "TEMP: %d C", temperature);

	        SSD1306_GotoXY(0, 0);
	        SSD1306_Puts(line, &Font_7x10, 1);

	        sprintf(line, "HUM : %d %%", humidity);

	        SSD1306_GotoXY(0, 14);
	        SSD1306_Puts(line, &Font_7x10, 1);
	    }
	    else
	    {
	        SSD1306_GotoXY(0, 0);
	        SSD1306_Puts("DHT11 ERROR", &Font_7x10, 1);
	    }

	    /* Motion status */
	    SSD1306_GotoXY(0, 30);

	    if (motion)
	    {
	        SSD1306_Puts("MOTION: YES", &Font_7x10, 1);
	    }
	    else
	    {
	        SSD1306_Puts("MOTION: NO", &Font_7x10, 1);
	    }

	    /* LED status */
	    SSD1306_GotoXY(0, 45);

	    if (motion)
	    {
	        SSD1306_Puts("LED: ON", &Font_7x10, 1);
	    }
	    else
	    {
	        SSD1306_Puts("LED: OFF", &Font_7x10, 1);
	    }

	    SSD1306_UpdateScreen();

	    HAL_Delay(200);
    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA2 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
#ifdef USE_FULL_ASSERT
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
