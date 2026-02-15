#include "input.h"
#include <algorithm>

// 僅C++14??? std::clamp ???悤????????????????A?J?[?p?????g???萔
static float clampFloat(float v, float minVal, float maxVal)
{
    return (v < minVal) ? minVal : (v > maxVal) ? maxVal : v;
}

//僐儞僗僩儔僋僞
Input::Input()
{
	VibrationTime = 0;
}

//僨僗僩儔僋僞
Input::~Input()
{
	//怳摦傪廔椆偝偣傞
	XINPUT_VIBRATION vibration;
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
	vibration.wLeftMotorSpeed = 0;
	vibration.wRightMotorSpeed = 0;
	XInputSetState(0, &vibration);
}

void Input::Update()
{
	//1僼儗乕儉慜偺擖椡傪婰榐偟偰偍偔
	for (int i = 0; i < 256; i++) { keyState_old[i] = keyState[i]; }
	controllerState_old = controllerState;

	//僉乕擖椡傪峏怴
	BOOL hr = GetKeyboardState(keyState);

	//僐儞僩儘乕儔乕擖椡傪峏怴(XInput)
	XInputGetState(0, &controllerState);

	//怳摦宲懕帪娫傪僇僂儞僩
	if (VibrationTime > 0) {
		VibrationTime--;
		if (VibrationTime == 0) { //怳摦宲懕帪娫偑宱偭偨帪偵怳摦傪巭傔傞
			XINPUT_VIBRATION vibration;
			ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
			vibration.wLeftMotorSpeed = 0;
			vibration.wRightMotorSpeed = 0;
			XInputSetState(0, &vibration);
		}
	}
}

//僉乕擖椡
bool Input::GetKeyPress(int key) //僾儗僗
{
	return keyState[key] & 0x80;
}
bool Input::GetKeyTrigger(int key) //僩儕僈乕
{
	return (keyState[key] & 0x80) && !(keyState_old[key] & 0x80);
}
bool Input::GetKeyRelease(int key) //儕儕乕僗
{
	return !(keyState[key] & 0x80) && (keyState_old[key] & 0x80);
}

// 内部工具：应用死区与归一化
static float ApplyDeadZone(float v, float dz)
{
	float av = std::abs(v);
	if (av <= dz) return 0.0f;
	// 线性重映射到 [0,1]: (av - dz) / (1 - dz) 并保留符号
	float nv = (av - dz) / (1.0f - dz);
	return v < 0 ? -nv : nv;
}

//嵍傾僫儘僌僗僥傿僢僋
DirectX::XMFLOAT2 Input::GetLeftAnalogStick(void)
{
	SHORT x = controllerState.Gamepad.sThumbLX; // -32768乣32767
	SHORT y = controllerState.Gamepad.sThumbLY; // -32768乣32767

	DirectX::XMFLOAT2 res;
	res.x = x / 32767.0f; //-1乣1
	res.y = y / 32767.0f; //-1乣1
	// 应用死区
	res.x = ApplyDeadZone(res.x, DeadZoneLeft);
	res.y = ApplyDeadZone(res.y, DeadZoneLeft);
	return res;
}
//塃傾僫儘僌僗僥傿僢僋
DirectX::XMFLOAT2 Input::GetRightAnalogStick(void)
{
	SHORT x = controllerState.Gamepad.sThumbRX; // -32768乣32767
	SHORT y = controllerState.Gamepad.sThumbRY; // -32768乣32767

	DirectX::XMFLOAT2 res;
	res.x = x / 32767.0f; //-1乣1
	res.y = y / 32767.0f; //-1乣1
	// 应用死区
	res.x = ApplyDeadZone(res.x, DeadZoneRight);
	res.y = ApplyDeadZone(res.y, DeadZoneRight);
	return res;
}

//嵍僩儕僈乕
float Input::GetLeftTrigger(void)
{
	BYTE t = controllerState.Gamepad.bLeftTrigger; // 0乣255
	return t / 255.0f;
}
//塃僩儕僈乕
float Input::GetRightTrigger(void)
{
	BYTE t = controllerState.Gamepad.bRightTrigger; // 0乣255
	return t / 255.0f;
}

//儃僞儞擖椡
bool Input::GetButtonPress(WORD btn) //僾儗僗
{
	return (controllerState.Gamepad.wButtons & btn) != 0;
}
bool Input::GetButtonTrigger(WORD btn) //僩儕僈乕
{
	return (controllerState.Gamepad.wButtons & btn) != 0 && (controllerState_old.Gamepad.wButtons & btn) == 0;
}
bool Input::GetButtonRelease(WORD btn) //儕儕乕僗
{
	return (controllerState.Gamepad.wButtons & btn) == 0 && (controllerState_old.Gamepad.wButtons & btn) != 0;
}

// 2D横版的语义化输入：水平/垂直、跳跃、冲刺等
float Input::GetAxisHorizontal()
{
	// 键盘优先：A/D 或 左/右
	float x = 0.0f;
	if (GetKeyPress(VK_LEFT) || GetKeyPress('A')) x -= 1.0f;
	if (GetKeyPress(VK_RIGHT) || GetKeyPress('D')) x += 1.0f;

	// 手柄左摇杆作为补充
	if (x == 0.0f) {
		x = GetLeftAnalogStick().x; // 已应用死区
	}
	return clampFloat(x, -1.0f, 1.0f);
}

float Input::GetAxisVertical()
{
	// 键盘优先：W/S 或 上/下（部分横版可能只用跳跃，不用纵向）
	float y = 0.0f;
	if (GetKeyPress(VK_UP) || GetKeyPress('W')) y += 1.0f;
	if (GetKeyPress(VK_DOWN) || GetKeyPress('S')) y -= 1.0f;

	if (y == 0.0f) {
		y = GetLeftAnalogStick().y; // 已应用死区
	}
	return clampFloat(y, -1.0f, 1.0f);
}

bool Input::IsJumpTrigger()
{
	// 空格 或 手柄 A
	return GetKeyTrigger(VK_SPACE) || GetButtonTrigger(XINPUT_GAMEPAD_A);
}

bool Input::IsDashPress()
{
	// 左Shift 或 手柄 B
	return GetKeyPress(VK_SHIFT) || GetButtonPress(XINPUT_GAMEPAD_B);
}

//怳摦
void Input::SetVibration(int frame, float powor)
{
	// XINPUT_VIBRATION峔憿懱偺僀儞僗僞儞僗傪嶌惉
	XINPUT_VIBRATION vibration;
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

	// 儌乕僞乕偺嫮搙傪愝掕乮0乣65535乯
	vibration.wLeftMotorSpeed = (WORD)(powor * 65535.0f);
	vibration.wRightMotorSpeed = (WORD)(powor * 65535.0f);
	XInputSetState(0, &vibration);

	//怳摦宲懕帪娫傪戙擖
	VibrationTime = frame;
}

