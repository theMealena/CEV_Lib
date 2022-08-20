//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  11-2016      |   0.0    |    creation    **/
//**   CEV    |  15-02-2017   |   1.0    |   rev & test   **/
//**********************************************************/


#ifndef ANIMATOR_H_INCLUDED
#define ANIMATOR_H_INCLUDED

#include "SDL.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SP_NUM_OF_REQ_MAX (5)
#define SP_MODE_LIST {"SP_LOOP_FOR", "SP_FOR_REV", "SP_FOR_ONCE", "SP_FOR_REV_LOCK", "SP_FOR_LOCK"}
#define SP_MODE_NUM (5)


enum {SP_NONE =-1, SP_NVIEW = 0, SP_XVIEW = 1};
enum {SP_CLIP = 0, SP_HBOX = 1};
enum {SP_LOOP_FOR = 0 , SP_FOR_REV = 1, SP_FOR_ONCE = 2, SP_FOR_REV_LOCK = 3, SP_FOR_LOCK = 4};
enum {SP_HIDE = 0, SP_SHOW = 1};


typedef struct SP_Anim SP_Anim;

typedef struct SP_AnimList SP_AnimList;

typedef struct SP_Sprite SP_Sprite;

/** \brief sp view structure.
 */
typedef struct SP_View
{/*view structure*/

    uint16_t picNum, /*number of picture*/
             picTime,/*frame delay time ms*/
             restart,/*restart frame index*/
             stop,   /*stop frame index*/
             mode;   /*read mode*/

    SDL_Rect rect[2];/*clip and hitbox*/
}
SP_View;


/** \brief sp animation structure.
 */
struct SP_Anim
{/*animation structure*/

    uint16_t  viewNum[2]; /*number of view / xtra */

    SP_View* view[2];     /*2 tables of SP_View / extra*/

    SDL_Texture* sheet;  /*spritesheet*/
};

// TODO (drx#1#): implémenter le stop dans les boucles

/** \brief sp sprite structure.
 */
struct SP_Sprite
{/*sprite structure*/

    uint8_t     isLocked[2],   /*animation is locked*/
                viewShow[2],   /*show view/xview*/
                run;           /*is animated or freezed*/

    int8_t      direction[2];  /*actual direction = +/-1 */

    uint16_t    picAct[2],     /*active picture index*/
                viewAct[2];    /*active view index*/

    int16_t     viewReq[2][SP_NUM_OF_REQ_MAX];/*next view/xview requests*/

    unsigned int timePrev[2];  /*last absolute frame change*/

    double      scale;         /*display scale*/

    SP_Anim*    anim;          /*base spritesheet*/
};


/** \brief sp animation set structure.
 */
struct SP_AnimList
{/*set of animations strcuture*/

    uint16_t    num;
    SP_Anim**   animSet;
};

void SP_viewDump(SP_View* view);
void SP_animDump(SP_Anim* anim);
int SP_fetchMode(SP_Sprite *sprite);


/*--spritesheet related functions --*/

/**creation and alloc of animation*/
/** \brief creates animation instance.
 *
 * \param nview : uint16_t number of Nview to create.
 * \param xview : uint16_t number of Xview to create
 * \param sheet : SDL_Texture* aka spritesheet.
 *
 * \return SP_Anim* if success, NULL on error.
 */
SP_Anim* SP_animCreate(uint16_t nview, uint16_t xview, SDL_Texture* sheet);


/**free it all*/
/** \brief fully free and destroy animation.
 *
 * \param anim : SP_Anim* to free.
 * \param freePic : let the function free the texture if true.
 *
 * \return N/A.
 */
void SP_animFree(SP_Anim* anim, char freePic);


/**gets texture*/
/** \brief getting embedded Texture.
 *
 * \param anim : SP_Anim* to fetch texture from.
 *
 * \return SDL_Texture* if any, NULL otherwise.
 */
SDL_Texture* SP_animTexture(SP_Anim *anim);


/** \brief query animation.
 *
 * \param anim : SP_Anim* to query from.
 * \param nView : uint16_t* ptr filled with number of nviews.
 * \param xView : uint16_t* ptr filled with number of xviews.
 *
 * \return N/A.
 */
void SP_animQuery(SP_Anim *anim, uint16_t* nView, uint16_t* xView);



/** \brief Load single animation or 1st in list.
 *
 * \param fileName : const char*.
 *
 * \return SP_Anim* or NULL on error.
 *
 */
SP_Anim* SP_AnimLoad(const char* fileName);


/**extracts animation set from sps file*/
/** \brief loads multiple spritesheets file.
 *
 * \param fileName : the file name to open.
 *
 * \return SP_AnimList* on success, NULL on error.
 */
SP_AnimList* SP_animListLoad(const char* fileName);


/**loads animation set from sps SDL_RWops*/
/** \brief loads multiple spritesheets file.
 *
 * \param ops : SDL_RWops* to use as file.
 * \param freeSrc : let the function close the SDL_RWops if not 0.
 *
 * \return SP_AnimList* on success, NULL on error.
 */
SP_AnimList* SP_animListLoad_RW(SDL_RWops* ops, uint8_t freeSrc);


/** \brief number of spritesheet in struture.
 *
 * \param set : SP_AnimList* to query.
 *
 * \return number of SP_Anim held within.
 */
uint16_t SP_animListNum(SP_AnimList *set);


/** \brief fetches the Nth spritesheet in list.
 *
 * \param set : SP_AnimList* to fetch from.
 * \param index : wich one to fetch.
 *
 * \return SP_Anim* on success, NULL on error.
 */
SP_Anim* SP_animListGetIndex(SP_AnimList *set, unsigned int index);

/**frees animation set*/
/** \brief frees and destroy animation set
 *
 * \param set : SP_AnimList* to free.
 * \param freePic : let the function free the texture if not 0.
 * \return void
 *
 */
void SP_animListFree(SP_AnimList *set, char freePic);

/*---animations and views settings---*/

/**view global setting*/
/** \brief view parameters set.
 *
 * \param anim : SP_Anim* to modify.
 * \param viewType : view type to modify.
 * \param viewIndex : view index to modify.
 * \param clip : SDL_Rect as clipper.
 * \param hitBox : SDL_Rect as hit box.
 * \param time : time delay between frames (ms).
 * \param mode : play mode.
 * \param restart : restart pic index.
 * \param stop : stop pic index.
 *
 * \return N/A.
 *
 */
void SP_viewSet(SP_Anim* anim, uint8_t viewType, uint16_t viewIndex, uint16_t frameNum, SDL_Rect clip, SDL_Rect hitBox, uint16_t time, uint16_t mode, uint16_t restart, uint16_t stop);


/**sets view time*/
/** \brief frame delay parameter
 *
 * \param anim : SP_Anim* to modify.
 * \param viewType : view type to modify.
 * \param viewIndex : view index to modify.
 * \param time : time delay between frames (ms).
 *
 * \return N/A.
 */
void SP_viewTime(SP_Anim* anim, uint8_t viewType, uint8_t viewIndex, uint16_t time);


/**sets view play mode*/
/** \brief display mode parameter
 *
 * \param anim : SP_Anim* to modify.
 * \param viewType : view type to modify.
 * \param viewIndex : view index to modify.
 * \param mode : play mode.
 *
 * \return N/A.
 */
void SP_viewMode(SP_Anim* anim, uint8_t viewType, uint16_t viewIndex, uint16_t mode);


/**sets clip*/
/** \brief display mode parameter
 *
 * \param anim : SP_Anim* to modify.
 * \param viewType : view type to modify.
 * \param viewIndex : view index to modify.
 * \param clip : SDL_Rect as clipper.
 *
 * \return N/A.
 */
void SP_viewClip(SP_Anim* anim, uint8_t viewType, uint16_t viewIndex, SDL_Rect* clip);


/**sets hitbox*/
/** \brief hit box parameter
 *
 * \param anim : SP_Anim* to modify.
 * \param viewType : view type to modify.
 * \param viewIndex : view index to modify.
 * \param hbox : SDL_Rect as hit box.
 *
 * \return N/A.
 */
void SP_viewHitBox(SP_Anim* anim, uint16_t viewIndex, SDL_Rect* hbox);

/**sets restart index*/
/** \brief restart parameter
 *
 * \param anim : SP_Anim* to modify.
 * \param viewType : view type to modify.
 * \param viewIndex : view index to modify.
 * \param restart : img index from which view will restart.
 *
 * \return N/A.
 */
void SP_viewRestart(SP_Anim* anim, uint8_t viewType, uint16_t viewIndex, uint16_t restart);



/*---sprites related functions---*/


/**creates new sprite based on anim*/
/** \brief new sprite.
 *
 * \param anim : SP_Anim* sprite is based upon.
 *
 * \return SP_Sprite* on success, NULL on error.
 */
SP_Sprite* SP_spriteCreateFromAnim(SP_Anim *anim);


/**links existing sprite to anim*/
/** \brief links sprite to anim.
 *
 * \param sprite : SP_Sprite* to modify.
 * \param anim : SP_Anim* the sprite will be related to.
 *
 * \return N/A.
 *
 */
void SP_spriteLinkToAnim(SP_Sprite* sprite, SP_Anim *anim);


/**resets sprite struct to default*/
/** \brief resets sprite.
 *
 * \param sprite : SP_Sprite* to reset.
 *
 * \return N/A.
 *
 * \note link to spritesheet is kept as is.
 */
void SP_spriteReset(SP_Sprite *sprite);


/**sets scale*/
/** \brief parameter a sprite's scale.
 *
 * \param sprite : SP_Sprite* to scale.
 * \param scale : double as scale value.(0.5=half, 2.0=twice).
 *
 * \return N/A.
 */
void SP_spriteScale(SP_Sprite* sprite, double scale);


/**fetches scale*/
/** \brief retrieve a sprite's scale.
 *
 * \param sprite : SP_Sprite* to fetch scale from.
 *
 * \return double as scale value.(0.5=half, 2.0=twice).
 */
double SP_spriteScaleGet(SP_Sprite* sprite);

/**animation**/
/** \brief performs aniamtion aka next picture selection
 *
 * \param sprite : SP_Sprite* to animate
 *
 * \return void N/A
 */
void SP_spriteAnim(SP_Sprite* sprite);


/**blits animation with ex*/
/** \brief blits sprite.
 *
 * \param sprite : SP_Sprite* to blit.
 * \param dst : SDL_Renderer* to blit into.
 * \param pos : SDL_Point* as blit position (gravity of sprite).
 * \param angle : applied rotation angle.
 * \param center : SDL_Point* to rotate around.
 * \param flip : one or combination of any SDL_RendererFlip.
 *
 * \return N/A.
 */
void SP_spriteBlitEx(SP_Sprite* sprite, SDL_Renderer* dst, const SDL_Point* pos,const double angle, const SDL_Point* center, const SDL_RendererFlip flip);


/**blits animation*/
/** \brief blits sprite.
 *
 * \param sprite : SP_Sprite* to blit.
 * \param dst : SDL_Renderer* to blit into.
 * \param pos : SDL_Point* as blit position (gravity of sprite).
 *
 * \return N/A.
 */
void SP_spriteBlit(SP_Sprite* sprite, SDL_Renderer* dst, const SDL_Point* pos);


/** returns clip pos in sprite sheet*/
/** \brief gets clip position.
 *
 * \param sprite : SP_Sprite* to query from.
 * \param viewType : type view to query.
 *
 * \return SDL_Rect* on actual SDL_Rect clipper.
 */
SDL_Rect* SP_clipGet(SP_Sprite* sprite, uint8_t viewType);


/** \brief returns hitbox pos in spritesheet
 *
 * \param sprite : SP_Sprite* to fetch actual hitbox from.
 * \param flip : flip status of sprite.
 *
 * \return SDL_Rect as hitBox pos relative to spritesheet.
 * \note Does not treat horizontal/vertical flip.
 */
SDL_Rect SP_sheetHBoxGet(SP_Sprite* sprite, SDL_RendererFlip flip);


/**returns absolute pos of scaled hitBox */
/** \brief queries hit box
 *
 * \param sprite : SP_Sprite* to query from.
 * \param pos : SDL_Point as actual sprite position.
 * \param flip : SDL_renderFlip to correct H/V hitbox position.
 * \return SDL_Rect : as absolute hitBox position and dimension (scaled).
 */
SDL_Rect SP_hBoxGet(SP_Sprite* sprite, SDL_Point pos, SDL_RendererFlip flip);


/**asynchroneous view change*/
/** \brief forces view display
 *
 * \param sprite  : SP_Sprite* to modify.
 * \param viewType : which view type to change.
 * \param viewIndex : which view to display.
 * \param frameIndex : which frame to display.
 *
 * \return N/A.
 */
void SP_viewForce(SP_Sprite* sprite, uint8_t viewType, uint16_t viewIndex, uint16_t frameIndex);


/**synchroneous view request*/
/** \brief view request
 *
 * \param sprite : SP_Sprite* to request view from.
 * \param viewType : which view type to change.
 * \param viewIndex : which view to request.
 *
 * \return index position of request in stack or -1 if stack is full.
 */
char SP_viewRequest(SP_Sprite* sprite, uint8_t viewType, uint16_t viewIndex);


/**enable / disable the display of a view*/
/** \brief show a view
 *
 * \param sprite : SP_Sprite* to modify.
 * \param viewType : which view type to change.
 * \param display : 0 disables display, any other value enables display.
 *
 * \return N/A.
 */
void SP_viewDisplay(SP_Sprite* sprite, uint8_t viewType, uint8_t display);

/**stops playing sprite*/
/** \brief freezes animation : pause.
 *
 * \param sprite : SP_Sprite* to pause.
 *
 * \return N/A.
 */
void SP_spriteStop(SP_Sprite* sprite);


/**starts playing sprite*/
/** \brief plays animation
 *
 * \param sprite : SP_Sprite* to animate.
 *
 * \return N/A.
 */
void SP_spriteStart(SP_Sprite* sprite);


/**queries locked status*/
/** \brief is animation locked
 *
 * \param sprite : SP_Sprite* to query.
 *
 * \return 1 if locked, 0 otherwise.
 */
char SP_spriteIsLocked(SP_Sprite* sprite);


/** \brief queries sprite status.
 *
 * \param sprite : SP_Sprite* to query.
 * \param actNview : int* filled with actual NVIEW view index.(or NULL)
 * \param actXview : int* filled with actual XVIEW view index.(or NULL)
 * \param actNpic : int* filled with actual NVIEW picture index.(or NULL)
 * \param actXpic : int* filled with actual XVIEW picture.(or NULL)
 *
 * \return N/A.
 */
void SP_spriteQuery(SP_Sprite* sprite, int* actNview, int* actXview, int* actNpic, int* actXpic);


    /*---FILE RELATED FUNCTIONS---*/

/**converts CSV file into sps file*/
/** \brief user config file to data file
 *
 * \param srcName : edition file to read from.
 * \param dstName : data file to create.
 *
 * \return readWriteErr is filled.
 *
 */
int SP_convertSpriteCSVToData(const char* srcName, const char* dstName);

#ifdef __cplusplus
}
#endif

#endif // ANIMATOR_H_INCLUDED
