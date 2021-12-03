#include <stdint.h>
#include <stdbool.h>
#include "15348.h"
#include "timer.h"
#include "serial.h"

#define epsilon 30000
#define header 4500 * 80
#define zero 560 * 80
#define one 1690 * 80
#define repeat 2250 * 80
#define header_begin 9000 * 80


void PLLInit()
{
    SYSCTL_RCC2_R |= 0x80000000;
    SYSCTL_RCC2_R |= 0x00000800;
    SYSCTL_RCC_R = (SYSCTL_RCC_R & ~0x000007C0) + 0x00000540;
    SYSCTL_RCC2_R &= ~0x00000070;
    SYSCTL_RCC2_R &= ~0x00002000;
    SYSCTL_RCC2_R |= 0x40000000;
    SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~0x1FC00000) + (4 << 22);
    while ((SYSCTL_RIS_R &0x00000040)==0){};
    SYSCTL_RCC2_R &= ~0x00000800;
}

static volatile int pulse_width;
static volatile int first;
static volatile int data;
static volatile int state;
static volatile int done;
static volatile int rising;
static volatile int num_bits;
static volatile int temp_data;
static volatile int p;

void PWMeasure_Init() {
	first = 0xFFFF;
	done = 0;
	data = 0;
	temp_data = 0;
	state = 0;
	// TIMER0_IMR_R |= TIMER_IMR_CAEIM; // disable all interrupts for timer0A
	// TIMER0_ICR_R = TIMER_ICR_CAECINT; // clear timer0A capture match flag
	
	SYSCTL_RCGCTIMER_R |= 0x01;     // Activate Timer 0
	SYSCTL_RCGCGPIO_R |= 0x02;      // Activate Port B
	GPIO_PORTB_DIR_R &= ~0x40; // Make PB6 an input
	GPIO_PORTB_AFSEL_R  |= 0x40; // Alternate Functionality Select for PB6
	GPIO_PORTB_DEN_R |= 0x40; // Enable Digital Functionality
	GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xF0FFFFFF) + 0x07000000;
	TIMER0_CTL_R &= ~0x00000001; // Disable timer 0 for configuration ? more on this in a bit
	TIMER0_CFG_R = 0x00000004; // Configure for 16-bit capture mode
	TIMER0_TAMR_R = 0x00000007; // configure for input capture mode
	TIMER0_CTL_R |= 0x0000000C; // configure for rising edge ? more on this in a bit
	TIMER0_TAILR_R = 0x0000FFFF; // Start value for count down
	TIMER0_TAPR_R = 0xFF; // Activate pre-scale
	TIMER0_IMR_R |= 0x00000004; // Enable Input capture interrupts
	TIMER0_ICR_R = 0x00000004; // Clear Timer0A capture match flag
	TIMER0_CTL_R |= 0x00000001; // Timer 0A 24-bit, rising and falling edge
	NVIC_PRI4_R = (NVIC_PRI4_R & 0x00FFFFFF) | 0x40000000;
	NVIC_EN0_R = 1<<19;
}

void setupPortE() {
	SYSCTL_RCGCGPIO_R |= 0x10; // enable clock for PORT E
	GPIO_PORTE_LOCK_R = 0x4C4F434B; // this value unlocks the GPIOCR register.
	GPIO_PORTE_CR_R = 0xFF;
	GPIO_PORTE_AMSEL_R = 0x00; // disable analog functionality
	GPIO_PORTE_PCTL_R = 0x00000000; // Select GPIO mode in PCTL
	GPIO_PORTE_DIR_R = 0x0; // everything is input
	GPIO_PORTE_AFSEL_R = 0x00; // Disable alternate functionality
	GPIO_PORTE_DEN_R = 0xFF; //Enable digital ports
}

int which_pulse(void);

int absDiff(int a, int b);

int absDiff(int a, int b) {
	return a > b ? a - b : b - a;
}

int which_pulse() {
	if (absDiff(pulse_width, zero) < epsilon)
		return 0;
	if (absDiff(pulse_width, one) < epsilon)
		return 1;
	if (absDiff(pulse_width, header) < epsilon)
		return 2;
	if (absDiff(pulse_width, repeat) < epsilon)
		return 3;
	if (absDiff(pulse_width, header_begin) < epsilon)
		return 4;
	return 5;
}

void TIMER_Handler() {
	TIMER0_ICR_R = 0x4; // acknowledge the interrupt
	pulse_width = (first - TIMER0_TAR_R) & 0xFFFFFF;
	first = TIMER0_TAR_R;
	rising = GPIO_PORTB_DATA_R;
	p = which_pulse();
	int s = state;
	int r = rising;
	if (state == 0) {
		if (rising && p == 4) state = 1;
	}
	else if (state == 1) {
		if (!rising && p == 2) state = 2;
		else state = 0;
	}
	else if (state == 2) {
		if (num_bits >= 32) {
			data = temp_data;
			num_bits = 0;
			temp_data = 0;
			state = 0;
		}
		else if (rising && p == 0) state = 3;
		else state = 0;
	}
	else if (state == 3) {
		if (p != 0 && p != 1) state = 0;
		if (rising) state = 0;
		else {
			temp_data = temp_data << 1 | p;
			state = 2;
			num_bits++;
		}
	}
	else 
		state = 0;
}

void write_function(int r, int c) {
	char row[128];
	sprintf(row, "row is %d\n", r);
	char col[128];
	sprintf(col, "col is %d\n", c);
	SerialWrite(row);
	SerialWrite(col);
}

/*
 * main.c
 */
int main(void)
{
  PLLInit();
  SystickInit();
  SetupSerial();
	PWMeasure_Init();
	//setupPortE();
								
	int signal;
	
	struct IR {
	int row;
	int col;
	};
	
	struct IR ir_struct;
	ir_struct.row = 0;
	ir_struct.col = 0;
		
	while(1) {
		while(!data) {}
		signal = data;
		if (signal == 0x00FF9867) {
			//up
			if(ir_struct.row != 0)
				ir_struct.row--;		
		}
		
		if (signal == 0x00FF38C7) {
			//down
			if(ir_struct.row != 8)
				ir_struct.row++;
		}
		
		if (signal == 0x00FF30CF) { //left
			if (ir_struct.col != 0)
				ir_struct.col--;
		}
		
		if (signal == 0x00FF7A85) { //right
			if(ir_struct.col != 8) 
				ir_struct.col++;
		}
		
		if (signal == 0x00FF18E7) write_function(ir_struct.row, ir_struct.col);
		data = 0;
		signal = 0;
	}
		
}
