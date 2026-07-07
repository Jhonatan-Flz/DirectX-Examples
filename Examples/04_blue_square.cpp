#include "include/common_direct2d.hpp"

constexpr int initial_window_width = 800;
constexpr int initial_window_height = 600;
constexpr float initial_square_size = 60;
constexpr float square_move_speed = 400;

struct square_app {

	ID2D1Factory* factory = nullptr;
	ID2D1HwndRenderTarget* render_target = nullptr;
	ID2D1SolidColorBrush* blue_brush = nullptr;

	int client_width = initial_window_width;
	int client_height = initial_window_height;

	float square_size = initial_square_size;
	float square_x = ( static_cast <float> ( initial_window_width ) - initial_square_size ) / 2;
	float square_y = ( static_cast <float> ( initial_window_height ) - initial_square_size ) / 2;

	float move_speed = square_move_speed;

};

square_app app;

inline float clamp_float ( float value, float low, float high ) { return maximum_float ( minimum_float ( value, high ), low ); }

HRESULT resources_create ( HWND handle ) {

	HRESULT result = S_OK;

	if ( ! app.factory ) { result = D2D1CreateFactory ( D2D1_FACTORY_TYPE_SINGLE_THREADED, &app.factory ); }
	if ( SUCCEEDED ( result ) && ! app.render_target ) {

		RECT client_rectangle;
		GetClientRect ( handle, &client_rectangle );

		D2D1_SIZE_U size = D2D1::SizeU (
			client_rectangle.right - client_rectangle.left,
			client_rectangle.bottom - client_rectangle.top
		);

		result = app.factory -> CreateHwndRenderTarget (
			D2D1::RenderTargetProperties (  ),
			D2D1::HwndRenderTargetProperties ( handle, size ),
			&app.render_target
		);

		if ( SUCCEEDED ( result ) ) {
			result = app.render_target -> CreateSolidColorBrush ( D2D1::ColorF ( D2D1::ColorF::Blue ), &app.blue_brush );
		}

	}

	return result;

}

void resources_discard (  ) {

	safe_release ( app.blue_brush );
	safe_release ( app.render_target );

}

void on_paint ( HWND handle ) {

	HRESULT result = resources_create ( handle );

	if ( SUCCEEDED ( result ) ) {

		app.render_target -> BeginDraw (  );
		app.render_target -> Clear ( D2D1::ColorF ( 0.176f, 0.176f, 0.176f ) );

		D2D1_RECT_F square_rectangle = D2D1::RectF (
			app.square_x,
			app.square_y,
			app.square_x + app.square_size,
			app.square_y + app.square_size
		);

		app.render_target -> FillRectangle ( square_rectangle, app.blue_brush );

		result = app.render_target -> EndDraw (  );

		if ( result == static_cast <HRESULT> ( D2DERR_RECREATE_TARGET ) ) { resources_discard (  ); }

	}

}

void on_resize ( UINT width, UINT height ) {

	app.client_width = static_cast <int> ( width );
	app.client_height = static_cast <int> ( height );

	if ( app.render_target ) { app.render_target -> Resize ( D2D1::SizeU ( width, height ) ); }

	app.square_x = clamp_float ( app.square_x, 0, static_cast <float> ( app.client_width ) - app.square_size );
	app.square_y = clamp_float ( app.square_y, 0, static_cast <float> ( app.client_height ) - app.square_size );

}

void update_square ( float delta_time ) {

	float horizontal = 0;
	float vertical = 0;

	if ( ( GetAsyncKeyState ( 'A' ) & 0x8000 ) ) { horizontal -= 1; }

	if ( ( GetAsyncKeyState ( 'D' ) & 0x8000 ) ) { horizontal += 1; }

	if ( ( GetAsyncKeyState ( 'W' ) & 0x8000 ) ) { vertical -= 1; }

	if ( ( GetAsyncKeyState ( 'S' ) & 0x8000 ) ) { vertical += 1; }

	// Normalize so moving diagonally isn't faster than moving on one axis.
	float magnitude_squared = horizontal * horizontal + vertical * vertical;

	if ( magnitude_squared > 0 ) {

		float inverse_magnitude = 1 / std::sqrt ( magnitude_squared );
		horizontal *= inverse_magnitude;
		vertical *= inverse_magnitude;

	}

	app.square_x += horizontal * app.move_speed * delta_time;
	app.square_y += vertical * app.move_speed * delta_time;

	float max_x = static_cast <float> ( app.client_width ) - app.square_size;
	float max_y = static_cast <float> ( app.client_height ) - app.square_size;

	app.square_x = clamp_float ( app.square_x, 0, max_x );
	app.square_y = clamp_float ( app.square_y, 0, max_y );

}

LRESULT window_procedure ( HWND handle, UINT message_number, WPARAM word_param, LPARAM long_param ) {

	switch ( message_number ) {

		case WM_SIZE: {

			on_resize ( LOWORD ( long_param ), HIWORD ( long_param ) );
			return 0;

		}

		case WM_ERASEBKGND: {

			return 1;

		}

		case WM_PAINT: {

			PAINTSTRUCT paint_struct;
			BeginPaint ( handle, &paint_struct );
			on_paint ( handle );
			EndPaint ( handle, &paint_struct );
			return 0;

		}

		case WM_KEYDOWN: {

			if ( word_param == VK_ESCAPE ) {
				PostQuitMessage ( 0 );
			}

			return 0;

		}

		case WM_DESTROY: {

			resources_discard (  );
			safe_release ( app.factory );
			PostQuitMessage ( 0 );
			return 0;

		}

	}

	return DefWindowProc ( handle, message_number, word_param, long_param );

}

int wWinMain ( HINSTANCE handle_instance, HINSTANCE deprecated_instance, PWSTR, int cmd ) {

     WNDCLASS wc = {

          CS_HREDRAW | CS_VREDRAW,                    // style
          window_procedure,                           // lpfnWndProc
          0,                                          // cbClsExtra
          0,                                          // cbWndExtra
          handle_instance,                            // hInstance
          LoadIcon ( NULL, IDI_APPLICATION ),         // hIcon
          LoadCursor ( NULL, IDC_ARROW ),             // hCursor
          ( HBRUSH ) GetStockObject ( BLACK_BRUSH ),  // Bacground
          NULL,                                       // lpszMenuName
          L"blue_square_window_class"                 // lpszClassName
     
	};

     RegisterClass ( &wc );

     RECT window_rectangle = { 0, 0, app.client_width, app.client_height };
     AdjustWindowRect ( &window_rectangle, WS_OVERLAPPEDWINDOW, FALSE );

     HWND handle = CreateWindow (
          L"blue_square_window_class",
          L"Blue Square - WASD to Move, Esc to Quit",
          WS_OVERLAPPEDWINDOW,
          CW_USEDEFAULT, CW_USEDEFAULT,
          window_rectangle.right - window_rectangle.left,
          window_rectangle.bottom - window_rectangle.top,
          nullptr, nullptr, handle_instance, nullptr
     );

     if ( !handle ) {
          MessageBox ( nullptr, L"Failed to create the window.", L"Error", MB_ICONERROR );
          return 0;
     }

     ShowWindow ( handle, cmd );
     UpdateWindow ( handle );

     LARGE_INTEGER frequency;
     QueryPerformanceFrequency ( &frequency );

     LARGE_INTEGER previous_time;
     QueryPerformanceCounter ( &previous_time );

     MSG message = { };

     while ( message.message != WM_QUIT ) {

          if ( PeekMessage ( &message, nullptr, 0, 0, PM_REMOVE ) ) {
          
			TranslateMessage ( &message );
               DispatchMessage ( &message );
          
		} else {

               LARGE_INTEGER current_time;
               QueryPerformanceCounter ( &current_time );

               float delta_time = static_cast <float> ( current_time.QuadPart - previous_time.QuadPart ) / static_cast <float> ( frequency.QuadPart );
               previous_time = current_time;

               update_square ( delta_time );
               InvalidateRect ( handle, nullptr, FALSE );

          }

     }

     return static_cast <int> ( message.wParam );

}