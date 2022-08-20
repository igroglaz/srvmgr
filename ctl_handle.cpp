#include "zxmgr.h"
#include <windows.h>
#include <stdio.h>
#include "srvmgr.h"
#include <fstream>
#include <vector>
#include <string>
#include "lib/utils.hpp"
#include "cheat_codes_new.h"
#include "config_new.h"

#undef SendMessage

extern bool exiting;
extern char ctl_dir[BUF_MAX];

std::vector<std::string> get_lines(std::string filename)
{
    std::vector<std::string> vs;
    std::ifstream f(filename.c_str());
    while (!f.eof())
    {
        vs.push_back(std::string());
        std::getline(f, vs.back());
    }
    return vs;
}

DWORD WINAPI ThreadProc(LPVOID lpParam)
{ 
    WIN32_FIND_DATA fd;
    HANDLE h;

    std::string mask = Config::ControlDirectory;
    mask += "\\*";

    Sleep(5000);

    while (!exiting)
    {
        h = FindFirstFile(mask.c_str(), &fd);
        if (h != INVALID_HANDLE_VALUE)
        {
            bool found = true;
            while (found)
            {
                if (fd.cFileName[0] != '.')
                {
                    std::string fileToDelete = Config::ControlDirectory+"\\"+std::string(fd.cFileName);

                    std::string prefix = fd.cFileName;
                    if (prefix.find(".") != prefix.npos)
                    {
                        prefix = std::string(prefix, 0, prefix.find("."));
                    }

                    bool identified = true;
                    if (prefix == "say")
                    {
                        std::vector<std::string> ll = get_lines(fileToDelete);
                        if (ll.size())
                        {
                            from_encoding(ll[0]);
                            Printf("[ctl] %s", ll[0].c_str());
                            zxmgr::SendMessage(NULL, ll[0].c_str());
                        }
                    }
                    else if (prefix == "cheat")
                    {
                        std::vector<std::string> ll = get_lines(fileToDelete);
                        std::string caster = "";
                        std::string command = "";
                        if (ll.size() >= 1)
                            from_encoding(ll[0]);
                        if (ll.size() == 1)
                            command = ll[0];
                        else if(ll.size() > 1)
                        {
                            from_encoding(ll[1]);
                            caster = ll[0];
                            command = ll[1];
                        }

                        if(caster.length())
                        {
                            byte* pp = zxmgr::FindByNickname(caster.c_str());
                            if(pp)
                            {
                                Printf("[ctl] %s: %s", caster.c_str(), command.c_str());
                                RunCommand(NULL, pp, command.c_str(), 0xFFFFFFFF, false);
                            }
                        }
                        else
                        {
                            Printf("[ctl] <server>: %s", command.c_str());
                            RunCommand(NULL, NULL, command.c_str(), 0xFFFFFFFF, true);
                        }
                    }
                    else
                    {
                        identified = false;
                    }

                    if (identified)
                    {
                        DeleteFile(fileToDelete.c_str());
                    }
                }
                found = FindNextFile(h, &fd) != 0;
            }
            FindClose(h);

        }

        Sleep(100);
    }

    return 0; 
} 

