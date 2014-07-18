/*
The MIT License (MIT)

Copyright (c) 2014 Jean-Philippe Cottin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

This little C program will print different information regarding the memory used 
by a program passed in argument.

Every .1 second the program will print 
VmPeak (peak virtual memory size) VmSize (total program size) 
VmRSS(size of memory portions) and VmHWM (peak resident set size or "high water mark") . All sizes are in KiBytes. 
More info on those values in 
http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/Documentation/filesystems/proc.txt

*/
#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define PATH_MAX 2048
#define MAX_LINE_SIZE 128

int child_pid;

static int usage(char *me)
{
  fprintf(stderr, "%s: filename args\n", me);
  fprintf(stderr, "Run program, and print VmPeak, VmSize, VmRSS and VmHWM (in KiB) to stderr\n");

  return 0;
}

static int child(int argc, char **argv)
{
  char **newargs = malloc(sizeof(char *) * argc);
  int i;
	
  for (i = 0; i < argc - 1; i++)
    {
      newargs[i] = argv[i+1];
    }
  newargs[argc - 1] = NULL;
	
  execvp(argv[1], newargs);
	
  return 0;
}

static void sig_chld(int dummy)
{
  int status, child_val;
  int pid;
	
  (void) dummy;

  pid = waitpid(-1, &status, WNOHANG);
  if (pid < 0)
    {
      fprintf(stderr, "waitpid failed\n");
      return;
    }
	
  if (pid != child_pid) return;

  if (WIFEXITED(status))
    {
      child_val = WEXITSTATUS(status);
      exit(child_val);
    }
}

static int main_loop(char *pidstatus)
{
  char *line;
  char *vmsize;
  char *vmpeak;
  char *vmrss;
  char *vmhwm;
	
  size_t len;
	
  FILE *f;

  vmsize = NULL;
  vmpeak = NULL;
  vmrss = NULL;
  vmhwm = NULL;
  line = malloc(MAX_LINE_SIZE);
  len = MAX_LINE_SIZE;
	
  f = fopen(pidstatus, "r");
  if (!f) return 1;

  /* Read memory size data from /proc/pid/status pid being the one of the program */
  while (!vmsize || !vmpeak || !vmrss || !vmhwm)
    {
      if (getline(&line, &len, f) == -1)
	{
	  /* Some of the information isn't there, die */
	  return 1;
	}
		
      /* Find VmPeak */
      if (!strncmp(line, "VmPeak:", 7))
	{
	  vmpeak = strdup(&line[7]);
	}
		
      /* Find VmSize */
      else if (!strncmp(line, "VmSize:", 7))
	{
	  vmsize = strdup(&line[7]);
	}
		
      /* Find VmRSS */
      else if (!strncmp(line, "VmRSS:", 6))
	{
	  vmrss = strdup(&line[7]);
	}
		
      /* Find VmHWM */
      else if (!strncmp(line, "VmHWM:", 6))
	{
	  vmhwm = strdup(&line[7]);
	}
    }
  free(line);
	
  fclose(f);

  /* Get rid of " kB\n"*/
  len = strlen(vmsize);
  vmsize[len - 4] = 0;
  len = strlen(vmpeak);
  vmpeak[len - 4] = 0;
  len = strlen(vmrss);
  vmrss[len - 4] = 0;
  len = strlen(vmhwm);
  vmhwm[len - 4] = 0;
	
  /* Output results to stderr */
  fprintf(stderr, "%s\t%s\t%s\t%s\n", vmsize, vmpeak, vmrss, vmhwm);
	
  free(vmpeak);
  free(vmsize);
  free(vmrss);
  free(vmhwm);
	
  /* Success */
  return 0;
}

int main(int argc, char **argv)
{
  char buf[PATH_MAX];
	
  struct sigaction act;
	
  if (argc < 2) return usage(argv[0]);

  act.sa_handler = sig_chld;

  /* We don't want to block any other signals */
  sigemptyset(&act.sa_mask);
	
  act.sa_flags = SA_NOCLDSTOP;

  if (sigaction(SIGCHLD, &act, NULL) < 0)
    {
      fprintf(stderr, "sigaction failed\n");
      return 1;
    }
	
  child_pid = fork();
	
  if (!child_pid) return child(argc, argv);
	
  snprintf(buf, PATH_MAX, "/proc/%d/status", child_pid);
	
  fprintf(stderr, "\t\tVmSize\t\tVmPeak\t\tVmRSS\t\tVmHwM\t(KiB)\n");


  /* Continual scan of proc */
  while (!main_loop(buf))
    {
      /* Wait for 0.1 sec or 100000 micro */
      usleep(100000);
    }
	
  return 1;
}
