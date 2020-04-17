/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

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

#pragma once

#include "scripting/js-bindings/jswrapper/SeApi.h"
#include "scripting/js-bindings/manual/jsb_classtype.hpp"

#include "cocos2d.h"
#include "network/CCDownloader.h"
#include "extensions/cocos-ext.h"

#include <string>
#include <typeinfo>
//#include "renderer/core/TypeDef.h"
//#include "cocos/renderer/core/gfx/GFXContext.h"
//#include "renderer/core/memory/StlAlloc.h"

//#include "Box2D/Box2D.h"

#define SE_PRECONDITION2_VOID(condition, ...) \
    do { \
        if ( ! (condition) ) { \
            SE_LOGE("jsb: ERROR: File %s: Line: %d, Function: %s\n", __FILE__, __LINE__, __FUNCTION__ ); \
            SE_LOGE(__VA_ARGS__); \
            return; \
        } \
    } while(0)

#define SE_PRECONDITION2(condition, ret_value, ...) \
    do { \
        if ( ! (condition) ) { \
            SE_LOGE("jsb: ERROR: File %s: Line: %d, Function: %s\n", __FILE__, __LINE__, __FUNCTION__ ); \
            SE_LOGE(__VA_ARGS__); \
            return (ret_value); \
        } \
    } while(0)


#define SE_PRECONDITION3(condition, ret_value, failed_code) \
    do { \
        if (!(condition)) \
        { \
            failed_code; \
            return (ret_value); \
        } \
    } while(0)

#define SE_PRECONDITION4(condition, ret_value, errorCode) \
    do { \
        if ( ! (condition) ) { \
            SE_LOGE("jsb: ERROR: File %s: Line: %d, Function: %s\n", __FILE__, __LINE__, __FUNCTION__ ); \
            __glErrorCode = errorCode; \
            return (ret_value); \
        } \
    } while(0)

#define SE_PRECONDITION_ERROR_BREAK(condition, ...) \
    if ( ! (condition) ) { \
        SE_LOGE("jsb: ERROR: File %s: Line: %d, Function: %s\n", __FILE__, __LINE__, __FUNCTION__ ); \
        SE_LOGE(__VA_ARGS__); \
        break; \
    }


// se value -> native value
bool seval_to_int32(const se::Value& v, int32_t* ret);
bool seval_to_uint32(const se::Value& v, uint32_t* ret);
bool seval_to_int8(const se::Value& v, int8_t* ret);
bool seval_to_uint8(const se::Value& v, uint8_t* ret);
bool seval_to_int16(const se::Value& v, int16_t* ret);
bool seval_to_uint16(const se::Value& v, uint16_t* ret);
bool seval_to_boolean(const se::Value& v, bool* ret);
bool seval_to_float(const se::Value& v, float* ret);
bool seval_to_double(const se::Value& v, double* ret);
bool seval_to_long(const se::Value& v, long* ret);
bool seval_to_ulong(const se::Value& v, unsigned long* ret);
bool seval_to_longlong(const se::Value& v, long long* ret);
bool seval_to_ssize(const se::Value& v, ssize_t* ret);
bool seval_to_size(const se::Value& v, size_t* ret);
bool seval_to_std_string(const se::Value& v, std::string* ret);
bool seval_to_Vec2(const se::Value& v, cocos2d::Vec2* pt);
bool seval_to_Vec3(const se::Value& v, cocos2d::Vec3* pt);
bool seval_to_Vec4(const se::Value& v, cocos2d::Vec4* pt);
bool seval_to_Mat4(const se::Value& v, cocos2d::Mat4* mat);
bool seval_to_Size(const se::Value& v, cocos2d::Size* size);
bool seval_to_ccvalue(const se::Value& v, cocos2d::Value* ret);
bool seval_to_ccvaluemap(const se::Value& v, cocos2d::ValueMap* ret);
bool seval_to_ccvaluemapintkey(const se::Value& v, cocos2d::ValueMapIntKey* ret);
bool seval_to_ccvaluevector(const se::Value& v, cocos2d::ValueVector* ret);
bool sevals_variadic_to_ccvaluevector(const se::ValueArray& args, cocos2d::ValueVector* ret);
bool seval_to_std_vector_string(const se::Value& v, std::vector<std::string>* ret);
bool seval_to_std_vector_int(const se::Value& v, std::vector<int>* ret);
bool seval_to_std_vector_uint16(const se::Value& v, std::vector<uint16_t>* ret);
bool seval_to_std_vector_float(const se::Value& v, std::vector<float>* ret);
bool seval_to_std_vector_Vec2(const se::Value& v, std::vector<cocos2d::Vec2>* ret);
bool seval_to_Uint8Array(const se::Value& v, uint8_t* ret);
bool seval_to_uintptr_t(const se::Value& v, uintptr_t* ret);

bool seval_to_std_map_string_string(const se::Value& v, std::map<std::string, std::string>* ret);
bool seval_to_Data(const se::Value& v, cocos2d::Data* ret);
bool seval_to_DownloaderHints(const se::Value& v, cocos2d::network::DownloaderHints* ret);

#if USE_GFX_RENDERER
#endif

template<typename T>
bool seval_to_native_ptr(const se::Value& v, T* ret)
{
    assert(ret != nullptr);

    if (v.isObject())
    {
        T ptr = (T)v.toObject()->getPrivateData();
        if (ptr == nullptr)
        {
            // This should never happen, return 'false' to mark the conversion fails.
            *ret = nullptr;
            return false;
        }

        *ret = ptr;
        return true;
    }
    else if (v.isNullOrUndefined())
    {
        // If js value is null or undefined, the convertion should be successful.
        // So we should return 'true' to indicate the convertion succeeds and mark
        // the out value to 'nullptr'.
        *ret = nullptr;
        return true;
    }

    // If js value isn't null, undefined and Object, mark the convertion fails.
    *ret = nullptr;
    return false;
}

template<typename T>
bool seval_to_Vector(const se::Value& v, cocos2d::Vector<T>* ret)
{
    assert(ret != nullptr);
    assert(v.isObject());
    se::Object* obj = v.toObject();
    assert(obj->isArray());

    bool ok = true;
    uint32_t len = 0;
    ok = obj->getArrayLength(&len);
    if (!ok)
    {
        ret->clear();
        return false;
    }

    se::Value tmp;
    for (uint32_t i = 0; i < len; ++i)
    {
        ok = obj->getArrayElement(i, &tmp);
        if (!ok || !tmp.isObject())
        {
            ret->clear();
            return false;
        }

        T nativeObj = (T)tmp.toObject()->getPrivateData();

        ret->pushBack(nativeObj);
    }

    return true;
}

template<typename T>
typename std::enable_if<std::is_class<T>::value && !std::is_same<T, std::string>::value, T>::type
seval_to_type(const se::Value &v, bool &ok)
{
    if(!v.isObject())
    {
        ok = false;
        return T();
    }
    T* nativeObj = (T*)v.toObject()->getPrivateData();
    ok = true;
    return *nativeObj;
}
template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
seval_to_type(const se::Value &v, bool &ok)
{
    if(!v.isNumber())
    {
        ok = false;
        return 0;
    }
    ok = true;
    return v.toInt32();
}

template<typename T>
typename std::enable_if<std::is_enum<T>::value, T>::type
seval_to_type(const se::Value &v, bool &ok)
{
    if(!v.isNumber())
    {
        ok = false;
        return (T)0;
    }
    ok = true;
    return (T)v.toInt32();
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
seval_to_type(const se::Value &v, bool &ok)
{
    if(!v.isNumber())
    {
        ok = false;
        return 0;
    }
    ok = true;
    return v.toFloat();
}

template<typename T>
typename std::enable_if<std::is_same<T, std::string>::value, T>::type
seval_to_type(const se::Value &v, bool &ok)
{
    if(!v.isString())
    {
        ok = false;
        return "";
    }
    ok = true;
    return v.toString();
}


template<typename T>
typename std::enable_if<std::is_pointer<T>::value && std::is_class<typename std::remove_pointer<T>::type>::value, bool>::type
seval_to_std_vector(const se::Value& v, std::vector<T>* ret)
{
    assert(ret != nullptr);
    assert(v.isObject());
    se::Object* obj = v.toObject();
    assert(obj->isArray());
    
    bool ok = true;
    uint32_t len = 0;
    ok = obj->getArrayLength(&len);
    if (!ok)
    {
        ret->clear();
        return false;
    }
    
    ret->resize(len);
    
    se::Value tmp;
    for (uint32_t i = 0; i < len; ++i)
    {
        ok = obj->getArrayElement(i, &tmp);
        if (!ok || !tmp.isObject())
        {
            ret->clear();
            return false;
        }
        
        T nativeObj = (T)tmp.toObject()->getPrivateData();
        
        (*ret)[i] = nativeObj;
    }
    
    return true;
}

template<typename T>
typename std::enable_if<!std::is_pointer<T>::value, bool>::type
seval_to_std_vector(const se::Value& v, std::vector<T>* ret)
{
    assert(ret != nullptr);
    assert(v.isObject());
    se::Object* obj = v.toObject();
    assert(obj->isArray());
    
    bool ok = true;
    uint32_t len = 0;
    ok = obj->getArrayLength(&len);
    if (!ok)
    {
        ret->clear();
        return false;
    }
    
    ret->resize(len);
    
    se::Value tmp;
    for (uint32_t i = 0; i < len; ++i)
    {
        ok = obj->getArrayElement(i, &tmp);
        if(!ok) {
            ret->clear();
            return false;
        }
        (*ret)[i] = seval_to_type<T>(tmp, ok);
        if(!ok) {
            return false;
        }
    }
    
    return true;
}

template<typename T>
bool seval_to_Map_string_key(const se::Value& v, cocos2d::Map<std::string, T>* ret)
{
    assert(ret != nullptr);
    assert(v.isObject());
    se::Object* obj = v.toObject();

    std::vector<std::string> allKeys;
    bool ok = obj->getAllKeys(&allKeys);
    if (!ok)
    {
        ret->clear();
        return false;
    }

    se::Value tmp;
    for (const auto& key : allKeys)
    {
        ok = obj->getProperty(key.c_str(), &tmp);
        if (!ok || !tmp.isObject())
        {
            ret->clear();
            return false;
        }

        T nativeObj = (T)tmp.toObject()->getPrivateData();

        ret->insert(key, nativeObj);
    }

    return true;
}

// native value -> se value
bool int8_to_seval(int8_t v, se::Value* ret);
bool uint8_to_seval(uint8_t v, se::Value* ret);
bool int32_to_seval(int32_t v, se::Value* ret);
bool uint32_to_seval(uint32_t v, se::Value* ret);
bool int16_to_seval(uint16_t v, se::Value* ret);
bool uint16_to_seval(uint16_t v, se::Value* ret);
bool boolean_to_seval(bool v, se::Value* ret);
bool float_to_seval(float v, se::Value* ret);
bool double_to_seval(double v, se::Value* ret);
bool long_to_seval(long v, se::Value* ret);
bool ulong_to_seval(unsigned long v, se::Value* ret);
bool longlong_to_seval(long long v, se::Value* ret);
bool uintptr_t_to_seval(uintptr_t v, se::Value* ret);
bool ssize_to_seval(ssize_t v, se::Value* ret);
bool size_to_seval(size_t v, se::Value* ret);
bool std_string_to_seval(const std::string& v, se::Value* ret);

bool Vec2_to_seval(const cocos2d::Vec2& v, se::Value* ret);
bool Vec3_to_seval(const cocos2d::Vec3& v, se::Value* ret);
bool Vec4_to_seval(const cocos2d::Vec4& v, se::Value* ret);
bool Mat4_to_seval(const cocos2d::Mat4& v, se::Value* ret);
bool Size_to_seval(const cocos2d::Size& v, se::Value* ret);
bool Rect_to_seval(const cocos2d::Rect& v, se::Value* ret);
bool ccvalue_to_seval(const cocos2d::Value& v, se::Value* ret);
bool ccvaluemap_to_seval(const cocos2d::ValueMap& v, se::Value* ret);
bool ccvaluemapintkey_to_seval(const cocos2d::ValueMapIntKey& v, se::Value* ret);
bool ccvaluevector_to_seval(const cocos2d::ValueVector& v, se::Value* ret);
bool std_vector_string_to_seval(const std::vector<std::string>& v, se::Value* ret);
bool std_vector_int_to_seval(const std::vector<int>& v, se::Value* ret);
bool std_vector_uint16_to_seval(const std::vector<uint16_t>& v, se::Value* ret);
bool std_vector_float_to_seval(const std::vector<float>& v, se::Value* ret);
bool std_map_string_string_to_seval(const std::map<std::string, std::string>& v, se::Value* ret);

bool ManifestAsset_to_seval(const cocos2d::extension::ManifestAsset& v, se::Value* ret);
bool Data_to_seval(const cocos2d::Data& v, se::Value* ret);
bool DownloadTask_to_seval(const cocos2d::network::DownloadTask& v, se::Value* ret);
bool std_vector_EffectDefine_to_seval(const std::vector<cocos2d::ValueMap>& v, se::Value* ret);

#if USE_GFX_RENDERER
#endif

template<typename T>
typename std::enable_if<!std::is_base_of<cocos2d::Ref,T>::value,bool>::type
native_ptr_to_seval(T* v_c, se::Value* ret, bool* isReturnCachedValue = nullptr)
{
    typedef typename std::decay<typename std::remove_const<T>::type>::type DecayT;
    DecayT* v = const_cast<DecayT*>(v_c);
    assert(ret != nullptr);
    if (v == nullptr)
    {
        ret->setNull();
        return true;
    }

    se::Object* obj = nullptr;
    auto iter = se::NativePtrToObjectMap::find(v);
    if (iter == se::NativePtrToObjectMap::end())
    { // If we couldn't find native object in map, then the native object is created from native code. e.g. TMXLayer::getTileAt
        // CCLOGWARN("WARNING: non-Ref type: (%s) isn't catched!", typeid(*v).name());
        se::Class* cls = JSBClassType::findClass<T>(v);
        assert(cls != nullptr);
        obj = se::Object::createObjectWithClass(cls);
        ret->setObject(obj, true);
        obj->setPrivateData(v);
        if (isReturnCachedValue != nullptr)
        {
            *isReturnCachedValue = false;
        }
    }
    else
    {
        obj = iter->second;
        if (isReturnCachedValue != nullptr)
        {
            *isReturnCachedValue = true;
        }
        ret->setObject(obj);
    }

    return true;
}

//handle reference
template<typename T>
typename std::enable_if<!std::is_base_of<cocos2d::Ref,T>::value && !std::is_pointer<T>::value,bool>::type
native_ptr_to_seval(T& v_ref, se::Value* ret, bool* isReturnCachedValue = nullptr)
{
    typedef typename std::decay<typename std::remove_const<decltype(v_ref)>::type>::type  DecayT;
    DecayT *v = const_cast<DecayT*>(&v_ref);
    
    assert(ret != nullptr);
    if (v == nullptr)
    {
        ret->setNull();
        return true;
    }
    
    se::Object* obj = nullptr;
    auto iter = se::NativePtrToObjectMap::find(v);
    if (iter == se::NativePtrToObjectMap::end())
    { // If we couldn't find native object in map, then the native object is created from native code. e.g. TMXLayer::getTileAt
        // CCLOGWARN("WARNING: non-Ref type: (%s) isn't catched!", typeid(*v).name());
        se::Class* cls = JSBClassType::findClass<DecayT>(v);
        assert(cls != nullptr);
        obj = se::Object::createObjectWithClass(cls);
        ret->setObject(obj, true);
        obj->setPrivateData(v);
        if (isReturnCachedValue != nullptr)
        {
            *isReturnCachedValue = false;
        }
    }
    else
    {
        obj = iter->second;
        if (isReturnCachedValue != nullptr)
        {
            *isReturnCachedValue = true;
        }
        ret->setObject(obj);
    }
    
    return true;
}

template<typename T>
bool native_ptr_to_rooted_seval(const typename std::enable_if<!std::is_base_of<cocos2d::Ref,T>::value,T>::type* v, se::Value* ret, bool* isReturnCachedValue = nullptr)
{
    assert(ret != nullptr);
    if (v == nullptr)
    {
        ret->setNull();
        return true;
    }

    se::Object* obj = nullptr;
    auto iter = se::NativePtrToObjectMap::find((void*)v);
    if (iter == se::NativePtrToObjectMap::end())
    { // If we couldn't find native object in map, then the native object is created from native code. e.g. TMXLayer::getTileAt
        se::Class* cls = JSBClassType::findClass<T>(v);
        assert(cls != nullptr);
        obj = se::Object::createObjectWithClass(cls);
        obj->root();
        obj->setPrivateData((void*)v);

        if (isReturnCachedValue != nullptr)
        {
            *isReturnCachedValue = false;
        }
        // CCLOGWARN("WARNING: non-Ref type: (%s) isn't catched!", typeid(*v).name());
    }
    else
    {
        obj = iter->second;
        assert(obj->isRooted());
        if (isReturnCachedValue != nullptr)
        {
            *isReturnCachedValue = true;
        }
        // CCLOG("return cached object: %s, se::Object:%p, native: %p", typeid(*v).name(), obj, v);
    }

    ret->setObject(obj);
    return true;
}

template<typename T>
typename std::enable_if<!std::is_base_of<cocos2d::Ref,T>::value,bool>::type
native_ptr_to_seval(T* vp, se::Class* cls, se::Value* ret, bool* isReturnCachedValue = nullptr)
{
    typedef typename std::decay<typename std::remove_const<T>::type>::type DecayT;
    DecayT *v = const_cast<DecayT*>(vp);
    assert(ret != nullptr);
    if (v == nullptr)
    {
        ret->setNull();
        return true;
    }

    se::Object* obj = nullptr;
    auto iter = se::NativePtrToObjectMap::find(v);
    if (iter == se::NativePtrToObjectMap::end())
    { // If we couldn't find native object in map, then the native object is created from native code. e.g. TMXLayer::getTileAt
//        CCLOGWARN("WARNING: Ref type: (%s) isn't catched!", typeid(*v).name());
        assert(cls != nullptr);
        obj = se::Object::createObjectWithClass(cls);
        ret->setObject(obj, true);
        obj->setPrivateData(v);

        if (isReturnCachedValue != nullptr)
        {
            *isReturnCachedValue = false;
        }
    }
    else
    {
        obj = iter->second;
        if (isReturnCachedValue != nullptr)
        {
            *isReturnCachedValue = true;
        }
        ret->setObject(obj);
    }

    return true;
}

//handle ref
template<typename T>
typename std::enable_if<!std::is_base_of<cocos2d::Ref,T>::value,bool>::type
native_ptr_to_seval(T& v_ref, se::Class* cls, se::Value* ret, bool* isReturnCachedValue = nullptr)
{
    typedef typename std::decay<typename std::remove_const<decltype(v_ref)>::type>::type  DecayT;
    DecayT *v = const_cast<DecayT*>(&v_ref);
    
    
    assert(ret != nullptr);
    if (v == nullptr)
    {
        ret->setNull();
        return true;
    }
    
    se::Object* obj = nullptr;
    auto iter = se::NativePtrToObjectMap::find(v);
    if (iter == se::NativePtrToObjectMap::end())
    { // If we couldn't find native object in map, then the native object is created from native code. e.g. TMXLayer::getTileAt
        //        CCLOGWARN("WARNING: Ref type: (%s) isn't catched!", typeid(*v).name());
        assert(cls != nullptr);
        obj = se::Object::createObjectWithClass(cls);
        ret->setObject(obj, true);
        obj->setPrivateData(v);
        
        if (isReturnCachedValue != nullptr)
        {
            *isReturnCachedValue = false;
        }
    }
    else
    {
        obj = iter->second;
        if (isReturnCachedValue != nullptr)
        {
            *isReturnCachedValue = true;
        }
        ret->setObject(obj);
    }
    
    return true;
}


template<typename T>
bool native_ptr_to_rooted_seval(typename std::enable_if<!std::is_base_of<cocos2d::Ref,T>::value,T>::type* v, se::Class* cls, se::Value* ret, bool* isReturnCachedValue = nullptr)
{
    assert(ret != nullptr);
    if (v == nullptr)
    {
        ret->setNull();
        return true;
    }

    se::Object* obj = nullptr;
    auto iter = se::NativePtrToObjectMap::find(v);
    if (iter == se::NativePtrToObjectMap::end())
    { // If we couldn't find native object in map, then the native object is created from native code. e.g. TMXLayer::getTileAt
        assert(cls != nullptr);
        obj = se::Object::createObjectWithClass(cls);
        obj->root();
        obj->setPrivateData(v);

        if (isReturnCachedValue != nullptr)
        {
            *isReturnCachedValue = false;
        }
        // CCLOGWARN("WARNING: non-Ref type: (%s) isn't catched, se::Object:%p, native: %p", typeid(*v).name(), obj, v);
    }
    else
    {
        obj = iter->second;
        assert(obj->isRooted());
        if (isReturnCachedValue != nullptr)
        {
            *isReturnCachedValue = true;
        }
        // CCLOG("return cached object: %s, se::Object:%p, native: %p", typeid(*v).name(), obj, v);
    }

    ret->setObject(obj);
    return true;
}

template<typename T>
typename std::enable_if<std::is_base_of<cocos2d::Ref,T>::value, bool>::type
native_ptr_to_seval(T* vp, se::Value* ret, bool* isReturnCachedValue = nullptr)
{
    typedef typename std::decay<typename std::remove_const<T>::type>::type DecayT;
    DecayT *v = const_cast<DecayT*>(vp);
    assert(ret != nullptr);
    if (v == nullptr)
    {
        ret->setNull();
        return true;
    }

    se::Object* obj = nullptr;
    auto iter = se::NativePtrToObjectMap::find(v);
    if (iter == se::NativePtrToObjectMap::end())
    { // If we couldn't find native object in map, then the native object is created from native code. e.g. TMXLayer::getTileAt
//        CCLOGWARN("WARNING: Ref type: (%s) isn't catched!", typeid(*v).name());
        se::Class* cls = JSBClassType::findClass<T>(v);
        assert(cls != nullptr);
        obj = se::Object::createObjectWithClass(cls);
        ret->setObject(obj, true);
        obj->setPrivateData(v);
        v->retain(); // Retain the native object to unify the logic in finalize method of js object.
        if (isReturnCachedValue != nullptr)
        {
            *isReturnCachedValue = false;
        }
    }
    else
    {
        obj = iter->second;
//        CCLOG("INFO: Found Ref type: (%s, native: %p, se: %p) from cache!", typeid(*v).name(), v, obj);
        if (isReturnCachedValue != nullptr)
        {
            *isReturnCachedValue = true;
        }
        ret->setObject(obj);
    }

    return true;
}

template<typename T>
typename std::enable_if<std::is_base_of<cocos2d::Ref,T>::value,bool>::type
native_ptr_to_seval(T* vp, se::Class* cls, se::Value* ret, bool* isReturnCachedValue = nullptr)
{
    typedef typename std::decay<typename std::remove_const<T>::type>::type DecayT;
    DecayT *v = const_cast<DecayT*>(vp);
    assert(ret != nullptr);
    if (v == nullptr)
    {
        ret->setNull();
        return true;
    }

    se::Object* obj = nullptr;
    auto iter = se::NativePtrToObjectMap::find(v);
    if (iter == se::NativePtrToObjectMap::end())
    { // If we couldn't find native object in map, then the native object is created from native code. e.g. TMXLayer::getTileAt
//        CCLOGWARN("WARNING: Ref type: (%s) isn't catched!", typeid(*v).name());
        assert(cls != nullptr);
        obj = se::Object::createObjectWithClass(cls);
        ret->setObject(obj, true);
        obj->setPrivateData(v);
        v->retain(); // Retain the native object to unify the logic in finalize method of js object.
        if (isReturnCachedValue != nullptr)
        {
            *isReturnCachedValue = false;
        }
    }
    else
    {
        obj = iter->second;
        if (isReturnCachedValue != nullptr)
        {
            *isReturnCachedValue = true;
        }
        ret->setObject(obj);
    }

    return true;
}

template<typename T>
bool std_vector_to_seval(const std::vector<T>& v, se::Value* ret)
{
    assert(ret != nullptr);
    bool ok = true;
    se::HandleObject obj(se::Object::createArrayObject(v.size()));

    uint32_t i = 0;
    se::Value tmp;
    for (const auto& e : v)
    {
        native_ptr_to_seval(e, &tmp);
        obj->setArrayElement(i, tmp);
        ++i;
    }

    ret->setObject(obj, true);

    return ok;
}

template<typename T>
bool seval_to_reference(const se::Value& v, T** ret)
{
    assert(ret != nullptr);
    assert(v.isObject());
    *ret = (T*)v.toObject()->getPrivateData();
    return true;
}

/////////////////////////////////// helpers //////////////////////////////////////////////////////////

////////////////////////// is jsb object ///////////////////////////
template<typename T>
struct is_jsb_object : std::false_type
{};

template<typename T>
constexpr bool is_jsb_object_v = is_jsb_object<T>::value;

#define JSB_REGISTER_OBJECT_TYPE(T) \
    template<> \
    struct is_jsb_object<T> : std::true_type {}\




template<typename Out, typename In>
constexpr Out holder_convert_to(In& input) {
    if constexpr (std::is_same_v< Out, In>)
    {
        return input;
    }
    else if constexpr (std::is_same_v<Out, std::add_pointer_t<In>>) 
    {
        return &input;
    } 
    else if constexpr (std::is_same_v<Out, std::remove_pointer_t<In>>)
    {
        return *input;
    }
    else if constexpr (std::is_enum_v<In>) 
    {
        return (Out)input;
    }
    else {
        static_assert(false, "types are not convertiable!");
    }
}

template<typename T, bool is_reference>
struct HolderType {

    using type = T;
    using local_type = typename std::conditional_t<is_reference && is_jsb_object_v<T>, std::add_pointer_t<T>, T>;

    static constexpr inline auto value(local_type & arg) 
    { 
        return holder_convert_to<T, local_type>(arg);
    }
};

///////////////////////////////////convertion//////////////////////////////////////////////////////////


template<typename T>
inline bool sevalue_to_native(const se::Value &from, T* to)
{
    static_assert(false, "sevalue_to_native not implemented for type");
    return false;
}


template<typename T>
inline bool sevalue_to_native(const se::Value &from, std::enable_if_t<std::is_enum_v<T>, T>* to)
{
    typename std::underlying_type_t<T> tmp;
    *to = static_cast<T>sevalue_to_native(from, &tmp);
    return true;
}

//////////////// vector type

template<typename T, size_t CNT>
bool sevalue_to_native(const se::Value &from, std::array<T, CNT>* to)
{
    assert(from.toObject());
    se::Object *array = from.toObject();
    assert(array->isArray());
    uint32_t len = 0;
    array->getArrayLength(&len);
    se::Value tmp;
    assert(len >= CNT);
    for (uint32_t i = 0; i < CNT; i++)
    {
        array->getArrayElement(i, &tmp);
        sevalue_to_native(tmp, &(*to)[i]);
    }
    return true;
}

template<size_t CNT>
bool sevalue_to_native(const se::Value& from, std::array<uint8_t, CNT>* to)
{
    assert(from.toObject());
    se::Object* array = from.toObject();
    assert(array->isArray() || array->isArrayBuffer() ||array->isTypedArray());
    if (array->isTypedArray())
    {
        uint8_t* data = nullptr;
        size_t size = 0;
        array->getTypedArrayData(&data, &size);
        for (int i = 0; i < std::min(size, CNT); i++) {
            (*to)[i] = data[i];
        }
    }
    else if (array->isArrayBuffer())
    {
        uint8_t* data = nullptr;
        size_t size = 0;
        array->getArrayBufferData(&data, &size);
        for (int i = 0; i < std::min(size, CNT); i++) {
            (*to)[i] = data[i];
        }
    }
    else if(array->isArray())
    {
        uint32_t len = 0;
        array->getArrayLength(&len);
        se::Value tmp;
        assert(len >= CNT);
        for (uint32_t i = 0; i < CNT; i++)
        {
            array->getArrayElement(i, &tmp);
            sevalue_to_native(tmp, &(*to)[i]);
        }
    }
    else 
    {
        return false;
    }
    return true;
}


template<typename T>
bool sevalue_to_native(const se::Value &from, std::vector<T>* to)
{
    assert(from.toObject());
    se::Object *array = from.toObject();
    assert(array->isArray());
    uint32_t len = 0;
    array->getArrayLength(&len);
    to->resize(len);
    se::Value tmp;
    for(uint32_t i = 0; i < len; i++)
    {
        array->getArrayElement(i, &tmp);
        sevalue_to_native(tmp, &(*to)[i]);
    }
    return true;
}
template<>
inline bool sevalue_to_native(const se::Value &from, std::string * to)
{
    *to = from.toString();
    return true;
}

///// integers
template<>
inline bool sevalue_to_native(const se::Value &from, bool* to)
{
    *to = from.toBoolean();
    return true;
}

template<>
inline bool sevalue_to_native(const se::Value &from, int32_t * to)
{
    *to = from.toInt32();
    return true;
}
template<>
inline bool sevalue_to_native(const se::Value &from, uint32_t * to)
{
    *to = from.toUint32();
    return true;
}

template<>
inline bool sevalue_to_native(const se::Value &from, int16_t * to)
{
    *to = from.toInt16();
    return true;
}
template<>
inline bool sevalue_to_native(const se::Value &from, uint16_t * to)
{
    *to = from.toUint16();
    return true;
}

template<>
inline bool sevalue_to_native(const se::Value &from, int8_t * to)
{
    *to = from.toInt8();
    return true;
}
template<>
inline bool sevalue_to_native(const se::Value &from, uint8_t * to)
{
    *to = from.toUint8();
    return true;
}

template<>
inline bool sevalue_to_native(const se::Value &from, uint64_t *to)
{
    *to = from.toUIntptr_t();
    return true;
}

template<>
inline bool sevalue_to_native(const se::Value &from, float * to)
{
    *to = from.toFloat();
    return true;
}


template<>
bool sevalue_to_native(const se::Value &from, std::vector<uint8_t>* to);
////////////////// pointer types

template<typename T>
bool sevalue_to_native(const se::Value &from,  T ** to)
{
    static_assert(is_jsb_object_v<T>, "Only JSB object are accepted!");
    if (from.isNullOrUndefined()) {
        //const std::string stack = se::ScriptEngine::getInstance()->getCurrentStackTrace();
        //SE_LOGE("[ERROR] sevalue_to_native jsval is null/undefined: %s\nstack: %s", typeid(T).name(), stack.c_str());
        *to = nullptr;
        return true;
    }
    *to = (T*) from.toObject()->getPrivateData();
    return true;
}

template<>
bool sevalue_to_native(const se::Value &from, void ** to);

template<>
bool sevalue_to_native(const se::Value &from, std::string ** to);

///////////////////////////////////////////////////////////////////

template<typename T>
bool nativevalue_to_se(const T &from, se::Value &to) {
    if constexpr (std::is_enum_v<T>) {
        typedef typename std::underlying_type_t<T> enum_type;
        enum_type tmp = static_cast<enum_type>(from);
        //nativevalue_to_se(from, to);
        to.setInt32((int32_t)tmp);
        return true;
    }
    else if constexpr (std::is_pointer_v<T>) {
        //static_assert(false, "Convert ptr to seobject???");
        return native_ptr_to_seval(from, &to);
    }
    else if constexpr (is_jsb_object_v<T>) {
        //static_assert(false, "Convert ref to seobject???");
        return native_ptr_to_seval(from, &to);
    }
    else
    {
        static_assert(false, "nativevalue_to_se not implemented for type");
    }
    return false;
}


namespace cocos2d{
    struct GFXDrawInfo;
    struct GFXUniform;
}

template<typename T>
inline bool nativevalue_to_se(const std::vector<T>& from, se::Value& to)
{
    se::Object* array = se::Object::createArrayObject(from.size());
    se::Value tmp;
    for (size_t i = 0; i < from.size(); i++) {
        nativevalue_to_se(from[i], tmp);
        array->setArrayElement(i, tmp);
    }
    to.setObject(array);
    return true;
}

template<>
inline bool nativevalue_to_se(const std::vector<int8_t>& from, se::Value& to)
{
    se::Object* array = se::Object::createTypedArray(se::Object::TypedArrayType::INT8, from.data(), from.size());
    to.setObject(array);
    return true;
}

template<>
inline bool nativevalue_to_se(const std::vector<uint8_t>& from, se::Value& to)
{
    se::Object* array = se::Object::createTypedArray(se::Object::TypedArrayType::UINT8, from.data(), from.size());
    to.setObject(array);
    return true;
}



template<typename T, size_t N>
inline bool nativevalue_to_se(const std::array<T, N>& from, se::Value& to)
{
    se::Object* array = se::Object::createArrayObject(N);
    se::Value tmp;
    for (size_t i = 0; i < N; i++) {
        nativevalue_to_se(from[i], tmp);
        array->setArrayElement(i, tmp);
    }
    to.setObject(array);
    return true;
}

template<size_t N>
inline bool nativevalue_to_se(const std::array<uint8_t, N>& from, se::Value& to)
{
    se::Object* array = se::Object::createUint8TypedArray(from.data(), N);
    to.setObject(array);
    return true;
}

template<size_t N>
inline bool nativevalue_to_se(const std::array<uint16_t, N>& from, se::Value& to)
{
    se::Object* array = se::Object::createTypedArray(se::Object::TypedArrayType::INT16, from.data(), N * sizeof(uint16_t));
    to.setObject(array);
    return true;
}

template<size_t N>
inline bool nativevalue_to_se(const std::array<float, N>& from, se::Value& to)
{
    se::Object* array = se::Object::createTypedArray(se::Object::TypedArrayType::FLOAT32, from.data(), N * sizeof(float));
    to.setObject(array);
    return true;
}


template<>
inline bool nativevalue_to_se(const int32_t &from, se::Value &to)
{
    to.setInt32(from);
    return true;
}

template<>
inline bool nativevalue_to_se(const uint32_t &from, se::Value &to)
{
    to.setUint32(from);
    return true;
}
template<>
inline bool nativevalue_to_se(const int16_t &from, se::Value &to)
{
    to.setInt16(from);
    return true;
}
template<>
inline bool nativevalue_to_se(const uint16_t &from, se::Value &to)
{
    to.setUint16(from);
    return true;
}

template<>
inline bool nativevalue_to_se(const int8_t &from, se::Value &to)
{
    to.setInt8(from);
    return true;
}

template<>
inline bool nativevalue_to_se(const uint8_t &from, se::Value &to)
{
    to.setUint8(from);
    return true;
}

template<>
inline bool nativevalue_to_se(const std::string &from, se::Value &to)
{
    to.setString(from);
    return true;
}
template<>
inline bool nativevalue_to_se(const float &from, se::Value &to)
{
    to.setFloat(from);
    return true;
}
template<>
inline bool nativevalue_to_se(const bool& from, se::Value& to)
{
    to.setBoolean(from);
    return true;
}


/////////////////////// FIXME: remove all code bellow
///////////////// gfx type
namespace cocos2d {
    struct GFXCommandAllocatorInfo;
    class GFXContext;
    struct GFXBufferTextureCopy;
}
//template<>
//bool sevalue_to_native(const se::Value &from, cocos2d::GFXCommandAllocatorInfo** to);
JSB_REGISTER_OBJECT_TYPE(cocos2d::GFXContext);
JSB_REGISTER_OBJECT_TYPE(cocos2d::GFXCommandAllocatorInfo);
