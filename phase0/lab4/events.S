/**********************************************************************
 *events.S, Phase 0, Exercise 4 -- Timer Event
 *********************************************************************/                                    

#include <spede/machine/asmacros.h> 
#include "events.h"

ENTRY(TimerEvent) 			// push eflag, cs, eip (by circuit)
  pusha						// push all: e-axdb-x, e-sb-p, e-sd-i
  cld						// clear direction flag
  call CNAME(TimerHandler)  // call C function TimerHandler()
  popa						// pop all: e-ds-i, e-bs-p, e-bdca-x
  iret						// pop eip, cs, eflags
