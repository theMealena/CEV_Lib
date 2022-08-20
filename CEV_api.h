//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |    05-2016    |   2.0    |    SDL2 rev    **/
//**   CEV    |    05-2019    |   2.1    |functions added **/
//**   CEV    |    04-2020    |   2.1    |    revision    **/
//**********************************************************/


#ifndef CEV_API_H_INCLUDED
#define CEV_API_H_INCLUDED

#include <SDL.h>
#include <stdbool.h>

#define API_MS (CLOCKS_PER_SEC/1000)

#define RING_BELL putchar(7)

#ifndef PI
 #define PI 3.14159265
#endif //PI

#ifndef BIT
 #define BIT(x) (1<<(x))
#endif // BIT

#ifndef MAX
 #define MAX(x,y) (((x)>(y))? (x) : (y))
#endif // MAX

#ifndef MIN
 #define MIN(x,y) (((x)<(y))? (x) : (y))
#endif // MIN


#ifndef IS_NULL
 #define IS_NULL(x) (NULL == (x))
#endif // IS_NULL

#ifndef NOT_NULL
 #define NOT_NULL(x) (NULL != (x))
#endif // NOT_NULL




#define CEV_X 0
#define CEV_Y 1
#define CEV_Z 2


#ifdef __cplusplus
extern "C" {
#endif

enum {INC = 0, DEC = 1};

enum {FATAL = -4, QUIT = -3, ARG_ERR = -2, FUNC_ERR = -1, FUNC_OK = 0};


/** \brief integer 3D coords.
 */
typedef struct CEV_ICoord
{/*integer coords**/
    int     x,  /**< horizontal position */
            y,  /**< vertical position */
            z;  /**< depth position */
}
CEV_ICoord;


/** \brief float 3D coords
 */
typedef struct CEV_FCoord
{/*float coords**/
    float   x,  /**< horizontal position */
            y,  /**< vertical position */
            z;  /**< depth position */
}
CEV_FCoord;


/** \brief edge structure
 */
typedef struct CEV_Edge
{/*edge structure**/
    uint8_t value,  /**< process value to be tested */
            memo,   /**< value memo / internal */
            re,     /**< rising edge has occured */
            fe,     /**< falling edge has occured */
            any,    /**< any change in value */
            out;    /**< functions result / compatibiliy */
}
CEV_Edge;

#define CLEAR_POINT     (SDL_Point){.x = 0, .y = 0}
#define CLEAR_ICOORD    (CEV_ICoord){.x = 0, .y = 0, .z = 0}
#define CLEAR_FCOORD    (CEV_FCoord){.x = 0.0, .y = 0.0, .z = 0.0}
#define CLEAR_RECT      (SDL_Rect){.x = 0, .y = 0, .w = 0, .h = 0}
#define CLEAR_EDGE      (CEV_Edge){.value = false, .memo = false, .re = false, .fe = false, .any = false .out = false}


/**miscellanous**/


/** \brief extracts bitIndex from flag.
 *
 * \param flag : int to extract bit from.
 * \param bitIndex : bit index to extract.
 *
 * \return bit status.
 */
bool CEV_bitGet(int flag, int bitIndex);


/** \brief sets bit (1) in flag.
 *
 * \param flag : int* to set bit into.
 * \param bitIndex : bit index to set.
 *
 * \return N/A.
 */
void CEV_bitSet(int* flag, int bitIndex);


/** \brief reset bit (0) in flag.
 *
 * \param flag : int* to clear bit into.
 * \param bitIndex : bit index to clear.
 *
 * \return N/A.
 */
void CEV_bitClear(int* flag, int bitIndex);


/** \brief Rising edge.
 *
 * \param edge : CEV_Edge* to test.
 *
 * \return true if rising edge occured, false otherwise.
 */
bool CEV_edgeRise(CEV_Edge *edge);


/** \brief Falling edge.
 *
 * \param edge : CEV_Edge* to test.
 *
 * \return true if falling edge occured, false otherwise.
 */
bool CEV_edgeFall(CEV_Edge *edge);


/** \brief Any edge.
 *
 * \param edge : CEV_Edge* to test.
 *
 * \return true if RE or FE occured, false otherwise.
 */
bool CEV_edgeAny(CEV_Edge *edge);


/** \brief Every edge / Update structure content.
 *
 * \param edge : CEV_Edge* to test.
 *
 * \return true on any edge.
 */
bool CEV_edgeUpdate(CEV_Edge *edge);


/** \brief int swap valOne <-> valTwo.
 *
 * \param valOne : int* to be swapped with valTwo*.
 * \param valTwo : int* to be swapped with valOne*.
 *
 * \return N/A.
 */
void CEV_intSwap(int* valOne, int* valTwo);


/** \brief swaps size bytes valOne<->valTwo.
 *
 * \param valOne : void* to content to be swapped with valTwo*.
 * \param valTwo : void* to content to be swapped with valOne*.
 * \param size : size_t of volume to be swapped.
 *
 * \return N/A.
 */
void CEV_anySwap(void* valOne, void* valTwo, size_t size);


/** \brief bit field copy.
 *
 * \param src : const void* to read from.
 * \param srcBit : start bit index to read from (low).
 * \param dst : void* to write into.
 * \param dstBit : start bit index to write at (low).
 * \param length : size_t number of bit to copy.
 *
 * \return void* on dst.
 *
 * \note undefined behaviour if src and dst are on same mem field.
 */
void* CEV_bitFieldDistribute(const void *src, int srcBit, void* dst, int dstBit, size_t length);


/** \brief Index from row as 2d matrix
 *
 * \param x : x coord in matrix.
 * \param y : y coord in matrix.
 * \param width : matrix single row width.
 *
 * \return int : linear index of (x,y).
 */
int CEV_2dCoordToRowIndex(int x, int y, int width);


/** \brief Index from row as 3d matrix
 *
 * \param x : x coord in matrix.
 * \param y : y coord in matrix.
 * \param z : z coord in matrix.
 * \param width : matrix single row width.
 * \param height : matrix single row height.
 *
 * \return int : linear index of (x,y,z).
 */
int CEV_3dCoordToRowIndex(int x, int y, int z, int width, int height);


/** \brief x,y from row as 2d matrix
 *
 * \param index : index in single row table.
 * \param width : matrix single row width.
 *
 * \return CEV_Icoord filled with (x,y).
 */
CEV_ICoord CEV_rowIndexTo2dCoord(int index, int width);


/** \brief x,y,z from row as 3d matrix
 *
 * \param index : index in single row table.
 * \param width : matrix single row width.
 * \param height : matrix single row height.
 *
 * \return CEV_Icoord filled with (x,y).
 */
CEV_ICoord CEV_rowIndexTo3dCoord(int index, int width, int height);

/** \brief inc/dec by 1 with modulo.
 *
 * \param mode : incremental method (INC/DEC).
 * \param val : int* to modify.
 * \param modulo : modulo to apply.
 *
 * \return true if modulo has been applied, false otherwise.
 */
bool CEV_addModulo(int mode, int *val, int modulo);


/** \brief inc/dec by 1 with limits.
 *
 * \param mode : incremental method (INC/DEC).
 * \param val : int* to modify.
 * \param num : number of value val can take [0; num[.
 *
 * \return true if val was inc/dec, false otherwise.
 */
bool CEV_addLimited(int mode, int *val, int num);


/** \brief memset implementation.
 *
 * \param ptr : mem field to modify.
 * \param value : value to apply.
 * \param size : size of mem field on which to apply modification.
 *
 * \return N/A.
 */
void CEV_memSet(void *ptr, char value, size_t size);


/** \brief memcopy imlpementation.
 *
 * \param src : mem field to read.
 * \param dst : mem field to write into.
 * \param count : number of bytes to be copied.
 *
 * \return N/A.
 */
void CEV_memCopy(void *src, void *dst, size_t count);


/** \brief Circular comparator.
 *
 * \param limInf : low limit.
 * \param val : value to test.
 * \param limSup : high limit.
 *
 * \return true if val belongs to [limInf, limSup] or [limSup, limInf].
 */
bool CEV_lim(int limInf, int val, int limSup);


/** \brief Value inc /dec.
 *
 * \param value : pointer to value to modify.
 * \param reach : value to reach.
 * \param by : step applied to value at each function call.
 *
 * \return true when "value" has reached "reach".
 */
bool CEV_reachValue(float* value, float reach, float by);


/** \brief Value corrected within [mini, maxi].
 *
 *
 * \param mini : minimum value can take.
 * \param value : value to test.
 * \param maxi : maximum value can take.
 *
 * \return true if value was corrected.
 */
bool CEV_constraint(int mini, int* value, int maxi);


/** \brief Value corrected within [mini, maxi].
 *
 * \param mini : minimum value can take.
 * \param value : value to test.
 * \param maxi : maximum value can take.
 *
 * \return 1 if value was corrected.
 */
bool CEV_fconstraint(float mini, float* value, float maxi);


/** \brief value scaling.
 *
 * \param realVal : value to test.
 * \param realMin : minimum range for realVal.
 * \param realMax : maximum range for realVal.
 * \param retMin : value returned for realVal=realMin.
 * \param retMax : value returned for realVal=realMax.
 *
 * \return scaled double value.
 */
double CEV_map(double realVal, double realMin, double realMax, double retMin, double retMax);


/** \brief random int value in [start, end[.
 *
 * \param start : minimum value.
 * \param end : maximum value.
 *
 * \return int
 */
int CEV_irand(int start, int end);


/** \brief random double value.
 *
 * \param start : minimum value.
 * \param end : maximum value.
 *
 * \return double
 */
double CEV_frand(double start, double end);


/** \brief floating point equality.
 *
 * \param value1 : first value to compare.
 * \param value2 : second value to compare.
 * \param tolerence : acceptable gap.
 *
 * \return true if value is within (vtarget +/- tolerence)
 */
bool CEV_floatIsEqual(double value1, double value2, double tolerence);


/** \brief double modulo.
 *
 * \param value : reference value.
 * \param mod : modulo to be applied.
 *
 * \return double as value%mod.
 */
double CEV_fModulo(double value, double mod);


/** \brief angle of vector.
 *
 * \param x := X vector.
 * \param y := Y vector.
 *
 * \return angle of ->(xy) in degres as double.
 */
double CEV_vectAngle(double x, double y);


/*** SDL_Point ***/

/** \brief same point.
 *
 * \param pta : SDL_Point.
 * \param ptb : SDL_Point.
 *
 * \return true if both point at same position.
 */
bool CEV_pointAreEqual(SDL_Point* pta, SDL_Point* ptb);


/** \brief cleared point.
 *
 * \return SDL_Point filled with 0.
 */
SDL_Point CEV_pointClear(void);


/** \brief CEV_Icoord to SDL_point.
 *
 * \param src : CEV_Icoord to convert.
 *
 * \return SDL_Point at same pos.
 */
SDL_Point CEV_icoordToPoint(CEV_ICoord src);


/** \brief CEV_Fcoord to SDL_point.
 *
 * \param src : CEV_Fcoord to convert.
 *
 * \return SDL_Point at same pos.
 */
SDL_Point CEV_fcoordToPoint(CEV_FCoord src);


/** \brief point as center of rect.
 *
 * \param rect : SDL_Rect to center point in.
 * \param point : SDL_Point* to correct.
 *
 * \return SDL_Point* on point.
 */
SDL_Point* CEV_rectMidToPoint(SDL_Rect rect, SDL_Point *point);


/** \brief rect to point (x,y) copy.
 *
 * \param src : SDL_Rect to convert.
 *
 * \return SDL_Point.
 */
SDL_Point CEV_rectPosToPoint(SDL_Rect src);


/** \brief median between SDL_Points
 *
 * \param pta : SDL_Point
 * \param ptb : SDL_Point
 *
 * \return SDL_Point as median point.
 */
SDL_Point CEV_pointMedian(SDL_Point pta, SDL_Point ptb);


/** \brief point in rect.
 *
 * \param point : SDL_Point to test.
 * \param rect : SDL_Rect to test.
 *
 * \return true if point is within rect.
 */
bool CEV_pointIsInRect(SDL_Point point, SDL_Rect rect);


/** \brief angle from pta to ptb.
 *
 * \param pta SDL_Point as "center".
 * \param ptb SDL_Point as angle.
 *
 * \return angle of pta->ptb in degres as double.
 */
double CEV_pointAngle(SDL_Point pta, SDL_Point ptb);


/** \brief Distance between 2 SDL_Point.
 *
 * \param pta : SDL_Point.
 * \param ptb : SDL_Point.
 *
 * \return distance between pta and ptb as double.
 */
double CEV_pointDist(SDL_Point pta, SDL_Point ptb);


/** \brief sums 2 SDL_Point.
 *
 * \param pta : SDL_Point.
 * \param ptb : SDL_Point.
 *
 * \return SDL_point filled with sum.
 */
SDL_Point CEV_pointSum(SDL_Point pta, SDL_Point ptb);


/** CEV_ICOORD **/

/** \brief same point.
 *
 * \param pta : CEV_ICoord.
 * \param ptb : CEV_ICoord.
 *
 * \return true if both point at same position.
 */
bool CEV_IcoordAreEqual(CEV_ICoord* pta, CEV_ICoord* ptb);

/** \brief empty Icoord.
 *
 * \param N/A.
 *
 * \return CEV_ICoord.
 */
CEV_ICoord CEV_icoordClear(void);


/** \brief SDL_Point to Icoord.
 *
 * \param src : SDL_Point to convert.
 *
 * \return CEV_ICoord.
 */
CEV_ICoord CEV_pointToIcoord(SDL_Point src);


/** \brief Fcoord to Icoord.
 *
 * \param src : Fcoord to convert.
 *
 * \return CEV_ICoord.
 */
CEV_ICoord CEV_fcoordToIcoord(CEV_FCoord src);


/** \brief Icoord as center of rect.
 *
 * \param src : Rect to center in.
 * \param dst : Icoord ptr to center.
 *
 * \return ptr on dst.
 *
 * \note z component is untouched.
 */
CEV_ICoord* CEV_rectMidToIcoord(SDL_Rect src, CEV_ICoord *dst );


/** \brief rect to Icoord (x,y) copy.
 *
 * \param src : SDL_Rect to convert.
 *
 * \return Icoord.
 */
CEV_ICoord CEV_rectPosToIcoord(SDL_Rect src);


/** \brief median between Icoord
 *
 * \param pta : Icoord
 * \param ptb : Icoord
 *
 * \return Icoord as median point.
 */
CEV_ICoord CEV_icoordMedian(CEV_ICoord pta, CEV_ICoord ptb);


/** \brief Icoord in rect.
 *
 * \param point : Icoord to test.
 * \param rect  : SDL_Rect to test.
 *
 * \return true if point is within rect.
 */
bool CEV_icoordIsInRect(CEV_ICoord point, SDL_Rect rect);


/** \brief Distance between 2 Icoord.
 *
 * \param pta : Icoord.
 * \param ptb : Icoord.
 *
 * \return distance between pta and ptb as double.
 *
 * \note z included.
 */
double CEV_icoordDist(CEV_ICoord pta, CEV_ICoord ptb);


/** \brief angle from pta to ptb.
 *
 * \param pta Icoord as "center".
 * \param ptb Icoord as angle.
 *
 * \return angle of pta->ptb in degres as double.
 *
 * \note : 2d: x,y computed only.
 */
double CEV_icoordAngle2d(CEV_ICoord pta, CEV_ICoord ptb);



/** \brief sums 2 Icoord.
 *
 * \param pta : Icoord.
 * \param ptb : Icoord.
 *
 * \return Icoord filled with sum.
 */
CEV_ICoord CEV_icoordSum(CEV_ICoord pta, CEV_ICoord ptb);





 /** CEV_FCOORD **/

 /** \brief same point.
 *
 * \param pta : CEV_FCoord.
 * \param ptb : CEV_FCoord.
 * \param tolerance : error acceptance.
 *
 * \return true if both point at same position within tolerance.
 */
 bool CEV_fcoordAreEqual(CEV_FCoord* pta, CEV_FCoord* ptb, double tolerance);


/** \brief empty Fcoord.
 *
 * \param N/A.
 *
 * \return CEV_FCoord.
 */
CEV_FCoord CEV_fcoordClear(void);


/** \brief SDL_Point to Fcoord.
 *
 * \param src : SDL_Point to convert.
 *
 * \return CEV_FCoord.
 */
CEV_FCoord CEV_pointToFcoord(SDL_Point src);


/** \brief Icoord to Fcoord.
 *
 * \param src : Icoord to convert.
 *
 * \return CEV_FCoord.
 */
CEV_FCoord CEV_icoordToFcoord(CEV_ICoord src);


/** \brief Fcoord as center of rect.
 *
 * \param src : Rect to center in.
 * \param dst : Fcoord ptr to center.
 *
 * \return ptr on dst.
 *
 * \note z component is untouched.
 */
CEV_FCoord* CEV_rectMidToFcoord(SDL_Rect src, CEV_FCoord *dst);


/** \brief rect to Fcoord (x,y) copy.
 *
 * \param src : SDL_Rect to convert.
 *
 * \return Fcoord.
 */
CEV_FCoord CEV_rectPosToFcoord(SDL_Rect src);


/** \brief median between Fcoord
 *
 * \param pta : Fcoord
 * \param ptb : Fcoord
 *
 * \return Fcoord as median point.
 */
CEV_FCoord CEV_fcoordMedian(CEV_FCoord pta, CEV_FCoord ptb);


/** \brief Fcoord in rect.
 *
 * \param point : Fcoord to test.
 * \param rect  : SDL_Rect to test.
 *
 * \return true if point is within rect.
 */
bool CEV_fcoordIsInRect(CEV_FCoord point, SDL_Rect rect);


/** \brief Distance between 2 Fcoord.
 *
 * \param pta : Fcoord.
 * \param ptb : Fcoord.
 *
 * \return distance between pta and ptb as double.
 *
 * \note z included.
 */
double CEV_fcoordDist(CEV_FCoord pta, CEV_FCoord ptb);


/** \brief angle from pta to ptb.
 *
 * \param pta Fcoord as "center".
 * \param ptb Fcoord as angle.
 *
 * \return angle of pta->ptb in degres as double.
 *
 * \note : 2d: x,y computed only.
 */
double CEV_fcoordAngle2d(CEV_FCoord pta, CEV_FCoord ptb);


/** \brief sums 2 Fcoord.
 *
 * \param pta : Fcoord.
 * \param ptb : Fcoord.
 *
 * \return Fcoord filled with sum.
 */
CEV_FCoord CEV_fcoordSum(CEV_FCoord pta, CEV_FCoord ptb);



/*** SDL_Rect ***/


/** \brief rect corrected around point as gravity center.
 *
 * \param src : SDL_Point to center rect on.
 * \param dst : SDL_Rect* to be corrected.
 *
 * \return rect.
 */
SDL_Rect* CEV_rectAroundPoint(SDL_Point src, SDL_Rect *dst);


/** \brief rect corrected around point as gravity center.
 *
 * \param src : CEV_ICoord point to center rect on.
 * \param dst : SDL_Rect* to be corrected.
 *
 * \return rect.
 */
SDL_Rect* CEV_rectAroundIcoord(CEV_ICoord src, SDL_Rect* dst);


/** \brief rect corrected around point as gravity center.
 *
 * \param src : CEV_FCoord point to center rect on.
 * \param dst : SDL_Rect* to be corrected.
 *
 * \return rect.
 */
SDL_Rect* CEV_rectAroundFcoord(CEV_FCoord src, SDL_Rect* dst);


/** \brief Rect pos from point.
 *
 * \param src : point to place rect at.
 * \param dst : destination *Rect.
 *
 * \return ptr on dst;
 */
SDL_Rect* CEV_pointToRectPos(SDL_Point src, SDL_Rect* dst);


/** \brief Rect pos from Icoord.
 *
 * \param src : Icoord to place rect at.
 * \param dst : destination *Rect.
 *
 * \return ptr on dst;
 */
SDL_Rect* CEV_icoordToRectPos(CEV_ICoord src, SDL_Rect* dst);


/** \brief Rect pos from Fcoord.
 *
 * \param src : Fcoord to place rect at.
 * \param dst : destination *Rect.
 *
 * \return ptr on dst;
 */
SDL_Rect* CEV_fcoordToRectPos(CEV_FCoord src, SDL_Rect* dst);


/** \brief Copies only rects dimensions.
 *
 * \param src : SDL_Rect* to copy from.
 * \param dst : SDL_Rect* to copy to.
 *
 * \return ptr on dst.
 */
SDL_Rect* CEV_rectDimCopy(SDL_Rect src, SDL_Rect* dst);


/** \brief Copies only pos members.
 *
 * \param src : SDL_Rect to read x,y from.
 * \param dst : SDL_Rect* to write x,y into.
 *
 * \return SDL_Rect* dst.
 */
SDL_Rect* CEV_rectPosCopy(SDL_Rect src, SDL_Rect* dst);


/** \brief rect x,y corrected to stay within border.
 *
 * \param rect : SDL_Rect* to be corrected.
 * \param border : SDL_rect to keep rect within.
 *
 * \return ptr to rect if modified, NULL *rect doesn't fit within border
 */
SDL_Rect* CEV_rectConstraint(SDL_Rect *rect, SDL_Rect border);


/** \brief point and rect corrected.
 *
 * \param point : SDL_Point* to correct.
 * \param rect : SDL_Rect* to correct .
 * \param w : width to keep rect into [0, w[.
 * \param h : height to keep rect into [0, h[.
 *
 * \return void
 */
void CEV_pointRectConstraint(SDL_Point *point, SDL_Rect *rect, int w, int h);


/** \brief rescale SDL_Rect dimensions.
 *
 * \param src : SDL_Rect* to resize.
 * \param scale : float ratio to apply.
 *
 * \return SDL_Rect* on itself, resized.
 */
SDL_Rect* CEV_rectDimScale(SDL_Rect *src, float scale);


/** \brief Fits / centers src within into
 *
 * \param src : SDL_Rect* to be fit.
 * \param into : SDL_Rect* as container of src.
 *
 * \return SDL_Rect as src result.
 */
SDL_Rect CEV_rectFitScaledInRect(SDL_Rect *src, SDL_Rect into);


/** \brief  sums 2 SDL_Rect.
 *
 * \param rect1 : first rectangle to sum.
 * \param rect2 : second rectangle to add.
 *
 * \return SDL_Rect as sum of rect1+rect2.
 */
SDL_Rect CEV_rectSum(SDL_Rect rect1, SDL_Rect rect2);



/** Allocations **/


/** \brief Allocates 2d matrix.
 *
 * \param x : size_t as num of columns.
 * \param y : size_t as num of rows.
 * \param size : size_t as unit of allocation.
 *
 * \return void** to allocation / NULL on failure.
 *
 */
void** CEV_allocate2d(size_t x, size_t y, size_t size);


/** \brief Allocates 3d cube.
 *
 * \param x : size_t as num of columns.
 * \param y : size_t as num of rows.
 * \param z : size_t as depth.
 * \param size : size_t as unit of allocation.
 *
 * \return void** to allocation / NULL on failure.
 *
 */
void*** CEV_allocate3d(size_t x, size_t y, size_t z, size_t size);







#ifdef __cplusplus
}
#endif

#endif /* CEV_API_H_INCLUDED */
