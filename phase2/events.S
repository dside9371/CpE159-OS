// events.S, initial stage handling an event, build proc frame mainly

#include <spede/machine/asmacros.h> // do not use/include "spede.h"

.comm k_stack, 8192, 1      // declare kernel stack space (8192 x 1 byte)
.text


// build proc frame, and go into kernel
ENTRY(TimerEvent)              // will first push EFL, CS, EIP (hardware/circuit)
   pusha $32
   jmp   TestRest

ENTRY(SyscallEvent)
   pusha $32
   jmp   TestRest

TestRest:
   movl  %ESP, %EBX
   cld
   movw  $0x10, %ax            // set kernel data segment registers
   mov   %ax, %ds              // set kernel data segment registers
   mov   %ax, %es              // set kernel data segment registers
   leal  k_stack + 8192, %esp  // point ESP to kernel stack
   pushl %EBX
   call  CNAME(Kernel)

//loads the trapframe of the selected process in order to run it
ENTRY(ProcLoader)
    movl 4(%esp) , %eax
    movl %eax    , %esp
    popa
    addl $4,     , %esp    //skip event_type 4 bytes
    iret
