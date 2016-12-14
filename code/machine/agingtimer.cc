
#include "copyright.h"
#include "timer.h"
#include "main.h"
#include "sysdep.h"
#include <iostream>
using namespace std;

AgingTimer::AgingTimer(Thread * owner)
{
    this->owner = owner;
    disable = FALSE;
}

void
AgingTimer::CallBack()
{
    /* Thread is still in queue , need aging */
    if(!disable)
    {
        /* Aging */
        int oldPriority = owner->getPriority();
        int newPriority = (oldPriority + 10 > 149) ? 149 : oldPriority + 10;
        owener->setPriority(newPriority);

        int nowTime = kernel->stats->totalTicks;
        cout << "Tick " << nowTime << ": Thread " << owner->getID();
        cout << " changes its priority from " << oldPriority << " to " << newPriority << endl;

        if(newPriority >= 100 && newPriority < 110) /* L2 -> L1 */
        {
            kernel->scheduler->L2Queue->Remove(owner);
            kernel->scheduler->L1Queue->Insert(owner);

            cout << "Tick " << nowTime << ": Thread " << owner->getID() << " is removed from queue L2" << endl;
            cout << "Tick " << nowTime << ": Thread " << owner->getID() << " is inserted into queue L1"<< endl;

            /* Preemptive , Only SJF */
            if( L1Queue->Front()->getBurstTime() < kernel->currentThread->getBurstTime() )
                kernel->currentThread->Yield();
        }
        else if(newPriority >= 50 && newPriority < 60) /* L3 -> L2 */
        {
            kernel->scheduler->readyList->Remove(owner);
            kernel->scheduler->L2Queue->Insert(owner);

            cout << "Tick " << nowTime << ": Thread " << owner->getID() << " is removed from queue L3" << endl;
            cout << "Tick " << nowTime << ": Thread " << owner->getID() << " is inserted into queue L2" << endl;
        }


        /* Next 1500 ticks */
        SetInterrupt();
    }
}

void
AgingTimer::SetInterrupt()
{
    if (!disable) {
        /* Check every 1500 ticks */
       int delay = 1500;

       // schedule the next timer device interrupt
       kernel->interrupt->Schedule(this, delay, TimerInt);
    }
}
