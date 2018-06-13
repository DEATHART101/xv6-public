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

// Halt (shutdown) the system by sending a special signal to QEMU.
// Based on: https://github.com/noah-mcaulay/XV6-Operating-System/blob/master/sysproc.c
int
sys_shutdown(void)
{
  outw(0x604, 0x0 | 0x2000);
  return 0;
}

//get current system timestamp
//Added by Zhao Zhehui
int
sys_gettimestamp(void)
{
  rtcdate date;
  datetime(&date);
  uint ret = 946684800; //utc+0 2000/1/1 0:0:0 946684800
  int days[]={31,28,31,30,31,30,31,31,30,31,30,31};
  int year4num = (date.year - 2000) / 4;
  ret += year4num*(365*4+1)*86400;
  int yearfor4 = date.year - year4num * 4 - 2000;
  if(yearfor4>0)
  {
    ret += 366 * 86400;
    yearfor4--;
    ret += yearfor4 * 365 * 86400;
  }
  else
    days[1] = 29;
  int i;
  for(i = 1; i < date.month; i++)
    ret += days[i-1]*86400;
  ret += (date.day - 1) * 86400;
  ret += (date.hour * 3600 + date.minute * 60 + date.second);
  return ret;
}

