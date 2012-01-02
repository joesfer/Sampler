/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/

#include "VoxelShapeUI.h"
#include "VoxelShape.h"
#include <maya/MColor.h>
#include <maya/MDrawData.h>
#include <maya/MSelectionMask.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>

// Object and component color defines
//
#define LEAD_COLOR				18	// green
#define ACTIVE_COLOR			15	// white
#define ACTIVE_AFFECTED_COLOR	8	// purple
#define DORMANT_COLOR			4	// blue
#define HILITE_COLOR			17	// pale blue
#define DORMANT_VERTEX_COLOR	8	// purple
#define ACTIVE_VERTEX_COLOR		16	// yellow

VoxelShapeUI::VoxelShapeUI() {}
VoxelShapeUI::~VoxelShapeUI() {}

void* VoxelShapeUI::creator()
{
	return new VoxelShapeUI();
}

//////////////////////////////////////////////////////////////////////////
// VoxelShapeUI::getDrawRequests (override)
//
// Description:
//
//     Add draw requests to the draw queue
//
// Arguments:
//
//     info                 - current drawing state
//     objectsAndActiveOnly - no components if true
//     queue                - queue of draw requests to add to
//
//////////////////////////////////////////////////////////////////////////

void VoxelShapeUI::getDrawRequests(	const MDrawInfo & info,
									bool objectAndActiveOnly,
									MDrawRequestQueue & queue ) {

	// Get the data necessary to draw the shape
	//
	VoxelShape* shape =  (VoxelShape*)surfaceShape();
	VoxelPreviewData* previewData = shape->getData();
	if ( previewData == NULL ) {
		cerr << "NO DrawRequest for VoxelShapeUI\n";
		return;
	}

	// This call creates a prototype draw request that we can fill
	// in and then add to the draw queue.
	//
	MDrawData data;
	MDrawRequest request = info.getPrototype( *this );
	getDrawData( previewData, data );
	request.setDrawData( data );

	// Decode the draw info and determine what needs to be drawn
	//

	M3dView::DisplayStyle  appearance    = info.displayStyle();
	M3dView::DisplayStatus displayStatus = info.displayStatus();

	// Are we displaying meshes?
	if ( ! info.objectDisplayStatus( M3dView::kDisplayMeshes ) )
	  return;

	switch ( appearance ) {

		case M3dView::kWireFrame:  // fall through
		case M3dView::kFlatShaded: // fall through
		case M3dView::kGouraudShaded:
		{
		  request.setToken( kDrawWireframe );
		  request.setDisplayStyle( M3dView::kWireFrame );

		  M3dView::ColorTable activeColorTable = M3dView::kActiveColors;
		  M3dView::ColorTable dormantColorTable = M3dView::kDormantColors;

		  switch ( displayStatus ) {
			  case M3dView::kLead :
				  request.setColor( LEAD_COLOR, activeColorTable );
				  break;
			  case M3dView::kActive :
				  request.setColor( ACTIVE_COLOR, activeColorTable );
				  break;
			  case M3dView::kActiveAffected :
				  request.setColor( ACTIVE_AFFECTED_COLOR, activeColorTable );
				  break;
			  case M3dView::kDormant :
				  request.setColor( DORMANT_COLOR, dormantColorTable );
				  break;
			  case M3dView::kHilite :
				  request.setColor( HILITE_COLOR, activeColorTable );
				  break;
			  default:	
				  break;
		  }

		  queue.add( request );
		  break;
		}

	default: 
	  break;
	}
}



//////////////////////////////////////////////////////////////////////////
// VoxelShapeUI::draw (override)
//
// Description:
//
//     Main (OpenGL) draw routine
//
// Arguments:
//
//     request - request to be drawn
//     view    - view to draw into
//
//////////////////////////////////////////////////////////////////////////

void VoxelShapeUI::draw( const MDrawRequest & request, M3dView & view ) const{ 
	// Get the token from the draw request.
	// The token specifies what needs to be drawn.
	//
	int token = request.token();
	MDrawData data = request.drawData();
	VoxelPreviewData* previewData = (VoxelPreviewData*)data.geometry();

	switch( token )
	{
	case kDrawWireframe :
		view.beginGL();
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		previewData->draw();
		view.endGL();
		break;
	default: break;
	}
}



//////////////////////////////////////////////////////////////////////////
// VoxelShapeUI::select (override)
//
// Description:
//
//     Main selection routine
//
// Arguments:
//
//     selectInfo           - the selection state information
//     selectionList        - the list of selected items to add to
//     worldSpaceSelectPts  -
//
//////////////////////////////////////////////////////////////////////////

bool VoxelShapeUI::select( MSelectInfo &selectInfo, 
					  MSelectionList &selectionList,
					  MPointArray &worldSpaceSelectPts ) const {
					  
	 VoxelShape* shape = (VoxelShape*)surfaceShape();
	 if ( shape == NULL ) return false;

	 // NOTE: If the geometry has an intersect routine it should
	 // be called here with the selection ray to determine if the
	 // the object was selected.

	 MSelectionMask priorityMask( MSelectionMask::kSelectMeshes );
	 MSelectionList item;
	 item.add( selectInfo.selectPath() );
	 MPoint xformedPt;
	 if ( selectInfo.singleSelection() ) {
		 MPoint center = shape->boundingBox().center();
		 xformedPt = center;
		 xformedPt *= selectInfo.selectPath().inclusiveMatrix();
	 }

	 selectInfo.addSelection( item, xformedPt, selectionList,
		 worldSpaceSelectPts, priorityMask, false );

	 return true;
}
