#ifndef __X_FILE_H_
#define __X_FILE_H_
#include "j_type.h"

class CXFile
{
public:
	CXFile();
	~CXFile();

public:
	char *GetVodDir(const char *pFrl, char *pDir);
	int CreateDir(char *pDir);

	int RenameFile(const char *oldName, const char *newName);
	int DeltmpByResid(const char *pDir, const char *pResid);
	
	int GetFilesByTime(const char *pDir, const char *pResid, time_t begin_time, time_t end_time, j_vec_file_info_t &vecFileInfo);

private:
    int DelFile(const char *filePath, const char *fileName);
    int ListFiles(char *pDir, j_vec_str_t &fileVec, const char *pResid = NULL);
};
#endif //~__X_FILE_H_
