#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include <CEV_zoom.h>
#include <CEV_api.h>
#include <CEV_mixSystem.h>


void CEV_zoomDump(CEV_Zoom src)
{
    puts("Dumping CEV_Zoom");
    printf("enable is %s\n", src.enable? "true": "false");
    printf("minimum scale is %f", src.scaleMin);
    printf("maximum scale is %f", src.scaleMax);
    printf("actual scale is %f", src.scaleAct);
    printf("stepping is is %f", src.step);
    printf("reference rect is : ");
    CEV_rectDump(src.baseDim);
    printf("renderer rect is : ");
    CEV_rectDump(src.renderDim);
    printf("clip rect is : ");
    CEV_rectDump(src.clip);
    printf("blit rect is : ");
    CEV_rectDump(src.blit);
}


CEV_Zoom CEV_zoomInit(int baseW, int baseH, float scaleMax, float step)
{
    CEV_Zoom result =
	{
		.enable		= 0,
		.scaleMax	= scaleMax,
		.scaleMin   = 1.0,
		.scaleAct   = 1.0,
		.step       = step,
		.baseDim.x  = 0,
		.baseDim.y  = 0,
		.baseDim.w  = baseW,
		.baseDim.h  = baseH,
		.renderDim.x  = 0,
		.renderDim.y  = 0,
		.renderDim.w  = SCREEN_WIDTH,
		.renderDim.h  = SCREEN_HEIGHT
	};

    //setting clip
    result.clip = result.baseDim; //at render dimension


    //setting blit
	CEV_rectDimCopy(result.baseDim, &result.blit); //at texture dim
	result.blit = CEV_rectCenteredInRect(result.blit, result.baseDim); //centering blit in render

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


bool CEV_zoomIsClip(CEV_Zoom *src)
{
    return (src->scaleAct > 1.0);
}


SDL_Rect* CEV_zoomClipGet(CEV_Zoom* zoom)
{

    if(CEV_zoomIsClip(zoom)) //if cliping src texture
        return &zoom->clip; //return clip
    else
        return NULL; //or blit all
}


SDL_Rect* CEV_zoomBlitGet(CEV_Zoom* zoom)
{
    if(CEV_zoomIsClip(zoom))
        return NULL;
    else
        return &zoom->blit;

}


SDL_Rect CEV_zoomAuto(CEV_Zoom* zoom, SDL_Point point)
{//automatic zoom

    SDL_Rect result = zoom->clip;

    result =  CEV_zoomOnCoord(zoom, point, zoom->enable? ZOOM_IN : ZOOM_OUT);

    return result;
}


SDL_Rect CEV_zoomOnCoord(CEV_Zoom *zoom, SDL_Point point, int direction)
{//returns an SDL_Rect as clip centered on point

    SDL_Rect result;

    if(direction)
        CEV_zoomScaleUpdate(zoom, direction); //scaling update

    if(zoom->scaleAct>=1.0)
    {//calculating clip

        result = zoom->baseDim;

        CEV_rectDimScale(&result, 1/zoom->scaleAct);        //scaling rect
        CEV_rectAroundPoint(point, &result);   //centering rect on point
        CEV_rectConstraint(&result, zoom->baseDim);         //keeping zoom in src rect

        zoom->clip = result;
    }

    {//zoom is blit
        result = zoom->baseDim;

        CEV_rectDimScale(&result, zoom->scaleAct); //scaling rect

        CEV_rectDimCopy(result,&zoom->blit);
        zoom->blit.x = (zoom->renderDim.w/2) - (point.x * zoom->scaleAct);
        zoom->blit.y = (zoom->renderDim.h/2) - (point.y * zoom->scaleAct);
    }

    return result;
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
