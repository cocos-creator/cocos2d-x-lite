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
#include "ProgramLib.h"
#include "../gfx/Program.h"
#include "gfx/DeviceGraphics.h"

#include <regex>
#include <string>
#include <sstream>
#include <iostream>

namespace {
    uint32_t _shdID = 0;

    std::string generateDefines(const std::vector<cocos2d::ValueMap*>& definesList)
    {
        std::string ret;
        std::string v;
        cocos2d::ValueMap cache;
        for (int i = (int)definesList.size() - 1; i >= 0; i--)
        {
            cocos2d::ValueMap* defMap = definesList[i];
            for (const auto& def : *defMap)
            {
                if (cache.find(def.first) != cache.end())
                {
                    continue;
                }
                
                if (def.second.getType() == cocos2d::Value::Type::BOOLEAN)
                {
                    v = def.second.asBool() ? "1" : "0";
                }
                else
                {
                    v = std::to_string(def.second.asUnsignedInt());
                }
                
                ret += "#define "  + def.first + " " + v + "\n";
                
                cache.emplace(def.first, def.second);
            }
        }
       
        return ret;
    }

    std::string replaceMacroNums(const std::string str, const std::vector<cocos2d::ValueMap*>& definesList)
    {
        cocos2d::ValueMap cache;
        std::string tmp = str;
        for (int i = (int)definesList.size() - 1; i >= 0; i--)
        {
            cocos2d::ValueMap* defMap = definesList[i];

            for (const auto& def : *defMap)
            {
                if (cache.find(def.first) != cache.end())
                {
                    continue;
                }
                
                if (def.second.getType() == cocos2d::Value::Type::INTEGER || def.second.getType() == cocos2d::Value::Type::UNSIGNED)
                {
                    cache.emplace(def.first, def.second);
                }
            }
        }
        
        for (const auto& def : cache)
        {
            std::regex pattern(def.first);
            tmp = std::regex_replace(tmp, pattern, def.second.asString());
        }
        
        return tmp;
    }

    using RegexReplaceCallback = std::function<std::string(const std::match_results<std::string::const_iterator>&)>;

    static std::string regexReplaceString(const std::string& text, const std::string& pattern, const RegexReplaceCallback& replaceCallback)
    {
        std::string ret = text;
        std::regex re(pattern);

        std::string::const_iterator text_iter = text.cbegin();
        std::match_results<std::string::const_iterator> results;

        size_t start = 0;
        size_t offset = 0;
        while (std::regex_search(text_iter, text.end(), results, re))
        {
            offset = start + results.position();
            std::string replacement = replaceCallback(results);
            ret = ret.replace(offset, results.length(), replacement);
            start = offset + replacement.length();

            text_iter = results[0].second;
        }

        return ret;
    }

    std::string unrollLoops(const std::string& text)
    {
        auto func = [](const std::match_results<std::string::const_iterator>& results) -> std::string {
            assert(results.size() == 5);
            std::string snippet = results[4].str();
            std::string replacePatternStr = "\\{" + results[1].str() + "\\}";
            std::regex replacePattern(replacePatternStr);

            int32_t parsedBegin = atoi(results[2].str().c_str());
            int32_t parsedEnd = atoi(results[3].str().c_str());
            if (parsedBegin < 0 || parsedEnd < 0)
            {
                RENDERER_LOGE("Unroll For Loops Error: begin and end of range must be an int num.");
            }

            std::string unroll;
            char tmp[256] = {0};
            for (int32_t i = parsedBegin; i < parsedEnd; ++i)
            {
                snprintf(tmp, 256, "%d", i);
                std::string replaceFormat(tmp);
                unroll += std::regex_replace(snippet, replacePattern, replaceFormat);
            }
            return unroll;
        };

        return regexReplaceString(text, "#pragma for (\\w+) in range\\(\\s*(\\d+)\\s*,\\s*(\\d+)\\s*\\)([\\s\\S]+?)#pragma endFor", func);
    }
}

std::string test_unrollLoops(const std::string& text)
{
    return unrollLoops(text);
}

RENDERER_BEGIN

ProgramLib::ProgramLib(DeviceGraphics* device, std::vector<Template>& templates)
: _device(device)
{
    RENDERER_SAFE_RETAIN(_device);
    
    for (auto& templ : templates)
        define(templ.name, templ.vert, templ.frag, templ.defines);
}

ProgramLib::~ProgramLib()
{
    RENDERER_SAFE_RELEASE(_device);
    _device = nullptr;
}

void ProgramLib::define(const std::string& name, const std::string& vert, const std::string& frag, ValueVector& defines)
{
    auto iter = _templates.find(name);
    if (iter != _templates.end())
    {
        RENDERER_LOGW("Failed to define shader %s: already exists.", name.c_str());
        return;
    }

    uint32_t id = ++_shdID;

    // calculate option mask offset
    uint32_t offset = 0;
    for (auto& def : defines)
    {
        ValueMap& oneDefMap = def.asValueMap();
        uint32_t cnt = 1;

        //IDEA: currently we don't use min, max.
//        if (def.min != -1 && def.max != -1) {
//            cnt = (uint32_t)std::ceil((def.max - def.min) * 0.5);
//
//            def._map = std::bind(&Option::_map_1, def, std::placeholders::_1);
//        }
//        else
//        {
//            def._map = std::bind(&Option::_map_2, def, std::placeholders::_1);
//        }

        offset += cnt;

        oneDefMap["_offset"] = offset;
    }

    std::string newVert = vert;
    std::string newFrag = frag;
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
    std::string::size_type pos = 0;
    pos = newVert.find(_precisionVert);
    if (pos != std::string::npos)
    {
        newVert.replace(pos, strlen(_precisionVert), "");
    }
    
    pos = newFrag.find(_precisionFrag);
    if (pos != std::string::npos)
    {
        newFrag.replace(pos, strlen(_precisionFrag), "");
    }
    
    while((pos = newVert.find(_mediump)) != std::string::npos)
    {
        newVert.replace(pos, strlen(_mediump), "");
    }
    
    while((pos = newFrag.find(_mediump)) != std::string::npos)
    {
        newFrag.replace(pos, strlen(_mediump), "");
    }
    
    while((pos = newVert.find(_lowp)) != std::string::npos)
    {
        newVert.replace(pos, strlen(_lowp), "");
    }
    
    while((pos = newFrag.find(_lowp)) != std::string::npos)
    {
        newFrag.replace(pos, strlen(_lowp), "");
    }
#else
    std::string::size_type pos = 0;
    pos = newVert.find(_precisionVert);
    if (pos == std::string::npos)
    {
        newVert = _precisionVertReplace + vert;
    }
    
    pos = newVert.find(_precisionFrag);
    if (pos == std::string::npos)
    {
        newFrag = _precisionFragReplace + frag;
    }
#endif
    
    // store it
    auto& templ = _templates[name];
    templ.id = id;
    templ.name = name;
    templ.vert = newVert;
    templ.frag = newFrag;
    templ.defines = defines;
}

const std::string& ProgramLib::getKey(const std::string& name, const std::vector<ValueMap*>& definesList, const std::string& definesKey)
{
    auto iter = _templates.find(name);
    assert(iter != _templates.end());
    
    static std::string key = "";
    // return key << 8 | tmpl.id;
    // key number maybe bigger than 32 bit, need use string to store value.
    key = std::to_string(iter->second.id) + ":" + definesKey;
    return key;
}

Program* ProgramLib::getProgram(const std::string& name, const std::vector<ValueMap*>& definesList, const std::string& definesKey)
{
    const std::string& key = getKey(name, definesList, definesKey);
    auto iter = _cache.find(key);
    if (iter != _cache.end()) {
        iter->second->retain();
        return iter->second;
    }

    Program* program = nullptr;
    // get template
    auto templIter = _templates.find(name);
    if (templIter != _templates.end())
    {
        const auto& tmpl = templIter->second;
        std::string customDef = generateDefines(definesList) + "\n";
        std::string vert = replaceMacroNums(tmpl.vert, definesList);
        vert = customDef + unrollLoops(vert);
        std::string frag = replaceMacroNums(tmpl.frag, definesList);
        frag = customDef + unrollLoops(frag);
        
        program = new Program();
        program->init(_device, vert.c_str(), frag.c_str());
        program->link();
        _cache.emplace(key, program);
        program->setKey(key);
    }

    return program;
}

const Value& ProgramLib::getValueFromDefineList(const std::string& name, const std::vector<ValueMap*>& definesList)
{
    for (int i = (int)definesList.size() - 1; i >= 0; i--)
    {
        ValueMap* defines = definesList[i];
        auto iter = defines->find(name);
        if (iter != defines->end())
        {
            return iter->second;
        }
    }
    
    return Value::Null;
}

uint32_t ProgramLib::getValueKey(const Value &v)
{
    if (v.getType() == Value::Type::BOOLEAN)
    {
        return v.asBool() ? 1 : 0;
    }
    else
    {
        return v.asUnsignedInt();
    }
}

RENDERER_END
