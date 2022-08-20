//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    10-2017    |   0.0    |  creation/SDL2 **/
//**   CEV    |    01-2018    |   0.1    |  offset added  **/ //allows sprite blit correction
//**********************************************************/


/*
-----------------------------------------------------------------------------------------------
Fichier Data .map
version major   u8
version minor   u8
num of layers   uint32
tileSet ID      uint32
tile width      uint32
tile height     uint32
tile size pix   uint32

per num of layer * width * height : single tile definition :
{
    index       uint32
    hardness    u8
    is anim     u8

    if is anim :
    {
        pic num     uint
        picStart    uint
        picAct      uint
        delay       uint
    }
    option value    10 * int
}

profile layer width*height :
type s32;

tileSet     CEV_Capsule //to be implemented
-----------------------------------------------------------------------------------------------
Layer index 0 is the farmost layer.
layers blit from top left to bottom right
*/

// TODO (drx#1#): insert CEV_Camera directly

#ifndef CEV_TILES_H_INCLUDED
#define CEV_TILES_H_INCLUDED

#define MAP_MAJOR_VERSION 1
#define MAP_MINOR_VERSION 1

#include <stdbool.h>
#include <SDL.h>


#define MAP_LAYER_ALL -1
#define TILE_NUM_OPTION 10

/*
Map file content :
u32 : num of layers
    : tileset ID
    : num of tiles width
    : num of tiles height
    : single tile size (w==h)

per layer 0 ->
    for 0->width
        for 0->height
            u32 : tileIndex
            u8  : is hard tile
                : is anim

                if anim :
                    u32 : num of pic
                        : pic start
                        : delay (ms)

            for TILE_NUM_OPTION
                u32 : option value

for 0->width
        for 0->height
            u32 : tile type
*/
typedef struct MAP_TileAnim
{
    unsigned int
            picNum,
            picStart,
            delayMs,
            picAct;

    SDL_Rect clip;
}
MAP_TileAnim;


//not implemented
typedef struct MAP_TileProps
{
    char type;
    int value;
    //add any options structure here (warp, chest, objects) must contain char type as first member
}
MAP_TileProps;


typedef struct MAP_Tile
{//single tile definition

    int
        index;  //tile index in tile src pic from top left to bottom right

    bool
        isHard,	//cannot walk through
        isAnim; //tile is animated

    MAP_TileAnim anim;

    SDL_Rect clip;  //clip in tile src pic

    int option[TILE_NUM_OPTION];
}
MAP_Tile;


typedef struct CEV_TileMap
{//tile map definition

    bool
        xScroll,   //true if map wider than display
        yScroll,   //true if map higher than display
        firstCall; //map draw first call

    int  numLayer;
       //number of layers
    uint32_t
            tileSetId,  //tile set index
            tileSize;   //tile size (pixels)

    struct
    {
        struct
        {
            SDL_Rect pixels,//size in pxl
                     tiles; //size in tiles
        } world,    //world size
        tileSet,    //tileset size
        display,    //display size
        dispOffset; //display offset size

    } dim;

    MAP_Tile*** tileMap;        //tiles matrix
    MAP_TileProps** tileProps;  //properties layer
    SDL_Texture *tileSetPic;    //tiles texture
}
CEV_TileMap;


/**
 *  \brief Load mapping from file
 *
 *  \param [in] fileName : name of file to be loaded
 *  \return ptr on CEV_TileMap on success, NULL on error
 *
 *  \details More details
 */
CEV_TileMap* CEV_mapLoad(const char* fileName);


/**
 *  \brief Load mapping from SDL_RWops filesystem
 *
 *  \param [in] vfile : SDL_RWops ptr to read from
 *  \param [in] freeSrc : Let decide whether close SDL_RWops or not.
 *  \return Return ptr on CEV_TileMap on success, NULL on error
 *
 *  \details More details
 */
CEV_TileMap* CEV_mapLoad_RW(SDL_RWops* vfile, char freeSrc);


/**
 *  \brief saves map into file
 *
 *  \param [in] map : CEV_TileMap ptr to structure to save
 *  \param [in] fileName : file name to save into
 *  \return any CEV standard.
 *
 *  \details More details
 */
int CEV_mapSave(const CEV_TileMap* map, const char* fileName);


/**
 *  \brief tile map creation
 *
 *  \param [in] layer : number of layers in map.
 *  \param [in] width : number of tiles.
 *  \param [in] height : number of tiles.
 *  \param [in] tilePix : tile size in pixel (16,32,64).
 *  \return Return a tile map pointer or NULL on failure.
 *
 *  \details More details
 */
CEV_TileMap* CEV_mapCreate(int layers, int width, int height, int tilePix);


/**
 *  \brief free and destroy allocated CEV_TileMap structure
 *
 *  \param [in] map : CEV_TileMap ptr to free
 *
 *  \return N/A
 *
 *  \details More details
 */
void CEV_mapDestroy(CEV_TileMap *map);


/**
 *  \brief frees CEV_TileMap structure content
 *
 *  \param [in] map : CEV_TileMap ptr to clear
 *
 *  \return N/A
 *
 *  \details More details
 */
void CEV_mapClear(CEV_TileMap *map);

/**
 *  \brief associate texture containing tiles to a CEV_TileMap
 *
 *  \param [in] map : CEV_TileMap ptr to associate texture with
 *  \param [in] texture : SDL_Texture ptr to assiciate map with
 *
 *  \return true if texture is succesfully applied.
 *
 *  \details More details
 */
bool CEV_mapTilesTextureSet(CEV_TileMap* map, SDL_Texture *texture);


/**
 *  \brief Draws map
 *
 *  \param [in] map : CEV_TileMap ptr to be drawn
 *  \param [in] dst : SDL_Renderer to draw onto
 *  \param [in] x : Left position of map drawing
 *  \param [in] y : Top position of map drawing
 *  \param [in] layer : layer to display, negatif value show them all.
 *
 *  \return N/A
 *
 *  \details More details
 */
void CEV_mapDraw(CEV_TileMap *src, SDL_Renderer* dst, int dispX, int dispY, int layer);


/** \brief world dimensions in pixel
 *
 * \param src : CEV_TileMap* to get dimensions from (pixels).
 *
 * \return SDL_Rect
 */
SDL_Rect CEV_mapWorldDimPxl(CEV_TileMap *src);


/** \brief calculate tiles displayed
 *
 * \param [in] src : CEV_TileMap ptr to be drawn
 * \param [in] dispX : X position of display in world (pixels)
 * \param [in] dispY : Y position of display in world (pixels)
 *
 * \return SDL_Rect filled with positions and dimensions (tiles)
 */
 SDL_Rect CEV_mapWhereInWorld(CEV_TileMap *src, int dispX, int dispY);


/** \brief Detects if map needs scrolling
 *
 * \param [in] src : CEV_TileMap ptr to check
 * \param [in] dst : SDL_Renderer to draw onto

 * \return true if map is too big for renderer
 */
bool CEV_mapNeedScroll(CEV_TileMap *src, SDL_Renderer* dst);


/** \brief tile position on display
 *
 * \param [in] src : CEV_TileMap ptr to check
 * \param [in] tileX : tile X position (tile)
 * \param [in] tileY : tile Y position (tile)
 * \param [in] dispX : left position of display in map (pixel)
 * \param [in] dispY : top position of display in map (pixel)
 *
 * \return SDL_Rect as position an dimensions of tile(x,y) on display (pixel)
 */
SDL_Rect CEV_mapBlitPos(CEV_TileMap *src, int tileX, int tileY, int dispX, int dispY);


/** \brief tile hardness status
 *
 * \param [in] src : CEV_TileMap ptr to check
 * \param [in] pos : position to check (in pixel)
 *
 * \return true if pos is on a hard tile
 */
bool CEV_mapIsHardTile(CEV_TileMap *src, SDL_Point pos);


/** \brief fetch tile parameter value
 *
 * \param [in] src : CEV_TileMap ptr to check
 * \param [in] pos : position in world to check (pixels)
 * \param [in] index : which option to check (0-9)
 *
 * \return parameter value
 */
int CEV_mapOptValue(CEV_TileMap *src, SDL_Point pos, int layer, int index);


/** \brief fetch tile properties
 *
 * \param [in] src : CEV_TileMap* to fetch tile from
 * \param [in] tileX : X position of tile in tile matrix
 * \param [in] tileY : Y position of tile in tile matrix
 * \param [in] layer : layer index of tile
 *
 * \return MAP_Tile* to requested tile.
 */
MAP_Tile *CEV_mapTileProps(CEV_TileMap *src, int tileX, int tileY, unsigned int layer);


/**Landmark translation**/

/**from world**/

/** \brief world point to display point
 *
 * \param [in] src : CEV_TileMap ptr to check
 * \param [in] pos : point position in map (pixel)
 *
 * \return SDL_Point as 'pos' position on screen (pixel)
 */
SDL_Point CEV_mapWorldPointToDisplayPoint(CEV_TileMap *src, SDL_Point pos);


/** \brief world point to tile's world pos & dim
 *
 * \param [in] src : CEV_TileMap ptr to check
 * \param [in] pos : point position in map (pixel)
 * \return SDL_Rect as world position of tile under 'pos'
 *
 */
SDL_Rect CEV_mapWorldPointToWorldTile(CEV_TileMap *src, SDL_Point pos);


/** \brief world pos & dim to tiles matrix pos & dim
 *
 * \param [in] src : CEV_TileMap ptr to check
 * \param [in] pos : rect position in map (pixel)
 *
 * \return SDL_Rect as tiles position in tile matrix
 */
SDL_Rect CEV_mapWorldRectToMatrixTile(CEV_TileMap *src, SDL_Rect pos);


/** \brief world point to tile display pos & dim
 *
 * \param [in] src : CEV_TileMap ptr to check
 * \param [in] pos : point position in map (pixel)
 *
 * \return SDL_Rect as display position of tile under 'pos'
 */
SDL_Rect CEV_mapWorldPointToDisplayTile(CEV_TileMap *src, SDL_Point pos);


MAP_Tile CEV_mapTileClear(void);

MAP_TileAnim CEV_mapTileAnimClear(void);

#endif // CEV_TILES_H_INCLUDED
