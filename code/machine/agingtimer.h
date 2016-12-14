

#ifndef AGINGTIMER_H
#define AGINGTIMER_H

#include "copyright.h"
#include "utility.h"
#include "callback.h"

// The following class defines a hardware timer.
class AgingTimer : public CallBackObj {
  public:
    AgingTimer( Thread * owner);
				// Initialize the timer, and callback to "toCall"
				// every time slice.
    virtual ~AgingTimer() {}

    void Disable() { disable = TRUE; }
    				// Turn timer device off, so it doesn't
				// generate any more interrupts.
    void Enable() { disable = FALSE; }

  private:

    Thread * owner;

    bool disable;		// Does this thread need aging now ?

    void CallBack();		// called internally when the hardware
				// timer generates an interrupt

    void SetInterrupt();  	// cause an interrupt to occur in the
    				// the future after a fixed or random
				// delay
};

#endif
