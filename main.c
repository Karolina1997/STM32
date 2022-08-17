#include "stm32f4xx.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "MT_Simple_ADC_DAC.h"


//deklaracje zmiennych
GPIO_InitTypeDef  GPIO_Init;
uint8_t KeyCode, KeyState, parametr=127;
static uint8_t parametr2=127;
char charpar[10];
uint16_t joyX, joyY;

//**************************************************************************************
//prototypy funkcji
void SystemClock_Config(void);
void MatrixKeyPad_GPIO_Conf(void);
void KeyPad_status(uint8_t);
void Joystick_status(uint16_t x, uint16_t y);
void Encoder_status(uint16_t x);
uint8_t ReadKey(void);
uint8_t ReadKeyboard(void);
void IRQ_PA9_Config(void);
void LCD_Init(void);
//**************************************************************************************


int main(void)
{
	HAL_Init();
	SystemClock_Config();
    //HAL_GPIO_Init(GPIOG,&GPIO_Init);
    LCD_Init();
    MatrixKeyPad_GPIO_Conf();

    SADC_Init_PA5();
    SADC_Init_PC3();
    IRQ_PA9_Config();


	 while (1)
	     {

		 KeyCode=ReadKeyboard();   //uzupe³nij funkcjê
		 KeyPad_status(KeyCode);   //uzupe³nij funkcjê

		 if (KeyCode==0x04) parametr--;
		 if (KeyCode==0x05) parametr++;

		 //Wyœwietlania zmiennej parametr na ekranie LCD
		 sprintf(charpar,							"  %03d    ",parametr);
		 BSP_LCD_DisplayStringAt(0,80, (uint8_t *)  "Parametr:",RIGHT_MODE);
		 BSP_LCD_DisplayStringAt(0,100, (uint8_t *) charpar,RIGHT_MODE);


		 HAL_Delay(100);

		 joyX=2047;  //uzupe³nij danymi z ADC osi X
		 joyY=2047;	 //Uzupe³nij danymi z ADC osi Y

		 //wyœwietlanie graficznie stanu joystica
		 Joystick_status(joyX, joyY);


		 //tutaj dopisz procedurê odpytywania encodera (polling) albo obs³ugê umieœc w procedurze/ach obs³ugi przerwania/ñ



		 //wyœwietlanie stanu encodera
		 Encoder_status(parametr2);

	     }


}






void SystemClock_Config(void) {
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

	/* Enable Power Control clock */
	__PWR_CLK_ENABLE();

	/* The voltage scaling allows optimizing the power consumption when the device is
	 clocked below the maximum system frequency, to update the voltage scaling value
	 regarding system frequency refer to product datasheet.  */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/*##-1- System Clock Configuration #########################################*/
	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 360;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	HAL_PWREx_ActivateOverDrive();

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

	/*##-2- LTDC Clock Configuration ###########################################*/
	/* LCD clock configuration */
	/* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
	/* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz */
	/* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/4 = 48 Mhz */
	/* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDIVR_8 = 48/8 = 6 Mhz */
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
	PeriphClkInitStruct.PLLSAI.PLLSAIR = 4;
	PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
}





void MatrixKeyPad_GPIO_Conf(void)
{


	GPIO_InitTypeDef   GPIO_InitStructure;


	//Porty PE2,PE3,PE4,PE5 - wiersze
	// skonfigurowac jako wejscia z pullup

	GPIO_InitStructure.Pin = (GPIO_PIN_2 |  GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
	GPIO_InitStructure.Mode   = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull   = GPIO_PULLUP;
    GPIO_InitStructure.Speed  = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);


	//Porty PE6,PC11,PC12,PC13 - kolumny
	// skonfigurowac jako wyjscia z otwartym drenem oraz pullup - NIE WOLNO KONFIGUROWAC JAKO PP!!!!!!!

	//PE6
	GPIO_InitStructure.Pin = GPIO_PIN_6;
	GPIO_InitStructure.Mode   = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStructure.Pull   = GPIO_PULLUP;
	GPIO_InitStructure.Speed  = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);

	//PE11,PE12,PE13
	GPIO_InitStructure.Pin = (GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13);
	GPIO_InitStructure.Mode   = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStructure.Pull   = GPIO_PULLUP;
	GPIO_InitStructure.Speed  = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);


	__GPIOE_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
}

void Joystick_status(uint16_t x, uint16_t y)
{
char txt[10];

	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_SetFont(&Font16);
	BSP_LCD_DisplayStringAt(0,150, (uint8_t *) "Joystick", CENTER_MODE);
	sprintf(txt,"X=%04d  ",x);
	BSP_LCD_DisplayStringAt(0,190, (uint8_t *) txt,RIGHT_MODE);
	sprintf(txt,"Y=%04d  ",y);
	BSP_LCD_DisplayStringAt(0,220, (uint8_t *) txt,RIGHT_MODE);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_FillRect(10, 170, 100,100);
	//BSP_LCD_FillCircle(60, 220, 50);
	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
	BSP_LCD_DrawRect(10, 170, 100, 100);
	//BSP_LCD_DrawCircle(60, 220, 50);
	BSP_LCD_SetTextColor(LCD_COLOR_RED);
	BSP_LCD_FillCircle((30+(x/68)),(190+(y/68)),20);
}

void Encoder_status(uint16_t x)
{
char txt[10];


	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_SetFont(&Font16);
	BSP_LCD_DisplayStringAt(0,280, (uint8_t *) "Encoder", CENTER_MODE);
	sprintf(txt,"Parametr=%04d",x);
	BSP_LCD_DisplayStringAt(0,300, (uint8_t *) txt,CENTER_MODE);
	}


void KeyPad_status(uint8_t stan)
{
char txt[10];
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0,0, (uint8_t *) "KeyPad 4x4", CENTER_MODE);

    sprintf(txt,"KOD:0x%02X ",stan);
    BSP_LCD_DisplayStringAt(0,40, (uint8_t *) txt,RIGHT_MODE);


	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
	BSP_LCD_DrawCircle(15, 30, 10);
	BSP_LCD_DrawCircle(45, 30, 10);
	BSP_LCD_DrawCircle(75, 30, 10);
	BSP_LCD_DrawCircle(105,30, 10);
	BSP_LCD_DrawCircle(15, 60, 10);
	BSP_LCD_DrawCircle(45, 60, 10);
	BSP_LCD_DrawCircle(75, 60, 10);
	BSP_LCD_DrawCircle(105,60, 10);
	BSP_LCD_DrawCircle(15, 90, 10);
	BSP_LCD_DrawCircle(45, 90, 10);
	BSP_LCD_DrawCircle(75, 90, 10);
	BSP_LCD_DrawCircle(105,90, 10);
	BSP_LCD_DrawCircle(15, 120, 10);
	BSP_LCD_DrawCircle(45, 120, 10);
	BSP_LCD_DrawCircle(75, 120, 10);
	BSP_LCD_DrawCircle(105,120, 10);


	if (stan == 0x01) 	BSP_LCD_SetTextColor(LCD_COLOR_RED); else BSP_LCD_SetTextColor(LCD_COLOR_BLACK); BSP_LCD_FillCircle(15, 30, 8);
	if (stan == 0x02) 	BSP_LCD_SetTextColor(LCD_COLOR_RED); else BSP_LCD_SetTextColor(LCD_COLOR_BLACK); BSP_LCD_FillCircle(45, 30, 8);
	if (stan == 0x03) 	BSP_LCD_SetTextColor(LCD_COLOR_RED); else BSP_LCD_SetTextColor(LCD_COLOR_BLACK); BSP_LCD_FillCircle(75, 30, 8);
	if (stan == 0x0A)	BSP_LCD_SetTextColor(LCD_COLOR_RED); else BSP_LCD_SetTextColor(LCD_COLOR_BLACK); BSP_LCD_FillCircle(105,30, 8);
	if (stan == 0x04)	BSP_LCD_SetTextColor(LCD_COLOR_RED); else BSP_LCD_SetTextColor(LCD_COLOR_BLACK); BSP_LCD_FillCircle(15,60, 8);
	if (stan == 0x05)	BSP_LCD_SetTextColor(LCD_COLOR_RED); else BSP_LCD_SetTextColor(LCD_COLOR_BLACK); BSP_LCD_FillCircle(45,60, 8);
	if (stan == 0x06)	BSP_LCD_SetTextColor(LCD_COLOR_RED); else BSP_LCD_SetTextColor(LCD_COLOR_BLACK); BSP_LCD_FillCircle(75,60, 8);
	if (stan == 0x0B)	BSP_LCD_SetTextColor(LCD_COLOR_RED); else BSP_LCD_SetTextColor(LCD_COLOR_BLACK); BSP_LCD_FillCircle(105,60,8);
	if (stan == 0x07) 	BSP_LCD_SetTextColor(LCD_COLOR_RED); else BSP_LCD_SetTextColor(LCD_COLOR_BLACK); BSP_LCD_FillCircle(15,90, 8);
	if (stan == 0x08) 	BSP_LCD_SetTextColor(LCD_COLOR_RED); else BSP_LCD_SetTextColor(LCD_COLOR_BLACK); BSP_LCD_FillCircle(45,90, 8);
	if (stan == 0x09) 	BSP_LCD_SetTextColor(LCD_COLOR_RED); else BSP_LCD_SetTextColor(LCD_COLOR_BLACK); BSP_LCD_FillCircle(75,90, 8);
	if (stan == 0x0C)	BSP_LCD_SetTextColor(LCD_COLOR_RED); else BSP_LCD_SetTextColor(LCD_COLOR_BLACK); BSP_LCD_FillCircle(105,90, 8);
	if (stan == 0x0E)	BSP_LCD_SetTextColor(LCD_COLOR_RED); else BSP_LCD_SetTextColor(LCD_COLOR_BLACK); BSP_LCD_FillCircle(15,120, 8);
	if (stan == 0x00)	BSP_LCD_SetTextColor(LCD_COLOR_RED); else BSP_LCD_SetTextColor(LCD_COLOR_BLACK); BSP_LCD_FillCircle(45,120, 8);
	if (stan == 0x0F)	BSP_LCD_SetTextColor(LCD_COLOR_RED); else BSP_LCD_SetTextColor(LCD_COLOR_BLACK); BSP_LCD_FillCircle(75,120, 8);
	if (stan == 0x0D)	BSP_LCD_SetTextColor(LCD_COLOR_RED); else BSP_LCD_SetTextColor(LCD_COLOR_BLACK); BSP_LCD_FillCircle(105,120,8);


}

void LCD_Init(void)
{
		BSP_LCD_Init();
	    BSP_LCD_LayerDefaultInit(0, (uint32_t) LCD_FRAME_BUFFER);
	    BSP_LCD_SetLayerVisible(0, ENABLE);
	    BSP_LCD_SelectLayer(0);
	    BSP_LCD_Clear(LCD_COLOR_WHITE);
	    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	    BSP_LCD_DisplayOn();
}



uint8_t ReadKey(void)
{

	//uzupe³nij
	//tu "sucha" procedura skanowania matrycy przycisków 4x4
	
	HAL_GPIO_WritePin  (   GPIOC,    GPIO_PIN_11, GPIO_PIN_RESET); // comum 1
	HAL_GPIO_WritePin  (   GPIOC,    GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin  (   GPIOC,    GPIO_PIN_13, GPIO_PIN_SET);
	HAL_GPIO_WritePin  (   GPIOE,    GPIO_PIN_6, GPIO_PIN_SET);
	HAL_Delay(1);
	// odczyta
	if( !HAL_GPIO_ReadPin  ( GPIOE, GPIO_PIN_2   ) ) return 0x01;
	if( !HAL_GPIO_ReadPin  ( GPIOE, GPIO_PIN_3   ) ) return 0x04;
	if( !HAL_GPIO_ReadPin  ( GPIOE, GPIO_PIN_4   ) ) return 0x07;
	if( !HAL_GPIO_ReadPin  ( GPIOE, GPIO_PIN_5   ) ) return 0x0E;
	
	HAL_GPIO_WritePin  ( GPIOC, GPIO_PIN_12, GPIO_PIN_RESET); // colum 2
	HAL_GPIO_WritePin  (   GPIOC,    GPIO_PIN_11, GPIO_PIN_SET);
	HAL_GPIO_WritePin  (   GPIOC,    GPIO_PIN_13, GPIO_PIN_SET);
	HAL_GPIO_WritePin  (   GPIOE,    GPIO_PIN_6, GPIO_PIN_SET);
	HAL_Delay(1);
	if( !HAL_GPIO_ReadPin  ( GPIOE,  GPIO_PIN_2   ) ) return 0x02;
	if( !HAL_GPIO_ReadPin  ( GPIOE,  GPIO_PIN_3   ) ) return 0x05;
	if( !HAL_GPIO_ReadPin  ( GPIOE,  GPIO_PIN_4   ) ) return 0x08;
	if( !HAL_GPIO_ReadPin  (  GPIOE, GPIO_PIN_5   ) ) return 0x00;
	
	HAL_GPIO_WritePin  ( GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); //colum 3
	HAL_GPIO_WritePin  (   GPIOC,    GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin  (   GPIOC,    GPIO_PIN_11, GPIO_PIN_SET);
	HAL_GPIO_WritePin  (   GPIOE,    GPIO_PIN_6, GPIO_PIN_SET);
	HAL_Delay(1);
	if( !HAL_GPIO_ReadPin  ( GPIOE, GPIO_PIN_2   ) ) return 0x03;
	if( !HAL_GPIO_ReadPin  ( GPIOE, GPIO_PIN_3   ) ) return 0x06;
	if( !HAL_GPIO_ReadPin  ( GPIOE, GPIO_PIN_4   ) ) return 0x09;
	if( !HAL_GPIO_ReadPin  ( GPIOE, GPIO_PIN_5   ) ) return 0x0F;
	
	HAL_GPIO_WritePin  (  GPIOE,  GPIO_PIN_6,  GPIO_PIN_RESET);  // colum4
	HAL_GPIO_WritePin  (   GPIOC,    GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin  (   GPIOC,    GPIO_PIN_13, GPIO_PIN_SET);
	HAL_GPIO_WritePin  (   GPIOC,    GPIO_PIN_11, GPIO_PIN_SET);
	HAL_Delay(1);
	if( !HAL_GPIO_ReadPin  ( GPIOE,  GPIO_PIN_2   ) ) return 0x0A;
	if( !HAL_GPIO_ReadPin  ( GPIOE,   GPIO_PIN_3   ) ) return 0x0B;
	if( !HAL_GPIO_ReadPin  (  GPIOE, GPIO_PIN_4   ) ) return 0x0C;
	if( !HAL_GPIO_ReadPin  ( GPIOE,  GPIO_PIN_5   ) ) return 0x0D;
		

	return 0xFF;

}

uint8_t ReadKeyboard(void)
{
//uzupe³nij

//funkcja powinna wywo³ywac uint8_t ReadKey(void)
//a dodatkowo zawiarac procedure eliminacji drgania styków i sprawdzanie zwolnienia przycisku.

	//procedura eliminacji drgania stykow
	KeyState = ReadKey();
	HAL_Delay(20);
	if(ReadKey()!=KeyState) return 0xFF;
	


 return KeyState;
}



void IRQ_PA9_Config(void)
{

	GPIO_InitTypeDef   GPIO_InitStructure;

	__GPIOA_CLK_ENABLE();

	//konfiguracja PA9
	GPIO_InitStructure.Pin = GPIO_PIN_9;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	//konfiguracja PA10
	GPIO_InitStructure.Pin = GPIO_PIN_10;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;;
	GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);


	//wy³¹czone przerwania
	//HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0x0F, 0x00);
	//HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	//HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0x0F, 0x00);
	//HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

//Procedura obs³ugi przerwania od PA9 jeœli je w³aczono
void EXTI9_5_IRQHandler(void)
{


	/* EXTI line interrupt detected */
	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_9) != RESET)
	{

										//Tu wpisz procedurê obs³ugi przewania od portu PA9 jesli to konieczne


	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_9);

	}

}

//Procedura obs³ugi przerwania od PA10 jeœli je w³aczono
void EXTI15_10_IRQHandler(void)
{

	/* EXTI line interrupt detected */
	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_10) != RESET)
	{

										//Tu wpisz procedurê obs³ugi przewania od portu PA10 jesli to konieczne

	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_10);

	}

}
