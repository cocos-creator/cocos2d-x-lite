
#include "PhysXInc.h"
#include "PhysXFilterShader.h"

namespace cc {
namespace physics {

PxFilterFlags SimpleFilterShader(
    PxFilterObjectAttributes attributes0, PxFilterData fd0,
    PxFilterObjectAttributes attributes1, PxFilterData fd1,
    PxPairFlags &pairFlags, const void *, PxU32) {
    // group mask filter
    if (!(fd0.word0 & fd1.word1) || !(fd0.word1 & fd1.word0)) {
        return PxFilterFlag::eSUPPRESS;
    }

    if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1)) {
        pairFlags |= PxPairFlag::eTRIGGER_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_CCD;
        return PxFilterFlag::eDEFAULT;
    }
    if (!PxFilterObjectIsKinematic(attributes0) || !PxFilterObjectIsKinematic(attributes1)) {
        pairFlags |= PxPairFlag::eSOLVE_CONTACT;
    }
    pairFlags |= PxPairFlag::eDETECT_DISCRETE_CONTACT |
                 PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_LOST | PxPairFlag::eNOTIFY_TOUCH_PERSISTS |
                 PxPairFlag::eDETECT_CCD_CONTACT | PxPairFlag::eNOTIFY_CONTACT_POINTS;
    return PxFilterFlag::eDEFAULT;
}

PxFilterFlags AdvanceFilterShader(
    PxFilterObjectAttributes attributes0, PxFilterData fd0,
    PxFilterObjectAttributes attributes1, PxFilterData fd1,
    PxPairFlags &pairFlags, const void *, PxU32) {
    // group mask filter
    if (!(fd0.word0 & fd1.word1) || !(fd0.word1 & fd1.word0)) {
        return PxFilterFlag::eSUPPRESS;
    }

    pairFlags = PxPairFlags(0);

    // trigger filter
    if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1)) {
        pairFlags |= PxPairFlag::eDETECT_DISCRETE_CONTACT;

        // need trigger event?
        const PxU16 needTriggerEvent = (fd0.word3 & DETECT_TRIGGER_EVENT) | (fd1.word3 & DETECT_TRIGGER_EVENT);
        if (needTriggerEvent) {
            pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_LOST;
            return PxFilterFlag::eDEFAULT;
        } else {
            return PxFilterFlag::eSUPPRESS;
        }
    }

    // need detect ccd contact?
    const PxU16 needDetectCCD = (fd0.word3 & DETECT_CONTACT_CCD) | (fd1.word3 & DETECT_CONTACT_CCD);
    if (needDetectCCD) pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;

    if (!PxFilterObjectIsKinematic(attributes0) || !PxFilterObjectIsKinematic(attributes1)) {
        pairFlags |= PxPairFlag::eSOLVE_CONTACT;
    }

    // simple collision process
    pairFlags |= PxPairFlag::eDETECT_DISCRETE_CONTACT;

    // need contact event?
    const PxU16 needContactEvent = (fd0.word3 & DETECT_CONTACT_EVENT) | (fd1.word3 & DETECT_CONTACT_EVENT);
    if (needContactEvent) pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_LOST | PxPairFlag::eNOTIFY_TOUCH_PERSISTS;

    // need contact point?
    const PxU16 needContactPoint = (fd0.word3 & DETECT_CONTACT_POINT) | (fd1.word3 & DETECT_CONTACT_POINT);
    if (needContactPoint) pairFlags |= PxPairFlag::eNOTIFY_CONTACT_POINTS;

    return PxFilterFlag::eDEFAULT;
}

PxQueryHitType::Enum QueryFilterShader::preFilter(const PxFilterData &filterData, const PxShape *shape,
                                                  const PxRigidActor *actor, PxHitFlags &queryFlags) {

    if ((filterData.word3 & QUERY_CHECK_TRIGGER) && shape->getFlags().isSet(PxShapeFlag::eTRIGGER_SHAPE)) {
        return PxQueryHitType::eNONE;
    }
    return filterData.word3 & QUERY_SINGLE_HIT ? PxQueryHitType::eBLOCK : PxQueryHitType::eTOUCH;
}

} // namespace physics
} // namespace cc