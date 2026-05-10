// globals.cpp —— 定义所有全局变量
#include "common.h"

ChessPiece board[ROW_NUM][COL_NUM];
int selected_row = -1, selected_col = -1;
bool is_selected = false;
Color turn = CHESS_RED;
GameMode game_mode = MODE_NONE;
bool game_over = false;
TCHAR game_result[64];
int last_from_r = -1, last_from_c = -1, last_to_r = -1, last_to_c = -1;
bool has_last_step = false;
UndoButton undo_btn = { 587, 254, 75, 75, _T(""), false };
std::vector<StepRecord> move_history;
TCHAR g_exeDir[MAX_PATH] = { 0 };
SkillButton btn_fog_blade, btn_invisible;
FogBladeState fog_blade = { false, -1, -1, 0, 0 };
bool invisible_mode = false;
int skill_piece_r = -1, skill_piece_c = -1;
bool show_jack_form = false;

IMAGE img_fog_active, img_fog_disable, img_invis_active, img_invis_disable;
IMAGE img_jack_fog, img_jack_invis, img_fog_slash;
IMAGE img_hover_mask, img_undo, img_board_bg;
IMAGE img_menu_bg;
IMAGE img_btn_2p, img_btn_2p_hover;
IMAGE img_btn_ai, img_btn_ai_hover;
bool img_load_success = false;