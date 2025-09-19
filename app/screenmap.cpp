#include <windows.h>
#include <iostream>
#include <vector>

struct ScreenInfo {
    int x, y, w, h;
    int resW, resH;
};

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    MONITORINFOEX mi = {};
    mi.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(hMonitor, &mi);

    DEVMODE dm = {};
    dm.dmSize = sizeof(DEVMODE);
    EnumDisplaySettings(mi.szDevice, ENUM_CURRENT_SETTINGS, &dm);

    std::vector<ScreenInfo>* screens = (std::vector<ScreenInfo>*)dwData;

    ScreenInfo screen;
    screen.x = mi.rcMonitor.left;
    screen.y = mi.rcMonitor.top;
    screen.w = mi.rcMonitor.right - mi.rcMonitor.left;
    screen.h = mi.rcMonitor.bottom - mi.rcMonitor.top;
    screen.resW = dm.dmPelsWidth;
    screen.resH = dm.dmPelsHeight;

    screens->push_back(screen);

    printf("Ecran %d: x=%d y=%d w=%d h=%d res=%dx%d\n",
           (int)screens->size(),
           screen.x, screen.y, screen.w, screen.h,
           screen.resW, screen.resH);

    return TRUE;
}

int main()
{
    std::vector<ScreenInfo> screens;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&screens);

    // Calculer les dimensions totales avec résolution réelle
    int minX = 0, minY = 0, maxX = 0, maxY = 0;
    for (const auto& screen : screens) {
        int realX = screen.x;
        int realY = screen.y;
        int realW = screen.resW;
        int realH = screen.resH;

        if (realX < minX) minX = realX;
        if (realY < minY) minY = realY;
        if (realX + realW > maxX) maxX = realX + realW;
        if (realY + realH > maxY) maxY = realY + realH;
    }

    int totalW = maxX - minX;
    int totalH = maxY - minY;
    printf("\nZone totale réelle: %dx%d offset=(%d,%d)\n", totalW, totalH, -minX, -minY);

    // Réduire la taille pour les tests (diviser par 4)
    int scale = 4;
    totalW /= scale;
    totalH /= scale;
    printf("Zone réduite pour test: %dx%d (échelle 1/%d)\n", totalW, totalH, scale);

    // Créer bitmap
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, totalW, totalH);
    SelectObject(memDC, hBitmap);

    // Fond noir
    HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    RECT totalRect = {0, 0, totalW, totalH};
    FillRect(memDC, &totalRect, blackBrush);
    DeleteObject(blackBrush);

    // Couleurs pour chaque écran
    COLORREF colors[] = {RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 0, 255)};

    for (int i = 0; i < screens.size(); i++) {
        HBRUSH brush = CreateSolidBrush(colors[i % 3]);

        RECT rect = {
            (screens[i].x - minX) / scale,
            (screens[i].y - minY) / scale,
            (screens[i].x - minX + screens[i].resW) / scale,
            (screens[i].y - minY + screens[i].resH) / scale
        };
        FillRect(memDC, &rect, brush);
        DeleteObject(brush);
    }

    // Sauver en BMP avec alignement correct
    int rowPadding = (4 - (totalW * 3) % 4) % 4;
    int rowSize = totalW * 3 + rowPadding;
    DWORD imageSize = rowSize * totalH;

    BITMAPINFOHEADER bi = {};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = totalW;
    bi.biHeight = totalH; // Bottom-up
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = imageSize;

    BYTE* imageData = new BYTE[imageSize];
    GetDIBits(hdc, hBitmap, 0, totalH, imageData, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    FILE* file = fopen("test_mapping.bmp", "wb");
    if (file) {
        BITMAPFILEHEADER bf = {};
        bf.bfType = 0x4D42; // "BM"
        bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + imageSize;
        bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, file);
        fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, file);
        fwrite(imageData, imageSize, 1, file);
        fclose(file);
        printf("Fichier test_mapping.bmp créé (taille: %d octets)\n", bf.bfSize);
    }

    delete[] imageData;
    DeleteObject(hBitmap);
    DeleteDC(memDC);
    ReleaseDC(NULL, hdc);

    system("pause");
    return 0;
}