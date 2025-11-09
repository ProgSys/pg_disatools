
#include <FunctionPatcher.h>
#include <Managers/DisaCustomCampaignManager.h>
#include <Managers/DisaCustomBaseModManager.h>
#include <Managers/DisaCustomCharacterManager.h>
#include <Managers/DisaCustomScriptManager.h>
#include <DisaFunctions.h>
#include <DisaTalkScriptFunctions.h>
#include <DisaScriptFunctions.h>
#include <DisaCharacterFunctions.h>
#include <DisaDataFunctions.h>
#include <DisaScriptManager.h>
#include <Windows.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_StringUtil.h>
#include <OS/Functions/Clipboard.h>
#include <Base/PG_Clipboard.h>
#include <Resources/PG_ResourceManager.h>
#undef max
#undef min

#include <iostream>
#include <fstream>
#include <thread> 
#include <sstream>
#include <vector>
#include <ranges>
#include <mutex>

using FUN_glViewportF = void(_stdcall*)(int, int, int, int);
static FUN_glViewportF FUN_glViewportPointer;


using FUN_glDrawArraysF = void(_stdcall*)(unsigned int, int, int);
static FUN_glDrawArraysF FUN_glDrawArraysPointer;

static size_t gRenderTextNewFuncs[] = {
    0x00440e9b, 0x00440f50, 0x00440fc2, 0x0044100c, 0x0044106d, 0x0044117c, 0x0044127f, 0x004412b8, 0x00441756, 0x004417f0, 0x0044186f, 0x004418cd, 0x00441924, 0x00441985, 0x004419f4, 0x00441a68, 0x00441bb1, 0x00441c53, 0x00441cd2, 0x00441d30, 0x00441dfc, 0x00441e9e, 0x00441f1d, 0x00441fbd, 0x0044205f, 0x00442135, 0x004421aa, 0x00442cd2, 0x00442d4d, 0x00442e07, 0x00442fae, 0x0044523c, 0x00445b8b, 0x00445c18, 0x00448014, 0x004480e9, 0x0044815e, 0x0044a0eb, 0x0044a170, 0x0044a1df, 0x0044a26f, 0x0044a2dd, 0x0044a341, 0x0044db23, 0x0044dbaa, 0x0044dbf4, 0x0044dc6c, 0x0044de0c, 0x0044de9d, 0x0044df0b, 0x0044df76, 0x0044dfd6, 0x0044e916, 0x0044f316, 0x0044f3c1, 0x0044f483, 0x0044f524, 0x0044f60e, 0x0044f668, 0x0044f706, 0x0044f7b6, 0x0044f82d, 0x0044f8b8, 0x0044f957, 0x0044f9cc, 0x0044fa54, 0x0044faee, 0x0044fb9f, 0x0044fcd9, 0x0044fd22, 0x0044fdb4, 0x0044fe6d, 0x0044fee4, 0x0044ff72, 0x0044ffd6, 0x00451139, 0x004511e0, 0x0045127e, 0x004512f1, 0x0045135c, 0x004513d8, 0x0045146a, 0x004514e4, 0x00451569, 0x004515da, 0x0045164e, 0x004516e6, 0x0045174c, 0x004517c2, 0x0045182b, 0x00451897, 0x00451900, 0x00451972, 0x00451c49, 0x0045273e, 0x0045284d, 0x00452fcf, 0x00453099, 0x00453236, 0x004533fb, 0x004534f1, 0x00453c5e, 0x00453ee2, 0x00453f86, 0x0045434a, 0x00454417, 0x00454547, 0x00454768, 0x004547c1, 0x0045496b, 0x00454b90, 0x00454c3f, 0x00454cd3, 0x00454d45, 0x00454dc6, 0x00454f49, 0x00454ff6, 0x004550a0, 0x0045511b, 0x004551c1, 0x0045523c, 0x004552e2, 0x00455357, 0x004553fd, 0x00455472, 0x00455518, 0x004555b0, 0x00455682, 0x004556ec, 0x00455765, 0x004557de, 0x00455851, 0x004558c4, 0x0045599b, 0x00455a0b, 0x00455a7b, 0x00455af1, 0x00455b61, 0x00455bd7, 0x00455c47, 0x00455cb7, 0x00455d27, 0x00455d97, 0x00455e2c, 0x00455e9d, 0x00455f22, 0x00455fd3, 0x00456040, 0x004560bc, 0x0045627e, 0x00456323, 0x004563a5, 0x004564dc, 0x00456732, 0x0045686c, 0x004568c5, 0x00456a36, 0x00456ac9, 0x00456b4f, 0x00456bce, 0x00456c4d, 0x00456ccc, 0x00456dd6, 0x00456e1f, 0x00456e98, 0x00456f08, 0x00456f71, 0x00456fda, 0x004570ea, 0x004571c1, 0x00457277, 0x004573c3, 0x00471442, 0x00471733, 0x00471c29, 0x004721ce, 0x00472242, 0x004722b6, 0x004722ff, 0x0047235c, 0x004723bb, 0x004724a9, 0x0047251b, 0x00472598, 0x0047260a, 0x00472698, 0x0047270a, 0x00472794, 0x004727f5, 0x00472863, 0x004728c4, 0x0047294e, 0x004729af, 0x004732d6, 0x004739f1, 0x00473a68, 0x00473ada, 0x00473b24, 0x00473b86, 0x00473be5, 0x00474520, 0x00474f30, 0x00474fa8, 0x0047501e, 0x00475091, 0x00475104, 0x0047515a, 0x004751bd, 0x00475221, 0x00475282, 0x004752e3, 0x004773c6, 0x00477432, 0x004774d3, 0x00477ac0, 0x00477e3e, 0x00477f05, 0x00478262, 0x00479900, 0x00479976, 0x004799ea, 0x00479a6b, 0x00479b32, 0x00479ba6, 0x00479c1a, 0x00479c8e, 0x00479d02, 0x00479d4e, 0x00479db3, 0x00479e1b, 0x00479e90, 0x00479ef8, 0x00479f4c, 0x00479fb4, 0x0047a01c, 0x0047a084, 0x0047a0ec, 0x0047a547, 0x0047a63d, 0x0047a6b1, 0x0047a723, 0x0047a795, 0x0047a807, 0x0047a879, 0x0047a8eb, 0x0047a935, 0x0047a998, 0x0047a9fc, 0x0047aa60, 0x0047aac4, 0x0047ab28, 0x0047ab8c, 0x0047b1a8, 0x0047b6d7, 0x0047b93f, 0x0047bd0f, 0x0047c1af, 0x0047c21e, 0x0047c28b, 0x0047c2f8, 0x0047c372, 0x0047c3e3, 0x0047c430, 0x0047c48e, 0x0047c4ec, 0x0047c54a, 0x0047c5b5, 0x0047c618, 0x0047ca1f, 0x0047ca93, 0x0047cb8e, 0x0047cd08, 0x0047cd83, 0x0047ce08, 0x0047ce82, 0x0047cefc, 0x0047d2b0, 0x0047d321, 0x0047d385, 0x0047d3e5, 0x0047dced, 0x0047e34f, 0x00481b03, 0x00481b83, 0x00481c02, 0x00481c68, 0x00481cd7, 0x00481d45, 0x0048300a, 0x00483186, 0x004832fd, 0x0048346a, 0x004835c2, 0x00483623, 0x00483687, 0x004836f6, 0x00483872, 0x00483a0c, 0x00483c00, 0x00483dfa, 0x00483fec, 0x004841dc, 0x004842d5, 0x00484362, 0x004843c6, 0x00484418, 0x0048447c, 0x004844e0, 0x00484546, 0x00484677, 0x004847ac, 0x004848dd, 0x00484a16, 0x00484b45, 0x00484f66, 0x00484ffa, 0x0048505b, 0x004850bf, 0x004852ac, 0x00485335, 0x004853a7, 0x004853f1, 0x00485452, 0x0048713f, 0x00487193, 0x00487269, 0x004872f0, 0x00487362, 0x004873ac, 0x0048740d, 0x0048d3ac, 0x0048d559, 0x0048d69c, 0x0048dabe, 0x0048dafd, 0x0048dd9f, 0x0048de5e, 0x0048ed96, 0x0048eefa, 0x0048ef3c, 0x0048effd, 0x0048f130, 0x0048f258, 0x0048f2eb, 0x0048f396, 0x0048f410, 0x0048f4bb, 0x0048f541, 0x0048f5ec, 0x0048f684, 0x0048f7c3, 0x0048f86e, 0x0048f8e8, 0x0048f993, 0x0048fa19, 0x0048fac4, 0x0048fb9a, 0x0048fc0f, 0x0048fc90, 0x0048fd45, 0x0048fe08, 0x0048fe7c, 0x0048ff38, 0x0048ffac, 0x00490074, 0x004900f7, 0x0049018a, 0x00490268, 0x004902ab, 0x00490350, 0x004903fb, 0x0049047c, 0x00490515, 0x004905b7, 0x0049067c, 0x004906f0, 0x004907b3, 0x004913c3, 0x004917c3, 0x00491974, 0x00491a00, 0x00491ab8, 0x00491b18, 0x00491b81, 0x00491bf8, 0x00491cdb, 0x00491d52, 0x00491de7, 0x0049365b, 0x00493aec, 0x00493b5b, 0x00493bb9, 0x00493c15, 0x00493cc8, 0x00493d60, 0x00493f05, 0x00493f7d, 0x004951a7, 0x004954f4, 0x00495553, 0x004956bd, 0x0049576c, 0x004957e8, 0x00495893, 0x00495932, 0x004959e1, 0x00495a7e, 0x00495b1b, 0x00495bdf, 0x00495c53, 0x00495d11, 0x00495fc0, 0x0049601f, 0x00496c74, 0x00496e3f, 0x00496eee, 0x00496f6b, 0x00497016, 0x0049708e, 0x00497139, 0x004971b1, 0x00497255, 0x00497304, 0x00497381, 0x0049742c, 0x004974a4, 0x0049754f, 0x004975e8, 0x00497664, 0x004976db, 0x00497778, 0x00497841, 0x004978b5, 0x00497978, 0x004979ec, 0x00497aaa, 0x00497b1e, 0x00497bdc, 0x00498b2a, 0x00498b98, 0x00498c0a, 0x00498c7c, 0x00498cbf, 0x00498d20, 0x00498d84, 0x00498e65, 0x00498ed7, 0x00498f21, 0x00498f82, 0x00499585, 0x00499872, 0x004998d8, 0x00499934, 0x00499998, 0x00499a09, 0x00499a7a, 0x00499c43, 0x00499ca9, 0x00499d34, 0x00499dab, 0x00499e0e, 0x00499e7d, 0x00499ee9, 0x00499f49, 0x00499fab, 0x0049a079, 0x0049a0eb, 0x0049a135, 0x0049a196, 0x0049abc7, 0x0049adab, 0x0049ae5a, 0x0049aed5, 0x0049af80, 0x0049affb, 0x0049b094, 0x0049b143, 0x0049b1be, 0x0049b269, 0x0049b305, 0x0049b37b, 0x0049b417, 0x0049b4d7, 0x0049b54b, 0x0049b608, 0x0049b67c, 0x0049b739, 0x0049b7df, 0x0049b879, 0x0049c8bb, 0x0049cacc, 0x0049ccab, 0x0049cd66, 0x0049cdea, 0x0049cea3, 0x0049cf4d, 0x0049cf97, 0x0049d02c, 0x0049d139, 0x0049d2fa, 0x0049d3ca, 0x0049d44e, 0x0049d4b1, 0x0049d707, 0x0049d7ef, 0x0049d867, 0x0049d922, 0x0049da06, 0x0049da69, 0x0049dedc, 0x0049df50, 0x0049dfc2, 0x0049e049, 0x0049e0ac, 0x0049e110, 0x004a023d, 0x004a028b, 0x004a639d, 0x004a64b9, 0x004a6573, 0x004a6ef7, 0x004a6fcd, 0x004a707a, 0x004a74ba, 0x004a75f6, 0x004a7897, 0x004a7cc6, 0x004a7f29, 0x004a8707, 0x004a87c5, 0x004a8846, 0x004a88fc, 0x004a897d, 0x004a8a27, 0x004a8acc, 0x004a8b43, 0x004a8c3b, 0x004a8caa, 0x004a8d1c, 0x004a8d83, 0x004a8df5, 0x004a8e5c, 0x004a9126, 0x004a9253, 0x004a9353, 0x004a9feb, 0x004aa106, 0x004aac36, 0x004aad30, 0x004ab5a2, 0x004ab67c, 0x004ab7ec, 0x004abae7, 0x004ac0ed, 0x004ac316, 0x004ac85a, 0x004ac90b, 0x004ac9af, 0x004aca54, 0x004acac6, 0x004acb4d, 0x004acbb6, 0x004acc28, 0x004acc91, 0x004acd03, 0x004acd6c, 0x004ad2bb, 0x004ad37d, 0x004ad441, 0x004ad505, 0x004ad5c9, 0x004ad688, 0x004ad6fd, 0x004ad779, 0x004ad7f0, 0x004ad86c, 0x004ad8e3, 0x004ad95f, 0x004ad9d6, 0x004ada52, 0x004adaf9, 0x004adb6f, 0x004adbe6, 0x004adc56, 0x004adccd, 0x004add3d, 0x004addb4, 0x004ade24, 0x004ade9b, 0x004adf0e, 0x004ae086, 0x004ae0d6, 0x004ae23d, 0x004ae3cb, 0x004ae70c, 0x004ae982, 0x004aeb5f, 0x004af3a2, 0x004af62f, 0x004af8c5, 0x004afdf0, 0x004b09f3, 0x004b0a3c, 0x004b0ad2, 0x004b0cd0, 0x004b0d5a, 0x004b0dbb, 0x004b0e3a, 0x004b0f08, 0x004b12fe, 0x004b1634, 0x004b1786, 0x004b17f8, 0x004b1cf3, 0x004b2889, 0x004b330f, 0x004b33a0, 0x004b3a73, 0x004b3af4, 0x004b3b65, 0x004b3be5, 0x004b3c49, 0x004b3dd2, 0x004b3e1c, 0x004b3f07, 0x004b3f7a, 0x004b407e, 0x004b40c7, 0x004b41c3, 0x004b4227, 0x004b4289, 0x004b4395, 0x004b4408, 0x004b4479, 0x004b44ea, 0x004b4536, 0x004b4598, 0x004b45fa, 0x004b465c, 0x004b4739, 0x004b47ab, 0x004b47f5, 0x004b4856, 0x004b4d74, 0x004b4dc3, 0x004b4ef8, 0x004b4f64, 0x004b506c, 0x004b5112, 0x004b5185, 0x004b525a, 0x004b52f9, 0x004b5365, 0x004b53e9, 0x004b5455, 0x004bf68b, 0x004c21c2, 0x004c2239, 0x004c2c77, 0x004c2cf1, 0x004c2d88, 0x004c2dee, 0x004c32be, 0x004c3359, 0x004c33bd, 0x004c3410, 0x004c349c, 0x004c3b28, 0x004c3e42, 0x004c3ea3, 0x004c3ef7, 0x004c3f7d, 0x004c3ff2, 0x004c41d0, 0x004c4539, 0x004c457f, 0x004c477c, 0x004c4ec2, 0x004c4f5a, 0x004c6b44, 0x004c6c73, 0x004c6cea, 0x004c7444, 0x004c7a3e, 0x004c7afd, 0x004c88a8, 0x004c891b, 0x004c897b, 0x004c89dd, 0x004c9069, 0x004c9166, 0x004c983b, 0x004c98a0, 0x004c9904, 0x004c9966, 0x004c9d65, 0x004c9e64, 0x004c9ee5, 0x004c9f8f, 0x004ca006, 0x004ca0ac, 0x004ca123, 0x004ca1c9, 0x004ca262, 0x004caaab, 0x004cab1e, 0x004cab8f, 0x004cac00, 0x004cac49, 0x004cacab, 0x004cad0d, 0x004cad6f, 0x004cb8ea, 0x004cb961, 0x004cb9ad, 0x004cba0f, 0x004cbc89, 0x004cbcff, 0x004cbd75, 0x004cbdbf, 0x004cbe20, 0x004cbe84, 0x004cbf5d, 0x004cbfbe, 0x004cc022, 0x004cc086, 0x004cc0ea, 0x004cc14e, 0x004cc1b2, 0x004cc311, 0x004cc372, 0x004cc3d6, 0x004cc43a, 0x004cc5ea, 0x004cc660, 0x004cc6d6, 0x004cc74c, 0x004cc81f, 0x004cc898, 0x004cc91d, 0x004cc992, 0x004ccbc6, 0x004ccc56, 0x004ccce6, 0x004ccd30, 0x004ccdad, 0x004cce2a, 0x004dd28b, 0x004dd307, 0x004dd3bd, 0x004dd4f7, 0x004f7c71, 0x004f7d30, 0x004f93da, 0x004f9559, 0x004f96db, 0x004f974f, 0x004f9799, 0x004f97fc, 0x004f9b2c, 0x004fa110, 0x004fa281, 0x004fa66c, 0x004fa7a8, 0x004fa823, 0x004faaba, 0x004fab3b, 0x004fab7f, 0x004fabf2, 0x004fad38, 0x004fadb9, 0x004fae03, 0x004fae74, 0x004fb407, 0x004fb493, 0x004fb6a9, 0x004fb724, 0x004fb8c2, 0x004fcf2a, 0x004fcf77, 0x004fec08, 0x004ff1ff, 0x004ff2be, 0x004ff574, 0x004ff5ea, 0x004ff660, 0x004ff6d6, 0x004ff754, 0x004ff7b5, 0x004ff819, 0x004ff87d, 0x004ff988, 0x004ff9fe, 0x004ffa74, 0x004ffaea, 0x004ffb60, 0x004ffbd6, 0x004ffc54, 0x004ffcb5, 0x004ffd19, 0x004ffd7d, 0x004ffde1, 0x004ffe45, 0x004fff48, 0x004fffbe, 0x00500034, 0x005000aa, 0x00500120, 0x00500196, 0x0050020c, 0x00500282, 0x00500300, 0x00500361, 0x005003c5, 0x00500429, 0x0050048d, 0x005004f1, 0x00500555, 0x005005b9, 0x005006ff, 0x00500774, 0x005007e9, 0x00500835, 0x00500895, 0x005008f7, 0x00500b82, 0x00500beb, 0x00500c54, 0x00500cbd, 0x00500d04, 0x00500d64, 0x00500dc6, 0x00500e28, 0x00501328, 0x00501460, 0x005014f2, 0x005015ab, 0x0050165f, 0x00501713, 0x005017ce, 0x00501889, 0x0050193d, 0x005019c3, 0x00501a4c, 0x00501aad, 0x00501b31, 0x00501b92, 0x00501c16, 0x00501c77, 0x00501d02, 0x00501d63, 0x00501dee, 0x00501e4f, 0x00501ed3, 0x00501f34, 0x00502016, 0x005020de, 0x005021a4, 0x0050226a, 0x00502337, 0x00502404, 0x005024ca, 0x0050257b, 0x0050264d, 0x00502703, 0x005027b7, 0x0050286b, 0x0050291f, 0x005029b2, 0x00502a54, 0x00502ab5, 0x00502b3b, 0x00502b9c, 0x00502c20, 0x00502c81, 0x00502d05, 0x00502d66, 0x00502dea, 0x00502e4b, 0x00502f3b, 0x0050301c, 0x005030e4, 0x005031aa, 0x00503270, 0x00503336, 0x005034be, 0x00503604, 0x0050370d, 0x00503775, 0x0050383a, 0x00503893, 0x005038f5, 0x0050f351,
};


struct HockManager {
    struct Entry {
        enum Type {
            Replace,
            ReplaceFunctionIndirectCall
        };
        Type type;
        std::string name;
        std::string description;
        unsigned int originalCallAdress;
        unsigned int overrideFunctionAdress;
        unsigned int originalFunctionAdress = 0;
        bool replaced = false;
    };

    std::vector<Entry> entries;

    void init() {
        
        replaceFunction("LoadDungeonsCategories", "", 0x0047108a, FUN_LoadDungeonsCategories);
        replaceFunction("LoadDungeonsCategories", "", 0x004710f6, FUN_LoadDungeonsCategories);

        replaceFunction("ProccessCategoryDungeonsScreen", "", 0x0048c74b, FUN_ProccessCategoryDungeonsScreen);
        replaceFunction("ProccessDungeonsScreen", "", 0x0048c741, FUN_ProccessDungeonsScreen);

        replaceFunction("LoadMPP", "", 0x004b6eea, FUN_004a59c0_LoadMPP); //load dungeon assets
        replaceFunction("LoadMPD", "", 0x004b6ecb, FUN_004d5060_LoadMPD);

        //hdTexture
        replaceFunction("LoadMPP", "", 0x004816b4, FUN_004a59c0_LoadMPP); //load dungeon assets
        replaceFunction("LoadMPD", "", 0x004816c3, FUN_004d5060_LoadMPD);

        replaceFunction("LoadMPP", "", 0x004816ef, FUN_004a59c0_LoadMPP); //load dungeon assets
        replaceFunction("LoadMPD", "", 0x0048170a, FUN_004d5060_LoadMPD);

        //status menu
        replaceFunction("statusMenu", "", 0x004803D1, FUN_0047f080_ProccessStatusMenu);

        replaceFunction("renderPortrait", "Renders the face (Portrait) of a character", 0x004a7455, FUN_004b5490_NoneStd); //render portrait
        replaceFunction("renderPortrait", "Renders the face (Portrait) of a character", 0x004a6f91, FUN_004b5490_NoneStd); //render portrait
        replaceFunction("renderPortrait", "Renders the face (Portrait) of a character", 0x004a7573, FUN_004b5490_NoneStd); //render portrait
        replaceFunction("renderPortrait", "Renders the face (Portrait) of a character", 0x004a7f85, FUN_004b5490_NoneStd); //render portrait
        replaceFunction("renderPortrait", "Renders the face (Portrait) of a character", 0x004a94a3, FUN_004b5490_NoneStd); //render portrait
        replaceFunction("renderPortrait", "Renders the face (Portrait) of a character", 0x004aac77, FUN_004b5490_NoneStd); //render portrait

        
        for (auto address : gRenderTextNewFuncs) {
            replaceFunction("renderTextNew", "Renders new text", address, FUN_RederTextNew); 
        }
        
        replaceFunction("renderTextNew", "Renders new text", 0x0050c594, FUN_0050c210_toUtf8);
        

        replaceFunction("assemblyMenu", "The dark assembly menu", 0x004a0d72, FUN_0048dfd0_ProccessAssemblyMenu);
        replaceFunction("assemblyMenu", "The dark assembly menu", 0x0049f06c, FUN_0048cd40_BuildAssemblyUnitInfosLookup);
        replaceFunction("assemblyMenu", "The dark assembly menu", 0x004a0bfc, FUN_0048cd40_BuildAssemblyUnitInfosLookup);
        replaceFunction("enemyUnitKilled", "Records history when enemy unit killed", 0x004764ea, FUN_004d63b0_EnemyUnitKilled);


        replaceFunction("getSpriteData", "Finds loaded sprite data with given sprite id", 0x0046b262, FUN_004de880_getSpriteData);
        replaceFunction("getSpriteData", "Finds loaded sprite data with given sprite id", 0x0048e461, FUN_004de880_getSpriteData);
        replaceFunction("getSpriteData", "Finds loaded sprite data with given sprite id", 0x00490875, FUN_004de880_getSpriteData);
        replaceFunction("getSpriteData", "Finds loaded sprite data with given sprite id", 0x004d53c1, FUN_004de880_getSpriteData);
        replaceFunction("getSpriteData", "Finds loaded sprite data with given sprite id", 0x004d5e87, FUN_004de880_getSpriteData);
        replaceFunction("getSpriteData", "Finds loaded sprite data with given sprite id", 0x004de915, FUN_004de880_getSpriteData);

        replaceFunction("afterCharDataLoad", "Used to init custom characters", 0x0048afc7, FUN_004cf3c0);

        replaceFunction("save", "Create new save file", 0x0043f6a7, FUN_00442260_Save);
        replaceFunction("loadSave", "Load save file", 0x00442495, FUN_0043e600_decompressSave_NoneStd); //Is run to decompress a save file, usualy after load
        replaceFunction("loadSave", "Load save file", 0x0043f8a9, FUN_004422c0_LoadSave);

        {
            auto f = &DisaScriptManager::FUN_004bf7e0_RunTalkScript; //find talk Dat
            unsigned int i = *((unsigned int*)&f);
            replaceFunction("RunTalkScript", "", 0x004b6b93, i);
            replaceFunction("RunTalkScript", "", 0x00463f7a, i);
        }

        replaceFunction("RunScript", "", 0x004c0286, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004b6d41, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004b6c85, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004b6ccb, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004b6dbf, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004b6d70, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004b6d9e, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004680c1, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004fbbeb, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x00464608, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004734a2, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004741ef, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x0048c4c6, FUN_004bd930_RunScript); //gameloop
        replaceFunction("RunScript", "", 0x00496251, FUN_004bd930_RunScript);
        //replaceFunction("RunScript", "", 0x00496a1b, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x0049efd6, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x0049ff33, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004b4abd, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004c305d, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004c6f15, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004c93be, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004c957e, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004c965a, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004ca778, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004cb2fd, FUN_004bd930_RunScript);
       
        replaceFunction("ScriptLoop", "", 0x0043cd28, FUN_004f7e70_ScriptLoop);
        replaceFunction("ScriptLoop", "", 0x00489ad1, FUN_004f7e70_ScriptLoop);
       
        //patch_FUN_004bd814_OrgAddress = FP::replaceFunction(0x004bd814, FUN_004b6420);
        replaceFunction("TalkScriptExec", "", 0x004bdef1, FUN_004bfa60_TalkScriptExec);

        //map end
        replaceFunction("MapEnd", "", 0x00458558, FUN_00446e20);
        replaceFunction("MapEnd", "", 0x004660b2, FUN_00446e20);
        replaceFunction("MapEnd", "", 0x0045da28, FUN_00446e20);

        //gameloop map exit
        replaceFunction("MapEndGameloop", "", 0x0048c71c, FUN_004710d0);

        FUN_glViewportPointer = &FUN_glViewport;
        FUN_glDrawArraysPointer = &FUN_glDrawArrays;

        replaceFunctionIndirectCall("glViewportPointer", "", 0x00420396, FUN_glViewportPointer); //render begin
        replaceFunctionIndirectCall("glDrawArraysPointer", "", 0x0042111c, FUN_glDrawArraysPointer); //render begin
        replaceFunction("RenderEnd", "", 0x00509d44, FUN_RenderEnd); //render begin

       
        {
            auto f = &DisaObjectRenderContext::FUN_004e2180; 
            unsigned int i = *((unsigned int*)&f);
            replaceFunction("TEST", "", 0x004E1E08, i);
            replaceFunction("TEST", "", 0x00508ba5, FUN_004201d0_RenderClearBegin);

            auto f2 = &DisaMemeoryManager::FUN_00425bb0;
            unsigned int i2 = *((unsigned int*)&f2);
            replaceFunction("TEST", "", 0x00425f82, i2);
        }
        
        replaceDefaultCase();
    }

    template<typename T>
    void replaceFunction(std::string&& name, std::string&& description, unsigned int originalFuncAddress, T&& replacementFunction) {
        replaceFunction(std::move(name), std::move(description), originalFuncAddress, reinterpret_cast<unsigned int>(std::addressof(replacementFunction)));
    }

    void replaceFunction(std::string&& name, std::string&& description, unsigned int originalFuncAddress, unsigned int replacementFunction) {
        entries.emplace_back(
            Entry::Replace,
            std::move(name),
            std::move(description),
            originalFuncAddress,
            replacementFunction
            );
    }

    template<typename T>
    void replaceFunctionIndirectCall(std::string&& name, std::string&& description, unsigned int originalFuncAddress, T&& replacementFunction) {
        replaceFunctionIndirectCall(std::move(name), std::move(description), originalFuncAddress, reinterpret_cast<unsigned int>(std::addressof(replacementFunction)));
    }

    void replaceFunctionIndirectCall(std::string&& name, std::string&& description, unsigned int originalFuncAddress, unsigned int replacementFunction) {
        entries.emplace_back(
            Entry::ReplaceFunctionIndirectCall,
            std::move(name),
            std::move(description),
            originalFuncAddress,
            replacementFunction
        );
    }


    void hook(const std::string& name = "") {

        for (auto& entry : entries) {
            if (entry.replaced) continue;
            if (!name.empty() && !PG_StringUtil::isEqualCaseless(entry.name, name)) continue;
            OUT_INFO("Replacing function call \"" << entry.name << "\" at 0x"<< entry.originalCallAdress<< ".");
            switch (entry.type)
            {
            case Entry::Replace: {
                entry.originalFunctionAdress = FP::replaceFunction(entry.originalCallAdress, entry.overrideFunctionAdress);
            }break;
            case Entry::ReplaceFunctionIndirectCall: {
                entry.originalFunctionAdress = FP::replaceFunctionIndirectCall(entry.originalCallAdress, entry.overrideFunctionAdress);
            }break;
            default:
                break;
            }
            entry.replaced = true;
        }
        OUT_INFO("Successfully injected custom code!");
    }

    void unhook(const std::string& name = "") {
        for (auto& entry : entries) {
            if (!entry.replaced) continue;
            if (!name.empty() && !PG_StringUtil::isEqualCaseless(entry.name, name)) continue;
            OUT_INFO("Replacing function call \"" << entry.name << "\" back to original at 0x" << entry.originalCallAdress << ".");
            switch (entry.type)
            {
            case Entry::Replace: {
                FP::replaceFunction(entry.originalCallAdress, entry.originalFunctionAdress);
            }break;
            case Entry::ReplaceFunctionIndirectCall: {
               FP::replaceFunctionIndirectCall(entry.originalCallAdress, entry.originalFunctionAdress);
            }break;
            default:
                break;
            }
            entry.replaced = false;
        }
        OUT_INFO("Successfully unloaded custom code!");
    }

    void info() {
        for (auto& entry : entries) {
            OUT_INFO("Registered replacement \"" << entry.name << "\" at 0x" << entry.originalCallAdress << ".");
        }
    }

};

HockManager gHockManager;

void hook() {
    gHockManager.hook();   
}

void hook(const std::string& name) {
    gHockManager.hook(name);
}

void unhook() {
    gHockManager.unhook();
}

void unhook(const std::string& name) {
    gHockManager.unhook(name);
}

void playTest() {
    DisaCustomCampaignManager::get()->runTestMap();
}

struct CommandTerminal {

    void openConsole() {
        if (coutBuffOld) return;
     
        AllocConsole();
        coutBuffOld = std::cout.rdbuf(); // save pointer to std::cout buffer

//#ifndef _DEBUG
        std::cout.rdbuf(std::cout.rdbuf());
        std::cin.rdbuf(std::cin.rdbuf());
        std::cerr.rdbuf(std::cerr.rdbuf());
        std::cout.flush();
//#endif // !_DEBUG

        FILE* pCout;
        freopen_s(&pCout, "CONIN$", "r", stdin);
        freopen_s(&pCout, "CONOUT$", "w", stdout);
        freopen_s(&pCout, "CONOUT$", "w", stderr);

        std::ios::sync_with_stdio(true);

        OUT_INFO("Console opened! Build Date "<< __DATE__<<".");
        auto path = DisaCustomBaseModManager::exePath();
        OUT_INFO("Current working path: "<< path);

    }


    void closeConsole() {
        if (!coutBuffOld) return;

        std::cout.rdbuf(coutBuffOld);
        coutBuffOld = nullptr;
        // Close the console
        FreeConsole();
    }

    void toggleConsole() {
        isConsoleOpen = !isConsoleOpen;
    }

    void sendMsg(std::string msg) {
        auto split = PG_StringUtil::splitOnce(msg, ':');
        if (split.empty()) return;
        msg = std::move(split.front());
        std::lock_guard g(mutex);
        if (msg == "e" || msg == "exit") {
            isRunning = false;
        }
        else if (msg == "h" || msg == "hook") {
            if (split.size() == 2) {
                hook(split[1]);
            }
            else {
                hook();
            }
        }
        else if (msg == "u" || msg == "unhook") {
            if (split.size() == 2) {
                unhook(split[1]);
            }
            else {
                unhook();
            }
        }
        else if (msg == "p" || msg == "playtest") {
            playTest();
        }
        else if (msg == "i" || msg == "info") {
            gHockManager.info();
        }
        else if (msg == "ds" || msg == "debugScript") {
            DisaCustomScriptManager::get()->setIsPrintScript(!DisaCustomScriptManager::get()->isPrintScript());
            std::cout << "Debug script: " << DisaCustomScriptManager::get()->isPrintScript() << std::endl;
        }
        else if (msg == "pp" || msg == "printParty") {
            if (split.size() == 2 && (split.back() == "j" || split.back() == "json")) {
                OS::Clipboard::textToClipboard(DisaPartyCharacterInfo::playerPartyToJson());
                std::cout << "Written player party into clipboard!" << std::endl;
            }
            else {
                std::cout << DisaPartyCharacterInfo::playerPartyToString() << std::endl;
            }
        }
        else if (msg == "pc" || msg == "printCharacters") {
            std::cout << DisaDataCharacterInfo::globalToString() << std::endl;
        }
        else if (msg == "pm" || msg == "printMagic") {
            std::cout << DisaDataMagicInfo::globalToString() << std::endl;
        }
        else if (msg == "ps" || msg == "printScript") {
            if (split.size() != 2) {
                std::cout << "Error: printScript needs arguments!" << std::endl;
                return;
            }
            auto args = PG_StringUtil::split(split.back(), ",");
            if (args.empty()) return;

            auto* manager = DisaCustomScriptManager::get();
            unsigned int* data = nullptr;
            manager->PG_DisaScriptsModel::readScript(data, PG_DisaIdentifier(args[0]));
            if (!data) {
                std::cout << "Script: \""<< args[0]<< "\" not found!" << std::endl;
                return;
            }

            const std::string script = manager->PG_DisaScriptsModel::scriptToString(data);
            if (args.size() == 2 && (args.back() == "c" || args.back() == "clipboard")) {
                OUT_INFO("Script \"" << args[0] << "\" added to clipboard!");
                OS::Clipboard::textToClipboard(script);
            }
            else {
                OUT_INFO("Script: \"" << args[0]<<"\"");
                std::cout << script << std::endl;
            }
        }
        else if (msg == "ac" || msg == "addCharacter") {
            if (split.size() != 2) {
                std::cout << "Error: addCharacter needs arguments!" << std::endl;
                return;
            }
          
            auto args = PG_StringUtil::split(split.back(), ",");
            if (args.size() < 1) {
                std::cout << "Error: addCharacter needs at least an identifier!" << std::endl;
                return;
            }

            PG_DisaIdentifier identifier(args.front());
            if (!identifier.isValid()) {
                std::cout << "Error: Invalid identifier!" << std::endl;
                return;
            }

            DisaCustomCharacterManager::get()->addCharacterToPary(identifier, 
                args.size() > 1 ? std::max(1, PG_StringUtil::toInt(args[1])) : 1,
                args.size() > 2 ? glm::clamp(-1,128, PG_StringUtil::toInt(args[2])): -1
            );
        }
        else if (msg == "rs" || msg == "runScript") {
            if (split.size() != 2) {
                std::cout << "Error: runScript needs an identifier as argument!" << std::endl;
                return;
            }

            PG_DisaIdentifier identifier(split.back());
            if (!identifier.isValid()) {
                std::cout << "Error: Invalid identifier!" << std::endl;
                return;
            }

            runCustomScript(identifier);
        }
        else if (msg == "help") {
            std::cout << "* hook:name (h:name) - Inject custom code. Optional second argument is a target name." << std::endl;
            std::cout << "* unhook:name (u:name) - Remove injected custom code. Optional second argument is a target name." << std::endl;
            std::cout << "* info (i) - Info about all registerd functions." << std::endl;
            std::cout << "* playtest (p) - Run playtest map from 'customCampainsTest'." << std::endl;
            std::cout << "* debugScript (ds) - Will print scripts into console when they are executed." << std::endl;
            std::cout << "* printParty:mode (pp:mode) - Will print the current party as info (default) or dump a full json into clipboard (pp:json)." << std::endl;
            std::cout << "* printCharacters (pc) - Will print the current loaded character data." << std::endl;
            std::cout << "* printMagic (pm) - Will print the current loaded magic/skills data." << std::endl;
            std::cout << "* printScript:c (ps:id,c) - Will will print script with given Id. Example 'ps:Disa::2001012'. You can add arg 'c' to dump into clipboard." << std::endl;
            std::cout << "* addCharacter:identifier,level,mentorIndex (ac:id,l,m) - Add a new character to your party. Example to add Etna with level 42: 'ac:Disa::2,42'. Level and mentor are optional." << std::endl;
            std::cout << "* runScript:identifier (rs:id) - Will execute the script with given identifier." << std::endl;
            std::cout << "* exit (e) - Close injection thread" << std::endl;
        }
        else {
            std::cout << "'" << msg << "'" << " not a valid input! Enter 'help' for instructions!" << std::endl;
        }
    }

    void run() {
        while (isRunning) {
            if (isConsoleOpen) {
                openConsole();
            }
            else {
                closeConsole();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            std::string input;
            std::cin >> input;

            sendMsg(input);
        }
    }

    bool isRunning = true;
    std::atomic_bool isConsoleOpen = false;
    std::streambuf* coutBuffOld = nullptr;
    std::mutex mutex;
};


CommandTerminal commandTerminal;
UINT g_playTestMessage;
WNDPROC g_originalWndProc = 0;

// Window procedure to handle messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == g_playTestMessage) {
        commandTerminal.sendMsg("p");
        return 0;
    }

    if (uMsg == WM_KEYDOWN && wParam == VK_F2) {
        commandTerminal.toggleConsole();
    }

    if (g_originalWndProc) {
        return CallWindowProc(g_originalWndProc, hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static DisaCustomBaseModManager g_baseManger;
/**
* Build settings:
* Build Events -> Post-Build Events -> copy /Y "$(TargetDir)$(ProjectName).dll" "D:\Steam\SteamApps\common\Disgaea PC\j.dll"
* (Only Debug) C++ -> Code Generation -> Basic Runtime Check -> /RTCu
* Debugging -> Command -> D:/Steam/SteamApps/common/Disgaea PC/dis1_st.exe
*/
DWORD WINAPI startMain() {
#ifdef _DEBUG
    commandTerminal.openConsole();
#endif // _DEBUG

    FP::init();
    if (!FP::get()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        commandTerminal.closeConsole();
        return 0;
    }

    PG_ResourceManager::create();
    gHockManager.init();

    g_playTestMessage = RegisterWindowMessage(TEXT("START_PLAY_TEST"));
    if (g_playTestMessage == 0) {
        std::cout << "Failed to register message!" << std::endl;
    }
    else {
        HWND hwnd = FindWindow(NULL, TEXT("DISGAEA"));
        if (hwnd == NULL) {
            std::cerr << "Error: FindWindow failed with error code " << GetLastError() << "!" << std::endl;
        }
        else {
            g_originalWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WindowProc);
        }
    }

    hook();
    DisaCustomCampaignManager::get()->loadCampaigns();


    commandTerminal.run();

    std::cout << "Exit!" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    commandTerminal.closeConsole();

    return 0;

}

bool IsVCppRedistInstalled()
{
    HKEY hKey;
    // Visual Studio 2015–2022 Redistributable (x86)
    const wchar_t* subkey = L"SOFTWARE\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\x86";

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, subkey, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return false;

    DWORD installed = 0, size = sizeof(installed);
    RegQueryValueExW(hKey, L"Installed", nullptr, nullptr, reinterpret_cast<LPBYTE>(&installed), &size);
    RegCloseKey(hKey);

    return installed == 1;
}


BOOL APIENTRY DllMain(HINSTANCE, DWORD  reason, LPVOID) {
    /*
    if (!IsVCppRedistInstalled()) {
        MessageBoxW(nullptr,
            L"Microsoft Visual C++ Redistributable (2015–2022) X86 (32bit) is not installed!\n\n"
            L"Please download and install it from microsoft!",
            L"Missing Runtime",
            MB_ICONERROR | MB_OK);
        return 1;
    }
    */

    if (reason == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)startMain, NULL, 0, NULL);
    }

    return TRUE;
}