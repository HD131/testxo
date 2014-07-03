#include "Tank.h"

LRESULT WINAPI WndProc( HWND hwnd, UINT Message, WPARAM wparam, LPARAM lparam )
{
	switch( Message )
	{
	case WM_CLOSE:
		{
			CGame::m_bEndGame = true;
			break;
		}

	case WM_KEYDOWN:
		{
			if( wparam == VK_ESCAPE )
				CGame::m_bEndGame = true;

			break;
		}
	}

	return DefWindowProc( hwnd, Message, wparam, lparam );
}

#include "String.h"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	MSG	Msg;
	String str1 = "1235456";
	String str2 = "0987654321";
	String str3 = str1 + str2;
	str1 = str2;
	str2 = "000";
	str2[2] = 50;
	str2[1] = str1[3];
	str2 += str1;
	str2 += "55555";

	if( str1 == "000" )
	{
		int a = 0;
	}

	String s;
	if( "000" != s )
	{
		int a = 0;
	}

	size_t n = str2.Find( "765" );

	remove( "log.txt" );
	Log("Begin");

	CGame g_Game;

	if( HWND hwnd = g_Game.Init( hInstance, WndProc ) )
	{
		ShowWindow( hwnd, nCmdShow );
		ZeroMemory( &Msg, sizeof( MSG ) );
		g_Game.InitInputDevice();
		DWORD FPS  = 0;
		float time_last = static_cast<float>( timeGetTime() );
		float time_cur  = time_last;
		float delta     = 0.f;
		float fps_time  = time_last;

		while( !CGame::m_bEndGame )
		{
			time_cur = static_cast<float>( timeGetTime() );

			g_Game.Update( delta );
			g_Game.RenderingScene();

			if( PeekMessage( &Msg, 0, 0, 0, PM_REMOVE ) )
			{
				TranslateMessage( &Msg );
				DispatchMessage( &Msg );
			}

			++FPS;
			time_last = static_cast<float>( timeGetTime() );
			delta = ( time_last - time_cur ) / 1000.f;

			if( ( time_cur - fps_time ) > 1000.f )
			{
				fps_time = time_cur;
				static char t[32] = {0};
				sprintf( t, "Tanks FPS: %d, delat: %f", FPS, delta );
				SetWindowText( hwnd, t );
				FPS = 0;
			}
		}

		g_Game.Release();
	}
	Log("End");

	return 0;
}

//-----------------------------------------------------------------------------------------------------------------------------------