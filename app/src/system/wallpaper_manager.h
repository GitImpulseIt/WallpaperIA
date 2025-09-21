#pragma once

#ifdef Q_OS_WIN
#include <windows.h>
#include <shlobj.h>
#include <objbase.h>
#include <objidl.h>
#include <shlguid.h>
#include <shobjidl.h>
#include <wingdi.h>

// Définitions pour IDesktopWallpaper si non disponibles dans MinGW
#ifndef __IDesktopWallpaper_INTERFACE_DEFINED__
#define __IDesktopWallpaper_INTERFACE_DEFINED__

DEFINE_GUID(CLSID_DesktopWallpaper, 0xC2CF3110, 0x460E, 0x4fc1, 0xB9, 0xD0, 0x8A, 0x1C, 0x0C, 0x9C, 0xC4, 0xBD);

// Énumérations pour les positions de fond d'écran
typedef enum DESKTOP_WALLPAPER_POSITION {
    DWPOS_CENTER = 0,
    DWPOS_TILE = 1,
    DWPOS_STRETCH = 2,
    DWPOS_FIT = 3,
    DWPOS_FILL = 4,
    DWPOS_SPAN = 5
} DESKTOP_WALLPAPER_POSITION;

interface IDesktopWallpaper : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE SetWallpaper(LPCWSTR monitorID, LPCWSTR wallpaper) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetWallpaper(LPCWSTR monitorID, LPWSTR *wallpaper) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMonitorDevicePathAt(UINT monitorIndex, LPWSTR *monitorID) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMonitorDevicePathCount(UINT *count) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMonitorRECT(LPCWSTR monitorID, RECT *displayRect) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetBackgroundColor(COLORREF color) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetBackgroundColor(COLORREF *color) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetPosition(DESKTOP_WALLPAPER_POSITION position) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetPosition(DESKTOP_WALLPAPER_POSITION *position) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetSlideshow(IShellItemArray *items) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetSlideshow(IShellItemArray **items) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetSlideshowOptions(DESKTOP_SLIDESHOW_OPTIONS options, UINT slideshowTick) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetSlideshowOptions(DESKTOP_SLIDESHOW_OPTIONS *options, UINT *slideshowTick) = 0;
    virtual HRESULT STDMETHODCALLTYPE AdvanceSlideshow(LPCWSTR monitorID, DESKTOP_SLIDESHOW_DIRECTION direction) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetStatus(DESKTOP_SLIDESHOW_STATE *state) = 0;
    virtual HRESULT STDMETHODCALLTYPE Enable(BOOL enable) = 0;
};

// Énumérations supplémentaires pour le slideshow
typedef enum DESKTOP_SLIDESHOW_OPTIONS {
    DSO_SHUFFLEIMAGES = 0x01
} DESKTOP_SLIDESHOW_OPTIONS;

typedef enum DESKTOP_SLIDESHOW_STATE {
    DSS_ENABLED = 0x01,
    DSS_SLIDESHOW = 0x02,
    DSS_DISABLED_BY_REMOTE_SESSION = 0x04
} DESKTOP_SLIDESHOW_STATE;

typedef enum DESKTOP_SLIDESHOW_DIRECTION {
    DSD_FORWARD = 0,
    DSD_BACKWARD = 1
} DESKTOP_SLIDESHOW_DIRECTION;

static const IID IID_IDesktopWallpaper = {0xB92B56A9, 0x8B55, 0x4E14, {0x9A, 0x89, 0x01, 0x99, 0xBB, 0xB6, 0xF9, 0x3B}};

#endif // __IDesktopWallpaper_INTERFACE_DEFINED__

#endif // Q_OS_WIN