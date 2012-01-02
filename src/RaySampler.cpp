/* 
	================================================================================
	Copyright (c) 2011, Jose Esteve. http://www.joesfer.com
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

#include "RaySampler.h"

#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MPointArray.h>
#include <maya/MFnPointArrayData.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFnMesh.h>
#include <maya/MGlobal.h>
#include <maya/MBoundingBox.h>
#include <maya/MMatrix.h>
#include <maya/MFloatMatrix.h>
#include <maya/MFloatArray.h>

#include <assert.h>
MTypeId     RaySampler::id( 0x83100 );

// Attributes
MObject		RaySampler::numSamples;
MObject     RaySampler::mesh;        
MObject     RaySampler::outSamples;

RaySampler::RaySampler() {}
RaySampler::~RaySampler() {}

MStatus RaySampler::compute( const MPlug& plug, MDataBlock& data )
//
//	Description:
//		This method computes the value of the given output plug based
//		on the values of the input attributes.
//
//	Arguments:
//		plug - the plug to compute
//		data - object that provides access to the attributes for this node
//
{
	MStatus returnStatus;

	// Check which output attribute we have been asked to compute.  If this 
	// node doesn't know how to compute it, we must return 
	// MS::kUnknownParameter.
	// 
	if ( plug == outSamples ) {

		// Read the input value from the handle.
		//
		int numSamples = data.inputValue( RaySampler::numSamples ).asInt();
		// by querying the voxels as input value we ensure the attribute is evaluated
		// if necessary and we're getting an up-to-date copy
		MFnMesh inMesh( data.inputValue( mesh ).asMesh() );

		// Get a handle to the output attribute.  This is similar to the
		// "inputValue" call above except that no dependency graph 
		// computation will be done as a result of this call.
		// 
		MFnPointArrayData samplesHandle( data.outputValue( RaySampler::outSamples ).data() );
		MPointArray samples = samplesHandle.array();

		Sample( inMesh, numSamples, samples );

	} else {
		return MS::kUnknownParameter;
	}

	// Mark the destination plug as being clean.  This will prevent the
	// dependency graph from repeating this calculation until an input 
	// of this node changes.
	// 
	data.setClean(plug);

	return MS::kSuccess;
}

void* RaySampler::creator()
//
//	Description:
//		this method exists to give Maya a way to create new objects
//      of this type. 
//
//	Return Value:
//		a new object of this type
//
{
	return new RaySampler();
}

MStatus RaySampler::initialize()
//
//	Description:
//		This method is called to create and initialize all of the attributes
//      and attribute dependencies for this node type.  This is only called 
//		once when the node type is registered with Maya.
//
//	Return Values:
//		MS::kSuccess
//		MS::kFailure
//		
{
	MFnTypedAttribute	tAttr;
	MFnNumericAttribute nAttr;
	MStatus				stat;

	numSamples = nAttr.create( "sampleCount", "sc", MFnNumericData::kInt, 100, &stat );
	if ( !stat ) return stat;
	nAttr.setMin( 1 );
	nAttr.setWritable( true );
	nAttr.setStorable( true );

	mesh = tAttr.create( "inputMesh", "in", MFnData::kMesh, MObject::kNullObj, &stat );
	if ( !stat ) return stat;
	tAttr.setWritable( true );
	tAttr.setStorable( false );
	tAttr.setHidden( true );

	MFnPointArrayData pCreator;
	MObject pa = pCreator.create();	
	outSamples = tAttr.create( "outSamples", "os", MFnData::kPointArray, pa, &stat );
	if ( !stat ) return stat;
	// Attribute is read-only because it is an output attribute
	tAttr.setWritable(false);
	// Attribute will not be written to files when this type of node is stored
	tAttr.setStorable(false);
	tAttr.setCached( false );// allow us to query it as often as we want

	// Add the attributes we have created to the node
	//
	addAttribute( numSamples );
	addAttribute( mesh );
	addAttribute( outSamples );

	// Set up a dependency between the input and the output.  This will cause
	// the output to be marked dirty when the input changes.  The output will
	// then be recomputed the next time the value of the output is requested.
	//
	attributeAffects( numSamples, outSamples );
	attributeAffects( mesh, outSamples );

	return MS::kSuccess;
}

static inline float random01() { return (float)rand() / RAND_MAX; }

void RaySampler::Sample( MFnMesh& mesh, int numSamples, MPointArray& samples ) {

	samples.clear();

	MStatus stat;

	MFloatPoint rayOrigin, rayEnd;

	// calculate mesh bounds
	MBoundingBox bounds;
	bounds.clear();
	{
		MPointArray points;
		mesh.getPoints( points, MSpace::kWorld );
		for( unsigned int i = 0; i < points.length(); i++ ) {
			bounds.expand( points[ i ] );
		}

		// expand the bounds slightly to avoid touching the mesh faces
		
		MBoundingBox expanded( MPoint( bounds.min().x - 1, bounds.min().y - 1, bounds.min().z - 1 ),
							   MPoint( bounds.max().x + 1, bounds.max().y + 1, bounds.max().z + 1 ) );
		bounds = expanded; 
	}
	

	const float boundsWidth = (float)bounds.width();
	const float boundsHeight = (float)bounds.height();
	const float boundsDepth = (float)bounds.depth();
	const MFloatPoint boundsMin( (float)bounds.min().x, (float)bounds.min().y, (float)bounds.min().z );
	
	// use a common accelerator params object between raytrace calls
	// so Maya reuses the internal accelerator structure
	MMeshIsectAccelParams accelerator = mesh.autoUniformGridParams();

	// Trace random rays between opposed pairs of faces and produce samples along each entry/exit segment

	float volume = (float)(bounds.width() * bounds.height() * bounds.depth());
	float linearDensity = std::max( 1e-4f,  (float)numSamples / volume );
	int maxSamplesPerRay = std::max( 1, (int)powf( volume, 1.0f / 3.0f ) ) >> 1;
	
	while( (int)samples.length() < numSamples ) {

		int boxFace = random() % 6;
		switch( boxFace ) {
			case 0:
				rayOrigin = boundsMin + MFloatPoint( 0, random01() * boundsHeight, random01() * boundsDepth );
				rayEnd = boundsMin + MFloatPoint( boundsWidth, random01() * boundsHeight, random01() * boundsDepth );
				break;
			case 1:
				rayOrigin = boundsMin + MFloatPoint( boundsWidth, random01() * boundsHeight, random01() * boundsDepth );
				rayEnd = boundsMin + MFloatPoint( 0, random01() * boundsHeight, random01() * boundsDepth );
				break;
			case 2:
				rayOrigin = boundsMin + MFloatPoint( random01() * boundsWidth, 0, random01() * boundsDepth );
				rayEnd = boundsMin + MFloatPoint( random01() * boundsWidth, boundsHeight, random01() * boundsDepth );
				break;
			case 3:
				rayOrigin = boundsMin + MFloatPoint( random01() * boundsWidth, boundsHeight, random01() * boundsDepth );
				rayEnd = boundsMin + MFloatPoint( random01() * boundsWidth, 0, random01() * boundsDepth );
				break;
			case 4:
				rayOrigin = boundsMin + MFloatPoint( random01() * boundsWidth, random01() * boundsHeight, 0 );
				rayEnd = boundsMin + MFloatPoint( random01() * boundsWidth, random01() * boundsHeight, boundsDepth );
				break;
			case 5:
				rayOrigin = boundsMin + MFloatPoint( random01() * boundsWidth, random01() * boundsHeight, boundsDepth );
				rayEnd = boundsMin + MFloatPoint( random01() * boundsWidth, random01() * boundsHeight, 0 );
				break;
			default: break;
		}

		MFloatVector rayDir = rayEnd - rayOrigin;
		
		MFloatPointArray hitPoints;
		mesh.allIntersections( rayOrigin, rayDir, 
							   NULL, NULL, 
							   false, 
							   MSpace::kWorld, 
							   1.0,
							   false,
							   &accelerator, 
							   true, // sort hits
							   hitPoints,
							   NULL,
							   NULL,
							   NULL,
							   NULL, NULL );

		if ( hitPoints.length() < 2 ) {
			continue;
		}

		for( unsigned int i = 0; i < hitPoints.length() - 1; i += 2 ) {
			const MFloatPoint segmentBegin = hitPoints[ i ];
			const MFloatPoint segmentEnd = hitPoints[ i + 1 ];
			const float length = segmentBegin.distanceTo(segmentEnd);
			const int ns = std::min( maxSamplesPerRay, (int)ceil( length * linearDensity ) );
			const MFloatVector dir = segmentEnd - segmentBegin;
			for( int j = 0; j < ns; j++ ) {
				MPoint sample = segmentBegin + random01() * dir;
				samples.append( sample );
			}
		}
	}
	mesh.freeCachedIntersectionAccelerator();
}