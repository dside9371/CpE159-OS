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
      int term, my_pid;
      char my_str[] = " ",
           my_msg[] = ": NULL Shell> ",
	         ls_str[] = "./\n\r../\n\r~rwx cowboy 7217 Nov 26 1:53 a.out*\n\r",
           get_str[100];

      Signal(SIGINT, Ouch);

      while(1) {
         my_pid = GetPid();
         my_str[0] = my_pid + '0';               // id str
         term = (my_pid%2 == 1)? TERM1 : TERM2;  // which term to use

         PutStr(term, my_str);
         PutStr(term, my_msg);
         GetStr(term, get_str, 100); // syscall will add null

	 	if(MyStrcmp(get_str, "ls")) {
			PutStr(term, ls_str);
		}
		else if(MyStrcmp(get_str, "clear")) {
			PutStr(term, "\e[2J");
		}
		else if(MyStrcmp(get_str, "exit")) {
			Exit(my_pid * 100);
		}
		else if(MyStrcmp(get_str, "a.out")) {
			switch(Fork()){
			case -1:
				PutStr(term, "ShellProc: cannot fork!\n\r");
				break;
			case 0:
				Exec(Aout);
				break;
			default:
				CallWaitPidNow();
			}
		}
		else if(MyStrcmp(get_str, "a.out&") || MyStrcmp(get_str, "a.out &")) {
			Signal(SIGCHLD, CallWaitPidNow);

			switch(Fork()){
			case -1:
				PutStr(term, "ShellProc: cannot fork!\n\r");
				Signal(SIGCHLD, (func_p_t)0);
				break;

			case 0:
				Exec(Aout);
				break;
			}
		}
   	}// end while 1
}//end of ShellProc


void Aout(void){
	int my_pid, sec;
	my_pid = GetPid();
	sec = my_pid % 5;
	if(sec == 0)sec = 5;
	Sleep(sec);
	Exit(my_pid * 100);
}



//phase 9 step 2
void CallWaitPidNow(void) {    // ShellProc's SIGCHLD handler
      int my_pid, term, child_pid, exit_num;
      char my_msg[100];

      child_pid = WaitPid(&exit_num);
      exit_num  = child_pid * 100;
      sprintf(my_msg, "Child %d exited, exit # %d.\n\r",
         child_pid, exit_num);

      my_pid = GetPid();
      term = (my_pid%2 == 1)? TERM1 : TERM2;  // which term to use
      PutStr(term, my_msg);
}
