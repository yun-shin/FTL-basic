
#pragma once
#include "FlashMemory.h"

class StaticBlock : FlashMemory {
public:
    ~StaticBlock ();
    string FTL_read (int LSN);                        // ftl 읽기
    void FTL_write (int LSN, string data);            // ftl 쓰기
    void make_table (char sb, char sd);                  // 테이블 생성
    void full_memory ();                            // 테이블 모두 채우기        필없
    void Print_table ();                            // 테이블 출력
    int GC (int swapBlock);
    int BS_number (int PSN, char choice);           // PBN, PSN 계산
    void test_table ();                             // 한블록 출력               필없
    void result ();                                 // 결과출력.

    int* mp, nextWrite = 0, lastBlock = -1;
    int testGC = 0;
};