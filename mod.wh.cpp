// ==WindhawkMod==
// @id              windraw
// @name            WinDraw - Screen Inking & Annotation
// @description     All-in-one hardware-accelerated screen drawing, shapes, radial quick menu, floating toolbar, and screenshot tool for Windows.
// @version         1.1.0
// @author          Zain
// @github          https://github.com/ZainYoussef/WinDraw
// @include         explorer.exe
// @compilerOptions -ld2d1 -ldwrite -lole32 -luser32 -lgdi32 -ldwmapi -lcomctl32 -lshlwapi -lwindowscodecs
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# WinDraw - Screen Inking & Annotation

A complete, zero-bloat, hardware-accelerated screen annotation and drawing suite running directly inside `explorer.exe` powered by **Direct2D**, **DirectWrite**, and **Windows Imaging Component (WIC)**.

### Complete Feature Set:
1. **Activation & Dismissal**:
   - Press **`Ctrl + Alt + G`** (customizable) anywhere in Windows to begin annotating immediately.
   - Press **`ESC`** or click the **`✕`** Exit button to dismiss overlay.

2. **Hardware-Accelerated Inking**:
   - 144Hz+ butter-smooth Direct2D drawing with quadratic Bézier curve interpolation.
   - Highlighter mode with alpha-blending transparency.
   - Mouse wheel dynamically resizes pen thickness with a visual circular indicator.

3. **Shapes & Geometry**:
   - **Freehand** inking (`F` key).
   - **Straight Line** (`L` key).
   - **Arrow** (`A` key) with automatically oriented arrowheads.
   - **Rectangle / Box** (`R` key).
   - **Ellipse / Circle** (`O` key).
   - Hold `Shift` during drawing to snap lines or constrain shapes.

4. **Right-Click Eraser & Radial Menu**:
   - **Hold & Move Right Mouse Button**: Instant stroke-level eraser with circular radius indicator.
   - **Quick Right-Click Tap**: Opens a floating **Circular Radial Menu** with a 360-degree color wheel and quick tools.

5. **Pan & Zoom Canvas Navigation**:
   - **Pan Mode** (`P` key): Drags and offsets all annotations across the screen.
   - **Canvas Zoom**: While holding the canvas or in Pan mode, scroll the mouse wheel to dynamically zoom in and out centered on the cursor (15% - 800%).
   - **Reset View**: Press `0` or `Ctrl + 0` to instantly reset zoom (100%) and pan offset (0, 0).

6. **Pointer / Click-Through Mode**:
   - **Pointer Mode** (`M` key): Allows clicking through directly to desktop apps/games while keeping drawings visible.

7. **Ink Visibility Toggle**:
   - **Eye Toggle** (`V` key): Hides or restores all ink without clearing strokes.

8. **History Stack**:
   - **Undo** (`Ctrl + Z`).
   - **Redo** (`Ctrl + Y`).
   - **Clear All** (`C` key) with undo capability.

9. **Draggable Windows 11 Fluent Toolbar**:
   - 5px rounded corners, specular top rim, and subtle group dividers.
   - Drag the toolbar by its handle or background to reposition anywhere.
   - Quick pen swatches, tool toggles, shape selectors, and action buttons.

10. **Snapshot & Clipboard**:
    - **Snapshot** (`S` or `Ctrl + S`): Copies annotated desktop directly to Windows clipboard (`CF_BITMAP`) and saves to `%USERPROFILE%\Pictures\WinDraw\`.
    - Floating toast badge confirms capture.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- hotkeyMod: 3
  $name: Hotkey Modifiers
  $description: 1=Alt, 2=Ctrl, 3=Ctrl+Alt (default), 4=Shift, 8=Win
- hotkeyKey: 0x47
  $name: Hotkey Virtual Key Code
  $description: Virtual key code for activation (0x47 = 'G', 0x44 = 'D')
- defaultPenWidth: 3.5
  $name: Default Pen Width
  $description: Drawing thickness in pixels
- defaultHighlighterWidth: 18.0
  $name: Default Highlighter Width
  $description: Highlighter thickness in pixels
- showBottomToolbar: true
  $name: Show Bottom Toolbar
  $description: Display floating compact toolbar on the overlay
- cornerRadius: 5
  $name: Toolbar Corner Radius
  $description: Corner radius for Windows 11 Fluent look (default 5px)
- autoSaveSnapshot: true
  $name: Auto-save Snapshot to Pictures/WinDraw
  $description: Automatically save PNG file in addition to copying to clipboard
- freezeScreen: false
  $name: Freeze Screen on Activation
  $description: When enabled, captures a static desktop screenshot so the background is frozen. When disabled (default), the overlay is completely live and transparent so running videos, animations, and apps keep running in real-time.
*/
// ==/WindhawkModSettings==

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <windowsx.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <dwmapi.h>
#include <wincodec.h>
#include <shlobj.h>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "windowscodecs.lib")

// ----------------------------------------------------------------------------
// Configuration & Settings
// ----------------------------------------------------------------------------

struct ModSettings {
    UINT hotkeyMod;
    UINT hotkeyKey;
    float defaultPenWidth;
    float defaultHighlighterWidth;
    bool showBottomToolbar;
    int cornerRadius;
    bool autoSaveSnapshot;
    bool freezeScreen;
} g_settings;

void LoadSettings() {
    g_settings.hotkeyMod = (UINT)Wh_GetIntSetting(L"hotkeyMod");
    if (g_settings.hotkeyMod == 0) g_settings.hotkeyMod = MOD_CONTROL | MOD_ALT;

    g_settings.hotkeyKey = (UINT)Wh_GetIntSetting(L"hotkeyKey");
    if (g_settings.hotkeyKey == 0) g_settings.hotkeyKey = 'G';

    g_settings.defaultPenWidth = (float)Wh_GetIntSetting(L"defaultPenWidth");
    if (g_settings.defaultPenWidth <= 0.5f) g_settings.defaultPenWidth = 3.5f;

    g_settings.defaultHighlighterWidth = (float)Wh_GetIntSetting(L"defaultHighlighterWidth");
    if (g_settings.defaultHighlighterWidth <= 1.0f) g_settings.defaultHighlighterWidth = 18.0f;

    g_settings.showBottomToolbar = Wh_GetIntSetting(L"showBottomToolbar") != 0;
    g_settings.cornerRadius = Wh_GetIntSetting(L"cornerRadius");
    if (g_settings.cornerRadius <= 0) g_settings.cornerRadius = 5;

    g_settings.autoSaveSnapshot = Wh_GetIntSetting(L"autoSaveSnapshot") != 0;
    g_settings.freezeScreen = Wh_GetIntSetting(L"freezeScreen") != 0;
}

// ----------------------------------------------------------------------------
// Geometry & Stroke Model
// ----------------------------------------------------------------------------

enum class ShapeType {
    Freehand,
    Line,
    Arrow,
    Rectangle,
    Ellipse
};

enum class ToolMode {
    Pen,
    Highlighter,
    Eraser,
    Pan,
    Pointer
};

struct StrokePoint {
    float x;
    float y;
};

struct Stroke {
    std::vector<StrokePoint> points;
    D2D1_COLOR_F color;
    float width;
    bool isHighlighter;
    ShapeType shapeType;
    StrokePoint startPt;
    StrokePoint endPt;
};

// ----------------------------------------------------------------------------
// 360-Degree Orbital & Preset Color Palette
// ----------------------------------------------------------------------------

static const D2D1_COLOR_F kPresetColors[] = {
    D2D1::ColorF(0.92f, 0.22f, 0.22f, 1.0f), // 0. Crimson Red
    D2D1::ColorF(0.96f, 0.42f, 0.15f, 1.0f), // 1. Tangelo Orange
    D2D1::ColorF(0.98f, 0.65f, 0.12f, 1.0f), // 2. Amber Gold
    D2D1::ColorF(0.98f, 0.82f, 0.15f, 1.0f), // 3. Sun Yellow
    D2D1::ColorF(0.65f, 0.85f, 0.18f, 1.0f), // 4. Lime
    D2D1::ColorF(0.20f, 0.78f, 0.35f, 1.0f), // 5. Emerald Green
    D2D1::ColorF(0.12f, 0.82f, 0.70f, 1.0f), // 6. Teal Mint
    D2D1::ColorF(0.15f, 0.80f, 0.95f, 1.0f), // 7. Electric Cyan
    D2D1::ColorF(0.18f, 0.52f, 0.95f, 1.0f), // 8. Cobalt Blue
    D2D1::ColorF(0.38f, 0.35f, 0.95f, 1.0f), // 9. Indigo
    D2D1::ColorF(0.65f, 0.28f, 0.92f, 1.0f), // 10. Violet Purple
    D2D1::ColorF(0.92f, 0.25f, 0.75f, 1.0f), // 11. Magenta Pink
    D2D1::ColorF(0.96f, 0.32f, 0.52f, 1.0f), // 12. Rose Coral
    D2D1::ColorF(0.95f, 0.96f, 0.98f, 1.0f), // 13. Titanium White
    D2D1::ColorF(0.55f, 0.58f, 0.64f, 1.0f), // 14. Silver Gray
    D2D1::ColorF(0.12f, 0.14f, 0.18f, 1.0f)  // 15. Charcoal Black
};

static const size_t kPresetColorCount = sizeof(kPresetColors) / sizeof(kPresetColors[0]);

// ----------------------------------------------------------------------------
// Application State
// ----------------------------------------------------------------------------

static HWND g_hOverlayWnd = NULL;
static HWND g_hHotkeyWnd = NULL;
static bool g_bIsActive = false;

static ID2D1Factory* g_pD2DFactory = NULL;
static ID2D1HwndRenderTarget* g_pRenderTarget = NULL;
static ID2D1StrokeStyle* g_pRoundStrokeStyle = NULL;
static ID2D1Bitmap* g_pDesktopBitmap = NULL;
static IDWriteFactory* g_pDWriteFactory = NULL;
static IDWriteTextFormat* g_pTextFormat = NULL;
static IDWriteTextFormat* g_pIconFormat = NULL;
static IDWriteTextFormat* g_pRadialIconFormat = NULL;
static IDWriteTextFormat* g_pCenterBadgeFormat = NULL;
static IWICImagingFactory* g_pWICFactory = NULL;

static std::vector<Stroke> g_strokes;
static std::vector<Stroke> g_redoStack;
static Stroke g_currentStroke;
static bool g_isDrawing = false;

static ToolMode g_currentTool = ToolMode::Pen;
static ShapeType g_currentShape = ShapeType::Freehand;
static D2D1_COLOR_F g_activeColor = kPresetColors[0];
static float g_currentPenWidth = 3.5f;
static bool g_inkVisible = true;

// Pan & Zoom state
static float g_panOffsetX = 0.0f;
static float g_panOffsetY = 0.0f;
static float g_zoomScale = 1.0f;
static bool g_isPanning = false;
static POINT g_panStartPos = { 0, 0 };

// Right-click eraser tracking
static bool g_isRightMouseDown = false;
static bool g_isRightClickErasing = false;
static bool g_wheelUsedWhileRightMouseDown = false;
static bool g_isLeftClickErasing = false;
static bool g_isRightClickClearing = false;
static POINT g_rightMouseDownPos = { 0, 0 };
static ULONGLONG g_rightMouseDownTime = 0;
static float g_cursorX = 0;
static float g_cursorY = 0;
static float g_eraserRadius = 24.0f;

// Radial Menu State
enum class RadialTarget {
    None = -1,
    Clear = 0,       // 0° (East): Clear All Ink
    Snapshot = 1,    // 45° (SE): Snapshot
    Eraser = 2,      // 90° (South): Eraser
    Undo = 3,        // 135° (SW): Undo
    Pointer = 4,     // 180° (West): Pointer
    InkVisible = 5,  // 225° (NW): Ink Visible
    Pan = 6,         // 270° (North): Pan
    Draw = 7,        // 315° (NE): Draw
    Center = 8,      // Center Hub
    ColorOrb = 9     // Outer orbital colors
};
static bool g_radialActive = false;
static float g_radialX = 0;
static float g_radialY = 0;
static RadialTarget g_radialHoverTarget = RadialTarget::None;
static int g_radialHoverSector = -1;
static int g_hoveredOrb = -1;

// Toolbar State & Dragging
struct ToolbarButton {
    int id;
    D2D1_RECT_F rect;
    std::wstring label;
    bool isPen;
    D2D1_COLOR_F penColor;
    bool isToggled;
};
static std::vector<ToolbarButton> g_toolbarButtons;
static std::vector<float> g_toolbarDividers;
static D2D1_RECT_F g_toolbarRect = { 0, 0, 0, 0 };
static int g_hoveredToolbarBtn = -1;
static bool g_isDraggingToolbar = false;
static POINT g_toolbarDragStart = { 0, 0 };
static float g_toolbarCustomX = -1.0f;
static float g_toolbarCustomY = -1.0f;

// Toast feedback
static ULONGLONG g_toastStartTime = 0;
static std::wstring g_toastMessage = L"";

// Brush & Zoom size preview timer
static ULONGLONG g_sizePreviewTime = 0;
static ULONGLONG g_zoomPreviewTime = 0;

#define WM_USER_TOGGLE_POINTER (WM_USER + 101)

// ----------------------------------------------------------------------------
// Forward Declarations
// ----------------------------------------------------------------------------

void ShowOverlay();
void HideOverlay();
void SetToolMode(ToolMode newMode);
void CaptureDesktop();
void ReleaseD2DResources();
void InvalidateOverlay();
void RenderOverlay();
void DrawZoomPreview(ID2D1HwndRenderTarget* pRT);
void CopySnapshotToClipboard();
void EraseBrushAt(float x, float y, float radius);
bool EraseWholeShapeAt(float x, float y, float radius);
void SaveBitmapToPNG(HBITMAP hBitmap, const std::wstring& filePath);

// ----------------------------------------------------------------------------
// Utility Math & Geometry
// ----------------------------------------------------------------------------

static const wchar_t* GetIconFontFamilyName() {
    static const wchar_t* s_fontName = nullptr;
    if (s_fontName) return s_fontName;

    if (g_pDWriteFactory) {
        IDWriteFontCollection* pFontCollection = nullptr;
        if (SUCCEEDED(g_pDWriteFactory->GetSystemFontCollection(&pFontCollection, FALSE)) && pFontCollection) {
            UINT32 index = 0;
            BOOL exists = FALSE;
            if (SUCCEEDED(pFontCollection->FindFamilyName(L"Segoe Fluent Icons", &index, &exists)) && exists) {
                s_fontName = L"Segoe Fluent Icons";
            } else {
                s_fontName = L"Segoe MDL2 Assets";
            }
            pFontCollection->Release();
            return s_fontName;
        }
    }
    s_fontName = L"Segoe Fluent Icons";
    return s_fontName;
}

static float DistanceSq(float x1, float y1, float x2, float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return dx * dx + dy * dy;
}

static float DistToSegmentSq(float px, float py, float x1, float y1, float x2, float y2) {
    float l2 = DistanceSq(x1, y1, x2, y2);
    if (l2 == 0.0f) return DistanceSq(px, py, x1, y1);
    float t = ((px - x1) * (x2 - x1) + (py - y1) * (y2 - y1)) / l2;
    t = std::max(0.0f, std::min(1.0f, t));
    return DistanceSq(px, py, x1 + t * (x2 - x1), y1 + t * (y2 - y1));
}

// ----------------------------------------------------------------------------
// Desktop Screen Capture
// ----------------------------------------------------------------------------

void CaptureDesktop() {
    if (!g_settings.freezeScreen) {
        if (g_pDesktopBitmap) {
            g_pDesktopBitmap->Release();
            g_pDesktopBitmap = nullptr;
        }
        return;
    }
    if (!g_pRenderTarget) return;

    if (g_pDesktopBitmap) {
        g_pDesktopBitmap->Release();
        g_pDesktopBitmap = nullptr;
    }

    int vx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int vy = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    HDC hScreenDC = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hScreenDC);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = vw;
    bmi.bmiHeader.biHeight = -vh; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(hMemDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

    BitBlt(hMemDC, 0, 0, vw, vh, hScreenDC, vx, vy, SRCCOPY | CAPTUREBLT);

    D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)
    );

    g_pRenderTarget->CreateBitmap(
        D2D1::SizeU(vw, vh),
        pBits,
        vw * 4,
        props,
        &g_pDesktopBitmap
    );

    SelectObject(hMemDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
    ReleaseDC(NULL, hScreenDC);
}

// ----------------------------------------------------------------------------
// Direct2D Resource Management
// ----------------------------------------------------------------------------

HRESULT CreateD2DResources(HWND hwnd) {
    if (g_pRenderTarget) return S_OK;

    RECT rc;
    GetClientRect(hwnd, &rc);
    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    HRESULT hr = g_pD2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
        ),
        D2D1::HwndRenderTargetProperties(hwnd, size, D2D1_PRESENT_OPTIONS_IMMEDIATELY),
        &g_pRenderTarget
    );

    if (SUCCEEDED(hr)) {
        g_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

        MARGINS margins = { -1, -1, -1, -1 };
        DwmExtendFrameIntoClientArea(hwnd, &margins);

        if (!g_pRoundStrokeStyle) {
            g_pD2DFactory->CreateStrokeStyle(
                D2D1::StrokeStyleProperties(
                    D2D1_CAP_STYLE_ROUND,
                    D2D1_CAP_STYLE_ROUND,
                    D2D1_CAP_STYLE_ROUND,
                    D2D1_LINE_JOIN_ROUND
                ),
                nullptr, 0,
                &g_pRoundStrokeStyle
            );
        }

        CaptureDesktop();
    }

    return hr;
}

void ReleaseD2DResources() {
    if (g_pDesktopBitmap) { g_pDesktopBitmap->Release(); g_pDesktopBitmap = nullptr; }
    if (g_pRoundStrokeStyle) { g_pRoundStrokeStyle->Release(); g_pRoundStrokeStyle = nullptr; }
    if (g_pRenderTarget) { g_pRenderTarget->Release(); g_pRenderTarget = nullptr; }
}

void InvalidateOverlay() {
    if (g_hOverlayWnd) {
        InvalidateRect(g_hOverlayWnd, NULL, FALSE);
    }
}

// ----------------------------------------------------------------------------
// Layout Setup: Compact Windows 11 Bottom Toolbar
// ----------------------------------------------------------------------------

void BuildToolbarLayout(int screenW, int screenH) {
    g_toolbarButtons.clear();
    g_toolbarDividers.clear();

    const float btnH = 34.0f;
    const float btnW = 34.0f;
    const float padY = 6.0f;
    const float barH = btnH + padY * 2.0f;
    const float itemGap = 3.0f;
    const float dividerGap = 10.0f;

    float curX = 6.0f;

    // Handle / Dock fold indicator
    ToolbarButton dockBtn;
    dockBtn.id = 0;
    dockBtn.isPen = false;
    dockBtn.label = L"\uE75E"; // GripperTool
    dockBtn.rect = D2D1::RectF(curX, padY, curX + 20.0f, padY + btnH);
    g_toolbarButtons.push_back(dockBtn);
    curX += 20.0f + itemGap;

    // Group 1: Pens (5 Preset Swatches)
    for (int i = 0; i < 5; ++i) {
        ToolbarButton btn;
        btn.id = 100 + i;
        btn.isPen = true;
        btn.penColor = kPresetColors[i];
        btn.rect = D2D1::RectF(curX, padY, curX + btnW, padY + btnH);
        g_toolbarButtons.push_back(btn);
        curX += btnW + itemGap;
    }

    // Divider 1
    curX -= itemGap;
    g_toolbarDividers.push_back(curX + dividerGap * 0.5f);
    curX += dividerGap;

    // Group 2: Shapes (Freehand, Line, Arrow, Rect, Ellipse)
    int shapeIds[] = { 20, 21, 22, 23, 24 };
    const wchar_t* shapeLabels[] = {
        L"\uEC87", // Freehand (Draw)
        L"\uED5E", // Line (Ruler)
        L"\uE72A", // Arrow (Forward)
        L"\uE739", // Rect (Checkbox)
        L"\uEA3A"  // Ellipse (CircleRing)
    };
    for (int i = 0; i < 5; ++i) {
        ToolbarButton btn;
        btn.id = shapeIds[i];
        btn.isPen = false;
        btn.label = shapeLabels[i];
        btn.rect = D2D1::RectF(curX, padY, curX + btnW, padY + btnH);
        g_toolbarButtons.push_back(btn);
        curX += btnW + itemGap;
    }

    // Divider 2
    curX -= itemGap;
    g_toolbarDividers.push_back(curX + dividerGap * 0.5f);
    curX += dividerGap;

    // Group 3: Navigation Tools (Highlighter, Eraser, Pan, Pointer, Eye)
    int toolIds[] = { 1, 2, 3, 4, 5 };
    const wchar_t* toolLabels[] = {
        L"\uE7E6", // Highlighter (Highlight)
        L"\uE75C", // Eraser (EraseTool)
        L"\uE7C2", // Pan (Move - 4-way arrows)
        L"\uE7C9", // Pointer (TouchPointer)
        L"\uE890"  // Eye (View)
    };
    for (int i = 0; i < 5; ++i) {
        ToolbarButton btn;
        btn.id = toolIds[i];
        btn.isPen = false;
        btn.label = toolLabels[i];
        btn.rect = D2D1::RectF(curX, padY, curX + btnW, padY + btnH);
        g_toolbarButtons.push_back(btn);
        curX += btnW + itemGap;
    }

    // Divider 3
    curX -= itemGap;
    g_toolbarDividers.push_back(curX + dividerGap * 0.5f);
    curX += dividerGap;

    // Group 4: Snapshot, Undo, Redo, Clear
    int actionIds[] = { 10, 11, 12, 13 };
    const wchar_t* actionLabels[] = {
        L"\uE722", // Snapshot (Camera)
        L"\uE7A7", // Undo
        L"\uE7A6", // Redo
        L"\uE74D"  // Clear (Delete)
    };
    for (int i = 0; i < 4; ++i) {
        ToolbarButton btn;
        btn.id = actionIds[i];
        btn.isPen = false;
        btn.label = actionLabels[i];
        btn.rect = D2D1::RectF(curX, padY, curX + btnW, padY + btnH);
        g_toolbarButtons.push_back(btn);
        curX += btnW + itemGap;
    }

    // Divider 4
    curX -= itemGap;
    g_toolbarDividers.push_back(curX + dividerGap * 0.5f);
    curX += dividerGap;

    // Group 5: Exit
    ToolbarButton exitBtn;
    exitBtn.id = 99;
    exitBtn.isPen = false;
    exitBtn.label = L"\uE8BB"; // Exit (ChromeClose)
    exitBtn.rect = D2D1::RectF(curX, padY, curX + btnW, padY + btnH);
    g_toolbarButtons.push_back(exitBtn);
    curX += btnW + 6.0f;

    float barW = curX;

    // Default placement: centered at the bottom of the Primary / Main Screen
    float startX = g_toolbarCustomX;
    float startY = g_toolbarCustomY;

    if (startX < 0.0f || startY < 0.0f) {
        int vx = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int vy = GetSystemMetrics(SM_YVIRTUALSCREEN);

        HMONITOR hPrimaryMon = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO mi = { sizeof(MONITORINFO) };
        if (hPrimaryMon && GetMonitorInfo(hPrimaryMon, &mi)) {
            float clientLeft = (float)(mi.rcMonitor.left - vx);
            float clientTop = (float)(mi.rcMonitor.top - vy);
            float monW = (float)(mi.rcMonitor.right - mi.rcMonitor.left);
            float monH = (float)(mi.rcMonitor.bottom - mi.rcMonitor.top);

            if (startX < 0.0f) {
                startX = clientLeft + (monW - barW) * 0.5f;
            }
            if (startY < 0.0f) {
                float workBottom = (float)(mi.rcWork.bottom - vy);
                startY = workBottom - barH - 16.0f;
                if (startY + barH > clientTop + monH - 8.0f) {
                    startY = clientTop + monH - barH - 8.0f;
                }
            }
        }
        else {
            if (startX < 0.0f) startX = (screenW - barW) * 0.5f;
            if (startY < 0.0f) startY = (screenH - barH - 24.0f);
        }
    }

    g_toolbarRect = D2D1::RectF(startX, startY, startX + barW, startY + barH);

    // Position buttons to absolute coordinates
    for (auto& btn : g_toolbarButtons) {
        btn.rect.left += startX;
        btn.rect.right += startX;
        btn.rect.top += startY;
        btn.rect.bottom += startY;
    }
    for (auto& divX : g_toolbarDividers) {
        divX += startX;
    }
}

// ----------------------------------------------------------------------------
// Direct2D Stroke & Shape Rendering
// ----------------------------------------------------------------------------

void DrawArrowhead(ID2D1HwndRenderTarget* pRT, ID2D1SolidColorBrush* pBrush, float x1, float y1, float x2, float y2, float strokeW) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 4.0f) return;

    float ux = dx / len;
    float uy = dy / len;
    float arrowLen = std::max(12.0f, strokeW * 3.5f);
    float arrowW = arrowLen * 0.55f;

    float basePx = x2 - ux * arrowLen;
    float basePy = y2 - uy * arrowLen;

    float leftX = basePx - uy * arrowW;
    float leftY = basePy + ux * arrowW;
    float rightX = basePx + uy * arrowW;
    float rightY = basePy - ux * arrowW;

    ID2D1PathGeometry* pArrowGeo = nullptr;
    g_pD2DFactory->CreatePathGeometry(&pArrowGeo);
    if (pArrowGeo) {
        ID2D1GeometrySink* pSink = nullptr;
        if (SUCCEEDED(pArrowGeo->Open(&pSink))) {
            pSink->BeginFigure(D2D1::Point2F(x2, y2), D2D1_FIGURE_BEGIN_FILLED);
            pSink->AddLine(D2D1::Point2F(leftX, leftY));
            pSink->AddLine(D2D1::Point2F(rightX, rightY));
            pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
            pSink->Close();
            pSink->Release();

            pRT->FillGeometry(pArrowGeo, pBrush);
        }
        pArrowGeo->Release();
    }
}

void DrawSmoothStroke(ID2D1HwndRenderTarget* pRT, const Stroke& stroke) {
    if (!g_inkVisible) return;

    ID2D1SolidColorBrush* pBrush = nullptr;
    D2D1_COLOR_F c = stroke.color;
    if (stroke.isHighlighter) {
        c.a = 0.35f;
    }
    pRT->CreateSolidColorBrush(c, &pBrush);
    if (!pBrush) return;

    if (stroke.shapeType == ShapeType::Line) {
        pRT->DrawLine(
            D2D1::Point2F(stroke.startPt.x, stroke.startPt.y),
            D2D1::Point2F(stroke.endPt.x, stroke.endPt.y),
            pBrush, stroke.width, g_pRoundStrokeStyle
        );
    }
    else if (stroke.shapeType == ShapeType::Arrow) {
        pRT->DrawLine(
            D2D1::Point2F(stroke.startPt.x, stroke.startPt.y),
            D2D1::Point2F(stroke.endPt.x, stroke.endPt.y),
            pBrush, stroke.width, g_pRoundStrokeStyle
        );
        DrawArrowhead(pRT, pBrush, stroke.startPt.x, stroke.startPt.y, stroke.endPt.x, stroke.endPt.y, stroke.width);
    }
    else if (stroke.shapeType == ShapeType::Rectangle) {
        float minX = std::min(stroke.startPt.x, stroke.endPt.x);
        float maxX = std::max(stroke.startPt.x, stroke.endPt.x);
        float minY = std::min(stroke.startPt.y, stroke.endPt.y);
        float maxY = std::max(stroke.startPt.y, stroke.endPt.y);
        pRT->DrawRectangle(D2D1::RectF(minX, minY, maxX, maxY), pBrush, stroke.width);
    }
    else if (stroke.shapeType == ShapeType::Ellipse) {
        float cx = (stroke.startPt.x + stroke.endPt.x) * 0.5f;
        float cy = (stroke.startPt.y + stroke.endPt.y) * 0.5f;
        float rx = std::abs(stroke.endPt.x - stroke.startPt.x) * 0.5f;
        float ry = std::abs(stroke.endPt.y - stroke.startPt.y) * 0.5f;
        pRT->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), rx, ry), pBrush, stroke.width);
    }
    else {
        // Freehand Inking with Quadratic Bézier smoothing
        if (stroke.points.empty()) {
            pBrush->Release();
            return;
        }

        if (stroke.points.size() == 1) {
            float r = stroke.width * 0.5f;
            pRT->FillEllipse(D2D1::Ellipse(D2D1::Point2F(stroke.points[0].x, stroke.points[0].y), r, r), pBrush);
        }
        else {
            ID2D1PathGeometry* pGeometry = nullptr;
            g_pD2DFactory->CreatePathGeometry(&pGeometry);
            if (pGeometry) {
                ID2D1GeometrySink* pSink = nullptr;
                if (SUCCEEDED(pGeometry->Open(&pSink))) {
                    pSink->SetFillMode(D2D1_FILL_MODE_WINDING);
                    pSink->BeginFigure(
                        D2D1::Point2F(stroke.points[0].x, stroke.points[0].y),
                        D2D1_FIGURE_BEGIN_HOLLOW
                    );

                    if (stroke.points.size() == 2) {
                        pSink->AddLine(D2D1::Point2F(stroke.points[1].x, stroke.points[1].y));
                    }
                    else {
                        for (size_t i = 1; i < stroke.points.size() - 1; ++i) {
                            D2D1_POINT_2F midPoint = D2D1::Point2F(
                                (stroke.points[i].x + stroke.points[i + 1].x) * 0.5f,
                                (stroke.points[i].y + stroke.points[i + 1].y) * 0.5f
                            );
                            pSink->AddQuadraticBezier(D2D1::QuadraticBezierSegment(
                                D2D1::Point2F(stroke.points[i].x, stroke.points[i].y),
                                midPoint
                            ));
                        }
                        pSink->AddLine(D2D1::Point2F(stroke.points.back().x, stroke.points.back().y));
                    }

                    pSink->EndFigure(D2D1_FIGURE_END_OPEN);
                    pSink->Close();
                    pSink->Release();

                    pRT->DrawGeometry(pGeometry, pBrush, stroke.width, g_pRoundStrokeStyle);
                }
                pGeometry->Release();
            }
        }
    }

    pBrush->Release();
}

void DrawToolbar(ID2D1HwndRenderTarget* pRT) {
    if (!g_settings.showBottomToolbar) return;

    ID2D1SolidColorBrush* pBgBrush = nullptr;
    ID2D1SolidColorBrush* pBorderBrush = nullptr;
    ID2D1SolidColorBrush* pRimBrush = nullptr;
    ID2D1SolidColorBrush* pTextBrush = nullptr;

    pRT->CreateSolidColorBrush(D2D1::ColorF(0.08f, 0.10f, 0.14f, 0.94f), &pBgBrush);
    pRT->CreateSolidColorBrush(D2D1::ColorF(0.22f, 0.26f, 0.34f, 1.00f), &pBorderBrush);
    pRT->CreateSolidColorBrush(D2D1::ColorF(0.40f, 0.48f, 0.60f, 0.50f), &pRimBrush);
    pRT->CreateSolidColorBrush(D2D1::ColorF(0.85f, 0.88f, 0.92f, 1.00f), &pTextBrush);

    float r = (float)g_settings.cornerRadius;
    D2D1_ROUNDED_RECT roundRect = D2D1::RoundedRect(g_toolbarRect, r, r);

    // Chassis background & 1px border
    pRT->FillRoundedRectangle(roundRect, pBgBrush);
    pRT->DrawRoundedRectangle(roundRect, pBorderBrush, 1.2f);

    // Specular top rim highlight
    pRT->DrawLine(
        D2D1::Point2F(g_toolbarRect.left + r + 2.0f, g_toolbarRect.top + 1.5f),
        D2D1::Point2F(g_toolbarRect.right - r - 2.0f, g_toolbarRect.top + 1.5f),
        pRimBrush, 1.0f
    );

    // Subtle vertical dividers
    float divY1 = g_toolbarRect.top + 7.0f;
    float divY2 = g_toolbarRect.bottom - 7.0f;
    for (float divX : g_toolbarDividers) {
        pRT->DrawLine(D2D1::Point2F(divX, divY1), D2D1::Point2F(divX, divY2), pBorderBrush, 1.0f);
    }

    // Draw buttons
    for (size_t i = 0; i < g_toolbarButtons.size(); ++i) {
        const auto& btn = g_toolbarButtons[i];
        bool isHovered = (g_hoveredToolbarBtn == (int)i);

        if (btn.isPen) {
            // Pen swatch
            ID2D1SolidColorBrush* pPenBrush = nullptr;
            pRT->CreateSolidColorBrush(btn.penColor, &pPenBrush);
            if (pPenBrush) {
                D2D1_ROUNDED_RECT penR = D2D1::RoundedRect(btn.rect, 4.0f, 4.0f);
                pRT->FillRoundedRectangle(penR, pPenBrush);

                bool isActivePen = (g_currentTool == ToolMode::Pen &&
                                    btn.penColor.r == g_activeColor.r &&
                                    btn.penColor.g == g_activeColor.g &&
                                    btn.penColor.b == g_activeColor.b);

                ID2D1SolidColorBrush* pPenBorder = nullptr;
                if (isActivePen) {
                    pRT->CreateSolidColorBrush(D2D1::ColorF(0.32f, 0.85f, 0.69f, 1.0f), &pPenBorder);
                    pRT->DrawRoundedRectangle(penR, pPenBorder, 2.5f);
                }
                else if (isHovered) {
                    pRT->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.8f), &pPenBorder);
                    pRT->DrawRoundedRectangle(penR, pPenBorder, 1.5f);
                }
                else {
                    pRT->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.35f), &pPenBorder);
                    pRT->DrawRoundedRectangle(penR, pPenBorder, 1.0f);
                }
                if (pPenBorder) pPenBorder->Release();
                pPenBrush->Release();
            }
        }
        else {
            // Determine active highlight
            bool isToolActive = false;
            if (btn.id == 1 && g_currentTool == ToolMode::Highlighter) isToolActive = true;
            if (btn.id == 2 && g_currentTool == ToolMode::Eraser) isToolActive = true;
            if (btn.id == 3 && g_currentTool == ToolMode::Pan) isToolActive = true;
            if (btn.id == 4 && g_currentTool == ToolMode::Pointer) isToolActive = true;
            if (btn.id == 5 && !g_inkVisible) isToolActive = true; // Eye closed
            if (btn.id == 20 && g_currentShape == ShapeType::Freehand && g_currentTool == ToolMode::Pen) isToolActive = true;
            if (btn.id == 21 && g_currentShape == ShapeType::Line) isToolActive = true;
            if (btn.id == 22 && g_currentShape == ShapeType::Arrow) isToolActive = true;
            if (btn.id == 23 && g_currentShape == ShapeType::Rectangle) isToolActive = true;
            if (btn.id == 24 && g_currentShape == ShapeType::Ellipse) isToolActive = true;

            if (isToolActive || isHovered) {
                ID2D1SolidColorBrush* pHoverBg = nullptr;
                pRT->CreateSolidColorBrush(isToolActive ? D2D1::ColorF(0.20f, 0.32f, 0.44f, 0.95f) : D2D1::ColorF(0.18f, 0.22f, 0.30f, 0.85f), &pHoverBg);
                if (pHoverBg) {
                    pRT->FillRoundedRectangle(D2D1::RoundedRect(btn.rect, 4.0f, 4.0f), pHoverBg);
                    pHoverBg->Release();
                }
            }

            // Draw label / icon
            if (g_pIconFormat && !btn.label.empty()) {
                ID2D1SolidColorBrush* pLblBrush = isToolActive ? nullptr : pTextBrush;
                if (isToolActive) {
                    pRT->CreateSolidColorBrush(D2D1::ColorF(0.40f, 0.90f, 0.75f, 1.0f), &pLblBrush);
                }
                const wchar_t* iconText = (btn.id == 5) ? (g_inkVisible ? L"\uE890" : L"\uED1A") : btn.label.c_str();
                pRT->DrawText(
                    iconText,
                    (UINT32)wcslen(iconText),
                    g_pIconFormat,
                    btn.rect,
                    pLblBrush,
                    D2D1_DRAW_TEXT_OPTIONS_NONE
                );
                if (isToolActive && pLblBrush) pLblBrush->Release();
            }
        }
    }

    if (pTextBrush) pTextBrush->Release();
    if (pRimBrush) pRimBrush->Release();
    if (pBorderBrush) pBorderBrush->Release();
    if (pBgBrush) pBgBrush->Release();
}

void DrawRadialMenu(ID2D1HwndRenderTarget* pRT) {
    if (!g_radialActive) return;

    ID2D1SolidColorBrush* pBgBrush = nullptr;
    ID2D1SolidColorBrush* pBorderBrush = nullptr;
    ID2D1SolidColorBrush* pSpokeBrush = nullptr;
    ID2D1SolidColorBrush* pGlowBrush = nullptr;
    ID2D1SolidColorBrush* pHoverBrush = nullptr;
    ID2D1SolidColorBrush* pTextBrush = nullptr;

    pRT->CreateSolidColorBrush(D2D1::ColorF(0.08f, 0.10f, 0.15f, 0.94f), &pBgBrush);
    pRT->CreateSolidColorBrush(D2D1::ColorF(0.24f, 0.28f, 0.38f, 0.90f), &pBorderBrush);
    pRT->CreateSolidColorBrush(D2D1::ColorF(0.20f, 0.24f, 0.32f, 0.80f), &pSpokeBrush);
    pRT->CreateSolidColorBrush(D2D1::ColorF(0.32f, 0.85f, 0.69f, 0.50f), &pGlowBrush);
    pRT->CreateSolidColorBrush(D2D1::ColorF(0.20f, 0.35f, 0.48f, 0.92f), &pHoverBrush);
    pRT->CreateSolidColorBrush(D2D1::ColorF(0.92f, 0.95f, 0.98f, 1.00f), &pTextBrush);

    float cx = g_radialX;
    float cy = g_radialY;
    const float kCenterRadius = 36.0f;
    const float kInnerRingR = 44.0f;
    const float kOuterRingR = 96.0f;
    const float kActionIconR = 70.0f;
    const float kOrbitalRadius = 126.0f;

    // 1. Draw Action Ring Annulus (between 44px and 96px)
    float ringMidR = (kInnerRingR + kOuterRingR) * 0.5f;
    float ringThick = (kOuterRingR - kInnerRingR);
    pRT->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), ringMidR, ringMidR), pBgBrush, ringThick);
    pRT->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), kInnerRingR, kInnerRingR), pBorderBrush, 1.2f);
    pRT->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), kOuterRingR, kOuterRingR), pBorderBrush, 1.2f);

    // 2. Draw 8 Radial Divider Spokes
    for (int k = 0; k < 8; ++k) {
        float spokeAngle = (float)(k * (3.14159265358979323846 / 4.0) + 3.14159265358979323846 / 8.0);
        float x1 = cx + std::cos(spokeAngle) * kInnerRingR;
        float y1 = cy + std::sin(spokeAngle) * kInnerRingR;
        float x2 = cx + std::cos(spokeAngle) * kOuterRingR;
        float y2 = cy + std::sin(spokeAngle) * kOuterRingR;
        pRT->DrawLine(D2D1::Point2F(x1, y1), D2D1::Point2F(x2, y2), pSpokeBrush, 1.2f);
    }

    // 3. Draw Hovered Sector Highlight
    if (g_radialHoverSector >= 0 && g_radialHoverSector < 8) {
        float secAngle = (float)(g_radialHoverSector * (3.14159265358979323846 / 4.0));
        float hx = cx + std::cos(secAngle) * kActionIconR;
        float hy = cy + std::sin(secAngle) * kActionIconR;
        pRT->FillEllipse(D2D1::Ellipse(D2D1::Point2F(hx, hy), 20.0f, 20.0f), pHoverBrush);
        pRT->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(hx, hy), 20.0f, 20.0f), pGlowBrush, 1.5f);
    }

    // 4. Draw the 8 Sector Action Icons
    const wchar_t* sectorIcons[] = {
        L"\uE74D", // 0: East (0°) Clear All (Delete)
        L"\uE722", // 1: SE (45°) Snapshot (Camera)
        L"\uE75C", // 2: South (90°) Eraser (EraseTool)
        L"\uE7A7", // 3: SW (135°) Undo
        L"\uE7C9", // 4: West (180°) Pointer (TouchPointer)
        g_inkVisible ? L"\uE890" : L"\uED1A", // 5: NW (225°) Ink Visible (View / Hide)
        L"\uE7C2", // 6: North (270°) Pan (Move - 4-way arrows)
        L"\uEC87"  // 7: NE (315°) Draw / Pen (Draw)
    };

    for (int k = 0; k < 8; ++k) {
        float secAngle = (float)(k * (3.14159265358979323846 / 4.0));
        float ix = cx + std::cos(secAngle) * kActionIconR;
        float iy = cy + std::sin(secAngle) * kActionIconR;

        D2D1_RECT_F iconRect = D2D1::RectF(ix - 16.0f, iy - 16.0f, ix + 16.0f, iy + 16.0f);
        if (g_pRadialIconFormat) {
            pRT->DrawText(
                sectorIcons[k], (UINT32)wcslen(sectorIcons[k]),
                g_pRadialIconFormat,
                iconRect,
                pTextBrush,
                D2D1_DRAW_TEXT_OPTIONS_NONE
            );
        }
    }

    // 5. Center Hub: Active Swatch & Pen/Highlighter Toggle
    bool centerHovered = (g_radialHoverTarget == RadialTarget::Center);
    if (centerHovered) {
        pRT->FillEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), kCenterRadius + 3.0f, kCenterRadius + 3.0f), pGlowBrush);
    }
    pRT->FillEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), kCenterRadius, kCenterRadius), pBgBrush);
    pRT->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), kCenterRadius, kCenterRadius), centerHovered ? pGlowBrush : pBorderBrush, centerHovered ? 2.0f : 1.5f);

    ID2D1SolidColorBrush* pActiveBrush = nullptr;
    pRT->CreateSolidColorBrush(g_activeColor, &pActiveBrush);
    if (pActiveBrush) {
        pRT->FillEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), 16.0f, 16.0f), pActiveBrush);
        pRT->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), 16.0f, 16.0f), pBorderBrush, 1.2f);
        pActiveBrush->Release();
    }

    const wchar_t* centerBadge = (g_currentTool == ToolMode::Highlighter) ? L"\uE7E6" : L"\uEC87";
    if (g_pCenterBadgeFormat) {
        D2D1_RECT_F centerTextRect = D2D1::RectF(cx - 14.0f, cy - 14.0f, cx + 14.0f, cy + 14.0f);
        ID2D1SolidColorBrush* pWhite = nullptr;
        pRT->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.95f), &pWhite);
        if (pWhite) {
            pRT->DrawText(centerBadge, (UINT32)wcslen(centerBadge), g_pCenterBadgeFormat, centerTextRect, pWhite);
            pWhite->Release();
        }
    }

    // 6. Outer Color Wheel Guide track
    pRT->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), kOrbitalRadius, kOrbitalRadius), pBorderBrush, 0.8f);

    // 7. 360-Degree Orbital Color Orbs (16 colors)
    for (size_t i = 0; i < kPresetColorCount; ++i) {
        float angle = (float)(i * (2.0 * 3.14159265358979323846 / kPresetColorCount) - 3.14159265358979323846 * 0.5);
        float ox = cx + std::cos(angle) * kOrbitalRadius;
        float oy = cy + std::sin(angle) * kOrbitalRadius;

        bool isHovered = (g_hoveredOrb == (int)i);
        float orbR = isHovered ? 15.5f : 11.5f;

        ID2D1SolidColorBrush* pOrbBrush = nullptr;
        pRT->CreateSolidColorBrush(kPresetColors[i], &pOrbBrush);
        if (pOrbBrush) {
            if (isHovered) {
                pRT->FillEllipse(D2D1::Ellipse(D2D1::Point2F(ox, oy), orbR + 4.0f, orbR + 4.0f), pGlowBrush);
            }
            pRT->FillEllipse(D2D1::Ellipse(D2D1::Point2F(ox, oy), orbR, orbR), pOrbBrush);

            ID2D1SolidColorBrush* pOrbBorder = nullptr;
            pRT->CreateSolidColorBrush(isHovered ? D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.95f) : D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.40f), &pOrbBorder);
            if (pOrbBorder) {
                pRT->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(ox, oy), orbR, orbR), pOrbBorder, isHovered ? 2.0f : 1.0f);
                pOrbBorder->Release();
            }

            pOrbBrush->Release();
        }
    }

    if (pTextBrush) pTextBrush->Release();
    if (pHoverBrush) pHoverBrush->Release();
    if (pGlowBrush) pGlowBrush->Release();
    if (pSpokeBrush) pSpokeBrush->Release();
    if (pBorderBrush) pBorderBrush->Release();
    if (pBgBrush) pBgBrush->Release();
}

void DrawEraserCursor(ID2D1HwndRenderTarget* pRT) {
    if (!g_isRightClickErasing && !g_isRightMouseDown && !g_isLeftClickErasing && !g_isRightClickClearing && g_currentTool != ToolMode::Eraser) return;

    ID2D1SolidColorBrush* pFillBrush = nullptr;
    ID2D1SolidColorBrush* pRingBrush = nullptr;

    pRT->CreateSolidColorBrush(D2D1::ColorF(0.95f, 0.35f, 0.40f, 0.22f), &pFillBrush);
    pRT->CreateSolidColorBrush(D2D1::ColorF(0.95f, 0.35f, 0.40f, 0.90f), &pRingBrush);

    D2D1_ELLIPSE ell = D2D1::Ellipse(D2D1::Point2F(g_cursorX, g_cursorY), g_eraserRadius, g_eraserRadius);
    pRT->FillEllipse(ell, pFillBrush);
    pRT->DrawEllipse(ell, pRingBrush, 1.5f);

    if (pRingBrush) pRingBrush->Release();
    if (pFillBrush) pFillBrush->Release();
}

void DrawPenSizePreview(ID2D1HwndRenderTarget* pRT) {
    if (g_sizePreviewTime == 0) return;
    ULONGLONG elapsed = GetTickCount64() - g_sizePreviewTime;
    if (elapsed > 900) {
        g_sizePreviewTime = 0;
        return;
    }

    float alpha = 1.0f;
    if (elapsed > 600) {
        alpha = 1.0f - (float)(elapsed - 600) / 300.0f;
    }

    ID2D1SolidColorBrush* pRing = nullptr;
    pRT->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.85f * alpha), &pRing);
    if (pRing) {
        float r = g_currentPenWidth * 0.5f;
        pRT->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(g_cursorX, g_cursorY), r, r), pRing, 1.5f);
        pRing->Release();
    }
}

void DrawZoomPreview(ID2D1HwndRenderTarget* pRT) {
    if (g_zoomPreviewTime == 0) return;
    ULONGLONG elapsed = GetTickCount64() - g_zoomPreviewTime;
    if (elapsed > 1100) {
        g_zoomPreviewTime = 0;
        return;
    }

    float alpha = 1.0f;
    if (elapsed > 800) {
        alpha = 1.0f - (float)(elapsed - 800) / 300.0f;
    }

    int zoomPct = (int)std::round(g_zoomScale * 100.0f);
    std::wstring text = L"Zoom: " + std::to_wstring(zoomPct) + L"%";

    const float badgeW = 110.0f;
    const float badgeH = 26.0f;
    float badgeX = (g_toolbarRect.left + g_toolbarRect.right - badgeW) * 0.5f;
    float badgeY = g_toolbarRect.top - badgeH - 8.0f;
    if (!g_settings.showBottomToolbar || badgeY < 10.0f) {
        D2D1_SIZE_F rtSize = pRT->GetSize();
        badgeX = (rtSize.width - badgeW) * 0.5f;
        badgeY = rtSize.height - 60.0f;
    }

    D2D1_RECT_F rect = D2D1::RectF(badgeX, badgeY, badgeX + badgeW, badgeY + badgeH);

    ID2D1SolidColorBrush* pBgBrush = nullptr;
    ID2D1SolidColorBrush* pBorderBrush = nullptr;
    ID2D1SolidColorBrush* pTextBrush = nullptr;

    pRT->CreateSolidColorBrush(D2D1::ColorF(0.10f, 0.12f, 0.16f, 0.88f * alpha), &pBgBrush);
    pRT->CreateSolidColorBrush(D2D1::ColorF(0.35f, 0.40f, 0.50f, 0.80f * alpha), &pBorderBrush);
    pRT->CreateSolidColorBrush(D2D1::ColorF(0.95f, 0.96f, 0.98f, 0.95f * alpha), &pTextBrush);

    if (pBgBrush && pBorderBrush && pTextBrush && g_pTextFormat) {
        pRT->FillRoundedRectangle(D2D1::RoundedRect(rect, 5.0f, 5.0f), pBgBrush);
        pRT->DrawRoundedRectangle(D2D1::RoundedRect(rect, 5.0f, 5.0f), pBorderBrush, 1.0f);
        pRT->DrawText(text.c_str(), (UINT32)text.length(), g_pTextFormat, rect, pTextBrush);
    }

    if (pTextBrush) pTextBrush->Release();
    if (pBorderBrush) pBorderBrush->Release();
    if (pBgBrush) pBgBrush->Release();
}

void DrawToast(ID2D1HwndRenderTarget* pRT, int screenW, int screenH) {
    if (g_toastStartTime == 0) return;
    ULONGLONG elapsed = GetTickCount64() - g_toastStartTime;
    if (elapsed > 2000) {
        g_toastStartTime = 0;
        return;
    }

    float alpha = 1.0f;
    if (elapsed > 1600) {
        alpha = 1.0f - (float)(elapsed - 1600) / 400.0f;
    }

    ID2D1SolidColorBrush* pBg = nullptr;
    ID2D1SolidColorBrush* pBorder = nullptr;
    ID2D1SolidColorBrush* pText = nullptr;

    pRT->CreateSolidColorBrush(D2D1::ColorF(0.10f, 0.13f, 0.18f, 0.94f * alpha), &pBg);
    pRT->CreateSolidColorBrush(D2D1::ColorF(0.32f, 0.85f, 0.69f, 0.90f * alpha), &pBorder);
    pRT->CreateSolidColorBrush(D2D1::ColorF(0.95f, 0.98f, 1.00f, 1.00f * alpha), &pText);

    const float tw = 340.0f;
    const float th = 40.0f;

    int vx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int vy = GetSystemMetrics(SM_YVIRTUALSCREEN);
    float toastX = (screenW - tw) * 0.5f;
    float toastY = 40.0f;

    HMONITOR hPrimaryMon = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi = { sizeof(MONITORINFO) };
    if (hPrimaryMon && GetMonitorInfo(hPrimaryMon, &mi)) {
        float clientLeft = (float)(mi.rcMonitor.left - vx);
        float monW = (float)(mi.rcMonitor.right - mi.rcMonitor.left);
        float workTop = (float)(mi.rcWork.top - vy);
        toastX = clientLeft + (monW - tw) * 0.5f;
        toastY = workTop + 24.0f;
    }

    D2D1_RECT_F toastRect = D2D1::RectF(
        toastX,
        toastY,
        toastX + tw,
        toastY + th
    );

    pRT->FillRoundedRectangle(D2D1::RoundedRect(toastRect, 6.0f, 6.0f), pBg);
    pRT->DrawRoundedRectangle(D2D1::RoundedRect(toastRect, 6.0f, 6.0f), pBorder, 1.2f);

    if (g_pIconFormat && g_pTextFormat) {
        D2D1_RECT_F iconRect = D2D1::RectF(toastRect.left + 12.0f, toastRect.top, toastRect.left + 36.0f, toastRect.bottom);
        D2D1_RECT_F textRect = D2D1::RectF(toastRect.left + 38.0f, toastRect.top, toastRect.right - 12.0f, toastRect.bottom);

        ID2D1SolidColorBrush* pCheckBrush = nullptr;
        pRT->CreateSolidColorBrush(D2D1::ColorF(0.32f, 0.85f, 0.69f, 1.00f * alpha), &pCheckBrush);
        pRT->DrawText(L"\uE73E", 1, g_pIconFormat, iconRect, pCheckBrush ? pCheckBrush : pText);
        if (pCheckBrush) pCheckBrush->Release();

        g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        pRT->DrawText(g_toastMessage.c_str(), (UINT32)g_toastMessage.length(), g_pTextFormat, textRect, pText);
        g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    }
    else if (g_pTextFormat) {
        pRT->DrawText(g_toastMessage.c_str(), (UINT32)g_toastMessage.length(), g_pTextFormat, toastRect, pText);
    }

    if (pText) pText->Release();
    if (pBorder) pBorder->Release();
    if (pBg) pBg->Release();
}

void RenderOverlay() {
    if (!g_pRenderTarget) return;

    g_pRenderTarget->BeginDraw();
    g_pRenderTarget->Clear(D2D1::ColorF(0, 0, 0, 0));

    // 1. Draw desktop backdrop (only when freezeScreen setting is enabled and not in click-through pointer mode)
    if (g_settings.freezeScreen && g_pDesktopBitmap && g_currentTool != ToolMode::Pointer) {
        D2D1_SIZE_F size = g_pRenderTarget->GetSize();
        g_pRenderTarget->DrawBitmap(
            g_pDesktopBitmap,
            D2D1::RectF(0, 0, size.width, size.height)
        );
    }

    // Apply Pan & Zoom Transform to Strokes
    D2D1_MATRIX_3X2_F canvasMatrix = D2D1::Matrix3x2F::Scale(g_zoomScale, g_zoomScale) *
                                     D2D1::Matrix3x2F::Translation(g_panOffsetX, g_panOffsetY);
    g_pRenderTarget->SetTransform(canvasMatrix);

    // 2. Draw completed strokes
    for (const auto& stroke : g_strokes) {
        DrawSmoothStroke(g_pRenderTarget, stroke);
    }

    // 3. Draw active stroke in progress
    if (g_isDrawing) {
        DrawSmoothStroke(g_pRenderTarget, g_currentStroke);
    }

    // Reset transform for HUD & Toolbar
    g_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

    // 4. Eraser cursor, Pen size bubble & Zoom badge
    DrawEraserCursor(g_pRenderTarget);
    DrawPenSizePreview(g_pRenderTarget);
    DrawZoomPreview(g_pRenderTarget);

    // 5. Compact Bottom Toolbar
    DrawToolbar(g_pRenderTarget);

    // 6. Circular Radial Quick Menu
    DrawRadialMenu(g_pRenderTarget);

    // 7. Toast feedback
    D2D1_SIZE_F s = g_pRenderTarget->GetSize();
    DrawToast(g_pRenderTarget, (int)s.width, (int)s.height);

    g_pRenderTarget->EndDraw();
}

// ----------------------------------------------------------------------------
// Erasing Logic (Brush Eraser & Whole-Shape Eraser)
// ----------------------------------------------------------------------------

void EraseBrushAt(float x, float y, float radius) {
    float adjustedRadius = radius / g_zoomScale;
    float rSq = adjustedRadius * adjustedRadius;
    bool changed = false;
    float adjustedX = (x - g_panOffsetX) / g_zoomScale;
    float adjustedY = (y - g_panOffsetY) / g_zoomScale;

    std::vector<Stroke> resultingStrokes;
    resultingStrokes.reserve(g_strokes.size() + 8);

    for (auto& stroke : g_strokes) {
        if (stroke.shapeType == ShapeType::Freehand) {
            bool touches = false;
            for (size_t i = 0; i < stroke.points.size(); ++i) {
                if (DistanceSq(adjustedX, adjustedY, stroke.points[i].x, stroke.points[i].y) <= rSq) {
                    touches = true;
                    break;
                }
                if (i + 1 < stroke.points.size()) {
                    if (DistToSegmentSq(adjustedX, adjustedY, stroke.points[i].x, stroke.points[i].y, stroke.points[i + 1].x, stroke.points[i + 1].y) <= rSq) {
                        touches = true;
                        break;
                    }
                }
            }

            if (!touches) {
                resultingStrokes.push_back(stroke);
                continue;
            }

            changed = true;
            std::vector<StrokePoint> detailed;
            detailed.reserve(stroke.points.size() * 2);
            for (size_t i = 0; i < stroke.points.size(); ++i) {
                detailed.push_back(stroke.points[i]);
                if (i + 1 < stroke.points.size()) {
                    float dx = stroke.points[i + 1].x - stroke.points[i].x;
                    float dy = stroke.points[i + 1].y - stroke.points[i].y;
                    float d = std::sqrt(dx * dx + dy * dy);
                    float step = std::max(1.0f, adjustedRadius * 0.35f);
                    if (d > step) {
                        int steps = (int)(d / step);
                        for (int s = 1; s < steps; ++s) {
                            float t = (float)s / (float)steps;
                            detailed.push_back({ stroke.points[i].x + dx * t, stroke.points[i].y + dy * t });
                        }
                    }
                }
            }

            std::vector<StrokePoint> curSeg;
            for (const auto& pt : detailed) {
                if (DistanceSq(adjustedX, adjustedY, pt.x, pt.y) <= rSq) {
                    if (!curSeg.empty()) {
                        Stroke subStroke = stroke;
                        subStroke.points = curSeg;
                        resultingStrokes.push_back(subStroke);
                        curSeg.clear();
                    }
                }
                else {
                    curSeg.push_back(pt);
                }
            }
            if (!curSeg.empty()) {
                Stroke subStroke = stroke;
                subStroke.points = curSeg;
                resultingStrokes.push_back(subStroke);
            }
        }
        else if (stroke.shapeType == ShapeType::Line || stroke.shapeType == ShapeType::Arrow) {
            if (DistToSegmentSq(adjustedX, adjustedY, stroke.startPt.x, stroke.startPt.y, stroke.endPt.x, stroke.endPt.y) > rSq) {
                resultingStrokes.push_back(stroke);
                continue;
            }

            changed = true;
            float dx = stroke.endPt.x - stroke.startPt.x;
            float dy = stroke.endPt.y - stroke.startPt.y;
            float len = std::sqrt(dx * dx + dy * dy);
            float step = std::max(1.0f, adjustedRadius * 0.35f);
            int steps = (int)(len / step);
            if (steps < 2) steps = 2;

            std::vector<StrokePoint> curSeg;
            for (int s = 0; s <= steps; ++s) {
                float t = (float)s / (float)steps;
                float px = stroke.startPt.x + dx * t;
                float py = stroke.startPt.y + dy * t;
                if (DistanceSq(adjustedX, adjustedY, px, py) <= rSq) {
                    if (!curSeg.empty()) {
                        Stroke subStroke = stroke;
                        subStroke.shapeType = ShapeType::Freehand;
                        subStroke.points = curSeg;
                        resultingStrokes.push_back(subStroke);
                        curSeg.clear();
                    }
                }
                else {
                    curSeg.push_back({ px, py });
                }
            }
            if (!curSeg.empty()) {
                Stroke subStroke = stroke;
                subStroke.shapeType = ShapeType::Freehand;
                subStroke.points = curSeg;
                resultingStrokes.push_back(subStroke);
            }
        }
        else if (stroke.shapeType == ShapeType::Rectangle) {
            float minX = std::min(stroke.startPt.x, stroke.endPt.x);
            float maxX = std::max(stroke.startPt.x, stroke.endPt.x);
            float minY = std::min(stroke.startPt.y, stroke.endPt.y);
            float maxY = std::max(stroke.startPt.y, stroke.endPt.y);

            bool touches = (DistToSegmentSq(adjustedX, adjustedY, minX, minY, maxX, minY) <= rSq ||
                            DistToSegmentSq(adjustedX, adjustedY, maxX, minY, maxX, maxY) <= rSq ||
                            DistToSegmentSq(adjustedX, adjustedY, maxX, maxY, minX, maxY) <= rSq ||
                            DistToSegmentSq(adjustedX, adjustedY, minX, maxY, minX, minY) <= rSq);
            if (!touches) {
                resultingStrokes.push_back(stroke);
                continue;
            }

            changed = true;
            std::vector<StrokePoint> rectPts;
            float step = std::max(1.0f, adjustedRadius * 0.35f);
            for (float px = minX; px < maxX; px += step) rectPts.push_back({ px, minY });
            for (float py = minY; py < maxY; py += step) rectPts.push_back({ maxX, py });
            for (float px = maxX; px > minX; px -= step) rectPts.push_back({ px, maxY });
            for (float py = maxY; py > minY; py -= step) rectPts.push_back({ minX, py });
            rectPts.push_back({ minX, minY });

            std::vector<StrokePoint> curSeg;
            for (const auto& pt : rectPts) {
                if (DistanceSq(adjustedX, adjustedY, pt.x, pt.y) <= rSq) {
                    if (!curSeg.empty()) {
                        Stroke subStroke = stroke;
                        subStroke.shapeType = ShapeType::Freehand;
                        subStroke.points = curSeg;
                        resultingStrokes.push_back(subStroke);
                        curSeg.clear();
                    }
                }
                else {
                    curSeg.push_back(pt);
                }
            }
            if (!curSeg.empty()) {
                Stroke subStroke = stroke;
                subStroke.shapeType = ShapeType::Freehand;
                subStroke.points = curSeg;
                resultingStrokes.push_back(subStroke);
            }
        }
        else if (stroke.shapeType == ShapeType::Ellipse) {
            float cx = (stroke.startPt.x + stroke.endPt.x) * 0.5f;
            float cy = (stroke.startPt.y + stroke.endPt.y) * 0.5f;
            float rx = std::abs(stroke.endPt.x - stroke.startPt.x) * 0.5f;
            float ry = std::abs(stroke.endPt.y - stroke.startPt.y) * 0.5f;

            std::vector<StrokePoint> ellPts;
            int numPts = 120;
            for (int k = 0; k <= numPts; ++k) {
                float angle = (float)(k * (2.0 * 3.14159265358979323846 / numPts));
                ellPts.push_back({ cx + rx * std::cos(angle), cy + ry * std::sin(angle) });
            }

            bool touches = false;
            for (const auto& pt : ellPts) {
                if (DistanceSq(adjustedX, adjustedY, pt.x, pt.y) <= rSq) {
                    touches = true;
                    break;
                }
            }

            if (!touches) {
                resultingStrokes.push_back(stroke);
                continue;
            }

            changed = true;
            std::vector<StrokePoint> curSeg;
            for (const auto& pt : ellPts) {
                if (DistanceSq(adjustedX, adjustedY, pt.x, pt.y) <= rSq) {
                    if (!curSeg.empty()) {
                        Stroke subStroke = stroke;
                        subStroke.shapeType = ShapeType::Freehand;
                        subStroke.points = curSeg;
                        resultingStrokes.push_back(subStroke);
                        curSeg.clear();
                    }
                }
                else {
                    curSeg.push_back(pt);
                }
            }
            if (!curSeg.empty()) {
                Stroke subStroke = stroke;
                subStroke.shapeType = ShapeType::Freehand;
                subStroke.points = curSeg;
                resultingStrokes.push_back(subStroke);
            }
        }
    }

    if (changed) {
        g_strokes = std::move(resultingStrokes);
        InvalidateOverlay();
    }
}

bool EraseWholeShapeAt(float x, float y, float radius) {
    float adjustedRadius = radius / g_zoomScale;
    float rSq = adjustedRadius * adjustedRadius;
    bool changed = false;
    float adjustedX = (x - g_panOffsetX) / g_zoomScale;
    float adjustedY = (y - g_panOffsetY) / g_zoomScale;

    for (auto it = g_strokes.begin(); it != g_strokes.end();) {
        bool hit = false;
        if (it->shapeType == ShapeType::Freehand) {
            const auto& pts = it->points;
            for (size_t i = 0; i < pts.size(); ++i) {
                if (DistanceSq(adjustedX, adjustedY, pts[i].x, pts[i].y) <= rSq) {
                    hit = true;
                    break;
                }
                if (i + 1 < pts.size()) {
                    if (DistToSegmentSq(adjustedX, adjustedY, pts[i].x, pts[i].y, pts[i + 1].x, pts[i + 1].y) <= rSq) {
                        hit = true;
                        break;
                    }
                }
            }
        }
        else if (it->shapeType == ShapeType::Line || it->shapeType == ShapeType::Arrow) {
            if (DistToSegmentSq(adjustedX, adjustedY, it->startPt.x, it->startPt.y, it->endPt.x, it->endPt.y) <= rSq) {
                hit = true;
            }
        }
        else if (it->shapeType == ShapeType::Rectangle) {
            float minX = std::min(it->startPt.x, it->endPt.x);
            float maxX = std::max(it->startPt.x, it->endPt.x);
            float minY = std::min(it->startPt.y, it->endPt.y);
            float maxY = std::max(it->startPt.y, it->endPt.y);
            if (DistToSegmentSq(adjustedX, adjustedY, minX, minY, maxX, minY) <= rSq ||
                DistToSegmentSq(adjustedX, adjustedY, maxX, minY, maxX, maxY) <= rSq ||
                DistToSegmentSq(adjustedX, adjustedY, maxX, maxY, minX, maxY) <= rSq ||
                DistToSegmentSq(adjustedX, adjustedY, minX, maxY, minX, minY) <= rSq) {
                hit = true;
            }
        }
        else if (it->shapeType == ShapeType::Ellipse) {
            float cx = (it->startPt.x + it->endPt.x) * 0.5f;
            float cy = (it->startPt.y + it->endPt.y) * 0.5f;
            float rx = std::abs(it->endPt.x - it->startPt.x) * 0.5f;
            float ry = std::abs(it->endPt.y - it->startPt.y) * 0.5f;
            float dx = adjustedX - cx;
            float dy = adjustedY - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float avgR = (rx + ry) * 0.5f;
            if (std::abs(dist - avgR) <= adjustedRadius) {
                hit = true;
            }
        }

        if (hit) {
            g_redoStack.push_back(*it);
            it = g_strokes.erase(it);
            changed = true;
        }
        else {
            ++it;
        }
    }

    if (changed) {
        InvalidateOverlay();
    }
    return changed;
}

// ----------------------------------------------------------------------------
// Snapshot & PNG Export via WIC
// ----------------------------------------------------------------------------

void SaveBitmapToPNG(HBITMAP hBitmap, const std::wstring& filePath) {
    if (!g_pWICFactory) return;

    IWICBitmap* pWicBitmap = nullptr;
    HRESULT hr = g_pWICFactory->CreateBitmapFromHBITMAP(hBitmap, NULL, WICBitmapUseAlpha, &pWicBitmap);
    if (FAILED(hr)) return;

    IWICStream* pStream = nullptr;
    hr = g_pWICFactory->CreateStream(&pStream);
    if (SUCCEEDED(hr)) {
        hr = pStream->InitializeFromFilename(filePath.c_str(), GENERIC_WRITE);
        if (SUCCEEDED(hr)) {
            IWICBitmapEncoder* pEncoder = nullptr;
            hr = g_pWICFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder);
            if (SUCCEEDED(hr)) {
                hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
                if (SUCCEEDED(hr)) {
                    IWICBitmapFrameEncode* pFrame = nullptr;
                    hr = pEncoder->CreateNewFrame(&pFrame, NULL);
                    if (SUCCEEDED(hr)) {
                        hr = pFrame->Initialize(NULL);
                        if (SUCCEEDED(hr)) {
                            UINT w = 0, h = 0;
                            pWicBitmap->GetSize(&w, &h);
                            pFrame->SetSize(w, h);

                            WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
                            pFrame->SetPixelFormat(&format);

                            hr = pFrame->WriteSource(pWicBitmap, NULL);
                            if (SUCCEEDED(hr)) {
                                pFrame->Commit();
                                pEncoder->Commit();
                            }
                        }
                        pFrame->Release();
                    }
                }
                pEncoder->Release();
            }
        }
        pStream->Release();
    }
    pWicBitmap->Release();
}

void CopySnapshotToClipboard() {
    int vx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int vy = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    HDC hScreenDC = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hScreenDC);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, vw, vh);
    HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hBitmap);

    BitBlt(hMemDC, 0, 0, vw, vh, hScreenDC, vx, vy, SRCCOPY | CAPTUREBLT);

    // Save to Clipboard
    if (OpenClipboard(g_hOverlayWnd)) {
        EmptyClipboard();
        SetClipboardData(CF_BITMAP, hBitmap);
        CloseClipboard();
    }

    // Auto-save PNG if enabled
    if (g_settings.autoSaveSnapshot) {
        wchar_t picturesPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_MYPICTURES, NULL, 0, picturesPath))) {
            std::wstring winDrawDir = std::wstring(picturesPath) + L"\\WinDraw";
            CreateDirectoryW(winDrawDir.c_str(), NULL);

            SYSTEMTIME st;
            GetLocalTime(&st);
            wchar_t filename[128];
            wsprintfW(filename, L"\\WinDraw_%04d-%02d-%02d_%02d%02d%02d.png",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

            std::wstring fullPath = winDrawDir + filename;
            SaveBitmapToPNG(hBitmap, fullPath);
            g_toastMessage = L"Saved to Pictures/WinDraw & Clipboard";
        }
    }
    else {
        g_toastMessage = L"Copied to Clipboard";
    }

    SelectObject(hMemDC, hOldBmp);
    DeleteDC(hMemDC);
    ReleaseDC(NULL, hScreenDC);

    g_toastStartTime = GetTickCount64();
    if (g_hOverlayWnd) SetTimer(g_hOverlayWnd, 1, 30, NULL);
    InvalidateOverlay();
}

void SetToolMode(ToolMode newMode) {
    if (g_currentTool == newMode) return;
    ToolMode oldMode = g_currentTool;
    g_currentTool = newMode;

    if (g_hOverlayWnd) {
        if (newMode == ToolMode::Pointer) {
            // Enter Pointer (Click-Through) mode:
            // Apply WS_EX_LAYERED | WS_EX_TRANSPARENT so mouse clicks pass through to background apps
            LONG_PTR exStyle = GetWindowLongPtr(g_hOverlayWnd, GWL_EXSTYLE);
            SetWindowLongPtr(g_hOverlayWnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED | WS_EX_TRANSPARENT);
            SetLayeredWindowAttributes(g_hOverlayWnd, 0, 255, LWA_ALPHA);
            SetWindowPos(g_hOverlayWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
            SetTimer(g_hOverlayWnd, 2, 20, NULL);

            g_toastMessage = L"Pointer Mode: Click-through active";
            g_toastStartTime = GetTickCount64();
            SetTimer(g_hOverlayWnd, 1, 30, NULL);
        }
        else if (oldMode == ToolMode::Pointer) {
            // Exit Pointer mode:
            KillTimer(g_hOverlayWnd, 2);
            LONG_PTR exStyle = GetWindowLongPtr(g_hOverlayWnd, GWL_EXSTYLE);
            SetWindowLongPtr(g_hOverlayWnd, GWL_EXSTYLE, exStyle & ~(WS_EX_LAYERED | WS_EX_TRANSPARENT));
            SetWindowPos(g_hOverlayWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

            g_toastMessage = L"Drawing Mode active";
            g_toastStartTime = GetTickCount64();
            SetTimer(g_hOverlayWnd, 1, 30, NULL);
        }
        InvalidateOverlay();
    }
}

// ----------------------------------------------------------------------------
// Overlay Window Procedure
// ----------------------------------------------------------------------------

LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_USER_TOGGLE_POINTER: {
        if (g_currentTool == ToolMode::Pointer) {
            SetToolMode(ToolMode::Pen);
            SetForegroundWindow(hwnd);
            SetFocus(hwnd);
        }
        else {
            SetToolMode(ToolMode::Pointer);
        }
        return 0;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        RenderOverlay();
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        float step = (delta > 0) ? 1.0f : -1.0f;

        POINT wheelPt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hwnd, &wheelPt);
        g_cursorX = (float)wheelPt.x;
        g_cursorY = (float)wheelPt.y;

        // Holding Right-click OR in Eraser Mode: Scroll wheel resizes eraser radius!
        if (g_isRightMouseDown || g_isRightClickErasing || g_currentTool == ToolMode::Eraser) {
            g_wheelUsedWhileRightMouseDown = true;
            g_eraserRadius = std::max(6.0f, std::min(150.0f, g_eraserRadius + step * 3.0f));
            InvalidateOverlay();
            return 0;
        }

        // Pan Mode / Holding Canvas: Scroll wheel zooms canvas in and out centered on cursor!
        if (g_currentTool == ToolMode::Pan || g_isPanning) {
            float notches = (float)delta / 120.0f;
            float factor = std::pow(1.12f, notches);
            float oldScale = g_zoomScale;
            float newScale = std::max(0.15f, std::min(8.0f, oldScale * factor));

            if (std::abs(newScale - oldScale) > 0.0005f) {
                // Zoom centered on cursor:
                g_panOffsetX = g_cursorX - (g_cursorX - g_panOffsetX) * (newScale / oldScale);
                g_panOffsetY = g_cursorY - (g_cursorY - g_panOffsetY) * (newScale / oldScale);
                g_zoomScale = newScale;

                if (g_isPanning) {
                    g_panStartPos = { (LONG)g_cursorX, (LONG)g_cursorY };
                }

                g_zoomPreviewTime = GetTickCount64();
                SetTimer(hwnd, 1, 30, NULL);
                InvalidateOverlay();
            }
            return 0;
        }

        if (g_currentTool == ToolMode::Highlighter) {
            g_settings.defaultHighlighterWidth = std::max(4.0f, std::min(80.0f, g_settings.defaultHighlighterWidth + step * 2.0f));
            g_currentPenWidth = g_settings.defaultHighlighterWidth;
        }
        else {
            g_settings.defaultPenWidth = std::max(1.0f, std::min(50.0f, g_settings.defaultPenWidth + step));
            g_currentPenWidth = g_settings.defaultPenWidth;
        }
        g_sizePreviewTime = GetTickCount64();
        SetTimer(hwnd, 1, 30, NULL);
        InvalidateOverlay();
        return 0;
    }

    case WM_TIMER: {
        if (wParam == 1) {
            bool needTimer = false;
            ULONGLONG now = GetTickCount64();
            if (g_zoomPreviewTime != 0) {
                if (now - g_zoomPreviewTime > 1100) {
                    g_zoomPreviewTime = 0;
                } else {
                    needTimer = true;
                }
            }
            if (g_sizePreviewTime != 0) {
                if (now - g_sizePreviewTime > 900) {
                    g_sizePreviewTime = 0;
                } else {
                    needTimer = true;
                }
            }
            if (g_toastStartTime != 0) {
                if (now - g_toastStartTime > 2000) {
                    g_toastStartTime = 0;
                } else {
                    needTimer = true;
                }
            }
            InvalidateOverlay();
            if (!needTimer) {
                KillTimer(hwnd, 1);
            }
            return 0;
        }
        if (wParam == 2) {
            // Pointer (Click-Through) mode: monitor mouse to allow interacting with the toolbar
            if (g_currentTool == ToolMode::Pointer && g_bIsActive) {
                POINT pt;
                GetCursorPos(&pt);
                ScreenToClient(hwnd, &pt);

                bool overToolbar = false;
                if (g_settings.showBottomToolbar) {
                    if (pt.x >= (g_toolbarRect.left - 6.0f) && pt.x <= (g_toolbarRect.right + 6.0f) &&
                        pt.y >= (g_toolbarRect.top - 6.0f) && pt.y <= (g_toolbarRect.bottom + 6.0f)) {
                        overToolbar = true;
                    }
                }

                LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
                if (overToolbar) {
                    // Over the toolbar: remove WS_EX_TRANSPARENT so user can hover and click buttons
                    if (exStyle & WS_EX_TRANSPARENT) {
                        SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_TRANSPARENT);
                        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
                        InvalidateOverlay();
                    }
                }
                else {
                    // Outside the toolbar: restore WS_EX_TRANSPARENT so clicks pass to apps underneath
                    if (!(exStyle & WS_EX_TRANSPARENT)) {
                        SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);
                        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
                    }
                }
            }
            else {
                KillTimer(hwnd, 2);
                LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
                if (exStyle & (WS_EX_LAYERED | WS_EX_TRANSPARENT)) {
                    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle & ~(WS_EX_LAYERED | WS_EX_TRANSPARENT));
                    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
                }
            }
            return 0;
        }
        return 0;
    }

    case WM_KEYDOWN: {
        if (wParam == VK_ESCAPE) {
            HideOverlay();
            return 0;
        }
        if (((GetKeyState(VK_CONTROL) & 0x8000) && (wParam == '0' || wParam == VK_NUMPAD0)) ||
            (wParam == '0' && g_currentTool == ToolMode::Pan)) {
            // Reset Pan & Zoom to default (100% scale, 0 offset)
            g_zoomScale = 1.0f;
            g_panOffsetX = 0.0f;
            g_panOffsetY = 0.0f;
            if (g_isPanning) {
                g_panStartPos = { (LONG)g_cursorX, (LONG)g_cursorY };
            }
            g_zoomPreviewTime = GetTickCount64();
            SetTimer(hwnd, 1, 30, NULL);
            InvalidateOverlay();
            return 0;
        }
        if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam == 'Z') {
            if (GetKeyState(VK_SHIFT) & 0x8000) {
                // Redo (Ctrl+Shift+Z)
                if (!g_redoStack.empty()) {
                    g_strokes.push_back(g_redoStack.back());
                    g_redoStack.pop_back();
                    InvalidateOverlay();
                }
            }
            else {
                // Undo (Ctrl+Z)
                if (!g_strokes.empty()) {
                    g_redoStack.push_back(g_strokes.back());
                    g_strokes.pop_back();
                    InvalidateOverlay();
                }
            }
            return 0;
        }
        if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam == 'Y') {
            // Redo (Ctrl+Y)
            if (!g_redoStack.empty()) {
                g_strokes.push_back(g_redoStack.back());
                g_redoStack.pop_back();
                InvalidateOverlay();
            }
            return 0;
        }
        if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam == 'S' || wParam == 'S') {
            CopySnapshotToClipboard();
            return 0;
        }
        if (wParam == 'E') { SetToolMode(ToolMode::Eraser); return 0; }
        if (wParam == 'P') { SetToolMode((g_currentTool == ToolMode::Pan) ? ToolMode::Pen : ToolMode::Pan); return 0; }
        if (wParam == 'M') { SetToolMode((g_currentTool == ToolMode::Pointer) ? ToolMode::Pen : ToolMode::Pointer); return 0; }
        if (wParam == 'H') { SetToolMode(ToolMode::Highlighter); return 0; }
        if (wParam == 'V') { g_inkVisible = !g_inkVisible; InvalidateOverlay(); return 0; }
        if (wParam == 'C') {
            if (!g_strokes.empty()) {
                g_redoStack.insert(g_redoStack.end(), g_strokes.begin(), g_strokes.end());
                g_strokes.clear();
                InvalidateOverlay();
            }
            return 0;
        }
        if (wParam == 'F') { g_currentShape = ShapeType::Freehand; SetToolMode(ToolMode::Pen); return 0; }
        if (wParam == 'L') { g_currentShape = ShapeType::Line; SetToolMode(ToolMode::Pen); return 0; }
        if (wParam == 'A') { g_currentShape = ShapeType::Arrow; SetToolMode(ToolMode::Pen); return 0; }
        if (wParam == 'R') { g_currentShape = ShapeType::Rectangle; SetToolMode(ToolMode::Pen); return 0; }
        if (wParam == 'O') { g_currentShape = ShapeType::Ellipse; SetToolMode(ToolMode::Pen); return 0; }
        if (wParam >= '1' && wParam <= '5') {
            int penIdx = (int)(wParam - '1');
            g_activeColor = kPresetColors[penIdx];
            SetToolMode(ToolMode::Pen);
            return 0;
        }
        if (wParam == VK_OEM_4) { // '['
            g_settings.defaultPenWidth = std::max(1.0f, g_settings.defaultPenWidth - 1.0f);
            g_currentPenWidth = g_settings.defaultPenWidth;
            g_sizePreviewTime = GetTickCount64();
            InvalidateOverlay();
            return 0;
        }
        if (wParam == VK_OEM_6) { // ']'
            g_settings.defaultPenWidth = std::min(60.0f, g_settings.defaultPenWidth + 1.0f);
            g_currentPenWidth = g_settings.defaultPenWidth;
            g_sizePreviewTime = GetTickCount64();
            InvalidateOverlay();
            return 0;
        }
        break;
    }

    case WM_MOUSEMOVE: {
        g_cursorX = (float)GET_X_LPARAM(lParam);
        g_cursorY = (float)GET_Y_LPARAM(lParam);

        // Toolbar Dragging
        if (g_isDraggingToolbar) {
            float dx = g_cursorX - g_toolbarDragStart.x;
            float dy = g_cursorY - g_toolbarDragStart.y;
            g_toolbarCustomX = g_toolbarRect.left + dx;
            g_toolbarCustomY = g_toolbarRect.top + dy;
            g_toolbarDragStart = { (LONG)g_cursorX, (LONG)g_cursorY };

            int vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
            int vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);
            BuildToolbarLayout(vw, vh);
            InvalidateOverlay();
            return 0;
        }

        // Check Radial Menu hover
        if (g_radialActive) {
            float dx = g_cursorX - g_radialX;
            float dy = g_cursorY - g_radialY;
            float dist = std::sqrt(dx * dx + dy * dy);

            RadialTarget oldTarget = g_radialHoverTarget;
            int oldSector = g_radialHoverSector;
            int oldOrb = g_hoveredOrb;

            g_radialHoverTarget = RadialTarget::None;
            g_radialHoverSector = -1;
            g_hoveredOrb = -1;

            if (dist <= 38.0f) {
                g_radialHoverTarget = RadialTarget::Center;
            }
            else if (dist >= 44.0f && dist <= 102.0f) {
                float angle = std::atan2(dy, dx);
                if (angle < 0) angle += 2.0f * 3.14159265f;
                int sector = (int)((angle + 3.14159265f / 8.0f) / (3.14159265f / 4.0f)) % 8;
                g_radialHoverSector = sector;
                g_radialHoverTarget = (RadialTarget)sector;
            }
            else if (dist >= 110.0f && dist <= 146.0f) {
                const float kOrbitalRadius = 126.0f;
                for (size_t i = 0; i < kPresetColorCount; ++i) {
                    float angle = (float)(i * (2.0 * 3.14159265358979323846 / kPresetColorCount) - 3.14159265358979323846 * 0.5);
                    float ox = g_radialX + std::cos(angle) * kOrbitalRadius;
                    float oy = g_radialY + std::sin(angle) * kOrbitalRadius;
                    if (DistanceSq(g_cursorX, g_cursorY, ox, oy) <= 18.0f * 18.0f) {
                        g_hoveredOrb = (int)i;
                        g_radialHoverTarget = RadialTarget::ColorOrb;
                        break;
                    }
                }
            }

            if (oldTarget != g_radialHoverTarget || oldSector != g_radialHoverSector || oldOrb != g_hoveredOrb) {
                InvalidateOverlay();
            }
            return 0;
        }

        // Check Toolbar Button Hover
        int oldBtn = g_hoveredToolbarBtn;
        g_hoveredToolbarBtn = -1;
        if (g_settings.showBottomToolbar &&
            g_cursorX >= g_toolbarRect.left && g_cursorX <= g_toolbarRect.right &&
            g_cursorY >= g_toolbarRect.top && g_cursorY <= g_toolbarRect.bottom) {
            for (size_t i = 0; i < g_toolbarButtons.size(); ++i) {
                const auto& b = g_toolbarButtons[i].rect;
                if (g_cursorX >= b.left && g_cursorX <= b.right &&
                    g_cursorY >= b.top && g_cursorY <= b.bottom) {
                    g_hoveredToolbarBtn = (int)i;
                    break;
                }
            }
        }
        if (oldBtn != g_hoveredToolbarBtn) {
            InvalidateOverlay();
        }

        // Active Left-Click Brush Erase Drag (when in Eraser mode)
        if (g_isLeftClickErasing) {
            EraseBrushAt(g_cursorX, g_cursorY, g_eraserRadius);
            InvalidateOverlay();
            return 0;
        }

        // Active Right-Click Whole-Shape Clear Drag (when in Eraser mode)
        if (g_isRightClickClearing) {
            EraseWholeShapeAt(g_cursorX, g_cursorY, g_eraserRadius);
            InvalidateOverlay();
            return 0;
        }

        // Right-Click Hold Eraser Brush (in normal drawing modes)
        if (g_isRightMouseDown && g_currentTool != ToolMode::Eraser) {
            float distMoved = std::sqrt(DistanceSq(g_cursorX, g_cursorY, (float)g_rightMouseDownPos.x, (float)g_rightMouseDownPos.y));
            if (distMoved > 4.0f || (GetTickCount64() - g_rightMouseDownTime > 120)) {
                g_isRightClickErasing = true;
            }
            if (g_isRightClickErasing) {
                EraseBrushAt(g_cursorX, g_cursorY, g_eraserRadius);
                InvalidateOverlay();
                return 0;
            }
        }

        // Active Panning
        if (g_isPanning) {
            g_panOffsetX += (g_cursorX - g_panStartPos.x);
            g_panOffsetY += (g_cursorY - g_panStartPos.y);
            g_panStartPos = { (LONG)g_cursorX, (LONG)g_cursorY };
            InvalidateOverlay();
            return 0;
        }

        // Active Drawing
        if (g_isDrawing) {
            float adjX = (g_cursorX - g_panOffsetX) / g_zoomScale;
            float adjY = (g_cursorY - g_panOffsetY) / g_zoomScale;

            if (g_currentStroke.shapeType == ShapeType::Freehand) {
                g_currentStroke.points.push_back({ adjX, adjY });
            }
            else {
                g_currentStroke.endPt = { adjX, adjY };
            }
            InvalidateOverlay();
            return 0;
        }

        // When Eraser mode is active or right-click is held down, red circle follows mouse smoothly!
        if (g_currentTool == ToolMode::Eraser || g_isRightMouseDown) {
            InvalidateOverlay();
            return 0;
        }

        break;
    }

    case WM_LBUTTONDOWN: {
        float x = (float)GET_X_LPARAM(lParam);
        float y = (float)GET_Y_LPARAM(lParam);

        // Radial Menu selection
        if (g_radialActive) {
            if (g_radialHoverTarget == RadialTarget::Center) {
                // Toggle between Pen and Highlighter
                g_currentTool = (g_currentTool == ToolMode::Highlighter) ? ToolMode::Pen : ToolMode::Highlighter;
            }
            else if (g_radialHoverTarget == RadialTarget::Clear) {
                if (!g_strokes.empty()) {
                    g_redoStack.insert(g_redoStack.end(), g_strokes.begin(), g_strokes.end());
                    g_strokes.clear();
                }
            }
            else if (g_radialHoverTarget == RadialTarget::Snapshot) {
                CopySnapshotToClipboard();
            }
            else if (g_radialHoverTarget == RadialTarget::Eraser) {
                SetToolMode(ToolMode::Eraser);
            }
            else if (g_radialHoverTarget == RadialTarget::Undo) {
                if (!g_strokes.empty()) {
                    g_redoStack.push_back(g_strokes.back());
                    g_strokes.pop_back();
                }
            }
            else if (g_radialHoverTarget == RadialTarget::Pointer) {
                SetToolMode(ToolMode::Pointer);
            }
            else if (g_radialHoverTarget == RadialTarget::InkVisible) {
                g_inkVisible = !g_inkVisible;
            }
            else if (g_radialHoverTarget == RadialTarget::Pan) {
                SetToolMode(ToolMode::Pan);
            }
            else if (g_radialHoverTarget == RadialTarget::Draw) {
                SetToolMode(ToolMode::Pen);
            }
            else if (g_radialHoverTarget == RadialTarget::ColorOrb && g_hoveredOrb >= 0 && g_hoveredOrb < (int)kPresetColorCount) {
                g_activeColor = kPresetColors[g_hoveredOrb];
                SetToolMode(ToolMode::Pen);
            }

            g_radialActive = false;
            g_radialHoverTarget = RadialTarget::None;
            g_radialHoverSector = -1;
            g_hoveredOrb = -1;
            InvalidateOverlay();
            return 0;
        }

        // Toolbar Drag Handle Check
        if (g_hoveredToolbarBtn == 0 || (x >= g_toolbarRect.left && x <= g_toolbarRect.right && y >= g_toolbarRect.top && y <= g_toolbarRect.bottom && g_hoveredToolbarBtn == -1)) {
            g_isDraggingToolbar = true;
            g_toolbarDragStart = { (LONG)x, (LONG)y };
            SetCapture(hwnd);
            return 0;
        }

        // Toolbar Button Click - verify button under (x, y)
        if (g_settings.showBottomToolbar &&
            x >= g_toolbarRect.left && x <= g_toolbarRect.right &&
            y >= g_toolbarRect.top && y <= g_toolbarRect.bottom) {
            for (size_t i = 0; i < g_toolbarButtons.size(); ++i) {
                const auto& b = g_toolbarButtons[i].rect;
                if (x >= b.left && x <= b.right && y >= b.top && y <= b.bottom) {
                    g_hoveredToolbarBtn = (int)i;
                    break;
                }
            }
        }

        if (g_hoveredToolbarBtn >= 0 && g_hoveredToolbarBtn < (int)g_toolbarButtons.size()) {
            const auto& btn = g_toolbarButtons[g_hoveredToolbarBtn];
            if (btn.isPen) {
                g_activeColor = btn.penColor;
                SetToolMode(ToolMode::Pen);
                SetForegroundWindow(hwnd);
            }
            else {
                switch (btn.id) {
                case 1: // Highlighter
                    SetToolMode((g_currentTool == ToolMode::Highlighter) ? ToolMode::Pen : ToolMode::Highlighter);
                    SetForegroundWindow(hwnd);
                    break;
                case 2: // Eraser
                    SetToolMode((g_currentTool == ToolMode::Eraser) ? ToolMode::Pen : ToolMode::Eraser);
                    SetForegroundWindow(hwnd);
                    break;
                case 3: // Pan
                    SetToolMode((g_currentTool == ToolMode::Pan) ? ToolMode::Pen : ToolMode::Pan);
                    SetForegroundWindow(hwnd);
                    break;
                case 4: // Pointer (Click-Through)
                    SetToolMode((g_currentTool == ToolMode::Pointer) ? ToolMode::Pen : ToolMode::Pointer);
                    break;
                case 5: // Eye Visibility
                    g_inkVisible = !g_inkVisible;
                    break;
                case 20: g_currentShape = ShapeType::Freehand; SetToolMode(ToolMode::Pen); SetForegroundWindow(hwnd); break;
                case 21: g_currentShape = ShapeType::Line; SetToolMode(ToolMode::Pen); SetForegroundWindow(hwnd); break;
                case 22: g_currentShape = ShapeType::Arrow; SetToolMode(ToolMode::Pen); SetForegroundWindow(hwnd); break;
                case 23: g_currentShape = ShapeType::Rectangle; SetToolMode(ToolMode::Pen); SetForegroundWindow(hwnd); break;
                case 24: g_currentShape = ShapeType::Ellipse; SetToolMode(ToolMode::Pen); SetForegroundWindow(hwnd); break;
                case 10: // Snapshot
                    CopySnapshotToClipboard();
                    break;
                case 11: // Undo
                    if (!g_strokes.empty()) {
                        g_redoStack.push_back(g_strokes.back());
                        g_strokes.pop_back();
                    }
                    break;
                case 12: // Redo
                    if (!g_redoStack.empty()) {
                        g_strokes.push_back(g_redoStack.back());
                        g_redoStack.pop_back();
                    }
                    break;
                case 13: // Clear
                    if (!g_strokes.empty()) {
                        g_redoStack.insert(g_redoStack.end(), g_strokes.begin(), g_strokes.end());
                        g_strokes.clear();
                    }
                    break;
                case 99: // Exit
                    HideOverlay();
                    return 0;
                }
            }
            InvalidateOverlay();
            return 0;
        }

        // Pointer mode: clicks pass through to desktop via WM_NCHITTEST
        if (g_currentTool == ToolMode::Pointer) {
            return 0;
        }

        // Pan Drag
        if (g_currentTool == ToolMode::Pan) {
            g_isPanning = true;
            g_panStartPos = { (LONG)x, (LONG)y };
            SetCapture(hwnd);
            return 0;
        }

        // Eraser Tool Left-Click: start Brush Erase Drag
        if (g_currentTool == ToolMode::Eraser) {
            g_isLeftClickErasing = true;
            SetCapture(hwnd);
            EraseBrushAt(x, y, g_eraserRadius);
        }
        else {
            // Start Drawing Stroke or Shape
            SetCapture(hwnd);
            g_isDrawing = true;
            float adjX = (x - g_panOffsetX) / g_zoomScale;
            float adjY = (y - g_panOffsetY) / g_zoomScale;

            g_currentStroke.points.clear();
            g_currentStroke.points.push_back({ adjX, adjY });
            g_currentStroke.startPt = { adjX, adjY };
            g_currentStroke.endPt = { adjX, adjY };
            g_currentStroke.shapeType = g_currentShape;
            g_currentStroke.color = g_activeColor;
            g_currentStroke.isHighlighter = (g_currentTool == ToolMode::Highlighter);
            g_currentStroke.width = g_currentStroke.isHighlighter ? g_settings.defaultHighlighterWidth : g_settings.defaultPenWidth;
            g_redoStack.clear();
        }

        InvalidateOverlay();
        return 0;
    }

    case WM_LBUTTONUP: {
        if (g_isLeftClickErasing) {
            ReleaseCapture();
            g_isLeftClickErasing = false;
            InvalidateOverlay();
            return 0;
        }

        if (g_isDraggingToolbar) {
            ReleaseCapture();
            g_isDraggingToolbar = false;
            return 0;
        }

        if (g_isPanning) {
            ReleaseCapture();
            g_isPanning = false;
            return 0;
        }

        if (g_isDrawing) {
            ReleaseCapture();
            g_isDrawing = false;
            if (g_currentStroke.shapeType == ShapeType::Freehand) {
                if (g_currentStroke.points.size() > 1) {
                    g_strokes.push_back(g_currentStroke);
                }
            }
            else {
                if (DistanceSq(g_currentStroke.startPt.x, g_currentStroke.startPt.y, g_currentStroke.endPt.x, g_currentStroke.endPt.y) > 4.0f) {
                    g_strokes.push_back(g_currentStroke);
                }
            }
            InvalidateOverlay();
        }
        return 0;
    }

    case WM_RBUTTONDOWN: {
        float x = (float)GET_X_LPARAM(lParam);
        float y = (float)GET_Y_LPARAM(lParam);

        // In Eraser Mode: Right-Click clears whole shape under cursor!
        if (g_currentTool == ToolMode::Eraser) {
            g_isRightClickClearing = true;
            SetCapture(hwnd);
            EraseWholeShapeAt(x, y, g_eraserRadius);
            InvalidateOverlay();
            return 0;
        }

        // In Normal Mode: Right-Click hold starts Brush Erase, tap opens radial menu
        g_isRightMouseDown = true;
        g_isRightClickErasing = false;
        g_wheelUsedWhileRightMouseDown = false;
        g_rightMouseDownPos.x = (LONG)x;
        g_rightMouseDownPos.y = (LONG)y;
        g_rightMouseDownTime = GetTickCount64();
        SetCapture(hwnd);
        InvalidateOverlay(); // Shows red eraser cursor immediately
        return 0;
    }

    case WM_RBUTTONUP: {
        if (g_isRightClickClearing) {
            ReleaseCapture();
            g_isRightClickClearing = false;
            InvalidateOverlay();
            return 0;
        }

        if (g_isRightMouseDown) {
            ReleaseCapture();
            g_isRightMouseDown = false;

            if (!g_isRightClickErasing && !g_wheelUsedWhileRightMouseDown) {
                // Quick right-click tap: Open Radial Menu!
                g_radialActive = true;
                g_radialX = (float)GET_X_LPARAM(lParam);
                g_radialY = (float)GET_Y_LPARAM(lParam);
                g_radialHoverTarget = RadialTarget::None;
                g_radialHoverSector = -1;
                g_hoveredOrb = -1;
            }
            g_isRightClickErasing = false;
            g_wheelUsedWhileRightMouseDown = false;
            InvalidateOverlay();
        }
        return 0;
    }

    case WM_NCHITTEST: {
        if (g_currentTool == ToolMode::Pointer) {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ScreenToClient(hwnd, &pt);
            if (g_settings.showBottomToolbar &&
                pt.x >= (g_toolbarRect.left - 6.0f) && pt.x <= (g_toolbarRect.right + 6.0f) &&
                pt.y >= (g_toolbarRect.top - 6.0f) && pt.y <= (g_toolbarRect.bottom + 6.0f)) {
                return HTCLIENT;
            }
            return HTTRANSPARENT;
        }
        return HTCLIENT;
    }

    case WM_SETCURSOR: {
        if (LOWORD(lParam) == HTCLIENT) {
            if (g_currentTool == ToolMode::Pan) {
                SetCursor(LoadCursor(NULL, IDC_SIZEALL));
                return TRUE;
            }
            if (g_currentTool == ToolMode::Eraser || g_isRightClickErasing || g_isRightMouseDown || g_isLeftClickErasing || g_isRightClickClearing) {
                SetCursor(LoadCursor(NULL, IDC_CROSS));
                return TRUE;
            }
            if (g_currentTool == ToolMode::Pointer) {
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                return TRUE;
            }
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hwnd, &pt);
            if (g_settings.showBottomToolbar &&
                pt.x >= g_toolbarRect.left && pt.x <= g_toolbarRect.right &&
                pt.y >= g_toolbarRect.top && pt.y <= g_toolbarRect.bottom) {
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                return TRUE;
            }
            SetCursor(LoadCursor(NULL, IDC_CROSS));
            return TRUE;
        }
        break;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_DESTROY:
        ReleaseD2DResources();
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ----------------------------------------------------------------------------
// Overlay Show / Hide Management
// ----------------------------------------------------------------------------

void ShowOverlay() {
    if (g_bIsActive) return;

    int vx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int vy = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    if (!g_hOverlayWnd) {
        WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
        wc.lpfnWndProc = OverlayWndProc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.lpszClassName = L"WindhawkNativeScreenInkOverlay";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        RegisterClassExW(&wc);

        g_hOverlayWnd = CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
            wc.lpszClassName,
            L"Screen Inking Overlay",
            WS_POPUP,
            vx, vy, vw, vh,
            NULL, NULL, wc.hInstance, NULL
        );

        CreateD2DResources(g_hOverlayWnd);
    }
    else {
        SetWindowPos(g_hOverlayWnd, HWND_TOPMOST, vx, vy, vw, vh, SWP_SHOWWINDOW);
        CreateD2DResources(g_hOverlayWnd);
        CaptureDesktop();
    }

    BuildToolbarLayout(vw, vh);

    ShowWindow(g_hOverlayWnd, SW_SHOW);
    SetForegroundWindow(g_hOverlayWnd);
    SetFocus(g_hOverlayWnd);
    g_bIsActive = true;
    SetToolMode(ToolMode::Pen);

    InvalidateOverlay();
}

void HideOverlay() {
    if (!g_bIsActive) return;
    g_bIsActive = false;
    g_radialActive = false;
    g_isDrawing = false;
    g_isPanning = false;
    g_isRightMouseDown = false;
    g_isRightClickErasing = false;
    g_isLeftClickErasing = false;
    g_isRightClickClearing = false;
    g_wheelUsedWhileRightMouseDown = false;

    if (g_hOverlayWnd) {
        KillTimer(g_hOverlayWnd, 1);
        KillTimer(g_hOverlayWnd, 2);
        LONG_PTR exStyle = GetWindowLongPtr(g_hOverlayWnd, GWL_EXSTYLE);
        if (exStyle & (WS_EX_LAYERED | WS_EX_TRANSPARENT)) {
            SetWindowLongPtr(g_hOverlayWnd, GWL_EXSTYLE, exStyle & ~(WS_EX_LAYERED | WS_EX_TRANSPARENT));
            SetWindowPos(g_hOverlayWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        }
        ShowWindow(g_hOverlayWnd, SW_HIDE);
    }
    g_zoomPreviewTime = 0;
    g_sizePreviewTime = 0;
    g_currentTool = ToolMode::Pen;
}

// ----------------------------------------------------------------------------
// Hotkey Background Message Loop
// ----------------------------------------------------------------------------

static const int kHotkeyId = 1042;

LRESULT CALLBACK HotkeyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_HOTKEY && wParam == kHotkeyId) {
        if (g_bIsActive) {
            // Toggling hotkey while active switches between click-through Mouse Pointer and Inking
            if (g_hOverlayWnd) {
                PostMessageW(g_hOverlayWnd, WM_USER_TOGGLE_POINTER, 0, 0);
            }
        }
        else {
            ShowOverlay();
        }
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

DWORD WINAPI HotkeyThread(LPVOID) {
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc = HotkeyWndProc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = L"WindhawkNativeInkHotkeyReceiver";
    RegisterClassExW(&wc);

    g_hHotkeyWnd = CreateWindowExW(
        0, wc.lpszClassName, L"HotkeyReceiver",
        0, 0, 0, 0, 0,
        HWND_MESSAGE, NULL, wc.hInstance, NULL
    );

    RegisterHotKey(g_hHotkeyWnd, kHotkeyId, g_settings.hotkeyMod, g_settings.hotkeyKey);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnregisterHotKey(g_hHotkeyWnd, kHotkeyId);
    DestroyWindow(g_hHotkeyWnd);
    return 0;
}

// ----------------------------------------------------------------------------
// Windhawk Mod Lifecycle
// ----------------------------------------------------------------------------

BOOL Wh_ModInit() {
    Wh_Log(L"WinDraw: Initializing");

    LoadSettings();

    CoInitialize(NULL);

    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pD2DFactory);
    if (FAILED(hr)) {
        Wh_Log(L"Failed to create Direct2D Factory");
        return FALSE;
    }

    DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&g_pDWriteFactory)
    );

    if (g_pDWriteFactory) {
        const wchar_t* iconFont = GetIconFontFamilyName();

        // General Text Format
        g_pDWriteFactory->CreateTextFormat(
            L"Segoe UI Variable Display",
            NULL,
            DWRITE_FONT_WEIGHT_SEMI_BOLD,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            13.0f,
            L"en-us",
            &g_pTextFormat
        );
        if (g_pTextFormat) {
            g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            g_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        }

        // Toolbar Icon Format (15.0f Segoe Fluent Icons)
        g_pDWriteFactory->CreateTextFormat(
            iconFont,
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            15.0f,
            L"en-us",
            &g_pIconFormat
        );
        if (g_pIconFormat) {
            g_pIconFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            g_pIconFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        }

        // Radial Menu Icon Format (17.0f Segoe Fluent Icons)
        g_pDWriteFactory->CreateTextFormat(
            iconFont,
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            17.0f,
            L"en-us",
            &g_pRadialIconFormat
        );
        if (g_pRadialIconFormat) {
            g_pRadialIconFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            g_pRadialIconFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        }

        // Radial Center Badge Format (14.0f Segoe Fluent Icons)
        g_pDWriteFactory->CreateTextFormat(
            iconFont,
            NULL,
            DWRITE_FONT_WEIGHT_SEMI_BOLD,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            14.0f,
            L"en-us",
            &g_pCenterBadgeFormat
        );
        if (g_pCenterBadgeFormat) {
            g_pCenterBadgeFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            g_pCenterBadgeFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        }
    }

    CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&g_pWICFactory)
    );

    CreateThread(NULL, 0, HotkeyThread, NULL, 0, NULL);

    Wh_Log(L"WinDraw: Ready (All features active. Press Hotkey to annotate)");
    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L"WinDraw: Unloading");

    HideOverlay();

    if (g_hHotkeyWnd) {
        PostMessage(g_hHotkeyWnd, WM_QUIT, 0, 0);
    }

    if (g_hOverlayWnd) {
        DestroyWindow(g_hOverlayWnd);
        g_hOverlayWnd = NULL;
    }

    ReleaseD2DResources();

    if (g_pWICFactory) { g_pWICFactory->Release(); g_pWICFactory = nullptr; }
    if (g_pCenterBadgeFormat) { g_pCenterBadgeFormat->Release(); g_pCenterBadgeFormat = nullptr; }
    if (g_pRadialIconFormat) { g_pRadialIconFormat->Release(); g_pRadialIconFormat = nullptr; }
    if (g_pIconFormat) { g_pIconFormat->Release(); g_pIconFormat = nullptr; }
    if (g_pTextFormat) { g_pTextFormat->Release(); g_pTextFormat = nullptr; }
    if (g_pDWriteFactory) { g_pDWriteFactory->Release(); g_pDWriteFactory = nullptr; }
    if (g_pD2DFactory) { g_pD2DFactory->Release(); g_pD2DFactory = nullptr; }

    CoUninitialize();
}

void Wh_ModSettingsChanged() {
    Wh_Log(L"WinDraw: Settings Changed");
    LoadSettings();

    if (g_hHotkeyWnd) {
        UnregisterHotKey(g_hHotkeyWnd, kHotkeyId);
        RegisterHotKey(g_hHotkeyWnd, kHotkeyId, g_settings.hotkeyMod, g_settings.hotkeyKey);
    }
}
