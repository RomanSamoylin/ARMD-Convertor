#include "stdafx.h"
#include "PathOperations.h"

CPathOperations::CPathOperations()
{
	path = new char[MAX_PATH];
	path[0] = 0;
	file_name = new char[MAX_PATH];
	file_name[0] = 0;
	tmp = new char[MAX_PATH];
	tmp[0] = 0;
}
CPathOperations::~CPathOperations()
{
	delete path;
	delete file_name;
	delete tmp;
}

void CPathOperations::SetFullPath(char* path_with_file_name)
{
	int str_len;
	char* slash;
	if (path_with_file_name == NULL) return;
	if (path_with_file_name[0] == 0) return;
	str_len = strlen(path_with_file_name);
	if (str_len >= MAX_PATH) return;
	if (path_with_file_name[strlen(path_with_file_name) - 1] == '\\') return;
	slash = strrchr(path_with_file_name, '\\');
	if (slash == NULL) return;
	str_len = slash - path_with_file_name;
	if (str_len > 0 && str_len < MAX_PATH)
	{
		memcpy(path, path_with_file_name, str_len + 1);
		path[str_len + 1] = 0;
	}
	str_len = &path_with_file_name[strlen(path_with_file_name)] - slash;
	if (str_len > 0 && str_len < MAX_PATH)
	{
		memcpy(file_name, slash + 1, str_len);
		file_name[str_len] = 0;
	}
	return;
}
void CPathOperations::SetPath(char* new_path)
{
	int str_len;
	if (new_path == NULL) return;
	if (new_path[0] == 0) return;
	str_len = strlen(new_path);
	if (str_len + strlen(file_name) < MAX_PATH)
	{
		strcpy(path, new_path);
		if (path[str_len - 1] != '\\') { path[str_len] = '\\'; str_len++; path[str_len] = 0; }
	}
	return;
}
void CPathOperations::SetFileName(char* new_file_name)
{
	int str_len;
	if (new_file_name == NULL) return;
	str_len = strlen(new_file_name);
	if (str_len + strlen(path) < MAX_PATH)
	{
		strcpy(file_name, new_file_name);
	}
	return;
}

char* CPathOperations::GetFullPath(char* returned_full_path, DWORD buf_len)
{
	int str_len;
	if (returned_full_path == NULL || buf_len == 0)
	{
		strcpy(tmp, path);
		strcat(tmp, file_name);
		return tmp;
	}
	else
	{
		str_len = strlen(path) + strlen(file_name) + 1;
		if (buf_len < str_len) return NULL;
		strcpy(returned_full_path, path);
		strcat(returned_full_path, file_name);
		return returned_full_path;
	}
}


char* CPathOperations::GetPath(char* returned_path, DWORD buf_len)
{
	int str_len;
	if (returned_path == NULL || buf_len == 0)
	{
		strcpy(tmp, path);
		return tmp;
	}
	else
	{
		str_len = strlen(path) + 1;
		if (buf_len < str_len) return NULL;
		strcpy(returned_path, path);
		return returned_path;
	}
	return NULL;
}

char* CPathOperations::GetFileName(char* returned_file_name, DWORD buf_len)
{
	int str_len;
	if (returned_file_name == NULL || buf_len == 0)
	{
		strcpy(tmp, file_name);
		return tmp;
	}
	else
	{
		str_len = strlen(file_name) + 1;
		if (buf_len < str_len) return NULL;
		strcpy(returned_file_name, file_name);
		return returned_file_name;
	}
}