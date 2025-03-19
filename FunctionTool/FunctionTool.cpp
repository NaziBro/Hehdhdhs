#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stddef.h>
#include <dlfcn.h>
#include <dirent.h>
#include <stdlib.h>
#include "FunctionTool/FunctionTool.h"


#define PAGE_SIZE 4096

namespace FunctionFlag{
	int Alive = 1;
	int Die = 0;
}



void FunctionTool::addFunction(uint64_t addr){
	FunctionInfo info(addr);
	std::lock_guard<std::mutex> lck(mapMutex);
	if(Functions.count(addr) == 0){
		Functions.insert(std::pair<uint64_t,FunctionInfo>(addr,info));
	}
}

void FunctionTool::killFunction(uint64_t addr,uint8_t ret){
	std::lock_guard<std::mutex> lck(mapMutex);
	if(Functions.count(addr) != 0){
		Functions[addr].kill(ret);
	}
}

void FunctionTool::rebuildFunction(uint64_t addr){
	std::lock_guard<std::mutex> lck(mapMutex);
	if(Functions.count(addr) != 0){
		Functions[addr].rebuild();
	}
}

void FunctionTool::killAll(uint8_t ret){
	std::lock_guard<std::mutex> lck(mapMutex);
	std::map<uint64_t,FunctionInfo>::iterator it;
	for(it=Functions.begin();it!=Functions.end();++it){
		it->second.kill(ret);
	}
}

void FunctionTool::rebuildAll(void){
	std::lock_guard<std::mutex> lck(mapMutex);
	std::map<uint64_t,FunctionInfo>::iterator it;
	for(it=Functions.begin();it!=Functions.end();++it){
		it->second.rebuild();
	}
}

void FunctionTool::clear(void){
	std::lock_guard<std::mutex> lck(mapMutex);
	Functions.erase(Functions.begin(),Functions.end());
}

void FunctionTool::rm(uint64_t addr){
	std::lock_guard<std::mutex> lck(mapMutex);
	Functions.erase(addr);
}

FunctionInfo::FunctionInfo(void){
	
}

FunctionInfo::FunctionInfo(uint64_t addr){
	address = addr;
	uint64_t memaddr = address;
	uint64_t start = memaddr - memaddr % PAGE_SIZE;
	mprotect((void*)start, PAGE_SIZE * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
	memcpy(opcodes, (void*)memaddr, 8);
	state = FunctionFlag::Alive;
	mprotect((void*)start, PAGE_SIZE * 2, PROT_READ | PROT_EXEC);
}

void FunctionInfo::kill(uint8_t ret)
{
	uint64_t memaddr = address;
	uint64_t start = memaddr - memaddr % PAGE_SIZE;
	mprotect((void*)start, PAGE_SIZE * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
	
	uint8_t new_opcodes[] = { (uint8_t)((ret*2%16)*16),(uint8_t)(ret*2/16),0x80,0x52, 0xC0 ,0x03 ,0x5F ,0xD6 };
	memcpy((void*)memaddr, new_opcodes, 8);

	state = FunctionFlag::Die;
	mprotect((void*)start, PAGE_SIZE * 2, PROT_READ | PROT_EXEC);
}

void FunctionInfo::rebuild(void)
{
	uint64_t memaddr = address;
	uint64_t start = memaddr - memaddr % PAGE_SIZE;
	mprotect((void*)start, PAGE_SIZE * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
	memcpy((void*)memaddr, opcodes, 8);
	state = FunctionFlag::Alive;
	mprotect((void*)start, PAGE_SIZE * 2, PROT_READ | PROT_EXEC);
}

void FunctionTool::WriteDataToCode(void* address,void* data,size_t len){
	uint64_t memaddr = (uint64_t)address;
	uint64_t start = memaddr - memaddr % PAGE_SIZE;
	int l = len / PAGE_SIZE + 1;
	mprotect((void*)start, PAGE_SIZE * l, PROT_READ | PROT_WRITE | PROT_EXEC);
	memcpy(address,data,len);
	mprotect((void*)start, PAGE_SIZE * l, PROT_READ | PROT_EXEC);
}





