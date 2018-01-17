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
  int IOBuff    = p -> EBX;
  char* strOut  = (char *)p -> ECX;

  if(IOBuff == STDOUT)  cons_printf(strOut);
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
          pcb[run_pid].state = RUN;
      }
}


