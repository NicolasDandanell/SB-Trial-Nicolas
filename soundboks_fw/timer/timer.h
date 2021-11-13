#ifndef TIMER_H
#define TIMER_H

#include <stddef.h>

// Based on data from datasheet:
// https://ww1.microchip.com/downloads/en/DeviceDoc/PIC32MX330350370430450470_Datasheet_DS60001185H.pdf

// IRQ NUMBERS & REFERENCE VECTORS
#define TIMER_2_IRQ_REG			9 	// Table 7-1 in datasheet
#define TIMER_2_VECTOR_REG		8	// Table 7-1 in datasheet
#define TIMER_3_IRQ_REG			14 	// Table 7-1 in datasheet
#define TIMER3_VECTOR_REG		12	// Table 7-1 in datasheet

// INTERRUPT PRIORITY REGISTERS - IPC2 & IPC3
#define TIMER_2_PRIORITY_REG	(*((volatile uint32_t*) 0xBF8810B0))	// Table 7-2 in datasheet
#define TIMER_3_PRIORITY_REG	(*((volatile uint32_t*) 0xBF8810C0))	// Table 7-2 in datasheet

// INTERRUPT FLAG REGISTERS - IFS0
#define TIMER_2_INT_FLAG_REG 	(*((volatile uint32_t*) 0xBF881030))	// Table 7-2 in datasheet
#define TIMER_3_INT_FLAG_REG	(*((volatile uint32_t*) 0xBF881030))	// Table 7-2 in datasheet

// INTERRUPT ENABLE REGISTERS - IEC0
#define TIMER_2_INT_EN_REG		(*((volatile uint32_t*) 0xBF881060))	// Table 7-2 in datasheet
#define TIMER_3_INT_EN_REG		(*((volatile uint32_t*) 0xBF881060))	// Table 7-2 in datasheet

// CONFIGURATION AND TIMER REGISTERS - T2CON, TMR2, PR2 & T3CON, TMR3, PR3
#define TIMER_2_CONF_REG		(*((volatile uint32_t*) 0xBF800800))	// Table 14-1 in datasheet
#define TIMER_2_COUNTER_REG		(*((volatile uint32_t*) 0xBF800810))	// Table 14-1 in datasheet
#define TIMER_2_PERIOD_REG		(*((volatile uint32_t*) 0xBF800820))	// Table 14-1 in datasheet
#define TIMER_3_CONF_REG		(*((volatile uint32_t*) 0xBF800A00))	// Table 14-1 in datasheet
#define TIMER_3_COUNTER_REG		(*((volatile uint32_t*) 0xBF800A10))	// Table 14-1 in datasheet
#define TIMER_3_PERIOD_REG		(*((volatile uint32_t*) 0xBF800A20))	// Table 14-1 in datasheet

// Utility & convenience
#define CLEAR_REGISTER_OFFSET 	0x4 
#define SET_REGISTER_OFFSET 	0x8 
#define INVERT_REGISTER_OFFSET 	0xC

#define USE_CLR_REG				true
#define USE_SET_REG				true
#define USE_INV_REG				true

#define NO_CRL_REG				false
#define NO_SET_REG				false
#define NO_INV_REG				false

#define TIMER_MAX_MS 			4581298

#define ANSI_RED     "\x1b[31m"	// For debug messages
#define ANSI_RESET   "\x1b[0m"	// For debug messages

#define ERROR(x, ...) printf(ANSI_RED x ANSI_RESET "\n", ##__VA_ARGS__)
#define PRINT(x, ...) printf(x "\n", ##__VA_ARGS__)

typedef int (*timer_callback_t)(void *ctx);

// Usable Functions
int initializeTimer();
int startTimer(size_t ms);
int cancelTimer(); // Could be useful, but little time to implement...

int callback_register(timer_callback_t cb, size_t time, void *ctx);

#endif // TIMER_H