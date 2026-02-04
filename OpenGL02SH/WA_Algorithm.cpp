// WA_Algorithm.cpp - 修改后的版本
#include "WA_Algorithm.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <map>
#include <set>

using namespace std;

// 静态变量定义
vector<Point> WeilerAtherton::g_allKeptSegments;
vector<Point> WeilerAtherton::g_allDiscardedSegments;
vector<Point> WeilerAtherton::g_allWindowSegments;
vector<Point> WeilerAtherton::g_allIntersections;
vector<Point> WeilerAtherton::g_finalPolygon;

// 判断点是否在窗口内
bool WeilerAtherton::isPointInsideWindow(const Point& p) {
    return p.x >= WIN_LEFT && p.x <= WIN_RIGHT &&
        p.y >= WIN_BOTTOM && p.y <= WIN_TOP;
}

// 查找线段交点
bool WeilerAtherton::findLineIntersection(const Point& p1, const Point& p2,
    const Point& w1, const Point& w2,
    Point& intersection, float& tPoly, float& tWindow) {
    float denom = (w2.y - w1.y) * (p2.x - p1.x) - (w2.x - w1.x) * (p2.y - p1.y);

    if (fabs(denom) < 1e-6) return false;

    float ua = ((w2.x - w1.x) * (p1.y - w1.y) - (w2.y - w1.y) * (p1.x - w1.x)) / denom;
    float ub = ((p2.x - p1.x) * (p1.y - w1.y) - (p2.y - p1.y) * (p1.x - w1.x)) / denom;

    if (ua < 0.0f || ua > 1.0f || ub < 0.0f || ub > 1.0f) return false;

    intersection.x = p1.x + ua * (p2.x - p1.x);
    intersection.y = p1.y + ua * (p2.y - p1.y);
    intersection.intersect = true;

    tPoly = ua;
    tWindow = ub;

    return true;
}

// 查找所有交点
vector<IntersectionInfo> WeilerAtherton::findAllIntersections(
    const vector<Point>& polygon, const vector<Point>& window) {

    vector<IntersectionInfo> intersections;
    int polySize = polygon.size();
    int windowSize = window.size();

    // 遍历多边形的每条边
    for (int i = 0; i < polySize; i++) {
        Point p1 = polygon[i];
        Point p2 = polygon[(i + 1) % polySize];

        // 遍历窗口的每条边
        for (int j = 0; j < windowSize; j++) {
            Point w1 = window[j];
            Point w2 = window[(j + 1) % windowSize];

            Point intersection;
            float tPoly, tWindow;

            if (findLineIntersection(p1, p2, w1, w2, intersection, tPoly, tWindow)) {
                // 判断是进点还是出点
                Point midPoint;
                if (tPoly < 0.5f) {
                    midPoint.x = p1.x + (p2.x - p1.x) * (tPoly + 0.01f);
                    midPoint.y = p1.y + (p2.y - p1.y) * (tPoly + 0.01f);
                }
                else {
                    midPoint.x = p1.x + (p2.x - p1.x) * (tPoly - 0.01f);
                    midPoint.y = p1.y + (p2.y - p1.y) * (tPoly - 0.01f);
                }

                bool isEnter = !isPointInsideWindow(p1) && isPointInsideWindow(midPoint);

                intersections.emplace_back(intersection, isEnter, tPoly, i, j);
            }
        }
    }

    // 按多边形边和参数值排序
    sort(intersections.begin(), intersections.end(),
        [](const IntersectionInfo& a, const IntersectionInfo& b) {
            if (a.polygonEdge != b.polygonEdge) return a.polygonEdge < b.polygonEdge;
            return a.t < b.t;
        });

    return intersections;
}

// 创建链表
WA_Node* WeilerAtherton::createLinkedList(const vector<Point>& points,
    const vector<IntersectionInfo>& intersections) {
    if (points.empty()) return nullptr;

    WA_Node* head = nullptr;
    WA_Node* tail = nullptr;
    WA_Node* prev = nullptr;

    int currentEdge = 0;
    vector<IntersectionInfo> edgeIntersections;

    for (int i = 0; i <= points.size(); i++) {
        Point currentPoint = points[i % points.size()];

        // 收集当前边的交点
        edgeIntersections.clear();
        for (const auto& inter : intersections) {
            if (inter.polygonEdge == currentEdge) {
                edgeIntersections.push_back(inter);
            }
        }

        // 按参数值排序
        sort(edgeIntersections.begin(), edgeIntersections.end());

        // 创建当前点的节点
        WA_Node* node = new WA_Node(currentPoint);
        if (!head) head = node;
        if (prev) {
            prev->next = node;
            node->prev = prev;
        }
        prev = node;

        // 插入交点
        for (const auto& inter : edgeIntersections) {
            WA_Node* interNode = new WA_Node(inter.point);
            interNode->point.intersect = true;
            interNode->point.entry = inter.isEntry;

            prev->next = interNode;
            interNode->prev = prev;
            prev = interNode;
        }

        if (i == points.size()) {
            tail = node;
        }
        currentEdge++;
    }

    // 闭合链表
    if (tail && head) {
        tail->next = head;
        head->prev = tail;
    }

    return head;
}

// 标记进点和出点
void WeilerAtherton::markEntryExitPoints(WA_Node* polyList, WA_Node* windowList) {
    if (!polyList || !windowList) return;

    // 创建交点映射
    map<pair<float, float>, WA_Node*> polyIntersections;
    map<pair<float, float>, WA_Node*> windowIntersections;

    // 收集多边形链表中的交点
    WA_Node* current = polyList;
    do {
        if (current->point.intersect) {
            auto key = make_pair(current->point.x, current->point.y);
            polyIntersections[key] = current;
        }
        current = current->next;
    } while (current != polyList);

    // 收集窗口链表中的交点
    current = windowList;
    do {
        if (current->point.intersect) {
            auto key = make_pair(current->point.x, current->point.y);
            windowIntersections[key] = current;
        }
        current = current->next;
    } while (current != windowList);

    // 建立双向链接
    for (auto& polyPair : polyIntersections) {
        auto windowIt = windowIntersections.find(polyPair.first);
        if (windowIt != windowIntersections.end()) {
            polyPair.second->neighbour = windowIt->second;
            windowIt->second->neighbour = polyPair.second;
        }
    }
}

// 遍历多边形生成裁剪结果
vector<Point> WeilerAtherton::traversePolygon(WA_Node* startNode) {
    vector<Point> result;
    if (!startNode) return result;

    set<WA_Node*> visited;
    WA_Node* current = startNode;

    do {
        // 添加当前点到结果
        result.push_back(current->point);
        visited.insert(current);

        // 如果当前点是交点且是出点，跳转到窗口链表
        if (current->point.intersect && !current->point.entry && current->neighbour) {
            WA_Node* neighbour = current->neighbour;

            // 沿着窗口链表前进，直到找到下一个进点
            WA_Node* windowNode = neighbour;
            do {
                if (windowNode->point.intersect && windowNode->point.entry) {
                    current = windowNode->neighbour;
                    break;
                }
                windowNode = windowNode->next;
            } while (windowNode != neighbour);
        }
        else {
            current = current->next;
        }

        // 防止无限循环
        if (result.size() > 100) break;

    } while (current != startNode && visited.find(current) == visited.end());

    return result;
}

// 重置累积数据
void WeilerAtherton::resetAccumulatedData() {
    g_allKeptSegments.clear();
    g_allDiscardedSegments.clear();
    g_allWindowSegments.clear();
    g_allIntersections.clear();
    g_finalPolygon.clear();
}

// 获取指定边的处理结果
EdgeClipResult WeilerAtherton::getEdgeProcessingResult(
    const vector<Point>& subjectPolygon,
    const vector<Point>& clipWindow,
    int edgeIndex) {

    EdgeClipResult result;

    if (edgeIndex < 0 || edgeIndex >= 6) return result;

    // 如果是第一条边，重置累积数据
    if (edgeIndex == 0) {
        resetAccumulatedData();
    }

    Point p1 = subjectPolygon[edgeIndex];
    Point p2 = subjectPolygon[(edgeIndex + 1) % 6];

    // 保存原始边
    result.originalEdge.push_back(p1);
    result.originalEdge.push_back(p2);

    bool p1Inside = isPointInsideWindow(p1);
    bool p2Inside = isPointInsideWindow(p2);

    // 检查与窗口每条边的交点
    vector<Point> windowPoints = {
        Point(WIN_LEFT, WIN_BOTTOM),
        Point(WIN_RIGHT, WIN_BOTTOM),
        Point(WIN_RIGHT, WIN_TOP),
        Point(WIN_LEFT, WIN_TOP)
    };

    for (int i = 0; i < 4; i++) {
        Point w1 = windowPoints[i];
        Point w2 = windowPoints[(i + 1) % 4];

        Point intersection;
        float tPoly, tWindow;

        if (findLineIntersection(p1, p2, w1, w2, intersection, tPoly, tWindow)) {
            intersection.edge = i + 1;

            // 判断进点和出点
            if (!p1Inside && p2Inside) {
                intersection.entry = true;
                result.entryPoints.push_back(intersection);
            }
            else if (p1Inside && !p2Inside) {
                intersection.entry = false;
                result.exitPoints.push_back(intersection);
            }

            result.intersections.push_back(intersection);
            g_allIntersections.push_back(intersection);
        }
    }

    // 累积交点
    result.allIntersections = g_allIntersections;

    // 处理线段
    if (!result.intersections.empty()) {
        // 有交点的情况
        sort(result.intersections.begin(), result.intersections.end(),
            [&p1](const Point& a, const Point& b) {
                float da = sqrt(pow(a.x - p1.x, 2) + pow(a.y - p1.y, 2));
                float db = sqrt(pow(b.x - p1.x, 2) + pow(b.y - p1.y, 2));
                return da < db;
            });

        if (p1Inside) {
            // 从可见到不可见
            if (result.intersections.size() > 0) {
                Point exitPoint = result.intersections[0];

                // p1到出点：蓝色
                result.allKeptSegments.push_back(p1);
                result.allKeptSegments.push_back(exitPoint);
                g_allKeptSegments.push_back(p1);
                g_allKeptSegments.push_back(exitPoint);

                // 出点到p2：红色
                result.allDiscardedSegments.push_back(exitPoint);
                result.allDiscardedSegments.push_back(p2);
                g_allDiscardedSegments.push_back(exitPoint);
                g_allDiscardedSegments.push_back(p2);
            }
        }
        else {
            // 从不可见到可见
            if (result.intersections.size() > 0) {
                Point entryPoint = result.intersections[0];

                // p1到进点：红色
                result.allDiscardedSegments.push_back(p1);
                result.allDiscardedSegments.push_back(entryPoint);
                g_allDiscardedSegments.push_back(p1);
                g_allDiscardedSegments.push_back(entryPoint);

                // 进点到p2：蓝色
                result.allKeptSegments.push_back(entryPoint);
                result.allKeptSegments.push_back(p2);
                g_allKeptSegments.push_back(entryPoint);
                g_allKeptSegments.push_back(p2);
            }
        }
    }
    else {
        // 没有交点
        if (p1Inside && p2Inside) {
            // 全部蓝色
            result.allKeptSegments.push_back(p1);
            result.allKeptSegments.push_back(p2);
            g_allKeptSegments.push_back(p1);
            g_allKeptSegments.push_back(p2);
        }
        else {
            // 全部红色
            result.allDiscardedSegments.push_back(p1);
            result.allDiscardedSegments.push_back(p2);
            g_allDiscardedSegments.push_back(p1);
            g_allDiscardedSegments.push_back(p2);
        }
    }

    // 设置累积结果
    result.allKeptSegments = g_allKeptSegments;
    result.allDiscardedSegments = g_allDiscardedSegments;

    // 如果是最后一条边，执行完整的WA算法
    if (edgeIndex == 5) {
        vector<Point> clippedPolygon = clip(subjectPolygon, clipWindow);
        g_finalPolygon = clippedPolygon;
        result.finalPolygonVertices = clippedPolygon;
    }

    return result;
}

// 完整的WA算法
vector<Point> WeilerAtherton::clip(const vector<Point>& polygon,
    const vector<Point>& window) {

    // 步骤1：找到所有交点
    vector<IntersectionInfo> intersections = findAllIntersections(polygon, window);

    // 步骤2：构建多边形链表（插入交点）
    WA_Node* polyList = createLinkedList(polygon, intersections);

    // 步骤3：构建窗口链表（插入交点）
    WA_Node* windowList = createLinkedList(window, intersections);

    // 步骤4：标记进点和出点，建立双向链接
    markEntryExitPoints(polyList, windowList);

    // 步骤5：找到第一个进点作为起点
    WA_Node* startNode = nullptr;
    WA_Node* current = polyList;
    do {
        if (current->point.intersect && current->point.entry) {
            startNode = current;
            break;
        }
        current = current->next;
    } while (current != polyList);

    // 步骤6：如果没有进点，检查多边形是否完全在窗口内或外
    if (!startNode) {
        if (isPointInsideWindow(polygon[0])) {
            return polygon; // 完全在窗口内
        }
        else {
            return vector<Point>(); // 完全在窗口外
        }
    }

    // 步骤7：遍历链表生成裁剪多边形
    vector<Point> result = traversePolygon(startNode);

    // 清理链表内存
    // TODO: 添加链表清理代码

    return result;
}