#pragma once

//copied from stdafx.h found in Kinect SDK 1.8 samples

#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 1
#include <new>
#include <windows.h>
#include <string>
#include <strsafe.h>

// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface*& pInterfaceToRelease)
{
    if (pInterfaceToRelease)
    {
        pInterfaceToRelease->Release();
        pInterfaceToRelease = nullptr;
    }
}

// Case-insensitive wide-character char traits
struct i_char_traits : public std::char_traits<wchar_t>
{
    static bool eq(wchar_t c1, wchar_t c2) { return towupper(c1) == towupper(c2); } 
    static bool ne(wchar_t c1, wchar_t c2) { return towupper(c1) != towupper(c2); } 
    static bool lt(wchar_t c1, wchar_t c2) { return towupper(c1) <  towupper(c2); }
    static int compare(const wchar_t* s1, const wchar_t* s2, size_t n)
    { 
        while( 0 != n-- )
        { 
            if( towupper(*s1) < towupper(*s2) ) return -1; 
            if( towupper(*s1) > towupper(*s2) ) return 1; 
            ++s1;
            ++s2; 
        }

        return 0; 
    }

    static const wchar_t* find(const wchar_t* s, size_t n, wchar_t a)
    {
        int remaining = static_cast<int>(n);
        while( (--remaining >= 0) && (towupper(*s) != towupper(a)) )
        { 
            ++s; 
        } 
        return (remaining >= 0) ? s : NULL; 
    } 
};

typedef std::basic_string<wchar_t, i_char_traits> wistring;