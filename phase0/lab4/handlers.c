/**********************************************************
handlers.c, Phase_0, Exercise 4 -- TimerEvent
**********************************************************/

#include  "spede.h"
#define   my_name_l  (unsigned)strlen(my_name)

char  my_name[] = "Andrew Peklar";
int   char_c     = 0;            //array index
int   char_e     = 0;            //erase counter
int   tick_count = 0;            //count no. of timer events

//video memory pointer
unsigned short *char_p = (unsigned short*) 0xB8000 + 12 * 80 + 35;

void TimerHandler() {
  if(tick_count++ % 75 == 0) {      //loop count every 75 seconds
    *char_p = 0xf00 + my_name[char_c];  
     char_p++;                      //move to next video memory location
     char_c++;
    
    if(char_c == my_name_l + 1) {//check length of my_name 
      char_c      = 0;              //reset char_count and pointer
      char_p      = (unsigned short*) 0xB8000 + 12 * 80 + 35; 
      
      for(char_e = 0; char_e <= my_name_l + 1; char_e++) {
        *char_p = ' ' + 0xf00;      //replace character with blank
         char_p++;                  //increment the pointer
      }
      //reset pointer once more
      char_p = (unsigned short*) 0xB8000 + 12 * 80 + 35;
    }
  }
  //0x20 = PIC cntrl reg, 0x60 dismisses IRQ 0
  outportb(0x20, 0x60);            
}
