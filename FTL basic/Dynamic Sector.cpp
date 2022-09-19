
#include "Dynamic Sector.h"
#define ONE_BLOCK_SECTOR 32

DynamicSector::~DynamicSector () {
	delete[] mp;
}

void DynamicSector::result () {
	cout << endl << "���� Ƚ�� : " << writeCount << "ȸ �б�Ƚ�� : " << readCount << "ȸ ����Ƚ�� : " << eraseCount << "ȸ" << endl;
}

void DynamicSector::make_table (char sb, char sd) {
	init (8);             // �޸� ����.
//	init (1);

	if (sb == 's') {                                                                        // ���� mp
		int end_LSN = ((_msize (memory) / 4) - ((_msize (memory) / 4) / 10)) * 32;
		mp = new int[end_LSN];
	}
	else if (sb == 'b') {                                                                   // ��� mp
		int end_LSN = (_msize (memory) / 4) - ((_msize (memory) / 4) / 10);
		mp = new int[end_LSN];
	}

	//------------------------------------------------------------------------------------------------------

	if (sd == 'd') {                                                                        // ���� mp
		for (int i = 0; i < _msize (mp) / sizeof (int); i++)
			mp[i] = -1;
	}
	else if (sd == 's') {                                                                   // ���� mp
		for (int i = 0; i < _msize (mp) / sizeof (int); i++)
			mp[i] = i;
	}
}

void DynamicSector::FTL_write (int LSN, string data) {
	int tableMax = _msize (mp) / sizeof (int);
	int megabytes = (_msize (memory) / sizeof (int)) / 64;
	int end_PSN = (_msize (memory) / 4) * 32;

	if (nextWrite >= end_PSN)
		nextWrite = 0;

	if (nextWrite == end_PSN - 32) {		// ù GC 
		lastBlock = GC (nextWrite);
		mp[LSN] = nextWrite;
		Flash_write (BS_number (nextWrite, 'b'), BS_number (nextWrite, 's'), data);
		nextWrite++;
	}
	else if (lastBlock > 0 && nextWrite == lastBlock * 32) {		// n�� GC
		lastBlock = GC (nextWrite - 1);
		mp[LSN] = nextWrite;
		Flash_write (BS_number (nextWrite, 'b'), BS_number (nextWrite, 's'), data);
		nextWrite++;
	}
	else {		// ���� write
		mp[LSN] = nextWrite;
		Flash_write (BS_number (nextWrite, 'b'), BS_number (nextWrite, 's'), data);
		nextWrite++;
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

void DynamicSector::Print_table () {
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

void DynamicSector::test_table () {
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

void DynamicSector::FTL_read (int LSN) {
	cout << Flash_read (BS_number (mp[LSN], 'b'), BS_number (mp[LSN], 's')) << endl;
}

int DynamicSector::BS_number (int PSN, char choice) {
	int bNum = PSN / ONE_BLOCK_SECTOR;
	int sNum = PSN % ONE_BLOCK_SECTOR;

	if (choice == 's')      return sNum;
	else if (choice == 'b') return bNum;
}

int DynamicSector::GC (int swapBlock) {
	int end_PSN = (_msize (memory) / 4) * 32;       // �׳� ���ؼ� ����        
	int spareBlockNum = (_msize (memory) / 4) / 10; // GC �ؾ� �� ��� ����. �ϴ� �γ��� ���߿� ���￹��. 8mb ���� 51�� ������ ����� swap
	int tableMax = _msize (mp) / sizeof (int), emptyNum = 0;
	int read = nextWrite + 32, gcStart = nextWrite;
	string readData;                                        // read ������ ����

	testGC++;
	cout << "gc ������ " << testGC << endl;

	if (read  >= end_PSN)
		read = 0;

	for (int i = 0; i < spareBlockNum * 32; i++) {                      // ������� ��� ����ŭ �ݺ�
		for (int j = 0; j < tableMax; j++) {                            // ��ȿ ������ ã�������� �ݺ�
			if (mp[j] == read) {              // �ش� ������ �����Ͱ� mp�� �����ϸ�?
				readData = Flash_read (BS_number (read, 'b'), BS_number (read, 's'));
				Flash_write (BS_number (nextWrite, 'b'), BS_number (nextWrite, 's'), readData);
				mp[j] = nextWrite;				// mp ������Ʈ
				nextWrite++;
				break;
			}
		}
		read++;
		int eraseNum = 0;

		if ((i + 1) % 32 == 0) {      // swapBlock�� ������ ���Ͱ� �������� �۾� �� ��� �����
			if (swapBlock + 33 >= end_PSN) {
				gcStart = 0;
				Flash_erase (BS_number (i + gcStart, 'b'));
			}
			else if (i + gcStart >= end_PSN) {
				Flash_erase (eraseNum);
				eraseNum++;
			}
			else {
				gcStart = nextWrite + 1;
				Flash_erase (BS_number (i + gcStart, 'b'));
			}

			if (read + 1 >= end_PSN)
				read = 0;
			else
				read = swapBlock + 32 + 1;
		}
	}

	if ((swapBlock /32) + spareBlockNum >= end_PSN / 32)
		return 50;
	else
		return (swapBlock / 32) + spareBlockNum - 1;
}
