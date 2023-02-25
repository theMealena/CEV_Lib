//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    02-2017    |   1.0    |    creation    **/
//**********************************************************/

/*
landmark used is as :
    0
    .___x
   /|
  / |
 z  y

with (float)Z = 1.0 as nearest, 0.0.. as furthest
*/

#ifndef WEATHER_H_INCLUDED
#define WEATHER_H_INCLUDED

#include <SDL.h>
#include <CEV_camera.h>

#define WEATHER_SNOW 0
#define WEATHER_RAIN 1
#define WEATHER_FALL 2
#define WEATHER_NUM 3

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SWeather CEV_Weather;


/** \brief Create a new weather instance
 *
 * \param texture : SDL_Texture* to use as particle.
 * \param type : uint8_t of enum of weather type.
 * \param num : number of particles for this instance.
 * \param vx : horizontal amplitude (delta x) for snow & fall, X velocity for rain.
 * \param vy : vertical speed.
 *
 * \return pointer to a CEV_Weather instance, NULL on failure.
 */
CEV_Weather *CEV_weatherCreate(SDL_Texture *texture, uint8_t type, unsigned int num, int vx, int vy);


/** \brief Free CEV_Weather allocations
 *
 * \param in : pointer to a CEV_Weather to free.
 * \param freePic : does function destroy embedded texture ?.
 *
 * \return N/A
 *
 * \note NULL compliant.
 */
void CEV_weatherDestroy(CEV_Weather *in, bool freePic);


/** \brief Updates and display weather on screen
 *
 * \param in : CEV_Weather* of instance to display.
 *
 * \return void
 */
void CEV_weatherShow(CEV_Weather*in);


/** \brief Updates and display specified sized particles
 *
 * \param in : CEV_Weather to display
 * \param min : float value as minimum size to display [0.0 ; 1.0]
 * \param max : float value as maximum size to display [0.0 ; 1.0]
 *
 * \return void
 */
void CEV_weatherShowWithLayer(CEV_Weather*in, float min, float max);


/** \brief Modify the number of particles to update/dsplay
 *
 * \param in CEV_Weather* : instance to modify
 * \param num unsigned int : new number of particles to process.
 *
 * \return number of active particles.
 */
unsigned int CEV_weatherNumSet(CEV_Weather *in, unsigned int num);


/** \brief Modify number of particles to base value (as in weatherCreate)
 *
 * \param in : CEV_Weather* to apply reset on.
 *
 * \return unsigned int as number of active particles.
 */
unsigned int CEV_weatherNumReset(CEV_Weather *in);



/** \brief Sets / attach camera to be used for scrolling effect.
 *
 * \param src : CEV_Camera* to be followed.
 * \param dst : CEV_Weather* to follow camera.
 *
 * \return void
 *
 * \note weather behave like on fix screen if no camera is attached.
 */
void CEV_weatherCameraSet(CEV_Camera *src, CEV_Weather* dst);


/** \brief Starts weather effect.
 *
 * \param in : CEV_Weather* to be started.
 *
 * \return void.
 *
 * \note Effect starts softly.
 */
void CEV_weatherStart(CEV_Weather *in);


/** \brief stop display
 *
 * \param in : CEV_Weather* to be stopped.
 *
 * \return void
 */
void CEV_weatherStop(CEV_Weather *in);
void CEV_weatherMaxSize(float src, CEV_Weather* dst, SDL_Rect textureDim);
void CEV_weatherMinSize(float src, CEV_Weather* dst, SDL_Rect textureDim);





/****** FILE CONSTRUCTOR ******/

/** \brief turns csv file into weather data type file
 *
 * \param src : name of source file to read from
 * \param dst : name of output file
 *
 * \return standard function status *
 */
int CEV_convertWeatherCSVToData(char* src, char* dst);


/******FILE LOADING******/

/** \brief loads weather file
 *
 * \param fileName : name of the weather file to read from
 *
 * \return CEV_Weather* on success, NULL on failure
 */
CEV_Weather *CEV_weatherLoad(char *fileName);


/** \brief loads weather file from RWops
 *
 * \param ops : RWops* to read from
 * \param freeSrc : if true, function will free source ops
 *
 * \return CEV_Weather* on success, NULL on failure
 */
CEV_Weather *CEV_weatherLoad_RW(SDL_RWops* ops, char freeSrc);

#ifdef __cplusplus
}
#endif

#endif // WEATHER_H_INCLUDED
