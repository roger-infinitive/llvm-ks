#include <stdio.h>

#include "types.h"
#include "tokenizer.h"
#include "file.h"

Token identifier;
double numberValue;

Allocator HeapAllocator = {
    malloc,
    free
};

int main() {
    FileBuffer file = {};
    bool read = ReadEntireFileAndNullTerminate("data/test.ks", &file, HeapAllocator);
    if (!read) {
        printf("Failed to read file!\n");
    }

    Tokenizer tokenizer = {};
    tokenizer.at = file.data;

    bool running = true;
    do {    
        Token token = GetToken(&tokenizer);

        switch (token.type) {
            case Token_EndOfStream: {
                running = false;
            } break;
        
            case Token_Identifier: {
                identifier = token;
                if (TokenEquals(identifier, "def")) {
                    printf("def\n");
                } else if (TokenEquals(identifier, "extern")) {
                    printf("extern\n");
                }
            } break;
            
            case Token_Number: {
                double d = TokenToDouble(token);
                printf("d: %f\n", d);
            } break;
            
            default: {
                printf("Unhandled token\n");
            }
        }
    } while (running);

    return 0;
}