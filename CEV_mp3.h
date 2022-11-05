//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  28-09-2022   |   1.0    |    creation    **/
//**********************************************************/

//CEV   - 2022/10/29 - music control added (upon SDL_mixer)
//      - play / stop / isPlaying


#ifndef CEV_MP3_H_INCLUDED
#define CEV_MP3_H_INCLUDED

#include <SDL.h>
#include <SDL_mixer.h>


typedef struct CEV_Mp3
{
    Mix_Music* music;   /**< resulting Mix_Music */

    char artist[61],/**< artist name (length define by mp3 standard) */
         title[61], /**< song title (length define by mp3 standard) */
         album[61], /**< album name (length define by mp3 standard) */
         year[10],  /**< release year */
         track[10]; /**< track number  */

    SDL_Texture *img;   /**< album cover */
    void *rawData;      /**< mp3 raw file */
}
CEV_Mp3;


void CEV_mp3TEST(void);


/** \brief Loads mp3 from file.
 *
 * \param fileName : char* as name of file to load.
 *
 * \return CEV_Mp3* as result on success, NULL on error.
 */
CEV_Mp3* CEV_mp3Load(char *fileName);


/** \brief Loads mp3 from RWops.
 *
 * \param src : SDL_RWops* to load from.
 * \param freeSrc : frees src if true.
 *
 * \return CEV_Mp3* on success, NULL on error.
 */
CEV_Mp3* CEV_mp3Load_RW(SDL_RWops* src, bool freeSrc);


/** \brief Fetches embedded picture if any
 *
 * \param src : CEV_Mp3* to fetch texture from.
 *
 * \return SDL_Texture* on succes, NULL on failure.
 */
SDL_Texture* CEV_mp3TextureFetch(CEV_Mp3* src);


/** \brief Plays mp3 music.
 *
 * \param src : CEV_Mp3* to play.
 * \param loops : int as numner of loops to perform.
 *
 * \return void.
 */
void CEV_mp3MusicPlay(CEV_Mp3 *src, int loops);


/** \brief Is mp3 actually plaing ?
 *
 * \return true if playing, false otherwise.
 */
bool CEV_mp3IsPlaying(void);


/** \brief Stops playing music.
 *
 * \return void.
 */
void CEV_mp3MusicStop(void);


/** \brief Destroys Mp3 structure and content.
 *
 * \param src : CEV_Mp3* to free.
 *
 * \return void.
 */
void CEV_mp3Destroy(CEV_Mp3* src);


/** \brief Frees Mp3 structure content.
 *
 * \param src : CEV_Mp3* to clear.
 *
 * \return void
 * note : freed content is set to NULL.
 */
void CEV_mp3Clear(CEV_Mp3* src);

#endif // CEV_MP3_H_INCLUDED
