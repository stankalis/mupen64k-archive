




/*
  Name: kaillera.c
  Copyright: 2007
  Author: okaygo
  Date: 09/08/07 13:07
  Description: kaillera.c - handles netplay calls for Mupen64
*/

// THIS NEEDS MAJOR CLEAN UP!
/* TODO:
        Organization
        Optimization
        Recode?
*/

#include <stdio.h>
#include <windows.h>
#include "kaillera.h"

#include "main_win.h"
#include "Gui_logWindow.h"
#include "rombrowser.h"
#include "../../main/plugin.h"

#include "../../main/guifuncs.h"
#include "../../main/vcr.h"
#include "../../main/rom.h"

#include "../../winproject/resource.h"

static HMODULE KailleraHandle = NULL;

static char	szKailleraNamedRoms[50 * 2000];

int  SizeToSend;
int  SaveDataPos = -1;
int  NoTrimBuffer = 0;

BYTE InformationToSend[0x20000];
BYTE InformationToRecv[0x20000];
extern int UseSave;
    
DWORD BufferKeys[4];
DWORD TempKeys[4];
DWORD FileSize[4];

void KailleraChatKeyDown( WPARAM wParam, LPARAM lParam )
{

       TCHAR strTo;  
       strTo = (TCHAR)wParam;

       switch (wParam)
       {
              
           case VK_BACK:
               KailleraChatMessage[strlen(KailleraChatMessage)-1] = NULL;
               SetStatusTranslatedString( hStatus, 0, KailleraChatMessage);     
               break;
           case VK_ESCAPE:
               *KailleraChatMessage = NULL;
               KailleraChatEnabled = FALSE;
               SetStatusTranslatedString( hStatus, 0, "Chat Cancelled");  
               break;
           case VK_RETURN:
               KailleraGameChatSend();
               break;
           case VK_TAB:
               break;                  
           default:
               if (strlen(KailleraChatMessage)+1 <= 128)
               {  
                   if (strlen(KailleraChatMessage) > 1)
                   sprintf(KailleraChatMessage,"%s%c",KailleraChatMessage,strTo);
                   else
                   sprintf(KailleraChatMessage,"%c%c",KailleraChatMessage,strTo);
                   
                   SetStatusTranslatedString( hStatus, 0, KailleraChatMessage);
               }
           break;  
      }          

}

void KailleraGameChatSend()
{
    kailleraChatSend(KailleraChatMessage); 
    *KailleraChatMessage = NULL; 
    KailleraChatEnabled = FALSE;
}


DWORD WINAPI LaunchKaillera(LPVOID lpParam)
{
       ShowInfo("Kaillera : Loaded");
       KailleraOpen = TRUE;
       LoadKaillera(TargetKailleraClient);
       ShowInfo("Kaillera : Shutdown"); 
       KailleraOpen = FALSE;
       EnableMenuItem(GetMenu(mainHWND), ID_SUPRACLIENT, MF_ENABLED);
       EnableMenuItem(GetMenu(mainHWND), ID_KAILLERACLIENT, MF_ENABLED);
      
       ExitThread(0);
       
       return 0;
}



DWORD WINAPI KailleraKeepAlive(LPVOID lpParam)
{
     /*  while (KailleraGamePaused)
       {
             NetPlayInformation[0].KailleraMessageType = 0x00;
             NetPlayInformation[0].KailleraMessage = 0x00;
             KailleraModifyPlayValues(NetPlayInformation,5);
             sleep(50);             
       } */
       ExitThread(0);
       
       return 0;
}

int LoadKaillera(char *ClientType)
{

    char TempStr[MAX_PATH];
   

    sprintf(TempStr,"%s%s",AppPath,ClientType) ; 
 
    KailleraHandle = LoadLibrary(TempStr);
    
    if (KailleraHandle)
    {
      ShowInfo("Kaillera Library found") ;
     
      EnableMenuItem(GetMenu(mainHWND), ID_SUPRACLIENT, MF_GRAYED);
      EnableMenuItem(GetMenu(mainHWND), ID_KAILLERACLIENT, MF_GRAYED);
    
      kailleraGetVersion = (void (__stdcall* ) ()) GetProcAddress( KailleraHandle, "_kailleraGetVersion@4");
      if (kailleraGetVersion==NULL) {
         ShowError("kailleraGetVersion not found") ;
         return 0 ;
      }   
      
      kailleraInit = (void (__stdcall *)(void)) GetProcAddress( KailleraHandle, "_kailleraInit@0");
      if (kailleraInit==NULL) {
         ShowError("kailleraInit not found") ;
         return 0  ;
      } 
      
      kailleraShutdown = (void (__stdcall *) (void)) GetProcAddress(KailleraHandle, "_kailleraShutdown@0");
	  if(kailleraShutdown == NULL) {
	     ShowError("kailleraShutdown not found") ;
         return 0  ;
      }
      
      kailleraSetInfos = (void(__stdcall *) (kailleraInfos *)) GetProcAddress(KailleraHandle, "_kailleraSetInfos@4");
      if(kailleraSetInfos == NULL) {
	     ShowError("kailleraSetInfos not found") ;
         return 0  ;
      } 
      
      kailleraSelectServerDialog = (void (__stdcall* ) (HWND parent)) GetProcAddress(KailleraHandle, "_kailleraSelectServerDialog@4");
      if (kailleraSelectServerDialog == NULL) {
        ShowError("kailleraSelectServerDialog not found"); 
        return 0  ;   
      }
      
      kailleraModifyPlayValues = (int (__stdcall *) (void *values, int size)) GetProcAddress ( KailleraHandle,"_kailleraModifyPlayValues@8");
    if(kailleraModifyPlayValues == NULL) {
      ShowError("kailleraModifyPlayValues not found");
         return 0  ;
       }
      
      kailleraChatSend = (void(__stdcall *) (char *)) GetProcAddress( KailleraHandle, "_kailleraChatSend@4");
	  if(kailleraChatSend == NULL) {
	    ShowError("kailleraChatSend not found");
        return 0  ;
      }

      kailleraEndGame = (void (__stdcall *) (void)) GetProcAddress( KailleraHandle, "_kailleraEndGame@0");
	  if(kailleraEndGame == NULL) {
	    ShowError("kailleraEndGame not found");
	    return 0 ;
      }
      kailleraGetVersion(TempStr);
      ShowInfo( "Kaillera version %s", TempStr);
      ShowInfo( "Starting Kaillera...") ;
      KailleraPlay() ;
      return 1;        
   }
   else {
      ShowError("Kaillera Library file 'kailleraclient.dll' not found ") ;
      return 0;
    }
}
      
     
int KailleraModifyPlayValues(void *values, int size)
{
    int BufferSize;
    int LoopPos;
    BYTE SaveDataTemp[4];

    BufferSize = kailleraModifyPlayValues((void *)values,size);
    *NetPlayInformationRecv = *((KailleraPacket*)values);
  
    
    if (BufferSize == -1)
    {
       // Game termination not found...
       EndGameKaillera();
       ShowInfo("Connection Lost");            
       return -1;
    } // Connection ended.
    if (BufferSize == 0)
    {          
       ShowInfo("buffering frame");   
       return 0;
    } // Buffering.
    else
    {
        *NetPlayInformationRecv = *((KailleraPacket*)values);
        // Question: Do we want other users to have some form of control, or just player 1?
        switch (NetPlayInformationRecv[0].KailleraMessageType)
        {

               case 0x00: // No data
               break;
               case 0x01: // Key data
               break;
               case 0x02: // Reset (incomplete)
               break;
               case 0x03: // Pause game (broken/incomplete)
                    KailleraGamePaused = TRUE;
                    pauseEmu();
                    CreateThread(NULL, 0, KailleraKeepAlive, NULL, 0, 0);
                    break;
               case 0x04: // Resume Game (broken/incomplete)
                    KailleraGamePaused = FALSE;
                    resumeEmu();
                    break;     
               case 0x05: // Save Data Size
                    ShowInfo("05");
                    SizeToSend = (int)NetPlayInformationRecv[0].KailleraMessage;
                    break;         
               case 0x06: // Save Data 
                    ((DWORD *)SaveDataTemp)[0] = NetPlayInformationRecv[0].KailleraMessage;
                
                    for (LoopPos = 0; LoopPos < 4; LoopPos++)
                    {
                        SaveDataPos++;
                        if (SaveDataPos > SizeToSend)
                           ShowInfo("Overflow for SaveData: %i > %i",SaveDataPos,SizeToSend);
                        else
                           InformationToRecv[SaveDataPos] = SaveDataTemp[LoopPos];
                    }
      
                    break; 
            
                 
        }         
    }
    return BufferSize;
}

void EndGameKaillera()
{
    if (KailleraHandle) { 
                     
       ShowInfo("Kaillera : kailleraEndGame()");
       
       UseSave = 1;
       SaveDataPos=-1;
       KailleraRunningGame = FALSE;
       kailleraEndGame();
    }   
}

void WINAPI kailleraMoreInfosCallback(char *gamename)
{
     // I might want to create a config line for this, it would be easier incase the site died.
     
     char SearchROM[MAX_PATH];
     
     sprintf(SearchROM,"http://daroms.com/KailleraRomInfoN64.php?query=%s",gamename);
     
     // Also this is Windows only code...
#ifdef __WIN32__
     ShellExecute(0,
     "open",
     SearchROM,
     NULL,     
     NULL,     
     SW_SHOW);
#endif

}

char path_buffer[_MAX_PATH];
int WINAPI kailleraGameCallback(char *game, int player, int numplayers)
{

    int i;
    extern RECT RealRect;
    DWORD DummyBuffer[3];
    extern char *KailleraSaveBuffer;
  
    int PlayerSlot = player - 1;

    char kaillera_info[_MAX_PATH];

    ROM_INFO *pRomInfo;
    
   ShowInfo("Kaillera : kailleraGameCallback()");
   for (i=0;i<ItemList.ListCount;i++)
   {
     pRomInfo = &ItemList.List[i];
    
     if (strcmp(pRomInfo->GoodName, game) == 0) {
     
         extern rom_settings ROM_SETTINGS;
   
         ShowInfo("Kaillera : Starting rom %s",pRomInfo->GoodName);
    
         int   MessagePosition;
         BYTE  KailleraDelay[4];
         DWORD SizeOfMessage[4];
         DWORD SaveMessage[4];
         if (PlayerSlot == 0)
         {
            UseSave = 1;
         }
         else
         {
            UseSave = 0;
         }         
         
         // We are going to want to trick Kaillera into starting our game, and sending the buffered information
         // we can do this by sending zeros. This is so our save data doesn't get trimmed off.
         // Kaillera should make good use of the cache at this point making the data transfer much faster.

         for (MessagePosition = 0; MessagePosition < 30;  MessagePosition++)
         {
             NetPlayInformation[0].KailleraMessageType = 0x00;
             NetPlayInformation[0].KailleraMessage = 0x00;
             KailleraModifyPlayValues(NetPlayInformation,5);
         }
         
         // Compress our save data, and get a buffer size.         
        
         SizeToSend = compress_save_read(pRomInfo->GoodName, InformationToSend);
    
         // Set up and send the information to other clients. We basically want to ensure this value gets sent over,
         // it's critical for sync.
         
         NetPlayInformation[0].KailleraMessageType = 0x05;

         NetPlayInformation[0].KailleraMessage = (unsigned long)SizeToSend;
     
         KailleraModifyPlayValues(NetPlayInformation,5);
        
         // Here we basically say, hey lets ensure EVERYONE is on the same page, 
         // everyone knows how much data is about to come in the form of 0x06
         // packets.
        for (MessagePosition = 0; MessagePosition < 30;  MessagePosition++)
        {
         
             NetPlayInformation[0].KailleraMessageType = 0x00;
             NetPlayInformation[0].KailleraMessage = 0x00;
             KailleraModifyPlayValues(NetPlayInformation,5);
             
        }

        // Now we start out loop to send the data over Kaillera, this is important.
        // It basically needs to happen or games wont sync...
                
        for (MessagePosition = 0; MessagePosition*4 < (SizeToSend - (SizeToSend % 4) + 4);MessagePosition++)
        {
         if (PlayerSlot == 0)
         {
             NetPlayInformation[0].KailleraMessageType = 0x06;
             int Loop;
            
             BYTE TempCharDWORD[4];
             
           
             for (Loop = MessagePosition; Loop < MessagePosition+4; Loop++)
             { 
     
                 if ( ((Loop-MessagePosition) + (MessagePosition*4)) < SizeToSend )
                    TempCharDWORD[Loop-MessagePosition] = InformationToSend[(Loop-MessagePosition) + (MessagePosition*4)];
                 else
                    TempCharDWORD[Loop-MessagePosition] = 0x00;
             }
            
             // Thanks SubDrag for the help, basically we are byte flipping our data and concating the bytes.
             // This little bugger gave me so much trouble.
            
             NetPlayInformation[0].KailleraMessage = ((((((TempCharDWORD[3] << 8)
             | TempCharDWORD[2]) << 8) | TempCharDWORD[1]) << 8) | TempCharDWORD[0]);
     
            
            }
            
            // All other players send zeros, which end up being cached.
            
            else { NetPlayInformation[0].KailleraMessage = 0x00; }
             
            KailleraModifyPlayValues(NetPlayInformation,5);

          }    
        
         // Once again we run this so we can catch up on our data.
        
         for (MessagePosition = 0; MessagePosition < 30;  MessagePosition++)
         {
             NetPlayInformation[0].KailleraMessageType = 0x00;
             NetPlayInformation[0].KailleraMessage = 0x00;
             KailleraModifyPlayValues(NetPlayInformation,5);
         }    
    
         // Now everyone uncompresses that save data, or should I just do other players after 1?
         
   
        if (uncompress_save_write(pRomInfo->GoodName, InformationToRecv, SizeToSend) == -1)
        {

               kailleraChatSend("Error uncompressing savedata! Desynch detected!");
               KailleraRunningGame = FALSE; 
               SaveDataPos=-1;
               kailleraEndGame();
             
        }
        else if( KailleraConfigOpen )
        {
             kailleraChatSend("Plugin configuration dialog opened! Desynch detected!");
             KailleraRunningGame = FALSE; 
             SaveDataPos=-1;
             kailleraEndGame(); 
        }
        else
        {
            
            
            KailleraGamePlayerCount = numplayers;
            
            
            if (KailleraGamePlayerCount > 4) { KailleraGamePlayerCount = 4; }
           
            
            KailleraRunningGame = TRUE; 
            SaveDataPos=-1;
            // This should actually allow us to have spectators when players count is greater than four.
         
            SetForegroundWindow( mainHWND) ;
            SetFocus( mainHWND) ;
            StartRom(pRomInfo->szFullFileName);
            ShowWindow(mainHWND, SW_SHOW) ;
            
           
            
     
        }
        
        // End the game here...
  
     
         return 0;
        }
        
    }
    kailleraEndGame();
    return 0;
}






/*
 =======================================================================================================================
 =======================================================================================================================
 */
void WINAPI kailleraChatReceivedCallback(char *nick, char *text)
{
	/* Do what you want with this :) */
	char TempString[MAX_PATH];

    sprintf(TempString,"<%s> %s",nick,text);
	SetStatusTranslatedString( hStatus, 0, TempString );
}


void WINAPI kailleraClientDroppedCallback(char *nick, int playernb)
{
	
	char TempString[MAX_PATH];

	sprintf(TempString,"%s (%d) dropped from current game!",nick,playernb);
	SetStatusTranslatedString( hStatus, 0, TempString );
    
    Controls[playernb-1].Present = FALSE;
    if (KailleraRunningGame && playernb == 1)
	{
          #ifdef __WIN32__
           SendMessage(mainHWND, WM_CLOSE, 0, 0);
          #endif
    }           
}



/*
 =======================================================================================================================
 =======================================================================================================================
 */



void buildRomList() {
    
    int i;
    int sort_column;
    char sort_method[10];
    ROM_INFO *pRomInfo;
    int itemsNum;
    LV_ITEM lvItem;
    memset(&lvItem, 0, sizeof(LV_ITEM));
    char *pszKailleraNamedRoms = szKailleraNamedRoms;
    *pszKailleraNamedRoms = '\0';
    
    sprintf( sort_method, Config.RomBrowserSortMethod) ;
    sort_column = Config.RomBrowserSortColumn ;
    
    sprintf(Config.RomBrowserSortMethod,"ASC");
    Config.RomBrowserSortColumn = 0 ;
    ListViewSort() ;
    
     
    itemsNum = ListView_GetItemCount(hRomList);
    for(i=0;i<itemsNum;i++)
    {
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = i;
        ListView_GetItem(hRomList,&lvItem);
        pRomInfo = &ItemList.List[lvItem.lParam];

            strncpy(pszKailleraNamedRoms, pRomInfo->GoodName, strlen(pRomInfo->GoodName) + 1);
            pszKailleraNamedRoms += strlen(pRomInfo->GoodName) + 1;
          
    } 
    strncpy(pszKailleraNamedRoms, "*Away (leave messages)", strlen("*Away (leave messages)")+1);  
    pszKailleraNamedRoms += strlen("*Away (leave messages)")+1;    
    strncpy(pszKailleraNamedRoms, "*Chat (not game)", strlen("*Chat (not game)")+1);  
    pszKailleraNamedRoms += strlen("*Chat (not game)")+1;    
    *(++pszKailleraNamedRoms) = '\0';
    
    sprintf( Config.RomBrowserSortMethod, sort_method ) ;
    Config.RomBrowserSortColumn = sort_column ;
    ListViewSort() ;
}

void KailleraPlay(void)
{

    // This is when Kaillera is loaded into the memory, and the thread has started.
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	kailleraInfos	kInfos;
    HWND emuHWND;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/* build roms list :) */
	
	buildRomList();

	kInfos.appName = MUPEN_VERSION;
	kInfos.gameList = szKailleraNamedRoms;
	kInfos.gameCallback = kailleraGameCallback;
	kInfos.chatReceivedCallback = kailleraChatReceivedCallback;
	kInfos.clientDroppedCallback = kailleraClientDroppedCallback;
	kInfos.moreInfosCallback = kailleraMoreInfosCallback;


	kailleraInit();
	
	kailleraSetInfos(&kInfos);
    kailleraSelectServerDialog(0);
 	


  

}

