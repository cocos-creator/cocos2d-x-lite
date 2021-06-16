/****************************************************************************
 Copyright (c) 2021 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos.com
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated engine source code (the "Software"), a limited,
 worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 to use Cocos Creator solely to develop games on your target platforms. You shall
 not use Cocos Creator software for developing other software or tools that's
 used for developing games. You are not granted to publish, distribute,
 sublicense, and/or sell copies of Cocos Creator.
 
 The software or tools in this License Agreement are licensed, not sold.
 Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "jsb_scene_manual.h"
#include "bindings/auto/jsb_scene_auto.h"
#include "scene/Model.h"
#include "scene/Node.h"

#ifndef JSB_ALLOC
    #define JSB_ALLOC(kls, ...) new (std::nothrow) kls(__VA_ARGS__)
#endif

#ifndef JSB_FREE
    #define JSB_FREE(ptr) delete ptr
#endif

static bool js_scene_Pass_setRootBlock(se::State& s) // NOLINT(readability-identifier-naming, google-runtime-references)
{
    auto* cobj = SE_THIS_OBJECT<cc::scene::Pass>(s);
    SE_PRECONDITION2(cobj, false, "js_scene_Pass_setRootBlock : Invalid Native Object");
    const auto&    args = s.args();
    size_t         argc = args.size();
    CC_UNUSED bool ok   = true;
    if (argc == 1) {
        uint8_t* rootBlock{nullptr};
        args[0].toObject()->getArrayBufferData(&rootBlock, nullptr);
        cobj->setRootBlock(rootBlock);
        return true;
    }
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", (int)argc, 1);
    return false;
}
SE_BIND_FUNC(js_scene_Pass_setRootBlock) // NOLINT(readability-identifier-naming, google-runtime-references)

static bool js_scene_Model_setInstancedAttrBlock(se::State& s) // NOLINT(readability-identifier-naming, google-runtime-references)
{
    auto* cobj = static_cast<cc::scene::Model*>(s.nativeThisObject());
    SE_PRECONDITION2(cobj, false, "js_scene_Model_setInstancedAttrBlock : Invalid Native Object");
    const auto& args = s.args();
    size_t      argc = args.size();

    if (argc == 3) {
        SE_PRECONDITION2(args[0].isObject() && args[0].toObject()->isArrayBuffer(), false, "js_gfx_Device_createBuffer: expected Array Buffer!");

        // instanced buffer
        uint8_t* instanceBuff{nullptr};
        args[0].toObject()->getArrayBufferData(&instanceBuff, nullptr);

        // views
        se::Object* dataObj = args[1].toObject();
        if (!dataObj->isArray()) {
            return false;
        }
        std::vector<uint8_t*> viewsData;
        uint32_t              length = 0;
        dataObj->getArrayLength(&length);
        viewsData.resize(length);
        se::Value value;
        for (uint32_t i = 0; i < length; i++) {
            dataObj->getArrayElement(i, &value);
            uint8_t* viewBuff{nullptr};
            value.toObject()->getArrayBufferData(&viewBuff, nullptr);
            viewsData[i] = viewBuff;
        }

        cc::scene::InstancedAttributeBlock attrBlock;
        attrBlock.views = viewsData;

        // attrs
        CC_UNUSED bool                                    ok   = true;
        HolderType<std::vector<cc::gfx::Attribute>, true> arg2 = {};
        ok &= sevalue_to_native(args[2], &arg2, s.thisObject());
        SE_PRECONDITION2(ok, false, "js_scene_Model_setInstancedAttrBlock : Error processing arguments");
        cobj->setInstancedAttrBlock(instanceBuff, &attrBlock, arg2.value());

        return true;
    }

    SE_REPORT_ERROR("wrong number of arguments: %d", (int)argc);
    return false;
}
SE_BIND_FUNC(js_scene_Model_setInstancedAttrBlock) // NOLINT(readability-identifier-naming, google-runtime-references)

static bool js_scene_Node_initWithData(se::State& s) // NOLINT(readability-identifier-naming, google-runtime-references)
{
    auto*          cobj = static_cast<cc::scene::Node*>(s.nativeThisObject());
    CC_UNUSED bool ok   = true;
    const auto&    args = s.args();
    size_t         argc = args.size();
    do {
        if (argc == 1) {
            SE_PRECONDITION2(args[0].isObject() && args[0].toObject()->isTypedArray(), false, "js_scene_Node_initWithData: expected Typed Array!");

            uint8_t* data{nullptr};
            size_t   dataBytes = 0;
            args[0].toObject()->getTypedArrayData(&data, &dataBytes);
            cobj->initWithData(data);
            return true;
        }
    } while (false);
    SE_REPORT_ERROR("wrong number of arguments: %d", (int)argc);
    return false;
}
SE_BIND_FUNC(js_scene_Node_initWithData) // NOLINT(readability-identifier-naming, google-runtime-references)

static bool js_scene_SubModel_setSubMeshBuffers(se::State& s) // NOLINT(readability-identifier-naming, google-runtime-references)
{
    auto* cobj = static_cast<cc::scene::SubModel*>(s.nativeThisObject());
    SE_PRECONDITION2(cobj, false, "js_scene_SubModel_setSubMeshBuffers : Invalid Native Object");
    const auto& args = s.args();
    size_t      argc = args.size();

    if (argc == 1) {
        if (args[0].isObject()) {
            se::Object* dataObj = args[0].toObject();
            if (!dataObj->isArray()) {
                return false;
            }
            uint32_t length = 0;
            dataObj->getArrayLength(&length);
            std::vector<cc::scene::FlatBuffer> flatBuffers;
            flatBuffers.resize(length);
            se::Value value;
            for (uint32_t i = 0; i < length; ++i) {
                if (dataObj->getArrayElement(i, &value)) {
                    if (value.isObject()) {
                        cc::scene::FlatBuffer currBuffer;
                        se::Value             bufferVal;
                        se::Object*           valObj = value.toObject();
                        valObj->getProperty("buffer", &bufferVal);
                        // data
                        CC_UNUSED size_t bufferLength = 0;
                        uint8_t*         address      = nullptr;
                        bufferVal.toObject()->getTypedArrayData(&address, &bufferLength);
                        currBuffer.data = address;
                        currBuffer.size = bufferLength;
                        // stride
                        se::Value strideVal;
                        valObj->getProperty("stride", &strideVal);
                        currBuffer.stride = strideVal.toUint32();
                        // count
                        se::Value countVal;
                        valObj->getProperty("count", &countVal);
                        currBuffer.count = countVal.toUint32();
                        flatBuffers[i]   = currBuffer;
                    }
                }
            }
            cobj->setSubMeshBuffers(flatBuffers);
            return true;
        }
    }

    SE_REPORT_ERROR("wrong number of arguments: %d", (int)argc);
    return false;
}
SE_BIND_FUNC(js_scene_SubModel_setSubMeshBuffers) // NOLINT(readability-identifier-naming, google-runtime-references)

bool register_all_scene_manual(se::Object* obj) // NOLINT(readability-identifier-naming, google-runtime-references)
{
    // Get the ns
    se::Value nsVal;
    if (!obj->getProperty("ns", &nsVal)) {
        se::HandleObject jsobj(se::Object::createPlainObject());
        nsVal.setObject(jsobj);
        obj->setProperty("ns", nsVal);
    }
    se::Object* ns = nsVal.toObject();

    __jsb_cc_scene_Model_proto->defineFunction("setInstancedAttrBlock", _SE(js_scene_Model_setInstancedAttrBlock));
    __jsb_cc_scene_Node_proto->defineFunction("initWithData", _SE(js_scene_Node_initWithData));

    __jsb_cc_scene_SubModel_proto->defineFunction("setSubMeshBuffers", _SE(js_scene_SubModel_setSubMeshBuffers));
    __jsb_cc_scene_Pass_proto->defineFunction("setRootBlock", _SE(js_scene_Pass_setRootBlock));
    return true;
}
