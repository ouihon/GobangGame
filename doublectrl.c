#include <windows.h>
#include <math.h>

#define BOARD_CELL_NUM 14
#define FIVE_MARK_POINT_RADIUS 4
#define CHESS_PIECE_RADIUS 13
#define BLACK_FLAG 1
#define WHITE_FLAG 2
#define NULL_FLAG 0
#define WIN_CONDITION 5

POINT cursorPos = {7, 7}; // ��ʼ���λ������������

typedef struct
{
    POINT pos;
    int flag;
} ChessMove;

ChessMove moveHistory[(BOARD_CELL_NUM + 1) * (BOARD_CELL_NUM + 1)];
int moveCount = 0;

typedef enum Enum_Direction
{
    Direction_Top = 0,
    Direction_RightTop = 1,
    Direction_Right = 2,
    Direction_RightBottom = 3,
    Direction_Bottom = 4,
    Direction_LeftBottom = 5,
    Direction_Left = 6,
    Direction_LeftTop = 7
} Win_Direction;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
    static TCHAR szAppName[] = TEXT("MyWindows");
    HWND hwnd;
    MSG msg;
    WNDCLASS wndclass;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass(&wndclass))
    {
        MessageBox(NULL, TEXT("���������Ҫ�� Windows NT ����ִ�У�"), szAppName, MB_ICONERROR);
        return 0;
    }

    hwnd = CreateWindow(szAppName,
                        TEXT("������"),
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        610,
                        610,
                        NULL,
                        NULL,
                        hInstance,
                        NULL);

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

// ���ƺ�ɫʵ�ĵ�
HRESULT _DrawBlackSolidPoint(HDC hdc, int radius, POINT postion)
{
    SelectObject(hdc, GetStockObject(BLACK_BRUSH));
    Ellipse(hdc, postion.x - radius, postion.y - radius, postion.x + radius, postion.y + radius);
    SelectObject(hdc, GetStockObject(WHITE_BRUSH));

    return S_OK;
}

// ���ư�ɫ���ĵ�
HRESULT _DrawWhiteHollowPoint(HDC hdc, int radius, POINT postion)
{
    SelectObject(hdc, GetStockObject(WHITE_BRUSH));
    Ellipse(hdc, postion.x - radius, postion.y - radius, postion.x + radius, postion.y + radius);

    return S_OK;
}

// ��ȡһС���Ⱥ͸߶�
HRESULT _GetCellWidthAndHeight(POINT ptLeftTop, int cxClient, int cyClient, int *cxCell, int *cyCell)
{
    *cxCell = (cxClient - ptLeftTop.x * 2) / BOARD_CELL_NUM;
    *cyCell = (cyClient - ptLeftTop.y * 2) / BOARD_CELL_NUM;

    return S_OK;
}

// ��ʵ������ת��Ϊ�߼����꣬������Ҫ����ʵ�ʵ㵽���̵��ת��
HRESULT _ExChangeLogicalPosition(POINT actualPostion, POINT ptLeftTop, int cxClient, int cyClient, POINT *logicalPostion)
{
    // ���һС��Ŀ�Ⱥ͸߶�
    int cxCell = 0, cyCell = 0;
    _GetCellWidthAndHeight(ptLeftTop, cxClient, cyClient, &cxCell, &cyCell);
    // �������Ч��
    if (actualPostion.x < ptLeftTop.x || actualPostion.x > ptLeftTop.x + BOARD_CELL_NUM * cxCell ||
        actualPostion.y < ptLeftTop.y || actualPostion.y > ptLeftTop.y + BOARD_CELL_NUM * cyCell)
    {
        MessageBox(NULL, TEXT("�������������壡"), TEXT("��ʾ"), MB_OK);
        return S_FALSE;
    }
    // ��ȡ�����ĸ���
    int xCount = 0, yCount = 0;
    POINT sidePoints[4] = {0};
    for (int x = ptLeftTop.x; x <= ptLeftTop.x + BOARD_CELL_NUM * cxCell; x += cxCell, xCount++)
    {
        if (actualPostion.x >= x && actualPostion.x <= x + cxCell)
        {
            sidePoints[0].x = x;
            sidePoints[2].x = x;
            sidePoints[1].x = x + cxCell;
            sidePoints[3].x = x + cxCell;
            break;
        }
    }
    for (int y = ptLeftTop.y; y <= ptLeftTop.y + BOARD_CELL_NUM * cyCell; y += cyCell, yCount++)
    {
        if (actualPostion.y >= y && actualPostion.y <= y + cyCell)
        {
            sidePoints[0].y = y;
            sidePoints[1].y = y;
            sidePoints[2].y = y + cyCell;
            sidePoints[3].y = y + cyCell;
            break;
        }
    }
    // ���㵱ǰ�㵽�ĸ��㵽��ǰ�����
    double lengthCount[4] = {0};
    for (int item = 0; item < 4; ++item)
    {
        lengthCount[item] = pow(abs(sidePoints[item].x - actualPostion.x), 2) + pow(abs(sidePoints[item].y - actualPostion.y), 2);
    }
    // ��ȡ�ĸ�����ֵ����̵�һ��
    int shortestIndex = 0;
    for (int item = 0; item < 4; ++item)
    {
        if (lengthCount[item] < lengthCount[shortestIndex])
        {
            shortestIndex = item;
        }
    }
    // �����߼����꣬�����±�Ϊ0�ĵ�Ϊ��׼��
    if (1 == shortestIndex)
    {
        xCount += 1;
    }
    else if (2 == shortestIndex)
    {
        yCount += 1;
    }
    else if (3 == shortestIndex)
    {
        xCount += 1;
        yCount += 1;
    }
    logicalPostion->x = xCount;
    logicalPostion->y = yCount;

    return S_OK;
}

// ���߼�����ת��Ϊʵ������
HRESULT _ExchangeActualPositon(POINT logicalPos, int cxCell, int cyCell, POINT ptLeftTop, POINT *actualPos)
{
    actualPos->x = ptLeftTop.x + logicalPos.x * cxCell;
    actualPos->y = ptLeftTop.y + logicalPos.y * cyCell;

    return S_OK;
}

// ��������
HRESULT DrawChessBoard(HDC hdc, POINT ptLeftTop, int cxClient, int cyClient)
{
    // ���һС��Ŀ�Ⱥ͸߶�
    int cxCell = 0, cyCell = 0;
    _GetCellWidthAndHeight(ptLeftTop, cxClient, cyClient, &cxCell, &cyCell);
    // ��������
    for (int col = 0; col < BOARD_CELL_NUM + 1; ++col)
    {
        MoveToEx(hdc, ptLeftTop.x + col * cxCell, ptLeftTop.y, NULL);
        LineTo(hdc, ptLeftTop.x + col * cxCell, ptLeftTop.y + BOARD_CELL_NUM * cyCell);
    }
    // ���ƻ�ɫ�ĺ���
    HPEN hPen, hOldPen;
    hPen = CreatePen(PS_SOLID, 1, RGB(190, 190, 190));
    hOldPen = SelectObject(hdc, hPen);
    for (int row = 0; row < 7; ++row)
    {
        MoveToEx(hdc, ptLeftTop.x, ptLeftTop.y + cyCell + row * 2 * cyCell, NULL);
        LineTo(hdc, ptLeftTop.x + BOARD_CELL_NUM * cxCell, ptLeftTop.y + cyCell + row * 2 * cyCell);
    }
    SelectObject(hdc, hOldPen);
    // ���ƺ�ɫ�ĺ���
    for (int row = 0; row < 8; ++row)
    {
        MoveToEx(hdc, ptLeftTop.x, ptLeftTop.y + row * 2 * cyCell, NULL);
        LineTo(hdc, ptLeftTop.x + BOARD_CELL_NUM * cxCell, ptLeftTop.y + row * 2 * cyCell);
    }
    // ���������ɫʵ�ĵ�
    _DrawBlackSolidPoint(hdc, FIVE_MARK_POINT_RADIUS, (POINT){ptLeftTop.x + 3 * cxCell, ptLeftTop.y + 3 * cyCell});
    _DrawBlackSolidPoint(hdc, FIVE_MARK_POINT_RADIUS, (POINT){ptLeftTop.x + 11 * cxCell, ptLeftTop.y + 3 * cyCell});
    _DrawBlackSolidPoint(hdc, FIVE_MARK_POINT_RADIUS, (POINT){ptLeftTop.x + 7 * cxCell, ptLeftTop.y + 7 * cyCell});
    _DrawBlackSolidPoint(hdc, FIVE_MARK_POINT_RADIUS, (POINT){ptLeftTop.x + 3 * cxCell, ptLeftTop.y + 11 * cyCell});
    _DrawBlackSolidPoint(hdc, FIVE_MARK_POINT_RADIUS, (POINT){ptLeftTop.x + 11 * cxCell, ptLeftTop.y + 11 * cyCell});

    return S_OK;
}

// �������̵����Ͻǵ�
POINT ptBoardTop = {10, 10};
int cxClient = 0, cyClient = 0;

// �߼�����
int chessBoard[BOARD_CELL_NUM + 1][BOARD_CELL_NUM + 1] = {0};
BOOL bIsBlackTurn = TRUE;

HRESULT UndoLastMove(HWND hwnd)
{
    if (moveCount == 0)
    {
        MessageBox(hwnd, TEXT("û�п��Ի���Ĳ����ˣ�"), TEXT("��ʾ"), MB_OK);
        return S_FALSE;
    }

    moveCount--;
    ChessMove lastMove = moveHistory[moveCount];
    chessBoard[lastMove.pos.y][lastMove.pos.x] = NULL_FLAG;

    InvalidateRect(hwnd, NULL, TRUE);
    return S_OK;
}

// �������
HRESULT ClearChessBoard()
{
    for (int row = 0; row < BOARD_CELL_NUM + 1; ++row)
    {
        for (int col = 0; col < BOARD_CELL_NUM + 1; ++col)
        {
            chessBoard[row][col] = NULL_FLAG;
        }
    }
    bIsBlackTurn = TRUE;
    moveCount = 0; // ��ղ����¼

    return S_OK;
}

// ��ȡ�߼������ı�־
HRESULT GetCellFlag(POINT logicalPos, int *cellFlag)
{
    *cellFlag = chessBoard[logicalPos.y][logicalPos.x];
    return S_OK;
}

// �����߼������ı�־
HRESULT SetCellFlag(POINT logicalPos, int cellFlag)
{
    chessBoard[logicalPos.y][logicalPos.x] = cellFlag;
    return S_OK;
}

// ��ָ��λ�û�������
HRESULT DrawChessPiece(HDC hdc, POINT logicalPos, int chessFlag)
{
    // ���һС��Ŀ�Ⱥ͸߶�
    int cxCell = 0, cyCell = 0;
    _GetCellWidthAndHeight(ptBoardTop, cxClient, cyClient, &cxCell, &cyCell);
    // ��ȡʵ�ʻ��Ƶ�
    POINT actualPos = {0};
    _ExchangeActualPositon(logicalPos, cxCell, cyCell, ptBoardTop, &actualPos);
    // ��������
    switch (chessFlag)
    {
    case BLACK_FLAG:
        _DrawBlackSolidPoint(hdc, CHESS_PIECE_RADIUS, actualPos);
        break;
    case WHITE_FLAG:
        _DrawWhiteHollowPoint(hdc, CHESS_PIECE_RADIUS, actualPos);
        break;
    default:
        break;
    }

    return S_OK;
}

// �Ƿ����ĳ��������
BOOL IsExistWinFlagInSomeDirection(Win_Direction winDirection, POINT logicalPos, int winChessFlag, int winChessPieceNum)
{
    POINT chessPos = logicalPos;
    switch (winDirection)
    {
    case Direction_Top:
        chessPos.y -= winChessPieceNum;
        break;
    case Direction_RightTop:
        chessPos.y -= winChessPieceNum;
        chessPos.x += winChessPieceNum;
        break;
    case Direction_Right:
        chessPos.x += winChessPieceNum;
        break;
    case Direction_RightBottom:
        chessPos.y += winChessPieceNum;
        chessPos.x += winChessPieceNum;
        break;
    case Direction_Bottom:
        chessPos.y += winChessPieceNum;
        break;
    case Direction_LeftBottom:
        chessPos.y += winChessPieceNum;
        chessPos.x -= winChessPieceNum;
        break;
    case Direction_Left:
        chessPos.x -= winChessPieceNum;
        break;
    case Direction_LeftTop:
        chessPos.y -= winChessPieceNum;
        chessPos.x -= winChessPieceNum;
        break;
    }
    // �������Ϸ���
    if (chessPos.x < 0 || chessPos.y < 0 || chessPos.x > BOARD_CELL_NUM || chessPos.y > BOARD_CELL_NUM)
    {
        return FALSE;
    }
    // ��ȡ����ֵ
    int cellFlag = 0;
    GetCellFlag(chessPos, &cellFlag);
    if (winChessFlag != cellFlag)
    {
        return FALSE;
    }

    return TRUE;
}

// �ж��Ƿ����ʤ����
// HRESULT IsSomeoneWin(POINT logicalPos, int chessFlag)
// {
//     // ����ĳ������з��򣬲鿴�Ƿ����5����
//     int maxWinPieceNum = WIN_CONDITION - 1;
//     for (int direction = Direction_Top; direction <= Direction_LeftTop; ++direction) {
//         int winPieceNum = 0;
//         for (int count = 1; count <= maxWinPieceNum; ++count) {
//             if (!IsExistWinFlagInSomeDirection(direction, logicalPos, chessFlag, count)) {
//                 break;
//             }
//             winPieceNum++;
//         }
//         for (int count = 1; count <= maxWinPieceNum; ++count) {
//             if (!IsExistWinFlagInSomeDirection((Win_Direction)((int)Direction_Bottom - direction), logicalPos, chessFlag, count)) {
//                 break;
//             }
//             winPieceNum++;
//         }
//         if (winPieceNum >= maxWinPieceNum) {
//             return S_OK;
//         }
//     }
//     return S_FALSE;
// }

#define BOARD_CELL_NUM 15
#define BLACK_FLAG 1
#define WHITE_FLAG 2
#define NULL_FLAG 0

// �������飬���ڱ���8������
int directions[8][2] = {
    {0, 1}, {1, 0}, {1, 1}, {1, -1}, {0, -1}, {-1, 0}, {-1, -1}, {-1, 1}};

BOOL IsSomeoneWin(POINT pos, int currentChessFlag)
{
    int count;
    int x, y;

    // ����8������
    for (int d = 0; d < 4; d++)
    {
        count = 1;

        // ������������
        for (int i = 1; i < 5; i++)
        {
            x = pos.x + directions[d][0] * i;
            y = pos.y + directions[d][1] * i;
            if (x < 0 || x >= BOARD_CELL_NUM + 1 || y < 0 || y >= BOARD_CELL_NUM + 1)
            {
                break;
            }
            if (chessBoard[y][x] == currentChessFlag)
            {
                count++;
            }
            else
            {
                break;
            }
        }

        // �򷴷�������
        for (int i = 1; i < 5; i++)
        {
            x = pos.x - directions[d][0] * i;
            y = pos.y - directions[d][1] * i;
            if (x < 0 || x >= BOARD_CELL_NUM + 1 || y < 0 || y >= BOARD_CELL_NUM + 1)
            {
                break;
            }
            if (chessBoard[y][x] == currentChessFlag)
            {
                count++;
            }
            else
            {
                break;
            }
        }

        // �ж��Ƿ���5������������
        if (count >= 5)
        {
            return TRUE;
        }
    }

    return FALSE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    POINT logicalPos = {0};

    switch (message)
    {
    case WM_CREATE:
        ClearChessBoard();
        CreateWindow(TEXT("button"), TEXT("����"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                     520, 10, 70, 30, hwnd, (HMENU)1, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        cursorPos.x = BOARD_CELL_NUM / 2;
        cursorPos.y = BOARD_CELL_NUM / 2;
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == 1)
        {
            UndoLastMove(hwnd);
        }
        return 0;

    case WM_SIZE:
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);
        return 0;

    case WM_LBUTTONDOWN:
        hdc = GetDC(hwnd);
        if (S_OK != _ExChangeLogicalPosition((POINT){LOWORD(lParam), HIWORD(lParam)}, ptBoardTop, cxClient, cyClient, &logicalPos))
        {
            return 0;
        }
        int cellFlag = 0;
        GetCellFlag(logicalPos, &cellFlag);
        if (NULL_FLAG != cellFlag)
        {
            MessageBox(NULL, TEXT("���λ���Ѿ��������ˣ��뻻���ط����壡"), TEXT("��ʾ"), MB_OK);
            return 0;
        }
        // ��ȡ����
        int currentChessFlag = bIsBlackTurn ? BLACK_FLAG : WHITE_FLAG;
        // ��¼��ǰ��
        moveHistory[moveCount].pos = logicalPos;
        moveHistory[moveCount].flag = currentChessFlag;
        moveCount++;
        // ��������
        DrawChessPiece(hdc, logicalPos, currentChessFlag);
        // ���ñ�־
        SetCellFlag(logicalPos, currentChessFlag);
        // �ж�ʤ��
        if (TRUE == IsSomeoneWin(logicalPos, currentChessFlag))
        {
            if (IDYES == MessageBox(hwnd, bIsBlackTurn ? TEXT("�ڷ���ʤ���Ƿ����¿�ʼ��") : TEXT("�׷���ʤ���Ƿ����¿�ʼ��"), TEXT("��ʾ"), MB_YESNO))
            {
                ClearChessBoard();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else
            {
                DestroyWindow(hwnd);
            }
        }
        bIsBlackTurn = !bIsBlackTurn;
        ReleaseDC(hwnd, hdc);
        return 0;

    // ���̿���
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_LEFT:
            if (cursorPos.x > 0)
                cursorPos.x--;
            break;
        case VK_RIGHT:
            if (cursorPos.x < BOARD_CELL_NUM)
                cursorPos.x++;
            break;
        case VK_UP:
            if (cursorPos.y > 0)
                cursorPos.y--;
            break;
        case VK_DOWN:
            if (cursorPos.y < BOARD_CELL_NUM)
                cursorPos.y++;
            break;
        case VK_SPACE:
        case VK_RETURN:
            hdc = GetDC(hwnd);
            int cellFlag = 0;
            GetCellFlag(cursorPos, &cellFlag);
            if (NULL_FLAG != cellFlag)
            {
                MessageBox(NULL, TEXT("���λ���Ѿ��������ˣ��뻻���ط����壡"), TEXT("��ʾ"), MB_OK);
                ReleaseDC(hwnd, hdc);
                return 0;
            }
            // ��ȡ����
            int currentChessFlag = bIsBlackTurn ? BLACK_FLAG : WHITE_FLAG;
            // ��¼��ǰ��
            moveHistory[moveCount].pos = cursorPos;
            moveHistory[moveCount].flag = currentChessFlag;
            moveCount++;
            // ��������
            DrawChessPiece(hdc, cursorPos, currentChessFlag);
            // ���ñ�־
            SetCellFlag(cursorPos, currentChessFlag);
            // �ж�ʤ��
            if (TRUE == IsSomeoneWin(cursorPos, currentChessFlag))
            {
                if (IDYES == MessageBox(hwnd, bIsBlackTurn ? TEXT("�ڷ���ʤ���Ƿ����¿�ʼ��") : TEXT("�׷���ʤ���Ƿ����¿�ʼ��"), TEXT("��ʾ"), MB_YESNO))
                {
                    ClearChessBoard();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                else
                {
                    DestroyWindow(hwnd);
                }
            }
            bIsBlackTurn = !bIsBlackTurn;
            ReleaseDC(hwnd, hdc);
            break;
        }
        InvalidateRect(hwnd, NULL, TRUE); // �ػ���������ʾ���
        return 0;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        // ���ƹ��
        POINT actualPos;
        int cxCell, cyCell;
        _GetCellWidthAndHeight(ptBoardTop, cxClient, cyClient, &cxCell, &cyCell);
        _ExchangeActualPositon(cursorPos, cxCell, cyCell, ptBoardTop, &actualPos);
        SetBkColor(hdc, RGB(255, 255, 255));
        Rectangle(hdc, actualPos.x - cxCell / 2, actualPos.y - cyCell / 2, actualPos.x + cxCell / 2, actualPos.y + cyCell / 2);

        DrawChessBoard(hdc, ptBoardTop, cxClient, cyClient);
        for (int row = 0; row < BOARD_CELL_NUM + 1; ++row)
        {
            for (int col = 0; col < BOARD_CELL_NUM + 1; ++col)
            {
                if (chessBoard[row][col] != NULL_FLAG)
                {
                    DrawChessPiece(hdc, (POINT){col, row}, chessBoard[row][col]);
                }
            }
        }
        EndPaint(hwnd, &ps);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}