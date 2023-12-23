#include "pch.h"
#include <stdio.h>   // sprintf_s 함수를 사용하기 위함
#include "tipsware.h"

#pragma comment(lib, "WS2_32.lib")   // Windows Socket API를 사용하기 위함

// 프로그램에서 사용할 내부 데이터
struct AppData
{
    void* p_client_socket;  // 클라이언트 소켓을 사용하는 객체의 주소
};

// 서버에 접속했거나 접속에 실패했을 때 호출되는 함수
void MyConnectToServer(void* ap_this, int a_client_index)
{
    char temp_str[64];
    if (IsConnect(ap_this)) sprintf_s(temp_str, 64, "서버에 접속했습니다 . . . . ");
    else sprintf_s(temp_str, 64, "오류: 서버에 접속할 수 없습니다.");
    // 서버 접속 결과를 리스트 박스(컨트롤 아이디 1000번)에 추가
    ListBox_InsertString(FindControl(1000), -1, temp_str);
}

// 서버가 데이터를 전송하면 호출되는 함수
int MyNetMessageFromServer(CurrentClientNetworkData* ap_data, void* ap_this, int a_client_index)
{
    if (ap_data->m_net_msg_id == 1) { // 채팅 데이터가 전달됨 (1번 아이디는 개발자가 정한 것임)
        // 서버가 전달한 채팅 내용을 리스트 박스(컨트롤 아이디 1000번)에 추가
        ListBox_InsertString(FindControl(1000), -1, ap_data->mp_net_body_data);
    }
    return 1;
}

// 서버와 접속 상태가 변경되면 호출되는 함수
void MyCloseUser(void* ap_this, int a_error_flag, int a_client_index)
{
    char temp_str[64];
    if (a_error_flag == 1) sprintf_s(temp_str, 64, "서버에서 접속을 해제했습니다."); // 서버가 해제함
    else sprintf_s(temp_str, 64, "서버와의 접속을 해제했습니다.");  // 클라이언트에서 해제함
    // 해제 상태를 리스트 박스(컨트롤 아이디 1000번)에 추가
    ListBox_InsertString(FindControl(1000), -1, temp_str);
}

// 컨트롤을 조작했을 때 호출할 함수
// 컨트롤의 아이디(a_ctrl_id), 컨트롤의 조작 상태(a_notify_code), 선택한 컨트롤 객체(ap_ctrl)
void OnCommand(INT32 a_ctrl_id, INT32 a_notify_code, void* ap_ctrl)
{
    // 응용 프로그램 내부 데이터의 주소를 가져옴
    AppData* p_app_data = (AppData*)GetAppData();

    if (a_ctrl_id == 1003 ||  // 1003번('입력') 버튼이 눌러진 경우
        (a_ctrl_id == 1004 && a_notify_code == 1000)) {  // 에디트 박스에서 엔터 키가 눌러진 경우
        void* p_edit = FindControl(1004); // 에디트 컨트롤의 주소를 얻음
        char str[128]; // 선택된 항목의 문자열을 저장할 변수
        // 에디터 컨트롤에 입력된 문자열을 str 배열에 복사
        GetCtrlName(p_edit, str, 128);
        SetCtrlName(p_edit, ""); // 에디트 컨트롤에 쓰여진 문자열을 지움

        // 클라이언트 소켓 객체가 만들어져있는지, 서버와 접속상태인지 확인
        if (p_app_data->p_client_socket && IsConnect(p_app_data->p_client_socket)) {
            // 입력된 채팅 내용을 서버로 전송(채팅 메시지 아이디는 1로 지정)
            SendFrameDataToServer(p_app_data->p_client_socket, 1, str, strlen(str) + 1);
        }
    }
    else if (a_ctrl_id == 1001) {  // [접속] 버튼을 누른 경우
        if (p_app_data->p_client_socket == NULL) {  // 클라이언트 소켓 객체가 만들어져 있는지 확인
            // 객체가 만들어지기 전이라면 클라이언트 소켓 객체를 생성
            // 이 객체 자신의 상태가 바뀌면 위에서 만든 MyConnectToServer, MyNetMessageFromServer,

              // MyCloseUser 함수를 호출
            p_app_data->p_client_socket = CreateClientSocket(MyConnectToServer,

                MyNetMessageFromServer, MyCloseUser);

        }

        if (!IsConnect(p_app_data->p_client_socket)) {  // 서버와의 접속 상태를 체크
            // "192.168.0.118" 주소에서 25001번 포트로 서비스 중인 서버에 접속을 시도
            // 접속의 결과는 MyConnectToServer 함수를 통해서 알 수 있음
            ConnectToServer(p_app_data->p_client_socket, "192.168.0.118", 25001);
        }
    }
    else if (a_ctrl_id == 1002) { // [해제] 버튼을 누른 경우
        if (p_app_data->p_client_socket != NULL) {  // 객체가 만들어져 있는 경우
            // 소켓 클라이언트 객체를 제거
            DeleteClientSocket(p_app_data->p_client_socket);
            p_app_data->p_client_socket = NULL;
        }
    }
}

// 컨트롤을 조작했을 때 호출할 함수 등록
CMD_MESSAGE(OnCommand)

int main()
{
    // 응용 프로그램이 내부적으로 사용할 메모리 선언
    AppData data = { NULL };
    // 지정한 변수를 내부 데이터로 저장
    SetAppData(&data, sizeof(AppData));

    Clear(0, RGB(72, 87, 114)); // 윈도우의 배경 색 지정
    StartSocketSystem(); // 이 프로그램이 소켓 시스템을 사용하겠다고 설정

    CreateListBox(10, 36, 500, 300, 1000);  // 1000번 아이디를 가진 리스트 박스를 생성
    CreateButton("접속", 407, 3, 50, 28, 1001);   // [접속] 버튼 생성
    CreateButton("해제", 460, 3, 50, 28, 1002);   // [해제] 버튼 생성
    CreateButton("입력", 460, 340, 50, 28, 1003); // [입력] 버튼 생성
    void* p = CreateEdit(10, 343, 446, 24, 1004, 0);  // 문자열을 입력할 에디트 컨트롤을 추가
    EnableEnterKey(p); // 에디트 컨트롤에 엔터키가 눌러지면 Notify Code(1000번)가 발생하게 설정

    SelectFontObject("굴림", 12); // 글꼴 변경
    TextOut(15, 10, RGB(200, 255, 200), "사용자 채팅글 목록");  // 리스트 박스의 제목 출력
    ShowDisplay(); // 정보 출력
    return 0;
}
