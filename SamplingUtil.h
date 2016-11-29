#ifndef SAMPLINGUTIL_H
#define SAMPLINGUTIL_H

#include <random>
#include "RTRVector3D.h"

class SamplingUtil
{
public:
    SamplingUtil();
    std::random_device randomDevice;
    std::seed_seq seed{randomDevice(), randomDevice(), randomDevice(),
        randomDevice(), randomDevice(), randomDevice(), randomDevice(),
        randomDevice()};
    std::mt19937 generator;
    std::uniform_real_distribution<> distributionZeroOne;
    std::uniform_real_distribution<> distributionMinusOneOne;
    void init();
    double generateRandomNumber();
    double generateRandomNumber(double a, double b);
    RTRVector3D generateRandomDirection();
};

#endif // SAMPLINGUTIL_H
