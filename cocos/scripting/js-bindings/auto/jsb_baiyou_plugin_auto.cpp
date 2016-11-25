#include "jsb_baiyou_plugin_auto.hpp"
#include "cocos2d_specifics.hpp"
#include "baiyou/BaiyouPlugin.h"

template<class T>
static bool dummy_constructor(JSContext *cx, uint32_t argc, jsval *vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedValue initializing(cx);
    bool isNewValid = true;
    JS::RootedObject global(cx, ScriptingCore::getInstance()->getGlobalObject());
    isNewValid = JS_GetProperty(cx, global, "initializing", &initializing) && initializing.toBoolean();
    if (isNewValid)
    {
        TypeTest<T> t;
        js_type_class_t *typeClass = nullptr;
        std::string typeName = t.s_name();
        auto typeMapIter = _js_global_type_map.find(typeName);
        CCASSERT(typeMapIter != _js_global_type_map.end(), "Can't find the class type!");
        typeClass = typeMapIter->second;
        CCASSERT(typeClass, "The value is null.");

        JS::RootedObject proto(cx, typeClass->proto.ref());
        JS::RootedObject parent(cx, typeClass->parentProto.ref());
        JS::RootedObject _tmp(cx, JS_NewObject(cx, typeClass->jsclass, proto, parent));
        
        args.rval().set(OBJECT_TO_JSVAL(_tmp));
        return true;
    }

    JS_ReportError(cx, "Constructor for the requested class is not available, please refer to the API reference.");
    return false;
}

static bool empty_constructor(JSContext *cx, uint32_t argc, jsval *vp) {
    return false;
}

static bool js_is_native_obj(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    args.rval().setBoolean(true);
    return true;
}
JSClass  *jsb_baiyou_BaiyouPlugin_class;
JSObject *jsb_baiyou_BaiyouPlugin_prototype;

bool js_baiyou_plugin_BaiyouPlugin_init(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    js_proxy_t *proxy = jsb_get_js_proxy(obj);
    baiyou::BaiyouPlugin* cobj = (baiyou::BaiyouPlugin *)(proxy ? proxy->ptr : NULL);
    JSB_PRECONDITION2( cobj, cx, false, "js_baiyou_plugin_BaiyouPlugin_init : Invalid Native Object");
    if (argc == 0) {
        bool ret = cobj->init();
        jsval jsret = JSVAL_NULL;
        jsret = BOOLEAN_TO_JSVAL(ret);
        args.rval().set(jsret);
        return true;
    }

    JS_ReportError(cx, "js_baiyou_plugin_BaiyouPlugin_init : wrong number of arguments: %d, was expecting %d", argc, 0);
    return false;
}
bool js_baiyou_plugin_BaiyouPlugin_getProperty(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    bool ok = true;
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    js_proxy_t *proxy = jsb_get_js_proxy(obj);
    baiyou::BaiyouPlugin* cobj = (baiyou::BaiyouPlugin *)(proxy ? proxy->ptr : NULL);
    JSB_PRECONDITION2( cobj, cx, false, "js_baiyou_plugin_BaiyouPlugin_getProperty : Invalid Native Object");
    if (argc == 1) {
        std::string arg0;
        ok &= jsval_to_std_string(cx, args.get(0), &arg0);
        JSB_PRECONDITION2(ok, cx, false, "js_baiyou_plugin_BaiyouPlugin_getProperty : Error processing arguments");
        std::string ret = cobj->getProperty(arg0);
        jsval jsret = JSVAL_NULL;
        jsret = std_string_to_jsval(cx, ret);
        args.rval().set(jsret);
        return true;
    }

    JS_ReportError(cx, "js_baiyou_plugin_BaiyouPlugin_getProperty : wrong number of arguments: %d, was expecting %d", argc, 1);
    return false;
}
bool js_baiyou_plugin_BaiyouPlugin_getInstance(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    if (argc == 0) {
        baiyou::BaiyouPlugin* ret = baiyou::BaiyouPlugin::getInstance();
        jsval jsret = JSVAL_NULL;
        do {
        if (ret) {
            js_proxy_t *jsProxy = js_get_or_create_proxy<baiyou::BaiyouPlugin>(cx, (baiyou::BaiyouPlugin*)ret);
            jsret = OBJECT_TO_JSVAL(jsProxy->obj);
        } else {
            jsret = JSVAL_NULL;
        }
    } while (0);
        args.rval().set(jsret);
        return true;
    }
    JS_ReportError(cx, "js_baiyou_plugin_BaiyouPlugin_getInstance : wrong number of arguments");
    return false;
}

void js_baiyou_BaiyouPlugin_finalize(JSFreeOp *fop, JSObject *obj) {
    CCLOGINFO("jsbindings: finalizing JS object %p (BaiyouPlugin)", obj);
}
void js_register_baiyou_plugin_BaiyouPlugin(JSContext *cx, JS::HandleObject global) {
    jsb_baiyou_BaiyouPlugin_class = (JSClass *)calloc(1, sizeof(JSClass));
    jsb_baiyou_BaiyouPlugin_class->name = "BaiyouPlugin";
    jsb_baiyou_BaiyouPlugin_class->addProperty = JS_PropertyStub;
    jsb_baiyou_BaiyouPlugin_class->delProperty = JS_DeletePropertyStub;
    jsb_baiyou_BaiyouPlugin_class->getProperty = JS_PropertyStub;
    jsb_baiyou_BaiyouPlugin_class->setProperty = JS_StrictPropertyStub;
    jsb_baiyou_BaiyouPlugin_class->enumerate = JS_EnumerateStub;
    jsb_baiyou_BaiyouPlugin_class->resolve = JS_ResolveStub;
    jsb_baiyou_BaiyouPlugin_class->convert = JS_ConvertStub;
    jsb_baiyou_BaiyouPlugin_class->finalize = js_baiyou_BaiyouPlugin_finalize;
    jsb_baiyou_BaiyouPlugin_class->flags = JSCLASS_HAS_RESERVED_SLOTS(2);

    static JSPropertySpec properties[] = {
        JS_PSG("__nativeObj", js_is_native_obj, JSPROP_PERMANENT | JSPROP_ENUMERATE),
        JS_PS_END
    };

    static JSFunctionSpec funcs[] = {
        JS_FN("init", js_baiyou_plugin_BaiyouPlugin_init, 0, JSPROP_PERMANENT | JSPROP_ENUMERATE),
        JS_FN("getProperty", js_baiyou_plugin_BaiyouPlugin_getProperty, 1, JSPROP_PERMANENT | JSPROP_ENUMERATE),
        JS_FS_END
    };

    static JSFunctionSpec st_funcs[] = {
        JS_FN("getInstance", js_baiyou_plugin_BaiyouPlugin_getInstance, 0, JSPROP_PERMANENT | JSPROP_ENUMERATE),
        JS_FS_END
    };

    jsb_baiyou_BaiyouPlugin_prototype = JS_InitClass(
        cx, global,
        JS::NullPtr(),
        jsb_baiyou_BaiyouPlugin_class,
        dummy_constructor<baiyou::BaiyouPlugin>, 0, // no constructor
        properties,
        funcs,
        NULL, // no static properties
        st_funcs);

    // add the proto and JSClass to the type->js info hash table
    JS::RootedObject proto(cx, jsb_baiyou_BaiyouPlugin_prototype);
    jsb_register_class<baiyou::BaiyouPlugin>(cx, jsb_baiyou_BaiyouPlugin_class, proto, JS::NullPtr());
}
void register_all_baiyou_plugin(JSContext* cx, JS::HandleObject obj) {
    // Get the ns
    JS::RootedObject ns(cx);
    get_or_create_js_obj(cx, obj, "baiyou", &ns);

    js_register_baiyou_plugin_BaiyouPlugin(cx, ns);
}

