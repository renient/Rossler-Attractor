#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstdio>

const unsigned int WINDOW_WIDTH  = 1440;
const unsigned int WINDOW_HEIGHT = 820;

const int   TOTAL_POINTS = 1000000;
const int   DRAW_POINTS  = 400000;
const float DT           = 0.005f;
const float SCALE        = 0.18f;

const float A = 0.2f;
const float B = 0.2f;
const float C = 5.7f;

struct Vec3 { float x, y, z; };

std::vector<Vec3> g_points;
std::vector<Vec3> g_raw;
HDC    g_hdc     = NULL;
bool   g_running = true;
GLuint g_fontList       = 0;
GLuint g_fontListLarge  = 0;

Vec3 rosslerStep(const Vec3 &p, float dt) {
    auto f = [](const Vec3 &v) -> Vec3 {
        return { -v.y - v.z, v.x + A * v.y, B + v.z * (v.x - C) };
    };
    Vec3 k1 = f(p);
    Vec3 k2 = f({p.x+0.5f*dt*k1.x, p.y+0.5f*dt*k1.y, p.z+0.5f*dt*k1.z});
    Vec3 k3 = f({p.x+0.5f*dt*k2.x, p.y+0.5f*dt*k2.y, p.z+0.5f*dt*k2.z});
    Vec3 k4 = f({p.x+dt*k3.x,      p.y+dt*k3.y,      p.z+dt*k3.z});
    return {
        p.x + dt/6.0f*(k1.x+2*k2.x+2*k3.x+k4.x),
        p.y + dt/6.0f*(k1.y+2*k2.y+2*k3.y+k4.y),
        p.z + dt/6.0f*(k1.z+2*k2.z+2*k3.z+k4.z)
    };
}

void initAttractor() {
    g_points.resize(TOTAL_POINTS);
    g_raw.resize(TOTAL_POINTS);

    Vec3 p{0.1f, 0.1f, 0.1f};
    for (int i = 0; i < 5000; ++i) p = rosslerStep(p, DT);

    Vec3 avg = {0,0,0};
    for (int i = 0; i < TOTAL_POINTS; ++i) {
        p = rosslerStep(p, DT);
        g_raw[i] = p;
        avg.x += p.x; avg.y += p.y; avg.z += p.z;
    }
    avg.x /= TOTAL_POINTS;
    avg.y /= TOTAL_POINTS;
    avg.z /= TOTAL_POINTS;

    for (int i = 0; i < TOTAL_POINTS; ++i) {
        float r1 = ((float)(rand()%1000)/500.0f - 1.0f) * 0.2f;
        float r2 = ((float)(rand()%1000)/500.0f - 1.0f) * 0.2f;
        float r3 = ((float)(rand()%1000)/500.0f - 1.0f) * 0.2f;
        g_points[i].x = (g_raw[i].x - avg.x + r1) * SCALE;
        g_points[i].y = (g_raw[i].y - avg.y + r2) * SCALE;
        g_points[i].z = (g_raw[i].z - avg.z + r3) * SCALE;
    }
}

void initFont() {
    g_fontList = glGenLists(96);
    HFONT f1 = CreateFontA(-13, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
                           OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                           FF_DONTCARE|DEFAULT_PITCH, "Consolas");
    SelectObject(g_hdc, f1);
    wglUseFontBitmaps(g_hdc, 32, 96, g_fontList);
    DeleteObject(f1);

    g_fontListLarge = glGenLists(96);
    HFONT f2 = CreateFontA(-15, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
                           OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                           FF_DONTCARE|DEFAULT_PITCH, "Consolas");
    SelectObject(g_hdc, f2);
    wglUseFontBitmaps(g_hdc, 32, 96, g_fontListLarge);
    DeleteObject(f2);
}

void drawText(float x, float y, const char *text) {
    glRasterPos2f(x, y);
    glListBase(g_fontList - 32);
    glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);
}

void drawTextLarge(float x, float y, const char *text) {
    glRasterPos2f(x, y);
    glListBase(g_fontListLarge - 32);
    glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);
}

void setup3DProjection(int width, int height, float timeSec) {
    float aspect = (height == 0) ? 1.0f : float(width) / float(height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float fov  = 45.0f * 3.14159f / 180.0f;
    float f    = 1.0f / std::tan(fov * 0.5f);
    float znear = 0.1f, zfar = 100.0f;
    float proj[16] = {0};
    proj[0]  = f / aspect;
    proj[5]  = f;
    proj[10] = (zfar + znear) / (znear - zfar);
    proj[11] = -1.0f;
    proj[14] = (2.0f * zfar * znear) / (znear - zfar);
    glLoadMatrixf(proj);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float angle  = 0.5f + 0.05f * std::sin(timeSec * 0.15f);
    float camH   = 2.5f + 0.5f  * std::cos(timeSec * 0.2f);
    float radius = 8.0f;
    float ex = radius * std::cos(angle), ey = camH, ez = radius * std::sin(angle);
    float fx = -ex, fy = -ey, fz = -ez;
    float fl = std::sqrt(fx*fx + fy*fy + fz*fz);
    fx /= fl; fy /= fl; fz /= fl;
    float sx = fy*0 - fz*1, sy = fz*0 - fx*0, sz = fx*1 - fy*0;
    float sl = std::sqrt(sx*sx + sy*sy + sz*sz);
    sx /= sl; sy /= sl; sz /= sl;
    float ux = sy*fz - sz*fy, uy = sz*fx - sx*fz, uz = sx*fy - sy*fx;
    float m[16] = {0};
    m[0]=sx; m[4]=sy; m[8]=sz;
    m[1]=ux; m[5]=uy; m[9]=uz;
    m[2]=-fx; m[6]=-fy; m[10]=-fz;
    m[15]=1.0f;
    glMultMatrixf(m);
    glTranslatef(-ex, -ey, -ez);
}

void renderSpiralGraph(float gx, float gy, float gsW, float gsH) {
    float xmin = 1e9f, xmax = -1e9f, ymin = 1e9f, ymax = -1e9f;
    int samples = 8000;
    for (int i = 0; i < samples; i++) {
        if (g_raw[i].x < xmin) xmin = g_raw[i].x;
        if (g_raw[i].x > xmax) xmax = g_raw[i].x;
        if (g_raw[i].y < ymin) ymin = g_raw[i].y;
        if (g_raw[i].y > ymax) ymax = g_raw[i].y;
    }
    float rx = xmax - xmin, ry = ymax - ymin;
    float margin = 30.0f;

    auto toSX = [&](float v) { return gx + margin + (v - xmin) / rx * (gsW - 2*margin); };
    auto toSY = [&](float v) { return gy + margin + (1.0f - (v - ymin) / ry) * (gsH - 2*margin); };

    glColor4f(0.0f, 0.25f, 0.9f, 0.75f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < samples; i += 2) {
        glVertex2f(toSX(g_raw[i].x), toSY(g_raw[i].y));
    }
    glEnd();

    glColor4f(0.9f, 0.05f, 0.15f, 0.75f);
    glBegin(GL_LINE_STRIP);
    for (int i = 1; i < samples; i += 2) {
        glVertex2f(toSX(g_raw[i].x), toSY(g_raw[i].y));
    }
    glEnd();

    glColor4f(0.8f, 0.0f, 0.15f, 0.9f);
    float rx0 = toSX(xmax), ry0 = toSY(ymax);
    float rx1 = toSX(xmax * 0.75f), ry1 = toSY(ymax * 0.7f);
    float rx2 = toSX(xmin * 0.5f),  ry2 = toSY(0);
    float rx3 = toSX(xmax * 0.3f),  ry3 = toSY(ymin * 0.4f);

    glBegin(GL_LINES);
    glVertex2f(rx0, ry0); glVertex2f(rx0 + 2, ry0 - 15);
    glVertex2f(rx1, ry1); glVertex2f(rx1 + 2, ry1 - 15);
    glEnd();
    glColor4f(1.0f, 0.15f, 0.2f, 1.0f);
    drawText(rx0 + 4, ry0 - 12, "105.986");
    drawText(rx1 + 4, ry1 - 12, "74.789");

    glColor4f(0.15f, 0.4f, 1.0f, 0.9f);
    glBegin(GL_LINES);
    glVertex2f(rx2, ry2); glVertex2f(rx2 - 2, ry2 - 15);
    glVertex2f(rx3, ry3); glVertex2f(rx3, ry3 + 15);
    glEnd();
    glColor4f(0.3f, 0.6f, 1.0f, 1.0f);
    drawText(rx2 - 55, ry2 - 12, "63.052");
    drawText(rx3 - 30, ry3 + 20, "26.662");
    drawText(toSX(xmin), toSY(ymin*0.2f), "44.436");
    drawText(toSX(xmin*0.3f), toSY(ymax*0.4f), "-20.415");
}

void renderEquationBlock(float x, float y, float w, float h) {
    glColor4f(0.12f, 0.12f, 0.22f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x, y+h);
    glEnd();

    glColor4f(0.5f, 0.55f, 0.75f, 0.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y); glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x, y+h);
    glEnd();

    float bx = x + 18;
    float by = y + h * 0.5f - 35;
    float bh = 70.0f;
    glColor4f(0.7f, 0.75f, 1.0f, 0.9f);
    glLineWidth(1.6f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(bx + 8, by - 4);
    glVertex2f(bx + 4, by);
    glVertex2f(bx + 4, by + bh*0.5f - 2);
    glVertex2f(bx,     by + bh*0.5f);
    glVertex2f(bx + 4, by + bh*0.5f + 2);
    glVertex2f(bx + 4, by + bh - 0);
    glVertex2f(bx + 8, by + bh + 4);
    glEnd();
    glLineWidth(1.0f);

    float tx = bx + 22;
    glColor4f(0.85f, 0.88f, 1.0f, 1.0f);
    drawTextLarge(tx, by + 8,  "dx/dt  =  -y - z");
    drawTextLarge(tx, by + 33, "dy/dt  =  x + ay");
    drawTextLarge(tx, by + 58, "dz/dt  =  b + z(x - c)");

    char buf[64];
    glColor4f(0.5f, 0.6f, 0.9f, 0.8f);
    sprintf(buf, "a = %.1f   b = %.1f   c = %.1f", A, B, C);
    drawText(x + 18, y + h - 16, buf);
}

void renderSidebar(int sx, int sy, int sw, int sh) {
    glViewport(sx, sy, sw, sh);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, sw, sh, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_BLEND);
    glColor4f(0.02f, 0.02f, 0.04f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(sw, 0);
    glVertex2f(sw, sh); glVertex2f(0, sh);
    glEnd();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.3f, 0.35f, 0.6f, 0.5f);
    glBegin(GL_LINES);
    glVertex2f(0, 0); glVertex2f(0, sh);
    glEnd();

    float graphH   = sh * 0.65f;
    float equationH = sh * 0.3f;
    float pad = 12.0f;

    glColor4f(0.8f, 0.85f, 1.0f, 0.95f);
    drawText(pad, 20, "ROSSLER ATTRACTOR  -  PHASE PORTRAIT");

    glColor4f(0.3f, 0.35f, 0.55f, 0.6f);
    glBegin(GL_LINES);
    glVertex2f(pad, 26); glVertex2f(sw - pad, 26);
    glEnd();

    renderSpiralGraph(pad, 32, sw - 2*pad, graphH - 10);

    float eqY = graphH + 14;
    renderEquationBlock(pad, eqY, sw - 2*pad, equationH);
}

void renderFrame(float timeSec, int width, int height) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int sidebarW = 420;
    int mainW    = width - sidebarW;

    glViewport(0, 0, mainW, height);
    setup3DProjection(mainW, height, timeSec);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_POINT_SMOOTH);

    int offset = (int)(timeSec * 180.0f) % (TOTAL_POINTS - DRAW_POINTS);

    glPointSize(1.15f);
    glBegin(GL_POINTS);
    for (int i = 0; i < DRAW_POINTS; ++i) {
        if (i % 900 == 0) continue;
        int idx = offset + i;
        float fv = (float)(idx % TOTAL_POINTS) / TOTAL_POINTS;
        float r  = 0.5f + 0.5f * std::sin(fv * 6.28f);
        float b  = 0.5f + 0.5f * std::cos(fv * 6.28f);
        float pulse = 0.6f + 0.4f * std::sin(timeSec * 2.0f + fv * 10.0f);
        glColor4f(r * 0.85f, 0.25f, b, 0.13f * pulse);
        glVertex3f(g_points[idx].x, g_points[idx].y, g_points[idx].z);
    }
    glEnd();

    glPointSize(2.5f);
    glBegin(GL_POINTS);
    for (int i = 0; i < DRAW_POINTS; i += 700) {
        int idx = offset + i;
        float pulse = 0.7f + 0.3f * std::sin(timeSec * 5.0f + i * 0.1f);
        glColor4f(1.0f, 1.0f, 1.0f, 0.4f * pulse);
        glVertex3f(g_points[idx].x, g_points[idx].y, g_points[idx].z);
    }
    glEnd();

    renderSidebar(mainW, 0, sidebarW, height);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CLOSE:   DestroyWindow(hWnd); return 0;
    case WM_DESTROY: g_running = false; PostQuitMessage(0); return 0;
    default:         return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style         = CS_OWNDC;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = TEXT("RosslerWin");
    if (!RegisterClass(&wc)) return 0;

    RECT rect = {0, 0, (LONG)WINDOW_WIDTH, (LONG)WINDOW_HEIGHT};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindow(wc.lpszClassName, TEXT("Rossler Attractor"),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);
    if (!hwnd) return 0;

    g_hdc = GetDC(hwnd);

    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize      = sizeof(pfd);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pf = ChoosePixelFormat(g_hdc, &pfd);
    SetPixelFormat(g_hdc, pf, &pfd);

    HGLRC rc = wglCreateContext(g_hdc);
    wglMakeCurrent(g_hdc, rc);

    srand((unsigned int)time(NULL));
    initAttractor();
    initFont();

    LARGE_INTEGER freq, start;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    while (g_running) {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { g_running = false; break; }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!g_running) break;

        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        float t = float(now.QuadPart - start.QuadPart) / float(freq.QuadPart);

        RECT cr;
        GetClientRect(hwnd, &cr);
        renderFrame(t, cr.right - cr.left, cr.bottom - cr.top);
        SwapBuffers(g_hdc);
    }

    glDeleteLists(g_fontList, 96);
    glDeleteLists(g_fontListLarge, 96);
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(rc);
    ReleaseDC(hwnd, g_hdc);

    return 0;
}
