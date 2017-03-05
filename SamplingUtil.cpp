#include "RTRGeometry.h"
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

int SamplingUtil::generateInteger(int a, int b)
{
    return std::uniform_int_distribution<int>(a, b)(generator);
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

RTRVector3D SamplingUtil::generateRandomPointInTriangle(const RTRTriangle3D &triangle)
{
    auto AB = triangle.vertices[1] - triangle.vertices[0];
    auto AC = triangle.vertices[2] - triangle.vertices[0];
    double a, b;
    do {
        a = generateRandomNumber();
        b = generateRandomNumber();
    } while (a + b > 1.0);
    return triangle.vertices[0] + AB * a + AC * b;
}
