#include <array>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <vector>
#include <string>   
using std::pow;
using std::vector;
using std::string;

#include "./ai.h"

// 軸ぷよを基準に回転した小ぷよの座標
int puyoSubPotions[][2] = {
	{0, -1} , // PUYO_ANGLE_0,
	{ -1, 0}, // PUYO_ANGLE_90,
	{0, 1},   // PUYO_ANGLE_180,
	{1, 0},   // PUYO_ANGLE_270,
};
// フィールドの状態
int cells[FIELD_HEIGHT][FIELD_WIDTH];
// 連結チェックが済んだフィールド
int checked[FIELD_HEIGHT][FIELD_WIDTH];
// 画面の状態
int screen[SCREEN_HEIGHT][SCREEN_WIDTH];

// ぷよのアスキーアート
char cellNames[][5] = {
	"・",   // CELL_NONE
	"■",	// CELL_WALL
	"◯",	// CELL_PUYO_0
	"▲",	// CELL_PUYO_1
	"☆",	// CELL_PUYO_2
	"★",	// CELL_PUYO_3
};

// 連鎖シュミレーター用
char chainSimulatorNames[][5] = {
	"a",     // CELL_NONE
	"",	    // CELL_WALL
	"b",	// CELL_PUYO_0
	"c",	// CELL_PUYO_1
	"e",	// CELL_PUYO_2
	"d",	// CELL_PUYO_3
};

// 軸ぷよのX座標
int puyoX = PUYO_START_X;
// 軸ぷよのY座標
int puyoY = PUYO_START_Y;
// 子ぷよの角度
int puyoAngle;
// ぷよの色
int puyoColor;
int puyoChildColor;
int puyoNextColor;
int puyoNextChildColor;
int puyoNextNextColor;
int puyoNextNextChildColor;

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

// 評価の高いノード
vector<Node> highRateNodes;
// ビームサーチの作業用ノード
vector<Node> nodes;

// 描画
void display() {
	// コンソールをクリア
	system("cls");

	// スクリーンにフィールドの情報を入れ込む
	for (int y = 0; y < SCREEN_HEIGHT; y++){
		for (int x = 0; x < SCREEN_WIDTH; x++){
			if (y < FIELD_HEIGHT && x < FIELD_WIDTH) {
				screen[y][x] = cells[y+1][x];
			}
		}
	}
	
	// ネクストぷよ
	screen[PUYO_NEXT_START_Y - 1][PUYO_NEXT_START_X - 1] = CELL_PUYO_0 + puyoNextChildColor;
	screen[PUYO_NEXT_START_Y][PUYO_NEXT_START_X - 1] = CELL_PUYO_0 + puyoNextColor;
	// ネクネクぷよ
	screen[PUYO_NEXT_NEXT_START_Y][PUYO_NEXT_NEXT_START_X - 1] = CELL_PUYO_0 + puyoNextNextChildColor;
	screen[PUYO_NEXT_NEXT_START_Y + 1][PUYO_NEXT_NEXT_START_X - 1] = CELL_PUYO_0 + puyoNextNextColor;

	// ボードを描画
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			printf("%s", cellNames[screen[y][x]]);
		}
		printf("\n");
	}
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
void fallPuyo(int _cells[FIELD_HEIGHT][FIELD_WIDTH]) {
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

// 連鎖があるかどうか
bool exsistChain(int _x, int _y, int _cell, int _cells[FIELD_HEIGHT][FIELD_WIDTH]){
	memset(checked, 0, sizeof checked);
	bool exists = false;
	for (int y = 0; y < FIELD_HEIGHT - 1; y++) {
		if (exists) break;
		for (int x = 1; x < FIELD_WIDTH - 1; x++) {
			if (exists) break;
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
ChainResult getChain(int _cells[FIELD_HEIGHT][FIELD_WIDTH], int chainCnt, int multiCnt) {
	int __cells[FIELD_HEIGHT][FIELD_WIDTH];
	for (int i = 0; i < FIELD_HEIGHT; i++) {
		for (int j = 0; j < FIELD_WIDTH; j++) {
			__cells[i][j] = _cells[i][j];
		}
	}
	
	int chainFlg = false;
	for (int y = 0; y < FIELD_HEIGHT - 1; y++) {
		for (int x = 1; x < FIELD_WIDTH - 1; x++) {
			if (__cells[y][x] == CELL_NONE)
				continue;
			if (getPuyoConnectedCount(x, y, __cells[y][x], 0, __cells) >= 4) {
				erasePuyo(x, y, __cells[y][x], __cells);
				chainFlg = true;
				multiCnt++;
			}
		}
	}
	if (!chainFlg) {
		ChainResult result = { chainCnt, multiCnt };
		return result;
	}
	fallPuyo(__cells);
	chainCnt++;
	memset(checked, 0, sizeof checked);
	return getChain(__cells, chainCnt, multiCnt);
}

// 移動可能な全てパターンのノードを作成
void createNodes(int tumoCount) {
	int _cells[FIELD_HEIGHT][FIELD_WIDTH];
	for (int nCnt = 0; nCnt < highRateNodes.size(); nCnt++) {
		for (int i = 0; i < SIZE_OF_ARRAY(moves); i++) {
			for (int j = 0; j < SIZE_OF_ARRAY(moves[0]); j++) {
				if (moves[i][j][0] == -9 && moves[i][j][1] == -9)
					continue;
				
				// フィールドの内容をコピー	
				memcpy(_cells, highRateNodes[nCnt].cells, sizeof highRateNodes[nCnt].cells);
				// 軸ぷよと子ぷよの座標
				int mainX = i + 1;
				int mainY = PUYO_START_Y;
				int subX = mainX + moves[i][j][0];
				int subY = mainY + moves[i][j][1];

				_cells[mainY][mainX] = CELL_PUYO_0 + puyoColor;
				_cells[subY][subX] = CELL_PUYO_0 + puyoChildColor;
				// 空中に浮いているぷよを落下させる
				fallPuyo(_cells);

				// 13段目以降にぷよを置いていない事
				if (_cells[0][1] != CELL_NONE
					||  _cells[0][2] != CELL_NONE
					|| _cells[0][3] != CELL_NONE
					|| _cells[0][4] != CELL_NONE
					|| _cells[0][5] != CELL_NONE
					||  _cells[0][6] != CELL_NONE) continue;

				// 連鎖していない事
				if (exsistChain(mainX, mainY, _cells[mainY][mainX], _cells)) {
					continue;
				}

				Node n;
				n.firstInput = 1;
				n.rate = 0;
				n.chain = 0;
				n.tumo = tumoCount;
				memcpy(n.cells, _cells, sizeof _cells);
				nodes.push_back(n);

			}
		}
	}
}

// 評価
void evaluation() {
	// 連結が多い
	for (int nCnt = 0; nCnt < nodes.size(); nCnt++) {
		int connectedRate = 0;
		memset(checked, 0, sizeof checked);
		for (int y = 0; y < FIELD_HEIGHT - 1; y++) {
			for (int x = 1; x < FIELD_WIDTH - 1; x++) {
				if (nodes[nCnt].cells[y][x] == CELL_NONE)
					continue;
				int connectedCount = getPuyoConnectedCount(x, y, nodes[nCnt].cells[y][x], 0, nodes[nCnt].cells);
				connectedRate += pow(getPuyoConnectedCount(x, y, nodes[nCnt].cells[y][x], 0, nodes[nCnt].cells), 2);
				
			}
		}
		nodes[nCnt].rate += connectedRate;
	}
	// 連鎖が多い
	for (int nCnt = 0; nCnt < nodes.size(); nCnt++) {
		int maxChain = 0;
		int connectedRate = 0;
		for (int x = 1; x < FIELD_WIDTH - 1; x++) {
			int __cells[FIELD_HEIGHT][FIELD_WIDTH];
			for (int i = 0; i < PUYO_COLOR_MAX - 1; i++) {
				for (int i = 0; i < FIELD_HEIGHT; i++) {
					for (int j = 0; j < FIELD_WIDTH; j++) {
						__cells[i][j] = nodes[nCnt].cells[i][j];
					}
				}
				__cells[0][x] = CELL_PUYO_0 + i;
				fallPuyo(__cells);

				memset(checked, 0, sizeof checked);
				ChainResult result = getChain(__cells, 0, 0);
				if (maxChain < result.chain) {
					maxChain = result.chain;
					if (maxChain >= 9) {

						int a = 0;
						// memcpy(cells, __cells, sizeof  __cells);
						//display();
						//printf("chain:%d", nodes[0].chain);
					}
				}
 			}
		}

		// 後々追加したい評処理
		// - 同時消しが少ない(不要？
		// - 4個消しが多い(不要?
		// - U字で組んでいるかどうか
		nodes[nCnt].rate += pow(maxChain, 3) * 10;
		nodes[nCnt].chain = maxChain;
	}

	//// 1,5列目が高い
	//for (int nCnt = 0; nCnt < nodes.size(); nCnt++) {
	//	int count1 = 0;
	//	int count2 = 0;
	//	int count5 = 0;
	//	int count6 = 0;
	//	int count3 = 0;
	//	int count4 = 0;
	//	memset(checked, 0, sizeof checked);
	//	for (int y = 0; y < FIELD_HEIGHT - 1; y++) {
	//		count1 += nodes[nCnt].cells[y][1] != CELL_NONE ? 1 : 0;
	//		count6 += nodes[nCnt].cells[y][6] != CELL_NONE ? 1 : 0;
	//		count2 += nodes[nCnt].cells[y][2] != CELL_NONE ? 0.5 : 0;
	//		count5 += nodes[nCnt].cells[y][5] != CELL_NONE ? 0.5 : 0;
	//	}
	//	int count = (count1 + count6 + count2 + count5 + count3 + count4);
	//	nodes[nCnt].rate += ((nodes[nCnt].tumo * CHAIN_TUMO)/ nodes[nCnt].tumo) * count / 30;
	//}
}

// 評価値の並び替え用
static bool nodeByRate(Node& a, Node& b) {
	return a.rate > b.rate;
}


#include <Windows.h>
// メイン
int main(void){
	HWND hwnd = nullptr;

	// get handles to a device context (DC)
	HDC hwindowDC = GetDC(hwnd);

	// フィールドに壁を書き込む
	for (int y = 0; y < FIELD_HEIGHT; y++){
		cells[y][0] = CELL_WALL;
		cells[y][FIELD_WIDTH - 1] = CELL_WALL;
	}
	for (int x = 0; x < FIELD_WIDTH; x++){
		cells[FIELD_HEIGHT - 1][x] = CELL_WALL;
	}

	// 擬似乱数の発生系列を変更する
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
	node.tumo = 0;
	highRateNodes.push_back(node);

	for (int i = 0; i < CHAIN_TUMO; i++){
		// 移動可能な全パターンのノードを生成
		createNodes(i + 1);
		// 評価
		evaluation();

		// 評価の高いノードを抜き出す。
		sort(nodes.begin(), nodes.end(), nodeByRate);
		if (nodes.size() > BEAM_WIDTH) {
			nodes.resize(BEAM_WIDTH);
		}
		highRateNodes.resize(nodes.size());
		for (size_t i = 0; i < nodes.size(); i++) {
			highRateNodes[i] = nodes[i];
		}
		nodes.resize(0);

		puyoColor = puyoNextColor;
		puyoChildColor = puyoNextChildColor;
		puyoNextColor = puyoNextNextColor;
		puyoNextChildColor = puyoNextNextChildColor;
		puyoNextNextColor = rand() % PUYO_COLOR_MAX;
		puyoNextNextChildColor = rand() % PUYO_COLOR_MAX;
	}
	
	// // 画面出力
	memcpy(cells, highRateNodes[0].cells, sizeof  highRateNodes[0].cells);
	display();
	printf("chain:%d\n", highRateNodes[0].chain);

	// 連鎖シュミレーターのURL生成
	string param = "";
	int beforCell = cells[0][1];
	int cellCnt = 0;
	for (int y = 0; y < FIELD_HEIGHT - 1; y++) {
		for (int x = 1; x < FIELD_WIDTH - 1; x++) {
			if (beforCell != cells[y][x]) {
				if (cellCnt == 1) {
					param = param + (chainSimulatorNames[beforCell]);
				}
				else {
					param = param + (chainSimulatorNames[beforCell]) + std::to_string(cellCnt);
				}

				cellCnt = 1;
				beforCell = cells[y][x];
			}
			else {
				cellCnt++;
			}
		}
	}
	if (cellCnt == 1) {
		param = param + (chainSimulatorNames[beforCell]);
	}
	else {
		param = param + (chainSimulatorNames[beforCell]) + std::to_string(cellCnt);
	}
	printf("https://www.pndsng.com/puyo/index.html?%s", param.c_str());

	getchar();
}
	