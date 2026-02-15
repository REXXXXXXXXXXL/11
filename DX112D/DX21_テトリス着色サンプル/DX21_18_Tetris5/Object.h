#pragma once
#include "Renderer.h"
#include "Texture.h" // テクスチャ読み込み
class Object {

private:
	// 頂点データ
	Vertex m_vertexList[4] =
	{
	   //x     y    z     r    g    b    a     u    v 
	   {-0.5f, 0.5f,0.5f ,1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f},
	   { 0.5f, 0.5f,0.5f ,1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f},
	   {-0.5f,-0.5f,0.5f ,1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f},
	   { 0.5f,-0.5f,0.5f ,1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f},
	};


	// 座標
	DirectX::XMFLOAT3 m_pos = { 0.0f, 0.0f, 0.0f };
	// 大きさ
	DirectX::XMFLOAT3 m_size = { 100.0f, 100.0f, 0.0f };
	// 角度
	float m_angle = 0.0f;
	// 色
	DirectX::XMFLOAT4 m_color = { 1.0f, 1.0f, 1.0f, 1.0f };

	// 頂点バッファ
	ID3D11Buffer* m_pVertexBuffer;
	// テクスチャ用変数
	ID3D11ShaderResourceView* m_pTextureView;

	//テクスチャが縦横に何分割されているか
	int m_splitX = 1;
	int m_splitY = 1;

	// ･ｫ･皈鱇mﾓﾃ･ﾕ･鬣ｰ｣ｨtrue､ﾇ･ｫ･皈鬣ｪ･ﾕ･ｻ･ﾃ･ﾈ､mﾓﾃ｣ｩ
	bool m_useCamera = true;

public:
	//左上から何段目を切り抜いて表示するか
	float numU = 0;
	float numV = 0;

	HRESULT Init(const char* imgname, int sx = 1, int sy = 1); //初期化
	void Draw();                       //描画
	void Uninit();                     //終了

	void SetPos(float x, float y, float z); //座標をセット
	void SetSize(float x, float y, float z);//大きさをセット
	void SetAngle(float a);//角度をセット
	void SetColor(float r, float g, float b, float a); //色をセット

	DirectX::XMFLOAT3 GetPos(void);   //座標をゲット
	DirectX::XMFLOAT3 GetSize(void);  //大きさをゲット
	float GetAngle(void);             //角度をゲット
	DirectX::XMFLOAT4 GetColor(void); //色をゲット

	// ･ｫ･皈鱇mﾓﾃ､ﾎON/OFF
	void SetUseCamera(bool use) { m_useCamera = use; }
	bool GetUseCamera() const { return m_useCamera; }

};





