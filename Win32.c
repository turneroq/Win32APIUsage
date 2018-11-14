#include <Windows.h>
#include <stdint.h>
#include <stdbool.h>

#define internal		static
#define local_persist   static
#define global_variable static

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;

global_variable bool Running;
global_variable void *BitMapMemory;
global_variable int  BitMapWidth;
global_variable int  BitMapHeight;
global_variable BITMAPINFO BitMapInfo;
global_variable int BytesPerPixel = 4;



internal void
RenderGradient(int BlueOffset, int GreenOffset)
{
	int Width = BitMapWidth;

	uint8 *Row = (uint8 *)BitMapMemory;	

	for (int Y = 0; Y < BitMapHeight; ++Y)
	{
		uint32 *Pixel = (uint32 *)Row;
		for (int X = 0; X < BitMapWidth; ++X)
		{			
			uint8 Blue = (X + BlueOffset);
			uint8 Green = (Y + GreenOffset);
			uint8 Red = 255;
			*Pixel++ = ((Red << 16) | (Green << 8) | Blue);
		}
		Row = (uint8 *)Pixel;
	}
}



internal void
Win32ResizeDIBSection(int Width, int Height)
{
	if (BitMapMemory)
	{
		VirtualFree(BitMapMemory, 0, MEM_RELEASE);
	}
	BitMapWidth = Width;
	BitMapHeight = Height;

	BitMapInfo.bmiHeader.biSize = sizeof(BitMapInfo.bmiHeader);
	BitMapInfo.bmiHeader.biWidth = BitMapWidth;
	BitMapInfo.bmiHeader.biHeight = -BitMapHeight;
	BitMapInfo.bmiHeader.biPlanes = 1;
	BitMapInfo.bmiHeader.biBitCount = 32;
	BitMapInfo.bmiHeader.biCompression = BI_RGB;

	int BitMapMemorySize = BitMapWidth * BitMapHeight * BytesPerPixel;	
	BitMapMemory = VirtualAlloc(0, BitMapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}



internal void
Win32UpdateWindow(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height)
{
	int WindowWidth = ClientRect->right - ClientRect->left;
	int WindowHeight = ClientRect->bottom - ClientRect->top;
	StretchDIBits(
		DeviceContext,		
		0, 0, BitMapWidth, BitMapHeight,
		0, 0, WindowWidth, WindowHeight,
		BitMapMemory,
		&BitMapInfo,
		DIB_RGB_COLORS,
		SRCCOPY
	);
}



LRESULT CALLBACK
Win32MainWindowCallback(HWND   Window,
	UINT   Message,
	WPARAM WParam,
	LPARAM LParam)
{
	LRESULT Result = 0;
	switch (Message)
	{
	case WM_SIZE:
	{
		RECT ClientRect;
		GetClientRect(Window, &ClientRect);
		LONG Height = ClientRect.bottom - ClientRect.top;
		LONG Width = ClientRect.right - ClientRect.left;
		Win32ResizeDIBSection(Width, Height);
		OutputDebugStringA("WM_SIZE\n");
	}break;
	case WM_DESTROY:
	{
		Running = false;
	}break;
	case WM_CLOSE:
	{
		Running = false;
	}break;
	case WM_ACTIVATEAPP:
	{
		OutputDebugStringA("WM_ACTIVATEAPP\n");
	}break;
	case WM_PAINT:
	{
		PAINTSTRUCT Paint;
		HDC DeviceContext = BeginPaint(Window, &Paint);
		LONG X = Paint.rcPaint.left;
		LONG Y = Paint.rcPaint.top;
		LONG Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
		LONG Width = Paint.rcPaint.right - Paint.rcPaint.left;

		RECT ClientRect;
		GetClientRect(Window, &ClientRect);

		Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
		EndPaint(Window, &Paint);
	}break;
	default: {Result = DefWindowProc(Window, Message, WParam, LParam); }break;
	}
	return Result;
}



int WINAPI WinMain(
	HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR CommandLine,
	int ShowCode
)
{
	WNDCLASS WindowClass = {0};
	WindowClass.style =
		CS_OWNDC |
		CS_HREDRAW |
		CS_VREDRAW;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = Instance;
	WindowClass.lpszClassName = "nedoGameWindowClass";

	if (RegisterClass(&WindowClass))
	{
		HWND Window =
			CreateWindowEx(
				0,
				WindowClass.lpszClassName,
				"nedoGameWindow",
				WS_OVERLAPPEDWINDOW | WS_VISIBLE,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				0,
				0,
				Instance,
				0);
		if (Window)
		{
			Running = true;
			int XOffset = 0, YOffset = 0;
			while (Running)
			{
				MSG Message;				
				while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{
					if (Message.message == WM_QUIT)
					{
						Running = false;
					}
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}
				RenderGradient(XOffset, YOffset);
				++XOffset;
				++YOffset;
				HDC DeviceContext = GetDC(Window);
				RECT ClientRect;
				GetClientRect(Window, &ClientRect);
				int WindowWidth = ClientRect.right - ClientRect.left;
				int WindowHeight = ClientRect.bottom - ClientRect.top;
				Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
				ReleaseDC(Window, DeviceContext);

			}
		}
		else
		{

		}
	}
	else
	{

	}
	return 0;
}