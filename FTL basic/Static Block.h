
#pragma once
#include "FlashMemory.h"

class StaticBlock : FlashMemory {
public:
    ~StaticBlock ();
    string FTL_read (int LSN);                        // ftl �б�
    void FTL_write (int LSN, string data);            // ftl ����
    void make_table (char sb, char sd);                  // ���̺� ����
    void full_memory ();                            // ���̺� ��� ä���        �ʾ�
    void Print_table ();                            // ���̺� ���
    int GC (int swapBlock);
    int BS_number (int PSN, char choice);           // PBN, PSN ���
    void test_table ();                             // �Ѻ�� ���               �ʾ�
    void result ();                                 // ������.

    int* mp, nextWrite = 0, lastBlock = -1;
    int testGC = 0;
};