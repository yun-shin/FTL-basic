
#include "workload read.h"

void fuction::readFile (int choice) {
#pragma warning(disable: 4996) // 오류 씹음
    ifstream file;
    string CommandType;
    int LSN;

    file.open ("workload temp.txt");                       // 원하는 워크로드 삽입

    switch (choice) {
    case 1:
        ss.make_table ('s', 's');           // mp 생성
        break;

    case 2:
        ds.make_table ('s', 'd');           // mp 생성
        break;

    case 3:
        sb.make_table ('b', 's');           // mp 생성
        break;

    case 4:
        db.make_table ('b', 'd');           // mp 생성
        break;
    }

    // 정렬
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
        sb.result ();           // mp 생성
        break;

    case 4:
        db.result ();           // mp 생성
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
        // 쓰기 함수
    }
    else if (CommandType == "r") {
        cout << "실패 !" << endl;
        // 읽기 함수
    }
}