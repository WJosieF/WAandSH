#include "SH_Algorithm.h"
#include <algorithm>
#include <iostream>

bool SutherlandHodgeman::isInsideWindow(const Point& p, int edge) {
    switch (edge) {
    case 1: return p.x >= WIN_LEFT;
    case 2: return p.x <= WIN_RIGHT;
    case 3: return p.y >= WIN_BOTTOM;
    case 4: return p.y <= WIN_TOP;
    }
    return false;
}

Point SutherlandHodgeman::computeIntersection(const Point& p1, const Point& p2, int edge) {
    Point intersection;

    if (fabs(p2.x - p1.x) < 0.001) {
        intersection.x = p1.x;
        switch (edge) {
        case 1: intersection.y = p1.y + (p2.y - p1.y) * (WIN_LEFT - p1.x) / 0.0001; break;
        case 2: intersection.y = p1.y + (p2.y - p1.y) * (WIN_RIGHT - p1.x) / 0.0001; break;
        case 3: intersection.y = WIN_BOTTOM; break;
        case 4: intersection.y = WIN_TOP; break;
        }
    }
    else if (fabs(p2.y - p1.y) < 0.001) {
        intersection.y = p1.y;
        switch (edge) {
        case 1: intersection.x = WIN_LEFT; break;
        case 2: intersection.x = WIN_RIGHT; break;
        case 3: intersection.x = p1.x + (p2.x - p1.x) * (WIN_BOTTOM - p1.y) / 0.0001; break;
        case 4: intersection.x = p1.x + (p2.x - p1.x) * (WIN_TOP - p1.y) / 0.0001; break;
        }
    }
    else {
        float slope = (p2.y - p1.y) / (p2.x - p1.x);
        float intercept = p1.y - slope * p1.x;

        switch (edge) {
        case 1:
            intersection.x = WIN_LEFT;
            intersection.y = slope * WIN_LEFT + intercept;
            break;
        case 2:
            intersection.x = WIN_RIGHT;
            intersection.y = slope * WIN_RIGHT + intercept;
            break;
        case 3:
            intersection.y = WIN_BOTTOM;
            intersection.x = (WIN_BOTTOM - intercept) / slope;
            break;
        case 4:
            intersection.y = WIN_TOP;
            intersection.x = (WIN_TOP - intercept) / slope;
            break;
        }
    }

    intersection.intersect = true;
    intersection.edge = edge;
    return intersection;
}

std::vector<Point> SutherlandHodgeman::clipEdge(const std::vector<Point>& inputPolygon, int edge) {
    std::vector<Point> output;
    if (inputPolygon.empty()) return output;

    int n = inputPolygon.size();
    for (int i = 0; i < n; i++) {
        Point current = inputPolygon[i];
        Point next = inputPolygon[(i + 1) % n];

        bool currentInside = isInsideWindow(current, edge);
        bool nextInside = isInsideWindow(next, edge);

        if (currentInside && nextInside) {
            output.push_back(next);
        }
        else if (currentInside && !nextInside) {
            output.push_back(computeIntersection(current, next, edge));
        }
        else if (!currentInside && !nextInside) {
            // 不输出
        }
        else {
            output.push_back(computeIntersection(current, next, edge));
            output.push_back(next);
        }
    }

    return output;
}

std::vector<Point> SutherlandHodgeman::getStageResult(const std::vector<Point>& originalPolygon, int stage) {
    if (stage == 0) return originalPolygon;

    std::vector<Point> result = originalPolygon;
    for (int i = 1; i <= stage; i++) {
        if (i == 1) result = clipEdge(result, 1); // 左
        else if (i == 2) result = clipEdge(result, 2); // 右
        else if (i == 3) result = clipEdge(result, 3); // 下
        else if (i == 4) result = clipEdge(result, 4); // 上
    }

    return result;
}