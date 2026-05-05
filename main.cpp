#define _CRT_SECURE_NO_WARNINGS
#include <graphics.h>
#include <conio.h>
#include <cstring>
#include <tchar.h>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <ctime>

// --- 1. 全局参数与类型定义 ---
#define GRID_SIZE 60
#define LEFT_MARGIN 80
#define TOP_MARGIN 80
#define ROW_NUM 10
#define COL_NUM 9
#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 800

// 棋子颜色
enum Color { CHESS_RED, CHESS_BLACK, CHESS_EMPTY };
// 棋子类型
enum Type { GENERAL, ADVISOR, ELEPHANT, HORSE, CHARIOT, CANNON, SOLDIER, TYPE_NONE };
// 游戏模式
enum GameMode { MODE_TWO_PLAYER, MODE_AI, MODE_NONE };

// 棋子结构体
struct ChessPiece {
    Color color;
    Type type;
    bool show;
};

// 走法结构体（AI用）
struct ChessMove {
    int from_r;
    int from_c;
    int to_r;
    int to_c;
    int score;
};

// 按钮结构体（模式选择界面用）
struct Button {
    int x;          // 左上角x坐标
    int y;          // 左上角y坐标
    int width;      // 宽度
    int height;     // 高度
    TCHAR text[32]; // 按钮文字
    bool is_hover;  // 鼠标是否悬浮
    GameMode mode;  // 按钮对应的模式
};

// 全局状态变量
ChessPiece board[ROW_NUM][COL_NUM];
int selected_row = -1;
int selected_col = -1;
bool is_selected = false;
Color turn = CHESS_RED;
GameMode game_mode = MODE_NONE;
bool game_over = false;
TCHAR game_result[64];
POINT find_general_pos(Color color);
bool is_checked(Color color);
bool is_move_safe(int from_r, int from_c, int to_r, int to_c);

// --- 2. 辅助工具函数 ---
// 坐标转换：棋盘行列 → 窗口像素
POINT get_pos(int row, int col) {
    POINT p;
    p.x = LEFT_MARGIN + col * GRID_SIZE;
    p.y = TOP_MARGIN + row * GRID_SIZE;
    return p;
}

// 判断鼠标是否在按钮内
bool is_point_in_button(int x, int y, Button btn) {
    return x >= btn.x && x <= btn.x + btn.width && y >= btn.y && y <= btn.y + btn.height;
}

// --- 3. 棋盘绘制 ---
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

// --- 4. 棋子绘制（含选中高亮） ---
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

// --- 5. 游戏初始化 ---
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
    game_over = false;
    memset(game_result, 0, sizeof(game_result));
    srand((unsigned int)time(NULL));
}

// --- 6. 全局画面重绘（双缓冲无闪屏） ---
void repaint_all() {
    cleardevice();
    draw_chessboard();
    for (int r = 0; r < ROW_NUM; r++) {
        for (int c = 0; c < COL_NUM; c++) {
            draw_piece(r, c);
        }
    }

    // 回合/模式提示
    settextcolor(BLUE);
    setbkmode(TRANSPARENT);
    settextstyle(20, 0, _T("宋体"));
    if (game_over) {
        outtextxy(100, 20, game_result);
        outtextxy(100, 50, _T("按ESC键退出程序"));
    }
    else {
        if (game_mode == MODE_TWO_PLAYER) {
            outtextxy(100, 20, _T("模式：双人对战"));
        }
        else {
            outtextxy(100, 20, _T("模式：人机对战（你是红方）"));
        }
        // 回合提示
        if (turn == CHESS_RED) {
            outtextxy(100, 50, _T("当前回合：红方"));
            // 新增：红方被将军提示
            if (is_checked(CHESS_RED)) {
                settextcolor(RED);
                outtextxy(100, 80, _T("警告！红方被将军！"));
            }
        }
        else {
            outtextxy(100, 50, _T("当前回合：黑方"));
            // 新增：黑方被将军提示
            if (is_checked(CHESS_BLACK)) {
                settextcolor(RED);
                outtextxy(100, 80, _T("警告！黑方被将军！"));
            }
        }
    }

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

// --- 8. 辅助：计算两点之间棋子数量 ---
int count_pieces_between(int r1, int c1, int r2, int c2) {
    int count = 0;
    if (r1 == r2) {
        int min_c = min(c1, c2);
        int max_c = max(c1, c2);
        for (int c = min_c + 1; c < max_c; c++) {
            if (board[r1][c].color != CHESS_EMPTY) count++;
        }
    }
    else if (c1 == c2) {
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

    if (to.color == from.color) return false;

    int dr = abs(to_r - from_r);
    int dc = abs(to_c - from_c);

    switch (from.type) {
    case CHARIOT: {
        if (dr != 0 && dc != 0) return false;
        return count_pieces_between(from_r, from_c, to_r, to_c) == 0;
    }
    case HORSE: {
        if (!((dr == 2 && dc == 1) || (dr == 1 && dc == 2))) return false;
        int block_r, block_c;
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
    case CANNON: {
        if (dr != 0 && dc != 0) return false;
        int cnt = count_pieces_between(from_r, from_c, to_r, to_c);
        if (to.color == CHESS_EMPTY) return cnt == 0;
        else return cnt == 1;
    }
    case GENERAL: {
        if (dr > 1 || dc > 1) return false;
        if (dc != 0 && dr != 0) return false;
        if (from.color == CHESS_RED) {
            if (to_r < 7 || to_r > 9 || to_c < 3 || to_c > 5) return false;
        }
        else {
            if (to_r < 0 || to_r > 2 || to_c < 3 || to_c > 5) return false;
        }
        return true;
    }
    case ADVISOR: {
        if (dr != 1 || dc != 1) return false;
        if (from.color == CHESS_RED) {
            if (to_r < 7 || to_r > 9 || to_c < 3 || to_c > 5) return false;
        }
        else {
            if (to_r < 0 || to_r > 2 || to_c < 3 || to_c > 5) return false;
        }
        return true;
    }
    case ELEPHANT: {
        if (dr != 2 || dc != 2) return false;
        if (from.color == CHESS_RED && to_r < 5) return false;
        if (from.color == CHESS_BLACK && to_r > 4) return false;
        int block_r = from_r + (to_r > from_r ? 1 : -1);
        int block_c = from_c + (to_c > from_c ? 1 : -1);
        if (board[block_r][block_c].color != CHESS_EMPTY) return false;
        return true;
    }
    case SOLDIER: {
        if (from.color == CHESS_RED) {
            if (from_r > 4) {
                if (to_r >= from_r || dr > 1 || dc > 0) return false;
            }
            else {
                if (dr > 1 || dc > 1) return false;
                if (dr == 1 && dc != 0) return false;
                if (dc == 1 && dr != 0) return false;
                if (to_r > from_r) return false;
            }
        }
        else {
            if (from_r < 5) {
                if (to_r <= from_r || dr > 1 || dc > 0) return false;
            }
            else {
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
// --- 【新增1】辅助：找到某一方将/帅的位置 ---
POINT find_general_pos(Color color) {
    POINT pos = { -1, -1 };
    for (int r = 0; r < ROW_NUM; r++) {
        for (int c = 0; c < COL_NUM; c++) {
            if (board[r][c].color == color && board[r][c].type == GENERAL) {
                pos.x = c;
                pos.y = r;
                return pos;
            }
        }
    }
    return pos;
}

// --- 【新增2】核心：判断某一方是否被将军 ---
// color：要判断的一方（比如CHESS_RED，就是判断红方是否被黑方将军）
// --- 【新增2】核心：判断某一方是否被将军 ---
bool is_checked(Color color) {
    POINT general_pos = find_general_pos(color);
    if (general_pos.x == -1) return false;

    Color enemy_color = (color == CHESS_RED) ? CHESS_BLACK : CHESS_RED;

    // 【新增规则1】检查对方棋子是否能将军
    for (int r = 0; r < ROW_NUM; r++) {
        for (int c = 0; c < COL_NUM; c++) {
            if (board[r][c].color == enemy_color) {
                if (is_move_valid(r, c, general_pos.y, general_pos.x)) {
                    return true;
                }
            }
        }
    }

    // 【新增规则2】检查将帅是否直接照面（飞将）
    POINT enemy_general_pos = find_general_pos(enemy_color);
    if (enemy_general_pos.x == general_pos.x) { // 同一列
        // 检查中间是否有棋子
        int min_r = min(general_pos.y, enemy_general_pos.y);
        int max_r = max(general_pos.y, enemy_general_pos.y);
        bool has_piece_between = false;
        for (int r = min_r + 1; r < max_r; r++) {
            if (board[r][general_pos.x].color != CHESS_EMPTY) {
                has_piece_between = true;
                break;
            }
        }
        // 如果中间没有棋子，就是将帅照面，属于将军
        if (!has_piece_between) {
            return true;
        }
    }

    return false;
}

// --- 【新增3】核心：判断走某一步棋之后，己方是否安全（不会被将军） ---
// 原理：模拟走这步棋，检查是否被将军，再恢复棋盘
bool is_move_safe(int from_r, int from_c, int to_r, int to_c) {
    Color current_color = board[from_r][from_c].color;
    // 保存目标位置的原始棋子（用于恢复）
    ChessPiece old_target = board[to_r][to_c];

    // 【模拟走棋】
    board[to_r][to_c] = board[from_r][from_c];
    board[from_r][from_c].color = CHESS_EMPTY;
    board[from_r][from_c].type = TYPE_NONE;
    board[from_r][from_c].show = false;

    // 检查模拟走棋后，己方是否被将军
    bool safe = !is_checked(current_color);

    // 【恢复棋盘】
    board[from_r][from_c] = board[to_r][to_c];
    board[to_r][to_c] = old_target;

    return safe;
}
// --- 10. 胜负判断 ---
bool is_general_alive(Color color) {
    for (int r = 0; r < ROW_NUM; r++) {
        for (int c = 0; c < COL_NUM; c++) {
            if (board[r][c].color == color && board[r][c].type == GENERAL) {
                return true;
            }
        }
    }
    return false;
}

// --- 11. 执行棋子移动 ---
void move_piece(int from_r, int from_c, int to_r, int to_c) {
    board[to_r][to_c] = board[from_r][from_c];
    board[from_r][from_c].color = CHESS_EMPTY;
    board[from_r][from_c].type = TYPE_NONE;
    board[from_r][from_c].show = false;

    // 胜负判定
    if (!is_general_alive(CHESS_RED)) {
        game_over = true;
        _tcscpy_s(game_result, _countof(game_result), _T("游戏结束！黑方胜利！"));
    }
    else if (!is_general_alive(CHESS_BLACK)) {
        game_over = true;
        _tcscpy_s(game_result, _countof(game_result), _T("游戏结束！红方胜利！"));
    }

    // 切换回合
    if (!game_over) {
        turn = (turn == CHESS_RED) ? CHESS_BLACK : CHESS_RED;
    }
}

// --- 12. AI逻辑：生成所有合法走法 ---
void generate_all_moves(Color color, std::vector<ChessMove>& moves) {
    moves.clear();
    for (int from_r = 0; from_r < ROW_NUM; from_r++) {
        for (int from_c = 0; from_c < COL_NUM; from_c++) {
            if (board[from_r][from_c].color != color) continue;

            for (int to_r = 0; to_r < ROW_NUM; to_r++) {
                for (int to_c = 0; to_c < COL_NUM; to_c++) {
                    // AI的走法也必须合法且安全
                    if (is_move_valid(from_r, from_c, to_r, to_c)
                        && is_move_safe(from_r, from_c, to_r, to_c)) {
                        ChessMove move;
                        move.from_r = from_r;
                        move.from_c = from_c;
                        move.to_r = to_r;
                        move.to_c = to_c;
                        move.score = 0;

                        ChessPiece target = board[to_r][to_c];
                        if (target.color != CHESS_EMPTY) {
                            switch (target.type) {
                            case GENERAL: move.score = 10000; break;
                            case CHARIOT: move.score = 500; break;
                            case HORSE: move.score = 300; break;
                            case CANNON: move.score = 300; break;
                            case SOLDIER: move.score = 100; break;
                            default: move.score = 50; break;
                            }
                        }
                        else {
                            if (color == CHESS_BLACK && to_r > from_r) {
                                move.score += 10;
                            }
                        }
                        moves.push_back(move);
                    }
                }
            }
        }
    }
}

// --- 13. AI执行走棋 ---
void ai_move() {
    if (game_over || turn != CHESS_BLACK) return;

    std::vector<ChessMove> moves;
    generate_all_moves(CHESS_BLACK, moves);

    if (moves.empty()) return;

    int max_score = -1;
    int best_index = 0;
    for (int i = 0; i < moves.size(); i++) {
        if (moves[i].score > max_score) {
            max_score = moves[i].score;
            best_index = i;
        }
    }

    ChessMove best_move = moves[best_index];
    move_piece(best_move.from_r, best_move.from_c, best_move.to_r, best_move.to_c);
    repaint_all();
}

// --- 14. 【核心优化】美化后的模式选择界面（鼠标点击） ---
void select_game_mode() {
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    setbkcolor(RGB(240, 230, 200)); // 和棋盘统一的宣纸背景色
    BeginBatchDraw(); // 开启双缓冲，避免界面闪烁

    // 初始化两个按钮（居中排版）
    int btn_width = 300;
    int btn_height = 80;
    int btn_x = (WINDOW_WIDTH - btn_width) / 2; // 水平居中
    Button btn_two_player = { btn_x, 350, btn_width, btn_height, _T("双人对战"), false, MODE_TWO_PLAYER };
    Button btn_ai_player = { btn_x, 480, btn_width, btn_height, _T("人机对战（你执红方）"), false, MODE_AI };

    ExMessage msg;
    GameMode selected_mode = MODE_NONE;

    // 模式选择循环
    while (selected_mode == MODE_NONE) {
        // 监听鼠标消息
        while (peekmessage(&msg, EM_MOUSE)) {
            // 鼠标移动：更新hover状态
            if (msg.message == WM_MOUSEMOVE) {
                btn_two_player.is_hover = is_point_in_button(msg.x, msg.y, btn_two_player);
                btn_ai_player.is_hover = is_point_in_button(msg.x, msg.y, btn_ai_player);
            }
            // 鼠标左键点击：判断是否点击按钮
            if (msg.message == WM_LBUTTONDOWN) {
                if (is_point_in_button(msg.x, msg.y, btn_two_player)) {
                    selected_mode = MODE_TWO_PLAYER;
                }
                if (is_point_in_button(msg.x, msg.y, btn_ai_player)) {
                    selected_mode = MODE_AI;
                }
            }
        }

        // 重绘界面
        cleardevice();

        // 绘制标题
        settextcolor(RGB(120, 50, 20));
        setbkmode(TRANSPARENT);
        settextstyle(60, 0, _T("楷体"));
        int title_x = (WINDOW_WIDTH - textwidth(_T("中国象棋"))) / 2;
        outtextxy(title_x, 150, _T("中国象棋"));

        // 绘制副标题
        settextstyle(24, 0, _T("宋体"));
        int sub_title_x = (WINDOW_WIDTH - textwidth(_T("C++课程设计作品"))) / 2;
        outtextxy(sub_title_x, 230, _T("C++课程设计作品"));

        // 绘制按钮
        Button buttons[] = { btn_two_player, btn_ai_player };
        for (int i = 0; i < 2; i++) {
            Button btn = buttons[i];
            // 按钮填充色：hover时用深一点的颜色，有交互反馈
            if (btn.is_hover) {
                setfillcolor(RGB(220, 200, 170));
            }
            else {
                setfillcolor(RGB(245, 235, 210));
            }
            setlinecolor(RGB(120, 50, 20)); // 深棕色边框
            setlinestyle(PS_SOLID, 2);
            // 圆角矩形按钮，比直角更美观
            fillroundrect(btn.x, btn.y, btn.x + btn.width, btn.y + btn.height, 10, 10);

            // 绘制按钮文字（居中）
            settextcolor(RGB(80, 30, 10));
            settextstyle(28, 0, _T("楷体"));
            int text_x = btn.x + (btn.width - textwidth(btn.text)) / 2;
            int text_y = btn.y + (btn.height - textheight(btn.text)) / 2;
            outtextxy(text_x, text_y, btn.text);
        }

        // 绘制底部提示文字
        settextstyle(20, 0, _T("宋体"));
        int tip_x = (WINDOW_WIDTH - textwidth(_T("点击按钮选择游戏模式"))) / 2;
        outtextxy(tip_x, 650, _T("点击按钮选择游戏模式"));

        FlushBatchDraw(); // 整帧刷新

        // 按ESC键直接退出
        if (_kbhit() && _getch() == 27) {
            EndBatchDraw();
            closegraph();
            exit(0);
        }
    }

    // 选择完成，保存模式，关闭选择界面，初始化游戏窗口
    game_mode = selected_mode;
    EndBatchDraw();
    closegraph();

    // 重新初始化游戏主窗口
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    setbkcolor(RGB(240, 230, 200));
    BeginBatchDraw();
}

// --- 主函数 ---
int main() {
    // 1. 选择游戏模式（美化后的鼠标点击版）
    select_game_mode();

    // 2. 初始化游戏
    init_game();
    repaint_all();

    ExMessage msg;

    // 游戏主循环
    while (true) {
        // 游戏结束，仅响应ESC退出
        if (game_over) {
            if (_kbhit() && _getch() == 27) break;
            continue;
        }

        // 【新增】回合开始时，判断当前方是否被将死
        if (is_checked(turn)) {
            bool has_legal_move = false;
            // 遍历所有己方棋子，看有没有合法的、能解将的走法
            for (int from_r = 0; from_r < ROW_NUM; from_r++) {
                for (int from_c = 0; from_c < COL_NUM; from_c++) {
                    if (board[from_r][from_c].color != turn) continue;
                    // 遍历所有可能的落点
                    for (int to_r = 0; to_r < ROW_NUM; to_r++) {
                        for (int to_c = 0; to_c < COL_NUM; to_c++) {
                            if (is_move_valid(from_r, from_c, to_r, to_c)
                                && is_move_safe(from_r, from_c, to_r, to_c)) {
                                has_legal_move = true;
                                goto end_check; // 找到合法步，直接跳出循环
                            }
                        }
                    }
                }
            }
        end_check:
            // 没有任何合法走法，就是被将死，游戏结束
            if (!has_legal_move) {
                game_over = true;
                if (turn == CHESS_RED) {
                    _tcscpy_s(game_result, _countof(game_result), _T("游戏结束！黑方胜利！红方被将死！"));
                }
                else {
                    _tcscpy_s(game_result, _countof(game_result), _T("游戏结束！红方胜利！黑方被将死！"));
                }
                repaint_all();
                continue;
            }
        }

        // 人机模式，AI自动走棋
        if (game_mode == MODE_AI && turn == CHESS_BLACK) {
            ai_move();
            continue;
        }

        // 玩家鼠标交互
        if (peekmessage(&msg, EM_MOUSE)) {
            if (msg.message == WM_LBUTTONDOWN) {
                int click_r, click_c;
                if (click_to_board(msg.x, msg.y, click_r, click_c)) {

                    if (!is_selected) {
                        // 选中当前回合的己方棋子
                        if (board[click_r][click_c].color == turn) {
                            selected_row = click_r;
                            selected_col = click_c;
                            is_selected = true;
                            repaint_all();
                        }
                    }
                    else {
                        // 切换选中己方棋子
                        if (board[click_r][click_c].color == turn) {
                            selected_row = click_r;
                            selected_col = click_c;
                            repaint_all();
                        }
                        // 尝试移动棋子
                        else
                        {
                            // 必须同时满足：棋子走法合法 + 走棋后自己不会被将军
                            if (is_move_valid(selected_row, selected_col, click_r, click_c)
                                && is_move_safe(selected_row, selected_col, click_r, click_c)) {
                                move_piece(selected_row, selected_col, click_r, click_c);
                            }
                            // 无论是否成功，都取消选中
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

    // 资源释放
    EndBatchDraw();
    closegraph();
    return 0;
}