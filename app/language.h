#ifndef LANGUAGE_H
#define LANGUAGE_H

// Sélection de la langue via #define LANG_XX
// Par défaut : français (LANG_FR)

#if defined(LANG_EN)
    #include "lang_en.h"
#elif defined(LANG_FR)
    #include "lang_fr.h"
#else
    // Langue par défaut : français
    #include "lang_fr.h"
#endif

#endif // LANGUAGE_H
