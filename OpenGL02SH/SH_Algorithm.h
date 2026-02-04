#pragma once
#ifndef SH_ALGORITHM_H
#define SH_ALGORITHM_H

#include <vector>
#include "Common.h"

class SutherlandHodgeman {
private:
    static bool isInsideWindow(const Point& p, int edge);
    static Point computeIntersection(const Point& p1, const Point& p2, int edge);

public:
    // 单边裁剪
    static std::vector<Point> clipEdge(const std::vector<Point>& inputPolygon, int edge);

    // 获取指定阶段的结果
    static std::vector<Point> getStageResult(const std::vector<Point>& originalPolygon, int stage);

    // 获取总阶段数
    static int getTotalStages() { return 5; } // 原始 + 4个边界
};

#endif