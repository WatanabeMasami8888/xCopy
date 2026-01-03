#include "COMMON.H"

extern char DosMes[MAX_PATH * 2];

//漢字の1ﾊﾞｲﾄ目
#define ISKANJI(X) ((((unsigned char)(X)>=0x81)&&((unsigned char)(X)<=0x9F))||(((unsigned char)(X)>=0xE0)&&((unsigned char)(X)<=0xFC)))
static int KanjiFirst(char ch) { return (ISKANJI(ch)); }
//漢字の2ﾊﾞｲﾄ目
bool KanjiSecond(char* buf, char* chr) {
	int idx, len;
	if (((len = (int)(chr - buf)) == 0) || len >= strlen(buf) || !KanjiFirst(*(chr - 1))) { return(false); }
	for (idx = 0; idx < len; ) { (KanjiFirst(buf[idx])) ? idx += 2 : idx++; }
	return((chr == buf + idx) ? false : true);
}
//ﾊﾟｽが存在しない場合はﾃﾞｨﾚｸﾄﾘ作成
static char *GetWord(char* in_str, char* out_str, const char* split_str, int max_str) {
	static char DLM[] = { CMM,SPC,TAB,CR,LF,0x00 };
	int idx, idy, idz;
	bool dlm_flg = false, dbl_flg = false;
	if (split_str == 0x00) { split_str = DLM; }
	for (idx = 0, idy = 0, idz = 0; in_str[idx] != 0x00 && idz < max_str - 1; idx++) {
		if (in_str[idx] == DBL) { dlm_flg = ((dbl_flg = dbl_flg ? false : true) == true) ? true : dlm_flg; continue; }
		if (dbl_flg == true)    { out_str[idz++] = in_str[idx]; continue; }
		for (idy = 0; split_str[idy] != 0x00; idy++) {
			if (in_str[idx] == split_str[idy] && (KanjiSecond(in_str, in_str + idx) == false)) { break; }
		}
		if (split_str[idy] == 0x00) { 
			out_str[idz++] = in_str[idx]; dlm_flg = true; 
		} else {
			if (dlm_flg == false) { continue; } else { break; }
		}
	}
	out_str[idz] = 0x00;
	return (out_str[0] ? &in_str[idx] : 0x00);
}
void MakeDir(const char *path) {
	int  ix;
	char *ptr;
	char str[_MAX_PATH*2] = { "" };
	char dir[_MAX_PATH*2];
    //存在する場合はﾊﾟｽ
	if (_access_s(path, 00) == 0) { return; }
	ptr = (char*)path;
	for (ix=0; ((ptr = GetWord(ptr, dir, ":\\", _MAX_PATH*2)) != 0x00); ix++) {
		if (ix==0) {
			if ((strlen(dir) == 1) && isalpha((unsigned char)dir[0])) {
				strcpy_s(str,sizeof(str),dir);
				strcat_s(str,sizeof(str),":\\");
				if (_access_s(str, 00)) { return; } else { continue; }
			} else {
				strcpy_s(str, sizeof(str), "\\\\");
				strcat_s(str, sizeof(str), dir);
				continue;
			}
		} else {
			strcat_s(str, sizeof(str), "\\");
			strcat_s(str, sizeof(str), dir);
		}
		if (_access_s(str, 00) != 0) { 
			if (!CreateDirectory(str, NULL)) { return; }
		}
	}
}
//ﾊﾟｽ名からﾌｧｲﾙ名を取得
#pragma warning (disable:4996)
char *StrRChr(char* buf, char chr) {
	int  idx;
	char* ptr = 0x00;
	for (idx = (int)strlen(buf) - 1; idx >= 0; idx--) {
		if ((buf[idx] == chr) && (KanjiSecond(buf, buf + idx) == false)) {
			ptr = buf + idx;
			break;
		}
	}
	return(ptr);
}
char *GetFileName(char* path) {
	static char file_str[MAX_PATH];
	char* ptr;
	if ((ptr = StrRChr(strcpy(file_str,path),'\\')) != 0x00) { return(ptr + 1); }
	return(file_str);
}
char *SftStr(char *str,int num) {
	int idx;
	int len = (int)strlen(str);
	if (num > 0) {
		for (idx = len; idx >= 0; idx--) { *(str + idx + num) = *(str + idx); }
		memset(str, SPC, num);
	} else {
		for (idx = 0; idx <= len; idx++) { 
			if ((idx + num) >= 0) { *(str + idx + num) = *(str + idx); }
		}
	}
	return(str);
}
char *AddCma(__int64 num) {
	static char str[64];
	__int64 xx, zz;
	int  px = 0, qx = 0;
	bool minus = false;
	if (num < 0) { minus = true; zz = xx = num * -1; }
	else { zz = xx = num; }
	do { xx /= 10; px++; } while (xx);
	px += (px - 1) / 3; str[px] = 0x00;
	while (px > 0) { str[--px] = (char)('0' + (zz % 10)); zz /= 10; qx++; if (qx == 3) { str[--px] = ','; qx = 0; } }
	if (minus == true) { SftStr(str, 1); str[0] = '-'; }
	return(str);
}
int Path_Len = 0;
bool FileCopy(char* iFile, char* oFile, char* buf, long long size) {
	int ifd = -1, ofd = -1, len = 0, rcd = 0;
	struct _stat64 ist = { 0 };
	struct _stat64 ost = { 0 };
	__int64 SumSize = 0;

	//DOS MESSAGE
	char *file_name = GetFileName(iFile);
	ClsLine(5,13,Path_Len);
	ClsLine(6,2,15);
	sprintf_s(DosMes,sizeof(DosMes), "FILENAME = %s",file_name);
	ScrWrite(5,2,DosMes);
	Path_Len = (int)strlen(file_name)*2;

	// 更新時間とサイズが同じなら複写しない
	if (_stat64(iFile, &ist) == 0 && _stat64(oFile, &ost) == 0) {
		if (ist.st_mtime == ost.st_mtime && ist.st_size == ost.st_size) {
			ScrWrite(6,2,"ALREADY COPIED!");
			return false;
		}
	}
	struct __utimbuf64 ut = { 0 }; // 初期化
	ut.actime  = ist.st_atime;     // 最終アクセス時刻
	ut.modtime = ist.st_mtime;     // 最終更新時刻

	if ((ifd = RD_OPEN(iFile)) < 0) { return false; }
	if ((ofd = WR_OPEN(oFile)) < 0) { close(ifd); return false; }
	while ((len = read(ifd, buf, (unsigned int)size)) > 0) {
		SumSize += len;
		rcd = write(ofd, buf, len);
		//DOS MESSAGE 進捗表示
		ClsLine(6,2,15);
		sprintf_s(DosMes, sizeof(DosMes), "%lld %%",(long long)((double)SumSize / ist.st_size * 100));
		ScrWrite(6,2,DosMes);
	}
	close(ifd);
	close(ofd);
	//更新時間設定
	_utime64(oFile, &ut);

	//DOS MESSAGE 進捗表示
	ClsLine(6, 2, 15);
	ScrWrite(6,2,"100 %");
	return true;
}
static void SetCur(int line, int column) {
	static HANDLE HStdOut = 0x00;
	if (line   < 1) { line   = 1; }
	if (column < 1) { column = 1; }
	COORD pos;
	pos.Y = (short)line - 1;
	pos.X = (short)column - 1;
	if (HStdOut == 0x00) { HStdOut = GetStdHandle(STD_OUTPUT_HANDLE); }
	SetConsoleCursorPosition(HStdOut, pos);
}
void ScrWrite(int line, int column, const char* str) {
	char w_buf[_MAX_PATH*2];
	char* ptr;
	strcpy(w_buf, str);
	if ((ptr = (char*)strchr(w_buf, '\r')) != 0x00) { *ptr = 0x00; }
	if ((ptr = (char*)strchr(w_buf, '\n')) != 0x00) { *ptr = 0x00; }
	ShowCursor(false);
	SetCur(line, column);
	printf("%s", w_buf);
	ShowCursor(true);
}
void ClsLine(int line, int column, int len) {
	for (int ix=1; ix < len; ix++) {
		ScrWrite(line,column+ix-1," ");
	}
}
