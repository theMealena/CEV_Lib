#include <CEV_api.h>
#include "CEV_gravity.h"


void CEV_vectParamSet(CEV_Vector *vect, float velMax, float acc, float jerk, unsigned int axis)
{//copying new value only if any // value kept if 0 as param

    if(velMax)
        vect->axis[axis].set.vel    = velMax;

    if(acc)
        vect->axis[axis].set.acc    = acc;

    if(jerk)
        vect->axis[axis].set.jerk   = jerk;

}


void CEV_vectQuickStop(CEV_Vector *vect, unsigned int axis)
{
    if( (axis == CEV_VECT_X) || (axis == CEV_VECT_Y))
    {
        vect->axis[axis].actual.vel  = 0.0;
        vect->axis[axis].actual.acc  = 0.0;
        vect->axis[axis].actual.jerk = 0.0;

        if (axis == CEV_VECT_Y)
        {
            vect->memDown   = false;
            vect->memUp     = false;
        }
    }
}


void CEV_vectSpeedSet(CEV_Vector *vect, float value, unsigned int axis)
{
    vect->axis[axis].actual.vel = value;
    vect->axis[axis].actual.acc = vect->axis[axis].set.acc;

    if (axis == CEV_VECT_Y)
    {
        vect->memDown   = false;
        vect->memUp     = false;
    }
}


int CEV_vectUpdate(CEV_Vector *vect, int mode, CEV_FCoord* point)
{

    bool    //extracting the actual directin requested
        up      = (mode & CEV_VECT_UP),
        down    = (mode & CEV_VECT_DOWN),
        left    = (mode & CEV_VECT_LEFT),
        right   = (mode & CEV_VECT_RIGHT),
        re_up   = up && !vect->memUp,
        re_down = down && !vect->memDown,
        fe_down = !down && vect->memDown;

    /***X inertia***/ //OK

    float reachVel; //velocity to reach

    if (left)
        reachVel = -vect->axis[CEV_VECT_X].set.vel;

    else if (right)
        reachVel = vect->axis[CEV_VECT_X].set.vel;

    else
        reachVel = 0.0;

    CEV_reachValue(&vect->axis[CEV_VECT_X].actual.vel, reachVel, vect->axis[CEV_VECT_X].set.acc);

    vect->x = vect->axis[CEV_VECT_X].actual.vel;

    point->x = vect->x;


    switch(vect->axis[CEV_VECT_Y].axisMode)
    {
        case CEV_VECT_INERTIA : /***Y Inertia***/

            if (down)
                reachVel = vect->axis[CEV_VECT_Y].set.vel;
            else if (up)
                reachVel = -vect->axis[CEV_VECT_Y].set.vel;
            else
                reachVel = 0.0;

            CEV_reachValue(&vect->axis[CEV_VECT_Y].actual.vel, reachVel, vect->axis[CEV_VECT_Y].set.acc);

        break;

        case CEV_VECT_GRAVITY : /***Y Gravity***/

        //vect->axis[CEV_VECT_Y].velMin = -vect->axis[CEV_VECT_Y].set.vel;
        //vect->axis[CEV_VECT_Y].velMax = vect->axis[CEV_VECT_Y].set.vel;

        if (re_up) //UP start jumping
        {//passing start jumping param
            vect->axis[CEV_VECT_Y].actual.vel = -vect->axis[CEV_VECT_Y].set.vel;
            vect->axis[CEV_VECT_Y].actual.acc = vect->axis[CEV_VECT_Y].set.acc;
        }

        if (re_down) //start falling
            vect->axis[CEV_VECT_Y].actual.acc = 0.0;

        if (fe_down)//DOWN stop falling
            vect->axis[CEV_VECT_Y].actual.vel = 0.0;

        if (up + down)
        {
            CEV_reachValue(&vect->axis[CEV_VECT_Y].actual.acc, vect->axis[CEV_VECT_Y].set.acc, vect->axis[CEV_VECT_Y].set.jerk);
            CEV_reachValue(&vect->axis[CEV_VECT_Y].actual.vel, vect->axis[CEV_VECT_Y].set.vel, vect->axis[CEV_VECT_Y].set.acc);
        }

        break;
    }

    vect->y = vect->axis[CEV_VECT_Y].actual.vel;

    point->y = vect->y;

    vect->memUp     = up;
    vect->memDown   = down;

    return vect->y;
}



