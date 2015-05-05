#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <bcm2835.h>
#include "RPI_gpio.h"



/* Private variables */
static volatile uint32_t * gpioReg = (volatile uint32_t *)MAP_FAILED;
static volatile uint32_t *bcm2835_tick;
static uint32_t arm_timer_ctl;

/* Public functions */
void RPIGPIO_Init()
{
    int fd;
    void *pmem = MAP_FAILED;

    if (!bcm2835_init())
    	printf("Cannot initialize bcm2835\n\r");

    fd = open("/dev/mem", O_RDWR | O_SYNC);

    if(fd < 0)
    {
        printf("Need to run as root\n");
        exit(1);
    }

    gpioReg = (volatile uint32_t *)mmap(0,
        GPIO_LEN,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_SHARED | MAP_LOCKED, fd,
        (__off_t)GPIO_BASE);


    // GPIO TIMER:
    pmem = mmap(NULL, BCM2835_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, BCM2835_TICK_BASE);

    if (pmem == MAP_FAILED)
    	printf("mmap failed\n\r");

    bcm2835_tick = (volatile uint32_t *)pmem;
    arm_timer_ctl = *(bcm2835_tick + ARM_TIMER_CONTROL/4);
    *(bcm2835_tick + ARM_TIMER_CONTROL/4) = ARM_TC_FRC_ENABLE | ARM_TC_TIMER_HALT;


    close(fd);

    //printf("gpios bank1=%08X bank2=%08X\n",
    //   RPIGPIO_Read_Bits_0_31(), RPIGPIO_Read_Bits_32_53());
}

/* Public functions */
void RPIGPIO_Deinit()
{
	if (bcm2835_tick == NULL)
		printf("Deinit error\n\r");

	// restore original control register
	*(bcm2835_tick + ARM_TIMER_CONTROL/4) = arm_timer_ctl;
	munmap((void *)bcm2835_tick, BCM2835_BLOCK_SIZE);
}

int RPIGPIO_SetMode(unsigned gpio, unsigned mode)
{
    int reg, shift;

    reg = gpio / 10;
    shift = (gpio % 10) * 3;

    gpioReg[reg] = (gpioReg[reg] & ~(7 << shift)) | (mode << shift);

    return 0;
}

int RPIGPIO_GetMode(unsigned gpio)
{
    int reg, shift;

    reg = gpio / 10;
    shift = (gpio % 10) * 3;

    return (*(gpioReg + reg) >> shift) & 7;
}

uint32_t RPIGPIO_Read_Bits_0_31(void)
{
    return (*(gpioReg + GPLEV0));
}

uint32_t RPIGPIO_Read_Bits_32_53(void)
{
    return (*(gpioReg + GPLEV1));
}

int RPIGPIO_Read(unsigned gpio)
{
    unsigned bank, bit;

    bank = gpio >> 5;

    bit = (1 << (gpio & 0x1F));

    if((*(gpioReg + GPLEV0 + bank) & bit) != 0)
        return 1;
    else
        return 0;
}

int RPIGPIO_Write(unsigned gpio, unsigned level)
{
    unsigned bank, bit;

    bank = gpio >> 5;

    bit = (1 << (gpio & 0x1F));

    if(level == 0)
        *(gpioReg + GPCLR0 + bank) = bit;
    else
        *(gpioReg + GPSET0 + bank) = bit;

    return 0;
}

// returns 32 bits of ARM free running counter (250MHz)
uint32_t RPIGPIO_Rdtsc(void)
{
	uint32_t stamp;

	stamp = *(bcm2835_tick + ARM_TIMER_FRC/4);

	return stamp;
}

// returns lower 32 bits of system microsecond free running counter
uint32_t RPIGPIO_Sysmks(void)
{
	uint32_t stamp;
	stamp = *(bcm2835_st + BCM2835_ST_CLO/4);
	return stamp;
}

// returns full 64 bits of system microsecond free running counter
uint64_t RPIGPIO_Systsc(void)
{
	uint64_t stamp;
	stamp = *(bcm2835_st + BCM2835_ST_CHI/4);
	stamp <<= 32;
	stamp += *(bcm2835_st + BCM2835_ST_CLO/4);
	return stamp;
}








void rpi_delay_ms(uint32_t ms)
{
	bcm2835_delay(ms);
}

void rpi_delay_mcs(uint64_t us)
{
	bcm2835_delayMicroseconds(us);
}

void rpi_set_pin_pud(uint8_t pin, uint8_t pud)
{
	pud &= RPI_GPIO_PUD_OFF;
	if (pud == RPI_GPIO_PUD_UP)
		bcm2835_gpio_set_pud(pin, BCM2835_GPIO_PUD_UP);
	else if (pud == RPI_GPIO_PUD_DOWN)
		bcm2835_gpio_set_pud(pin, BCM2835_GPIO_PUD_DOWN);
	else
		bcm2835_gpio_set_pud(pin, BCM2835_GPIO_PUD_OFF);
}

int rpi_set_pin_mode(uint8_t pin, uint8_t mode)
{
	if (mode & RPI_GPIO_IN) {
		bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
		rpi_set_pin_pud(pin, mode);
		return 0;
	}

	bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
	(mode & RPI_GPIO_OUT_HIGH) ? bcm2835_gpio_set(pin) : bcm2835_gpio_clr(pin);
	return 0;
}


void rpi_pin_set(uint8_t pin)
{
	bcm2835_gpio_set(pin);
}

void rpi_pin_clr(uint8_t pin)
{
	bcm2835_gpio_clr(pin);
}

void rpi_pin_write(uint8_t pin, uint8_t on_off)
{
	bcm2835_gpio_write(pin, on_off);
}

void rpi_pin_set_mask(uint32_t mask)
{
	mask &= RPI_VALID_GPIO;
	bcm2835_gpio_set_multi(mask);
}

void rpi_pin_clr_mask(uint32_t mask)
{
	mask &= RPI_VALID_GPIO;
	bcm2835_gpio_clr_multi(mask);
}

void rpi_pin_write_mask(uint32_t mask, uint8_t on_off)
{
	mask &= RPI_VALID_GPIO;
	bcm2835_gpio_write_multi(mask, on_off);
}

int rpi_pin_get(uint8_t pin)
{
	return bcm2835_gpio_lev(pin);
}
