
// This is the file that allows the netRadio code to know which App it is being compiled in.
// it also contains the code to 'link' netRadio 'dialog' with the MF Audio system 
#ifndef MF_NR_BRIDGE 
#define MF_NR_BRIDGE

#ifndef IN_NETRADIO 
#define IN_MANYFRACTALS 
#endif

//#ifdef IN_MANYFRACTALS
//#define Q_OS_WIN
//#endif

#ifndef HSTREAM
typedef unsigned long HSTREAM;		// sample stream handle
#endif

extern HSTREAM chan ;

#ifdef IN_MANYFRACTALS
#define FGZ_NETRADIO_CLEANUP  1072
extern void PostCleanUp();
extern void PumpCleanUp() ;
#endif

extern bool bNR_Exists;

#endif