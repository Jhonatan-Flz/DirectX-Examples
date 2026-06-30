#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <cmath>

template<typename T> void safe_release ( T*& pointer ) {

	if ( pointer ) {

		pointer->Release ( );
		pointer = nullptr;

	}

}

ID2D1Factory* g_pFactory = nullptr;
ID2D1HwndRenderTarget* g_pRenderTarget = nullptr;
ID2D1SolidColorBrush* g_pBrush = nullptr;
IDWriteFactory* g_pWriteFactory = nullptr;
IDWriteTextFormat* g_pTextFormat = nullptr;

UINT64 g_start_time_ms = 0;
bool g_paused = false;
UINT64 g_pause_start_ms = 0;
UINT64 g_pause_offset_ms = 0;

bool g_dragging = false;
bool g_was_paused_before_drag = false;
float g_mouse_x = 0.0f, g_mouse_y = 0.0f;

const double GRAVITY = 9.81;
const double PIXELS_PER_METER = 200.0;
double g_length_meters = 1.0;
double g_length_pixels = g_length_meters * PIXELS_PER_METER;
double g_initial_angle = 0.8;

const UINT_PTR TIMER_ID = 1;
const UINT TIMER_INTERVAL_MS = 16;

const wchar_t* MAIN_TITLE = L"Pendulo - Win32 + Direct2D";
const wchar_t* PAUSE_TITLE = L"Pendulo - Pausado";
const wchar_t* DRAG_TITLE = L"Pendulo - Arrastrando";
const wchar_t* PAUSE_HINT = L"Press Space to pause";

float g_bob_radius = 16.0f;

void get_pivot ( HWND handle, float& out_x, float& out_y ) {

	RECT rect_handle;
	GetClientRect ( handle, &rect_handle );

	float width = static_cast<float> ( rect_handle.right - rect_handle.left );
	out_x = width / 2.0f;
	out_y = 50.0f;

}

void discard_graphics_resources ( ) {

	safe_release ( g_pBrush );
	safe_release ( g_pRenderTarget );

}

HRESULT create_text_resources ( ) {

	if ( g_pTextFormat ) { return S_OK; }

	if ( !g_pWriteFactory ) {

		HRESULT hr = DWriteCreateFactory (
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof ( IDWriteFactory ),
			reinterpret_cast<IUnknown**> ( &g_pWriteFactory )
		);

		if ( FAILED ( hr ) ) { return hr; }

	}

	HRESULT hr = g_pWriteFactory->CreateTextFormat (
		L"Segoe UI",
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		18.0f,
		L"en-us",
		&g_pTextFormat
	);

	if ( SUCCEEDED ( hr ) ) {

		g_pTextFormat->SetTextAlignment ( DWRITE_TEXT_ALIGNMENT_LEADING );
		g_pTextFormat->SetParagraphAlignment ( DWRITE_PARAGRAPH_ALIGNMENT_NEAR );

	}

	return hr;

}

HRESULT create_graphics_resources ( HWND handle ) {

	if ( g_pRenderTarget ) { return S_OK; }

	RECT rect_handle;
	GetClientRect ( handle, &rect_handle );

	D2D1_SIZE_U size = D2D1::SizeU ( rect_handle.right - rect_handle.left, rect_handle.bottom - rect_handle.top );

	HRESULT hr = g_pFactory->CreateHwndRenderTarget (
		D2D1::RenderTargetProperties ( ),
		D2D1::HwndRenderTargetProperties ( handle, size ),
		&g_pRenderTarget
	);

	if ( SUCCEEDED ( hr ) ) {

		hr = g_pRenderTarget->CreateSolidColorBrush ( D2D1::ColorF ( D2D1::ColorF::White ), &g_pBrush );

	}

	if ( SUCCEEDED ( hr ) ) { hr = create_text_resources ( ); }

	return hr;

}

void resize_render_target ( UINT width, UINT height ) {

	if ( g_pRenderTarget ) {

		D2D1_SIZE_U size;
		size.width = width;
		size.height = height;

		g_pRenderTarget->Resize ( size );

	}

}

void get_current_bob_position ( HWND handle, float& bob_x, float& bob_y ) {

	if ( g_dragging ) {

		bob_x = g_mouse_x;
		bob_y = g_mouse_y;
		return;

	}

	float pivot_x, pivot_y;
	get_pivot ( handle, pivot_x, pivot_y );

	UINT64 now_ms = GetTickCount ( );
	if ( g_paused ) { now_ms = g_pause_start_ms; }

	double time = static_cast<double> ( now_ms - g_start_time_ms - g_pause_offset_ms ) / 1000.0;
	double omega = sqrt ( GRAVITY / g_length_meters );
	double angle = g_initial_angle * cos ( omega * time );

	bob_x = pivot_x + static_cast<float> ( g_length_pixels * sin ( angle ) );
	bob_y = pivot_y + static_cast<float> ( g_length_pixels * cos ( angle ) );

}

void paint_window ( HWND handle ) {

	HRESULT hr = create_graphics_resources ( handle );
	if ( FAILED ( hr ) || !g_pRenderTarget ) { return; }

	float pivot_x, pivot_y;
	get_pivot ( handle, pivot_x, pivot_y );

	float bob_x, bob_y;
	get_current_bob_position ( handle, bob_x, bob_y );

	g_pRenderTarget->BeginDraw ( );
	g_pRenderTarget->Clear ( D2D1::ColorF ( D2D1::ColorF::Black ) );

	g_pBrush->SetColor ( D2D1::ColorF ( D2D1::ColorF::LightGray ) );
	g_pRenderTarget->DrawLine ( D2D1::Point2F ( pivot_x, pivot_y ), D2D1::Point2F ( bob_x, bob_y ), g_pBrush, 3.0f );

	g_pBrush->SetColor ( D2D1::ColorF ( D2D1::ColorF::CornflowerBlue ) );
	g_pRenderTarget->FillEllipse ( D2D1::Ellipse ( D2D1::Point2F ( bob_x, bob_y ), g_bob_radius, g_bob_radius ), g_pBrush );

	g_pBrush->SetColor ( D2D1::ColorF ( D2D1::ColorF::Orange ) );
	g_pRenderTarget->FillEllipse ( D2D1::Ellipse ( D2D1::Point2F ( pivot_x, pivot_y ), 4.0f, 4.0f ), g_pBrush );

	g_pBrush->SetColor ( D2D1::ColorF ( D2D1::ColorF::White ) );
	g_pRenderTarget->DrawText (
		PAUSE_HINT,
		static_cast<UINT32> ( wcslen ( PAUSE_HINT ) ),
		g_pTextFormat,
		D2D1::RectF ( 16.0f, 14.0f, 260.0f, 44.0f ),
		g_pBrush
	);

	hr = g_pRenderTarget->EndDraw ( );
	if ( hr == D2DERR_RECREATE_TARGET ) { discard_graphics_resources ( ); }

}

void toggle_pause ( HWND handle ) {

	UINT64 now = GetTickCount ( );

	if ( !g_paused ) {

		g_paused = true;
		g_pause_start_ms = now;
		SetWindowText ( handle, PAUSE_TITLE );

	}
	else {

		g_paused = false;
		g_pause_offset_ms += ( now - g_pause_start_ms );
		SetWindowText ( handle, MAIN_TITLE );

	}

	InvalidateRect ( handle, nullptr, FALSE );

}

void start_drag_if_close_to_bob ( HWND handle, int mouse_x, int mouse_y ) {

	float bob_x, bob_y;
	get_current_bob_position ( handle, bob_x, bob_y );

	double dx = mouse_x - bob_x;
	double dy = mouse_y - bob_y;
	double distance = sqrt ( dx * dx + dy * dy );

	if ( distance <= g_bob_radius * 1.5 ) {

		g_dragging = true;
		g_mouse_x = static_cast<float> ( mouse_x );
		g_mouse_y = static_cast<float> ( mouse_y );
		g_was_paused_before_drag = g_paused;

		if ( !g_paused ) {

			g_paused = true;
			g_pause_start_ms = GetTickCount ( );

		}

		SetWindowText ( handle, DRAG_TITLE );
		SetCapture ( handle );
		InvalidateRect ( handle, nullptr, FALSE );

	}

}

void finish_drag ( HWND handle, int mouse_x, int mouse_y ) {

	ReleaseCapture ( );
	g_dragging = false;

	float pivot_x, pivot_y;
	get_pivot ( handle, pivot_x, pivot_y );

	double dx = static_cast<double> ( mouse_x - pivot_x );
	double dy = static_cast<double> ( mouse_y - pivot_y );
	double new_length = sqrt ( dx * dx + dy * dy );
	if ( new_length < 10.0 ) { new_length = 10.0; }

	g_length_pixels = new_length;
	g_length_meters = g_length_pixels / PIXELS_PER_METER;
	g_initial_angle = atan2 ( dx, dy );

	UINT64 now = GetTickCount ( );
	g_start_time_ms = now;
	g_pause_offset_ms = 0;

	if ( g_was_paused_before_drag ) {

		g_paused = true;
		g_pause_start_ms = now;
		SetWindowText ( handle, PAUSE_TITLE );

	}
	else {

		g_paused = false;
		SetWindowText ( handle, MAIN_TITLE );

	}

	InvalidateRect ( handle, nullptr, FALSE );

}

LRESULT window_procedure ( HWND handle, UINT message_number, WPARAM word_param, LPARAM long_param ) {

	switch ( message_number ) {

		case WM_CREATE:
			D2D1CreateFactory ( D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pFactory );
			g_start_time_ms = GetTickCount ( );
			g_pause_offset_ms = 0;
			g_paused = false;
			SetTimer ( handle, TIMER_ID, TIMER_INTERVAL_MS, nullptr );
		return 0;

		case WM_SIZE:
			resize_render_target ( LOWORD ( long_param ), HIWORD ( long_param ) );
		return 0;

		case WM_TIMER:
			if ( word_param == TIMER_ID ) { InvalidateRect ( handle, nullptr, FALSE ); }
		return 0;

		case WM_KEYDOWN:
			if ( word_param == VK_SPACE ) { toggle_pause ( handle ); }
		return 0;

		case WM_LBUTTONDOWN:
			start_drag_if_close_to_bob ( handle, LOWORD ( long_param ), HIWORD ( long_param ) );
		return 0;

		case WM_MOUSEMOVE:
			if ( g_dragging ) {

				g_mouse_x = static_cast<float> ( LOWORD ( long_param ) );
				g_mouse_y = static_cast<float> ( HIWORD ( long_param ) );
				InvalidateRect ( handle, nullptr, FALSE );

			}
		return 0;

		case WM_LBUTTONUP:
			if ( g_dragging ) { finish_drag ( handle, LOWORD ( long_param ), HIWORD ( long_param ) ); }
		return 0;

		case WM_PAINT:
			paint_window ( handle );
			ValidateRect ( handle, nullptr );
		return 0;

		case WM_DESTROY:
			KillTimer ( handle, TIMER_ID );
			discard_graphics_resources ( );
			safe_release ( g_pTextFormat );
			safe_release ( g_pWriteFactory );
			safe_release ( g_pFactory );
			PostQuitMessage ( 0 );
		return 0;

	}

	return DefWindowProc ( handle, message_number, word_param, long_param );

}

int WINAPI wWinMain ( HINSTANCE handle_instance, HINSTANCE deprecated_instance, LPWSTR chain, int cmd ) {

	WNDCLASS wc = {

		( CS_HREDRAW | CS_VREDRAW ),        // Style
		window_procedure,                   // Procedure
		0,                                  // Class extra bytes
		0,                                  // Window extra bytes
		handle_instance,                    // Handle instance
		LoadIcon ( NULL, IDI_APPLICATION ), // Icon
		LoadCursor ( NULL, IDC_ARROW ),     // Cursor
		( HBRUSH ) ( COLOR_WINDOW + 1 ),    // Background brush
		NULL,                               // Pointer to menu name
		L"pendulum example"                 // Window class name

	};

	RegisterClass ( &wc );

	HWND handle = CreateWindow (
		L"pendulum example",
		MAIN_TITLE,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		640,
		480,
		nullptr,
		nullptr,
		handle_instance,
		nullptr
	);

	if ( handle != nullptr ) { ShowWindow ( handle, cmd ); }

	MSG message = { };
	while ( GetMessage ( &message, nullptr, 0, 0 ) ) {

		TranslateMessage ( &message );
		DispatchMessage ( &message );

	}

	return static_cast<int> ( message.wParam );

}
