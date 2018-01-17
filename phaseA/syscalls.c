// syscalls.c
// API calls to kernel system services
#include "types.h"
// GetPid() call
int GetPid(void) {          // no input, has return
   int pid;

   asm("movl  $100, %%EAX;
        int   $128;
        movl  %%EAX, %0"                    // restore original EAX
       : "=g" (pid)                     // output syntax, for output argument
       :
       : "eax"                                // no input items
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
	   : "eax", "ebx", "ecx"
       );
}

void Sleep(int seconds2sleep){
   asm("movl  $101, %%EAX;
        movl  %0,   %%EBX;
        int   $128"
        :
        : "g" (seconds2sleep)
	   : "eax", "ebx"
       );
}
void Mutex(int mutex_id,int mutexstate) {
   asm("movl  $102, %%EAX;
        movl  %0,   %%EBX;
	      movl  %1,   %%ECX;
        int   $128"
        :
        : "g" (mutex_id), "g" (mutexstate)
	      : "eax", "ebx", "ecx"
       );
}

char GetChar(int fileno){
   int ch;
   asm("movl  $103, %%EAX;
        movl  %1,   %%EBX;
        int   $128;
        movl  %%ECX,%0"
        : "=g"(ch)
        : "g" (fileno)
	   : "eax", "ebx", "ecx"
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
	   : "eax", "ebx", "ecx"
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
	int mutex_id;
 	mutex_id = (fileno == TERM1)? SCREEN1 : SCREEN2;
	Mutex(mutex_id, LOCK);
	while(*p){   		      //loop until the string 'p' points to ends - do post if we are missing a letter
		PutChar(fileno, *p);  //PutChar() with 'fileno' and a char (has to do with 'p')
   	 p++;
  	}
  	Mutex(mutex_id, UNLOCK);
}

void GetStr(int fileno, char *p, int size) {
  	 char ch;
     int mutex_id;
	   mutex_id = (fileno == TERM1)? KB1 : KB2;
	   Mutex(mutex_id, LOCK);
     while(size > 1){
    		ch = GetChar(fileno);
		PutChar(fileno,ch);
		if(ch == '\r')	PutChar(fileno, '\n');
    		if(ch == '\n' || ch == '\r') 	break;
		    *p = ch;
		    p++;
		    size--;
     }
	Mutex(mutex_id, UNLOCK);
	 *p= '\0';
}

void Signal(int signal_num, func_p_t p){  
   asm("movl  $48, %%EAX;
        movl  %0, %%EBX;
        movl  %1, %%ECX;
        int   $128"
        :                               // no outputs, otherwise use "=g" (...)
        : "g" (signal_num), "g" ((int)p)   // inputs, %0 and %1
	   : "eax", "ebx", "ecx"
       );
}

void Exit(int exit_num) {
  asm("movl $1, %%EAX;
       movl %0, %%EBX;
       int $128"
       :
       : "g" (exit_num)
       : "eax", "ebx"
     );//end asm
}//end of Exit()

int WaitPid(int *exit_num_p)  {
  int child_pid;

  asm("movl $7, %%EAX;
       movl %1, %%EBX;
       int $128;
       movl %%ECX, %0"
       : "=g" (child_pid)
       : "g" ((int)exit_num_p)
       : "eax", "ebx", "ecx"
     );// end asm
  return child_pid;
}//end WaitPid

void Exec(func_p_t p) {
	asm("movl $11, %%EAX;
		movl %0,  %%EBX;
		int $128"
		:
		: "g" ((int)p)
		: "eax", "ebx"
	);
}
