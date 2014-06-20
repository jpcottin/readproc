readproc
========

Tools to extract information of the proc filesystem .
Every .1 second the program will print 
VmPeak (peak virtual memory size) 

VmSize (total program size) 

VmRSS(size of memory portions)

VmHWM (peak resident set size or "high water mark") 

All sizes are in KiBytes. 
More info on those values in 
http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/Documentation/filesystems/proc.txt


