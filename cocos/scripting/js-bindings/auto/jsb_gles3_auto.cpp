#include "scripting/js-bindings/auto/jsb_gles3_auto.hpp"
#include "scripting/js-bindings/manual/jsb_conversions.hpp"
#include "scripting/js-bindings/manual/jsb_global.h"
#include "renderer/gfx-gles3/GFXGLES3.h"

#ifndef JSB_ALLOC
#define JSB_ALLOC(kls, ...) new (std::nothrow) kls(__VA_ARGS__)
#endif

#ifndef JSB_FREE
#define JSB_FREE(ptr) delete ptr
#endif
se::Object* __jsb_cocos2d_GLES3Device_proto = nullptr;
se::Class* __jsb_cocos2d_GLES3Device_class = nullptr;

static bool js_gles3_GLES3Device_checkExtension(se::State& s)
{
    cocos2d::GLES3Device* cobj = SE_THIS_OBJECT<cocos2d::GLES3Device>(s);
    SE_PRECONDITION2(cobj, false, "js_gles3_GLES3Device_checkExtension : Invalid Native Object");
    const auto& args = s.args();
    size_t argc = args.size();
    CC_UNUSED bool ok = true;
    if (argc == 1) {
        HolderType<cocos2d::String, true>::local_type arg0 = {};
        ok &= sevalue_to_native(args[0], &arg0, s.thisObject()); //is_reference True;
        SE_PRECONDITION2(ok, false, "js_gles3_GLES3Device_checkExtension : Error processing arguments");
        bool result = cobj->checkExtension(HolderType<cocos2d::String, true>::value(arg0));
        ok &= nativevalue_to_se(result, s.rval(), nullptr /*ctx*/);
        SE_PRECONDITION2(ok, false, "js_gles3_GLES3Device_checkExtension : Error processing arguments");
        return true;
    }
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", (int)argc, 1);
    return false;
}
SE_BIND_FUNC(js_gles3_GLES3Device_checkExtension)

static bool js_gles3_GLES3Device_useVAO(se::State& s)
{
    cocos2d::GLES3Device* cobj = SE_THIS_OBJECT<cocos2d::GLES3Device>(s);
    SE_PRECONDITION2(cobj, false, "js_gles3_GLES3Device_useVAO : Invalid Native Object");
    const auto& args = s.args();
    size_t argc = args.size();
    CC_UNUSED bool ok = true;
    if (argc == 0) {
        bool result = cobj->useVAO();
        ok &= nativevalue_to_se(result, s.rval(), nullptr /*ctx*/);
        SE_PRECONDITION2(ok, false, "js_gles3_GLES3Device_useVAO : Error processing arguments");
        return true;
    }
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", (int)argc, 0);
    return false;
}
SE_BIND_FUNC(js_gles3_GLES3Device_useVAO)

SE_DECLARE_FINALIZE_FUNC(js_cocos2d_GLES3Device_finalize)

static bool js_gles3_GLES3Device_constructor(se::State& s) // constructor.c
{
    cocos2d::GLES3Device* cobj = JSB_ALLOC(cocos2d::GLES3Device);
    s.thisObject()->setPrivateData(cobj);
    se::NonRefNativePtrCreatedByCtorMap::emplace(cobj);
    return true;
}
SE_BIND_CTOR(js_gles3_GLES3Device_constructor, __jsb_cocos2d_GLES3Device_class, js_cocos2d_GLES3Device_finalize)



extern se::Object* __jsb_cocos2d_GFXDevice_proto;

static bool js_cocos2d_GLES3Device_finalize(se::State& s)
{
    CCLOGINFO("jsbindings: finalizing JS object %p (cocos2d::GLES3Device)", SE_THIS_OBJECT<cocos2d::GLES3Device>(s););
    auto iter = se::NonRefNativePtrCreatedByCtorMap::find(SE_THIS_OBJECT<cocos2d::GLES3Device>(s));
    if (iter != se::NonRefNativePtrCreatedByCtorMap::end())
    {
        se::NonRefNativePtrCreatedByCtorMap::erase(iter);
        cocos2d::GLES3Device* cobj = SE_THIS_OBJECT<cocos2d::GLES3Device>(s);
        JSB_FREE(cobj);
    }
    return true;
}
SE_BIND_FINALIZE_FUNC(js_cocos2d_GLES3Device_finalize)

bool js_register_gles3_GLES3Device(se::Object* obj)
{
    auto cls = se::Class::create("GLES3Device", obj, __jsb_cocos2d_GFXDevice_proto, _SE(js_gles3_GLES3Device_constructor));

    cls->defineFunction("checkExtension", _SE(js_gles3_GLES3Device_checkExtension));
    cls->defineFunction("useVAO", _SE(js_gles3_GLES3Device_useVAO));
    cls->defineFinalizeFunction(_SE(js_cocos2d_GLES3Device_finalize));
    cls->install();
    JSBClassType::registerClass<cocos2d::GLES3Device>(cls);
    
    if constexpr (std::is_standard_layout_v<cocos2d::GLES3Device>) {
        auto array = se::Object::createArrayObject(0);
        int idx = 0;
        {
            //stateCache : GLES3StateCache*
            using field_type = decltype(cocos2d::GLES3Device::stateCache);
            auto attr = se::Object::createPlainObject();
            attr->setProperty("name", se::Value("stateCache"));
            attr->setProperty("offset", se::Value(offsetof(cocos2d::GLES3Device, stateCache)));
            attr->setProperty("size", se::Value(sizeof(field_type)));
            attr->setProperty("type", se::Value(SE_UNDERLYING_TYPE_NAME<field_type>()));
            array->setArrayElement(idx++, se::Value(attr));
            attr->decRef();
        }
        cls->getProto()->setProperty("__attrMeta", se::Value(array));
        //obj->setProperty("__attrMeta_GLES3Device", se::Value(array));
        array->decRef();
    }

    __jsb_cocos2d_GLES3Device_proto = cls->getProto();
    __jsb_cocos2d_GLES3Device_class = cls;

    se::ScriptEngine::getInstance()->clearException();
    return true;
}

bool register_all_gles3(se::Object* obj)
{
    // Get the ns
    se::Value nsVal;
    if (!obj->getProperty("gfx", &nsVal))
    {
        se::HandleObject jsobj(se::Object::createPlainObject());
        nsVal.setObject(jsobj);
        obj->setProperty("gfx", nsVal);
    }
    se::Object* ns = nsVal.toObject();

    js_register_gles3_GLES3Device(ns);
    return true;
}

