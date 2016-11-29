#include "SamplingUtil.h"

SamplingUtil::SamplingUtil()
    :generator(seed), distributionZeroOne(0.0, 1.0),
    distributionMinusOneOne(-1.0, 1.0)
{
}

double SamplingUtil::generateRandomNumber()
{
    return distributionZeroOne(generator);
}

double SamplingUtil::generateRandomNumber(double a, double b)
{
    return std::uniform_real_distribution<>(a, b)(generator);
}

RTRVector3D SamplingUtil::generateRandomDirection()
{
    double x, y, z;
    do {
        x = distributionMinusOneOne(generator);
        y = distributionMinusOneOne(generator);
        z = distributionMinusOneOne(generator);
    } while(x * x + y * y + z * z > 1.0);
    RTRVector3D ret(x, y, z);
    ret.vectorNormalize();
    return ret;
}
