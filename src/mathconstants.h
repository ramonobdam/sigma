#ifndef MATHCONSTANTS_H
#define MATHCONSTANTS_H

#include <limits>

// Namespace with mathematical constants
namespace MathConstants {
    constexpr double maxInfinity { std::numeric_limits<double>::infinity() };
    constexpr double minInfinity { -std::numeric_limits<double>::infinity() };
    constexpr double nan { std::numeric_limits<double>::quiet_NaN() };
    constexpr double pi { 3.14159265358979323846264338327950288419716939937510 };
    constexpr int maxInt { std::numeric_limits<int>::max() };
}

#endif // MATHCONSTANTS_H
