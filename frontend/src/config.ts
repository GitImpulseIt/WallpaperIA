export const config = {
  apiBaseUrl: 'https://kazflow.com/wallpaperai/api',
  thumbnailsPath: '/wallpaperai/api/miniatures/',
  supportedLanguages: ['en', 'fr', 'es', 'pt', 'it', 'de', 'ru'],
  defaultLanguage: 'en'
} as const;

export type Language = typeof config.supportedLanguages[number];
