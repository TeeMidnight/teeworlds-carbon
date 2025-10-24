/*
 * This file is part of NewTeeworldsCN, a modified version of Teeworlds.
 *
 * Copyright (C) 2025 NewTeeworldsCN
 *
 * This software is provided 'as-is', under the zlib License.
 * See license.txt in the root of the distribution for more information.
 * If you are missing that file, acquire a complete release at github.com/NewTeeworldsCN/teeworlds-carbon
 */
#ifndef GAME_SERVER_CONFIG_H
#define GAME_SERVER_CONFIG_H

#include <base/system.h>
#include <base/system_new.hpp>
#include <base/types.h>
#include <base/tl/string_apocalypse.h>

#include <stdexcept>  // for std::out_of_range
#include <unordered_map>
#include <variant>

using ValueVariant = std::variant<bool, int, string>;

class CValue
{
public:
    const char* m_pKeyName;
    ValueVariant m_Value;

    template<typename T>
    CValue(const char* pKeyName, T&& value) :
        m_pKeyName(pKeyName),
		m_Value(std::forward<T>(value))
    {
        static_assert(std::is_constructible_v<ValueVariant, T>, "Type not supported in CValue");
    }

    template<typename T>
    const T& Get() const
    {
        return std::get<T>(m_Value);
    }

    template<typename T>
    T& Get()
    {
        return std::get<T>(m_Value);
    }

    template<typename T>
    operator const T&() const
    {
        return Get<T>();
    }
};

class CWorldConfig
{
	std::unordered_map<unsigned, CValue> m_uValues;
	class Proxy
    {
        CWorldConfig &m_Config;
        const char* m_pKeyName;

    public:
        Proxy(CWorldConfig &Config, const char* pKey) : m_Config(Config), m_pKeyName(pKey) {}

        template<typename T>
        operator T() const
        {
            return m_Config.Get<T>(m_pKeyName);
        }

        template<typename T>
        Proxy& operator=(T&& value)
        {
            m_Config.Set(m_pKeyName, std::forward<T>(value));
            return *this;
        }
    };

	template<typename T>
    T GetValue(const char* pKeyName, const T& DefaultValue) const
    {
        unsigned hash = str_quickhash(pKeyName);
        auto Iter = m_uValues.find(hash);
        if(Iter != m_uValues.end())
        {
            try
            {
                return Iter->second.Get<T>();
            }
            catch (const std::bad_variant_access&)
            {
                return DefaultValue;
            }
        }
        return DefaultValue;
    }
public:
    Proxy operator[](const char* KeyName)
    {
        return Proxy(*this, KeyName);
    }

    template<typename T>
    T Get(const char* KeyName) const
    {
        if constexpr (std::is_same_v<T, int>)
        {
            return GetValue(KeyName, static_cast<T>(-1));
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            return GetValue(KeyName, static_cast<T>(false));
        }
        else if constexpr (std::is_constructible_v<T, const char*>)
        {
            return GetValue(KeyName, T("unknown"));
        }
        else
        {
            return T{};
        }
    }

    template<typename T>
    void Set(const char* pKey, T&& value)
    {
        unsigned hash = str_quickhash(pKey);
        auto Iter = m_uValues.find(hash);
        if(Iter != m_uValues.end())
        {
			m_uValues[hash] = CValue(Iter->second.m_pKeyName, std::forward<T>(value));
			return;
		}
    	throw std::out_of_range(string("CWorldConfig::Set: Key not found:") + pKey);
    }

    template<typename T>
    void Init(const char* pKey, const char* KeyName, T&& value)
    {
        unsigned Hash = str_quickhash(KeyName);
        m_uValues[Hash] = CValue(KeyName, std::forward<T>(value));
    }
};

#endif // GAME_SERVER_CONFIG_H
