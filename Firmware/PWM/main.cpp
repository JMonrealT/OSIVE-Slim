/*
 * PWM.cpp
 *
 * Created: 30/06/2020 16:03:04
 * Author : admin
 */ 


#include "sam.h"
#include "TimerClass.h"
#include "Serial.h"

TimerOut its_chewsday;
TimerOut T1;
TimerOut T2;
TimerOut T3;
TimerOut Tint;
uint8_t a = 0;
int16_t ka = 0;
volatile uint8_t data;

void keepaliveInit();
void keepalive();

char serialChars[8];
uint8_t serialPointer = 0;
bool msgrecv = false;

void UART_Handler(void) {
	if (UART->UART_SR & UART_SR_RXRDY) {
		UART->UART_IDR = 0xFFFFFFFF;
		data = UART->UART_RHR;
		serialChars[serialPointer] = data;
		serialPointer++;
		if(serialPointer == 8 || data == 0){
			serialPointer = 0;
			msgrecv = true;
		}
		uart_putchar(data);
		UART->UART_IER = UART_IER_RXRDY;
	}
}

void TC1_Handler(void){
	if (TC0->TC_CHANNEL[1].TC_SR && 0x000000010){ // If interrupt is caused by RC compare...
		its_chewsday.checkTimes();
		keepalive();
	}
}

int main(void)
{
    /* Initialize the SAM system */
    SystemInit();
	WDT->WDT_MR = WDT_MR_WDDIS;
	keepaliveInit();
	
	//Timer.init(numero del timer, frecuencia, duty A, duty B);
	its_chewsday.initPWM(0,20000,100,100); //Timer 0 (TC0 canal 0) salidas 2 (A) y 13 (B)
	its_chewsday.set_Times(1,999,1000,1000);
	T1.initPWM(6,2,50,50); //Timer 6 (TC2 canal 0) salidas 5 (A) y 4 (B)
	
	T2.initPWM(7,5,50,50); //Timer 7 (TC2 canal 1) salidas 3 (A) y 10 (B)
	
	T3.initPWM(8,10,50,50); // Timer 8 (TC2 canal 2) salidas 11 (A) y 12 (B)
	
	Tint.initINT(1,1);
	
	configure_uart();
	
   while (1) 
    {
		/*for(uint32_t i=0; i<320000; i++);
		if(ud) dc++; else dc--;
		if(dc == 100) ud = 0; else if (dc == 1) ud = 1;
		its_chewsday.set_B_Duty(dc);*/
		if(msgrecv){
			int t_on = (serialChars[2]<<2) + (serialChars[3]>>6);
			int t_off = (serialChars[4]<<2) + (serialChars[5]>>6);
			its_chewsday.set_Times(t_on,t_off,1000,1000);
			uart_putchar((char)t_on); uart_putchar('\n');
			uart_putchar((char)t_off); uart_putchar('\n');
			msgrecv = false;
		}
    }
}

void keepaliveInit(){
	PIOC->PIO_PDR |= PIO_PC25;
	PIOC->PIO_OER |= PIO_PC25;
	PIOC->PIO_CODR |= PIO_PC25;
}
void keepalive(){
	ka++;
	if(ka >= 990){
		PIOC->PIO_SODR |= PIO_PC25;
		if(ka>=1000){
			PIOC->PIO_CODR |= PIO_PC25;
			ka = 0;
			uart_putchar('a'); uart_putchar('\n');
		}
	}
}