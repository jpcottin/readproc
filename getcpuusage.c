/*
The MIT License (MIT)

Copyright (c) 2015 Jean-Philippe Cottin

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

STILL UNDER CONSTRUCTION 
This little C program will print the cpu usage of the system

Every 1 second the program calculate the differential of the values coming from the 
 /proc/stat
 To do do, this program read /proc/stat

user: normal processes executing in user mode
nice: niced processes executing in user mode
system: processes executing in kernel mode
idle: idle

These numbers identify the amount of time the CPU has spent performing different kinds of work. Time units are in USER_HZ or Jiffies (typically hundredths of a second).

The 3 first values are added and divided by the sum of the 4 first values 

see 
http://www.linuxhowtos.org/System/procstat.htm
*/

#include <stdio.h>
#include <stdlib.h>
#define _GNU_SOURCE

int main(void)
{
  long double res1[4], res2[4], load_avg;
  FILE *file_pointer;

  for(;;)
    {
      file_pointer = fopen("/proc/stat","r");
      fscanf(file_pointer,"%*s %Lf %Lf %Lf %Lf",&res1[0],&res1[1],&res1[2],&res1[3]);
      fclose(file_pointer);
      sleep(1);

      file_pointer = fopen("/proc/stat","r");
      fscanf(file_pointer,"%*s %Lf %Lf %Lf %Lf",&res2[0],&res2[1],&res2[2],&res2[3]);
      fclose(file_pointer);

      load_avg = ((res2[0]+res2[1]+res2[2]) - (res1[0]+res1[1]+res1[2])) / ((res2[0]+res2[1]+res2[2]+res2[3]) - (res1[0]+res1[1]+res1[2]+res1[3]));
      fprintf(stderr,"%2.3Lf %%\n", 100*load_avg);
    }

  return(0);
}
