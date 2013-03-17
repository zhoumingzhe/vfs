#include <memory.h>
#include <stdio.h>
#include "../VFS/GUtMd5.h"
const unsigned char pContext1[] = "sdfaweiofpupasdojfioeapyopaisdfsadkalfaweuyfpioasjfioewapfyasdpofupejfsdf";
const unsigned char pContext2[] = "sdfaweiofpupasdojfioeapyopaisdfs";
const unsigned char pContext3[] = "adkalfaweuyfpioasjfioewapfyasdpofupejfsdf";
int main()
{
    unsigned char result1[16], result2[16];
    MD5Context Md5Context1;
    MD5Init(&Md5Context1, 0);
    MD5Update(&Md5Context1, pContext1, sizeof(pContext1)-1);
    MD5Final(result1, &Md5Context1);


    MD5Context Md5Context2;
    MD5Init(&Md5Context2, 0);
    MD5Update(&Md5Context2, pContext2, sizeof(pContext2)-1);
    MD5Update(&Md5Context2, pContext3, sizeof(pContext3)-1);
    MD5Final(result2, &Md5Context2);
    if(!memcmp(result2, result1, sizeof(result1)))
    {
        printf("OK\n");
    }
    else
    {
        printf("WRONG\n");
    }
}