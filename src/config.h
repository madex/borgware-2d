#ifndef CONFIG_H_
#define CONFIG_H_

#include "autoconf.h"

#if defined(__cplusplus)
#define __BEGIN_DECLS \
	extern "C"        \
	{
#define __END_DECLS }
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif

#ifdef CAN_SUPPORT
#define BORG_CAN

// spi.[ch] defines
// spi port default config
#if SPI_PORTIDX == 0
#define SPI_PORT PORTA
#define SPI_DDR DDRA
#define SPI_PIN PINA
#elif SPI_PORTIDX == 1
#define SPI_PORT PORTB
#define SPI_DDR DDRB
#define SPI_PIN PINB
#elif SPI_PORTIDX == 2
#define SPI_PORT PORTC
#define SPI_DDR DDRC
#define SPI_PIN PINC
#elif SPI_PORTIDX == 3
#define SPI_PORT PORTD
#define SPI_DDR DDRD
#define SPI_PIN PIND
#endif

// can.[ch] defines
#define CAN_RX_BUFFER_SIZE 2 // only used for Interrupt
#define CAN_TX_BUFFER_SIZE 2 // only used for Interrupt
#endif

#define INIT_EEPROM

#if defined(UART_SUPPORT) && defined(LOLSHIELD) && defined(__AVR_ATmega32U4__)
#error UART not supported on Arduino Leonardo
#endif

#endif /* CONFIG_H_ */
