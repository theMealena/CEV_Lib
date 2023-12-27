//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  11-2016      |   0.0    |    creation    **/
//**   CEV    |  15-02-2017   |   1.0    |   rev & test   **/
//**   CEV    |  04-2023      |   1.1.0  | CEV_lib format **/
//**   CEV    |  12-2023      |   1.1.1  | export added   **/
//**********************************************************/


/** data file format :
u32 id
    num of Nview
    num of Xview
    pic Id

    per view :
        u32 : num of pic
            : delay
            : pic restart
            : loop mode
            : clip x,y,w,h
            : hit box x,y,w,h

capsule : img if no pic Id
*/

#ifndef ANIMATOR_H_INCLUDED
#define ANIMATOR_H_INCLUDED



#include <SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <CEV_types.h>

#define SP_NUM_OF_REQ_MAX (5)   /**< maximum of request */

#define SP_MODE_LIST {"SP_LOOP_FOR", "SP_FOR_REV", "SP_FOR_ONCE", "SP_FOR_REV_LOCK", "SP_FOR_LOCK"}
//                          0               1           2               3                   4
#define SP_TYPE_ID (IS_SPS<<24)


#ifdef __cplusplus
extern "C" {
#endif



/** \brief View type
 */
enum
{
    SP_NONE     = -1,
    SP_NVIEW    = 0,
    SP_XVIEW    = 1,
    SP_VIEW_LAST
};


/** \brief Rect type
 */
enum {SP_CLIP = 0, SP_HBOX = 1};


/** \brief Playing mode
 */
typedef enum
{
    SP_LOOP_FOR     = 0,
    SP_FOR_REV      = 1,
    SP_FOR_ONCE     = 2,
    SP_FOR_REV_LOCK = 3,
    SP_FOR_LOCK     = 4,
    SP_MODE_LAST
}
SP_LOOP_MODE;


/** \brief View display
 */
enum {SP_HIDE = 0, SP_SHOW = 1};


/** \brief sp view structure.
 */
typedef struct SP_View
{

    uint32_t numOfFrame,/**<number of frame*/
             delay,     /**<frame delay time ms*/
             restart,   /**<restart frame index*/
             stop;      /**<stop frame index*/

    SP_LOOP_MODE mode;  /**<read mode*/

    SDL_Rect rect[2];   /**<clip and hitbox; hitbox position is relative to clip*/
}
SP_View;


/** \brief sp animation structure.
 */
typedef struct SP_Anim
{
    uint32_t id,            /**< Unique Id */
             picId,         /**< Picture unique ID if not embedded */
             numOfView[2];  /**<number of view / xtra */
    SP_View* view[2];       /**<2 tables of SP_View / xtra*/
    SDL_Texture* pic;     /**<spritesheet*/
}
SP_Anim;

// TODO (drx#6#): implémenter le stop dans les boucles

/** \brief sp sprite structure.
 */
typedef struct SP_Sprite
{//sprite structure

    struct
    {
        bool    isLocked,   /**<animation is locked and will not execute request until done*/
                viewShow;   /**<show view/xview*/


        int8_t      direction;  /**<actual direction = +/-1 */

        uint32_t    picAct,      /**<active picture index*/
                    viewAct,     /**<active view index*/
                    timePrev;    /**<last absolute frame change*/

        int32_t     viewReq[SP_NUM_OF_REQ_MAX];/**<next view/xview requests*/
    }control[2];

    bool run;       /**<is animated or freezed*/
    double  scale;  /**<display scale*/

    SP_Anim *anim;  /**<base spritesheet*/
}
SP_Sprite;



void TEST_sprite(void);


/** \brief Dumps sprite content to stdout.
 *
 * \param this : SP_Sprite* to dump.
 *
 * \return void.
 */
void SP_spriteDump(SP_Sprite* this);


/** \brief Dumps view content to stdout.
 *
 * \param this : SP_View* to dump.
 *
 * \return void.
 */
void SP_viewDump(SP_View* this);


/** \brief Dumps animation content
 *
 * \param anim : SP_Anim* to dump.
 *
 * \return void
 */
void SP_animDump(SP_Anim* anim);


/** \brief Fetches playing mode
 *
 * \param sprite : SP_Sprite* to fetch from.
 *
 * \return int as playing mode.
 *
 * \note Result is from play mode enum.
 */
int SP_spriteModeGet(SP_Sprite *sprite);


/*--spritesheet related functions --*/

/**creation and alloc of animation*/

/** \brief creates animation instance.
 *
 * \param nview : uint32_t number of Nview to create.
 * \param xview : uint32_t number of Xview to create
 * \param pic : SDL_Texture* aka spritesheet.
 *
 * \return SP_Anim* if success, NULL on error.
 */
SP_Anim* SP_animCreate(uint32_t nview, uint32_t xview, SDL_Texture* pic);


/** \brief Loads single animation.
 *
 * \param fileName : const char* as name of file to load.
 *
 * \return SP_Anim* or NULL on error.
 *
 * \note readWriteErr is filled.
 */
SP_Anim* SP_animLoad(const char* fileName);


/** \brief Loads animation from RWops.
 *
 * \param src : SDL_RWops* to load from;
 * \param freeSrc : bool closes src if true.
 *
 * \return SP_Anim* on success / NULL on error.
 *
 * \note If requested src is closed weither function succeeds or not.\n
 * \note readWriteErr is filled.
 */
SP_Anim* SP_animLoad_RW(SDL_RWops* src, bool freeSrc);


/** \brief Saves animation.
 *
 * \param src : SP_Anim* to be saved.
 * \param fileName : char* as name of file to save into.
 *
 * \return int as sdt funcSts.
 *
 * \note readWriteErr is filled.
 */
int SP_animSave(SP_Anim *src, char* fileName);


/** \brief Writes animation into file.
 *
 * \param src : SP_Anim* to be written.
 * \param dst : FILE* to write into.
 *
 * \return int int as sdt funcSts.
 * \note readWriteErr is filled.
 */
int SP_animTypeWrite(SP_Anim *src, FILE *dst);



/** \brief User friendly file into data file
 *
 * \param srcName : edited file to read from.
 * \param dstName : data file to create.
 *
 * \return int as sdt funcSts.
 * \note readWriteErr is filled.
 */
int SP_animConvertTxtToData(const char* srcName, const char* dstName);



/** \brief Export SP_Anim as editable file.
 *
 * \param src : SP_Anim* to export.
 * \param dstName : const char* as path and resulting fileName.
 *
 * \return int  as sdt funcSts.
 * \note picture is produced  next to resulting file
 * random name given to embedded picture if any.
 */
int SP_animExport(SP_Anim *src, const char* dstName);


/** \brief fully frees and destroys animation.
 *
 * \param anim : SP_Anim* to free.
 *
 * \return void.
 * \note Texture is freed only if picture is embedded (picId == 0).
 */
void SP_animDestroy(SP_Anim* anim);


/** \brief Clear anim structure content.
 *
 * \param anim : SP_Anim* to clear.
 *
 * \return void.
 * \note Texture is freed only if picture is embedded (picId == 0).
 */
void SP_animClear(SP_Anim* anim);


/** \brief Attaches SDL_Texture to SP_Anim.
 *
 * \param src : SDL_Texture* to attach.
 * \param dst : SP_Anim* to attach texture to.
 *
 * \return void.
 */
void SP_animAttachTexture(SDL_Texture *src, SP_Anim* dst);


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
 * \return void.
 */
void SP_spriteLinkToAnim(SP_Sprite* sprite, SP_Anim *anim);


/** \brief Frees sprite and content.
 *
 * \param src : SP_Sprite* to destroy.
 *
 * \return void.
 */
void SP_spriteDestroy(SP_Sprite *src);


/** \brief Clears structure content.
 *
 * \param src : SP_Sprite* to clear.
 *
 * \return void.
 */
void SP_spriteClear(SP_Sprite *src);


/** \brief resets sprite.
 *
 * \param sprite : SP_Sprite* to reset.
 *
 * \return void.
 *
 * \note Link to spritesheet is kept as is.
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
void SP_spriteScaleSet(SP_Sprite* sprite, double scale);


/**fetches scale*/
/** \brief retrieve a sprite's scale.
 *
 * \param sprite : SP_Sprite* to fetch scale from.
 *
 * \return double as scale value.(0.5=half, 2.0=twice).
 */
double SP_spriteScaleGet(SP_Sprite* sprite);


/** \brief Performs animation calculations.
 *
 * \param sprite : SP_Sprite* to animate
 *
 * \return void void
 */
void SP_spriteUpdate(SP_Sprite* sprite);


/** \brief Blits sprite.
 *
 * \param sprite : SP_Sprite* to blit.
 * \param pos : SDL_Point* as blit position (gravity of sprite).
 * \param angle : applied rotation angle.
 * \param center : SDL_Point* to rotate around.
 * \param flip : one or combination of any SDL_RendererFlip.
 *
 * \return void.
 */
void SP_spriteBlitEx(SP_Sprite* sprite, const SDL_Point* pos,const double angle, const SDL_Point* center, const SDL_RendererFlip flip);


/** \brief Blits sprite.
 *
 * \param sprite : SP_Sprite* to blit.
 * \param pos : SDL_Point* as blit position (gravity of sprite).
 *
 * \return void.
 */
void SP_spriteBlit(SP_Sprite* sprite, const SDL_Point* pos);


/** \brief Gets sprite actual playing mode.
 *
 * \param sprite : SP_Sprite* to fetch from.
 *
 * \return int from SP_LOOP_MODE enum.
 * \sa SP_LOOP_MODE
 */
int SP_spriteModeGet(SP_Sprite *sprite);


/** \brief Gets clip position.
 *
 * \param sprite : SP_Sprite* to query from.
 * \param viewType : view type to query.
 *
 * \return SDL_Rect on actual SDL_Rect clipper relative to spritesheet.
 */
SDL_Rect SP_spriteClipGet(SP_Sprite* sprite, uint32_t viewType);


/** \brief Hitbox pos in spritesheet
 *
 * \param sprite : SP_Sprite* to fetch actual hitbox from.
 *
 * \return SDL_Rect as hitBox pos relative to spritesheet.
 *
 * \note Does not treat horizontal/vertical flip.
 */
SDL_Rect SP_spriteHBoxClipGet(SP_Sprite* sprite);



/** \brief Queries hit box position in sprite's world.
 *
 * \param sprite : SP_Sprite* to query from.
 * \param pos : SDL_Point as actual sprite position.
 * \param flip : SDL_renderFlip to correct H/V hitbox position.
 *
 * \return SDL_Rect : as absolute hitBox position and dimension (scaled).
 */
SDL_Rect SP_spriteHBoxGet(SP_Sprite* sprite, SDL_Point pos, SDL_RendererFlip flip);


/** \brief forces view display
 *
 * \param sprite  : SP_Sprite* to modify.
 * \param viewType : which view type to change.
 * \param viewIndex : which view to display.
 * \param frameIndex : which frame to display.
 *
 * \return void.
 */
void SP_viewForce(SP_Sprite* sprite, uint32_t viewType, uint32_t viewIndex, uint32_t frameIndex);


/**synchroneous view request*/
/** \brief view request
 *
 * \param sprite : SP_Sprite* to request view from.
 * \param viewType : which view type to change.
 * \param viewIndex : which view to request.
 *
 * \return index position of request in stack or -1 if stack is full.
 */
char SP_viewRequest(SP_Sprite* sprite, uint32_t viewType, uint32_t viewIndex);


/**enable / disable the display of a view*/
/** \brief show a view
 *
 * \param sprite : SP_Sprite* to modify.
 * \param viewType : which view type to change.
 * \param display : 0 disables display, any other value enables display.
 *
 * \return void.
 */
void SP_viewDisplay(SP_Sprite* sprite, uint32_t viewType, bool display);


/** \brief Plays animation.
 *
 * \param sprite : SP_Sprite* to animate.
 *
 * \return void.
 */
void SP_spriteStart(SP_Sprite* sprite);


/** \brief Freezes animation : pause.
 *
 * \param sprite : SP_Sprite* to pause.
 *
 * \return void.
 */
void SP_spriteStop(SP_Sprite* sprite);


/** \brief Queries locked status.
 *
 * \param sprite : SP_Sprite* to query.
 *
 * \return 1 if locked, 0 otherwise.
 * \note NVIEW status is returned.
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

#ifdef __cplusplus
}
#endif

#endif // ANIMATOR_H_INCLUDED
