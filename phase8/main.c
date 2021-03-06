// main.c, 159
// OS bootstrap and kernel code for OS phase 8
//

#include "spede.h"      // given SPEDE stuff
#include "types.h"      // data types
#include "events.h"     // events for kernel to serve
#include "tools.h"      // small functions for handlers
#include "proc.h"       // process names such as SystemProc()
#include "handlers.h"   // handler code

// kernel data are all declared here:
int run_pid;            // currently running PID; if -1, none selected
q_t ready_q, run_q, terminal_buffer[2], terminal_wait_queue[2], term_kb_wait_q[2], term_screen_wait_q[2];    // processes ready to be created and runables
pcb_t pcb[PROC_NUM];    // Process Control Blocks
char proc_stack[PROC_NUM][PROC_STACK_SIZE]; // process runtime stacks
mutex_t mutex;
int pies;
unsigned int timer_ticks; //<---------------

void ProcScheduler(void) {  // choose run_pid to load/run
   if(run_pid > 0) return;

   if(run_q.size == 0) run_pid = 0;
   else run_pid = DeQ(&run_q);

   pcb[run_pid].life_time += pcb[run_pid].run_time;
   pcb[run_pid].run_time = 0;
}

void InitTerms(int port){
  int i;
  char my_str[] = "\e[2J";
  outportb(port + CFCR, CFCR_DLAB);
// set baud, Control Format Control Register 7-E-1 (data- parity-stop bits)
  // raise DTR, RTS of the serial port to start read/write
  outportb(port + BAUDLO, LOBYTE(115200/9600));      // period of each of 9600 bauds
  outportb(port + BAUDHI, HIBYTE(115200/9600));
                // CFCR_DLAB is 0x80
  outportb(port + CFCR, CFCR_PEVEN | CFCR_PENAB | CFCR_7BITS);
  outportb(port + IER, 0);
  outportb(port + MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
  outportb(port + IER, IER_ERXRDY | IER_ETXRDY);
  for(i=0;i<LOOP;i++) asm("inb $0x80");

  outportb(port + DATA,my_str[0]);
  for(i = 0; i < 5000; i++) asm("inb $0x80");
  outportb(port + DATA,my_str[1]);
  for(i = 0; i < 5000; i++) asm("inb $0x80");
  outportb(port + DATA,my_str[2]);
  for(i = 0; i < 5000; i++) asm("inb $0x80");
  outportb(port + DATA,my_str[3]);
  for(i = 0; i < 5000; i++) asm("inb $0x80");
  outportb(port + DATA,my_str[4]);
  for(i = 0; i < 5000; i++) asm("inb $0x80");
}

int main(void) {  // OS bootstraps
   int i;
   struct i386_gate *IDT_p; // DRAM location where IDT is
   pies = 0;
   timer_ticks = 0; //<--------------------
   run_pid = -1;

   MyBzero((char *)&run_q, sizeof(q_t));
   MyBzero((char *)&ready_q, sizeof(q_t));
   MyBzero((char *)&mutex,sizeof(mutex_t));

   MyBzero((char *)&terminal_buffer,sizeof(q_t [2]));
   MyBzero((char *)&terminal_wait_queue,sizeof(q_t [2]));
   MyBzero((char *)&term_kb_wait_q,sizeof(q_t [2]));             //phase 8
   MyBzero((char *)&term_screen_wait_q,sizeof(q_t [2]));
   mutex.lock = UNLOCK;

   InitTerms(TERM1);
   InitTerms(TERM2);

   for(i=0; i<Q_SIZE; i++) EnQ(i, &ready_q);

   IDT_p = get_idt_base();
   cons_printf("IDT located at DRAM addr %x (%d).\n", (int)IDT_p);

   fill_gate(&IDT_p[TIMER_EVENT],   (int)TimerEvent,   get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[SYSCALL_EVENT], (int)SyscallEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[TERM1_EVENT],   (int)Term1Event, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[TERM2_EVENT],   (int)Term2Event, get_cs(), ACC_INTR_GATE, 0);
   outportb(0x21, ~0x19); // IRQ 0,3,4 1101 = ~0x19


   NewProcHandler(SystemProc);
   ProcScheduler();
   ProcLoader(pcb[run_pid].proc_frame_p);

   return 0; // compiler needs for syntax altho this statement is never exec
}

void Kernel(proc_frame_t *proc_frame_p) {   // kernel code runs (100 times/second)
   char key;

   pcb[run_pid].proc_frame_p = proc_frame_p;


   switch(proc_frame_p -> event_type) {   //might just do ifs...
     case TIMER_EVENT :
        TimerHandler();
        break;
     case SYSCALL_EVENT :
        if(proc_frame_p -> EAX == WRITE)	WriteHandler(proc_frame_p);
        else if(proc_frame_p -> EAX == SLEEP)	SleepHandler();
        else if(proc_frame_p -> EAX == GETPID)	GetPidHandler();
        else if(proc_frame_p -> EAX == MUTEX) {
	  if(proc_frame_p -> EBX == LOCK)	MutexLockHandler();
	  if(proc_frame_p -> EBX == UNLOCK)	MutexUnlockHandler();
        }
        else if(proc_frame_p -> EAX == GETCHAR)     GetCharHandler(proc_frame_p);
        else if(proc_frame_p -> EAX == PUTCHAR)     PutCharHandler(proc_frame_p);
        else if(proc_frame_p -> EAX == FORK)      ForkHandler(proc_frame_p);   //PHASE7
	else if(proc_frame_p -> EAX == SIGNAL)      SignalHandler(proc_frame_p);   //PHASE8
	else cons_printf("Kernel Panic !!! EAX Value is: %x (%d).\n",(int)proc_frame_p -> EAX);
        break;
     case TERM1_EVENT:
        TermHandler(TERM1);
        outportb(0x20, 0x63);
        break;
     case TERM2_EVENT:
        TermHandler(TERM2);
        outportb(0x20, 0x64);
        break;
   }

   if (cons_kbhit()) {
      key = cons_getchar();
      if (key == 'n')	NewProcHandler(ShellProc);
      if (key == 'b')	breakpoint();
      //if (key == 'c') NewProcHandler(CookerProc); phase 7
      //if (key == 'e') NewProcHandler(EaterProc);
   }

   ProcScheduler();
   ProcLoader(pcb[run_pid].proc_frame_p);
}
