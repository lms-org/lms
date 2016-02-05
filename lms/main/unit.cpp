#include "lms/unit.h"
#include <string>
#include <cstdlib>
#include <math.h>

namespace lms {

float parseUnitHelper(std::string const& str, std::map<std::string, float> const& units) {
    float result = 0;

    for(auto const& part : lms::extra::split(str, ' ')) {
        size_t index = part.find_first_not_of("0123456789.-");
        if(index == std::string::npos) {
            throw std::runtime_error("Can not parse " + str);
        }

        std::string unit = part.substr(index);
        float value = ::atof(part.c_str());
        std::map<std::string, float>::const_iterator it = units.find(unit);

        if(it == units.end()) {
            throw std::runtime_error("Unknown unit in " + str);
        }

        result += value * it->second;
    }

    return result;
}

Distance::Distance() : meters(0) {}

Distance::Distance(float meters) : meters(meters) {}

float Distance::toSI() const {
    return meters;
}

Angle::Angle() : rad(0) {}
Angle::Angle(float rad) : rad(rad) {}
float Angle::toRad() const {
    return rad;
}
float Angle::toDeg() const {
    return rad * 180 / M_PI;
}

template<>
bool parse<Distance>(const std::string &src, Distance &dst) {
    const std::map<std::string, float> units {
        {"m", 1.f},
        {"cm", 0.01f},
        {"mm", 0.001f}
    };
    dst = Distance(parseUnitHelper(src, units));
    return true;
}

template<>
bool parse<Angle>(const std::string &src, Angle &dst) {
    const std::map<std::string, float> units {
        {"rad", 1.f},
        {"deg", M_PI / 180}
    };
    dst = Angle(parseUnitHelper(src, units));
    return true;
}

}  // namespace lms
