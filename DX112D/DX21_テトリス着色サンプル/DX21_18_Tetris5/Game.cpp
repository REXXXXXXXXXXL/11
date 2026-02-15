#include "Game.h"
#include <time.h>
#include <fstream>

static const float GRAVITY = -0.6f;
static const float MOVE_SPEED = 4.0f;
static const float JUMP_SPEED = 16.0f;
static const float DASH_SPEED = 12.0f;
static const int   DASH_DURATION = 12;
static const int   ATTACK_DURATION = 10;
static const int   SLOT_RELEASE_COOLDOWN_FRAMES = 20;

// 简单AABB
static bool AABB(const DirectX::XMFLOAT3& p1, const DirectX::XMFLOAT3& s1,
                 const DirectX::XMFLOAT3& p2, const DirectX::XMFLOAT3& s2)
{
    return fabsf(p1.x - p2.x) * 2.0f < (s1.x + s2.x)
        && fabsf(p1.y - p2.y) * 2.0f < (s1.y + s2.y);
}

DirectX::XMFLOAT2 Game::ClientToWorld(int px, int py)
{
    float x = (float)px - (float)SCREEN_WIDTH * 0.5f;
    float y = (float)SCREEN_HEIGHT * 0.5f - (float)py;
    return { x, y };
}

void Game::LoadMapFromFile(const std::string& path, float tileW, float tileH)
{
    m_platforms.clear();
    m_doors.clear();
    m_mapButtons.clear();
    m_pickups.clear();

    std::ifstream ifs(path);
    std::vector<std::string> lines;
    std::string line;
    size_t maxCols = 0;
    while (std::getline(ifs, line))
    {
        lines.push_back(line);
        if (line.size() > maxCols) maxCols = line.size();
    }
    size_t rows = lines.size();
    size_t cols = maxCols;
    if (rows == 0 || cols == 0) return;

    // 地图占据左侧 4/5 并填满画面高度
    float mapAreaWidth = (float)SCREEN_WIDTH * 0.8f;      // 左侧 4/5
    float mapAreaHeight = (float)SCREEN_HEIGHT;           // 全高度
    float startX = -(float)SCREEN_WIDTH * 0.5f;           // 屏幕左边界

    // 瓦片大小严格按行列平均分配，保证填满区域
    float tileWidth = mapAreaWidth / (float)cols;
    float tileHeight = mapAreaHeight / (float)rows;

    for (size_t r = 0; r < rows; ++r)
    {
        const std::string& ln = lines[r];
        for (size_t c = 0; c < cols; ++c)
        {
            char ch = (c < ln.size()) ? ln[c] : ' ';
            float x = startX + (float)c * tileWidth + tileWidth * 0.5f;                // 从左边界开始
            float y = ((float)SCREEN_HEIGHT * 0.5f) - (float)r * tileHeight - tileHeight * 0.5f; // 从上往下

            if (ch == '#')
            {
                Platform pf{};
                pf.pos = {x, y, 0.0f};
                pf.size = {tileWidth, tileHeight, 0.0f};
                pf.obj.Init("asset/block.png");
                pf.obj.SetPos(x, y, 0.0f);
                pf.obj.SetSize(tileWidth, tileHeight, 0.0f);
                pf.obj.SetAngle(0.0f);
                pf.obj.SetColor(0.8f, 0.8f, 0.9f, 1.0f);
                pf.obj.SetUseCamera(true);
                m_platforms.push_back(pf);
            }
            else if (ch == 'S')
            {
                // 出生点（玩家中心）
                m_spawnPos = { x, y, 0.0f };
            }
            else if (ch == 'D')
            {
                // 门（通往下一关）
                Door d{};
                d.pos = {x, y, 0.0f};
                d.size = {tileWidth, tileHeight, 0.0f};
                d.obj.Init("asset/block.png");
                d.obj.SetPos(x, y, 0.0f);
                d.obj.SetSize(tileWidth * 0.8f, tileHeight * 0.8f, 0.0f);
                d.obj.SetAngle(0.0f);
                d.obj.SetColor(0.2f, 1.0f, 0.2f, 1.0f);
                d.obj.SetUseCamera(true);
                m_doors.push_back(d);
            }
            else if (ch == 'B')
            {
                // 交互按钮：默认蓝色，大小为tile方块大小
                MapButton b{};
                b.pos = {x, y, 0.0f};
                b.size = {tileWidth, tileHeight, 0.0f};
                b.obj.Init("asset/block.png");
                b.obj.SetPos(x, y, 0.0f);
                b.obj.SetSize(tileWidth * 0.8f, tileHeight * 0.8f, 0.0f);
                b.obj.SetAngle(0.0f);
                b.obj.SetColor(0.2f, 0.5f, 1.0f, 1.0f);
                b.obj.SetUseCamera(true);
                m_mapButtons.push_back(b);
            }
            else if (ch == 'J')
            {
                // 可拾取的跳跃按钮（拾取后解锁UI跳跃）
                MapPickup p{};
                p.type = PickupType::JumpButton;
                p.pos = {x, y, 0.0f};
                p.size = {tileWidth, tileHeight, 0.0f};
                p.obj.Init("asset/block.png");
                p.obj.SetPos(x, y, 0.0f);
                p.obj.SetSize(tileWidth * 0.7f, tileHeight * 0.7f, 0.0f);
                p.obj.SetAngle(0.0f);
                p.obj.SetColor(0.3f, 0.9f, 0.3f, 1.0f); // 绿色表示跳跃Pickup
                p.obj.SetUseCamera(true);
                m_pickups.push_back(p);
            }
            else if (ch == 'A')
            {
                // 可拾取的攻击按钮（拾取后解锁UI攻击）
                MapPickup p{};
                p.type = PickupType::AttackButton;
                p.pos = {x, y, 0.0f};
                p.size = {tileWidth, tileHeight, 0.0f};
                p.obj.Init("asset/block.png");
                p.obj.SetPos(x, y, 0.0f);
                p.obj.SetSize(tileWidth * 0.7f, tileHeight * 0.7f, 0.0f);
                p.obj.SetAngle(0.0f);
                p.obj.SetColor(0.9f, 0.6f, 0.2f, 1.0f); // 橙色表示攻击Pickup
                p.obj.SetUseCamera(true);
                m_pickups.push_back(p);
            }
            else if (ch == 'H')
            {
                // 可拾取的冲刺按钮（拾取后解锁UI冲刺）
                MapPickup p{};
                p.type = PickupType::DashButton;
                p.pos = {x, y, 0.0f};
                p.size = {tileWidth, tileHeight, 0.0f};
                p.obj.Init("asset/block.png");
                p.obj.SetPos(x, y, 0.0f);
                p.obj.SetSize(tileWidth * 0.7f, tileHeight * 0.7f, 0.0f);
                p.obj.SetAngle(0.0f);
                p.obj.SetColor(0.3f, 0.6f, 0.9f, 1.0f); // 蓝色表示冲刺Pickup
                p.obj.SetUseCamera(true);
                m_pickups.push_back(p);
            }
        }
    }
}

void Game::GoToNextLevel()
{
    m_currentLevel++;
    // 根据关卡索引加载不同地图
    if (m_currentLevel == 2) {
        LoadMapFromFile("asset/map2.txt", 0.0f, 0.0f);
    } else {
        LoadMapFromFile("asset/map.txt", 0.0f, 0.0f);
    }
    // 重置玩家位置与速度
    m_player.SetPos(m_spawnPos.x, m_spawnPos.y, m_spawnPos.z);
    m_playerVel = {0.0f, 0.0f, 0.0f};
    m_onGround = false;
}

void Game::OnMapButtonTriggered(MapButton& btn)
{
    // 示例交互：按钮变绿，打开最近的门（或执行自定义逻辑）
    btn.obj.SetColor(0.3f, 1.0f, 0.3f, 1.0f);
    btn.triggered = true;
    // 可扩展：比如将某个平台移除、改变全局状态等
}

void Game::CollectPickup(MapPickup& p)
{
    if (p.collected) return;
    p.collected = true;
    // 简单反馈：变色并缩小
    p.obj.SetColor(1.0f, 1.0f, 1.0f, 0.4f);
    p.obj.SetSize(p.size.x * 0.5f, p.size.y * 0.5f, 0.0f);

    float iconW = 40.0f, iconH = 40.0f;
    float bottomY = -(float)SCREEN_HEIGHT * 0.5f + iconH * 0.5f + 10.0f;
    float rightEdge = (float)SCREEN_WIDTH * 0.5f - 10.0f;

    if (p.type == PickupType::JumpButton)
    {
        // 解锁右下角跳跃交互方块
        m_jumpButtonUnlocked = true;

        // 刷新右下角三个交互方块中的跳跃方块为绿色并可交互
        float rightAreaX = -(float)SCREEN_WIDTH * 0.5f + (float)SCREEN_WIDTH * 0.8f;
        float rightAreaWidth = (float)SCREEN_WIDTH * 0.2f;
        float margin = 10.0f;
        float btnSize = rightAreaWidth * 0.28f;
        float bottomRowY = -(float)SCREEN_HEIGHT * 0.5f + margin + btnSize * 0.5f;

        // 跳跃按钮位置（保持与初始化一致）
        float jumpX = rightAreaX + margin + btnSize * 0.5f + (btnSize + margin) * 0;
        m_btnJump.obj.SetPos(jumpX, bottomRowY, 0.0f);
        m_btnJump.pos = { jumpX, bottomRowY, 0.0f };
        m_btnJump.obj.SetSize(btnSize, btnSize, 0.0f);
        m_btnJump.obj.SetColor(0.3f, 0.9f, 0.3f, 1.0f); // 绿色可交互
        m_btnJump.obj.SetUseCamera(false);

        // 右下角图标区（可选展示）
        if (!m_hasJumpPickupIcon)
        {
            m_invJumpIcon.Init("asset/block.png");
            m_invJumpIcon.SetPos(rightEdge - iconW * 0.5f, bottomY, 0.0f);
            m_invJumpIcon.SetSize(iconW, iconH, 0.0f);
            m_invJumpIcon.SetAngle(0.0f);
            m_invJumpIcon.SetColor(0.3f, 0.9f, 0.3f, 1.0f);
            m_invJumpIcon.SetUseCamera(false);
            m_hasJumpPickupIcon = true;
        }
    }
    else if (p.type == PickupType::AttackButton)
    {
        // 解锁右下角攻击交互方块
        m_attackButtonUnlocked = true;

        // 刷新右下角三个交互方块中的攻击方块为橙色并可交互
        float rightAreaX = -(float)SCREEN_WIDTH * 0.5f + (float)SCREEN_WIDTH * 0.8f;
        float rightAreaWidth = (float)SCREEN_WIDTH * 0.2f;
        float margin = 10.0f;
        float btnSize = rightAreaWidth * 0.28f;
        float bottomRowY = -(float)SCREEN_HEIGHT * 0.5f + margin + btnSize * 0.5f;
        float attackX = rightAreaX + margin + btnSize * 0.5f + (btnSize + margin) * 1;
        m_btnAttack.obj.SetPos(attackX, bottomRowY, 0.0f);
        m_btnAttack.pos = { attackX, bottomRowY, 0.0f };
        m_btnAttack.obj.SetSize(btnSize, btnSize, 0.0f);
        m_btnAttack.obj.SetColor(0.9f, 0.6f, 0.2f, 1.0f); // 橙色可交互
        m_btnAttack.obj.SetUseCamera(false);

        // 右下角图标区（可选展示）
        if (!m_hasAttackPickupIcon)
        {
            m_invAttackIcon.Init("asset/block.png");
            m_invAttackIcon.SetPos(rightEdge - iconW * 1.5f, bottomY, 0.0f);
            m_invAttackIcon.SetSize(iconW, iconH, 0.0f);
            m_invAttackIcon.SetAngle(0.0f);
            m_invAttackIcon.SetColor(0.9f, 0.6f, 0.2f, 1.0f);
            m_invAttackIcon.SetUseCamera(false);
            m_hasAttackPickupIcon = true;
        }
    }
    else if (p.type == PickupType::DashButton)
    {
        // 解锁右下角冲刺交互方块
        m_dashButtonUnlocked = true;

        // 刷新右下角三个交互方块中的冲刺方块为蓝色并可交互
        float rightAreaX = -(float)SCREEN_WIDTH * 0.5f + (float)SCREEN_WIDTH * 0.8f;
        float rightAreaWidth = (float)SCREEN_WIDTH * 0.2f;
        float margin = 10.0f;
        float btnSize = rightAreaWidth * 0.28f;
        float bottomRowY = -(float)SCREEN_HEIGHT * 0.5f + margin + btnSize * 0.5f;
        float dashX = rightAreaX + margin + btnSize * 0.5f + (btnSize + margin) * 2;
        m_btnDash.obj.SetPos(dashX, bottomRowY, 0.0f);
        m_btnDash.pos = { dashX, bottomRowY, 0.0f };
        m_btnDash.obj.SetSize(btnSize, btnSize, 0.0f);
        m_btnDash.obj.SetColor(0.3f, 0.6f, 0.9f, 1.0f); // 蓝色可交互
        m_btnDash.obj.SetUseCamera(false);

        // 右下角图标区（可选展示）
        if (!m_hasDashPickupIcon)
        {
            m_invDashIcon.Init("asset/block.png");
            m_invDashIcon.SetPos(rightEdge - iconW * 2.5f, bottomY, 0.0f);
            m_invDashIcon.SetSize(iconW, iconH, 0.0f);
            m_invDashIcon.SetAngle(0.0f);
            m_invDashIcon.SetColor(0.3f, 0.6f, 0.9f, 1.0f);
            m_invDashIcon.SetUseCamera(false);
            m_hasDashPickupIcon = true;
        }
    }
}

void Game::Init(HWND hWnd)
{
    RendererInit(hWnd);

    // 背景
    m_background.Init("asset/back_img_01.png");
    m_background.SetPos(0.0f, 0.0f, 0.0f);
    m_background.SetSize((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f);
    m_background.SetAngle(0.0f);
    m_background.SetUseCamera(true);

    // 玩家
    m_player.Init("asset/block.png");
    m_player.SetPos(m_spawnPos.x, m_spawnPos.y, m_spawnPos.z);
    m_player.SetSize(10.0f, 50.0f, 0.0f);
    m_player.SetAngle(0.0f);
    m_player.SetColor(0.7f, 0.5f, 0.3f, 1.0f);
    m_player.SetUseCamera(true);

    // 地图
    LoadMapFromFile("asset/map.txt", 0.0f, 0.0f);
    // 将玩家放到出生点
    m_player.SetPos(m_spawnPos.x, m_spawnPos.y, m_spawnPos.z);

    // 右侧区域与槽位/按钮（4个槽位，竖直）
    float rightAreaX = -(float)SCREEN_WIDTH * 0.5f + (float)SCREEN_WIDTH * 0.8f;
    float rightAreaWidth = (float)SCREEN_WIDTH * 0.2f;
    float margin = 10.0f;
    float btnSize = rightAreaWidth * 0.28f;

    float slotSize = btnSize;
    float slotSpacing = slotSize + margin;
    float centerX = rightAreaX + rightAreaWidth * 0.5f;
    float topY = (float)SCREEN_HEIGHT * 0.5f - margin - slotSize * 0.5f;

    m_slots.clear();
    m_slots.resize(4);
    for (int i = 0; i < 4; ++i)
    {
        float cx = centerX;
        float cy = topY - i * slotSpacing;
        m_slots[i].pos = { cx, cy, 0.0f };
        m_slots[i].size = { slotSize, slotSize, 0.0f };
        m_slots[i].occupied = false;
        m_slots[i].actionType = -1;
        m_slots[i].frame.Init("asset/block.png");
        m_slots[i].frame.SetPos(cx, cy, 0.0f);
        m_slots[i].frame.SetSize(slotSize, slotSize, 0.0f);
        m_slots[i].frame.SetAngle(0.0f);
        m_slots[i].frame.SetColor(0.9f, 0.9f, 0.9f, 0.5f);
        m_slots[i].frame.SetUseCamera(false);
    }

    float bottomY = -(float)SCREEN_HEIGHT * 0.5f;
    float startBtnX = rightAreaX + margin + btnSize * 0.5f;
    float rowY = bottomY + margin + btnSize * 0.5f;

    auto initButtonSq = [&](Button& b, float centerX, float centerY, float r, float g, float bl)
    {
        b.pos = { centerX, centerY, 0.0f };
        b.size = { btnSize, btnSize, 0.0f };
        b.obj.Init("asset/block.png");
        b.obj.SetPos(b.pos.x, b.pos.y, 0.0f);
        b.obj.SetSize(b.size.x, b.size.y, 0.0f);
        b.obj.SetAngle(0.0f);
        b.obj.SetColor(r, g, bl, 1.0f);
        b.obj.SetUseCamera(false);
    };

    // 初始右下角三个方块：跳跃灰色（锁定），攻击橙色，冲刺蓝色
    initButtonSq(m_btnJump,   startBtnX + (btnSize + margin) * 0, rowY, 0.5f, 0.5f, 0.5f);
    m_btnJump.obj.SetColor(0.5f, 0.5f, 0.5f, 0.3f);
    initButtonSq(m_btnAttack, startBtnX + (btnSize + margin) * 1, rowY, 0.9f, 0.6f, 0.2f);
    initButtonSq(m_btnDash,   startBtnX + (btnSize + margin) * 2, rowY, 0.3f, 0.6f, 0.9f);

    // 初始化释放控制
    m_nextSlotIndex = 0;
    m_slotReleaseCooldown = 0;

    // 视角缩放（可选）
    RendererSetZoom(1.5f);
}

void Game::Update(void)
{
    input.Update();

    // UI拖拽同前（可考虑禁用未解锁跳跃按钮的拖拽）
    POINT pt; GetCursorPos(&pt); ScreenToClient(GetForegroundWindow(), &pt); auto world = ClientToWorld(pt.x, pt.y);
    auto hitButton = [&](const Button& b){ return fabsf(world.x - b.pos.x) * 2.0f < (b.size.x) && fabsf(world.y - b.pos.y) * 2.0f < (b.size.y); };

    static bool dragging = false; static DirectX::XMFLOAT2 dragOffset{0,0}; bool mouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    if (!dragging && mouseDown)
    {
        if (m_jumpButtonUnlocked && hitButton(m_btnJump))   { m_selectedButton = 0; dragging = true; dragOffset = { world.x - m_btnJump.pos.x,   world.y - m_btnJump.pos.y }; }
        else if (m_attackButtonUnlocked && hitButton(m_btnAttack)) { m_selectedButton = 1; dragging = true; dragOffset = { world.x - m_btnAttack.pos.x, world.y - m_btnAttack.pos.y }; }
        else if (m_dashButtonUnlocked && hitButton(m_btnDash))   { m_selectedButton = 2; dragging = true; dragOffset = { world.x - m_btnDash.pos.x,   world.y - m_btnDash.pos.y }; }
    }
    else if (dragging)
    {
        DirectX::XMFLOAT3 followPos{ world.x - dragOffset.x, world.y - dragOffset.y, 0.0f };
        Button* btn = nullptr;
        switch (m_selectedButton)
        { case 0: btn = &m_btnJump; break; case 1: btn = &m_btnAttack; break; case 2: btn = &m_btnDash; break; }
        if (btn)
        {
            if (btn->slotIndex != -1)
            {
                int idx = btn->slotIndex;
                m_slots[idx].occupied = false;
                m_slots[idx].actionType = -1;
                btn->slotIndex = -1;
            }
            btn->obj.SetPos(followPos.x, followPos.y, 0.0f);
            btn->pos = followPos;
        }
        if (!mouseDown)
        {
            int bestIdx = -1; float bestDist2 = FLT_MAX; DirectX::XMFLOAT3 curPos = followPos;
            for (size_t i = 0; i < m_slots.size(); ++i)
            {
                if (m_slots[i].occupied) continue;
                float dx = curPos.x - m_slots[i].pos.x; float dy = curPos.y - m_slots[i].pos.y; float d2 = dx * dx + dy * dy;
                if (d2 < bestDist2) { bestDist2 = d2; bestIdx = (int)i; }
            }
            if (bestIdx != -1)
            {
                float radius = m_slots[bestIdx].size.x * 0.5f; float snapRange = radius * 1.2f;
                if (bestDist2 <= snapRange * snapRange)
                {
                    DirectX::XMFLOAT3 target = m_slots[bestIdx].pos;
                    if (btn)
                    {
                        btn->obj.SetPos(target.x, target.y, 0.0f);
                        btn->pos = target;
                        btn->slotIndex = bestIdx;
                    }
                    int action = m_selectedButton;
                    m_slots[bestIdx].occupied = true;
                    m_slots[bestIdx].actionType = action;
                }
            }
            dragging = false; m_selectedButton = -1;
        }
    }

    // 槽位依次释放（循环）——同前
    if (m_slotReleaseCooldown > 0) { m_slotReleaseCooldown--; }
    else
    {
        for (int i = 0; i < (int)m_slots.size(); ++i)
        {
            int idx = (m_nextSlotIndex + i) % (int)m_slots.size();
            if (m_slots[idx].occupied && m_slots[idx].actionType != -1)
            {
                int act = m_slots[idx].actionType;
                if (act == 0 && m_jumpButtonUnlocked) { if (m_onGround) { m_playerVel.y = JUMP_SPEED; m_onGround = false; } }
                else if (act == 1 && m_attackButtonUnlocked) { if (!m_isAttacking) { m_isAttacking = true; m_attackFrames = ATTACK_DURATION; m_player.SetColor(1.0f, 0.6f, 0.2f, 1.0f); } }
                else if (act == 2 && m_dashButtonUnlocked) { if (!m_isDashing) { m_isDashing = true; m_dashFrames = DASH_DURATION; } }

                if (!m_loopSlots) { m_slots[idx].occupied = false; m_slots[idx].actionType = -1; }
                m_nextSlotIndex = (idx + 1) % (int)m_slots.size();
                m_slotReleaseCooldown = SLOT_RELEASE_COOLDOWN_FRAMES;
                break;
            }
        }
    }

    // 键盘/手柄动作保留
    if (!m_isAttacking && (input.GetKeyTrigger('J') || input.GetButtonTrigger(XINPUT_GAMEPAD_X)))
    { m_isAttacking = true; m_attackFrames = ATTACK_DURATION; m_player.SetColor(1.0f, 0.6f, 0.2f, 1.0f); }
    if (!m_isDashing && (input.GetKeyTrigger(VK_SHIFT) || input.GetButtonTrigger(XINPUT_GAMEPAD_B)))
    { m_isDashing = true; m_dashFrames = DASH_DURATION; }
    if (m_isAttacking && --m_attackFrames <= 0) { m_isAttacking = false; m_player.SetColor(0.7f, 0.5f, 0.3f, 1.0f); }
    if (m_isDashing && --m_dashFrames <= 0) { m_isDashing = false; }

    // 移动 & 重力 & 碰撞
    float ax = 0.0f; if (input.GetKeyPress(VK_LEFT) || input.GetKeyPress('A')) ax -= 1.0f; if (input.GetKeyPress(VK_RIGHT) || input.GetKeyPress('D')) ax += 1.0f;
    float moveSpeed = m_isDashing ? DASH_SPEED : MOVE_SPEED; m_playerVel.x = ax * moveSpeed; m_playerVel.y += GRAVITY;

    DirectX::XMFLOAT3 pos = m_player.GetPos(); DirectX::XMFLOAT3 size = m_player.GetSize();
    DirectX::XMFLOAT3 nextX{pos.x + m_playerVel.x, pos.y, pos.z}; bool blockX = false;
    for (size_t i = 0; i < m_platforms.size(); ++i) { Platform& pf = m_platforms[i]; if (AABB(nextX, size, pf.pos, pf.size)) { blockX = true; break; } }
    if (!blockX) pos.x = nextX.x; else m_playerVel.x = 0.0f;

    DirectX::XMFLOAT3 nextY{pos.x, pos.y + m_playerVel.y, pos.z}; bool blockY = false; m_onGround = false;
    for (size_t i = 0; i < m_platforms.size(); ++i)
    { Platform& pf = m_platforms[i]; if (AABB(nextY, size, pf.pos, pf.size)) { blockY = true; if (pos.y >= pf.pos.y) { pos.y = pf.pos.y + (pf.size.y + size.y) * 0.5f; m_onGround = true; m_playerVel.y = 0.0f; } else { pos.y = pf.pos.y - (pf.size.y + size.y) * 0.5f; m_playerVel.y = 0.0f; } break; } }
    if (!blockY) pos.y = nextY.y;

    float halfW = (float)SCREEN_WIDTH * 0.5f; float halfH = (float)SCREEN_HEIGHT * 0.5f; float mapAreaWidth = (float)SCREEN_WIDTH * 0.8f; float leftBound = -halfW; float rightBound = leftBound + mapAreaWidth;
    float halfPlayerW = size.x * 0.5f; float halfPlayerH = size.y * 0.5f;
    if (pos.x < leftBound + halfPlayerW) pos.x = leftBound + halfPlayerW; if (pos.x > rightBound - halfPlayerW) pos.x = rightBound - halfPlayerW; if (pos.y < -halfH + halfPlayerH) pos.y = -halfH + halfPlayerH; if (pos.y > halfH - halfPlayerH) pos.y = halfH - halfPlayerH;

    // 门碰撞（进入下一关）
    for (size_t i = 0; i < m_doors.size(); ++i)
    {
        Door& d = m_doors[i];
        if (AABB(pos, size, d.pos, d.size))
        {
            GoToNextLevel();
            // 更新pos为新的出生点
            pos = m_spawnPos;
            break;
        }
    }

    // 拾取检测：玩家碰到后收集
    for (size_t i = 0; i < m_pickups.size(); ++i)
    {
        MapPickup& p = m_pickups[i];
        if (!p.collected && AABB(pos, size, p.pos, p.size))
        {
            CollectPickup(p);
        }
    }

    // 地图按钮交互：碰到后触发（可按键确认）
    for (size_t i = 0; i < m_mapButtons.size(); ++i)
    {
        MapButton& b = m_mapButtons[i];
        if (AABB(pos, size, b.pos, b.size))
        {
            // 如果需要按键确认，检查例如'E'键
            if (!b.triggered)
            {
                if (input.GetKeyTrigger('E'))
                {
                    OnMapButtonTriggered(b);
                }
            }
        }
    }

    m_player.SetPos(pos.x, pos.y, pos.z);
}

void Game::Draw(void)
{
    // 视角跟随
    DirectX::XMFLOAT3 p = m_player.GetPos();
    RendererSetCameraOffset(p.x, p.y);

    RendererDrawStart();
    m_background.Draw();
    for (size_t i = 0; i < m_platforms.size(); ++i) m_platforms[i].obj.Draw();
    for (size_t i = 0; i < m_doors.size(); ++i) m_doors[i].obj.Draw();
    for (size_t i = 0; i < m_mapButtons.size(); ++i) m_mapButtons[i].obj.Draw();
    for (size_t i = 0; i < m_pickups.size(); ++i) if (!m_pickups[i].collected) m_pickups[i].obj.Draw();
    m_player.Draw();
    for (size_t i = 0; i < m_slots.size(); ++i) m_slots[i].frame.Draw();

    // 按标记隐藏右下角跳跃UI
    if (!m_hideJumpUIButton) m_btnJump.obj.Draw();
    m_btnAttack.obj.Draw(); m_btnDash.obj.Draw();

    if (m_hasJumpPickupIcon) m_invJumpIcon.Draw();
    if (m_hasAttackPickupIcon) m_invAttackIcon.Draw();
    if (m_hasDashPickupIcon) m_invDashIcon.Draw();
    RendererDrawEnd();
}

void Game::Uninit(void)
{
    m_player.Uninit();
    for (size_t i = 0; i < m_platforms.size(); ++i) m_platforms[i].obj.Uninit();
    for (size_t i = 0; i < m_doors.size(); ++i) m_doors[i].obj.Uninit();
    for (size_t i = 0; i < m_mapButtons.size(); ++i) m_mapButtons[i].obj.Uninit();
    for (size_t i = 0; i < m_pickups.size(); ++i) m_pickups[i].obj.Uninit();
    for (size_t i = 0; i < m_slots.size(); ++i) m_slots[i].frame.Uninit();
    m_btnJump.obj.Uninit(); m_btnAttack.obj.Uninit(); m_btnDash.obj.Uninit();
    if (m_hasJumpPickupIcon) m_invJumpIcon.Uninit();
    if (m_hasAttackPickupIcon) m_invAttackIcon.Uninit();
    if (m_hasDashPickupIcon) m_invDashIcon.Uninit();
    m_background.Uninit(); RendererUninit();
}
