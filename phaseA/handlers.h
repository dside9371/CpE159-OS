// handlers.h, 159

#ifndef _HANDLERS_H_
#define _HANDLERS_H_

#include "types.h"   // need definition of 'func_p_t' below

void NewProcHandler(func_p_t);
void TimerHandler(void);

void GetPidHandler(void);
void WriteHandler(proc_frame_t *); //<---------
void SleepHandler(void);

void MutexLockHandler(int);
void MutexUnlockHandler(int);

void TermHandler(int);
void GetCharHandler(proc_frame_t *);

void PutCharHandler(proc_frame_t *);
void ForkHandler(proc_frame_t *);
void SignalHandler(proc_frame_t *);   //phase8
void InsertWrapper(int, func_p_t);
void WaitPidHandler(proc_frame_t *);
void ExitHandler(proc_frame_t *);
void ExecHandler(proc_frame_t *);
#endif
