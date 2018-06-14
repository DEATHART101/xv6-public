#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_inittaskmgr(void)
{
  return inittaskmgr();
}

int
sys_closetaskmgr(void)
{
  return closetaskmgr();
}

int
sys_getprocinfo(void)
{
  int *pid;
  char (*name)[16];
  int *state;
  uint *sz;

  if(argptr(0, (void*)&pid, 64*sizeof(int)) < 0 ||
     argptr(1, (void*)&name, 64*16*sizeof(char)) < 0 ||
     argptr(2, (void*)&state, 64*sizeof(int)) < 0 ||
     argptr(3, (void*)&sz, 64*sizeof(uint)) < 0)
    return -1;
  return getprocinfo(pid, name, state, sz);
}

int
sys_updscrcont(void)
{
  char *buf;
  int curline;

  if(argptr(0, &buf, 24 * 80) < 0 || argint(1, &curline))
    return -1;
  return updscrcont(buf, curline);
}
