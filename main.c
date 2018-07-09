// Michael Cheng 20672092
// Raveena D'Souza 20656255

#include <lpc17xx.h>
#include "stdio.h"
#include "stdlib.h"
#include "GLCD.h"
#include <RTL.h>
#include "timer.h"

// Define track distance
#define TRACK 70

// global variables
int lane; // 0 - 4
int health = 5;
int healthCheck = 0;
int firstGame = 0;
int distTrav = 0;
float speed;

// global times
float prevTime;
float time;
float startTime;
float endTime;
float totalTime;
char totalTimeC[12];
float counter;

// obstacle struct
// id: 0 pothole, 1 bird
typedef struct
{
	int id;
	int division;
	int lane;
} obs_t;                                           

// Create array of obstacles
obs_t obstacles[6];

// declare semaphores and mutexes
OS_SEM moveObsSem;
OS_SEM moveCarSem;
OS_SEM collisionSem;
OS_SEM pushbuttonSem;
OS_SEM changeSpeedSem;

// Initialize peripherals
void init(void)
{
	// lcd
	GLCD_Init();
	// timer
	timer_setup();
	// leds
	LPC_GPIO2->FIODIR |= 0x0000007C;
	LPC_GPIO1->FIODIR |= 0xB0000000;
	// potentiometer
	LPC_PINCON->PINSEL1 &= ~(0x03<<18);
	LPC_PINCON->PINSEL1 |= (0x01<<18);
	LPC_SC->PCONP |= 1<<12;
	LPC_ADC->ADCR = (1 << 2) | (4 << 8) | (1 << 21); 
}

// Draw lanes 
void drawLanes()
{
	int i, j;
	
	GLCD_Clear(DarkGrey);
	GLCD_SetBackColor(DarkGrey);
	GLCD_SetTextColor(White);
	
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

	GLCD_SetTextColor(Black);
	
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
			
	GLCD_SetTextColor(Red);
	
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

// potholes
void drawPot(obs_t p)
{
	int i, j;
	
	GLCD_SetTextColor(Black);
	
	for(i = -15; i < 16; i++)
		for(j = -15; j < 16; j++)
			if(i*i + j*j <= 256)
				GLCD_PutPixel(i+24+p.division*44, j+24+p.lane*48);
}

// draw bird
void drawBird(obs_t b)
{
	int i, j;
	
	GLCD_SetTextColor(Yellow);
	
	// Body
	for(i = -10; i < 11; i++)
		for(j = -10; j < 11; j++)
				GLCD_PutPixel(i+24+b.division*44, j+24+b.lane*48);
	
	GLCD_SetTextColor(Black);
	
	// Legs
	for(i = 0; i < 8; i++)
		for(j = 0; j < 3; j++)
			GLCD_PutPixel(i+6+b.division*44, j+21+b.lane*48);
	for(i = 0; i < 8; i++)
		for(j = 0; j < 3; j++)
			GLCD_PutPixel(i+6+b.division*44, j+27+b.lane*48);

	// Eyes
	for(i = -1; i < 1; i++)
		for(j = -1; j < 1; j++)
				GLCD_PutPixel(i+30+b.division*44, j+21+b.lane*48);
	for(i = -1; i < 1; i++)
		for(j = -1; j < 1; j++)
				GLCD_PutPixel(i+30+b.division*44, j+27+b.lane*48);
	
	GLCD_SetTextColor(Maroon);
	
	// Beak
	for(i = -2; i < 2; i++)
		for(j = -2; j < 2; j++)
				GLCD_PutPixel(i+24+b.division*44, j+24+b.lane*48);
}

// Erase car
void eraseCar(void)
{
	int i, j;
	
	GLCD_SetTextColor(DarkGrey);
	
	for(i = 0; i < 44; i++)
		for(j = 0; j < 36; j++)
			GLCD_PutPixel(i+2, j+6+lane*48);
}

// Erase the obstacles
void eraseObs(obs_t ob)
{
	int i, j;
	
	GLCD_SetTextColor(DarkGrey);
	
	for(i = 0; i < 44; i++)
		for(j = 0; j < 44; j++)
			GLCD_PutPixel(ob.division*44+i+2, j+2+ob.lane*48);
}

// Generate and update new obstacles
void updateObs(void)
{
	int r1, r2, r3, l1, l2, l3;
	
	if (obstacles[0].division < 0)
	{
		// randomly generate IDs for 3 new obstables
		r1 = rand() % 2;
		l1 = rand() % 5;
		
		r2 = rand() % 2;
		l2 = rand() % 5;
		while(l2 == l1)
			l2 = rand() % 5; 
		
		r3 = rand() % 2;
		l3 = rand() % 5;
		while(l3 == l1 || l3 == l2)
			l3 = rand() % 5; 
		
		// assign obstacles
		
		obstacles[0].id = r1;
		obstacles[0].division = 6;
		obstacles[0].lane = l1;
		
		obstacles[1].id = r2;
		obstacles[1].division = 6;
		obstacles[1].lane = l2;
		
		obstacles[2].id = r3;
		obstacles[2].division = 6;
		obstacles[2].lane = l3;
	}	
	if (obstacles[3].division < 0)
	{
		// randomly generate IDs for 3 new obstables
		l1 = rand() % 5;
		
		l2 = rand() % 5;
		while(l2 == l1)
			l2 = rand() % 5;
		
		l3 = rand() % 5;
		while(l3 == l1 || l3 == l2)
			l3 = rand() % 5; 
		
		// assign obstacles
		
		obstacles[3].id = rand() % 2;
		obstacles[3].division = 6;
		obstacles[3].lane = l1;
		
		obstacles[4].id = rand() % 2;
		obstacles[4].division = 6;
		obstacles[4].lane = l2;
		
		obstacles[5].id = rand() % 2;
		obstacles[5].division = 6;
		obstacles[5].lane = l3;
	}	
}

// Initialize everything
void initRestart(void)
{
	int l1, l2, l3;
	
	// LEDs
	LPC_GPIO1->FIOSET = 0x10000000;
	LPC_GPIO1->FIOSET = 0x20000000;
	LPC_GPIO1->FIOSET = 0x80000000;
	LPC_GPIO2->FIOSET = 0x00000004;
	LPC_GPIO2->FIOSET = 0x00000008;

	// variables
	health = 5;
	lane = 2;
	distTrav = 0;
	drawLanes();
	
	// obstacles
	
	l1 = rand() % 5;
	
	l2 = rand() % 5;
	while(l2 == l1)
		l2 = rand() % 5;
	
	l3 = rand() % 5;
	while(l3 == l1 || l3 == l2)
		l3 = rand() % 5; 
	
	obstacles[0].id = rand() % 2;
	obstacles[0].division = 6;
	obstacles[0].lane = l1;
	
	obstacles[1].id = rand() % 2;
	obstacles[1].division = 6;
	obstacles[1].lane = l2;
	
	obstacles[2].id = rand() % 2;
	obstacles[2].division = 6;
	obstacles[2].lane = l3;
	
	l1 = rand() % 5;
	
	l2 = rand() % 5;
	while(l2 == l1)
		l2 = rand() % 5;
	
	l3 = rand() % 5;
	while(l3 == l1 || l3 == l2)
		l3 = rand() % 5; 
	
	obstacles[3].id = rand() % 2;
	obstacles[3].division = 10;
	obstacles[3].lane = l1;
	
	obstacles[4].id = rand() % 2;
	obstacles[4].division = 10;
	obstacles[4].lane = l2;
	
	obstacles[5].id = rand() % 2;
	obstacles[5].division = 10;
	obstacles[5].lane = l3;
	
	updateObs();
	
	// car speed
	LPC_ADC->ADCR |= 0x01000000;
	speed = ((LPC_ADC->ADGDR & 0x0000FFF0) >> 4)/4000.0;
	//printf("pot: %f\n", speed);
	
	// timers
	startTime = timer_read()/1E6;
	prevTime = timer_read()/1E6;
}

// Countdown screen
void countdown(void)
{
	counter = timer_read()/1E6;
	
	GLCD_Clear(White);
	GLCD_SetBackColor(White);
	GLCD_SetTextColor(Black);
	
	GLCD_DisplayString(4, 9, 1, "3");
	while(timer_read()/1E6 - counter <= 1);
	GLCD_DisplayString(4, 9, 1, "2");
	while(timer_read()/1E6 - counter <= 2);
	GLCD_DisplayString(4, 9, 1, "1");
	while(timer_read()/1E6 - counter <= 3);
	GLCD_DisplayString(4, 8, 1, "Go!");
	while(timer_read()/1E6 - counter <= 3.5);
}

// change lanes
__task void moveCar (void)
{
	while(1)
	{
		os_sem_wait(&moveCarSem, 0xffff);
		
		// move left
		if (((LPC_GPIO1->FIOPIN & 0x00800000) != 0x00800000) && lane != 0)
		{
			eraseCar();
			lane--;
			drawCar();
			while(((LPC_GPIO1->FIOPIN & 0x00800000) != 0x00800000)) {}
		}
		
		// move right
		if (((LPC_GPIO1->FIOPIN & 0x02000000) != 0x02000000) && lane != 4)
		{
			eraseCar();
			lane++;
			drawCar();
			while((((LPC_GPIO1->FIOPIN & 0x02000000) != 0x02000000))) {}
		}
		
		// redraw car
		drawCar();
		
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
		
		// draw obstacles
		for (i = 0; i < 6; i++)
		{
			if (obstacles[i].id == 0 && obstacles[i].division >= 0)
				drawPot(obstacles[i]);
			else if (obstacles[i].id == 1 && obstacles[i].division >= 0)
				drawBird(obstacles[i]);
			else if (obstacles[i].division  < 0)
				updateObs();		
		}		
		
		// wait before redrawing obstacles based on speed
		if(timer_read()/1E6 - prevTime >= ((1.025/(1.025-speed))-1.0)/1.5)
		{
			for (i = 0; i < 6; i++)
			{
				eraseObs(obstacles[i]);
				if (obstacles[i].division >= 0)
					obstacles[i].division--;
			}
				//printf("1: %d, %d\n2: %d, %d\n3: %d, %d\n4: %d, %d\n5: %d, %d\n6: %d, %d\n\n", obstacles[0].lane, obstacles[0].division, obstacles[1].lane, obstacles[1].division, obstacles[2].lane, obstacles[2].division, obstacles[3].lane, obstacles[3].division, obstacles[4].lane, obstacles[4].division, obstacles[5].lane, obstacles[5].division);

			distTrav++;
			healthCheck = distTrav;
			prevTime = timer_read()/1E6;
		}
		
		os_sem_send(&collisionSem);
		os_tsk_pass();
	}
}

// Change speed of car
__task void changeSpeed()
{
	while(1)
	{
		os_sem_wait(&changeSpeedSem, 0xffff);
		LPC_ADC->ADCR |= 0x01000000;
		speed = ((LPC_ADC->ADGDR & 0x0000FFF0) >> 4)/4000.0;
	//	printf("pot: %f\n", speed);
		os_sem_send(&moveCarSem);
		os_tsk_pass();
	}
}

// Collision of car and obstacle
__task void collision()
{
	int i;
	while(1)
	{
		os_sem_wait(&collisionSem, 0xffff);
		for(i = 0; i < 6; i++)
		{
			// decrease health
			if (obstacles[i].lane == lane && obstacles[i].division == 0 && distTrav == healthCheck)
			{
				health--;
				healthCheck++;
				if (health == 4)
					LPC_GPIO2->FIOCLR = 0x00000008;
				else if (health == 3)
					LPC_GPIO2->FIOCLR = 0x00000004;
				else if(health == 2) 
					LPC_GPIO1->FIOCLR = 0x80000000;
				else if(health == 1)
					LPC_GPIO1->FIOCLR = 0x20000000;
				else if(health == 0)
					LPC_GPIO1->FIOCLR = 0x10000000;		
			}
		}
		os_sem_send(&pushbuttonSem);
		os_tsk_pass();
	}
}

// Start/restart game
__task void pushbutton()
{
	while(1)
	{
		os_sem_wait(&pushbuttonSem, 0xffff);
		
		// pressed pushbutton during game
		if ((LPC_GPIO2->FIOPIN & 0x00000400) != 0x00000400)
		{
			countdown();
			initRestart();
		}
		
		// first game
		else if (firstGame == 0)
		{
			// Start screen with instructions
			GLCD_Clear(Red);
			GLCD_SetBackColor(Red);
			GLCD_SetTextColor(Black);
			GLCD_DisplayString(2, 4, 1, "Speed Racer!");
			GLCD_DisplayString(10, 11, 0, "- Joystick to change lanes");
			GLCD_DisplayString(12, 11, 0, "- Potentiometer to change speed");
			GLCD_DisplayString(14, 11, 0, "- Pushbutton to start/restart");
			GLCD_DisplayString(16, 11, 0, "- LED's show lives remaining");
			GLCD_DisplayString(18, 11, 0, "- Go the distance as fast as possible");
			GLCD_DisplayString(20, 11, 0, "- Avoid the birds and potholes");
			GLCD_DisplayString(22, 11, 0, "- Have fun!");
			
			//wait for pushbutton
			while((LPC_GPIO2->FIOPIN & 0x00000400) == 0x00000400);
			firstGame = 1;
			countdown();
			initRestart();
		}
		
		// lost game
		else if (health <= 0)
		{
			GLCD_Clear(Red);
			GLCD_SetBackColor(Red);
			GLCD_SetTextColor(Black);
			GLCD_DisplayString(4, 5, 1, "FAILED :^(");
			
			//wait for pushbutton
			while((LPC_GPIO2->FIOPIN & 0x00000400) == 0x00000400);
			countdown();
			initRestart();
		}
		
		// finished game
		else if (distTrav >= TRACK)
		{
			endTime = timer_read()/1E6;
			totalTime = endTime - startTime;
			snprintf(totalTimeC, 12, "Time: %fs", totalTime);
			GLCD_Clear(Red);
			GLCD_SetBackColor(Red);
			GLCD_SetTextColor(Black);
			GLCD_DisplayString(4, 4, 1, "FINISHED :^)");
			GLCD_DisplayString(5, 4, 1, totalTimeC);
			
			//wait for pushbutton
			while((LPC_GPIO2->FIOPIN & 0x00000400) == 0x00000400);
			countdown();
			initRestart();
		}
		
		os_sem_send(&changeSpeedSem);
		os_tsk_pass();
	}
}

// Initialize all the tasks
__task void start_tasks()
{
	// semaphores
	os_sem_init(&moveCarSem, 0);
	os_sem_init(&moveObsSem, 0);
	os_sem_init(&collisionSem, 0);
	os_sem_init(&pushbuttonSem, 1);
	os_sem_init(&changeSpeedSem, 0);
	
	// tasks
	os_tsk_create(changeSpeed, 1);
	os_tsk_create(moveCar, 1);
	os_tsk_create(moveObs, 1);
	os_tsk_create(collision, 1);
	os_tsk_create(pushbutton, 1);
	
	while(1);
}

int main() 
{	
	printf("init");
	init();
	os_sys_init(start_tasks);
	
	return 0;
}
