/*
  Copyright (C) May 2011 Frederick G. Zacharias

  Permission is hereby granted, free of charge, to any person obtaining a copy of this 
  software and associated documentation files (the "Software"), to deal in the Software
  without restriction, including without limitation the rights to use, copy, modify, 
  merge, distribute, web post copies of the Software, and to 
  permit persons to whom the Software is furnished to do so, subject to the following conditions:

  (Zac Labs Hybrid Open/Commercial Condition)
  You Agree that all previous authors are justly, reasonable, and fairly compensated in a 
  written legal agreement between you and them Before the software can be Sublicensed, Sold or 
  in any other way exchanged for any type of financial, fiscal, monetary or otherwise benifited 
  by the distribution of the software, with consideration of the percentage of modifications by 
  you and your associates in future works of this software.
  This condition is not required on in house distribution within a single company or entity.
  This condition implies a certain, to be agreed upon in the future written agreement : 
  Warranty of fitness and quality that is to be reflected in the future written future agreement,
  and without which the below LACK OF WARRANTY DOES APPLY.
  
  The above copyright notice and this permission notice shall be included in all copies or 
  substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
*/


// This is a custom named memory allocation utility for maintence purposes.
// as of using VS 2008 professional, VirtualAlloc tests being 2.5 times faster than
// All other memory allocation functions. and VirtualAlloc always zeros out memory,
// where as the other other memory functions in the test did not. Meaning VirtualAlloc
// did more in much less time. Therefore we'll use it from now on.
// 
// 05-05-09 switching to using new / DEBUG_NEW and a zeromem as virtualalloc allocates in pages(4096 bytes) 
//             ie: 2 reasons to not use virtualalloc .
//
// VirtualAlloc( NULL, xBytes, MEM_COMMIT | MEM_RESERVE , PAGE_READWRITE )  is equal to :
// VirtualAlloc( 0, xBytes, 0x1000 | 0x2000 , 0x04 )
//
// VirtualFree( pMem, 0, MEM_RELEASE )  is equal to :
// VirtualFree( pMem, 0, 0x8000 )
#pragma once

// #define SuperAlloc(ZeroMem, xBytes)  VirtualAlloc( 0, xBytes, 0x1000 | 0x2000 , 0x04 )
// #define SuperFree(pMem)  VirtualFree( pMem, 0, 0x8000 )

#include <intrin.h>
#pragma intrinsic(__stosd)

#ifdef _DEBUG

inline void* SuperAlloc(bool ZeroMem, int xBytes) { int sid = (xBytes+((4-(xBytes%4))%4))/4; void* ret = DEBUG_NEW int[sid]; if(ZeroMem) __stosd((unsigned long*)ret, 0, sid); return ret; }

#else

inline void* SuperAlloc(bool ZeroMem, int xBytes) 
{ 
	int sid = 0;
	void* ret = NULL;
	try 
	{
		sid = (xBytes+((4-(xBytes%4))%4))/4; 
		ret = new int[sid]; 
	}
	catch(...) { return NULL; }
	try
	{
		if(ZeroMem) __stosd((unsigned long*)ret, 0, sid); 
		return ret;
	}
	catch(...) 
	{ 
		delete [] ((int*)ret); 
		return NULL; 
	}
}

#endif

#define SuperFree(pMem)  { if(pMem != NULL) { delete [] ((int*)pMem); pMem = NULL; }} 


