#include <GL/glut.h>
#include <iostream>
#include <vector>
#include "Common.h"
#include "SH_Algorithm.h"
#include "WA_Algorithm.h"

#include <algorithm> 

using namespace std;

// 获取阶段结果
vector<Point> currentPoly;

// 全局变量
vector<Point> originalPolygon = {
    Point(100, 500),   // A
    Point(600, 800),   // B
    Point(700, 500),   // C
    Point(300, 400),   // D
    Point(500, 300),   // E
    Point(300, 200)    // F
};

vector<Point> clipWindow = {
    Point(WIN_LEFT, WIN_BOTTOM),
    Point(WIN_RIGHT, WIN_BOTTOM),
    Point(WIN_RIGHT, WIN_TOP),
    Point(WIN_LEFT, WIN_TOP)
};

int currentStage = 0;
int algorithmMode = 0;  // 0: SH算法, 1: WA算法
bool showWindow = true;
int winWidth = 1000, winHeight = 900;

// 绘制裁剪窗口
void drawClipWindow() {
    if (!showWindow) return;

    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(WIN_LEFT, WIN_BOTTOM);
    glVertex2f(WIN_RIGHT, WIN_BOTTOM);
    glVertex2f(WIN_RIGHT, WIN_TOP);
    glVertex2f(WIN_LEFT, WIN_TOP);
    glEnd();
}

// 绘制多边形
void drawPolygon(const vector<Point>& polygon, float r, float g, float b, bool isFinalStage = false) {
    if (polygon.empty()) return;

    // 绘制填充
    if (isFinalStage) {
        glColor4f(r * 0.6f, g * 0.6f, b * 0.6f, 0.3f);
        glBegin(GL_POLYGON);
        for (const auto& p : polygon) {
            glVertex2f(p.x, p.y);
        }
        glEnd();
    }

    // 绘制边线
    glColor3f(r, g, b);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    for (const auto& p : polygon) {
        glVertex2f(p.x, p.y);
    }
    glEnd();

    // 绘制顶点
    glPointSize(8.0f);
    glBegin(GL_POINTS);
    for (const auto& p : polygon) {
        glVertex2f(p.x, p.y);
    }
    glEnd();
}

// 绘制交点
void drawIntersections(const vector<Point>& polygon) {
    glColor3f(1.0f, 0.5f, 0.0f); // 橙色
    glPointSize(10.0f);
    glBegin(GL_POINTS);
    for (const auto& p : polygon) {
        if (p.intersect) {
            glVertex2f(p.x, p.y);
        }
    }
    glEnd();
}



// 显示当前阶段
void displayCurrentStage() {
    vector<Point> currentPoly;
    int totalStages;
    string algorithmName;
    bool isFinalStage = false;

    if (algorithmMode == 0) { // SH算法
        currentPoly = SutherlandHodgeman::getStageResult(originalPolygon, currentStage);
        totalStages = SutherlandHodgeman::getTotalStages();
        algorithmName = "Sutherland-Hodgeman算法";
        isFinalStage = (currentStage == totalStages - 1); // SH的最后一个阶段
    }
    else { // WA算法
        totalStages = WeilerAtherton::getTotalStages();
        algorithmName = "Weiler-Atherton算法";
        isFinalStage = (currentStage == totalStages - 1);

        if (currentStage == 0) {
            // 阶段0：显示原始多边形
            currentPoly = originalPolygon;
        }
        else if (currentStage >= 1 && currentStage <= 6) {
            // 阶段1-6：显示每条边的处理结果
            // 当前不需要获取currentPoly，因为我们要分别绘制保留和摒弃的部分
        }
    }

    // 绘制
    // 绘制
    glClear(GL_COLOR_BUFFER_BIT);
    drawClipWindow();

    if (algorithmMode == 0) {
        // SH算法：正常绘制
        drawPolygon(currentPoly, 1.0f, 0.0f, 0.0f, isFinalStage);
    }
    else {
        // WA算法：特殊绘制
        int edgeIndex = currentStage - 1;

        EdgeClipResult edgeResult = WeilerAtherton::getEdgeProcessingResult(
            originalPolygon, clipWindow, edgeIndex);

        // 1. 绘制所有已摒弃的线段（累积的红色）
        if (!edgeResult.allDiscardedSegments.empty()) {
            glColor3f(1.0f, 0.0f, 0.0f); // 红色
            glLineWidth(2.0f);
            glBegin(GL_LINES);
            for (size_t i = 0; i < edgeResult.allDiscardedSegments.size(); i += 2) {
                if (i + 1 < edgeResult.allDiscardedSegments.size()) {
                    glVertex2f(edgeResult.allDiscardedSegments[i].x,
                        edgeResult.allDiscardedSegments[i].y);
                    glVertex2f(edgeResult.allDiscardedSegments[i + 1].x,
                        edgeResult.allDiscardedSegments[i + 1].y);
                }
            }
            glEnd();
        }

        // 2. 绘制所有多边形内的蓝色线段（累积）
        if (!edgeResult.allKeptSegments.empty()) {
            glColor3f(0.0f, 0.0f, 1.0f); // 蓝色
            glLineWidth(3.0f);
            glBegin(GL_LINES);
            for (size_t i = 0; i < edgeResult.allKeptSegments.size(); i += 2) {
                if (i + 1 < edgeResult.allKeptSegments.size()) {
                    glVertex2f(edgeResult.allKeptSegments[i].x,
                        edgeResult.allKeptSegments[i].y);
                    glVertex2f(edgeResult.allKeptSegments[i + 1].x,
                        edgeResult.allKeptSegments[i + 1].y);
                }
            }
            glEnd();
        }

        // 3. 绘制所有窗口边界上的蓝色线段（累积）
        if (!edgeResult.allWindowSegments.empty()) {
            glColor3f(0.0f, 0.0f, 1.0f); // 蓝色
            glLineWidth(3.0f);
            glLineStipple(1, 0xF0F0); // 点划线
            glEnable(GL_LINE_STIPPLE);
            glBegin(GL_LINES);
            for (size_t i = 0; i < edgeResult.allWindowSegments.size(); i += 2) {
                if (i + 1 < edgeResult.allWindowSegments.size()) {
                    glVertex2f(edgeResult.allWindowSegments[i].x,
                        edgeResult.allWindowSegments[i].y);
                    glVertex2f(edgeResult.allWindowSegments[i + 1].x,
                        edgeResult.allWindowSegments[i + 1].y);
                }
            }
            glEnd();
            glDisable(GL_LINE_STIPPLE);
        }

        // 4. 如果是第6步，填充最终多边形
        if (currentStage == 6 && !edgeResult.finalPolygonVertices.empty()) {
            // 填充多边形
            glColor4f(0.0f, 0.0f, 1.0f, 0.3f); // 半透明蓝色填充
            glBegin(GL_POLYGON);
            for (const auto& v : edgeResult.finalPolygonVertices) {
                glVertex2f(v.x, v.y);
            }
            glEnd();

            // 绘制填充区域的边框
            glColor3f(0.0f, 0.0f, 1.0f);
            glLineWidth(4.0f);
            glBegin(GL_LINE_LOOP);
            for (const auto& v : edgeResult.finalPolygonVertices) {
                glVertex2f(v.x, v.y);
            }
            glEnd();
        }

        // 5. 绘制所有交点（累积的）
        if (!edgeResult.allIntersections.empty()) {
            // 绘制橙色交点
            glColor3f(1.0f, 0.5f, 0.0f);
            glPointSize(12.0f);
            glBegin(GL_POINTS);
            for (const auto& inter : edgeResult.allIntersections) {
                glVertex2f(inter.x, inter.y);
            }
            glEnd();

            // 标记进点和出点
            for (const auto& inter : edgeResult.allIntersections) {
                if (inter.entry) {
                    glColor3f(0.0f, 1.0f, 0.0f); // 绿色"Entry"
                    glRasterPos2f(inter.x + 8, inter.y + 8);
                    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'E');
                }
                else {
                    glColor3f(1.0f, 0.0f, 0.0f); // 红色"Exit"
                    glRasterPos2f(inter.x + 8, inter.y + 8);
                    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'X');
                }
            }
        }
    }

    // 显示信息
    glColor3f(0.0f, 0.0f, 0.0f);

    // 算法名称和阶段
    glRasterPos2f(50, 850);
    char info[100];
    sprintf_s(info, sizeof(info), "%s - 阶段 %d/%d",
        algorithmName.c_str(), currentStage, totalStages - 1);
    for (int i = 0; info[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, info[i]);
    }

    // 顶点数
    glRasterPos2f(50, 820);
    sprintf_s(info, sizeof(info), "当前顶点数: %d", (int)currentPoly.size());
    for (int i = 0; info[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, info[i]);
    }

    // 阶段描述
    glRasterPos2f(50, 790);
    if (algorithmMode == 0) { // SH算法
        const char* stageDesc[5] = {
            "原始多边形",
            "左边界裁剪后",
            "右边界裁剪后",
            "下边界裁剪后",
            "上边界裁剪后（最终结果）"
        };
        if (currentStage < 5) {
            for (int i = 0; stageDesc[currentStage][i] != '\0'; i++) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, stageDesc[currentStage][i]);
            }
        }
    }
    else { // WA算法
        const char* stageDesc[7] = {
        "原始多边形",
        "处理边 A-B",
        "处理边 B-C ",
        "处理边 C-D",
        "处理边 D-E",
        "处理边 E-F",
        "处理边 F-A（完成）"
        };
        if (currentStage < 7) {
            for (int i = 0; stageDesc[currentStage][i] != '\0'; i++) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, stageDesc[currentStage][i]);
            }
        }
    }

    // 如果是最終阶段，显示"填充显示"提示
    if (isFinalStage) {
        glColor3f(0.0f, 0.5f, 0.0f); // 绿色提示
        glRasterPos2f(50, 760);
        const char* finalHint = "★ 最终结果（已填充颜色） ★";
        for (int i = 0; finalHint[i] != '\0'; i++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, finalHint[i]);
        }
    }

    // 控制提示
    glColor3f(0.4f, 0.4f, 0.4f);
    glRasterPos2f(50, 50);
    const char* hint = "空格:下一步  A:切换算法  R:重置  W:显示/隐藏窗口  ESC:退出";
    for (int i = 0; hint[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, hint[i]);
    }

    glFlush();
}

// 键盘响应
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case ' ': // 空格键：下一步
        if (algorithmMode == 0) { // SH算法
            int totalStages = SutherlandHodgeman::getTotalStages();
            if (currentStage < totalStages - 1) {
                currentStage++;
                cout << "SH算法 - 阶段 " << currentStage << "/" << totalStages - 1 << endl;
            }
            else {
                cout << "SH算法已完成所有步骤" << endl;
            }
        }
        else { // WA算法
            int totalStages = WeilerAtherton::getTotalStages(); // 7
            currentStage = (currentStage + 1) % totalStages;
            cout << "WA算法 - 处理边 " << currentStage << "/" << totalStages - 1;
            if (currentStage >= 1 && currentStage <= 6) {
                // 显示正在处理哪条边
                char edgeNames[6][2] = { "A", "B", "C", "D", "E", "F" };
                cout << " (" << edgeNames[currentStage - 1] << "-"
                    << edgeNames[currentStage % 6] << "边)";
            }
            cout << endl;
        }
        glutPostRedisplay();
        break;

    case 'a': case 'A': // 切换算法
        algorithmMode = 1 - algorithmMode;
        currentStage = 0;
        glutPostRedisplay();
        break;

    case 'r': case 'R': // 重置
        currentStage = 0;
        glutPostRedisplay();
        break;

    case 'w': case 'W': // 显示/隐藏窗口
        showWindow = !showWindow;
        glutPostRedisplay();
        break;

    case 27: // ESC退出
        exit(0);
        break;
    }
}

// 添加窗口焦点回调函数
void windowFocus(int state) {
    if (state == GLUT_ENTERED) {
        // 窗口获得焦点
        cout << "窗口获得焦点" << endl;
    }
    else {
        // 窗口失去焦点
        cout << "窗口失去焦点" << endl;
    }
}

// 窗口初始化
void init() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(-100, winWidth + 100, -100, winHeight + 100);
}

// 窗口大小变化
void reshape(int w, int h) {
    winWidth = w;
    winHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-100, w + 100, -100, h + 100);
}

int main(int argc, char** argv) {
    cout << "=== 多边形裁剪算法演示 ===" << endl;
    cout << "重要提示：请先点击OpenGL窗口（图形窗口）使其获得焦点！" << endl;
    cout << "然后才能使用键盘控制。" << endl;
    cout << endl;
    cout << "控制说明：" << endl;
    cout << "  空格键 - 下一步" << endl;
    cout << "  A键   - 切换算法 (SH/WA)" << endl;
    cout << "  R键   - 重置到开始" << endl;
    cout << "  W键   - 显示/隐藏裁剪窗口" << endl;
    cout << "  ESC键 - 退出" << endl;
    cout << endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(winWidth, winHeight);
    glutInitWindowPosition(100, 50);

    // 创建一个醒目的窗口标题
    glutCreateWindow("多边形裁剪算法演示 - [请点击此窗口] - 按空格键开始");

    // 注册回调函数
    glutDisplayFunc(displayCurrentStage);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    // 添加鼠标事件处理，确保窗口可以获得焦点
    glutMouseFunc([](int button, int state, int x, int y) {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
            cout << "窗口已获得焦点，现在可以使用键盘控制！" << endl;
        }
        });

    init();

    // 添加初始提示
    cout << "==========================================" << endl;
    cout << "如果按键没有反应：" << endl;
    cout << "1. 请点击上面的OpenGL图形窗口" << endl;
    cout << "2. 确保该窗口的标题栏是蓝色的（表示获得焦点）" << endl;
    cout << "3. 然后再按空格键等按键" << endl;
    cout << "==========================================" << endl;

    glutMainLoop();
    return 0;
}