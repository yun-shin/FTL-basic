
#include "Static Block.h"
#define ONE_BLOCK_SECTOR 32

StaticBlock::~StaticBlock () {
	delete[] mp;
}

void StaticBlock::result () {
	cout << endl << "���� Ƚ�� : " << writeCount << "ȸ �б�Ƚ�� : " << readCount << "ȸ ����Ƚ�� : " << eraseCount << "ȸ" << endl;
}

void StaticBlock::make_table (char sb, char sd) {
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

		nextWrite = _msize (mp) / sizeof (int);
	}
}

void StaticBlock::FTL_write (int LSN, string data) {
	int tableMax = _msize (mp) / sizeof (int);
	int megabytes = (_msize (memory) / sizeof (int)) / 64;
	int end_PSN = (_msize (memory) / 4) * 32;
	string readData;

	if (nextWrite == (end_PSN / 32) - 1) {		// ù GC 
		//cout << "�ߺ� mp�� ����á���. ������..." << endl;

		lastBlock = GC (nextWrite);
	}
	else if (lastBlock > 0 && nextWrite + 1 == lastBlock) {		// n�� GC
		lastBlock = GC (nextWrite);
	}

	if (nextWrite >= (end_PSN / 32) - 1)
		nextWrite = 0;

	if (mp[LSN / ONE_BLOCK_SECTOR] != -1 && Flash_read (mp[LSN / ONE_BLOCK_SECTOR], BS_number (LSN, 's')) != "-1") {		// ��� overwrite
//		cout << "-------------------------------------------------------" << endl;
		for (int i = 0; i < ONE_BLOCK_SECTOR; i++) {
			readData = Flash_read (nextWrite, i);

			if (BS_number (LSN, 's') == i && readData != "-1")				// ������Ʈ ���ʹ� ������ �Է�.
				Flash_write (nextWrite + 1, i, data);
			else if (readData != "-1")										// ���� ������ ����.
				Flash_write (nextWrite + 1, i, readData);
		}

		nextWrite++;
		mp[LSN / ONE_BLOCK_SECTOR] = nextWrite;
	}
	else if (mp[LSN / ONE_BLOCK_SECTOR] == -1) {					// ù write
		nextWrite++;
		Flash_write (nextWrite, BS_number (LSN, 's'), data);
		mp[LSN / ONE_BLOCK_SECTOR] = nextWrite;
	}
	else if (mp[LSN / ONE_BLOCK_SECTOR] == LSN / ONE_BLOCK_SECTOR && Flash_read (mp[LSN / ONE_BLOCK_SECTOR], BS_number (LSN, 's')) == "-1") {					// ù write ����հ�, psn = lsn �̸�
		Flash_write (mp[LSN / ONE_BLOCK_SECTOR], BS_number (LSN, 's'), data);
	}
	else if (mp[LSN / ONE_BLOCK_SECTOR] != -1) {					// ���� ��� �ٸ� offset write
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
	int end_PSN = (_msize (memory) / 4) * 32;       // �׳� ���ؼ� ����        
	int spareBlockNum = (_msize (memory) / 4) / 10; // GC �ؾ� �� ��� ����. �ϴ� �γ��� ���߿� ���￹��. 8mb ���� 51�� ������ ����� swap
	int tableMax = _msize (mp) / sizeof (int), emptyNum = 0;
	int read = nextWrite + 1, gcStart = nextWrite + 1, pass = 0;
	string readData;                                        // read ������ ����

	testGC++;
	nextWrite++;
	cout << "gc ������ " << testGC << endl;

	if (read >= end_PSN)
		read = 0;

	for (int i = 0; i < spareBlockNum; i++) {                      // ������� ��� ����ŭ �ݺ�
		for (int j = 0; j < tableMax; j++) {                            // ��ȿ ������ ã�������� �ݺ�
			if (mp[j] == read) {						 // �ش� ����� �����Ͱ� mp�� �����ϸ�?
				for (int k = 0; k < ONE_BLOCK_SECTOR; k++) {		// �ش� ��� swapBlock�� ����.
					if (Flash_read (j, k) != "-1") {
						readData = Flash_read (read, k);
						pass = 1;

						if (readData != "-1")										// readData �� �����ϸ� ����.
							Flash_write (nextWrite, k, readData);
					}				
				}

				if (pass == 1) {
					nextWrite++;
					pass = 0;
				}

				if (nextWrite >= end_PSN / 32)
					nextWrite = 0;

				mp[j] = nextWrite;				// mp ������Ʈ
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
	for (int i = 0; i < spareBlockNum; i++) {				// ������ ��� mp �ʱ�ȭ
		for (int j = 0; j < tableMax; j++) {
			if (mp[j] == init + i) {
				if (init + i >= (end_PSN / 32) - 1)
					init = 0;
				mp[j] = -1;
			}
		}
	}

	cout << "gc ������ " << testGC << endl;
	if (swapBlock + spareBlockNum >= end_PSN / 32)
		return 50;
	else
		return swapBlock + spareBlockNum + 1;
}

