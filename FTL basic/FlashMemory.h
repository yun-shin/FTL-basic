
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
    ~FlashMemory ();                                            // �޸� ����
    void init (int megabytes);                                  // x megabytes �÷��� �޸� ����
    string Flash_write (int PBN, int PSN, string data);                      // ������ �б� ����
    string Flash_read (int PBN, int PSN);                                  // ������ ���� ����
    void Flash_erase (int PBN);                                // �ش� ��� �����
    void print ();                 

    Memory** memory;
    int writeCount = 0, eraseCount = 0, readCount = 0, updateCount = 0;
};