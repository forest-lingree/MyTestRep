#ifndef FOREST_BUFFER_UTIL
#define FOREST_BUFFER_UTIL

#include <iostream>
namespace Forest
{
    constexpr int kReadIovecNum = 4;
	constexpr int kBufferSizeMin = 1024;
	constexpr size_t kBufferChainMax = UINT64_MAX;
    struct bufferChain
    {
        bufferChain* _next;
        size_t _bufferLen;
        size_t _misalign;
        size_t _off;
        char* _buffer;
        bufferChain()
			:_next(nullptr),_bufferLen(0),_misalign(0),_off(0),_buffer(nullptr){};
    };
    constexpr size_t kBufferChainSize = sizeof(bufferChain);

    //for searching
	struct buffer_ptr
	{
		size_t _pos;
		bufferChain *_chain;
		size_t _posInChain;
	};
}

#endif