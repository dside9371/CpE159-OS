// main.c, 159
// OS bootstrap and kernel code for OS phase 1
//


#include "spede.h"      // given SPEDE stuff
#include "types.h"      // data types
#include "events.h"     // events for kernel to serve
#include "tools.h"      // small functions for handlers
#include "proc.h"       // process names such as SystemProc()
#include "handlers.h"   // handler code

// kernel data are all declared here:
int run_pid;            // currently running PID; if -1, none selected
q_t ready_q, run_q;     // processes ready to be created and runables
pcb_t pcb[PROC_NUM];    // Process Control Blocks
char proc_stack[PROC_NUM][PROC_STACK_SIZE]; // process runtime stacks

void ProcScheduler(void) {              // choose run_pid to load/run
   if(run_pid > 0) return;

   if(run_q.size == 0) run_pid = 0;
   else run_pid = DeQ(&run_q);

   pcb[run_pid].life_time += pcb[run_pid].run_time;
   pcb[run_pid].run_time = 0;
}

int main(void) {  // OS bootstraps
   int i;
   struct i386_gate *IDT_p; // DRAM location where IDT is

   run_pid = -1;
   MyBzero((char *)&run_q, sizeof(q_t));
   MyBzero((char *)&ready_q, sizeof(q_t));

   for(i=0; i<Q_SIZE; i++) EnQ(i, &ready_q);

   IDT_p = get_idt_base();
   cons_printf("IDT located at DRAM addr %x (%d).\n", (int)IDT_p);

   fill_gate(&IDT_p[TIMER_EVENT], (int)TimerEvent, get_cs(), ACC_INTR_GATE, 0);
   outportb(0x21, ~0x1);

   NewProcHandler(SystemProc);
   ProcScheduler();
   ProcLoader(pcb[run_pid].proc_frame_p);

   return 0; // compiler needs for syntax altho this statement is never exec
}

void Kernel(proc_frame_t *proc_frame_p) {   // kernel code runs (100 times/second)
   char key;

   pcb[run_pid].proc_frame_p = proc_frame_p;

   TimerHandler();

   if (cons_kbhit()) {
      key = cons_getchar();
      if (key == 'n' ) NewProcHandler(UserProc);
      else if (key == 'b') breakpoint();
   }

   ProcScheduler();
   ProcLoader(pcb[run_pid].proc_frame_p);
}

