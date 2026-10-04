#include <list>
#include <vector>
#include <string.h>
#include <pthread.h>
#include <thread>
#include <cstring>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <dlfcn.h>
#include <cctype>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include "Includes/obfuscate.h"
#include "Includes/Logger.h"
#include "Includes/Utils.h"
#include "KittyMemory/MemoryPatch.h"
#include "Includes/Strings.h"
#include "Includes/Macros.h"
#include "Includes/Array.h"
#include "Includes/StringUtils.cpp"

#define targetLibName OBFUSCATE("libil2cpp.so")

static bool inTourx;
static int setCustomGamemode = 0;

void* (*createSprite)(void* texture, void* rect, void* pivot) = nullptr;
void* (*gameObjectFind)(String* name) = nullptr;
void* (*objectGetComponent)(void* instance, String* typeName) = nullptr;
void* (*getTransform)(void* instance) = nullptr;
void* (*getParent)(void* instance) = nullptr;
void* (*objectInstantiate)(void* original, void* parent) = nullptr;
void* (*objectDestroy)(void* obj) = nullptr;
void* (*transformSetPosition)(void* transform, void* value) = nullptr;
void* (*transformGetPosition)(void* instance) = nullptr;
void* (*vector3Ctor)(float x, float y, float z) = nullptr;
void* (*gameObjectSetActive)(void* _this, bool value) = nullptr;
void* (*objectSetName)(void* instance, String* value) = nullptr;
void* (*componentSetActive)(void* instance, bool value) = nullptr;
void* (*loadSprite)(String* path, void* systemTypeInstance) = nullptr;
String* (*objectGetName)(void* instance) = nullptr;
void* (*toVec)(void*, float z, float y, float x) = nullptr;
void* (*objectGetComponentInChildren)(void* instance, void* type) = nullptr;
void* (*transformTranslate)(void* instance, float x, float y, float z) = nullptr;
void* (*findObject)(void* type) = nullptr;
void* (*activatorCreateInstance)(void* type) = nullptr;
String* (*getTraducao)(String* key) = nullptr;
void* (*scriptableObjectCreateInstance)(void* type) = nullptr;
void* (*createUri)(String* s) = nullptr;
void* (*cloneCustomRoomProperties)(void* instance) = nullptr;
void* (*getHashtableItem)(void* hashtable, String* key) = nullptr;
void* levelManagerInstance;
void* (*openPopup)(void*, String*, float, bool) = nullptr;
void* (*setPopupItems)(void*, String*, String*) = nullptr;
void* (*getCurrencySprite)(void* instance, String* spriteID) = nullptr;
bool (*imageConversionLoadImage)(void* texture, void* data, bool markNonReadable) = nullptr;
static String* respawnLegendaryString = nullptr;
static String* respawnDashString = nullptr;
void* (*actacteCreateInstanceParams)(void* type, void* args[]) = nullptr;


static void* tmpTextMeshPro = nullptr;
static bool isAnimating = false;
static float hueOffset = 0.0f;

void* (*levelEntry)(void* instance, void* levelDef);
void* customLevelDef;
static bool addedMap = false;

int userId;

void* (*getType)(String* typeName);

void* (*oldLogin)(void* instance, void* callback);
void* login(void* instance, void* callback) {
    if (instance) {
        void* user = *(void**)((uintptr_t)instance + 0x20);
        if (user) {
            userId = *(int*)((uint8_t*)user + 0x10);
        }
    }
    return oldLogin(instance, callback);
}

int getUserId() {
    if (userId != 0) return userId;
    std::srand(std::time(nullptr));
    return std::rand() % 100 + 1;
}

void* loadSpriteFunc(char* spriteID) {
    void* initializerType = getType(String::Create("Stumble.Initializer, Assembly-CSharp"));
    if (!initializerType) {
        return nullptr;
    }
    
    void* initializerInstance = findObject(initializerType);
    if (!initializerInstance) {
        return nullptr;
    }
    
    String* spriteIDString = String::Create(spriteID);
    if (!spriteIDString) {
        return nullptr;
    }
   
    void* uiArtInstance = *(void**)((uintptr_t)initializerInstance + 0xB8);
    
    return getCurrencySprite(uiArtInstance, spriteIDString);
}

void* openPopupFunc(char* name, char* header, char* content) {
    void* popup = nullptr;
    void* controllerInstance = findObject(getType(String::Create("PopupManager, Assembly-CSharp")));
    if (controllerInstance) {
        popup = openPopup(controllerInstance, String::Create(name), 0.0f, false);
        if (setPopupItems && popup) {
            setPopupItems(popup, String::Create(header), String::Create(content));
        }
    }
    return popup;
}

char* colorToHex(float r, float g, float b) {
    static char hex[8];
    int ri = (int)(r * 255);
    int gi = (int)(g * 255);
    int bi = (int)(b * 255);
    ri = std::max(0, std::min(255, ri));
    gi = std::max(0, std::min(255, gi));
    bi = std::max(0, std::min(255, bi));
    snprintf(hex, sizeof(hex), "%02X%02X%02X", ri, gi, bi);
    return hex;
}

void* animationThread(void*) {
    hueOffset = 0.0f;
    const char* text = "StumbleCole 1.5";

    while (true) {
        usleep(75000);
        hueOffset += 0.02f;
        if (hueOffset > 1.0f) hueOffset -= 1.0f;

        char finalText[2048] = "";
        for (int i = 0; i < strlen(text); i++) {
            float t = (sinf(hueOffset * M_PI * 2.0f + i * 0.3f) + 1.0f) / 2.0f;

            float lightCyanR = 0.2f, lightCyanG = 0.6f, lightCyanB = 0.6f;
            float darkCyanR = 0.0f, darkCyanG = 0.3f, darkCyanB = 0.3f;
            
            float r = lightCyanR + (darkCyanR - lightCyanR) * t;
            float g = lightCyanG + (darkCyanG - lightCyanG) * t;
            float b = lightCyanB + (darkCyanB - lightCyanB) * t;

            r = std::max(0.0f, std::min(1.0f, r));
            g = std::max(0.0f, std::min(1.0f, g));
            b = std::max(0.0f, std::min(1.0f, b));

            char charText[256];
            snprintf(charText, sizeof(charText), "<color=#%s>%c</color>", colorToHex(r, g, b), text[i]);
            strcat(finalText, charText);
        }

        String* animatedText = String::Create(finalText);
        if (animatedText && tmpTextMeshPro) {
            *(String**)((uintptr_t)tmpTextMeshPro + 0xD0) = animatedText;
        }
    }
    
    isAnimating = true;
    return nullptr;
}



void (*oldPlayViewController)(void* instance);
void playViewController(void* instance) {
    oldPlayViewController(instance);
    
    void* gameObject = gameObjectFind(String::Create("PassButton"));
    if (gameObject) {
        gameObjectSetActive(gameObject, false);
    }

    void* gameObject2 = gameObjectFind(String::Create("NewBadge"));
    if (gameObject2) {
        gameObjectSetActive(gameObject2, false);
    }
    
     void* tournamentXButton = gameObjectFind(String::Create("PLAY_VIEW_TournamentXButton"));
    if (tournamentXButton && getTransform && transformTranslate) {
        transformTranslate(getTransform(tournamentXButton), 0.0f, -110.0f, 0.0f);
    }

    void* header = gameObjectFind(String::Create("HeaderLobby"));
    if (header) {
        void* image = objectGetComponent(header, String::Create("Image"));
        if (image) {
            void* color = (void* (*))((uintptr_t)image + 0x20);
            if (color) {
                float* a = (float*)((uintptr_t)color + 0xC);
                if (a) {
                    *a = 0.0f;
                }
            }
        }
    }

    if (!addedMap) {
        void* inicializerType = getType(String::Create("Stumble.Initializer, Assembly-CSharp"));
        void* inicializerInstance = findObject(inicializerType);
        if (inicializerInstance) {
            void* levelManager = *(void**)((uintptr_t)inicializerInstance + 0x120);
            if (levelManager) {
                void* levelGroupListType = getType(String::Create("System.Collections.Generic.List`1[[LevelGroupDef, Assembly-CSharp]], mscorlib"));
                if (levelGroupListType) {
                    void* levelGroupList = activatorCreateInstance(levelGroupListType);
                    if (levelGroupList) {
                        void* levelDefType = getType(String::Create("LevelDef, Assembly-CSharp"));
                        if (levelDefType) {
                            void* levelDef1 = scriptableObjectCreateInstance(levelDefType);
                            if (levelDef1) {
                                *(String**)((uintptr_t)levelDef1 + 0x18) = String::Create("CustomLevel1");
                                *(String**)((uintptr_t)levelDef1 + 0x20) = String::Create("Core Dash");
                                *(String**)((uintptr_t)levelDef1 + 0x28) = String::Create("Respawn Dash");
                                *(String**)((uintptr_t)levelDef1 + 0x30) = String::Create("Level19");
                                *(String**)((uintptr_t)levelDef1 + 0x38) = String::Create(".GG/SGCORE");
                                *(String**)((uintptr_t)levelDef1 + 0x40) = String::Create(".GG/SGCORE");
                                *(void**)((uintptr_t)levelDef1 + 0x48) = levelGroupList;
                                levelEntry(levelManager, levelDef1);

                                void* levelDef2 = scriptableObjectCreateInstance(levelDefType);
                                if (levelDef2) {
                                    *(String**)((uintptr_t)levelDef2 + 0x18) = String::Create("CustomLevel2");
                                    *(String**)((uintptr_t)levelDef2 + 0x20) = String::Create("Core Legendary");
                                    *(String**)((uintptr_t)levelDef2 + 0x28) = String::Create("Respawn Legendary");
                                    *(String**)((uintptr_t)levelDef2 + 0x30) = String::Create("eventlevel13_block_legendary");
                                    *(String**)((uintptr_t)levelDef2 + 0x38) = String::Create(".GG/SGCORE");
                                    *(String**)((uintptr_t)levelDef2 + 0x40) = String::Create(".GG/SGCORE");
                                    *(void**)((uintptr_t)levelDef2 + 0x48) = levelGroupList;
                                    // levelEntry(levelManager, levelDef2);
                                }
                                addedMap = true;
                            }
                        }
                    }
                }
            }
        }
    }
}

void (*oldHeaderViewHelper)(void* instance);
void headerViewHelper(void* instance) {
    oldHeaderViewHelper(instance);
    inTourx = false;
    void* text = *(void**)((uintptr_t)instance + 0x18);
    if (text) {
        *(String**)((uintptr_t)text + 0xD0) = String::Create("StumbleCore Mob 1.6.1");
    }
}

void (*oldHeaderLobbyViewHelper)(void* instance);
void headerLobbyViewHelper(void* instance) {
    oldHeaderLobbyViewHelper(instance);
    void* text = *(void**)((uintptr_t)instance + 0x38);
    if (text) {
        tmpTextMeshPro = text;
        *(String**)((uintptr_t)text + 0xD0) = String::Create("StumbleCore Mob 1.6.1");
    }
}

void* (*oldUiController)(void* instance);
void* uiController(void* instance) {
    if (setCustomGamemode == 1) {
        void* leaveRoot = gameObjectFind(String::Create("LeaveRoot"));
        if (leaveRoot) {
            gameObjectSetActive(leaveRoot, true);
        }
    }
    
    void* playerNameGO = gameObjectFind(String::Create("PlayerName(Clone)"));
    
    if (playerNameGO) {
        void* playerNameParent = objectGetComponent(gameObjectFind(String::Create("PlayerNames")), String::Create("Transform"));
        void* beastWatermark = objectInstantiate(playerNameGO, playerNameParent);

        objectSetName(beastWatermark, String::Create("Core Watermark"));
        gameObjectSetActive(beastWatermark, true);
        
        void* beastWatermarkTransform = objectGetComponent(beastWatermark, String::Create("Transform"));
        if (beastWatermarkTransform) {
            transformTranslate(beastWatermarkTransform, 0.0f, 70.0f, 0.0f);
        }
        
        void* text = objectGetComponent(beastWatermark, String::Create("TextMeshProUGUI"));
        if (text) {
            *(String**)((uintptr_t)text + 0xD0) = String::Create("StumbleCore 1.6");
            float* fontSize = (float*)((uintptr_t)text + 0x1DC);
            if (fontSize) {
                *fontSize = 30.0f;
            }
        }
        
        void* arrowTransform = gameObjectFind(String::Create("UICamera/UI/NonPersistentRoot/GameHUD/PlayerHUDViews/PlayerHUDView(Clone)/Container/PlayerNames/Beast Watermark/Arrow"));
        if (arrowTransform) {
            gameObjectSetActive(arrowTransform, false);
        }
    }
    
    return oldUiController(instance);  
}


bool (*old_isVersionMinimium)(void* instance, void* client);
bool isVersionMinimium(void* instance, void* client)
{
    return true;
}

void (*oldLoginViewController)(void* instance);
void loginViewController(void* instance) {
    oldLoginViewController(instance);
    void* gameObject = gameObjectFind(String::Create("LogoBG (1)"));
    if (gameObject) {
        gameObjectSetActive(gameObject, false);
    }
}

void (*oldProfileViewController)(void* instance);
void profileViewController(void* instance) {
    oldProfileViewController(instance);
    
    void* original = gameObjectFind(String::Create("MenuStateRoot/PROFILE_VIEW(Clone)/Canvas/Root/GameObject/Right/SkinList/Viewport/ProfileDataRoot/NameArea/CrownDisplay"));
    if (original) {
        void* parentObj = gameObjectFind(String::Create("MenuStateRoot/PROFILE_VIEW(Clone)/Canvas/Root/GameObject/Right/SkinList/Viewport/ProfileDataRoot/NameArea"));
        void* parent = objectGetComponent(parentObj, String::Create("Transform"));

        void* idDisplay = objectInstantiate(original, parent);
        objectSetName(idDisplay, String::Create("idDisplay"));
        if (idDisplay) {
            gameObjectSetActive(idDisplay, true);
            void* transform = objectGetComponent(idDisplay, String::Create("Transform"));
            transformTranslate(transform, -110.0f, -70.10f, -0.0f);

            void* currencyIcon = gameObjectFind(String::Create("idDisplay/CurrencyIcon"));
            if (currencyIcon) {
                void* img = objectGetComponent(currencyIcon, String::Create("Image"));
                if (img) {
                    void* newSprite = loadSpriteFunc("collectables_mallgold_icon");
                    if (newSprite) {
                        *(void**)((uintptr_t)img + 0xD0) = newSprite;                  
                    }
                }
            }
  
            void* textT = getType(String::Create("TMPro.TextMeshProUGUI, Unity.TextMeshPro"));
            if (textT) {
                void* text = objectGetComponentInChildren(idDisplay, textT);
                if (text) {
                    char playerId[12];
                    snprintf(playerId, sizeof(playerId), "%d", getUserId());
                    *(String**)((uintptr_t)text + 0xD0) = String::Create(playerId);
                }
            }
        }
    }
}

void* (*old_backboneHttpClient)(void* instance, void* baseUri, String* applicationI);
void* backboneHttpClient(void* instance, void* baseUri, String* applicationId)
{
    return old_backboneHttpClient(instance, createUri(String::Create("https://backbone-core-production-c200.up.railway.app/")), String::Create("CoreSG"));
}

void* (*old_backboneInitialize)(void* baseUri, String* applicationId);
void* backboneInitialize(void* baseUri, String* applicationId)
{
    return old_backboneInitialize(createUri(String::Create("https://backbone-core-production-c200.up.railway.app/")), applicationId);
}


void (*oldInitializer)(void* instance);
void initializer(void* instance) {
    void** runtimeConfigurationPtr = (void**)((uintptr_t)instance + 0x20);
    if (runtimeConfigurationPtr && *runtimeConfigurationPtr) {
        void* runtimeConfiguration = *runtimeConfigurationPtr;
        void** environmentConfigPtr = (void**)((uintptr_t)runtimeConfiguration + 0x18);
        if (environmentConfigPtr && *environmentConfigPtr) {
            void* environmentConfig = *environmentConfigPtr;
            String** backendHost = (String**)((uintptr_t)environmentConfig + 0x20);
            if (backendHost) {
                *backendHost = String::Create("https://backend-production-92a7.up.railway.app");
            }

            bool* displayEnviroment = (bool*)((uintptr_t)environmentConfig + 0x4A);
            if (displayEnviroment) {
                *displayEnviroment = true;
            }

            bool* allowTour = (bool*)((uintptr_t)environmentConfig + 0x30);
            if (allowTour) {
                *allowTour = true;
            }
        }
    }

    oldInitializer(instance);
}    

String* (*oldGetBotName)(void* instance);
String* getBotName(void* instance) {
    srand((unsigned)time(nullptr) ^ (unsigned)pthread_self());
    int botId = 1000 + (rand() % 9000);
    char botIdChar[32];
    snprintf(botIdChar, sizeof(botIdChar), "%d", botId);
    
    char fullText[32];
    snprintf(fullText, sizeof(fullText), ".gg/coresg<sup><#ffff00>%s", botIdChar);
    return String::Create(fullText);
}

bool (*originalIsUpdateAvailable)(void* instance);
bool isUpdateAvailable(void* instance) {
    return false;
}

void (*oldLobbyPopup)(void* instance);
void lobbyPopup(void* instance) {
}

void (*oldInicialize)(void* instance);
void inicialize(void* instance) {
}

void (*oldSend)(void* instance);
void asend(void* instance) {
}

bool (*oldSpecialEmoteFilter)(void* instance);
bool specialEmoteFilter(void* instance) {
    return true;
}

bool (*oldValidateTourxData)(void* instance);
bool validateTourxData(void* instance) {
    return true;
}

bool (*oldCustomParty)(void* instance);
bool customParty(void* instance) {
    return true;
}

bool (*oldTourx)(void* instance);
bool tourx(void* instance) {
    return true;
}

bool (*oldTourxMeta)(void* instance);
bool tourxMeta(void* instance) {
    return true;
}

bool (*oldCreatorcode)(void* instance);
bool creatorcode(void* instance) {
    return true;
}

bool (*oldUnlockCosmetic)(void* instance);
bool unlockCosmetic(void* instance) {
    return true;
}

bool (*originalIsUpdateRequired)(void* instance);
bool isUpdateRequired(void* instance) {
    return false;
}

void (*oldJoinTourx)(void* instance, void* tournament);
void joinTourx(void* instance, void* tournament) {
    oldJoinTourx(instance, tournament);
    inTourx = true;
}

void (*oldLeaveTourx)(void* instance, void* tournament);
void leaveTourx(void* instance, void* tournament) {
    oldLeaveTourx(instance, tournament);
    inTourx = false;
}

void (*oldRuntimeConfig)(void* instance, void* stream);
void runtimeConfig(void* instance, void* stream) {
    if (instance) {
        int* botCount = (int*)((uintptr_t)instance + 0x68);
        if (botCount && inTourx) {
            *botCount = 0;
        }
    }
    oldRuntimeConfig(instance, stream);
}

bool (*oldOpAuthenticate)(void* instance, String* appId, String* appVersion, void* authValues, String* regionCode, bool getLobbyStatistics);
bool opAuthenticate(void* instance, String* appId, String* appVersion, void* authValues, String* regionCode, bool getLobbyStatistics) {
    if (authValues) {
        uint8_t* authTypeField = (uint8_t*)((uintptr_t)authValues + 0x10);
        *authTypeField = 255;

        String** userIdField = (String**)((uintptr_t)authValues + 0x30);
        if (userIdField) {
            int playerId = getUserId();
            char buf[32];
            snprintf(buf, sizeof(buf), "%d", playerId);
            *userIdField = String::Create(buf);
        }
    }
    return oldOpAuthenticate(instance, appId, appVersion, authValues, regionCode, getLobbyStatistics);
}

void* (*oldCloneAppSettings)(void* appSettings);
void* cloneAppSettings(void* appSettings) {
    void* result = oldCloneAppSettings(appSettings);
    if (result) {
        String** appIdRealtime = (String**)((uint8_t*)result + 0x10);
        if (appIdRealtime) {
            *appIdRealtime = String::Create("3e8a970f-12be-41fc-b8d0-93c657234f85");
        }
    }
    return result;
}

String* (*oldGetTranslation)(void* instance, String* key, bool fixForRtl, int maxLineLengthForRTL, bool ignoreRTLnumbers);
String* getTranslation(void* instance, String* key, bool fixForRtl, int maxLineLengthForRTL, bool ignoreRTLnumbers) {
    const char* keyStr = key->getChars();
    return oldGetTranslation(instance, key, fixForRtl, maxLineLengthForRTL, ignoreRTLnumbers);
}

void* (*oldMapConfigGetter)(void* instance);
void* mapConfigGetter(void* instance) {
    void* result = oldMapConfigGetter(instance);
    if (result && setCustomGamemode == 1) { 
        int* levelType = (int*)((uint8_t*)result + 0x78);
        if (levelType) {
            *levelType = 0;
        }
     }
    return result;
}

String* (*oldGetMapName)(void* instance);
String* getMapName(void* instance) {
    String* result = oldGetMapName(instance);
    const char* nameChars = result->getChars();

    if (nameChars) {
        if (strcmp(nameChars, String::Create("Respawn Dash")->getChars()) == 0) {
            setCustomGamemode = 1;
            return String::Create("level19_block");
        }
        if (strcmp(nameChars, String::Create("Respawn Legendary")->getChars()) == 0)
        {
            setCustomGamemode = 1;
        
            return String::Create("eventlevel13_block_legendary");
        }
    }
    setCustomGamemode = 0;
    return result;
}


void* hackThread(void*) {
    int attempts = 0;
    do {
        sleep(1);
        attempts++;
        if (attempts > 30) {
            return nullptr;
        }
    } while (!isLibraryLoaded(targetLibName));

    HOOK_LIB("libil2cpp.so", "0x136C188", initializer, oldInitializer);
    HOOK_LIB("libil2cpp.so", "0x12C46D4", getTranslation, oldGetTranslation);
    HOOK_LIB("libil2cpp.so", "0x2D7DEF8", opAuthenticate, oldOpAuthenticate);
    HOOK_LIB("libil2cpp.so", "0x1373200", isUpdateAvailable, originalIsUpdateAvailable);
    HOOK_LIB("libil2cpp.so", "0x11C39CC", lobbyPopup, oldLobbyPopup);
    HOOK_LIB("libil2cpp.so", "0x1305C4C", unlockCosmetic, oldUnlockCosmetic);
    HOOK_LIB("libil2cpp.so", "0x14E5D28", customParty, oldCustomParty);
    HOOK_LIB("libil2cpp.so", "0x14E5304", creatorcode, oldCreatorcode);
    HOOK_LIB("libil2cpp.so", "0x149ED3C", tourxMeta, oldTourxMeta);
    HOOK_LIB("libil2cpp.so", "0x14998D8", tourxMeta, oldTourx);
    HOOK_LIB("libil2cpp.so", "0x12BAF00", inicialize, oldInicialize);
    HOOK_LIB("libil2cpp.so", "0x12B8A74", asend, oldSend);
    HOOK_LIB("libil2cpp.so", "0x1495958", validateTourxData, oldValidateTourxData);
    HOOK_LIB("libil2cpp.so", "0x1373214", isUpdateRequired, originalIsUpdateRequired);
    HOOK_LIB("libil2cpp.so", "0x11B4188", cloneAppSettings, oldCloneAppSettings);
    HOOK_LIB("libil2cpp.so", "0x13E2520", specialEmoteFilter, oldSpecialEmoteFilter);
    HOOK_LIB("libil2cpp.so", "0x14A20AC", joinTourx, oldJoinTourx);
    HOOK_LIB("libil2cpp.so", "0x149E1CC", leaveTourx, oldLeaveTourx);
    HOOK_LIB("libil2cpp.so", "0x3D683FC", runtimeConfig, oldRuntimeConfig);
    HOOK_LIB("libil2cpp.so", "0x3D40A18", mapConfigGetter, oldMapConfigGetter);
    HOOK_LIB("libil2cpp.so", "0x14250A4", getMapName, oldGetMapName);
    HOOK_LIB("libil2cpp.so", "0x11C504C", playViewController, oldPlayViewController);
    HOOK_LIB("libil2cpp.so", "0x1569318", loginViewController, oldLoginViewController);
    HOOK_LIB("libil2cpp.so", "0x1221DE8", uiController, oldUiController);
    HOOK_LIB("libil2cpp.so", "0x11CACAC", profileViewController, oldProfileViewController);
    HOOK_LIB("libil2cpp.so", "0x155D710", headerViewHelper, oldHeaderViewHelper);
    HOOK_LIB("libil2cpp.so", "0x13C777C", login, oldLogin);
    HOOK_LIB("libil2cpp.so", "0x1438194", getBotName, oldGetBotName);
    HOOK_LIB("libil2cpp.so", "0x155DDB4", headerLobbyViewHelper, oldHeaderLobbyViewHelper);
    HOOK_LIB("libil2cpp.so", "0x15D8DC4", backboneHttpClient, old_backboneHttpClient);
    HOOK_LIB("libil2cpp.so", "0x15D8E08", backboneInitialize, old_backboneInitialize);
    HOOK_LIB("libil2cpp.so", "0x12C2638", isVersionMinimium, old_isVersionMinimium);
    levelManagerInstance = (void (*))getAbsoluteAddress(targetLibName, 0x15675A8);

    buildString = (String * (*)(void*, char*, int, int))getAbsoluteAddress(targetLibName, 0x3A64F70);
    createSprite = (void* (*)(void*, void*, void*))getAbsoluteAddress(targetLibName, 0x3801E4C);
    gameObjectFind = (void* (*)(String*))getAbsoluteAddress(targetLibName, 0x37EA304);
    objectGetComponent = (void* (*)(void*, String*))getAbsoluteAddress(targetLibName, 0x37E9194);
    getTransform = (void* (*)(void*))getAbsoluteAddress(targetLibName, 0x37E9AB8);
    getParent = (void* (*)(void*))getAbsoluteAddress(targetLibName, 0x37FC6A8);
    vector3Ctor = (void* (*)(float, float, float))getAbsoluteAddress(targetLibName, 0x37F4A00);
    gameObjectSetActive = (void* (*)(void*, bool))getAbsoluteAddress(targetLibName, 0x37E9BF4);
    objectGetName = (String * (*)(void*))getAbsoluteAddress(targetLibName, 0x37EA87C);
    objectSetName = (void* (*)(void*, String*))getAbsoluteAddress(targetLibName, 0x37EA93C);
    scriptableObjectCreateInstance = (void* (*)(void*))getAbsoluteAddress(targetLibName, 0x37ED120);
    objectInstantiate = (void* (*)(void*, void*))getAbsoluteAddress(targetLibName, 0x37EAFA4);
    objectDestroy = (void* (*)(void*))getAbsoluteAddress(targetLibName, 0x37EB1F8);
    transformGetPosition = (void* (*)(void*))getAbsoluteAddress(targetLibName, 0x37FBBA8);
    transformSetPosition = (void* (*)(void*, void*))getAbsoluteAddress(targetLibName, 0x37FBC48);
    componentSetActive = (void* (*)(void*, bool))getAbsoluteAddress(targetLibName, 0x37E9BB0);
    toVec = (void* (*)(void*, float, float, float))getAbsoluteAddress(targetLibName, 0x37FD7C4);
    objectGetComponentInChildren = (void* (*)(void*, void*))getAbsoluteAddress(targetLibName, 0x37E922C);
    transformTranslate = (void* (*)(void*, float, float, float))getAbsoluteAddress(targetLibName, 0x37FCD78);
    activatorCreateInstance = (void* (*)(void*))getAbsoluteAddress(targetLibName, 0x3C11CA8);
    findObject = (void* (*)(void*))getAbsoluteAddress(targetLibName, 0x37EB714);
    levelEntry = (void* (*)(void*, void*))getAbsoluteAddress(targetLibName, 0x15675A8);
    getTraducao = (String* (*)(String*))getAbsoluteAddress(targetLibName, 0x12C46D4);
    getType =  (void* (*)(String*))getAbsoluteAddress(targetLibName, 0x3C02250);
    createUri = (void* (*)(String*))getAbsoluteAddress(targetLibName, 0x34E68B8);
    cloneCustomRoomProperties = (void* (*)(void*))getAbsoluteAddress(targetLibName, 0x142DB84);
    getHashtableItem = (void* (*)(void*, String*))getAbsoluteAddress(targetLibName, 0x2D06CF8);
    openPopup = (void* (*)(void*, String*, float, bool))getAbsoluteAddress(targetLibName, 0x15832F0);
    setPopupItems = (void* (*)(void*, String*, String*))getAbsoluteAddress(targetLibName, 0x1580630);
    getCurrencySprite =  (void* (*)(void*, String*))getAbsoluteAddress(targetLibName, 0x122588C);
    imageConversionLoadImage =  (bool (*)(void*, void*, bool))getAbsoluteAddress(targetLibName, 0x382A5F0);
    actacteCreateInstanceParams = (void* (*)(void*, void*[]))getAbsoluteAddress(targetLibName, 0x3C11C78);
    
    
    return NULL;
}

__attribute__((constructor))
void libMain() {
    pthread_t ptid;
    pthread_create(&ptid, NULL, hackThread, NULL);
}
