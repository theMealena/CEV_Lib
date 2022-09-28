//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  28-09-2022   |   1.0    |    creation    **/
//**********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <SDL.h>
#include <CEV_input.h>
#include <CEV_mixSystem.h>
#include <CEV_types.h>
#include "CEV_mp3.h"

// TODO (drx#1#): Tester plusieurs fichier, semble ok, nettoyer et finaliser ...


/** \brief loads texture from mp3 file.
 *
 * \param src : SDL_RWops* to load texture from.
 *
 * \return SDL_Texture* on success, NULL on failure.
 *
 */
SDL_Texture *L_mp3TextureFetch(SDL_RWops* src, bool freeSrc);

/** \brief fetches ID3V1 tags if available.
 *
 * \param src : SDL_RWops* to fetch from.
 * \param dst : CEV_Mp3* to fill with tags.
 *
 * \return bool : true on success.
 *
 */
bool L_mp3TagFetch(SDL_RWops* src, CEV_Mp3* dst);

/** \brief Fetches ID3V2 artist tag
 *
 * \param src : SDL_RWops* to fetch from.
 * \param dst[] : char[] filled with result.
 *
 * \return int as num of char fetched.
 *
 * note : dst must be large enough to hold result [61] recommended.
 */
int L_mp3ArtistFetch(SDL_RWops* src, char dst[]);

/** \brief Fetches ID3V2 title tag
 *
 * \param src : SDL_RWops* to fetch from.
 * \param dst[] : char[] filled with result.
 *
 * \return int as num of char fetched.
 *
 * note : dst must be large enough to hold result [61] recommended.
 */
int L_mp3TitleFetch(SDL_RWops* src, char dst[]);

/** \brief Fetches ID3V2 year tag
 *
 * \param src : SDL_RWops* to fetch from.
 * \param dst[] : char[] filled with result.
 *
 * \return int as num of char fetched.
 *
 * note : dst must be large enough to hold result [61] recommended.
 */
int L_mp3YearFetch(SDL_RWops* src, char dst[]);

/** \brief Fetches ID3V2 track tag
 *
 * \param src : SDL_RWops* to fetch from.
 * \param dst[] : char[] filled with result.
 *
 * \return int as num of char fetched.
 *
 * note : dst must be large enough to hold result [61] recommended.
 */
int L_mp3TrackFetch(SDL_RWops* src, char dst[]);

/** \brief Fetches ID3V2 album tag
 *
 * \param src : SDL_RWops* to fetch from.
 * \param dst[] : char[] filled with result.
 *
 * \return int as num of char fetched.
 *
 * note : dst must be large enough to hold result [61] recommended.
 */
int L_mp3AlbumFetch(SDL_RWops* src, char dst[]);

/** \brief set file ptr at begining of next available string.
 *
 * \param src : SDL_RWops* on which to perform operation.
 *
 * \return void
 */
void L_gotoNxtStr(SDL_RWops* src);

/** \brief Scans file for tag.
 *
 * \param src : SDL_RWops* in which to perform scan.
 * \param tag : const char* as tag to be found.
 *
 * \return size_t as tag offset / 0 if not found.
 */
size_t L_findTag(SDL_RWops* src, const char* tag);


/** \brief Reads single string
 *
 * \param src : SDL_RWops* to read from.
 * \param dst[] : char[] filled with result.
 *
 * \return int as num of char fetched.
 *
 * note : dst must be large enough to hold result [61] recommended.
 */
int L_getStr(SDL_RWops* src, char dst[]);

/** \brief Reads single tag content.
 *
 * \param src : SDL_RWops* to read from.
 * \param dst[] : char[] filled with result.
 *
 * \return int as num of char fetched.
 *
 * note : dst must be large enough to hold result [61] recommended.
 */
int L_getTagContent(SDL_RWops* src, char dst[]);


void CEV_mp3TEST(void)
{
    CEV_Input *input = CEV_inputGet();
    CEV_inputClear();
    CEV_Capsule caps;
    SDL_Renderer *render = CEV_videoSystemGet()->render;

    CEV_capsuleLoad(&caps, "mp3Test.mp3");

    SDL_RWops* vFile = SDL_RWFromMem(caps.data, caps.size);

    CEV_Mp3 *music = CEV_mp3Load_RW(vFile, 1);

    //memset(caps.data, 0, caps.size);
    printf("album is %s\n", music->album);
    printf("artist is %s\n", music->artist);
    printf("title is %s\n", music->title);
    printf("year is %s\n", music->year);
    printf("track is %s\n", music->track);
    //CEV_systemGet()->sound.loadedMusic = music;

    Mix_PlayMusic(music->music, 1);

    SDL_Rect blit = CEV_textureDimGet(music->img);
    blit = CEV_rectCenteredInRect(blit, (SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT});

    SDL_RenderCopy(render, music->img, NULL, &blit);
    SDL_RenderPresent(render);

    bool stop = false;

    while(!stop)
    {
        CEV_inputUpdate();
        stop = !Mix_PlayingMusic() || input->window.quitApp;
        SDL_Delay(1000);
    }

    Mix_HaltMusic();
    SDL_DestroyTexture(music->img);

    Mix_FreeMusic(music->music);
    free(music);
    CEV_capsuleClear(&caps);
}


CEV_Mp3* CEV_mp3Load(char *fileName)
{//Loads mp3 file

    SDL_RWops* vFile = SDL_RWFromFile(fileName, "rb");

    if (IS_NULL(vFile))
    {
        fprintf(stderr, "Err at %s / %d : unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    CEV_Mp3* result = CEV_mp3Load_RW(vFile, 1);

    if (IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Could not extract mp3 from file.\n", __FUNCTION__, __LINE__, Mix_GetError());
    }

    return result;
}


CEV_Mp3* CEV_mp3Load_RW(SDL_RWops* src, bool freeSrc)
{//Loads mp3 file from RWops
    if (IS_NULL(src))//MIX does not check SDL_RWops* != NULL better do it myself*/
    {
        fprintf(stderr, "Err at %s / %d : arg is NULL.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_Mp3* result = calloc(1, sizeof(CEV_Mp3));

    //doing my own business before givin vfile,
    result->img = CEV_mp3TextureFetch(src, 0);

    if(IS_NULL(result->img))
    {
        fprintf(stderr, "Err at %s / %d :  img is NULL .\n", __FUNCTION__, __LINE__ );
    }
    printf("at load img is %p\n", result->img);

    //if(!CEV_mp3TagFetch(src, result))
    {//retrying with ID3 tags
        CEV_mp3ArtistFetch(src, result->artist);
        CEV_mp3TitleFetch(src, result->title);
        CEV_mp3AlbumFetch(src, result->album);
        CEV_mp3YearFetch(src, result->year);
        CEV_mp3TrackFetch(src, result->track);
    }

    SDL_RWseek(src, 0, SEEK_SET);

    result->music = Mix_LoadMUS_RW(src, freeSrc); //ask to close rwops

    if (result->music == NULL)
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, Mix_GetError());
        return NULL;
    }

    //strcpy(result->album, Mix_GetMusicAlbumTag(result->music));

    return result;
}


void CEV_mp3Destroy(CEV_Mp3* src)
{//Frees content and itself

    CEV_Mp3Clear(src);
    free(src);
}


void CEV_mp3Clear(CEV_Mp3* src)
{//Frees content

    SDL_DestroyTexture(src->img);
    src->img = NULL;

    Mix_FreeMusic(src->music);
    src->music = NULL;

    free(src->rawData);
    src->rawData = NULL;
}


    /*** Locals ***/


SDL_Texture *L_mp3TextureFetch(SDL_RWops* src)
{//fetches embbeded picture
//APIC

    SDL_Renderer *render = CEV_videoSystemGet()->render;

    const char *picID   = "APIC",
               *imgDesc = "image/";

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : arg is NULL.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    SDL_RWseek(src, 0, SEEK_SET);

    if(!L_findTag(src, picID))
    {
        fprintf(stderr, "Err at %s / %d : mp3 file does not include picture.\n", __FUNCTION__, __LINE__ );
        //SDL_RWclose(src);
        return NULL;
    }

    L_findTag(src, imgDesc);
    while(SDL_ReadU8(src));

    SDL_RWseek(src, 2, SEEK_CUR);

    SDL_Texture *img = IMG_LoadTexture_RW(render, src, false);

    if(IS_NULL(img))
    {
        fprintf(stderr, "Err at %s / %d : Unable to create texture  .\n", __FUNCTION__, __LINE__, IMG_GetError());
        return NULL;
    }

   return img;
}


bool L_mp3TagFetch(SDL_RWops* src, CEV_Mp3* dst)
{//fetches all available tags
//TAG

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : arg is NULL.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    size_t tagExists = L_findTag(src, "TAG");

    if(!tagExists)
    {
        fprintf(stderr, "Err at %s / %d : no TAG found in this file.\n", __FUNCTION__, __LINE__ );
        return FUNC_ERR;
    }

    L_getStr(src, dst->album);
    L_gotoNxtStr(src);
    L_getStr(src, dst->artist);
    L_gotoNxtStr(src);
    L_getStr(src, dst->title);
    L_gotoNxtStr(src);
    L_getStr(src, dst->year);

    return tagExists;
}


int L_mp3ArtistFetch(SDL_RWops* src, char dst[])
{//feetches mp3 artist from RWops
//TPE1

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : arg is NULL.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    SDL_RWseek(src, 0, SEEK_SET);

    if(!L_findTag(src, "TPE1"))
    {
        fprintf(stderr, "Err at %s / %d : mp3 file does not include band name.\n", __FUNCTION__, __LINE__ );
        //SDL_RWclose(vmp3);
        return NULL;
    }

    return L_getTagContent(src, dst);
}


int L_mp3TitleFetch(SDL_RWops* src, char dst[])
{//feetches mp3 title from RWops
//TIT2

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : arg is NULL.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    SDL_RWseek(src, 0, SEEK_SET);

    if(!L_findTag(src, "TIT2"))
    {
        fprintf(stderr, "Err at %s / %d : mp3 file does not include title.\n", __FUNCTION__, __LINE__ );
        //SDL_RWclose(vmp3);
        return NULL;
    }

    return L_getTagContent(src, dst);

}


int L_mp3YearFetch(SDL_RWops* src, char dst[])
{//feetches mp3 year from RWops
//TYER
    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : arg is NULL.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    SDL_RWseek(src, 0, SEEK_SET);

    if(!L_findTag(src, "TYER"))
    {
        fprintf(stderr, "Err at %s / %d : mp3 file does not include year.\n", __FUNCTION__, __LINE__ );
        //SDL_RWclose(vmp3);
        return NULL;
    }

    return L_getTagContent(src, dst);


}


int L_mp3TrackFetch(SDL_RWops* src, char dst[])
{//feetches mp3 track from RWops
//TRCK
    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : arg is NULL.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    SDL_RWseek(src, 0, SEEK_SET);

    if(!L_findTag(src, "TRCK"))
    {
        fprintf(stderr, "Err at %s / %d : mp3 file does not include year.\n", __FUNCTION__, __LINE__ );
        //SDL_RWclose(vmp3);
        return NULL;
    }

    return L_getTagContent(src, dst);

}


int L_mp3AlbumFetch(SDL_RWops* src, char dst[])
{//feetches mp3 album from RWops
//TALB
    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : arg is NULL.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    SDL_RWseek(src, 0, SEEK_SET);

    if(!L_findTag(src, "TALB"))
    {
        fprintf(stderr, "Err at %s / %d : mp3 file does not include year.\n", __FUNCTION__, __LINE__ );
        //SDL_RWclose(vmp3);
        return NULL;
    }

    return L_getTagContent(src, dst);

}


void L_gotoNxtStr(SDL_RWops* src)
{//set file ptr at begining of next available string

    char byte = 0;
    uint64_t count = SDL_RWsize(src) - SDL_RWtell(src);

    while(!SDL_ReadU8(src) && count)
        count--;

    if(count)
        SDL_RWseek(src, -1, SEEK_CUR);
}


size_t L_findTag(SDL_RWops* src, const char* tag)
{//reaches specified tag

    unsigned int occ    = 0,
                 ok     = strlen(tag),
                 pos    = 0;

    uint8_t      byte   = 0;

    size_t fileSize     = SDL_RWsize(src),
           actPos       = SDL_RWtell(src),
           maxRead      = fileSize- actPos;

    while(maxRead--)
    {
        byte = SDL_ReadU8(src);

        if(byte == tag[occ])
        {
            if(!occ)
                pos = SDL_RWtell(src);

            occ++;
        }
        else
            occ = 0;

        if(occ == ok)
        {
            return pos;
        }
    }

    return 0;
}


int L_getStr(SDL_RWops* src, char dst[])
{//reads string
    int count = 0;

    while(dst[count] = SDL_ReadU8(src))
        count++;

    return count;
}


int L_getTagContent(SDL_RWops* src, char dst[])
{//reads single tag content

    size_t strLen = SDL_ReadBE32(src);

    printf("len = %d\n", strLen);
    SDL_RWseek(src, 3, SEEK_CUR);

    int index = 0;

    while(--strLen)
    {
        dst[index++] = SDL_ReadU8(src);
    }

    dst[++index] = '\0';

    return index;
}


