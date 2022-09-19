
#pragma once
#include <iostream>

using namespace std;

class Memory {
public:
    Memory ();

    string MainSector = "-1";
};


class FlashMemory {
public:
    ~FlashMemory ();                                            // 메모리 해제
    void init (int megabytes);                                  // x megabytes 플레시 메모리 생성
    string Flash_write (int PBN, int PSN, string data);                      // 물리적 읽기 수행
    string Flash_read (int PBN, int PSN);                                  // 물리적 쓰기 수행
    void Flash_erase (int PBN);                                // 해당 블록 지우기
    void print ();                 

    Memory** memory;
    int writeCount = 0, eraseCount = 0, readCount = 0, updateCount = 0;
};