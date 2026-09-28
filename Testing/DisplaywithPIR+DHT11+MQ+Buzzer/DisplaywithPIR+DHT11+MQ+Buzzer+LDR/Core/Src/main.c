/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : PIR + DHT11 + MQ-2 + LDR + Buzzer + OLED
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

#define DHT11_PORT          GPIOA
#define DHT11_PIN           GPIO_PIN_2

/* MQ-2 alarm threshold */
#define GAS_THRESHOLD       2500U

/* LDR */
#define LDR_PORT            GPIOA
#define LDR_PIN             GPIO_PIN_5

/* LDR dark threshold */
#define LDR_DARK_THRESHOLD  1200U

/* USER CODE END PD */


/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */


/* Private variables ---------------------------------------------------------*/

ADC_HandleTypeDef hadc1;
I2C_HandleTypeDef hi2c1;


/* USER CODE BEGIN PV */

/* USER CODE END PV */


/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);


/* USER CODE BEGIN PFP */

/* DHT11 */
static void DHT11_Pin_Output(void);
static void DHT11_Pin_Input(void);
static void DHT11_Delay_us(uint32_t us);
static void DHT11_DWT_Init(void);

static uint8_t DHT11_WaitForState(
    GPIO_PinState state,
    uint32_t timeout_us
);

static uint8_t DHT11_Read(
    uint8_t *temperature,
    uint8_t *humidity
);


/* ADC sensors */
static uint32_t MQ2_Read(void);
static uint32_t LDR_Read(void);


/* USER CODE END PFP */


/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/* ============================================================
   DHT11 FUNCTIONS
   ============================================================ */

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


/* ============================================================
   Microsecond delay using Cortex-M3 DWT
   ============================================================ */

static void DHT11_Delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;

    uint32_t ticks =
        us * (HAL_RCC_GetHCLKFreq() / 1000000U);

    while ((DWT->CYCCNT - start) < ticks)
    {
    }
}


/* ============================================================
   Initialize DWT
   ============================================================ */

static void DHT11_DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    DWT->CYCCNT = 0;
}


/* ============================================================
   Wait for DHT11 pin state
   ============================================================ */

static uint8_t DHT11_WaitForState(
    GPIO_PinState state,
    uint32_t timeout_us)
{
    uint32_t start = DWT->CYCCNT;

    uint32_t ticks =
        timeout_us *
        (HAL_RCC_GetHCLKFreq() / 1000000U);

    while (
        HAL_GPIO_ReadPin(
            DHT11_PORT,
            DHT11_PIN
        ) != state)
    {
        if ((DWT->CYCCNT - start) > ticks)
        {
            return 0;
        }
    }

    return 1;
}


/* ============================================================
   DHT11 READ
   ============================================================ */

static uint8_t DHT11_Read(
    uint8_t *temperature,
    uint8_t *humidity)
{
    uint8_t data[5] = {0};


    /* Send start signal */

    DHT11_Pin_Output();

    HAL_GPIO_WritePin(
        DHT11_PORT,
        DHT11_PIN,
        GPIO_PIN_RESET
    );

    HAL_Delay(20);


    HAL_GPIO_WritePin(
        DHT11_PORT,
        DHT11_PIN,
        GPIO_PIN_SET
    );

    DHT11_Delay_us(30);


    /* Release data line */

    DHT11_Pin_Input();


    /* DHT11 response */

    if (!DHT11_WaitForState(
            GPIO_PIN_RESET,
            100))
    {
        return 0;
    }


    if (!DHT11_WaitForState(
            GPIO_PIN_SET,
            100))
    {
        return 0;
    }


    if (!DHT11_WaitForState(
            GPIO_PIN_RESET,
            100))
    {
        return 0;
    }


    /* Read 40 bits */

    for (uint8_t i = 0; i < 40; i++)
    {
        /* Wait for HIGH pulse */

        if (!DHT11_WaitForState(
                GPIO_PIN_SET,
                100))
        {
            return 0;
        }


        uint32_t start = DWT->CYCCNT;

        uint32_t timeout_ticks =
            120U *
            (HAL_RCC_GetHCLKFreq() / 1000000U);


        while (
            HAL_GPIO_ReadPin(
                DHT11_PORT,
                DHT11_PIN
            ) == GPIO_PIN_SET)
        {
            if ((DWT->CYCCNT - start) >
                timeout_ticks)
            {
                return 0;
            }
        }


        uint32_t high_ticks =
            DWT->CYCCNT - start;


        uint32_t high_us =
            high_ticks /
            (HAL_RCC_GetHCLKFreq() / 1000000U);


        data[i / 8] <<= 1;


        /*
         * DHT11:
         *
         * ~26-28 us HIGH = 0
         * ~70 us HIGH    = 1
         */

        if (high_us > 40)
        {
            data[i / 8] |= 1;
        }
    }


    /* Verify checksum */

    if (
        (uint8_t)(
            data[0] +
            data[1] +
            data[2] +
            data[3]
        ) != data[4])
    {
        return 0;
    }


    *humidity = data[0];

    *temperature = data[2];


    return 1;
}


/* ============================================================
   MQ-2 ADC FUNCTION
   PA3 = ADC1_IN3
   ============================================================ */

static uint32_t MQ2_Read(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    uint32_t adc_value = 0;


    sConfig.Channel =
        ADC_CHANNEL_3;

    sConfig.Rank =
        ADC_REGULAR_RANK_1;

    sConfig.SamplingTime =
        ADC_SAMPLETIME_239CYCLES_5;


    if (
        HAL_ADC_ConfigChannel(
            &hadc1,
            &sConfig
        ) != HAL_OK)
    {
        Error_Handler();
    }


    HAL_ADC_Start(&hadc1);


    if (
        HAL_ADC_PollForConversion(
            &hadc1,
            100
        ) == HAL_OK)
    {
        adc_value =
            HAL_ADC_GetValue(&hadc1);
    }


    HAL_ADC_Stop(&hadc1);


    return adc_value;
}


/* ============================================================
   LDR ADC FUNCTION
   PA5 = ADC1_IN5
   ============================================================ */

static uint32_t LDR_Read(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    uint32_t adc_value = 0;


    sConfig.Channel =
        ADC_CHANNEL_5;

    sConfig.Rank =
        ADC_REGULAR_RANK_1;

    sConfig.SamplingTime =
        ADC_SAMPLETIME_239CYCLES_5;


    if (
        HAL_ADC_ConfigChannel(
            &hadc1,
            &sConfig
        ) != HAL_OK)
    {
        Error_Handler();
    }


    HAL_ADC_Start(&hadc1);


    if (
        HAL_ADC_PollForConversion(
            &hadc1,
            100
        ) == HAL_OK)
    {
        adc_value =
            HAL_ADC_GetValue(&hadc1);
    }


    HAL_ADC_Stop(&hadc1);


    return adc_value;
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


    /* MCU Configuration */

    HAL_Init();


    /* Configure the system clock */

    SystemClock_Config();


    /* Initialize all configured peripherals */

    MX_GPIO_Init();

    MX_ADC1_Init();

    MX_I2C1_Init();


    /* USER CODE BEGIN 2 */

    /* Initialize DWT */

    DHT11_DWT_Init();


    /* ADC calibration */

    HAL_ADCEx_Calibration_Start(
        &hadc1
    );


    /* Initialize OLED */

    SSD1306_Init();

    SSD1306_Clear();


    SSD1306_GotoXY(0, 0);

    SSD1306_Puts(
        "SMART SENSOR",
        &Font_7x10,
        1
    );


    SSD1306_GotoXY(0, 15);

    SSD1306_Puts(
        "PIR+DHT11+MQ2",
        &Font_7x10,
        1
    );


    SSD1306_GotoXY(0, 30);

    SSD1306_Puts(
        "LDR+BUZZER",
        &Font_7x10,
        1
    );


    SSD1306_UpdateScreen();

    HAL_Delay(1500);


    /* USER CODE END 2 */


    /* Infinite loop */

    /* USER CODE BEGIN WHILE */

    while (1)
    {
        /* ====================================================
           DHT11 VARIABLES
           ==================================================== */

        static uint8_t temperature = 0;

        static uint8_t humidity = 0;

        static uint8_t dht11_ok = 0;

        static uint32_t last_dht_read = 0;


        /* ====================================================
           SENSOR VARIABLES
           ==================================================== */

        uint32_t mq2_value;

        uint32_t ldr_value;

        uint8_t dark_condition;


        /* ====================================================
           DHT11
           ==================================================== */

        if (
            (HAL_GetTick() - last_dht_read)
            >= 2000
        )
        {
            last_dht_read =
                HAL_GetTick();


            dht11_ok =
                DHT11_Read(
                    &temperature,
                    &humidity
                );
        }


        /* ====================================================
           MQ-2
           ==================================================== */

        mq2_value =
            MQ2_Read();


        /* ====================================================
           LDR
           ==================================================== */

        ldr_value =
            LDR_Read();


        /*
         * LDR threshold.
         *
         * With the current wiring we assume:
         * lower ADC value = darker.
         *
         * If your module behaves opposite,
         * change < to > here.
         */

        dark_condition =
            (ldr_value < LDR_DARK_THRESHOLD);


        /* ====================================================
           PIR
           ==================================================== */

        uint8_t motion =
            (
                HAL_GPIO_ReadPin(
                    GPIOA,
                    GPIO_PIN_0
                ) == GPIO_PIN_SET
            );


        /* ====================================================
           EXTERNAL LED
           ==================================================== */

        if (motion)
        {
            HAL_GPIO_WritePin(
                GPIOA,
                GPIO_PIN_1,
                GPIO_PIN_SET
            );
        }
        else
        {
            HAL_GPIO_WritePin(
                GPIOA,
                GPIO_PIN_1,
                GPIO_PIN_RESET
            );
        }


        /* ====================================================
           BUZZER

           ON when:
           1. Motion detected
           OR
           2. Gas exceeds threshold
           OR
           3. Environment is dark
           ==================================================== */

        if (
            motion ||
            (mq2_value > GAS_THRESHOLD) ||
            dark_condition
        )
        {
            HAL_GPIO_WritePin(
                GPIOA,
                GPIO_PIN_4,
                GPIO_PIN_SET
            );
        }
        else
        {
            HAL_GPIO_WritePin(
                GPIOA,
                GPIO_PIN_4,
                GPIO_PIN_RESET
            );
        }


        /* ====================================================
           OLED
           ==================================================== */

        char line[22];


        SSD1306_Clear();


        /* ----------------------------------------------------
           Temperature
           ---------------------------------------------------- */

        if (dht11_ok)
        {
            sprintf(
                line,
                "TEMP: %d C",
                temperature
            );

            SSD1306_GotoXY(0, 0);

            SSD1306_Puts(
                line,
                &Font_7x10,
                1
            );


            /* Humidity */

            sprintf(
                line,
                "HUM : %d %%",
                humidity
            );

            SSD1306_GotoXY(0, 10);

            SSD1306_Puts(
                line,
                &Font_7x10,
                1
            );
        }
        else
        {
            SSD1306_GotoXY(0, 0);

            SSD1306_Puts(
                "DHT11 ERROR",
                &Font_7x10,
                1
            );


            SSD1306_GotoXY(0, 10);

            SSD1306_Puts(
                "CHECK SENSOR",
                &Font_7x10,
                1
            );
        }


        /* ----------------------------------------------------
           Motion
           ---------------------------------------------------- */

        SSD1306_GotoXY(0, 20);

        if (motion)
        {
            SSD1306_Puts(
                "MOTION: YES",
                &Font_7x10,
                1
            );
        }
        else
        {
            SSD1306_Puts(
                "MOTION: NO",
                &Font_7x10,
                1
            );
        }


        /* ----------------------------------------------------
           Gas
           ---------------------------------------------------- */

        sprintf(
            line,
            "GAS: %lu",
            mq2_value
        );

        SSD1306_GotoXY(0, 30);

        SSD1306_Puts(
            line,
            &Font_7x10,
            1
        );


        /* ----------------------------------------------------
           LDR
           ---------------------------------------------------- */

        sprintf(
            line,
            "LDR: %lu",
            ldr_value
        );

        SSD1306_GotoXY(0, 40);

        SSD1306_Puts(
            line,
            &Font_7x10,
            1
        );


        /* ----------------------------------------------------
           Buzzer / status
           ---------------------------------------------------- */

        SSD1306_GotoXY(0, 50);

        if (
            motion ||
            (mq2_value > GAS_THRESHOLD) ||
            dark_condition
        )
        {
            SSD1306_Puts(
                "ALERT: BUZZ ON",
                &Font_7x10,
                1
            );
        }
        else
        {
            SSD1306_Puts(
                "STATUS: NORMAL",
                &Font_7x10,
                1
            );
        }


        SSD1306_UpdateScreen();


        /* Small loop delay */

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

    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};


    /* HSE + PLL */

    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSE;

    RCC_OscInitStruct.HSEState =
        RCC_HSE_ON;

    RCC_OscInitStruct.HSEPredivValue =
        RCC_HSE_PREDIV_DIV1;

    RCC_OscInitStruct.HSIState =
        RCC_HSI_ON;

    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_ON;

    RCC_OscInitStruct.PLL.PLLSource =
        RCC_PLLSOURCE_HSE;

    RCC_OscInitStruct.PLL.PLLMUL =
        RCC_PLL_MUL9;


    if (
        HAL_RCC_OscConfig(
            &RCC_OscInitStruct
        ) != HAL_OK)
    {
        Error_Handler();
    }


    /* CPU / AHB / APB clocks */

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;


    RCC_ClkInitStruct.SYSCLKSource =
        RCC_SYSCLKSOURCE_PLLCLK;


    RCC_ClkInitStruct.AHBCLKDivider =
        RCC_SYSCLK_DIV1;


    RCC_ClkInitStruct.APB1CLKDivider =
        RCC_HCLK_DIV2;


    RCC_ClkInitStruct.APB2CLKDivider =
        RCC_HCLK_DIV1;


    if (
        HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct,
            FLASH_LATENCY_2
        ) != HAL_OK)
    {
        Error_Handler();
    }


    /* ADC clock:
       72 MHz / 6 = 12 MHz */

    PeriphClkInit.PeriphClockSelection =
        RCC_PERIPHCLK_ADC;


    PeriphClkInit.AdcClockSelection =
        RCC_ADCPCLK2_DIV6;


    if (
        HAL_RCCEx_PeriphCLKConfig(
            &PeriphClkInit
        ) != HAL_OK)
    {
        Error_Handler();
    }
}


/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */

static void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};


    /* ADC1 */

    hadc1.Instance = ADC1;


    hadc1.Init.ScanConvMode =
        ADC_SCAN_DISABLE;


    hadc1.Init.ContinuousConvMode =
        DISABLE;


    hadc1.Init.DiscontinuousConvMode =
        DISABLE;


    hadc1.Init.ExternalTrigConv =
        ADC_SOFTWARE_START;


    hadc1.Init.DataAlign =
        ADC_DATAALIGN_RIGHT;


    hadc1.Init.NbrOfConversion =
        1;


    if (
        HAL_ADC_Init(&hadc1)
        != HAL_OK)
    {
        Error_Handler();
    }


    /*
     * Initial ADC channel.
     *
     * MQ-2 = PA3 = ADC1_IN3
     *
     * The LDR function later changes
     * the ADC channel to ADC1_IN5.
     */

    sConfig.Channel =
        ADC_CHANNEL_3;

    sConfig.Rank =
        ADC_REGULAR_RANK_1;

    sConfig.SamplingTime =
        ADC_SAMPLETIME_239CYCLES_5;


    if (
        HAL_ADC_ConfigChannel(
            &hadc1,
            &sConfig
        ) != HAL_OK)
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
    hi2c1.Instance = I2C1;


    hi2c1.Init.ClockSpeed =
        400000;


    hi2c1.Init.DutyCycle =
        I2C_DUTYCYCLE_2;


    hi2c1.Init.OwnAddress1 =
        0;


    hi2c1.Init.AddressingMode =
        I2C_ADDRESSINGMODE_7BIT;


    hi2c1.Init.DualAddressMode =
        I2C_DUALADDRESS_DISABLE;


    hi2c1.Init.OwnAddress2 =
        0;


    hi2c1.Init.GeneralCallMode =
        I2C_GENERALCALL_DISABLE;


    hi2c1.Init.NoStretchMode =
        I2C_NOSTRETCH_DISABLE;


    if (
        HAL_I2C_Init(&hi2c1)
        != HAL_OK)
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


    /* GPIO clocks */

    __HAL_RCC_GPIOD_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();


    /*
     * Initial state:
     *
     * PA1 = LED OFF
     * PA2 = DHT11 LOW initially
     * PA4 = BUZZER OFF
     */

    HAL_GPIO_WritePin(
        GPIOA,
        GPIO_PIN_1 |
        GPIO_PIN_2 |
        GPIO_PIN_4,
        GPIO_PIN_RESET
    );


    /* --------------------------------------------------------
       PA0 = PIR input
       -------------------------------------------------------- */

    GPIO_InitStruct.Pin =
        GPIO_PIN_0;

    GPIO_InitStruct.Mode =
        GPIO_MODE_INPUT;

    GPIO_InitStruct.Pull =
        GPIO_NOPULL;

    HAL_GPIO_Init(
        GPIOA,
        &GPIO_InitStruct
    );


    /* --------------------------------------------------------
       PA1 = LED
       PA2 = DHT11
       PA4 = BUZZER
       -------------------------------------------------------- */

    GPIO_InitStruct.Pin =
        GPIO_PIN_1 |
        GPIO_PIN_2 |
        GPIO_PIN_4;

    GPIO_InitStruct.Mode =
        GPIO_MODE_OUTPUT_PP;

    GPIO_InitStruct.Pull =
        GPIO_NOPULL;

    GPIO_InitStruct.Speed =
        GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(
        GPIOA,
        &GPIO_InitStruct
    );


    /*
     * PA3 and PA5 are ADC pins.
     *
     * CubeMX configures them as analog inputs.
     *
     * PA3 = MQ-2
     * PA5 = LDR
     */
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */

void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}


#ifdef USE_FULL_ASSERT

void assert_failed(
    uint8_t *file,
    uint32_t line)
{
    /* USER CODE BEGIN 6 */

    /* USER CODE END 6 */
}

#endif
