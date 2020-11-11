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

#include "bindings/jswrapper/SeApi.h"
#include "bindings/manual/jsb_classtype.h"

#include "cocos2d.h"
#include "extensions/cocos-ext.h"
#include "network/Downloader.h"

namespace cc {
namespace gfx {
struct Rect;
struct Viewport;
struct Color;
struct Offset;
struct Extent;
struct TextureSubres;
struct TextureCopy;
struct BufferTextureCopy;
struct BufferInfo;
struct BufferViewInfo;
struct TextureInfo;
struct DescriptorSetInfo;
struct BindingMappingInfo;
struct ShaderStage;
struct UniformSampler;
struct UniformBlock;
struct Uniform;
struct ShaderInfo;
struct DrawInfo;
struct IndirectBuffer;
struct SamplerInfo;
}
} // namespace cc

//#include "Box2D/Box2D.h"

#define SE_PRECONDITION2_VOID(condition, ...)                                                           \
    do {                                                                                                \
        if (!(condition)) {                                                                             \
            SE_LOGE("jsb: ERROR: File %s: Line: %d, Function: %s\n", __FILE__, __LINE__, __FUNCTION__); \
            SE_LOGE(__VA_ARGS__);                                                                       \
            return;                                                                                     \
        }                                                                                               \
    } while (0)

#define SE_PRECONDITION2(condition, ret_value, ...)                                                     \
    do {                                                                                                \
        if (!(condition)) {                                                                             \
            SE_LOGE("jsb: ERROR: File %s: Line: %d, Function: %s\n", __FILE__, __LINE__, __FUNCTION__); \
            SE_LOGE(__VA_ARGS__);                                                                       \
            return (ret_value);                                                                         \
        }                                                                                               \
    } while (0)

#define SE_PRECONDITION3(condition, ret_value, failed_code) \
    do {                                                    \
        if (!(condition)) {                                 \
            failed_code;                                    \
            return (ret_value);                             \
        }                                                   \
    } while (0)

#define SE_PRECONDITION4(condition, ret_value, errorCode)                                               \
    do {                                                                                                \
        if (!(condition)) {                                                                             \
            SE_LOGE("jsb: ERROR: File %s: Line: %d, Function: %s\n", __FILE__, __LINE__, __FUNCTION__); \
            __glErrorCode = errorCode;                                                                  \
            return (ret_value);                                                                         \
        }                                                                                               \
    } while (0)

#define SE_PRECONDITION_ERROR_BREAK(condition, ...)                                                 \
    if (!(condition)) {                                                                             \
        SE_LOGE("jsb: ERROR: File %s: Line: %d, Function: %s\n", __FILE__, __LINE__, __FUNCTION__); \
        SE_LOGE(__VA_ARGS__);                                                                       \
        break;                                                                                      \
    }

// se value -> native value
bool seval_to_int32(const se::Value &v, int32_t *ret);
bool seval_to_uint32(const se::Value &v, uint32_t *ret);
bool seval_to_uint(const se::Value &v, unsigned int *ret);
bool seval_to_int8(const se::Value &v, int8_t *ret);
bool seval_to_uint8(const se::Value &v, uint8_t *ret);
bool seval_to_int16(const se::Value &v, int16_t *ret);
bool seval_to_uint16(const se::Value &v, uint16_t *ret);
bool seval_to_boolean(const se::Value &v, bool *ret);
bool seval_to_float(const se::Value &v, float *ret);
bool seval_to_double(const se::Value &v, double *ret);
bool seval_to_long(const se::Value &v, long *ret);
bool seval_to_ulong(const se::Value &v, unsigned long *ret);
bool seval_to_longlong(const se::Value &v, long long *ret);
bool seval_to_size(const se::Value &v, size_t *ret);
bool seval_to_std_string(const se::Value &v, std::string *ret);
bool seval_to_Vec2(const se::Value &v, cc::Vec2 *pt);
bool seval_to_Vec3(const se::Value &v, cc::Vec3 *pt);
bool seval_to_Vec4(const se::Value &v, cc::Vec4 *pt);
bool seval_to_Mat4(const se::Value &v, cc::Mat4 *mat);
bool seval_to_Size(const se::Value &v, cc::Size *size);
bool seval_to_ccvalue(const se::Value &v, cc::Value *ret);
bool seval_to_ccvaluemap(const se::Value &v, cc::ValueMap *ret);
bool seval_to_ccvaluemapintkey(const se::Value &v, cc::ValueMapIntKey *ret);
bool seval_to_ccvaluevector(const se::Value &v, cc::ValueVector *ret);
bool sevals_variadic_to_ccvaluevector(const se::ValueArray &args, cc::ValueVector *ret);
bool seval_to_std_vector_string(const se::Value &v, std::vector<std::string> *ret);
bool seval_to_std_vector_int(const se::Value &v, std::vector<int> *ret);
bool seval_to_std_vector_uint16(const se::Value &v, std::vector<uint16_t> *ret);
bool seval_to_std_vector_float(const se::Value &v, std::vector<float> *ret);
bool seval_to_std_vector_Vec2(const se::Value &v, std::vector<cc::Vec2> *ret);
bool seval_to_Uint8Array(const se::Value &v, uint8_t *ret);
bool seval_to_uintptr_t(const se::Value &v, uintptr_t *ret);

bool seval_to_std_map_string_string(const se::Value &v, std::map<std::string, std::string> *ret);
bool seval_to_Data(const se::Value &v, cc::Data *ret);
bool seval_to_DownloaderHints(const se::Value &v, cc::network::DownloaderHints *ret);

bool seval_to_gfx_rect(const se::Value &v, cc::gfx::Rect *rect);
bool seval_to_gfx_viewport(const se::Value &v, cc::gfx::Viewport *viewport);
bool seval_to_gfx_color(const se::Value &v, cc::gfx::Color *color);
bool seval_to_gfx_color_list(const se::Value &v, cc::vector<cc::gfx::Color> *colorList);
bool seval_to_gfx_offset(const se::Value &v, cc::gfx::Offset *offset);
bool seval_to_gfx_extent(const se::Value &v, cc::gfx::Extent *extent);
bool seval_to_gfx_texture_subres(const se::Value &v, cc::gfx::TextureSubres *textureSubres);
bool seval_to_gfx_texture_copy(const se::Value &v, cc::gfx::TextureCopy *textureCopy);
bool seval_to_gfx_buffer_texture_copy(const se::Value &v, cc::gfx::BufferTextureCopy *bufferTextureCopy);
bool seval_to_gfx_buffer_texture_copy_list(const se::Value &v, cc::vector<cc::gfx::BufferTextureCopy> *bufferTextureCopyList);
bool seval_to_gfx_buffer_info(const se::Value &v, cc::gfx::BufferInfo *bufferInfo);
bool seval_to_gfx_buffer_view_info(const se::Value &v, cc::gfx::BufferViewInfo *bufferViewInfo);
bool seval_to_gfx_texture_info(const se::Value &v, cc::gfx::TextureInfo *textureInfo);
bool seval_to_gfx_descriptor_set_info(const se::Value &v, cc::gfx::DescriptorSetInfo *descriptorSetInfo);
bool seval_to_gfx_binding_mapping_info(const se::Value &v, cc::gfx::BindingMappingInfo *bindingMappingInfo);
bool seval_to_gfx_shader_stage(const se::Value &v, cc::gfx::ShaderStage *shaderStage);
bool seval_to_gfx_uniform_sampler(const se::Value &v, cc::gfx::UniformSampler *uniformSampler);
bool seval_to_gfx_uniform_block(const se::Value &v, cc::gfx::UniformBlock *uniformBlock);
bool seval_to_gfx_uniform(const se::Value &v, cc::gfx::Uniform *uniform);
bool seval_to_gfx_shader_info(const se::Value &v, cc::gfx::ShaderInfo *shaderInfo);
bool seval_to_gfx_draw_info(const se::Value &v, cc::gfx::DrawInfo *drawInfo);
bool seval_to_gfx_indirect_buffer(const se::Value &v, cc::gfx::IndirectBuffer *indirectBuffer);
bool seval_to_gfx_sampler_info(const se::Value &v, cc::gfx::SamplerInfo *samplerInfo);

#if USE_GFX_RENDERER
#endif

template <typename T>
bool seval_to_native_ptr(const se::Value &v, T *ret) {
    assert(ret != nullptr);

    if (v.isObject()) {
        T ptr = (T)v.toObject()->getPrivateData();
        if (ptr == nullptr) {
            // This should never happen, return 'false' to mark the conversion fails.
            *ret = nullptr;
            return false;
        }

        *ret = ptr;
        return true;
    } else if (v.isNullOrUndefined()) {
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

template <typename T>
bool seval_to_Vector(const se::Value &v, cc::Vector<T> *ret) {
    assert(ret != nullptr);
    assert(v.isObject());
    se::Object *obj = v.toObject();
    assert(obj->isArray());

    bool ok = true;
    uint32_t len = 0;
    ok = obj->getArrayLength(&len);
    if (!ok) {
        ret->clear();
        return false;
    }

    se::Value tmp;
    for (uint32_t i = 0; i < len; ++i) {
        ok = obj->getArrayElement(i, &tmp);
        if (!ok || !tmp.isObject()) {
            ret->clear();
            return false;
        }

        T nativeObj = (T)tmp.toObject()->getPrivateData();

        ret->pushBack(nativeObj);
    }

    return true;
}

template <typename T>
typename std::enable_if<std::is_class<T>::value && !std::is_same<T, std::string>::value, T>::type
seval_to_type(const se::Value &v, bool &ok) {
    if (!v.isObject()) {
        ok = false;
        return T();
    }
    T *nativeObj = (T *)v.toObject()->getPrivateData();
    ok = true;
    return *nativeObj;
}
template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
seval_to_type(const se::Value &v, bool &ok) {
    if (!v.isNumber()) {
        ok = false;
        return 0;
    }
    ok = true;
    return v.toInt32();
}

template <typename T>
typename std::enable_if<std::is_enum<T>::value, T>::type
seval_to_type(const se::Value &v, bool &ok) {
    if (!v.isNumber()) {
        ok = false;
        return (T)0;
    }
    ok = true;
    return (T)v.toInt32();
}

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
seval_to_type(const se::Value &v, bool &ok) {
    if (!v.isNumber()) {
        ok = false;
        return 0;
    }
    ok = true;
    return v.toFloat();
}

template <typename T>
typename std::enable_if<std::is_same<T, std::string>::value, T>::type
seval_to_type(const se::Value &v, bool &ok) {
    if (!v.isString()) {
        ok = false;
        return "";
    }
    ok = true;
    return v.toString();
}

template <typename T>
typename std::enable_if<std::is_pointer<T>::value && std::is_class<typename std::remove_pointer<T>::type>::value, bool>::type
seval_to_std_vector(const se::Value &v, std::vector<T> *ret) {
    assert(ret != nullptr);
    assert(v.isObject());
    se::Object *obj = v.toObject();
    assert(obj->isArray());

    bool ok = true;
    uint32_t len = 0;
    ok = obj->getArrayLength(&len);
    if (!ok) {
        ret->clear();
        return false;
    }

    ret->resize(len);

    se::Value tmp;
    for (uint32_t i = 0; i < len; ++i) {
        ok = obj->getArrayElement(i, &tmp);
        if (!ok) {
            ret->clear();
            return false;
        }

        if (tmp.isObject()) {
            T nativeObj = (T)tmp.toObject()->getPrivateData();
            (*ret)[i] = nativeObj;
        } else if (tmp.isNullOrUndefined()) {
            (*ret)[i] = nullptr;
        } else {
            ret->clear();
            return false;
        }
    }

    return true;
}

template <typename T>
typename std::enable_if<!std::is_pointer<T>::value, bool>::type
seval_to_std_vector(const se::Value &v, std::vector<T> *ret) {
    assert(ret != nullptr);
    assert(v.isObject());
    se::Object *obj = v.toObject();
    assert(obj->isArray());

    bool ok = true;
    uint32_t len = 0;
    ok = obj->getArrayLength(&len);
    if (!ok) {
        ret->clear();
        return false;
    }

    ret->resize(len);

    se::Value tmp;
    for (uint32_t i = 0; i < len; ++i) {
        ok = obj->getArrayElement(i, &tmp);
        if (!ok) {
            ret->clear();
            return false;
        }
        (*ret)[i] = seval_to_type<T>(tmp, ok);
        if (!ok) {
            return false;
        }
    }

    return true;
}

template <typename T>
bool seval_to_Map_string_key(const se::Value &v, cc::Map<std::string, T> *ret) {
    assert(ret != nullptr);
    assert(v.isObject());
    se::Object *obj = v.toObject();

    std::vector<std::string> allKeys;
    bool ok = obj->getAllKeys(&allKeys);
    if (!ok) {
        ret->clear();
        return false;
    }

    se::Value tmp;
    for (const auto &key : allKeys) {
        ok = obj->getProperty(key.c_str(), &tmp);
        if (!ok || !tmp.isObject()) {
            ret->clear();
            return false;
        }

        T nativeObj = (T)tmp.toObject()->getPrivateData();
        ret->insert(key, nativeObj);
    }

    return true;
}

// native value -> se value
bool int8_to_seval(int8_t v, se::Value *ret);
bool uint8_to_seval(uint8_t v, se::Value *ret);
bool int32_to_seval(int32_t v, se::Value *ret);
bool uint32_to_seval(uint32_t v, se::Value *ret);
bool int16_to_seval(uint16_t v, se::Value *ret);
bool uint16_to_seval(uint16_t v, se::Value *ret);
bool boolean_to_seval(bool v, se::Value *ret);
bool float_to_seval(float v, se::Value *ret);
bool double_to_seval(double v, se::Value *ret);
bool long_to_seval(long v, se::Value *ret);
bool ulong_to_seval(unsigned long v, se::Value *ret);
bool longlong_to_seval(long long v, se::Value *ret);
bool uintptr_t_to_seval(uintptr_t v, se::Value *ret);
bool size_to_seval(size_t v, se::Value *ret);
bool std_string_to_seval(const std::string &v, se::Value *ret);

bool Vec2_to_seval(const cc::Vec2 &v, se::Value *ret);
bool Vec3_to_seval(const cc::Vec3 &v, se::Value *ret);
bool Vec4_to_seval(const cc::Vec4 &v, se::Value *ret);
bool Mat4_to_seval(const cc::Mat4 &v, se::Value *ret);
bool Size_to_seval(const cc::Size &v, se::Value *ret);
bool Rect_to_seval(const cc::Rect &v, se::Value *ret);
bool ccvalue_to_seval(const cc::Value &v, se::Value *ret);
bool ccvaluemap_to_seval(const cc::ValueMap &v, se::Value *ret);
bool ccvaluemapintkey_to_seval(const cc::ValueMapIntKey &v, se::Value *ret);
bool ccvaluevector_to_seval(const cc::ValueVector &v, se::Value *ret);
bool std_vector_string_to_seval(const std::vector<std::string> &v, se::Value *ret);
bool std_vector_int_to_seval(const std::vector<int> &v, se::Value *ret);
bool std_vector_uint16_to_seval(const std::vector<uint16_t> &v, se::Value *ret);
bool std_vector_float_to_seval(const std::vector<float> &v, se::Value *ret);
bool std_map_string_string_to_seval(const std::map<std::string, std::string> &v, se::Value *ret);

bool ManifestAsset_to_seval(const cc::extension::ManifestAsset &v, se::Value *ret);
bool Data_to_seval(const cc::Data &v, se::Value *ret);
bool DownloadTask_to_seval(const cc::network::DownloadTask &v, se::Value *ret);

#if USE_GFX_RENDERER
#endif

template <typename T>
typename std::enable_if<!std::is_base_of<cc::Ref, T>::value, bool>::type
native_ptr_to_seval(T *v_c, se::Value *ret, bool *isReturnCachedValue = nullptr) {
    typedef typename std::decay<typename std::remove_const<T>::type>::type DecayT;
    DecayT *v = const_cast<DecayT *>(v_c);
    assert(ret != nullptr);
    if (v == nullptr) {
        ret->setNull();
        return true;
    }

    se::Object *obj = nullptr;
    auto iter = se::NativePtrToObjectMap::find(v);
    if (iter == se::NativePtrToObjectMap::end()) { // If we couldn't find native object in map, then the native object is created from native code. e.g. TMXLayer::getTileAt
        // CC_LOG_DEBUGWARN("WARNING: non-Ref type: (%s) isn't catched!", typeid(*v).name());
        se::Class *cls = JSBClassType::findClass<T>(v);
        assert(cls != nullptr);
        obj = se::Object::createObjectWithClass(cls);
        ret->setObject(obj, true);
        obj->setPrivateData(v);
        if (isReturnCachedValue != nullptr) {
            *isReturnCachedValue = false;
        }
    } else {
        obj = iter->second;
        if (isReturnCachedValue != nullptr) {
            *isReturnCachedValue = true;
        }
        ret->setObject(obj);
    }

    return true;
}

//handle reference
template <typename T>
typename std::enable_if<!std::is_base_of<cc::Ref, T>::value && !std::is_pointer<T>::value, bool>::type
native_ptr_to_seval(T &v_ref, se::Value *ret, bool *isReturnCachedValue = nullptr) {
    typedef typename std::decay<typename std::remove_const<decltype(v_ref)>::type>::type DecayT;
    DecayT *v = const_cast<DecayT *>(&v_ref);

    assert(ret != nullptr);
    if (v == nullptr) {
        ret->setNull();
        return true;
    }

    se::Object *obj = nullptr;
    auto iter = se::NativePtrToObjectMap::find(v);
    if (iter == se::NativePtrToObjectMap::end()) { // If we couldn't find native object in map, then the native object is created from native code. e.g. TMXLayer::getTileAt
        // CC_LOG_DEBUGWARN("WARNING: non-Ref type: (%s) isn't catched!", typeid(*v).name());
        se::Class *cls = JSBClassType::findClass<DecayT>(v);
        assert(cls != nullptr);
        obj = se::Object::createObjectWithClass(cls);
        ret->setObject(obj, true);
        obj->setPrivateData(v);
        if (isReturnCachedValue != nullptr) {
            *isReturnCachedValue = false;
        }
    } else {
        obj = iter->second;
        if (isReturnCachedValue != nullptr) {
            *isReturnCachedValue = true;
        }
        ret->setObject(obj);
    }

    return true;
}

template <typename T>
bool native_ptr_to_rooted_seval(const typename std::enable_if<!std::is_base_of<cc::Ref, T>::value, T>::type *v, se::Value *ret, bool *isReturnCachedValue = nullptr) {
    assert(ret != nullptr);
    if (v == nullptr) {
        ret->setNull();
        return true;
    }

    se::Object *obj = nullptr;
    auto iter = se::NativePtrToObjectMap::find((void *)v);
    if (iter == se::NativePtrToObjectMap::end()) { // If we couldn't find native object in map, then the native object is created from native code. e.g. TMXLayer::getTileAt
        se::Class *cls = JSBClassType::findClass<T>(v);
        assert(cls != nullptr);
        obj = se::Object::createObjectWithClass(cls);
        obj->root();
        obj->setPrivateData((void *)v);

        if (isReturnCachedValue != nullptr) {
            *isReturnCachedValue = false;
        }
        // CC_LOG_DEBUGWARN("WARNING: non-Ref type: (%s) isn't catched!", typeid(*v).name());
    } else {
        obj = iter->second;
        assert(obj->isRooted());
        if (isReturnCachedValue != nullptr) {
            *isReturnCachedValue = true;
        }
        // CC_LOG_DEBUG("return cached object: %s, se::Object:%p, native: %p", typeid(*v).name(), obj, v);
    }

    ret->setObject(obj);
    return true;
}

template <typename T>
typename std::enable_if<!std::is_base_of<cc::Ref, T>::value, bool>::type
native_ptr_to_seval(T *vp, se::Class *cls, se::Value *ret, bool *isReturnCachedValue = nullptr) {
    typedef typename std::decay<typename std::remove_const<T>::type>::type DecayT;
    DecayT *v = const_cast<DecayT *>(vp);
    assert(ret != nullptr);
    if (v == nullptr) {
        ret->setNull();
        return true;
    }

    se::Object *obj = nullptr;
    auto iter = se::NativePtrToObjectMap::find(v);
    if (iter == se::NativePtrToObjectMap::end()) { // If we couldn't find native object in map, then the native object is created from native code. e.g. TMXLayer::getTileAt
                                                   //        CC_LOG_DEBUGWARN("WARNING: Ref type: (%s) isn't catched!", typeid(*v).name());
        assert(cls != nullptr);
        obj = se::Object::createObjectWithClass(cls);
        ret->setObject(obj, true);
        obj->setPrivateData(v);

        if (isReturnCachedValue != nullptr) {
            *isReturnCachedValue = false;
        }
    } else {
        obj = iter->second;
        if (isReturnCachedValue != nullptr) {
            *isReturnCachedValue = true;
        }
        ret->setObject(obj);
    }

    return true;
}

//handle ref
template <typename T>
typename std::enable_if<!std::is_base_of<cc::Ref, T>::value, bool>::type
native_ptr_to_seval(T &v_ref, se::Class *cls, se::Value *ret, bool *isReturnCachedValue = nullptr) {
    typedef typename std::decay<typename std::remove_const<decltype(v_ref)>::type>::type DecayT;
    DecayT *v = const_cast<DecayT *>(&v_ref);

    assert(ret != nullptr);
    if (v == nullptr) {
        ret->setNull();
        return true;
    }

    se::Object *obj = nullptr;
    auto iter = se::NativePtrToObjectMap::find(v);
    if (iter == se::NativePtrToObjectMap::end()) { // If we couldn't find native object in map, then the native object is created from native code. e.g. TMXLayer::getTileAt
        //        CC_LOG_DEBUGWARN("WARNING: Ref type: (%s) isn't catched!", typeid(*v).name());
        assert(cls != nullptr);
        obj = se::Object::createObjectWithClass(cls);
        ret->setObject(obj, true);
        obj->setPrivateData(v);

        if (isReturnCachedValue != nullptr) {
            *isReturnCachedValue = false;
        }
    } else {
        obj = iter->second;
        if (isReturnCachedValue != nullptr) {
            *isReturnCachedValue = true;
        }
        ret->setObject(obj);
    }

    return true;
}

template <typename T>
bool native_ptr_to_rooted_seval(typename std::enable_if<!std::is_base_of<cc::Ref, T>::value, T>::type *v, se::Class *cls, se::Value *ret, bool *isReturnCachedValue = nullptr) {
    assert(ret != nullptr);
    if (v == nullptr) {
        ret->setNull();
        return true;
    }

    se::Object *obj = nullptr;
    auto iter = se::NativePtrToObjectMap::find(v);
    if (iter == se::NativePtrToObjectMap::end()) { // If we couldn't find native object in map, then the native object is created from native code. e.g. TMXLayer::getTileAt
        assert(cls != nullptr);
        obj = se::Object::createObjectWithClass(cls);
        obj->root();
        obj->setPrivateData(v);

        if (isReturnCachedValue != nullptr) {
            *isReturnCachedValue = false;
        }
        // CC_LOG_DEBUGWARN("WARNING: non-Ref type: (%s) isn't catched, se::Object:%p, native: %p", typeid(*v).name(), obj, v);
    } else {
        obj = iter->second;
        assert(obj->isRooted());
        if (isReturnCachedValue != nullptr) {
            *isReturnCachedValue = true;
        }
        // CC_LOG_DEBUG("return cached object: %s, se::Object:%p, native: %p", typeid(*v).name(), obj, v);
    }

    ret->setObject(obj);
    return true;
}

template <typename T>
typename std::enable_if<std::is_base_of<cc::Ref, T>::value, bool>::type
native_ptr_to_seval(T *vp, se::Value *ret, bool *isReturnCachedValue = nullptr) {
    typedef typename std::decay<typename std::remove_const<T>::type>::type DecayT;
    DecayT *v = const_cast<DecayT *>(vp);
    assert(ret != nullptr);
    if (v == nullptr) {
        ret->setNull();
        return true;
    }

    se::Object *obj = nullptr;
    auto iter = se::NativePtrToObjectMap::find(v);
    if (iter == se::NativePtrToObjectMap::end()) { // If we couldn't find native object in map, then the native object is created from native code. e.g. TMXLayer::getTileAt
                                                   //        CC_LOG_DEBUGWARN("WARNING: Ref type: (%s) isn't catched!", typeid(*v).name());
        se::Class *cls = JSBClassType::findClass<T>(v);
        assert(cls != nullptr);
        obj = se::Object::createObjectWithClass(cls);
        ret->setObject(obj, true);
        obj->setPrivateData(v);
        v->retain(); // Retain the native object to unify the logic in finalize method of js object.
        if (isReturnCachedValue != nullptr) {
            *isReturnCachedValue = false;
        }
    } else {
        obj = iter->second;
        //        CC_LOG_DEBUG("INFO: Found Ref type: (%s, native: %p, se: %p) from cache!", typeid(*v).name(), v, obj);
        if (isReturnCachedValue != nullptr) {
            *isReturnCachedValue = true;
        }
        ret->setObject(obj);
    }

    return true;
}

template <typename T>
typename std::enable_if<std::is_base_of<cc::Ref, T>::value, bool>::type
native_ptr_to_seval(T *vp, se::Class *cls, se::Value *ret, bool *isReturnCachedValue = nullptr) {
    typedef typename std::decay<typename std::remove_const<T>::type>::type DecayT;
    DecayT *v = const_cast<DecayT *>(vp);
    assert(ret != nullptr);
    if (v == nullptr) {
        ret->setNull();
        return true;
    }

    se::Object *obj = nullptr;
    auto iter = se::NativePtrToObjectMap::find(v);
    if (iter == se::NativePtrToObjectMap::end()) { // If we couldn't find native object in map, then the native object is created from native code. e.g. TMXLayer::getTileAt
                                                   //        CC_LOG_DEBUGWARN("WARNING: Ref type: (%s) isn't catched!", typeid(*v).name());
        assert(cls != nullptr);
        obj = se::Object::createObjectWithClass(cls);
        ret->setObject(obj, true);
        obj->setPrivateData(v);
        v->retain(); // Retain the native object to unify the logic in finalize method of js object.
        if (isReturnCachedValue != nullptr) {
            *isReturnCachedValue = false;
        }
    } else {
        obj = iter->second;
        if (isReturnCachedValue != nullptr) {
            *isReturnCachedValue = true;
        }
        ret->setObject(obj);
    }

    return true;
}

template <typename T>
bool std_vector_to_seval(const std::vector<T> &v, se::Value *ret) {
    assert(ret != nullptr);
    bool ok = true;
    se::HandleObject obj(se::Object::createArrayObject(v.size()));

    uint32_t i = 0;
    se::Value tmp;
    for (const auto &e : v) {
        native_ptr_to_seval(e, &tmp);
        obj->setArrayElement(i, tmp);
        ++i;
    }

    ret->setObject(obj, true);

    return ok;
}

template <typename T>
bool seval_to_reference(const se::Value &v, T **ret) {
    assert(ret != nullptr);
    assert(v.isObject());
    *ret = (T *)v.toObject()->getPrivateData();
    return true;
}
