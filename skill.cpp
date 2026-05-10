#include "skill.h"
#include "ui_bank.h"   // repaint_all

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
            fog_blade.is_flying = false; show_jack_form = false;
            turn = (turn == CHESS_RED ? CHESS_BLACK : CHESS_RED);
            is_selected = false; selected_row = selected_col = -1;
            skill_piece_r = skill_piece_c = -1;
            btn_fog_blade.is_active = false;
            btn_invisible.is_active = false;
            repaint_all(); return;
        }
        if (board[fog_blade.current_r][fog_blade.current_c].color != CHESS_EMPTY) {
            fog_blade.is_flying = false; show_jack_form = false;
            StepRecord rec;
            rec.from_r = skill_piece_r; rec.from_c = skill_piece_c;
            rec.to_r = fog_blade.current_r; rec.to_c = fog_blade.current_c;
            rec.old_target = board[fog_blade.current_r][fog_blade.current_c];
            rec.old_turn = turn; rec.old_game_over = game_over;
            rec.last_fr = last_from_r; rec.last_fc = last_from_c;
            rec.last_tr = last_to_r; rec.last_tc = last_to_c;
            rec.has_last = has_last_step;
            rec.move_type = MOVE_FOG;
            rec.old_show_jack = show_jack_form; rec.old_invisible = false;
            rec.old_skill_r = skill_piece_r; rec.old_skill_c = skill_piece_c;
            rec.old_fog_active = btn_fog_blade.is_active;
            rec.old_invis_active = btn_invisible.is_active;
            move_history.push_back(rec);

            if (board[fog_blade.current_r][fog_blade.current_c].type == GENERAL) {
                game_over = true;
                Color winner = (turn == CHESS_RED ? CHESS_RED : CHESS_BLACK);
                _tcscpy_s(game_result, _countof(game_result),
                    winner == CHESS_RED ? _T("雾刃命中！红方胜利！") : _T("雾刃命中！黑方胜利！"));
            }
            else {
                board[fog_blade.current_r][fog_blade.current_c].color = CHESS_EMPTY;
                board[fog_blade.current_r][fog_blade.current_c].type = TYPE_NONE;
                board[fog_blade.current_r][fog_blade.current_c].show = false;
            }
            turn = (turn == CHESS_RED ? CHESS_BLACK : CHESS_RED);
            is_selected = false; selected_row = selected_col = -1;
            skill_piece_r = skill_piece_c = -1;
            btn_fog_blade.is_active = false; btn_invisible.is_active = false;
            repaint_all(); return;
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
    rec.from_r = skill_piece_r; rec.from_c = skill_piece_c;
    rec.to_r = to_r; rec.to_c = to_c;
    rec.old_target = board[to_r][to_c];
    rec.old_turn = turn; rec.old_game_over = game_over;
    rec.last_fr = last_from_r; rec.last_fc = last_from_c;
    rec.last_tr = last_to_r; rec.last_tc = last_to_c;
    rec.has_last = has_last_step;
    rec.move_type = MOVE_INVISIBLE;
    rec.old_show_jack = show_jack_form; rec.old_invisible = invisible_mode;
    rec.old_skill_r = skill_piece_r; rec.old_skill_c = skill_piece_c;
    rec.old_fog_active = btn_fog_blade.is_active;
    rec.old_invis_active = btn_invisible.is_active;
    move_history.push_back(rec);

    board[to_r][to_c] = board[skill_piece_r][skill_piece_c];
    board[skill_piece_r][skill_piece_c].color = CHESS_EMPTY;
    board[skill_piece_r][skill_piece_c].type = TYPE_NONE;
    board[skill_piece_r][skill_piece_c].show = false;

    invisible_mode = false; show_jack_form = false;
    turn = (turn == CHESS_RED ? CHESS_BLACK : CHESS_RED);
    is_selected = false; selected_row = -1; selected_col = -1;
    skill_piece_r = skill_piece_c = -1;
    btn_fog_blade.is_active = false; btn_invisible.is_active = false;
    repaint_all();
}