//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |    05-2016    |   1.0    |    SDL2 rev    **/
//**   CEV    |    11-2017    |   1.0.1  | diag improved  **/
//**********************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include "CEV_mixSystem.h"
#include "CEV_api.h"
#include "CEV_picSfx.h"
#include "project_def.h"

    /*LOCAL DEFINES*/
#define PIC_VFX_DEBUG 0

    /*LOCAL STRUCTURES DECLARATION*/

/** \brief Local memo structure
 */
typedef struct L_VfxMemo
{
    char ok;
    SDL_Rect blit;
}
L_VfxMemo;


/** \brief Local status structure
 */
typedef struct L_VfxState
{
    uint8_t index;
    uint8_t active;
    uint32_t start;
    SDL_Rect blit;
}
L_VfxState;


/** \brief Local texture redefinition
 */
typedef struct L_Texture
{
    SDL_Rect dim;
    SDL_Texture *texture;
}
L_Texture;


    /*LOCAL FUNCTIONS DECLARATION*/
// TODO (drx#1#02/28/17): VFX At work...
static void L_videoFx0(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*comes from left*/
static void L_videoFx1(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*comes from top*/
static void L_videoFx2(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*unstretch from top*/
static void L_videoFx3(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*incrusted zoom in*/
static void L_videoFx4(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*zoom in to out*/
static void L_videoFx5(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*rotating zoom in*/
static void L_videoFx6(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*centered side stretch*/
static void L_videoFx7(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*fade in over*/
static void L_videoFx8(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*horizontal crossed lines*/
static void L_videoFx9(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*squares grow*/
static void L_videoFx10(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*square drops*/
static void L_videoFx11(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*square snail*/
static void L_videoFx12(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*square recto/verso*/
static void L_videoFx13(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*fx13 sub functions*/
static void L_videoFx13Sub(int x, int y, L_VfxState memo[10][10]);
static void L_videoFx13Activate(L_VfxState *memo);

/*whole recto/verso*/
static void L_videoFx14(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*corners in*/
static void L_videoFx15(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*face turn*/
static void L_videoFx16(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*static slide left to right*/
static void L_videoFx17(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*random squares in*/
static void L_videoFx18(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

/*soft alpha left to right*/
static void L_videoFx19(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

static void L_videoFx20(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime);

static SDL_Rect* L_ctrlPosToBlit(int centreX, int centreY, SDL_Rect* blit);

static L_Texture L_textureToLocal(SDL_Texture * src);

static L_Texture L_vfxCopyRender(SDL_Texture * reference);



void CEV_videoSfxSelect(SDL_Texture* dst, int which, uint32_t time)
{/*main fx selector*/

    SDL_Renderer *render = CEV_videoSystemGet()->render;

    if(dst == NULL)
        return;

    L_Texture l_src = L_vfxCopyRender(dst),
              l_dst = L_textureToLocal(dst);

    if(l_src.texture == NULL)
        return;

    if (which >= VFX_NUM)
        which = -1;

    if(!time)
        time = 1000;

    void(*function[VFX_NUM])(SDL_Renderer*, L_Texture*, L_Texture*, uint32_t)={L_videoFx0, L_videoFx1, L_videoFx2,
                                                                                L_videoFx3, L_videoFx4, L_videoFx5,
                                                                                L_videoFx6, L_videoFx7, L_videoFx8,
                                                                                L_videoFx9, L_videoFx10, L_videoFx11,
                                                                                L_videoFx12, L_videoFx13, L_videoFx14,
                                                                                L_videoFx15, L_videoFx16, L_videoFx17,
                                                                                L_videoFx18, L_videoFx19, L_videoFx20};
    if(which < 0)
        which = rand()%VFX_NUM;

#if PIC_VFX_DEBUG
    printf("pic sfx detected : %d\n", which);
#endif // PIC_VFX_DEBUG

    function[which](render, &l_src, &l_dst, time);

    SDL_DestroyTexture(l_src.texture);
}


static void L_videoFx0(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*vertical lines rotation like*/

    const int divider = 10;

    SDL_Rect blitSrc  = {.x = 0, .y = 0, .w = src->dim.w/divider, .h = src->dim.h},
             clip     = blitSrc,
             blitDst  = blitSrc;

    uint32_t startTime = SDL_GetTicks(),
            actTime    = startTime;

    int width , prev =-1;

    while(actTime <(startTime+setTime))
    {
        actTime = SDL_GetTicks();

        width  = (int)CEV_map(actTime, startTime, startTime+setTime,  0, clip.w);

        if(width != prev)
        {
            for(int j=0; j<divider; j++)
                {
                    clip.x = clip.w*j;

                    blitSrc.x = clip.x + width;
                    blitSrc.w = clip.w - width;

                    blitDst.x = clip.x;
                    blitDst.w = width;

                    SDL_RenderCopy(render, src->texture, &clip, &blitSrc);
                    SDL_RenderCopy(render, dst->texture, &clip, &blitDst);
                }
            prev = width;
            SDL_RenderPresent(render);
        }
    }
}


static void L_videoFx1(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*comes from left*/

    SDL_Rect blitPos = src->dim;
    uint32_t startTime = SDL_GetTicks();


    while( blitPos.x<=0)
    {
        blitPos.x = (int)CEV_map(SDL_GetTicks(), startTime, startTime+setTime, -blitPos.w, 0);
        SDL_RenderCopy(render, src->texture, NULL, NULL);
        SDL_RenderCopy(render, dst->texture, NULL, &blitPos);
        SDL_RenderPresent(render);
    }

    SDL_RenderCopy(render, dst->texture, NULL, NULL);
    SDL_RenderPresent(render);
}


static void L_videoFx2(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*comes from top*/

    SDL_Rect blitPos = dst->dim;
    uint32_t startTime = SDL_GetTicks();

    while( blitPos.y <=0 )
    {
        blitPos.y = (int)CEV_map(SDL_GetTicks(), startTime, startTime+setTime, -blitPos.h, 0);
        SDL_RenderCopy(render, src->texture, NULL, NULL);
        SDL_RenderCopy(render, dst->texture, NULL, &blitPos);
        SDL_RenderPresent(render);
    }

    SDL_RenderCopy(render, dst->texture, NULL, &dst->dim);
    SDL_RenderPresent(render);
}


static void L_videoFx3(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*unstretch from top*/

    SDL_Rect clip       = dst->dim;
    uint32_t startTime  = SDL_GetTicks(),
             actTime;

    while (clip.h <= src->dim.h)
    {
        actTime =SDL_GetTicks();
        clip.y  =(int)CEV_map(actTime, startTime, startTime+setTime, dst->dim.h, 0);
        clip.h  =(int)CEV_map(actTime, startTime, startTime+setTime, 0, dst->dim.h);

        SDL_RenderCopy(render, dst->texture, &clip, NULL);
        SDL_RenderPresent(render);
    }

    SDL_RenderCopy(render, dst->texture, NULL, &dst->dim);
    SDL_RenderPresent(render);
}


static void L_videoFx4(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*incrusted zoom in*/

    SDL_Rect blitPos    = dst->dim,
             clip       = src->dim;

    uint32_t startTime    = SDL_GetTicks(),
           actTime;

    while (blitPos.h <= src->dim.h)
    {
        actTime     = SDL_GetTicks();
        blitPos.w   = (int)CEV_map(actTime, startTime, startTime+setTime, 0, dst->dim.w);
        blitPos.h   = (int)CEV_map(actTime, startTime, startTime+setTime, 0, dst->dim.h);
        clip.w      = (int)CEV_map(actTime, startTime, startTime+setTime, src->dim.w, 0);
        clip.h      = (int)CEV_map(actTime, startTime, startTime+setTime, src->dim.h, 0);

        SDL_RenderCopy(render, src->texture, L_ctrlPosToBlit(src->dim.w/2, src->dim.h/2, &clip), NULL);
        SDL_RenderCopy(render, dst->texture, NULL, L_ctrlPosToBlit(dst->dim.w/2, dst->dim.h/2, &blitPos));
        SDL_RenderPresent(render);
    }

    SDL_RenderCopy(render, dst->texture, NULL, &dst->dim);
    SDL_RenderPresent(render);
}


static void L_videoFx5(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*zoom in to out*/

    SDL_Rect clip       = src->dim;

    uint32_t startTime  = SDL_GetTicks(),
             actTime     = startTime;

    while (clip.h>0)
    {
        actTime     = SDL_GetTicks();
        clip.w      = (int)CEV_map(actTime, startTime, startTime+(setTime/2), src->dim.w, 0);
        clip.h      = (int)CEV_map(actTime, startTime, startTime+(setTime/2), src->dim.h, 0);
        SDL_RenderCopy(render, src->texture, L_ctrlPosToBlit(src->dim.w/2, src->dim.h/2, &clip), NULL);
        SDL_RenderPresent(render);
    }

    startTime = actTime;

    while(clip.h < dst->dim.h)
    {
        actTime     = SDL_GetTicks();
        clip.w      = (int)CEV_map(actTime, startTime, startTime+setTime, 0, dst->dim.w);
        clip.h      = (int)CEV_map(actTime, startTime, startTime+setTime, 0, dst->dim.h);
        L_ctrlPosToBlit(dst->dim.w/2, dst->dim.h/2, &clip);
        SDL_RenderCopy(render, dst->texture, &clip, NULL);
        SDL_RenderPresent(render);
    }

    SDL_RenderCopy(render, dst->texture, NULL, &dst->dim);
    SDL_RenderPresent(render);
}


static void L_videoFx6(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*rotating zoom in*/

    SDL_Rect blitPos    = dst->dim;
    uint32_t startTime  = SDL_GetTicks(),
             actTime;

    double angle = 0.0;

    while(blitPos.h <= dst->dim.h)
    {
        actTime     = SDL_GetTicks();
        blitPos.w   = (int)CEV_map(actTime, startTime, startTime+setTime, 0, dst->dim.w);
        blitPos.h   = (int)CEV_map(actTime, startTime, startTime+setTime, 0, dst->dim.h);
        angle       = (double)CEV_map(actTime, startTime, startTime+setTime, 0, 360);

        SDL_RenderCopy(render, src->texture, NULL, &src->dim);
        L_ctrlPosToBlit(dst->dim.w/2, dst->dim.h/2, &blitPos);
        SDL_RenderCopyEx(render, dst->texture, NULL, &blitPos, angle, NULL, 0);
        SDL_RenderPresent(render);
    }

    SDL_RenderCopy(render, dst->texture, NULL, &dst->dim);
    SDL_RenderPresent(render);
}


static void L_videoFx7(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*centered side stretch*/

    SDL_Rect blitPos    = dst->dim;
    uint32_t startTime  = SDL_GetTicks(),
             actTime;

    while(blitPos.w <= dst->dim.w)
    {
        actTime     = SDL_GetTicks();
        blitPos.w   = (int)CEV_map(actTime, startTime, startTime+setTime, 0, dst->dim.w);
        blitPos.x   = (int)CEV_map(actTime, startTime, startTime+setTime, dst->dim.w/2, 0);

        SDL_RenderCopy(render, dst->texture, NULL, &blitPos);
        SDL_RenderPresent(render);
    }

    SDL_RenderCopy(render, dst->texture, NULL, &dst->dim);
    SDL_RenderPresent(render);
}


static void L_videoFx8(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*fade*/

    uint32_t startTime  = SDL_GetTicks();
    int      alpha      = 0;

    SDL_SetTextureBlendMode(dst->texture, SDL_BLENDMODE_BLEND);

    while(alpha<255)
    {
        alpha = (int)CEV_map(SDL_GetTicks(), startTime, startTime+setTime, 0, 255);

        if(alpha <= 255)
            SDL_SetTextureAlphaMod(dst->texture, (uint8_t)alpha);

        SDL_RenderCopy(render, src->texture, NULL, &src->dim);
        SDL_RenderCopy(render, dst->texture, NULL, &dst->dim);

        SDL_RenderPresent(render);
    }

    SDL_SetTextureBlendMode(dst->texture, SDL_BLENDMODE_NONE);
    SDL_RenderCopy(render, dst->texture, NULL, &dst->dim);
    SDL_RenderPresent(render);
}


static void L_videoFx9(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*horizontal crossed lines*/

    uint32_t  startTime = SDL_GetTicks();
    int       i, pos;

    SDL_Rect clip, blitPos;

    blitPos = clip = dst->dim;
    blitPos.h = clip.h /= 10;

    do
    {
        pos = (int)CEV_map(SDL_GetTicks(), startTime, startTime+setTime, dst->dim.w, 0);

        for(i=0; i<10; i++)
        {
            blitPos.y = clip.y = i*clip.h;
            blitPos.x = (i%2)? pos : -pos;

            SDL_RenderCopy(render, dst->texture, &clip, &blitPos);

        }

        SDL_RenderPresent(render);
    }while(pos>0);

    SDL_RenderCopy(render, dst->texture, NULL, &dst->dim);
    SDL_RenderPresent(render);
}


static void L_videoFx10(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*squares grow*/

    uint32_t  startTime = SDL_GetTicks(),
              actTime;

    int     i,j,
            x_space = dst->dim.w/10,
            y_space = dst->dim.h/10,
            x_add   = x_space/2,
            y_add   = y_space/2;

    SDL_Rect clip;


    do
    {
        actTime = SDL_GetTicks();
        clip.w  = (int)CEV_map(actTime, startTime, startTime+setTime, 1, x_space);
        clip.h  = (int)CEV_map(actTime, startTime, startTime+setTime, 1, y_space);

        for(i=0; i<x_space; i++)
        {
            for(j=0; j<y_space; j++)
            {
                L_ctrlPosToBlit(i*x_space + x_add, j*y_space + y_add, &clip);

                SDL_RenderCopy(render, dst->texture, &clip, &clip);
            }
        }

        SDL_RenderPresent(render);

    }while(clip.h <= dst->dim.h/10);

    SDL_RenderCopy(render, dst->texture, NULL, &dst->dim);
    SDL_RenderPresent(render);
}


static void L_videoFx11(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*square drops*/

    uint32_t    startTime,
                actTime,
                ColumnTime = 200,
                tileTime;

    int         x,y,i,j,
                direction   = INC;

    SDL_Rect    clip    = dst->dim,
                blit;

    L_VfxMemo   memo[10][10];

    blit.w = clip.w = dst->dim.w/10;
    blit.h = clip.h = dst->dim.h/10;



    for(x=0; x<10; x++)
    {
        for(y=0; y<10; y++)
        {
            memo[x][y].ok       = 0;
            memo[x][y].blit     = clip;
            memo[x][y].blit.x   = x*clip.w;
            memo[x][y].blit.y   = y*clip.h;
        }
    }

    x = 0;
    y = 9;

    do
    {
        clip.y = y*clip.h;

        do
        {
            tileTime    = (y+1)*ColumnTime/50;
            startTime   = SDL_GetTicks();
            blit.x      = clip.x     = x*clip.w;

            do
            {
                actTime = SDL_GetTicks();

                blit.y  = (int)CEV_map(actTime, startTime, startTime+tileTime, -blit.h, clip.y);

                if(blit.y > clip.y)
                    blit.y = clip.y;

                SDL_RenderCopy(render, src->texture, NULL, NULL);

                for(i=0; i<10; i++)
                {
                    for(j=0; j<10; j++)
                    {
                        if (memo[i][j].ok)
                            SDL_RenderCopy(render, dst->texture, &memo[i][j].blit, &memo[i][j].blit);
                    }
                }

                SDL_RenderCopy(render, dst->texture, &clip, &blit);
                SDL_RenderPresent(render);

            }while(blit.y < clip.y);

            memo[x][y].ok = 1;

        }while(CEV_addLimited(direction, &x, (direction)? 0: 10));

        direction^=1;

    }while(CEV_addLimited(DEC, &y, 0));

    SDL_RenderCopy(render, dst->texture, NULL, &dst->dim);
    SDL_RenderPresent(render);
}


static void L_videoFx12(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*square snail*/

    uint32_t  startTime   = SDL_GetTicks();
    int i,j,
        max_v,min_v,
        max_h,min_h,
        sens_v,sens_h,
        count,direction,
        top             = 0,
        topMemo         = 0;


    SDL_Rect clipper;

        /*****PRELIMINAIRE*****/

    count           = 100;/*10x10*/
    max_v           = max_h         = 9;
    direction       = i             = j  = 0; /*direction : 0-horizontal ; 1-vertical*/
    min_v           = min_h         = 0;
    sens_h          = sens_v        = 1; /*sens : 0-negatif 1-positif*/
    clipper.h       = dst->dim.h/10;
    clipper.w       = dst->dim.w/10;

        /*****EXECUTION*****/

    while(count)
    {
        clipper.x   = i*clipper.w;
        clipper.y   = j*clipper.h;

        top = (int)CEV_map(SDL_GetTicks(), startTime, startTime+setTime, 0, 100);

        if(top != topMemo)
        {
            topMemo = top;

            if (!direction)
            {/*horizontal*/

                if(sens_h)  /*positif*/
                    i++;
                else        /*negatif*/
                    i--;

                if (i>max_h) /*butée droite*/
                {
                    i = max_h;
                    min_v++;
                    sens_v = 1;/*on part en bas*/
                    j++;
                    direction ^= 1;/*on change de direction*/
                }
                else if(i<min_h) /*butée gauche*/
                {
                    i = min_h;
                    max_v--;
                    sens_v = 0;/*on part en haut*/
                    j--;
                    direction ^= 1;/*on change de direction*/
                }
            }
            else
            {/*vertical*/

                if(sens_v)  /*positif*/
                    j++;
                else        /*negatif*/
                    j--;

                if(j>max_v) /*butée basse*/
                {
                    j = max_v;
                    max_h--;
                    sens_h = 0;/*on part à gauche*/
                    i--;
                    direction ^= 1;/*on change de direction*/
                }
                else if(j<min_v) /*butée haute*/
                {
                    j = min_v;
                    min_h++;
                    sens_h = 1;/*on part à droite*/
                    i++;
                    direction ^= 1;/*on change de direction*/
                }
            }

            SDL_RenderCopy(render, dst->texture, &clipper, &clipper);
            SDL_RenderPresent(render);
            count--;
        }
    }
}


static void L_videoFx13(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*square recto/verso*/

    uint32_t  actTime,
            tiletime    = setTime/20;

    int     x,y;

    SDL_Rect clip   = dst->dim;

    L_Texture* picture[2] = {src, dst};

    L_VfxState memo[10][10];

    clip.w = dst->dim.w/10;
    clip.h = dst->dim.h/10;

    for(x=0; x<10; x++)
    {
        for(y=0; y<10; y++)
        {
            memo[x][y].index    = 0;
            memo[x][y].active   = 0;
            memo[x][y].blit     = clip;
            memo[x][y].blit.x   = x*clip.w;
            memo[x][y].blit.y   = y*clip.h;
        }
    }
    memo[0][0].active   = 1;
    memo[0][0].start    = SDL_GetTicks();

    while( !( (memo[9][9].index) && (memo[9][9].blit.w == clip.w) ) )
    {
        actTime = SDL_GetTicks();
        for(y=0; y<10; y++)
        {
            for(x=0; x<10; x++)
            {
                clip.x = x*clip.w;
                clip.y = y*clip.h;

                if(memo[x][y].active)
                {
                    switch(memo[x][y].index)
                    {
                        case 0:
                            memo[x][y].blit.w = CEV_map(actTime, memo[x][y].start, memo[x][y].start + tiletime, clip.w, 0);

                            if(memo[x][y].blit.w < clip.w/10)
                                L_videoFx13Sub(x,y,memo);

                            if(memo[x][y].blit.w<=0)
                            {
                                memo[x][y].index++;
                                memo[x][y].start=actTime;
                            }

                        break;

                        case 1:
                            memo[x][y].blit.w = CEV_map(actTime, memo[x][y].start, memo[x][y].start + tiletime, 0, clip.w);

                            if(memo[x][y].blit.w > clip.w)
                                memo[x][y].blit.w = clip.w;
                        break;
                    }
                }

                L_ctrlPosToBlit(clip.x+clip.w/2, clip.y+clip.h/2, &(memo[x][y].blit));
                SDL_RenderCopy(render, picture[memo[x][y].index]->texture, &clip, &(memo[x][y].blit));

            }
        }

        SDL_RenderPresent(render);
        SDL_RenderClear(render);
    }

    SDL_RenderCopy(render, dst->texture, NULL, NULL);
    SDL_RenderClear(render);
}


static void L_videoFx13Sub(int x, int y, L_VfxState memo[10][10])
{/*fx13 sub function*/
    if (y+1<10 && !memo[x][y+1].active)
        L_videoFx13Activate(&memo[x][y+1]);

    if(x+1<10 && !memo[x+1][y].active)
        L_videoFx13Activate(&memo[x+1][y]);
}


static void L_videoFx13Activate(L_VfxState *memo)
{/*fx13 sub function*/
    memo->active    = 1;
    memo->start     = SDL_GetTicks();
}


static void L_videoFx14(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*whole recto/verso*/

    SDL_Rect blitPos    = dst->dim;
    uint32_t startTime  = SDL_GetTicks(),
             actTime;

    while(blitPos.w >= 0)
    {
        actTime     = SDL_GetTicks();
        blitPos.w   = (int)CEV_map(actTime, startTime, startTime+setTime/2, dst->dim.w, 0);
        blitPos.x   = (int)CEV_map(actTime, startTime, startTime+setTime/2, 0, dst->dim.w/2);

        SDL_RenderClear(render);
        SDL_RenderCopy(render,src->texture,NULL,&blitPos);
        SDL_RenderPresent(render);
    }

    startTime = SDL_GetTicks();

    while(blitPos.w <= dst->dim.w)
    {
        actTime     = SDL_GetTicks();
        blitPos.w   = (int)CEV_map(actTime, startTime, startTime+setTime/2, 0, dst->dim.w);
        blitPos.x   = (int)CEV_map(actTime, startTime, startTime+setTime/2, dst->dim.w/2, 0);

        SDL_RenderClear(render);
        SDL_RenderCopy(render, dst->texture, NULL, &blitPos);
        SDL_RenderPresent(render);
    }


    SDL_RenderCopy(render, dst->texture, NULL, &dst->dim);
    SDL_RenderPresent(render);
}


static void L_videoFx15(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*corners in*/

    SDL_Rect blitPos    = dst->dim;
    SDL_Rect clip       = {0, 0, 0, 0};
    uint32_t startTime  = SDL_GetTicks(),
             actTime;

    while(clip.w < dst->dim.w/2)
    {
        actTime = SDL_GetTicks();

        clip.w      = (int)CEV_map(actTime, startTime, startTime+setTime,  0, dst->dim.w/2);
        clip.h      = (int)CEV_map(actTime, startTime, startTime+setTime, 0, dst->dim.h/2);
        blitPos.x   = 0;
        blitPos.y   = 0;
        blitPos.h   = clip.h;
        blitPos.w   = clip.w;

        clip.x = dst->dim.w/2 - clip.w;                      //1 haut gauche
        clip.y = dst->dim.h/2 - clip.h;

        SDL_RenderCopy(render, dst->texture, &clip, &blitPos);

        clip.x      = dst->dim.w/2;                              //2 haut droite
        blitPos.x   = dst->dim.w - clip.w;

        SDL_RenderCopy(render, dst->texture, &clip, &blitPos);

        clip.y      = dst->dim.h/2;                              //3 bas droite
        blitPos.y   = dst->dim.h - clip.h;

        SDL_RenderCopy(render, dst->texture, &clip, &blitPos);

        clip.x      = dst->dim.w/2 - clip.w;                     //4 bas gauche
        blitPos.x   = 0;

        SDL_RenderCopy(render, dst->texture, &clip, &blitPos);

        SDL_RenderPresent(render);
    }
}


static void L_videoFx16(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*face turn*/

    SDL_Rect blitPos    = dst->dim;
    uint32_t startTime  = SDL_GetTicks(),
             actTime;
    int x               = 0;

    while(x < dst->dim.w)
    {
        actTime = SDL_GetTicks();

        x  = (int)CEV_map(actTime, startTime, startTime+setTime,  0, dst->dim.w);
        blitPos.x = 0;
        blitPos.w = x;

        SDL_RenderCopy(render, dst->texture, NULL, &blitPos);

        blitPos.x = x;
        blitPos.w = dst->dim.w - x;
        SDL_RenderCopy(render, src->texture, NULL, &blitPos);

        SDL_RenderPresent(render);
    }
}


static void L_videoFx17(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*static slide left to right*/

    SDL_Rect blitPos    = dst->dim;
    SDL_Rect clip       = {0, 0, 0, 0};
    uint32_t startTime  = SDL_GetTicks(),
             actTime;
    int x               = 0;

    clip.w = dst->dim.w;
    clip.h = dst->dim.h;

    while(x < dst->dim.w)
    {
        actTime = SDL_GetTicks();

        x           = (int)CEV_map(actTime, startTime, startTime+setTime,  0, dst->dim.w);
        clip.w      = x,
        blitPos.w   = x;

        SDL_RenderCopy(render, dst->texture, &clip, &blitPos);
        SDL_RenderPresent(render);
    }
}


static void L_videoFx18(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*randomized squares*/

    const int div = 10;

    SDL_Rect clip = {0, 0, dst->dim.w/div, dst->dim.h/div};

    int length = div*div;
    int tab[length];

    //filling tab
    for(int i=0; i<length; i++)
        tab[i] = i;

    //shaking tab
    for(int i=0; i<length; i++)
    {
        int from = CEV_irand(0, length),
            to   = CEV_irand(0, length);

        CEV_intSwap(&tab[from], &tab[to]);
    }

    int index = 0;

    while(index < length)
    {
        clip.x = clip.w *(tab[index]%div);//clip.w * blit.x;
        clip.y = clip.h *(tab[index]/div);//clip.h * blit.y;

        SDL_RenderCopy(render, dst->texture, &clip, &clip);

        SDL_RenderPresent(render);
        SDL_Delay(setTime/length);
        index++;
    }

    SDL_RenderCopy(render, dst->texture, NULL, NULL);
    SDL_RenderPresent(render);
}

static void L_videoFx19(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*alpha from left*/

    int alpha           = 0,
        end             = 1,
        max             = dst->dim.w + 255;

    bool done           = false;

    uint32_t now = SDL_GetTicks();

    SDL_Rect clip = {0, 0, 1, dst->dim.h},
             blit = {0, 0, 1, src->dim.h};

    while (!done)
    {
        end = (int)CEV_map(SDL_GetTicks(),now, now+setTime, 0, max);
        CEV_constraint(0, &end, max);

        SDL_RenderCopy(render, src->texture, NULL, NULL);

        for(int i=0; i<end; i++)
        {
            clip.x = i;
            CEV_constraint(0, &clip.x, dst->dim.w-1);
            blit.x = clip.x;

            alpha = (end-i<255)? end-i : 255;

            SDL_SetTextureAlphaMod(dst->texture, alpha);
            SDL_RenderCopy(render, dst->texture, &clip, &blit);
        }

        SDL_RenderPresent(render);
        //SDL_Delay(10);
        //end++;
        done = end >= max;
    }
}


static void L_videoFx20(SDL_Renderer* render, L_Texture* src, L_Texture* dst, uint32_t setTime)
{/*flash in to alpha fade out*/

    uint32_t start = SDL_GetTicks();
    bool done = false;
    int alpha = SDL_ALPHA_OPAQUE;

    while(!done)
    {
        alpha = CEV_map(SDL_GetTicks(), start, start+setTime, 255, 0);

        if(alpha>=0)
        {
            SDL_SetTextureAlphaMod(dst->texture, alpha);
            SDL_RenderCopy(render, dst->texture, NULL, NULL);
            SDL_RenderPresent(render);
            SDL_RenderClear(render);
        }
        else
            break;
    }
}

static SDL_Rect* L_ctrlPosToBlit(int centreX, int centreY, SDL_Rect* blit)
{/*position de blit depuis le centre de gravité*/

    blit->x=centreX-(blit->w/2);
    blit->y=centreY-(blit->h/2);

    return blit;
}


static L_Texture L_textureToLocal(SDL_Texture * src)
{
    L_Texture result;

    SDL_QueryTexture(src, NULL, NULL, &result.dim.w, &result.dim.h);
    result.texture = src;
    result.dim.x = 0;
    result.dim.y = 0;

    return result;
}


static L_Texture L_vfxCopyRender(SDL_Texture *reference)
{//copying render display into texture

    L_Texture result = {.texture = NULL, .dim = (SDL_Rect){.x=0, .y=0, .w=0, .h=0}};

    SDL_Texture *dst = NULL;

    uint32_t refFormat = 0;

    int refW    = 0,
        refH    = 0,
        pitch   = 0;

    void *renderPxl = NULL,
          *dstPxl   = NULL;

    SDL_Renderer *render = CEV_videoSystemGet()->render;

    SDL_QueryTexture(reference, &refFormat, NULL, &refW, &refH);//reprise format texture envoyée
    //printf("texture size is %d, %d\n", refW, refH);
    //SDL_RenderGetLogicalSize(render, &refW, &refH); //reprise dimension render OK
    SDL_GetRendererOutputSize(render, &refW, &refH);//test alt
    //printf("render size = %d*%d\n", refW, refH);

    dst = SDL_CreateTexture(render, refFormat, SDL_TEXTUREACCESS_STREAMING, refW, refH);//création surface

    if(dst == NULL)
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        goto exit;
    }

    SDL_LockTexture(dst, NULL, &dstPxl, &pitch);

    renderPxl = calloc(pitch * refH, sizeof(uint32_t));

    if(renderPxl == NULL)
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        goto exit;
    }

    SDL_RenderReadPixels(render, NULL, refFormat, renderPxl, pitch);

    memcpy(dstPxl, renderPxl, refH * pitch);

    SDL_UnlockTexture(dst);

    SDL_SetTextureBlendMode(dst, SDL_BLENDMODE_BLEND);

    result = L_textureToLocal(dst);

    free(renderPxl);

exit:
    return result;
}

