// Détection du mode développement
const isDev = import.meta.env.DEV;

export const config = {
  apiBaseUrl: 'https://kazflow.com/wallpaperai/api',
  // En mode dev, utiliser l'URL complète pour les miniatures car elles sont sur le serveur de production
  thumbnailsPath: isDev
    ? 'https://kazflow.com/wallpaperai/api/miniatures/'
    : '/wallpaperai/api/miniatures/',
  supportedLanguages: ['en', 'fr', 'es', 'pt', 'it', 'de', 'ru'],
  defaultLanguage: 'en'
} as const;

export type Language = typeof config.supportedLanguages[number];
