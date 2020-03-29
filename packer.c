/**
 * Simple crypter/packer implementation.
 * There are many different ways of creating a file packer.
 * What we'll be going for...
 *
 * Our packer will consist of several components, the user interfacing packer,
 * a stub that is capable of taking our payload and executing it and the 
 * payload itself of course.
 *
 * The packer will take the payload from the user, apply some simple 
 * encryption, and append both the key and the payload to the end of the stub.
 *
 * The stub does a similar process in which it takes the payload from the end
 * of itself, it'll have to read it's own header in to figure out exactly
 * where the payload begins, perhaps map it to memory and launch it? Yea no.
 * We'll just write it to disk and do the ol' fork/exec rigamarole.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <elf.h>
#include <unistd.h>
#include <stdbool.h>

/**
 * TODO: Need to be able to read ELF size from headers
 *       The way to get this done easily, on ocelot, is to simply get
 *       the last section's offset from the section header table and add it
 *       to its size. This will result in the exact number of bytes the ELF
 *       file takes up on disk.
 *
 * TODO: Read binary file from disk
 * TODO: Append packed file to the end of a stub.
 * TODO: Have stub launch the binary it's carrying.
 *
 */

#define ENC_KEY 0xbaadf00d

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
    int c = 0;
    bool printPayload = false;
    char* szPayload = "hw";
    char* szTarget = "output";

    while((c = getopt(argc, argv, "p")) != -1)
    {
        switch(c)
        {
            case 'p':
                printPayload = true;
                break;
        }
    }
 

    // get size of both stub and target elf
    FILE* pStub = fopen("stub", "r");
    if(!pStub)
    {
        perror("stub");
        return -1;
    }

    fseek(pStub, 0, SEEK_END);
    int stubSize = ftell(pStub);
    fseek(pStub, 0, SEEK_SET);

    FILE* pElf = fopen(szPayload, "r");
    if(!pElf)
    {
        perror(szPayload);
        return -1;
    }
    fseek(pElf, 0, SEEK_END);
    int payloadSize = ftell(pElf);
    fseek(pElf, 0, SEEK_SET);
    
    // open a file for writing 
    FILE* pTarget = fopen(szTarget, "w");
    if(!pTarget)
    {
        perror(szTarget);
        return -1;
    }
    
    // read in stub and write to target file
    void* pBuffer = malloc(stubSize);
    fread(pBuffer, stubSize, 1, pStub);
    fwrite(pBuffer, stubSize, 1, pTarget);
    free(pBuffer);

    // read in elf and append to target file
    pBuffer = malloc(payloadSize);
    fread(pBuffer, payloadSize, 1, pElf); 
    
    // pack/encrypt payload
    xor_crypt(ENC_KEY, pBuffer, payloadSize);
    
    // append encrypted payload to the target/stub file
    fwrite(pBuffer, payloadSize, 1, pTarget);
    
    if(printPayload)
        fwrite(pBuffer, payloadSize, 1, stdout);

    free(pBuffer);

    // close all the open files
    fclose(pStub);
    fclose(pElf);
    fclose(pTarget);
    
    pid_t pid = fork();
    if(!pid)
    {
        execlp("chmod", "chmod", "+x", "output", 0);
        perror("exec");
        return -1;
    }
    while((pid = wait(NULL)) != -1);
    return 0;
}
