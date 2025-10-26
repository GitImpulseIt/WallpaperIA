import { config, type Language } from '../config';

import en from './en.json';
import fr from './fr.json';
import es from './es.json';
import pt from './pt.json';
import it from './it.json';
import de from './de.json';
import ru from './ru.json';

const translations = { en, fr, es, pt, it, de, ru };

export function getTranslation(lang: string) {
  const language = (config.supportedLanguages.includes(lang as Language)
    ? lang
    : config.defaultLanguage) as Language;

  return translations[language];
}

export function getLanguageName(lang: string): string {
  const t = getTranslation(lang);
  return t.langName;
}

export function getLanguageFlag(lang: string): string {
  const flags: Record<string, string> = {
    en: '🇬🇧',
    fr: '🇫🇷',
    es: '🇪🇸',
    pt: '🇵🇹',
    it: '🇮🇹',
    de: '🇩🇪',
    ru: '🇷🇺'
  };
  return flags[lang] || '🌐';
}
