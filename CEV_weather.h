//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    02-2017    |   1.0    |    creation    **/
//**   CEV    |    02-2023    |   2.0    |    creation    **/
//**********************************************************/

/**
- landmark used is as :
    0
    .___x
   /|
  / |
 z  y

with (float)Z = 1.0 as nearest, 0.1.. as furthest

- file extension : file.wtr

- ID as 0xTTOOIIII
TT = type of file = IS_WTHR (15 - 0x0F)
OO = type of object (0)
IIII = ID for this game object


- CSV file content / tab separator :
12  WEATHER_RAIN    200         -2  5   thisPic.png
ID  type            num of part Vx  Vy  picture file relative to CSV

- weather file content :
    u32 : Unique ID
        : Texture's picture unique ID (0) if embedded
        : Num of particles (num is saved, numax is read)
    s32 : Vx
        : Vy
    u8  : Weather type

    if !SrcID
    CEV_Capsule : png as texture.
**/

#ifndef WEATHER_H_INCLUDED
#define WEATHER_H_INCLUDED

#include <stdint.h>
#include <SDL.h>

#include "CEV_types.h"
#include "CEV_camera.h"

#define WEATHER_ID 0x0F000000

enum {WEATHER_SNOW, WEATHER_RAIN, WEATHER_FALL, WEATHER_NUM};

#ifdef __cplusplus
extern "C" {
#endif

typedef struct S_CEV_Particle CEV_Particle;

/** \brief Weather main structure.
 */
typedef struct S_CEV_Weather
{//weather

    uint8_t type;   /**< particle type from enum. filed*/

    bool run,       /**< weather calculation / display is active. */
         anyActive; /**< 1 particle at least is still alive (used when stopped). */

    int Vx,                 /**< x velocity pxl/frame. filed*/
        Vy,                 /**< y velocity pxl/frame. filed*/
        offScreen,          /**< off screen offset. */
        *scrollCorrectionX, /**< enables scroll correction, link to camera x pos. */
        *scrollCorrectionY; /**< enables scroll correction, link to camera y pos. */

    uint32_t ID,        /**< own identifier. filed*/
             srcID,     /**< picture (texture) id. filed 0x0 if embedded*/
             num,        /**< actual num of particles to display. filed*/
             numax;      /**< max num of particles available for display. */

    float angle;        /**< RAIN ONLY as falling rain angle / direction. */

    SDL_Rect renderSize,    /**< main render size. */
             textureSize;   /**< particle picture size. */

    SDL_Texture* texture;   /**< particle pic. filed*/

    CEV_Particle* particles;        /**< array of numax particles instances. */
}
CEV_Weather;


/** \brief Dumps weather content to stdout.
 *
 * \param this : CEV_Weather* to dump.
 * \param dumpParticles : dumps every allocated particle if true (slow).
 *
 * \return void.
 */
void CEV_weatherDump(CEV_Weather* this, bool dumpParticles);


/** \brief Dumps single particle content to stdout.
 *
 * \param this : CEV_Particle* to dump.
 *
 * \return void.
 */
void CEV_weatherParticleDump(CEV_Particle* this);


/** \brief Create a new weather instance
 *
 * \param type : uint8_t of enum of weather type.
 * \param num : number of particles for this instance.
 * \param vx : horizontal amplitude (delta x) for snow & fall, X velocity for rain.
 * \param vy : vertical speed.
 *
 * \return CEV_Weather* to instance, NULL on failure.
 */
CEV_Weather* CEV_weatherCreate(uint8_t type, unsigned int num, int vx, int vy);


/** \brief Free CEV_Weather allocations
 *
 * \param in : pointer to a CEV_Weather to free.
 * \param freePic : does function destroy embedded texture ?.
 *
 * \return N/A
 *
 * \note NULL compliant.
 */
void CEV_weatherDestroy(CEV_Weather* in, bool freePic);


/** \brief Frees CEV_Weather content and sets to 0 / NULL.
 *
 * \param in : pointer to a CEV_Weather to clear.
 * \param freePic : does function destroy embedded texture ?.
 *
 * \return N/A
 *
 * \note NULL compliant.
 */
void CEV_weatherClear(CEV_Weather* in, bool freePic);


/** \brief Updates and display weather on screen
 *
 * \param in : CEV_Weather* of instance to display.
 *
 * \return void
 */
void CEV_weatherShow(CEV_Weather* in);


/** \brief Updates and display specified sized particles
 *
 * \param in : CEV_Weather to display
 * \param min : float value as minimum size to display [0.0 ; 1.0]
 * \param max : float value as maximum size to display [0.0 ; 1.0]
 *
 * \return void
 */
void CEV_weatherShowWithLayer(CEV_Weather* in, float min, float max);


/** \brief Modify the number of particles to update/dsplay
 *
 * \param in CEV_Weather* : instance to modify
 * \param num unsigned int : new number of particles to process.
 *
 * \return number of active particles.
 */
unsigned int CEV_weatherParticleNumSet(CEV_Weather* in, unsigned int num);


/** \brief Modify number of particles to base value (as in weatherCreate)
 *
 * \param in : CEV_Weather* to apply reset on.
 *
 * \return unsigned int as number of active particles.
 */
unsigned int CEV_weatherParticleNumReset(CEV_Weather* in);


/** \brief Starts weather effect.
 *
 * \param in : CEV_Weather* to be started.
 *
 * \return void.
 *
 * \note Effect starts softly.
 */
void CEV_weatherStart(CEV_Weather* in);


/** \brief stop display
 *
 * \param in : CEV_Weather* to be stopped.
 *
 * \return void.
 */
void CEV_weatherStop(CEV_Weather* in);


/** \brief Builds / rebuilds particles.
 *
 * \param dst : CEV_Weather* of which particles must be build.
 *
 * \return void.
 *
 * \note Usefull if behaviour needs to be changed.
 * Might need to call texture attach to resize particles display.
 */
void CEV_weatherParticleBuild(CEV_Weather* dst);


/** \brief Sets maximum particle size.
 *
 * \param src : float as maximum particle size (texture size multiplier)
 * \param dst : CEV_Weather* to modify.
 *
 * \return void.
 */
void CEV_weatherParticleMaxSize(float src, CEV_Weather* dst);


/** \brief Sets minimum particle size.
 *
 * \param src : float as maximum particle size (texture size multiplier)
 * \param dst : CEV_Weather* to modify.
 *
 * \return void.
 */
void CEV_weatherParticleMinSize(float src, CEV_Weather* dst);


/** \brief Returns singe particle structure sizeof
 *
 * \param void
 *
 * \return size_t as size of single particle structure size in bytes.
 * \note Particles is opaque, to be used if alloc / realloc is needed.
 */
size_t CEV_weatherParticleSizeOf(void);



/** \brief Attached texture to use as particle.
 *
 * \param src : SDL_Texture* to be used as particle.
 * \param dst : CEV_Weather* to use this texture.
 *
 * \return void.
 *
 * \note Already attached Texture is not destroyed.
 */
void CEV_weatherTextureAttach(SDL_Texture* src, CEV_Weather* dst);


/** \brief Sets / attach camera to be used for scrolling effect.
 *
 * \param src : CEV_Camera* to be followed.
 * \param dst : CEV_Weather* to follow camera.
 *
 * \return void
 *
 * \note weather behave like on fix screen if no camera is attached.
 */
void CEV_weatherCameraAttach(CEV_Camera* src, CEV_Weather* dst);



/******FILE ******/

/** \brief loads weather file
 *
 * \param fileName : name of the weather file to read from
 *
 * \return CEV_Weather* on success, NULL on failure
 */
CEV_Weather *CEV_weatherLoad(char* fileName);


/** \brief loads weather file from RWops
 *
 * \param ops : RWops* to read from
 * \param freeSrc : if true, function will free source ops
 *
 * \return CEV_Weather* on success, NULL on failure
 */
CEV_Weather *CEV_weatherLoad_RW(SDL_RWops* ops, char freeSrc);


/** \brief Saves CEV_Weather into file.
 *
 * \param src : CEV_Weather* to save.
 * \param picture : CEV_Capsule* as picture to include in file.
 * \param fileName : char* as resulting file name.
 *
 * \return int : of standard function status.
 *
 * \note picture file is not embedded if picture is NULL.
 */
int CEV_weatherSave(CEV_Weather* src, CEV_Capsule* picture, char* fileName);


/** \brief Writes  CEV_Weather into file.
 *
 * \param src : CEV_Weather* to save.
 * \param picture : CEV_Capsule* as picture to include in file.
 * \param dst : FILE* to write into.
 *
 * \return int : of standard function status.
 *
 * \note picture file is not embedded if picture is NULL.
 */
int CEV_weatherTypeWrite(CEV_Weather* src, CEV_Capsule* picture, FILE* dst);


/****** FILE CONSTRUCTOR ******/

/** \brief turns txt file into weather data type file
 *
 * \param srcFileName : name of source file to read from
 * \param dstFileName : name of output file
 *
 * \return standard function status *
 */
int CEV_weatherConvertTxtToData(char* srcFileName, char* dstFileName);

#ifdef __cplusplus
}
#endif

#endif // WEATHER_H_INCLUDED
