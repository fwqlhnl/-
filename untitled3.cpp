#include<bits/stdc++.h>
#include <graphics.h>
#include <vector>
#include <conio.h>
#include <cstdio>
#include <thread>
#include <atomic>
#include <cmath>
#include <sstream>
#include <string>

using namespace std;

// 常量定义
const int WINDOW_WIDTH = 1000;       // 窗口宽度
const int WINDOW_HEIGHT = 700;       // 窗口高度
const int FRAME_DELAY = 100;         // 每帧延迟（毫秒）
const int STEP_SIZE = 5;             // 每步移动的距离
const int INF = -1;                  // 表示无穷大
const int MAX_VERTEX_NUM = 100;      // 最大顶点数量

// 全局变量
vector<pair<int, int>> nodes;        // 顶点坐标
vector<vector<int>> dist;            // 边权矩阵
int pre[MAX_VERTEX_NUM];             // 前驱节点
int proce[MAX_VERTEX_NUM];           // 路径节点
int startVertex = -1, endVertex = -1; // 起点和终点
atomic<bool> isRunning(true);        // 控制动画线程是否运行

// 点的结构体
typedef struct Point {
	int x, y;
} Point;

Point vertexPoints[MAX_VERTEX_NUM];  // 顶点坐标

// 将整数转换为字符串
std::string intToString(int num) {
	std::stringstream ss;
	ss << num;
	return ss.str();
}

// 动画函数
void cartoon() {
	std::vector<IMAGE> frames;
	for (int i = 1; i <= 16; i++) {
		IMAGE img;
		char path[20];
		sprintf(path, "%d.jpg", i);  // 生成图片路径
		loadimage(&img, path);       // 加载图片
		// 检查图片是否加载成功
		if (img.getwidth() > 0 && img.getheight() > 0) {
			frames.push_back(img);
		} else {
			printf("Failed to load image: %s\n", path);
		}
	}
	if (frames.empty()) {
		printf("No images loaded!\n");
		return;
	}
	// 开启批量绘制模式
	BeginBatchDraw();
	int x = WINDOW_WIDTH;// 图片从最右边开始
	int y = WINDOW_HEIGHT - frames[0].getheight();  // 图片在底部
	// 动画循环
	int currentFrame = 0;
	while (isRunning) {  // 根据 isRunning 控制循环	
		putimage(x, y, &frames[currentFrame]);// 绘制当前帧	
		x -= STEP_SIZE;// 更新图片位置
		if (x + frames[currentFrame].getwidth() < 0) {// 如果图片完全移出窗口左侧，重置位置到最右边
			x = WINDOW_WIDTH;
		}
		currentFrame = (currentFrame + 1) % frames.size();// 切换到下一张图片
		// 刷新屏幕
		FlushBatchDraw();
		// 延时（控制动画速度）
		Sleep(FRAME_DELAY);
	}
	// 关闭批量绘制模式
	EndBatchDraw();
}

// 绘制缓慢移动的线段
void drawLineSlowly(int x1, int y1, int x2, int y2, int delay) {
	float dx = x2 - x1;
	float dy = y2 - y1;
	float length = sqrt(dx * dx + dy * dy); // 线段长度
	
	// 计算每一步的增量
	float stepX = dx / length;
	float stepY = dy / length;
	
	// 设置线条粗细
	int thickness = 9; // 线条粗细
	int halfThickness = thickness / 2;
	
	// 逐步绘制线段
	float x = x1, y = y1;
	for (int i = 0; i <= length; i++) {
		// 绘制一个矩形区域来模拟粗线
		for (int tx = -halfThickness; tx <= halfThickness; tx++) {
			for (int ty = -halfThickness; ty <= halfThickness; ty++) {
				putpixel((int)x + tx, (int)y + ty, GREEN); // 绘制当前点
			}
		}
		x += stepX; // 更新 x 坐标
		y += stepY; // 更新 y 坐标
		Sleep(delay); // 延迟，控制绘制速度
	}
}

// 闪烁源点
void blinkSourcePoint(int x, int y) {
	setwritemode(R2_XORPEN);
	for (int i = 0; i < 3; i++) {
		// 绘制源点（圆形）
		setfillcolor(BLUE); // 设置填充颜色
		solidcircle(x, y, 25); // 绘制实心圆
		
		// 延迟
		Sleep(150);
		
		// 清除源点（用背景色覆盖）
		setfillcolor(WHITE); // 设置背景色
		solidcircle(x, y, 25); // 用背景色覆盖圆
		
		// 延迟
		Sleep(150);
	}
	setlinecolor(BLACK);
	circle(x, y, 25);
	setwritemode(R2_COPYPEN);
}

// 闪烁线段
void blinkLine(int x1, int y1, int x2, int y2) {
	float dx = x2 - x1;
	float dy = y2 - y1;
	float d = sqrt(dx * dx + dy * dy);
	
	// 计算切点坐标
	float tx1 = x1 + (25 * dx / d);
	float ty1 = y1 + (25 * dy / d);
	float tx2 = x2 - (25 * dx / d);
	float ty2 = y2 - (25 * dy / d);
	setlinestyle(PS_SOLID, 5);
	for (int i = 0; i < 2; i++) {
		// 绘制线段
		setlinecolor(YELLOW); // 设置线段颜色
		line(tx1, ty1, tx2, ty2); // 绘制线段
		
		// 延迟
		Sleep(200);
		
		// 清除线段（用背景色覆盖）
		setlinecolor(BLACK); // 设置背景色
		line(tx1, ty1, tx2, ty2); // 用背景色覆盖线段
		
		// 延迟
		Sleep(200);
	}
	setlinestyle(PS_SOLID, 6);
	setlinecolor(RGB(192, 192, 192)); // 设置背景色
	line(tx1, ty1, tx2, ty2); // 用背景色覆盖线段
	Sleep(350);
	setlinestyle(PS_SOLID, 4);
	setlinecolor(BLACK); // 设置背景色
	line(tx1, ty1, tx2, ty2); // 用背景色覆盖线段
	Sleep(350);
}

// 更新表格
void changeTable(int k, int *distance, int *selected) {
	int left = k * 48 + 500;
	for (int i = 0; i <= 9; i++) {
		if (selected[i]) continue;
		if (distance[i] == INF) {
			setlinecolor(BLACK);
			line(left, i * 30 + 110, left + 48, i * 30 + 140); // 反斜杠
		} else {
			std::string text = intToString(distance[i]); // 将整数转换为字符串
			outtextxy(left + 15, 120 + i * 30, text.c_str());
		}
	}
}

// Dijkstra算法的实现
void dijkstra() {
	int distance[10] = {INF}; // 距离数组
	int selected[10] = {0};   // 已选择数组
	selected[startVertex] = 1;
	for (int i = 0; i < 10; i++) {
		distance[i] = dist[startVertex][i];
		if (dist[startVertex][i] != INF) {
			pre[i] = startVertex;
		}
	}
	int k = 1;
	int pot = startVertex;
	for (int a = 0; a < 9; a++) {
		blinkSourcePoint(nodes[pot].first, nodes[pot].second);
		setwritemode(R2_XORPEN);
		setfillcolor(RED);
		solidcircle(nodes[pot].first, nodes[pot].second, 25);
		setwritemode(R2_COPYPEN);
		
		int SMIN = INF;
		int min = -1;
		for (int j = 0; j < 10; j++) {
			if (selected[j] == 0 && dist[pot][j] != INF) {
				blinkLine(nodes[pot].first, nodes[pot].second, nodes[j].first, nodes[j].second);
			}
			if (selected[j] == 0 && ((distance[j] < SMIN) || (SMIN == INF)) && distance[j] != INF) {
				min = j;
				SMIN = distance[j];
			}
		}
		
		if (min == -1) break;
		
		changeTable(k, distance, selected);
		Sleep(1000);
		setwritemode(R2_XORPEN);
		setfillcolor(RED);
		solidrectangle(k * 48 + 500, min * 30 + 110, k * 48 + 500 + 48, min * 30 + 140);
		setwritemode(R2_COPYPEN);
		selected[min] = 1; // 最小距离的被选出
		pot = min;
		k++;
		for (int k = 0; k < 10; k++) { // 根据被选出的顶点更新distance数组
			if (selected[k] == 1) continue;
			if (distance[k] == INF && dist[min][k] != INF) {
				distance[k] = distance[min] + dist[min][k] + 1;
				pre[k] = min;
			} else if (distance[k] != INF && dist[min][k] != INF) {
				if (selected[k] == 0 && distance[k] > distance[min] + dist[min][k]) {
					distance[k] = distance[min] + dist[min][k];
					pre[k] = min;
				}
			}
		}
	}
	blinkSourcePoint(nodes[pot].first, nodes[pot].second);
	setwritemode(R2_XORPEN);
	setfillcolor(RED);
	solidcircle(nodes[pot].first, nodes[pot].second, 25);
	setwritemode(R2_COPYPEN);
}

// 绘制界面
void Create_interface() {
	initgraph(WINDOW_WIDTH, WINDOW_HEIGHT); // 初始化图形窗口
	setbkcolor(YELLOW);
	cleardevice();
	setlinecolor(RGB(0, 0, 0));
	settextcolor(RGB(0, 0, 0));
}

// 绘制节点
void drawNodes(const vector<pair<int, int>> &nodes) {
	for (int i = 0; i < int(nodes.size()); i++) {
		int x = nodes[i].first;
		int y = nodes[i].second;
		vertexPoints[i].x = x;
		vertexPoints[i].y = y;
		circle(x, y, 25); // 绘制节点
		char label[2];
		sprintf(label, "%d", i);
		outtextxy(x - 7, y - 7, label); // 在节点上显示编号
	}
}

// 绘制边及边权
void drawEdges(const vector<pair<int, int>> &nodes, const vector<vector<int>> &dist) {
	for (int i = 0; i < 10; i++) {
		for (int j = i + 1; j < 10; j++) {
			if (dist[i][j] != INF && i != j) {
				int x1 = nodes[i].first;
				int y1 = nodes[i].second;
				int x2 = nodes[j].first;
				int y2 = nodes[j].second;
				
				float dx = x2 - x1;
				float dy = y2 - y1;
				float d = sqrt(dx * dx + dy * dy);
				
				int tx1 = static_cast<int>(round(x1 + (25 * dx / d)));
				int tx2 = static_cast<int>(round(x2 - (25 * dx / d)));
				int ty1 = static_cast<int>(round(y1 + (25 * dy / d)));
				int ty2 = static_cast<int>(round(y2 - (25 * dy / d)));
				
				setlinecolor(BLACK); // 设置线条颜色
				line(tx1, ty1, tx2, ty2); // 绘制边
				char weight[10];
				sprintf(weight, "%d", dist[i][j]);
				settextcolor(BLACK);
				outtextxy((tx1 + tx2) / 2 + 3, (ty1 + ty2) / 2 + 3, weight); // 绘制权值
			}
		}
	}
}

// 绘制路径
void drawProcess() {
	int pot = 0, tu = endVertex;
	while (tu != startVertex) {
		proce[pot++] = tu;
		tu = pre[tu];
	}
	proce[pot] = tu;
	settextstyle(24, 0, _T("黑体"));
	char text[2];
	sprintf(text, "%d", proce[pot]);
	outtextxy(500, 450, text);
	cout<<proce[pot];
	Sleep(350);
	for (int i = pot; i > 0; i--) {
		outtextxy(500 + (pot - i) * 33 + 15, 450, "-");
		outtextxy(500 + (pot - i) * 33 + 17, 450, "-");
		outtextxy(500 + (pot - i) * 33 + 18, 450, ">");
		cout<<"—>";
		char text[2];
		sprintf(text, "%d", proce[i - 1]);
		outtextxy(500 + (pot - i) * 33 + 33, 450, text);
		cout<<proce[i-1];
		Sleep(350);
		int x1 = nodes[proce[i]].first;
		int y1 = nodes[proce[i]].second;
		int x2 = nodes[proce[i - 1]].first;
		int y2 = nodes[proce[i - 1]].second;
		float dx = x2 - x1;
		float dy = y2 - y1;
		float d = sqrt(dx * dx + dy * dy);
		float tx1 = x1 + (25 * dx / d);
		float ty1 = y1 + (25 * dy / d);
		float tx2 = x2 - (25 * dx / d);
		float ty2 = y2 - (25 * dy / d);
		setlinecolor(YELLOW);
		drawLineSlowly(tx1, ty1, tx2, ty2, 10); // 绘制路径
	}
}

// 绘制表格
void drawTable() {
	setlinestyle(PS_SOLID, 3);
	int rows = 11;
	int cols = 10;
	int cellWidth = 48;
	int cellHeight = 30;
	setlinecolor(BLACK);
	for (int i = 0; i <= rows; i++) {
		line(500, 80 + i * cellHeight, 500 + cols * cellWidth, 80 + i * cellHeight);
	}
	for (int j = 0; j <= cols; j++) {
		line(500 + j * cellWidth, 80, 500 + j * cellWidth, 80 + rows * cellHeight);
	}
	std::string a[9] = {"一", "二", "三", "四", "五", "六", "七", "八", "九"};
	std::string b[10] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
	int ant = 48;
	for (int i = 0; i < 9; i++) {
		outtextxy(565 + ant * i, 90, a[i].c_str()); // 使用 a[i].c_str() 传递字符串
	}
	int cnt = 30;
	for (int i = 0; i <= 9; i++) {
		outtextxy(524, 120 + i * cnt, b[i].c_str()); // 使用 b[i].c_str() 传递字符串
	}
}

// 绘制按钮
void drawButton() {
	line(0, 490, 1000, 490);
	rectangle(50, 500, 170, 540);
	outtextxy(95, 515, "重置");
	rectangle(200, 500, 320, 540);
	outtextxy(230, 515, "退出系统");
}

// 绘制初始界面
void drawStart() {
	setbkmode(TRANSPARENT);
	setfillcolor(RGB(192, 192, 192));
	fillrectangle(0, 0, 1000, 490);
	setfillcolor(WHITE);
	fillrectangle(0, 490, 1000, 700);
	setlinestyle(PS_SOLID, 1);
	settextstyle(17, 0, _T("黑体"));
	setcolor(BLACK);
	setlinestyle(PS_SOLID, 4);
	drawEdges(nodes, dist);
	setlinestyle(PS_SOLID, 3);
	drawNodes(nodes);
	drawTable();
	drawButton();
}

// 获取点击的顶点索引
int getVertexIndexByClick(Point clickPoint) {
	for (int i = 0; i < 10; i++) {
		int dx = clickPoint.x - vertexPoints[i].x;
		int dy = clickPoint.y - vertexPoints[i].y;
		if (dx * dx + dy * dy <= 25 * 25) {  // 判断点击是否在顶点的圆形范围内
			return i;
		}
	}
	if (clickPoint.x >= 50 && clickPoint.x <= 170 && clickPoint.y >= 500 && clickPoint.y <= 540) {
		return -2; // 重置按钮
	}
	if (clickPoint.x >= 200 && clickPoint.x <= 320 && clickPoint.y >= 500 && clickPoint.y <= 540) {
		return -3; // 退出按钮
	}
	return -1; // 未点击到有效顶点
}

// 处理鼠标点击事件
void handleMouseClick() {
	MOUSEMSG msg;
	while (isRunning) {
		if (_kbhit()) {
			char ch = _getch();
			if (ch == 27) {  // ESC 键退出
				isRunning = false;
				break;
			}
		}
		
		if (MouseHit()) {  // 检测鼠标点击
			msg = GetMouseMsg();
			if (msg.uMsg == WM_LBUTTONDOWN) {
				int clickedVertex = getVertexIndexByClick((Point){msg.x, msg.y});
				if (clickedVertex == -2) { // 重置按钮
					cleardevice();
					drawStart();
					startVertex = -1, endVertex = -1;
				} else if (clickedVertex == -3) { // 退出按钮
					isRunning = false;
					break;
				} else if (startVertex == -1) { // 选择起点
					startVertex = clickedVertex;
					setwritemode(R2_XORPEN);
					setfillcolor(RGB(255, 0, 0));
					fillcircle(vertexPoints[startVertex].x, vertexPoints[startVertex].y, 25);
					setwritemode(R2_COPYPEN);
				} else if (endVertex == -1 && clickedVertex != startVertex) { // 选择终点
					endVertex = clickedVertex;
					setwritemode(R2_XORPEN);
					setfillcolor(RGB(255, 0, 0));
					fillcircle(vertexPoints[endVertex].x, vertexPoints[endVertex].y, 25);
					setwritemode(R2_COPYPEN);
					dijkstra(); // 执行 Dijkstra 算法
					drawProcess(); // 绘制结果路径
				}
			}
		}
	}
}


// 主函数

int main() {
	// 初始化节点坐标
	nodes = {
		{100, 220}, // 0
		{200, 300}, // 1
		{120, 350}, // 2
		{300, 220}, // 3
		{200, 420}, // 4
		{430, 270}, // 5
		{340, 340}, // 6
		{370, 400}, // 7
		{400, 110}, // 8
		{150, 50}   // 9
	};
	// 初始化边权矩阵

	dist = {
		{INF, INF, INF, 5, INF, INF, INF, INF, INF, 3},
		{INF, INF, 8, 4, 1, INF, INF, INF, INF, INF},
		{INF, 8, INF, INF, INF, INF, INF, INF, INF, INF},
		{5, 4, INF, INF, INF, INF, INF, INF, 7, INF},
		{INF, 1, INF, INF, INF, INF, 7, INF, INF, INF},
		{INF, INF, INF, INF, INF, INF, 5, 10, INF, INF},
		{INF, INF, INF, INF, 7, 5, INF, INF, 12, INF},
		{INF, INF, INF, INF, INF, 10, INF, INF, INF, INF},
		{INF, INF, INF, 7, INF, INF, 12, INF, INF, 2},
		{3, INF, INF, INF, INF, INF, INF, INF, 2, INF}
	};
	
	// 创建界面
	Create_interface();
	drawStart(); // 绘制初始界面

	// 创建动画线程
	std::thread animationThread(cartoon);

	// 处理鼠标点击
	handleMouseClick();

	// 等待动画线程结束
	animationThread.join();

	// 关闭图形窗口
	closegraph();
	return 0;

}
