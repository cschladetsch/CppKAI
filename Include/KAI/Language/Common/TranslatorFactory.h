#pragma once

#include <KAI/Language/Common/Language.h>
#include <KAI/Language/Common/TranslatorCommon.h>

#include <functional>
#include <map>
#include <memory>

KAI_BEGIN

class TranslatorFactory {
   public:
    using CreatorFunction =
        std::function<std::shared_ptr<TranslatorCommon>(Registry&)>;

    static TranslatorFactory& Instance() {
        static TranslatorFactory instance;
        return instance;
    }

    // Register a translator creator for a language
    void RegisterTranslator(Language lang, CreatorFunction creator) {
        creators_[lang] = creator;
    }

    // Create a translator for the given language
    std::shared_ptr<TranslatorCommon> CreateTranslator(Language lang,
                                                       Registry& reg) {
        auto it = creators_.find(lang);
        if (it != creators_.end()) {
            return it->second(reg);
        }
        return nullptr;
    }

    // Check if a language is supported
    bool IsLanguageSupported(Language lang) const {
        return creators_.find(lang) != creators_.end();
    }

    // Get list of supported languages
    std::vector<Language> GetSupportedLanguages() const {
        std::vector<Language> languages;
        for (const auto& pair : creators_) {
            languages.push_back(pair.first);
        }
        return languages;
    }

   private:
    TranslatorFactory() = default;
    std::map<Language, CreatorFunction> creators_;
};

// Helper macro to register a translator
#define REGISTER_TRANSLATOR(lang, translatorClass)                             \
    namespace {                                                                \
    struct TranslatorRegistrar_##translatorClass {                             \
        TranslatorRegistrar_##translatorClass() {                              \
            TranslatorFactory::Instance().RegisterTranslator(                  \
                lang, [](Registry& reg) -> std::shared_ptr<TranslatorCommon> { \
                    return std::make_shared<translatorClass>(reg);             \
                });                                                            \
        }                                                                      \
    } translatorRegistrar_##translatorClass;                                   \
    }

KAI_END