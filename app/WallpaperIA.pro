QT += core widgets

CONFIG += c++17

TARGET = WallpaperIA
TEMPLATE = app

SOURCES += main.cpp

# Deployment
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/WallpaperIA
INSTALLS += target