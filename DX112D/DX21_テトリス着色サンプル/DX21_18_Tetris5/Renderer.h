#pragma once
#include <d3d11.h>  // DirectX11傪巊偆偨傔偺僿僢僟乕僼傽僀儖
#include <DirectXMath.h> // DirextX偺悢妛娭楢偺僿僢僟乕僼傽僀儖

#define SCREEN_WIDTH (1280)	// 僂僀儞僪僂偺暆
#define SCREEN_HEIGHT (720)	// 僂僀儞僪僂偺崅偝

// 娭悢偺僾儘僩僞僀僾愰尵
HRESULT RendererInit(HWND hwnd);
void    RendererDrawStart();
void    RendererDrawEnd();
void    RendererUninit();

// カメラ（視点）オフセット制御
void    RendererSetCameraOffset(float x, float y);
extern DirectX::XMFLOAT3 g_CameraOffset;

// ズーム（拡大?縮小）制御（>1で拡大）
void    RendererSetZoom(float zoom);
extern float g_CameraZoom;

// 峔憿懱偺掕媊
// 捀揰僨乕僞傪昞偡峔憿懱
struct Vertex
{
	// 捀揰偺埵抲嵗昗
	float x, y, z;
	//怓
	float r, g, b, a;
	// 僥僋僗僠儍嵗昗乮UV嵗昗乯
	float u, v;
};

// 掕悢僶僢僼傽梡峔憿懱
struct ConstBuffer
{
	// 捀揰僇儔乕峴楍
	DirectX::XMFLOAT4 color;

	// UV嵗昗峴楍
	DirectX::XMMATRIX matrixTex;

	// 僾儘僕僃僋僔儑儞曄姺峴楍
	DirectX::XMMATRIX matrixProj;

	// 儚乕儖僪曄姺峴楍
	DirectX::XMMATRIX matrixWorld;
};

// 僨僶僀僗亖DirectX偺奺庬婡擻傪嶌傞 仸ID3D11偱巒傑傞億僀儞僞宆偺曄悢偼丄夝曻偡傞昁梫偑偁傞
extern ID3D11Device* g_pDevice;
// 僐儞僥僉僗僩亖昤夋娭楢傪巌傞婡擻
extern ID3D11DeviceContext* g_pDeviceContext;

extern ID3D11Buffer* g_pConstantBuffer;

// Direct3D夝曻偺娙棯壔儅僋儘
#define SAFE_RELEASE(p) { if( NULL != p ) { p->Release(); p = NULL; } }
