#include "include/common.hpp"

ID2D1Factory* pD2DFactory = nullptr;
ID2D1HwndRenderTarget* pRenderTarget = nullptr;
ID2D1SolidColorBrush* pBrush = nullptr;
IDWriteFactory* pDWriteFactory = nullptr;
IDWriteTextFormat* pTextFormat = nullptr;

void InitDirectX ( HWND handle ) {

	ensure_d2d_factory ( pD2DFactory );
	ensure_hwnd_render_target ( pD2DFactory, handle, pRenderTarget );
	ensure_solid_color_brush ( pRenderTarget, D2D1::ColorF ( D2D1::ColorF::White ), pBrush );
	ensure_dwrite_factory ( pDWriteFactory );
	create_text_format ( pDWriteFactory, L"Segoe UI", DWRITE_FONT_WEIGHT_EXTRA_BOLD, DWRITE_FONT_STYLE_NORMAL, 60.0f, pTextFormat );

	pTextFormat -> SetTextAlignment ( DWRITE_TEXT_ALIGNMENT_CENTER );
	pTextFormat -> SetParagraphAlignment ( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );

}

void CleanupDirectX (  ) { safe_release_all ( pTextFormat, pDWriteFactory, pBrush, pRenderTarget, pD2DFactory ); }

LRESULT window_procedure ( HWND handle, UINT message_number, WPARAM word_param, LPARAM long_param ) {

	switch ( message_number ) {

		case WM_PAINT:

			if ( pRenderTarget ) {

				pRenderTarget -> BeginDraw (  );
				pRenderTarget -> Clear ( D2D1::ColorF ( D2D1::ColorF::Black ) );

				D2D1_SIZE_F renderSize = pRenderTarget -> GetSize (  );
				D2D1_RECT_F layoutRect = D2D1::RectF ( 0, 0, renderSize.width, renderSize.height );

				draw_text ( pRenderTarget, L"DVD", pTextFormat, layoutRect, pBrush );
				pRenderTarget -> EndDraw (  );

			}

			ValidateRect ( handle, NULL );
		
		return 0;

		case WM_DESTROY:

			CleanupDirectX (  );
			PostQuitMessage ( 0 );

		return 0;

	}

	return DefWindowProc ( handle, message_number, word_param, long_param );

}

int wWinMain ( HINSTANCE handle_instance, HINSTANCE deprecated_instance, LPWSTR chain, int CMD ) {

	srand ( static_cast <unsigned int> ( time ( NULL ) ) );

	WNDCLASS wc = {

		( CS_HREDRAW | CS_VREDRAW ),              // Style
		window_procedure,                         // Procedure
		0,                                        // Class extra bytes
		0,                                        // Window extra bytes
		handle_instance,                          // Handle instance
		LoadIcon ( NULL, IDI_APPLICATION ),       // Icon
		LoadCursor ( NULL, IDC_ARROW ),           // Cursor
		( HBRUSH ) ( COLOR_WINDOW + 1 ),          // Background brush
		NULL,                                     // Pointer to menu name
		L"Screen saver"		                 // Window class name

	};

	RegisterClass ( &wc );
	HWND handle = CreateWindow (

		L"Screen saver",
		L"DVD Screensaver",
		WS_POPUP,
		300,
		300,
		300,
		300,
		nullptr,
		nullptr,
		GetModuleHandle ( NULL ),
		0
	
	);

	if ( handle != nullptr ) { InitDirectX ( handle ); ShowWindow ( handle, CMD ); }

	RECT screen;
	SystemParametersInfo ( SPI_GETWORKAREA, 0, &screen, 0 );
	RECT rect_handle;
	GetWindowRect ( handle, &rect_handle );

	float dx = 4, dy = 4;
	MSG message = {  };

	while ( message.message != WM_QUIT ) {

		bool colision = false;

		if ( rect_handle.right >= screen.right || rect_handle.left <= screen.left ) { dx *= -1; colision = true; }
		if ( rect_handle.bottom >= screen.bottom || rect_handle.top <= screen.top ) { dy *= -1; colision = true; }

		if ( colision && pBrush ) {

			float r = static_cast <float> ( rand (  ) % 100 ) / 100.0f;
			float g = static_cast <float> ( rand (  ) % 100 ) / 100.0f;
			float b = static_cast <float> ( rand (  ) % 100 ) / 100.0f;

			pBrush -> SetColor ( D2D1::ColorF ( r + 0.2f, g + 0.2f, b + 0.2f ) );

		}

		OffsetRect ( &rect_handle, dx, dy );
		while ( PeekMessage ( &message, nullptr, 0, 0, PM_REMOVE ) ) {

			TranslateMessage ( &message );
			DispatchMessage ( &message );
		
		}

		SetWindowPos ( handle, nullptr, rect_handle.left, rect_handle.top, rect_handle.right - rect_handle.left, rect_handle.bottom - rect_handle.top, 0 );
		InvalidateRect ( handle, NULL, FALSE );
		Sleep ( 1000 / 500 );

	}

	return 0;

}
