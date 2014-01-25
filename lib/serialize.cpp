#include "serialize.hpp"
#include <fstream>

BinaryStream::BinaryStream()
{
    Reset();
}

BinaryStream::~BinaryStream()
{
    Reset();
}

bool BinaryStream::LoadFromFile(std::string filename)
{
    Reset();
    std::ifstream strm;
    strm.open(filename.c_str(), std::ios::in | std::ios::binary);
    if(!strm.is_open()) return false;

    strm.seekg(0, std::ios::end);
    size_t size = strm.tellg();
    strm.seekg(0, std::ios::beg);

    for(size_t i = 0; i < size; i++)
    {
        uint8_t d;
        strm.read((char*)&d, 1);
        myBuffer.push_back(d);
    }

    strm.close();
    return true;
}

bool BinaryStream::SaveToFile(std::string filename)
{
    std::ofstream strm;
    strm.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
    if(!strm.is_open()) return false;

    for(size_t i = 0; i < myBuffer.size(); i++)
        strm.write((char*)&myBuffer[i], 1);

    strm.close();
    return true;
}

bool BinaryStream::LoadFromStream(BinaryStream& stream, uint32_t size)
{
    Reset();
    if(stream.myGetPos + size > stream.myBuffer.size()) return false;
    for(size_t i = 0; i < size; i++)
        myBuffer.push_back(stream.myBuffer[stream.myGetPos+i]);
    stream.myGetPos += size;
    return true;
}

void BinaryStream::SaveToStream(BinaryStream& stream)
{
    if(stream.mySetPos > stream.myBuffer.size()) stream.mySetPos = stream.myBuffer.size();
    for(size_t i = 0; i < myBuffer.size(); i++)
    {
        stream.myBuffer.insert(stream.myBuffer.begin()+stream.mySetPos, myBuffer[i]);
        stream.mySetPos++;
    }
}

uint32_t BinaryStream::Seek(uint32_t pos)
{
    if(pos > myBuffer.size()) pos = myBuffer.size();
    myGetPos = pos;
    mySetPos = pos;
    return pos;
}

uint8_t BinaryStream::ReadUInt8()
{
    if(myGetPos+1 > myBuffer.size())
    {
        myGetPos = myBuffer.size();
        return 0;
    }

    uint8_t d = myBuffer[myGetPos];
    myGetPos++;
    return d;
}

uint16_t BinaryStream::ReadUInt16()
{
    if(myGetPos+2 > myBuffer.size())
    {
        myGetPos = myBuffer.size();
        return 0;
    }

    uint16_t d = myBuffer[myGetPos+1];
    d <<= 8;
    d |= myBuffer[myGetPos];
    myGetPos += 2;
    return d;
}

uint32_t BinaryStream::ReadUInt32()
{
    if(myGetPos+4 > myBuffer.size())
    {
        myGetPos = myBuffer.size();
        return 0;
    }

    uint32_t d = myBuffer[myGetPos+3];
    d <<= 8;
    d |= myBuffer[myGetPos+2];
    d <<= 8;
    d |= myBuffer[myGetPos+1];
    d <<= 8;
    d |= myBuffer[myGetPos];
    myGetPos += 4;
    return d;
}

double BinaryStream::ReadDouble()
{
    double d;
    *(uint32_t*)(&d) = ReadUInt32();
    *(uint32_t*)((uint8_t*)&d + 4) = ReadUInt32();
    return d;
}

float BinaryStream::ReadFloat()
{
    float d;
    *(uint32_t*)(&d) = ReadUInt32();
    return d;
}

void BinaryStream::WriteUInt8(uint8_t what)
{
    if(mySetPos > myBuffer.size()) mySetPos = myBuffer.size();
    myBuffer.insert(myBuffer.begin()+mySetPos, what);
    mySetPos++;
}

void BinaryStream::WriteUInt16(uint16_t what)
{
    if(mySetPos > myBuffer.size()) mySetPos = myBuffer.size();
    uint8_t d1 = (what & 0xFF00) >> 8;
    uint8_t d2 = (what & 0x00FF);
    myBuffer.insert(myBuffer.begin()+mySetPos, d2);
    mySetPos++;
    myBuffer.insert(myBuffer.begin()+mySetPos, d1);
    mySetPos++;
}

void BinaryStream::WriteUInt32(uint32_t what)
{
    if(mySetPos > myBuffer.size()) mySetPos = myBuffer.size();
    uint8_t d1 = (what & 0xFF000000) >> 24;
    uint8_t d2 = (what & 0x00FF0000) >> 16;
    uint8_t d3 = (what & 0x0000FF00) >> 8;
    uint8_t d4 = (what & 0x000000FF);
    myBuffer.insert(myBuffer.begin()+mySetPos, d4);
    mySetPos++;
    myBuffer.insert(myBuffer.begin()+mySetPos, d3);
    mySetPos++;
    myBuffer.insert(myBuffer.begin()+mySetPos, d2);
    mySetPos++;
    myBuffer.insert(myBuffer.begin()+mySetPos, d1);
    mySetPos++;
}

void BinaryStream::WriteDouble(double what)
{
    WriteUInt32(*(uint32_t*)(&what));
    WriteUInt32(*(uint32_t*)((uint8_t*)&what + 4));
}

void BinaryStream::WriteFloat(float what)
{
    WriteUInt32(*(uint32_t*)(&what));
}

std::string BinaryStream::ReadString()
{
    if(myGetPos+2 > myBuffer.size())
    {
        myGetPos = myBuffer.size();
        return "";
    }

    uint16_t size = ReadUInt16();
    if(!size) return "";

    if(myGetPos+size+1 > myBuffer.size())
    {
        myGetPos = myBuffer.size();
        return "";
    }

    bool nowrite = false;
    std::string ret;
    for(uint16_t i = 0; i < size; i++)
    {
        if(myBuffer[myGetPos] == 0) nowrite = true;
        else if(!nowrite) ret += myBuffer[myGetPos];
        myGetPos++;
    }
    return ret;
}

std::string BinaryStream::ReadBigString()
{
    if(myGetPos+4 > myBuffer.size())
    {
        myGetPos = myBuffer.size();
        return "";
    }

    uint32_t size = ReadUInt32();
    if(myGetPos+size+1 > myBuffer.size())
    {
        myGetPos = myBuffer.size();
        return "";
    }

    bool nowrite = false;
    std::string ret;
    for(uint32_t i = 0; i < size; i++)
    {
        if(myBuffer[myGetPos] == 0) nowrite = true;
        else if(!nowrite) ret += myBuffer[myGetPos];
        myGetPos++;
    }
    return ret;
}

std::string BinaryStream::ReadSmallString()
{
    if(myGetPos+1 > myBuffer.size())
    {
        myGetPos = myBuffer.size();
        return "";
    }

    uint8_t size = ReadUInt8();
    if(myGetPos+size+1 > myBuffer.size())
    {
        myGetPos = myBuffer.size();
        return "";
    }

    bool nowrite = false;
    std::string ret;
    for(uint8_t i = 0; i < size; i++)
    {
        if(myBuffer[myGetPos] == 0) nowrite = true;
        else if(!nowrite) ret += myBuffer[myGetPos];
        myGetPos++;
    }
    return ret;
}

std::string BinaryStream::ReadFixedString(uint32_t size)
{
    if(myGetPos+size > myBuffer.size())
    {
        myGetPos = myBuffer.size();
        return "";
    }

    bool nowrite = false;
    std::string ret;
    for(uint32_t i = 0; i < size; i++)
    {
        if(myBuffer[myGetPos] == 0) nowrite = true;
        else if(!nowrite) ret += myBuffer[myGetPos];
        myGetPos++;
    }

    return ret;
}

void BinaryStream::WriteString(std::string what)
{
    if(mySetPos > myBuffer.size()) mySetPos = myBuffer.size();
    WriteUInt16((uint16_t)what.size());
    for(uint16_t i = 0; i < (uint16_t)what.size(); i++)
    {
        myBuffer.insert(myBuffer.begin()+mySetPos, what[i]);
        mySetPos++;
    }

    myBuffer.insert(myBuffer.begin()+mySetPos, 0);
    mySetPos++;
}

void BinaryStream::WriteBigString(std::string what)
{
    if(mySetPos > myBuffer.size()) mySetPos = myBuffer.size();
    WriteUInt32((uint32_t)what.size());
    for(uint32_t i = 0; i < (uint32_t)what.size(); i++)
    {
        myBuffer.insert(myBuffer.begin()+mySetPos, what[i]);
        mySetPos++;
    }

    myBuffer.insert(myBuffer.begin()+mySetPos, 0);
    mySetPos++;
}

void BinaryStream::WriteSmallString(std::string what)
{
    if(mySetPos > myBuffer.size()) mySetPos = myBuffer.size();
    WriteUInt8((uint8_t)what.size());
    for(uint8_t i = 0; i < (uint8_t)what.size(); i++)
    {
        myBuffer.insert(myBuffer.begin()+mySetPos, what[i]);
        mySetPos++;
    }

    myBuffer.insert(myBuffer.begin()+mySetPos, 0);
    mySetPos++;
}

void BinaryStream::WriteFixedString(std::string what, uint32_t size)
{
    if(mySetPos > myBuffer.size()) mySetPos = myBuffer.size();
    for(uint32_t i = 0; i < size; i++)
    {
        if(i < (uint32_t)what.size())
            myBuffer.insert(myBuffer.begin()+mySetPos, what[i]);
        else myBuffer.insert(myBuffer.begin()+mySetPos, 0);
        mySetPos++;
    }
}

bool BinaryStream::EndOfStream()
{
    bool eos = false;
    if(mySetPos > myBuffer.size()) mySetPos = myBuffer.size();
    if(myGetPos >= myBuffer.size())
    {
        eos = true;
        myGetPos = myBuffer.size();
    }

    return eos;
}

void BinaryStream::Reset()
{
    myBuffer.clear();
    myGetPos = 0;
    mySetPos = 0;
}

uint32_t BinaryStream::GetLength()
{
    return (uint32_t)myBuffer.size();
}

uint32_t BinaryStream::GetPosition()
{
    return myGetPos;
}

/*bool BinaryStream::LoadFromResource(std::string filename)
{
    Reset();
    CFile file;
    if(!file.Open(filename)) return false;
    file.Seek(0);
    uint8_t* ch_siz = NULL;
    if(file.GetLength())
    {
        ch_siz = new uint8_t[file.GetLength()];
        file.Read(ch_siz, file.GetLength());
    }
    file.Close();

    myBuffer.resize(file.GetLength());
    for(uint32_t i = 0; i < file.GetLength(); i++)
        myBuffer[i] = ch_siz[i];
    if(ch_siz) delete[] ch_siz;
    ch_siz = NULL;

    return true;
}*/

void BinaryStream::WriteData(void* what, uint32_t size)
{
    uint8_t* data = (uint8_t*)what;
    for(uint32_t i = 0; i < size; i++)
        WriteUInt8(data[i]);
}

void BinaryStream::ReadData(void* where, uint32_t size)
{
    uint8_t* data = (uint8_t*)where;
    for(uint32_t i = 0; i < size; i++)
        data[i] = ReadUInt8();
}
