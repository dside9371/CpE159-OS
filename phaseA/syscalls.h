// syscalls.h

#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

int GetPid(void);         // no input, 1 return
void Write(int, char *);
void Sleep(int);
void Mutex(int, int);
char GetChar(int);
void PutChar(int, char);
void PutStr(int, char *);
void GetStr(int, char *, int);   //???
int Fork(void);       //phase 7
void Signal(int, func_p_t );
int WaitPid(int *);
void Exit(int);
void Exec(func_p_t); // phase A
#endif
