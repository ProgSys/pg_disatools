#pragma once

#include <array>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <Util/PG_Ray.h>
#include <Util/Color/RGBA.h>
#include <Disa/dataStructs/PG_DisaScriptManager.h>

static unsigned int patch_emptyFunc = 0;
static inline void emptyFunc(unsigned char* b) {}

#pragma pack(push, 1)
struct DisaDungeon {
    unsigned short difficulty; //used to spawn chracters that have the difficulty_spawn_rule set
    unsigned short sx2;

    unsigned short id;
    unsigned char bonusRank;
    bool active;


    unsigned short getMainCategoryId() const {
        return id > 0? id / 100 - 1: 0;
    }
};



//The DUNGEON.DAT table file form START.DAT
struct DisaDungeonDAT {
    std::array<unsigned short, 17> dungeonName; //in shift jis
    unsigned short bonusRank; //0x34
    unsigned short dungeonId; //0x36
    PG_DisaGameStateFilter filter;
};

/**
* @brief Stores the last key states
* Starts at 0x00778d98
*/
struct DisaKeyStates {
    enum Key : int {
        GeoEffectOnOff = 0, //Tab
        Btn1 = 1, //?
        Btn2 = 2, //?
        Expansion = 3, //Ctrl
        Up = 4, //W
        Right = 5, //D
        Down = 6, //S
        Left = 7,//A
        PreviousPage = 8, //Q
        NextPage = 9, //E
        Btn10 = 10, //O
        Menu = 12, //I
        Cancel = 13, //L (0x00778da5)
        Select = 14, //K (0x00778da6)
        Btn15 = 15, //J
        Invalid = 99,
    };

    std::array<unsigned char, 16> states;

    /**
    * @brief Will set all keys to 0
    * Used so detection don't happen twice
    */
    inline void clear() {
        states.fill(0);
    }

    inline bool isKeyPressed(Key key) {
        return states[key] == 2;
    }
};

struct DisaSettings {
    enum KeyAssigned : int {
        Menu,
        Cancel,
        Select,
        SwitchWindow,
        PreviousPage,
        NextPage,
        Up,
        Right,
        Down,
        Left,
        Expansion,
        GeoEffectVisibility,
        ToTheItemWorld = 20,
        HealAll = 21,
        Invalid = 99,
    };

    unsigned int field_0x0;
    unsigned int field_0x4;
    unsigned int field_0x8;
    unsigned int field_0xc;
    unsigned int field_0x10;
    unsigned int field_0x14;
    unsigned int field_0x18;
    unsigned short field_0x1c;
    unsigned short field_0x1e;
    bool isMouseActive; //0x20 does player use mause
    unsigned char field_0x21; //new/old text
    unsigned char field_0x22; //new/old cursor
    bool isHDTextures; //0x23
    unsigned int shadowMapQuality;
    unsigned char isUsePositionBlur;
    unsigned char isUseDepthOfField;
    unsigned char isUseSsao;
    unsigned char field_0x2b;
    unsigned char field_0x2c;
    unsigned char field_0x2d;
    unsigned char field_0x2e;
    unsigned char field_0x2f;
    unsigned int field_0x30; //new/old ui style
    std::array<unsigned char, 76> field_0x34; //unknown
    std::array<unsigned char, 31> assignedKeys; //0x80 List of assigned keys by user (navigate UI, gameplay and so on), see KeyAssigned
    std::array<unsigned char, 31> assignedControllerKeys; //0x9fList of assigned keys by user (navigate UI, gameplay and so on), see KeyAssigned
    std::array<unsigned char, 1135> field_0xbe; //unknown

    /**
    * @returns true if a controler is used
    */
    bool isController() const {
        return static_cast<bool>(field_0xbe[0x46e]);
    }

    /**
    * @returns true when map faces are sorted and batched into VBOs (FUN_004f02e0) instead of drawn one quad at a time
    * (FUN_004f1070). Options menu entry 0xd, byte 0x37.
    */
    bool isBatchedMapRendering() const {
        return static_cast<bool>(field_0x34[3]);
    }

    inline DisaKeyStates::Key getAssignedKey(KeyAssigned key) const{
        return (DisaKeyStates::Key)assignedKeys[key];
    }

    static DisaKeyStates::Key toKeyState(KeyAssigned key) {
        switch (key)
        {
        case DisaSettings::KeyAssigned::Cancel: return DisaKeyStates::Cancel; 
        case DisaSettings::KeyAssigned::Select: return DisaKeyStates::Select;
        case DisaSettings::KeyAssigned::Menu:return  DisaKeyStates::Menu; 
        case DisaSettings::KeyAssigned::GeoEffectVisibility: return DisaKeyStates::GeoEffectOnOff; 
        default: return static_cast<DisaKeyStates::Key>(0);
        }
    }

};

struct DisaCutout {
    unsigned int y = 0, x = 0, width = 0, height = 0;

    static DisaCutout* getXBoxController();
    static DisaCutout* getPlayController();
};

struct DisaKeyToAtlasMapping {
    int key;
    DisaCutout cutout;

    static DisaKeyToAtlasMapping* getKeyboard();
};




/**
* @brief Defines a simple trasformation struct
*/
struct DisaTransform {
    glm::vec4 position = glm::vec4(0.f, 0.f, 0.f, 1.f);
    glm::vec4 rotation = glm::vec4(0.f, 0.f, 0.f, 1.f);
    glm::vec4 scale = glm::vec4(1.f, 1.f, 1.f, 1.f);

    void clear() {
        position = glm::vec4(0.f, 0.f, 0.f, 1.f);
        rotation = glm::vec4(0.f, 0.f, 0.f, 1.f);
        scale = glm::vec4(1.f, 1.f, 1.f, 1.f);
    }
};





struct DisaCameraInfo {
    glm::vec3 eye; //0x0
    float field_0xc;
    float field_0x10;
    float field_0x14;
    float field_0x18;
    float field_0x1c;
    glm::vec3 up; //0x20
    float field_0x2c;
    glm::vec3 field_0x30; //something rotation
    float field_0x3c;
    float field_0x40;
    float field_0x44;
    float field_0x48;
    float field_0x4c;
    float field_0x50;
    float field_0x54;
    float field_0x58;
    float field_0x5c;
    float field_0x60;
    float field_0x64;
    float field_0x68;
    float field_0x6c;
    float field_0x70;
    float field_0x74;
    float field_0x78;
    float field_0x7c;
    glm::vec3 center; //0x80
    float field_0x8c;
    glm::vec3 eyeOffset; //0x90
    glm::vec3 rotation; //in radiance
    float zoomOffset; //0xa8 Current zoom value added to the base zoom
    glm::vec3 eyeOffsetTarget; //0xac
    glm::vec3 eyeOffsetMovement; //0xb8
    float field_0xc4;
    float eyeOffsetSpeed; // 0xc8
    float zoomTarget; // 0xcc
    float zoomMovement; // 0xd0
    float zoomSpeed; // 0xd4 How long it takes to reach from zoomOffset to zoomTarget
    glm::vec3 rotationTarget; // 0xd8
    glm::vec3 rotationMovement; // 0xf0 change over time
    float rotationSpeed;
    float field_0xf4;
    float field_0xf8;
    float field_0xfc;
    float field_0x100;
    float field_0x104;
    float field_0x108;
    float field_0x10c;
    float field_0x110;
    float field_0x114;
    float field_0x118;
    float field_0x11c;
    float field_0x120;

    /**
    * Moves camera towards targetPosition with given speed.
    * Is called each tick.
    * Note, if a speed is given this will ONLY move the camera.
    * FUN_004f8350
    * @param speed if bigger zero then camera will move to the target over time, otherwise movement will be instend.
    */
    void move(glm::vec3 targetPosition, float speed = 0.f);
    /**
    * Rotates camera around center towards targetRotation with given seed.
    * Is called each tick.
    * Note, if a speed is given this will ONLY rotate the camera.
    * FUN_004f8500
    * @targetRotation Target rotation in radiance
    * @param speed if bigger zero then camera will move to the target over time, otherwise movement will be instend.
    */
    void rotate(glm::vec3 targetRotation, float speed = 0.f);
    /**
    * Zoom the camera towards target zoom level with given seed.
    * Is called each tick.
    * Note, if a speed is given this will ONLY zoom the camera.
    * FUN_004f84a0
    * @targetZoom Target zoom (distance from look at point)
    * @param speed if bigger zero then camera will zoom to the target over time, otherwise movement will be instend.
    */
    void zoom(float targetZoom, float speed = 0.f);
    /**
    * @brief Default camera processing tick 
    * Will proccess everything - position, rotation, zoom, etc.
    * FUN_004f8800
    */
    void processTick();
    /**
    * @brief Will compute the a view ray at given cursor and window size
    * Is custom function.
    * @details Unlike FUN_004d1070 the cursor is not truncated to the 480x272 PSP screen and the unprojection runs in double.
    * The ray is in the drawn space of the screen matrix at 0x00819200, so its y is the negated game height.
    * @param cursor The cursor in client pixels, like GetCursorPos and ScreenToClient return it.
    * @param windowSize The client size, GetClientRect without the window frame.
    * @return The ray from the near plane into the scene, its direction is normalized.
    */
    PG_Ray getRay(glm::ivec2 cursor, glm::ivec2 windowSize) const;
    /**
    * @brief Will compute the a view ray at current cursor postion
    * Is custom function.
    * @details Cursor and client size come from DisaCustomApplication, so it needs the first frame to have run.
    * @return The ray from the near plane into the scene, its direction is normalized.
    */
    PG_Ray getRayFromCursor() const;
};


/**
* @brief Like a std::vector
*/
template<typename T>
struct BasicList {
    T* data;
    unsigned int size;

    inline T& operator[](unsigned int i) noexcept { return *(data + i); }
    inline const T& operator[](unsigned int i) const noexcept { return *(data + i); }

    inline T* begin() noexcept { return data; }
    inline const T* begin() const noexcept { return data; }
    inline T* end() noexcept { return data + size; }
    inline const T* end() const noexcept { return data + size; }

    inline void push_back(T&& v) noexcept {
        *(data + size) = v;
        ++size;
    }
    inline void push_back(const T& v) noexcept {
        *(data + size) = v;
        ++size;
    }
};


/**
* @brief Like a std::array
*/
template<typename T>
struct ArrayList {
    unsigned int size;
    unsigned int pad1;
    unsigned int pad2;
    T data;

    inline T& operator[](unsigned int i) noexcept { return *(&data + i); }
    inline const T& operator[](unsigned int i) const noexcept { return *(&data + i); }

    inline T* begin() noexcept { return &data; }
    inline const T* begin() const noexcept { return &data; }
    inline T* end() noexcept { return &data + size; }
    inline const T* end() const noexcept { return &data + size; }

    inline void push_back(T&& v) noexcept {
        *(&data + size) = v;
        ++size;
    }
    inline void push_back(const T& v) noexcept {
        *(&data + size) = v;
        ++size;
    }

    inline void clear() noexcept {
        size = 0;
    }
};

#pragma pack(pop)

// Known memory addresses
constexpr unsigned int aDungeonEntries = 0x00653aa0; //DisaDungeon
constexpr unsigned int aDungeonDATEntries = 0x00664020; //DisaDungeonDAT, begin of list (of .DAT file)
constexpr unsigned int aDungeonDATSize = 0x00664024; //DisaDungeonDAT number of entries in list
constexpr unsigned int aDisplayDungeonEntries = 0x0088ac90; //int
constexpr unsigned int aNumberOfDungeonEntries = 0x0088ac84; //int
constexpr unsigned int aDisplayDungeonEntriesScrollOffset = 0x0088abf0; //int, used for scrolling the doungeon list
constexpr unsigned int aDisplayDungeonEntriesHoverIndex = 0x0088abec; //int
constexpr unsigned int aKeyStates = 0x00778d98; //DisaKeyStates
constexpr unsigned int aGlobalStateBits = 0x00619330; //The state of the game (bitmask)
constexpr unsigned int aUnitsOwned = 0x0065db10; //The number of units the player has (ushort)
//assebly
constexpr unsigned int aAssemblyUnitInfos = 0x00619598; //A list of character header infos slots that will be inside the assembly (size 288)
constexpr unsigned int aAssemblyUnitInfosLookup = 0x0088ba10; //A list of indeces (unsigned short) to slots in aAssemblyUnitInfos where characterId is set (max size 288)
constexpr unsigned int aAssemblyUnitInfosLookupSize = 0x0088b588; //unsigned short, the size of aAssemblyUnitInfosLookup

// Known function addresses
constexpr unsigned int fRenderCutout = 0x0050e9e0; //imagePtr?, colloring, ?
constexpr unsigned int fRenderTextBox = 0x004dd3f0; //x, y, width (-1 auto with), rows, fontAtlasPtr, ?, str (shift Jis or string, however then invalid size calc)
constexpr unsigned int fRenderImg = 0x0050e5d0; //func
constexpr unsigned int fRenderBgBox = 0x004f6a40; //func
constexpr unsigned int fRenderTextOld = 0x004f4d20; //x, y, ?, ?, stringPtr, terninator?, ?
constexpr unsigned int fRenderTextNew = 0x0050c4d0; //func
constexpr unsigned int fGetWidthOfString = 0x004f3930; //stringPtr, ?
constexpr unsigned int fPlaySound = 0x004de090; // Play a sound (like howering a button) sound Index, volume?, ?, ?, ?
constexpr unsigned int fHandleListScrolling = 0x004d0b70; // Scroll list via cursor or mouse wheel: current hover index, scroll offset index, number of list entries, last scroll index, selection changed?
constexpr unsigned int fIsMouseCursorInAABB = 0x0050f590; // is true if mouse cursor is inside AABB
constexpr unsigned int fIsLeftMouseBtnClicked = 0x0050f7c0; // is true if left mouse was clicked
constexpr unsigned int fIsExitPressed = 0x0050f950; // is true if ESC or right mouse (only once?) was pressed
constexpr unsigned int fTeleportPannelX = 0x0065dae8; 
constexpr unsigned int fTeleportPannelZ = 0x0065daec;
constexpr unsigned int fRender3DObject = 0x004e13a0;//pointer to 3D data, frame value, pointer to camera info, DisaTransform, ...

//custom fields/data
constexpr unsigned short vCustomCampaignsCategoryIdentifier = 0x8000;

//fitlers
constexpr PG_DisaGameStateFilter filterPlayStory = { 0, 275 };

/**
* @brief Render a simple background box
* The same as for text, but without the text
* @param x Coord from 0 to 480. Is center of box if centered is true
* @param y Coord from 0 to 272.
*/
void _cdecl renderSimpleBgBox(unsigned int x, unsigned int y, int width, int height);

enum EMenuOption: int {
    MenuOption_Exit,
    MenuOption_SkipStory,
    MenuOption_Select,
    MenuOption_SelectHidden,
    MenuOption_None,
};

/**
* @brief Process if a value was selected
* @returns The selected entry.
*/
EMenuOption processMenuBox(DisaSettings* settings, const std::vector<EMenuOption>& options = { MenuOption_Exit });
/**
* @brief Render the navigation menu at the bottom corner
*/
void renderMenuBox(DisaSettings* settings, const std::vector<EMenuOption>& options = { MenuOption_Exit });

/**
* @brief Process a simple text box at given position
* @returns true when selected
*/
bool processSingleLineTextBox(const DisaSettings* settings, glm::ivec2 pos, const std::string& text, bool& isHover, DisaSettings::KeyAssigned = DisaSettings::KeyAssigned::Invalid, unsigned int width = 0);
/**
* @brief Render a singleLineTextBox
*/
void renderSingleLineTextBox(const DisaSettings* settings, glm::ivec2 pos, const std::string& text, bool isHover, DisaSettings::KeyAssigned = DisaSettings::KeyAssigned::Invalid, unsigned int width = 0);


void __fastcall FUN_RederTextNew(int x, int y, int param_3, int* param_4, unsigned short* jis_string, float param_6, float param_7, int param_8, int param_9, int param_10);

void __fastcall FUN_0050c210_toUtf8(char* str, char* outStr, size_t* outLength);

int __fastcall FUN_GetWidthOfString(char* str, int param_2, int param_3, int* param_4, int param_5);

/// Replaces the string with its translation when the translation manager has one, FUN_GetWidthOfString does it first.
void __fastcall translateReplace(char*& jis_string);

/**
* @brief The fill of a progress bar in the new UI style (HP, SP, weapon mastery, skill EXP, rank, aptitude).
* Drawn as a left cap, a stretched middle and a right cap from DAT_0074ce28, filled up to (width - 4) * value / max.
* The track behind it is FUN_004a6cf0 or part of the background. See reverse/ProgressBars.md.
* While g_renderBarNewCustom is set (default) it submits one M_DisaProgressBar rectangle instead.
* @param depth Render priority, like 0xc7a7.
* @param width The full width of the bar, the same as given to the track.
* @param max Must not be 0, the callers test it.
* @param colorRow The row in DAT_0074ce28, 2 is the yellow of the progress bars, HP and SP pass the team.
* @param drawEndCap False for a bar that is stacked on top of another one.
*/
void __fastcall FUN_004a6af0_drawBarNew(int x, int y, int depth, int width, int64_t value, int64_t max, int colorRow, bool drawEndCap);

/// The game's call sites remove the 32 pushed bytes themselves, this forwards to FUN_004a6af0_drawBarNew without doing it twice.
void FUN_004a6af0_drawBarNew_NoneStd();

//======== FUNCTIONS ========

/**
* @brief Function that loads the list of dungeon categories ids into memory when entring dungeons menu
* Fills the number of entires and a list of catagores ids of type unsigned short.
* @see aNumberOfDungeonEntries
* @see aDisplayDungeonEntries
*/
void FUN_LoadDungeonsCategories();

/**
* @brief Function that renders the dungens categories screen
*/
void FUN_00471240_RenderDungeonScreeen(void);

/**
* @brief The dungeon categories, like Tutorial
* Handles button interactions and then call render function.
*/
void FUN_00471780_ProccessCategoryDungeonsScreen();
 


/**
* @brief The dungeons inside a catagory
* Handles button interactions and then call render function.
*/
void _stdcall FUN_00472a40_ProccessDungeonsScreen();

void* __fastcall FUN_0050a1f0(int param_1, int** param_2, int param_3, char param_4);

/**
* @brief Reads a game file into memory.
* @param fileData Target buffer, allocated when it points to nullptr.
*/
void FUN_0050a1f0_GetFileData(int fileIndex, int** fileData, bool param_4);

//call SCRIPT
void __fastcall FUN_004b6420(unsigned int param_1, int param_2); //procces script command

//end map?
void __fastcall FUN_00446e20(int param_1, int param_2);

//reenter after battle ends
void __stdcall FUN_004710d0();


//rendering
/**
* @brief Top of the game loop, called once per frame before anything is submitted.
* Its only guard is the PeekMessage check that gates the whole frame body.
*/
void __cdecl FUN_00424ae0_FrameBegin();

void _stdcall FUN_004201d0_RenderClearBegin(); //called at the begin of each frame to clear buffers

void _stdcall FUN_glViewport(int x, int y, int width, int height); //render Begin

void _stdcall FUN_glDrawArrays(unsigned int mode, int first, int count); //render inbetween

#include <Disa/fileformats/PG_GEO.h>
struct DisaObjectScrollInfo {
    float* uvScrollX;
    float* uvScrollY;
    int8_t* uvScrollSpeedX;
    int8_t* uvScrollSpeedY;
};
/*!
* @brief Points to the loaded geometry file and contains the curret state how the geomery should be rendered
*/
struct DisaObjectModel {
    glm::frgba* m_colorMultiplier; //0x0 per-item color multipliers (r,g,b,a)
    uint32_t m_field0x4; //0x4 Decompiler never reads?
    DisaObjectModel* m_self; //0x8 pointer to self for some reason
    float m_poligonOffset; //0xC 
    uint32_t m_field0x10; //0x10 
    uint32_t m_field0x14; //0x14 
    uint32_t m_field0x18; //0x18 
    geofileMeshHeader* m_geofileHeader; //0x1C Pointer to geometry file header. Defines how the mesh is structured .
    glm::vec3* m_triangles; //0x20 
    glm::vec2* m_uvs; //0x24 
    glm::vec3* m_tangentAndBitangent; //0x28
    uint32_t m_numberOfTriangels; //0x2C 
    uint32_t m_field0x30; //0x30 same as unk9 field from geofileMeshHeader
    uint32_t m_numberOfSurfaces; //0x34
    uint32_t m_numberOfTextures; //0x38
    const geofileSurface* m_geofileSurfaces; //0x3C Pointer to geometry file header texture settings. Defines how the surface looks.
    std::array<geofileTextureHeader*, 31> m_textures; //0x40 Pointers to geometry file colore textures settings. (first texture is present twice)
    int** m_textureDatas; //0xBC
    std::array<uint32_t, 30> m_field0xC0; //0xC0
    int** m_textureHDDatas; //0x138
    std::array<uint32_t, 13> m_field0x13c; //0x13c
    float m_alphaScale; //0x170
    std::array<uint32_t, 2> m_unknown0x174; //0x174
    DisaObjectScrollInfo* m_uvScrollObject; //0x17C
    glm::vec2 m_uvScroll; //0x180 copy from  m_uvScrollObject
    int* m_field0x188; //0x188
    uint m_field0x18c; //0x18c
    glm::vec2 m_uvScrollSpeed; //0x190 copy from  m_uvScrollObject

    void __thiscall FUN_004e2180(glm::mat4* param1, glm::frgba* colorMultiplier, uint32_t param_3, float param_4);
};

struct DisaMemeoryManager {
    unsigned char* __thiscall FUN_00425bb0( int requestedMemorySize);
};