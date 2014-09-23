/**********************************************************************************
Common Cheat plugin spec, version #1.0 maintained by hacktarux (hacktarux@yahoo.fr) and okaygo (okaygossb@yahoo.com)

All questions or suggestions should go through yahoo messenger or email.

**********************************************************************************
Notes:
------


**********************************************************************************/
#ifndef _CHEAT_H_INCLUDED__
#define _CHEAT_H_INCLUDED__

#if defined(__cplusplus)
extern "C" {
#endif

/* Note: BOOL, BYTE, WORD, DWORD, TRUE, FALSE are defined in windows.h */

#define PLUGIN_TYPE_CHEAT			5

#define EXPORT						__declspec(dllexport)
#define CALL						_cdecl

#ifndef _BIG_ENDIAN

#define sl(mot) \
( \
((mot & 0x000000FF) << 24) | \
((mot & 0x0000FF00) <<  8) | \
((mot & 0x00FF0000) >>  8) | \
((mot & 0xFF000000) >> 24) \
)

#define S8 3
#define S16 2
#define Sh16 1

#else

#define sl(mot) mot
#define S8 0
#define S16 0
#define Sh16 0

#endif

/***** Structures *****/
typedef struct {
	WORD Version;        /* Should be set to 0x0100 */
	WORD Type;           /* Set to PLUGIN_TYPE_CHEAT */
	char Name[100];      /* Name of the DLL */

	/* If DLL supports memory these memory options then set them to TRUE or FALSE
	   if it does not support it */
	BOOL NormalMemory;   /* a normal BYTE array */ 
	BOOL MemoryBswaped;  /* a normal BYTE array where the memory has been pre
	                          bswap on a dword (32 bits) boundry */
} PLUGIN_INFO;


typedef struct {
	HWND hwnd;
	HINSTANCE hinst;

	BYTE * HEADER;	// This is the rom header (first 40h bytes of the rom
					// This will be in the same memory format as the rest of the memory.
	BYTE * RDRAM;

} CHEAT_INFO;

/******************************************************************
  Function: ApplyCheatsOnVI
  Purpose:  This function is called during each vi interrupt, it
  will apply all codes that get applied during each interrupt.
  input:    none
  output:   none
*******************************************************************/ 
EXPORT void CALL ApplyCheatsOnVI (void);

/******************************************************************
  Function: RomOpen
  Purpose:  This function is called when a rom is opened.
  input:    none
  output:   none
*******************************************************************/ 
EXPORT void CALL RomOpen (void);

/******************************************************************
  Function: CloseDLL
  Purpose:  This function is called when the emulator is closing
            down allowing the dll to de-initialise.
  input:    none
  output:   none
*******************************************************************/ 
EXPORT void CALL CloseDLL (void);

/******************************************************************
  Function: DllAbout
  Purpose:  This function is optional function that is provided
            to give further information about the DLL.
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/ 
EXPORT void CALL DllAbout ( HWND hParent );

/******************************************************************
  Function: DllConfig
  Purpose:  This function is optional function that is provided
            to allow the user to configure the dll
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/ 
EXPORT void CALL DllConfig ( HWND hParent );

/******************************************************************
  Function: DllTest
  Purpose:  This function is optional function that is provided
            to allow the user to test the dll
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/ 
EXPORT void CALL DllTest ( HWND hParent );

/******************************************************************
  Function: GetDllInfo
  Purpose:  This function allows the emulator to gather information
            about the dll by filling in the PluginInfo structure.
  input:    a pointer to a PLUGIN_INFO stucture that needs to be
            filled by the function. (see def above)
  output:   none
*******************************************************************/ 
EXPORT void CALL GetDllInfo ( PLUGIN_INFO * PluginInfo );

/******************************************************************
  Function: RomClosed
  Purpose:  This function is called when a rom is closed.
  input:    none
  output:   none
*******************************************************************/ 
EXPORT void CALL RomClosed (void);

/******************************************************************
  Function: InitiateGameshark
  Purpose:  nondescript
  input:    nondescript
  output:   nondescript
*******************************************************************/ 
EXPORT BOOL CALL InitiateCheat (CHEAT_INFO CheatInfo);


#if defined(__cplusplus)
}
#endif

#endif
