#ifndef _RPI_GPIO_H_
#define _RPI_GPIO_H_

#include <stdint.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GPIO_IN  0
#define GPIO_OUT 1


#define RPI_GPIO_04 RPI_V2_GPIO_P1_07 // Version 2, Pin P1-07
#define RPI_GPIO_17 RPI_V2_GPIO_P1_11 // Version 2, Pin P1-11
#define RPI_GPIO_18 RPI_V2_GPIO_P1_12 // Version 2, Pin P1-12
#define RPI_GPIO_22 RPI_V2_GPIO_P1_15 // Version 2, Pin P1-15
#define RPI_GPIO_23 RPI_V2_GPIO_P1_16 // Version 2, Pin P1-16
#define RPI_GPIO_24	RPI_V2_GPIO_P1_18 // Version 2, Pin P1-18
#define RPI_GPIO_25 RPI_V2_GPIO_P1_22 // Version 2, Pin P1-22
#define RPI_GPIO_27 RPI_V2_GPIO_P1_13 // Version 2, Pin P1-13
// RPi Version 2, new plug P5
#define RPI_GPIO_28 RPI_V2_GPIO_P5_03 // Version 2, Pin P5-03
#define RPI_GPIO_29 RPI_V2_GPIO_P5_04 // Version 2, Pin P5-04
#define RPI_GPIO_30 RPI_V2_GPIO_P5_05 // Version 2, Pin P5-05
#define RPI_GPIO_31	RPI_V2_GPIO_P5_06 // Version 2, Pin P5-06

#define RPI_GPIO_PWM RPI_V2_GPIO_P1_12 // Version 2, Pin P1-12

#define RPI_VALID_GPIO (RPI_GPIO_04 | RPI_GPIO_17 | RPI_GPIO_18 | RPI_GPIO_22 | \
						RPI_GPIO_23 | RPI_GPIO_24 | RPI_GPIO_25 | RPI_GPIO_27 | \
						RPI_GPIO_28 | RPI_GPIO_29 | RPI_GPIO_30 | RPI_GPIO_31)

#define RPI_GPIO_OUT 0x00

#define RPI_GPIO_OUT_HIGH  0x01 // Set Output to High
#define RPI_GPIO_OUT_LOW   0x00 // Set Output to Low

#define RPI_GPIO_IN  0x80
// IN mode
#define RPI_GPIO_PUD_UP   0x01 // Enable Pull Up control
#define RPI_GPIO_PUD_DOWN 0x02 // Enable Pull Down control
#define RPI_GPIO_PUD_OFF  0x03  // Off ? disable pull-up/down

/* Macros */
#define GPIO_BASE   0x20200000
#define GPIO_LEN    0xB4
#define GPSET0      7
#define GPSET1      8
#define GPCLR0      10
#define GPCLR1      11
#define GPLEV0      13
#define GPLEV1      14

// page 196 http://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
#define	ARM_TIMER_CONTROL    0x0408
#define ARM_TC_32BIT         0x0002 // enable 32bit counters
#define ARM_TC_IRQ_ENABLE    0x0020 // enable timer irq
#define ARM_TC_TIMER_ENABLE  0x0080 // enable timer
#define ARM_TC_TIMER_HALT    0x0010 // halt timer in debug mode
#define ARM_TC_FRC_ENABLE    0x0200 // enable free running counter

#define	ARM_TIMER_FRC        0x0420

#define BCM2835_TICK_BASE (BCM2835_PERI_BASE +  0x0000B000)




void        RPIGPIO_Init();
int         RPIGPIO_SetMode(unsigned gpio, unsigned mode);
int         RPIGPIO_GetMode(unsigned gpio);
uint32_t    RPIGPIO_Read_Bits_0_31(void);
uint32_t    RPIGPIO_Read_Bits_32_53(void);
int         RPIGPIO_Read(unsigned gpio);
int         RPIGPIO_Write(unsigned gpio, unsigned level);




void rpi_delay_ms(uint32_t ms);
void rpi_delay_mcs(uint64_t us);
void rpi_set_pin_pud(uint8_t pin, uint8_t pud);
int rpi_set_pin_mode(uint8_t pin, uint8_t mode);
void rpi_pin_set(uint8_t pin);
void rpi_pin_clr(uint8_t pin);
void rpi_pin_write(uint8_t pin, uint8_t on_off);
void rpi_pin_set_mask(uint32_t mask);
void rpi_pin_clr_mask(uint32_t mask);
void rpi_pin_write_mask(uint32_t mask, uint8_t on_off);
int rpi_pin_get(uint8_t pin);

int rpi_init(void);
int rpi_close(void);

// some extra functions
uint32_t rpi_rdtsc(void);  // ARM based 250MHz free running counter
uint32_t rpi_sysmks(void); // lower 32 bits of system 1MHz free running counter
uint64_t rpi_systsc(void); // system 1MHz free running counter

#ifdef __cplusplus
}
#endif

#endif
