# Utilisation de l'endpoint POST /wallpapers

## Configuration préalable

1. Créer le fichier `.credentials` avec vos utilisateurs :
```bash
# Exemple : créer un utilisateur "admin" avec password "secret123"
echo -n "secret123" | sha256sum
# Résultat : 2bb80d537b1da3e38bd30361aa855686bde0eacd7162fef6a25fe97bf527a25b

# Ajouter dans .credentials :
echo "admin:2bb80d537b1da3e38bd30361aa855686bde0eacd7162fef6a25fe97bf527a25b" > .credentials
```

## Exemples d'utilisation avec curl

### Ajouter une entrée simple

```bash
curl -X POST https://your-domain.com/api/wallpapers \
  -u admin:secret123 \
  -H "Content-Type: application/json" \
  -d '{
    "category": "CYBERPUNK/FUTURISTIC",
    "filename": "neon_city_night.png",
    "date": "29/09/2025"
  }'
```

### Réponse succès :

```json
{
  "success": true,
  "message": "Wallpaper entry added successfully",
  "entry": {
    "category": "CYBERPUNK/FUTURISTIC",
    "filename": "neon_city_night.png",
    "date": "29/09/2025",
    "id": 1
  },
  "authenticated_as": "admin"
}
```

### Exemples avec différentes catégories

```bash
# OCEAN & MARINE
curl -X POST https://your-domain.com/api/wallpapers \
  -u admin:secret123 \
  -H "Content-Type: application/json" \
  -d '{
    "category": "OCEAN & MARINE",
    "filename": "coral_reef_underwater.png",
    "date": "30/09/2025"
  }'

# MOUNTAINS & PEAKS
curl -X POST https://your-domain.com/api/wallpapers \
  -u admin:secret123 \
  -H "Content-Type: application/json" \
  -d '{
    "category": "MOUNTAINS & PEAKS",
    "filename": "summit_sunrise_alpenglow.jpg",
    "date": "01/10/2025"
  }'
```

## Erreurs possibles

### Erreur d'authentification (401)

```bash
# Mauvais password
curl -X POST https://your-domain.com/api/wallpapers \
  -u admin:wrongpassword \
  -H "Content-Type: application/json" \
  -d '{}'

# Réponse :
{
  "success": false,
  "error": "Invalid username or password"
}
```

### Paramètres manquants (400)

```bash
# Sans filename
curl -X POST https://your-domain.com/api/wallpapers \
  -u admin:secret123 \
  -H "Content-Type: application/json" \
  -d '{
    "category": "CYBERPUNK/FUTURISTIC",
    "date": "29/09/2025"
  }'

# Réponse :
{
  "success": false,
  "error": "Missing required parameters: category, filename, date",
  "required_format": {
    "category": "string (e.g., \"CYBERPUNK/FUTURISTIC\")",
    "filename": "string with extension (e.g., \"neon_city_night.png\")",
    "date": "string DD/MM/YYYY (e.g., \"29/09/2025\")"
  }
}
```

### Entrée déjà existante

```bash
# Même category/filename/date
curl -X POST https://your-domain.com/api/wallpapers \
  -u admin:secret123 \
  -H "Content-Type: application/json" \
  -d '{
    "category": "CYBERPUNK/FUTURISTIC",
    "filename": "neon_city_night.png",
    "date": "29/09/2025"
  }'

# Réponse :
{
  "success": false,
  "error": "Entry already exists",
  "entry": {
    "category": "CYBERPUNK/FUTURISTIC",
    "filename": "neon_city_night.png",
    "date": "29/09/2025"
  }
}
```

### Format de date invalide

```bash
curl -X POST https://your-domain.com/api/wallpapers \
  -u admin:secret123 \
  -H "Content-Type: application/json" \
  -d '{
    "category": "CYBERPUNK/FUTURISTIC",
    "filename": "neon_city_night.png",
    "date": "2025-09-29"
  }'

# Réponse :
{
  "success": false,
  "error": "Date must be in DD/MM/YYYY format"
}
```

## Notes importantes

1. **Format de filename** : Accepte maintenant n'importe quel nom descriptif avec extension (plus seulement SHA256)
2. **Incrémentation automatique de l'ID** : L'ID est calculé automatiquement pour chaque category/date
3. **Détection de doublons** : Vérifie que la combinaison category/filename/date n'existe pas déjà
4. **Sécurité** : Le fichier .credentials est protégé par .htaccess (accès HTTP interdit)

## Script d'ajout en masse

```bash
#!/bin/bash
# add_wallpapers.sh

API_URL="https://your-domain.com/api/wallpapers"
USERNAME="admin"
PASSWORD="secret123"

# Tableau de wallpapers à ajouter
declare -a wallpapers=(
  "CYBERPUNK/FUTURISTIC|neon_city_night.png|29/09/2025"
  "OCEAN & MARINE|coral_reef_underwater.png|29/09/2025"
  "MOUNTAINS & PEAKS|summit_sunrise_alpenglow.png|29/09/2025"
)

# Boucle pour ajouter chaque wallpaper
for entry in "${wallpapers[@]}"; do
  IFS='|' read -r category filename date <<< "$entry"

  echo "Adding: $category / $filename / $date"

  curl -X POST "$API_URL" \
    -u "$USERNAME:$PASSWORD" \
    -H "Content-Type: application/json" \
    -d "{
      \"category\": \"$category\",
      \"filename\": \"$filename\",
      \"date\": \"$date\"
    }"

  echo -e "\n---\n"
done
```
