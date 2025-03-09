#include <stdio.h>

#include "types.h"
#include "tokenizer.h"
#include "file.h"
#include "memory_arena.h"

Token identifier;
double numberValue;

Allocator HeapAllocator = {
    malloc,
    free
};

MemoryArena globalArena;

void* ArenaAlloc(size_t size) {
    return PushMemory(&globalArena, size);
}

void ArenaFree(void* mem) {
    // do nothing
}

Allocator ArenaAllocator = {
    .alloc = ArenaAlloc,
    .free  = ArenaFree
};

enum NodeType {
    Node_Unknown = 0,
    Node_Number,
    Node_Variable,
    Node_Binary,
    Node_Function,
};

struct Node {
    NodeType type;
    
    union {
        // Node_Number
        double numberValue;
        
        // Node_Variable
        char* variableName;
        
        // Node_Binary
        struct {
            char op;
            Node* lhs;
            Node* rhs;            
        };
        
        // Node_Function
        struct {
            char* callee;
            u32 argumentCount;
            Node** arguments;
        };
    };
};

struct Prototype {
    char* name;
    char** argumentNames;
};

struct Function {
    Prototype* prototype;
    Node* body;
};

struct Array {
    void* items;
    u32 count;
    u32 capacity;
    size_t itemSize;
    
    Allocator allocator;
};

#define InitArray(arr, type, capacity, backingAllocator) \
    do { \
        (arr).items = (backingAllocator).alloc(capacity * sizeof(type)); \
        (arr).count = 0; \
        (arr).capacity = capacity; \
        (arr).itemSize = sizeof(type); \
        (arr).allocator = (backingAllocator); \
    } while (0)

void* Next(Array* arr) {
    if (arr->count >= arr->capacity) {
        arr->capacity *= 2;
        void* newItems = arr->allocator.alloc(arr->capacity * arr->itemSize);
        memcpy(newItems, arr->items, arr->count * arr->itemSize);
        arr->allocator.free(arr->items);
        arr->items = newItems;
        
        printf("Resized array to %u capacity\n", arr->capacity);
    }

    void* item = (u8*)arr->items + arr->count * arr->itemSize;
    arr->count++;
    return item;
}; 

int main() {
    InitMemoryArena(&globalArena, MEGABYTES(256));    

    FileBuffer file = {};
    bool read = ReadEntireFileAndNullTerminate("data/test.ks", &file, ArenaAllocator);
    if (!read) {
        printf("Failed to read file!\n");
    }

    Tokenizer tokenizer = {};
    tokenizer.at = file.data;

    Array nodes = {};
    u32 capacity = 1024;
    InitArray(nodes, Node, capacity, ArenaAllocator);
    
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