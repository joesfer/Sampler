/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/

#pragma once


#include <maya/MPxNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MTypeId.h> 
#include <maya/MPointArray.h>
#include <maya/MFnMesh.h>

/* ==========================================
	Class RaySampler

	Implements a volume sampler by using raymarching. 
	Requires a polygonal mesh to be connected to it's inMesh
	attribute, and outputs a MFnPointArray with the sample
	locations.

   ========================================== */

class RaySampler : public MPxNode
{
public:
	RaySampler();
	virtual				~RaySampler(); 

	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );

	static  void*		creator();
	static  MStatus		initialize();

public:

	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
	static MObject  numSamples;
	static MObject  mesh;        
	static MObject	outSamples;

	// The typeid is a unique 32bit identifier that describes this node.
	// It is used to save and retrieve nodes of this type from the binary
	// file format.  If it is not unique, it will cause file IO problems.
	//
	static	MTypeId		id;

private:

	void Sample( MFnMesh& mesh, int numSamples, MPointArray& samples );

};
