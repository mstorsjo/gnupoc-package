#ifndef __STYLE_TOKENIZER
#define __STYLE_TOKENIZER

extern "C" void initStyleTokenizer(const char* str);
extern "C" int stylelex();
extern "C" const char* styleStringToken();

#endif
