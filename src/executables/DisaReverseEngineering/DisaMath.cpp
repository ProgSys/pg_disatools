#include "DisaMath.h"

#include <cmath>

#include <FunctionPatcher.h>

void FUN_004e32a0_rotateXZ(float angleDegrees, float x, float z) {
    if (angleDegrees < 0.f) angleDegrees = static_cast<float>(angleDegrees + 360.0);
    if (angleDegrees >= 360.f) angleDegrees = static_cast<float>(angleDegrees - 360.0);

    //the game's factor is 0.017453 as a float, slightly below pi / 180
    const float radians = static_cast<float>(angleDegrees * double(0.017453f));
    const float cosine = static_cast<float>(std::cos(double(radians)));
    const float sine = static_cast<float>(std::sin(double(radians)));

    FP::getData<float>(0x006b791c) = static_cast<float>(double(x) * cosine - double(z) * sine);
    FP::getData<float>(0x006b7a60) = static_cast<float>(double(z) * cosine + double(x) * sine);
}

int __fastcall FUN_0046e410_sideOfLine(const glm::vec3* begin, const glm::vec3* end, const glm::vec3* point) {
    const float lineX = end->x - begin->x;
    const float lineZ = end->z - begin->z;
    const float toPointX = point->x - begin->x;
    const float toPointZ = point->z - begin->z;

    const float crossA = lineX * toPointZ;
    const float crossB = lineZ * toPointX;
    if (crossA > crossB) return 1;
    if (crossA < crossB) return -1;

    if (double(toPointX) * lineX < 0.0 || double(toPointZ) * lineZ < 0.0) return -1;
    const double lineLengthSquared = double(lineZ) * lineZ + double(lineX) * lineX;
    const double pointLengthSquared = double(toPointX) * toPointX + double(toPointZ) * toPointZ;
    return pointLengthSquared > lineLengthSquared ? 1 : 0;
}
