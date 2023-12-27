//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    10-2017    |   1.0    |  creation/SDL2 **/
//**   CEV    |    07-2021    |   1.1    |  pic embedded & NULL tolerance **/
//**********************************************************/
//2022/09/18 CEV : CEV_mapWorldPointToDisplayTile corrected


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <SDL.h>
#include "project_def.h"
#include "CEV_API.h"
#include "CEV_maps.h"
#include "CEV_dataFile.h"
#include "rwtypes.h"




	/** Local functions declarations **/


/** \brief animates tile
 *
 * \param [in] tile : tile to animate
 * \param [in] now : time ticks
 *
 * \return void
 */
static void L_mapAnimUpdate(MAP_Tile *tile, unsigned int now);


/**\brief Fills map structure reading from file
 *
 *  \param [in] src : SDL_RWops ptr to read from
 *  \param [in] dst : CEV_TileMap ptr to structure to be filled
 *
 *  \return N/A
 *
 *  note : Local function called by CEV_MapLoad_RW
 */
static void L_mapTypeRead_RW(SDL_RWops* src, CEV_TileMap* dst);


/**\brief write mapping into file
 *
 *  \param [in] src : CEV_TileMap to be saved
 *  \param [in] dst : file ptr to write into
 *
 *  \return Return description
 *
 *  note : More details
 */
static void L_mapTypeWrite(const CEV_TileMap* src, FILE* dst);


 /**\brief Read lone tile structure
 *
 *  \param [in] src : SDL_RWops to read from
 *  \param [in] dst : MAP_Tile ptr to structure to be filled
 *
 *  \return N/A
 *
 *  note : Local function called by L_TMapReadFromFile_RW
 */
static void L_tileTypeRead_RW(SDL_RWops* src, MAP_Tile* dst);


/** \brief Write lone tile structure
 *
 * \param src : const MAP_Tile* to be saved
 * \param dst : FILE* to write into
 *
 * \return void
 *
 * note : Sets readWriteErr
 */
static void L_tileTypeWrite(MAP_Tile* src, FILE* dst);


/** \brief Reads tile animation parameters
 *
 * \param [in] src : SDL_RWops to read from
 * \param [in] dst : MAP_TileAnim ptr to structure to be filled
 * \return
 *
 */
static void L_tileAnimTypeRead_RW(SDL_RWops *src, MAP_TileAnim *dst);


/** \brief Write tile animation parameters
 *
 * \param [in] src : const MAP_TileAnim* to be saved
 * \param [in] dst : FILE* to write into
 *
 * \return void
 *
 * note : Sets readWriteErr
 */
static void L_tileAnimTypeWrite(MAP_TileAnim *src, FILE *dst);


/** \brief init single tile with default values
 *
 * \param [in] tile : pointer to MAP_Tile to init
 *
 * \return void
 */
static void L_tileInit(MAP_Tile* tile);


void CEV_mapDump(CEV_TileMap* this)
{//dumps CEV_TileMap content (main parameters)

    puts("*** BEGIN CEV_TileMap  ***");

    if(IS_NULL(this))
	{
		puts("This CEV_TileMap is NULL");
        goto end;
	}
    printf("\tis at %p\n", this);
    printf("\ttileSize : %d\n", this->tileSize);
    printf("\tpicture id : %08X\n", this->picId);
    printf("\ttexture is at %p\n", this->pic);

    puts("World in pixels is :");
    CEV_rectDump(this->dim.world.pixels);
    puts("World in tiles is :");
    CEV_rectDump(this->dim.world.tiles);

    puts("TileSet in pixels is :");
    CEV_rectDump(this->dim.tileSet.pixels);
    puts("TileSet in tiles is :");
    CEV_rectDump(this->dim.tileSet.tiles);

    puts("Display in pixels is :");
    CEV_rectDump(this->dim.display.pixels);
    puts("Display in tiles is :");
    CEV_rectDump(this->dim.display.tiles);

    puts("Display offset in pixels is :");
    CEV_rectDump(this->dim.dispOffset.pixels);
    puts("Display offset in tiles is :");
    CEV_rectDump(this->dim.dispOffset.tiles);


	// #1 printf("\town sprite's content is : \n");

    // #1 CEV_spriteMiniDump(&this->sprite);

end:
    puts("***END CEV_TileMap ***");



}

	/*User functions implementation*/

CEV_TileMap* CEV_mapLoad(const char* fileName)
{//loads map from file

	/**PRL**/

	if IS_NULL(fileName)
	{
		fprintf(stderr, "Err at %s / %d : argument is NULL.\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	SDL_RWops *vfile = NULL;

	CEV_TileMap *result = NULL;

	/**EXE**/

	//turning file into SDL_RWops
	vfile = SDL_RWFromFile(fileName, "rb");

	if IS_NULL(vfile)
	{//on error
		fprintf(stderr, "Err at %s / %d : unable to read file %s : %s.\n", __FUNCTION__, __LINE__, fileName, SDL_GetError());
		return NULL;
	}

	//go and read file
	result = CEV_mapLoad_RW(vfile, 1);

	return result;
}


int CEV_mapSave(CEV_TileMap* src, const char* dstFileName)
{//saves map into file

	/**PRL**/

	if IS_NULL(src)
	{
		fprintf(stderr, "Err at %s / %d : argument 1 is NULL.\n", __FUNCTION__, __LINE__);
		return ARG_ERR;
	}
	else if IS_NULL(dstFileName)
	{
		fprintf(stderr, "Err at %s / %d : argument 2 is NULL.\n", __FUNCTION__, __LINE__);
		return ARG_ERR;
	}

	/**EXE**/

	readWriteErr = 0;
    bool embeddPic = false;
	CEV_Capsule caps = {0};

	//if tile set is to be embedded
	if(!src->picId)
    {
        if NOT_NULL(src->pic)
        {
            //saving texture as png first
            //CEV_textureSavePNG(src->pic, dstFileName);

            //now picking this file as capsule
            //CEV_capsuleFromFile(&caps, dstFileName);
            //creating png capsule from texture
            CEV_textureToCapsule(src->pic, &caps);
            embeddPic = true;

        }
        else
        {
            fprintf(stderr, "Err at %s / %d : Cannot embedd NULL tileSet.\n", __FUNCTION__, __LINE__ );
            embeddPic = false;
        }
    }

    FILE* dst = fopen(dstFileName, "wb");//will remove previous file

    if IS_NULL(dst)
    {
        fprintf(stderr, "Err at %s / %d : unable to open file %s : %s", __FUNCTION__, __LINE__, dstFileName, strerror(errno));
        return FUNC_ERR;
    }

    write_u8(embeddPic, dst);

	L_mapTypeWrite(src, dst);

	if(embeddPic)
        CEV_capsuleTypeWrite(&caps, dst);

	fclose(dst);

	CEV_capsuleClear(&caps);

	return (readWriteErr)? FUNC_ERR : FUNC_OK ;
}


CEV_TileMap* CEV_mapLoad_RW(SDL_RWops* src, char freeSrc)
{//loads map from RWops

	/**PRL**/

	if IS_NULL(src)
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        return NULL;
    }

    uint8_t embed   = SDL_ReadU8(src);  //is tileset embedded ?

    uint32_t
            layer   = SDL_ReadLE32(src),    //reading num of layers
            tileSet = SDL_ReadLE32(src),    //reading tile set ID
            width 	= SDL_ReadLE32(src), 	//reading map width in tile
            height 	= SDL_ReadLE32(src),	//reading map height in tile
            tilePx 	= SDL_ReadLE32(src);	//reading tile size

	//creating structure
	CEV_TileMap* result = CEV_mapCreate(layer, width, height, tilePx);

	if IS_NULL(result)
	{//on error
		fprintf(stderr, "Err at %s / %d : unable to create tile map.\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	result->picId = tileSet;

	//go and read mapping
	L_mapTypeRead_RW(src, result);

	CEV_Capsule caps = {0, 0, NULL};

	if(embed)
    {//tileset is embedded, going to fetch it

        CEV_capsuleTypeRead_RW(src, &caps);

        if(!IS_PIC(caps.type))
        {
            fprintf(stderr, "Err at %s / %d : Capsule content is not picture.\n", __FUNCTION__, __LINE__ );
            goto err;
        }

        SDL_Texture *tex = CEV_capsuleExtract(&caps, true);

        if IS_NULL(tex)
        {
            fprintf(stderr, "Err at %s / %d : Could not extract texture from capsule.\n", __FUNCTION__, __LINE__ );
            goto err;
        }

        CEV_mapAttachTexture(result, tex);
    }

	//close RWops if requested
	if(freeSrc)
		SDL_RWclose(src);

    CEV_capsuleClear(&caps);

	return result;

err:
    CEV_capsuleClear(&caps);
    CEV_mapDestroy(result);

    return NULL;
}


CEV_TileMap* CEV_mapCreate(int layer, int width, int height, int tilePix)
{//creates map structure

    /*---PRL---*/

	if (layer<=0 || width<=0 || height<=0 || tilePix<=0)
	{//arg control

		char arg = 0;

        if(layer<=0)
            arg = 1;
		else if(width<=0)
			arg = 2;
		else if (height<=0)
			arg = 3;
		else if (tilePix<=0)
			arg = 4;

		fprintf(stderr, "Err at %s / %d : argument %d at function call is unvalid.\n", __FUNCTION__, __LINE__, arg);
		return NULL;
	}

    /*---EXECUTION---*/

	//allocating structure
	CEV_TileMap* result = calloc(1, sizeof(*result));

	if IS_NULL(result)
	{//on error
		fprintf(stderr, "Err at %s / %d : unable to allocate memory : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
		return NULL;
	}

	//attributting members default values
	result->numOfLayer              = layer;
	result->picId                   = 0;
	result->firstCall               = true;
	result->xScroll                 = false;
	result->yScroll                 = false;
	result->dim.world.tiles         = (SDL_Rect){.x = 0, .y = 0, .w = width, .h = height};
	result->dim.world.pixels        = (SDL_Rect){.x = 0, .y = 0, .w = width*tilePix, .h = height*tilePix};
	result->tileSize 	            = tilePix;
	result->dim.tileSet.tiles       = CLEAR_RECT;
	result->dim.tileSet.pixels      = CLEAR_RECT;
	result->pic                     = NULL;
	result->dim.dispOffset.pixels   = CLEAR_RECT;

	//allocating tilemap content matrix
	result->tileMap = (MAP_Tile***)CEV_allocate3d(layer, width, height, sizeof(MAP_Tile));

	if IS_NULL(result->tileMap)
	{//on error
		fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
		goto err_1;
	}

	for(int lay=0; lay<layer; lay++)
        for(int x=0; x<width; x++)
            for(int y=0; y<height; y++)
                result->tileMap[lay][x][y] = CEV_mapTileClear();

    //allocating properties matrix
	result->tileProps = (MAP_TileProps**)CEV_allocate2d(width, height, sizeof(MAP_TileProps));

	if IS_NULL(result->tileProps)
	{//on error
		fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
		goto err_2;
	}

	for(int i=0; i<width; i++)
        for(int j=0; j<height; j++)
        result->tileProps[i][j] = (MAP_TileProps){-1, -1, -1};

    return result;

//error exit
err_2:

    for(int i=0; i<layer; i++)
    {
        for(int j=0; j<width; j++)
            free(result->tileMap[i][j]);

        free(result->tileMap[i]);
    }
    free(result->tileMap);

err_1:
	free(result);

	return NULL;
}


void CEV_mapDestroy(CEV_TileMap *map)
{//frees content and structur

    if IS_NULL(map)
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        return;
    }

    CEV_mapClear(map);  //frees content
	free(map);	        //frees itself
}


void CEV_mapClear(CEV_TileMap *map)
{//frees TileMap content

    if IS_NULL(map)
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        return;
    }

    //frees tilemap content
    for (unsigned i=0; i < map->numOfLayer; i++)
    {//for every layer
        for (int j=0; j<map->dim.world.tiles.w; j++)
            free(map->tileMap[i][j]); //for every column

        free(map->tileMap[i]);//free column
    }
    free(map->tileMap);//free map


    for (int i=0; i<map->dim.world.tiles.w; i++)
        free(map->tileProps[i]);

    free(map->tileProps);

	SDL_DestroyTexture(map->pic);
}


bool CEV_mapAttachTexture(CEV_TileMap* map, SDL_Texture* texture)
{//associates texture containing tiles and sets clips

    if(IS_NULL(map) || IS_NULL(texture))
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        return false;
    }

	map->pic = texture;

	SDL_QueryTexture(texture,
                        NULL,
                        NULL,
                        &map->dim.tileSet.pixels.w,
                        &map->dim.tileSet.pixels.h);

	map->dim.tileSet.tiles.w  = map->dim.tileSet.pixels.w / map->tileSize;
	map->dim.tileSet.tiles.h  = map->dim.tileSet.pixels.h / map->tileSize;

    for(int i=0; i<map->numOfLayer; i++)
    {
        for (int x=0; x<map->dim.world.tiles.w; x++)
        {
            for(int y=0; y<map->dim.world.tiles.h; y++)
            {
                if(map->tileMap[i][x][y].index >=0)
                {
                    map->tileMap[i][x][y].clip.h = map->tileMap[i][x][y].clip.w = (int)map->tileSize;
                    map->tileMap[i][x][y].clip.x = (map->tileMap[i][x][y].index % map->dim.tileSet.tiles.w) * map->tileSize;
                    map->tileMap[i][x][y].clip.y = (map->tileMap[i][x][y].index / map->dim.tileSet.tiles.w) * map->tileSize;

                    if(map->tileMap[i][x][y].isAnim)
                    {
                        map->tileMap[i][x][y].anim.clip = map->tileMap[i][x][y].clip;
                    }
                }
            }
        }
    }

    return true;
}


void CEV_mapDraw(CEV_TileMap *src, SDL_Renderer* dst, int dispX, int dispY, int layer)
{//draws map onto SDL_Renderer

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        return;
    }

    if(layer >= src->numOfLayer)
    {
        fprintf(stderr, "Err at %s / %d : layer index (%d) is not available.\n ", __FUNCTION__, __LINE__, layer);
        return;
    }

    if(src->firstCall)
    {
        CEV_mapNeedScroll(src, dst);
        src->firstCall = false;
    }

    //keeping values in range
    CEV_constraint(0, &dispX, src->dim.world.pixels.w - src->dim.display.pixels.w -1);
    CEV_constraint(0, &dispY, src->dim.world.pixels.h - src->dim.display.pixels.h -1);

    //updating camera pos
    src->dim.display.pixels.x = dispX;
    src->dim.display.pixels.y = dispY;


    if (!src->xScroll)//centering if map smaller than display
        src->dim.dispOffset.pixels.x = (src->dim.display.pixels.w - src->dim.world.pixels.w)/2;
    else
        src->dim.dispOffset.pixels.x = -dispX;

    if (!src->yScroll)//centering if map smaller than display
        src->dim.dispOffset.pixels.y = (src->dim.display.pixels.h - src->dim.world.pixels.h)/2;
    else
        src->dim.dispOffset.pixels.y = -dispY;

    SDL_Rect inWorld = CEV_mapWhereInWorld(src, dispX, dispY);

    unsigned int now = SDL_GetTicks();

	for(int x = inWorld.x; x <= inWorld.x + inWorld.w; x++)
	{
	    for(int y = inWorld.y; y <= inWorld.y + inWorld.h; y++)
        {
            //where to draw tile
            SDL_Rect blitPos = CEV_mapBlitPos(src, x, y, dispX, dispY);
            SDL_Rect clipPos;

            //drawing
            if(layer<0)
            {
                for(unsigned i=0; i<src->numOfLayer; i++)
                {
                    if(src->tileMap[i][x][y].index>=0)
                    {
                        //where to pick tile
                        if(src->tileMap[i][x][y].isAnim)
                        {
                            L_mapAnimUpdate(&src->tileMap[i][x][y], now);
                            clipPos = src->tileMap[i][x][y].anim.clip;
                        }
                        else
                        {
                            clipPos = src->tileMap[i][x][y].clip;
                        }

                        CEV_rectConstraint(&clipPos, src->dim.tileSet.pixels);

                        SDL_RenderCopy(dst, src->pic, &clipPos, &blitPos);
                    }
                }
            }
            else
            {
                if(src->tileMap[layer][x][y].index>=0)
                {
                    //where to pick tile
                    if(src->tileMap[layer][x][y].isAnim)
                    {
                        L_mapAnimUpdate(&src->tileMap[layer][x][y], now);
                        clipPos = src->tileMap[layer][x][y].anim.clip;
                    }
                    else
                    {
                        clipPos = src->tileMap[layer][x][y].clip;
                    }

                    CEV_rectConstraint(&clipPos, src->dim.tileSet.pixels);

                    SDL_RenderCopy(dst, src->pic, &clipPos, &blitPos);
                }
            }
        }
	}
}


SDL_Rect CEV_mapBlitPos(CEV_TileMap *src, int tileX, int tileY, int dispX, int dispY)
{//calculate tile blit position

    if IS_NULL(src)
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        return CLEAR_RECT;
    }

    SDL_Rect result;

    /*int startBlitX = ((dispW - (src->dim.world.tiles.w * src->tileSize)) / 2),
        startBlitY = ((dispH - (src->mapHeight*src->tileSize)) / 2);*/

    //centering in x or offset
    if (src->xScroll)
        result.x = (tileX * src->tileSize) - dispX;
    else
        result.x = ((src->dim.display.pixels.w - src->dim.world.pixels.w) / 2) + (tileX * src->tileSize);

    //centering in y or offset
    if (src->yScroll)
        result.y = (tileY * src->tileSize) - dispY;
    else
        result.y = ((src->dim.display.pixels.h - src->dim.world.pixels.h) / 2) + (tileY * src->tileSize);

    result.w = result.h = (int)src->tileSize;

    return result;
}


SDL_Rect CEV_mapWorldDimPxl(CEV_TileMap *src)
{//world size in pixels

    if IS_NULL(src)
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        return CLEAR_RECT;
    }

    return src->dim.world.pixels;
}


SDL_Rect CEV_mapWhereInWorld(CEV_TileMap *src, int dispX, int dispY)
{//calculates tiles included in display


    if IS_NULL(src)
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        return CLEAR_RECT;
    }

    SDL_Rect result;
    bool addTile;

    if(src->xScroll)
    {//map.w > camera.w
        result.x = dispX / src->tileSize;
        addTile = ((dispX + src->dim.display.pixels.w) % src->tileSize);//camera on full tile ?
        result.w = src->dim.display.tiles.w + addTile;

        if((result.x + result.w) >= src->dim.world.tiles.w)//correction if off map
            result.w = src->dim.display.tiles.w;
    }
    else
    {
        result.x = 0;
        result.w = src->dim.world.tiles.w;
    }

    if(src->yScroll)
    {//map.h > camera.h
        result.y = dispY / src->tileSize;
        addTile = ((dispY + src->dim.display.pixels.h) % src->tileSize);//camera on full tile ?
        result.h = src->dim.display.tiles.h + addTile;

        if((result.y + result.h) >= src->dim.world.tiles.h)//correction if off map
            result.h = src->dim.display.tiles.h;
    }
    else
    {
        result.y = 0;
        result.h = src->dim.world.tiles.h;
    }

    return result;
}


bool CEV_mapNeedScroll(CEV_TileMap *src, SDL_Renderer* dst)
{//does the map needs scrolling or not

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        return false;
    }

    int dispW,
        dispH;

        SDL_RenderGetLogicalSize(dst, &dispW, &dispH);

        src->xScroll                = dispW < src->dim.world.pixels.w;
        src->yScroll                = dispH < src->dim.world.pixels.h;
        src->dim.display.pixels.w   = dispW;
        src->dim.display.pixels.h   = dispH;
        src->dim.display.tiles.w    = dispW / src->tileSize;
        src->dim.display.tiles.h    = dispH / src->tileSize;


    return src->xScroll || src->yScroll;
}


bool CEV_mapIsHardTile(CEV_TileMap *src, SDL_Point pos)
{//id that map pos hard ?

    if IS_NULL(src)
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        return false;
    }

    return src->tileMap[0][pos.x/src->tileSize][pos.y/src->tileSize].isHard;
}


int CEV_mapOptValue(CEV_TileMap *src, SDL_Point pos, int layer, int index)
{//fetches tile option value

    if IS_NULL(src)
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        return -1;
    }

    if ((pos.x < 0) || (pos.x >= src->dim.world.pixels.w))
        return ARG_ERR;

    else if ((pos.y < 0) || (pos.y >= src->dim.world.pixels.h))
        return ARG_ERR;

    else if (layer > (src->numOfLayer-1))
        return ARG_ERR;

    else if (index<0 || index>9)
        return ARG_ERR;

    else
        return src->tileMap[layer][pos.x / src->tileSize][pos.y / src->tileSize].option[index];
}


SDL_Point CEV_mapWorldPointToDisplayPoint(CEV_TileMap *src, SDL_Point pos)
{//position in world to display position

    if IS_NULL(src)
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        return CLEAR_POINT;
    }

    return (SDL_Point){pos.x + src->dim.dispOffset.pixels.x, pos.y + src->dim.dispOffset.pixels.y};
}


SDL_Rect CEV_mapWorldPointToDisplayTile(CEV_TileMap *src, SDL_Point pos)
{//tile rect under position in world relative to display

    if IS_NULL(src)
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        return CLEAR_RECT;
    }

    return (SDL_Rect){ pos.x -(pos.x%src->tileSize) + src->dim.dispOffset.pixels.x, //X
                       pos.y -(pos.y%src->tileSize) + src->dim.dispOffset.pixels.y, //Y
                       src->tileSize, //W
                       src->tileSize};//H
}


SDL_Rect CEV_mapWorldPointToWorldTile(CEV_TileMap *src, SDL_Point pos)
{//tile rect under position in world relative to world

    if IS_NULL(src)
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        return CLEAR_RECT;
    }

    SDL_Rect result;

    result.x = pos.x-(pos.x%src->tileSize);
    result.y = pos.y -(pos.y%src->tileSize);
    result.h = result.w = src->tileSize;

    return result;
}


SDL_Rect CEV_mapWorldRectToMatrixTile(CEV_TileMap *src, SDL_Rect pos)
{//rect pos in world to tile rect

    if IS_NULL(src)
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        return CLEAR_RECT;
    }

    SDL_Rect result;

    if (!src->xScroll)
    {
        result.x = 0,
        result.w = src->dim.world.tiles.w;
    }
    else
    {
        result.x = pos.x / src->tileSize;
        result.w = ((pos.x + pos.w - 1)/src->tileSize) - result.x + 1;
    }

    if(!src->yScroll)
    {
        result.y = 0,
        result.h = src->dim.world.tiles.h;
    }
    else
    {
        result.y = pos.y / src->tileSize;
        /*old
        result.w = ((pos.w) / src->tileSize) + (((pos.x % src->tileSize) + pos.w) > src->tileSize) + (pos.w < src->tileSize);
        result.h = ((pos.h) / src->tileSize) + (((pos.y % src->tileSize) + pos.h) > src->tileSize) + (pos.h < src->tileSize);
        */

        result.h = ((pos.y + pos.h - 1)/src->tileSize) - result.y + 1;
    }

    return result;
}


MAP_TileProps *CEV_mapTileProps(CEV_TileMap *src, int tileX, int tileY)
{//fetches tile prperties

    if IS_NULL(src)
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        return NULL;
    }

    if ((tileX < 0) || (tileX >= src->dim.world.tiles.w))
        return NULL;

    else if ((tileY < 0) || (tileY >= src->dim.world.tiles.h))
         return NULL;

    else
        return &src->tileProps[tileX][tileY];
}


MAP_Tile CEV_mapTileClear(void)
{//returns clear MAP_Tile

    MAP_Tile thisTile;
    L_tileInit(&thisTile);

    return thisTile;
}


MAP_TileAnim CEV_mapTileAnimClear(void)
{//returns clear MAP_TileAnim

    MAP_TileAnim thisAnim;

    CEV_memSet(&thisAnim, 0, sizeof(thisAnim));

    return thisAnim;
}


    /***LOCALS***/

static void L_mapAnimUpdate(MAP_Tile *tile, unsigned int now)
{

    if IS_NULL(tile)
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        return;
    }
    else if (!tile->anim.delayMs || !tile->anim.numOfFrame)
        return;

    unsigned int picAct = tile->anim.picAct;

    tile->anim.picAct = (((now/tile->anim.delayMs)% tile->anim.numOfFrame) + tile->anim.picStart)%tile->anim.numOfFrame;

    if(picAct != tile->anim.picAct)
    {
        tile->anim.clip.x = tile->clip.x +  tile->anim.picAct * tile->clip.w;
    }
}


static void L_mapTypeRead_RW(SDL_RWops* src, CEV_TileMap* dst)
{//reads tile mapping from Vfile

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        readWriteErr++;
        return;
    }

    //reading tiles matrix
    for (unsigned layer=0; layer<dst->numOfLayer; layer++)
    {
        for(int x=0; x<dst->dim.world.tiles.w; x++)
        {
            for(int y=0; y<dst->dim.world.tiles.h; y++)
            {
                L_tileTypeRead_RW(src, &dst->tileMap[layer][x][y]);
            }
        }
    }

    //reading profile matrix
    for(int x = 0; x<dst->dim.world.tiles.w; x++)
    {
        for(int y=0; y<dst->dim.world.tiles.h; y++)
        {
            dst->tileProps[x][y].type = SDL_ReadLE32(src);
        }
    }
}


static void L_mapTypeWrite(const CEV_TileMap* src, FILE* dst)
{//writes mapping into file

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        readWriteErr++;
        return;
    }

    //map parameters
    write_u32le(src->numOfLayer, dst);
    write_u32le(src->picId, dst);
	write_u32le(src->dim.world.tiles.w, dst);
	write_u32le(src->dim.world.tiles.h, dst);
	write_u32le(src->tileSize, dst);

    //tiles
    for(unsigned layer=0; layer<src->numOfLayer; layer++)
    {
        for(int x=0; x<src->dim.world.tiles.w; x++)
        {
            for(int y=0; y<src->dim.world.tiles.h; y++)
            {
                L_tileTypeWrite(&src->tileMap[layer][x][y], dst);
            }
        }
    }

    //profile
    for(int x = 0; x<src->dim.world.tiles.w; x++)
    {
        for(int y=0; y<src->dim.world.tiles.h; y++)
        {
            write_s32le(src->tileProps[x][y].type, dst);
        }
    }
}


static void L_tileTypeRead_RW(SDL_RWops* src, MAP_Tile* dst)
{//reads tile from Vfile

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        readWriteErr++;
        return;
    }

	dst->index  = SDL_ReadLE32(src);
	dst->isHard = SDL_ReadU8(src);
	dst->isAnim = SDL_ReadU8(src);

	if(dst->isAnim)
	{
        L_tileAnimTypeRead_RW(src, &dst->anim);
	}

	for(int i=0; i<TILE_NUM_OPTION; i++)
    {
        dst->option[i] 	= SDL_ReadLE32(src);
    }
}


static void L_tileTypeWrite(MAP_Tile* src, FILE* dst)
{//writes tile into file

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        readWriteErr++;
        return;
    }

	write_u32le(src->index, dst);
	write_u8(src->isHard, dst);
	write_u8(src->isAnim, dst);

	if(src->isAnim)
	{
        L_tileAnimTypeWrite(&src->anim, dst);
	}

	for(int i=0; i<TILE_NUM_OPTION; i++)
    {
      write_u32le(src->option[i], dst);
    }
}


static void L_tileAnimTypeRead_RW(SDL_RWops *src, MAP_TileAnim *dst)
{//read animation setting from file

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        readWriteErr++;
        return;
    }

    dst->numOfFrame = SDL_ReadLE32(src);
    dst->picStart   = SDL_ReadLE32(src);
    dst->picAct     = dst->picStart;
    dst->delayMs    = SDL_ReadLE32(src);
}


static void L_tileAnimTypeWrite(MAP_TileAnim *src, FILE *dst)
{//writes animation settings to file

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        readWriteErr++;
        return;
    }

    write_u32le(src->numOfFrame, dst);
    write_u32le(src->picStart, dst);
    write_u32le(src->delayMs, dst);
}


static void L_tileInit(MAP_Tile* tile)
{//tile default settings

    if IS_NULL(tile)
    {
        fprintf(stderr, "Err at %s / %d : Received NULL Arg.\n", __FUNCTION__, __LINE__ );
        return;
    }

    tile->index = -1;
    tile->isHard = false;
    tile->isAnim = false;

    tile->anim = CEV_mapTileAnimClear();

    for(int i=0; i<TILE_NUM_OPTION; i++)
        tile->option[i] = -1;
}

