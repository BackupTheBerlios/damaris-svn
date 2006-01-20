#define MAXBRD 16
#define VERSION 1.00

#ifdef __BORLANDC__
#  ifdef _Windows
#    define BC_WIN31
#  else
#    define BC_DOS
#  endif
#endif

#ifdef _WINSTDCALL
#  ifdef _MSC_VER
#    ifdef _WINNT
#      define VC_STDCALLNT
#    else
#      define VC_STDCALL95
#    endif
#  endif
#elif _MSC_VER
#  ifdef _WIN32
#    ifdef _WINNT
#      define VC_WINNT
#    else
#      define VC_WIN95
#    endif
#  else
#    define VC_WIN31
#  endif
#endif

#ifdef __GNUC__
#  define _LINUX
#endif

#ifdef BC_DOS
#  define int16   int
#  define uint16  unsigned int
#  define int8    char
#  define uint8   unsigned char
#  define int32   long int
#  define uint32  unsigned long int
#  define dataptr void huge*
#  define ptr8    char huge*
#  define uptr8   unsigned char huge*
#  define ptr16   int huge*
#  define uptr16  unsigned int huge*
#  define ptr32   long int huge*
#  define uptr32  unsigned long int huge*
#  define EXP     extern "C" _export int16
#  define EXPC    extern  _export int16
#  define IMP     extern "C" _import int16
#  define HEAD    extern "C" int16
#endif

#ifdef BC_WIN31
#  define int16   int
#  define uint16  unsigned int
#  define int8    char
#  define uint8   unsigned char
#  define int32   long int
#  define uint32  unsigned long int
#  define dataptr void huge*
#  define ptr8    char huge*
#  define uptr8   unsigned char huge*
#  define ptr16   int huge*
#  define uptr16  unsigned int huge*
#  define ptr32   long int huge*
#  define uptr32  unsigned long int huge*
#  ifdef _EasyWin
#    define EXP     extern "C" _export int16
#    define IMP     extern "C" _import int16
#    define HEAD    extern "C" int16
#  else
#    define EXP     extern "C" _export int16 FAR PASCAL
#    define EXPC    extern     _export int16 FAR PASCAL
#    define IMP     extern "C" _import int16 FAR PASCAL
#    define HEAD    extern "C" int16 FAR PASCAL
#  endif
#endif

#ifdef VC_WIN31
#  define int8    char
#  define uint8   unsigned char
#  define int16   short int
#  define uint16  unsigned short int
#  define int32   long int
#  define uint32  unsigned long int
#  define dataptr void huge*
#  define ptr8    char*
#  define uptr8   unsigned char*
#  define ptr16   short int*
#  define uptr16  unsigned short int*
#  define ptr32   long int*
#  define uptr32  unsigned long int*
#  define EXP     extern "C" __declspec (dllexport) int16
#  define IMP     extern "C" __declspec (dllimport) int16
#  define HEAD    extern "C" __declspec (dllexport) int16
#endif

#if defined(VC_WIN95) || defined(VC_WINNT)
#  define int8    char
#  define uint8   unsigned char
#  define int16   short int
#  define uint16  unsigned short int
#  define int32   long int
#  define uint32  unsigned long int
#  define dataptr void*
#  define ptr8    char*
#  define uptr8   unsigned char*
#  define ptr16   short int*
#  define uptr16  unsigned short int*
#  define ptr32   long int*
#  define uptr32  unsigned long int*
#  define EXP     extern "C" __declspec (dllexport) int16
#  define EXPC    extern __declspec (dllexport) int16
#  define IMP     extern "C" __declspec (dllimport) int16
#  define HEAD    extern "C" __declspec (dllexport) int16
#endif

#if defined(VC_STDCALL95) || defined(VC_STDCALLNT)
#  define int8    char
#  define uint8   unsigned char
#  define int16   short int
#  define uint16  unsigned short int
#  define int32   long int
#  define uint32  unsigned long int
#  define dataptr void*
#  define ptr8    char*
#  define uptr8   unsigned char*
#  define ptr16   short int*
#  define uptr16  unsigned short int*
#  define ptr32   long int*
#  define uptr32  unsigned long int*
#  define EXP     extern "C" __declspec (dllexport) int16 _stdcall
#  define EXPC    extern     __declspec (dllexport) int16 _stdcall
#  define IMP     extern "C" __declspec (dllimport) int16 _stdcall
#  define HEAD    extern "C" __declspec (dllexport) int16 _stdcall
#endif

#ifdef _LINUX
#  define int8 char
#  define int16 short int
#  define int32 int
#  define uint8 unsigned char
#  define uint16 unsigned short int
#  define uint32 unsigned int
#  define dataptr void *
#  define ptr8 int8*
#  define ptr16 int16*
#  define ptr32 int32*
#  define uptr8 uint8*
#  define uptr16 uint16*
#  define uptr32 uint32*
#  define EXPC int16
#  define HEAD int16
#  define SPEC_IOC_MAGIC 's'
#  define SPEC_IOC_MAXNR 6
#  define GETPARAM  _IOR(SPEC_IOC_MAGIC,1,int32[2])
#  define SETPARAM  _IOW(SPEC_IOC_MAGIC,2,int32[2])
#  define GETCH     _IOR(SPEC_IOC_MAGIC,3,int32[1])
#  define SETCH     _IOW(SPEC_IOC_MAGIC,4,int32[1])
#endif
