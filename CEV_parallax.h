//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    06-2019    |   1.0    |    creation    **/
//**   CEV    |    12-2019    |   1.0.1  | GIF integrated **/
//**   CEV    |    04-2020    |   1.0.2  | free corrected **/
//**   CEV    |    06-2021    |   1.0.3  | features added **/
//**********************************************************/

/**LOG**/
//21/12/2019 CEV : added to display GIF animation
//06/04/2020 CEV : modified destroy functions to allow NULL as argument causing crash otherwise
//17/05/2021 CEV : structure dump added
//06/06/2021 CEV : structure modified X/Y axis parallax with options.


#ifndef PARALLAX_H_INCLUDED
#define PARALLAX_H_INCLUDED

#include <SDL.h>
#include <stdbool.h>

#include "CEV_gif.h"
#include "CEV_types.h"
#include "CEV_camera.h"
#include "CEV_texts.h"


/*
----------------------------------------------------------------------------------------------
Fichier CSV with tab separator

nombre de couche :	N (uint)
couche [N] :    Ratio X-(float) Vitesse X-(float)   is repeat X-(0/1)   posMode X-(u8)
                Ratio Y-(float) Vitesse Y-(float)   is repeat Y-(0/1)   posMode Y-(u8)
                nomFichierImage-(relatif)

-----------------------------------------------------------------------------------------------
Fichier Data .plx

uint 32	id
        nombre de couche N

couche [N] :	float	ratio X,Y
                float	vitesse X,Y
                uint 8	is repeat X,Y
                uint 8  posMode X,Y
                Capsule image
-----------------------------------------------------------------------------------------------

Layer index 0 is the farmost layer as in blit order.
*/


#define PRLX_TYPE_NAMES {"PRLX_AUTO", "PRLX_CENTERED", "PRLX_LEFT", "PRLX_RIGHT", "PRLX_TOP", "PRLX_BOTTOM"}
//                          0               1               2           3               4           5
#define PRLX_TYPE_ID (IS_PRLX<<24)


typedef enum PRLX_MODE
{
    PRLX_AUTO,      /**< Automatically handles background positioning. */
    PRLX_CENTERED,  /**< Unused. */
    PRLX_LEFT,      /**< Left border is refererence. */
    PRLX_RIGHT,     /**< Right border is reference. */
    PRLX_TOP,       /**< Top border is reference. */
    PRLX_BOTTOM,    /**< Bottom border is reference. */
    PRLX_LAST
}
PRLX_MODE;


typedef struct
{
    float pos,  /**< position offset if vel not nul. */
        ratio,  /**< scroll ratio. */
        vel;    /**< auto scroll velocity. */

    bool isRepeat;/**< weither to repeat or proportionnal. */

    uint8_t posMode;
}
PrlxAxis;


/** \brief Parallax single layer
 */
typedef struct CEV_ParaLayer
{
    SDL_Texture *pic;   /**< displayed texture. */
    SDL_Rect picSize;       /**< picture size. */
    PrlxAxis axisPar[2];    /**<axis parameters / instance. */
    bool isGif;             /**< is animated as gif file. */
    CEV_GifAnim *anim;      /**< gif instance if so. */
}
CEV_ParaLayer;


/** \brief Parallax definition
 */
typedef struct CEV_Parallax
{
    uint32_t id,            /**< Unique ID. */
             numOfLayer;    /**< number of layer. */

    SDL_Rect *cameraPos,    /**< ptr to camera position. */
              worldDim;     /**< world dimension. */

    CEV_ParaLayer *layers;  /**< array of layers. */
}
CEV_Parallax;


/** \brief Allocates new parallax object.
 *
 * \param numOfLayer : number of layer.
 * \param cameraPos : ptr to the camera position.
 *
 * \return CEV_Parallax* on success, NULL otherwise.
 */
CEV_Parallax* CEV_parallaxCreate(int numOfLayer, SDL_Rect *cameraPos);


/** \brief Destroys parallax object and its content.
 *
 * \param in : parallax object to destroy.
 *
 * \return N/A.
 */
void CEV_parallaxDestroy(CEV_Parallax *in);


/** \brief Clears parallax object content.
 *
 * \param in : parallax object to clear.
 *
 * \return N/A.
 *
 * \note All ptr freed & set to NULL.
 */
void CEV_parallaxClear(CEV_Parallax *in);


/** \brief Clears parallax layer object content.
 *
 * \param in : parallax layer to clear.
 * \return N/A.
 * note : all ptr freed & set to NULL.
 */
void CEV_parallaxLayerClear(CEV_ParaLayer* in);


/** \brief Displays all parallax layers.
 *
 * \param in : parallax object to display.
 *
 * \return N/A.
 */
void CEV_parallaxShowAll(CEV_Parallax *in);


/** \brief Displays selected parallax layer.
 *
 * \param in : parallax object to display layer from.
 * \param index : layer to display.
 *
 * \return N/A.
 */
void CEV_parallaxShowLayer(CEV_Parallax *in, unsigned index);

/** \brief Dumps structure into stdout.
 *
 * \param this : parallax object to dump.
 *
 * \return N/A.
 */
void CEV_parallaxDump(CEV_Parallax *this);


/** \brief attaches camera to parallax
 *
 * \param src : CEV_Camera* to attach.
 * \param dst : CEV_Parallax* to be attached to.
 *
 * \return void.
 */
void CEV_parallaxAttachCamera(CEV_Camera *src, CEV_Parallax *dst);


/** \brief Dumps structure into stdout.
 *
 * \param this : parallax layer to dump.
 *
 * \return N/A.
 */
void CEV_parallaxLayerDump(CEV_ParaLayer* this);


/** \brief Converts CSV descritpor file into data file.
 *
 * \param srcName : CSV file to read.
 * \param dstName : prlx file to create.
 *
 * \return any of standard return value.
 */
int CEV_parallaxConvertToData(const char* srcName, const char* dstName);


/** \brief Writes to data file from CEV_Text.
 *
 * \param src : CEV_Text* build from txt file.
 * \param dst : FILE* as destination file.
 * \param srcName : char* as name of file of src.
 *
 * \return int of standard function status.
 *
 * \note src & dst are not freed in this function.
 */
int CEV_parallaxConvertTxtToDataFile(CEV_Text *src, FILE *dst, const char* srcName);


/** \brief Loads parallax object from file.
 *
 * \param fileName : file to load.
 *
 * \return ptr to allocated parallax object.
 */
CEV_Parallax* CEV_parallaxLoad(const char* fileName);


/** \brief Saves parallax as file.
 *
 * \param src : CEV_Parallax* to be saved.
 * \param fileName : const char* as the name of file.
 *
 * \return int of std function standard.
 * \note Picture embedded whatever.
 */
int CEV_parallaxSave(CEV_Parallax* src, const char* fileName);


/** \brief Loads parallax object from virtual RWops file.
 *
 * \param src : SDL_RWops* to load from.
 * \param freeSrc : internaly frees src if true.
 *
 * \return ptr to allocated parallax object.
 * \note If freeSrc is true, src is freed weither the function succeed or not.
 */
CEV_Parallax* CEV_parallaxLoad_RW(SDL_RWops* src, uint8_t freeSrc);


/** \brief Writes parallax structure into virtual file.
 *
 * \param src : CEV_Parallax* to save.
 * \param dst : SDL_RWops* to write into.
 *
 * \return int : 0 on success, any value on error.
 */
int CEV_parallaxWrite_RW(CEV_Parallax* src, SDL_RWops* dst);

#endif // PARALLAX_H_INCLUDED
