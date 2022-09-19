
#include "Static Block.h"
#define ONE_BLOCK_SECTOR 32

StaticBlock::~StaticBlock () {
	delete[] mp;
}

void StaticBlock::result () {
	cout << endl << "쓰기 횟수 : " << writeCount << "회 읽기횟수 : " << readCount << "회 삭제횟수 : " << eraseCount << "회" << endl;
}

void StaticBlock::make_table (char sb, char sd) {
	init (8);             // 메모리 생성.
//	init (1);

	if (sb == 's') {                                                                        // 섹터 mp
		int end_LSN = ((_msize (memory) / 4) - ((_msize (memory) / 4) / 10)) * 32;
		mp = new int[end_LSN];
	}
	else if (sb == 'b') {                                                                   // 블록 mp
		int end_LSN = (_msize (memory) / 4) - ((_msize (memory) / 4) / 10);
		mp = new int[end_LSN];
	}

	//------------------------------------------------------------------------------------------------------

	if (sd == 'd') {                                                                        // 동적 mp
		for (int i = 0; i < _msize (mp) / sizeof (int); i++)
			mp[i] = -1;
	}
	else if (sd == 's') {                                                                   // 정적 mp
		for (int i = 0; i < _msize (mp) / sizeof (int); i++)
			mp[i] = i;

		nextWrite = _msize (mp) / sizeof (int);
	}
}

void StaticBlock::FTL_write (int LSN, string data) {
	int tableMax = _msize (mp) / sizeof (int);
	int megabytes = (_msize (memory) / sizeof (int)) / 64;
	int end_PSN = (_msize (memory) / 4) * 32;
	string readData;

	if (nextWrite == (end_PSN / 32) - 1) {		// 첫 GC 
		//cout << "삐빅 mp가 가득찼어요. 헤으응..." << endl;

		lastBlock = GC (nextWrite);
	}
	else if (lastBlock > 0 && nextWrite + 1 == lastBlock) {		// n번 GC
		lastBlock = GC (nextWrite);
	}

	if (nextWrite >= (end_PSN / 32) - 1)
		nextWrite = 0;

	if (mp[LSN / ONE_BLOCK_SECTOR] != -1 && Flash_read (mp[LSN / ONE_BLOCK_SECTOR], BS_number (LSN, 's')) != "-1") {		// 블록 overwrite
//		cout << "-------------------------------------------------------" << endl;
		for (int i = 0; i < ONE_BLOCK_SECTOR; i++) {
			readData = Flash_read (nextWrite, i);

			if (BS_number (LSN, 's') == i && readData != "-1")				// 업데이트 섹터는 데이터 입력.
				Flash_write (nextWrite + 1, i, data);
			else if (readData != "-1")										// 기존 데이터 복사.
				Flash_write (nextWrite + 1, i, readData);
		}

		nextWrite++;
		mp[LSN / ONE_BLOCK_SECTOR] = nextWrite;
	}
	else if (mp[LSN / ONE_BLOCK_SECTOR] == -1) {					// 첫 write
		nextWrite++;
		Flash_write (nextWrite, BS_number (LSN, 's'), data);
		mp[LSN / ONE_BLOCK_SECTOR] = nextWrite;
	}
	else if (mp[LSN / ONE_BLOCK_SECTOR] == LSN / ONE_BLOCK_SECTOR && Flash_read (mp[LSN / ONE_BLOCK_SECTOR], BS_number (LSN, 's')) == "-1") {					// 첫 write 비어잇고, psn = lsn 이면
		Flash_write (mp[LSN / ONE_BLOCK_SECTOR], BS_number (LSN, 's'), data);
	}
	else if (mp[LSN / ONE_BLOCK_SECTOR] != -1) {					// 동일 블록 다른 offset write
		Flash_write (mp[LSN / ONE_BLOCK_SECTOR], BS_number (LSN, 's'), data);
	}
	else
		exit (0);
}

/*
void DynamicSector::full_memory () {
	int tableMax = _msize (mp) / sizeof (int);

	for (int i = 0; i < tableMax; i++) {
		mp[i] = i;
		Flash_write (BS_number (i, 'b'), BS_number (i, 's'), "a");
	}
	cout << "done." << endl;
	writeCount = 0;
	readCount = 0;
	eraseCount = 0;
}*/

void StaticBlock::Print_table () {
	int tableMax = _msize (mp) / sizeof (int), count = 1;

	for (int i = 0; i < tableMax; i++) {
		cout.width (10);
		cout << i << " : " << mp[i];
		if (count % 5 == 0)
			cout << endl;
		count++;
	}
	cout << endl;
}

void StaticBlock::test_table () {
	int count = 1;
	int swapBlock = (_msize (mp) / sizeof (int)) - 32;

	for (int i = 0; i < 32; i++) {
		cout.width (10);
		cout << i << " : " << mp[swapBlock + i];
		if (count % 5 == 0)
			cout << endl;
		count++;
	}
	cout << endl;
}

string StaticBlock::FTL_read (int LSN) {
//	cout << Flash_read (BS_number (mp[LSN], 'b'), BS_number (mp[LSN], 's')) << endl;
	return Flash_read (BS_number (mp[LSN], 'b'), BS_number (mp[LSN], 's'));
}

int StaticBlock::BS_number (int PSN, char choice) {
	int bNum = PSN / ONE_BLOCK_SECTOR;
	int sNum = PSN % ONE_BLOCK_SECTOR;

	if (choice == 's')      return sNum;
	else if (choice == 'b') return bNum;
}

int StaticBlock::GC (int swapBlock) {
	int end_PSN = (_msize (memory) / 4) * 32;       // 그냥 편해서 넣음        
	int spareBlockNum = (_msize (memory) / 4) / 10; // GC 해야 할 블록 숫자. 일단 싸놓고 나중에 지울예정. 8mb 기준 51개 마지막 블록은 swap
	int tableMax = _msize (mp) / sizeof (int), emptyNum = 0;
	int read = nextWrite + 1, gcStart = nextWrite + 1, pass = 0;
	string readData;                                        // read 데이터 버퍼

	testGC++;
	nextWrite++;
	cout << "gc 실행함 " << testGC << endl;

	if (read >= end_PSN)
		read = 0;

	for (int i = 0; i < spareBlockNum; i++) {                      // 비워야할 블록 수만큼 반복
		for (int j = 0; j < tableMax; j++) {                            // 유효 데이터 찾을때까지 반복
			if (mp[j] == read) {						 // 해당 블록의 데이터가 mp에 존재하면?
				for (int k = 0; k < ONE_BLOCK_SECTOR; k++) {		// 해당 블록 swapBlock에 저장.
					if (Flash_read (j, k) != "-1") {
						readData = Flash_read (read, k);
						pass = 1;

						if (readData != "-1")										// readData 가 존재하면 실행.
							Flash_write (nextWrite, k, readData);
					}				
				}

				if (pass == 1) {
					nextWrite++;
					pass = 0;
				}

				if (nextWrite >= end_PSN / 32)
					nextWrite = 0;

				mp[j] = nextWrite;				// mp 업데이트
				break;
			}
		}
		read++;

		if (gcStart == (end_PSN / 32) - 1) {
			gcStart = 0;
			Flash_erase (gcStart + i);
		}
		else if (gcStart + i == end_PSN / 32) {
			gcStart = 0;
			Flash_erase (gcStart);
		}
		else {
			Flash_erase (gcStart + i);
		}

		if (gcStart + i == 50)
			cout << endl;
	}

	int init = swapBlock;
	for (int i = 0; i < spareBlockNum; i++) {				// 피해자 블록 mp 초기화
		for (int j = 0; j < tableMax; j++) {
			if (mp[j] == init + i) {
				if (init + i >= (end_PSN / 32) - 1)
					init = 0;
				mp[j] = -1;
			}
		}
	}

	cout << "gc 실행함 " << testGC << endl;
	if (swapBlock + spareBlockNum >= end_PSN / 32)
		return 50;
	else
		return swapBlock + spareBlockNum + 1;
}

