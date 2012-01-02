/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	All rights reserved. 

	Redistribution and use in source and binary forms, with or without modification, 
	are permitted provided that the following conditions are met: 

	* Redistributions of source code must retain the above copyright notice, this 
	  list of conditions and the following disclaimer. 
	
	* Redistributions in binary form must reproduce the above copyright notice, 
	  this list of conditions and the following disclaimer in the documentation 
	  and/or other materials provided with the distribution. 
	
	* Neither the name of the organization nor the names of its contributors may 
	  be used to endorse or promote products derived from this software without 
	  specific prior written permission. 

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR 
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
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
