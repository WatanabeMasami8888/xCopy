#include "COMMON.H"
void xxCopy(char* iPath,char *oPath,char *buf,long long size);
char DosMes[MAX_PATH * 2];
int  CopyCount = 0;

int main(int argc,char *argv[]) {
	struct _stat64 ist;
	struct _stat64 ost;
	char iPath[_MAX_PATH*2],oPath[_MAX_PATH*2];
	int  iMode,oMode;
	int  ix,rcd;

	//引数の検査
	if (argc != 3) {
		printf("Usage  : XXCOPY FROM_DIRECTORY TO_DIRECTORY \n");
		printf("Example: XXCOPY C:\\PICTURE D:\\PICTURE \n");
		return(NG);
	}
	//入力ﾌｫﾙﾀﾞ検査
	strcpy_s(iPath,sizeof(iPath),argv[1]);
	if (strlen(iPath) == 2 && isalpha((unsigned char)iPath[0]) && iPath[1] == ':') {
		strcat_s(iPath, sizeof(iPath), "\\");
	}
	if ((rcd =_stat64(iPath,&ist)) != 0) {
		if (rcd == ENOENT) {
			printf("FROM_DIRECTORY[%s] DOES NOT EXIST.\n",iPath);     return(NG);
		} else {
			printf("FROM_DIRECTORY[%s] CANNOT BE ACCESSED.\n",iPath); return(NG);
		}
	}
	if (ist.st_mode & _S_IFDIR) { iMode = DIR; } else { iMode = FIL; }

	//出力ﾌｫﾙﾀﾞ検査
	strcpy_s(oPath, sizeof(oPath), argv[2]);
	if (strlen(oPath) == 2 && isalpha((unsigned char)oPath[0]) && oPath[1] == ':') {
		strcat_s(oPath, sizeof(oPath), "\\");
	}
	MakeDir(oPath);
	if ((rcd = _stat64(oPath, &ost)) != 0) {
		if (rcd == ENOENT) {
			printf("TO_DIRECTORY[%s] DOES NOT EXIST.\n",oPath);     return(NG);
		} else {
			printf("TO_DIRECTORY[%s] CANNOT BE ACCESSED.\n",oPath); return(NG);
		}
	}
	if (ost.st_mode & _S_IFDIR) { oMode = DIR; } else { oMode = FIL; }

	//ﾌｧｲﾙ指定不可
	if (iMode == FIL || oMode == FIL) {
		printf("FROM[%s] or TO[%s] IS NOT DIRECTORY.\n",iPath,oPath); return(NG);
	}
	//同じﾌｧｲﾙをFROM-TOに指定
	if (!_stricmp(iPath, oPath)) {
		printf("FROM and TO VALUES[%s] ARE THE SAME.\n",iPath); return(NG);
	}
    //出力ﾌｫﾙﾀﾞが入力ﾌｫﾙﾀﾞのｻﾌﾞﾌｫﾙﾀﾞ(またはその逆)
	for (ix = 0; iPath[ix]; ix++) { if (iPath[ix] != oPath[ix]) { break; } }
	if (strlen(iPath) == ix) {
		printf("TO_DIRECTORY[%s] IS A SUBFOLDER OF THE FROM_DIRECTORY(%s).\n", oPath, iPath); return(NG);
	}
	for (ix = 0; oPath[ix]; ix++) { if (iPath[ix] != oPath[ix]) { break; } }
	if (strlen(oPath) == ix) {
		printf("FROM_DIRECTORY[%s] IS A SUBFOLDER OF THE TO_DIRECTORY(%s).\n", iPath, oPath); return(NG);
	}
    //XXCOPY START 
	char *buf = NULL;
	long long size = MEGA * 32; //32MB
	if ((buf = (char*)malloc(size)) == NULL) {
		printf("SYSTEM RESOURCES ARE EXHAUSTED.\n"); return(NG);
	}
	xxCopy(iPath,oPath,buf,size);
	if (buf) { free(buf); }

	//DOS MESSAGE
	sprintf_s(DosMes,"XXCOPY COUNT = %s\n",AddCma(CopyCount));
	ScrWrite(8,2,DosMes);
	return(OK);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void xxCopy(char *iPath,char *oPath,char *buf,long long size) {
	WIN32_FIND_DATA fnd_dat;
	HANDLE h_fnd;
	char rPath[MAX_PATH*2];
	char wPath[MAX_PATH*2];

	//DOS MESSAGE
	system("cls");
	sprintf_s(DosMes, sizeof(DosMes), "FROM DIRECTORY = %s",iPath);
	ScrWrite(2, 2, DosMes);
	sprintf_s(DosMes, sizeof(DosMes), "TO   DIRECTORY = %s",oPath);
	ScrWrite(3, 2, DosMes);

	strcpy_s(rPath,sizeof(rPath),iPath);
	strcpy_s(wPath,sizeof(wPath),oPath);

	if (KanjiSecond(rPath, &rPath[strlen(rPath) - 1]) == true) { strcat_s(rPath, sizeof(rPath), "\\"); } else
	if (rPath[strlen(rPath) - 1] != '\\') { strcat_s(rPath, sizeof(rPath), "\\"); }
	if (KanjiSecond(wPath, &wPath[strlen(wPath) - 1]) == true) { strcat_s(wPath, sizeof(wPath), "\\"); } else
	if (wPath[strlen(wPath) - 1] != '\\') { strcat_s(wPath, sizeof(wPath), "\\"); }

	strcat_s(rPath,sizeof(rPath),"*.*");
	h_fnd = FindFirstFile(rPath,&fnd_dat);
	if (h_fnd == INVALID_HANDLE_VALUE) { return; }
	do {
		if (!strcmp(fnd_dat.cFileName, ".") || !strcmp(fnd_dat.cFileName, "..")) { continue; }

		strcpy_s(rPath, sizeof(rPath), iPath);
		strcpy_s(wPath, sizeof(wPath), oPath);

		if (KanjiSecond(rPath, &rPath[strlen(rPath) - 1]) == true) { strcat_s(rPath, sizeof(rPath), "\\"); } else
		if (rPath[strlen(rPath) - 1] != '\\') { strcat_s(rPath, sizeof(rPath), "\\"); }
		if (KanjiSecond(wPath, &wPath[strlen(wPath) - 1]) == true) { strcat_s(wPath, sizeof(wPath), "\\"); } else
		if (wPath[strlen(wPath) - 1] != '\\') { strcat_s(wPath, sizeof(wPath), "\\"); }

		strcat_s(rPath, sizeof(rPath), fnd_dat.cFileName);
		strcat_s(wPath, sizeof(wPath), fnd_dat.cFileName);

		if (fnd_dat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			MakeDir(wPath);
			xxCopy(rPath,wPath,buf,size);
		} else {
			if (FileCopy(rPath, wPath, buf, size) == true) {
				CopyCount++;
			}
		}
	} while (FindNextFile(h_fnd, &fnd_dat));
	FindClose(h_fnd);
	return;
}
