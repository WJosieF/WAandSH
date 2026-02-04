// WA_Algorithm.h - 修改后的版本
#ifndef WA_ALGORITHM_H
#define WA_ALGORITHM_H

#include <vector>
#include "Common.h"

// 交点信息（增强版）
struct IntersectionInfo {
    Point point;
    bool isEntry;           // 是否为进入点
    float t;               // 在边上的参数值
    int polygonEdge;       // 多边形边的索引
    int windowEdge;        // 窗口边的索引
    WA_Node* polyNode;     // 指向多边形链表中的节点
    WA_Node* windowNode;   // 指向窗口链表中的节点

    IntersectionInfo(const Point& p, bool entry, float param, int polyEdge, int winEdge)
        : point(p), isEntry(entry), t(param), polygonEdge(polyEdge), windowEdge(winEdge),
        polyNode(nullptr), windowNode(nullptr) {
    }

    bool operator<(const IntersectionInfo& other) const {
        return t < other.t;
    }
};

// 链表管理类
class WALinkedList {
private:
    WA_Node* head;
    WA_Node* tail;

public:
    WALinkedList() : head(nullptr), tail(nullptr) {}
    ~WALinkedList() { clear(); }

    void clear();
    void insertAfter(WA_Node* node, const Point& point);
    WA_Node* findNode(const Point& point) const;
    WA_Node* getHead() const { return head; }
    WA_Node* getTail() const { return tail; }
};

// 裁剪结果
struct EdgeClipResult {
    // 当前边处理结果
    std::vector<Point> originalEdge;      // 原始边
    std::vector<Point> intersections;     // 当前边的交点
    std::vector<Point> entryPoints;       // 进点
    std::vector<Point> exitPoints;        // 出点

    // 累积结果
    std::vector<Point> allKeptSegments;      // 所有已保留的线段
    std::vector<Point> allDiscardedSegments; // 所有已丢弃的线段
    std::vector<Point> allWindowSegments;    // 所有窗口边界上的线段
    std::vector<Point> allIntersections;     // 所有交点
    std::vector<Point> finalPolygonVertices; // 最终多边形顶点
};

class WeilerAtherton {
private:
    // 私有静态变量
    static std::vector<Point> g_allKeptSegments;
    static std::vector<Point> g_allDiscardedSegments;
    static std::vector<Point> g_allWindowSegments;
    static std::vector<Point> g_allIntersections;
    static std::vector<Point> g_finalPolygon;

    // 核心算法函数
    static bool isPointInsideWindow(const Point& p);
    static bool findLineIntersection(const Point& p1, const Point& p2,
        const Point& w1, const Point& w2,
        Point& intersection, float& tPoly, float& tWindow);
    static std::vector<IntersectionInfo> findAllIntersections(
        const std::vector<Point>& polygon, const std::vector<Point>& window);
    static WA_Node* createLinkedList(const std::vector<Point>& points,
        const std::vector<IntersectionInfo>& intersections);
    static void markEntryExitPoints(WA_Node* polyList, WA_Node* windowList);
    static std::vector<Point> traversePolygon(WA_Node* startNode);

public:
    // 获取指定边的处理结果
    static EdgeClipResult getEdgeProcessingResult(
        const std::vector<Point>& subjectPolygon,
        const std::vector<Point>& clipWindow,
        int edgeIndex);

    // 获取总阶段数
    static int getTotalStages() { return 7; } // 原始 + 6条边

    // 重置累积数据
    static void resetAccumulatedData();

    // 完整的WA算法
    static std::vector<Point> clip(const std::vector<Point>& polygon,
        const std::vector<Point>& window);
};

#endif