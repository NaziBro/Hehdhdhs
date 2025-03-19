#ifndef _FUNCTIONTOOL_H
#define _FUNCTIONTOOL_H
#include <stdio.h>
#include <map>
#include <mutex>
#include <sys/mman.h>

namespace FunctionFlag{
	extern int Alive;
	extern int Die;
}


class FunctionInfo{
	public:
	uint64_t address;
	char opcodes[8];
	int state;
	FunctionInfo(uint64_t addr);
	FunctionInfo(void);
	void kill(uint8_t ret);
	void rebuild(void);
};

class FunctionTool{
	public:
	std::map<uint64_t,FunctionInfo> Functions;
	std::mutex mapMutex;
	void addFunction(uint64_t addr);
	void killFunction(uint64_t addr,uint8_t ret = 1);
	void rebuildFunction(uint64_t addr);
	void killAll(uint8_t ret = 1);
	void rebuildAll(void);
	void clear(void);
	void rm(uint64_t addr);
	static void WriteDataToCode(void* address,void* data,size_t len);
};

#endif


