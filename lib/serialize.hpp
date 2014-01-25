#ifndef BINARYSTREAM_HPP_INCLUDED
#define BINARYSTREAM_HPP_INCLUDED

#include <string>
#include <vector>
#include <stdint.h>

class BinaryStream
{
public:
    // base
    BinaryStream();
    ~BinaryStream();

    void Reset();

    // save, load functions
    bool LoadFromFile(std::string filename);
    bool SaveToFile(std::string filename);

    bool LoadFromStream(BinaryStream& stream, uint32_t size);
    void SaveToStream(BinaryStream& stream);

    //bool LoadFromResource(std::string filename);

    // seek
    uint32_t Seek(uint32_t position);
    bool EndOfStream();
    uint32_t GetLength();
    uint32_t GetPosition();

    // read functions
    uint8_t ReadUInt8();
    int8_t ReadInt8() { return (int8_t)ReadUInt8(); }
    uint16_t ReadUInt16();
    int16_t ReadInt16() { return (int16_t)ReadUInt16(); }
    uint32_t ReadUInt32();
    int32_t ReadInt32() { return (int32_t)ReadUInt32(); }
    double ReadDouble();
    float ReadFloat();

    std::string ReadString();                       // string with 16-bit size (almost everywhere)
    std::string ReadBigString();                    // string with 32-bit size (packets, server list)
    std::string ReadSmallString();                  // string with 8-bit size (data.bin headers)
    std::string ReadFixedString(uint32_t size);     // string with fixed size (example: 32-byte nickname in a2c)

    void ReadData(void* where, uint32_t size);

    // write functions
    void WriteUInt8(uint8_t);
    void WriteInt8(int8_t what) { WriteUInt8((uint8_t)what); }
    void WriteUInt16(uint16_t);
    void WriteInt16(int16_t what) { WriteUInt16((uint16_t)what); }
    void WriteUInt32(uint32_t);
    void WriteInt32(int32_t what) { WriteUInt32((uint32_t)what); }
    void WriteDouble(double what);
    void WriteFloat(float what);

    void WriteString(std::string);
    void WriteBigString(std::string);
    void WriteSmallString(std::string);
    void WriteFixedString(std::string, uint32_t size);

    void WriteData(void* what, uint32_t size);

    // special
    std::vector<uint8_t>& GetBuffer() { return myBuffer; }  // for crypt functions ONLY

private:
    std::vector<uint8_t> myBuffer;
    uint32_t myGetPos;
    uint32_t mySetPos;
};

#endif // BINARYSTREAM_HPP_INCLUDED
