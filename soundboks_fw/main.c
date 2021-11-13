#include "timer.h"
#include <stdio.h>

/*

    Sleep timer implementation:

    • Check every 5 min while active if any interaction has occured using a fictional getlastInteractionTime() function.

    • If no interaction, wait 5 further min, and check again, and repeat a third time if still no interactions.

    • If a total of 15 min have passed with no interaction, end the program.

    More technical implementation notes can be found in "timer.c".

*/

typedef struct example_context_t {
    uint32_t timerLastSetTo;                // In seconds
    uint32_t totalContextRunTime;           // In seconds
    main_state_t* mainState;
} example_context_t;

typedef enum {
    STATE_INIT,     // Initialization
    STATE_ACTIVE,   // Checking interaction status
    STATE_PASSIVE,  // Waiting for timer
    STATE_END
} main_state_t;

// Callback funtion for timer
int cb(void *ctx) {
    // De-referece pointer
    example_context_t *context = (example_context_t*)ctx;
    // Update the context run time
    context->totalContextRunTime += context->timerLastSetTo;
    // Set the main() state to ACTIVE
    *(context->mainState) = STATE_ACTIVE;
    return 0;
}

// Fictional function that gets the Real Time Clock time (in seconds)
extern uint32_t getRTCTime();

// Fictional function that returns the RTC time that the last interaction was registered (in seconds)
extern uint32_t getlastInteractionTime(); 

void main(void) {
    static main_state_t state = STATE_INIT;
    static bool interactionsInTheLast5Min = true;
    static bool runProgram = true;

    static example_context_t context = { 0, &state, &runProgram };    

    while(runProgram)
    {
        switch (state)
        {
            case STATE_INIT:
                initializeTimer();
                state = STATE_ACTIVE;
                lastInteractionTime 
                break;
            
            case STATE_ACTIVE:
                
                interactionsInTheLast5Min = (getRTCTime() - getlastInteractionTime() < 300);

                // Start the timer
                if (interactionsInTheLast5Min) {
                    // Interactions were registered, so wait 5 min to check again
                    context.totalContextRunTime = 0;
                    context.timerLastSetTo = 300;
                } else {
                    if (context.totalContextRunTime >= 900) {
                        // 15 min have passed and still no interactions. End program.
                        runProgram = false;
                        break;
                    }

                    // No interactions were registered, so wait 5 min more, until 15 have been reached to enter sleep mode
                    context.timerLastSetTo = 300;
                }

                callback_register(&cb, 300000, (void*)&context);

                state = STATE_PASSIVE;
                break;
            
            case STATE_PASSIVE:
                // Wait until the timer callback changes the state
                while (state == STATE_PASSIVE) {
                    sleep(1);
                }
                break;
            
            default:
                /* Default handler should not be hit in this case */
                ERROR("Main swich defaulted");
                break;
        }
    }

    return;
}