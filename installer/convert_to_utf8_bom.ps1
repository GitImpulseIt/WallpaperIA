# Script PowerShell pour convertir un fichier en UTF-8 avec BOM
# Nécessaire pour que NSIS détecte correctement l'encodage Unicode

param(
    [string]$FilePath = "WallpaperAI_Multilang.nsi"
)

Write-Host "Conversion de $FilePath en UTF-8 avec BOM..."

# Lire le contenu du fichier
$content = [System.IO.File]::ReadAllText($FilePath)

# Créer un encodeur UTF-8 avec BOM
$Utf8BomEncoding = New-Object System.Text.UTF8Encoding $true

# Écrire le fichier avec BOM
[System.IO.File]::WriteAllText($FilePath, $content, $Utf8BomEncoding)

Write-Host "Conversion terminée avec succès !"
Write-Host ""

# Vérifier avec la commande file (si disponible)
$fileInfo = Get-Item $FilePath
Write-Host "Fichier : $($fileInfo.Name)"
Write-Host "Taille : $($fileInfo.Length) octets"
Write-Host ""
Write-Host "Le fichier a été converti en UTF-8 avec BOM."
