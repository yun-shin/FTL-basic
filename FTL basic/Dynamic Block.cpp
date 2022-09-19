
#include "Dynamic Block.h"
#define ONE_BLOCK_SECTOR 32

/*
void DynamicSector::menu () {
	string Operation;
	int number;
	char data;

	cout << "write or read after init" << endl;
	cin >> Operation >> megabytes;
	if (!Operation.compare ("init") && cin.fail () == false && megabytes >= 0) {                   // x megabytes �÷��� �޸� ����
		f.init (megabytes);
		make_table ('s');

		while (true) {
			cin >> Operation >> number;
			if (!Operation.compare ("read") && cin.fail () == false && number >= -1)                // �б� ���
				FTL_read (number);
			else if (!Operation.compare ("write") && cin.fail () == false && number >= -1) {        // ���� ���
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
				cout << "���Է�" << endl;
				cin.clear ();                           // ������Ʈ �ʱ�ȭ
				cin.ignore (1000, '\n');
			}
		}
	}
	else {
		cout << "���Է�" << endl;
		cin.clear ();                           // ������Ʈ �ʱ�ȭ
		cin.ignore (1000, '\n');
		return;
	}
}*/


DynamicBlock::~DynamicBlock () {
	delete[] mp;
}

void DynamicBlock::result () {
	cout << endl << "���� Ƚ�� : " << writeCount << "ȸ �б�Ƚ�� : " << readCount << "ȸ ����Ƚ�� : " << eraseCount << "ȸ" << endl;
}

void DynamicBlock::make_table (char sb, char sd) {
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

void DynamicBlock::FTL_write (int LSN, string data) {
	int tableMax = _msize (mp) / sizeof (int);
	int megabytes = (_msize (memory) / sizeof (int)) / 64;
	int end_PSN = (_msize (memory) / 4) * 32;
	string readData;

	if (nextWrite == (end_PSN / 32) - 1) {		// ù GC 
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

			if (BS_number (LSN, 's') == i && readData != "-1")
				Flash_write (nextWrite + 1, i, data);
			else if (readData != "-1")
				Flash_write (nextWrite + 1, i, readData);
		}

		nextWrite++;
		mp[LSN / ONE_BLOCK_SECTOR] = nextWrite;
	}
	else if (mp[LSN / ONE_BLOCK_SECTOR] == -1) {					// ù write
		nextWrite++;
		Flash_write (nextWrite, BS_number(LSN, 's') , data);
		mp[LSN / ONE_BLOCK_SECTOR] = nextWrite;
	}
	else if (mp[LSN / ONE_BLOCK_SECTOR] != -1) {					// ���� ��� �ٸ� offset write
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
	int end_PSN = (_msize (memory) / 4) * 32;       // �׳� ���ؼ� ����        
	int spareBlockNum = (_msize (memory) / 4) / 10; // GC �ؾ� �� ��� ����. �ϴ� �γ��� ���߿� ���￹��. 8mb ���� 51�� ������ ����� swap
	int tableMax = _msize (mp) / sizeof (int), emptyNum = 0;
	int read = nextWrite + 1, gcStart = nextWrite + 1;
	string readData;                                        // read ������ ����

	testGC++;
	nextWrite++;
	cout << "gc ������ " << testGC << endl;

	if (read >= end_PSN)
		read = 0;

	for (int i = 0; i < spareBlockNum; i++) {                      // ������� ��� ����ŭ �ݺ�
		for (int j = 0; j < tableMax; j++) {                            // ��ȿ ������ ã�������� �ݺ�
			if (mp[j] == read) {									// �ش� ����� �����Ͱ� mp�� �����ϸ�?
				for (int k = 0; k < ONE_BLOCK_SECTOR; k++) {		// �ش� ��� swapBlock�� ����.
					readData = Flash_read (read, k);

					if (readData != "-1")										// readData �� �����ϸ� ����.
						Flash_write (nextWrite, k, readData);
				}

				nextWrite++;

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
		return 49;
	else
		return swapBlock + spareBlockNum + 1;
}
