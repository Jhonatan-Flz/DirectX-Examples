#include <windows.h>
#include <d2d1.h>
#include "include\common_direct2d.hpp"

static ID2D1Factory* pFactory = nullptr;
static ID2D1HwndRenderTarget* pRenderTarget = nullptr;
static ID2D1SolidColorBrush* pPinkBrush = nullptr;
static ID2D1SolidColorBrush* pGreenBrush = nullptr;
static ID2D1SolidColorBrush* pBlueBrush = nullptr;
static ID2D1SolidColorBrush* pRedBrush = nullptr;
static ID2D1SolidColorBrush* pGridBrush = nullptr;

void release_brush ( ID2D1SolidColorBrush *& brush ) {

	if ( brush ) {

		brush -> Release (  );
		brush = nullptr;

	}

}

void discard_device_resources (  ) {

	release_brush ( pPinkBrush );
	release_brush ( pGreenBrush );
	release_brush ( pBlueBrush );
	release_brush ( pRedBrush );
	release_brush ( pGridBrush );

	if ( pRenderTarget ) {

		pRenderTarget -> Release (  );
		pRenderTarget = nullptr;

	}

}

HRESULT create_colored_brushes (  ) {

	HRESULT hr = pRenderTarget -> CreateSolidColorBrush ( D2D1::ColorF ( 1.0f, 0.10f, 0.50f ), &pPinkBrush );

	if ( SUCCEEDED ( hr ) ) {

		hr = pRenderTarget -> CreateSolidColorBrush ( D2D1::ColorF ( 0.18f, 0.82f, 0.32f ), &pGreenBrush );

	}

	if ( SUCCEEDED ( hr ) ) {

		hr = pRenderTarget -> CreateSolidColorBrush ( D2D1::ColorF ( 0.12f, 0.46f, 1.0f ), &pBlueBrush );

	}

	if ( SUCCEEDED ( hr ) ) {

		hr = pRenderTarget -> CreateSolidColorBrush ( D2D1::ColorF ( 1.0f, 0.16f, 0.12f ), &pRedBrush );

	}

	if ( SUCCEEDED ( hr ) ) {

		hr = pRenderTarget -> CreateSolidColorBrush ( D2D1::ColorF ( 0.36f, 0.36f, 0.36f ), &pGridBrush );

	}

	return hr;

}

HRESULT create_device_resources ( HWND handle ) {

	if ( pRenderTarget ) { return S_OK; }

	if ( !pFactory ) {

		HRESULT hr = D2D1CreateFactory ( D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory );
		if ( FAILED ( hr ) ) { return hr; }

	}

	RECT rect_handle;
	GetClientRect ( handle, &rect_handle );

	D2D1_SIZE_U size = D2D1::SizeU ( rect_handle.right - rect_handle.left, rect_handle.bottom - rect_handle.top );

	HRESULT hr = pFactory -> CreateHwndRenderTarget (
		D2D1::RenderTargetProperties (  ),
		D2D1::HwndRenderTargetProperties ( handle, size ),
		&pRenderTarget
	);

	if ( SUCCEEDED ( hr ) ) { hr = create_colored_brushes (  ); }

	if ( FAILED ( hr ) ) { discard_device_resources (  ); }

	return hr;

}

D2D1_RECT_F get_shape_rect ( float left, float top, float cell_size ) {

	float padding = cell_size * 0.18f;

	return D2D1::RectF ( left + padding, top + padding, left + cell_size - padding, top + cell_size - padding );

}

void draw_grid ( float left, float top, float grid_size, float cell_size ) {

	float right = left + grid_size;
	float bottom = top + grid_size;
	float middle_x = left + cell_size;
	float middle_y = top + cell_size;

	pRenderTarget -> DrawRectangle ( D2D1::RectF ( left, top, right, bottom ), pGridBrush, 1.0f );
	pRenderTarget -> DrawLine ( D2D1::Point2F ( middle_x, top ), D2D1::Point2F ( middle_x, bottom ), pGridBrush, 1.0f );
	pRenderTarget -> DrawLine ( D2D1::Point2F ( left, middle_y ), D2D1::Point2F ( right, middle_y ), pGridBrush, 1.0f );

}

void draw_triangle ( D2D1_RECT_F bounds, float stroke_width ) {

	D2D1_POINT_2F top = D2D1::Point2F ( ( bounds.left + bounds.right ) / 2.0f, bounds.top );
	D2D1_POINT_2F left = D2D1::Point2F ( bounds.left, bounds.bottom );
	D2D1_POINT_2F right = D2D1::Point2F ( bounds.right, bounds.bottom );

	pRenderTarget -> DrawLine ( top, left, pGreenBrush, stroke_width );
	pRenderTarget -> DrawLine ( left, right, pGreenBrush, stroke_width );
	pRenderTarget -> DrawLine ( right, top, pGreenBrush, stroke_width );

}

void draw_cross ( D2D1_RECT_F bounds, float stroke_width ) {

	pRenderTarget -> DrawLine ( D2D1::Point2F ( bounds.left, bounds.top ), D2D1::Point2F ( bounds.right, bounds.bottom ), pBlueBrush, stroke_width );
	pRenderTarget -> DrawLine ( D2D1::Point2F ( bounds.right, bounds.top ), D2D1::Point2F ( bounds.left, bounds.bottom ), pBlueBrush, stroke_width );

}

void draw_centered_shapes_grid ( D2D1_SIZE_F window_size ) {

	float grid_size = minimum_float ( window_size.width, window_size.height ) * 0.78f;
	float cell_size = grid_size / 2.0f;
	float left = ( window_size.width - grid_size ) / 2.0f;
	float top = ( window_size.height - grid_size ) / 2.0f;
	float stroke_width = cell_size * 0.045f;

	if ( stroke_width < 2.0f ) { stroke_width = 2.0f; }
	if ( stroke_width > 8.0f ) { stroke_width = 8.0f; }

	D2D1_RECT_F square = get_shape_rect ( left, top, cell_size );
	D2D1_RECT_F triangle = get_shape_rect ( left + cell_size, top, cell_size );
	D2D1_RECT_F cross = get_shape_rect ( left, top + cell_size, cell_size );
	D2D1_RECT_F circle = get_shape_rect ( left + cell_size, top + cell_size, cell_size );

	// The layout remains in a 2x2 grid, but the guide lines are kept invisible.
	pRenderTarget -> DrawRectangle ( square, pPinkBrush, stroke_width );
	draw_triangle ( triangle, stroke_width );
	draw_cross ( cross, stroke_width );
	pRenderTarget -> DrawEllipse (
		D2D1::Ellipse (
			D2D1::Point2F ( ( circle.left + circle.right ) / 2.0f, ( circle.top + circle.bottom ) / 2.0f ),
			( circle.right - circle.left ) / 2.0f,
			( circle.bottom - circle.top ) / 2.0f
		),
		pRedBrush,
		stroke_width
	);

}

void render ( HWND handle ) {

	HRESULT hr = create_device_resources ( handle );
	if ( FAILED ( hr ) ) { return; }

	pRenderTarget -> BeginDraw (  );
	pRenderTarget -> Clear ( D2D1::ColorF ( 0.176f, 0.176f, 0.176f ) );
	pRenderTarget -> SetTransform ( D2D1::Matrix3x2F::Identity (  ) );

	draw_centered_shapes_grid ( pRenderTarget -> GetSize (  ) );

	hr = pRenderTarget -> EndDraw (  );
	if ( hr == D2DERR_RECREATE_TARGET ) { discard_device_resources (  ); }

}

LRESULT window_procedure ( HWND handle, UINT message_number, WPARAM word_param, LPARAM long_param ) {

	switch ( message_number ) {

		case WM_SIZE:
			if ( pRenderTarget ) {

				RECT rect_handle;
				GetClientRect ( handle, &rect_handle );
				pRenderTarget -> Resize ( D2D1::SizeU ( rect_handle.right - rect_handle.left, rect_handle.bottom - rect_handle.top ) );
				InvalidateRect ( handle, nullptr, FALSE );

			}
		return 0;

		case WM_PAINT: {

			PAINTSTRUCT paint;
			BeginPaint ( handle, &paint );
			render ( handle );
			EndPaint ( handle, &paint );

		}
		return 0;

		case WM_DESTROY:
			discard_device_resources (  );

			if ( pFactory ) {

				pFactory -> Release (  );
				pFactory = nullptr;

			}

			PostQuitMessage ( 0 );
		return 0;

	}

	return DefWindowProc ( handle, message_number, word_param, long_param );

}

int wWinMain ( HINSTANCE handle_instance, HINSTANCE deprecated_instance, LPWSTR chain, int cmd ) {

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
		L"direct2d drawing example"               // Window class name

	};

	RegisterClass ( &wc );

	HWND handle = CreateWindow (
		L"direct2d drawing example",
		L"Direct2D drawing example",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		600,
		600,
		nullptr,
		nullptr,
		handle_instance,
		nullptr
	);

	if ( handle != nullptr ) {

		ShowWindow ( handle, cmd );
		UpdateWindow ( handle );

	}

	MSG message = { };
	while ( GetMessage ( &message, nullptr, 0, 0 ) ) {

		TranslateMessage ( &message );
		DispatchMessage ( &message );

	}

	return static_cast<int> ( message.wParam );

}