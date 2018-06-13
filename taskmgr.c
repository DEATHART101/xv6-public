#include "types.h"
#include "stat.h"
#include "user.h"

#define NPROC 64  // maximum number of processes
#define KEY_UP 0xE2
#define KEY_DN 0xE3
#define KEY_LF 0xE4
#define KEY_RT 0xE5

const char procstate[6][9] = {"UNUSED", "EMBRYO", "SLEEPING", "RUNNABLE", "RUNNING", "ZOMBIE"};

char buff[96][80];
int curline = 4;  //表示当前高亮显示的行序号，范围4-19
int curpage = 0;  //表示当前页码，范围0-3
int procID[NPROC];
char procName[NPROC][16];
int procState[NPROC];
uint procSize[NPROC];

void runTaskMgr(void);
int getCmd(char *buf);
void procCmd(char *cmd);
//int fork2(void);
void initTaskMgrInfo();
void updateTaskMgrInfo(void);
//void printTaskMgrInfo(void);
//void printString(char *fmt, int line, ...);
//void printPageInfo();
//int printint(int xx, int base, int sign, int pos, int limit, char *buff);

void
runTaskMgr(void)
{
  static char buf[2];
  char *c;

  initTaskMgrInfo();
  updateTaskMgrInfo();
  updscrcont(buff[curpage*24], curline);
  
  while(getCmd(buf) >= 0)
  {
    /*
    if(fork2() == 0)
    {
      c = buf;
      procCommand(c);
    }
    sleep(1);
    */
    c = buf;
    procCmd(c);
    updateTaskMgrInfo();
    updscrcont(buff[curpage*24], curline);
  }
}

int
getCmd(char *buf)
{
  memset(buf, 0, sizeof(char) * 2);
  gets(buf, 2);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

void
procCmd(char *cmd)
{
  switch((*cmd) & 0xff){
  case KEY_UP:
    if(curline > 4)
      curline--;
    break;
  case KEY_DN:
    if(curline < 19)
      curline++;
    break;
  case KEY_LF:
    if(curpage > 0)
      curpage--;
    break;
  case KEY_RT:
    if(curpage < 3)
      curpage++;
    break;
  case 'k':
    if(strcmp("taskmgr", procName[curpage*16+curline-4]) == 0){
      closetaskmgr();
      exit();
      break;
    }
    else{
      kill(procID[curpage*16+curline-4]);
      break;
    }
  case 'q':
    closetaskmgr();
    exit();
    break;
  default:
    break;
  }
}

/*
int
fork2(void)
{
  int pid;

  pid = fork();
  if(pid == -1)
    exit();
  return pid;
}
*/

void
initTaskMgrInfo(void)
{
  char title1[] = "Task Maneger";
  char title2[] = "pid        name       state      memory(bytes)";
  char footNote[] = "page: ";
  int curDigit;
  for(int page = 0; page < 4; page++)
  {
    curDigit = 0;
    for(char *s = title1; *s != '\0'; s++)
    {
      buff[24*page + 0][curDigit++] = *s;
    }
    curDigit = 0;
    for(char *s = title2; *s != '\0'; s++)
    {
      buff[24*page + 1][curDigit++] = *s;
    }
    curDigit = 40;
    for(char *s = footNote; *s != '\0'; s++)
    {
      buff[24*page + 23][curDigit++] = *s;
    }
    buff[24*page + 23][curDigit++] = page + '1';
    buff[24*page + 23][curDigit++] = '/';
    buff[24*page + 23][curDigit++] = '4';
  }
}

void
updateTaskMgrInfo(void)
{
  int curDigit;
  char localBuff[10];
  static char digits[] = "0123456789";
  getprocinfo(procID, procName, procState, procSize);
  for(int page = 0; page < 4; page++)
  {
    for(int i = 0; i < 16; i++)
    {
      curDigit = 0;
      int localPartDigit = 0;
      do
      {
        localBuff[localPartDigit++] = digits[procID[24*page + i] % 10];
      } while((procID[24*page + i] /= 10) != 0 && localPartDigit < 10);
      while(localPartDigit > 0)
      {
        buff[24*page + (i+4)][curDigit++] = localBuff[--localPartDigit]; 
      }
      while(curDigit < 11)
      {
        buff[24*page + (i+4)][curDigit++] = 0;
      }

      localPartDigit = 0;
      if(procName[24*page + i][0] == 0)
      {
        buff[24*page + (i+4)][curDigit++] = '0';
      }
      else
      {
        while(procName[24*page + i][localPartDigit] && curDigit < 21)
        {
          buff[24*page + (i+4)][curDigit++] = procName[24*page + i][localPartDigit++];
        }
      }
      while(curDigit < 22)
      {
        buff[24*page + (i+4)][curDigit++] = 0;
      }

      localPartDigit = 0;
      while(procstate[procState[24*page + i]][localPartDigit] != '\0' && curDigit < 32)
      {
        buff[24*page + (i+4)][curDigit++] = procstate[procState[24*page + i]][localPartDigit++];
      }
      while(curDigit < 33)
      {
        buff[24*page + (i+4)][curDigit++] = 0; 
      }

      localPartDigit = 0;
      do
      {
        localBuff[localPartDigit++] = digits[procSize[24*page + i] % 10];
      } while((procSize[24*page + i] /= 10) != 0 && localPartDigit < 10);
      while(localPartDigit > 0)
      {
        buff[24*page + (i+4)][curDigit++] = localBuff[--localPartDigit]; 
      }
      while(curDigit < 44)
      {
        buff[24*page + (i+4)][curDigit++] = 0;
      }
    }
  }
}

/*
void
printTaskMgrInfo(void)
{
  printf(1, "printTaskMgrInfo\n");
}

void
printString(char *fmt, int line, ...) // 打印在第line行
{
  int c, digitsDesired, digitsUsed, remain, currentDigit = 0; //目前輸出到第line行第currentDigit位
  char *s;
  memset(buff[line/80], 0, sizeof(char)*80);
  uint *argp;
  
  if (fmt == 0)
  {
    printf(1, "null fmt\n");
  }
  
  for(int i = 0; (c = fmt[i] & 0xff) != 0; i++)
  {
    if (currentDigit = 0)
    {
      goto bad;
    }
    if (c != '%')
    {
      if (c == '\n')
      {
        buff[line/80][currentDigit++] = 0;
        currentDigit = 80;
      }
      else
      {
        buff[line/80][currentDigit++] = c;
      }
      continue;
    }
    c = fmt[++i] & 0xff;
    if (c == 0)
    {
      break;
    }
    digitsDesired = 0;
    digitsUsed = 0;
    while (c >= '0' && c <= '9')
    {
      digitsDesired = digitsDesired * 10 + c - '0';
      c = fmt[++i] & 0xff;
    }
    if (currentDigit + digitsDesired > 80)
    {
      goto bad;
    }
    remain = (digitsDesired == 0) ? (80 - currentDigit) : digitsDesired;
    
    switch(c)
    {
      case 'd':
        digitsUsed = printint(*argp++, 10, 1, currentDigit, remain, buff[line / 80]);
        currentDigit += digitsUsed;
        while(digitsUsed <= digitsDesired)
        {
          buff[line / 80][currentDigit++] = 0;
          digitsUsed++;
        }
        break;
      case 'x':
      case 'p':
        digitsUsed = printint(*argp++, 16, 0, currentDigit, remain, buff[line / 80]);
        currentDigit += digitsUsed;
        while(digitsUsed <= digitsDesired)
        {
          buff[line / 80][currentDigit++] = 0;
          digitsUsed++;
        }
        break;
      case 's':
        if((s = (char*)*argp++) == 0)
    	    s = "(null)";
        while(digitsUsed < remain)
        {
          buff[line / 80][currentDigit++] = *s;
          s++;
          digitsUsed++;
        }
	  		while(digitsUsed <= digitsDesired) //補空格
	  		{
				  buff[line / 80][currentDigit++] = 0;
				  digitsUsed++;
	  		}
        break;
      case '%':
        buff[line / 80][currentDigit] = c;
        break;
      default:
        if (currentDigit > 78)
        {
          goto bad;
        }
        else
        {
          buff[line / 80][currentDigit++] = '%';
          buff[line / 80][currentDigit++] = c;
        }
        break;
    }
    return;
  }
  bad:
}

int printint(int xx, int base, int sign, int pos, int limit, char *buff)
{

}

void
printPageInfo()
{

}
*/

int
main(void)
{
  inittaskmgr();
  runTaskMgr();
  exit();
}
