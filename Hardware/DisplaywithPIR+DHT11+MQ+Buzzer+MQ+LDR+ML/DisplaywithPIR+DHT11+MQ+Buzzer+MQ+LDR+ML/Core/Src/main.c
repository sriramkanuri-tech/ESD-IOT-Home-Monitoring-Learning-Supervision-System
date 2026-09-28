/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : HomeMonitoringSystem - PIR, DHT11, MQ-2, LDR DO,
  *                   Buzzer, SSD1306 OLED and compact TinyML classifier.
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"

/* USER CODE BEGIN Includes */
#include "fonts.h"
#include "ssd1306.h"
#include <stdio.h>
/* USER CODE END Includes */

/* USER CODE BEGIN PD */

#define DHT11_PORT              GPIOA
#define DHT11_PIN               GPIO_PIN_2

/* MQ-2 is connected to PA3 / ADC1_IN3. */

/*
 * MQ-2 thresholds are raw STM32 12-bit ADC readings.
 *
 * Initial values only. Tune after observing the normal-air
 * "MQ2 RAW" reading on OLED Page 2.
 */
#define GAS_WARNING_LEVEL       1200U
#define GAS_THRESHOLD           1600U

/* MQ-2 averaging and smoothing. */
#define MQ2_SAMPLE_COUNT        8U
#define MQ2_FILTER_SHIFT        2U

/* Display and sensor scheduling. */
#define DHT_READ_INTERVAL_MS    2000U
#define SENSOR_READ_INTERVAL_MS 200U
#define PAGE_INTERVAL_MS        1000U

/* PIR requires time to stabilize after power-up. */
#define PIR_WARMUP_TIME_MS      60000U

/* TinyML safety-classifier configuration. */
#define ML_ADC_MAX              4095.0f
#define ML_TEMP_NORMAL          25.0f
#define ML_TEMP_WARNING         35.0f
#define ML_TEMP_DANGER          45.0f

typedef enum
{
    ML_SAFE = 0,
    ML_WATCH,
    ML_GAS_LEAK,
    ML_FIRE_RISK,
    ML_INTRUDER
} ML_Class;

typedef struct
{
    ML_Class prediction;
    float confidence;
    float safe_score;
    float watch_score;
    float gas_score;
    float fire_score;
    float intruder_score;
    uint8_t alarm_required;
    uint8_t extinguisher_sim_on;
} ML_Result;

typedef enum
{
    PAGE_ENVIRONMENT = 0,
    PAGE_GAS_LIGHT,
    PAGE_MOTION_ML
} Display_Page;

/* USER CODE END PD */

ADC_HandleTypeDef hadc1;
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

/*
 * Smoothed MQ-2 ADC value.
 */
static uint32_t mq2_filtered_value = 0U;
static uint8_t mq2_filter_ready = 0U;

/*
 * Time at which PIR stabilization starts.
 */
static uint32_t pir_startup_time = 0U;

/* USER CODE END PV */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);

/* USER CODE BEGIN PFP */

/* DHT11 functions */
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

/* MQ-2 functions */
static uint32_t MQ2_ReadRaw(void);
static uint32_t MQ2_ReadAverage(void);
static uint32_t MQ2_ReadFiltered(void);

/* PIR and LDR functions */
static uint8_t ReadStableDigitalInput(
    uint16_t pin
);

static uint8_t PIR_ReadTuned(void);

/* TinyML functions */
static float ML_Clamp01(float value);

static float ML_Normalize(
    float value,
    float minimum,
    float maximum
);

static ML_Result ML_RunInference(
    uint8_t temperature,
    uint8_t humidity,
    uint32_t mq2_value,
    uint8_t dark_condition,
    uint8_t motion
);

static const char *ML_GetLabel(
    ML_Class prediction
);

/* OLED functions */
static void OLED_ShowHeader(
    uint8_t page_number
);

static void OLED_ShowCommonML(
    const ML_Result *ml_result
);

static void OLED_ShowEnvironmentPage(
    uint8_t dht11_ok,
    uint8_t temperature,
    uint8_t humidity,
    const ML_Result *ml_result
);

static void OLED_ShowGasLightPage(
    uint32_t mq2_value,
    uint8_t dark_condition,
    const ML_Result *ml_result
);

static void OLED_ShowMotionPage(
    uint8_t motion,
    uint32_t mq2_value,
    const ML_Result *ml_result
);

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* ============================================================
   DHT11 functions
   ============================================================ */

static void DHT11_Pin_Output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_PIN;

    /*
     * Open-drain avoids driving the data line high against
     * the DHT11 sensor's pull-up resistor.
     */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
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

static void DHT11_Delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks =
        us * (HAL_RCC_GetHCLKFreq() / 1000000U);

    while ((DWT->CYCCNT - start) < ticks)
    {
    }
}

static void DHT11_DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static uint8_t DHT11_WaitForState(
    GPIO_PinState state,
    uint32_t timeout_us
)
{
    uint32_t start = DWT->CYCCNT;

    uint32_t ticks =
        timeout_us *
        (HAL_RCC_GetHCLKFreq() / 1000000U);

    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) != state)
    {
        if ((DWT->CYCCNT - start) > ticks)
        {
            return 0U;
        }
    }

    return 1U;
}

static uint8_t DHT11_Read(
    uint8_t *temperature,
    uint8_t *humidity
)
{
    uint8_t data[5] = {0U};
    uint8_t i;

    uint32_t start;
    uint32_t high_ticks;
    uint32_t high_us;
    uint32_t timeout_ticks;

    uint32_t cycles_per_us =
        HAL_RCC_GetHCLKFreq() / 1000000U;

    DHT11_Pin_Output();

    HAL_GPIO_WritePin(
        DHT11_PORT,
        DHT11_PIN,
        GPIO_PIN_RESET
    );

    HAL_Delay(20U);

    HAL_GPIO_WritePin(
        DHT11_PORT,
        DHT11_PIN,
        GPIO_PIN_SET
    );

    DHT11_Delay_us(30U);
    DHT11_Pin_Input();

    if (!DHT11_WaitForState(GPIO_PIN_RESET, 100U))
    {
        return 0U;
    }

    if (!DHT11_WaitForState(GPIO_PIN_SET, 100U))
    {
        return 0U;
    }

    if (!DHT11_WaitForState(GPIO_PIN_RESET, 100U))
    {
        return 0U;
    }

    for (i = 0U; i < 40U; i++)
    {
        if (!DHT11_WaitForState(GPIO_PIN_SET, 100U))
        {
            return 0U;
        }

        start = DWT->CYCCNT;
        timeout_ticks = 120U * cycles_per_us;

        while (
            HAL_GPIO_ReadPin(
                DHT11_PORT,
                DHT11_PIN
            ) == GPIO_PIN_SET
        )
        {
            if ((DWT->CYCCNT - start) > timeout_ticks)
            {
                return 0U;
            }
        }

        high_ticks = DWT->CYCCNT - start;
        high_us = high_ticks / cycles_per_us;

        data[i / 8U] <<= 1U;

        if (high_us > 40U)
        {
            data[i / 8U] |= 1U;
        }
    }

    if (
        (uint8_t)(
            data[0] +
            data[1] +
            data[2] +
            data[3]
        ) != data[4]
    )
    {
        return 0U;
    }

    *humidity = data[0];
    *temperature = data[2];

    return 1U;
}

/* ============================================================
   MQ-2 functions
   ============================================================ */

static uint32_t MQ2_ReadRaw(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    uint32_t adc_value = 0U;

    sConfig.Channel = ADC_CHANNEL_3;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_ADC_Start(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_ADC_PollForConversion(&hadc1, 100U) == HAL_OK)
    {
        adc_value = HAL_ADC_GetValue(&hadc1);
    }

    HAL_ADC_Stop(&hadc1);

    return adc_value;
}

static uint32_t MQ2_ReadAverage(void)
{
    uint32_t sum = 0U;
    uint8_t i;

    for (i = 0U; i < MQ2_SAMPLE_COUNT; i++)
    {
        sum += MQ2_ReadRaw();
        HAL_Delay(2U);
    }

    return sum / MQ2_SAMPLE_COUNT;
}

static uint32_t MQ2_ReadFiltered(void)
{
    uint32_t new_average;
    int32_t difference;

    new_average = MQ2_ReadAverage();

    if (mq2_filter_ready == 0U)
    {
        mq2_filtered_value = new_average;
        mq2_filter_ready = 1U;

        return mq2_filtered_value;
    }

    /*
     * Exponential moving average:
     *
     * filtered = filtered + (new - filtered) / 4
     *
     * MQ2_FILTER_SHIFT = 2 means division by 4.
     */
    difference =
        (int32_t)new_average -
        (int32_t)mq2_filtered_value;

    mq2_filtered_value =
        (uint32_t)(
            (int32_t)mq2_filtered_value +
            (difference >> MQ2_FILTER_SHIFT)
        );

    return mq2_filtered_value;
}

/* ============================================================
   PIR and LDR digital functions
   ============================================================ */

static uint8_t ReadStableDigitalInput(
    uint16_t pin
)
{
    uint8_t high_count = 0U;
    uint8_t i;

    for (i = 0U; i < 3U; i++)
    {
        if (HAL_GPIO_ReadPin(GPIOA, pin) == GPIO_PIN_SET)
        {
            high_count++;
        }

        HAL_Delay(5U);
    }

    return (high_count >= 2U) ? 1U : 0U;
}

static uint8_t PIR_ReadTuned(void)
{
    /*
     * HC-SR501 may be unstable for around 30 to 60 seconds
     * after the power supply is connected.
     */
    if (
        (HAL_GetTick() - pir_startup_time) <
        PIR_WARMUP_TIME_MS
    )
    {
        return 0U;
    }

    return ReadStableDigitalInput(GPIO_PIN_0);
}

/* ============================================================
   TinyML classifier
   ============================================================ */

static float ML_Clamp01(float value)
{
    if (value < 0.0f)
    {
        return 0.0f;
    }

    if (value > 1.0f)
    {
        return 1.0f;
    }

    return value;
}

static float ML_Normalize(
    float value,
    float minimum,
    float maximum
)
{
    if (maximum <= minimum)
    {
        return 0.0f;
    }

    return ML_Clamp01(
        (value - minimum) /
        (maximum - minimum)
    );
}

static ML_Result ML_RunInference(
    uint8_t temperature,
    uint8_t humidity,
    uint32_t mq2_value,
    uint8_t dark_condition,
    uint8_t motion
)
{
    ML_Result result = {0};

    float x_temp;
    float x_humidity;
    float x_gas;
    float x_dark;
    float x_motion;
    float temp_difference;
    float normal_temp_score;

    x_temp = ML_Normalize(
        (float)temperature,
        15.0f,
        50.0f
    );

    x_humidity = ML_Normalize(
        (float)humidity,
        20.0f,
        95.0f
    );

    x_gas = ML_Normalize(
        (float)mq2_value,
        0.0f,
        ML_ADC_MAX
    );

    x_dark = dark_condition ? 1.0f : 0.0f;
    x_motion = motion ? 1.0f : 0.0f;

    temp_difference =
        (float)temperature -
        ML_TEMP_NORMAL;

    if (temp_difference < 0.0f)
    {
        temp_difference = -temp_difference;
    }

    normal_temp_score =
        1.0f -
        ML_Clamp01(
            temp_difference / 20.0f
        );

    result.safe_score = ML_Clamp01(
        normal_temp_score * 0.30f +
        (1.0f - x_gas) * 0.35f +
        (1.0f - x_dark) * 0.10f +
        (1.0f - x_motion) * 0.20f +
        (1.0f - x_humidity) * 0.05f
    );

    result.watch_score = ML_Clamp01(
        x_temp * 0.25f +
        x_gas * 0.30f +
        x_dark * 0.20f +
        x_humidity * 0.10f +
        x_motion * 0.15f
    );

    result.gas_score = ML_Clamp01(
        x_gas * 0.75f +
        x_dark * 0.10f +
        x_temp * 0.10f +
        x_humidity * 0.05f
    );

    result.fire_score = ML_Clamp01(
        x_temp * 0.48f +
        x_gas * 0.35f +
        x_dark * 0.10f +
        x_humidity * 0.07f
    );

    result.intruder_score = ML_Clamp01(
        x_motion * 0.92f +
        x_dark * 0.05f +
        x_gas * 0.02f +
        x_temp * 0.01f
    );

    if (motion && (mq2_value >= GAS_THRESHOLD))
    {
        result.prediction = ML_INTRUDER;

        result.confidence = ML_Clamp01(
            (
                result.intruder_score +
                result.gas_score
            ) / 2.0f
        );

        result.alarm_required = 1U;

        return result;
    }

    if (motion)
    {
        result.prediction = ML_INTRUDER;
        result.confidence = result.intruder_score;
        result.alarm_required = 1U;

        return result;
    }

    if (mq2_value >= GAS_THRESHOLD)
    {
        result.prediction = ML_GAS_LEAK;
        result.confidence = result.gas_score;

        if (result.confidence < 0.70f)
        {
            result.confidence = 0.70f;
        }

        result.alarm_required = 1U;
        result.extinguisher_sim_on = 1U;

        return result;
    }

    if (
        (temperature >= ML_TEMP_DANGER) ||
        (
            (temperature >= ML_TEMP_WARNING) &&
            (mq2_value >= GAS_WARNING_LEVEL)
        ) ||
        (result.fire_score >= 0.68f)
    )
    {
        result.prediction = ML_FIRE_RISK;
        result.confidence = result.fire_score;

        if (result.confidence < 0.70f)
        {
            result.confidence = 0.70f;
        }

        result.alarm_required = 1U;

        return result;
    }

    if (
        (temperature >= ML_TEMP_WARNING) ||
        (mq2_value >= GAS_WARNING_LEVEL) ||
        dark_condition ||
        (result.watch_score >= 0.42f)
    )
    {
        result.prediction = ML_WATCH;
        result.confidence = result.watch_score;
        result.alarm_required = 1U;

        return result;
    }

    result.prediction = ML_SAFE;
    result.confidence = result.safe_score;
    result.alarm_required = 0U;

    return result;
}

static const char *ML_GetLabel(
    ML_Class prediction
)
{
    switch (prediction)
    {
        case ML_SAFE:
            return "SAFE";

        case ML_WATCH:
            return "WATCH";

        case ML_GAS_LEAK:
            return "GAS LEAK";

        case ML_FIRE_RISK:
            return "FIRE RISK";

        case ML_INTRUDER:
            return "INTRUDER";

        default:
            return "UNKNOWN";
    }
}

/* ============================================================
   OLED display functions
   PRESERVED FROM YOUR ORIGINAL CODE
   ============================================================ */

static void OLED_ShowHeader(
    uint8_t page_number
)
{
    char header[22];

    snprintf(
        header,
        sizeof(header),
        "HomeMonitoring %u/3",
        page_number
    );

    SSD1306_GotoXY(0, 0);
    SSD1306_Puts(
        header,
        &Font_7x10,
        1
    );
}

static void OLED_ShowCommonML(
    const ML_Result *ml_result
)
{
    char line[22];

    snprintf(
        line,
        sizeof(line),
        "ML:%s %u%%",
        ML_GetLabel(
            ml_result->prediction
        ),
        (unsigned int)(
            ml_result->confidence * 100.0f
        )
    );

    SSD1306_GotoXY(0, 50);
    SSD1306_Puts(
        line,
        &Font_7x10,
        1
    );
}

static void OLED_ShowEnvironmentPage(
    uint8_t dht11_ok,
    uint8_t temperature,
    uint8_t humidity,
    const ML_Result *ml_result
)
{
    char line[22];

    SSD1306_Clear();

    OLED_ShowHeader(1U);

    if (dht11_ok)
    {
        snprintf(
            line,
            sizeof(line),
            "TEMP: %u C",
            temperature
        );

        SSD1306_GotoXY(0, 15);
        SSD1306_Puts(
            line,
            &Font_7x10,
            1
        );

        snprintf(
            line,
            sizeof(line),
            "HUM : %u %%",
            humidity
        );

        SSD1306_GotoXY(0, 28);
        SSD1306_Puts(
            line,
            &Font_7x10,
            1
        );
    }
    else
    {
        SSD1306_GotoXY(0, 15);
        SSD1306_Puts(
            "DHT11 ERROR",
            &Font_7x10,
            1
        );

        SSD1306_GotoXY(0, 28);
        SSD1306_Puts(
            "CHECK DHT11",
            &Font_7x10,
            1
        );
    }

    SSD1306_GotoXY(0, 39);
    SSD1306_Puts(
        "ENVIRONMENT PAGE",
        &Font_7x10,
        1
    );

    OLED_ShowCommonML(ml_result);

    SSD1306_UpdateScreen();
}

static void OLED_ShowGasLightPage(
    uint32_t mq2_value,
    uint8_t dark_condition,
    const ML_Result *ml_result
)
{
    char line[22];

    SSD1306_Clear();

    OLED_ShowHeader(2U);

    snprintf(
        line,
        sizeof(line),
        "MQ2 RAW: %lu",
        (unsigned long)mq2_value
    );

    SSD1306_GotoXY(0, 15);
    SSD1306_Puts(
        line,
        &Font_7x10,
        1
    );

    SSD1306_GotoXY(0, 28);

    if (mq2_value >= GAS_THRESHOLD)
    {
        SSD1306_Puts(
            "GAS: DANGER",
            &Font_7x10,
            1
        );
    }
    else if (mq2_value >= GAS_WARNING_LEVEL)
    {
        SSD1306_Puts(
            "GAS: WATCH",
            &Font_7x10,
            1
        );
    }
    else
    {
        SSD1306_Puts(
            "GAS: NORMAL",
            &Font_7x10,
            1
        );
    }

    SSD1306_GotoXY(0, 39);

    SSD1306_Puts(
        dark_condition
            ? "LDR: DARK"
            : "LDR: LIGHT",
        &Font_7x10,
        1
    );

    OLED_ShowCommonML(ml_result);

    SSD1306_UpdateScreen();
}

static void OLED_ShowMotionPage(
    uint8_t motion,
    uint32_t mq2_value,
    const ML_Result *ml_result
)
{
    char line[22];
    uint8_t buzzer_should_ring;

    buzzer_should_ring =
        (
            (mq2_value >= GAS_THRESHOLD) &&
            (motion == 0U)
        ) ? 1U : 0U;

    SSD1306_Clear();

    OLED_ShowHeader(3U);

    SSD1306_GotoXY(0, 15);

    SSD1306_Puts(
        motion
            ? "PIR: MOTION YES"
            : "PIR: NO MOTION",
        &Font_7x10,
        1
    );

    SSD1306_GotoXY(0, 28);

    if (buzzer_should_ring)
    {
        SSD1306_Puts(
            "ACTION: BUZZER ON",
            &Font_7x10,
            1
        );
    }
    else if (ml_result->extinguisher_sim_on)
    {
        SSD1306_Puts(
            "ACTION: EXTING ON",
            &Font_7x10,
            1
        );
    }
    else
    {
        SSD1306_Puts(
            "ACTION: BUZZER OFF",
            &Font_7x10,
            1
        );
    }

    snprintf(
        line,
        sizeof(line),
        "CONF: %u%%",
        (unsigned int)(
            ml_result->confidence * 100.0f
        )
    );

    SSD1306_GotoXY(0, 39);
    SSD1306_Puts(
        line,
        &Font_7x10,
        1
    );

    OLED_ShowCommonML(ml_result);

    SSD1306_UpdateScreen();
}

/* USER CODE END 0 */

int main(void)
{
    uint8_t temperature = 0U;
    uint8_t humidity = 0U;
    uint8_t dht11_ok = 0U;

    uint8_t motion = 0U;
    uint8_t ldr_digital = 0U;
    uint8_t dark_condition = 0U;

    uint8_t buzzer_should_ring = 0U;

    uint32_t mq2_value = 0U;

    uint32_t last_dht_read = 0U;
    uint32_t last_sensor_read = 0U;
    uint32_t last_page_change = 0U;

    Display_Page current_page = PAGE_ENVIRONMENT;

    ML_Result ml_result = {0};

    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_I2C1_Init();

    /* USER CODE BEGIN 2 */

    DHT11_DWT_Init();

    if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    SSD1306_Init();

    SSD1306_Clear();

    SSD1306_GotoXY(0, 8);
    SSD1306_Puts(
        "HomeMonitoring",
        &Font_7x10,
        1
    );

    SSD1306_GotoXY(0, 24);
    SSD1306_Puts(
        "System",
        &Font_11x18,
        1
    );

    SSD1306_GotoXY(0, 48);
    SSD1306_Puts(
        "Starting...",
        &Font_7x10,
        1
    );

    SSD1306_UpdateScreen();

    HAL_Delay(1500U);

    /*
     * Start PIR stabilization timing after the startup screen.
     * For the next 60 seconds, PIR_ReadTuned() returns no motion.
     */
    pir_startup_time = HAL_GetTick();

    /*
     * Take initial measurements.
     */
    dht11_ok = DHT11_Read(
        &temperature,
        &humidity
    );

    mq2_value = MQ2_ReadFiltered();

    ldr_digital = ReadStableDigitalInput(
        GPIO_PIN_5
    );

    /*
     * LDR DO polarity:
     * HIGH = LIGHT
     * LOW  = DARK
     */
    dark_condition = !ldr_digital;

    motion = PIR_ReadTuned();

    ml_result = ML_RunInference(
        temperature,
        humidity,
        mq2_value,
        dark_condition,
        motion
    );

    last_dht_read = HAL_GetTick();
    last_sensor_read = HAL_GetTick();
    last_page_change = HAL_GetTick();

    /* USER CODE END 2 */

    while (1)
    {
        /*
         * DHT11 read once per two seconds.
         */
        if (
            (HAL_GetTick() - last_dht_read) >=
            DHT_READ_INTERVAL_MS
        )
        {
            last_dht_read = HAL_GetTick();

            dht11_ok = DHT11_Read(
                &temperature,
                &humidity
            );
        }

        /*
         * MQ-2, LDR, PIR and classifier update every 200 ms.
         */
        if (
            (HAL_GetTick() - last_sensor_read) >=
            SENSOR_READ_INTERVAL_MS
        )
        {
            last_sensor_read = HAL_GetTick();

            mq2_value = MQ2_ReadFiltered();

            ldr_digital = ReadStableDigitalInput(
                GPIO_PIN_5
            );

            /*
             * HIGH = LIGHT
             * LOW  = DARK
             */
            dark_condition = !ldr_digital;

            motion = PIR_ReadTuned();

            ml_result = ML_RunInference(
                temperature,
                humidity,
                mq2_value,
                dark_condition,
                motion
            );

            /*
             * LED follows PIR state.
             */
            HAL_GPIO_WritePin(
                GPIOA,
                GPIO_PIN_1,
                motion
                    ? GPIO_PIN_SET
                    : GPIO_PIN_RESET
            );

            /*
             * Buzzer ON only when:
             * MQ-2 exceeds the danger threshold
             * AND PIR detects no motion.
             */
            buzzer_should_ring =
                (
                    (mq2_value >= GAS_THRESHOLD) &&
                    (motion == 0U)
                ) ? 1U : 0U;

            HAL_GPIO_WritePin(
                GPIOA,
                GPIO_PIN_4,
                buzzer_should_ring
                    ? GPIO_PIN_SET
                    : GPIO_PIN_RESET
            );
        }

        /*
         * Original one-second page rotation.
         */
        if (
            (HAL_GetTick() - last_page_change) >=
            PAGE_INTERVAL_MS
        )
        {
            last_page_change = HAL_GetTick();

            if (current_page == PAGE_ENVIRONMENT)
            {
                current_page = PAGE_GAS_LIGHT;
            }
            else if (current_page == PAGE_GAS_LIGHT)
            {
                current_page = PAGE_MOTION_ML;
            }
            else
            {
                current_page = PAGE_ENVIRONMENT;
            }
        }

        /*
         * Original display update logic retained unchanged.
         */
        if (current_page == PAGE_ENVIRONMENT)
        {
            OLED_ShowEnvironmentPage(
                dht11_ok,
                temperature,
                humidity,
                &ml_result
            );
        }
        else if (current_page == PAGE_GAS_LIGHT)
        {
            OLED_ShowGasLightPage(
                mq2_value,
                dark_condition,
                &ml_result
            );
        }
        else
        {
            OLED_ShowMotionPage(
                motion,
                mq2_value,
                &ml_result
            );
        }

        HAL_Delay(40U);
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

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
        ) != HAL_OK
    )
    {
        Error_Handler();
    }

    PeriphClkInit.PeriphClockSelection =
        RCC_PERIPHCLK_ADC;

    PeriphClkInit.AdcClockSelection =
        RCC_ADCPCLK2_DIV6;

    if (
        HAL_RCCEx_PeriphCLKConfig(
            &PeriphClkInit
        ) != HAL_OK
    )
    {
        Error_Handler();
    }
}

static void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

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

    hadc1.Init.NbrOfConversion = 1;

    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    sConfig.Channel = ADC_CHANNEL_3;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime =
        ADC_SAMPLETIME_239CYCLES_5;

    if (
        HAL_ADC_ConfigChannel(
            &hadc1,
            &sConfig
        ) != HAL_OK
    )
    {
        Error_Handler();
    }
}

static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;

    /*
     * Original I2C configuration retained.
     */
    hi2c1.Init.ClockSpeed = 400000;

    hi2c1.Init.DutyCycle =
        I2C_DUTYCYCLE_2;

    hi2c1.Init.OwnAddress1 = 0;

    hi2c1.Init.AddressingMode =
        I2C_ADDRESSINGMODE_7BIT;

    hi2c1.Init.DualAddressMode =
        I2C_DUALADDRESS_DISABLE;

    hi2c1.Init.OwnAddress2 = 0;

    hi2c1.Init.GeneralCallMode =
        I2C_GENERALCALL_DISABLE;

    hi2c1.Init.NoStretchMode =
        I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(
        GPIOA,
        GPIO_PIN_1 |
        GPIO_PIN_2 |
        GPIO_PIN_4,
        GPIO_PIN_RESET
    );

    /*
     * PA0 = PIR output.
     * PA5 = LDR digital DO.
     */
    GPIO_InitStruct.Pin =
        GPIO_PIN_0 |
        GPIO_PIN_5;

    GPIO_InitStruct.Mode =
        GPIO_MODE_INPUT;

    GPIO_InitStruct.Pull =
        GPIO_NOPULL;

    HAL_GPIO_Init(
        GPIOA,
        &GPIO_InitStruct
    );

    /*
     * PA1 = LED
     * PA2 = DHT11 data
     * PA4 = Buzzer
     */
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
     * PA3 = ADC1_IN3, MQ-2 analog output.
     */
    GPIO_InitStruct.Pin = GPIO_PIN_3;

    GPIO_InitStruct.Mode =
        GPIO_MODE_ANALOG;

    HAL_GPIO_Init(
        GPIOA,
        &GPIO_InitStruct
    );
}

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
    uint32_t line
)
{
    (void)file;
    (void)line;
}

#endif
