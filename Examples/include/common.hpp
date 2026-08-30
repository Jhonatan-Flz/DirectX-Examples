#pragma once
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>

#include <cmath>
#include <cwchar>
#include <string>
#include <time.h>

template <typename T>
inline void safe_release ( T*& pointer ) {

	if ( pointer ) {

		pointer -> Release (  );
		pointer = nullptr;
 
	}

}

inline float minimum_float ( float first, float second ) { return ( first < second ) ? first : second; }
inline float maximum_float ( float first, float second ) { return ( first > second ) ? first : second; }
inline float clamp_float ( float value, float low, float high ) { return maximum_float ( minimum_float ( value, high ), low ); }

inline void safe_release_all (  ) {
}

template <typename T, typename... Others>
inline void safe_release_all ( T*& pointer, Others*&... other_pointers ) {

	safe_release ( pointer );
	safe_release_all ( other_pointers... );

}

inline D2D1_SIZE_U get_client_size ( HWND handle ) {

	RECT client_rectangle;
	GetClientRect ( handle, &client_rectangle );

	return D2D1::SizeU ( client_rectangle.right - client_rectangle.left, client_rectangle.bottom - client_rectangle.top );

}

inline HRESULT ensure_d2d_factory ( ID2D1Factory*& factory ) {

	if ( factory ) { return S_OK; }

	return D2D1CreateFactory ( D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory );

}

inline HRESULT ensure_dwrite_factory ( IDWriteFactory*& factory ) {

	if ( factory ) { return S_OK; }

	return DWriteCreateFactory (

		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof ( IDWriteFactory ),
		reinterpret_cast<IUnknown**> ( &factory )
	
	);

}

inline HRESULT ensure_hwnd_render_target ( ID2D1Factory* factory, HWND handle, ID2D1HwndRenderTarget*& render_target ) {

	if ( render_target ) { return S_OK; }

	return factory -> CreateHwndRenderTarget (

		D2D1::RenderTargetProperties (  ),
		D2D1::HwndRenderTargetProperties ( handle, get_client_size ( handle ) ),
		&render_target
	
	);

}

inline HRESULT ensure_solid_color_brush ( ID2D1RenderTarget* render_target, D2D1_COLOR_F color, ID2D1SolidColorBrush*& brush ) {

	if ( brush ) { return S_OK; }

	return render_target -> CreateSolidColorBrush ( color, &brush );

}

inline HRESULT create_text_format (

	IDWriteFactory* factory,
	const wchar_t* font_family,
	DWRITE_FONT_WEIGHT weight,
	DWRITE_FONT_STYLE style,
	float font_size,
	IDWriteTextFormat*& text_format,
	const wchar_t* locale = L"en-us"

) {

	return factory -> CreateTextFormat (

		font_family,
		nullptr,
		weight,
		style,
		DWRITE_FONT_STRETCH_NORMAL,
		font_size,
		locale,
		&text_format
	
	);

}

inline void resize_hwnd_render_target ( ID2D1HwndRenderTarget* render_target, UINT width, UINT height ) {

	if ( render_target ) { render_target -> Resize ( D2D1::SizeU ( width, height ) ); }

}

inline void draw_text (

	ID2D1RenderTarget* render_target,
	const wchar_t* text,
	IDWriteTextFormat* text_format,
	D2D1_RECT_F layout_rectangle,
	ID2D1Brush* brush

) {

	render_target -> DrawTextW ( text, static_cast<UINT32> ( wcslen ( text ) ), text_format, layout_rectangle, brush );

}
