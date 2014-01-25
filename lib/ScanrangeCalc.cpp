#include "ScanrangeCalc.hpp"
#include <string.h>
#include <math.h>
#include <stdio.h>

CScanrangeCalc::CScanrangeCalc()
{
    ulScanShift = 7;
}

void CScanrangeCalc::InitializeTables()
{
    uint32_t ulScanShifted = 1 << ulScanShift;
    memset(pTablesCost, 0, sizeof(pTablesCost));
    memset(pTablesOffset, 0, sizeof(pTablesOffset));

    for(int32_t i = 0; i <= 20; ++i)
    {
        for(int32_t j = 0; j <= 20; ++j)
        {
            if (j >= (i / 2))
            {
                if (j <= (i * 2))
                {
                    pTablesOffset[20+i][20+j][0] = -1;
                    pTablesOffset[20+i][20+j][1] = -1;
                    pTablesOffset[20-i][20-j][0] = 1;
                    pTablesOffset[20-i][20-j][1] = 1;
                    pTablesOffset[20+i][20-j][0] = -1;
                    pTablesOffset[20+i][20-j][1] = 1;
                    pTablesOffset[20-i][20+j][0] = 1;
                    pTablesOffset[20-i][20+j][1] = -1;
                }
                else
                {
                    pTablesOffset[20+i][20+j][0] = 0;
                    pTablesOffset[20+i][20+j][1] = -1;
                    pTablesOffset[20-i][20-j][0] = 0;
                    pTablesOffset[20-i][20-j][1] = 1;
                    pTablesOffset[20+i][20-j][0] = 0;
                    pTablesOffset[20+i][20-j][1] = 1;
                    pTablesOffset[20-i][20+j][0] = 0;
                    pTablesOffset[20-i][20+j][1] = -1;
                }
            }
            else
            {
                pTablesOffset[20+i][20+j][0] = -1;
                pTablesOffset[20+i][20+j][1] = 0;
                pTablesOffset[20-i][20-j][0] = 1;
                pTablesOffset[20-i][20-j][1] = 0;
                pTablesOffset[20+i][20-j][0] = -1;
                pTablesOffset[20+i][20-j][1] = 0;
                pTablesOffset[20-i][20+j][0] = 1;
                pTablesOffset[20-i][20+j][1] = 0;
            }

            int32_t v4;
            if(i > j) v4 = i;
            else v4 = j;

            int16_t v1 = (int16_t)(sqrt(double(j*j + i*i)) * (double)ulScanShifted / (double)v4);
            pTablesCost[20+i][20+j] = v1;
            pTablesCost[20+i][20-j] = v1;
            pTablesCost[20-i][20+j] = v1;
            pTablesCost[20-i][20-j] = v1;
        }
    }

    pTablesCost[20][20] = 0;
}

bool CScanrangeCalc::SetCell(uint8_t x, uint8_t y, uint8_t height_origin, uint8_t height_cell)
{
    int16_t vision_previous = pTablesVision[x+pTablesOffset[x][y][0]][y+pTablesOffset[x][y][1]];
    uint16_t cost = pTablesCost[x][y];
    //if(bDiv2) cost -= 18;
    pTablesVision[x][y] = vision_previous - (height_cell - height_origin + cost);
    return (pTablesVision[x][y] >= 0);
}

void CScanrangeCalc::CalculateVision(int32_t x, int32_t y, uint16_t scanrange, uint8_t (*func_GetHeight)(int16_t,int16_t), bool (*func_CheckValid)(int16_t,int16_t))
{
    memset(pTablesVision, 0, sizeof(pTablesVision));
    if(!func_GetHeight) return;
    if(!func_CheckValid) return;

    uint16_t vision = scanrange;

    uint16_t vision2 = (1 << (ulScanShift - 1)) + (vision >> (8 - ulScanShift));

    int32_t genX = x - 20;
    int32_t genY = y - 20;
    uint8_t ht_origin = func_GetHeight(x, y);

	uint32_t result = 0;
    pTablesVision[20][20] = vision2;
    for(int32_t i = 1; i < 20; i++)
    {
        bool fdisp = false;
        for(int32_t j = -i; j < i+1; j++)
        {
            bDiv2 = false;
            if(func_CheckValid(genX+(20+j), genY+(20-i)) &&
               SetCell(20+j, 20-i, ht_origin, func_GetHeight(genX+(20+j), genY+(20-i))))
                fdisp = true;
            if(func_CheckValid(genX+(20+j), genY+(20+i)) &&
               SetCell(20+j, 20+i, ht_origin, func_GetHeight(genX+(20+j), genY+(20+i))))
                fdisp = true;
            bDiv2 = true;
            if(func_CheckValid(genX+(20-i), genY+(20+j)) &&
               SetCell(20-i, 20+j, ht_origin, func_GetHeight(genX+(20-i), genY+(20+j))))
                fdisp = true;
            if(func_CheckValid(genX+(20+i), genY+(20-j)) &&
               SetCell(20+i, 20-j, ht_origin, func_GetHeight(genX+(20+i), genY+(20-j))))
                fdisp = true;
        }

		if(!fdisp) break;
    }
}
