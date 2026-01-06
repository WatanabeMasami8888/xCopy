#include "COMMON.H"
#pragma warning (disable:4996)
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
static void SetCur(int line, int column) {
	static HANDLE HStdOut = 0x00;
	if (line < 1) { line = 1; }
	if (column < 1) { column = 1; }
	COORD pos;
	pos.Y = (short)line - 1;
	pos.X = (short)column - 1;
	if (HStdOut == 0x00) { HStdOut = GetStdHandle(STD_OUTPUT_HANDLE); }
	SetConsoleCursorPosition(HStdOut, pos);
}
void ScrWrite(int line, int column, const char* str) {
	char w_buf[_MAX_PATH * 2];
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
	for (int ix = 1; ix < len; ix++) {
		ScrWrite(line, column + ix - 1, " ");
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////
// ファイル複写
int Path_Len = 0;
bool GetSectorSize(const char* filePath, DWORD& sectorSize) {
	if (!filePath || strlen(filePath) < 2 || filePath[1] != ':') { printf("Invalid file path\n"); return false; }
	char rootPath[4] = { filePath[0], ':', '\\', '\0' };
	DWORD sectorsPerCluster, bytesPerSector, freeClusters, totalClusters;
	if (!GetDiskFreeSpaceA(rootPath, &sectorsPerCluster, &bytesPerSector,&freeClusters,&totalClusters)) {
		return false;
	}
	sectorSize = bytesPerSector;
	return true;
}

bool FileCopy(char *iFile,char *oFile) {
	struct _stat64 ist = { 0 };
	struct _stat64 ost = { 0 };
	HANDLE hSrc;
	HANDLE hDst;
	DWORD         isector = 0;
	FILETIME      ftime;
	LARGE_INTEGER fsize;

	BYTE    *buffer   = 0x00;
	LONGLONG fullsize = 0;
	LONGLONG copied   = 0;
	LONGLONG tailsize = 0;
	size_t   alignsize= 0;
	size_t   base_buf = 4 * 1024 * 1024; //4MB

	//★DOS MESSAGE
	_stat64(iFile, &ist);
	ClsLine(5, 13, Path_Len);
	ClsLine(6, 13, 16);
	ClsLine(7, 2, 16); //"ALREADY COPIED!" Clear
	char* file_name = GetFileName(iFile);
	ScrWrite(5, 13, file_name);
	Path_Len = (int)strlen(file_name) + 1;
	ScrWrite(6, 13, AddCma(ist.st_size));

	//同一ﾌｧｲﾙは無視
	if (_stat64(oFile, &ost) == 0) {
		if (ist.st_mtime == ost.st_mtime && ist.st_size == ost.st_size) {
			//★DOS MESSAGE
			ScrWrite(7,2,"ALREADY COPIED!");
			return false;
		}
	}
	if (isalpha(oFile[0]) && oFile[1]==':' && oFile[2] == '\\') {
		ULARGE_INTEGER FreeByte;
		ULARGE_INTEGER TotalByte;
		ULARGE_INTEGER NumberFreeByte;
		char   drive[4] = {0};
		memcpy(drive,oFile,3); drive[3] = 0x00;
		GetDiskFreeSpaceEx(drive, &FreeByte, &TotalByte, &NumberFreeByte);
		if (FreeByte.QuadPart < (ULONGLONG)ist.st_size) {
			ScrWrite(7, 2, "DISK SPACE ERR!");
			return false;
		}
	}
	//入力ﾌｧｲﾙｵｰﾌﾟﾝ
	if ((hSrc = CreateFileA(iFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL)) == INVALID_HANDLE_VALUE) { 
		return false; 
	}
	GetFileTime(hSrc,NULL,NULL,&ftime);
	GetFileSizeEx(hSrc, &fsize);

	//出力ﾌｧｲﾙｵｰﾌﾟﾝ
	if ((hDst = CreateFileA(oFile,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_FLAG_NO_BUFFERING|FILE_FLAG_SEQUENTIAL_SCAN,NULL)) == INVALID_HANDLE_VALUE) {	
		CloseHandle(hSrc); 
		return false; 
	}

	//非ﾊﾞｯﾌｧﾘﾝｸﾞｻｲｽﾞとﾊﾞｯﾌｧ確保
	if (!GetSectorSize(iFile, isector)) { 
		CloseHandle(hSrc); 
		CloseHandle(hDst); 
		return false; 
	}
	fullsize =(fsize.QuadPart / isector) * isector;
	tailsize = fsize.QuadPart - fullsize;
	alignsize = (base_buf / isector) * isector;
	if (alignsize == 0) { alignsize = isector; }
	if ((buffer = (BYTE*)_aligned_malloc(alignsize,isector))==0x00) {
		CloseHandle(hSrc);
		CloseHandle(hDst);
		return false;
	}

	//複写
	while (copied < fullsize) {
		DWORD to_read = (DWORD)std::min((LONGLONG)alignsize,fullsize - copied);
		DWORD byte_read = 0, byte_write = 0;

		if (!ReadFile(hSrc,buffer,to_read,&byte_read,NULL)) { break; }
		if (byte_read == 0) { break; }

		if (!WriteFile(hDst,buffer,byte_read, &byte_write, NULL)) { break; }
		copied += byte_read;

		//★DOS MESSAGE 進捗表示
		ClsLine  (7, 2, 8); //999.99 %
		sprintf_s(DosMes, sizeof(DosMes), "%.2f %%", (double)copied / fsize.QuadPart * 100.00);
		ScrWrite (7, 2, DosMes);
	}
	SetFileTime(hDst,NULL,NULL,&ftime);
	CloseHandle(hDst);

	// 末尾処理（I/O）
	if (tailsize > 0) {
		LARGE_INTEGER pos;
		pos.QuadPart = fullsize;
		SetFilePointerEx(hSrc,pos,NULL,FILE_BEGIN);
		std::vector<BYTE> tailBuf((size_t)tailsize);
		DWORD bytesRead = 0;
		if (!ReadFile(hSrc, tailBuf.data(), (DWORD)tailsize, &bytesRead, NULL)) {
			CloseHandle(hSrc);
			_aligned_free(buffer);
			return false;
		}
		HANDLE hDstTail = CreateFileA(oFile,GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		SetFilePointerEx(hDstTail,pos,NULL,FILE_BEGIN);
		DWORD bytesWritten = 0;
		WriteFile  (hDstTail,tailBuf.data(),bytesRead,&bytesWritten,NULL);
		SetFileTime(hDstTail,NULL,NULL,&ftime);
		CloseHandle(hDstTail);
		copied += bytesRead;
	}
	//★DOS MESSAGE 進捗表示
	ClsLine (7, 2, 8);
	ScrWrite(7, 2, "100.00 %");

	// 更新日時を差し替え 
	CloseHandle(hSrc);
	//資源解放	
	_aligned_free(buffer);

	return true;
}
