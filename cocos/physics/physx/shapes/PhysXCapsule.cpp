
#include "PhysXShape.h"
#include "PhysXCapsule.h"
#include "../PhysXUtils.h"
#include "../PhysXWorld.h"

namespace cc {
namespace physics {

PhysXCapsule::PhysXCapsule() : mRadius(0.5f),
                               mCylinderHeight(1.0f),
                               mDirection(EAxisDirection::Y_AXIS),
                               PhysXShape(){};

void PhysXCapsule::setRadius(float r) {
    mRadius = r;
    updateGeometry();
    getShape().setGeometry(getPxGeometry<PxCapsuleGeometry>());
}

void PhysXCapsule::setCylinderHeight(float v) {
    mCylinderHeight = v;
    updateGeometry();
    getShape().setGeometry(getPxGeometry<PxCapsuleGeometry>());
}

void PhysXCapsule::setDirection(EAxisDirection v) {
    mDirection = v;
    updateGeometry();
    getShape().setGeometry(getPxGeometry<PxCapsuleGeometry>());
}

void PhysXCapsule::onComponentSet() {
    updateGeometry();
    mShape = PxGetPhysics().createShape(getPxGeometry<PxCapsuleGeometry>(), getDefaultMaterial(), true);
}

void PhysXCapsule::updateScale() {
    updateGeometry();
    getShape().setGeometry(getPxGeometry<PxCapsuleGeometry>());
    updateCenter();
}

void PhysXCapsule::updateGeometry() {
    auto &node = getSharedBody().getNode();
    auto &geo = getPxGeometry<PxCapsuleGeometry>();
    float rs = 1.f, hs = 1.f;
    switch (mDirection) {
        case EAxisDirection::X_AXIS:
            hs = PxAbs(node.worldScale.x);
            rs = PxAbsMax(node.worldScale.y, node.worldScale.z);
            mRotation = PxQuat{PxIdentity};
            break;
        case EAxisDirection::Z_AXIS:
            hs = PxAbs(node.worldScale.z);
            rs = PxAbsMax(node.worldScale.x, node.worldScale.y);
            mRotation = PxQuat(PxPiDivTwo, PxVec3{0.f, 1.f, 0.f});
            break;
        case EAxisDirection::Y_AXIS:
        default:
            hs = PxAbs(node.worldScale.y);
            rs = PxAbsMax(node.worldScale.x, node.worldScale.z);
            mRotation = PxQuat(PxPiDivTwo, PxVec3{0.f, 0.f, 1.f});
            break;
    }
    geo.radius = PxMax(PxAbs(mRadius * rs), PX_NORMALIZATION_EPSILON);
    geo.halfHeight = PxMax(PxAbs(mCylinderHeight / 2 * hs), PX_NORMALIZATION_EPSILON);
}

} // namespace physics
} // namespace cc