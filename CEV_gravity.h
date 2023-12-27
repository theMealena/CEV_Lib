#ifndef GRAVITY_H_INCLUDED
#define GRAVITY_H_INCLUDED

#include <stdbool.h>
#include <SDL.h>
#include <CEV_api.h>

#define CEV_VECT_NONE   0x0 //0
#define CEV_VECT_UP     0x1 //0001
#define CEV_VECT_RIGHT  0x2 //0010
#define CEV_VECT_DOWN   0x4 //0100
#define CEV_VECT_LEFT   0x8 //1000

#define CEV_VECT_X 0
#define CEV_VECT_Y 1
#define CEV_VECT_INERTIA 0
#define CEV_VECT_GRAVITY 1

//all values positives, axis direction will automaticaly define signeness
//jerk is applied only in down gravity (falling)

/** \brief single axis parameters
 */
typedef struct SVectPar
{
    float
        vel,    /**< velocity */
        acc,    /**< acceleration */
        jerk;   /**< jerk */
}
L_VectPar;


typedef struct SVector
{
    struct
    {
        //float
            //velMin,     /**< minimum velocity value */
            //velMax;     /**< maximum velocity value */

        int axisMode;   /**< axis mode inertia / gravity */

        L_VectPar
            set,        /**< setting set   */
            actual;     /**< actual values */
    }
    axis[2];

    float x,    /**< x vector actual value */
          y;    /**< y vector actual value */

    bool
        memUp,      /**< memory for up request front   */
        memDown;    /**< memory for domn request front */
}
CEV_Vector;



void CEV_vectParamSet(CEV_Vector *vect, float velMax, float acc, float jerk, unsigned int axis);

void CEV_vectQuickStop(CEV_Vector *vect, unsigned int axis);

void CEV_vectSpeedSet(CEV_Vector *vect, float value, unsigned int axis);

int CEV_vectUpdate(CEV_Vector *vect, int mode, CEV_FCoord* point);
#endif // GRAVITY_H_INCLUDED
