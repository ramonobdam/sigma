#ifndef MATHCONSTANTS_H
#define MATHCONSTANTS_H

#include <limits>

// Namespace with mathematical constants
namespace MathConstants {
    constexpr double pi { 3.14159265358979323846264338327950288419716939937510 };
    constexpr double nan { std::numeric_limits<double>::quiet_NaN() };
    constexpr int maxInt { std::numeric_limits<int>::max() };
}

#endif // MATHCONSTANTS_H
