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

#include "mainwindow.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#endif


unsigned long Runtime_OS_Version = 0;

bool SetRuntimeOS() 
{
#ifdef Q_OS_WIN

    OSVERSIONINFO osvi;
   	 
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    
    if( GetVersionEx( &osvi ) == FALSE ) { Runtime_OS_Version = 0x400; return false; }
    
    Runtime_OS_Version = ((osvi.dwMajorVersion << 8) | osvi.dwMinorVersion) ;
#else
#ifdef Q_OS_MAC
    Runtime_OS_Version = OS_MacOSX;
#else
    Runtime_OS_Version = OS_Linux;
#endif
#endif
   return true;
}
