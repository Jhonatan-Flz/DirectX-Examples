#include "include/common.hpp"

ID2D1Factory* factory = nullptr;
ID2D1HwndRenderTarget* target = nullptr;
IDWriteFactory* write_factory = nullptr;
IDWriteTextFormat* text_format = nullptr;
ID2D1SolidColorBrush* brush = nullptr;

const char * rows [ ] = { "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM" };
float offset [ ] = { 0.f, 20.f, 40.f };

void render (  ) {

	target -> BeginDraw (  );
	target -> Clear ( D2D1::ColorF ( 0.08f, 0.08f, 0.08f ) );

	for ( int i = 0; i < 3; i++ ) {

		for ( int j = 0; rows [i][j]; j++ ) {

			float x = offset [i] + j * 45.f + 10.f, y = i * 45.f + 10.f;
			D2D1_RECT_F rect = D2D1::RectF ( x, y, x + 40.f, y + 40.f );
			bool pressed = GetAsyncKeyState ( rows[ i ][ j ] ) & 0x8000;

			brush -> SetColor ( pressed ? D2D1::ColorF ( 0.2f, 0.8f, 0.3f ) : D2D1::ColorF ( 0.25f, 0.25f, 0.25f ) );
			target -> FillRectangle ( rect, brush );

			brush -> SetColor ( D2D1::ColorF ( 1, 1, 1 ) );
			target -> DrawRectangle ( rect, brush, 1.f );

			wchar_t label [2] = { ( wchar_t ) rows [i][j], 0 };
			target -> DrawText ( label, 1, text_format, rect, brush );

		}

	}

	target -> EndDraw (  );

}

LRESULT CALLBACK wnd_proc ( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp ) {

	if ( msg == WM_DESTROY ) { PostQuitMessage ( 0 ); return 0; }
	return DefWindowProc ( hwnd, msg, wp, lp );

}

int WINAPI wWinMain ( HINSTANCE handle_instance, HINSTANCE deprecated_instance, LPWSTR chain, int CMD ) {

	WNDCLASS wc = {  };
	wc.lpfnWndProc = wnd_proc;
	wc.hInstance = handle_instance;
	wc.lpszClassName = L"key_overlay";
	RegisterClass ( &wc );

	HWND hwnd = CreateWindowEx ( WS_EX_TOPMOST, L"key_overlay", L"Keyboard", WS_POPUP | WS_VISIBLE, 100, 100, 470, 170, nullptr, nullptr, handle_instance, nullptr );

	D2D1CreateFactory ( D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory );
	DWriteCreateFactory ( DWRITE_FACTORY_TYPE_SHARED, __uuidof ( IDWriteFactory ), ( IUnknown** ) &write_factory );
	write_factory -> CreateTextFormat ( L"Consolas", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 14.f, L"en-us", &text_format );
	text_format -> SetTextAlignment ( DWRITE_TEXT_ALIGNMENT_CENTER );
	text_format -> SetParagraphAlignment ( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );

	RECT rc;
	GetClientRect ( hwnd, &rc );
	factory -> CreateHwndRenderTarget ( 
		
		D2D1::RenderTargetProperties (   ), 
		D2D1::HwndRenderTargetProperties ( hwnd, D2D1::SizeU ( rc.right, rc.bottom ) ), 
		&target 
	
	);
	
	target -> CreateSolidColorBrush ( D2D1::ColorF ( 1, 1, 1 ), &brush );

	MSG msg = { };

	while ( msg.message != WM_QUIT && !( GetAsyncKeyState ( VK_ESCAPE ) & 0x8000 ) ) {

		if ( PeekMessage ( &msg, nullptr, 0, 0, PM_REMOVE ) ) { TranslateMessage ( &msg ); DispatchMessage ( &msg ); }
		else { render (   ); Sleep ( 10 ); }

	}

	safe_release ( brush );
	safe_release ( text_format );
	safe_release ( write_factory );
	safe_release ( target );
	safe_release ( factory );

	return 0;

}