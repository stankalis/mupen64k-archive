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

int   SizeToSend;
int   SaveDataPos = -1;

BYTE InformationToSend[0x20000];
BYTE HexToSend[0x20000];
BYTE InformationToRecv[0x20000];
         
    
DWORD BufferKeys[4];
DWORD TempKeys[4];
DWORD FileSize[4];
/*

char KailleraChatKeyDown( WPARAM wParam, LPARAM lParam )
{
       // Number keys do not work!
       
       TCHAR strTo;
       char temp_buffer[MAX_PATH];     
           
       strTo = (TCHAR) wParam;

       switch (wParam)
       {
              
       case VK_BACK:
          
           chat_buffer[strlen(chat_buffer)-1] = NULL;
           SetStatusTranslatedString( hStatus, 1, chat_buffer );     
           break;
       case VK_ESCAPE:
           *chat_buffer = NULL;
           KChatEnable = FALSE;
           break;
       case VK_RETURN:
           break;                  
       default:

           sprintf(chat_buffer,"%s%c",chat_buffer,strTo) ;
           SetStatusTranslatedString( hStatus, 1, chat_buffer );
       
         break;  
    }          
    
          
    return chat_char;    
}

int k_ChatSend()
{
    kailleraChatSend(chat_buffer); *chat_buffer = NULL; KChatEnable = FALSE; return 1; // send stuff }
}



int Kaillera_Game()
{
     return Kaillera_Running;
}

int Kaillera_Players()
{
    return k_players;
}
*/
BYTE CharToHex(char c)
{
   if(isdigit(c))
      return(c - '0');
   else
      return(toupper(c) - 'A' + 0x0a);
}

char StrToHex(BYTE *b, char *str, int len)
{
   for(; len>0; len--)
      if(isxdigit(*str) && isxdigit(*(str+1)))
         *(b++) = ((CharToHex(*(str++))) << 4) | (CharToHex(*(str++)));
      else
         return(1);
   return(0);
}
//---------------------------------------------------------------------------



DWORD WINAPI LaunchKaillera(LPVOID lpParam)
{
       ShowInfo("Kaillera : Loaded");
       KailleraOpen = TRUE;
       char testhex[1];
       StrToHex(testhex,"4E",2);
       ShowInfo("%c",testhex[0]);
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
       int MessageCount = 0;
       
       while (!KailleraRunningGame && MessageCount < 60)
       {
             NetPlayInformation[0].KailleraMessageType = 0x00;
             NetPlayInformation[0].KailleraMessage = 0x00;
             KailleraModifyPlayValues(NetPlayInformation,5);
             MessageCount++;
             sleep(16);             
       }
       for (;MessageCount < 60; MessageCount++)
       {
             NetPlayInformation[0].KailleraMessageType = 0x00;
             NetPlayInformation[0].KailleraMessage = 0x00;
             KailleraModifyPlayValues(NetPlayInformation,5);
       }
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
    int Loop;
    
    char *SaveDataTemp;
    
    BufferSize = kailleraModifyPlayValues((void *)values,size);
   
    *NetPlayInformationRecv = *((KailleraPacket*)values);
    if (BufferSize == -1)
    {
       return -1;
    } // Connection ended.
    if (BufferSize == 0)
    {           
       return 0;
    } // Buffering.
    else
    {
        
        // Question: Do we want other users to have some form of control, or just player 1?
        switch (NetPlayInformationRecv[0].KailleraMessageType)
        {

               case 0x00: // No data
               //ShowInfo("00");
               break;
               case 0x01: // Key data
              // ShowInfo("01");
               break;
               case 0x02: // Reset (incomplete)
                    break;
               case 0x03: // Pause game (broken/incomplete)
                    KailleraGamePaused = TRUE;
                    pauseEmu();
                    // CreateThread(NULL, 0, KailleraKeepAlive, NULL, 0, 0);
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
               
                    SaveDataTemp = NetPlayInformationRecv[0].KailleraMessage;
                    ShowInfo("dword recv: %c",((char *)SaveDataTemp)[0]);
                         
         
                   for (Loop = 0; Loop < 4; Loop++)
                   {
                       
                         SaveDataPos++;
                         if (SaveDataPos > SizeToSend)
                         {
                           ShowInfo("too much info:: %i > %i",SaveDataPos,SizeToSend);
                         }
                         else
                         {
                             InformationToRecv[SaveDataPos] = ((char *)NetPlayInformationRecv[0].KailleraMessage)[Loop];
                         
                           //  ShowInfo("this: %08X vs real: %08X",InformationToRecv[SaveDataPos], InformationToSend[SaveDataPos]);
                         }  

                       
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
    //   Kaillera_Running = 0;
       kailleraEndGame();
    //   droptwice = TRUE;
    }   
}
void OutputHex(char * outb, void *inb, int len, int res1, BOOL usespace){ 
     char HEXDIGITS [] = { 
          '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' 
     }; 
     char * xx = (char*)inb;
     int x,dx,hib,lob; 
     for (x = 0; x < len; x++) { 
          dx = *xx++; 
          hib = (dx & 0xF0)>>4; 
          lob = (dx & 0x0F); 
          *outb++ = HEXDIGITS[hib]; 
          *outb++ = HEXDIGITS[lob]; 
          if (usespace) 
               *outb++ = ' '; 
     } 
     *outb = 0; 
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

/// Used Parts of 1964 Kaillera code as referece,not finished
char path_buffer[_MAX_PATH];
int WINAPI kailleraGameCallback(char *game, int player, int numplayers)
{

    int i;
    
    DWORD DummyBuffer[3];
    KailleraGameName = game;
    extern char *KailleraSaveBuffer;
//  k_players = numplayers;
    
    int PlayerSlot = player - 1;
//    ShowInfo("Kaillera : player = %i",my_player + 1);
    char kaillera_info[_MAX_PATH];

    ROM_INFO *pRomInfo;
      KailleraAcceptChatInfo = TRUE;
   ShowInfo("Kaillera : kailleraGameCallback()");
   for (i=0;i<ItemList.ListCount;i++)
   {
     pRomInfo = &ItemList.List[i];
    
     if (strcmp(pRomInfo->GoodName, game) == 0) {
//         k_game = pRomInfo->GoodName;
         
         extern rom_settings ROM_SETTINGS;
         memset(ROM_SETTINGS.MD5,pRomInfo->MD5,33);
         //ROM_SETTINGS->goodname = pRomInfo->GoodName;
         
         ShowInfo("Kaillera : Starting rom %s",pRomInfo->GoodName);
    
         int   MessagePosition;
         BYTE  KailleraDelay[4];
         DWORD SizeOfMessage[4];
         DWORD SaveMessage[4];
         char ChatInformation[128];
         
         // Once again we run this so we can catch up on our data.
       
       for (MessagePosition = 0; MessagePosition < 120;  MessagePosition++)
       {
             NetPlayInformation[0].KailleraMessageType = 0x00;
             NetPlayInformation[0].KailleraMessage = 0x00;
             KailleraModifyPlayValues(NetPlayInformation,5);
       }        
       CreateThread(NULL, 0, KailleraKeepAlive, NULL, 0, 0);
    
         sprintf(ChatInformation,"mynum::%i",PlayerSlot);
         kailleraChatSend(ChatInformation);
         free(ChatInformation);
         
         // Compress our save data, and get a buffer size.         
         if (PlayerSlot == 0)
         {
        
             SizeToSend = compress_save_read(pRomInfo->GoodName, InformationToSend);
             OutputHex(HexToSend,InformationToSend,SizeToSend,0,FALSE);
             sprintf(ChatInformation,"sdlen::%i",strlen(HexToSend));
             kailleraChatSend(ChatInformation);
             free(ChatInformation);
         }
             
    
          if (PlayerSlot == 0)
         {
              for (MessagePosition = 0; MessagePosition*100 < (strlen(HexToSend) - (strlen(HexToSend) % 100) + 100);MessagePosition++)
              {
      
             
             int Loop;
            
             unsigned char TempCharDWORD[128];
             for (Loop = 0; Loop < 128; Loop++)
             {
                 TempCharDWORD[Loop] = 0x00;
             }
             memcpy(TempCharDWORD,"sdmsg::",7);
           
             for (Loop = MessagePosition; Loop < MessagePosition+100; Loop++)
             { 
     
                 if (((Loop-MessagePosition) + (MessagePosition*100)) < strlen(HexToSend))
                 {
                                             
                   TempCharDWORD[7+Loop-MessagePosition] = HexToSend[(Loop-MessagePosition) + (MessagePosition*100)];

                       // ShowInfo("messy deubg: %i info: %i",(Loop-MessagePosition) + (MessagePosition*96),InformationToSend[(Loop-MessagePosition) + (MessagePosition*96)] ); 
                 }  
             }
            
            
             
             //ShowInfo("chunk out: %s",TempCharDWORD); 
             kailleraChatSend(TempCharDWORD);
             free(TempCharDWORD);
            }
           
    
          }    
        if (PlayerSlot == 0)
        {
         kailleraChatSend("!!sduncompress");
        }
        KailleraRunningGame = TRUE; 
        
        KailleraGamePlayerCount = numplayers;
        
        if (KailleraGamePlayerCount > 4) { KailleraGamePlayerCount = 4; }
        // This should actually allow us to have spectators when players count is greater than four.
 
        
        ShowInfo("%i",KailleraGamePlayerCount); 
        StartRom(pRomInfo->szFullFileName);
        
        SetForegroundWindow(mainHWND);
        SetFocus(mainHWND);


              
         
         // Should I place a check for a -1 value?
         /*
         player 1 buffers 3
         player 2 buffers 2
         player 2 recieves player 1's 3rd 0, but knows delay cant be 0 so discards it?
         */
         // kailleraModifyPlayValues(KailleraDelay,1);
         // Now we should have the delays... 
         
         
        
        // ShowInfo("%s",InformationToSend);
         // We start our protocol to send the save data over Kaillera.
       
        /* SizeOfMessage[0] = (DWORD)SizeToSend;
         ShowInfo("%i",(int)SizeOfMessage[0]);
        // K_KMPV(SizeOfMessage,4);
         ShowInfo("%i",(int)SizeOfMessage[0]);
         if (PlayerSlot == 0) {  sprintf(kaillera_info,"@SIZETOSEND = %i",(int)SizeOfMessage[0]); }
         else {  sprintf(kaillera_info,"@SIZETORECV = %i",(int)SizeOfMessage[0]); }
         kailleraChatSend(kaillera_info);
         while (K_KMPV(SaveMessage,1)) { ; }
         //InformationToRecv = malloc((int)SizeOfMessage[0]);
         /*for (MessagePosition = 0;MessagePosition < (int)SizeOfMessage[0]; MessagePosition++)
         {
   
             char DummyChar;
             
             if (PlayerSlot == 0)
             {
                        DummyChar = InformationToSend[MessagePosition]; 
                        SaveMessage[0] = (BYTE)DummyChar; 
             }
             else { SaveMessage[0] = 0xFF; } 
             // SaveMessage[0] = 0xFF; 
             kailleraModifyPlayValues(SaveMessage,1);
             InformationToRecv[MessagePosition] = (char)SaveMessage[0];
         }
         uncompress_save_write(pRomInfo->GoodName, &InformationToRecv,(int)SizeOfMessage[0]);
         SaveMessage[0] = 0x00;*/
        // kailleraEndGame();
         //SetForegroundWindow(mainHWND) ;
         //SetFocus(mainHWND) ;
         
         // Need to clean up data before we resend it.
         
      
         //  ShowMessage(KailleraSaveBuffer);
 //        kaillera_write_savedata(pRomInfo->GoodName); 
//        Kaillera_Running = 1;
         //
         
         // sprintf(kaillera_info,"@INI => %08X-%08X-C%02X",(int)pRomInfo->CRC1,(int)pRomInfo->CRC2,pRomInfo->Country);
         // kailleraChatSend(kaillera_info);
         //DummyBuffer[0] = sizeof(KailleraSaveBuffer);
         //sprintf(kaillera_info,"%i",sizeof(KailleraSaveBuffer));
        // kailleraChatSend(kaillera_info);
         // ShowMessage(sizeof(KailleraSaveBuffer));
         
       /* if (kailleraModifyPlayValues((void *)DummyBuffer,4) > 0)
         {
                next++;
                kailleraChatSend(DummyBuffer[0]);
                InformationToRecv[next] = DummyBuffer[0];
             }
         } */
         
         // kailleraChatSend(InformationToRecv);
         // kaillera_write_savedata(pRomInfo->GoodName,InformationToRecv); 
//         droptwice = FALSE;*/
         //SetForegroundWindow(mainHWND) ;
         //SetFocus(mainHWND) ;
       
         // End the game here...
         return 0;
        }
        
    }
    kailleraEndGame();
    return 0;
}

/* int kaillera_my_player()
{
    return my_player;
} */




/*
 =======================================================================================================================
 =======================================================================================================================
 */
void WINAPI kailleraChatReceivedCallback(char *nick, char *text)
{
    // 
	/* Do what you want with this :) */
	char TempString[MAX_PATH];
	int  Loop;
    if (KailleraAcceptChatInfo)
    {
       char * KailleraChatMessage;
       KailleraChatMessage = strchr ( text, ':' ) + 2;
       
       // ShowInfo("kcmess: %s", KailleraChatMessage);
       
       if (strncmp("mynum::",text,7) == 0)
       {
           ShowInfo("%s is playerslot %i", nick, atoi(KailleraChatMessage));
       }
       if (strncmp("sdlen::",text,7) == 0)
       {
           SizeToSend = atoi(KailleraChatMessage);
           ShowInfo("%i is sizetosend", SizeToSend);
       }
       if (strncmp("sdmsg::",text,7) == 0)
       {
                                   
           strcat(InformationToRecv,KailleraChatMessage);
           ShowInfo("chunk in: %s and len is %i",KailleraChatMessage, strlen(InformationToRecv));
           
       }
       if (strncmp("!!sduncompress",text,14) == 0)
       {
          
   
              ShowInfo("compressing w00t!");      
              
             // SizeToSend=strlen(InformationToRecv);
              StrToHex(InformationToSend,InformationToRecv,SizeToSend);
              uncompress_save_write(KailleraGameName, InformationToSend, (SizeToSend/2));
    
       }
    }        
    
    sprintf(TempString,"<%s> %s",nick,text);
	SetStatusTranslatedString( hStatus, 1, TempString );
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void WINAPI kailleraClientDroppedCallback(char *nick, int playernb)
{
	
	char TempString[MAX_PATH];

	sprintf(TempString,"%s (%d) dropped from current game!",nick,playernb);
	SetStatusTranslatedString( hStatus, 1, TempString );
    if (1) //	if (playernb == 1 && kaillera_my_player() != 0 && Kaillera_Game() == 1)
	{
          
         //  dropGame();
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
    extern HWND emuHWND;
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
