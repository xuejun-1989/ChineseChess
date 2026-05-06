#define _CRT_SECURE_NO_WARNINGS
// ========== 音效依赖 ==========
#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
// ==============================
#pragma comment(lib, "msimg32.lib") // 提供 AlphaBlend 函数
#include <graphics.h>
#include <conio.h>
#include <cstring>
#include <tchar.h>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <ctime>

// --- 全局参数与类型定义 ---
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

// ===================== 结构体声明（顺序固定，修复未定义错误）=====================
struct ChessPiece {
    Color color;
    Type type;
    bool show;
};
struct ChessMove {
    int from_r;
    int from_c;
    int to_r;
    int to_c;
    int score;
};
struct Button {
    int x;
    int y;
    int width;
    int height;
    TCHAR text[32];
    bool is_hover;
    GameMode mode;
};
struct UndoButton {
    int x, y, w, h;
    TCHAR text[16];
    bool hover;
};
struct SkillButton {
    int x, y, w, h;
    TCHAR text[16];
    bool is_active;
    bool is_hover;
};
struct FogBladeState {
    bool is_flying;
    int current_r;
    int current_c;
    int direction;
    int frame_count;
};

enum MoveType { MOVE_NORMAL, MOVE_FOG, MOVE_INVISIBLE };

struct StepRecord {
    int from_r, from_c;
    int to_r, to_c;
    ChessPiece old_target;
    Color old_turn;
    bool old_game_over;
    int last_fr, last_fc, last_tr, last_tc;
    bool has_last;
    MoveType move_type;               // 新增
    bool old_show_jack;               // 恢复杰克形态显示
    bool old_invisible;               // 恢复隐身模式
    int old_skill_r, old_skill_c;     // 恢复选中棋子
    bool old_fog_active, old_invis_active; // 恢复按钮激活
};
// =====================================================================================

// ========== 函数前置声明 ==========
POINT get_pos(int row, int col);
bool is_point_in_button(int x, int y, Button btn);
void draw_chessboard();
void draw_last_step();
void draw_piece(int row, int col);
void init_game();
void repaint_all();
bool click_to_board(int x, int y, int& row, int& col);
int count_pieces_between(int r1, int c1, int r2, int c2);
bool is_move_valid(int from_r, int from_c, int to_r, int to_c);
POINT find_general_pos(Color color);
bool is_checked(Color color);
bool is_move_safe(int from_r, int from_c, int to_r, int to_c);
bool is_general_alive(Color color);
void move_piece(int from_r, int from_c, int to_r, int to_c);
void undo_move();
bool is_in_undo_btn(int x, int y);
int getPieceValue(Type t);
int evaluate();
void getAllLegalMoves(Color color, std::vector<ChessMove>& moves);
void fakeMove(int fr, int fc, int tr, int tc, ChessPiece& oldTar);
void undoMove(int fr, int fc, int tr, int tc, ChessPiece& oldTar);
int alphaBeta(int depth, int alpha, int beta, bool isMaxTurn);
void ai_move();
void select_game_mode();
// 杰克技能函数
void activate_fog_blade();
void update_fog_blade();
void activate_invisible();
void execute_invisible_move(int to_r, int to_c);
// ==================================

// ===================== 全局状态变量（仅定义一次，无重定义）=====================
ChessPiece board[ROW_NUM][COL_NUM];
int selected_row = -1;
int selected_col = -1;
bool is_selected = false;
Color turn = CHESS_RED;
GameMode game_mode = MODE_NONE;
bool game_over = false;
TCHAR game_result[64];

// 上一步走棋高亮
int last_from_r = -1;
int last_from_c = -1;
int last_to_r = -1;
int last_to_c = -1;
bool has_last_step = false;

// 悔棋按钮
UndoButton undo_btn = { 610, 120, 100, 50, _T("悔棋"), false };
std::vector<StepRecord> move_history;

// 杰克技能全局变量
SkillButton btn_fog_blade;
SkillButton btn_invisible;
FogBladeState fog_blade = { false, -1, -1, 0, 0 };
bool skill_mode = false;
bool invisible_mode = false;
int skill_piece_r = -1;
int skill_piece_c = -1;
bool show_jack_form = false;

// ========== 【新增】技能图片全局变量 ==========
IMAGE img_fog_active, img_fog_disable;
IMAGE img_invis_active, img_invis_disable;
IMAGE img_jack_fog, img_jack_invis;
IMAGE img_fog_slash;   // 雾刃飞行动画图片
IMAGE img_hover_mask;  // 半透明白色遮罩 (100x50)
IMAGE img_undo;
bool img_load_success = false; // 图片加载成功标记
// ==============================================

// EasyX 透明绘图函数 (使用 GDI AlphaBlend 实现)
void putimage_alpha(int x, int y, IMAGE* pSrcImg)
{
    if (!pSrcImg) return;

    // 获取窗口的 HDC（即绘制目标）
    HDC hdc = GetImageHDC(NULL);
    // 获取源图像的 HDC（它的所有绘图信息都在这里）
    HDC hdcSrc = GetImageHDC(pSrcImg);

    // 设置混合模式：使用源图像的 Alpha 通道
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

    // 执行带透明通道的绘制
    AlphaBlend(hdc, x, y, pSrcImg->getwidth(), pSrcImg->getheight(),
        hdcSrc, 0, 0, pSrcImg->getwidth(), pSrcImg->getheight(),
        bf);

    // 注意：不要释放 GetImageHDC 返回的 DC，EasyX 内部会自行管理
}

// ========== 音效播放函数 ==========
void play_sound(LPCTSTR sound_file) {
    TCHAR cmd[512];
    mciSendString(_T("close all"), NULL, 0, NULL);
    _stprintf_s(cmd, _T("open \"%s\" type mpegvideo alias sound"), sound_file);
    if (mciSendString(cmd, NULL, 0, NULL) != 0) return;
    mciSendString(_T("play sound from 0"), NULL, 0, NULL);
}

// --- 辅助工具函数 ---
POINT get_pos(int row, int col) {
    POINT p;
    p.x = LEFT_MARGIN + col * GRID_SIZE;
    p.y = TOP_MARGIN + row * GRID_SIZE;
    return p;
}

bool is_point_in_button(int x, int y, Button btn) {
    return x >= btn.x && x <= btn.x + btn.width && y >= btn.y && y <= btn.y + btn.height;
}

// --- 棋盘绘制 ---
void draw_chessboard() {
    setlinecolor(BLACK);
    setlinestyle(PS_SOLID, 2);
    // 横线
    for (int i = 0; i < ROW_NUM; i++) {
        POINT p_start = get_pos(i, 0);
        POINT p_end = get_pos(i, COL_NUM - 1);
        line(p_start.x, p_start.y, p_end.x, p_end.y);
    }
    // 竖线（楚河汉界断开）
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

// 绘制上一步走棋高亮
void draw_last_step() {
    if (!has_last_step) return;
    COLORREF old_color = getcolor();
    // 起点蓝色边框
    POINT from_pos = get_pos(last_from_r, last_from_c);
    setcolor(RGB(0, 150, 255));
    rectangle(from_pos.x - GRID_SIZE / 2 + 1, from_pos.y - GRID_SIZE / 2 + 1,
        from_pos.x + GRID_SIZE / 2 - 1, from_pos.y + GRID_SIZE / 2 - 1);
    setcolor(WHITE);
    rectangle(from_pos.x - GRID_SIZE / 2 + 3, from_pos.y - GRID_SIZE / 2 + 3,
        from_pos.x + GRID_SIZE / 2 - 3, from_pos.y + GRID_SIZE / 2 - 3);
    // 终点红色边框
    POINT to_pos = get_pos(last_to_r, last_to_c);
    setcolor(RGB(255, 100, 100));
    rectangle(to_pos.x - GRID_SIZE / 2 + 1, to_pos.y - GRID_SIZE / 2 + 1,
        to_pos.x + GRID_SIZE / 2 - 1, to_pos.y + GRID_SIZE / 2 - 1);
    setcolor(WHITE);
    rectangle(to_pos.x - GRID_SIZE / 2 + 3, to_pos.y - GRID_SIZE / 2 + 3,
        to_pos.x + GRID_SIZE / 2 - 3, to_pos.y + GRID_SIZE / 2 - 3);
    setcolor(old_color);
}

// --- 棋子绘制（含杰克图片形态）---
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

    // ========== 杰克形态：用你准备的图片绘制 ==========
    if (show_jack_form && row == skill_piece_r && col == skill_piece_c) {
        // 雾刃用jack_fog.png，隐身用jack_invisible.png
        IMAGE* use_jack = invisible_mode ? &img_jack_invis : &img_jack_fog;
        if (img_load_success) {
            // 50x50图片居中绘制
            putimage_alpha(pos.x - 25, pos.y - 25, use_jack);
        }
        // 图片加载失败兜底：紫色杰克
        else {
            setfillcolor(RGB(128, 0, 128));
            setlinecolor(BLACK);
            fillcircle(pos.x, pos.y, radius);
            settextcolor(WHITE);
            setbkmode(TRANSPARENT);
            settextstyle(20, 0, _T("黑体"));
            outtextxy(pos.x - 10, pos.y - 10, _T("J"));
        }
        return;
    }

    // 隐身状态：半透明图片
    if (invisible_mode && row == skill_piece_r && col == skill_piece_c) {
        if (img_load_success) {
            putimage_alpha(pos.x - 25, pos.y - 25, &img_jack_invis);
        }
        else {
            setfillcolor(RGB(200, 200, 200));
            setlinecolor(RGB(150, 150, 150));
            setlinestyle(PS_DASH, 2);
            fillcircle(pos.x, pos.y, radius);
        }
        return;
    }
    // ==================================================

    // 普通棋子绘制
    setfillcolor(RGB(255, 250, 200));
    setlinecolor(BLACK);
    setlinestyle(PS_SOLID, 2);
    fillcircle(pos.x, pos.y, radius);

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

    setbkmode(TRANSPARENT);
    settextstyle(36, 0, _T("楷体"));
    int tx = pos.x - textwidth(text) / 2;
    int ty = pos.y - textheight(text) / 2;
    outtextxy(tx, ty, text);
}

// --- 游戏初始化（含图片加载）---
void init_game() {
    // ========== 加载技能图片 ==========
    // 加载按钮图片，固定100x50尺寸，自动拉伸适配
    loadimage(&img_fog_active, _T("fog_blade_active.png"), 0, 0);
    loadimage(&img_fog_disable, _T("fog_blade_disable.png"), 0, 0);
    loadimage(&img_invis_active, _T("invisible_active.png"), 0, 0);
    loadimage(&img_invis_disable, _T("invisible_disable.png"), 0, 0);
    // 加载杰克棋子图片，固定50x50
    loadimage(&img_jack_fog, _T("jack_fog.png"), 50, 50);
    loadimage(&img_jack_invis, _T("jack_invisible.png"), 50, 50);
    //雾刃图
    loadimage(&img_fog_slash, _T("fog_slash.png"), 0, 0);  
    
    loadimage(&img_undo, _T("undo.png"), 0, 0);   // 按钮大小
    
    img_load_success = (img_fog_active.getwidth()  > 0);
    // ==============================================
    // 初始化技能按钮
    btn_fog_blade.x = 610;
    btn_fog_blade.y = 200;
    btn_fog_blade.w = 50;
    btn_fog_blade.h = 50;
    _tcscpy_s(btn_fog_blade.text, _countof(btn_fog_blade.text), _T("雾刃"));
    btn_fog_blade.is_active = false;
    btn_fog_blade.is_hover = false;

    btn_invisible.x = 610;
    btn_invisible.y = 280;
    btn_invisible.w = 50;
    btn_invisible.h = 50;
    _tcscpy_s(btn_invisible.text, _countof(btn_invisible.text), _T("隐身"));
    btn_invisible.is_active = false;
    btn_invisible.is_hover = false;

    // 重置技能状态
    fog_blade = { false, -1, -1, 0, 0 };
    skill_mode = false;
    invisible_mode = false;
    skill_piece_r = -1;
    skill_piece_c = -1;
    show_jack_form = false;

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

    // ========== 生成半透明悬停遮罩 ==========
    img_hover_mask.Resize(75, 75);                     // 与按钮同大
    DWORD* buf = GetImageBuffer(&img_hover_mask);
    if (buf) {
        // 构造带 Alpha 的白色像素 (A=80, B=255, G=255, R=255)
        DWORD color = (80 << 24) | (255 << 16) | (255 << 8) | 255;
        int total = 75 * 75;
        for (int i = 0; i < total; i++) {
            buf[i] = color;
        }
    }
    // ==========================================
    
    // 重置游戏状态
    selected_row = -1;
    selected_col = -1;
    is_selected = false;
    turn = CHESS_RED;
    game_over = false;
    memset(game_result, 0, sizeof(game_result));
    srand((unsigned int)time(NULL));
    move_history.clear();
    last_from_r = last_from_c = last_to_r = last_to_c = -1;
    has_last_step = false;
}

// --- 全局画面重绘（含图片按钮绘制）---
void repaint_all() {
    cleardevice();
    draw_chessboard();
    draw_last_step();
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
        if (turn == CHESS_RED) {
            outtextxy(100, 50, _T("当前回合：红方"));
            if (is_checked(CHESS_RED)) {
                settextcolor(RED);
                outtextxy(100, 80, _T("警告！红方被将军！"));
            }
        }
        else {
            outtextxy(100, 50, _T("当前回合：黑方"));
            if (is_checked(CHESS_BLACK)) {
                settextcolor(RED);
                outtextxy(100, 80, _T("警告！黑方被将军！"));
            }
        }
    }

    // 绘制悔棋按钮（用图片）
    if (img_undo.getwidth() > 0) {
        putimage_alpha(undo_btn.x, undo_btn.y, &img_undo);
        // 悬停半透明遮罩（与技能按钮相同）
        if (undo_btn.hover) {
            putimage_alpha(undo_btn.x, undo_btn.y, &img_hover_mask);
        }
    }
    else {
        // 兜底：没加载到图片时用文字按钮
        setfillcolor(undo_btn.hover ? RGB(255, 200, 200) : RGB(255, 230, 230));
        setcolor(BLACK);
        fillrectangle(undo_btn.x, undo_btn.y, undo_btn.x + undo_btn.w, undo_btn.y + undo_btn.h);
        settextcolor(BLACK);
        setbkmode(TRANSPARENT);
        settextstyle(24, 0, _T("黑体"));
        int tx = undo_btn.x + (undo_btn.w - textwidth(undo_btn.text)) / 2;
        int ty = undo_btn.y + (undo_btn.h - textheight(undo_btn.text)) / 2;
        outtextxy(tx, ty, undo_btn.text);
    }

    // ========== 【修改】绘制技能按钮（用图片）==========
// 1. 雾刃按钮
    if (img_load_success) {
        IMAGE* use_img = btn_fog_blade.is_active ? &img_fog_active : &img_fog_disable;
        putimage_alpha(btn_fog_blade.x, btn_fog_blade.y, use_img);
        // 悬停时画半透明遮罩
        if (btn_fog_blade.is_hover && btn_fog_blade.is_active) {
            putimage_alpha(btn_fog_blade.x, btn_fog_blade.y, &img_hover_mask);
        }
    }
    // 图片加载失败兜底：纯色按钮
    else {
        COLORREF fb_color = btn_fog_blade.is_active ? RGB(100, 150, 255) : RGB(150, 150, 150);
        COLORREF fb_hover = btn_fog_blade.is_active ? RGB(150, 180, 255) : RGB(180, 180, 180);
        setfillcolor(btn_fog_blade.is_hover ? fb_hover : fb_color);
        setcolor(BLACK);
        fillrectangle(btn_fog_blade.x, btn_fog_blade.y, btn_fog_blade.x + btn_fog_blade.w, btn_fog_blade.y + btn_fog_blade.h);
        settextcolor(btn_fog_blade.is_active ? WHITE : RGB(100, 100, 100));
        setbkmode(TRANSPARENT);
        settextstyle(20, 0, _T("黑体"));
        int ftx = btn_fog_blade.x + (btn_fog_blade.w - textwidth(btn_fog_blade.text)) / 2;
        int fty = btn_fog_blade.y + (btn_fog_blade.h - textheight(btn_fog_blade.text)) / 2;
        outtextxy(ftx, fty, btn_fog_blade.text);
    }

    // 2. 隐身按钮
    if (img_load_success) {
        IMAGE* use_img = btn_invisible.is_active ? &img_invis_active : &img_invis_disable;
        putimage_alpha(btn_invisible.x, btn_invisible.y, use_img);
        if (btn_invisible.is_hover && btn_invisible.is_active) {
            putimage_alpha(btn_invisible.x, btn_invisible.y, &img_hover_mask);
        }
    }
    // 图片加载失败兜底：纯色按钮
    else {
        COLORREF inv_color = btn_invisible.is_active ? RGB(200, 150, 255) : RGB(150, 150, 150);
        COLORREF inv_hover = btn_invisible.is_active ? RGB(220, 180, 255) : RGB(180, 180, 180);
        setfillcolor(btn_invisible.is_hover ? inv_hover : inv_color);
        fillrectangle(btn_invisible.x, btn_invisible.y, btn_invisible.x + btn_invisible.w, btn_invisible.y + btn_invisible.h);
        settextcolor(btn_invisible.is_active ? WHITE : RGB(100, 100, 100));
        int itx = btn_invisible.x + (btn_invisible.w - textwidth(btn_invisible.text)) / 2;
        int ity = btn_invisible.y + (btn_invisible.h - textheight(btn_invisible.text)) / 2;
        outtextxy(itx, ity, btn_invisible.text);
    }
    // ==================================================

 // 绘制雾刃动画（使用透明图片）
    if (fog_blade.is_flying && img_fog_slash.getwidth() > 0) {
        POINT pos = get_pos(fog_blade.current_r, fog_blade.current_c);
        // 图片 50x50 居中绘制（假设图片尺寸为 50x50，如果不是请调整）
        int drawX = pos.x - img_fog_slash.getwidth() / 2;
        int drawY = pos.y - img_fog_slash.getheight() / 2;
        putimage_alpha(drawX, drawY, &img_fog_slash);
    }

    FlushBatchDraw();
}

// --- 鼠标点击转棋盘行列 ---
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

// --- 计算两点间棋子数量 ---
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

// --- 走棋规则校验 ---
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
                if ((dr == 1 && dc != 0) || (dc == 1 && dr != 0)) {
                    if (to_r > from_r) return false;
                }
            }
        }
        else {
            if (from_r < 5) {
                if (to_r <= from_r || dr > 1 || dc > 0) return false;
            }
            else {
                if (dr > 1 || dc > 1) return false;
                if ((dr == 1 && dc != 0) || (dc == 1 && dr != 0)) {
                    if (to_r < from_r) return false;
                }
            }
        }
        return true;
    }
    default:
        return false;
    }
}

// --- 找到将帅位置 ---
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

// --- 判断是否被将军 ---
bool is_checked(Color color) {
    POINT general_pos = find_general_pos(color);
    if (general_pos.x == -1) return false;
    Color enemy_color = (color == CHESS_RED) ? CHESS_BLACK : CHESS_RED;

    for (int r = 0; r < ROW_NUM; r++) {
        for (int c = 0; c < COL_NUM; c++) {
            if (board[r][c].color == enemy_color) {
                if (is_move_valid(r, c, general_pos.y, general_pos.x)) {
                    return true;
                }
            }
        }
    }

    POINT enemy_general_pos = find_general_pos(enemy_color);
    if (enemy_general_pos.x == general_pos.x) {
        int min_r = min(general_pos.y, enemy_general_pos.y);
        int max_r = max(general_pos.y, enemy_general_pos.y);
        bool has_piece_between = false;
        for (int r = min_r + 1; r < max_r; r++) {
            if (board[r][general_pos.x].color != CHESS_EMPTY) {
                has_piece_between = true;
                break;
            }
        }
        if (!has_piece_between) return true;
    }
    return false;
}

// --- 判断走棋是否安全 ---
bool is_move_safe(int from_r, int from_c, int to_r, int to_c) {
    Color current_color = board[from_r][from_c].color;
    ChessPiece old_target = board[to_r][to_c];

    board[to_r][to_c] = board[from_r][from_c];
    board[from_r][from_c].color = CHESS_EMPTY;
    board[from_r][from_c].type = TYPE_NONE;
    board[from_r][from_c].show = false;

    bool safe = !is_checked(current_color);

    board[from_r][from_c] = board[to_r][to_c];
    board[to_r][to_c] = old_target;
    return safe;
}

// --- 判断将帅是否存活 ---
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

// --- 执行棋子移动 ---
void move_piece(int from_r, int from_c, int to_r, int to_c) {
    StepRecord rec;
    rec.from_r = from_r; rec.from_c = from_c;
    rec.to_r = to_r; rec.to_c = to_c;
    rec.old_target = board[to_r][to_c];
    rec.old_turn = turn;
    rec.old_game_over = game_over;
    rec.last_fr = last_from_r; rec.last_fc = last_from_c;
    rec.last_tr = last_to_r; rec.last_tc = last_to_c;
    rec.has_last = has_last_step;
    rec.move_type = MOVE_NORMAL;
    rec.old_show_jack = show_jack_form;
    rec.old_invisible = invisible_mode;
    rec.old_skill_r = skill_piece_r;
    rec.old_skill_c = skill_piece_c;
    rec.old_fog_active = btn_fog_blade.is_active;
    rec.old_invis_active = btn_invisible.is_active;
    move_history.push_back(rec);

    last_from_r = from_r; last_from_c = from_c;
    last_to_r = to_r; last_to_c = to_c;
    has_last_step = true;

    bool is_eat = (board[to_r][to_c].color != CHESS_EMPTY);
    board[to_r][to_c] = board[from_r][from_c];
    board[from_r][from_c].color = CHESS_EMPTY;
    board[from_r][from_c].type = TYPE_NONE;
    board[from_r][from_c].show = false;

    if (is_eat) play_sound(_T("吃子.mp3"));
    else play_sound(_T("落子.mp3"));

    if (!is_general_alive(CHESS_RED)) {
        game_over = true;
        _tcscpy_s(game_result, _T("游戏结束！黑方胜利！"));
    }
    else if (!is_general_alive(CHESS_BLACK)) {
        game_over = true;
        _tcscpy_s(game_result, _T("游戏结束！红方胜利！"));
    }

    if (!game_over) turn = (turn == CHESS_RED) ? CHESS_BLACK : CHESS_RED;
}

// --- 悔棋函数 ---
void undo_move() {
    if (move_history.empty() || game_over) return;
    StepRecord rec = move_history.back();
    move_history.pop_back();

    // 恢复棋盘
    if (rec.move_type == MOVE_FOG) {
        // 雾刃：只恢复目标位置的棋子，源位置不动
        board[rec.to_r][rec.to_c] = rec.old_target;
    }
    else {
        // 普通走棋 / 隐身移动：标准恢复
        board[rec.from_r][rec.from_c] = board[rec.to_r][rec.to_c];
        board[rec.to_r][rec.to_c] = rec.old_target;
    }

    // 恢复全局游戏状态
    turn = rec.old_turn;
    game_over = rec.old_game_over;

    // 恢复特殊机制状态
    show_jack_form = rec.old_show_jack;
    invisible_mode = rec.old_invisible;
    skill_piece_r = rec.old_skill_r;
    skill_piece_c = rec.old_skill_c;
    btn_fog_blade.is_active = rec.old_fog_active;
    btn_invisible.is_active = rec.old_invis_active;

    // 恢复上一步高亮
    last_from_r = rec.last_fr;
    last_from_c = rec.last_fc;
    last_to_r = rec.last_tr;
    last_to_c = rec.last_tc;
    has_last_step = rec.has_last;

    repaint_all();
}

// --- 判断是否在悔棋按钮上 ---
bool is_in_undo_btn(int x, int y) {
    return x >= undo_btn.x && x <= undo_btn.x + undo_btn.w
        && y >= undo_btn.y && y <= undo_btn.y + undo_btn.h;
}

// ===================== 杰克技能核心函数 =====================
void activate_fog_blade() {
    if (skill_piece_r == -1) return;
    fog_blade.is_flying = true;
    fog_blade.current_r = skill_piece_r;
    fog_blade.current_c = skill_piece_c;
    fog_blade.direction = (board[skill_piece_r][skill_piece_c].color == CHESS_RED) ? -1 : 1;
    fog_blade.frame_count = 0;
    show_jack_form = true;
    repaint_all();
}

void update_fog_blade() {
    if (!fog_blade.is_flying) return;
    fog_blade.frame_count++;
    if (fog_blade.frame_count % 5 == 0) {
        fog_blade.current_r += fog_blade.direction;
        if (fog_blade.current_r < 0 || fog_blade.current_r >= ROW_NUM) {
            fog_blade.is_flying = false;
            show_jack_form = false;
            turn = (turn == CHESS_RED) ? CHESS_BLACK : CHESS_RED;
            is_selected = false;
            selected_row = selected_col = -1;
            skill_piece_r = skill_piece_c = -1;
            btn_fog_blade.is_active = false;
            btn_invisible.is_active = false;
            repaint_all();
            return;
        }
        if (board[fog_blade.current_r][fog_blade.current_c].color != CHESS_EMPTY) {
            fog_blade.is_flying = false;
            show_jack_form = false;
            StepRecord rec;
            rec.from_r = skill_piece_r;
            rec.from_c = skill_piece_c;
            rec.to_r = fog_blade.current_r;
            rec.to_c = fog_blade.current_c;
            rec.old_target = board[fog_blade.current_r][fog_blade.current_c];
            rec.old_turn = turn;
            rec.old_game_over = game_over;
            rec.last_fr = last_from_r; rec.last_fc = last_from_c;
            rec.last_tr = last_to_r; rec.last_tc = last_to_c;
            rec.has_last = has_last_step;
            rec.move_type = MOVE_FOG;
            rec.old_show_jack = show_jack_form;
            rec.old_invisible = false;   // 雾刃时没有隐身
            rec.old_skill_r = skill_piece_r;
            rec.old_skill_c = skill_piece_c;
            rec.old_fog_active = btn_fog_blade.is_active;
            rec.old_invis_active = btn_invisible.is_active;
            move_history.push_back(rec);

            if (board[fog_blade.current_r][fog_blade.current_c].type == GENERAL) {
                game_over = true;
                Color winner = (turn == CHESS_RED) ? CHESS_RED : CHESS_BLACK;
                _tcscpy_s(game_result, _countof(game_result),
                    winner == CHESS_RED ? _T("雾刃命中！红方胜利！") : _T("雾刃命中！黑方胜利！"));
            }
            else {
                board[fog_blade.current_r][fog_blade.current_c].color = CHESS_EMPTY;
                board[fog_blade.current_r][fog_blade.current_c].type = TYPE_NONE;
                board[fog_blade.current_r][fog_blade.current_c].show = false;
            }
            turn = (turn == CHESS_RED) ? CHESS_BLACK : CHESS_RED;
            is_selected = false;
            selected_row = selected_col = -1;
            skill_piece_r = skill_piece_c = -1;
            btn_fog_blade.is_active = false;
            btn_invisible.is_active = false;
            repaint_all();
            return;
        }
    }
    repaint_all();
}

void activate_invisible() {
    if (skill_piece_r == -1) return;
    invisible_mode = true;
    show_jack_form = true;
    repaint_all();
}

void execute_invisible_move(int to_r, int to_c) {
    if (board[to_r][to_c].color != CHESS_EMPTY) return;
    StepRecord rec;
    rec.from_r = skill_piece_r;
    rec.from_c = skill_piece_c;
    rec.to_r = to_r;
    rec.to_c = to_c;
    rec.old_target = board[to_r][to_c];
    rec.old_turn = turn;
    rec.old_game_over = game_over;
    rec.last_fr = last_from_r; rec.last_fc = last_from_c;
    rec.last_tr = last_to_r; rec.last_tc = last_to_c;
    rec.has_last = has_last_step;
    rec.move_type = MOVE_INVISIBLE;
    rec.old_show_jack = show_jack_form;
    rec.old_invisible = invisible_mode;
    rec.old_skill_r = skill_piece_r;
    rec.old_skill_c = skill_piece_c;
    rec.old_fog_active = btn_fog_blade.is_active;
    rec.old_invis_active = btn_invisible.is_active;
    move_history.push_back(rec);

    board[to_r][to_c] = board[skill_piece_r][skill_piece_c];
    board[skill_piece_r][skill_piece_c].color = CHESS_EMPTY;
    board[skill_piece_r][skill_piece_c].type = TYPE_NONE;
    board[skill_piece_r][skill_piece_c].show = false;

    invisible_mode = false;
    show_jack_form = false;
    turn = (turn == CHESS_RED) ? CHESS_BLACK : CHESS_RED;
    is_selected = false;
    selected_row = -1;
    selected_col = -1;
    skill_piece_r = skill_piece_c = -1;
    btn_fog_blade.is_active = false;
    btn_invisible.is_active = false;
    repaint_all();
}
// ============================================================

// --- AI相关函数 ---
enum PieceValue {
    VAL_GENERAL = 10000,
    VAL_CHARIOT = 900,
    VAL_CANNON = 450,
    VAL_HORSE = 400,
    VAL_ADVISOR = 200,
    VAL_ELEPHANT = 200,
    VAL_SOLDIER = 100
};

int getPieceValue(Type t) {
    switch (t) {
    case GENERAL:return VAL_GENERAL;
    case CHARIOT:return VAL_CHARIOT;
    case CANNON: return VAL_CANNON;
    case HORSE:  return VAL_HORSE;
    case ADVISOR:return VAL_ADVISOR;
    case ELEPHANT:return VAL_ELEPHANT;
    case SOLDIER:return VAL_SOLDIER;
    default:return 0;
    }
}

int evaluate() {
    int score = 0;
    for (int r = 0;r < ROW_NUM;r++) {
        for (int c = 0;c < COL_NUM;c++) {
            ChessPiece p = board[r][c];
            if (p.color == CHESS_EMPTY) continue;
            int val = getPieceValue(p.type);
            if (p.color == CHESS_BLACK) {
                score += val;
                score += r * 2;
            }
            else {
                score -= val;
                score -= (9 - r) * 2;
            }
        }
    }
    if (is_checked(CHESS_BLACK)) score -= 800;
    if (is_checked(CHESS_RED))  score += 800;
    return score;
}

void getAllLegalMoves(Color color, std::vector<ChessMove>& moves) {
    moves.clear();
    for (int fr = 0;fr < ROW_NUM;fr++) {
        for (int fc = 0;fc < COL_NUM;fc++) {
            if (board[fr][fc].color != color) continue;
            for (int tr = 0;tr < ROW_NUM;tr++) {
                for (int tc = 0;tc < COL_NUM;tc++) {
                    if (is_move_valid(fr, fc, tr, tc) && is_move_safe(fr, fc, tr, tc)) {
                        ChessMove m;
                        m.from_r = fr; m.from_c = fc;
                        m.to_r = tr;   m.to_c = tc;
                        m.score = 0;
                        moves.push_back(m);
                    }
                }
            }
        }
    }
}

void fakeMove(int fr, int fc, int tr, int tc, ChessPiece& oldTar) {
    oldTar = board[tr][tc];
    board[tr][tc] = board[fr][fc];
    board[fr][fc].color = CHESS_EMPTY;
    board[fr][fc].type = TYPE_NONE;
    board[fr][fc].show = false;
}

void undoMove(int fr, int fc, int tr, int tc, ChessPiece& oldTar) {
    board[fr][fc] = board[tr][tc];
    board[tr][tc] = oldTar;
}

int alphaBeta(int depth, int alpha, int beta, bool isMaxTurn) {
    if (depth == 0) return evaluate();
    Color me = isMaxTurn ? CHESS_BLACK : CHESS_RED;
    std::vector<ChessMove> moves;
    getAllLegalMoves(me, moves);
    if (moves.empty()) return isMaxTurn ? -100000 : 100000;

    if (isMaxTurn) {
        int best = -999999;
        for (auto& m : moves) {
            ChessPiece oldTar;
            fakeMove(m.from_r, m.from_c, m.to_r, m.to_c, oldTar);
            int val = alphaBeta(depth - 1, alpha, beta, false);
            best = max(best, val);
            alpha = max(alpha, best);
            undoMove(m.from_r, m.from_c, m.to_r, m.to_c, oldTar);
            if (beta <= alpha) break;
        }
        return best;
    }
    else {
        int best = 999999;
        for (auto& m : moves) {
            ChessPiece oldTar;
            fakeMove(m.from_r, m.from_c, m.to_r, m.to_c, oldTar);
            int val = alphaBeta(depth - 1, alpha, beta, true);
            best = min(best, val);
            beta = min(beta, best);
            undoMove(m.from_r, m.from_c, m.to_r, m.to_c, oldTar);
            if (beta <= alpha) break;
        }
        return best;
    }
}

void ai_move() {
    if (game_over || turn != CHESS_BLACK) return;
    std::vector<ChessMove> moves;
    getAllLegalMoves(CHESS_BLACK, moves);
    if (moves.empty()) return;

    int bestVal = -999999;
    ChessMove bestMove = moves[0];
    for (auto& m : moves) {
        ChessPiece oldTar;
        fakeMove(m.from_r, m.from_c, m.to_r, m.to_c, oldTar);
        int val = alphaBeta(3, -999999, 999999, false);
        undoMove(m.from_r, m.from_c, m.to_r, m.to_c, oldTar);
        if (val > bestVal) {
            bestVal = val;
            bestMove = m;
        }
    }
    move_piece(bestMove.from_r, bestMove.from_c, bestMove.to_r, bestMove.to_c);
    repaint_all();
}

// --- 模式选择界面 ---
void select_game_mode() {
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    setbkcolor(RGB(240, 230, 200));
    BeginBatchDraw();

    int btn_width = 300;
    int btn_height = 80;
    int btn_x = (WINDOW_WIDTH - btn_width) / 2;
    Button btn_two_player = { btn_x, 350, btn_width, btn_height, _T("双人对战"), false, MODE_TWO_PLAYER };
    Button btn_ai_player = { btn_x, 480, btn_width, btn_height, _T("人机对战（你执红方）"), false, MODE_AI };

    ExMessage msg;
    GameMode selected_mode = MODE_NONE;

    while (selected_mode == MODE_NONE) {
        while (peekmessage(&msg, EM_MOUSE)) {
            if (msg.message == WM_MOUSEMOVE) {
                btn_two_player.is_hover = is_point_in_button(msg.x, msg.y, btn_two_player);
                btn_ai_player.is_hover = is_point_in_button(msg.x, msg.y, btn_ai_player);
            }
            if (msg.message == WM_LBUTTONDOWN) {
                if (is_point_in_button(msg.x, msg.y, btn_two_player)) {
                    selected_mode = MODE_TWO_PLAYER;
                }
                if (is_point_in_button(msg.x, msg.y, btn_ai_player)) {
                    selected_mode = MODE_AI;
                }
            }
        }

        cleardevice();
        settextcolor(RGB(120, 50, 20));
        setbkmode(TRANSPARENT);
        settextstyle(60, 0, _T("楷体"));
        int title_x = (WINDOW_WIDTH - textwidth(_T("中国象棋"))) / 2;
        outtextxy(title_x, 150, _T("中国象棋"));

        settextstyle(24, 0, _T("宋体"));
        int sub_title_x = (WINDOW_WIDTH - textwidth(_T("C++课程设计作品"))) / 2;
        outtextxy(sub_title_x, 230, _T("C++课程设计作品"));

        Button buttons[] = { btn_two_player, btn_ai_player };
        for (int i = 0; i < 2; i++) {
            Button btn = buttons[i];
            setfillcolor(btn.is_hover ? RGB(220, 200, 170) : RGB(245, 235, 210));
            setlinecolor(RGB(120, 50, 20));
            setlinestyle(PS_SOLID, 2);
            fillroundrect(btn.x, btn.y, btn.x + btn.width, btn.y + btn.height, 10, 10);

            settextcolor(RGB(80, 30, 10));
            settextstyle(28, 0, _T("楷体"));
            int text_x = btn.x + (btn.width - textwidth(btn.text)) / 2;
            int text_y = btn.y + (btn.height - textheight(btn.text)) / 2;
            outtextxy(text_x, text_y, btn.text);
        }

        settextstyle(20, 0, _T("宋体"));
        int tip_x = (WINDOW_WIDTH - textwidth(_T("点击按钮选择游戏模式"))) / 2;
        outtextxy(tip_x, 650, _T("点击按钮选择游戏模式"));

        FlushBatchDraw();
        if (_kbhit() && _getch() == 27) {
            EndBatchDraw();
            closegraph();
            exit(0);
        }
    }

    game_mode = selected_mode;
    EndBatchDraw();
    closegraph();
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    setbkcolor(RGB(240, 230, 200));
    BeginBatchDraw();
}

// --- 主函数 ---
int main() {
    select_game_mode();
    init_game();
    repaint_all();
    ExMessage msg;

    while (true) {
        if (game_over) {
            if (_kbhit() && _getch() == 27) break;
            continue;
        }

        if (fog_blade.is_flying) {
            update_fog_blade();
            Sleep(20);
            continue;
        }

        if (is_checked(turn)) {
            bool has_legal_move = false;
            for (int from_r = 0; from_r < ROW_NUM; from_r++) {
                for (int from_c = 0; from_c < COL_NUM; from_c++) {
                    if (board[from_r][from_c].color != turn) continue;
                    for (int to_r = 0; to_r < ROW_NUM; to_r++) {
                        for (int to_c = 0; to_c < COL_NUM; to_c++) {
                            if (is_move_valid(from_r, from_c, to_r, to_c)
                                && is_move_safe(from_r, from_c, to_r, to_c)) {
                                has_legal_move = true;
                                goto end_check;
                            }
                        }
                    }
                }
            }
        end_check:
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

        if (game_mode == MODE_AI && turn == CHESS_BLACK) {
            ai_move();
            continue;
        }

        if (peekmessage(&msg, EM_MOUSE)) {
            undo_btn.hover = is_in_undo_btn(msg.x, msg.y);
            btn_fog_blade.is_hover = (msg.x >= btn_fog_blade.x && msg.x <= btn_fog_blade.x + btn_fog_blade.w && msg.y >= btn_fog_blade.y && msg.y <= btn_fog_blade.y + btn_fog_blade.h);
            btn_invisible.is_hover = (msg.x >= btn_invisible.x && msg.x <= btn_invisible.x + btn_invisible.w && msg.y >= btn_invisible.y && msg.y <= btn_invisible.y + btn_invisible.h);

            if (msg.message == WM_LBUTTONDOWN) {
                if (invisible_mode) {
                    // 首先检查是否再次点击了“隐身”按钮 → 取消隐身
                    if (btn_invisible.is_hover) {  // 按钮 hover 状态已在前面更新
                        invisible_mode = false;
                        show_jack_form = false;
                        // 按钮仍然可用，因为选中棋子不变，技能可以重新激活
                        repaint_all();
                        continue;
                    }

                    // 否则处理隐身移动
                    int cr, cc;
                    if (click_to_board(msg.x, msg.y, cr, cc)) {
                        if (board[cr][cc].color == CHESS_EMPTY) {
                            execute_invisible_move(cr, cc);
                        }
                    }
                    continue;
                }

                if (is_in_undo_btn(msg.x, msg.y)) {
                    undo_move();
                    continue;
                }

                if (btn_fog_blade.is_active && btn_fog_blade.is_hover) {
                    // 弹出确认对话框
                    if (MessageBox(GetHWnd(), _T("是否要使用超模雾刃？"), _T("确认"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
                        activate_fog_blade();
                    }
                    // 无论确认与否，都跳过后续棋盘点击处理
                    continue;
                }

                if (btn_invisible.is_active && btn_invisible.is_hover) {
                    // 此时 invisible_mode 必然为 false（因为该变量为真时已在上面处理）
                    activate_invisible();
                    continue;
                }

                int click_r, click_c;
                if (click_to_board(msg.x, msg.y, click_r, click_c)) {
                    if (!is_selected) {
                        if (board[click_r][click_c].color == turn) {
                            selected_row = click_r;
                            selected_col = click_c;
                            is_selected = true;
                            skill_piece_r = click_r;
                            skill_piece_c = click_c;

                            btn_fog_blade.is_active = true;
                            Type t = board[click_r][click_c].type;
                            btn_invisible.is_active = (t == CHARIOT || t == SOLDIER);

                            repaint_all();
                        }
                    }
                    else {
                        if (board[click_r][click_c].color == turn) {
                            selected_row = click_r;
                            selected_col = click_c;
                            skill_piece_r = click_r;
                            skill_piece_c = click_c;

                            btn_fog_blade.is_active = true;
                            Type t = board[click_r][click_c].type;
                            btn_invisible.is_active = (t == CHARIOT || t == SOLDIER);
                            repaint_all();
                        }
                        else {
                            if (is_move_valid(selected_row, selected_col, click_r, click_c) && is_move_safe(selected_row, selected_col, click_r, click_c)) {
                                move_piece(selected_row, selected_col, click_r, click_c);
                                btn_fog_blade.is_active = false;
                                btn_invisible.is_active = false;
                                skill_piece_r = skill_piece_c = -1;
                            }
                            is_selected = false;
                            selected_row = -1;
                            selected_col = -1;
                            repaint_all();
                        }
                    }
                }
            }
        }
        if (_kbhit() && _getch() == 27) break;
    }

    EndBatchDraw();
    closegraph();
    return 0;
}