#ifdef USE_GTK
#include <gtk/gtk.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <unistd.h>
#include "resource.h"
#include <sys/time.h>

#include <string.h>
#include <limits.h>
#ifdef __WIN32__
#include <windows.h>
#else
#include "../main/winlnxdefs.h"
#endif
#include "Cheat_#1.0.h"

typedef struct {
        unsigned long address;
        unsigned short value;
        char type;
        
        } cheat_code;

long fsize;
int numon;
HWND form1;

static CHEAT_INFO mycheatinfo;
cheat_code cheat_list[1024];


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

EXPORT void CALL CloseDLL( void )
{
          int x;
          for (x = 0; x < 1024; x++)
          {
              cheat_list[x].address = 0;
              cheat_list[x].type = 0;
              cheat_list[x].value = 0;
//              cheat_list[x].toggle = 0;
          }
          return;
}

EXPORT void CALL DllAbout( HWND hParent )
{
#ifdef USE_GTK
   char tMsg[256];
   GtkWidget *dialog, *label, *okay_button;
   
   dialog = gtk_dialog_new();
   sprintf(tMsg,"Mupen64 Gameshark by okaygo");
   label = gtk_label_new(tMsg);
   okay_button = gtk_button_new_with_label("OK");
   
   gtk_signal_connect_object(GTK_OBJECT(okay_button), "clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy),
			     GTK_OBJECT(dialog));
   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),
		     okay_button);
   
   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),
		     label);
   gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
   gtk_widget_show_all(dialog);
#else
   char tMsg[256];
   sprintf(tMsg,"Mupen64 Gameshark by okaygo");
   fprintf(stderr, "About\n%s\n", tMsg);
#endif
}

static void GetPluginDir(char *Directory)
{
   int n = -1; // readlink("/proc/self/exe", Directory, PATH_MAX);
   if (n == -1)
     strcpy(Directory, "./");
   else
     {
	Directory[n] = '\0';
	while(Directory[strlen(Directory)-1] != '/')
	  Directory[strlen(Directory)-1] = '\0';
     }
   strcat(Directory, "plugins/");
}


LRESULT CALLBACK CheatConfigProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
 /*   char country [50];
    md5_byte_t digest[16];
    ROM_INFO *pRomInfo;
    char tempname[100];
    HWND hwndPB;    
    pRomInfo = getSelectedRom();
    DEFAULT_ROM_SETTINGS TempRomSettings;
    
    if (pRomInfo==NULL) { 
       EndDialog(hwnd, IDOK);
       return FALSE; 
      }
    switch(Message)
    {
        case WM_INITDIALOG:
             romInfoHWND = hwnd;
             
             rewind_plugin();           
             while(get_plugin_type() != -1) {
                switch (get_plugin_type())
                {
                case PLUGIN_TYPE_GFX:
                    SendDlgItemMessage(hwnd, IDC_COMBO_GFX, CB_ADDSTRING, 0, (LPARAM)next_plugin());
                    break;
                case PLUGIN_TYPE_CONTROLLER:
                    SendDlgItemMessage(hwnd, IDC_COMBO_INPUT, CB_ADDSTRING, 0, (LPARAM)next_plugin());
                    break;
                case PLUGIN_TYPE_AUDIO:
                    SendDlgItemMessage(hwnd, IDC_COMBO_SOUND, CB_ADDSTRING, 0, (LPARAM)next_plugin());
                    break;
                case PLUGIN_TYPE_RSP:
                    SendDlgItemMessage(hwnd, IDC_COMBO_RSP, CB_ADDSTRING, 0, (LPARAM)next_plugin());
                    break;                                
                default:
                    next_plugin();
                }
             }
             
             TempRomSettings = GetDefaultRomSettings( pRomInfo->InternalName) ;
             
             WriteComboBoxValue( hwnd, IDC_COMBO_GFX, TempRomSettings.GfxPluginName, gfx_name);
             WriteComboBoxValue( hwnd, IDC_COMBO_INPUT, TempRomSettings.InputPluginName, input_name);
             WriteComboBoxValue( hwnd, IDC_COMBO_SOUND, TempRomSettings.SoundPluginName, sound_name);
             WriteComboBoxValue( hwnd, IDC_COMBO_RSP, TempRomSettings.RspPluginName, rsp_name);
             
             if (Config.OverwritePluginSettings) {
                 EnableWindow( GetDlgItem(hwnd,IDC_COMBO_GFX), FALSE );
                 EnableWindow( GetDlgItem(hwnd,IDC_COMBO_INPUT), FALSE );
                 EnableWindow( GetDlgItem(hwnd,IDC_COMBO_SOUND), FALSE );
                 EnableWindow( GetDlgItem(hwnd,IDC_COMBO_RSP), FALSE );         
             }
             
             //Disables the button because of a bug in the emulator:
             //Sound gets distorted if you push the button while ingame
             //Hack: you should check this
             if (emu_launched) EnableWindow( GetDlgItem(hwnd,IDC_MD5_CALCULATE), FALSE );             
             
             SetDlgItemText(hwnd,IDC_ROM_FULLPATH,pRomInfo->szFullFileName);
             SetDlgItemText(hwnd, IDC_ROM_GOODNAME, pRomInfo->GoodName);
             SetDlgItemText(hwnd,IDC_ROM_INTERNAL_NAME,pRomInfo->InternalName);
             sprintf(TempMessage,"%.1f MBit",(float)pRomInfo->RomSize/0x20000); 
             SetDlgItemText(hwnd, IDC_ROM_SIZE, TempMessage); 
             CountryCodeToCountryName(pRomInfo->Country,country);
             SetDlgItemText(hwnd, IDC_ROM_COUNTRY, country);
             sprintf(TempMessage,"%08X-%08X-C%02X",(int)pRomInfo->CRC1,(int)pRomInfo->CRC2,pRomInfo->Country);
             SetDlgItemText(hwnd, IDC_ROM_INICODE, TempMessage);
             SetDlgItemText(hwnd, IDC_ROM_MD5, pRomInfo->MD5);
             getIniComments(pRomInfo,TempMessage);
             SetDlgItemText(hwnd, IDC_INI_COMMENTS, TempMessage);
             TranslateRomInfoDialog(hwnd);
             return FALSE;
        case WM_CLOSE:
              EndDialog(hwnd, IDOK);
              romInfoHWND = NULL;   
        break; 
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_OK:
                    GetDlgItemText(hwnd, IDC_INI_COMMENTS, (LPSTR) TempMessage, 128 );
                    setIniComments(pRomInfo,TempMessage);
                    strncpy(pRomInfo->UserNotes,TempMessage,sizeof(pRomInfo->UserNotes));
                    if (!emu_launched) {                    //Refreshes the ROM Browser
                        ShowWindow( hRomList, FALSE ); 
                        ShowWindow( hRomList, TRUE );
                    }
                    EndDialog(hwnd, IDOK);
                    romInfoHWND = NULL; 
                break;
                case IDC_CANCEL:
                    if (!emu_launched) {
                        ShowWindow( hRomList, FALSE ); 
                        ShowWindow( hRomList, TRUE );
                    }
                    EndDialog(hwnd, IDOK);
                    romInfoHWND = NULL;
                break;
                case IDC_MD5_CALCULATE:
                    hwndPB = GetDlgItem( hwnd, IDC_MD5_PROGRESS_BAR );
                    SendMessage( hwndPB, PBM_SETRANGE, 0, MAKELPARAM(0, 100) ); 
//                    ShowWindow( GetDlgItem(hwnd, IDC_MD5_CALCULATE), FALSE );  //Hides "Calculate" Button
                    ShowWindow( hwndPB, TRUE );                                //Shows Progress bar
                                        
                    calculateGlobalStop = 0;
                    calculateMD5(pRomInfo->szFullFileName,digest);
                    MD5toString(digest,TempMessage);
                    SetDlgItemText(hwnd, IDC_ROM_MD5, TempMessage);
                    getIniGoodNameByMD5(TempMessage,tempname);
                    SetDlgItemText(hwnd, IDC_ROM_GOODNAME, tempname);
                    saveMD5toCache(TempMessage);
 
                    ShowWindow( hwndPB, FALSE );                    //Hides Progress bar
                    ShowWindow( GetDlgItem(hwnd, IDC_MD5_CALCULATE), TRUE );  //Shows "Calculate" Button
                break;
                case IDC_SAVE_PROFILE:
                    ReadComboBoxValue( hwnd, IDC_COMBO_GFX,   TempRomSettings.GfxPluginName) ;
                    ReadComboBoxValue( hwnd, IDC_COMBO_INPUT, TempRomSettings.InputPluginName) ;
                    ReadComboBoxValue( hwnd, IDC_COMBO_SOUND, TempRomSettings.SoundPluginName) ;
                    ReadComboBoxValue( hwnd, IDC_COMBO_RSP,   TempRomSettings.RspPluginName) ; 
                    saveDefaultRomSettings( pRomInfo->InternalName, TempRomSettings);         
                break;
            }
        break;
    }
    return FALSE;*/
}

EXPORT void CALL DllConfig ( HWND hParent )
{
              	MSG msg;//Used to get messages from the thread's message queue
		WNDCLASSEX wcex; //Used to register window class information
		HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
	

		wcex.cbSize         = sizeof(WNDCLASSEX);
		wcex.style          =  CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc    = (WNDPROC)CheatConfigProc;
		wcex.cbClsExtra     = 0;
		wcex.cbWndExtra     = 0;
		wcex.hInstance      = hInstance;
		wcex.hIcon          = LoadIcon(hInstance, IDI_APPLICATION);
		wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground  = (HBRUSH)COLOR_BTNFACE + 2;
		wcex.lpszMenuName   = NULL;
		wcex.lpszClassName  = "Gameshark";
		wcex.hIconSm        = NULL;

		//Class must be registered with the system before we can create the window
		RegisterClassEx(&wcex);
//		LoadLibrary("Riched32.dll");
 	  //  InitCommonControlsEx(0);


		//Create Child Window
		form1 = CreateWindow("Gameshark", "Gameshark",  WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION | WS_VISIBLE, 0, 0,, 156, NULL, NULL, hInstance, NULL);
         
		//kailleraInit();
		//createChatroom();
		//createInitialWindow();


		//Adjust Parent Window
		EnableWindow(hParent, FALSE);
	//	ShowWindow(hParent, SW_HIDE);


		//Show GUI
		ShowWindow(form1, SW_SHOWNORMAL);
		UpdateWindow(form1);       
    DialogBox(GetModuleHandle(NULL), 
                     MAKEINTRESOURCE(IDD_CHEATMENU), form1, 0);
}

EXPORT BOOL CALL InitiateCheat (CHEAT_INFO CheatInfo)
{
       mycheatinfo = CheatInfo;
       char address_string[8];
       char value_string[5];
        int x;
        
       FILE *f = fopen("./gameshark.cfg", "rb");
	   if (f)
 	   {
          fseek (f , 0 , SEEK_END);
          fsize = ftell (f);
          fsize -= 1;
          rewind (f);
          for (x=0;x*9<(fsize - (fsize%9)) / 9;x++)
          {
    		  fread(address_string, 1, 8, f);
    		  fread(value_string,1,5,f);
    		  address_string[0] = '0';
    		  address_string[1] = '0';
    		  value_string[0] = '0';
    		  sscanf(address_string,"%X",&cheat_list[x].address);
    		  sscanf(value_string,"%X",&cheat_list[x].value);
    		  numon++;
           }

          fclose(f);
	   }
	   else { f = fopen("./gameshark.cfg", "wb"); fclose(f); }
       return TRUE;
}

EXPORT void CALL DllTest ( HWND hParent )
{
        
}

EXPORT void CALL GetDllInfo( PLUGIN_INFO * PluginInfo )
{
   PluginInfo->Version = 0x0100;
   PluginInfo->Type    = PLUGIN_TYPE_CHEAT;
   sprintf(PluginInfo->Name,"Mupen64 Gameshark");
   PluginInfo->NormalMemory  = FALSE;
   PluginInfo->MemoryBswaped = FALSE;

}

EXPORT void CALL ApplyCheatsOnVI ( void )
{
   //    ((unsigned char*)mycheatinfo.RDRAM)[0x99110^S8] = 0xFFFF;
   //    ((unsigned char*)mycheatinfo.RDRAM)[0x99112^S8] = 0xFFFF;
//   char *cheat = "80099110 FFFF";
  //char *cheat2 = "80099112 FFFF";
//   char delims = " ";
//   char result[30];
 //  BYTE addy[8];
//   BYTE *value;
      int x;
   //result = strtok( cheat, delims );
  // if (strlen(result[0]) == 8 && (strlen(result[1]) == 4))
   //{

          // result[0] = "0";
          // result[1] = "0";
           
  //         unsigned long int test;
           //unsigned long int tesa = 626962;
    //       unsigned short values;
           
       //    sscanf("099110","%X",&test);
        
         
           
//           result = strtok( cheat, delims );
      //     StrToHex((BYTE *)values,result,4);


           
          // double unsigned long text = 626962;
          for (x=0;x<numon;x++)
          {
           mycheatinfo.RDRAM[cheat_list[x].address^S8] = cheat_list[x].value;
          }
          // mycheatinfo.RDRAM[tesa^S8] = value;
         //  mycheatinfo.RDRAM[text^S8] = 0xFFFF;
       
   
  // mycheatinfo.RDRAM[0x99110^S8] = 0xFFFF;
  // mycheatinfo.RDRAM[0x99112^S8] = 0xFFFF;
       
  // }
  // mycheatinfo.RDRAM[0x99110^S8] = 0xFFFF;
  // mycheatinfo.RDRAM[0x99112^S8] = 0xFFFF;
   return;
}

EXPORT void CALL RomOpen( void )
{
       return;
}

EXPORT void CALL RomClosed( void )
{
        return;
}
