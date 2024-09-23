/*
 * Serial.h
 *
 * Created: 06/07/2020 11:36:08
 *  Author: admin
 */ 


#ifndef SERIAL_H_
#define SERIAL_H_

void configure_uart(void);
int uart_getchar(uint8_t *c);
int uart_putchar(const uint8_t c);
void uart_getString(uint8_t *c, int length);
void uart_putString(uint8_t *c, int length);



#endif /* SERIAL_H_ */