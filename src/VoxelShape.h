/* 
	/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the MIT license: http://www.opensource.org/licenses/MIT	
	================================================================================
*/

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.h>

#include <maya/MPxNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MTypeId.h> 
#include <maya/MPxSurfaceShape.h>
#include <maya/MBoundingBox.h>

#include <maya/MPxGeometryData.h>
#include <maya/MTypeId.h>
#include <maya/MString.h>

class MPointArray;

/* ==========================================
	
	Class VoxelShape

	Helper class used to preview the voxels provided in the 'voxelData'
	attribute. A different class, VoxelShapeUI will be in charge of 
	displaying the results in the viewports.

========================================== */

class VoxelPreviewData;

class VoxelShape : public MPxSurfaceShape
{
public:
						VoxelShape() {}
	virtual				~VoxelShape() {}

	// overrides

	virtual MStatus			compute( const MPlug& plug, MDataBlock& data );

	virtual bool			isBounded() const { return true;}
	virtual MBoundingBox	boundingBox() const { return bounds; }

	virtual MObject			localShapeOutAttr() const { return outData; }
	virtual MObject			geometryData() const;

	// methods

	MObject					meshDataRef();
	VoxelPreviewData*		getData();


	static  void*		creator();
	static  MStatus		initialize();
public:
	static const MString typeName;
	static const MTypeId id;

	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
	static MObject		voxelData;	// input voxel data
	static MObject		outData;	// output data

private:
	MBoundingBox		bounds;
	
};

/* ==========================================
	
	Class VoxelPreviewData

	Encapsulates precalculated OpenGL data generated from
	the input attribute that will be used from the UI class

========================================== */

class VoxelPreviewData {
public:
	explicit VoxelPreviewData( const MPointArray& points );

	void destroy();
	void draw() const;

	inline void incRef() { references++; }
	inline int decRef() { references--; return references; }
	MBoundingBox bounds() const { return boundingBox; }
private:
	GLuint listId;

	// maya will make copies of this object, but in order to prevent deleting
	// the GL data whenever the destructor is called, we'll carry a reference counting
	// mechanism
	int references;

	MBoundingBox boundingBox;

};

/* ==========================================
	
	Class VoxelPreviewDataWrapper

	Shared data pointer wrapper for passing resources along the DAG

========================================== */

class VoxelPreviewDataWrapper : public MPxGeometryData {
public:
	explicit VoxelPreviewDataWrapper() : data( NULL ) {}
	explicit VoxelPreviewDataWrapper( const MPointArray& points ) {
		data = new VoxelPreviewData( points );
		data->incRef();
	}

	// copy constructor
	VoxelPreviewDataWrapper( const VoxelPreviewDataWrapper& other ) {
		copy(other);	
	}

	virtual ~VoxelPreviewDataWrapper() { 
		releaseData();
	}

	VoxelPreviewData* getData() { return data; }

	void reset( VoxelPreviewData* dataPtr ) {
		releaseData();
		data = dataPtr;
		if ( data ) data->incRef();
	}

	// overrides 

	virtual	void			copy ( const MPxData& );

	virtual MTypeId         typeId() const { return id; }
	virtual MString         name() const { return typeName; }

	static void * creator();

public:

	static const MString typeName;
	static const MTypeId id;

private:

	void releaseData() {
		if( data != NULL && data->decRef() == 0 ) {
			delete data;
		}
		data = NULL;
	}

	VoxelPreviewData* data;
};
