#ifndef LANGUAGE_H
#define LANGUAGE_H

// Sélection de la langue via #define LANG_XX
// Par défaut : français (LANG_FR)
// Langues supportées : FR, EN, ES, PT, IT, DE, RU

#if defined(LANG_EN)
    #include "lang_en.h"
#elif defined(LANG_ES)
    #include "lang_es.h"
#elif defined(LANG_PT)
    #include "lang_pt.h"
#elif defined(LANG_IT)
    #include "lang_it.h"
#elif defined(LANG_DE)
    #include "lang_de.h"
#elif defined(LANG_RU)
    #include "lang_ru.h"
#elif defined(LANG_FR)
    #include "lang_fr.h"
#else
    // Langue par défaut : français
    #include "lang_fr.h"
#endif

#endif // LANGUAGE_H
