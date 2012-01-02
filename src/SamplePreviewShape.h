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
#include <maya/MPxSurfaceShape.h>
#include <maya/MBoundingBox.h>
#include <maya/MPointArray.h>
#include <maya/MPxGeometryData.h>
#include <maya/MTypeId.h>
#include <maya/MString.h>

class MPointArray;


/////////////////////////////////////////////////////////////////////
//
// class SampleShape
//
//	Implements the custom shape generation. A different class, 
//  SampleShapeUI will be in charge of displaying the results 
//  in the viewports.
// 
/////////////////////////////////////////////////////////////////////

class SamplePreviewData;

/* ==========================================
	Class SampleShape

	Helper node used to preview the sample positions.
	Plug a MFnPointArray output attribute from the
	samplers to 'sampleData' and trigger the evaluation
	of 'outData'
   ========================================== */

class SampleShape : public MPxSurfaceShape
{
public:
						SampleShape() {}
	virtual				~SampleShape() {}

	// overrides

	virtual MStatus			compute( const MPlug& plug, MDataBlock& data );

	virtual bool			isBounded() const { return true;}
	virtual MBoundingBox	boundingBox() const { return bounds; }

	virtual MObject			localShapeOutAttr() const { return outData; }
	virtual MObject			geometryData() const;

	// methods

	MObject					meshDataRef();
	SamplePreviewData*		getData();


	static  void*		creator();
	static  MStatus		initialize();
public:
	static const MString typeName;
	static const MTypeId id;

	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
	static MObject		sampleData;	// input sample data
	static MObject		outData;	// output data

private:
	MBoundingBox		bounds;
	
};


/////////////////////////////////////////////////////////////////////
//
// class SamplePreviewData
//
/////////////////////////////////////////////////////////////////////

class SamplePreviewData : public MPxGeometryData {
public:

	MPointArray& getSamples() { return samples; }

	// overrides 

	virtual	void			copy ( const MPxData& );

	virtual MTypeId         typeId() const { return id; }
	virtual MString         name() const { return typeName; }

	static void * creator();

public:

	static const MString typeName;
	static const MTypeId id;

	MPointArray  samples;
};
