#pragma once
#include <iostream>
#include <windows.h>
#include <stdio.h> 
#include <string.h>
#include <memory.h>
#include <ctype.h >
#include <io.h>
#include <conio.h>
#include <fcntl.h>
#include <sys/utime.h> //__utimbuf64
#pragma comment(lib, "User32.lib") //ShowCursor

#define OK     0
#define NG    -1

#define FIL    0
#define DIR    1

#define DBL 0x22
#define SPC 0x20
#define CMM 0x2C
#define TAB 0x09
#define CR  0x0D
#define LF  0x0A
#define CM  0x2C

//Ãß≤Ÿµ∞Ãﬂ›
#define RD_OPEN(X) _open(X,_O_RDONLY|_O_BINARY)
#define WR_OPEN(X) _open(X,_O_WRONLY|_O_CREAT|_O_BINARY|_O_TRUNC ,_S_IREAD|_S_IWRITE)
#define EX_OPEN(X) _open(X,_O_WRONLY|_O_CREAT|_O_BINARY|_O_APPEND,_S_IREAD|_S_IWRITE)
#define MEGA       1000000

//√ﬁ®⁄∏ƒÿ¡™≤›
typedef struct dir_tag {
	struct dir_tag* next;
	char* data;
} DirQue;

//Ãﬂ€ƒ¿≤ÃﬂêÈåæ
bool KanjiSecond(char* buf, char* chr);
void MakeDir(const char* path);
bool FileCopy(char* iFile, char* oFile, char* buf, long long size);
char *GetFileName(char* path);
char *AddCma(__int64 num);
void ScrWrite(int line, int column, const char* str);
void ClsLine(int line, int column, int len);