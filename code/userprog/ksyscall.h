/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__
#define __USERPROG_KSYSCALL_H__

#include "kernel.h"

#include "synchconsole.h"


void SysHalt()
{
  kernel->interrupt->Halt();
}

int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

#ifdef FILESYS_STUB
int SysCreate(char *filename)
{
	// return value
	// 1: success
	// 0: failed
	return kernel->interrupt->CreateFile(filename);
}
#endif

/* MP1 */
int SysClose(int id)
{
    return kernel->interrupt->Close(id);
}

int SysRead(char* buffer , int size , int id)
{
    return kernel->interrupt->Read(buffer, size, id);
}

int SysWrite(char* buffer , int size , int id)
{
    return kernel->interrupt->Write(buffer, size, id);
}

int SysOpen(char *filename)
{
    return kernel->interrupt->Open(filename);
}

/* MP4 */
int SysCreate(char *filename, int size)
{
	// return value
	// 1: success
	// 0: failed
	return kernel->interrupt->CreateFile(filename, size);
}


#endif /* ! __USERPROG_KSYSCALL_H__ */
