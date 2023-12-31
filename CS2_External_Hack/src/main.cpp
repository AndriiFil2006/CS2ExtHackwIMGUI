//Required stuff
#include <Windows.h>
#include <dwmapi.h>
#include <d3d11.h>
#include <thread>

//Imgui
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

//Game offsets
#include <offsets.hpp>
#include <ProcessManager.hpp>
#include <client.dll.hpp>
#include <vector.h>
#include <Render.h>
#include <Bone.h>
#include <C_UTL_VECTOR.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using namespace client_dll;
using namespace CBasePlayerWeaponVData;
using namespace C_BaseEntity;
using namespace CEffectData;
using namespace CCSPlayerController;
using namespace C_CSPlayerPawnBase;
using namespace CCSPlayerController;
using namespace C_BasePlayerPawn;
using namespace C_BaseModelEntity;
using namespace EntitySpottedState_t;
using namespace CPlayer_MovementServices;
using namespace CBasePlayerController;
using namespace C_CSPlayerPawn;

int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);

float PI = atan(1) * 4;

struct vec3
{
    float x, y, z;
};

struct Vector2
{
    float x, y;
};

struct CurrProcess
{
    DWORD64 localPlayer;
    DWORD64 processID;
    uintptr_t moduleBase;
    BYTE flag;
    DWORD64 jump;
    DWORD64 localPawn;
    uintptr_t entityList;
    DWORD64 listEntry;
    DWORD64 playerPawn;
    int localTeam;
    float tDelay;
    DWORD64 glowObj;
    Vector3 localPlayerHeadPos;
    Vector3 localViewAngles;
    DWORD64 engine;
    DWORD64 localController;
}proc;

struct GameOffsets
{
    DWORD localPawn = dwLocalPlayerPawn;
    DWORD playerPawn = m_hPlayerPawn;
    DWORD fJump = dwForceJump;
    DWORD flags = C_BaseEntity::m_fFlags;
    DWORD locPlayerController = dwLocalPlayerController;
    DWORD entList = dwEntityList;
    DWORD health = m_iHealth;
    DWORD flashDur = m_flFlashDuration;
    DWORD crosshairID = m_iIDEntIndex;
    DWORD team = m_iTeamNum;
    DWORD forceAttack = dwForceAttack;
    DWORD currWeapon = m_pClippingWeapon;
    DWORD isScoped = m_bIsScoped;
    DWORD glowObjMgr = dwGlowManager;
    DWORD clrRender = m_clrRender;
    DWORD viewMatrix = dwViewMatrix;
    DWORD vecOrigin = m_vOldOrigin;
    DWORD gameScene = m_pGameSceneNode;
    DWORD spottedByMask = m_bSpottedByMask;
    DWORD viewAngles = dwViewAngles;
    DWORD entitySpottedState = m_entitySpottedState;
    DWORD hPawn = m_hPawn;
    DWORD shotsFired = m_iShotsFired;
    DWORD punchCache = m_aimPunchCache;
    DWORD aimPunchAngle = m_aimPunchAngle;
}offset;


void getGameAddresses()
{
    proc.processID = ProcessMgr.GetProcessID("cs2.exe");
    //proc.moduleBase = GetProcessModuleHandle(proc.processID, "client.dll");
    proc.moduleBase = reinterpret_cast<DWORD64>(ProcessMgr.GetProcessModuleHandle(proc.processID, "client.dll"));

    ProcessMgr.ProcessID = proc.processID;
    ProcessMgr.ModuleAddress = proc.moduleBase;
    ProcessMgr.hProcess = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_CREATE_THREAD, TRUE, proc.processID);

    do
    {
        ProcessMgr.ReadMemory<DWORD64>(proc.moduleBase + offset.locPlayerController, proc.localController);
    } while (proc.localController == NULL);

    ProcessMgr.ReadMemory<DWORD64>(proc.moduleBase + offset.entList, proc.entityList);

    ProcessMgr.ReadMemory<DWORD64>(proc.localController + offset.playerPawn, proc.localPawn);

    ProcessMgr.ReadMemory<DWORD64>(proc.entityList + 0x8 * ((proc.localPawn & 0x7FFF) >> 9) + 16, proc.listEntry);

    ProcessMgr.ReadMemory<DWORD64>(proc.listEntry + 120 * (proc.localPawn & 0x1FF), proc.localPlayer);

    ProcessMgr.ReadMemory<int>(proc.localPlayer + offset.team, proc.localTeam);

    proc.engine = reinterpret_cast<DWORD64>(ProcessMgr.GetProcessModuleHandle(proc.processID, "engine2.dll"));
}

void antiFlashHandler()
{
    float flashDur = 0, flD;
    ProcessMgr.WriteMemory<float>(proc.localPlayer + offset.flashDur, flashDur);
}


void bHophandler()
{
    ProcessMgr.ReadMemory<BYTE>(proc.localPlayer + offset.flags, proc.flag);
    proc.jump = proc.moduleBase + offset.fJump;
    if (GetAsyncKeyState(VK_SPACE))
    {
        DWORD val1 = 256, val2 = 65537;
        if (proc.flag & (1 << 0))
        {
            ProcessMgr.WriteMemory<DWORD>(proc.jump, val2);
            Sleep(2);
            ProcessMgr.WriteMemory<DWORD>(proc.jump, val1);
            Sleep(2);
        }
    }
}

void setTBDelay(float dist, std::string currWeapon)
{
    float delay = 0;

    if (currWeapon == "m4a1") // M4A1-S
    {
        delay = 3.0;
    }
    else if (currWeapon == "m4a4") // M4A4
    {
        delay = 3.3;
    }
    else if (currWeapon == "mp5sd") // MP5-SD
    {
        delay = 3.0;
    }
    else if (currWeapon == "aug") // AUG
    {
        delay = 3.5;
    }
    else if (currWeapon == "famas") // FAMAS
    {
        delay = 3.3;
    }
    else if (currWeapon == "elite") // DUAL BERETTAS
    {
        delay = 3.0;
    }
    else if (currWeapon == "fiveseven") // Five Seven
    {
        delay = 3.0;
    }
    else if (currWeapon == "p90") // P90
    {
        delay = 0.5;
    }
    else if (currWeapon == "mac10") // Mac-10
    {
        delay = 0.5;
    }
    else if (currWeapon == "ump45") // UMP-45
    {
        delay = 2.5;
    }
    else if (currWeapon == "bizon") // PP-bizon
    {
        delay = 0.5;
    }
    else if (currWeapon == "mp7") // mp7
    {
        delay = 2.5;
    }
    else if (currWeapon == "mp9") // mp9
    {
        delay = 1.5;
    }
    else if (currWeapon == "ak47") // ak47
    {
        delay = 3.3;
    }
    else if (currWeapon == "ssg08") // ssg08
    {
        delay = 0.2;
    }
    else if (currWeapon == "hkp2000") // usp-s
    {
        delay = 4.5;
    }
    else if (currWeapon == "glock") // glock-18
    {
        delay = 1.5;
    }
    else if (currWeapon == "galilar") // Galil-AR
    {
        delay = 3.0;
    }
    else if (currWeapon == "sg556") // SSG-553
    {
        delay = 3.5;
    }
    else if (currWeapon == "awp") // AWP
    {
        delay = 0.2;
    }
    else if (currWeapon == "deagle") // Desert Eagle
    {
        delay = 12.5;
    }
    else
    {
        delay = 3.0;
    }

    proc.tDelay = delay * dist;
}

std::string getCurrweapon()
{
    DWORD64 CurrWeaponAddress = 0;
    char Buffer[40]{};

    CurrWeaponAddress = ProcessMgr.TraceAddress(proc.localPlayer + offset.currWeapon, { 0x10, 0x20, 0x0 });

    if (CurrWeaponAddress == 0)
    {
        return "ERROR";
    }

    ProcessMgr.ReadMemory(CurrWeaponAddress, Buffer, 40);

    std::string WeaponName = std::string(Buffer);

    std::size_t index = WeaponName.find("_");

    if (index == std::string::npos || WeaponName.empty())
    {
        return "ERROR";
    }
    else
    {
        return (WeaponName.substr(index + 1, WeaponName.size() - index - 1));
    }
    return "ERROR";
}

float getDistance(DWORD64 ent)
{
    vec3 localPlayerLoc, enemyLoc;

    ProcessMgr.ReadMemory<vec3>(proc.localPlayer + offset.vecOrigin, localPlayerLoc);
    ProcessMgr.ReadMemory<vec3>(ent + offset.vecOrigin, enemyLoc);

    return (sqrt(pow(localPlayerLoc.x - enemyLoc.x, 2) + pow(localPlayerLoc.y - enemyLoc.y, 2) + pow(localPlayerLoc.z - enemyLoc.z, 2)) * 0.0254);
}

void shoot()
{
    int shoot = 65537, stopTheViolence = 256;

    Sleep(proc.tDelay);
    ProcessMgr.WriteMemory<int>(proc.moduleBase + offset.forceAttack, shoot);

    Sleep(1);
    ProcessMgr.WriteMemory<int>(proc.moduleBase + offset.forceAttack, stopTheViolence);

    std::string currWeapon = getCurrweapon();
}

bool checkTBot()
{
    DWORD crosshair;
    ProcessMgr.ReadMemory<DWORD>(proc.localPlayer + offset.crosshairID, crosshair);

    if (crosshair != -1)
    {
        if (proc.listEntry == 0)
        {
            return false;
        }

        DWORD64 ent;
        ProcessMgr.ReadMemory<DWORD64>(proc.listEntry + 0x78 * (crosshair & 0x1FF), ent);
        int enTeam, enHealth;

        ProcessMgr.ReadMemory<int>(ent + offset.team, enTeam);
        ProcessMgr.ReadMemory<int>(ent + offset.health, enHealth);
        if (enTeam != proc.localTeam && enHealth > 0)
        {
            std::string currWeapon = getCurrweapon();

            if (currWeapon != "ERROR")
            {
                setTBDelay(getDistance(ent), currWeapon);

                if (currWeapon == "awp" || currWeapon == "ssg08")
                {
                    bool isScoped;
                    ProcessMgr.ReadMemory<bool>(proc.localPlayer + offset.isScoped, isScoped);
                    return isScoped;
                }

                return true;
            }
        }
    }

    return false;
}

void handleTBot()
{
    if (checkTBot())
    {
        shoot();
    }
}

float getDistCoord(Vector3 enHeadPos)
{
    return (sqrt(pow((enHeadPos.x - proc.localPlayerHeadPos.x), 2) + pow((enHeadPos.y - proc.localPlayerHeadPos.y), 2) + pow((enHeadPos.z - proc.localPlayerHeadPos.z), 2)));
}

Vector3 calculateAngles(Vector3 entHeadPos, float height)
{
    Vector3 oppPos = entHeadPos - proc.localPlayerHeadPos;

    float distance = getDistCoord(entHeadPos);

    //float hyp = sqrt(pow(oppPos.x, 2) + pow(oppPos.y, 2) + pow(oppPos.z, 2));

    float Yaw = atan2(oppPos.y, oppPos.x) * 180 / PI;
    //float Pitch = -atan(oppPos.z / distance) * 180 / PI;
    float Pitch = 0;

    float zeroPitchChangeHeight = -sin(proc.localViewAngles.x * PI / 180) * distance + proc.localPlayerHeadPos.z;

    if (zeroPitchChangeHeight <= entHeadPos.z && zeroPitchChangeHeight > (entHeadPos.z - height / 2))
    {
        Pitch = proc.localViewAngles.x;
    }
    else if (zeroPitchChangeHeight <= (entHeadPos.z - height / 2))
    {
        Pitch = -asin((oppPos.z - height / 2) / distance) * 180 / PI;
    }
    else
    {
        Pitch = -asin(oppPos.z / distance) * 180 / PI;
    }

    Vector3 newAngles{ Pitch, Yaw, 0 };

    return newAngles;
}

bool checkTheEnt(int entTeam, Vector3 entHeadPos, bool entSpotted, bool localSpotted, float height, float closestAngle, float maxDist, float maxAngle)
{
    if (entTeam == proc.localTeam)
    {
        return false;
    }

    float distance = getDistCoord(entHeadPos);

    if (distance > maxDist)
    {
        return false;
    }

    Vector3 newAngles = calculateAngles(entHeadPos, height);

    float deltaAngle = sqrt(pow(newAngles.x - proc.localViewAngles.x, 2) + pow(newAngles.y - proc.localViewAngles.y, 2));

    if (deltaAngle > maxAngle && deltaAngle < closestAngle)
    {
        return false;
    }

    if (!entSpotted && !localSpotted)
    {
        return false;
    }

    return true;
}

int findLocalPlayerInex()
{
    for (int i = 1; i < 32; i++)
    {
        DWORD64 listEntry;
        ProcessMgr.ReadMemory<DWORD64>(proc.entityList + (8 * (i & 0x7FFF) >> 9) + 16, listEntry);

        if (!listEntry)
        {
            continue;
        }

        DWORD64 ent;
        ProcessMgr.ReadMemory<DWORD64>(listEntry + 120 * (i & 0x1FF), ent);

        if (!ent)
        {
            continue;
        }

        DWORD64 entPawn;
        ProcessMgr.ReadMemory<DWORD64>(ent + offset.playerPawn, entPawn);

        DWORD64 listEntry2;
        ProcessMgr.ReadMemory<DWORD64>(proc.entityList + 0x8 * ((entPawn & 0x7FFF) >> 9) + 16, listEntry2);

        if (!listEntry2)
        {
            continue;
        }

        DWORD64 pCSPlayerPawn;
        ProcessMgr.ReadMemory<DWORD64>(listEntry2 + 120 * (entPawn & 0x1FF), pCSPlayerPawn);

        if (!pCSPlayerPawn)
        {
            continue;
        }

        if (pCSPlayerPawn == proc.localPlayer)
        {
            return i;
        }
    }
    return 0;
}


void smoothAim(Vector3 desiredAngles)
{
    int numberOfChanges = 10;
    Vector3 AngleDiff = desiredAngles - proc.localViewAngles;
    float oneStepPitch = AngleDiff.x / numberOfChanges;
    float oneStepYawn = AngleDiff.y / numberOfChanges;

    for (int i = 0; i < numberOfChanges; i++)
    {
        Vector3 newAngles{ proc.localViewAngles.x + oneStepPitch, proc.localViewAngles.y + oneStepYawn, proc.localViewAngles.z };
        proc.localViewAngles = newAngles;
        ProcessMgr.WriteMemory<Vector3>(proc.moduleBase + offset.viewAngles, newAngles);
        Sleep(0.8);
    }
}

Vector2 oldPunch = { };

Vector2 noRecoil(Vector3 desiredAngle)
{
    int locShotsFired;
    ProcessMgr.ReadMemory<int>(proc.localPlayer + offset.shotsFired, locShotsFired);

    if (locShotsFired > 1)
    {
        Vector2 punchAngle;
        C_UTL_VECTOR punchCache;
        ProcessMgr.ReadMemory<C_UTL_VECTOR>(proc.localPlayer + offset.punchCache, punchCache);

        if (punchCache.Count > 0 && punchCache.Count < 0xFFFF)
        {
            //if (ProcessMgr.ReadMemory<Vector2>(punchCache.Data +
                //(punchCache.Count - 1) * sizeof(Vector2), punchAngle))
            if (ProcessMgr.ReadMemory<Vector2>(proc.localPlayer + offset.aimPunchAngle, punchAngle));
            {
                Vector2 newAngles = Vector2
                {
                    desiredAngle.x + oldPunch.x - punchAngle.x * 2.f,
                    desiredAngle.y + oldPunch.y - punchAngle.y * 2.f,
                };

                if (newAngles.x > 89.0f)
                {
                    newAngles.x = 89.f;
                }
                if (newAngles.x < -89.0f)
                {
                    newAngles.x = -89.f;
                }

                while (newAngles.y > 180.f)
                {
                    newAngles.y -= 360.f;
                }

                while (newAngles.y < -180.f)
                {
                    newAngles.y += 360.f;
                }

                oldPunch.x = punchAngle.x * 2;
                oldPunch.y = punchAngle.y * 2;

                return newAngles;
            }
        }
    }
    else
    {
        oldPunch.x = oldPunch.y = 0.f;
    }
    return { proc.localViewAngles.x, proc.localViewAngles.y };
}

LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param))
    {
        return 0L;
    }

    if (message == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0L;
    }

    return DefWindowProc(window, message, w_param, l_param);
}


INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE, PSTR, INT cmd_show)
{
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = window_procedure;
    wc.hInstance = instance;
    wc.lpszClassName = L"Wall Hack";

    RegisterClassExW(&wc);

    const HWND window = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
        wc.lpszClassName,
        L"Wall Hack",
        WS_POPUP,
        0,
        0,
        1920, 1080,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr
    );

    SetLayeredWindowAttributes(window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

    {
        RECT client_area{};
        GetClientRect(window, &client_area);

        RECT window_area{};
        GetWindowRect(window, &window_area);

        POINT diff{};
        ClientToScreen(window, &diff);

        const MARGINS margins
        {
            window_area.left + (diff.x - window_area.left),
            window_area.top + (diff.y - window_area.top),
            client_area.right,
            client_area.bottom
        };

        DwmExtendFrameIntoClientArea(window, &margins);
    }

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferDesc.RefreshRate.Numerator = 60U;
    sd.BufferDesc.RefreshRate.Denominator = 60U;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1U;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2U;
    sd.OutputWindow = window;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    constexpr D3D_FEATURE_LEVEL levels[2]
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };

    ID3D11Device* device{ nullptr };
    ID3D11DeviceContext* device_context{ nullptr };
    IDXGISwapChain* swap_chain{ nullptr };
    ID3D11RenderTargetView* render_target_view{ nullptr };
    D3D_FEATURE_LEVEL level{};

    D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0U,
        levels,
        2U,
        D3D11_SDK_VERSION,
        &sd,
        &swap_chain,
        &device,
        &level,
        &device_context
    );

    ID3D11Texture2D* back_buffer{ nullptr };
    swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer));

    if (back_buffer)
    {
        device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view);
        back_buffer->Release();
    }
    else
    {
        return 1;
    }

    ShowWindow(window, cmd_show);
    UpdateWindow(window);

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(device, device_context);

    bool running = true;

    bool bBhop = false, bAntiFlash = false, bTriggerBot = false, bNoRecoil = false, bWallHack = false,
        bAimBot = false;

    while (running)
    {
        MSG msg;

        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                running = false;
        }

        if (!running)
        {
            break;
        }


        getGameAddresses();
        Vector3 localOrigin;
        ProcessMgr.ReadMemory<Vector3>(proc.localPlayer + offset.vecOrigin, localOrigin);
        view_matrix_t view_matrix;
        ProcessMgr.ReadMemory<view_matrix_t>(proc.moduleBase + offset.viewMatrix, view_matrix);
        ProcessMgr.ReadMemory<Vector3>(proc.moduleBase + offset.viewAngles, proc.localViewAngles);

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();

        ImGui::NewFrame();


        if (GetAsyncKeyState(VK_NUMPAD1) & 1)
        {
            bBhop = !bBhop;
        }


        if (GetAsyncKeyState(VK_NUMPAD2) & 1)
        {
            bAntiFlash = !bAntiFlash;
        }

        if (GetAsyncKeyState(VK_NUMPAD3) & 1)
        {
            bTriggerBot = !bTriggerBot;
        }

        if (GetAsyncKeyState(VK_NUMPAD4) & 1)
        {
            bNoRecoil = !bNoRecoil;
        }

        if (GetAsyncKeyState(VK_NUMPAD5) & 1)
        {
            bWallHack = !bWallHack;
        }

        if (GetAsyncKeyState(VK_NUMPAD6) & 1)
        {
            bAimBot = !bAimBot;
        }


        //Cheat Function calls

        if (bAntiFlash)
        {
            antiFlashHandler();
        }

        if (bBhop)
        {
            bHophandler();
        }

        if (bTriggerBot)
        {
            handleTBot();
        }

        if (bNoRecoil)
        {
            Vector2 recoilAngles = noRecoil(proc.localViewAngles);

            ProcessMgr.WriteMemory<Vector2>(proc.moduleBase + offset.viewAngles, recoilAngles);
        }

        //rendering goes here
        if (bWallHack || bAimBot)
        {
            float closestDistance = 9999999999;
            DWORD64 closestEnemy = NULL;
            Vector3 enemyHead;

            DWORD64 locGameScene;
            ProcessMgr.ReadMemory<DWORD64>(proc.localPlayer + offset.gameScene, locGameScene);
            DWORD64 locBoneArray;
            ProcessMgr.ReadMemory<DWORD64>(locGameScene + 0x160 + 0x80, locBoneArray);

            ProcessMgr.ReadMemory<Vector3>(locBoneArray + bones::head * 32, proc.localPlayerHeadPos);

            float closestHeight = 0;
            float closestAngle = 0;

            ProcessMgr.ReadMemory<Vector3>(proc.moduleBase + offset.viewAngles, proc.localViewAngles);

            for (int i = 1; i < 32; i++)
            {
                RGB entityColor = { 255, 0, 0 };
                DWORD64 listEntry;
                ProcessMgr.ReadMemory<DWORD64>(proc.entityList + (8 * (i & 0x7FFF) >> 9) + 16, listEntry);

                if (!listEntry)
                {
                    continue;
                }

                DWORD64 ent;
                ProcessMgr.ReadMemory<DWORD64>(listEntry + 120 * (i & 0x1FF), ent);

                if (!ent)
                {
                    continue;
                }

                int entTeam;
                ProcessMgr.ReadMemory<int>(ent + offset.team, entTeam);


                if (proc.localTeam == entTeam)
                {
                    entityColor = { 0, 0, 255 };
                }

                DWORD64 entPawn;
                ProcessMgr.ReadMemory<DWORD64>(ent + offset.playerPawn, entPawn);

                DWORD64 listEntry2;
                ProcessMgr.ReadMemory<DWORD64>(proc.entityList + 0x8 * ((entPawn & 0x7FFF) >> 9) + 16, listEntry2);

                if (!listEntry2)
                {
                    continue;
                }

                DWORD64 pCSPlayerPawn;
                ProcessMgr.ReadMemory<DWORD64>(listEntry2 + 120 * (entPawn & 0x1FF), pCSPlayerPawn);

                if (!pCSPlayerPawn)
                {
                    continue;
                }

                int eHealth;
                ProcessMgr.ReadMemory<int>(pCSPlayerPawn + offset.health, eHealth);

                if (eHealth <= 0)
                {
                    continue;
                }

                if (pCSPlayerPawn == proc.localPlayer)
                {
                    continue;
                }

                ProcessMgr.ReadMemory<DWORD64>(proc.moduleBase + offset.locPlayerController, proc.localController);

                int localPlayerIndex = findLocalPlayerInex();

                int mask;
                ProcessMgr.ReadMemory<int>(pCSPlayerPawn + offset.entitySpottedState + offset.spottedByMask, mask);
                bool isSpoted = mask & (DWORD(1) << (localPlayerIndex - 1));

                int localMask;
                ProcessMgr.ReadMemory<int>(proc.localPlayer + offset.entitySpottedState + offset.spottedByMask, localMask);
                bool isLocalSpoted = localMask & (DWORD(1) << (i - 1));

                DWORD64 gameScene;
                ProcessMgr.ReadMemory<DWORD64>(pCSPlayerPawn + offset.gameScene, gameScene);
                DWORD64 boneArray;
                ProcessMgr.ReadMemory<DWORD64>(gameScene + 0x160 + 0x80, boneArray);

                Vector3 eOrigin;
                ProcessMgr.ReadMemory<Vector3>(pCSPlayerPawn + offset.vecOrigin, eOrigin);

                Vector3 eHead;
                ProcessMgr.ReadMemory<Vector3>(boneArray + bones::head * 32, eHead);

                Vector3 screenPos = eOrigin.WTS(view_matrix);
                Vector3 screenHead = eHead.WTS(view_matrix);
                float headHeight = (screenPos.y - screenHead.y) / 8;
                float headWidth = (screenPos.x - screenHead.x);



                float height = screenPos.y - screenHead.y;
                float width = height / 2.32f;


                RGB entityHealth = { 2.55 * (100 - eHealth), 2.55 * (eHealth), 0 };

                RGB white(255, 255, 255);

                if (bAimBot)
                {
                    if (checkTheEnt(entTeam, eHead, isSpoted, isLocalSpoted, height / 2,
                        closestAngle, 7500, 30))
                    {
                        Vector3 newAngles = calculateAngles(eHead, height / 2);
                        closestAngle = sqrt(pow(newAngles.x - proc.localViewAngles.x, 2) + pow(newAngles.y - proc.localViewAngles.y, 2));
                        closestEnemy = pCSPlayerPawn;
                        enemyHead = eHead;
                        closestHeight = height / 2;
                    }
                }

                if (bWallHack)
                {
                    Render::DrawRect(
                        screenHead.x - width / 2,
                        screenHead.y - headHeight,
                        width,
                        height + headHeight * 2,
                        entityColor,
                        2.5,
                        false,
                        255
                    );

                    Render::DrawHealth(
                        screenHead.x - width / 2,
                        screenHead.y,
                        height,
                        entityHealth,
                        3.5,
                        eHealth,
                        255
                    );

                    Render::DrawCircle(
                        screenHead.x,
                        screenHead.y,
                        headHeight - 3,
                        white,
                        1.5,
                        false,
                        255
                    );

                    for (int i = 0; i < sizeof(boneConnections) / sizeof(boneConnections[0]); i++)
                    {
                        int bone1 = boneConnections[i].bone1;
                        int bone2 = boneConnections[i].bone2;

                        Vector3 vecBone, vecBone2;
                        ProcessMgr.ReadMemory<Vector3>(boneArray + bone1 * 32, vecBone);
                        ProcessMgr.ReadMemory<Vector3>(boneArray + bone2 * 32, vecBone2);

                        Vector3 b1 = vecBone.WTS(view_matrix);
                        Vector3 b2 = vecBone2.WTS(view_matrix);

                        Render::DrawLine(b1.x, b1.y, b2.x, b2.y, white, 1.5, 255);
                    }
                }
            }

            int locShotsFired;
            ProcessMgr.ReadMemory<int>(proc.localPlayer + offset.shotsFired, locShotsFired);

            if (locShotsFired == 0 || (GetKeyState(VK_LBUTTON) & 0x8000) != 0)
            {
                if (closestEnemy != NULL)
                {
                    Vector3 aimBotAngles = calculateAngles(enemyHead, closestHeight);
                    //ProcessMgr.WriteMemory<Vector3>(proc.moduleBase + offset.viewAngles, aimBotAngles);
                    if (locShotsFired > 1)
                    {
                        Vector2 controlRecoil = noRecoil(proc.localViewAngles);

                        aimBotAngles.x = controlRecoil.x;
                        aimBotAngles.y = controlRecoil.y;
                    }
                    smoothAim(aimBotAngles);
                }
            }
        }

        /*
            Next Steps:
            Make Dinamic Width;
            make a menu allowing to turn off wh and bones;
            add other hacks
        */

        ImGui::Render();

        constexpr float color[4]{ 0.f, 0.f, 0.f, 0.f };
        device_context->OMSetRenderTargets(1U, &render_target_view, nullptr);
        device_context->ClearRenderTargetView(render_target_view, color);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        swap_chain->Present(0U, 0U);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();

    ImGui::DestroyContext();

    if (swap_chain)
    {
        swap_chain->Release();
    }

    if (device_context)
    {
        device_context->Release();
    }
    if (device)
    {
        device->Release();
    }
    if (render_target_view)
    {
        render_target_view->Release();
    }

    DestroyWindow(window);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}