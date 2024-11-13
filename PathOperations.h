#pragma once

class CPathOperations {
public:
	CPathOperations(void);
	~CPathOperations(void);
	char* tmp;

	void SetFullPath(char* path_with_file_name);
	void SetPath(char* new_path);
	void SetFileName(char* new_file_name);

	char* GetFullPath(char* returned_full_path, DWORD buf_len);
	char* GetPath(char* returned_path, DWORD buf_len);
	char* GetFileName(char* returned_file_name, DWORD buf_len);
private:
	char* path;
	char* file_name;

};
