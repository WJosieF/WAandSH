// OpenGL02WA.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <GL/glut.h>
#include <Windows.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstdio>
using namespace std;

// 点结构体
struct Point {
    float x, y;
    int type;  // 0:顶点, 1:交点, 2:进入点, 3:离开点
    Point(float _x = 0, float _y = 0, int _type = 0) : x(_x), y(_y), type(_type) {}

    bool operator==(const Point& other) const {
        return fabs(x - other.x) < 1e-6 && fabs(y - other.y) < 1e-6;
    }
};

// 边结构体
struct Edge {
    Point start, end;
    Edge(Point s, Point e) : start(s), end(e) {}
};

// 窗口边界定义
const float WIN_LEFT = 100.0f;
const float WIN_RIGHT = 500.0f;
const float WIN_BOTTOM = 100.0f;
const float WIN_TOP = 500.0f;

// 原始多边形顶点（顺时针顺序）
vector<Point> originalPolygon = {
    Point(100, 500),  // A
    Point(300, 700),  // B
    Point(500, 500),  // C
    Point(500, 300),  // D
    Point(300, 100),  // E
    Point(100, 300)   // F
};

// 裁剪窗口顶点（顺时针顺序）
vector<Point> clipWindow = {
    Point(WIN_LEFT, WIN_BOTTOM),
    Point(WIN_RIGHT, WIN_BOTTOM),
    Point(WIN_RIGHT, WIN_TOP),
    Point(WIN_LEFT, WIN_TOP)
};

vector<vector<Point>> clippedPolygons_WA;  // WA算法裁剪结果
vector<Point> intersectionPoints;          // 所有交点
vector<Point> subjectPolygon;              // 带交点的多边形环
vector<Point> clipPolygon;                 // 带交点的窗口环

int winWidth = 800, winHeight = 800;
int currentStep = 0;  // 当前演示步骤
bool showIntersections = true;  // 是否显示交点

// 判断点是否在窗口内
bool isPointInsideWindow(float x, float y) {
    return x >= WIN_LEFT && x <= WIN_RIGHT && y >= WIN_BOTTOM && y <= WIN_TOP;
}

// 判断点是否在边上
bool isPointOnLine(Point p, Point a, Point b) {
    // 检查点是否在线段ab上
    float cross = (p.y - a.y) * (b.x - a.x) - (p.x - a.x) * (b.y - a.y);
    if (fabs(cross) > 1e-6) return false;

    float dot = (p.x - a.x) * (b.x - a.x) + (p.y - a.y) * (b.y - a.y);
    if (dot < 0) return false;

    float lengthSquared = (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
    if (dot > lengthSquared) return false;

    return true;
}

// 计算两条线段的交点
Point computeLineIntersection(Point p1, Point p2, Point q1, Point q2) {
    float x1 = p1.x, y1 = p1.y;
    float x2 = p2.x, y2 = p2.y;
    float x3 = q1.x, y3 = q1.y;
    float x4 = q2.x, y4 = q2.y;

    float denominator = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

    if (fabs(denominator) < 1e-6) {
        // 平行或重合，返回无效点
        return Point(-1, -1);
    }

    float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denominator;
    float u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denominator;

    // 检查交点是否在线段上
    if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
        float x = x1 + t * (x2 - x1);
        float y = y1 + t * (y2 - y1);
        return Point(x, y, 1);
    }

    return Point(-1, -1);
}

// 判断交点类型：进入点(2)或离开点(3)
int getIntersectionType(Point p, Point next, Point intersection) {
    bool pInside = isPointInsideWindow(p.x, p.y);
    bool nextInside = isPointInsideWindow(next.x, next.y);

    if (!pInside && nextInside) {
        return 2;  // 进入点：从外到内
    }
    else if (pInside && !nextInside) {
        return 3;  // 离开点：从内到外
    }
    return 1;  // 普通交点
}

// 计算多边形和窗口的所有交点
void computeAllIntersections() {
    subjectPolygon = originalPolygon;
    clipPolygon = clipWindow;
    intersectionPoints.clear();

    int nPoly = originalPolygon.size();
    int nClip = clipWindow.size();

    // 计算所有交点
    for (int i = 0; i < nPoly; i++) {
        Point p1 = originalPolygon[i];
        Point p2 = originalPolygon[(i + 1) % nPoly];

        for (int j = 0; j < nClip; j++) {
            Point q1 = clipWindow[j];
            Point q2 = clipWindow[(j + 1) % nClip];

            Point intersection = computeLineIntersection(p1, p2, q1, q2);
            if (intersection.x >= 0) {  // 有效交点
                intersection.type = getIntersectionType(p1, p2, intersection);
                intersectionPoints.push_back(intersection);
            }
        }
    }

    // 将交点按插入位置排序并插入到多边形和窗口中
    // （简化实现：这里使用一个简化的交点处理）
    cout << "找到 " << intersectionPoints.size() << " 个交点" << endl;
}

// 获取多边形在交点处的下一个点
Point getNextPointInPolygon(const vector<Point>& polygon, Point current) {
    for (size_t i = 0; i < polygon.size(); i++) {
        if (polygon[i] == current) {
            return polygon[(i + 1) % polygon.size()];
        }
    }
    return Point(-1, -1);
}

// 判断点是否在多边形顶点列表中
int findPointIndex(const vector<Point>& polygon, Point p) {
    for (size_t i = 0; i < polygon.size(); i++) {
        if (polygon[i] == p) {
            return i;
        }
    }
    return -1;
}

// Weiler-Atherton 算法实现（简化版本）
void performWAClipping() {
    computeAllIntersections();
    clippedPolygons_WA.clear();

    if (intersectionPoints.empty()) {
        // 没有交点，检查多边形是否完全在窗口内或外
        bool allInside = true;
        for (const auto& p : originalPolygon) {
            if (!isPointInsideWindow(p.x, p.y)) {
                allInside = false;
                break;
            }
        }

        if (allInside) {
            clippedPolygons_WA.push_back(originalPolygon);
        }
        return;
    }

    // 简化的WA算法实现：逐个处理交点
    vector<bool> processed(intersectionPoints.size(), false);

    for (size_t i = 0; i < intersectionPoints.size(); i++) {
        if (processed[i]) continue;

        vector<Point> clippedPolygon;
        Point current = intersectionPoints[i];
        bool followingSubject = true;  // 当前是否沿着多边形边界前进

        do {
            clippedPolygon.push_back(current);

            if (followingSubject) {
                // 沿着多边形边界前进
                // 找到下一个点（简化：直接找下一个交点）
                for (size_t j = 0; j < intersectionPoints.size(); j++) {
                    if (!processed[j] && !(intersectionPoints[j] == current)) {
                        current = intersectionPoints[j];
                        processed[j] = true;

                        // 检查是否需要切换到窗口边界
                        if (current.type == 3) {  // 离开点
                            followingSubject = false;
                        }
                        break;
                    }
                }
            }
            else {
                // 沿着窗口边界前进
                // 找到下一个点（简化：直接找下一个交点）
                for (size_t j = 0; j < intersectionPoints.size(); j++) {
                    if (!processed[j] && !(intersectionPoints[j] == current)) {
                        current = intersectionPoints[j];
                        processed[j] = true;

                        // 检查是否需要切换回多边形边界
                        if (current.type == 2) {  // 进入点
                            followingSubject = true;
                        }
                        break;
                    }
                }
            }

        } while (!(current == intersectionPoints[i]) && clippedPolygon.size() < 100);

        if (clippedPolygon.size() > 2) {
            clippedPolygons_WA.push_back(clippedPolygon);
        }
    }

    cout << "WA算法生成 " << clippedPolygons_WA.size() << " 个裁剪多边形" << endl;
}

// 绘制多边形
void drawPolygon(const vector<Point>& polygon, float r, float g, float b, bool fill = false) {
    if (polygon.empty()) return;

    if (fill) {
        glColor3f(r * 0.7f, g * 0.7f, b * 0.7f);
        glBegin(GL_POLYGON);
    }
    else {
        glColor3f(r, g, b);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
    }

    for (const auto& p : polygon) {
        glVertex2f(p.x, p.y);
    }

    glEnd();

    // 绘制顶点
    glPointSize(6.0f);
    glBegin(GL_POINTS);
    for (const auto& p : polygon) {
        glVertex2f(p.x, p.y);
    }
    glEnd();
}

// 绘制裁剪窗口
void drawClipWindow() {
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    for (const auto& p : clipWindow) {
        glVertex2f(p.x, p.y);
    }
    glEnd();
}

// 绘制交点
void drawIntersections() {
    if (!showIntersections) return;

    glPointSize(10.0f);
    for (const auto& p : intersectionPoints) {
        if (p.type == 2) {  // 进入点：绿色
            glColor3f(0.0f, 1.0f, 0.0f);
        }
        else if (p.type == 3) {  // 离开点：红色
            glColor3f(1.0f, 0.0f, 0.0f);
        }
        else {  // 普通交点：黄色
            glColor3f(1.0f, 1.0f, 0.0f);
        }
        glBegin(GL_POINTS);
        glVertex2f(p.x, p.y);
        glEnd();
    }
}

// 显示图形
void Display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // 绘制坐标网格
    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_LINES);
    for (int x = 0; x <= winWidth; x += 50) {
        glVertex2f(x, 0);
        glVertex2f(x, winHeight);
    }
    for (int y = 0; y <= winHeight; y += 50) {
        glVertex2f(0, y);
        glVertex2f(winWidth, y);
    }
    glEnd();

    // 绘制裁剪窗口
    drawClipWindow();

    // 根据当前步骤显示不同内容
    switch (currentStep) {
    case 0: // 显示原始多边形
        drawPolygon(originalPolygon, 1.0f, 0.0f, 0.0f, true);
        break;

    case 1: // 显示交点
        drawPolygon(originalPolygon, 0.7f, 0.3f, 0.3f, false);
        showIntersections = true;
        drawIntersections();
        break;

    case 2: // 显示裁剪结果
        showIntersections = false;
        // 用不同颜色绘制每个裁剪多边形
        float colors[][3] = {
            {0.0f, 0.5f, 1.0f},  // 蓝色
            {0.0f, 1.0f, 0.5f},  // 绿色
            {1.0f, 0.5f, 0.0f},  // 橙色
            {0.5f, 0.0f, 1.0f}   // 紫色
        };

        for (size_t i = 0; i < clippedPolygons_WA.size(); i++) {
            float r = colors[i % 4][0];
            float g = colors[i % 4][1];
            float b = colors[i % 4][2];
            drawPolygon(clippedPolygons_WA[i], r, g, b, true);
        }
        break;

    case 3: // 同时显示所有元素
        drawPolygon(originalPolygon, 0.7f, 0.3f, 0.3f, false);
        for (size_t i = 0; i < clippedPolygons_WA.size(); i++) {
            float r = 0.0f, g = 0.5f, b = 1.0f;
            drawPolygon(clippedPolygons_WA[i], r, g, b, true);
        }
        showIntersections = true;
        drawIntersections();
        break;
    }

    // 显示当前步骤信息
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2f(10, 780);

    char stepInfo[100];
    switch (currentStep) {
    case 0: sprintf(stepInfo, "步骤0: 原始多边形 (按空格键继续)"); break;
    case 1: sprintf(stepInfo, "步骤1: 计算多边形与窗口的交点"); break;
    case 2: sprintf(stepInfo, "步骤2: Weiler-Atherton裁剪结果"); break;
    case 3: sprintf(stepInfo, "步骤3: 综合显示 (I:显示/隐藏交点)"); break;
    }

    for (int i = 0; stepInfo[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, stepInfo[i]);
    }

    // 显示交点数量
    glColor3f(0.3f, 0.3f, 0.3f);
    glRasterPos2f(10, 750);
    char intersectInfo[50];
    sprintf(intersectInfo, "交点数量: %d", (int)intersectionPoints.size());
    for (int i = 0; intersectInfo[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, intersectInfo[i]);
    }

    // 显示裁剪结果数量
    glRasterPos2f(10, 730);
    char resultInfo[50];
    sprintf(resultInfo, "裁剪多边形数量: %d", (int)clippedPolygons_WA.size());
    for (int i = 0; resultInfo[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, resultInfo[i]);
    }

    glFlush();
}

// 初始化OpenGL场景
void Initial() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, winWidth, 0, winHeight);
}

// 键盘响应函数
void KeyEvent(unsigned char key, int x, int y) {
    switch (key) {
    case ' ':  // 空格键下一步
        currentStep = (currentStep + 1) % 4;
        glutPostRedisplay();
        break;

    case 'i':  // I键切换交点显示
    case 'I':
        showIntersections = !showIntersections;
        glutPostRedisplay();
        break;

    case 'r':  // R键重置
        currentStep = 0;
        glutPostRedisplay();
        break;

    case '0':  // 直接跳转到步骤
    case '1':
    case '2':
    case '3':
        currentStep = key - '0';
        glutPostRedisplay();
        break;

    case 27:  // ESC键退出
        exit(0);
        break;
    }
}

// 窗口大小变化响应函数
void ChangeSize(int w, int h) {
    winWidth = w;
    winHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
}

int main(int argc, char* argv[]) {
    // 执行WA算法裁剪
    performWAClipping();

    // 打印结果信息
    cout << "\nWeiler-Atherton多边形裁剪算法" << endl;
    cout << "==============================" << endl;
    cout << "原始多边形顶点数: " << originalPolygon.size() << endl;
    cout << "裁剪窗口顶点数: " << clipWindow.size() << endl;
    cout << "找到的交点数量: " << intersectionPoints.size() << endl;
    cout << "生成的裁剪多边形数量: " << clippedPolygons_WA.size() << endl;

    for (size_t i = 0; i < clippedPolygons_WA.size(); i++) {
        cout << "裁剪多边形" << i + 1 << "顶点数: " << clippedPolygons_WA[i].size() << endl;
    }

    // 初始化GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(winWidth, winHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Weiler-Atherton多边形裁剪算法");

    glutDisplayFunc(Display);
    glutReshapeFunc(ChangeSize);
    glutKeyboardFunc(KeyEvent);

    Initial();
    glutMainLoop();

    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
