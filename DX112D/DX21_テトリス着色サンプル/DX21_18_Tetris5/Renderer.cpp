#define _CRT_SECURE_NO_WARNINGS
#include <d3dcompiler.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <locale.h>
#include <atltypes.h> // CRect‚ğg‚¤‚½‚ß‚Ìƒwƒbƒ_[ƒtƒ@ƒCƒ‹
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "Renderer.h"

//ƒvƒƒgƒ^ƒCƒvéŒ¾
// ’¸“_ƒVƒF[ƒ_[ƒIƒuƒWƒFƒNƒg‚ğ¶¬
HRESULT CreateVertexShader(ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppVertexLayout, D3D11_INPUT_ELEMENT_DESC* pLayout, unsigned int numElements, const char* szFileName);
// ƒsƒNƒZƒ‹ƒVƒF[ƒ_[ƒIƒuƒWƒFƒNƒg‚ğ¶¬
HRESULT CreatePixelShader(ID3D11PixelShader** ppPixelShader, const char* szFileName);

//--------------------------------------------------------------------------------------
// ƒOƒ[ƒoƒ‹•Ï”
//--------------------------------------------------------------------------------------

//Direct3D‚Ìƒo[ƒWƒ‡ƒ“
D3D_FEATURE_LEVEL g_FeatureLevel = D3D_FEATURE_LEVEL_11_0;
// ƒfƒoƒCƒXDirectX‚ÌŠeí‹@”\‚ğì‚é ¦ID3D11‚Ån‚Ü‚éƒ|ƒCƒ“ƒ^Œ^‚Ì•Ï”‚ÍA‰ğ•ú‚·‚é•K—v‚ª‚ ‚é
ID3D11Device* g_pDevice;
// ƒRƒ“ƒeƒLƒXƒg•`‰æŠÖ˜A‚ği‚é‹@”\
ID3D11DeviceContext* g_pDeviceContext;
// ƒXƒƒbƒvƒ`ƒFƒCƒ“ƒ_ƒuƒ‹ƒoƒbƒtƒ@‹@”\
IDXGISwapChain* g_pSwapChain;
// ƒŒƒ“ƒ_[ƒ^[ƒQƒbƒg•`‰ææ‚ğ•\‚·‹@”\
ID3D11RenderTargetView* g_pRenderTargetView;
// ƒfƒvƒXƒoƒbƒtƒ@
ID3D11DepthStencilView* g_pDepthStencilView;
// ƒCƒ“ƒvƒbƒgƒŒƒCƒAƒEƒg
ID3D11InputLayout* g_pInputLayout;
// ’¸“_ƒVƒF[ƒ_[ƒIƒuƒWƒFƒNƒg
ID3D11VertexShader* g_pVertexShader;
// ƒsƒNƒZƒ‹ƒVƒF[ƒ_[ƒIƒuƒWƒFƒNƒg
ID3D11PixelShader* g_pPixelShader;
// ƒTƒ“ƒvƒ‰[—p•Ï”
ID3D11SamplerState* g_pSampler;
// ’è”ƒoƒbƒtƒ@—p•Ï”
ID3D11Buffer* g_pConstantBuffer;
// ƒuƒŒƒ“ƒhƒXƒe[ƒg—p•Ï”iƒAƒ‹ƒtƒ@ƒuƒŒƒ“ƒfƒBƒ“ƒOj
ID3D11BlendState* g_pBlendState;

// ¥«¥á¥é£¨Ò•µã£©¥ª¥Õ¥»¥Ã¥È
DirectX::XMFLOAT3 g_CameraOffset = {0.0f, 0.0f, 0.0f};
void RendererSetCameraOffset(float x, float y)
{
    g_CameraOffset.x = x;
    g_CameraOffset.y = y;
    g_CameraOffset.z = 0.0f;
}

// ¥º©`¥à£¨’ˆ´ó?¿sĞ¡£©
float g_CameraZoom = 1.0f;
void RendererSetZoom(float zoom)
{
    if (zoom < 0.1f) zoom = 0.1f; // avoid zero/negative
    g_CameraZoom = zoom;
}

//--------------------------------------------------------------------------------------
// DirectX‚Ì‰Šú‰»‚ğs‚¤
//--------------------------------------------------------------------------------------
HRESULT RendererInit(HWND hwnd)
{
	HRESULT hr = S_OK; // HRESULTŒ^¨WindowsƒvƒƒOƒ‰ƒ€‚ÅŠÖ”Às‚Ì¬Œ÷/¸”s‚ğó‚¯æ‚é

	// ƒfƒoƒCƒXAƒXƒƒbƒvƒ`ƒF[ƒ“ì¬
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 1;                       // ƒoƒbƒNƒoƒbƒtƒ@‚Ì”iƒ_ƒuƒ‹ƒoƒbƒtƒ@j
	swapChainDesc.BufferDesc.Width = SCREEN_WIDTH;       // ƒoƒbƒtƒ@‚Ì•‚ğƒEƒBƒ“ƒhƒEƒTƒCƒY‚É‡‚í‚¹‚é
	swapChainDesc.BufferDesc.Height = SCREEN_HEIGHT;     // ƒoƒbƒtƒ@‚Ì‚‚³‚ğƒEƒBƒ“ƒhƒEƒTƒCƒY‚É‡‚í‚¹‚é
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // ƒoƒbƒtƒ@‚ÌƒsƒNƒZƒ‹ƒtƒH[ƒ}ƒbƒg
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60; // ƒŠƒtƒŒƒbƒVƒ…ƒŒ[ƒg‚ğİ’è(Hz)
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // ƒoƒbƒtƒ@‚Ìg—p—p“r‚ğİ’è
	swapChainDesc.OutputWindow = hwnd;          // ƒXƒƒbƒvƒ`ƒF[ƒ“‚Ìƒ^[ƒQƒbƒgƒEƒBƒ“ƒhƒE‚ğİ’è
	swapChainDesc.SampleDesc.Count = 1;         // ƒ}ƒ‹ƒ`ƒTƒ“ƒvƒŠƒ“ƒO‚Ìİ’èiƒAƒ“ƒ`ƒGƒCƒŠƒAƒX‚ÌƒTƒ“ƒvƒ‹”‚ÆƒNƒIƒŠƒeƒBj
	swapChainDesc.SampleDesc.Quality = 0;       //“¯ã
	swapChainDesc.Windowed = TRUE;              // ƒEƒBƒ“ƒhƒEƒ‚[ƒhiƒtƒ‹ƒXƒNƒŠ[ƒ“‚Å‚Í‚È‚­AƒEƒBƒ“ƒhƒEƒ‚[ƒh‚ÅÀsj

	// ƒfƒoƒCƒX‚ÆƒXƒƒbƒvƒ`ƒFƒCƒ“‚ğ“¯‚Éì¬‚·‚éŠÖ”‚ÌŒÄ‚Ño‚µ
	hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE, // ƒhƒ‰ƒCƒo[ƒ^ƒCƒv(ƒn[ƒhƒEƒFƒAGPU‚ğg—p)
		NULL,               // ƒ\ƒtƒgƒEƒFƒAƒ‰ƒXƒ^ƒ‰ƒCƒU‚ğw’è‚µ‚È‚¢‚Ì‚ÅNULL
		0,                  // ƒtƒ‰ƒOiD3D11_CREATE_DEVICE_DEBUG‚È‚Çj 0‚Í‰½‚àw’è‚µ‚È‚¢
		NULL,               // ‹@”\ƒŒƒxƒ‹‚Ì”z—ñBNULL‚È‚çƒfƒtƒHƒ‹ƒg‚Ì‹@”\ƒŒƒxƒ‹ƒZƒbƒg‚ªg‚í‚ê‚é
		0,                  // ‹@”\ƒŒƒxƒ‹‚Ì”z—ñ‚Ì—v‘f”(NULL‚È‚ç0‚ÅOK)
		D3D11_SDK_VERSION,  // SDK‚Ìƒo[ƒWƒ‡ƒ“ í‚ÉuD3D11_SDK_VERSIONv‚ğw’è
		&swapChainDesc,     // ƒXƒƒbƒvƒ`ƒF[ƒ“‚Ìİ’è\‘¢‘Ì‚Ö‚Ìƒ|ƒCƒ“ƒ^
		&g_pSwapChain,      // ì¬‚³‚ê‚½ƒXƒƒbƒvƒ`ƒF[ƒ“‚ğó‚¯æ‚éƒ|ƒCƒ“ƒ^
		&g_pDevice,	        // ì¬‚³‚ê‚½ƒfƒoƒCƒX‚ğó‚¯æ‚éƒ|ƒCƒ“ƒ^
		&g_FeatureLevel,    // ì¬‚³‚ê‚½ƒfƒoƒCƒX‚Ì‹@”\ƒŒƒxƒ‹‚ğó‚¯æ‚é•Ï”‚Ö‚Ìƒ|ƒCƒ“ƒ^
		&g_pDeviceContext); // ´ß¬‚³‚ê‚½ƒfƒoƒCƒXƒRƒ“ƒeƒLƒXƒg‚ğó‚¯æ‚éƒ|ƒCƒ“ƒ^
	if (FAILED(hr)) return hr;

	// ƒŒƒ“ƒ_[ƒ^[ƒQƒbƒgƒrƒ…[ì¬
	ID3D11Texture2D* renderTarget;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&renderTarget);
	if (FAILED(hr)) return hr;
	hr = g_pDevice->CreateRenderTargetView(renderTarget, NULL, &g_pRenderTargetView);
	renderTarget->Release();
	if (FAILED(hr)) return hr;

	// ƒfƒvƒXƒXƒeƒ“ƒVƒ‹ƒoƒbƒtƒ@ì¬
	// ¦iƒfƒvƒXƒoƒbƒtƒ@ = [“xƒoƒbƒtƒ@ = Zƒoƒbƒtƒ@j¨‰œs‚ğ”»’è‚µ‚Ä‘OŒãŠÖŒW‚ğ³‚µ‚­•`‰æ‚Å‚«‚é
	ID3D11Texture2D* depthStencile{};
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = swapChainDesc.BufferDesc.Width;   // ƒoƒbƒtƒ@‚Ì•‚ğƒXƒƒbƒvƒ`ƒF[ƒ“‚É‡‚í‚¹‚é
	textureDesc.Height = swapChainDesc.BufferDesc.Height; // ƒoƒbƒtƒ@‚Ì‚‚³‚ğƒXƒƒbƒvƒ`ƒF[ƒ“‚É‡‚í‚¹‚é
	textureDesc.MipLevels = 1;                            // ƒ~ƒbƒvƒŒƒxƒ‹‚Í1iƒ~ƒbƒvƒ}ƒbƒv‚Íg—p‚µ‚È‚¢j
	textureDesc.ArraySize = 1;                            // ƒeƒNƒXƒ`ƒƒ‚Ì”z—ñƒTƒCƒYi’Êí1j
	textureDesc.Format = DXGI_FORMAT_D16_UNORM;           // ƒtƒH[ƒ}ƒbƒg‚Í16ƒrƒbƒg‚Ì[“xƒoƒbƒtƒ@‚ğg—p
	textureDesc.SampleDesc = swapChainDesc.SampleDesc;    // ƒXƒƒbƒvƒ`ƒF[ƒ“‚Æ“¯‚¶ƒTƒ“ƒvƒ‹İ’è
	textureDesc.Usage = D3D11_USAGE_DEFAULT;              // g—p•û–@‚ÍƒfƒtƒHƒ‹ƒgiGPU‚Åg—pj
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;     // [“xƒXƒeƒ“ƒVƒ‹ƒoƒbƒtƒ@‚Æ‚µ‚Äg—p
	textureDesc.CPUAccessFlags = 0;                       // CPU‚©‚ç‚ÌƒAƒNƒZƒX‚Í•s—v
	textureDesc.MiscFlags = 0;                            // ‚»‚Ì‘¼‚Ìƒtƒ‰ƒO‚Íİ’è‚È‚µ
	hr = g_pDevice->CreateTexture2D(&textureDesc, NULL, &depthStencile);
	if (FAILED(hr)) return hr;

	// ƒfƒvƒXƒXƒeƒ“ƒVƒ‹ƒrƒ…[ì¬
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = textureDesc.Format; // ƒfƒvƒXƒXƒeƒ“ƒVƒ‹ƒoƒbƒtƒ@‚ÌƒtƒH[ƒ}ƒbƒg‚ğİ’è
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D; // ƒrƒ…[‚ÌŸŒ³‚ğ2DƒeƒNƒXƒ`ƒƒ‚Æ‚µ‚Äİ’èi2DƒeƒNƒXƒ`ƒƒ—p‚ÌƒfƒvƒXƒXƒeƒ“ƒVƒ‹ƒrƒ…[j
	depthStencilViewDesc.Flags = 0; // “Á•Ê‚Èƒtƒ‰ƒO‚Íİ’è‚µ‚È‚¢iƒfƒtƒHƒ‹ƒg‚Ì“®ìj
	hr = g_pDevice->CreateDepthStencilView(depthStencile, &depthStencilViewDesc, &g_pDepthStencilView);
	if (FAILED(hr)) return hr;
	depthStencile->Release();

	// ƒrƒ…[ƒ|[ƒg‚ğì¬i¨‰æ–Ê•ªŠ„‚È‚Ç‚Ég‚¤A•`‰æ—Ìˆæ‚Ìw’è‚Ì‚±‚Æj
	CRect rect;
	GetClientRect(hwnd, &rect);
	D3D11_VIEWPORT viewport;
	viewport.Width = (FLOAT)rect.Width();   // ƒrƒ…[ƒ|[ƒg‚Ì•
	viewport.Height = (FLOAT)rect.Height(); // ƒrƒ…[ƒ|[ƒg‚Ì‚‚³
	viewport.MinDepth = 0.0f;               // [“x”ÍˆÍ‚ÌÅ¬’l
	viewport.MaxDepth = 1.0f;               // [“x”ÍˆÍ‚ÌÅ‘å’l
	viewport.TopLeftX = 0;                  // ƒrƒ…[ƒ|[ƒg‚Ì¶ã‹÷‚ÌXÀ•W
	viewport.TopLeftY = 0;                  // ƒrƒ…[ƒ|[ƒg‚Ì¶ã‹÷‚ÌYÀ•Wj
	g_pDeviceContext->RSSetViewports(1, &viewport);


	// ´ßƒ“ƒvƒbƒgƒŒƒCƒAƒEƒgì¬
	D3D11_INPUT_ELEMENT_DESC layout[]
	{
		// ˆÊ’uÀ•W‚ª‚ ‚é‚Æ‚¢‚¤‚±‚Æ‚ğ“`‚¦‚é
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// Fî•ñ‚ª‚ ‚é‚Æ‚¢‚¤‚±‚Æ‚ğ“`‚¦‚é
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// UVÀ•Wiuvj
		{ "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	unsigned int numElements = ARRAYSIZE(layout);

	// ’¸“_ƒVƒF[ƒ_[ƒIƒuƒWƒFƒNƒg‚ğ¶¬A“¯‚É’¸“_ƒŒƒCƒAƒEƒg‚à¶¬
	hr = CreateVertexShader(&g_pVertexShader, &g_pInputLayout, layout, numElements, "VertexShader.hlsl");
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreateVertexShader error", "error", MB_OK);
		return hr;
	}

	// ƒsƒNƒZƒ‹ƒVƒF[ƒ_[ƒIƒuƒWƒFƒNƒg‚ğ¶¬
	hr = CreatePixelShader(&g_pPixelShader, "PixelShader.hlsl");
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreatePixelShader error", "error", MB_OK);
		return hr;
	}

	// ƒTƒ“ƒvƒ‰[ì¬
	// ¨@ƒeƒNƒXƒ`ƒƒ‚ğƒ|ƒŠƒSƒ“‚É“\‚é‚Æ‚«‚ÉAŠg‘åk¬‚³‚ê‚éÛ‚ÌƒAƒ‹ƒSƒŠƒYƒ€
	D3D11_SAMPLER_DESC smpDesc{};
	smpDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	smpDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	smpDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	smpDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	hr = g_pDevice->CreateSamplerState(&smpDesc, &g_pSampler);
	if (FAILED(hr)) return hr;

	// ’è”ƒoƒbƒtƒ@ì¬
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(ConstBuffer);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;
	hr = g_pDevice->CreateBuffer(&cbDesc, NULL, &g_pConstantBuffer);
	if (FAILED(hr)) return hr;

	// ƒuƒŒƒ“ƒhƒXƒe[ƒgì¬ ¨“§‰ßˆ—‚â‰ÁZ‡¬‚ğ‰Â”\‚É‚·‚éF‚Ì‡¬•û–@
	D3D11_BLEND_DESC BlendState{};
	BlendState.AlphaToCoverageEnable = FALSE;
	BlendState.IndependentBlendEnable = FALSE;
	BlendState.RenderTarget[0].BlendEnable = TRUE;
	BlendState.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendState.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BlendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = g_pDevice->CreateBlendState(&BlendState, &g_pBlendState);
	if (FAILED(hr)) return hr;

	//[“xƒeƒXƒg‚ğ–³Œø‚É‚·‚é
	ID3D11DepthStencilState* pDSState;
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));
	dsDesc.DepthEnable = FALSE; // [“xƒeƒXƒg‚ğ–³Œø‚É‚·‚é
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	hr = g_pDevice->CreateDepthStencilState(&dsDesc, &pDSState);
	if (FAILED(hr)) return hr;
	g_pDeviceContext->OMSetDepthStencilState(pDSState, 1);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// DirectX‚Ì•`‰æ‚ğs‚¤
//--------------------------------------------------------------------------------------
void RendererDrawStart()
{
	// ‰æ–Ê“h‚è‚Â‚Ô‚µF
	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f }; //red,green,blue,alpha

	// •`‰ææ‚ÌƒLƒƒƒ“ƒoƒX‚Æg—p‚·‚é[“xƒoƒbƒtƒ@‚ğw’è‚·‚é
	g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
	// •`‰ææƒLƒƒƒ“ƒoƒX‚ğ“h‚è‚Â‚Ô‚·
	g_pDeviceContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);
	// [“xƒoƒbƒtƒ@‚ğƒŠƒZƒbƒg‚·‚é
	g_pDeviceContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	g_pDeviceContext->IASetInputLayout(g_pInputLayout);
	g_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	g_pDeviceContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pDeviceContext->PSSetShader(g_pPixelShader, NULL, 0);

	// ƒTƒ“ƒvƒ‰[‚ğƒsƒNƒZƒ‹ƒVƒF[ƒ_[‚É“n‚·
	g_pDeviceContext->PSSetSamplers(0, 1, &g_pSampler);

	// ’è”ƒoƒbƒtƒ@‚ğ’¸“_ƒVƒF[ƒ_[‚ÉƒZƒbƒg‚·‚é
	g_pDeviceContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	//ƒuƒŒƒ“ƒhƒXƒe[ƒg‚ğƒZƒbƒg‚·‚é
	g_pDeviceContext->OMSetBlendState( g_pBlendState, NULL, 0xffffffff);

}
void RendererDrawEnd()
{
	// ƒ_ƒuƒ‹ƒoƒbƒtƒ@‚ÌØ‚è‘Ö‚¦‚ğs‚¢‰æ–Ê‚ğXV‚·‚é
	g_pSwapChain->Present(0, 0);
}

//--------------------------------------------------------------------------------------
// DirectX‚Ì‰ğ•úiI—¹ˆ—j‚ğs‚¤
//--------------------------------------------------------------------------------------
void RendererUninit()
{
	// ¦DirectX‚ÌŠe‹@”\‚Íì¬‚µ‚½ŒãAƒAƒvƒŠI—¹‚É•K‚¸‰ğ•ú‚µ‚È‚¯‚ê‚Î‚È‚ç‚È‚¢
	if (g_pDeviceContext) g_pDeviceContext->ClearState();

	SAFE_RELEASE(g_pBlendState);
	SAFE_RELEASE(g_pConstantBuffer);
	SAFE_RELEASE(g_pSampler);
	SAFE_RELEASE(g_pPixelShader);
	SAFE_RELEASE(g_pVertexShader);
	SAFE_RELEASE(g_pInputLayout);
	SAFE_RELEASE(g_pDepthStencilView);
	SAFE_RELEASE(g_pRenderTargetView);
	SAFE_RELEASE(g_pSwapChain);
	SAFE_RELEASE(g_pDeviceContext);
	SAFE_RELEASE(g_pDevice);
}

//--------------------------------------------------------------------------------------
// ƒVƒF[ƒ_[‚ğƒtƒ@ƒCƒ‹Šg’£q‚É‡‚í‚¹‚ÄƒRƒ“ƒpƒCƒ‹
//--------------------------------------------------------------------------------------
HRESULT CompileShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, void** ppShaderObject, int* pShaderObjectSize)
{
	//Šg’£qcso‚Ìƒtƒ@ƒCƒ‹–¼‚ğì¬
	char csoFileName[256];
	const char* dot = strrchr(szFileName, '.');  // ÅŒã‚Ì '.' ‚ğ’T‚·
	if (dot) {
		int basenameLen = dot - szFileName;
		strncpy(csoFileName, szFileName, basenameLen); // Šg’£q‚ª‚ ‚éê‡‚ÍŠg’£q‚ğœ‚¢‚½ƒtƒ@ƒCƒ‹–¼‚ğƒRƒs[
		csoFileName[basenameLen] = '\0';   // I’[•¶š‚ğ’Ç‰Á
	}
	else {
		strcpy(csoFileName, szFileName);   // Šg’£q‚ª‚È‚¢ê‡‚Í‚»‚Ì‚Ü‚ÜƒRƒs[
	}
	strcat(csoFileName, ".cso");// ".cso" Šg’£q‚ğ•t‰Á

	//csoƒtƒ@ƒCƒ‹‚ª‚ ‚ê‚ÎŠJ‚­
	FILE* fp;
	int ret = fopen_s(&fp, csoFileName, "rb");
	if (ret == 0)
	{
		// ƒtƒ@ƒCƒ‹ƒTƒCƒY‚ğæ“¾
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		// ƒoƒCƒiƒŠƒf[ƒ^‚ğ“Ç‚İ‚İ—p‚Éƒƒ‚ƒŠŠm•Û
		unsigned char* byteArray = new unsigned char[size];
		fread(byteArray, size, 1, fp);
		fclose(fp);

		// ŒÄ‚Ño‚µŒ³‚Éƒ|ƒCƒ“ƒ^‚ÆƒTƒCƒY‚ğ“n‚·
		*ppShaderObject = byteArray;
		*pShaderObjectSize = size;
	}
	//csoƒtƒ@ƒCƒ‹‚ª‚È‚¯‚ê‚Îhlslƒtƒ@ƒCƒ‹‚ğƒRƒ“ƒpƒCƒ‹‚·‚é
	else
	{
		HRESULT hr = S_OK;
		WCHAR	filename[512];
		size_t 	wLen = 0;
		int err = 0;

		// •¶šƒR[ƒh‚ğ Shift-JIS ¨ UTF-16 ‚É•ÏŠ·
		setlocale(LC_ALL, "japanese");  // ƒƒP[ƒ‹‚ğİ’èiWindows“Á—Lj
		err = mbstowcs_s(&wLen, filename, 512, szFileName, _TRUNCATE);

		// ƒVƒF[ƒ_[ƒRƒ“ƒpƒCƒ‹ƒIƒvƒVƒ‡ƒ“‚ğİ’è
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		dwShaderFlags |= D3DCOMPILE_DEBUG; // ƒfƒoƒbƒOƒrƒ‹ƒh‚Ìê‡‚ÍƒfƒoƒbƒOî•ñ‚àŠÜ‚ß‚é
#endif

		// ƒRƒ“ƒpƒCƒ‹Œ‹‰Ê‚¨‚æ‚ÑƒGƒ‰[î•ñŠi”[—p‚ÌBlob
		ID3DBlob* pErrorBlob = nullptr;
		ID3DBlob* pBlob = nullptr;

		// HLSLƒtƒ@ƒCƒ‹‚ğƒRƒ“ƒpƒCƒ‹
		hr = D3DCompileFromFile(
			filename,							// ƒtƒ@ƒCƒ‹–¼
			nullptr,							// ƒ}ƒNƒ’è‹`‚È‚µ 
			D3D_COMPILE_STANDARD_FILE_INCLUDE,	// #include ‘Î‰ 
			szEntryPoint,						// ƒGƒ“ƒgƒŠ[ƒ|ƒCƒ“ƒg–¼
			szShaderModel,						// ƒVƒF[ƒ_[ƒ‚ƒfƒ‹
			dwShaderFlags,						// ƒRƒ“ƒpƒCƒ‹ƒtƒ‰ƒO
			0,									// ƒGƒtƒFƒNƒgƒtƒ‰ƒO
			&pBlob,								// ¬Œ÷‚ÌƒRƒ“ƒpƒCƒ‹Œ‹‰Ê
			&pErrorBlob);						// ƒRƒ“ƒpƒCƒ‹ƒGƒ‰[o—Í

		// ƒRƒ“ƒpƒCƒ‹¸”s‚ÌƒGƒ‰[ƒƒbƒZ[ƒW‚ğ•\¦
		if (FAILED(hr))
		{
			if (pErrorBlob != nullptr) {
				MessageBoxA(NULL, (char*)pErrorBlob->GetBufferPointer(), "Error", MB_OK);
			}
			if (pErrorBlob) pErrorBlob->Release();
			if (pBlob)(pBlob)->Release();
			return E_FAIL;
		}

		// ƒGƒ‰[ƒuƒƒu‚ª‚ ‚ê‚Î‰ğ•ú
		if (pErrorBlob) pErrorBlob->Release();

		// ƒRƒ“ƒpƒCƒ‹¬Œ÷‚ÌƒoƒCƒiƒŠƒf[ƒ^‚ğŒÄ‚Ño‚µŒ³‚É“n‚·
		*ppShaderObject = (pBlob)->GetBufferPointer();
		*pShaderObjectSize = (pBlob)->GetBufferSize();
	}

	return S_OK;
}

//--------------------------------------------------------------------------------------
// ’¸“_ƒVƒF[ƒ_[ƒIƒuƒWƒFƒNƒg‚ğ¶¬‚·‚é
//--------------------------------------------------------------------------------------
HRESULT CreateVertexShader(ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppVertexLayout, D3D11_INPUT_ELEMENT_DESC* pLayout, unsigned int numElements, const char* szFileName)
{
	void* ShaderObject;
	int	ShaderObjectSize;

	// ƒtƒ@ƒCƒ‹‚ÌŠg’£q‚É‡‚í‚¹‚ÄƒRƒ“ƒpƒCƒ‹
	HRESULT hr = CompileShader(szFileName, "main", "vs_5_0", &ShaderObject, &ShaderObjectSize);
	if (FAILED(hr)) return E_FAIL;

	// ƒfƒoƒCƒX‚ğg‚Á‚Ä’¸“_ƒVƒF[ƒ_[‚ğì¬
	hr = g_pDevice->CreateVertexShader(ShaderObject, ShaderObjectSize, NULL, ppVertexShader);

	// ƒfƒoƒCƒX‚ğg‚Á‚Ä’¸“_ƒŒƒCƒAƒEƒg‚ğì¬
	g_pDevice->CreateInputLayout(pLayout, numElements, ShaderObject, ShaderObjectSize, ppVertexLayout);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// ƒsƒNƒZƒ‹ƒVƒF[ƒ_[ƒIƒuƒWƒFƒNƒg‚ğ¶¬‚·‚é
//--------------------------------------------------------------------------------------
HRESULT CreatePixelShader(ID3D11PixelShader** ppPixelShader, const char* szFileName)
{
	void* ShaderObject;
	int	ShaderObjectSize;

	// ƒtƒ@ƒCƒ‹‚ÌŠg’£q‚É‡‚í‚¹‚ÄƒRƒ“ƒpƒCƒ‹
	HRESULT hr = CompileShader(szFileName, "main", "ps_5_0", &ShaderObject, &ShaderObjectSize);
	if (FAILED(hr)) return hr;

	// ƒsƒNƒZƒ‹ƒVƒF[ƒ_[‚ğ¶¬
	hr = g_pDevice->CreatePixelShader(ShaderObject, ShaderObjectSize, nullptr, ppPixelShader);
	if (FAILED(hr)) return hr;

	return S_OK;
}

