// 標準入出力
#include <stdio.h>
// 文字列操作
#include <string.h>
// 描画クリア用
#include <stdlib.h>
// コンソール入出力
#include <conio.h>
// 時間
#include <time.h>
#include <windows.h>

#define BOARD_WIDTH 12
#define BOARD_HEIGHT 13
#define PUYO_NEXT_START_X 10
#define PUYO_NEXT_START_Y 1
#define PUYO_NEXT_NEXT_START_X 10
#define PUYO_NEXT_NEXT_START_Y 4

#define FIELD_WIDTH 8
#define FIELD_HEIGHT 14

// 組ぷよの初期位置
#define PUYO_START_X 3
#define PUYO_START_Y 1

#define PUYO_COLOR_MAX 4

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

// 軸ぷよを基準に回転した小ぷよの座標
int puyoSubPotions[][2] = {
	{0, -1} , // PUYO_ANGLE_0,
	{ -1, 0}, // PUYO_ANGLE_90,
	{0, 1},  // PUYO_ANGLE_180,
	{1, 0},  // PUYO_ANGLE_270,
};

// フィールドの状態
int cells[FIELD_HEIGHT][FIELD_WIDTH];
// フィールドの状態(描画用)
int displayBuffer[FIELD_HEIGHT][FIELD_WIDTH];
// 連結チェックが済んだフィールド
int checked[FIELD_HEIGHT][FIELD_WIDTH];

// ボードの状態
int boards[BOARD_HEIGHT][BOARD_WIDTH];

// ぷよのアスキーアート
char cellNames[][5] = {
	"・",   // CELL_NONE
	"■",	// CELL_WALL
	"◯",	// CELL_PUYO_0
	"▲",	// CELL_PUYO_1
	"☆",	// CELL_PUYO_2
	"★",	// CELL_PUYO_3
};

int puyoX = PUYO_START_X;
int puyoY = PUYO_START_Y;
int puyoColor;
int puyoChildColor;
int puyoAngle;
bool moveLock = false;

int puyoNextColor;
int puyoNextChildColor;
int puyoNextNextColor;
int puyoNextNextChildColor;

// 描画
void display() {
	// コンソールをクリア
	system("cls");
	// cellsのサイズ分だけバッファーにコピーする
	memcpy(displayBuffer, cells, sizeof cells);
	
	if (!moveLock) {
		// 子ぷよを回転させる
		int subX = puyoX + puyoSubPotions[puyoAngle][0];
		int subY = puyoY + puyoSubPotions[puyoAngle][1];
		displayBuffer[puyoY][puyoX] = CELL_PUYO_0 + puyoColor;;
		displayBuffer[subY][subX] = CELL_PUYO_0 + puyoChildColor;
	}

	// ボードにフィールドのセル情報を入れ込む
	for (int y = 0; y < BOARD_HEIGHT; y++){
		for (int x = 0; x < BOARD_WIDTH; x++){
			if (y < FIELD_HEIGHT && x < FIELD_WIDTH) {
				boards[y][x] = displayBuffer[y+1][x];
			}
		}
	}
	
	// ネクストぷよ
	boards[PUYO_NEXT_START_Y - 1][PUYO_NEXT_START_X - 1] = CELL_PUYO_0 + puyoNextChildColor;
	boards[PUYO_NEXT_START_Y][PUYO_NEXT_START_X - 1] = CELL_PUYO_0 + puyoNextColor;

	// ネクネクぷよ
	boards[PUYO_NEXT_NEXT_START_Y][PUYO_NEXT_NEXT_START_X - 1] = CELL_PUYO_0 + puyoNextNextChildColor;
	boards[PUYO_NEXT_NEXT_START_Y + 1][PUYO_NEXT_NEXT_START_X - 1] = CELL_PUYO_0 + puyoNextNextColor;

	// ボードを描画
	for (int y = 0; y < BOARD_HEIGHT; y++) {
		for (int x = 0; x < BOARD_WIDTH; x++) {
			printf("%s", cellNames[boards[y][x]]);
		}
		printf("\n");
	}
}

// ぷよ移動後の当たり判定
bool intersectPuyoToField(int _puyoX, int _puyoY, int _puyoAngle) {
	// 軸ぷよの当たり判定
	if (cells[_puyoY][_puyoX] != CELL_NONE)
		return true;
	// 子ぷよの当たり判定
	int subX = _puyoX + puyoSubPotions[_puyoAngle][0];
	int subY = _puyoY + puyoSubPotions[_puyoAngle][1];
	if (cells[subY][subX] != CELL_NONE)
		return true;

	return false;
}


// ぷよの連結数を取得
int getPuyoConnectedCount(int _x, int _y, int _cell, int _count) {
	// チェック済 or 別の色かどうか
	if (checked[_y][_x]
		|| (cells[_y][_x] != _cell)) return _count;
	_count++;
	checked[_y][_x] = true;

	// 上下左右の座標を取得して連結数を取得する
	for (int i = 0; i < PUYO_ANGLE_MAX; i++) {
		int x = _x + puyoSubPotions[i][0];
		int y = _y + puyoSubPotions[i][1];
		_count = getPuyoConnectedCount(x, y, _cell, _count);
	}
		
	return _count;
}

// ぷよを消す
void erasePuyo(int _x, int _y, int _cell) {
	if (cells[_y][_x] != _cell)
		return;
	cells[_y][_x] = CELL_NONE;
	// 上下左右の座標を取得して消す
	for (int i = 0; i < PUYO_ANGLE_MAX; i++) {
		int x = _x + puyoSubPotions[i][0];
		int y = _y + puyoSubPotions[i][1];
		erasePuyo(x, y, _cell);
	}
}

// メイン
int main(void){
	// フィールﾄﾞに壁を書き込む
	for (int y = 0; y < FIELD_HEIGHT; y++){
		cells[y][0] = CELL_WALL;
		cells[y][FIELD_WIDTH - 1] = CELL_WALL;
	}
	for (int x = 0; x < FIELD_WIDTH; x++){
		cells[FIELD_HEIGHT - 1][x] = CELL_WALL;
	}
	
	srand((unsigned int)time(NULL));

	// 軸ぷよ、小ぷよ、ネクストぷよ、ネクネクぷよの色を取得
	puyoColor = rand() % PUYO_COLOR_MAX;
	puyoChildColor = rand() % PUYO_COLOR_MAX;
	puyoNextColor = rand() % PUYO_COLOR_MAX;
	puyoNextChildColor = rand() % PUYO_COLOR_MAX;
	puyoNextNextColor = rand() % PUYO_COLOR_MAX;
	puyoNextNextChildColor = rand() % PUYO_COLOR_MAX;

	clock_t t = clock();
	while (1){
		// 0.5秒毎に落下させる
		if (clock() - t >= 500) {
			t = clock();
			if (!moveLock) {
				// 当たり判定
				if (!intersectPuyoToField(puyoX, puyoY + 1, puyoAngle)) {
					puyoY++;
				} else {
					int subX = puyoX + puyoSubPotions[puyoAngle][0];
					int subY = puyoY + puyoSubPotions[puyoAngle][1];
					cells[puyoY][puyoX] = CELL_PUYO_0 + puyoColor;
					cells[subY][subX] = CELL_PUYO_0 + puyoChildColor;

					puyoX = PUYO_START_X;
					puyoY = PUYO_START_Y;
					puyoAngle = PUYO_ANGLE_0;
					puyoColor = puyoNextColor;
					puyoChildColor = puyoNextChildColor;
					puyoNextColor = puyoNextNextColor;
					puyoNextChildColor = puyoNextNextChildColor;
					puyoNextNextColor = rand() % PUYO_COLOR_MAX;
					puyoNextNextChildColor = rand() % PUYO_COLOR_MAX;
					

					// ぷよを落下させるのでロックする
					moveLock = true;
				}
			}

			if (moveLock) {
				moveLock = false;
				// 空中に浮いているぷよを落下させる
				for (int y = FIELD_HEIGHT - 3; y >= 0; y--) {
					for (int x = 1; x < FIELD_WIDTH - 1; x++) {
						if ((cells[y][x] != CELL_NONE) && (cells[y + 1][x] == CELL_NONE)) {
							cells[y + 1][x] = cells[y][x];
							cells[y][x] = CELL_NONE;
							moveLock = true;
						}
					}
				}

				if (!moveLock) {
					// 連結しているぷよを消す
					memset(checked, 0, sizeof checked);
					for (int y = 0; y < FIELD_HEIGHT-1; y++) {
						for (int x = 1; x < FIELD_WIDTH - 1; x++) {
							if (cells[y][x] == CELL_NONE)
								continue;
							if (getPuyoConnectedCount(x, y, cells[y][x], 0) >= 4) {
								erasePuyo(x, y, cells[y][x]);
								moveLock = true;
							}
						}
					}
				}

			}
			display();
		}
		
		// キーボードの入力があるか
		if (_kbhit()) {
			if (!moveLock) {
				int x = puyoX;
				int y = puyoY;
				int angle = puyoAngle;
				switch (_getch()) {
				case 0x48:
					// ↑
					y--;
					break;
				case 0x50:
					// ↓
					y++;
					break;
				case 0x4B:
					// ←
					x--;
					break;
				case 0x4D:
					// →
					x++;
					break;
				case 'z':
					angle = (++angle) % PUYO_ANGLE_MAX;
					break;
				case 'x':
					angle = angle == PUYO_ANGLE_0 ? PUYO_ANGLE_270 : angle - 1;
				}
				// 当たり判定
				if (!intersectPuyoToField(x, y, angle)) {
					puyoX = x;
					puyoY = y;
					puyoAngle = angle;
				}
				display();
			}
		}

	}
}
