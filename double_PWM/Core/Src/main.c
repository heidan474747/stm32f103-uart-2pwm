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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "string.h"
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
volatile int i=0;//初始通道1占空比
volatile int i2=0;//初始通道2占空比
volatile uint16_t flag=0;//提示错误的标志
volatile uint8_t mode=0;//用来存放通道1的模式
volatile uint8_t mode2=0;//用来存放通道2的模式
 uint8_t rx_data;//存放收到的字符
volatile int duty1;//存放通道1的占空比
volatile int duty2;//存放通道2的占空比

volatile uint8_t rx_buf[100];//存放累积收到的字符串
volatile uint8_t rx_index=0;//用来给给存放收到的数据计数
volatile uint16_t num_acc=0;//用来做通道1数值的累计
volatile uint16_t num_acc2=0;//用来做通道2数值的累计

volatile uint32_t last_digit_tick=0;//用来统计时间
volatile uint8_t oled_update=0;//通道1是否在oled屏幕上显示的标志
volatile uint8_t oled_update2=0;//通道2是否在oled屏幕上显示的标志

const char *cmdA="LED1 A";
const char *cmdM="LED1 M";
const char *cmdA2="LED2 A";
const char *cmdM2="LED2 M";//提前设置的正确字符串，用于后面比较。
uint8_t is_prefix=0;//用于判断是否是上方的字符串
volatile int dir1=1;//后面改变符号，用来自动模式增加或减少
volatile int dir2=1;
volatile uint32_t last_step_tick_ch1=0;//用来记录时间
volatile uint32_t last_step_tick_ch2=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	
	if (huart==&huart1)
	{
		rx_buf[rx_index]=rx_data;
		rx_index++;//把一次次收到的字符连接成字符串
		if(strcmp((char *)rx_buf,"LED1 A")==0)//识别收到的字符串

		{
			mode=1;//改变模式
			dir1=1;
			last_step_tick_ch1=HAL_GetTick();//获取时间
			flag=0;//错误提示的标志
			for(int k=0;k<100;k++)//已经识别到了，所以清空数组
			{
				rx_buf[k]=0;
			}
			rx_index=0;
			
		}
else if(strcmp((char *)rx_buf,"LED2 A")==0)//识别收到的字符

		{
			mode2=1;//改变模式
			dir2=1;
			last_step_tick_ch2=HAL_GetTick();
			flag=0;//错误提示的标志
			for(int k=0;k<100;k++)
			{
				rx_buf[k]=0;
			}
			rx_index=0;
			
		}		
		else if(strcmp((char *)rx_buf,"LED1 M")==0)//识别收到的字符
		{
			mode=2;
			if(mode2==2)
				mode2=0;//确保输入数字是传给通道2的，不然通道1，2同时打开会传给通道1
			flag=0;
			for(int k=0;k<100;k++)//依旧清空数组
			{
				rx_buf[k]=0;
			}
			rx_index=0;
		}
		else if(strcmp((char *)rx_buf,"LED2 M")==0)//识别收到的字符
		{
			mode2=2;
			if(mode==2)
			mode=0;
			flag=0;
			for(int k=0;k<100;k++)
			{
				rx_buf[k]=0;
			}
			rx_index=0;
		}
		
		
		 
		
			else if( mode==2)//识别收到的字符
			{
				
				
				is_prefix=1;//判断标志
				for(int k=0;k<rx_index;k++)//收到一个字符比较一次，只要一摸一样就不进入，不清空数组，这样才能在手动模式下换成别的模式
		{
			if(rx_buf[k]!=(uint8_t)cmdA[k]&&rx_buf[k]!=(uint8_t)cmdM[k]&&rx_buf[k]!=(uint8_t)cmdA2[k]&&rx_buf[k]!=(uint8_t)cmdM2[k])
			{
				is_prefix=0;
				break;
			}
		}
		if(is_prefix==0)
		{
			
				
				if(rx_data>='0'&&rx_data<='9')
				{
					uint32_t now=HAL_GetTick();
					if(now-last_digit_tick>500)//这样才能确保多次输入数字，否则每次输入数字都会到后面的累加里
					{
						num_acc=0;
					}
					last_digit_tick=now;
					num_acc=num_acc*10+(rx_data-'0');//输入二位数或者三位数要这样计算，因为收到字符是一个一个收到的
				
				
				if(num_acc>=100)//把i锁定在最大值
	            {
		        num_acc=100;
	            }
				i=num_acc;
				duty1=i*100/(htim2.Init.Period+1);//计算占空比
				 __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_4,i);//改变PWM的值
				oled_update=1;
	            
				flag=0;//改变错误提示的标志
				for(int k=0;k<100;k++)//依旧清空缓冲区
			{
				rx_buf[k]=0;
			}
			rx_index=0;
		}
				else {flag=1;
		for(int k=0;k<100;k++)
			{
				rx_buf[k]=0;
			}
			rx_index=0;}
			}	
		
		}
			
			
			/*此处下方为通道2    此处下方为通道2    此处下方为通道2  */
			/*此处下方为通道2    此处下方为通道2    此处下方为通道2  */
			/*此处下方为通道2    此处下方为通道2    此处下方为通道2  */
			
			
			
				
		
		 
		
			else if( mode2==2)//识别收到的字符
			{
				
				
				is_prefix=1;
		for(int k=0;k<rx_index;k++)
		{
			if(rx_buf[k]!=(uint8_t)cmdA[k]&&rx_buf[k]!=(uint8_t)cmdM[k]&&rx_buf[k]!=(uint8_t)cmdA2[k]&&rx_buf[k]!=(uint8_t)cmdM2[k])
			{
				is_prefix=0;
				break;
			}
		}
		if(is_prefix==0)
		{
			
				
				if(rx_data>='0'&&rx_data<='9')
				{
					uint32_t now=HAL_GetTick();
					if(now-last_digit_tick>500)
					{
						num_acc2=0;
					}
					last_digit_tick=now;
					num_acc2=num_acc2*10+(rx_data-'0');
				
				
				if(num_acc2>=100)//把i锁定在最大值
	            {
		        num_acc2=100;
	            }
				i2=num_acc2;
				duty2=i2*100/(htim2.Init.Period+1);
				 __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_3,i2);//改变PWM的值
				oled_update2=1;
	            
				flag=0;//改变错误提示的标志
				for(int k=0;k<100;k++)
			{
				rx_buf[k]=0;
			}
			rx_index=0;
		}
				else {flag=1;
		for(int k=0;k<100;k++)
			{
				rx_buf[k]=0;
			}
			rx_index=0;}
			}	
		}
	
			else 
			{	
			    flag=1;
				is_prefix=1;
		for(int k=0;k<rx_index;k++)
		{
			if(rx_buf[k]!=(uint8_t)cmdA[k]&&rx_buf[k]!=(uint8_t)cmdM[k]&&rx_buf[k]!=(uint8_t)cmdA2[k]&&rx_buf[k]!=(uint8_t)cmdM2[k])
			{
				is_prefix=0;
				break;
			}
		}
		if(is_prefix==0)
			{

				for(int k=0;k<=rx_index;k++)
			{
				rx_buf[k]=0;
			}
			rx_index=0;
			}	
			
			
			
	}
}		
	HAL_UART_Receive_IT(&huart1,&rx_data,1);//开启UART接收中断
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
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_4);//开启PWM
    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_3);//开启PWM
  OLED_Clear(0x00);//OLED清屏
  OLED_ShowString(0,0,(uint8_t*)"Duty1:",16);
  OLED_ShowString(0,2,(uint8_t*)"Duty2:",16);
  HAL_UART_Receive_IT(&huart1,&rx_data,1);
  __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_4,i);
    __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_3,i2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(oled_update==1)
	  {
		   OLED_ShowNum(48,0,duty1,3,16);//输出结果
	       OLED_ShowChar(72,0,'%',16);
	  }
	  	  if(oled_update2==1)
	  {
		   OLED_ShowNum(48,2,duty2,3,16);//输出结果
	       OLED_ShowChar(72,2,'%',16);
	  }

	  OLED_ShowString(0,0,(uint8_t*)"Duty1:",16);
	  if(flag==1)
	  {
		 OLED_ShowString(0,4,(uint8_t*)"Input Invalid",16); //若输入错误弹出提示
	  }
	  if(flag==0)
	  {
		 OLED_ShowString(0,4,(uint8_t*)"             ",16); //输入正确覆盖提示
	  }
	  
		
if(mode==1)
{
    if(i<20) i=20;
    if(i>80) i=80;//限制范围

    if(HAL_GetTick() - last_step_tick_ch1 >= 20)//只有时间过了一会再变化，不然变化太快，看不出来，相当于haley，但是不会阻断整个单片机的运行
    {
        last_step_tick_ch1 = HAL_GetTick();

        i += dir1;//通过主循环一次次加来改变占空比
        if(i>=80) 
			{ 
		i=80; 
		dir1=-1; //i到80了就逐渐减少
		}
        if(i<=20) 
			{
		i=20; 
		dir1= 1;//i到20了就逐渐增加
		}

        duty1 = i*100/(htim2.Init.Period+1);
        __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_4,i);
        OLED_ShowNum(48,0,duty1,3,16);
        OLED_ShowChar(72,0,'%',16);
    }
}					
		


		
			/*此处下方为通道2    此处下方为通道2    此处下方为通道2  */
			/*此处下方为通道2    此处下方为通道2    此处下方为通道2  */
			/*此处下方为通道2    此处下方为通道2    此处下方为通道2  */
		
		
if(mode2==1)
{
    if(i2<20) i2=20;
    if(i2>80) i2=80;

    if(HAL_GetTick() - last_step_tick_ch2 >= 20)
    {
        last_step_tick_ch2 = HAL_GetTick();

        i2 += dir2;
        if(i2>=80)
 { 
 i2=80;
 dir2=-1; 
 }
        if(i2<=20)
 { 
 i2=20;
 dir2= 1;
 }

        duty2 = i2*100/(htim2.Init.Period+1);
        __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_3,i2);
        OLED_ShowNum(48,2,duty2,3,16);
        OLED_ShowChar(72,2,'%',16);
    }
}		

    /* USER CODE END WHILE */

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
