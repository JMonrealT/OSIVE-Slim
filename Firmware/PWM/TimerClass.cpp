/*
 * TimerClass.cpp
 *
 * Created: 01/07/2020 14:17:34
 *  Author: admin
 */ 

#include "TimerClass.h"
#include "sam.h"
#include <stddef.h>

void TimerOut::initPWM(uint8_t timerNum, uint32_t in_freq,float in_aduty,float in_bduty)
{
	freq = in_freq;
	A_Duty = in_aduty;
	B_Duty = in_bduty;
	num = timerNum;
	
	//Clock settings will be div by 2 in all timers by default so:
	// freq = 1/(RC*clk/2)
	RC = SystemCoreClock/(2.0*freq);
	RA = (uint32_t)((float)RC*((100.0-A_Duty)/100.0))+1;
	RB = (uint32_t)((float)RC*((100.0-B_Duty)/100.0))+1;
	
	timers[timerNum].pio->PIO_PDR |= timers[timerNum].pio_numa; // Disable I/O on pin PB25 to enable periph function
	timers[timerNum].pio->PIO_PDR |= timers[timerNum].pio_numb; // Disable I/O on pin PB27 (13) to enable periph function
	timers[timerNum].pio->PIO_ABSR |= timers[timerNum].pio_absra; // Assign peripheral B function
	timers[timerNum].pio->PIO_ABSR |= timers[timerNum].pio_absrb;
	
	if(timerNum < 5){
		PMC->PMC_PCER0 |= 1 << timers[timerNum].pmc_off;
	} else PMC->PMC_PCER1 |= 1 << timers[timerNum].pmc_off;

	
	// Está todo parametrizado para usar distintos timers y canales
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_CCR = TC_CCR_CLKDIS; // Disable clock while configuring
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_SR; // Clear status register (by reading it)
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_CMR = 0; // Clear configuration
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_CMR |= TC_CMR_TCCLKS_TIMER_CLOCK1; // Clock is MCL/2
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_CMR |= TC_CMR_CPCTRG; // RC Compare resets the counter
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_CMR |= TC_CMR_WAVE; // Compare mode, RC triggers interrupt
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_CMR |= TC_CMR_ASWTRG_CLEAR;
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_CMR |= TC_CMR_BSWTRG_CLEAR;
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_CMR |= TC_CMR_ACPA_SET; // On RA compare, set output
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_CMR |= TC_CMR_BCPB_CLEAR; // On RB compare, set output
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_CMR |= TC_CMR_ACPC_CLEAR; // On RC compare, reset output A
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_CMR |= TC_CMR_BCPC_SET; // On RC compare, reset output B
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_CMR |= TC_CMR_EEVT_XC0; // Do not use TIOxB as input
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_RA = RA; // Compare register values go here
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_RB = RB;
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_RC = RC;
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_CCR |= TC_CCR_SWTRG | TC_CCR_CLKEN; // Reset and enable the clock
}

void TimerOut::initINT(uint8_t timerNum, uint32_t in_temp){
	num = timerNum;
	temp = in_temp;
	
	RC = 0.001*temp*(SystemCoreClock/2.0);
	
	if(timerNum < 5){
		PMC->PMC_PCER0 |= 1 << timers[timerNum].pmc_off;
	} else PMC->PMC_PCER1 |= 1 << timers[timerNum].pmc_off;
	
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_CCR = 0; //Disable the timer while configuring
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_IDR = 0xFFFFFFFF; // Disable all interrupts
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_SR; // Clear status register (by reading it)
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_CMR = 0; //Deinitialize register
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_CMR |= TC_CMR_TCCLKS_TIMER_CLOCK1; // Clock is MCL/128 (probably 84 MHz / 128)
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_CMR |= 0x00004000; // Compare mode, RC triggers interrupt
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_RC = RC; // Value for 1 s? Approx
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_IER = 0x00000010; // Enable RC Compare interrupt
	NVIC_EnableIRQ(timers[timerNum].irq); // Enable TC0 interrupts in the NVIC periph
	timers[timerNum].tc->TC_CHANNEL[timers[timerNum].channel].TC_CCR |= 0x00000005; // Reset counter and enable clock
}

void TimerOut::set_A_Duty(uint8_t duty){
	RA = RC*(duty/100.0);
	timers[num].tc->TC_CHANNEL[timers[num].channel].TC_RA = RA;
}

void TimerOut::set_B_Duty(uint8_t duty){
	RB = RC*(duty/100.0);
	timers[num].tc->TC_CHANNEL[timers[num].channel].TC_RB = RB;
}

void TimerOut::disable(){
	timers[num].tc->TC_CHANNEL[timers[num].channel].TC_CCR = TC_CCR_SWTRG | TC_CCR_CLKDIS;
	timers[num].tc->TC_CHANNEL[timers[num].channel].TC_SR; // Clear status register (by reading it)
}

void TimerOut::enable(){
	timers[num].tc->TC_CHANNEL[timers[num].channel].TC_SR; // Clear status register (by reading it)
	timers[num].tc->TC_CHANNEL[timers[num].channel].TC_CCR = TC_CCR_SWTRG | TC_CCR_CLKEN;
}

// En esta parte de la librería especificamos con qué constantes rellenamos la estructura para asociar a los timers
//0 TC0 TC0-0 PCER0 ID_TC0 27 PWM
//1 TC1 TC0-1 PCER0 ID_TC1 28
//2 TC2 TC0-2 PCER0 ID_TC2 29
//3 TC3 TC1-0 PCER0 ID_TC3 30
//4 TC4 TC1-1 PCER0 ID_TC4 31
//5 TC5 TC1-2 PCER1 ID_TC5 32
//6 TC6 TC2-0 PCER1 ID_TC6 33 PWM
//7 TC7 TC2-1 PCER1 ID_TC7 34 PWM
//8 TC8 TC2-2 PCER1 ID_TC8 35 PWM
TimerOut::Timer TimerOut::timers[9] = {
	{TC0, 0, 27, PIOB, PIO_PB25,PIO_PB27,PIO_PB25B_TIOA0, PIO_PB27B_TIOB0, TC0_IRQn},
	{TC0, 1, 28, NULL, NULL, NULL, NULL, NULL, TC1_IRQn},
	{TC0, 2, 29, NULL, NULL, NULL, NULL, NULL, TC2_IRQn},
	{TC1, 0, 30, NULL, NULL, NULL, NULL, NULL, TC3_IRQn},
	{TC1, 1, 31, NULL, NULL, NULL, NULL, NULL, TC4_IRQn},
	{TC1, 2, 0, NULL, NULL, NULL, NULL, NULL, TC5_IRQn},
	{TC2, 0, 1, PIOC, PIO_PC25, PIO_PC26,PIO_PC25B_TIOA6, PIO_PC26B_TIOB6, TC6_IRQn},
	{TC2, 1, 2, PIOC, PIO_PC28, PIO_PC29,PIO_PC28B_TIOA7,PIO_PC29B_TIOB7, TC7_IRQn},
	{TC2, 2, 3, PIOD, PIO_PD7, PIO_PD8,PIO_PD7B_TIOA8,PIO_PD8B_TIOB8, TC8_IRQn},
}; // La declaracion de tantos nulls para rellenar uint32 va a dar warnings

void TimerOut::out_A_Off(){
	timers[num].tc->TC_CHANNEL[timers[num].channel].TC_RA = 0;
}
void TimerOut::out_B_Off(){
	timers[num].tc->TC_CHANNEL[timers[num].channel].TC_RB = 0;
}
void TimerOut::out_A_On(){
	timers[num].tc->TC_CHANNEL[timers[num].channel].TC_RA = RA;
}
void TimerOut::out_B_On(){
	timers[num].tc->TC_CHANNEL[timers[num].channel].TC_RB = RB;
}

void TimerOut::set_INT_temp(uint32_t temp_ms){
	timers[num].tc->TC_CHANNEL[timers[num].channel].TC_RC = 0.001*temp_ms*(SystemCoreClock/128.0);
}

void TimerOut::set_Times(uint32_t A_Tonin, uint32_t A_Toffin, uint32_t B_Tonin, uint32_t B_Toffin){
	A_Ton = A_Tonin;
	A_Toff = A_Toffin;
	B_Ton = B_Tonin;
	B_Toff = B_Toffin;
}

void TimerOut::checkTimes(){
	A_OnOff_Time++;
	B_OnOff_Time++;
	
	if(A_OnOff_Time >= (A_Ton + A_Toff)){ 
		out_A_On();
		A_OnOff_Time = 0;
	}else if(A_OnOff_Time >= A_Ton) out_A_Off();
	
	if(B_OnOff_Time >= (B_Ton + B_Toff)){
		out_B_On();
		B_OnOff_Time = 0;
	}else if(B_OnOff_Time >= B_Ton) out_B_Off();
}