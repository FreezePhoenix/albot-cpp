#pragma once

#ifndef ALBOT_MOVEMENTMATH_HPP_
#define ALBOT_MOVEMENTMATH_HPP_

#ifndef INCLUDE_NLOHMANN_JSON_HPP_
#include <nlohmann/json.hpp>
#endif

class MovementMath {
public:
    static void stopLogic(nlohmann::json& entity) {
        if (entity["moving"] == false)
            return;
        double x = entity["x"];
        double y = entity["y"];
        double goingX = entity["going_x"];
        double goingY = entity["going_y"];
        double fromX = entity["from_x"];
        double fromY = entity["from_y"];

        if (((fromX <= goingX && x >= goingX - 0.1) || (fromX >= goingX && x <= goingX + 0.1)) && ((fromY <= goingY && y >= goingY - 0.1) || (fromY >= goingY && y <= goingY + 0.1))) {
            entity["x"] = goingX;
            entity["y"] = goingY;

            entity["moving"] = entity.value("amoving", false);

        }
    }

    static void moveEntity(nlohmann::json& entity, double cDelta) {
    
        if (entity.value("moving", false) == true) {
            assert(entity.contains("vx"));
            entity["x"] = double(entity["x"]) + double(entity["vx"]) * std::min(cDelta, 50.0) / 1000.0;
            entity["y"] = double(entity["y"]) + double(entity["vy"]) * std::min(cDelta, 50.0) / 1000.0;
        }
    }
    static double getDouble(const nlohmann::json& value) {
        return double(value);
    }
    static std::pair<double, double> calculateVelocity(const nlohmann::json& entity) {
        
        double ref = std::sqrt(0.0001 +
                             std::pow(double(entity["going_x"]) - double(entity["from_x"]), 2) +
                             std::pow(double(entity["going_y"]) - double(entity["from_y"]), 2));
        auto speed = double(entity["speed"]);
        double vx = speed * (double(entity["going_x"]) - double(entity["from_x"])) / ref;
        double vy = speed * (double(entity["going_y"]) - double(entity["from_y"])) / ref;
        
        return std::make_pair(vx, vy);
    }

    static double pythagoras(double x1, double y1, double x2, double y2) {
        return std::sqrt(
                std::pow(x1 - x2, 2) +
                std::pow(y1 - y2, 2));
    }

    static double manhatten(const std::pair<int, int>& A, const std::pair<int, int>& B) {
        return std::abs(A.first - B.first) + std::abs(A.second - B.second);
    }
};

#endif /* ALBOT_MOVEMENTMATH_HPP_ */