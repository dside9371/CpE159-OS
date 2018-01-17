// syscalls.c
// API calls to kernel system services

// GetPid() call
int GetPid(void) {          // no input, has return
   int pid;

   asm("pushl %%EAX;                    // save register EAX to stack
        movl  $100, %%EAX;              // service #100
        int   $128;                     // interrupt CPU with IDT Event 128
        movl  %%EAX, %0;                // after, copy EAX to variable 'pid' (%0 means 1st item below)
        popl  %%EAX"                    // restore original EAX
       : "=g" (pid)                     // output syntax, for output argument
       :                                // no input items
      );
   return pid;
}

// Write() call
void Write(int fileno, char *p) {       //<---------------
   asm("pushl %%EAX;
        pushl %%EBX;
        pushl %%ECX;
        movl  $4, %%EAX;
        movl  %0, %%EBX;                // if issues persist, will probably be with
        movl  %1, %%ECX;                // what regs used and where in this.
        int   $128;                     // issue syscall
        popl  %%ECX;
        popl  %%EBX;
        popl  %%EAX;"
        :                               // no outputs, otherwise use "=g" (...)
        : "g" (fileno), "g" ((int)p);   // inputs, %0 and %1
       );
// save registers that will be used here
// send in service #, fileno, and p via
// three suitable registers
// issue syscall
// recover those saved registers
}

void Sleep(int seconds2sleep) {
   asm("pushl %%EAX;
        pushl %%EBX;
        movl  $101, %%EAX                //something may be fucky here too
        movl  %0  , %%EBX;
        int   $128;
        popl  %%EBX;
        popl  %%EAX;"
        :1
        : "g" (seconds2sleep)
       );
// send in # of seconds to sleep (in EBX)
// 101 -> EAX, call "int 128",
}
