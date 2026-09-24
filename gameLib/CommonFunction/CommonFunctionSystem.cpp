#include "CommonFunctionSystem.h"
#include <stdio.h>
#include "../Misc/md5.h"
#include "../Misc/sha1.h"

namespace th5w{

CCommonFunctionSystem::CCommonFunctionSystem(void)
{
}

CCommonFunctionSystem::~CCommonFunctionSystem(void)
{
}

void CCommonFunctionSystem::CreateConsoleWindow()
{
    setvbuf(stdout, nullptr, _IONBF, 0);
}

bool CCommonFunctionSystem::GetFileMD5(unsigned char *retMD5,const char *fileName)
{
	MD5_CTX ctxMD5;
	MD5Init(&ctxMD5);
	FILE *fp=fopen(fileName,"rb");
	if (fp==NULL)
		return false;
	int blockSize=1024*128;
	unsigned char *buffer=new unsigned char[blockSize];
	if (buffer==NULL)
	{
		fclose(fp);
		return false;
	}
	int nRead;
	while((nRead=(int)fread(buffer,1,blockSize,fp))>0)
		MD5Update(&ctxMD5,buffer,nRead);
	delete[] buffer;
	fclose(fp);
	MD5Final(retMD5,&ctxMD5);

	return true;
}

}































