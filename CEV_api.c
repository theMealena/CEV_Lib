//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |    05-2016    |   2.0    |    SDL2 rev    **/
//**   CEV    |    05-2019    |   2.1    |functions added **/
//**   CEV    |    04-2020    |   2.1    |    revision    **/
//**********************************************************/
//CEV 2022/08/21 hotfix CEV_rectFitScaledInRect corrected

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
//#include "project_def.h"
#include "CEV_api.h"


/**miscellanous**/


bool CEV_bitGet(int flag, int bitIndex)
{/*defini si bit bitIndex présent dans valeur***VALIDE***/

            return (flag>>bitIndex) & 1;
}


void CEV_bitSet(int* flag, int bitIndex)
{/*set le bit bitIndex dans valeur***VALIDE***/

    *flag |= 1<<bitIndex;
}


void CEV_bitClear(int* flag, int bitIndex)
{/*reset le bit bitIndex dans valeur***VALIDE***/

   *flag &= (~(1<<bitIndex));
}


bool CEV_edgeRise(CEV_Edge * edge)
{/*calcul un front montant***VALIDE***/

    edge->out = edge->re = (edge->value && !edge->memo);

    edge->memo = edge->value;

    return (edge->out);
}


bool CEV_edgeFall(CEV_Edge *edge)
{/*calcul un front descendant***VALIDE***/

    edge->out = edge->fe = (!edge->value && edge->memo);

    edge->memo = edge->value;

    return(edge->out);
}


bool CEV_edgeAny(CEV_Edge *edge)
{/*n'importe quel front***VALIDE**/

    edge->out = edge->any = (edge->value != edge->memo);

    edge->memo = edge->value;

    return(edge->out);
}


bool CEV_edgeUpdate(CEV_Edge *edge)
{/*calcul de tous les fronts*/

    edge->re    = edge->value && !edge->memo;
    edge->fe    = !edge->value && edge->memo;
    edge->any   = edge->re || edge->fe;
    edge->out   = edge->any;
    edge->memo  = edge->value;

    return edge->out;
}


void CEV_intSwap(int* valOne, int* valTwo)
{/*intervertie 2 int*/
    /*(*valOne)^=((*valTwo)^=((*valOne)^=(*valTwo)));*/
    *valOne^=*valTwo;
    *valTwo^=*valOne;
    *valOne^=*valTwo;
}


void CEV_anySwap(void* valOne, void* valTwo, size_t size)
{/*intervertie 2 valeurs sur size octets*/
	char temp,
         *src =(char*)valOne,
         *dst =(char*)valTwo;

	for(unsigned i=0; i<size; i++)
	{
		temp    =   dst[i];
		dst[i]  =   src[i];
		src[i]  =   temp;
	}
}


void* CEV_bitFieldDistribute(const void *src, int srcBit, void* dst, int dstBit, size_t length)
{/*recopie un champs de bit*/

	char *srcPtr    = (char*)src;/*au format byte*/
	char *dstPtr    = (char*)dst;/*au format byte*/
	char temp       = 1;
	unsigned int i;/*variable de boucle*/
	char srcActBit;/*index du bit src actuel*/
	char dstActBit;/*index du bit destination actuel*/

	if (src==dst)
		return(NULL);

	srcPtr      = (char*)src+(srcBit/8);/*index de l'octet src à traiter*/
	dstPtr      = (char*)dst+(dstBit/8);/*index de l'octet destination à traiter*/
	srcActBit   = srcBit%8;/*index du bit src à traiter*/
	dstActBit   = dstBit%8;/*index du bit destination à traiter*/

	for (i=0; i<length; i++)
	{/*pour chaque bit à traiter*/

		temp=((*srcPtr>>srcActBit) & 1);/*on l'isole*/

        switch (temp)
        {/*selon temp*/

            case 0:
                temp    = ((1<<dstActBit)^0xff);/*création et inversion du masque*/
                *dstPtr = *dstPtr & temp;/*appliquation du masque*/
            break;

            case 1:
                temp    = 1<<dstActBit;/*création du masque*/
                *dstPtr = *dstPtr | temp;/*application du masque*/
            break;
        }

		srcActBit++;/*bit src suivant*/
		dstActBit++;/*bit destination suivant*/

		if (srcActBit>7)
		{/*si index bit src en fin d'octet*/
			srcActBit   = 0;/*retour au bit 0*/
			srcPtr      +=1;/*octet suivant*/
		}

		if(dstActBit>7)
		{/*si index bit destination en fin d'octet*/
			dstActBit   = 0;/*retour au bit 0*/
			dstPtr      +=1;/*octet suivant*/
		}
	}

	return dst;
}


int CEV_2dCoordToRowIndex(int x, int y, int width)
{/*x,y to linear index*/

 return  y*width + x;
}


CEV_ICoord CEV_rowIndexTo2dCoord(int index, int width)
{/*linear index to x,y*/

    return (CEV_ICoord){.x = index%width, .y = index/width, .z = 0};
}


int CEV_3dCoordToRowIndex(int x, int y, int z, int width, int height)
{/*x,y to linear index*/

 return  z*width*height + y*width + x;
}


CEV_ICoord CEV_rowIndexTo3dCoord(int index, int width, int height)
{/*linear index to x,y,z*/

    return (CEV_ICoord){.x = index%width, .y = index/width, .z = index/(width*height)};

}


bool CEV_addModulo(int mode, int* val, int modulo)
{/*incrément / décrément de val par modulo***VALIDE***/

    bool sts = false;

    switch (mode)
    {
        case INC :

            (*val)++;
            sts = !((*val) %= modulo);

        break;

        case DEC :

            if(*val)
            {
                (*val)--;
                (*val) %= modulo;
            }
            else
            {
                (*val) = modulo-1;
                sts  = true;
            }

        break;

        default :
        break;
    }

    return sts;
}


/*true if inc done**/
bool CEV_addLimited(int mode, int *val, int num)
{/*incrément / décrément de val limité***VALIDE***/

    switch(mode)
    {
        case INC :
            if ((*val+1) < num)
            {
                (*val)++;
                return true;
            }
            else
                return false;
        break;

        case DEC :
            if (*val>0)
            {
                (*val)--;
                return true;
            }
            else
                return false;
        break;

        default:
        break;
    }

    return 0;
}


void CEV_memSet(void *ptr, char value, size_t size)
{/*memset*/

    char* cleaner = (char*)ptr;
    int i;

    for(i=0; i<(int)size; i++)
        *(cleaner+i) = value;
}


void CEV_memCopy(void *src, void *dst, size_t count)
{/*copie count bytes de src vers dst*/

    char*tempSrc = (char*)src;
    char*tempDst = (char*)dst;

    tempSrc--;
    tempDst--;

    while (count--)
    {
        *++tempDst = *++tempSrc;
    }
}


bool CEV_lim(int limInf, int val, int limSup)
{/*interval vrai (limites incluse) si entre, faux si hors*/

    if(limInf <= limSup)
        return (val >= limInf && val <= limSup);
    else
        return (val >= limInf || val <= limSup);
}


bool CEV_reachValue(float* value, float reach, float by)
{/*value to reach*/

    if(*value < reach)
    {
        *value += by;

        if(*value > reach)
        {
            *value = reach;
            return true;
        }
    }

    else if (*value > reach)
    {
        *value -= by;

        if(*value < reach)
        {
            *value = reach;
            return true;
        }
    }

    return false;
}


bool CEV_constraint(int mini, int* value, int maxi)
{/*keeps value within [mini, maxi]*/

    if (*value < mini)
    {
        *value = mini;
        return true;
    }
    else if (*value>maxi)
    {
        *value = maxi;
        return true;
    }

    return false;
}


bool CEV_fconstraint(float mini, float* value, float maxi)
{/*keeps value within [mini, maxi] for floats*/

    if (*value < mini)
    {
        *value = mini;
        return true;
    }
    else if (*value>maxi)
    {
        *value = maxi;
        return true;
    }

    return false;
}


double CEV_map(double realVal, double realMin, double realMax, double retMin, double retMax)
{/*scaling*/
    if(((realMax-realMin) + retMin)!= 0)
        return((retMax-retMin)*(realVal-realMin)/(realMax-realMin) + retMin);
    else
        return 0;
}


int CEV_irand(int start, int end)
{/*random int value in [start, end]*/

    if (end <= start)
        return 0;

    return rand() * (end-start) / RAND_MAX + start;
}


double CEV_frand(double start, double end)
{/*random double in [start, end]*/

    if (end<=start)
        return 0.0;

    return (double)rand() * (end-start) / (double)RAND_MAX + start;
}


bool CEV_floatIsEqual(double value, double target, double tolerance)
{/*retruns true if value is within (target +/- tolerance)*/

    return ((value >= target-tolerance) && (value <= target+tolerance));
}


double CEV_fModulo(double value, double mod)
{/*floatting point modulo*/
    return (value - ((int)(value/mod) * mod));
}


double CEV_vectAngle(double x, double y)
{/*angle of vect (x,y)*/

    return ((atan2(y, x)) * 180 / PI);
}


/*** SDL Point ***/

bool CEV_pointAreEqual(SDL_Point* pta, SDL_Point* ptb)
{/*same point*/

	return ((pta->x == ptb->x) && (pta->y == ptb->y));
}


SDL_Point CEV_pointClear(void)
{/*clear point*/

    return (SDL_Point){.x = 0, .y = 0};
}


SDL_Point CEV_icoordToPoint(CEV_ICoord src)
{/*CEV_icoord to point*/

	return (SDL_Point){.x = src.x, .y = src.y};
}


SDL_Point CEV_fcoordToPoint(CEV_FCoord src)
{/*CEV_fcoord to point*/

	return (SDL_Point){.x = (int)src.x, .y = (int)src.y};
}


SDL_Point* CEV_rectMidToPoint(SDL_Rect src, SDL_Point *dst)
{/*set point to be gravity center of rect*/

    dst->x = src.x + src.w/2;
    dst->y = src.y + src.h/2;

    return dst;
}


SDL_Point CEV_rectPosToPoint(SDL_Rect src)
{/*x, y copy*/
    return (SDL_Point){.x = src.x, .y = src.y};
}


SDL_Point CEV_pointMedian(SDL_Point pta, SDL_Point ptb)
{/*median between 2 points*/

    return (SDL_Point){ .x = (pta.x + ptb.x)/2,
                        .y = (pta.y + ptb.y)/2};
}


bool CEV_pointIsInRect(SDL_Point point, SDL_Rect rect)
{/*true if point inside rect*/

    return ((point.x >= rect.x)
            && (point.x < (rect.x + rect.w))
            && (point.y >= rect.y)
            && (point.y < (rect.y + rect.h)));
}


double CEV_pointDist(SDL_Point pta, SDL_Point ptb)
{/*distance between points*/
    double x = (double)pta.x - ptb.x,
           y = (double)pta.y - ptb.y;

    return sqrt(x*x + y*y);
}


double CEV_pointAngle(SDL_Point pta, SDL_Point ptb)
{/*angle from pta to ptb*/

    double x   = (double)ptb.x - pta.x,
           y   = (double)ptb.y - pta.y;


    return CEV_vectAngle(x, y);
}


SDL_Point CEV_pointSum(SDL_Point pta, SDL_Point ptb)
{/*sums 2 points*/

    return (SDL_Point){.x = pta.x + ptb.x, .y = pta.y + ptb.y};
}


/*** CEV_Icoord ***/

bool CEV_IcoordAreEqual(CEV_ICoord* pta, CEV_ICoord* ptb)
{//two point are at same position

	return ((pta->x == ptb->x) && (pta->y == ptb->y) && (pta->z == ptb->z));
}


CEV_ICoord CEV_icoordClear(void)
{/*clear point*/

    return (CEV_ICoord){.x = 0, .y = 0, .z = 0};
}


CEV_ICoord CEV_pointToIcoord(SDL_Point src)
{/*SDL_Point to Icoord*/

    return (CEV_ICoord){.x = src.x, .y = src.y, .z = 0};
}


CEV_ICoord CEV_fcoordToIcoord(CEV_FCoord src)
{/*FCoord to ICoord*/

    return (CEV_ICoord){.x = (int)src.x, .y = (int)src.y, .z = (int)src.z};
}


CEV_ICoord* CEV_rectMidToIcoord(SDL_Rect src, CEV_ICoord *dst )
{/*set dst to be gravity center of src*/

    dst->x = src.x + src.w/2;
    dst->y = src.y + src.h/2;
    //z unchanged

    return dst;
}


CEV_ICoord CEV_rectPosToIcoord(SDL_Rect src)
{/*x, y copy*/

    return (CEV_ICoord){.x = src.x, .y = src.y, .z = 0};
}


CEV_ICoord CEV_icoordMedian(CEV_ICoord pta, CEV_ICoord ptb)
{/*median between 2 icoord*/

    return (CEV_ICoord){ .x = (pta.x + ptb.x)/2,
                         .y = (pta.y + ptb.y)/2,
                         .z = (pta.z + ptb.z)/2};
}


bool CEV_icoordIsInRect(CEV_ICoord point, SDL_Rect rect)
{/*true if point inside rect*/

    return ((point.x >= rect.x)
            && (point.x < (rect.x + rect.w))
            && (point.y >= rect.y)
            && (point.y < (rect.y + rect.h)));
}


double CEV_icoordDist(CEV_ICoord pta, CEV_ICoord ptb)
{/*distance entre 2 points pythagore*/

    double x = (double)pta.x - ptb.x,
           y = (double)pta.y - ptb.y,
           z = (double)pta.z - ptb.z;

    return sqrt(x*x + y*y + z*z);
}


double CEV_icoordAngle2d(CEV_ICoord pta, CEV_ICoord ptb)
{/*angle(x y) from pta to ptb*/

    double x   = (double)ptb.x - pta.x,
           y   = (double)ptb.y - pta.y;


    return CEV_vectAngle(x, y);
}


CEV_ICoord CEV_icoordSum(CEV_ICoord pta, CEV_ICoord ptb)
{/*sums 2 icoord*/

    return (CEV_ICoord){.x = pta.x + ptb.x, .y = pta.y + ptb.y, .z = pta.z + ptb.z};
}


/*** CEV_Fcoord ***/

bool CEV_fcoordAreEqual(CEV_FCoord* pta, CEV_FCoord* ptb, double tolerance)
{//two points at same position within tolerance

	return CEV_fcoordDist(*pta, *ptb) <= tolerance;
}


CEV_FCoord CEV_fcoordClear(void)
{/*clear point*/

    return (CEV_FCoord){.x = 0.0, .y = 0.0, .z = 0.0};
}


CEV_FCoord CEV_pointToFcoord(SDL_Point src)
{/*SDL_point to CEV_fcoord*/
    return (CEV_FCoord){.x = (float)src.x, .y = (float)src.y, .z = 0.0};
}


CEV_FCoord CEV_icoordToFcoord(CEV_ICoord src)
{/*ICoord to FCoord*/
    return (CEV_FCoord){.x = (float)src.x, .y = (float)src.y, .z = (float)src.z};
}


CEV_FCoord* CEV_rectMidToFcoord(SDL_Rect src, CEV_FCoord *dst)
{/*set dst to be gravity center of src*/

    dst->x = (float)src.x + (float)src.w / 2.0;
    dst->y = (float)src.y + (float)src.h / 2.0;
    //z unchanged

    return dst;
}


CEV_FCoord CEV_rectPosToFcoord(SDL_Rect src)
{/*set point to be gravity center of rect*/

    return (CEV_FCoord){.x = (float)src.x, .y = (float)src.y, .z = 0.0};
}


CEV_FCoord CEV_fcoordMedian(CEV_FCoord pta, CEV_FCoord ptb)
{/*median between 2 icoord*/

    return (CEV_FCoord){ .x = (pta.x + ptb.x)/2,
                         .y = (pta.y + ptb.y)/2,
                         .z = (pta.z + ptb.z)/2};
}


bool CEV_fcoordIsInRect(CEV_FCoord point, SDL_Rect rect)
{/*true if point inside rect*/

    return ((point.x >= (float)rect.x)
         && (point.x <  (float)(rect.x + rect.w))
         && (point.y >= (float)rect.y)
         && (point.y <  (float)(rect.y + rect.h)));
}


double CEV_fcoordDist(CEV_FCoord pta, CEV_FCoord ptb)
{/*distance entre 2 points pythagore*/

    double x = (double)pta.x - ptb.x,
           y = (double)pta.y - ptb.y,
           z = (double)pta.z - ptb.z;

    return sqrt(x*x + y*y + z*z);
}


double CEV_fcoordAngle2d(CEV_FCoord pta, CEV_FCoord ptb)
{/*angle(x y) from pta to ptb*/

    double   x   = (double)ptb.x - pta.x,
             y   = (double)ptb.y - pta.y;

    return CEV_vectAngle(x, y);
}


CEV_FCoord CEV_fcoordSum(CEV_FCoord pta, CEV_FCoord ptb)
{/*sums 2 fcoord*/

    return (CEV_FCoord){.x = pta.x + ptb.x, .y = pta.y + ptb.y, .z = pta.z + ptb.z};
}



/*** SDL_Rect ***/

void CEV_rectDump(SDL_Rect in)
{
    printf("x=%d, y=%d, w=%d, h=%d\n", in.x, in.y, in.w, in.h);
}


SDL_Rect *CEV_rectAroundPoint(SDL_Point src, SDL_Rect *dst)
{/*sets rect with point as gravity center*/

    dst->x = src.x - (dst->w/2);
    dst->y = src.y - (dst->h/2);

    return dst;
}


SDL_Rect* CEV_rectAroundIcoord(CEV_ICoord src, SDL_Rect* dst)
{/*sets rect with icoord as gravity center*/

    dst->x = src.x - (dst->w/2);
    dst->y = src.y - (dst->h/2);

    return dst;
}


SDL_Rect* CEV_rectAroundFcoord(CEV_FCoord src, SDL_Rect* dst)
{/*sets rect with fcoord as gravity center*/

    dst->x = (int)src.x - (dst->w/2);
    dst->y = (int)src.y - (dst->h/2);

    return dst;
}


SDL_Rect* CEV_pointToRectPos(SDL_Point src, SDL_Rect* dst)
{/*x, y copy*/

    dst->x = src.x;
    dst->y = src.y;

    return dst;
}


SDL_Rect* CEV_icoordToRectPos(CEV_ICoord src, SDL_Rect* dst)
{/*x, y copy*/

    dst->x = (int)src.x;
    dst->y = (int)src.y;

    return dst;
}


SDL_Rect* CEV_fcoordToRectPos(CEV_FCoord src, SDL_Rect* dst)
{/*x, y copy*/

    dst->x = (int)src.x;
    dst->y = (int)src.y;

    return dst;
}


SDL_Rect* CEV_rectDimCopy(SDL_Rect src, SDL_Rect* dst)
{/*w, h copy*/

    dst->w = src.w;
    dst->h = src.h;

    return dst;
}


SDL_Rect* CEV_rectPosCopy(SDL_Rect src, SDL_Rect* dst)
{//x, y copy
    dst->x = src.x;
    dst->y = src.y;

    return dst;
}


SDL_Rect* CEV_rectConstraint(SDL_Rect *rect, SDL_Rect border)
{/*keeps rect inside border */

    if(rect->w > border.w)
        rect->w = border.w;

    if(rect->h > border.h)
        rect->h = border.h;

    if (rect->x < border.x)
        rect->x = border.x;

    else if ((rect->x + rect->w) > (border.x + border.w))
        rect->x = border.w - rect->w /*- 1*/;

    if (rect->y < border.y )
        rect->y = border.y;

    else if ((rect->y + rect->h) > (border.y + border.h))
        rect->y = border.h - rect->h /*- 1*/;

    return rect;
}


void CEV_pointRectConstraint(SDL_Point *point, SDL_Rect *rect, int w, int h)
{/*keeps point and rect within w*h*/

    SDL_Rect lrect = {0, 0, w, h};

    CEV_rectAroundPoint(*point, rect);
    CEV_rectConstraint(rect, lrect);
    CEV_rectMidToPoint(*rect, point);
}


SDL_Rect* CEV_rectDimScale(SDL_Rect *src, float scale)
{/*redim w*h according to scale*/

    float temp = src->w * scale;
    src->w = (int)temp;
    temp = src->h * scale;
    src->h = (int)temp;

    return src;
}


SDL_Rect CEV_rectFitScaledInRect(SDL_Rect *src, SDL_Rect into)
{//modify src to fit into "into" while keeping aspect ratio

    CEV_rectDimScale(src, MIN((float)into.h/src->h, (float)into.w/src->w));
    //*src = CEV_rectCenteredInRect(*src, into);

    if(src->w < into.w)
        src->x = into.x + (into.w - src->w)/2;
    else
        src->x = into.x;

    if(src->h < into.h)
        src->y = into.y + (into.h - src->h)/2;
    else
        src->y = into.y;

    return *src;
}


SDL_Rect CEV_rectCenteredInRect(SDL_Rect src, SDL_Rect into)
{//centers src into "into"
    src.x = into.x + (into.w - src.w)/2;
    src.y = into.y + (into.h - src.h)/2;

    return src;
}


SDL_Rect CEV_rectSum(SDL_Rect rect1, SDL_Rect rect2)
{//sums SDL_Rect
    SDL_Rect result;

    int leftX   = MIN(rect1.x, rect2.x),
        rightX  = MAX(rect1.x + rect1.w, rect2.x + rect2.w),
        topY    = MIN(rect1.y, rect2.y),
        bottomY = MAX(rect1.y + rect1.h, rect2.y + rect2.h);

    result.x = leftX;
    result.y = topY;
    result.w = rightX - leftX;
    result.h = bottomY - topY;

    return result;
}


/*** Allocations **/

void** CEV_allocate2d(size_t x, size_t y, size_t size)
{
    unsigned i = 0;
    void** result = calloc(x, sizeof(void*));

    if IS_NULL(result)
        return NULL;

    for(i = 0; i<x; i++)
    {
        result[i] = calloc(y, size);

        if IS_NULL(result[i])
            goto err;
    }

    return result;

err:

    for(unsigned j=0; j<i; j++)
        free(result[j]);

    return NULL;
}


void*** CEV_allocate3d(size_t x, size_t y, size_t z, size_t size)
{
    unsigned i = 0;

    void*** result = calloc(x, sizeof(void**));

    if IS_NULL(result)
        return NULL;

    for(i=0; i<x; i++)
    {
        result[i] = CEV_allocate2d(y, z, size);

        if IS_NULL(result[i])
            goto err;
    }

    return result;

err:

    for(unsigned j=0; j<i; j++)
    {
        for(unsigned k=0; k<y; k++)
            free(result[j][k]);
    }

    return NULL;
}
