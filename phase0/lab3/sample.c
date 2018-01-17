#include <spede/stdio.h>
#include <spede/flames.h>
void DisplayMsg(long i);

int main(void) {
  
  long i = 111;
  int  k =  -1;
  while(++k != 5) {
    DisplayMsg(i);
    ++i;
  }
  return 0;
}

void DisplayMsg(long i) {
  printf("%d Hello world %d \nECS", i, 2*i);
  cons_printf("--> Hello world <--\nCPE/CSC");  //target printer
}

  

