#include "ui_bank.h"

void draw_chessboard() {
    setlinecolor(WHITE); setlinestyle(PS_SOLID, 2);
    for (int i = 0; i < ROW_NUM; i++) {
        POINT s = get_pos(i, 0), e = get_pos(i, COL_NUM - 1);
        line(s.x, s.y, e.x, e.y);
    }
    for (int i = 0; i < COL_NUM; i++) {
        POINT s = get_pos(0, i), m = get_pos(4, i); line(s.x, s.y, m.x, m.y);
        m = get_pos(5, i); POINT e = get_pos(ROW_NUM - 1, i); line(m.x, m.y, e.x, e.y);
    }
    settextcolor(WHITE); setbkmode(TRANSPARENT);
    settextstyle(30, 0, _T("楷体"));
    outtextxy(LEFT_MARGIN + GRID_SIZE * 2, TOP_MARGIN + GRID_SIZE * 4 + 15, _T("楚 河"));
    outtextxy(LEFT_MARGIN + GRID_SIZE * 5, TOP_MARGIN + GRID_SIZE * 4 + 15, _T("汉 界"));
    line(get_pos(0, 3).x, get_pos(0, 3).y, get_pos(2, 5).x, get_pos(2, 5).y);
    line(get_pos(0, 5).x, get_pos(0, 5).y, get_pos(2, 3).x, get_pos(2, 3).y);
    line(get_pos(7, 3).x, get_pos(7, 3).y, get_pos(9, 5).x, get_pos(9, 5).y);
    line(get_pos(7, 5).x, get_pos(7, 5).y, get_pos(9, 3).x, get_pos(9, 3).y);
}

void draw_last_step() {
    if (!has_last_step) return;
    COLORREF old = getcolor();
    POINT fp = get_pos(last_from_r, last_from_c), tp = get_pos(last_to_r, last_to_c);
    setcolor(RGB(0, 150, 255));
    rectangle(fp.x - GRID_SIZE / 2 + 1, fp.y - GRID_SIZE / 2 + 1, fp.x + GRID_SIZE / 2 - 1, fp.y + GRID_SIZE / 2 - 1);
    setcolor(WHITE);
    rectangle(fp.x - GRID_SIZE / 2 + 3, fp.y - GRID_SIZE / 2 + 3, fp.x + GRID_SIZE / 2 - 3, fp.y + GRID_SIZE / 2 - 3);
    setcolor(RGB(255, 100, 100));
    rectangle(tp.x - GRID_SIZE / 2 + 1, tp.y - GRID_SIZE / 2 + 1, tp.x + GRID_SIZE / 2 - 1, tp.y + GRID_SIZE / 2 - 1);
    setcolor(WHITE);
    rectangle(tp.x - GRID_SIZE / 2 + 3, tp.y - GRID_SIZE / 2 + 3, tp.x + GRID_SIZE / 2 - 3, tp.y + GRID_SIZE / 2 - 3);
    setcolor(old);
}

void draw_piece(int row, int col) {
    ChessPiece p = board[row][col];
    if (p.color == CHESS_EMPTY || !p.show) return;
    POINT pos = get_pos(row, col);
    int r = GRID_SIZE / 2 - 10;

    if (show_jack_form && row == skill_piece_r && col == skill_piece_c) {
        IMAGE* use = invisible_mode ? &img_jack_invis : &img_jack_fog;
        if (img_load_success) {
            putimage_alpha(pos.x - 25, pos.y - 25, use);
        }
        else {
            setfillcolor(RGB(128, 0, 128)); setlinecolor(BLACK);
            fillcircle(pos.x, pos.y, r);
            settextcolor(WHITE); setbkmode(TRANSPARENT); settextstyle(20, 0, _T("黑体"));
            outtextxy(pos.x - 10, pos.y - 10, _T("J"));
        }
        return;
    }
    if (invisible_mode && row == skill_piece_r && col == skill_piece_c) {
        if (img_load_success) {
            putimage_alpha(pos.x - 25, pos.y - 25, &img_jack_invis);
        }
        else {
            setfillcolor(RGB(200, 200, 200)); setlinecolor(RGB(150, 150, 150)); setlinestyle(PS_DASH, 2);
            fillcircle(pos.x, pos.y, r);
        }
        return;
    }

    // 普通棋子绘制（用图片）
    IMAGE* use_piece = NULL;
    if (p.color == CHESS_RED) {
        switch (p.type) {
        case GENERAL:  use_piece = &img_piece_red_general; break;
        case ADVISOR:  use_piece = &img_piece_red_advisor; break;
        case ELEPHANT: use_piece = &img_piece_red_elephant; break;
        case HORSE:    use_piece = &img_piece_red_horse; break;
        case CHARIOT:  use_piece = &img_piece_red_chariot; break;
        case CANNON:   use_piece = &img_piece_red_cannon; break;
        case SOLDIER:  use_piece = &img_piece_red_soldier; break;
        }
    }
    else {
        switch (p.type) {
        case GENERAL:  use_piece = &img_piece_black_general; break;
        case ADVISOR:  use_piece = &img_piece_black_advisor; break;
        case ELEPHANT: use_piece = &img_piece_black_elephant; break;
        case HORSE:    use_piece = &img_piece_black_horse; break;
        case CHARIOT:  use_piece = &img_piece_black_chariot; break;
        case CANNON:   use_piece = &img_piece_black_cannon; break;
        case SOLDIER:  use_piece = &img_piece_black_soldier; break;
        }
    }

    if (use_piece && use_piece->getwidth() > 0) {
        // 动态居中绘制
        int pw = use_piece->getwidth();
        int ph = use_piece->getheight();
        putimage_alpha(pos.x - pw / 2, pos.y - ph / 2, use_piece);
    }
    else {
        // 兜底：图片未加载时，仍用原来的文字棋子
        setfillcolor(RGB(255, 250, 200)); setlinecolor(BLACK); setlinestyle(PS_SOLID, 2);
        fillcircle(pos.x, pos.y, r);
        TCHAR text[4] = { 0 };
        if (p.color == CHESS_RED) {
            switch (p.type) {
            case GENERAL: _tcscpy_s(text, _T("帥")); break;
            case ADVISOR: _tcscpy_s(text, _T("仕")); break;
            case ELEPHANT:_tcscpy_s(text, _T("相")); break;
            case HORSE:   _tcscpy_s(text, _T("馬")); break;
            case CHARIOT: _tcscpy_s(text, _T("車")); break;
            case CANNON:  _tcscpy_s(text, _T("炮")); break;
            case SOLDIER: _tcscpy_s(text, _T("兵")); break;
            }
            settextcolor(RGB(200, 0, 0));
        }
        else {
            switch (p.type) {
            case GENERAL: _tcscpy_s(text, _T("將")); break;
            case ADVISOR: _tcscpy_s(text, _T("士")); break;
            case ELEPHANT:_tcscpy_s(text, _T("象")); break;
            case HORSE:   _tcscpy_s(text, _T("馬")); break;
            case CHARIOT: _tcscpy_s(text, _T("車")); break;
            case CANNON:  _tcscpy_s(text, _T("砲")); break;
            case SOLDIER: _tcscpy_s(text, _T("卒")); break;
            }
            settextcolor(RGB(0, 0, 0));
        }
        setbkmode(TRANSPARENT); settextstyle(30, 0, _T("楷体"));
        outtextxy(pos.x - textwidth(text) / 2, pos.y - textheight(text) / 2, text);
    }
}

//圆形碰撞检测
bool is_in_circle_button(int mx, int my, const SkillButton& btn)
{
    int cx = btn.x + btn.w / 2;
    int cy = btn.y + btn.h / 2;
    int r = btn.w / 2;

    int dx = mx - cx;
    int dy = my - cy;

    return dx * dx + dy * dy <= r * r;
}

void repaint_all() {
    cleardevice();
    if (img_board_bg.getwidth() > 0) putimage(0, 0, &img_board_bg);
    draw_last_step();
    for (int r = 0; r < ROW_NUM; r++) for (int c = 0; c < COL_NUM; c++) draw_piece(r, c);

    // 绘制选中棋子高亮圈（放在最上层，不被棋子图片遮盖）
    if (is_selected && selected_row >= 0 && selected_col >= 0) {
        POINT pos = get_pos(selected_row, selected_col);
        int radius = GRID_SIZE / 2 - 10;   // 与你棋子半径保持一致
        setlinecolor(YELLOW);
        setlinestyle(PS_SOLID, 3);
        circle(pos.x, pos.y, radius + 3);
    }

    settextcolor(BLUE); setbkmode(TRANSPARENT); settextstyle(20, 0, _T("宋体"));
    if (game_over) {
        outtextxy(100, 20, game_result); outtextxy(100, 50, _T("按ESC键退出程序"));
    }
    else {
        if (game_mode == MODE_TWO_PLAYER) outtextxy(100, 20, _T("模式：双人对战"));
        else outtextxy(100, 20, _T("模式：人机对战（你是红方）"));
        if (turn == CHESS_RED) {
            outtextxy(100, 50, _T("当前回合：红方"));
            if (is_checked(CHESS_RED)) { settextcolor(RED); outtextxy(100, 80, _T("警告！红方被将军！")); }
        }
        else {
            outtextxy(100, 50, _T("当前回合：黑方"));
            if (is_checked(CHESS_BLACK)) { settextcolor(RED); outtextxy(100, 80, _T("警告！黑方被将军！")); }
        }
    }

    // 悔棋按钮
    if (img_undo.getwidth() > 0) {
        putimage_alpha(undo_btn.x, undo_btn.y, &img_undo);
        if (undo_btn.hover) putimage_alpha(undo_btn.x, undo_btn.y, &img_hover_mask);
    }
    else {
        setfillcolor(undo_btn.hover ? RGB(255, 200, 200) : RGB(255, 230, 230));
        setcolor(BLACK); fillrectangle(undo_btn.x, undo_btn.y, undo_btn.x + undo_btn.w, undo_btn.y + undo_btn.h);
        settextcolor(BLACK); settextstyle(24, 0, _T("黑体"));
        int tx = undo_btn.x + (undo_btn.w - textwidth(undo_btn.text)) / 2;
        int ty = undo_btn.y + (undo_btn.h - textheight(undo_btn.text)) / 2;
        outtextxy(tx, ty, undo_btn.text);
    }

    // 雾刃按钮
    if (img_load_success) {
        IMAGE* use = btn_fog_blade.is_active ? &img_fog_active : &img_fog_disable;
        putimage_alpha(btn_fog_blade.x, btn_fog_blade.y, use);
        if (btn_fog_blade.is_hover && btn_fog_blade.is_active)
            putimage_alpha(btn_fog_blade.x, btn_fog_blade.y, &img_hover_mask);
    }
    else {
        COLORREF fb = btn_fog_blade.is_active ? RGB(100, 150, 255) : RGB(150, 150, 150);
        COLORREF fh = btn_fog_blade.is_active ? RGB(150, 180, 255) : RGB(180, 180, 180);
        setfillcolor(btn_fog_blade.is_hover ? fh : fb); setcolor(BLACK);
        fillrectangle(btn_fog_blade.x, btn_fog_blade.y, btn_fog_blade.x + btn_fog_blade.w, btn_fog_blade.y + btn_fog_blade.h);
        settextcolor(btn_fog_blade.is_active ? WHITE : RGB(100, 100, 100));
        settextstyle(20, 0, _T("黑体"));
        int ftx = btn_fog_blade.x + (btn_fog_blade.w - textwidth(btn_fog_blade.text)) / 2;
        int fty = btn_fog_blade.y + (btn_fog_blade.h - textheight(btn_fog_blade.text)) / 2;
        outtextxy(ftx, fty, btn_fog_blade.text);
    }
    // 隐身按钮
    if (img_load_success) {
        IMAGE* use = btn_invisible.is_active ? &img_invis_active : &img_invis_disable;
        putimage_alpha(btn_invisible.x, btn_invisible.y, use);
        if (btn_invisible.is_hover && btn_invisible.is_active)
            putimage_alpha(btn_invisible.x, btn_invisible.y, &img_hover_mask);
    }
    else {
        COLORREF inv = btn_invisible.is_active ? RGB(200, 150, 255) : RGB(150, 150, 150);
        COLORREF invh = btn_invisible.is_active ? RGB(220, 180, 255) : RGB(180, 180, 180);
        setfillcolor(btn_invisible.is_hover ? invh : inv);
        fillrectangle(btn_invisible.x, btn_invisible.y, btn_invisible.x + btn_invisible.w, btn_invisible.y + btn_invisible.h);
        settextcolor(btn_invisible.is_active ? WHITE : RGB(100, 100, 100));
        int itx = btn_invisible.x + (btn_invisible.w - textwidth(btn_invisible.text)) / 2;
        int ity = btn_invisible.y + (btn_invisible.h - textheight(btn_invisible.text)) / 2;
        outtextxy(itx, ity, btn_invisible.text);
    }

    // 雾刃动画
    if (fog_blade.is_flying && img_fog_slash.getwidth() > 0) {
        POINT pos = get_pos(fog_blade.current_r, fog_blade.current_c);
        int dx = pos.x - img_fog_slash.getwidth() / 2;
        int dy = pos.y - img_fog_slash.getheight() / 2;
        putimage_alpha(dx, dy, &img_fog_slash);
    }
    FlushBatchDraw();
}