//#include "utils.h"
#include "config_new.h"
//#include <vector>
//#define _USE_MATH_DEFINES
//#include "math.h"
//#include "this_call.h"

__int16 __stdcall increase_potions_in_shops(__int16 original_value){
	return original_value * Config::shop_potions_factor;
}

void __declspec(naked) imp_increase_potions_in_shops()
{ // 0054D86B
    __asm
    {
		/*
		original code is:
		add eax,32
		mov edx,[ebp-14]
		*/
		// original code
		add	eax, 0x32
		//original code
		
		// modify eax
		push eax
		call increase_potions_in_shops

		// original code
		mov edx,[ebp-0x14]
		//
		ret
    }
}