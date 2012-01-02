/* 
	/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the MIT license: http://www.opensource.org/licenses/MIT	
	================================================================================
*/

#pragma once

#include <maya/MPxNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MTypeId.h> 
#include <maya/MPointArray.h>
#include <maya/MFnMesh.h>

 
/* ==========================================
	Class VoxelSampler

	Samples the input mesh plugged to its 'mesh' attribute
	by voxelizing with a resolution set by 'voxelRes' and 
	generating points within each voxels.

	The samples are provided as a MFnPointArray in the 
	'outVoxels' output attribute. Additionally the voxels
	can be retrieved from the 'outVoxels' attribute as 
	a point array where each pair of points describes the
	min and max points of an axis-aligned voxel.
		
========================================== */

class VoxelSampler : public MPxNode
{
public:
						VoxelSampler();
	virtual				~VoxelSampler(); 

	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );

	static  void*		creator();
	static  MStatus		initialize();

public:

	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
	static MObject  voxelRes;
	static MObject  numSamples;
	static MObject  mesh;        
	static MObject	outVoxels;
	static MObject	outSamples;

	// The typeid is a unique 32bit identifier that describes this node.
	// It is used to save and retrieve nodes of this type from the binary
	// file format.  If it is not unique, it will cause file IO problems.
	//
	static	MTypeId		id;

private:

	static bool Voxelize( const MFnMesh& inMesh, int resX, int resY, int resZ, 
						  MPointArray& voxels );

	static bool SampleVoxels( const MPointArray& voxels, int numSamples,
							  MPointArray& samples );

};
