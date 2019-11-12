#pragma once

#include <string>

void Python_Init();
void Python_Quit();
void Python_ExecuteFile(std::string filename);
void Python_CallMethod(std::string method);