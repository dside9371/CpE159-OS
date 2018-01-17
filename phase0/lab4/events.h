/**********************************************************************
 *events.h, Phase 0, Exercise 4 -- Timer Event
 *********************************************************************/ 

#ifndef __EVENTS_H__
#define __EVENTS_H__

define TIMER_EVENT 32 

#ifndef ASSEMBLER  // skip if ASSEMBLER defined (in assembly code)
void TimerEvent(); // defined in events.S
#endif

#endif
