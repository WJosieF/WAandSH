#pragma once
#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <cmath>

// 点结构体
struct Point {
    float x, y;
    bool intersect; // 是否为交点
    bool entry; // 进入点(Weiler-Atherton算法用)
    int edge; // 交点所在的窗口边(1:左,2:右,3:下,4:上)
    bool visited; // 追踪时是否访问过

    Point(float _x = 0, float _y = 0, bool _inter = false)
        : x(_x), y(_y), intersect(_inter), entry(false), edge(0) {
    }

    // 添加相等运算符重载
    bool operator==(const Point& other) const {
        const float EPSILON = 1e-3f; // 容差
        return fabs(x - other.x) < EPSILON &&
            fabs(y - other.y) < EPSILON;
    }

    // 添加不相等运算符重载
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
};

// 链表节点结构 - WA算法的核心数据结构
struct WA_Node {
    Point point;                 // 顶点信息
    WA_Node* next;              // 在当前多边形链表中的下一个节点
    WA_Node* neighbour;         // 指向另一个多边形链表中相同交点的节点（双向指针）
    WA_Node* prev;              // 前驱节点，方便插入和调试

    WA_Node(const Point& p)
        : point(p), next(nullptr), neighbour(nullptr), prev(nullptr) {
    }
};

// 窗口边界定义（坐标乘以100）
const float WIN_LEFT = 400.0f;    // 4 * 100
const float WIN_RIGHT = 800.0f;   // 8 * 100
const float WIN_BOTTOM = 100.0f;  // 1 * 100
const float WIN_TOP = 600.0f;     // 6 * 100

// 原始多边形顶点（坐标乘以100，顺时针顺序）
extern std::vector<Point> originalPolygon;

// 裁剪窗口顶点
extern std::vector<Point> clipWindow;

#endif
