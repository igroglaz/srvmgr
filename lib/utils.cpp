#include "utils.hpp"
#include "stdint.h"
#include "..\config_new.h"

#include <cstdarg>
#include <cstdio>
#include <ctime>

using namespace std;

/*
    utils::Format: �������, ����������� sprintf(), �� ��� STL �����.

    ���������� ����������������� STL ������.

    format: ������
    ...: ��������� ��� �������
*/
std::string Format(const string format, ...)
{
    char line[2049];
    line[2048] = 0;
    va_list list;
    va_start(list, format);
    vsnprintf(line, 2048, format.c_str(), list);
    va_end(list);

    return string(line);
}

/*
    utils::Explode: ������� ��� ��������� ������ �� ���������.

    ���������� ������ �� ��������� ��������.

    what: ����������� ������
    separator: �����������
*/
vector<string> Explode(const string& what, const string& separator)
{
    string curstr;
    vector<string> retval;
    for(size_t i = 0; i < what.length(); i++)
    {
        if(what.find(separator, i) == i)
        {
            retval.push_back(curstr);
            curstr.assign("");
            i += separator.length()-1;
            continue;
        }

        curstr += what[i];
    }
    retval.push_back(curstr);
    return retval;
}

/*
    utils::TrimLeft, utils::TrimRight, utils::Trim: �������� ���������� ������� � ������ ������ (TrimLeft), � ����� (TrimRight)
        ��� � ���, � ��� (Trim).

    ���������� ���������� ������.

    what: ���������� ������
    callback: �������, ������������ ���������� �������. ��. IsWhitespace
*/
string TrimLeft(const string& what, bool (callback)(char))
{
    string ret = what;
    for(string::iterator i = ret.begin(); i != ret.end(); ++i)
    {
        if(callback((*i)))
            continue;
        ret.erase(ret.begin(), i);
        return ret;
    }
    return "";
}

string TrimRight(const string& what, bool (callback)(char))
{
    string ret = what;
    for(string::reverse_iterator i = ret.rbegin(); i != ret.rend(); ++i)
    {
        if(callback((*i)))
            continue;
        ret.erase(i.base(), ret.end());
        return ret;
    }
    return "";
}

string Trim(const string& what, bool (callback)(char))
{
    return TrimRight(TrimLeft(what, callback));
}

/*
    utils::IsWhitespace: ��������������� ������� ��� Trim*

    ���������� true, ���� ������ �������� �������� � ��� ����� ��������.

    what: ����������� ������.
*/
bool IsWhitespace(char what)
{
    switch((unsigned char)what)
    {
        case ' ':
        case '\r':
        case '\n':
        case '\t':
        case 0xFF:
            return true;
        default:
            return false;
    }
}

/*
    utils::ToLower, utils::ToUpper: �������������� ������ � ������� ��� ������ �������.

    ���������� ��������������� ������.

    what: ����������������� ������.
*/
string ToLower(const string& what)
{
    string ret = what;
    for(string::iterator i = ret.begin(); i != ret.end(); ++i)
        (*i) = tolower((*i));
    return ret;
}

string ToUpper(const string& what)
{
    string ret = what;
    for(string::iterator i = ret.begin(); i != ret.end(); ++i)
        (*i) = toupper((*i));
    return ret;
}

#include <fstream>

/*
    utils::FileExists: �������� �� ������������� �����.

    ���������� true, ���� ���� ����������.

    filename: �������� ������������ �����.
*/
bool FileExists(const string& filename)
{
    ifstream f_temp;
    f_temp.open(filename.c_str(), ios::in | ios::binary);
    if(!f_temp.is_open()) return false;
    f_temp.close();
    return true;
}

/*
    utils::Basename: ������� ���� � �����, ������� ������ ��� ��������.

    ������������ ��������: ���������� ������.

    filename: ������ �������� �����.
*/
string Basename(const string& filename)
{
    string ret = FixSlashes(filename);
    uint32_t where = ret.find_last_of('/');
    if(where == string::npos) return ret;
    ret.erase(0, where+1);
    return ret;
}

/*
    utils::FixSlashes: ������� ���� � ����� POSIX-����������� (�.�. � ������������� ������� ������ backslash, ������������� � DOS/Windows)

    ������������ ��������: ������������ ������.

    filename: ������ �������� �����.
*/
string FixSlashes(const string& filename)
{
    string ret = filename;
    for(string::iterator i = ret.begin(); i != ret.end(); ++i)
        if((*i) == '\\') (*i) = '/';
    return ret;
}

/*
    utils::TruncateSlashes: ������� ������������� ����� (����. main//graphics/mainmenu//menu_.bmp).

    ���������� ������������ ������.

    filename: ������ �������� �����.
*/
string TruncateSlashes(const string& filename)
{
    string ret = filename;
    char lastchar = 0;
    char thischar = 0;
    for(string::iterator i = ret.begin(); i != ret.end(); ++i)
    {
        thischar = (*i);
        if((thischar == '/' || thischar == '\\') &&
           (lastchar == '/' || lastchar == '\\'))
        {
            ret.erase(i);
            i--;
        }
        lastchar = thischar;
    }
    return ret;
}

unsigned long StrToInt(const string& what)
{
	unsigned long retval;
	sscanf(what.c_str(), "%u", &retval);
	return retval;
}

float StrToFloat(const string& what)
{
	float retval;
	sscanf(what.c_str(), "%f", &retval);
	return retval;
}

bool CheckInt(const string& what)
{
	for(size_t i = 0; i < what.length(); i++)
		if(what[i] < 0x30 || 0x39 < what[i]) return false;
	return true;
}

bool CheckBool(const string& what)
{
    string wh2 = ToLower(Trim(what));
    if(wh2 == "true" || wh2 == "false" || wh2 == "yes" || wh2 == "no" || wh2 == "y" || wh2 == "n" || wh2 == "0" || wh2 == "1")
        return true;
    return false;
}

bool StrToBool(const string& what)
{
	string cr = Trim(ToLower(what));
	if(cr == "yes" || cr == "true" || cr == "1" || cr == "y")
		return true;
	return false;
}

bool CheckIP(std::string addr)
{
	std::vector<std::string> addr2 = Explode(addr, ".");
	if(!((addr2.size() == 4) && CheckInt(addr2[0]) && CheckInt(addr2[1]) && CheckInt(addr2[2]) && CheckInt(addr2[3])))
        return false;
    uint32_t oct[4];
    oct[0] = StrToInt(addr2[0]);
    oct[1] = StrToInt(addr2[1]);
    oct[2] = StrToInt(addr2[2]);
    oct[3] = StrToInt(addr2[3]);
    if(oct[0] > 255 || oct[1] > 255 || oct[2] > 255 || oct[3] > 255)
        return false;
    return true;
}

bool CheckFloat(const string& what)
{
	for(size_t i = 0; i < what.length(); i++)
		if((what[i] < 0x30 || 0x39 < what[i]) && what[i] != '.') return false;
	return true;
}

#include "../srvmgr.h"

void _stdcall Printf(const char* format, ...)
{
    char* line = NULL;
    va_list list;
    va_start(list, format);
    size_t linesize = (vsnprintf(NULL, 0, format, list) + 1);
    line = new char[linesize + 1];
    if(!line) return;
    line[linesize] = 0;
    vsnprintf(line, linesize, format, list);
    va_end(list);

    time_t t;
    struct tm *tm;
    time(&t);
    tm = localtime(&t);

	if(Config::ServerStarted) print_log(line);
	else log_format("%s\n", line);

	delete[] line;
}

unsigned long HexToInt(const string& what)
{
	unsigned int retval;
	sscanf(what.c_str(), "%X", &retval);
	return retval;
}

bool CheckHex(const string& what)
{
	for(size_t i = 0; i < what.length(); i++)
		if((what[i] < 0x30 || 0x39 < what[i]) && !(what[i] >= 'A' && what[i] <= 'F') && !(what[i] >= 'a' && what[i] <= 'f')) return false;
	return true;
}

std::vector<std::string> ParseSpaceDelimited(std::string message, bool comments)
{
    std::vector<std::string> argv;
    std::string current = "";
    bool in_encaps = false;
    bool notwhitespace = false;
    for(size_t i = 0; i < message.length(); i++)
    {
        char ch = message[i];
        if(!notwhitespace && // i.e. whitespaces only up to this point
           (ch == '\t' || ch == ' ')) continue; // ignore it

        if(ch != '\t' && ch != ' ') // not a whitespace, i.e. beginning of parameter
            notwhitespace = true;

        if(ch == '"')
        {
            in_encaps = !in_encaps;
            if(in_encaps) notwhitespace = true;
            continue;
        }

        if(ch == '\\' && i+1 < message.length())
        {
            notwhitespace = true;
            current.push_back(message[i+1]);
            i++;
            continue;
        }

        if(in_encaps)
        {
            current.push_back(ch);
            continue;
        }

        if(ch == ' ' || ch == '\t')
        {
            notwhitespace = false;
            argv.push_back(current);
            current.clear();
            continue;
        }
        else if(ch == '#' && comments)
            break;

        current.push_back(ch);
    }

    if(current.length()) argv.push_back(current);
    current.clear();

    return argv;
}

void tokenize(std::string const &str, const char delim, std::vector<std::string> &out)
{
    // construct a stream from the string
    std::stringstream ss(str);

    std::string s;
    while (std::getline(ss, s, delim)) {
        out.push_back(s);
    }
}