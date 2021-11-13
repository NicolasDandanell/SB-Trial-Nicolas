#include "timer.h"

/*  

    Assumptions and approach 
    -------------------------

    • Internal clock assumed to be used, and further assumed to be running at 120 MHz

    • Assuming clock cycles are a bottleneck due to audio processing and bluetooth streaming, thus will use the build in hardware timer as much as possible to limit CPU use. 

    Timer 2 & 3 can be combined to form a 32 bit timer with a max pre-scaler of 256.
    This gives us a timer with a limit of 1.099.511.627.776 cycles (Around 152 minutes on 120 MHz)
    This Timer can then be expanded to cover an infinite amount of time with the addition of a loop in the timer function, which has not been implemented due to time constraints.

*/ 

// Variable definitions
// ---------------------

// bool to ensure that the timer is properly initialized 
bool timerInitialized = false; // No scope, thus static by default

timer_callback_t timerCallback; // No scope, thus static by default
void* callbackContext; // No scope, thus static by default

// Functions
// ----------

// ISR for when hardware timer is done. Given priority 5 out of 7
void __attribute__(TIMER_3_VECTOR, 5) timerDoneISR() {
    
    // Clear interrupt flags
    clearBit(TIMER_2_INT_FLAG_REG, 9, NO_CRL_REG)   // Set interrupt flag to 0
    clearBit(TIMER_3_INT_FLAG_REG, 14, NO_CRL_REG)  // Set interrupt flag to 0

    // Perform the callback
    timerCallback(callbackContext);
}

// Utility clear bit function, with optional use of embedded clear registers in PIC32
int clearBit(uint32_t address, uint8_t bit, bool hasClrReg) {
    if (hasClrReg) {
        *(address + CLEAR_REGISTER_OFFSET) |= 1 << bit;
    } else {
        address &= ~(1 << bit);
    }

    return 0;
}

// Utility set bit function, with optional use of embedded set registers in PIC32
int setBit(uint32_t address, uint8_t bit, bool hasSetReg) {
    if (hasSetReg) {
        *(address + SET_REGISTER_OFFSET) |= 1 << bit;
    } else {
        address |= 1 << bit;
    }

    return 0;
}

// Initialization function for timer
int initializeTimer() {

    /* 
        Implementation note
        
        Due to issues tracking down the precise address of the register containing the general interrupt toggle, I have 
        chosen to implement the "di" and "ei" assembly commands instead, which should have the same effect. The corresponding 
        table in the datasheet can be found in table TABLE 3-2, in register 12.
    */
    
    // Disable interrupts while initializing timer
    asm volatile("di");     // Disables interrupt
    asm volatile("ehb");    // Execution Hazard Barrier instruction, to make sure previous instruction take effect before proceeding

    // Setup the timer
    // ----------------

    // Disable timers before setup
    clearBit(TIMER_2_CONF_REG, 15, USE_CLR_REG);
    clearBit(TIMER_3_CONF_REG, 15, USE_CLR_REG);

    // Enable 32 bit mode
    setBit(TIMER_2_CONF_REG, 3, USE_SET_REG);

    // Set clock to internal peripheral clock
    setBit(TIMER_2_CONF_REG, 1, USE_SET_REG);

    // Given that the use of the timer is assumed to be for applications with > 1 second of waiting, a high 1:256 scaler will be used
    setBit(TIMER_2_CONF_REG, 4, USE_SET_REG); 
    setBit(TIMER_2_CONF_REG, 5, USE_SET_REG);
    setBit(TIMER_2_CONF_REG, 6, USE_SET_REG);

    // Set Priority to 5 and clear interrrupt flags
    clearBit(TIMER_3_INT_EN_REG, 14, NO_CRL_REG)    // Disable interrupt, and enable only when starting timer
    setBit(TIMER_3_PRIORITY_REG, 10, NO_SET_REG);   // Setting priority to 5 (101)
    clearBit(TIMER_3_PRIORITY_REG, 11, NO_CRL_REG); // Setting priority to 5 (101)
    setBit(TIMER_3_PRIORITY_REG, 12, NO_SET_REG);   // Setting priority to 5 (101)
    clearBit(TIMER_2_INT_FLAG_REG, 9, NO_CRL_REG)   // Set interrupt flag to 0
    clearBit(TIMER_3_INT_FLAG_REG, 14, NO_CRL_REG)  // Set interrupt flag to 0

    // Re-enable interrupts
    asm volatile("ei");     // Enables interrupt
    asm volatile("ehb");    // Execution Hazard Barrier instruction, to make sure previous instruction take effect before proceeding
}

// Start function for timer
int startTimer(size_t ms) {

    /* 
        Implementation note
        
        Due to issues tracking down the precise address of the register containing the general interrupt toggle, I have 
        chosen to implement the "di" and "ei" assembly commands instead, which should have the same effect. The corresponding 
        table in the datasheet can be found in table TABLE 3-2, in register 12.
    */

    // Check if the amount of time is greater than the 32 bit timer can handle (4.581.298 ms, which is 2.147.483.647 post-prescaler clock cycles)
    if (if ms > TIMER_MAX_MS) {
        ERROR("startTimer() parameter ms was %i, which is larger than the allowed %i.", ms, TIMER_MAX_MS);
        return -1;
        // A more thorough implementation of this timer would then utilized a loop which worked in conjunction with the ISR to run the timer the required amount of times.
    }

    // Check if timer has been initialized. If not, initialize it before starting the timer.
    if (!timerInitialized) {
        PRINT("Timer was not initialized! Initializing timer before starting it.");
        initializeTimer();
    }

    // Configure the registers to the desired time
    // --------------------------------------------

    // Disable interrupts while initializing timer
    asm volatile("di");     // Disables interrupt
    asm volatile("ehb");    // Execution Hazard Barrier instruction, to make sure previous instruction take effect before proceeding

    // Set the period register using the equation: T = PS × (PR + 1)
    // T:  Time in clock cycles (120.000 cycles per millisecond at 120 MHz)
    // PS: Pre-scaler value (Set to 256)
    // PR: Period register (Value the counter is counting to, and will trigger the interrupt at. Since its 32 bit, the timer 3 one will be the one triggering it) 

    static uint32_t periodRegisterValue = ((1875 * ms) / 4) - 1;

    TIMER_2_PERIOD_REG = periodRegisterValue & 0x0000FFFF; // Simplified version of: ((120.000.000 / 1000) * ms) / 256 - 1 = Period Register Value
    if (ms > 139) { // Check if period register will be larger than 16 bit, and thus use part of the timer 3 PR
        TIMER_3_PERIOD_REG = ((((1875 * ms) / 4) - 1) >> 16) & 0x0000FFFF;
    }

    // Reset counter registers
    TIMER_2_COUNTER_REG = 0;
    TIMER_3_COUNTER_REG = 0;

    // Enable timers (Just T2 since T3 is now an extension of it)
    setBit(TIMER_2_CONF_REG, 15, USE_SET_REG);

    // Enable timer interrupts (Of timer 3, since timer 2 has the least significant bits and will then overflow to timer 2)
    setBit(TIMER_2_INT_EN_REG, 14, NO_SET_REG);

    /* This part with setting up what part needs to be setup in timer 2 & 3 is still very confusing to me, and given that I don't have a PIC32, i can't test whether it is implemented correctly */

    // Re-enable interrupts
    asm volatile("ei");     // Enables interrupt
    asm volatile("ehb");    // Execution Hazard Barrier instruction, to make sure previous instruction take effect before proceeding
}

// Assumed to be the function that is called externally to start the timer
int callback_register(timer_callback_t cb, size_t time, void *ctx) {
    //@TODO: Implement
    
    // Save the callback function and context in the static variables
    timerCallback = cb;
    callbackContext = ctx;

    if (startTimer(time)) {
        ERROR("Could not register callback. \"time\" parameter out of bounds");
        return -1;
    }

    return 0;
}