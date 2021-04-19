

#include "PhysXCone.h"
#include "../PhysXUtils.h"
#include "../PhysXWorld.h"
#include "PhysXShape.h"
#include "math/Quaternion.h"
#include "renderer/pipeline/helper/SharedMemory.h"
#include <algorithm>

namespace cc {
namespace physics {

PhysXCone::PhysXCone() : mMeshHandle(0),
                         PhysXShape(){};

PhysXCone::~PhysXCone(){};

void PhysXCone::setConvex(intptr_t handle) {
    if (mMeshHandle == handle) return;
    mMeshHandle = handle;
    if (mShape) {
        // TODO: ...
    }
}

void PhysXCone::onComponentSet() {
    if (mMeshHandle) {
        PxConvexMeshGeometry geom;
        geom.convexMesh = (PxConvexMesh *)mMeshHandle;
        // geom.meshFlags = PxConvexMeshGeometryFlags::eTIGHT_BOUNDS;
        auto mat = (PxMaterial *)getPxMaterialMap()[0];
        mShape = PxGetPhysics().createShape(geom, *mat, true);
        updateGeometry();
    }
}

void PhysXCone::setCone(float r, float h, EAxisDirection d) {
    mData.radius = r;
    mData.height = h;
    mData.direction = d;
    updateGeometry();
}

void PhysXCone::updateGeometry() {
    if (!mShape) return;
    static PxMeshScale scale;
    PxSetVec3Ext(scale.scale, getSharedBody().getNode().worldScale);
    scale.scale.y *= std::max(0.0001f, mData.height / 2);
    const auto xz = std::max(0.0001f, mData.radius / 0.5f);
    scale.scale.x *= xz;
    scale.scale.z *= xz;
    Quaternion quat;
    switch (mData.direction) {
        case EAxisDirection::X_AXIS:
            quat.set(Vec3::UNIT_Z, PxPiDivTwo);
            PxSetQuatExt(scale.rotation, quat);
            break;
        case EAxisDirection::Y_AXIS:
        default:
            scale.rotation = PxQuat{PxIdentity};
            break;
        case EAxisDirection::Z_AXIS:
            quat.set(Vec3::UNIT_X, PxPiDivTwo);
            PxSetQuatExt(scale.rotation, quat);
            break;
    }
    PxConvexMeshGeometry geom;
    if (getShape().getConvexMeshGeometry(geom)) {
        geom.scale = scale;
        getShape().setGeometry(geom);
    }
    PxSetQuatExt(mRotation, quat);
}

void PhysXCone::updateScale() {
    updateGeometry();
    updateCenter();
}

} // namespace physics
} // namespace cc
