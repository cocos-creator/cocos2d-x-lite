
#include "PhysXShape.h"
#include "PhysXBox.h"
#include "../PhysXUtils.h"
#include "../PhysXWorld.h"

namespace cc {
namespace physics {

PhysXBox::PhysXBox() : mHalfExtents(0.5f),
                       PhysXShape(){};

void PhysXBox::setSize(float x, float y, float z) {
    mHalfExtents = PxVec3{x / 2, y / 2, z / 2};
    updateGeometry();
    getShape().setGeometry(getPxGeometry<PxBoxGeometry>());
}

void PhysXBox::onComponentSet() {
    updateGeometry();
    mShape = PxGetPhysics().createShape(getPxGeometry<PxBoxGeometry>(), getDefaultMaterial(), true);
}

void PhysXBox::updateScale() {
    updateGeometry();
    getShape().setGeometry(getPxGeometry<PxBoxGeometry>());
    updateCenter();
}

void PhysXBox::updateGeometry() {
    auto &node = getSharedBody().getNode();
    auto &geo = getPxGeometry<PxBoxGeometry>();
    geo.halfExtents = mHalfExtents;
    geo.halfExtents *= node.worldScale;
    geo.halfExtents = geo.halfExtents.abs();
    if (geo.halfExtents.minElement() <= 0.0) {
        geo.halfExtents = geo.halfExtents.maximum(PxVec3{PX_NORMALIZATION_EPSILON});
    }
}

} // namespace physics
} // namespace cc