#include "pch.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <exception>
#include "HookGod.h"
#include <fmt/core.h>

BOOL CALLBACK getProcessWindowCallback(_In_ HWND hwnd, _In_ LPARAM lParam) {
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	if (pid == GetCurrentProcessId()) {
		*reinterpret_cast<HWND*>(lParam) = hwnd;
		return FALSE;
	}
	return TRUE;
}

HWND getProcessWindow() {
	HWND window = nullptr;
	EnumWindows(getProcessWindowCallback, reinterpret_cast<LPARAM>(&window));
	if (window == nullptr)
		throw std::exception("no window found sorry man");
	return window;
}

/*class HackD9 : IDirect3DDevice9 {
public:
	HRESULT Present(
		const RECT* pSourceRect,
		const RECT* pDestRect,
		HWND          hDestWindowOverride,
		const RGNDATA* pDirtyRegion
	) {
	}
};*/

Hooking::TrampolineHook hook;

HRESULT IDirect3DDevice9::Present(
	const RECT* pSourceRect,
	const RECT* pDestRect,
	HWND          hDestWindowOverride,
	const RGNDATA* pDirtyRegion
) {
	hook.revert();
	auto retval = Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	hook.apply();
	return retval;
}

HRESULT ImMadPresent(
	const RECT* pSourceRect,
	const RECT* pDestRect,
	HWND          hDestWindowOverride,
	const RGNDATA* pDirtyRegion
) {
	return D3D_OK;
	IDirect3DDevice9* thisPtr;
	_asm {
		mov[thisPtr], ecx;
	}
	hook.revert();
	auto retval = thisPtr->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	hook.apply();
	return retval;
}

//HRESULT (_stdcall *origEndScene)(IDirect3DDevice9* thisPtr);
struct CUSTOMVERTEX
{
	FLOAT x, y, z, rhw;    // from the D3DFVF_XYZRHW flag
	DWORD color;    // from the D3DFVF_DIFFUSE flag
};
#define CUSTOMFVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)


//class 

inline void DrawTextString(IDirect3DDevice9* g_pd3dDevice, int x, int y, DWORD color, const char*
	str)
{

	//HRESULT r = 0;


	// Get a handle for the font to use

	HFONT hFont = (HFONT)GetStockObject(SYSTEM_FONT);

	LPD3DXFONT pFont = 0;

	// Create the D3DX Font
	// r = D3DXCreateFont(g_pd3dDevice, hFont, &pFont);

	auto r = D3DXCreateFont(g_pd3dDevice,     //D3D Device

		22,               //Font height

		0,                //Font width

		FW_NORMAL,        //Font Weight

		1,                //MipLevels

		false,            //Italic

		DEFAULT_CHARSET,  //CharSet

		OUT_DEFAULT_PRECIS, //OutputPrecision

		ANTIALIASED_QUALITY, //Quality

		DEFAULT_PITCH | FF_DONTCARE,//PitchAndFamily

		"Arial",          //pFacename,

		& pFont);         //ppFont

	//if (FAILED(r))
		//Do Debugging

		// Rectangle where the text will be located
	RECT TextRect = { x,y,0,0 };

	// Inform font it is about to be used
	//pFont->Begin();

	// Calculate the rectangle the text will occupy
	//pFont->DrawText(str, -1, &TextRect, DT_CALCRECT, 0);
	pFont->DrawText(NULL,        //pSprite

		str,  //pString

		-1,          //Count

		&TextRect,  //pRect

		DT_CALCRECT,//Format,

		0); //Color

	pFont->DrawText(NULL,        //pSprite

		str,  //pString

		-1,          //Count

		&TextRect,  //pRect

		DT_LEFT,//Format,

		color); //Color

	// Output the text, left aligned
	// pFont->DrawText(str, -1, &TextRect, DT_LEFT, color);

	// Finish up drawing
	//pFont->End();


	// Release the font
	pFont->Release();

}

LARGE_INTEGER last, freq;

HRESULT _stdcall MadEndScene(IDirect3DDevice9* thisPtr) {
	/*IDirect3DDevice9* thisPtr;
	_asm {
		mov[thisPtr], ecx;
	}*/
	//Sleep(500);
	/*CUSTOMVERTEX OurVertices[] =
	{
		{320.0f, 50.0f, 1.0f, 1.0f, D3DCOLOR_XRGB(0, 0, 255),},
		{520.0f, 400.0f, 1.0f, 1.0f, D3DCOLOR_XRGB(0, 255, 0),},
		{120.0f, 400.0f, 1.0f, 1.0f, D3DCOLOR_XRGB(255, 0, 0),},
	};

	LPDIRECT3DVERTEXBUFFER9 v_buffer;

	thisPtr->CreateVertexBuffer(3 * sizeof(CUSTOMVERTEX),
		0,
		CUSTOMFVF,
		D3DPOOL_MANAGED,
		&v_buffer,
		NULL);
	VOID* pVoid;    // the void* we were talking about

	v_buffer->Lock(0, 0, (void**)&pVoid, 0);    // locks v_buffer, the buffer we made earlier
	memcpy(pVoid, OurVertices, sizeof(OurVertices));    // copy vertices to the vertex buffer
	v_buffer->Unlock();    // unlock v_buffer
	thisPtr->SetFVF(CUSTOMFVF);
	thisPtr->SetStreamSource(0, v_buffer, 0, sizeof(CUSTOMVERTEX));
	thisPtr->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);*/

	LARGE_INTEGER curr;
	QueryPerformanceCounter(&curr);
	auto durationInMilliseconds = (curr.QuadPart - last.QuadPart) * pow(10, 3) / freq.QuadPart;
	last = curr;
	//OutputDebugString(fmt::format("{}", durationInMilliseconds).c_str());
	auto fps = pow(10, 3) / durationInMilliseconds;
	DrawTextString(thisPtr, 0, 0, D3DCOLOR_ARGB(255, 255, 0, 0), fmt::format("{:.2f}FPS", fps).c_str());

	D3DRECT rect = { 100,100,100 + 100,100 + 100 };
	//thisPtr->Clear(1, &rect, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255,255,255,255), 0, 0);
	hook.revert();
	auto retval = thisPtr->EndScene();
	hook.apply();
	//v_buffer->Release();
	// Sleep(500);
	//auto retval = origEndScene(thisPtr);
	return retval;
}

DWORD WINAPI doD9stuff(_In_ LPVOID lpParameter) {
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&last);
	auto g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (nullptr == g_pD3D)
		throw std::exception("Couldn't create d9");

	IDirect3DDevice9* pDevice = NULL;
	D3DPRESENT_PARAMETERS d3dpp = {};
	d3dpp.Windowed = false;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = getProcessWindow();
	/*WNDPROC TempWndProc = {};
	WNDCLASSEX wc = { sizeof(WNDCLASSEX),CS_CLASSDC,TempWndProc,0L,0L,GetModuleHandle(NULL),NULL,NULL,NULL,NULL,("1"),NULL };
	RegisterClassEx(&wc);
	HWND hWnd = CreateWindow(("1"), NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, GetDesktopWindow(), NULL, wc.hInstance, NULL);*/

	if (D3D_OK != g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &pDevice)) {
		d3dpp.Windowed = true;
		if (D3D_OK != g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&d3dpp, &pDevice))
			throw std::exception("Couldn't create device");
	}
		
	//DestroyWindow(hWnd);

	auto vtable = *reinterpret_cast<void***>(pDevice);
	//auto present = *reinterpret_cast<decltype(&IDirect3DDevice9::Present)*>(vtable + 17);
	auto present = vtable[168/4];
	// OutputDebugString(present);
	hook = Hooking::TrampolineHook(present, MadEndScene);
	hook.apply();
	pDevice->Release();
	g_pD3D->Release();
	return 0;
}

auto thread = wil::unique_handle(CreateThread(NULL, 0, doD9stuff, NULL, 0, NULL));

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		// doD9stuff(NULL);
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

