


#define		LOOP 1666000  // loop to time 0.6 ns
#include 	"spede.h"
#include	"events.h"		//needs address of TimerEvent

typedef	void (* func_ptr_t)();
struct 	i386_gate *IDT_p;
int i;

void RunningProcess(void) {
  i = -1;
  while(++i != LOOP) 
    if (cons_kbhit()) break;
    else              asm("inb $0x80");
  cons_putchar('z');
}	 

int main() {
	IDT_p = get_idt_base();
	cons_printf("IDT at DRM address %x (%d).\n" , IDT_p, IDT_p);  //show IDT address

	fill_gate(&IDT_p[TIMER_EVENT], (int)TimerEvent, get_cs(), ACC_INTR_GATE, 0);
	outportb(0x21, ~0x01);	//0x21 = PIC mask and ~1 = mask
	asm("sti");             //set and or enable interrupt in CPU EFLAGS register (default = 0)
  while(1) 
    if(cons_kbhit())  break;
    else               RunningProcess();
  return 0;				//exit main
}	

