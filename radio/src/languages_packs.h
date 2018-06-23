
#if defined(CPUARM) && defined(VOICE)
struct LanguagePack {
  const char * id;
  const char * name;
  void (*playNumber)(getvalue_t number, uint8_t unit, uint8_t flags, uint8_t id);
  void (*playDuration)(int seconds, uint8_t flags, uint8_t id);
};

extern const LanguagePack * currentLanguagePack;
extern uint8_t currentLanguagePackIdx;

extern const LanguagePack czLanguagePack;
extern const LanguagePack enLanguagePack;
extern const LanguagePack esLanguagePack;
extern const LanguagePack frLanguagePack;
extern const LanguagePack deLanguagePack;
extern const LanguagePack itLanguagePack;
extern const LanguagePack plLanguagePack;
extern const LanguagePack ptLanguagePack;
extern const LanguagePack skLanguagePack;
extern const LanguagePack seLanguagePack;
extern const LanguagePack huLanguagePack;
extern const LanguagePack ruLanguagePack;
extern const LanguagePack * const languagePacks[];
#if defined(LANGUAGE_PACKS_DEFINITION)
const LanguagePack * const languagePacks[] = {
    // alphabetical order
    &czLanguagePack,
    &deLanguagePack,
    &enLanguagePack,
    &esLanguagePack,
    &frLanguagePack,
    &huLanguagePack,
    &itLanguagePack,
    &plLanguagePack,
    &ptLanguagePack,
    &ruLanguagePack,
    &seLanguagePack,
    &skLanguagePack,
    NULL
  };
#endif
#if defined(SIMU)
#define LANGUAGE_PACK_DECLARE(lng, name) const LanguagePack lng ## LanguagePack = { #lng, name, lng ## _ ## playNumber, lng ## _ ## playDuration }
#else
#define LANGUAGE_PACK_DECLARE(lng, name) extern const LanguagePack lng ## LanguagePack = { #lng, name, lng ## _ ## playNumber, lng ## _ ## playDuration }
#endif
#define LANGUAGE_PACK_DECLARE_DEFAULT(lng, name) LANGUAGE_PACK_DECLARE(lng, name); const LanguagePack * currentLanguagePack = & lng ## LanguagePack; uint8_t currentLanguagePackIdx
inline PLAY_FUNCTION(playNumber, getvalue_t number, uint8_t unit, uint8_t flags) { currentLanguagePack->playNumber(number, unit, flags, id); }
inline PLAY_FUNCTION(playDuration, int seconds, uint8_t flags) { currentLanguagePack->playDuration(seconds, flags, id); }
#elif defined(VOICE)
PLAY_FUNCTION(playNumber, getvalue_t number, uint8_t unit, uint8_t att);
  PLAY_FUNCTION(playDuration, int seconds);
  #define LANGUAGE_PACK_DECLARE(lng, name)
  #define LANGUAGE_PACK_DECLARE_DEFAULT(lng, name)
#else
  #define LANGUAGE_PACK_DECLARE(lng, name)
  #define LANGUAGE_PACK_DECLARE_DEFAULT(lng, name)
#endif
