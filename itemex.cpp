#include <string>
#include <vector>
#include "lib/utils.hpp"
#include "syslib.h"
#include <stdint.h>
#include "File.h"
#include "srvmgr.h"

enum
{
	IgnoreFlags_Id			= 0x01,
	IgnoreFlags_Type		= 0x02,
	IgnoreFlags_Name		= 0x04,
	IgnoreFlags_Material	= 0x08,
	IgnoreFlags_Class		= 0x10,
	IgnoreFlags_Slot		= 0x20
};

bool itemex_check_bit(uint32_t flags, uint8_t bit)
{
	if(bit > 31) return false;

	uint32_t flag = 1;
	if(bit > 0) flag <<= (bit-1);
	return (flags & flag);
}

void itemex_set_bit(uint32_t& flags, uint8_t bit, bool value)
{
	if(bit > 31) return;

	uint32_t flag = 1;
	if(bit > 0) flag <<= (bit-1);
	
	if(value) flags |= flag;
	else flags &= ~flag;
}

struct IgnoreStructure
{
	uint32_t classIgnored;
	uint32_t materialIgnored;
	uint32_t typeIgnored;
	uint32_t slotIgnored;
	std::vector<std::string> nameIgnored;
	bool nameNot;
	std::vector<uint16_t> idsIgnored;
	bool idsNot;

	uint32_t flags;
};

bool itemex_ParseLine(std::string line, IgnoreStructure& structure)
{
	static std::string strings_classes[] =
    {
        "common",
        "uncommon",
        "rare",
        "very rare",
        "elven",
        "bad",
        "good",
        "",
    };

	static std::string strings_materials[] =
    {
        "iron",
        "bronze",
        "steel",
        "silver",
        "gold",
        "mithrill",
        "adamantium",
        "meteoric",
        "wood",
        "magic wood",
        "leather",
        "hard leather",
        "dragon leather",
        "bone",
        "crystal",
        "none",
    };

	std::string whatIgnoring = "";
	bool not = false;
	bool waitingNot = false;

	structure.flags = 0;

	std::vector<std::string> args = ParseSpaceDelimited(line, true);
	if(ToLower(args[0]) != "ignore") return true;

	structure.classIgnored = 0;
	structure.materialIgnored = 0;
	structure.typeIgnored = 0;
	structure.slotIgnored = 0;
	structure.nameIgnored.clear();
	structure.nameNot = false;
	structure.idsIgnored.clear();
	structure.idsNot = false;
	structure.flags = 0;

	for(uint32_t i = 1; i < args.size(); i++)
	{
		std::string arg = ToLower(args[i]);
		if(arg == "material" ||
			arg == "type" ||
			arg == "id" ||
			arg == "class" ||
			arg == "name" ||
			arg == "slot")
		{
			whatIgnoring = ToLower(arg);
			waitingNot = true;
			if(arg == "material")
				structure.flags |= IgnoreFlags_Material;
			else if(arg == "type")
				structure.flags |= IgnoreFlags_Type;
			else if(arg == "id")
				structure.flags |= IgnoreFlags_Id;
			else if(arg == "class")
				structure.flags |= IgnoreFlags_Class;
			else if(arg == "name")
				structure.flags |= IgnoreFlags_Name;
			else if(arg == "slot")
				structure.flags |= IgnoreFlags_Slot;
			continue;
		}

		if(waitingNot && arg == "not")
		{
			not = true;
			if(whatIgnoring == "material")
				structure.materialIgnored = 0xFFFFFFFF;
			else if(whatIgnoring == "type")
				structure.typeIgnored = 0xFFFFFFFF;
			else if(whatIgnoring == "class")
				structure.classIgnored = 0xFFFFFFFF;
			else if(whatIgnoring == "slot")
				structure.slotIgnored = 0xFFFFFFFF;
			else if(whatIgnoring == "id")
				structure.idsNot = not;
			else if(whatIgnoring == "name")
				structure.nameNot = not;
			waitingNot = false;
			continue;
		}

		if(whatIgnoring == "class")
		{
			int cls = -1;
			for(int i = 0; i < 8; i++)
			{
				if(strings_classes[i] == arg)
				{
					cls = i;
					break;
				}
			}

			if(cls != -1)
				itemex_set_bit(structure.classIgnored, cls, !not);
			else return false;
		}
		else if(whatIgnoring == "material")
		{
			int mat = -1;
			for(int i = 0; i < 16; i++)
			{
				if(strings_materials[i] == arg)
				{
					mat = i;
					break;
				}
			}

			if(mat != -1)
				itemex_set_bit(structure.materialIgnored, mat, !not);
			else return false;
		}
		else if(whatIgnoring == "slot")
		{
			int slt = -1;
			if(CheckInt(arg))
				slt = StrToInt(arg);
			if(slt != -1)
				itemex_set_bit(structure.slotIgnored, slt, !not);
			else return false;
		}
		else if(whatIgnoring == "type")
		{
			int typ = -1;
			if(arg == "buckler" || arg == "ring") typ = 1;
			else if(arg == "amulet" || arg == "dagger" || arg == "metal small shield") typ = 2;
			else if(arg == "short sword" || arg == "leather small shield" || arg == "hat") typ = 3;
			else if(arg == "long sword" || arg == "cap" || arg == "wood small shield") typ = 4;
			else if(arg == "metal large shield" || arg == "low hat" || arg == "bastard sword") typ = 5;
			else if(arg == "metal helm" || arg == "two handed sword" || arg == "leather large shield") typ = 6;
			else if(arg == "leather helm" || arg == "club" || arg == "wood large shield") typ = 7;
			else if(arg == "spiked club" || arg == "chain helm" || arg == "metal tower shield") typ = 8;
			else if(arg == "mace" || arg == "full helm" || arg == "wood tower shield") typ = 9;
			else if(arg == "morning star" || arg == "plate helm") typ = 10;
			else if(arg == "cloak" || arg == "pick hammer") typ = 11;
			else if(arg == "war hammer" || arg == "cape") typ = 12;
			else if(arg == "robe" || arg == "staff") typ = 13;
			else if(arg == "dress" || arg == "shaman staff") typ = 14;
			else if(arg == "pike" || arg == "leather mail") typ = 15;
			else if(arg == "halberd" || arg == "chain mail") typ = 16;
			else if(arg == "lance" || arg == "scale mail") typ = 17;
			else if(arg == "axe" || arg == "cuirass") typ = 18;
			else if(arg == "two handed axe" || arg == "plate cuirass") typ = 19;
			else if(arg == "metal bracers" || arg == "wood short bow") typ = 20;
			else if(arg == "leather bracers" || arg == "wood long bow") typ = 21;
			else if(arg == "wood crossbow" || arg == "plate bracers") typ = 22;
			else if(arg == "gloves") typ = 23;
			else if(arg == "leather gauntlets") typ = 24;
			else if(arg == "chain gauntlets") typ = 25;
			else if(arg == "scale gauntlets") typ = 26;
			else if(arg == "shoes") typ = 27;
			else if(arg == "leather boots") typ = 28;
			else if(arg == "chain boots") typ = 29;
			else if(arg == "plate boots") typ = 30;

			if(typ != -1)
				itemex_set_bit(structure.typeIgnored, typ, !not);
			else return false;
		}
		else if(whatIgnoring == "id")
		{
			uint16_t id = 0;
			if(CheckHex(arg))
				id = HexToInt(arg);
			if(id != 0)
				structure.idsIgnored.push_back(id);
			else return false;
		}
		else if(whatIgnoring == "name")
		{
			std::string name = Trim(args[i]);
			structure.nameIgnored.push_back(name);
		}
	}

	return true;
}

std::vector<IgnoreStructure> itemex_ignoreStructures;

bool itemex_Initialize()
{
	File f;
	if(!f.Open("world/itemex.txt"))
	{
		log_format("Error: couldn't open \"world/itemex.txt\".\n");
		return false;
	}

	std::string line = "";
	int ln_idx = -1;
	/*while(f.GetLine(line))
	{
		ln_idx++;
		log_format("%u: %s\n", ln_idx, line.c_str());
		line = Trim(line);
		if(!line.length()) continue;

		IgnoreStructure is;
		if(!itemex_ParseLine(line, is))
		{
			log_format("Warning: couldn't parse line %u of \"world/itemex.txt\".\n", ln_idx);
			continue;
		}

		if(is.flags == 0) continue;

		std::string a_s = Format("Added item. Flags = %08X", is.flags);
		if(is.flags & IgnoreFlags_Class)
			a_s += Format("; Class = %08X", is.classIgnored);
		if(is.flags & IgnoreFlags_Material)
			a_s += Format("; Material = %08X", is.materialIgnored);
		if(is.flags & IgnoreFlags_Slot)
			a_s += Format("; Slot = %08X", is.slotIgnored);
		if(is.flags & IgnoreFlags_Type)
			a_s += Format("; Type = %08X", is.typeIgnored);
		if(is.flags & IgnoreFlags_Id)
		{
			std::string a_i;
			for(size_t i = 0; i < is.idsIgnored.size(); i++)
				a_i += Format("%04X,", is.idsIgnored[i]);
			a_s += Format("; Id = [%s]", a_i.c_str());
		}
		if(is.flags & IgnoreFlags_Name)
		{
			std::string a_i;
			for(size_t i = 0; i < is.nameIgnored.size(); i++)
				a_i += Format("\"%s\",", is.nameIgnored[i].c_str());
			a_s += Format("; Name = [%s]", a_i.c_str());
		}
		a_s += "\n";
		log_format((char*)a_s.c_str());
		itemex_ignoreStructures.push_back(is);
	}*/

	f.Close();

	return true;
}

bool itemex_CheckItemIgnored(byte* item)
{
	uint32_t item_class = *(uint8_t*)(item + 0x45);
	uint32_t item_material = *(uint8_t*)(item + 0x46);
	uint32_t item_option = *(uint16_t*)(item + 0x0C);
	uint32_t item_slot = *(uint8_t*)(item + 0x58);
	uint16_t item_id = *(uint16_t*)(item + 0x40);
	std::string item_name(*(const char**)(*(byte**)(item + 0x3C) + 4));

	// special case: (null)
	if(item_class == 0 && item_material == 0 && item_option == 0 && item_slot == 0) return 0;

	if(item_slot == 0 && item_class == 0 && item_material == 0 && *(uint16_t*)(item + 0x4A) == 1)
	{
		// that is, slot = 14
		item_slot = 14;
	}

	for(std::vector<IgnoreStructure>::iterator it = itemex_ignoreStructures.begin();
		it != itemex_ignoreStructures.end(); ++it)
	{
		IgnoreStructure& s = (*it);
		if((s.flags & IgnoreFlags_Class) && itemex_check_bit(s.classIgnored, item_class))
			return true;
		if((s.flags & IgnoreFlags_Material) && itemex_check_bit(s.materialIgnored, item_material))
			return true;
		if((s.flags & IgnoreFlags_Slot) && itemex_check_bit(s.slotIgnored, item_slot))
			return true;
		if((s.flags & IgnoreFlags_Type) && itemex_check_bit(s.typeIgnored, item_option))
			return true;
		if(s.flags & IgnoreFlags_Id)
		{
			for(std::vector<uint16_t>::iterator jt = s.idsIgnored.begin();
				jt != s.idsIgnored.end(); ++jt)
			{
				uint16_t& iid = (*jt);
				if(iid == item_id)
					return true;
			}
		}
		if(s.flags & IgnoreFlags_Name)
		{
			for(std::vector<std::string>::iterator jt = s.nameIgnored.begin();
				jt != s.nameIgnored.end(); ++jt)
			{
				std::string& name = (*jt);
				if(name == item_name)
					return true;
			}
		}
	}

	return false;
}