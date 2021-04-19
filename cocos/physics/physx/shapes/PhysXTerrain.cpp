
#include "physics/physx/shapes/PhysXShape.h"
#include "physics/physx/shapes/PhysXTerrain.h"
#include "physics/physx/PhysXUtils.h"
#include "physics/physx/PhysXWorld.h"
#include "renderer/pipeline/helper/SharedMemory.h"

namespace cc {
namespace physics {

PhysXTerrain::PhysXTerrain() : _mTerrain(nullptr),
                               _mRowScale(1.F),
                               _mColScale(1.F),
                               _mHeightScale(1.F){};

void PhysXTerrain::setTerrain(uintptr_t handle, float rs, float cs, float hs) {
    if (reinterpret_cast<uintptr_t>(_mTerrain) == handle) return;
    _mTerrain     = reinterpret_cast<physx::PxHeightField *>(handle);
    _mRowScale    = rs;
    _mColScale    = cs;
    _mHeightScale = hs;
    if (_mShape) {
    }
}

void PhysXTerrain::onComponentSet() {
    if (_mTerrain) {
        physx::PxHeightFieldGeometry geom;
        geom.rowScale    = _mRowScale;
        geom.columnScale = _mColScale;
        geom.heightScale = _mHeightScale;
        geom.heightField = _mTerrain;
        _mShape          = PxGetPhysics().createShape(geom, getDefaultMaterial(), true);
    }
}

void PhysXTerrain::updateScale() {
    // updateCenter(); needed?
}

void PhysXTerrain::updateCenter() {
    getShape().setLocalPose(physx::PxTransform{_mCenter, _mRotation});
}

} // namespace physics
} // namespace cc
