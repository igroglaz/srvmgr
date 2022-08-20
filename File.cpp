#include "File.h"
#include "CRC_32.h"
#include "lib/utils.hpp"

FileArchive Archives;

bool FileArchive::Open(std::string filename, std::string directory)
{
    std::ifstream str;
    str.open(filename.c_str(), std::ios::in | std::ios::binary);
    if(!str.is_open())
        return false;

    str.seekg(0);

    uint32_t magic;
    str.read((char*)&magic, 4);
    if(magic != 0x31415926)
        return false;

    uint32_t fat_root, fat_offset, fat_size;
    str.read((char*)&fat_root, 4);
    str.seekg(16);
    str.read((char*)&fat_offset, 4);
    str.read((char*)&fat_size, 4);

    str.seekg(fat_offset);
    size_t pos = str.tellg();
    if(pos != (size_t)fat_offset)
        return false;

    uint32_t root_size = fat_size-fat_root;

    Record rec;
    rec.ResourceName = filename;
    rec.Name = directory;
    rec.Flags = 1;
    rec.Length = root_size;
    rec.Offset = 0;

    std::vector<uint32_t> stack_sizes;
    std::vector<uint32_t> stack_positions;
    std::vector<Record*> stack_records;

    stack_sizes.push_back(fat_root+root_size);
    stack_positions.push_back(fat_root);
    stack_records.push_back(&rec);

    // replaced monstrous recursion with this
    while(stack_sizes.size())
    {
        bool hreturn = false;

        for(uint32_t i = stack_positions.back(); i < stack_sizes.back(); i++)
        {
            str.seekg(fat_offset+i*32);

            uint32_t r_junk, r_offset, r_size, r_type;
            char r_name[17];
            r_name[16] = 0;

            str.read((char*)&r_junk, 4);
            str.read((char*)&r_offset, 4);
            str.read((char*)&r_size, 4);
            str.read((char*)&r_type, 4);
            str.read(r_name, 16);

            Record r;
            r.Offset = r_offset;
            r.Length = r_size;
            r.Flags = r_type;
            r.Name = r_name;
            r.ResourceName = filename;
            stack_records.back()->Children.push_back(r);

            if(r.Flags & 1) // directory
            {
                stack_positions.back() = i+1;
                stack_sizes.push_back(r.Offset+r.Length);
                stack_positions.push_back(r.Offset);
                stack_records.push_back(&stack_records.back()->Children.back());
                hreturn = true;
                break;
            }
        }

        if(hreturn) continue;
        stack_sizes.pop_back();
        stack_positions.pop_back();
        stack_records.pop_back();
    }

    myRoot.push_back(rec);
    str.close();
    return true;
}

const FileArchive::Record& FileArchive::GetRecord(std::string filename)
{
    static Record rec;
    filename = TruncateSlashes(FixSlashes(filename));

    std::vector<std::string> path = Explode(filename, "/");
    std::vector<Record>* where_search = &myRoot;
    for(uint32_t i = 0; i < path.size(); i++)
    {
        bool found = true;
        for(uint32_t j = 0; j < where_search->size(); j++)
        {
            Record& record = where_search->at(j);
            if(ToLower(record.Name) == (path[i]))
            {
                if(i < path.size()-1)
                {
                    where_search = &record.Children;
                    found = true;
                    break;
                }
                else return record;
            }
        }

        if(!found) break;
    }

    return rec;
}

File::File()
{
    myOpen = false;
    myName = "";
    myOffset = 0;
    myLength = 0;
    myPosition = 0;
}

bool File::Open(std::string filename)
{
    filename = TruncateSlashes(FixSlashes(filename));
    if(filename.find("patch/") == std::string::npos)
    {
        bool ffo = Open("patch/"+filename);
        if(ffo) return true;
        Close();
    }

    myStream.open(filename.c_str(), std::ios::in | std::ios::binary);

    if(myStream.is_open())
    {
        myName = filename;
        myOffset = 0;
        myStream.seekg(0, std::ios::end);
        myLength = myStream.tellg();
        myStream.seekg(0, std::ios::beg);
        myPosition = 0;
        myOpen = true;
        return true;
    }
    else
    {
        const FileArchive::Record& record = Archives.GetRecord(filename);
        if(record.Flags & 2) return false;
        if(record.Flags & 1) return false;
        myStream.open(record.ResourceName.c_str(), std::ios::in | std::ios::binary);
        if(!myStream.is_open()) return false;

        myName = filename;
        myOffset = record.Offset;
        myLength = record.Length;
        myPosition = 0;
        myStream.seekg(myOffset, std::ios::beg);
        myOpen = true;
        return true;
    }
}

void File::Close()
{
    if(myStream.is_open()) myStream.close();
    myOpen = false;
}

uint32_t File::Read(void* where, uint32_t size)
{
    if(!myOpen) return 0;
    if(size > myLength-myPosition)
        size = myLength-myPosition;
    if(!size) return 0;

    myStream.seekg(myOffset+myPosition);
    myStream.read((char*)where, size);
    uint32_t cnt = myStream.gcount();
    myPosition += cnt;

    return cnt;
}

void File::Seek(uint32_t position)
{
    if(!myOpen) return;
    if(position > myLength)
        position = myLength;
    myPosition = position;
    myStream.seekg(myOffset+myPosition);
}

uint32_t File::GetPosition()
{
    if(!myOpen) return 0;
    return myPosition;
}

uint32_t File::GetLength()
{
    if(!myOpen) return 0;
    return myLength;
}

bool File::GetLine(std::string& str)
{
    str.clear();
    if(myPosition >= myLength) return false;
    while(myPosition < myLength)
    {
        char ch = 0;
        Read(&ch, 1);
        if(ch == '\n' || ch == 0) break;

        str.insert(str.end(), ch);
    }

    return true;
}

uint32_t File::CalcCRC()
{
    if(!GetLength()) return 0;

    CRC_32 crc;
    uint32_t pos = myPosition;
    Seek(0);
    uint8_t* mp = new uint8_t[GetLength()];
    Read(mp, GetLength());
    Seek(pos);
    uint32_t ret_value = crc.CalcCRC(mp, GetLength(), NULL);
    delete[] mp;

    return ret_value;
}
