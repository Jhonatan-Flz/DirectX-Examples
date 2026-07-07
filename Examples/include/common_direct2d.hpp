#pragma once

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <cmath>

template <typename T>
inline void safe_release ( T*& pointer ) {

    if ( pointer ) {

		pointer -> Release (  );
		pointer = nullptr;
 
	}

}

inline float minimum_float ( float first, float second ) { return ( first < second ) ? first : second; }
inline float maximum_float ( float first, float second ) { return ( first > second ) ? first : second; }