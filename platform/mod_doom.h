#include <windows.h>
#include <stdio.h>

void MiniDoomIteration(char *p_imgBuf, int x, int y, int imgWidth, int imgHeight);
void MiniDoomKeyDown(int vKey);
void MiniDoomKeyUp(int vKey);
void MiniDoomStart(const char* homeDir, const char* fileName);
int MiniDoomKeyTranslate(int key);
