/* 
	/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/

#pragma once

#include <maya/MPxSurfaceShapeUI.h> 

/* ==========================================
	
	Class VoxelShapeUI

	Issues draw calls using the VolumeShape data

========================================== */

class VoxelShapeUI : public MPxSurfaceShapeUI
{
public:
	VoxelShapeUI();
	virtual ~VoxelShapeUI();

	//
	// Overrides
	//

	// Puts draw request on the draw queue
	//
	virtual void	getDrawRequests( const MDrawInfo & info, bool objectAndActiveOnly, MDrawRequestQueue & requests );

	// Main draw routine. Gets called by maya with draw requests.
	//
	virtual void	draw( const MDrawRequest & request, M3dView & view ) const;

	// Main selection routine
	//
	virtual bool	select( MSelectInfo &selectInfo,
							MSelectionList &selectionList,
							MPointArray &worldSpaceSelectPts ) const;

	//
	// Helper routines
	//

	static  void *      creator();

private:

	// Draw Tokens
	//
	enum {
		kDrawVertices, // component token
		kDrawWireframe,
		kDrawWireframeOnShaded,
		kDrawSmoothShaded,
		kDrawFlatShaded,
		kDrawBoundingBox,
		kDrawRedPointAtCenter,  // for userInteraction example code
		kLastToken
	};

};
