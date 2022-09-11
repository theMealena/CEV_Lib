#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include "CEV_zoom.h"
#include "CEV_api.h"


void CEV_zoomDump(CEV_Zoom src)
{
    printf("pos :  x=%d, y=%d, w=%d, h=%d\n", src.pos.x, src.pos.y, src.pos.w, src.pos.h);
}


CEV_Zoom CEV_zoomInit(int baseW, int baseH, float scaleMax, float step)
{
    CEV_Zoom result =
	{
		.enable		= 0,
		.scaleMax	= scaleMax,
		.scaleMin	= 1.0,
		.scaleAct   = 1.0,
		.baseDim.x  = 0,
		.baseDim.y  = 0,
		.baseDim.w  = baseW,
		.baseDim.h  = baseH,
		.pos.x      = 0,
		.pos.y      = 0,
		.pos.w      = baseW,
		.pos.h      = baseH,
		.step       = step
	};

    return result;
}


void CEV_zoomScaleMaxSet(CEV_Zoom* dst, float scaleMax)
{
    dst->scaleMax = scaleMax;
}


void CEV_zoomScaleSet(CEV_Zoom* dst, float scale)
{
    dst->scaleAct= scale;
}



SDL_Rect CEV_zoomClipGet(CEV_Zoom* zoom)
{
    SDL_Rect result = zoom->pos;
    CEV_rectDimScale(&result, 1/zoom->scaleAct); //scaling rect
    return result;
}


SDL_Rect CEV_zoomAuto(CEV_Zoom* zoom, SDL_Point point)
{//automatic zoom

    SDL_Rect result = zoom->pos;

    result =  CEV_zoomOnCoord(zoom, point, zoom->enable? ZOOM_IN : ZOOM_OUT);

    return result;
}


SDL_Rect CEV_zoomOnCoord(CEV_Zoom *zoom, SDL_Point point, int direction)
{//returns an SDL_Rect as clip centered on point

    SDL_Rect funcSts = zoom->pos;

    point.x = zoom->pos.x + point.x / zoom->scaleAct;
    point.y = zoom->pos.y + point.y / zoom->scaleAct;

    if(direction)
    {
        CEV_zoomScaleUpdate(zoom, direction); //scaling update
    }

    CEV_rectDimScale(&funcSts, 1/zoom->scaleAct); //scaling rect
    CEV_rectAroundPoint(point, &funcSts); //centering rect on point
    CEV_rectConstraint(&funcSts, zoom->baseDim); //keeping zoom in src rect

    zoom->pos.x = funcSts.x;
    zoom->pos.y = funcSts.y;

    return funcSts;
}


bool CEV_zoomScaleUpdate(CEV_Zoom *zoom, int direction)
{//updates zoom scale

    bool reached = true;

    switch (direction)
    {
        case ZOOM_IN :
            reached = CEV_reachValue(&zoom->scaleAct, zoom->scaleMax, zoom->step);
        break;

        case ZOOM_OUT :
            reached = CEV_reachValue(&zoom->scaleAct, zoom->scaleMin, zoom->step);
        break;

        case ZOOM_NONE :
        break;
    }

    return reached;
}
