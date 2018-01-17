// proc.c, 159
// all processes are coded here
// processes do not use kernel space (data.h) or code (handlers, tools, etc.)
// all must be done thru system service calls

#include "spede.h"      // cons_...() needs
#include "data.h"       // run_pid needed
#include "proc.h"       // prototypes of processes
#include "syscalls.h"   // API of system service calls
#include "tools.h"

void SystemProc(void) {
   while(1) asm("inb $0x80"); // do nothing for now
}
void Wrapper(func_p_t sig_handler) { // arg implanted in stack
      asm("pusha");                     // save regs
      sig_handler();                    // call user's handler
      asm("popa");                      // pop back regs
}
void Ouch(void) {                    // user's signal handler
      int term = (GetPid()%2)? TERM1 : TERM2;      // which terminal?
      PutStr(term, "Ouch, that stings, Cowboy! "); // just a msg
}
void ShellProc(void) {
      int term, my_pid, forked_pid;
      char my_str[] = " ",
           my_msg[] = ": NULL Shell> ",
           get_str[100];

      Signal(SIGINT, Ouch);

      while(1) {
         my_pid = GetPid();
         my_str[0] = my_pid + '0';               // id str
         term = (my_pid%2 == 1)? TERM1 : TERM2;  // which term to use

         PutStr(term, my_str);
         PutStr(term, my_msg);
         GetStr(term, get_str, 100); // syscall will add null
         //Phase 9 part 2 additions
         if(MyStrcmp(get_str, "fork") == 1)  {
           forked_pid = Fork();
           if(forked_pid == -1)   PutStr(term, "ShellProc: cannot fork!\n\r");
           if(forked_pid  >  0)   CallWaitPidNow();   //parent waits, causes block
         }//end if
         else if(MyStrcmp(get_str, "fork&") == 1 || MyStrcmp(get_str, "fork &") == 1) { //background runner
           Signal(SIGCHLD, CallWaitPidNow);   // register handler before fork
           forked_pid = Fork();                // in case child too fast and exits first

           if(forked_pid == -1)  {
             PutStr(term, "ShellProc: cannot fork!\n\r");
             Signal(SIGCHLD, (func_p_t)0);     // cancel handler
           }// end if
         }//end else if
         else if(MyStrcmp(get_str, "exit") == 1)  {   //only try on child process
           Exit(my_pid * 100);                //what if parent exits? child exits?
         }//end else if
      }// end while 1
}//end of ShellProc


//phase 9 step 2
void CallWaitPidNow(void) {    // ShellProc's SIGCHLD handler
      int my_pid, term, child_pid, exit_num;
      char my_msg[100];

      child_pid = WaitPid(&exit_num);
      sprintf(my_msg, "Child %d exited, exit # %d.\n\r",
         child_pid, exit_num);

      my_pid = GetPid();
      term = (my_pid%2 == 1)? TERM1 : TERM2;  // which term to use
      PutStr(term, my_msg);
}
