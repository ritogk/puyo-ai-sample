// 標準入出力
#include <stdio.h>
// 文字列操作
#include <string.h>
// 描画クリア用
#include <stdlib.h>
// コンソール入出力
#include <conio.h>

#define FIELD_WIDTH 8
#define FIELD_HEIGHT 14

// 組ぷよの初期位置
#define PUYO_START_X 3
#define PUYO_START_Y 1

#define PUYO_COLOR_4

enum
{
	CELL_NONE,
	CELL_WALL,
	CELL_PUYO_0,
	CELL_PUYO_1,
	CELL_PUYO_2,
	CELL_PUYO_3,
	CELL_MAX
};

// フィールドの状態(0で初期化されている)
int cells[FIELD_HEIGHT][FIELD_WIDTH];
int displayBuffer[FIELD_HEIGHT][FIELD_WIDTH];

char cellNames[][5] = {
		"ー", // CELL_NONE
		"■",	// CELL_WALL
		"◯",	// CELL_PUYO_0
		"△",	// CELL_PUYO_1
		"□",	// CELL_PUYO_2
		"☆",	// CELL_PUYO_3
};

// ぷよの位置
int puyoX = PUYO_START_X,
puyoY = PUYO_START_Y;
int puyoColor;

int main(void)
{
	// フィールドの壁を書き込む
	for (int y = 0; y < FIELD_HEIGHT; y++)
	{
		cells[y][0] = CELL_WALL;
		cells[y][FIELD_WIDTH - 1] = CELL_WALL;
	}
	for (int x = 0; x < FIELD_WIDTH; x++)
	{
		cells[FIELD_HEIGHT - 1][x] = CELL_WALL;
	}

	while (1)
	{
		// // コンソールウィンドウをクリア
		system("cls");
		// cellsのサイズ分だけバッファーにコピーする
		memcpy(displayBuffer, cells, sizeof cells);
		displayBuffer[puyoY][puyoX] = CELL_PUYO_0 + puyoColor;
		// 描画
		for (int y = 0; y < FIELD_HEIGHT; y++)
		{
			for (int x = 0; x < FIELD_WIDTH; x++)
			{
				printf("%s", cellNames[displayBuffer[y][x]]);
			}
			printf("\n");
		}

		int end = 0;
		switch (_getch())
		{
		case 'w':
			puyoY--;
			break;
		case 's':
			puyoY++;
			break;
		case 'a':
			puyoX--;
			break;
		case 'd':
			puyoX++;
			break;
		case 'z':
			end = 1;
		}

		if (end)
			break;
	}
}
