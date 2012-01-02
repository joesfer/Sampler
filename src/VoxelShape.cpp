/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/

#include "VoxelShape.h"

#include <assert.h>

#include <math.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>

#include <maya/MGlobal.h>

#include <maya/MFnPluginData.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MPointArray.h>
#include <maya/MFnPointArrayData.h>

const MTypeId VoxelShape::id( 0x80100 );
const MString VoxelShape::typeName( "VoxelPreview" );
const MTypeId VoxelPreviewDataWrapper::id( 0x80101 );
const MString VoxelPreviewDataWrapper::typeName( "VoxelPreviewData" );

MObject     VoxelShape::voxelData;
MObject     VoxelShape::outData;

//////////////////////////////////////////////////////////////////////
//
// Error checking
//
//    MCHECKERROR       - check the status and print the given error message
//    MCHECKERRORNORET  - same as above but does not return
//
//////////////////////////////////////////////////////////////////////

#define MCHECKERROR(STAT,MSG)       \
	if ( MS::kSuccess != STAT ) {   \
	cerr << MSG << endl;        \
	return MS::kFailure;    \
	}

#define MCHECKERRORNORET(STAT,MSG)  \
	if ( MS::kSuccess != STAT ) {   \
	cerr << MSG << endl;        \
	}


//////////////////////////////////////////////////////////////////////////
// VoxelShape::geometryData (override)
//
// Returns the data object for the surface. This gets
// called internally for grouping (set) information.
//////////////////////////////////////////////////////////////////////////

MObject VoxelShape::geometryData() const {
	VoxelShape* nonConstThis = const_cast<VoxelShape*>(this);
	MDataBlock datablock = nonConstThis->forceCache();
	MDataHandle handle = datablock.inputValue( outData );
	return handle.data();
}


//////////////////////////////////////////////////////////////////////////
// VoxelShape::compute (override)
//
//	Description:
//		This method computes the value of the given output plug based
//		on the values of the input attributes.
//
//	Arguments:
//		plug - the plug to compute
//		data - object that provides access to the attributes for this node
////////////////////////////////////////////////////////////////////////////

MStatus VoxelShape::compute( const MPlug& plug, MDataBlock& data ) {
	MStatus stat;

	// Check which output attribute we have been asked to compute.  If this 
	// node doesn't know how to compute it, we must return 
	// MS::kUnknownParameter.
	// 
	if( plug == outData )
	{
		// Get a handle to the input attribute that we will need for the
		// computation.  If the value is being supplied via a connection 
		// in the dependency graph, then this call will cause all upstream  
		// connections to be evaluated so that the correct value is supplied.
		// 
		MDataHandle inputDataHandle = data.inputValue( voxelData, &stat );

		MObject inputDataObj = inputDataHandle.data();
		
		MFnPluginData fnDataCreator;
		MTypeId tmpid( VoxelPreviewDataWrapper::id );
		VoxelPreviewDataWrapper * newData = NULL;

		MDataHandle outHandle = data.outputValue( outData );	
		newData = (VoxelPreviewDataWrapper*)outHandle.asPluginData();

		if ( newData == NULL ) {
			// Create some output data
			fnDataCreator.create( tmpid, &stat );
			MCHECKERROR( stat, "compute : error creating VoxelPreviewDataWrapper")
				newData = (VoxelPreviewDataWrapper*)fnDataCreator.data( &stat );
			MCHECKERROR( stat, "compute : error getting proxy VoxelPreviewDataWrapper object")
		}

		// compute the output values			
		MFnPointArrayData inputData;
		inputData.setObject(inputDataObj);
		MPointArray voxels = inputData.array();
		newData->reset( new VoxelPreviewData( voxels ) );
		bounds = newData->getData()->bounds();

		#if 0 // enable to dump voxels to Maya
		{
			// WARNING: unlike the voxels preview using OpenGL, which is
			// handled efficiently by the GPU, this code will actually dump 
			// potentially thousands of objects into the Maya scene, and that 
			// will likely make Maya unhappy and crashy - use with caution with 
			// low voxel densities.

			char cmd[ 128];
			for( unsigned int i = 0; i < voxels.length(); i += 2 ) {
				MPoint bbMin = voxels[ i ];
				MPoint bbMax = voxels[ i + 1 ];
				MVector extents = bbMax - bbMin;
				MPoint center = ( bbMax + bbMin ) * 0.5f;
				sprintf_s( cmd, 128, "$c = `polyCube -ch on -o on -w %f -h %f -d %f`; move -a %f %f %f $c;", extents.x, extents.y, extents.z, center.x, center.y, center.z );
				MGlobal::executeCommand( cmd );
			}
		}
		#endif

		// Assign the new data to the outputSurface handle

		if ( newData != outHandle.asPluginData() ) {
			outHandle.set( newData );
		}

	} else {
		return MS::kUnknownParameter;
	}

	data.setClean( plug );

	return MS::kSuccess;
}


//////////////////////////////////////////////////////////////////////////
// VoxelShape::meshDataRef
//
//	Get a reference to the mesh data (aoMeshData)
//	from the datablock. If dirty then an evaluation is
//	triggered.
////////////////////////////////////////////////////////////////////////////

MObject VoxelShape::meshDataRef() {
	// Get the datablock for this node
	//
	MDataBlock datablock = forceCache();

	// Calling inputValue will force a recompute if the
	// connection is dirty. This means the most up-to-date
	// mesh data will be returned by this method.
	//
	MDataHandle handle = datablock.inputValue( outData );
	return handle.data();
}



//////////////////////////////////////////////////////////////////////////
// VoxelShape::getData
//
// Returns a pointer to an updated version of the internal data
////////////////////////////////////////////////////////////////////////////

VoxelPreviewData* VoxelShape::getData() {
	MStatus stat;
	VoxelPreviewData * result = NULL;

	MObject tmpObj = meshDataRef();
	MFnPluginData fnData( tmpObj );
	VoxelPreviewDataWrapper * data = (VoxelPreviewDataWrapper*)fnData.data( &stat );
	MCHECKERRORNORET( stat, "getData : Failed to get VoxelPreviewDataWrapper");

	if ( NULL != data ) {
		result = data->getData();
	}

	return result;
}


//////////////////////////////////////////////////////////////////////////
// VoxelShape::creator
//
//	This method exists to give Maya a way to create new objects
//	of this type. 
////////////////////////////////////////////////////////////////////////////

void* VoxelShape::creator() {
	return new VoxelShape();
}

//////////////////////////////////////////////////////////////////////////
// VoxelShape::creator
//
//	This method is called to create and initialize all of the attributes
//	and attribute dependencies for this node type.  This is only called
//	once when the node type is registered with Maya.
////////////////////////////////////////////////////////////////////////////

MStatus VoxelShape::initialize() {
	MFnTypedAttribute	typedAttr;
	MFnNumericAttribute nAttr;
	MStatus				stat;

	// Input attributes

	MPointArray defaultPointArray;
	MFnPointArrayData pointArrayDataFn;
	pointArrayDataFn.create( defaultPointArray );

	voxelData = typedAttr.create( "voxelData", "vd", MFnData::kPointArray, pointArrayDataFn.object() );
	typedAttr.setWritable( true );
	typedAttr.setReadable( true );

	outData = typedAttr.create( "output", "out", VoxelPreviewDataWrapper::id );
	typedAttr.setWritable( false );
	typedAttr.setStorable(false);
	typedAttr.setHidden( true );

	// Add the attributes to the node

	addAttribute( voxelData );
	addAttribute( outData );

	// Set the attribute dependencies
	attributeAffects( voxelData, outData );
	
	return MS::kSuccess;

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

VoxelPreviewData::VoxelPreviewData( const MPointArray& points ) : references(0) {

	listId = glGenLists( 1 );

	boundingBox.clear();

	float bbMin[3];
	float bbMax[3];

	glNewList(listId, GL_COMPILE);
	glBegin(GL_QUADS);

	for( unsigned int i = 0; i < points.length(); i += 2 ) {

		boundingBox.expand( points[ i ] );
		boundingBox.expand( points[ i + 1 ] );

		bbMin[ 0 ] = (float)points[ i ].x;
		bbMin[ 1 ] = (float)points[ i ].y;
		bbMin[ 2 ] = (float)points[ i ].z;
		bbMax[ 0 ] = (float)points[ i + 1 ].x;
		bbMax[ 1 ] = (float)points[ i + 1 ].y;
		bbMax[ 2 ] = (float)points[ i + 1 ].z;

		// Bottom Face
		glTexCoord2f( 1.0f, 1.0f ); glVertex3f( bbMin[ 0 ], bbMin[ 1 ], bbMin[ 2 ] );	// Top Right Of The Texture and Quad
		glTexCoord2f( 0.0f, 1.0f ); glVertex3f( bbMax[ 0 ], bbMin[ 1 ], bbMin[ 2 ] );	// Top Left Of The Texture and Quad
		glTexCoord2f( 0.0f, 0.0f ); glVertex3f( bbMax[ 0 ], bbMin[ 1 ], bbMax[ 2 ] );	// Bottom Left Of The Texture and Quad
		glTexCoord2f( 1.0f, 0.0f ); glVertex3f( bbMin[ 0 ], bbMin[ 1 ], bbMax[ 2 ] );	// Bottom Right Of The Texture and Quad
		// Front Face
		glTexCoord2f( 0.0f, 0.0f ); glVertex3f( bbMin[ 0 ], bbMin[ 1 ], bbMax[ 2 ] );	// Bottom Left Of The Texture and Quad
		glTexCoord2f( 1.0f, 0.0f ); glVertex3f( bbMax[ 0 ], bbMin[ 1 ], bbMax[ 2 ] );	// Bottom Right Of The Texture and Quad
		glTexCoord2f( 1.0f, 1.0f ); glVertex3f( bbMax[ 0 ], bbMax[ 1 ], bbMax[ 2 ] );	// Top Right Of The Texture and Quad
		glTexCoord2f( 0.0f, 1.0f ); glVertex3f( bbMin[ 0 ], bbMax[ 1 ], bbMax[ 2 ] );	// Top Left Of The Texture and Quad
		// Back Face
		glTexCoord2f( 1.0f, 0.0f ); glVertex3f( bbMin[ 0 ], bbMin[ 1 ], bbMin[ 2 ] );	// Bottom Right Of The Texture and Quad
		glTexCoord2f( 1.0f, 1.0f ); glVertex3f( bbMin[ 0 ], bbMax[ 1 ], bbMin[ 2 ] );	// Top Right Of The Texture and Quad
		glTexCoord2f( 0.0f, 1.0f ); glVertex3f( bbMax[ 0 ], bbMax[ 1 ], bbMin[ 2 ] );	// Top Left Of The Texture and Quad
		glTexCoord2f( 0.0f, 0.0f ); glVertex3f( bbMax[ 0 ], bbMin[ 1 ], bbMin[ 2 ] );	// Bottom Left Of The Texture and Quad
		// Right face
		glTexCoord2f( 1.0f, 0.0f ); glVertex3f( bbMax[ 0 ], bbMin[ 1 ], bbMin[ 2 ] );	// Bottom Right Of The Texture and Quad
		glTexCoord2f( 1.0f, 1.0f ); glVertex3f( bbMax[ 0 ], bbMax[ 1 ], bbMin[ 2 ] );	// Top Right Of The Texture and Quad
		glTexCoord2f( 0.0f, 1.0f ); glVertex3f( bbMax[ 0 ], bbMax[ 1 ], bbMax[ 2 ] );	// Top Left Of The Texture and Quad
		glTexCoord2f( 0.0f, 0.0f ); glVertex3f( bbMax[ 0 ], bbMin[ 1 ], bbMax[ 2 ] );	// Bottom Left Of The Texture and Quad
		// Left Face
		glTexCoord2f( 0.0f, 0.0f ); glVertex3f( bbMin[ 0 ], bbMin[ 1 ], bbMin[ 2 ] );	// Bottom Left Of The Texture and Quad
		glTexCoord2f( 1.0f, 0.0f ); glVertex3f( bbMin[ 0 ], bbMin[ 1 ], bbMax[ 2 ] );	// Bottom Right Of The Texture and Quad
		glTexCoord2f( 1.0f, 1.0f ); glVertex3f( bbMin[ 0 ], bbMax[ 1 ], bbMax[ 2 ] );	// Top Right Of The Texture and Quad
		glTexCoord2f( 0.0f, 1.0f ); glVertex3f( bbMin[ 0 ], bbMax[ 1 ], bbMin[ 2 ] );	// Top Left Of The Texture and Quad
	}

	glEnd();	
	glEndList();
}

//////////////////////////////////////////////////////////////////////////
// VoxelPreviewDataWrapper::copy (override)
//////////////////////////////////////////////////////////////////////////

void VoxelPreviewDataWrapper::copy( const MPxData& other ) {
	if ( other.typeId() == typeId() ) {
		releaseData();
		data = ((const VoxelPreviewDataWrapper &)other).data;
		data->incRef();	
	}
}


//////////////////////////////////////////////////////////////////////////
// MeshData::creator
//
//	This method exists to give Maya a way to create new objects
//	of this type. 
//////////////////////////////////////////////////////////////////////////
void * VoxelPreviewDataWrapper::creator() {
	return new VoxelPreviewDataWrapper();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void VoxelPreviewData::draw() const {
	glCallList( listId );
}

void VoxelPreviewData::destroy() {
	assert( references == 0 );
	glDeleteLists(listId, 1);
}