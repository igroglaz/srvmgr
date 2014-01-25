#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include <string>
#include <vector>
#include <fstream>
#include <stdint.h>

class FileArchive
{
public:
    struct Record
    {
        uint32_t Flags;
        std::string Name;

        uint32_t Length;
        uint32_t Offset;

        std::vector<Record> Children;
        std::string ResourceName;

        Record()
        {
            Flags = 2;
            Name = "";
            Length = 0;
            Offset = 0;
        }
    };

    bool Open(std::string filename, std::string directory);
    const Record& GetRecord(std::string filename);

private:
    std::string myFileName;
    std::string myDirectoryName;

    std::vector<Record> myRoot;
};

extern FileArchive Archives;

class File
{
public:
    File();
    bool Open(std::string filename);
    void Close();

    uint32_t Read(void* destination, uint32_t size);
    void Seek(uint32_t position);
    uint32_t GetPosition();
    uint32_t GetLength();
    uint32_t CalcCRC();

    bool GetLine(std::string& line);

private:
    bool myOpen;
    std::string myName;
    uint32_t myLength;
    uint32_t myPosition;
    uint32_t myOffset;

    std::ifstream myStream;
};

#endif // FILE_H_INCLUDED
