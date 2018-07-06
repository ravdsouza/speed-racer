// Michael Cheng 20672092
// Raveena D'Souza 20656255

#include <lpc17xx.h>
#include "stdio.h"
#include "stdlib.h"
#include "GLCD.h"
#include <RTL.h>
#include "timer.h"

// delay: 		os_dly_wait(300);

// global variables
int lane; // 0 - 4
int speed;
int oneY;
int oneX;
float prevTime = 0;
float time;

// obstacle struct
// id: 0 pothole, 1 bird
typedef struct
{
	int id;
	int division;
	int lane;
} obs_t;

obs_t obstacles[6];
obs_t pot;

// declare semaphores and mutexes
OS_SEM moveObsSem;
OS_SEM moveCarSem;
OS_SEM changeSpeedSem;

OS_MUT mut;

// initialize everything
void init(void)
{
	GLCD_Init();
	timer_setup();
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
			
	GLCD_SetTextColor(31727);
}

// potholes
void drawPot(obs_t p)
{
	int i, j;
	GLCD_SetTextColor(0);
	for(i = -15; i < 16; i++)
		for(j = -15; j < 16; j++)
		{
			if(i*i + j*j <= 256)
				GLCD_PutPixel(i+24+p.division*44, j+24+p.lane*48);
		}
}

// draw bird
void drawBird(obs_t b)
{
	int i, j;
	GLCD_SetTextColor(0);
	for(i = -15; i < 16; i++)
		for(j = -15; j < 16; j++)
		{
				GLCD_PutPixel(i+24+b.division*44, j+24+b.lane*48);
		}
}
 
void eraseCar(void)
{
	int i, j;
	GLCD_SetTextColor(31727);
	
	for(i = 0; i < 44; i++)
		for(j = 0; j < 36; j++)
			GLCD_PutPixel(i+2, j+6+lane*48);
}

void eraseObs(obs_t ob)
{
	int i, j;
	GLCD_SetTextColor(31727);
	
	for(i = 0; i < 44; i++)
		for(j = 0; j < 44; j++)
			GLCD_PutPixel(ob.division*44+i+2, j+2+ob.lane*48);
}

// Update Obstacles
void updateObs(void)
{
	int r1, r2, r3, l1, l2, l3;
	if (obstacles[0].division <= 0)
	{
		// randomly generate IDs for 2 new obstables
		r1 = rand() % 2;
		l1 = rand() % 5;
		
		r2 = rand() % 2;
		l2 = rand() % 5;
		
		r3 = rand() % 2;
		l3 = rand() % 5;
		
		obstacles[0] = obstacles[3];
		obstacles[1] = obstacles[4];
		obstacles[2] = obstacles[5];
		
		obstacles[3].id = r1;
		obstacles[3].division = 6;
		obstacles[3].lane= l1;
		
		obstacles[4].id = r2;
		obstacles[4].division = 6;
		obstacles[4].lane= l2;
		
		obstacles[5].id = r3;
		obstacles[5].division = 6;
		obstacles[5].lane= l3;
	}
}

// change lanes
__task void moveCar (void)
{
	while(1)
	{
		os_sem_wait(&moveCarSem, 0xffff);
		if (((LPC_GPIO1->FIOPIN & 0x00800000) != 0x00800000) && lane != 0)
		{
			//os_sem_wait(&moveCarSem, 0xffff);
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
			//os_sem_send(&moveObsSem);
			while((((LPC_GPIO1->FIOPIN & 0x02000000) != 0x02000000))) {}
		}
		drawCar();
		//while(((LPC_GPIO1->FIOPIN & 0x00800000) != 0x00800000) || (((LPC_GPIO1->FIOPIN & 0x02000000) != 0x02000000))) {}
		os_sem_send(&moveObsSem);
		os_tsk_pass();		
	}
}

// move obstacles down the LCD
__task void moveObs(void)
{
	int i;
	while(1)
	{
		os_sem_wait(&moveObsSem, 0xffff);
		for (i = 0; i < 4; i++)
		{
			if (obstacles[i].id == 0 && obstacles[i].division >= 0)
				drawPot(obstacles[i]);
			else if (obstacles[i].id == 1 && obstacles[i].division >= 0)
				drawBird(obstacles[i]);
			else if (obstacles[i].division  < 0)
				updateObs();		
		}		
		//os_dly_wait(50);
		if(timer_read()-prevTime >= 0)
			for (i = 0; i < 4; i++)
			{
				eraseObs(obstacles[i]);
				if (obstacles[i].division >= 0)
					obstacles[i].division--;
			}
		prevTime = timer_read();			
		os_sem_send(&moveCarSem); // change this sem so its not circular
		os_tsk_pass();
	}
}

/*
__task void changeSpeed()
{
	LPC_PINCON->PINSEL1 &= ~(0x03<<18);
	LPC_PINCON->PINSEL1 |= (0x01<<18);
	LPC_SC->PCONP |= 1<<12;
	LPC_ADC->ADCR = (1 << 2) | (4 << 8) | (1 << 21); 
	while(1)
	{
		LPC_ADC->ADCR |= 0x01000000;
		if(LPC_ADC->ADGDR & 0x80000000)
		{
			printf("potentiometer: %d\n\n", (LPC_ADC->ADGDR & 0x0000FFF0) >> 4); 
			speed = (LPC_ADC->ADGDR & 0x0000FFF0) >> 4;
			printf("%d", speed);
		}
		os_tsk_pass();
	}
}
*/
__task void start_tasks()
{
	os_sem_init(&moveCarSem, 1);
	os_sem_init(&moveObsSem, 0);
	
	//os_tsk_create(changeSpeed, 1);
	os_tsk_create(moveCar, 1);
	os_tsk_create(moveObs, 1);
	while(1);
}


int main() 
{	
	printf("init");
	init();
	lane = 0;
	drawLanes();
	
	obstacles[0].id = rand() % 2;
	obstacles[0].division = 6;
	obstacles[0].lane= rand() % 5;
	
	obstacles[1].id = rand() % 2;
	obstacles[1].division = 6;
	obstacles[1].lane= rand() % 5;
	
	obstacles[2].id = rand() % 2;
	obstacles[2].division = 6;
	obstacles[2].lane= rand() % 5;
	
	obstacles[3].id = rand() % 2;
	obstacles[3].division = 10;
	obstacles[3].lane= rand() % 5;
	
	obstacles[4].id = rand() % 2;
	obstacles[4].division = 10;
	obstacles[4].lane= rand() % 5;
	
	obstacles[5].id = rand() % 2;
	obstacles[5].division = 10;
	obstacles[5].lane= rand() % 5;
	
	updateObs();
	
	os_sys_init(start_tasks);
	
	return 0;
}
