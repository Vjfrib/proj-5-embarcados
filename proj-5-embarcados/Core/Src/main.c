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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

#define PCF8591_ADDR       (0x48 << 1)

#define PCF_AIN_LDR        0
#define PCF_AIN_TEMP       1
#define PCF_AIN_VOLT       3

uint8_t uart_rx_byte;
char uart_cmd[32];
uint8_t uart_idx = 0;

uint8_t i2c_rx_buf[2];
uint8_t dac_buf[2];
uint8_t pcf_control_byte;
volatile uint8_t i2c_busy = 0;
volatile uint8_t spi_busy = 0;

uint8_t last_value = 0;
uint8_t canal_atual = 0;
char current_letter = 0;
char current_sign = 0;
uint8_t matrix_toggle = 0;
uint32_t last_toggle_tick = 0;

uint8_t spi_tx_buf[2];

const uint8_t CHAR_T[8] = {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00};
const uint8_t CHAR_V[8] = {0x42,0x42,0x42,0x42,0x24,0x24,0x18,0x00};
const uint8_t CHAR_L[8] = {0x40,0x40,0x40,0x40,0x40,0x40,0x7E,0x00};
const uint8_t CHAR_PLUS[8] = {0x00,0x18,0x18,0x7E,0x7E,0x18,0x18,0x00};
const uint8_t CHAR_MINUS[8] = {0x00,0x00,0x00,0x7E,0x7E,0x00,0x00,0x00};
const uint8_t CHAR_BLANK[8] = {0,0,0,0,0,0,0,0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

void UART_ProcessCommand(char *cmd);
void PCF8591_Read_IT(uint8_t channel);
void PCF8591_SetDAC_IT(uint8_t value);

void MAX7219_Send_IT(uint8_t address, uint8_t data);
void MAX7219_Init_Blocking(void);
void MAX7219_DisplayChar_Blocking(const uint8_t character[8]);
void MAX7219_Clear_Blocking(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_SPI2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  HAL_GPIO_WritePin(MAX7219_CS_GPIO_Port, MAX7219_CS_Pin, GPIO_PIN_SET);

  MAX7219_Init_Blocking();
  MAX7219_Clear_Blocking();

  HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1);

  //char start_msg[] = "\r\nSistema iniciado. Comandos: Temp, Volt, LDR, Set_DAC_<0-255>\r\n";
  char start_msg[] =
  "\r\n====================================\r\n"
  " Projeto 5 - I2C + SPI + PCF8591 + MAX7219\r\n"
  " NUCLEO-L476RG + USART2 + I2C1 + SPI2\r\n"
  "====================================\r\n"
  "Comandos disponiveis:\r\n"
  "Read_AIN0  // LDR: matriz alternara entre L e + ou -\r\n"
  "Read_AIN1  // Temperatura: matriz alternara entre T e + ou -\r\n"
  //"Read_AIN3  // Tensao: matriz alternara entre V e + ou -\r\n"
  "Read_AIN3  // Tensao do potenciometro: matriz alternara entre V e + ou -\r\n"
  "Set_DAC_0 ate Set_DAC_255\r\n\r\n";
  HAL_UART_Transmit(&huart2, (uint8_t*)start_msg, strlen(start_msg), HAL_MAX_DELAY);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  if (current_letter != 0 && HAL_GetTick() - last_toggle_tick >= 500)
	    {
	      last_toggle_tick = HAL_GetTick();
	      matrix_toggle = !matrix_toggle;

	      if (matrix_toggle == 0)
	      {
	        if (current_letter == 'T') MAX7219_DisplayChar_Blocking(CHAR_T);
	        else if (current_letter == 'V') MAX7219_DisplayChar_Blocking(CHAR_V);
	        else if (current_letter == 'L') MAX7219_DisplayChar_Blocking(CHAR_L);
	      }
	      else
	      {
	        if (current_sign == '+') MAX7219_DisplayChar_Blocking(CHAR_PLUS);
	        else MAX7219_DisplayChar_Blocking(CHAR_MINUS);
	      }
	    }

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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
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
  hi2c1.Init.Timing = 0x10D19CE4;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
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
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(MAX7219_CS_GPIO_Port, MAX7219_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MAX7219_CS_Pin */
  GPIO_InitStruct.Pin = MAX7219_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(MAX7219_CS_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void UART_ProcessCommand(char *cmd)
{
  //if (strcmp(cmd, "Temp") == 0 || strcmp(cmd, "Read_AIN1") == 0)
  if (strcmp(cmd, "Read_AIN1") == 0)
  {
    current_letter = 'T';
    PCF8591_Read_IT(PCF_AIN_TEMP);
  }
  //else if (strcmp(cmd, "Volt") == 0 || strcmp(cmd, "Read_AIN3") == 0)
  else if (strcmp(cmd, "Read_AIN3") == 0)
  {
    current_letter = 'V';
    PCF8591_Read_IT(PCF_AIN_VOLT);
  }
  //else if (strcmp(cmd, "LDR") == 0 || strcmp(cmd, "Read_AIN0") == 0)
  else if (strcmp(cmd, "Read_AIN0") == 0)
  {
    current_letter = 'L';
    PCF8591_Read_IT(PCF_AIN_LDR);
  }
  else if (strncmp(cmd, "Set_DAC_", 8) == 0)
  {
    int value = atoi(&cmd[8]);

    if (value < 0) value = 0;
    if (value > 255) value = 255;

    PCF8591_SetDAC_IT((uint8_t)value);
  }
  else
  {
    char msg[] = "Comando invalido\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
  }
}

void PCF8591_Read_IT(uint8_t channel)
{
  if (i2c_busy) return;

  //pcf_control_byte = 0x00 | (channel & 0x03);
  canal_atual = channel;
  pcf_control_byte = 0x00 | (channel & 0x03);
  i2c_busy = 1;

  HAL_I2C_Master_Transmit_IT(&hi2c1, PCF8591_ADDR, &pcf_control_byte, 1);
}

void PCF8591_SetDAC_IT(uint8_t value)
{
  //static uint8_t dac_buf[2];

  if (i2c_busy) return;

  canal_atual = 99;
  dac_buf[0] = 0x40;
  dac_buf[1] = value;
  i2c_busy = 1;

  HAL_I2C_Master_Transmit_IT(&hi2c1, PCF8591_ADDR, dac_buf, 2);
}

void MAX7219_Send_IT(uint8_t address, uint8_t data)
{
  if (spi_busy) return;

  spi_tx_buf[0] = address;
  spi_tx_buf[1] = data;

  spi_busy = 1;
  HAL_GPIO_WritePin(MAX7219_CS_GPIO_Port, MAX7219_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit_IT(&hspi2, spi_tx_buf, 2);
}

void MAX7219_Send_Blocking(uint8_t address, uint8_t data)
{
  uint8_t data_tx[2] = {address, data};

  HAL_GPIO_WritePin(MAX7219_CS_GPIO_Port, MAX7219_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi2, data_tx, 2, HAL_MAX_DELAY);
  HAL_GPIO_WritePin(MAX7219_CS_GPIO_Port, MAX7219_CS_Pin, GPIO_PIN_SET);
}

void MAX7219_Init_Blocking(void)
{
  MAX7219_Send_Blocking(0x09, 0x00);
  MAX7219_Send_Blocking(0x0A, 0x03);
  MAX7219_Send_Blocking(0x0B, 0x07);
  MAX7219_Send_Blocking(0x0C, 0x01);
  MAX7219_Send_Blocking(0x0F, 0x00);
}
/*
void MAX7219_DisplayChar_Blocking(const uint8_t character[8])
{
  for (uint8_t i = 0; i < 8; i++)
  {
    MAX7219_Send_Blocking(i + 1, character[i]);
  }
}
*/
void MAX7219_DisplayChar_Blocking(const uint8_t character[8])
{
  uint8_t rotated[8] = {0};

  /*
   * Corrige a orientacao fisica da matriz na protoboard.
   * Antes, V aparecia como > e - aparecia como |.
   * Aqui fazemos uma rotacao de 90 graus no sentido horario.
   */
  for (uint8_t row = 0; row < 8; row++)
  {
    for (uint8_t col = 0; col < 8; col++)
    {
      if (character[row] & (1 << col))
      {
        //rotated[col] |= (1 << (7 - row));
    	rotated[7 - col] |= (1 << row);
      }
    }
  }

  for (uint8_t i = 0; i < 8; i++)
  {
    MAX7219_Send_Blocking(i + 1, rotated[i]);
  }
}

void MAX7219_Clear_Blocking(void)
{
  MAX7219_DisplayChar_Blocking(CHAR_BLANK);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    if (uart_rx_byte == '\r' || uart_rx_byte == '\n')
    {
      if (uart_idx > 0)
      {
        uart_cmd[uart_idx] = '\0';
        UART_ProcessCommand(uart_cmd);
        uart_idx = 0;
      }
    }
    else
    {
      if (uart_idx < sizeof(uart_cmd) - 1)
      {
        uart_cmd[uart_idx++] = uart_rx_byte;
      }
    }

    HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1);
  }
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C1)
  {
    //if ((pcf_control_byte & 0x40) == 0x40)
	if (canal_atual == 99)
    {
      i2c_busy = 0;
      //char msg[] = "DAC atualizado\r\n";
      //HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
      uint32_t tensao_mV = (dac_buf[1] * 3300) / 255;
      char msg[80];

      sprintf(msg, "Valor do DAC: %u | %lumV\r\n", dac_buf[1], tensao_mV);
      HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }
    else
    {
      HAL_I2C_Master_Receive_IT(&hi2c1, PCF8591_ADDR, i2c_rx_buf, 2);
    }
  }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C1)
  {
    i2c_busy = 0;

    last_value = i2c_rx_buf[1];

    if (last_value < 128) current_sign = '-';
    else current_sign = '+';

    matrix_toggle = 0;
    last_toggle_tick = HAL_GetTick() - 500;

    /*
    matrix_toggle = 0;
    last_toggle_tick = HAL_GetTick();

    if (current_letter == 'T') MAX7219_DisplayChar_Blocking(CHAR_T);
    else if (current_letter == 'V') MAX7219_DisplayChar_Blocking(CHAR_V);
    else if (current_letter == 'L') MAX7219_DisplayChar_Blocking(CHAR_L);
    */

    //char msg[64];
    char msg[120];
    //sprintf(msg, "Valor lido: %u | %c\r\n", last_value, current_sign);
    uint32_t tensao_mV = (last_value * 3300) / 255;

    if (canal_atual == PCF_AIN_LDR)
    {
      sprintf(msg,
              "AIN0: %u | %lumV // matriz alternara entre L e %c\r\n",
              last_value, tensao_mV, current_sign);
    }
    else if (canal_atual == PCF_AIN_TEMP)
    {
      sprintf(msg,
              "AIN1: %u | %lumV // matriz alternara entre T e %c\r\n",
              last_value, tensao_mV, current_sign);
    }
    else if (canal_atual == PCF_AIN_VOLT)
    {
      sprintf(msg,
              //"AIN3: %u | %lumV // matriz alternara entre V e %c\r\n",
			  "AIN3 (potenciometro): %u | %lumV // matriz alternara entre V e %c\r\n",
              last_value, tensao_mV, current_sign);
    }
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
  }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C1)
  {
    i2c_busy = 0;
    char msg[] = "Erro I2C\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
  }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi->Instance == SPI2)
  {
    HAL_GPIO_WritePin(MAX7219_CS_GPIO_Port, MAX7219_CS_Pin, GPIO_PIN_SET);
    spi_busy = 0;
  }
}

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
