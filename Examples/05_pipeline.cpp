#include "include/common.hpp"

constexpr int initial_window_width = 1000;
constexpr int initial_window_height = 780;

struct stage_content {

    const wchar_t* title;
    const wchar_t* body;

};

const stage_content stages [  ] = {

    {
        L"The Direct2D Pipeline",
        L"Direct2D is a 2D vector graphics API that runs on top of Direct3D 11. You never touch its shader "
        L"stages directly, but your draw calls still pass through several distinct steps before becoming "
        L"pixels." 
	   L"This walkthrough follows one triangle through each step, using the real Direct2D calls involved. "
	   L"Press Space to begin."
    },
    {
        L"1. Vertex Data",
        L"Every shape starts as raw geometric data - points in 2D space, stored as D2D1_POINT_2F structures. "
        L"Below are the three corners of our triangle. At this stage they're just numbers sitting in memory; "
        L"nothing has been drawn yet."
    },
    {
        L"2. Building the Geometry",
        L"The points are assembled into an ID2D1PathGeometry through a geometry sink: BeginFigure marks the "
        L"start, AddLine connects each corner, EndFigure closes the shape. This produces a CPU-side "
        L"description of the outline - still not rasterized or colored."
    },
    {
        L"3. Transform",
        L"Before drawing, Direct2D can apply a D2D1::Matrix3x2F to rotate, scale, translate, or skew geometry "
        L"- without touching the original point data at all. The faint triangle below is the original; the "
        L"solid one has a rotation and scale applied."
    },
    {
        L"4. Rasterization",
        L"Rasterization converts the vector outline into pixel coverage - for every pixel, the GPU works out "
        L"whether (and how much) the shape covers it. This grid is a simplified stand-in for that test: each "
        L"square is a sample point checked against the triangle."
    },
    {
        L"5. Brush & Fill",
        L"A Brush supplies the color or pattern applied to covered pixels - solid colors, gradients, or "
        L"bitmaps. FillGeometry combines the rasterized coverage with the brush to produce the final "
        L"anti-aliased shape, similar in spirit to a pixel shader in a 3D pipeline."
    },
    {
        L"6. Composition & Present",
        L"Everything drawn between BeginDraw (  ) and EndDraw (  ) is batched together. EndDraw (  ) flushes those "
        L"commands to the render target, and for a window, the finished frame is presented to screen. A real "
        L"scene composites many shapes this way, not just one."
    },
    {
        L"Recap",
        L"Six steps take a shape from raw numbers to pixels on screen:"
    },

};

constexpr int stage_count = sizeof ( stages ) / sizeof ( stages [0] );
struct pipeline_app {

	ID2D1Factory* factory = nullptr;
	ID2D1HwndRenderTarget* render_target = nullptr;
	IDWriteFactory* dwrite_factory = nullptr;

	IDWriteTextFormat* title_format = nullptr;
	IDWriteTextFormat* body_format = nullptr;
	IDWriteTextFormat* label_format = nullptr;
	IDWriteTextFormat* caption_format = nullptr;
	IDWriteTextFormat* hint_format = nullptr;

	ID2D1SolidColorBrush* text_brush = nullptr;
	ID2D1SolidColorBrush* accent_brush = nullptr;
	ID2D1SolidColorBrush* faint_brush = nullptr;
	ID2D1SolidColorBrush* point_brush = nullptr;
	ID2D1SolidColorBrush* second_shape_brush = nullptr;

	int client_width = initial_window_width;
	int client_height = initial_window_height;

	int current_stage = 0;

};

pipeline_app app;
bool point_in_triangle ( D2D1_POINT_2F point, D2D1_POINT_2F a, D2D1_POINT_2F b, D2D1_POINT_2F c ) {

	auto sign = [  ] ( D2D1_POINT_2F p1, D2D1_POINT_2F p2, D2D1_POINT_2F p3 ) { 
			
		return ( p1.x - p3.x ) * ( p2.y - p3.y ) - ( p2.x - p3.x ) * ( p1.y - p3.y ); 

	};

    float d1 = sign ( point, a, b );
    float d2 = sign ( point, b, c );
    float d3 = sign ( point, c, a );

    bool has_negative = ( d1 < 0 ) || ( d2 < 0 ) || ( d3 < 0 );
    bool has_positive = ( d1 > 0 ) || ( d2 > 0 ) || ( d3 > 0 );

    return !( has_negative && has_positive );

}

HRESULT resources_create ( HWND hwnd ) {

	HRESULT result = S_OK;

	if ( !app.factory ) { result = D2D1CreateFactory ( D2D1_FACTORY_TYPE_SINGLE_THREADED, &app.factory ); }

	if ( SUCCEEDED ( result ) && ! app.dwrite_factory ) {

		result = DWriteCreateFactory (

			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof ( IDWriteFactory ),
			reinterpret_cast<IUnknown**> ( &app.dwrite_factory )
		
		);

	}

	if ( SUCCEEDED ( result ) && !app.title_format ) {

		result = app.dwrite_factory -> CreateTextFormat (
		
			L"Segoe UI", nullptr,
			DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
			30.0f, L"en-us",
			&app.title_format
		
		);

		if ( SUCCEEDED ( result ) ) { app.title_format -> SetWordWrapping ( DWRITE_WORD_WRAPPING_NO_WRAP ); }

	}

	if ( SUCCEEDED ( result ) && ! app.body_format ) {

		result = app.dwrite_factory -> CreateTextFormat (
		
			L"Segoe UI", nullptr,
			DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
			18.0f, L"en-us",
			&app.body_format
		
		);

		if ( SUCCEEDED ( result ) ) { app.body_format -> SetLineSpacing ( DWRITE_LINE_SPACING_METHOD_UNIFORM, 26.0f, 21.0f ); }

	}

	if ( SUCCEEDED ( result ) && ! app.label_format ) {

		result = app.dwrite_factory -> CreateTextFormat (

			L"Segoe UI", nullptr,
			DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
			14.0f, L"en-us",
			&app.label_format
		
		);

	}

	if ( SUCCEEDED ( result ) && ! app.caption_format ) {

		result = app.dwrite_factory -> CreateTextFormat (

			L"Segoe UI", nullptr,
			DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_ITALIC, DWRITE_FONT_STRETCH_NORMAL,
			14.0f, L"en-us",
			&app.caption_format
		
		);

	}

	if ( SUCCEEDED ( result ) && ! app.hint_format ) {

		result = app.dwrite_factory -> CreateTextFormat (

			L"Segoe UI", nullptr,
			DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
			14.0f, L"en-us",
			&app.hint_format
		
		);

		if ( SUCCEEDED ( result ) ) { app.hint_format -> SetTextAlignment ( DWRITE_TEXT_ALIGNMENT_CENTER ); }

	}

	if ( SUCCEEDED ( result ) && !app.render_target ) {

		RECT client_rectangle;
		GetClientRect ( hwnd, &client_rectangle );

		D2D1_SIZE_U size = D2D1::SizeU ( client_rectangle.right - client_rectangle.left, client_rectangle.bottom - client_rectangle.top );
		result = app.factory -> CreateHwndRenderTarget ( D2D1::RenderTargetProperties (  ), D2D1::HwndRenderTargetProperties ( hwnd, size ), &app.render_target );

	}

	if ( SUCCEEDED ( result ) && ! app.text_brush ) {
	
		result = app.render_target -> CreateSolidColorBrush ( D2D1::ColorF ( D2D1::ColorF::Black ), &app.text_brush );
	
	}

	if ( SUCCEEDED ( result ) && ! app.accent_brush ) {
	
		result = app.render_target -> CreateSolidColorBrush ( D2D1::ColorF ( 0.16f, 0.38f, 0.92f ), &app.accent_brush );
	
	}

	if ( SUCCEEDED ( result ) && ! app.faint_brush ) {
	
		result = app.render_target -> CreateSolidColorBrush ( D2D1::ColorF ( 0.75f, 0.75f, 0.78f ), &app.faint_brush );
	
	}

	if ( SUCCEEDED ( result ) && ! app.point_brush ) {
	
		result = app.render_target -> CreateSolidColorBrush ( D2D1::ColorF ( 0.92f, 0.42f, 0.13f ), &app.point_brush );
	
	}

	if ( SUCCEEDED ( result ) && ! app.second_shape_brush ) {

		result = app.render_target -> CreateSolidColorBrush ( D2D1::ColorF ( 0.20f, 0.70f, 0.45f ), &app.second_shape_brush );
	
	}

	return result;

}

void resources_discard (  ) {

    safe_release ( app.second_shape_brush );
    safe_release ( app.point_brush );
    safe_release ( app.faint_brush );
    safe_release ( app.accent_brush );
    safe_release ( app.text_brush );
    safe_release ( app.render_target );

}

HRESULT create_triangle_geometry ( D2D1_POINT_2F a, D2D1_POINT_2F b, D2D1_POINT_2F c, ID2D1PathGeometry** out_geometry ) {

	HRESULT result = app.factory -> CreatePathGeometry ( out_geometry );

	if ( SUCCEEDED ( result ) ) {

		ID2D1GeometrySink* sink = nullptr;
		result = ( *out_geometry ) -> Open ( &sink );

		if ( SUCCEEDED ( result ) ) {

			sink -> BeginFigure ( a, D2D1_FIGURE_BEGIN_FILLED );
			sink -> AddLine ( b );
			sink -> AddLine ( c );
			sink -> EndFigure ( D2D1_FIGURE_END_CLOSED );

			result = sink -> Close (  );

			safe_release ( sink );

		}

    	}

    	return result;

}

HRESULT create_pipeline_gradient_brush ( D2D1_POINT_2F start, D2D1_POINT_2F end, ID2D1LinearGradientBrush** out_brush ) {

	D2D1_GRADIENT_STOP gradient_stops [2];
	gradient_stops [0].position = 0.0f;
	gradient_stops [0].color = D2D1::ColorF ( 0.16f, 0.38f, 0.92f );
	gradient_stops [1].position = 1.0f;
	gradient_stops [1].color = D2D1::ColorF ( 0.55f, 0.22f, 0.85f );

	ID2D1GradientStopCollection* stops = nullptr;
	HRESULT result = app.render_target -> CreateGradientStopCollection ( gradient_stops, 2, &stops );

	if ( SUCCEEDED ( result ) ) { 
		
		result = app.render_target -> CreateLinearGradientBrush ( D2D1::LinearGradientBrushProperties ( start, end ), stops, out_brush );

	}

	safe_release ( stops );
	return result;

}

void draw_triangle_outline ( D2D1_POINT_2F a, D2D1_POINT_2F b, D2D1_POINT_2F c, ID2D1Brush* brush, float stroke_width ) {

    app.render_target -> DrawLine ( a, b, brush, stroke_width );
    app.render_target -> DrawLine ( b, c, brush, stroke_width );
    app.render_target -> DrawLine ( c, a, brush, stroke_width );

}

void draw_point_dot ( D2D1_POINT_2F point ) {

    D2D1_ELLIPSE dot = D2D1::Ellipse ( point, 6.0f, 6.0f );
    app.render_target -> FillEllipse ( dot, app.point_brush );

}

void draw_point_label ( D2D1_POINT_2F point, const wchar_t* name, float offset_x, float offset_y ) {

    wchar_t buffer[64];
    swprintf ( buffer, 64, L"%ls (%.0f, %.0f)", name, point.x, point.y );

    D2D1_RECT_F label_rect = D2D1::RectF (
        point.x + offset_x, point.y + offset_y,
        point.x + offset_x + 170.0f, point.y + offset_y + 22.0f
    );

    app.render_target -> DrawText ( buffer, static_cast<UINT32> ( wcslen ( buffer ) ), app.label_format, label_rect, app.point_brush );

}

void draw_caption ( const wchar_t* text ) {

    D2D1_RECT_F caption_rect = D2D1::RectF (
        60.0f, static_cast<float> ( app.client_height ) - 92.0f,
        static_cast<float> ( app.client_width ) - 60.0f, static_cast<float> ( app.client_height ) - 58.0f
    );

    app.render_target -> DrawText ( text, static_cast<UINT32> ( wcslen ( text ) ), app.caption_format, caption_rect, app.faint_brush );

}

void draw_header (  ) {

    float margin = 60.0f;
    float width = static_cast<float> ( app.client_width ) - ( margin * 2.0f );

    D2D1_RECT_F title_rect = D2D1::RectF ( margin, 34.0f, margin + width, 74.0f );
    D2D1_RECT_F body_rect = D2D1::RectF ( margin, 82.0f, margin + width, 200.0f );

    const stage_content& content = stages[ app.current_stage ];

    app.render_target -> DrawText (
        content.title, static_cast<UINT32> ( wcslen ( content.title ) ),
        app.title_format, title_rect, app.accent_brush
    );

    app.render_target -> DrawText (
        content.body, static_cast<UINT32> ( wcslen ( content.body ) ),
        app.body_format, body_rect, app.text_brush
    );

}

void draw_hint_bar (  ) {

    wchar_t buffer[160];
    swprintf ( buffer, 160, L"Step %d of %d   \u00B7   Space: next   \u00B7   Backspace: previous   \u00B7   Esc: quit", app.current_stage + 1, stage_count );

    D2D1_RECT_F hint_rect = D2D1::RectF (
        0.0f, static_cast<float> ( app.client_height ) - 42.0f,
        static_cast<float> ( app.client_width ), static_cast<float> ( app.client_height ) - 14.0f
    );

    app.render_target -> DrawText ( buffer, static_cast<UINT32> ( wcslen ( buffer ) ), app.hint_format, hint_rect, app.faint_brush );

}

void draw_diagram (  ) {

	float center_x = static_cast<float> ( app.client_width ) / 2.0f;
	float center_y = static_cast<float> ( app.client_height ) * 0.58f;

	D2D1_POINT_2F center = { center_x, center_y };

	D2D1_POINT_2F vertex_a = { center.x, center.y - 140.0f };
	D2D1_POINT_2F vertex_b = { center.x - 140.0f, center.y + 110.0f };
	D2D1_POINT_2F vertex_c = { center.x + 140.0f, center.y + 110.0f };

	switch ( app.current_stage ) {

		case 0: {

			draw_triangle_outline ( vertex_a, vertex_b, vertex_c, app.faint_brush, 2.0f );
			break;

		}

		case 1: {

			draw_point_dot ( vertex_a );
			draw_point_dot ( vertex_b );
			draw_point_dot ( vertex_c );

			draw_point_label ( vertex_a, L"A", 12.0f, -30.0f );
			draw_point_label ( vertex_b, L"B", -172.0f, 4.0f );
			draw_point_label ( vertex_c, L"C", 12.0f, 4.0f );

			break;

		}

		case 2: {

			ID2D1PathGeometry* geometry = nullptr;

			if ( SUCCEEDED ( create_triangle_geometry ( vertex_a, vertex_b, vertex_c, &geometry ) ) ) { 
				
				app.render_target -> DrawGeometry ( geometry, app.accent_brush, 2.5f );
			
			}

			safe_release ( geometry );

			draw_point_dot ( vertex_a );
			draw_point_dot ( vertex_b );
			draw_point_dot ( vertex_c );

			draw_caption ( L"ID2D1PathGeometry  \u00B7  BeginFigure -> AddLine -> AddLine -> EndFigure -> Close" );

			break;

		}

		case 3: {

			D2D1::Matrix3x2F transform =
				D2D1::Matrix3x2F::Rotation ( 24.0f, center ) *
				D2D1::Matrix3x2F::Scale ( D2D1::SizeF ( 0.82f, 0.82f ), center );

			D2D1_POINT_2F transformed_a = transform.TransformPoint ( vertex_a );
			D2D1_POINT_2F transformed_b = transform.TransformPoint ( vertex_b );
			D2D1_POINT_2F transformed_c = transform.TransformPoint ( vertex_c );

			draw_triangle_outline ( vertex_a, vertex_b, vertex_c, app.faint_brush, 1.5f );
			draw_triangle_outline ( transformed_a, transformed_b, transformed_c, app.accent_brush, 2.5f );

			draw_point_dot ( transformed_a );
			draw_point_dot ( transformed_b );
			draw_point_dot ( transformed_c );

			draw_caption ( L"D2D1::Matrix3x2F  \u00B7  Rotation ( 24\u00B0 ) x Scale ( 0.82 )  \u00B7  faint = before, solid = after" );

			break;

		}

		case 4: {

			float cell_size = 18.0f;
			
			float min_x = minimum_float ( vertex_a.x, minimum_float ( vertex_b.x, vertex_c.x ) );
			float min_y = minimum_float ( vertex_a.y, minimum_float ( vertex_b.y, vertex_c.y ) );

			float max_x = maximum_float ( vertex_a.x, maximum_float ( vertex_b.x, vertex_c.x ) );
			float max_y = maximum_float ( vertex_a.y, maximum_float ( vertex_b.y, vertex_c.y ) );
			
			for ( float y = min_y; y < max_y; y += cell_size ) {

				for ( float x = min_x; x < max_x; x += cell_size ) {

					D2D1_POINT_2F sample = { x + cell_size * 0.5f, y + cell_size * 0.5f };
					if ( point_in_triangle ( sample, vertex_a, vertex_b, vertex_c ) ) {

						D2D1_RECT_F cell = D2D1::RectF ( x + 1.5f, y + 1.5f, x + cell_size - 1.5f, y + cell_size - 1.5f );
						app.render_target -> FillRectangle ( cell, app.accent_brush );

					}

				}

			}

			draw_triangle_outline ( vertex_a, vertex_b, vertex_c, app.faint_brush, 1.5f );
			draw_caption ( L"Simplified sample-coverage test  \u00B7  the real rasterizer runs on the GPU with anti-aliasing" );
			break;

		}

		case 5: {

			ID2D1LinearGradientBrush* gradient_brush = nullptr;
			if ( SUCCEEDED ( create_pipeline_gradient_brush ( vertex_b, vertex_c, &gradient_brush ) ) ) {

				ID2D1PathGeometry* geometry = nullptr;
				if ( SUCCEEDED ( create_triangle_geometry ( vertex_a, vertex_b, vertex_c, &geometry ) ) ) {

					app.render_target -> FillGeometry ( geometry, gradient_brush );
					app.render_target -> DrawGeometry ( geometry, app.accent_brush, 1.5f );

				}

				safe_release ( geometry );

			}

			safe_release ( gradient_brush );
			draw_caption ( L"ID2D1LinearGradientBrush  \u00B7  FillGeometry combines coverage + brush color" );
			break;

		}

		case 6: {

			ID2D1LinearGradientBrush* gradient_brush = nullptr;
			if ( SUCCEEDED ( create_pipeline_gradient_brush ( vertex_b, vertex_c, &gradient_brush ) ) ) {

				ID2D1PathGeometry* geometry = nullptr;
				if ( SUCCEEDED ( create_triangle_geometry ( vertex_a, vertex_b, vertex_c, &geometry ) ) ) {

					app.render_target -> FillGeometry ( geometry, gradient_brush );
					app.render_target -> DrawGeometry ( geometry, app.accent_brush, 1.5f );

				}

				safe_release ( geometry );

			}

			safe_release ( gradient_brush );
			D2D1_POINT_2F second_a = { center.x - 260.0f, center.y + 130.0f };
			D2D1_POINT_2F second_b = { center.x - 335.0f, center.y + 195.0f };
			D2D1_POINT_2F second_c = { center.x - 195.0f, center.y + 195.0f };

			ID2D1PathGeometry* second_geometry = nullptr;

			if ( SUCCEEDED ( create_triangle_geometry ( second_a, second_b, second_c, &second_geometry ) ) ) {

				app.render_target -> FillGeometry ( second_geometry, app.second_shape_brush );
			
			}

			safe_release ( second_geometry );
			draw_caption ( L"BeginDraw (  ) -> draw calls -> EndDraw (  ) -> Present  \u00B7  a real scene composites many shapes" );
			break;

		}

		case 7: {

			std::wstring recap = L"";
			for ( int index = 1; index <= 6; ++index ) {

				recap += L"\u2713  ";
				recap += stages [index].title;
				recap += L"\n";

			}

			D2D1_RECT_F recap_rect = D2D1::RectF ( center.x - 220.0f, center.y - 170.0f, center.x + 220.0f, center.y + 170.0f );
			app.render_target -> DrawText (


				recap.c_str (  ), static_cast <UINT32> ( recap.length (  ) ),
				app.body_format, recap_rect, app.text_brush
			
			);

			break;

		}

	}

}

void on_paint ( HWND hwnd ) {

	HRESULT result = resources_create ( hwnd );

	if ( SUCCEEDED ( result ) ) {

		app.render_target -> BeginDraw (  );

		app.render_target -> Clear ( D2D1::ColorF ( D2D1::ColorF::White ) );

		draw_header (  );
		draw_diagram (  );
		draw_hint_bar (  );

		result = app.render_target -> EndDraw (  );

		if ( result == static_cast<HRESULT> ( D2DERR_RECREATE_TARGET ) ) { resources_discard (  ); }

	}

}

void on_resize ( UINT width, UINT height ) {

	app.client_width = static_cast<int> ( width );
	app.client_height = static_cast<int> ( height );

	if ( app.render_target ) { app.render_target -> Resize ( D2D1::SizeU ( width, height ) ); }

}

LRESULT CALLBACK window_procedure ( HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param ) {

	switch ( message ) {

		case WM_SIZE: {

			on_resize ( LOWORD ( l_param ), HIWORD ( l_param ) );
			InvalidateRect ( hwnd, nullptr, FALSE );
			return 0;

		}

		case WM_ERASEBKGND: {

			return 1;

		}

		case WM_PAINT: {

			PAINTSTRUCT paint_struct;
			BeginPaint ( hwnd, &paint_struct );
			on_paint ( hwnd );
			EndPaint ( hwnd, &paint_struct );
			return 0;

		}

		case WM_KEYDOWN: {

			if ( w_param == VK_ESCAPE ) { 
				
				PostQuitMessage ( 0 ); 
			
			}

			if ( w_param == VK_SPACE ) { 
				
				app.current_stage = ( app.current_stage + 1 ) % stage_count; 
				InvalidateRect ( hwnd, nullptr, FALSE ); 
			
			}

			if ( w_param == VK_BACK ) { 
				
				app.current_stage = ( app.current_stage + stage_count - 1 ) % stage_count; 
				InvalidateRect ( hwnd, nullptr, FALSE ); 
			
			}

			return 0;

		}

		case WM_DESTROY: {

			resources_discard (  );
			safe_release ( app.title_format );
			safe_release ( app.body_format );
			safe_release ( app.label_format );
			safe_release ( app.caption_format );
			safe_release ( app.hint_format );
			safe_release ( app.dwrite_factory );
			safe_release ( app.factory );
			PostQuitMessage ( 0 );
			return 0;

		}

	}

	return DefWindowProc ( hwnd, message, w_param, l_param );

}

int WINAPI wWinMain ( HINSTANCE handle_instance, HINSTANCE, PWSTR, int show_command ) {

	WNDCLASS wc = {

		( CS_HREDRAW | CS_VREDRAW ),               // Style
		window_procedure,                          // Procedure
		0,                                         // Class extra bytes
		0,                                         // Window extra bytes
		handle_instance,                           // Handle instance
		LoadIcon ( NULL, IDI_APPLICATION ),        // Icon
		LoadCursor ( NULL, IDC_ARROW ),            // Cursor
		( HBRUSH ) GetStockObject ( WHITE_BRUSH ), // Background brush
		NULL,                                      // Pointer to menu name
		L"direct2d pipeline walkthrough"           // Window class name

	};

	RegisterClass ( &wc );

	RECT window_rectangle = { 0, 0, app.client_width, app.client_height };
	AdjustWindowRectEx ( &window_rectangle, WS_OVERLAPPEDWINDOW, FALSE, 0 );

	HWND hwnd = CreateWindowEx (

		0,
		L"direct2d pipeline walkthrough",
		L"The Direct2D Pipeline - Space: Next, Backspace: Previous, Esc: Quit",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		window_rectangle.right - window_rectangle.left,
		window_rectangle.bottom - window_rectangle.top,
		nullptr, nullptr, handle_instance, nullptr
    
	);

	if ( !hwnd ) {

		MessageBox ( nullptr, L"Failed to create the window.", L"Error", MB_ICONERROR );
		return 0;

	}

	ShowWindow ( hwnd, show_command );
	UpdateWindow ( hwnd );

	MSG message = { };
	while ( GetMessage ( &message, nullptr, 0, 0 ) > 0 ) {

     	TranslateMessage ( &message );
     	DispatchMessage ( &message );

	}

	return static_cast<int> ( message.wParam );

}
