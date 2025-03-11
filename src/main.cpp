#include <stdio.h>

#include "types.h"
#include "tokenizer.h"
#include "file.h"
#include "memory_arena.h"
#include "array.h"

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
    Node_Error = 0,
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
            Node *lhs;
            Node *rhs;            
        };
        
        // Node_Function
        struct {
            char *callee;
            Array arguments;
        };
    };
};

struct Prototype {
    char *name;
    char **argumentNames;
};

struct Function {
    Prototype *prototype;
    Node *body;
};

char* AllocString(Token token) {
    char* mem = (char*)ArenaAlloc(token.length + 1);
    memcpy(mem, token.text, token.length);
    mem[token.length] = 0;
    return mem;
}

Tokenizer tokenizer;
Array nodes;
Token currentToken;

void NextToken() {
    currentToken = GetToken(&tokenizer);
}

Node* ErrorNode(const char* errorMessage) {
    Node *node = (Node*)Next(&nodes);
    node->type = Node_Error;
    printf("Error: %s\n", errorMessage);
    return node;
}

Node* ParseExpression() {
    return ErrorNode("ParseExpression is not implemented.");
}

Node* ParseNumber() {
    Node *node = (Node*)Next(&nodes);
    node->type = Node_Number;
    node->numberValue = TokenToDouble(currentToken); 
    
    NextToken();
    return node;
}

Node* ParseParens() {
    NextToken(); // Eat '('
    Node* expression = ParseExpression();
    if (!TokenEquals(currentToken, ")")) {
        return ErrorNode("Expected ')'");
    }
    NextToken(); // Eat ')'
    return expression;
}

Node* ParseIdentifier() {
    Node *node = (Node*)Next(&nodes);
    Token identifier = currentToken;

    NextToken();
    if (currentToken.type != Token_OpenParen) {
        // simple variable reference
        node->type = Node_Variable;
        node->variableName = AllocString(identifier);
        return node;
    }
    
    // function call
    NextToken();
    
    Array arguments = {};
    InitArray(arguments, Node*, 8, ArenaAllocator);
    
    if (currentToken.type != Token_CloseParen) {
        while (true) {
            // TOOD(roger): Implement ParseExpression
            Node *arg = ParseExpression();
            if (arg->type != Node_Error) {
                Node **next = (Node**)Next(&arguments);
                *next = arg;
            } else {
                // TODO(roger): Maybe a better way to handle errors?
                return arg;
            }
            
            if (currentToken.type == Token_OpenParen) {
                break;
            }
            
            if (currentToken.type == Token_Comma) {
                return ErrorNode("Expected ')' of ',' in argument list.");
            }

            NextToken();
        }
    }
    
    NextToken(); // Eat ')'
    
    node->type = Node_Function;
    node->callee = AllocString(identifier);
    node->arguments = arguments;
    return node;
}

int main() {
    InitMemoryArena(&globalArena, MEGABYTES(256));    

    FileBuffer file = {};
    bool read = ReadEntireFileAndNullTerminate("data/test.ks", &file, ArenaAllocator);
    if (!read) {
        printf("Failed to read file!\n");
    }

    tokenizer.at = file.data;
    InitArray(nodes, Node, 1024, ArenaAllocator);
    
    bool running = true;
    NextToken();
    do {    
        switch (currentToken.type) {
            case Token_EndOfStream: {
                running = false;
            } break;
        
            case Token_Identifier: {
                ParseIdentifier();
            } break;
            
            case Token_Number: {
                ParseNumber();
            } break;
            
            case Token_OpenParen: {
                ParseParens();
            } break;
            
            default: {
                ErrorNode("Unknown token!");
            }
        }
    } while (running);
    
    for (u32 i = 0; i < nodes.count; i++) {
        Node *node = (Node*)GetElement(&nodes, i);
        switch(node->type) {
            case Node_Error: {
            } break;
        
            case Node_Variable:  {
                printf("%s\n", node->variableName);
            } break;
            
            case Node_Function: {
                printf("Function: %s\n", node->callee);
            } break;
            
            default: {
                printf("Unhandled node type\n");
            }
        }
    }

    return 0;
}