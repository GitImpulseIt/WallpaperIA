// @ts-check
import { defineConfig } from 'astro/config';

// https://astro.build/config
export default defineConfig({
  // Configuration pour le build
  build: {
    // Générer des fichiers HTML statiques
    format: 'directory',
  },
  // Les fichiers PHP et .htaccess dans public/ seront automatiquement copiés dans dist/
  publicDir: './public',
});
