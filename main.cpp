#include <graphics.h>
#include <conio.h>

// 棋盘全局参数定义，方便后续修改和调试
#define GRID_SIZE 60      // 每个格子的大小
#define LEFT_MARGIN 80    // 棋盘左边距
#define TOP_MARGIN 80     // 棋盘上边距
#define ROW_NUM 10        // 棋盘行数（横线数）
#define COL_NUM 9         // 棋盘列数（竖线数）

// 把棋盘的行列坐标，转换成窗口的像素坐标，后续棋子绘制、鼠标点击都会用到
// 行row：0-9，列col：0-8
POINT get_pos(int row, int col)
{
    POINT p;
    p.x = LEFT_MARGIN + col * GRID_SIZE;
    p.y = TOP_MARGIN + row * GRID_SIZE;
    return p;
}

// 绘制棋盘的函数，封装起来，后续刷新界面直接调用
void draw_chessboard()
{
    // 1. 绘制横线
    for (int i = 0; i < ROW_NUM; i++)
    {
        POINT p_start = get_pos(i, 0);
        POINT p_end = get_pos(i, COL_NUM - 1);
        line(p_start.x, p_start.y, p_end.x, p_end.y);
    }

    // 2. 绘制竖线（注意：中间楚河汉界处，竖线是断开的）
    for (int i = 0; i < COL_NUM; i++)
    {
        // 上半部分竖线（0-4行）
        POINT p_start = get_pos(0, i);
        POINT p_mid = get_pos(4, i);
        line(p_start.x, p_start.y, p_mid.x, p_mid.y);

        // 下半部分竖线（5-9行）
        p_mid = get_pos(5, i);
        POINT p_end = get_pos(ROW_NUM - 1, i);
        line(p_mid.x, p_mid.y, p_end.x, p_end.y);
    }

    // 3. 绘制楚河汉界
    settextcolor(BLACK);
    setbkmode(TRANSPARENT);
    settextstyle(30, 0, _T("楷体")); // 设置字体
    outtextxy(LEFT_MARGIN + GRID_SIZE * 2, TOP_MARGIN + GRID_SIZE * 4 + 15, _T("楚 河"));
    outtextxy(LEFT_MARGIN + GRID_SIZE * 5, TOP_MARGIN + GRID_SIZE * 4 + 15, _T("汉 界"));

    // 4. 绘制九宫格斜线（双方的九宫）
    // 上方九宫（红方）
    line(get_pos(0, 3).x, get_pos(0, 3).y, get_pos(2, 5).x, get_pos(2, 5).y);
    line(get_pos(0, 5).x, get_pos(0, 5).y, get_pos(2, 3).x, get_pos(2, 3).y);
    // 下方九宫（黑方）
    line(get_pos(7, 3).x, get_pos(7, 3).y, get_pos(9, 5).x, get_pos(9, 5).y);
    line(get_pos(7, 5).x, get_pos(7, 5).y, get_pos(9, 3).x, get_pos(9, 3).y);
}

int main()
{
    initgraph(720, 800);
    setbkcolor(RGB(240, 230, 200));
    cleardevice();

    setlinecolor(BLACK); // 设置棋盘线条颜色为黑色
    setlinestyle(PS_SOLID, 2); // 设置线条宽度为2，更清晰
    draw_chessboard(); // 调用函数绘制棋盘

    outtextxy(100, 20, _T("中国象棋程序 v0.2 - 棋盘绘制完成"));

    _getch();
    closegraph();
    return 0;
}