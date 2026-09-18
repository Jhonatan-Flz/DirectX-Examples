#include "include/common.hpp"

ID2D1Factory* pD2DFactory = nullptr;
ID2D1HwndRenderTarget* pRenderTarget = nullptr;
ID2D1SolidColorBrush* pBrush = nullptr;

const int grid_size = 32;
const float cell_size = 14.0f;
const float canvas_x = 20.0f;
const float canvas_y = 20.0f;

D2D1_COLOR_F canvas [grid_size][grid_size];

D2D1_COLOR_F palette [  ] = {

	D2D1::ColorF ( 0.0f, 0.0f, 0.0f ),      // black
	D2D1::ColorF ( 1.0f, 1.0f, 1.0f ),      // white
	D2D1::ColorF ( 0.9f, 0.1f, 0.1f ),      // red
	D2D1::ColorF ( 1.0f, 0.5f, 0.0f ),      // orange
	D2D1::ColorF ( 1.0f, 0.9f, 0.0f ),      // yellow
	D2D1::ColorF ( 0.1f, 0.7f, 0.2f ),      // green
	D2D1::ColorF ( 0.1f, 0.4f, 0.9f ),      // blue
	D2D1::ColorF ( 0.6f, 0.1f, 0.8f ),      // purple
	D2D1::ColorF ( 0.55f, 0.27f, 0.07f ),   // brown
	D2D1::ColorF ( 0.5f, 0.5f, 0.5f ),      // gray

};

const int palette_count = 10;
const float swatch_size = 40.0f;
const float swatch_stride = 46.0f;
const float palette_x = 20.0f;
const float palette_y = 484.0f;

int selected_index = 0;
bool left_down = false;
bool right_down = false;

void paint_at ( int mx, int my, D2D1_COLOR_F color ) {

	if ( mx < canvas_x || my < canvas_y ) { return; }

	int col = ( int ) ( ( mx - canvas_x ) / cell_size );
	int row = ( int ) ( ( my - canvas_y ) / cell_size );

	if ( col >= 0 && col < grid_size && row >= 0 && row < grid_size ) { canvas [row][col] = color; }

}

void try_select_palette ( int mx, int my ) {

	for ( int i = 0; i < palette_count; i++ ) {

		float sx = palette_x + i * swatch_stride;

		if ( mx >= sx && mx < sx + swatch_size && my >= palette_y && my < palette_y + swatch_size ) { selected_index = i; return; }

	}

}

void render (  ) {

	if ( !pRenderTarget ) { return; }

	pRenderTarget -> BeginDraw (  );
	pRenderTarget -> Clear ( D2D1::ColorF ( 0.2f, 0.2f, 0.2f ) );

	for ( int row = 0; row < grid_size; row++ ) {

		for ( int col = 0; col < grid_size; col++ ) {

			D2D1_RECT_F rect = D2D1::RectF ( canvas_x + col * cell_size, canvas_y + row * cell_size, canvas_x + ( col + 1 ) * cell_size, canvas_y + ( row + 1 ) * cell_size );
			pBrush -> SetColor ( canvas [row][col] );
			pRenderTarget -> FillRectangle ( rect, pBrush );

		}

	}

	pBrush -> SetColor ( D2D1::ColorF ( 0.5f, 0.5f, 0.5f, 0.4f ) );

	for ( int i = 0; i <= grid_size; i++ ) {

		float gx = canvas_x + i * cell_size;
		float gy = canvas_y + i * cell_size;

		pRenderTarget -> DrawLine ( D2D1::Point2F ( gx, canvas_y ), D2D1::Point2F ( gx, canvas_y + grid_size * cell_size ), pBrush, 0.5f );
		pRenderTarget -> DrawLine ( D2D1::Point2F ( canvas_x, gy ), D2D1::Point2F ( canvas_x + grid_size * cell_size, gy ), pBrush, 0.5f );

	}

	pBrush -> SetColor ( D2D1::ColorF ( D2D1::ColorF::White ) );
	pRenderTarget -> DrawRectangle ( D2D1::RectF ( canvas_x, canvas_y, canvas_x + grid_size * cell_size, canvas_y + grid_size * cell_size ), pBrush, 2.0f );

	for ( int i = 0; i < palette_count; i++ ) {

		float sx = palette_x + i * swatch_stride;
		D2D1_RECT_F rect = D2D1::RectF ( sx, palette_y, sx + swatch_size, palette_y + swatch_size );

		pBrush -> SetColor ( palette[i] );
		pRenderTarget -> FillRectangle ( rect, pBrush );

		pBrush -> SetColor ( ( i == selected_index ) ? D2D1::ColorF ( 0.0f, 1.0f, 1.0f ) : D2D1::ColorF ( 0.5f, 0.5f, 0.5f ) );
		pRenderTarget -> DrawRectangle ( rect, pBrush, ( i == selected_index ) ? 3.0f : 1.0f );

	}

	pRenderTarget -> EndDraw (  );

}

LRESULT window_procedure ( HWND handle, UINT message_number, WPARAM word_param, LPARAM long_param ) {

	int mx = LOWORD ( long_param );
	int my = HIWORD ( long_param );

	switch ( message_number ) {

		case WM_PAINT:

			render (  );
			ValidateRect ( handle, NULL );

		return 0;

		case WM_LBUTTONDOWN:

			SetCapture ( handle );
			try_select_palette ( mx, my );
			paint_at ( mx, my, palette[selected_index] );
			left_down = true;
			InvalidateRect ( handle, NULL, FALSE );

		return 0;

		case WM_RBUTTONDOWN:

			SetCapture ( handle );
			paint_at ( mx, my, D2D1::ColorF ( D2D1::ColorF::White ) );
			right_down = true;
			InvalidateRect ( handle, NULL, FALSE );

		return 0;

		case WM_MOUSEMOVE:

			if ( left_down ) { paint_at ( mx, my, palette[selected_index] ); InvalidateRect ( handle, NULL, FALSE ); }
			if ( right_down ) { paint_at ( mx, my, D2D1::ColorF ( D2D1::ColorF::White ) ); InvalidateRect ( handle, NULL, FALSE ); }

		return 0;

		case WM_LBUTTONUP:

			left_down = false;
			ReleaseCapture (  );

		return 0;

		case WM_RBUTTONUP:

			right_down = false;
			ReleaseCapture (  );

		return 0;

		case WM_DESTROY:

			safe_release_all ( pBrush, pRenderTarget, pD2DFactory );
			PostQuitMessage ( 0 );

		return 0;

	}

	return DefWindowProc ( handle, message_number, word_param, long_param );

}

int wWinMain ( HINSTANCE handle_instance, HINSTANCE deprecated_instance, LPWSTR chain, int CMD ) {

	for ( int row = 0; row < grid_size; row++ ) {

		for ( int col = 0; col < grid_size; col++ ) { canvas [row][col] = D2D1::ColorF ( D2D1::ColorF::White ); }

	}

	WNDCLASS wc = { ( CS_HREDRAW | CS_VREDRAW ), window_procedure, 0, 0, handle_instance, LoadIcon ( NULL, IDI_APPLICATION ), LoadCursor ( NULL, IDC_CROSS ), ( HBRUSH ) ( COLOR_WINDOW + 1 ), NULL, L"Pixel Art" };
	RegisterClass ( &wc );

	HWND handle = CreateWindow ( L"Pixel Art", L"Pixel Art Paint", WS_CAPTION | WS_SYSMENU, 200, 100, 494, 580, nullptr, nullptr, handle_instance, 0 );

	ensure_d2d_factory ( pD2DFactory );
	ensure_hwnd_render_target ( pD2DFactory, handle, pRenderTarget );
	ensure_solid_color_brush ( pRenderTarget, D2D1::ColorF ( D2D1::ColorF::White ), pBrush );

	ShowWindow ( handle, CMD );

	MSG message;

	while ( GetMessage ( &message, nullptr, 0, 0 ) ) {

		TranslateMessage ( &message );
		DispatchMessage ( &message );

	}

	return 0;

}