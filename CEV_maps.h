//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    10-2017    |   0.0    |  creation/SDL2 **/
//**   CEV    |    01-2018    |   0.1    |  offset added  **/ //allows sprite blit correction
//**********************************************************/


/*
-----------------------------------------------------------------------------------------------
Fichier Data .map
pic embedded    u8
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

tileSet     CEV_Capsule
-----------------------------------------------------------------------------------------------
Layer index 0 is the farmost layer.
layers blit from top left to bottom right
*/

// TODO (drx#6#): insert CEV_Camera directly

#ifndef CEV_MAP_H_INCLUDED
#define CEV_MAP_H_INCLUDED

#define MAP_MAJOR_VERSION 1
#define MAP_MINOR_VERSION 1

#include <stdbool.h>
#include <SDL.h>


#define MAP_LAYER_ALL -1
#define TILE_NUM_OPTION 10

/*
Map file content :
u8  : tileSet embedded
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



/** \brief animated tile definition
 */
typedef struct MAP_TileAnim
{
    unsigned int
            numOfFrame, /**< number of tiles in animation */
            picStart,   /**< first tile index */
            delayMs,    /**< animation delay */
            picAct;     /**< active index */

    SDL_Rect clip;      /**< clip position */
}
MAP_TileAnim;


//not implemented
typedef struct MAP_TileProps
{
    uint32_t id, type;
    int value;
    //add any options structure here (warp, chest, objects) must contain char type as first member
}
MAP_TileProps;


/** \brief single tile definition
 */
typedef struct MAP_Tile
{//single tile definition

    int
        index;  /**< tile index in tile src pic from top left to bottom right */

    bool
        isHard,	/**< cannot walk through */
        isAnim; /**< tile is animated */

    MAP_TileAnim anim; /**< tile animation params if so */

    SDL_Rect clip;  /**< clip in tile src pic */

    int option[TILE_NUM_OPTION]; /**< option list */
}
MAP_Tile;


/** \brief tile map definition
 */
typedef struct CEV_TileMap
{//tile map definition

    bool
        xScroll,   /**< true if map wider than display */
        yScroll,   /**< true if map higher than display */
        firstCall; /**< map draw first call */

	int numOfLayer;   /**< number of layers */

    uint32_t
            picId,  /**< tile set index */
            tileSize;   /**< single tile size (pixels) */

    struct TileMapDim
    {
        struct TileMapDimMember
        {
            SDL_Rect pixels,/**< size in pxl */
                     tiles; /**< size in tiles */
        } world,    /**< world size */
        tileSet,    /**< tileset size */
        display,    /**< display size */
        dispOffset; /**< display offset size */

    } dim;

    MAP_Tile*** tileMap;        /**< tiles matrix */
    MAP_TileProps** tileProps;  /**< properties layer */
    SDL_Texture *pic;    /**< tiles texture */
}
CEV_TileMap;


/** \brief Dump CEV_TileMap content into stdout
 *
 * \param this : CEV_TileMap* to dump.
 *
 * \return void
 */
void CEV_mapDump(CEV_TileMap* this);

/** \brief Load mapping from file.
 *
 * \param fileName : name of file to be loaded.
 *
 * \return CEV_TileMap* on success, NULL on error.
 */
CEV_TileMap* CEV_mapLoad(const char* fileName);


/** \brief Load mapping from SDL_RWops filesystem.
 *
 * \param vfile : SDL_RWops ptr to read from.
 * \param freeSrc : Let decide whether close SDL_RWops or not.
 *
 * \return CEV_TileMap* on success, NULL on error.
 *
 */
CEV_TileMap* CEV_mapLoad_RW(SDL_RWops* vfile, char freeSrc);


/** \brief saves map into file.
 *
 * \param map : CEV_TileMap ptr to structure to save.
 * \param fileName : file name to save into.
 *
 * \return any CEV standard.
 *
 * \note Picture is embedded depending on id value : Saved if 0.
 *
 */
int CEV_mapSave(CEV_TileMap* src, const char* dstFileName);


/** \brief tile map creation
 *
 * \param layers : number of layers in map.
 * \param width : number of tiles.
 * \param height : number of tiles.
 * \param tilePix : tile size in pixel (16,32,64).
 *
 * \return Return a tile map pointer or NULL on failure.
 */
CEV_TileMap* CEV_mapCreate(int layers, int width, int height, int tilePix);


/** \brief free and destroy allocated CEV_TileMap structure.
 *
 * \param map : CEV_TileMap ptr to free.
 *
 * \return N/A.
 */
void CEV_mapDestroy(CEV_TileMap *map);


/** \brief frees CEV_TileMap structure content.
 *
 * \param map : CEV_TileMap ptr to clear.
 *
 * \return N/A.
 */
void CEV_mapClear(CEV_TileMap *map);

/**\brief associate texture containing tiles to a CEV_TileMap.
 *
 * \param  map : CEV_TileMap ptr to associate texture with.
 * \param  texture : SDL_Texture ptr to assiciate map with.
 *
 * \return true if texture is succesfully applied.
 */
bool CEV_mapAttachTexture(CEV_TileMap* map, SDL_Texture *texture);


/** \brief Draws map.
 *
 * \param src : CEV_TileMap ptr to be drawn.
 * \param dst : SDL_Renderer to draw onto.
 * \param dispX : Left position of map drawing (camera position).
 * \param dispY : Top position of map drawing (camera position).
 * \param layer : layer to display, negatif value show them all.
 *
 * \return N/A.
 */
void CEV_mapDraw(CEV_TileMap *src, SDL_Renderer* dst, int dispX, int dispY, int layer);


/** \brief world dimensions in pixel.
 *
 * \param src : CEV_TileMap* to get dimensions from (pixels).
 *
 * \return SDL_Rect.
 */
SDL_Rect CEV_mapWorldDimPxl(CEV_TileMap *src);


/** \brief calculate tiles displayed.
 *
 * \param src : CEV_TileMap ptr to be drawn.
 * \param dispX : X position of display in world (pixels).
 * \param dispY : Y position of display in world (pixels).
 *
 * \return SDL_Rect filled with positions and dimensions (tiles).
 */
 SDL_Rect CEV_mapWhereInWorld(CEV_TileMap *src, int dispX, int dispY);


/** \brief Detects if map needs scrolling.
 *
 * \param src : CEV_TileMap ptr to check.
 * \param dst : SDL_Renderer to draw onto.

 * \return true if map is too big for renderer.
 */
bool CEV_mapNeedScroll(CEV_TileMap *src, SDL_Renderer* dst);


/** \brief tile position on display.
 *
 * \param  src : CEV_TileMap ptr to check.
 * \param  tileX : tile X position (tile).
 * \param  tileY : tile Y position (tile).
 * \param  dispX : left position of display in map (pixel).
 * \param  dispY : top position of display in map (pixel).
 *
 * \return SDL_Rect as position an dimensions of tile(x,y) on display (pixel).
 */
SDL_Rect CEV_mapBlitPos(CEV_TileMap *src, int tileX, int tileY, int dispX, int dispY);


/** \brief tile hardness status.
 *
 * \param  src : CEV_TileMap ptr to check.
 * \param  pos : position to check (in pixel).
 *
 * \return true if pos is on a hard tile.
 */
bool CEV_mapIsHardTile(CEV_TileMap *src, SDL_Point pos);


/** \brief fetch tile parameter value.
 *
 * \param  src : CEV_TileMap ptr to check.
 * \param  pos : position in world to check (pixels).
 * \param  layer : which layer to check (pixels).
 * \param  index : which option to check (0-9).
 *
 * \return Requested parameter value.
 */
int CEV_mapOptValue(CEV_TileMap *src, SDL_Point pos, int layer, int index);


/** \brief fetch tile properties.
 *
 * \param  src : CEV_TileMap* to fetch tile from.
 * \param  tileX : X position of tile in tile matrix.
 * \param  tileY : Y position of tile in tile matrix.
 *
 * \return MAP_TileProps* to requested prperties.
 */
MAP_TileProps *CEV_mapTileProps(CEV_TileMap *src, int tileX, int tileY);


/**Landmark translation**/

/**from world**/

/** \brief world point to display point.
 *
 * \param  src : CEV_TileMap ptr to check.
 * \param  pos : point position in map (pixel).
 *
 * \return SDL_Point as 'pos' position on screen (pixel).
 */
SDL_Point CEV_mapWorldPointToDisplayPoint(CEV_TileMap *src, SDL_Point pos);


/** \brief world point to tile's world pos & dim.
 *
 * \param  src : CEV_TileMap ptr to check.
 * \param  pos : point position in map (pixel).
 * \return SDL_Rect as world position of tile under 'pos'.
 */
SDL_Rect CEV_mapWorldPointToWorldTile(CEV_TileMap *src, SDL_Point pos);


/** \brief world pos & dim to tiles matrix pos & dim.
 *
 * \param  src : CEV_TileMap ptr to check.
 * \param  pos : rect position in map (pixel).
 *
 * \return SDL_Rect as tiles position in tile matrix.
 */
SDL_Rect CEV_mapWorldRectToMatrixTile(CEV_TileMap *src, SDL_Rect pos);


/** \brief world point to tile display pos & dim.
 *
 * \param  src : CEV_TileMap ptr to check.
 * \param  pos : point position in map (pixel).
 *
 * \return SDL_Rect as display position of tile under 'pos'.
 */
SDL_Rect CEV_mapWorldPointToDisplayTile(CEV_TileMap *src, SDL_Point pos);


/** \brief provides a clear 0'ed single tile.
 *
 * \param void
 *
 * \return MAP_Tile as clear tile.
 */
MAP_Tile CEV_mapTileClear(void);


/** \brief provides a clear O'ed tile animation instance.
 *
 * \param void
 *
 * \return MAP_TileAnim as clear tile animation.
 */
MAP_TileAnim CEV_mapTileAnimClear(void);

#endif // CEV_MAP_H_INCLUDED
