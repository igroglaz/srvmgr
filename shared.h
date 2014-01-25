#pragma once
#define _CRT_SECURE_NO_DEPRECATE

extern const unsigned char cr08[0x50];
extern const unsigned char cr10[0x50];
extern const unsigned char cr11[0x50];

#define C_CR cr10
#define C_VER 10 

#define CUR_VER 10
#define HAT_CR cr10 

void cryptver(unsigned char *p, int ver, int len);

