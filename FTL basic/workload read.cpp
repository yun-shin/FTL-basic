
#include "workload read.h"

void fuction::readFile (int choice) {
#pragma warning(disable: 4996) // ���� ����
    ifstream file;
    string CommandType;
    int LSN;

    file.open ("workload temp.txt");                       // ���ϴ� ��ũ�ε� ����

    switch (choice) {
    case 1:
        ss.make_table ('s', 's');           // mp ����
        break;

    case 2:
        ds.make_table ('s', 'd');           // mp ����
        break;

    case 3:
        sb.make_table ('b', 's');           // mp ����
        break;

    case 4:
        db.make_table ('b', 'd');           // mp ����
        break;
    }

    // ����
    while (file >> CommandType >> LSN) {
     //   cout << CommandType << " " << LSN << endl;

        CommandCheck (CommandType, LSN, choice);
    }

    file.close ();

    switch (choice) {
    case 1:
        ss.result ();          
        break;

    case 2:
        ds.result ();           
        break;

    case 3:
        sb.result ();           // mp ����
        break;

    case 4:
        db.result ();           // mp ����
        break;
    }
}

void fuction::CommandCheck (string CommandType, int LSN, int choice) {
    if (CommandType == "w") {
        switch (choice) {
        case 1:
            ss.FTL_write (LSN, to_string (queue));          
            break;

        case 2:
            ds.FTL_write (LSN, to_string (queue));
            break;

        case 3:
            sb.FTL_write (LSN, to_string (queue));
            break;

        case 4:
            db.FTL_write (LSN, to_string (queue));
            break;
        }

        queue++;
        // ���� �Լ�
    }
    else if (CommandType == "r") {
        cout << "���� !" << endl;
        // �б� �Լ�
    }
}