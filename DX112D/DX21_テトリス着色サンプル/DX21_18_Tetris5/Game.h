#pragma once
#include "Object.h"
#include "input.h"
#include <vector>
#include <string>

class Game {
private:
	Input input;
	Object m_background; // 背景
	Object m_player;     // 玩家（竖直木棍）

	// 地图（平台）列表
	struct Platform { Object obj; DirectX::XMFLOAT3 pos; DirectX::XMFLOAT3 size; };
	std::vector<Platform> m_platforms;

	// 门（下一关入口）
	struct Door { Object obj; DirectX::XMFLOAT3 pos; DirectX::XMFLOAT3 size; };
	std::vector<Door> m_doors;

	// 地图交互按钮（可触碰触发）
	struct MapButton { Object obj; DirectX::XMFLOAT3 pos; DirectX::XMFLOAT3 size; bool triggered = false; };
	std::vector<MapButton> m_mapButtons;

	// 可拾取类型扩展：跳跃/攻击/冲刺
	enum class PickupType { JumpButton, AttackButton, DashButton };
	struct MapPickup { PickupType type; Object obj; DirectX::XMFLOAT3 pos; DirectX::XMFLOAT3 size; bool collected = false; };
	std::vector<MapPickup> m_pickups;

	// 右下角拾取展示图标（可选）
	Object m_invJumpIcon; bool m_hasJumpPickupIcon = false;
	Object m_invAttackIcon; bool m_hasAttackPickupIcon = false;
	Object m_invDashIcon; bool m_hasDashPickupIcon = false;

	// 玩家出生点
	DirectX::XMFLOAT3 m_spawnPos{ -200.0f, -160.0f, 0.0f };

	// 关卡索引
	int m_currentLevel = 1;

	// 跳跃与重力相关状态
	DirectX::XMFLOAT3 m_playerVel{0.0f, 0.0f, 0.0f};
	bool m_onGround = false;

	// 动作状态
	bool m_isDashing = false;
	int  m_dashFrames = 0;       // 冲刺剩余帧数
	bool m_isAttacking = false;
	int  m_attackFrames = 0;     // 攻击剩余帧数

	// UI解锁状态
	bool m_jumpButtonUnlocked = false; // 初始为未解锁，拾取绿色方块后解锁
	bool m_attackButtonUnlocked = true; // 若需拾取解锁可改为false
	bool m_dashButtonUnlocked = true;   // 若需拾取解锁可改为false

	// 控制是否隐藏右下角跳跃方块
	bool m_hideJumpUIButton = false;

	// 右侧按钮（UI）
	struct Button { Object obj; DirectX::XMFLOAT3 pos; DirectX::XMFLOAT3 size; int slotIndex = -1; };
	Button m_btnJump;
	Button m_btnAttack;
	Button m_btnDash;

	// 槽位：带动作类型与占用标记
	struct Slot { Object frame; DirectX::XMFLOAT3 pos; DirectX::XMFLOAT3 size; bool occupied = false; int actionType = -1; };
	std::vector<Slot> m_slots; // 槽位

	// 当前选中的按钮（-1 无，0 跳跃，1 攻击，2 冲刺）
	int m_selectedButton = -1;

	// 槽位依次释放控制
	int m_nextSlotIndex = 0;     // 下一个检查的槽位索引（左到右）
	int m_slotReleaseCooldown = 0; // 冷却帧，控制释放节奏

	// 是否循环执行槽位动作（不清空，占用保持）
	bool m_loopSlots = true;

	// 从文本地图加载平台、出生点、门、交互按钮
	void LoadMapFromFile(const std::string& path, float tileW, float tileH);

	// 坐标转换：从客户端像素到世界坐标（中心为原点）
	DirectX::XMFLOAT2 ClientToWorld(int px, int py);

	// 进入下一关
	void GoToNextLevel();

	// 触发地图按钮交互（示例：改变颜色或打开门等）
	void OnMapButtonTriggered(MapButton& btn);

	// 拾取处理
	void CollectPickup(MapPickup& p);

public:
	void Init(HWND hWnd);   // 初始化
	void Update(void);      // 更新
	void Draw();            // 绘制
	void Uninit();          // 释放
};

