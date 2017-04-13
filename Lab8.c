// Lab8.c
// Runs on LM4F120 or TM4C123
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly
// Last Modified: 4/5/2016 

// Analog Input connected to PE2=ADC1
// displays on Sitronox ST7735
// PF3, PF2, PF1 are heartbeats

#include <stdint.h>

#include "ST7735.h"
#include "TExaS.h"
#include "ADC.h"
#include "print.h"
#include "tm4c123gh6pm.h"

//*****the first three main programs are for debugging *****
// main1 tests just the ADC and slide pot, use debugger to see data
// main2 adds the LCD to the ADC and slide pot, ADC data is on Nokia
// main3 adds your convert function, position data is no Nokia

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))

uint64_t ADCMail;
uint64_t ADCStatus;
// Initialize Port F so PF1, PF2 and PF3 are heartbeats
void PortF_Init(void){ 
	
		volatile int delay;
		SYSCTL_RCGCGPIO_R |= 0x20;		//enable the clock for PortF
		delay = SYSCTL_RCGCADC_R;       // extra time to stabilize
		delay = SYSCTL_RCGCADC_R;       // extra time to stabilize
		delay = SYSCTL_RCGCADC_R;       // extra time to stabilize
		delay = SYSCTL_RCGCADC_R;
		GPIO_PORTF_DIR_R |= 0x04;				//setting PF2 as output
		GPIO_PORTF_AMSEL_R &= ~ 0x04;
		GPIO_PORTF_PCTL_R &= ~ 0x04;
		GPIO_PORTF_DIR_R |= 0x04;
		GPIO_PORTF_AFSEL_R &= ~ 0x04;
		GPIO_PORTF_DEN_R |= 0x04;
	
}
uint32_t Data;        // 12-bit ADC
uint32_t Position;    // 32-bit fixed-point 0.001 cm
int main1(void){      // single step this program and look at Data
  TExaS_Init();       // Bus clock is 80 MHz 
  ADC_Init();         // turn on ADC, set channel to 1
  while(1){                
    Data = ADC_In();  // sample 12-bit channel 1
  }
}

int main2(void){			//gets a regression line
  TExaS_Init();       // Bus clock is 80 MHz 
  ADC_Init();         // turn on ADC, set channel to 1
  ST7735_InitR(INITR_REDTAB); 
  PortF_Init();
  while(1){           // use scope to measure execution time for ADC_In and LCD_OutDec           
    PF2 = 0x04;       // Profile ADC
    Data = ADC_In();  // sample 12-bit channel 1
    PF2 = 0x00;       // end of ADC Profile
    ST7735_SetCursor(0,0);
    PF1 = 0x02;       // Profile LCD
    LCD_OutDec(Data); 
    ST7735_OutString("   ");  // these spaces are used to coverup characters from last output
    PF1 = 0;          // end of LCD Profile
  }
}

uint32_t Convert(uint32_t input){
	uint32_t output;
	output = input - 0.5909;
	output= (output/2);
	ST7735_OutString(""); 
  return (output);
}


void SysTick_Init(){
	NVIC_ST_CTRL_R &= 0x00;
	NVIC_ST_RELOAD_R = 3000000;  //40 hz freq
	NVIC_ST_CURRENT_R = 0x00;
	NVIC_SYS_PRI3_R= (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000;
	NVIC_ST_CTRL_R = 0x00000007;
}


int main3(void){ 				//
  TExaS_Init();         // Bus clock is 80 MHz 
  ST7735_InitR(INITR_REDTAB); 
  PortF_Init();
  ADC_Init();         // turn on ADC, set channel to 1
  SysTick_Init();
	while(1){  
//    PF2 ^= 0x04;      // Heartbeat
//    Data = ADC_In();  // sample 12-bit channel 1
//    PF3 = 0x08;       // Profile Convert
//    Position = Convert(Data); 
//    PF3 = 0;          // end of Convert Profile
//    PF1 = 0x02;       // Profile LCD
//    ST7735_SetCursor(0,0);
//    LCD_OutDec(Data); ST7735_OutString(" "); 
//    ST7735_SetCursor(6,0);
//    LCD_OutFix(Position);
//    PF1 = 0;          // end of LCD Profile
		if (ADCStatus == 1){  ///////////////
      uint64_t mailbox;
      mailbox = ADCMail;			//read the mailbox
			ADCStatus = 0;
			mailbox = Convert(mailbox);
			LCD_OutFix(mailbox);
			ST7735_OutString("cm");
			ST7735_SetCursor(0,0);
    }
  }  
}
int main(void){
  main3();
}


//SysTick_Init
//    ; disable SysTick during setup
//    LDR R1, =NVIC_ST_CTRL_R         ; R1 = &NVIC_ST_CTRL_R
//    MOV R0, #0                      ; R0 = 0
//    STR R0, [R1]                    ; [R1] = R0 = 0
//    ; maximum reload value
//    LDR R1, =NVIC_ST_RELOAD_R       ; R1 = &NVIC_ST_RELOAD_R
//    LDR R0, =NVIC_ST_RELOAD_M;      ; R0 = NVIC_ST_RELOAD_M
//    STR R0, [R1]                    ; [R1] = R0 = NVIC_ST_RELOAD_M
//    ; any write to current clears it
//    LDR R1, =NVIC_ST_CURRENT_R      ; R1 = &NVIC_ST_CURRENT_R
//    MOV R0, #0                      ; R0 = 0
//    STR R0, [R1]                    ; [R1] = R0 = 0
//    ; enable SysTick with core clock
//    LDR R1, =NVIC_ST_CTRL_R         ; R1 = &NVIC_ST_CTRL_R
//                                    ; R0 = ENABLE and CLK_SRC bits set
//    MOV R0, #(NVIC_ST_CTRL_ENABLE+NVIC_ST_CTRL_CLK_SRC)
//    STR R0, [R1]                    ; [R1] = R0 = (NVIC_ST_CTRL_ENABLE|NVIC_ST_CTRL_CLK_SRC)
//    BX  LR                          ; return

void SysTick_Wait(){ // translated from assembly
	
	NVIC_ST_RELOAD_R--;
	
}
void SysTick_Wait_loop(){ //	translated from assembly
	
	while(NVIC_ST_CTRL_R == 0x00010000)
	{
	}
}

void SysTick_Handler (void){
	  PF2 ^= 0x04;
	  PF2 ^= 0x04;
		ADCMail = ADC_In();
	  ADCStatus = 1;
	  PF2 ^= 0x04;
}
//;------------SysTick_Wait------------
//; Time delay using busy wait.
//; Input: R0  delay parameter in units of the core clock (units of 12.5 nsec for 80 MHz clock)
//; Output: none
//; Modifies: R0, R1, R3
//SysTick_Wait
//    LDR  R1, =NVIC_ST_RELOAD_R      ; R1 = &NVIC_ST_RELOAD_R
//    SUB  R0, #1
//    STR  R0, [R1]                   ;delay-1;  // number of counts to wait
//    LDR  R1, =NVIC_ST_CTRL_R        ; R1 = &NVIC_ST_CTRL_R
//SysTick_Wait_loop
//    LDR  R3, [R1]                   ; R3 = NVIC_ST_CTRL_R
//    ANDS R3, R3, #0x00010000       ; Count set?
//    BEQ  SysTick_Wait_loop
//    BX   LR                         ; return
