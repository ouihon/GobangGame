#include <windows.h>
#include <math.h>

#define BOARD_CELL_NUM 14
#define FIVE_MARK_POINT_RADIUS 4
#define CHESS_PIECE_RADIUS 13
#define BLACK_FLAG 1
#define WHITE_FLAG 2
#define NULL_FLAG 0
#define WIN_CONDITION 5

typedef enum Enum_Direction {
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
        MessageBox(NULL, TEXT("这个程序需要在 Windows NT 才能执行！"), szAppName, MB_ICONERROR);
        return 0;
    }

    hwnd = CreateWindow(szAppName, 
        TEXT("我的五子棋游戏"), 
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

// 绘制黑色实心点
HRESULT _DrawBlackSolidPoint(HDC hdc, int radius, POINT postion)
{
    SelectObject(hdc, GetStockObject(BLACK_BRUSH));
    Ellipse(hdc, postion.x - radius, postion.y - radius, postion.x + radius, postion.y + radius);
    SelectObject(hdc, GetStockObject(WHITE_BRUSH));

    return S_OK;
}

// 绘制白色空心点
HRESULT _DrawWhiteHollowPoint(HDC hdc, int radius, POINT postion)
{
    SelectObject(hdc, GetStockObject(WHITE_BRUSH));
    Ellipse(hdc, postion.x - radius, postion.y - radius, postion.x + radius, postion.y + radius);

    return S_OK;
}

// 获取一小格宽度和高度
HRESULT _GetCellWidthAndHeight(POINT ptLeftTop, int cxClient, int cyClient, int *cxCell, int *cyCell)
{
    *cxCell = (cxClient - ptLeftTop.x * 2) / BOARD_CELL_NUM;
    *cyCell = (cyClient - ptLeftTop.y * 2) / BOARD_CELL_NUM;

    return S_OK;
}

// 将实际坐标转化为逻辑坐标，这里需要进行实际点到棋盘点的转化
HRESULT _ExChangeLogicalPosition(POINT actualPostion, POINT ptLeftTop, int cxClient, int cyClient, POINT *logicalPostion)
{
    // 获得一小格的宽度和高度
    int cxCell = 0, cyCell = 0;
    _GetCellWidthAndHeight(ptLeftTop, cxClient, cyClient, &cxCell, &cyCell);
    // 检查点击有效性
    if (actualPostion.x < ptLeftTop.x || actualPostion.x > ptLeftTop.x + BOARD_CELL_NUM * cxCell ||
        actualPostion.y < ptLeftTop.y || actualPostion.y > ptLeftTop.y + BOARD_CELL_NUM * cyCell) {
        MessageBox(NULL ,TEXT("请点击棋盘内下棋！"), TEXT("提示"), MB_OK);
        return S_FALSE;
    }
    // 获取相邻四个点
    int xCount = 0, yCount = 0;
    POINT sidePoints[4] = { 0 };
    for (int x = ptLeftTop.x; x <= ptLeftTop.x + BOARD_CELL_NUM * cxCell; x += cxCell, xCount++) {
        if (actualPostion.x >= x && actualPostion.x <= x + cxCell) {
            sidePoints[0].x = x;
            sidePoints[2].x = x;
            sidePoints[1].x = x + cxCell;
            sidePoints[3].x = x + cxCell;
            break;
        }
    }
    for (int y = ptLeftTop.y; y <= ptLeftTop.y + BOARD_CELL_NUM * cyCell; y += cyCell, yCount++) {
        if (actualPostion.y >= y && actualPostion.y <= y + cyCell) {
            sidePoints[0].y = y;
            sidePoints[1].y = y;
            sidePoints[2].y = y + cyCell;
            sidePoints[3].y = y + cyCell;
            break;
        }
    }
    // 计算当前点到四个点到当前点距离
    double lengthCount[4] = { 0 };
    for (int item = 0; item < 4; ++item) {
        lengthCount[item] = pow(abs(sidePoints[item].x - actualPostion.x), 2) + pow(abs(sidePoints[item].y - actualPostion.y), 2);
    }
    // 获取四个距离值中最短的一个
    int shortestIndex = 0;
    for (int item = 0; item < 4; ++item) {
        if (lengthCount[item] < lengthCount[shortestIndex]) {
            shortestIndex = item;
        }
    }
    // 计算逻辑坐标，其中下标为0的点为基准点
    if (1 == shortestIndex) {
        xCount += 1;
    } 
    else if (2 == shortestIndex) {
        yCount += 1; 
    }
    else if (3 == shortestIndex) {
        xCount += 1;
        yCount += 1;
    }
    logicalPostion->x = xCount;
    logicalPostion->y = yCount;

    return S_OK;
}

// 将逻辑坐标转化为实际坐标
HRESULT _ExchangeActualPositon(POINT logicalPos, int cxCell, int cyCell, POINT ptLeftTop, POINT *actualPos)
{
    actualPos->x = ptLeftTop.x + logicalPos.x * cxCell;
    actualPos->y = ptLeftTop.y + logicalPos.y * cyCell;

    return S_OK;
}

// 绘制棋盘
HRESULT DrawChessBoard(HDC hdc, POINT ptLeftTop, int cxClient, int cyClient)
{
    // 获得一小格的宽度和高度
    int cxCell = 0, cyCell = 0;
    _GetCellWidthAndHeight(ptLeftTop, cxClient, cyClient, &cxCell, &cyCell);
    // 绘制竖线
    for (int col = 0; col < BOARD_CELL_NUM + 1; ++col) {
        MoveToEx(hdc, ptLeftTop.x + col * cxCell, ptLeftTop.y, NULL);
        LineTo(hdc, ptLeftTop.x + col * cxCell, ptLeftTop.y + BOARD_CELL_NUM * cyCell);
    }
    // 绘制灰色的横线
    HPEN hPen, hOldPen;
    hPen = CreatePen(PS_SOLID, 1, RGB(190, 190, 190));
    hOldPen = SelectObject(hdc, hPen);
    for (int row = 0; row < 7; ++row) {
        MoveToEx(hdc, ptLeftTop.x, ptLeftTop.y + cyCell + row * 2 * cyCell, NULL);
        LineTo(hdc, ptLeftTop.x + BOARD_CELL_NUM * cxCell, ptLeftTop.y + cyCell + row * 2 * cyCell);
    }
    SelectObject(hdc, hOldPen);
    // 绘制黑色的横线
    for (int row = 0; row < 8; ++row) {
        MoveToEx(hdc, ptLeftTop.x, ptLeftTop.y + row * 2 * cyCell, NULL);
        LineTo(hdc, ptLeftTop.x + BOARD_CELL_NUM * cxCell, ptLeftTop.y + row * 2 * cyCell);
    }
    // 绘制五个黑色实心点
    _DrawBlackSolidPoint(hdc, FIVE_MARK_POINT_RADIUS, (POINT) {ptLeftTop.x + 3 * cxCell, ptLeftTop.y + 3 * cyCell});
    _DrawBlackSolidPoint(hdc, FIVE_MARK_POINT_RADIUS, (POINT) {ptLeftTop.x + 11 * cxCell, ptLeftTop.y + 3 * cyCell});
    _DrawBlackSolidPoint(hdc, FIVE_MARK_POINT_RADIUS, (POINT) {ptLeftTop.x + 7 * cxCell, ptLeftTop.y + 7 * cyCell});
    _DrawBlackSolidPoint(hdc, FIVE_MARK_POINT_RADIUS, (POINT) {ptLeftTop.x + 3 * cxCell, ptLeftTop.y + 11 * cyCell});
    _DrawBlackSolidPoint(hdc, FIVE_MARK_POINT_RADIUS, (POINT) {ptLeftTop.x + 11 * cxCell, ptLeftTop.y + 11 * cyCell});

    return S_OK;
}

// 定义棋盘的左上角点
POINT ptBoardTop = {10, 10};
int cxClient = 0, cyClient = 0;

// 逻辑棋盘
int chessBoard[BOARD_CELL_NUM+1][BOARD_CELL_NUM+1] = {0};
BOOL bIsBlackTurn = TRUE;

// 清空棋盘
HRESULT ClearChessBoard()
{
    for (int row = 0; row < BOARD_CELL_NUM + 1; ++row) {
        for (int col = 0; col < BOARD_CELL_NUM + 1; ++col) {
            chessBoard[row][col] = NULL_FLAG;
        }
    }
    bIsBlackTurn = TRUE;

    return S_OK;
}

// 获取逻辑坐标点的标志
HRESULT GetCellFlag(POINT logicalPos, int *cellFlag)
{
    *cellFlag = chessBoard[logicalPos.y][logicalPos.x];
    return S_OK;
}

// 设置逻辑坐标点的标志
HRESULT SetCellFlag(POINT logicalPos, int cellFlag)
{
    chessBoard[logicalPos.y][logicalPos.x] = cellFlag;
    return S_OK;
}

// 在指定位置绘制棋子
HRESULT DrawChessPiece(HDC hdc, POINT logicalPos, int chessFlag)
{
    // 获得一小格的宽度和高度
    int cxCell = 0, cyCell = 0;
    _GetCellWidthAndHeight(ptBoardTop, cxClient, cyClient, &cxCell, &cyCell);
    // 获取实际绘制点
    POINT actualPos = {0};
    _ExchangeActualPositon(logicalPos, cxCell, cyCell, ptBoardTop, &actualPos);
    // 绘制棋子
    switch (chessFlag) {
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

// 是否存在某方向连子
BOOL IsExistWinFlagInSomeDirection(Win_Direction winDirection, POINT logicalPos, int winChessFlag, int winChessPieceNum)
{
    POINT chessPos = logicalPos;
    switch (winDirection) {
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
    // 检查坐标合法性
    if (chessPos.x < 0 || chessPos.y < 0 || chessPos.x > BOARD_CELL_NUM || chessPos.y > BOARD_CELL_NUM) {
        return FALSE;
    }
    // 获取坐标值
    int cellFlag = 0;
    GetCellFlag(chessPos, &cellFlag);
    if (winChessFlag != cellFlag) {
        return FALSE;
    }

    return TRUE;
}

// 判断是否存在胜利者
HRESULT IsSomeoneWin(POINT logicalPos, int chessFlag)
{
    // 遍历某点的所有方向，查看是否存在5连子
    int maxWinPieceNum = WIN_CONDITION - 1;
    for (int direction = Direction_Top; direction <= Direction_LeftTop; ++direction) {
        int winPieceNum = 0;
        for (int count = 1; count <= maxWinPieceNum; ++count) {
            if (!IsExistWinFlagInSomeDirection(direction, logicalPos, chessFlag, count)) {
                break;
            }
            winPieceNum++;
        }
        for (int count = 1; count <= maxWinPieceNum; ++count) {
            if (!IsExistWinFlagInSomeDirection((Win_Direction)((int)Direction_Bottom - direction), logicalPos, chessFlag, count)) {
                break;
            }
            winPieceNum++;
        }
		printf("%d %d", winPieceNum, maxWinPieceNum);
        if (winPieceNum >= maxWinPieceNum) {
            return S_OK;
        }
    }
    return S_FALSE;
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
        return 0;

    case WM_SIZE:
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);
        return 0;

    case WM_LBUTTONDOWN:
        hdc = GetDC(hwnd);
        if (S_OK != _ExChangeLogicalPosition((POINT) {LOWORD(lParam), HIWORD(lParam)}, ptBoardTop, cxClient, cyClient, &logicalPos)) {
            return 0;
        }
        int cellFlag = 0;
        GetCellFlag(logicalPos, &cellFlag);
        if (NULL_FLAG != cellFlag) {
            MessageBox(NULL, TEXT("这个位置已经有棋子了，请换个地方下棋！"), TEXT("提示"), MB_OK);
            return 0;
        }
        // 获取棋子
        int currentChessFlag = bIsBlackTurn ? BLACK_FLAG : WHITE_FLAG;
        // 绘制棋子
        DrawChessPiece(hdc, logicalPos, currentChessFlag);
        // 设置标志
        SetCellFlag(logicalPos, currentChessFlag);
        // 判断胜利
		printf("%d %d", IsSomeoneWin(logicalPos, currentChessFlag), S_OK);
        if (S_OK == IsSomeoneWin(logicalPos, currentChessFlag)) {
            if (IDYES == MessageBox(hwnd, bIsBlackTurn ? TEXT("黑方获胜！是否重新开始？") : TEXT("白方获胜！是否重新开始？"), TEXT("提示"), MB_YESNO)) {
                ClearChessBoard();
                InvalidateRect(hwnd, NULL, TRUE);
            } else {
                DestroyWindow(hwnd);
            }
        }
        bIsBlackTurn = !bIsBlackTurn;
        ReleaseDC(hwnd, hdc);
        return 0;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        DrawChessBoard(hdc, ptBoardTop, cxClient, cyClient);
        for (int row = 0; row < BOARD_CELL_NUM + 1; ++row) {
            for (int col = 0; col < BOARD_CELL_NUM + 1; ++col) {
                if (chessBoard[row][col] != NULL_FLAG) {
                    DrawChessPiece(hdc, (POINT) {col, row}, chessBoard[row][col]);
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
