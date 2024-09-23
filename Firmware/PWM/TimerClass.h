/*
 * TimerClass.h
 *
 * Created: 01/07/2020 14:17:49
 *  Author: admin
 */ 
#include <stdint.h>
#include "sam.h"

#ifndef TIMERCLASS_H_
#define TIMERCLASS_H_

class TimerOut
{
	private:
	uint32_t freq; // PWM frequency (Hz)
	float A_Duty; // Channel A duty cycle (0 to 100)
	float B_Duty; // Channel B duty cycle (0 to 100)
	uint32_t num; // Timer number saved for later use
	uint32_t temp;
	uint32_t A_Ton, A_Toff, B_Ton, B_Toff;
	
	
	public:
	uint32_t RC, RA, RB; // Compare register values (0 to 2^32)
	uint32_t A_OnOff_Time = 0;
	uint32_t B_OnOff_Time = 0;
	void initPWM(uint8_t timerNum, uint32_t in_freq,float in_aduty,float in_bduty);
	void initINT(uint8_t timerNum, uint32_t in_temp);
	void set_A_Duty(uint8_t duty);
	void set_B_Duty(uint8_t duty);
	void out_A_Off();
	void out_B_Off();
	void out_A_On();
	void out_B_On();
	void set_INT_temp(uint32_t temp_ms);
	void disable();
	void enable();
	void set_Times(uint32_t A_Tonin, uint32_t A_Toffin, uint32_t B_Tonin, uint32_t B_Toffin);
	void checkTimes();
	
	// Se crea un struct Timer con las constantes que están asociadas a ese timer,
	// por ejemplo el timer 0 (TC0) tiene un offset en el registro PMC de 27 bits, tiene los pines PB25 y PB27 etc
	// así luego si al programa le decimos usa el timer número 0, se consulta este struct para el número 0 y se configura
	// con lo que se ha guardado previamente
	struct Timer
	{
		Tc *tc; //Timer counter type
		uint32_t channel; // Channel number
		uint32_t pmc_off; // PMC register offset for the timer/channel
		Pio *pio; // gpio type
		uint32_t pio_numa; // Channel A pin number
		uint32_t pio_numb; // Channel B pin number
		uint32_t pio_absra; // Channel A pin function
		uint32_t pio_absrb; // Channel B pin function
		IRQn_Type irq;
	};
	
	// Vamos a tener 4 timers y 8 canales, los timers se numeran del 0 al 3 pero serán T0[0], T2[0, 1 y 2]
	static Timer timers[9];
};


#endif /* TIMERCLASS_H_ */