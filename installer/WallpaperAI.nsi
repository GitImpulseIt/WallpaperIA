; WallpaperAI - Script d'installation NSIS Multilingue
; Installateur intelligent qui installe uniquement le binaire de la langue choisie
; Créé avec Claude Code

;--------------------------------
; Configuration de l'encodage
; Encodage UTF-8 avec BOM pour support correct de l'Unicode
Unicode true

;--------------------------------
; Includes

!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"

;--------------------------------
; Configuration générale

!define APP_NAME "WallpaperAI"
!define APP_VERSION "1.0.3"
!define APP_PUBLISHER "WallpaperAI Team"
!define APP_WEBSITE "https://github.com/kazuya/wallpaperai"
!define APP_EXE "WallpaperAI.exe"
!define APP_ICON "..\app\assets\icon.ico"
!define UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"

; Nom de l'installateur
Name "${APP_NAME} ${APP_VERSION}"
OutFile "WallpaperAI-Setup-Multilang-${APP_VERSION}.exe"

; Dossier d'installation par défaut
InstallDir "$LOCALAPPDATA\${APP_NAME}"

; Récupérer le dossier d'installation depuis le registre si disponible
InstallDirRegKey HKCU "Software\${APP_NAME}" "InstallDir"

; Demander les privilèges utilisateur
RequestExecutionLevel user

; Compression
SetCompressor /SOLID lzma
SetCompressorDictSize 64

;--------------------------------
; Variables globales

Var SelectedLanguage
Var LanguageCode

;--------------------------------
; Interface Moderne (MUI)

!define MUI_ABORTWARNING
!define MUI_ICON "${APP_ICON}"
!define MUI_UNICON "${APP_ICON}"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_WELCOMEFINISHPAGE_BITMAP_NOSTRETCH
!define MUI_UNWELCOMEFINISHPAGE_BITMAP_NOSTRETCH

; Page de sélection de langue customisée
Page custom LanguageSelectionPage LanguageSelectionLeave

; Page de bienvenue
!insertmacro MUI_PAGE_WELCOME

; Page de choix du répertoire
!insertmacro MUI_PAGE_DIRECTORY

; Page de progression de l'installation
!insertmacro MUI_PAGE_INSTFILES

; Page de fin avec option de lancement
!define MUI_FINISHPAGE_RUN "$INSTDIR\${APP_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT_FRENCH "Lancer ${APP_NAME}"
!define MUI_FINISHPAGE_RUN_TEXT_ENGLISH "Launch ${APP_NAME}"
!define MUI_FINISHPAGE_TITLE_FRENCH "Installation terminée"
!define MUI_FINISHPAGE_TITLE_ENGLISH "Installation Complete"
!insertmacro MUI_PAGE_FINISH

; Pages de désinstallation
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Langues de l'installateur
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "Portuguese"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Russian"

;--------------------------------
; Strings personnalisées pour la page de sélection de langue

LangString PAGE_TITLE_LANG ${LANG_FRENCH} "Choix de la langue"
LangString PAGE_TITLE_LANG ${LANG_ENGLISH} "Language Selection"
LangString PAGE_TITLE_LANG ${LANG_SPANISH} "Selección de idioma"
LangString PAGE_TITLE_LANG ${LANG_PORTUGUESE} "Seleção de idioma"
LangString PAGE_TITLE_LANG ${LANG_ITALIAN} "Selezione della lingua"
LangString PAGE_TITLE_LANG ${LANG_GERMAN} "Sprachauswahl"
LangString PAGE_TITLE_LANG ${LANG_RUSSIAN} "Выбор языка"

LangString PAGE_SUBTITLE_LANG ${LANG_FRENCH} "Choisissez la langue de l'application"
LangString PAGE_SUBTITLE_LANG ${LANG_ENGLISH} "Choose the application language"
LangString PAGE_SUBTITLE_LANG ${LANG_SPANISH} "Elija el idioma de la aplicación"
LangString PAGE_SUBTITLE_LANG ${LANG_PORTUGUESE} "Escolha o idioma da aplicação"
LangString PAGE_SUBTITLE_LANG ${LANG_ITALIAN} "Scegli la lingua dell'applicazione"
LangString PAGE_SUBTITLE_LANG ${LANG_GERMAN} "Wählen Sie die Anwendungssprache"
LangString PAGE_SUBTITLE_LANG ${LANG_RUSSIAN} "Выберите язык приложения"

LangString LANG_SELECT_TEXT ${LANG_FRENCH} "Langue de l'application :"
LangString LANG_SELECT_TEXT ${LANG_ENGLISH} "Application language:"
LangString LANG_SELECT_TEXT ${LANG_SPANISH} "Idioma de la aplicación:"
LangString LANG_SELECT_TEXT ${LANG_PORTUGUESE} "Idioma da aplicação:"
LangString LANG_SELECT_TEXT ${LANG_ITALIAN} "Lingua dell'applicazione:"
LangString LANG_SELECT_TEXT ${LANG_GERMAN} "Anwendungssprache:"
LangString LANG_SELECT_TEXT ${LANG_RUSSIAN} "Язык приложения:"

;--------------------------------
; Fonctions

Function .onInit
    ; Initialiser la langue par défaut selon la langue de l'installateur
    ${If} $LANGUAGE == ${LANG_FRENCH}
        StrCpy $LanguageCode "FR"
    ${ElseIf} $LANGUAGE == ${LANG_ENGLISH}
        StrCpy $LanguageCode "EN"
    ${ElseIf} $LANGUAGE == ${LANG_SPANISH}
        StrCpy $LanguageCode "ES"
    ${ElseIf} $LANGUAGE == ${LANG_PORTUGUESE}
        StrCpy $LanguageCode "PT"
    ${ElseIf} $LANGUAGE == ${LANG_ITALIAN}
        StrCpy $LanguageCode "IT"
    ${ElseIf} $LANGUAGE == ${LANG_GERMAN}
        StrCpy $LanguageCode "DE"
    ${ElseIf} $LANGUAGE == ${LANG_RUSSIAN}
        StrCpy $LanguageCode "RU"
    ${Else}
        StrCpy $LanguageCode "EN"
    ${EndIf}

    ; Vérifier si une version est déjà installée
    ReadRegStr $R0 HKCU "${UNINSTALL_KEY}" "UninstallString"
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

; Page de sélection de langue
Function LanguageSelectionPage
    !insertmacro MUI_HEADER_TEXT "$(PAGE_TITLE_LANG)" "$(PAGE_SUBTITLE_LANG)"

    nsDialogs::Create 1018
    Pop $0
    ${If} $0 == error
        Abort
    ${EndIf}

    ; Texte d'explication
    ${NSD_CreateLabel} 0 0 100% 12u "$(LANG_SELECT_TEXT)"
    Pop $0

    ; Liste déroulante des langues
    ${NSD_CreateDropList} 0 20u 200u 80u ""
    Pop $SelectedLanguage

    ; Ajouter les langues disponibles
    ${NSD_CB_AddString} $SelectedLanguage "Français (French)"
    ${NSD_CB_AddString} $SelectedLanguage "English"
    ${NSD_CB_AddString} $SelectedLanguage "Español (Spanish)"
    ${NSD_CB_AddString} $SelectedLanguage "Português (Portuguese)"
    ${NSD_CB_AddString} $SelectedLanguage "Italiano (Italian)"
    ${NSD_CB_AddString} $SelectedLanguage "Deutsch (German)"
    ${NSD_CB_AddString} $SelectedLanguage "Русский (Russian)"

    ; Sélectionner la langue par défaut selon le code
    ${If} $LanguageCode == "FR"
        ${NSD_CB_SelectString} $SelectedLanguage "Français (French)"
    ${ElseIf} $LanguageCode == "EN"
        ${NSD_CB_SelectString} $SelectedLanguage "English"
    ${ElseIf} $LanguageCode == "ES"
        ${NSD_CB_SelectString} $SelectedLanguage "Español (Spanish)"
    ${ElseIf} $LanguageCode == "PT"
        ${NSD_CB_SelectString} $SelectedLanguage "Português (Portuguese)"
    ${ElseIf} $LanguageCode == "IT"
        ${NSD_CB_SelectString} $SelectedLanguage "Italiano (Italian)"
    ${ElseIf} $LanguageCode == "DE"
        ${NSD_CB_SelectString} $SelectedLanguage "Deutsch (German)"
    ${ElseIf} $LanguageCode == "RU"
        ${NSD_CB_SelectString} $SelectedLanguage "Русский (Russian)"
    ${Else}
        ${NSD_CB_SelectString} $SelectedLanguage "English"
    ${EndIf}

    nsDialogs::Show
FunctionEnd

Function LanguageSelectionLeave
    ; Récupérer la langue sélectionnée
    ${NSD_GetText} $SelectedLanguage $0

    ${If} $0 == "Français (French)"
        StrCpy $LanguageCode "FR"
    ${ElseIf} $0 == "English"
        StrCpy $LanguageCode "EN"
    ${ElseIf} $0 == "Español (Spanish)"
        StrCpy $LanguageCode "ES"
    ${ElseIf} $0 == "Português (Portuguese)"
        StrCpy $LanguageCode "PT"
    ${ElseIf} $0 == "Italiano (Italian)"
        StrCpy $LanguageCode "IT"
    ${ElseIf} $0 == "Deutsch (German)"
        StrCpy $LanguageCode "DE"
    ${ElseIf} $0 == "Русский (Russian)"
        StrCpy $LanguageCode "RU"
    ${Else}
        StrCpy $LanguageCode "EN"
    ${EndIf}

    DetailPrint "Langue sélectionnée : $LanguageCode"
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

    ; Installer l'exécutable correspondant à la langue choisie
    DetailPrint "Installation de l'exécutable ($LanguageCode)..."

    ${If} $LanguageCode == "FR"
        File /oname=${APP_EXE} "..\app\release\WallpaperAI_FR.exe"
    ${ElseIf} $LanguageCode == "EN"
        File /oname=${APP_EXE} "..\app\release\WallpaperAI_EN.exe"
    ${ElseIf} $LanguageCode == "ES"
        File /oname=${APP_EXE} "..\app\release\WallpaperAI_ES.exe"
    ${ElseIf} $LanguageCode == "PT"
        File /oname=${APP_EXE} "..\app\release\WallpaperAI_PT.exe"
    ${ElseIf} $LanguageCode == "IT"
        File /oname=${APP_EXE} "..\app\release\WallpaperAI_IT.exe"
    ${ElseIf} $LanguageCode == "DE"
        File /oname=${APP_EXE} "..\app\release\WallpaperAI_DE.exe"
    ${ElseIf} $LanguageCode == "RU"
        File /oname=${APP_EXE} "..\app\release\WallpaperAI_RU.exe"
    ${Else}
        File /oname=${APP_EXE} "..\app\release\WallpaperAI_EN.exe"
    ${EndIf}

    ; Enregistrer la langue choisie dans le registre pour référence
    WriteRegStr HKCU "Software\${APP_NAME}" "Language" "$LanguageCode"

    ; Fichiers communs (DLLs et ressources)
    DetailPrint "Installation des dépendances Qt..."
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
    WriteRegStr HKCU "Software\${APP_NAME}" "InstallDir" "$INSTDIR"
    WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayName" "${APP_NAME}"
    WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayVersion" "${APP_VERSION}"
    WriteRegStr HKCU "${UNINSTALL_KEY}" "Publisher" "${APP_PUBLISHER}"
    WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayIcon" "$INSTDIR\${APP_EXE}"
    WriteRegStr HKCU "${UNINSTALL_KEY}" "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKCU "${UNINSTALL_KEY}" "QuietUninstallString" "$INSTDIR\Uninstall.exe /S"
    WriteRegStr HKCU "${UNINSTALL_KEY}" "URLInfoAbout" "${APP_WEBSITE}"
    WriteRegStr HKCU "${UNINSTALL_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegDWORD HKCU "${UNINSTALL_KEY}" "NoModify" 1
    WriteRegDWORD HKCU "${UNINSTALL_KEY}" "NoRepair" 1

    ; Calculer la taille de l'installation
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKCU "${UNINSTALL_KEY}" "EstimatedSize" "$0"

    ; Créer des raccourcis dans le menu Démarrer
    DetailPrint "Création des raccourcis..."
    CreateDirectory "$SMPROGRAMS\${APP_NAME}"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\Désinstaller ${APP_NAME}.lnk" "$INSTDIR\Uninstall.exe"

    ; Raccourci sur le bureau
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
    DeleteRegKey HKCU "${UNINSTALL_KEY}"
    DeleteRegKey HKCU "Software\${APP_NAME}"

    ; Note : Les données utilisateur dans AppData ne sont PAS supprimées
    ; pour préserver l'historique et les paramètres
    DetailPrint "Désinstallation terminée."
    DetailPrint "Note : Les paramètres utilisateur ont été conservés."

SectionEnd
