
#include "FlashMemory.h"
#define ONE_BLOCK_SECTOR 32

Memory::Memory () {
    MainSector = "-1";
}

FlashMemory::~FlashMemory () {
 /*   for (int i = 0; i < 10; i++)
        delete[] memory[i];
    delete[] memory;*/
}

void FlashMemory::init (int megabytes) {
    memory = new Memory *[megabytes * 64];                 // 1block = 32sector
        for(int i=0; i< megabytes * 64; i++)
            memory[i] = new Memory [ONE_BLOCK_SECTOR];
}

void FlashMemory::print () {
    for (int i = 0; i < _msize (memory)/sizeof (int); i++) {                                 // 1block = 32sector
        for (int j = 0; j < 32; j++)
         cout << memory[i][j].MainSector << " " << i <<endl;
    }
}

string FlashMemory::Flash_write (int PBN, int PSN, string data) {
    if (memory[PBN][PSN].MainSector != "-1") {                      // 현재 섹터가 비어있으면
 //       cout << "1" << endl;
        writeCount++;
        return "is full";
    }
    else {
        memory[PBN][PSN].MainSector = data;
 //       cout << "PSN : " << (PBN * ONE_BLOCK_SECTOR) + PSN << "\t Data : " << memory[PBN][PSN].MainSector << endl;
        writeCount++;
        return "done";
    }
}

string FlashMemory::Flash_read (int PBN, int PSN) {
//    cout << "PSN : " << PBN * PSN << "  PSN data : " << memory[PBN][PSN].MainSector << endl;
    readCount++;
    return memory[PBN][PSN].MainSector;
}

void FlashMemory::Flash_erase (int PBN) {
//    cout << PBN << endl;
    for (int i = 0; i < ONE_BLOCK_SECTOR; i++) {
        memory[PBN][i].MainSector = "-1";
    }

    eraseCount++;
}