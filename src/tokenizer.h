#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdlib.h>
#include <cstring>
#include "allocators.h"

bool IsEndOfLine(char c) {
    return c == '\n' || c == '\r';
}

bool IsWhitespace(char c) {
    return (c == ' '   || 
            c == '\t'  || 
            IsEndOfLine(c));
}

bool IsAlpha(char c) {
    return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')));
}

bool IsNumber(char c) {
    return ((c >= '0') && (c <= '9'));
}

enum TokenType {
    Token_Unknown = 0,
    
    Token_OpenParen,
    Token_CloseParen,
    Token_Colon,
    Token_Semicolon,
    Token_Comma,
    Token_Asterisk,
    Token_OpenBracket,
    Token_CloseBracket,
    Token_OpenBrace,
    Token_CloseBrace,
    Token_Equals,
    Token_Underscore,
    Token_Period,
    Token_ForwardSlash,
    Token_BackSlash,
    
    Token_Identifier,
    Token_Number,
    Token_String,
    
    Token_EndOfStream
};

struct Token {
    TokenType type;
    int length;
    char* text;
};

struct Tokenizer {
    char* at;
};

bool TokenEquals(Token token, const char* match) {
    const char* at = match;
    for (int index = 0; index < token.length; ++index, ++at) {
        if ((*at == 0) ||
            (token.text[index] != *at)) 
        {
            return false;
        }
    }
    
    return (*at == 0);
}

Token GetToken(Tokenizer* tokenizer) {
    for (;;) {
        if (IsWhitespace(tokenizer->at[0])) {
            ++tokenizer->at;
        } else if ((tokenizer->at[0] == '/') && (tokenizer->at[1] == '/')) {
            tokenizer->at += 2;
            while (tokenizer->at[0] && !IsEndOfLine(tokenizer->at[0])) {
                ++tokenizer->at;
            }
        } else if ((tokenizer->at[0] == '/') && (tokenizer->at[1] == '*')) {
            tokenizer->at += 2;
            while (tokenizer->at[0] && !((tokenizer->at[0] == '*') && (tokenizer->at[1] == '/'))) {
                ++tokenizer->at;
            }
            
            if (tokenizer->at[0] == '*') {
                tokenizer->at += 2;
            }
        } else {
            break;
        }
    }
    
    Token token = {};
    token.length = 1;
    token.text = tokenizer->at;
    
    char c = tokenizer->at[0];
    ++tokenizer->at;
    switch (c) {
        case '\0': { token.type = Token_EndOfStream; } break;
        
        case '(': { token.type = Token_OpenParen;    } break;
        case ')': { token.type = Token_CloseParen;   } break;
        case ':': { token.type = Token_Colon;        } break;
        case ',': { token.type = Token_Comma;        } break;
        case ';': { token.type = Token_Semicolon;    } break;
        case '*': { token.type = Token_Asterisk;     } break;
        case '[': { token.type = Token_OpenBracket;  } break;
        case ']': { token.type = Token_CloseBracket; } break;
        case '{': { token.type = Token_OpenBrace;    } break;
        case '}': { token.type = Token_CloseBrace;   } break;
        case '=': { token.type = Token_Equals;       } break;
        case '.': { token.type = Token_Period;       } break;
        
        case '/':  { token.type = Token_ForwardSlash; }; break;
        case '\\': { token.type = Token_BackSlash;    }; break;
        
        case '\"': {
            token.type = Token_String;
            token.text = tokenizer->at;
            while (tokenizer->at[0] != '"' && tokenizer->at[0] != '\0') {
                if ((tokenizer->at[0] == '\\') && tokenizer->at[1]) {
                    ++tokenizer->at;
                }
                ++tokenizer->at;
            }
            token.length = tokenizer->at - token.text;
            if (tokenizer->at[0] == '"') {
                ++tokenizer->at;
            }
        }; break;
        
        default: {
            if (IsAlpha(c)) {
                token.type = Token_Identifier;
                token.text = --tokenizer->at;
                while (IsAlpha(tokenizer->at[0]) ||
                       IsNumber(tokenizer->at[0]) ||
                       (tokenizer->at[0] == '_')) 
                {
                    ++tokenizer->at;
                }
                token.length = tokenizer->at - token.text;
            } else if ((c == '-' && IsNumber(tokenizer->at[0])) || IsNumber(c)) {
                token.type = Token_Number;
                token.text = tokenizer->at - 1;
                while (IsNumber(tokenizer->at[0])) {
                    ++tokenizer->at;
                }
                //Floating Point
                if (tokenizer->at[0] == '.') {
                    ++tokenizer->at;
                    while (IsNumber(tokenizer->at[0])) {
                        ++tokenizer->at;
                    }
                }
                token.length = tokenizer->at - token.text;
            } else {
                token.type = Token_Unknown;
            }

        } break;
    }
    
    return token;
}

bool RequireToken(Tokenizer* tokenizer, TokenType tokenType) {
    Token token = GetToken(tokenizer);
    return token.type == tokenType;
}

char* CreateNullTerminatedStringFromToken(Token token) {
    char* type = (char*)malloc((token.length+1) * sizeof(char));
    memcpy(type, token.text, token.length);
    type[token.length] = 0;
    return type;
}

int TokenToInt(Token token) {
    char* end = 0;
    return (int)strtol(token.text, &end, 10);
}

double TokenToDouble(const Token& token) {
    char* end = 0;
    return (double)strtod(token.text, 0);
}

int GetIntToken(Tokenizer* tokenizer) {
    Token token = GetToken(tokenizer);
    char* end = 0;
    return (int)strtol(token.text, &end, 10);
}

bool GetBoolToken(Tokenizer* tokenizer) {
    return (bool)GetIntToken(tokenizer);
}

u16 GetU8Token(Tokenizer* tokenizer) {
    Token token = GetToken(tokenizer);
    char* end = 0;
    return (u8)strtoul(token.text, &end, 10);
}

u16 GetU16Token(Tokenizer* tokenizer) {
    Token token = GetToken(tokenizer);
    char* end = 0;
    return (u16)strtoul(token.text, &end, 10);
}

u32 GetU32Token(Tokenizer* tokenizer) {
    Token token = GetToken(tokenizer);
    char* end = 0;
    return strtoul(token.text, &end, 10);
}

u64 GetU64Token(Tokenizer* tokenizer) {
    Token token = GetToken(tokenizer);
    char* end = 0;
    return (u64)strtoull(token.text, &end, 10);
}

float GetFloatToken(Tokenizer* tokenizer) {
    Token token = GetToken(tokenizer);
    char* end = 0;
    return strtof(token.text, &end);
}

bool GetIntVariable(Tokenizer* tokenizer, int* value) {
    if (!RequireToken(tokenizer, Token_Identifier)) {
        return false;
    }
    Token token = GetToken(tokenizer);
    
    char* end = 0;
    *value = (int)strtol(token.text, &end, 10);
    return true;
}

bool GetU32Variable(Tokenizer* tokenizer, u32* value) {
    if (!RequireToken(tokenizer, Token_Identifier)) {
        return false;
    }
    Token token = GetToken(tokenizer);
    char* end = 0;
    *value = strtoul(token.text, &end, 10);
    return true;
}

bool GetU16Variable(Tokenizer* tokenizer, u16* value) {
    if (!RequireToken(tokenizer, Token_Identifier)) {
        return false;
    }
    Token token = GetToken(tokenizer);
    char* end = 0;
    *value = (u16)strtoul(token.text, &end, 10);
    return true;
}

bool GetU8Variable(Tokenizer* tokenizer, u8* value) {
    if (!RequireToken(tokenizer, Token_Identifier)) {
        return false;
    }
    Token token = GetToken(tokenizer);
    char* end = 0;
    *value = (u8)strtoul(token.text, &end, 10);
    return true;
}

bool GetFloatVariable(Tokenizer* tokenizer, float* value) {
    if (!RequireToken(tokenizer, Token_Identifier)) {
        return false;
    }
    Token token = GetToken(tokenizer);
    char* end = 0;
    *value = strtof(token.text, &end);
    return true;
}

bool GetStringVariable(Tokenizer* tokenizer, char* buffer) {
    if (!RequireToken(tokenizer, Token_Identifier)) {
        return false;
    }
    Token token = GetToken(tokenizer);
    
    memcpy(buffer, token.text, token.length);
    buffer[token.length] = '\0';
    return true;
}

char* GetFormattedStringFromToken(Token token, Allocator allocator) {
    char* string = (char*)allocator.alloc(sizeof(char) * (token.length + 1));
    u32 strIndex = 0;
    for (u32 i = 0; i < token.length; i++) {
        char c = token.text[i];
        if (c == '\\') {
            if (i != token.length-1) {
                char peekC = token.text[i+1];
                if (peekC == 't') {
                    string[strIndex++] = '\t';
                } else if (peekC == 'n') {
                    string[strIndex++] = '\n';
                } else if (peekC == '0') {
                    string[strIndex++] = '\0';
                } else if (peekC == '\\') {
                    string[strIndex++] = '\\';
                }
                i++;
            }
        } else {
            string[strIndex++] = c;
        }
    }
    string[strIndex] = '\0';
    return string;
}

#endif //Tokenizer