/*
  Kaillera client API v0.84
  (c) 2001 Christophe Thibault

  History:
  0.84: added moreInfos callback
  0.83: added clientDropped callback
  0.8 : added in-game chat function/callback
  0.72: added kailleraEndGame() method
        added kailleraGetVersion() method
  0.7:  modified kailleraModifyPlayValues behaviour
*/

#ifndef KAILLERA_CLIENT_H
#define KAILLERA_CLIENT_H

#include <windows.h>
#include "../../main/plugin.h"

#define KAILLERA_CLIENT_API_VERSION "0.8"



  typedef struct {
    char *appName;
    char *gameList;

    int (WINAPI *gameCallback)(char *game, int player, int numplayers);

    void (WINAPI *chatReceivedCallback)(char *nick, char *text);
    void (WINAPI *clientDroppedCallback)(char *nick, int playernb);

    void (WINAPI *moreInfosCallback)(char *gamename);
  } kailleraInfos;

  void (__stdcall* kailleraGetVersion) (char *version);

  void (__stdcall* kailleraInit) ();

  void (__stdcall* kailleraShutdown) ();

  void (__stdcall* kailleraSetInfos) (kailleraInfos *infos);

  void (__stdcall* kailleraSelectServerDialog) (HWND parent);

 
  int (__stdcall* kailleraModifyPlayValues)  (void *values, int size);
  int KailleraModifyPlayValues(void *values, int size);

  void (__stdcall* kailleraChatSend)  (char *text);

  void (__stdcall* kailleraEndGame) ();

  int WINAPI kailleraGameCallback(char *game, int player, int numplayers);
  void WINAPI kailleraChatReceivedCallback(char *nick, char *text);
  void WINAPI kailleraClientDroppedCallback(char *nick, int playernb);
  void WINAPI kailleraMoreInfosCallback(char *gamename);
  
  /// Emulator functions 
  int LoadKaillera(char *ClientType);
  int kaillera_my_player();
  void EndGameKaillera();
  void CloseKaillera();

  void EndGameKaillera();
  void KailleraPlay(void);
 

  DWORD WINAPI LaunchKaillera(LPVOID lpParam);



  char *k_game;
  
  extern char *KailleraSaveBuffer;
  
  // Please remember to keep the rest of this organized
  
  
  /*================================
   Message Structure 
  ================================*/
   
 /*
 
 Each packet should be forumlated like this:
        
   [BYTE MessageType] [DWORD Data]
        
 Types of Messages:
         00 : NO DATA
         01 : KEY DATA
         02 : RESET (NO DATA)
         03 : PAUSE (NO DATA)
         04 : UNPAUSE (NO DATA)
         05 : SAVE DATA LENGTH
         06 : SAVE DATA
  */
  
  #pragma pack(push, 1)
  typedef struct
  {
     BYTE KailleraMessageType;
     DWORD KailleraMessage;
  } KailleraPacket;
  #pragma pack(pop)
  
  KailleraPacket NetPlayInformation[8];
  KailleraPacket KeyInformation[8];
  KailleraPacket NetPlayInformationRecv[8];
  
  char *TargetKailleraClient;
  char *KailleraGameName;
  char KailleraChatMessage[256];
  
  BOOL KailleraOpen;
  BOOL KailleraRunningGame;
  BOOL KailleraGamePaused;
  BOOL KailleraChatEnabled;
  BOOL KailleraSendKey;
  BOOL StartSendingStuff;
  BOOL KailleraDontUseSave;
  
  BOOL KailleraConfigOpen;
  BOOL KailleraControl[4];
  
  int KailleraGameMyPlayer;
  int KCount;
  int KailleraSleepGame;
  int KailleraGamePlayerCount;

  HWND KailleraCPHWND;
  BYTE KCPToggles[4];
  BYTE KCPSlot[8];
  BYTE KCPDummySlot[8];
  int  KCPSwapTarget;
  int  KCPSwapDest;
  // KCPSlot is the array that defines what controller is defined to what player, defualt is (0,0) (1,1) (2,2) (3,3)
  
  extern void KailleraChatKeyDown( WPARAM wParam, LPARAM lParam );
  extern void KailleraGameChatSend();
  
#endif
