#include <stdint.h>
#include <stdbool.h>
#include "15348.h"
#include "timer.h"
#include "serial.h"

#define PE0 (*((volatile uint32_t *)0x40024004))
#define PE1 (*((volatile uint32_t *)0x40024008))
#define PE2 (*((volatile uint32_t *)0x40024010))
#define PE3 (*((volatile uint32_t *)0x40024020))
#define PE4 (*((volatile uint32_t *)0x40024040))
#define PE5 (*((volatile uint32_t *)0x40024080))
  
#define PA2 (*((volatile uint32_t *)0x40004010))
#define PA3 (*((volatile uint32_t *)0x40004020))
#define PA4 (*((volatile uint32_t *)0x40004040))
#define PA5 (*((volatile uint32_t *)0x40004080))
#define PA6 (*((volatile uint32_t *)0x40004100))
#define PA7 (*((volatile uint32_t *)0x40004200))
  
#define PB0 (*((volatile uint32_t *)0x40005004))
#define PB1 (*((volatile uint32_t *)0x40005008))

/*
 * n = G F E D C B A
 * 0 = 1 0 0 0 0 0 0
 * 1 = 1 1 1 1 0 0 1
 * 2 = 0 1 0 0 1 0 0
 * 3 = 0 1 1 0 0 0 0
 * 4 = 0 0 1 1 0 0 1
 * 5 = 0 0 1 0 0 1 0
 * 6 = 0 0 0 0 0 1 0
 * 7 = 1 1 1 1 0 0 0
 * 8 = 0 0 0 0 0 0 0
 * 9 = 0 0 1 0 0 0 0
 */

static int segment[10][7] = {{0x01,0x00,0x00,0x00,0x00,0x00,0x00}, {0x01,0x01,0x01,0x01,0x00,0x00,0x01},
                             {0x00,0x01,0x00,0x00,0x01,0x00,0x00}, {0x00,0x01,0x01,0x00,0x00,0x00,0x00},
                             {0x00,0x00,0x01,0x01,0x00,0x00,0x01}, {0x00,0x00,0x01,0x00,0x00,0x01,0x00},
                             {0x00,0x00,0x00,0x00,0x00,0x01,0x00}, {0x01,0x01,0x01,0x01,0x00,0x00,0x00},
                             {0x00,0x00,0x00,0x00,0x00,0x00,0x00}, {0x00,0x00,0x01,0x00,0x00,0x00,0x00}};


                             
                             
/*
 * n = 0 1 2 3 4 5 6 7 8
 * 0 = 1 0 0 0 0 0 0 0 0
 * 1 = 0 1 0 0 0 0 0 0 0
 * 2 = 0 0 1 0 0 0 0 0 0
 * 3 = 0 0 0 1 0 0 0 0 0
 * 4 = 0 0 0 0 1 0 0 0 0
 * 5 = 0 0 0 0 0 1 0 0 0
 * 6 = 0 0 0 0 0 0 1 0 0
 * 7 = 0 0 0 0 0 0 0 1 0
 * 8 = 0 0 0 0 0 0 0 0 1
 */
                             
static int LCDs[9][9] = {{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, {0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
                         {0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00}, {0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00},
                         {0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00}, {0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00},
                         {0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00}, {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00},
                         {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01}};
                             
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

void set(int r, int i)
{
  switch (r)
  {
    case 0:
    {
      PE1 = 0X00;
      PE0 = i;
      PE1 = 0X02;
      break;
    }
    
    case 1:
    {
      PE4 = 0X00;
      if (i) PE3 = 0x08;
      else PE3 = 0x00;
      PE4 = 0X10;
      break;
    }
    
    case 2:
    {
      PA2 = 0X00;
      if (i) PE5 = 0x20; // 00100000
      else PE5 = 0x00;
      PA2 = 0X04;
      break;
    }
    
    case 3:
    {
      PA4 = 0X00;
      if (i) PA3 = 0x08; // 00100000
      else PA3 = 0x00;
      PA4 = 0X10;
      break;
    }
    
    case 4:
    {
      PA7 = 0X00;
      if (i) PA6 = 0x40; // 01000000
      else PA6 = 0x00;
      PA7 = 0X80;
      break;
    }

    case 5:
    {
      PB1 = 0X00;
      if (i) PB0 = 0x01; // 01000000
      else PB0 = 0x00;
      PB1 = 0X02;
      break;
    }
    
    default:
      break;
  }
  
  return;
}

void display_LCD_row(int r, int lcd, int digit)
{
  for (int j = 0; j < 16; j++)
  {
    if (j >= 0 && j < 7)
      set(r, segment[digit][j]);
    
    else set(r, LCDs[lcd][j - 7]);
  }
  
  return;
}


void update_LCD()
{
  for (int d = 0; d < 10; d++)
  {
    for (int c = 0; c < 500; c++)
    {
      for (int lcd = 0; lcd < 9; lcd++)
      {
        //for (int c = 0; c < 800; c++)`
        //{
          for (int r = 0; r < 9; r++)
          {
            display_LCD_row(r, lcd, d); 
          }
          PE2 = 0X00;
          PE2 = 0X04;
          PA5 = 0x00;
          PA5 = 0x20;
        //}
      }
    }
  }
}

/*
 * main.c
 */
int main(void)
{
  PLLInit();
  SystickInit();
  SetupSerial();
  
  
  // Configuring Port E
  SYSCTL_RCGCGPIO_R |= 0x10; // enable clock for PORT E
  GPIO_PORTE_LOCK_R = 0x4C4F434B; // this value unlocks the GPIOCR register.
  GPIO_PORTE_CR_R = 0xFF;
  GPIO_PORTE_AMSEL_R = 0x00; // disable analog functionality
  GPIO_PORTE_PCTL_R = 0x00000000; // Select GPIO mode in PCTL
  GPIO_PORTE_DIR_R = 0x3F; // Ports E0,1,2,3,4,5 are output, rest are input
  GPIO_PORTE_AFSEL_R = 0x00; // Disable alternate functionality
  GPIO_PORTE_DEN_R = 0xFF; //Enable digital ports

  // Configuring Port A
  SYSCTL_RCGCGPIO_R |= 0x01; // enable clock for PORT A
  GPIO_PORTA_LOCK_R = 0x4C4F434B; // this value unlocks the GPIOCR register.
  GPIO_PORTA_CR_R = 0xFF;
  GPIO_PORTA_AMSEL_R = 0x00; // disable analog functionality
  GPIO_PORTA_PCTL_R = 0x00000000; // Select GPIO mode in PCTL
  GPIO_PORTA_DIR_R = 0xFC; // Ports E0,1,2 are output, rest are input  11111100
  GPIO_PORTA_AFSEL_R = 0x00; // Disable alternate functionality
  GPIO_PORTA_DEN_R = 0xFF; //Enable digital ports

  // Configuring Port B
  SYSCTL_RCGCGPIO_R |= 0x02; // enable clock for PORT B
  GPIO_PORTB_LOCK_R = 0x4C4F434B; // this value unlocks the GPIOCR register.
  GPIO_PORTB_CR_R = 0xFF;
  GPIO_PORTB_AMSEL_R = 0x00; // disable analog functionality
  GPIO_PORTB_PCTL_R = 0x00000000; // Select GPIO mode in PCTL
  GPIO_PORTB_DIR_R = 0xFF; // Ports E0,1,2 are output, rest are input  11111100
  GPIO_PORTB_AFSEL_R = 0x00; // Disable alternate functionality
  GPIO_PORTB_DEN_R = 0xFF; //Enable digital ports

  while (1)
  {
    update_LCD();
  }

}
