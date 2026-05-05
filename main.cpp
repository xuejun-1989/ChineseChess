#define _CRT_SECURE_NO_WARNINGS
#include <graphics.h>
#include <conio.h>
#include <cstring>
#include <tchar.h>
#include <algorithm>

// --- 1. 全局参数与类型定义 ---
#define GRID_SIZE 60
#define LEFT_MARGIN 80
#define TOP_MARGIN 80
#define ROW_NUM 10
#define COL_NUM 9

enum Color { CHESS_RED, CHESS_BLACK, CHESS_EMPTY };
enum Type { GENERAL, ADVISOR, ELEPHANT, HORSE, CHARIOT, CANNON, SOLDIER, TYPE_NONE };

struct ChessPiece {
    Color color;
    Type type;
    bool show;
};

// 全局状态变量
ChessPiece board[ROW_NUM][COL_NUM];
int selected_row = -1;
int selected_col = -1;
bool is_selected = false;
Color turn = CHESS_RED;

// --- 2. 坐标转换：棋盘行列 → 窗口像素 ---
POINT get_pos(int row, int col) {
    POINT p;
    p.x = LEFT_MARGIN + col * GRID_SIZE;
    p.y = TOP_MARGIN + row * GRID_SIZE;
    return p;
}

// --- 3. 绘制棋盘 ---
void draw_chessboard() {
    setlinecolor(BLACK);
    setlinestyle(PS_SOLID, 2);

    // 横线
    for (int i = 0; i < ROW_NUM; i++) {
        POINT p_start = get_pos(i, 0);
        POINT p_end = get_pos(i, COL_NUM - 1);
        line(p_start.x, p_start.y, p_end.x, p_end.y);
    }

    // 竖线（楚河汉界处断开）
    for (int i = 0; i < COL_NUM; i++) {
        POINT p_start = get_pos(0, i);
        POINT p_mid = get_pos(4, i);
        line(p_start.x, p_start.y, p_mid.x, p_mid.y);

        p_mid = get_pos(5, i);
        POINT p_end = get_pos(ROW_NUM - 1, i);
        line(p_mid.x, p_mid.y, p_end.x, p_end.y);
    }

    // 楚河汉界文字
    settextcolor(BLACK);
    setbkmode(TRANSPARENT);
    settextstyle(30, 0, _T("楷体"));
    outtextxy(LEFT_MARGIN + GRID_SIZE * 2, TOP_MARGIN + GRID_SIZE * 4 + 15, _T("楚 河"));
    outtextxy(LEFT_MARGIN + GRID_SIZE * 5, TOP_MARGIN + GRID_SIZE * 4 + 15, _T("汉 界"));

    // 九宫格斜线
    line(get_pos(0, 3).x, get_pos(0, 3).y, get_pos(2, 5).x, get_pos(2, 5).y);
    line(get_pos(0, 5).x, get_pos(0, 5).y, get_pos(2, 3).x, get_pos(2, 3).y);
    line(get_pos(7, 3).x, get_pos(7, 3).y, get_pos(9, 5).x, get_pos(9, 5).y);
    line(get_pos(7, 5).x, get_pos(7, 5).y, get_pos(9, 3).x, get_pos(9, 3).y);
}

// --- 4. 绘制棋子（含选中高亮） ---
void draw_piece(int row, int col) {
    ChessPiece p = board[row][col];
    if (p.color == CHESS_EMPTY || !p.show) return;

    POINT pos = get_pos(row, col);
    int radius = GRID_SIZE / 2 - 5;

    // 选中高亮圈
    if (is_selected && row == selected_row && col == selected_col) {
        setlinecolor(YELLOW);
        setlinestyle(PS_SOLID, 3);
        circle(pos.x, pos.y, radius + 3);
    }

    // 棋子底色+边框
    setfillcolor(RGB(255, 250, 200));
    setlinecolor(BLACK);
    setlinestyle(PS_SOLID, 2);
    fillcircle(pos.x, pos.y, radius);

    // 棋子文字
    TCHAR text[4];
    if (p.color == CHESS_RED) {
        switch (p.type) {
        case GENERAL: _tcscpy_s(text, _countof(text), _T("帥")); break;
        case ADVISOR: _tcscpy_s(text, _countof(text), _T("仕")); break;
        case ELEPHANT: _tcscpy_s(text, _countof(text), _T("相")); break;
        case HORSE: _tcscpy_s(text, _countof(text), _T("馬")); break;
        case CHARIOT: _tcscpy_s(text, _countof(text), _T("車")); break;
        case CANNON: _tcscpy_s(text, _countof(text), _T("炮")); break;
        case SOLDIER: _tcscpy_s(text, _countof(text), _T("兵")); break;
        }
        settextcolor(RGB(200, 0, 0));
    }
    else {
        switch (p.type) {
        case GENERAL: _tcscpy_s(text, _countof(text), _T("將")); break;
        case ADVISOR: _tcscpy_s(text, _countof(text), _T("士")); break;
        case ELEPHANT: _tcscpy_s(text, _countof(text), _T("象")); break;
        case HORSE: _tcscpy_s(text, _countof(text), _T("馬")); break;
        case CHARIOT: _tcscpy_s(text, _countof(text), _T("車")); break;
        case CANNON: _tcscpy_s(text, _countof(text), _T("砲")); break;
        case SOLDIER: _tcscpy_s(text, _countof(text), _T("卒")); break;
        }
        settextcolor(RGB(0, 0, 0));
    }

    // 文字居中绘制
    setbkmode(TRANSPARENT);
    settextstyle(36, 0, _T("楷体"));
    int tx = pos.x - textwidth(text) / 2;
    int ty = pos.y - textheight(text) / 2;
    outtextxy(tx, ty, text);
}

// --- 5. 初始化棋盘 ---
void init_game() {
    // 清空棋盘
    for (int r = 0; r < ROW_NUM; r++) {
        for (int c = 0; c < COL_NUM; c++) {
            board[r][c].color = CHESS_EMPTY;
            board[r][c].type = TYPE_NONE;
            board[r][c].show = false;
        }
    }

#define SET_PIECE(r, c, co, ty) \
    board[r][c].color = co; \
    board[r][c].type = ty; \
    board[r][c].show = true;

    // 黑方棋子（上方）
    SET_PIECE(0, 0, CHESS_BLACK, CHARIOT);
    SET_PIECE(0, 1, CHESS_BLACK, HORSE);
    SET_PIECE(0, 2, CHESS_BLACK, ELEPHANT);
    SET_PIECE(0, 3, CHESS_BLACK, ADVISOR);
    SET_PIECE(0, 4, CHESS_BLACK, GENERAL);
    SET_PIECE(0, 5, CHESS_BLACK, ADVISOR);
    SET_PIECE(0, 6, CHESS_BLACK, ELEPHANT);
    SET_PIECE(0, 7, CHESS_BLACK, HORSE);
    SET_PIECE(0, 8, CHESS_BLACK, CHARIOT);
    SET_PIECE(2, 1, CHESS_BLACK, CANNON);
    SET_PIECE(2, 7, CHESS_BLACK, CANNON);
    SET_PIECE(3, 0, CHESS_BLACK, SOLDIER);
    SET_PIECE(3, 2, CHESS_BLACK, SOLDIER);
    SET_PIECE(3, 4, CHESS_BLACK, SOLDIER);
    SET_PIECE(3, 6, CHESS_BLACK, SOLDIER);
    SET_PIECE(3, 8, CHESS_BLACK, SOLDIER);

    // 红方棋子（下方）
    SET_PIECE(9, 0, CHESS_RED, CHARIOT);
    SET_PIECE(9, 1, CHESS_RED, HORSE);
    SET_PIECE(9, 2, CHESS_RED, ELEPHANT);
    SET_PIECE(9, 3, CHESS_RED, ADVISOR);
    SET_PIECE(9, 4, CHESS_RED, GENERAL);
    SET_PIECE(9, 5, CHESS_RED, ADVISOR);
    SET_PIECE(9, 6, CHESS_RED, ELEPHANT);
    SET_PIECE(9, 7, CHESS_RED, HORSE);
    SET_PIECE(9, 8, CHESS_RED, CHARIOT);
    SET_PIECE(7, 1, CHESS_RED, CANNON);
    SET_PIECE(7, 7, CHESS_RED, CANNON);
    SET_PIECE(6, 0, CHESS_RED, SOLDIER);
    SET_PIECE(6, 2, CHESS_RED, SOLDIER);
    SET_PIECE(6, 4, CHESS_RED, SOLDIER);
    SET_PIECE(6, 6, CHESS_RED, SOLDIER);
    SET_PIECE(6, 8, CHESS_RED, SOLDIER);

    // 重置状态
    selected_row = -1;
    selected_col = -1;
    is_selected = false;
    turn = CHESS_RED;
}

// --- 6. 重绘整个画面（双缓冲核心修改点） ---
void repaint_all() {
    // 所有绘制操作都在后台缓冲区完成
    cleardevice();
    draw_chessboard();
    for (int r = 0; r < ROW_NUM; r++) {
        for (int c = 0; c < COL_NUM; c++) {
            draw_piece(r, c);
        }
    }

    // 回合提示
    settextcolor(BLUE);
    setbkmode(TRANSPARENT);
    settextstyle(20, 0, _T("宋体"));
    if (turn == CHESS_RED) {
        outtextxy(100, 20, _T("当前回合：红方"));
    }
    else {
        outtextxy(100, 20, _T("当前回合：黑方"));
    }

    // 画完一整帧后，一次性刷新到屏幕
    FlushBatchDraw();
}

// --- 7. 鼠标点击坐标转棋盘行列 ---
bool click_to_board(int x, int y, int& row, int& col) {
    for (int r = 0; r < ROW_NUM; r++) {
        for (int c = 0; c < COL_NUM; c++) {
            POINT p = get_pos(r, c);
            int dx = x - p.x;
            int dy = y - p.y;
            if (dx * dx + dy * dy < (GRID_SIZE / 2) * (GRID_SIZE / 2)) {
                row = r;
                col = c;
                return true;
            }
        }
    }
    return false;
}

// --- 8. 辅助：计算两点之间棋子数量（车/炮规则用） ---
int count_pieces_between(int r1, int c1, int r2, int c2) {
    int count = 0;
    if (r1 == r2) { // 同一行
        int min_c = min(c1, c2);
        int max_c = max(c1, c2);
        for (int c = min_c + 1; c < max_c; c++) {
            if (board[r1][c].color != CHESS_EMPTY) count++;
        }
    }
    else if (c1 == c2) { // 同一列
        int min_r = min(r1, r2);
        int max_r = max(r1, r2);
        for (int r = min_r + 1; r < max_r; r++) {
            if (board[r][c1].color != CHESS_EMPTY) count++;
        }
    }
    return count;
}

// --- 9. 核心：走棋规则校验 ---
bool is_move_valid(int from_r, int from_c, int to_r, int to_c) {
    ChessPiece from = board[from_r][from_c];
    ChessPiece to = board[to_r][to_c];

    // 1. 不能吃己方棋子
    if (to.color == from.color) return false;

    int dr = abs(to_r - from_r);
    int dc = abs(to_c - from_c);

    switch (from.type) {
    case CHARIOT: { // 车：直线走，中间不能有棋子
        if (dr != 0 && dc != 0) return false;
        return count_pieces_between(from_r, from_c, to_r, to_c) == 0;
    }

    case HORSE: { // 马：走日，蹩马腿
        if (!((dr == 2 && dc == 1) || (dr == 1 && dc == 2))) return false;
        int block_r, block_c;
        // 蹩马腿判断
        if (dr == 2) {
            block_r = from_r + (to_r > from_r ? 1 : -1);
            if (board[block_r][from_c].color != CHESS_EMPTY) return false;
        }
        else {
            block_c = from_c + (to_c > from_c ? 1 : -1);
            if (board[from_r][block_c].color != CHESS_EMPTY) return false;
        }
        return true;
    }

    case CANNON: { // 炮：直线走，吃子时需要一个炮架
        if (dr != 0 && dc != 0) return false;
        int cnt = count_pieces_between(from_r, from_c, to_r, to_c);
        if (to.color == CHESS_EMPTY) return cnt == 0;
        else return cnt == 1;
    }

    case GENERAL: { // 将/帅：九宫格内直线走，一步一格
        if (dr > 1 || dc > 1) return false;
        if (dc != 0 && dr != 0) return false;
        // 九宫格范围
        if (from.color == CHESS_RED) {
            if (to_r < 7 || to_r > 9 || to_c < 3 || to_c > 5) return false;
        }
        else {
            if (to_r < 0 || to_r > 2 || to_c < 3 || to_c > 5) return false;
        }
        return true;
    }

    case ADVISOR: { // 士：九宫格内斜走，一步一格
        if (dr != 1 || dc != 1) return false;
        if (from.color == CHESS_RED) {
            if (to_r < 7 || to_r > 9 || to_c < 3 || to_c > 5) return false;
        }
        else {
            if (to_r < 0 || to_r > 2 || to_c < 3 || to_c > 5) return false;
        }
        return true;
    }

    case ELEPHANT: { // 象/相：走田，不越河，不蹩象眼
        if (dr != 2 || dc != 2) return false;
        // 不越河
        if (from.color == CHESS_RED && to_r < 5) return false;
        if (from.color == CHESS_BLACK && to_r > 4) return false;
        // 蹩象眼
        int block_r = from_r + (to_r > from_r ? 1 : -1);
        int block_c = from_c + (to_c > from_c ? 1 : -1);
        if (board[block_r][block_c].color != CHESS_EMPTY) return false;
        return true;
    }

    case SOLDIER: { // 兵/卒：过河前只能向前，过河后可左右
        if (from.color == CHESS_RED) {
            if (from_r > 4) { // 未过河，只能向上（行号减小）
                if (to_r >= from_r || dr > 1 || dc > 0) return false;
            }
            else { // 已过河，可左右或向上
                if (dr > 1 || dc > 1) return false;
                if (dr == 1 && dc != 0) return false;
                if (dc == 1 && dr != 0) return false;
                if (to_r > from_r) return false;
            }
        }
        else {
            if (from_r < 5) { // 未过河，只能向下（行号增大）
                if (to_r <= from_r || dr > 1 || dc > 0) return false;
            }
            else { // 已过河，可左右或向下
                if (dr > 1 || dc > 1) return false;
                if (dr == 1 && dc != 0) return false;
                if (dc == 1 && dr != 0) return false;
                if (to_r < from_r) return false;
            }
        }
        return true;
    }

    default:
        return false;
    }
}

// --- 10. 移动棋子（执行吃子和状态切换） ---
void move_piece(int from_r, int from_c, int to_r, int to_c) {
    // 移动棋子
    board[to_r][to_c] = board[from_r][from_c];
    // 清空原位置
    board[from_r][from_c].color = CHESS_EMPTY;
    board[from_r][from_c].type = TYPE_NONE;
    board[from_r][from_c].show = false;
    // 切换回合
    turn = (turn == CHESS_RED) ? CHESS_BLACK : CHESS_RED;
}

// --- 主函数：双缓冲初始化 + 消息循环 ---
int main() {
    initgraph(720, 800);
    setbkcolor(RGB(240, 230, 200));

    // 【关键1】初始化窗口后，立刻开启双缓冲（批量绘图）
    BeginBatchDraw();

    init_game();
    repaint_all(); // 首次绘制

    ExMessage msg;

    while (true) {
        if (peekmessage(&msg, EM_MOUSE)) {
            if (msg.message == WM_LBUTTONDOWN) {
                int click_r, click_c;
                if (click_to_board(msg.x, msg.y, click_r, click_c)) {

                    // 情况1：当前没有选中棋子
                    if (!is_selected) {
                        // 只能选中当前回合的己方棋子
                        if (board[click_r][click_c].color == turn) {
                            selected_row = click_r;
                            selected_col = click_c;
                            is_selected = true;
                            repaint_all();
                        }
                    }
                    // 情况2：已经选中了棋子
                    else {
                        // 子情况A：点击了己方另一个棋子 → 切换选中目标
                        if (board[click_r][click_c].color == turn) {
                            selected_row = click_r;
                            selected_col = click_c;
                            repaint_all();
                        }
                        // 子情况B：点击了敌方棋子或空地 → 尝试移动
                        else {
                            if (is_move_valid(selected_row, selected_col, click_r, click_c)) {
                                move_piece(selected_row, selected_col, click_r, click_c);
                            }
                            // 无论是否成功，都取消选中状态
                            is_selected = false;
                            selected_row = -1;
                            selected_col = -1;
                            repaint_all();
                        }
                    }
                }
            }
        }
        // 按ESC键退出
        if (_kbhit() && _getch() == 27) break;
    }

    // 【关键2】程序退出前，关闭双缓冲
    EndBatchDraw();
    closegraph();
    return 0;
}