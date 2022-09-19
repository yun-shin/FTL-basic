
#include "Dynamic Block.h"
#define ONE_BLOCK_SECTOR 32

/*
void DynamicSector::menu () {
	string Operation;
	int number;
	char data;

	cout << "write or read after init" << endl;
	cin >> Operation >> megabytes;
	if (!Operation.compare ("init") && cin.fail () == false && megabytes >= 0) {                   // x megabytes 플레시 메모리 생성
		f.init (megabytes);
		make_table ('s');

		while (true) {
			cin >> Operation >> number;
			if (!Operation.compare ("read") && cin.fail () == false && number >= -1)                // 읽기 명령
				FTL_read (number);
			else if (!Operation.compare ("write") && cin.fail () == false && number >= -1) {        // 쓰기 명령
				cin >> data;
				FTL_write (number, data);
			}
			else if (!Operation.compare ("full") && cin.fail () == false) {
				full_memory ();
			}
			else if (!Operation.compare ("print") && cin.fail () == false) {
				Print_table ();
			}
			else {
				cout << "재입력" << endl;
				cin.clear ();                           // 에러비트 초기화
				cin.ignore (1000, '\n');
			}
		}
	}
	else {
		cout << "재입력" << endl;
		cin.clear ();                           // 에러비트 초기화
		cin.ignore (1000, '\n');
		return;
	}
}*/


DynamicBlock::~DynamicBlock () {
	delete[] mp;
}

void DynamicBlock::result () {
	cout << endl << "쓰기 횟수 : " << writeCount << "회 읽기횟수 : " << readCount << "회 삭제횟수 : " << eraseCount << "회" << endl;
}

void DynamicBlock::make_table (char sb, char sd) {
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
	}
}

void DynamicBlock::FTL_write (int LSN, string data) {
	int tableMax = _msize (mp) / sizeof (int);
	int megabytes = (_msize (memory) / sizeof (int)) / 64;
	int end_PSN = (_msize (memory) / 4) * 32;
	string readData;

	if (nextWrite == (end_PSN / 32) - 1) {		// 첫 GC 
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

			if (BS_number (LSN, 's') == i && readData != "-1")
				Flash_write (nextWrite + 1, i, data);
			else if (readData != "-1")
				Flash_write (nextWrite + 1, i, readData);
		}

		nextWrite++;
		mp[LSN / ONE_BLOCK_SECTOR] = nextWrite;
	}
	else if (mp[LSN / ONE_BLOCK_SECTOR] == -1) {					// 첫 write
		nextWrite++;
		Flash_write (nextWrite, BS_number(LSN, 's') , data);
		mp[LSN / ONE_BLOCK_SECTOR] = nextWrite;
	}
	else if (mp[LSN / ONE_BLOCK_SECTOR] != -1) {					// 동일 블록 다른 offset write
		Flash_write (mp[LSN / ONE_BLOCK_SECTOR], BS_number (LSN, 's'), data);
	}
	else {
		exit (0);
	}
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

void DynamicBlock::Print_table () {
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

void DynamicBlock::test_table () {
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

void DynamicBlock::FTL_read (int LSN) {
	cout << Flash_read (BS_number (mp[LSN], 'b'), BS_number (mp[LSN], 's')) << endl;
}

int DynamicBlock::BS_number (int PSN, char choice) {
	int bNum = PSN / ONE_BLOCK_SECTOR;
	int sNum = PSN % ONE_BLOCK_SECTOR;

	if (choice == 's')      return sNum;
	else if (choice == 'b') return bNum;
}

int DynamicBlock::GC (int swapBlock) {
	int end_PSN = (_msize (memory) / 4) * 32;       // 그냥 편해서 넣음        
	int spareBlockNum = (_msize (memory) / 4) / 10; // GC 해야 할 블록 숫자. 일단 싸놓고 나중에 지울예정. 8mb 기준 51개 마지막 블록은 swap
	int tableMax = _msize (mp) / sizeof (int), emptyNum = 0;
	int read = nextWrite + 1, gcStart = nextWrite + 1;
	string readData;                                        // read 데이터 버퍼

	testGC++;
	nextWrite++;
	cout << "gc 실행함 " << testGC << endl;

	if (read >= end_PSN)
		read = 0;

	for (int i = 0; i < spareBlockNum; i++) {                      // 비워야할 블록 수만큼 반복
		for (int j = 0; j < tableMax; j++) {                            // 유효 데이터 찾을때까지 반복
			if (mp[j] == read) {									// 해당 블록의 데이터가 mp에 존재하면?
				for (int k = 0; k < ONE_BLOCK_SECTOR; k++) {		// 해당 블록 swapBlock에 저장.
					readData = Flash_read (read, k);

					if (readData != "-1")										// readData 가 존재하면 실행.
						Flash_write (nextWrite, k, readData);
				}

				nextWrite++;

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
		return 49;
	else
		return swapBlock + spareBlockNum + 1;
}
