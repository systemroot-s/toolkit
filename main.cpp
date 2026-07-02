#include <windows.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>

HWND g_hWnd = NULL;
HINSTANCE g_hInst;

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(s);
    while (std::getline(iss, token, delim)) {
        if (!token.empty()) tokens.push_back(token);
    }
    return tokens;
}

// 转大写，用于不区分大小写比较
std::string toUpper(std::string s) {
    for (size_t i = 0; i < s.size(); i++)
        s[i] = toupper(s[i]);
    return s;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

void createWindow(int w, int h) {
    const char CLASS_NAME[] = "LangScriptClass";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = g_hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClass(&wc);
    
    g_hWnd = CreateWindowEx(
        0, CLASS_NAME, "LS Runtime",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, w, h,
        NULL, NULL, g_hInst, NULL
    );
    
    ShowWindow(g_hWnd, SW_SHOW);
    UpdateWindow(g_hWnd);
}

void setTitle(const char* title) {
    if (g_hWnd) SetWindowText(g_hWnd, title);
}

void showBox(const char* text) {
    MessageBox(NULL, text, "Message", MB_OK | MB_ICONINFORMATION);
}

void drawText(int x, int y, const char* text) {
    if (!g_hWnd) return;
    HDC hdc = GetDC(g_hWnd);
    TextOut(hdc, x, y, text, strlen(text));
    ReleaseDC(g_hWnd, hdc);
}

void runScript(const char* filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        MessageBox(NULL, "File is corrupted", "Error", MB_OK | MB_ICONERROR);
        return;
    }
    
    std::string firstLine;
    std::getline(file, firstLine);
    
    if (!firstLine.empty() && firstLine.back() == '\r')
        firstLine.pop_back();
    
    // 魔数改为 LS，大小写不敏感
    if (toUpper(firstLine) != "LS") {
        MessageBox(NULL, "File is corrupted", "Error", MB_OK | MB_ICONERROR);
        file.close();
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        
        if (line.empty() || line[0] == '#')
            continue;
        
        std::vector<std::string> parts = split(line, ' ');
        if (parts.empty()) continue;
        
        std::string cmd = parts[0];
        
        if (cmd == "wind" && parts.size() >= 3) {
            int w = atoi(parts[1].c_str());
            int h = atoi(parts[2].c_str());
            createWindow(w, h);
        }
        else if (cmd == "titl") {
            std::string rest;
            for (size_t i = 1; i < parts.size(); i++) {
                if (i > 1) rest += " ";
                rest += parts[i];
            }
            setTitle(rest.c_str());
        }
        else if (cmd == "box") {
            std::string rest;
            for (size_t i = 1; i < parts.size(); i++) {
                if (i > 1) rest += " ";
                rest += parts[i];
            }
            showBox(rest.c_str());
        }
        else if (cmd == "text" && parts.size() >= 4) {
            int x = atoi(parts[1].c_str());
            int y = atoi(parts[2].c_str());
            std::string rest;
            for (size_t i = 3; i < parts.size(); i++) {
                if (i > 3) rest += " ";
                rest += parts[i];
            }
            drawText(x, y, rest.c_str());
        }
        else if (cmd == "wait" && parts.size() >= 2) {
            int ms = atoi(parts[1].c_str());
            Sleep(ms);
        }
        else if (cmd == "exit") {
            break;
        }
    }
    
    file.close();
    
    if (g_hWnd) {
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nShow) {
    g_hInst = hInstance;
    
    if (strlen(lpCmdLine) == 0) {
        MessageBox(NULL, "Usage: Open a .ls file", "LS Interpreter", MB_OK);
        return 0;
    }
    
    std::string path = lpCmdLine;
    if (!path.empty() && path.front() == '"') path.erase(0, 1);
    if (!path.empty() && path.back() == '"') path.pop_back();
    
    runScript(path.c_str());
    return 0;
}
