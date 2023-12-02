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

int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);

struct vec3
{
    float x, y, z;
};

struct ClrRender
{
    BYTE red, green, blue;
};

struct GlowStruct
{
    BYTE base[4];
    float red, green, blue, alpha;
    BYTE buffer[16];
    bool renderWhenIncluded, renderWhenUnIncluded, fullBloom;
    BYTE buffer1[5];
    int glowStyle;
}Glow;

ClrRender clrTeam;
ClrRender clrEnemy;

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
        ProcessMgr.ReadMemory<DWORD64>(proc.moduleBase + offset.locPlayerController, proc.localPlayer);
    } while (proc.localPlayer == NULL);

    ProcessMgr.ReadMemory<DWORD64>(proc.moduleBase + offset.entList, proc.entityList);

    ProcessMgr.ReadMemory<DWORD64>(proc.localPlayer + offset.playerPawn, proc.localPawn);

    ProcessMgr.ReadMemory<DWORD64>(proc.entityList + 0x8 * ((proc.localPawn & 0x7FFF) >> 9) + 16, proc.listEntry);

    ProcessMgr.ReadMemory<DWORD64>(proc.listEntry + 120 * (proc.localPawn & 0x1FF), proc.localPlayer);

    ProcessMgr.ReadMemory<int>(proc.localPlayer + offset.team, proc.localTeam);
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

    bool bBhop = false, bAntiFlash = false, bTriggerBot = false, bWallHack = false;

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

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();

        ImGui::NewFrame();


        if (GetAsyncKeyState(VK_NUMPAD1) & 1)
        {
            bBhop = !bBhop;

            // writeCheatstoConsole(bBhop, bAntiFlash, bTriggerBot, bWallHack);
        }


        if (GetAsyncKeyState(VK_NUMPAD2) & 1)
        {
            bAntiFlash = !bAntiFlash;

            //writeCheatstoConsole(bBhop, bAntiFlash, bTriggerBot, bWallHack);
        }

        if (GetAsyncKeyState(VK_NUMPAD3) & 1)
        {
            bTriggerBot = !bTriggerBot;

            // writeCheatstoConsole(bBhop, bAntiFlash, bTriggerBot, bWallHack);
        }

        if (GetAsyncKeyState(VK_NUMPAD5) & 1)
        {
            bWallHack = !bWallHack;

            //writeCheatstoConsole(bBhop, bAntiFlash, bTriggerBot, bWallHack);
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

        //rendering goes here
        if (bWallHack)
        {
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

                Vector3 eLeftArm;
                ProcessMgr.ReadMemory<Vector3>(boneArray + bones::left_arm * 32, eLeftArm);

                Vector3 eRightArm;
                ProcessMgr.ReadMemory<Vector3>(boneArray + bones::right_arm * 32, eRightArm);

                Vector3 screenLeftArm = eLeftArm.WTS(view_matrix);
                Vector3 screenRightArm = eRightArm.WTS(view_matrix);

                float boneWidth = (screenRightArm.x - screenLeftArm.x);

                float height = screenPos.y - screenHead.y;
                float width = height / 2.4f;


                RGB entityHealth = { 2.55 * (100 - eHealth), 2.55 * (eHealth), 0 };

                RGB white(255, 255, 255);


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
                /*
                Render::DrawRect(
                    screenLeftArm.x,
                    screenHead.y - headHeight,
                    boneWidth,
                    height + headHeight * 2,
                    entityColor,
                    2.5,
                    true,
                    32
                );*/

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

