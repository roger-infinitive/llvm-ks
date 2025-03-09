#ifndef _FILE_H_
#define _FILE_H_

#include <cstdio>
#include "types.h"
#include "allocators.h"

struct FileBuffer {
    u64 size;
    char* data;
};

bool ReadEntireFileAndNullTerminate(const char* filePath, FileBuffer* fileBuffer, Allocator allocator) {
    if (filePath == 0) { 
        return false; 
    }
    
    FILE* file;
    fopen_s(&file, filePath, "rb");
    if (file == 0) {
        return false;
    }
    
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);
    
    char* data = (char*)allocator.alloc(fileSize + 1);
    fileBuffer->size = fileSize;
    if (data == 0) {
        fclose(file);
        return false;
    }
    
    size_t bytesRead = fread(data, 1, fileSize, file);
    if (bytesRead < fileSize) {
        allocator.free(data);
        fclose(file);
        return false;
    }
    
    data[fileSize] = '\0';
    fileBuffer->data = data;
    fclose(file);
    return true;
}

#endif //_FILE_H_