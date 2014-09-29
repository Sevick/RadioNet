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

#pragma once


#ifdef Q_OS_WIN
// The following macros define the minimum required platform.  The minimum required platform
// is the earliest version of Windows, Internet Explorer etc. that has the necessary features to run 
// your application.  The macros work by enabling all features available on platform versions up to and 
// including the version specified.

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER                          // Specifies that the minimum required platform is Windows Vista.
#define WINVER 0x0502           // Change this to the appropriate value to target other versions of Windows.
#endif

////////  Windows 2000 == 500  //  Windows XP == 501  //  Windows XP sp2 == 502  // Vista-icky == 666  :-) 

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0502     // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS          // Specifies that the minimum required platform is Windows 98.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE                       // Specifies that the minimum required platform is Internet Explorer 7.0.
#define _WIN32_IE 0x0500        // Change this to the appropriate value to target other versions of IE.
#endif
#endif

extern bool SetRuntimeOS(); 
 
extern unsigned long Runtime_OS_Version;

enum OS_TYPE
{
	OS_Win2000 = 0x500,
	OS_WinXP   = 0x501,
	OS_Srv2003 = 0x502,
    OS_Linux   = 0x550,
    OS_MacOSX  = 0x580,
	OS_Vista   = 0x600,
	OS_Win7    = 0x601 
};

