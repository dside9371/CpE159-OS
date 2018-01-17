// syscalls.c
// API calls to kernel system services

// GetPid() call
int GetPid(void) {          // no input, has return
   int pid;

   asm("pushl %%EAX;
        movl  $100, %%EAX;
        int   $128;
        movl  %%EAX, %0;
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
        movl  %0, %%EBX;
        movl  %1, %%ECX;
        int   $128;
        popl  %%ECX;
        popl  %%EBX;
        popl  %%EAX"
        :                               // no outputs, otherwise use "=g" (...)
        : "g" (fileno), "g" ((int)p)   // inputs, %0 and %1
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
        movl  $101, %%EAX;
        movl  %0,   %%EBX;
        int   $128;
        popl  %%EBX;
        popl  %%EAX"
        :
        : "g" (seconds2sleep)
       );
}
void Mutex(int mutexstate) {
   asm("pushl %%EAX;
        pushl %%EBX;
        movl  $102, %%EAX;
        movl  %0,   %%EBX;
        int   $128;
        popl  %%EBX;
        popl  %%EAX"
        :
        : "g" (mutexstate)
       );
}

char GetChar(int fileno){
   int ch;
   asm("pushl %%EAX;
        pushl %%EBX;
        pushl %%ECX;
        movl  $103, %%EAX;
        movl  %1,   %%EBX;
        int   $128;
        movl  %%ECX,   %0;

        popl  %%ECX;
        popl  %%EBX;
        popl  %%EAX"
        : "=g"(ch)
        : "g" (fileno)
       );
    return (char)ch;
}
// send in # of seconds to sleep (in EBX)
// 101 -> EAX, call "int 128",
void PutChar(int fileno, char ch) {       //<---------------
   asm("pushl %%EAX;
        pushl %%EBX;
        pushl %%ECX;
        movl  $104, %%EAX;
        movl  %0, %%EBX;
        movl  %1, %%ECX;
        int   $128;
        popl  %%ECX;
        popl  %%EBX;
        popl  %%EAX"
        :                               // no outputs, otherwise use "=g" (...)
        : "g" (fileno), "g" ((int)ch)   // inputs, %0 and %1
       );
// save registers that will be used here
// send in service #, fileno, and p via
// three suitable registers
// issue syscall
// recover those saved registers
}
int Fork(void){           //phase 7
  int pid;
  asm("movl $2, %%EAX;
       int $128;
       movl %%EBX, %0"
       : "=g" (pid)
       :
       : "eax", "ebx"
     );
  return pid;
}


void PutStr(int fileno, char *p) {
	while(*p){   		      //loop until the string 'p' points to ends - do post if we are missing a letter
		PutChar(fileno, *p);  //PutChar() with 'fileno' and a char (has to do with 'p')
    		p++;
  	}
}

void GetStr(int fileno, char *p, int size) {
  	char ch;
        while(size > 1){
    		ch = GetChar(fileno);
		PutChar(fileno,ch);
		if(ch == '\r') PutChar(fileno, '\n');
    		if(ch == '\n' || ch == '\r')  break;
		*p = ch;
		p++;
		size--;
	}
	*p= '\0';
}
