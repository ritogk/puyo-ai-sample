// 標準入出力
#include <stdio.h>
// 文字列操作
#include <string.h>
// 描画クリア用
#include <stdlib.h>
// コンソール入出力
#include <conio.h>
// 配列
#include <array>

#include <cmath>
using std::pow;

#include <algorithm>

#include <ctime>

#include <vector>
using std::vector;


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


#define NODE_MAX 50
#define MOVE_MAX 22
// 1列目~6列目で置ける全てのパターン
int moves[6][4][2] = {
	// 1列目
	{
		{0, -1} , // PUYO_ANGLE_0,
		{0, 1},  // PUYO_ANGLE_180,
		{1, 0},  // PUYO_ANGLE_270,
		{-9, -9}
	},
	// 2列目
	{
		{0, -1} , // PUYO_ANGLE_0,
		{ -1, 0}, // PUYO_ANGLE_90,
		{0, 1},  // PUYO_ANGLE_180,
		{1, 0},  // PUYO_ANGLE_270,
	},
	// 3列目
	{
		{0, -1} , // PUYO_ANGLE_0,
		{ -1, 0}, // PUYO_ANGLE_90,
		{0, 1},  // PUYO_ANGLE_180,
		{1, 0},  // PUYO_ANGLE_270,
	},
	// 4列目
	{
		{0, -1} , // PUYO_ANGLE_0,
		{ -1, 0}, // PUYO_ANGLE_90,
		{0, 1},  // PUYO_ANGLE_180,
		{1, 0},  // PUYO_ANGLE_270,
	},
	// 5列目
	{
		{0, -1} , // PUYO_ANGLE_0,
		{ -1, 0}, // PUYO_ANGLE_90,
		{0, 1},  // PUYO_ANGLE_180,
		{1, 0},  // PUYO_ANGLE_270,
	},
	// 6列目
	{
		{0, -1} , // PUYO_ANGLE_0,
		{ -1, 0}, // PUYO_ANGLE_90,
		{0, 1},  // PUYO_ANGLE_180,
		{-9, -9}
	}
};
struct Node
{
	int firstInput;
	int rate;
	int cells[FIELD_HEIGHT][FIELD_WIDTH];
	int chain;
};
//Node nodes[NODE_MAX];
vector<Node> nodes;
int nodeCnt = 0;
vector<Node> nodesBuffer;
//Node nodesBuffer[NODE_MAX * MOVE_MAX];
int nodesBufferCnt = 0;

// 描画
void display() {
	// コンソールをクリア
	system("cls");
	// cellsのサイズ分だけバッファーにコピーする
	memcpy(displayBuffer, cells, sizeof cells);

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
int getPuyoConnectedCount(int _x, int _y, int _cell, int _count, int _cells[FIELD_HEIGHT][FIELD_WIDTH]) {
	// チェック済 or 別の色かどうか
	if (checked[_y][_x]
		|| (_cells[_y][_x] != _cell)) return _count;
	_count++;
	checked[_y][_x] = true;

	// 上下左右の座標を取得して連結数を取得する
	for (int i = 0; i < PUYO_ANGLE_MAX; i++) {
		int x = _x + puyoSubPotions[i][0];
		int y = _y + puyoSubPotions[i][1];
		_count = getPuyoConnectedCount(x, y, _cell, _count, _cells);
	}
		
	return _count;
}

// ぷよを消す
void erasePuyo(int _x, int _y, int _cell, int _cells[FIELD_HEIGHT][FIELD_WIDTH]) {
	if (_cells[_y][_x] != _cell)
		return;
	_cells[_y][_x] = CELL_NONE;
	// 上下左右の座標を取得して消す
	for (int i = 0; i < PUYO_ANGLE_MAX; i++) {
		int x = _x + puyoSubPotions[i][0];
		int y = _y + puyoSubPotions[i][1];
		erasePuyo(x, y, _cell, _cells);
	}
}

// ぷよを落下させる
void fall(int _cells[FIELD_HEIGHT][FIELD_WIDTH]) {
	for (int a = 0; a < FIELD_HEIGHT - 1; a++) {
		for (int y = FIELD_HEIGHT - 3; y >= 0; y--) {
			for (int x = 1; x < FIELD_WIDTH - 1; x++) {
				if ((_cells[y][x] != CELL_NONE) && (_cells[y + 1][x] == CELL_NONE)) {
					_cells[y + 1][x] = _cells[y][x];
					_cells[y][x] = CELL_NONE;
				}
			}
		}
	}
}

// ぷよをフィールドの置ける場所に全通り置く
void movePuyo(int puyo, int puyoChild) {
	int _cells[FIELD_HEIGHT][FIELD_WIDTH];

	// 落下処理の前に置けるかどうかの判別を行う。①14段目には置けない。②連鎖しない

	for (int i = 0; i < sizeof(moves) / sizeof(moves[0]); i++) {
		for (int j = 0; j < sizeof(moves[0]) / sizeof(moves[0][0]); j++) {
			if (moves[i][j][0] == -9 && moves[i][j][1] == -9)
				continue;
			// フィールドの内容をコピー
			memcpy(_cells, cells, sizeof cells);
			// ぷよの座標
			int mainX = i + 1;
			int mainY = PUYO_START_Y;
			int subX = mainX + moves[i][j][0];
			int subY = mainY + moves[i][j][1];
			_cells[mainY][mainX] = CELL_PUYO_0 + puyoColor;
			_cells[subY][subX] = CELL_PUYO_0 + puyoChildColor;
			// 空中に浮いているぷよを落下させる
			fall(_cells);
			Node node;
			node.firstInput = 1;
			node.rate = 0;
			node.chain = 0;
			memcpy(node.cells, _cells, sizeof _cells);
			nodes[nodeCnt] = node;
			nodeCnt++;
		}
	}
	
	// ビームサーチ
	int a = 1;
}

// ビームサーチ
void beamSearch() {
	// 評価
	// ソート
	// 切り捨て
}

// 連鎖があるかどうか
bool isChain(int _x, int _y, int _cell, int _cells[FIELD_HEIGHT][FIELD_WIDTH]){
	memset(checked, 0, sizeof checked);
	bool exists = false;
	for (int y = 0; y < FIELD_HEIGHT - 1; y++) {
		for (int x = 1; x < FIELD_WIDTH - 1; x++) {
			if (_cells[y][x] == CELL_NONE)
				continue;
			if (getPuyoConnectedCount(x, y, _cells[y][x], 0, _cells) >= 4) {
				exists = true;
			}
		}
	}
	return exists;
}

// 連鎖数を取得
int getChain(int _cells[FIELD_HEIGHT][FIELD_WIDTH]) {
	int __cells[FIELD_HEIGHT][FIELD_WIDTH];
	for (int i = 0; i < FIELD_HEIGHT; i++) {
		for (int j = 0; j < FIELD_WIDTH; j++) {
			__cells[i][j] = _cells[i][j];
		}
	}

	int chain = 0;
	for (int y = 0; y < FIELD_HEIGHT - 1; y++) {
		for (int x = 1; x < FIELD_WIDTH - 1; x++) {
			if (__cells[y][x] == CELL_NONE)
				continue;
			if (getPuyoConnectedCount(x, y, __cells[y][x], 0, __cells) >= 4) {
				erasePuyo(x, y, __cells[y][x], __cells);
				fall(__cells);
				chain++;
				memset(checked, 0, sizeof checked);
			}
		}
	}
	return chain;
}

void tumoAllMove() {
	int _cells[FIELD_HEIGHT][FIELD_WIDTH];
	for (int nCnt = 0; nCnt < nodes.size(); nCnt++) {
		for (int i = 0; i < sizeof(moves) / sizeof(moves[0]); i++) {
			for (int j = 0; j < sizeof(moves[0]) / sizeof(moves[0][0]); j++) {
				if (moves[i][j][0] == -9 && moves[i][j][1] == -9)
					continue;
				
				// フィールドの内容をコピー	
				memcpy(_cells, nodes[nCnt].cells, sizeof nodes[nCnt].cells);
				// ぷよの座標
				int mainX = i + 1;
				int mainY = PUYO_START_Y;
				int subX = mainX + moves[i][j][0];
				int subY = mainY + moves[i][j][1];

				_cells[mainY][mainX] = CELL_PUYO_0 + puyoColor;
				_cells[subY][subX] = CELL_PUYO_0 + puyoChildColor;
				// 空中に浮いているぷよを落下させる
				fall(_cells);

				// 13段目以降にぷよを置いていない事
				if (_cells[1][1] != CELL_NONE
					||  _cells[1][2] != CELL_NONE
					|| _cells[1][3] != CELL_NONE
					|| _cells[1][4] != CELL_NONE
					|| _cells[1][5] != CELL_NONE
					||  _cells[1][6] != CELL_NONE) continue;

				// 連鎖していない事
				if (isChain(mainX, mainY, _cells[mainY][mainX], _cells)) {
					continue;
				}

				Node n;
				n.firstInput = 1;
				n.rate = 0;
				n.chain = 0;
				memcpy(n.cells, _cells, sizeof _cells);
				nodesBuffer.push_back(n);

			}
		}
	}
}

// 評価
void evaluation() {
	// 連結が多い
	for (int nCnt = 0; nCnt < nodesBuffer.size(); nCnt++) {
		int connectedRate = 0;
		memset(checked, 0, sizeof checked);
		for (int y = 0; y < FIELD_HEIGHT - 1; y++) {
			for (int x = 1; x < FIELD_WIDTH - 1; x++) {
				if (nodesBuffer[nCnt].cells[y][x] == CELL_NONE)
					continue;
				connectedRate += pow(getPuyoConnectedCount(x, y, nodesBuffer[nCnt].cells[y][x], 0, nodesBuffer[nCnt].cells), 2);
			}
		}
		nodesBuffer[nCnt].rate += connectedRate;
	}
	// 連鎖が多い
	for (int nCnt = 0; nCnt < nodesBuffer.size(); nCnt++) {
		int maxChain = 0;
		int connectedRate = 0;
		for (int x = 1; x < FIELD_WIDTH - 1; x++) {
			int __cells[FIELD_HEIGHT][FIELD_WIDTH];
			for (int i = 0; i < PUYO_COLOR_MAX - 1; i++) {
				for (int i = 0; i < FIELD_HEIGHT; i++) {
					for (int j = 0; j < FIELD_WIDTH; j++) {
						__cells[i][j] = nodesBuffer[nCnt].cells[i][j];
					}
				}
				__cells[0][x] = CELL_PUYO_0 + i;
				fall(__cells);

				// getChainの計算式がおかしい・・・
				// 2色同時消し = 2連鎖ってカウントしてる。
				memset(checked, 0, sizeof checked);
				int chain = getChain(__cells);
				if (maxChain < chain) {
					maxChain = chain;
					if (maxChain >= 9) {
						int a = 1;
						memcpy(cells, __cells, sizeof  __cells);
						display();
						printf("chain:%d", nodes[0].chain);
					}
				}
 			}
		}
		
		nodesBuffer[nCnt].rate += pow(maxChain, 2) * 10;
		nodesBuffer[nCnt].chain = maxChain;
	}
}

static bool compareCpusByProperty1(Node& a, Node& b) {
	return a.rate > b.rate;
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

	//std::vector<int> va = {1,2};
	vector<int> numbers = { 10, 20, 50, 100 };
	numbers.resize(1);


	srand((unsigned int)time(NULL));

	// 軸ぷよ、小ぷよ、ネクストぷよ、ネクネクぷよの色を取得
	puyoColor = rand() % PUYO_COLOR_MAX;
	puyoChildColor = rand() % PUYO_COLOR_MAX;
	puyoNextColor = rand() % PUYO_COLOR_MAX;
	puyoNextChildColor = rand() % PUYO_COLOR_MAX;
	puyoNextNextColor = rand() % PUYO_COLOR_MAX;
	puyoNextNextChildColor = rand() % PUYO_COLOR_MAX;

	Node node;
	memcpy(node.cells, cells, sizeof cells);
	node.firstInput = 1;
	node.rate = 0;
	nodes.push_back(node);

	for (int i = 0; i < 25; i++){
		// 指定階層より下のツモを全部置く
		tumoAllMove();
		// 評価
		evaluation();
		// ソート
		sort(nodesBuffer.begin(), nodesBuffer.end(), compareCpusByProperty1);
		// 切り捨て(50)
		if (nodesBuffer.size() > 50) {
			nodesBuffer.resize(50);
		}
		// nodesにnodesBuffterを入れ込む
		nodes.resize(nodesBuffer.size());
		for (size_t i = 0; i < nodesBuffer.size(); i++) {
			nodes[i] = nodesBuffer[i];
		}
		nodesBuffer.resize(0);

		puyoColor = puyoNextColor;
		puyoChildColor = puyoNextChildColor;
		puyoNextColor = puyoNextNextColor;
		puyoNextChildColor = puyoNextNextChildColor;
		puyoNextNextColor = rand() % PUYO_COLOR_MAX;
		puyoNextNextChildColor = rand() % PUYO_COLOR_MAX;

		int a = 1;
	}
	


	
	
	// // 画面出力
	// display();
	memcpy(cells, nodes[0].cells, sizeof  nodes[0].cells);
	display();
	printf("chain:%d", nodes[0].chain);
}
