/*
 *  ImageFilterCollection.h
 *  Created by jason van cleave on 8/7/13.
 */

#pragma once

#include "ofMain.h"

#include "OMX_Maps.h"




class ImageFilterCollection
{
	
public:
    
    OMX_IMAGEFILTERTYPE currentFilter;
    string currentFilterName;
    size_t currentFilterIndex;
    
	ImageFilterCollection()
	{
        currentFilterName = "None";
        currentFilterIndex = 0;
        currentFilter = OMX_Maps::getInstance().getImageFilter(currentFilterName);
	}
	
	void setup(string initialFilterName = "None")
	{
        setCurrentFilter(initialFilterName);
	}
	
	
	OMX_IMAGEFILTERTYPE setCurrentFilter(string filterName)
	{
        for(size_t i=0; i<OMX_Maps::getInstance().imageFilterNames.size(); ++i)
        {
            if(OMX_Maps::getInstance().imageFilterNames[i] == filterName)
            {
                currentFilterIndex = i;
            }
        }
        currentFilterName = filterName;
        currentFilter = OMX_Maps::getInstance().getImageFilter(currentFilterName);
        ofLogVerbose() << "currentFilterName: " << currentFilterName;
		return currentFilter;
	}
	
	OMX_IMAGEFILTERTYPE getNextFilter()
	{
		if (currentFilterIndex+1 < OMX_Maps::getInstance().imageFilterNames.size()) 
		{
            currentFilterIndex++;
		}else 
		{
            currentFilterIndex = 0;
		}
        return setCurrentFilter(OMX_Maps::getInstance().imageFilterNames[currentFilterIndex]);
                                
	}
	string getCurrentFilterName()
	{
		return currentFilterName;
	}
	

};