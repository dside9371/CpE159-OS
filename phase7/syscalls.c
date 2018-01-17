// syscalls.c
// API calls to kernel system services

// GetPid() call
int GetPid(void) {          // no input, has return
   int pid;

   asm("movl  $100, %%EAX;
        int   $128;
        movl  %%EAX, %0"                    // restore original EAX
       : "=g" (pid)                     // output syntax, for output argument
       :   
       : "eax"                             // no input items
      );
   return pid;
}

// Write() call
void Write(int fileno, char *p) {       //<---------------
   asm("movl  $4, %%EAX;
        movl  %0, %%EBX;
        movl  %1, %%ECX;
        int   $128"
        :                               // no outputs, otherwise use "=g" (...)
        : "g" (fileno), "g" ((int)p)   // inputs, %0 and %1
	: "eax", "ebx" ,"ecx"
       );
// save registers that will be used here
// send in service #, fileno, and p via
// three suitable registers
// issue syscall
// recover those saved registers
}

void Sleep(int seconds2sleep) {
   asm("movl  $101, %%EAX;
        movl  %0,   %%EBX;
        int   $128"
        :
        : "g" (seconds2sleep)
	: "eax", "ebx"
       );
}
void Mutex(int mutexstate) {
   asm("movl  $102, %%EAX;
        movl  %0,   %%EBX;
        int   $128"
        :
        : "g" (mutexstate)
	: "eax", "ebx"
       );
}

char GetChar(int fileno){
   int ch;
   asm("movl  $103, %%EAX;
        movl  %1,   %%EBX;
        int   $128;
        movl  %%ECX,   %0"
        : "=g"(ch)
        : "g" (fileno)
	: "eax", "ebx" ,"ecx"
       );
    return (char)ch;
}
// send in # of seconds to sleep (in EBX)
// 101 -> EAX, call "int 128",
void PutChar(int fileno, char ch) {       //<---------------
   asm("movl  $104, %%EAX;
        movl  %0, %%EBX;
        movl  %1, %%ECX;
        int   $128"
        :                               // no outputs, otherwise use "=g" (...)
        : "g" (fileno), "g" ((int)ch)   // inputs, %0 and %1
	: "eax", "ebx" ,"ecx"
       );

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
	*p=(char)0;
}
