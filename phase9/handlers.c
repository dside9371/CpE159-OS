//handlers.c, 159

#include "spede.h"
#include "types.h"
#include "data.h"
#include "tools.h"
#include "proc.h"
#include "handlers.h"

// to create process, 1st alloc PID, PCB, and process stack space
// build process frame, initialize PCB, record PID to run_q (if not 0)
void NewProcHandler(func_p_t p) {  // arg: where process code starts
   int pid;

   if(0 == ready_q.size) { // may occur as too many processes been created
      cons_printf("Kernel Panic: cannot create more process!\n");
      return;                   // alternative: breakpoint() into GDB
   }

   pid = DeQ(&ready_q);
   MyBzero((char *)&pcb[pid], sizeof(pcb_t));
   MyBzero((char *)proc_stack[pid], PROC_STACK_SIZE);
   pcb[pid].state = RUN;
   if(pid != 0)   EnQ(pid, &run_q);       //<--------- may be issue

   pcb[pid].proc_frame_p = (proc_frame_t *)&proc_stack[pid][PROC_STACK_SIZE-sizeof(proc_frame_t)];

   pcb[pid].proc_frame_p->EFL = EF_DEFAULT_VALUE|EF_INTR;
   pcb[pid].proc_frame_p->EIP =(int) p;
   pcb[pid].proc_frame_p->CS = get_cs();
}


void WriteHandler(proc_frame_t* p){
  int i;
  int IOBuff    = p -> EBX;
  char* strOut  = (char *)p -> ECX;

  if(IOBuff == STDOUT)  cons_printf(strOut);
  else
    while(*strOut){
      outportb(IOBuff + DATA, *strOut);
      for(i = 0; i < 5000; i++) asm("inb $0x80");
      strOut++;
    }
}

// count run_time of running process and preempt it if reaching time slice
void TimerHandler(void) {
   int i;
   timer_ticks++;  //<--------------

   for(i = 0; i < PROC_NUM; i++) {
     if((pcb[i].state == SLEEPING) && (timer_ticks == pcb[i].wake_time)) {
       EnQ(i, &run_q);
       pcb[i].state = RUN;		//<---- per instruct, wrong in submission
       }
   }
   //dismiss irq0
   outportb(0x20, 0x60); // 0x61 0x62

   //if SystemProc then return here
   if(run_pid == 0)   return;

   //increment cpu time
   pcb[run_pid].run_time++;

   //check to see if it used up time slice
   if(pcb[run_pid].run_time >= TIME_SLICE) {
      EnQ(run_pid, &run_q);
      run_pid = -1;
   }
}

void GetPidHandler(void) {
    pcb[run_pid].proc_frame_p-> EAX = run_pid;
}

void SleepHandler(void) {
    pcb[run_pid].wake_time = timer_ticks + 100 * (pcb[run_pid].proc_frame_p-> EBX);
    pcb[run_pid].state = SLEEPING;
    run_pid = -1;
}

///////////////////////////////////////////////////////

void MutexLockHandler(void){
	if(mutex.lock == UNLOCK) mutex.lock = LOCK;
	else{
		EnQ(run_pid, &mutex.wait_q);
		pcb[run_pid].state = WAIT;
		run_pid=-1;
	}
}
void MutexUnlockHandler(void){
      int pid;
      if(mutex.wait_q.size == 0) mutex.lock = UNLOCK;
      else if(mutex.wait_q.size != 0){
          pid=DeQ(&mutex.wait_q);
          EnQ(pid,&run_q);
          pcb[pid].state = RUN;
      }
}
void GetCharHandler(proc_frame_t* p){
      int i;
      int fileno;
      fileno = p->EBX;
      if(fileno == TERM1) i = 0;
      else i = 1;

      if(terminal_buffer[i].size > 0)
	pcb[run_pid].proc_frame_p->ECX = DeQ(&terminal_buffer[i]);
      else {
        EnQ(run_pid, &term_kb_wait_q[i]);
        pcb[run_pid].state = WAIT;
        run_pid = -1;
       }

}
void PutCharHandler(proc_frame_t* p){
  int i;
  char ch;
  int fileno;

  fileno = p->EBX;
  ch = p->ECX;

  if(fileno == TERM1) i = 0;
  else i = 1;

  outportb(fileno + DATA, ch);

  EnQ(run_pid, &term_screen_wait_q[i]);
  pcb[run_pid].state = WAIT;
  run_pid = -1;


}

void TermHandler(int port){
     int i;
     char ch;
     int wait_pid;
     int indicator;
     if(port == TERM1) i = 0;
     else i = 1;

     indicator = inportb(port + IIR);

     if (indicator == IIR_RXRDY){
        ch = inportb(port + DATA) & 0x7F;
       	if(term_kb_wait_q[i].size == 0)
		EnQ((int)ch,&terminal_buffer[i]);

	else{
       		wait_pid = DeQ(&term_kb_wait_q[i]);
       		pcb[wait_pid].state = RUN;
       		EnQ(wait_pid,&run_q);
       		pcb[wait_pid].proc_frame_p->ECX = ch;

		if(ch == 3 && pcb[wait_pid].sigint_handler)
			InsertWrapper(wait_pid, pcb[wait_pid].sigint_handler);
     	}
     }
     else{
	if(term_screen_wait_q[i].size > 0) {
     		wait_pid = DeQ(&term_screen_wait_q[i]);
     		pcb[wait_pid].state = RUN;
     		EnQ(wait_pid,&run_q);
        }
     }
}


void ForkHandler(proc_frame_t *parent_frame_p) { // Kernel() provides this ptr
      int child_pid, delta, *bp;
      proc_frame_t *child_frame_p;



      if(1 > ready_q.size) { // may occur as too many processes been created
         cons_printf("Kernel Panic: cannot create more process!\n");
	    parent_frame_p->EBX = -1;
         return;                   // alternative: breakpoint() into GDB
      }

      child_pid = DeQ(&ready_q);				//1. get child_pid from ready queue
      EnQ(child_pid, &run_q); 					//2. add it to the run queue
      MyBzero((char *)&pcb[child_pid], sizeof(pcb_t));		//3. zap the PCB of the child process
      pcb[child_pid].state = RUN;				//4. set its state to RUN
      pcb[child_pid].ppid = run_pid; 				//5. set its 'ppid' to the current r PID
	    pcb[child_pid].sigint_handler = pcb[run_pid].sigint_handler;

      MyMemcpy((char *)&proc_stack[child_pid][0],(char *)&proc_stack[run_pid][0],PROC_STACK_SIZE);

      delta = (int)&proc_stack[child_pid] - (int)&proc_stack[run_pid];	//a. delta = child stack <--- byte distance ---> parent stack

      child_frame_p = pcb[child_pid].proc_frame_p = (proc_frame_t *)((int)parent_frame_p + delta);
      child_frame_p->ESP += delta;					//c. the same goes to the ESP, EBP, ESI, and
      child_frame_p->EBP += delta;
      child_frame_p->ESI += delta;
      child_frame_p->EDI += delta;

      parent_frame_p->EBX = child_pid;  //d. set the EBX in the parent's process frame to the
      child_frame_p->EBX = 0;          //  it's given 0 to the EBX in the child's process frame
      bp = (int *)child_frame_p->EBP;
      while(*bp){
        *bp += delta;
         bp = (int *)*bp;
      }
} // end of ForkHandler()


void SignalHandler(proc_frame_t *p){

	if(p->EBX == SIGINT)
		pcb[run_pid].sigint_handler = (func_p_t)p->ECX;
	else if(p->EBX == SIGCHLD)
		pcb[run_pid].sigchld_handler = (func_p_t)p->ECX;

}
void InsertWrapper(int pid, func_p_t handler){
	int *p;
	proc_frame_t temp_frame;

	temp_frame = *pcb[pid].proc_frame_p;
     p = (int *)&pcb[pid].proc_frame_p -> EFL;
	*p = (int)handler;
	p--;
	*p = (int)temp_frame.EIP;

	pcb[pid].proc_frame_p = (proc_frame_t *)((int)pcb[pid].proc_frame_p - sizeof(int [2]));

	MyMemcpy((char *)pcb[pid].proc_frame_p,(char *)&temp_frame, sizeof(proc_frame_t));

	pcb[pid].proc_frame_p -> EIP = (unsigned int)Wrapper;

}
void ExitHandler(proc_frame_t *p) {               // when child calls Exit()
	int ppid;

	ppid = pcb[run_pid].ppid;

	if(pcb[ppid].state != WAITCHLD){
		pcb[run_pid].state = ZOMBIE;
		run_pid = -1;
		if(pcb[ppid].sigchld_handler != NULL){
			InsertWrapper(ppid, pcb[ppid].sigchld_handler);
		}
	}else{
		pcb[ppid].state = RUN;
		EnQ(ppid, &run_q);
		pcb[ppid].proc_frame_p->EBX = pcb[run_pid].proc_frame_p->EBX;
		pcb[ppid].proc_frame_p->ECX = run_pid;
		EnQ(run_pid,&ready_q);
		pcb[run_pid].state = READY;
		run_pid = -1;
	}

}

void WaitPidHandler(proc_frame_t *p) {           // when parent calls WaitPid()
	int child_pid;

	for(child_pid = 0; child_pid < PROC_NUM; child_pid++){
		if(pcb[child_pid].state == ZOMBIE && pcb[child_pid].ppid == run_pid)	break;
	}
	if(child_pid == PROC_NUM){
		pcb[run_pid].state = WAITCHLD;
		run_pid = -1;
	}else{
    pcb[run_pid].proc_frame_p -> EBX = pcb[child_pid].proc_frame_p -> EBX;
		pcb[run_pid].proc_frame_p -> ECX = child_pid;
		EnQ(child_pid, &ready_q);
		pcb[child_pid].state = READY;
	}

}
