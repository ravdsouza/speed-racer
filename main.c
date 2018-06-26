// Michael Cheng 20672092
// Raveena D'Souza 20656255

#include <lpc17xx.h>
#include "stdio.h"
#include "stdlib.h"
#include "GLCD.h"
#include <RTL.h>

// global variables
int lane; // 0 - 4
int speed;

// initialize everything
void init(void)
{
	GLCD_Init();
}

// draw lanes
void drawLanes()
{
	int i, j;
	
	GLCD_Clear(31727);
	GLCD_SetBackColor(31727);
	GLCD_SetTextColor(65535);
	
	for (i = 0; i < 320; i++)
	{
		GLCD_PutPixel(i, 0);
		GLCD_PutPixel(i, 1);
		
		GLCD_PutPixel(i, 238);
		GLCD_PutPixel(i, 239);
		
		for (j = 0; j < 4; j++)
		{			
			GLCD_PutPixel(i, j+46);
			GLCD_PutPixel(i, j+94);
			GLCD_PutPixel(i, j+142);
			GLCD_PutPixel(i, j+190);
			
			GLCD_PutPixel(i, j+45);
		}
	}
	
	for (j = 0; j < 240; j++)
	{
		GLCD_PutPixel(0, j);
		GLCD_PutPixel(1, j);
		
		GLCD_PutPixel(318, j);
		GLCD_PutPixel(319, j);
	}
}

// draw car
void drawCar()
{
	int i, j;
	// Black
	GLCD_SetTextColor(0);
	// Axles
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			GLCD_PutPixel(i+10, j+10+lane*48);
	
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			GLCD_PutPixel(i+22, j+10+lane*48);
	
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			GLCD_PutPixel(i+10, j+34+lane*48);
	
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			GLCD_PutPixel(i+22, j+34+lane*48);
	// Wheels
	for(i = 0; i < 8; i++)
		for(j = 0; j < 4; j++)
			GLCD_PutPixel(i+8, j+6+lane*48);
	
	for(i = 0; i < 8; i++)
		for(j = 0; j < 4; j++)
			GLCD_PutPixel(i+20, j+6+lane*48);
	
	for(i = 0; i < 8; i++)
		for(j = 0; j < 4; j++)
			GLCD_PutPixel(i+8, j+38+lane*48);
			
	for(i = 0; i < 8; i++)
		for(j = 0; j < 4; j++)
			GLCD_PutPixel(i+20, j+38+lane*48);
			
	// Red
	GLCD_SetTextColor(63488);
	// Body
	for(i = 0; i < 24; i++)
		for(j = 0; j < 20; j++)
			GLCD_PutPixel(i+6, j+14+lane*48);
	// Neck		
	for(i = 0; i < 12; i++)
		for(j = 0; j < 8; j++)
			GLCD_PutPixel(i+30, j+20+lane*48);
	//Spoiler
	for(i = 0; i < 8; i++)
		for(j = 0; j < 36; j++)
			GLCD_PutPixel(i+38, j+6+lane*48);
}

void eraseCar()
{
	int i, j;
	GLCD_SetTextColor(31727);
	
	for(i = 0; i < 44; i++)
		for(j = 0; j < 36; j++)
			GLCD_PutPixel(i+2, j+6+lane*48);
}

// change lanes
__task void moveCar (void)
{
	while(1)
	{
		if (((LPC_GPIO1->FIOPIN & 0x00800000) != 0x00800000) && lane != 0)
		{
			lane--;
		}
		else if (((LPC_GPIO1->FIOPIN & 0x02000000) != 0x02000000) && lane != 4)
		{
			lane++;
		}
		drawCar();
		while(((LPC_GPIO1->FIOPIN & 0x00800000) != 0x00800000) || (((LPC_GPIO1->FIOPIN & 0x02000000) != 0x02000000)))
		{}
	}
}

__task void start_tasks()
{
	os_tsk_create(moveCar, 1);
	while(1);
}

int main() 
{
	init();
	lane = 0;
	drawLanes();
	//os_sys_init(start_tasks);
	
	while(1)
	{
		if (((LPC_GPIO1->FIOPIN & 0x00800000) != 0x00800000) && lane != 0)
		{
			eraseCar();
			lane--;
			drawCar();
			while(((LPC_GPIO1->FIOPIN & 0x00800000) != 0x00800000)) {}
		}
		if (((LPC_GPIO1->FIOPIN & 0x02000000) != 0x02000000) && lane != 4)
		{
			eraseCar();
			lane++;
			drawCar();
			while((((LPC_GPIO1->FIOPIN & 0x02000000) != 0x02000000))) {}
		}
		drawCar();
		//while(((LPC_GPIO1->FIOPIN & 0x00800000) != 0x00800000) || (((LPC_GPIO1->FIOPIN & 0x02000000) != 0x02000000))) {}

		
	}
	
	return 0;
}
