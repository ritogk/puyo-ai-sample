#pragma once

#define SCREEN_WIDTH 12
#define SCREEN_HEIGHT 13
#define PUYO_NEXT_START_X 10
#define PUYO_NEXT_START_Y 1
#define PUYO_NEXT_NEXT_START_X 10
#define PUYO_NEXT_NEXT_START_Y 4
// 
#define FIELD_WIDTH 8
#define FIELD_HEIGHT 14

// 組ぷよの初期位置
#define PUYO_START_X 3
#define PUYO_START_Y 1

#define PUYO_COLOR_MAX 4

// 連鎖を組む際のツモ数
#define CHAIN_TUMO 30

// ビームサーチのビーム幅
#define BEAM_WIDTH 50

// 配列の要素数を取得する
#define SIZE_OF_ARRAY(array) (sizeof(array)/sizeof(array[0]))

// フィールドのオブジェクト
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

// ぷよ角度(左回り)
enum {
	PUYO_ANGLE_0,
	PUYO_ANGLE_90,
	PUYO_ANGLE_180,
	PUYO_ANGLE_270,
	PUYO_ANGLE_MAX,
};

struct Node
{
	int firstInput;
	int rate;
	int cells[FIELD_HEIGHT][FIELD_WIDTH];
	int chain;
};

