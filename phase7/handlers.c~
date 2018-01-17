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
   MyBzero((char *)proc_stack[pid], sizeof(proc_stack[pid]));
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
        EnQ(run_pid, &terminal_wait_queue[i]);
        pcb[run_pid].state = WAIT;
        run_pid = -1;
       }

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
        ch = inportb(port + DATA);

       	if(terminal_wait_queue[i].size == 0)
		EnQ((int)ch,&terminal_buffer[i]);

	else if(terminal_wait_queue[i].size != 0) {
       		wait_pid = DeQ(&terminal_wait_queue[i]);
       		pcb[wait_pid].state = RUN;
       		EnQ(wait_pid,&run_q);
       		pcb[wait_pid].proc_frame_p->ECX = ch;
     	}
     }
     else
	if(terminal_wait_queue[i].size != 0) {
     	wait_pid = DeQ(&terminal_wait_queue[i]);
     	pcb[wait_pid].state = RUN;
     	EnQ(wait_pid,&run_q);
     	pcb[wait_pid].proc_frame_p->ECX = ch;
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

  EnQ(run_pid, &terminal_wait_queue[i]);
  pcb[run_pid].state = WAIT;
  run_pid = -1;


}

void ForkHandler(proc_frame_t *parent_frame_p) { // Kernel() provides this ptr
      int child_pid, delta, *bp;
      proc_frame_t *child_frame_p;

      if(0 == ready_q.size) { // may occur as too many processes been created
         cons_printf("Kernel Panic: cannot create more process!\n");
         child_pid = -1;
         return;                   // alternative: breakpoint() into GDB
      }

      child_pid = DeQ(&ready_q);				//1. get child_pid from ready queue
      EnQ(child_pid, &run_q); 					//2. add it to the run queue
      MyBzero((char *)&pcb[child_pid], sizeof(pcb_t));		//3. zap the PCB of the child process
      pcb[child_pid].state = RUN;				//4. set its state to RUN
      pcb[child_pid].ppid = run_pid; 				//5. set its 'ppid' to the current running PID
      
      //6. instead of zapping the runtime stack of the new process, use the
      //   new tool MyMemcpy() to copy runtime stack from the parent process
      MyMemcpy((char *)proc_stack[child_pid],(char *)proc_stack[run_pid],sizeof(proc_stack[run_pid]));


      /*
      but of course a new process has its own process frame (instead of
      loading the parent's process frame when run) so the proc_frame_p in
      its PCB should be set to where the copied version is

      there're also differences in ESP, EBP, ESI, EDI, and EBX that they
      need to be within the new process stack instead of the old stack
      the same difference is the 'delta' that is applied to all these
      'mirrored' locations (registers)

      the delta is the difference from where the parent stack is to where
      the new child stack is (where copied values are):
      */

    delta = proc_stack[child_pid] - proc_stack[run_pid];	//a. delta = child stack <--- byte distance ---> parent stack

    child_frame_p = parent_frame_p + delta;			//b. set child frame location = parent frame location + delta

    child_frame_p->ESP = parent_frame_p->ESP + delta;		//c. the same goes to the ESP, EBP, ESI, and EDI; in the new child
    child_frame_p->EBP = parent_frame_p->EBP + delta; 		//-- process frame each of these is added with delta
    child_frame_p->ESI = parent_frame_p->ESI + delta;
    child_frame_p->EDI = parent_frame_p->EDI + delta;

    parent_frame_p->EBX = child_pid;      			//d. set the EBX in the parent's process frame to the child_pid, but
    child_frame_p->EBX = 0;                        		//  it's given 0 to the EBX in the child's process frame
     
      bp = child_frame_p->EBP;
      while(*bp){
        *bp += delta;
         bp = *bp;
      }

} // end of ForkHandler()
