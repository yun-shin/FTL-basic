
#pragma once
#include <fstream> // 파일 입출력
#include <string>
#include "Dynamic Sector.h"
#include "Static Sector.h"
#include "Dynamic Block.h"
#include "Static Block.h"

class fuction {
public:
    void readFile (int choice);
    void CommandCheck (string CommandType, int LSN, int choice);

    FlashMemory f;
    DynamicSector ds;
    StaticSector ss;
    DynamicBlock db;
    StaticBlock sb;
    int queue = 0;
};