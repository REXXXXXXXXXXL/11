#include "Object.h"

HRESULT Object::Init(const char* imgname, int sx, int sy)
{
	//UV嵗昗傪愝掕
	m_splitX = sx;
	m_splitY = sy;
	m_vertexList[1].u = 1.0f / m_splitX;
	m_vertexList[2].v = 1.0f / m_splitY;
	m_vertexList[3].u = 1.0f / m_splitX;
	m_vertexList[3].v = 1.0f / m_splitY;

	// 捀揰僶僢僼傽傪嶌惉偡傞
	// 仸捀揰僶僢僼傽仺VRAM偵捀揰僨乕僞傪抲偔偨傔偺婡擻
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof(m_vertexList);// 妋曐偡傞僶僢僼傽僒僀僘傪巜掕
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;// 捀揰僶僢僼傽嶌惉傪巜掕
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subResourceData;
	subResourceData.pSysMem = m_vertexList;// VRAM偵憲傞僨乕僞傪巜掕
	subResourceData.SysMemPitch = 0;
	subResourceData.SysMemSlicePitch = 0;

	HRESULT hr = g_pDevice->CreateBuffer(&bufferDesc, &subResourceData, &m_pVertexBuffer);
	if (FAILED(hr)) return hr;

	// 僥僋僗僠儍撉傒崬傒
	hr = LoadTexture(g_pDevice, imgname, &m_pTextureView);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "僥僋僗僠儍撉傒崬傒幐攕", "僄儔乕", MB_ICONERROR | MB_OK);
		return hr;
	}
	return S_OK;
}


void Object::Draw() {

	//捀揰僶僢僼傽傪愝掕
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;
	g_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &strides, &offsets);

	// 僥僋僗僠儍傪僺僋僙儖僔僃乕僟乕偵搉偡
	g_pDeviceContext->PSSetShaderResources(0, 1, &m_pTextureView);

	// 正射影行列：UIなどカメラ無効のものはズーム無視
	DirectX::XMMATRIX matrixProj;
	if (m_useCamera) {
		float viewW = SCREEN_WIDTH / g_CameraZoom;
		float viewH = SCREEN_HEIGHT / g_CameraZoom;
		matrixProj = DirectX::XMMatrixOrthographicLH(viewW, viewH, 0.0f, 3.0f);
	} else {
		matrixProj = DirectX::XMMatrixOrthographicLH(SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 3.0f);
	}

	// 儚乕儖僪曄姺峴楍偺嶌惉
	DirectX::XMFLOAT3 posWithCam = m_pos;
	if (m_useCamera) {
		posWithCam.x -= g_CameraOffset.x;
		posWithCam.y -= g_CameraOffset.y;
	}
	DirectX::XMMATRIX matrixScale = DirectX::XMMatrixScaling(m_size.x, m_size.y, m_size.z); //戝偒偝
	DirectX::XMMATRIX matrixAngle = DirectX::XMMatrixRotationZ(m_angle * 3.14f / 180); //岦偒
	DirectX::XMMATRIX matrixPositioin = DirectX::XMMatrixTranslation(posWithCam.x, posWithCam.y, posWithCam.z); //埵抲
	DirectX::XMMATRIX matrixWorld = matrixScale * matrixAngle * matrixPositioin; //儚乕儖僪曄姺峴楍傪嶌惉

	// UV傾僯儊乕僔儑儞偺峴楍嶌惉
	float u = (float)(int)numU / m_splitX;
	float v = (float)numV / m_splitY;
	DirectX::XMMATRIX matrixTex = DirectX::XMMatrixTranslation(u, v, 0.0f);

	//掕悢僶僢僼傽傪峏怴
	ConstBuffer cb;
	cb.matrixProj = DirectX::XMMatrixTranspose(matrixProj); //僾儘僕僃僋僔儑儞曄姺峴楍
	cb.matrixWorld = DirectX::XMMatrixTranspose(matrixWorld); //儚乕儖僪曄姺峴楍
	cb.matrixTex = DirectX::XMMatrixTranspose(matrixTex); // UV傾僯儊乕僔儑儞峴楍
	cb.color = m_color; //捀揰僇儔乕偺僨乕僞傪嶌惉

	// 峴楍傪僔僃乕僟乕偵搉偡
	g_pDeviceContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb, 0, 0);

	g_pDeviceContext->Draw(4, 0); // 昤夋柦椷

}
void Object::Uninit() {
	//奐曻張棠
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pTextureView);
}

void Object::SetPos(float x, float y, float z) {
	//嵗昗傪僙僢僩偡傞
	m_pos.x = x;
	m_pos.y = y;
	m_pos.z = z;
}


void Object::SetSize(float x, float y, float z) {
	//戝偒偝傪僙僢僩偡傞
	m_size.x = x;
	m_size.y = y;
	m_size.z = z;
}

void Object::SetAngle(float a) {
	//妏搙傪僙僢僩偡傞
	m_angle = a;
}

void Object::SetColor(float r, float g, float b, float a) {
	//怓傪僙僢僩偡傞
	m_color.x = r;
	m_color.y = g;
	m_color.z = b;
	m_color.w = a;
}

DirectX::XMFLOAT3 Object::GetPos(void)
{
	return m_pos; //嵗昗傪僎僢僩
}

DirectX::XMFLOAT3 Object::GetSize(void)
{
	return m_size; //戝偒偝傪僎僢僩
}

float Object::GetAngle(void)
{
	return m_angle; //妏搙傪僎僢僩
}

DirectX::XMFLOAT4 Object::GetColor(void)
{
	return m_color; //怓傪僎僢僩
}

