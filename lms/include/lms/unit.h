#ifndef LMS_UNIT_H
#define LMS_UNIT_H

#include <iostream>
#include <string>
#include <map>
#include "config.h"

namespace lms {

/**
 * @brief Parses a unit string to a float as SI unit
 * @param str string to parse
 * @param units map of all units and their conversion factor to the SI unit
 * @return SI value
 */
float parseUnitHelper(std::string const& str, std::map<std::string, float> const& units);

/**
 * @brief Wrapper type of a float denoting a distance.
 */
class Distance {
public:
    Distance();
    Distance(float meters);
    float toSI() const;
private:
    float meters;
};

/**
 * @brief Wrapper type of a float denoting an angle.
 */
class Angle {
public:
    Angle();
    Angle(float rad);
    float toRad() const;
    float toDeg() const;
private:
    float rad;
};

/**
 * @brief Specialization for distance used in lms::Config parsing
 */
//template<>
//Distance Config::get<Distance>(const std::string &key, const Distance &defaultValue) const;

/**
 * @brief Specialization for angles used in lms::Config parsing
 */
//template<>
//Angle Config::get<Angle>(const std::string &key, const Angle &defaultValue) const;

}  // namespace lms

#endif // LMS_UNIT_H
