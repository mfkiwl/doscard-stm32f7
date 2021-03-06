#include "stm32f4xx.h"
#include "main.h"
#include "os.h"

#define IS42S16400J_SIZE             0x400000

int main(void)
{ 
  uint8_t ubWritedata_8b = 0x3C, ubReaddata_8b = 0;  
  uint16_t uhWritedata_16b = 0x1E5A, uhReaddata_16b = 0;  
  uint32_t uwReadwritestatus = 0;
  uint32_t counter = 0x0;
  
  /*!< At this stage the microcontroller clock setting is already configured, 
  this is done through SystemInit() function which is called from startup
  file (startup_stm32f429_439xx.s) before to branch to application main.
  To reconfigure the default setting of SystemInit() function, refer to
  system_stm32f4xx.c file
  */
  
  /* Initialize LEDs and user-push button mounted on STM32F429I-DISCOVERY */
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_GPIO); 
  
  /* Initialize the LCD */
  LCD_Init();
  LCD_LayerInit();
  LTDC_Cmd(ENABLE);
  LCD_SetLayer(LCD_FOREGROUND_LAYER);
  LCD_Clear(LCD_COLOR_WHITE);
  LCD_SetTransparency(200);
  LTDC_ReloadConfig(LTDC_IMReload);
  
  /* SDRAM Initialization */  
  SDRAM_Init();
  
  /* FMC SDRAM GPIOs Configuration */
  SDRAM_GPIOConfig();
  
  /* Disable write protection */
  FMC_SDRAMWriteProtectionConfig(FMC_Bank2_SDRAM,DISABLE); 

  /* Clear whole available SDRAM area */
  for (uint32_t counter = 0x00; counter < IS42S16400J_SIZE; counter++)
  {
	  *(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*counter) = (uint16_t)0x00;
  }

  /* Start our OS */
  OS();

  for (;;) ; //stop right there
  return 0; //would never happen, but make code analyzers happy
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  while (1)
  {}
}
#endif
