

#ifndef PROJECT_DEF_H_INCLUDED
#define PROJECT_DEF_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*default screen value*/
#define SCREEN_DEFAULT_IS_DESKTOP 0

#define SCREEN_WDEFAULT   1280
#define SCREEN_HDEFAULT   720

/** if system auto size activated **/
//neo geo definition 4/3
#define SCREEN_WSMALL   320
#define SCREEN_HSMALL   224

// 16/10
#define SCREEN_WLARGE    336
#define SCREEN_HLARGE    210

// 16/9
#define SCREEN_WXLARGE    384
#define SCREEN_HXLARGE    216
/**end auto size*/

#define FONT_MAX 10
#define FONT_SIZE_L 15
#define FONT_SIZE_M 12
#define FONT_SIZE_S 10

#define APP_NAME "level creator"
#define MY_ORG "the mealena"

#define RELEASE 0

#if !RELEASE
    #define DEBUG 0     //show player's boxes
    #define VERBOSE 1   //displays prints in console
    #define FILE_MAKE 0 //build ressources
    #define SOUND_EN 0  //enables / stops musics
#else
    #define DEBUG 0     //show player's boxes
    #define VERBOSE 0   //displays prints in console
    #define FILE_MAKE 0 //build ressources
    #define SOUND_EN 1  //enables / stops musics
#endif



/***ADD local project definitions here below**/

#define DATA_FILE "ressources\\data.dat"

/*profile purpose to save levels datas*/
#define NUM_OF_WORLD 1      //max
#define NUM_OF_SUBWORLD 3   //max
#define NUM_OF_SCREEN 6     //number of total level (worlds + bonus)
#define MAX_BONUS 5         //max bonus per subworld

#define RSC_NUMOF_BCKGRD 4      //num of background parallax
#define RSC_NUMOF_SPRITE 2      //num of sprites anim (total)
#define RSC_NUMOF_PLYR_SPRITE 1 //num of player anim
#define RSC_NUMOF_PNJ_SPRITE 1  //num of pnj anim

#define RSC_NUMOF_OBJECT_PIC 4          //num of object pics (total)
#define RSC_NUMOF_OBJECT_PIC_PLATE 2    //num of plateforme pic
#define RSC_NUMOF_OBJECT_PIC_ZIP 1      //num of zipline pic
#define RSC_NUMOF_OBJECT_PIC_BUMP 1     //num of bumper pics

#define RSC_NUMOF_SOUND 9           //num of sound (total)
#define RSC_NUMOF_OBJECT_SOUND 2    //num of object sounds
#define RSC_NUMOF_PLYR_SOUND 5      //num of player sound
#define RSC_NUMOF_PNJ_SOUND 2       //num of pnj sounds


#define BACKGROUND_INDEX 0  //prl files
#define FONT_INDEX       4  //font
#define MAP_INDEX        5  //map files
#define WRLD_MAP_INDEX   5  //world maps
#define BNS_MAP_INDEX    8  //bonus maps
#define WTHR_INDEX       11 //wtr files
#define SPRITE_INDEX     15
#define PLYR_SPRITE_INDEX  15 //player sprite
#define PNJ_SPRITE_INDEX   16 //pnj sprites
#define SCROLL_INTRO       17 //intro scl
#define SCROLL_OUTRO       18 //outro scl
#define MENU_MAIN          19 //main men
#define MENU_OPTION        20 //option men
#define TEXT_INDEX         21 //texts
#define TEXT_GAME          22
#define TILESET_INDEX      23 //tile sets
#define PIC_DEMO           24 //demo pics
#define PIC_INTRO          30 //intro pic
#define PIC_MINI_LVL       31 //mini mpas pic
#define OBJ_PIC_INDEX       32 //obects pic
#define OBJ_PIC_PLATE_INDEX  32 //plateforms pic
#define OBJ_PIC_ZIP_INDEX   34  //zipline pic
#define OBJ_PIC_BUMP_INDEX  35  //bumper pic
#define GIF_DECRUNCH        36
#define GIF_OPTION          37
#define GIF_CONFIRM         38
#define GIF_OUTRO           39
#define CHAPTER_INDEX       40
#define MUSIC_INDEX         43 //mp3 index
#define MUSIC_GAME_INDEX    45 //in game/menu mp3
#define SOUND_INDEX         55
#define OBJECT_SOUND_INDEX  55
#define PLYR_SOUND_INDEX    57
#define PNJ_SOUND_INDEX     62
#define LEVEL_INDEX         64

#ifdef __cplusplus
}
#endif

#endif // PROJECT_DEF_H_INCLUDED
