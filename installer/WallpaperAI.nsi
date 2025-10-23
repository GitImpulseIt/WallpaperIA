; WallpaperAI - Script d'installation NSIS
; Générateur d'installateur pour WallpaperAI
; Créé avec Claude Code

;--------------------------------
; Includes

!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"

;--------------------------------
; Configuration générale

!define APP_NAME "WallpaperAI"
!define APP_VERSION "1.0.0"
!define APP_PUBLISHER "WallpaperAI Team"
!define APP_WEBSITE "https://github.com/kazuya/wallpaperai"
!define APP_EXE "WallpaperAI.exe"
!define APP_ICON "..\app\assets\icon.ico"
!define UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"

; Nom de l'installateur
Name "${APP_NAME} ${APP_VERSION}"
OutFile "WallpaperAI-Setup-${APP_VERSION}.exe"

; Dossier d'installation par défaut
InstallDir "$PROGRAMFILES64\${APP_NAME}"

; Récupérer le dossier d'installation depuis le registre si disponible
InstallDirRegKey HKLM "Software\${APP_NAME}" "InstallDir"

; Demander les privilèges administrateur
RequestExecutionLevel admin

; Compression
SetCompressor /SOLID lzma
SetCompressorDictSize 64

;--------------------------------
; Interface Moderne (MUI)

!define MUI_ABORTWARNING
!define MUI_ICON "${APP_ICON}"
!define MUI_UNICON "${APP_ICON}"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_WELCOMEFINISHPAGE_BITMAP_NOSTRETCH
!define MUI_UNWELCOMEFINISHPAGE_BITMAP_NOSTRETCH

; Page de bienvenue
!define MUI_WELCOMEPAGE_TITLE "Bienvenue dans l'installation de ${APP_NAME}"
!define MUI_WELCOMEPAGE_TEXT "Cet assistant va vous guider dans l'installation de ${APP_NAME}.$\r$\n$\r$\nIl est recommandé de fermer toutes les instances de ${APP_NAME} avant de continuer.$\r$\n$\r$\nCliquez sur Suivant pour continuer."
!insertmacro MUI_PAGE_WELCOME

; Page de licence (optionnelle - décommentez si vous avez une licence)
; !insertmacro MUI_PAGE_LICENSE "..\LICENSE.txt"

; Page de choix du répertoire
!insertmacro MUI_PAGE_DIRECTORY

; Page de progression de l'installation
!insertmacro MUI_PAGE_INSTFILES

; Page de fin avec option de lancement
!define MUI_FINISHPAGE_RUN "$INSTDIR\${APP_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "Lancer ${APP_NAME}"
!define MUI_FINISHPAGE_SHOWREADME ""
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
!define MUI_FINISHPAGE_TITLE "Installation terminée"
!define MUI_FINISHPAGE_TEXT "${APP_NAME} a été installé avec succès sur votre ordinateur.$\r$\n$\r$\nCliquez sur Terminer pour quitter l'assistant."
!insertmacro MUI_PAGE_FINISH

; Pages de désinstallation
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Langues
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; Fonctions

Function .onInit
    ; Vérifier si l'application est déjà en cours d'exécution
    System::Call 'kernel32::CreateMutex(i 0, i 0, t "${APP_NAME}") i .r1 ?e'
    Pop $R0

    ; Vérifier si une version est déjà installée
    ReadRegStr $R0 HKLM "${UNINSTALL_KEY}" "UninstallString"
    StrCmp $R0 "" done

    MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
    "${APP_NAME} est déjà installé. $\n$\nCliquez sur OK pour désinstaller la version précédente ou sur Annuler pour annuler l'installation." \
    IDOK uninst
    Abort

    uninst:
        ClearErrors
        ; Exécuter le désinstallateur
        ExecWait '$R0 _?=$INSTDIR'
        IfErrors no_remove_uninstaller done

        no_remove_uninstaller:

    done:
FunctionEnd

;--------------------------------
; Section d'installation

Section "Installation" SecInstall
    SetOutPath "$INSTDIR"

    ; Arrêter l'application si elle est en cours d'exécution
    DetailPrint "Arrêt de ${APP_NAME} si en cours d'exécution..."
    nsExec::ExecToLog 'taskkill /F /IM ${APP_EXE} /T'
    Pop $0
    Sleep 1000

    ; Fichiers principaux
    DetailPrint "Installation des fichiers principaux..."
    File "..\app\release\${APP_EXE}"
    File "..\app\release\*.dll"
    File "..\app\release\*.png"

    ; Plugins Qt
    DetailPrint "Installation des plugins Qt..."
    SetOutPath "$INSTDIR\platforms"
    File "..\app\release\platforms\*.dll"

    SetOutPath "$INSTDIR\imageformats"
    File "..\app\release\imageformats\*.dll"

    SetOutPath "$INSTDIR\iconengines"
    File "..\app\release\iconengines\*.dll"

    SetOutPath "$INSTDIR\styles"
    File "..\app\release\styles\*.dll"

    SetOutPath "$INSTDIR\tls"
    File "..\app\release\tls\*.dll"

    ; Retour au répertoire principal
    SetOutPath "$INSTDIR"

    ; Créer le désinstallateur
    DetailPrint "Création du désinstallateur..."
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    ; Enregistrer dans le registre
    DetailPrint "Enregistrement dans le registre Windows..."
    WriteRegStr HKLM "Software\${APP_NAME}" "InstallDir" "$INSTDIR"
    WriteRegStr HKLM "${UNINSTALL_KEY}" "DisplayName" "${APP_NAME}"
    WriteRegStr HKLM "${UNINSTALL_KEY}" "DisplayVersion" "${APP_VERSION}"
    WriteRegStr HKLM "${UNINSTALL_KEY}" "Publisher" "${APP_PUBLISHER}"
    WriteRegStr HKLM "${UNINSTALL_KEY}" "DisplayIcon" "$INSTDIR\${APP_EXE}"
    WriteRegStr HKLM "${UNINSTALL_KEY}" "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKLM "${UNINSTALL_KEY}" "QuietUninstallString" "$INSTDIR\Uninstall.exe /S"
    WriteRegStr HKLM "${UNINSTALL_KEY}" "URLInfoAbout" "${APP_WEBSITE}"
    WriteRegStr HKLM "${UNINSTALL_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoModify" 1
    WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoRepair" 1

    ; Calculer la taille de l'installation
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKLM "${UNINSTALL_KEY}" "EstimatedSize" "$0"

    ; Créer des raccourcis dans le menu Démarrer
    DetailPrint "Création des raccourcis..."
    CreateDirectory "$SMPROGRAMS\${APP_NAME}"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\Désinstaller ${APP_NAME}.lnk" "$INSTDIR\Uninstall.exe"

    ; Raccourci sur le bureau (optionnel)
    CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"

    DetailPrint "Installation terminée avec succès !"
SectionEnd

;--------------------------------
; Section de désinstallation

Section "Uninstall"
    ; Arrêter l'application si elle est en cours d'exécution
    DetailPrint "Arrêt de ${APP_NAME}..."
    nsExec::ExecToLog 'taskkill /F /IM ${APP_EXE} /T'
    Pop $0
    Sleep 1000

    ; Supprimer du démarrage automatique (si configuré)
    DetailPrint "Suppression du démarrage automatique..."
    DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "${APP_NAME}"

    ; Supprimer les fichiers
    DetailPrint "Suppression des fichiers..."
    Delete "$INSTDIR\${APP_EXE}"
    Delete "$INSTDIR\*.dll"
    Delete "$INSTDIR\*.png"
    Delete "$INSTDIR\Uninstall.exe"

    ; Supprimer les plugins
    Delete "$INSTDIR\platforms\*.dll"
    Delete "$INSTDIR\imageformats\*.dll"
    Delete "$INSTDIR\iconengines\*.dll"
    Delete "$INSTDIR\styles\*.dll"
    Delete "$INSTDIR\tls\*.dll"

    ; Supprimer les répertoires
    RMDir "$INSTDIR\platforms"
    RMDir "$INSTDIR\imageformats"
    RMDir "$INSTDIR\iconengines"
    RMDir "$INSTDIR\styles"
    RMDir "$INSTDIR\tls"
    RMDir "$INSTDIR"

    ; Supprimer les raccourcis
    DetailPrint "Suppression des raccourcis..."
    Delete "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk"
    Delete "$SMPROGRAMS\${APP_NAME}\Désinstaller ${APP_NAME}.lnk"
    RMDir "$SMPROGRAMS\${APP_NAME}"
    Delete "$DESKTOP\${APP_NAME}.lnk"

    ; Supprimer du registre
    DetailPrint "Nettoyage du registre..."
    DeleteRegKey HKLM "${UNINSTALL_KEY}"
    DeleteRegKey HKLM "Software\${APP_NAME}"

    ; Note : Les données utilisateur dans AppData ne sont PAS supprimées
    ; pour préserver l'historique et les paramètres
    DetailPrint "Désinstallation terminée."
    DetailPrint "Note : Les paramètres utilisateur ont été conservés."

SectionEnd
