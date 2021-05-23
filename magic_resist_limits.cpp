#include "srvmgr.h"
#include "utils.h"
#include "config_new.h"

int getLimit(T_UNIT* unit, int magic_ind){
	bool warrior = (unit->unit_attrs & 4) ==0;
	bool female = unit->wordE == 34 || unit->wordE == 36;

	MainCharacterParameters &params = Config::WarriorMaleMaxParameters;
	if(warrior && !female){
	    params = Config::WarriorMaleMaxParameters;
	}else if(female & warrior){
	    params = Config::WarriorFemaleMaxParameters;
	}else if(!female && !warrior){
	    params = Config::MageMaleMaxParameters;
	}else if(female && !warrior){
	    params = Config::MageFemaleMaxParameters;
	}
	else{
	    log_format("getLimit->Error\n");
	}
	if(female){
		if(warrior){
			params = Config::WarriorFemaleMaxParameters;
		}else{
			params = Config::MageFemaleMaxParameters;
		}
	}else{
		if(warrior){
			params = Config::WarriorMaleMaxParameters;
		}else{
			params = Config::MageMaleMaxParameters;
		}
	}
	switch(magic_ind){
		case 1: return params.ResistFire;
		case 2: return params.ResistWater;
		case 3: return params.ResistAir;
		case 4: return params.ResistEarth;
		case 5: return params.ResistAstral;
	}
	return 100;
}

int __stdcall imp_limit_magic_resist(T_UNIT* unit, int magic_ind, int resist)
{
	int limit = getLimit(unit, magic_ind);
	return resist < limit ? resist: limit;
}

int __declspec(naked) imp_limit_magic_resist_wrapper()
{ // 00532243
	__asm
	{
		push eax
			push ecx
			push edx
			push eax
			push ecx
			call imp_limit_magic_resist
			mov edx, eax
			pop ecx
			pop eax
			mov [ecx+eax*2+0C2h], dx
			ret
	}
}
