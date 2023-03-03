#include "stm32f4xx.h"

/************************************************************************
Function:       Boot1Init
Parameters:     none
Returns:        none
************************************************************************/
void BootJumperInit(void)
{

GPIO_InitTypeDef  GPIO_InitStructure;
/*!< Enable GPIO clocks */
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
GPIO_Init(GPIOH, &GPIO_InitStructure);
}/***** Boot1Init() *****/


/************************************************************************
Function:       GetBoot1State
Parameters:     none
Returns:        none
************************************************************************/
char GetBootJumperState(void)
{
if((GPIOH->IDR & 0x0002))
	return 1;
else
	return 0;
}/***** GetBootJumperState() *****/

/************************************************************************
Function:       ResetJumperInit
Parameters:     none
Returns:        none
************************************************************************/
void ResetJumperInit(void)
{

GPIO_InitTypeDef  GPIO_InitStructure;
/*!< Enable GPIO clocks */
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
GPIO_Init(GPIOB, &GPIO_InitStructure);
}/***** Boot1Init() *****/


/************************************************************************
Function:       GetBoot1State
Parameters:     none
Returns:        none
************************************************************************/
char GetResetJumperState(void)
{
if((GPIOB->IDR & 0x0002))
	return 1;
else
	return 0;
}/***** GetResetJumperState() *****/

