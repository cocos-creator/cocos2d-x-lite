/****************************************************************************
 Copyright (c) 2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "CustomProperties.hpp"

RENDERER_BEGIN
CustomProperties::CustomProperties()
{
}

CustomProperties::~CustomProperties()
{
    _properties.clear();
    _defines.clear();
}

void CustomProperties::setProperty(const std::string name, const Property& property)
{
    auto iter = _properties.find(name);
    if (iter != _properties.end() && iter->second == property)
    {
        return;
    }
    _properties[name] = property;
    _dirty = true;
}

const CustomProperties::Property& CustomProperties::getProperty(std::string name) const
{
    static Property EMPTY_PROPERTY;
    if (_properties.end() == _properties.find(name))
        return EMPTY_PROPERTY;
    else
        return _properties.at(name);
}

void CustomProperties::define(const std::string& name, const Value& value)
{
    if (_defines[name] == value)
    {
        return;
    };

    _dirty = true;
    _defines[name] = value;
    
    generateKey();
}

Value CustomProperties::getDefine(const std::string& name) const
{
    auto iter = _defines.find(name);
    if (iter != _defines.end())
    {
        return iter->second;
    }
    
    RENDERER_LOGW("Failed to get CustomProperties define %s, define not found.", name.c_str());
    return Value::Null;
}

void CustomProperties::generateKey()
{
    _definesKey = "";
    
    uint32_t key = 0;
    uint32_t offset = 0;
    for (const auto& tmplDef : _defines)
    {
        uint32_t vkey = ProgramLib::getValueKey(tmplDef.second);
        
        key |= vkey << offset;
        
        if (tmplDef.second.getType() != Value::Type::BOOLEAN)
        {
            offset += ceil(log2(vkey));
        }
        else
        {
            offset += 1;
        }
    }
    
    _definesKey = std::to_string(key);
}

std::unordered_map<std::string, CustomProperties::Property>* CustomProperties::extractProperties()
{
    return &_properties;
}

ValueMap* CustomProperties::extractDefines()
{
    return &_defines;
}

RENDERER_END
