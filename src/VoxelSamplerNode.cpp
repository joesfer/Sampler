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


#include "VoxelSamplerNode.h"

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

#include <assert.h>
#include <vector>

#include <gl/glew.h>
#include <gl/GL.h>
#include <gl/GLU.h>

#include <time.h>

//
MTypeId     VoxelSampler::id( 0x83099 );

// Attributes
MObject		VoxelSampler::voxelRes;
MObject		VoxelSampler::numSamples;
MObject     VoxelSampler::mesh;        
MObject     VoxelSampler::outVoxels;
MObject     VoxelSampler::outSamples;

VoxelSampler::VoxelSampler() {}
VoxelSampler::~VoxelSampler() {}

MStatus VoxelSampler::compute( const MPlug& plug, MDataBlock& data )
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
	if( plug == outVoxels )
	{
		// Read the input value from the handle.
		//
		int3& numVoxels = data.inputValue( VoxelSampler::voxelRes ).asInt3();
		MFnMesh inMesh( data.inputValue( mesh ).asMesh() );

		// Get a handle to the output attribute.  This is similar to the
		// "inputValue" call above except that no dependency graph 
		// computation will be done as a result of this call.
		// 
		MFnPointArrayData voxelsHandle( data.outputValue( VoxelSampler::outVoxels ).data() );
		MPointArray voxels = voxelsHandle.array();
	
		Voxelize( inMesh, numVoxels[ 0 ], numVoxels[ 1 ], numVoxels[ 2 ], voxels );

	} if ( plug == outSamples ) {

		// Read the input value from the handle.
		//
		int numSamples = data.inputValue( VoxelSampler::numSamples ).asInt();
		// by querying the voxels as input value we ensure the attribute is evaluated
		// if necessary and we're getting an up-to-date copy
		MFnPointArrayData voxelsHandle( data.inputValue( VoxelSampler::outVoxels ).data() );
		MPointArray voxels = voxelsHandle.array();

		// Get a handle to the output attribute.  This is similar to the
		// "inputValue" call above except that no dependency graph 
		// computation will be done as a result of this call.
		// 
		MFnPointArrayData samplesHandle( data.outputValue( VoxelSampler::outSamples ).data() );
		MPointArray samples = samplesHandle.array();

		SampleVoxels( voxels, numSamples, samples );

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

void* VoxelSampler::creator()
//
//	Description:
//		this method exists to give Maya a way to create new objects
//      of this type. 
//
//	Return Value:
//		a new object of this type
//
{
	return new VoxelSampler();
}

MStatus VoxelSampler::initialize()
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


	voxelRes = nAttr.create( "voxelResolution", "vr", MFnNumericData::k3Int, 16, &stat );
	if ( !stat ) return stat;
	nAttr.setMin( 1 );
	nAttr.setMax( 32 );
	nAttr.setWritable( true );
	nAttr.setStorable( true );

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

	{
		MFnPointArrayData pCreator;
		MObject pa = pCreator.create();
		outVoxels = tAttr.create( "outVoxels", "ov", MFnData::kPointArray, pa, &stat );
		if ( !stat ) return stat;
		// Attribute is read-only because it is an output attribute
		tAttr.setWritable(false);
		// Attribute will not be written to files when this type of node is stored
		tAttr.setStorable(false);
		tAttr.setCached( false );// allow us to query it as often as we want
	}

	{
		MFnPointArrayData pCreator;
		MObject pa = pCreator.create();
		outSamples = tAttr.create( "outSamples", "os", MFnData::kPointArray, pa, &stat );
		if ( !stat ) return stat;
		// Attribute is read-only because it is an output attribute
		tAttr.setWritable(false);
		// Attribute will not be written to files when this type of node is stored
		tAttr.setStorable(false);
		tAttr.setCached( false );// allow us to query it as often as we want
	}

	// Add the attributes we have created to the node
	//
	addAttribute( voxelRes );
	addAttribute( numSamples );
	addAttribute( mesh );
	addAttribute( outVoxels );
	addAttribute( outSamples );

	// Set up a dependency between the input and the output.  This will cause
	// the output to be marked dirty when the input changes.  The output will
	// then be recomputed the next time the value of the output is requested.
	//
	attributeAffects( voxelRes, outSamples );
	attributeAffects( voxelRes, outVoxels );
	attributeAffects( numSamples, outSamples );
	attributeAffects( mesh, outSamples );
	attributeAffects( mesh, outVoxels );



	return MS::kSuccess;
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

bool VoxelSampler::Voxelize( const MFnMesh& mesh, int resX, int resY, int resZ, MPointArray& voxels ) {
	
	// This method is an implementation of the paper "Single-Pass GPU Solid 
	// Voxelization for Real-Time Applications"
	// http://hal.inria.fr/docs/00/34/52/91/PDF/solidvoxelizationAuthorVersion.pdf
	//
	// The idea is to render the provided mesh through an orthographic view fitted
	// around the bounding box and, for each fragment, packing the depth information
	// on the color components, and accumulating the results in the framebuffer using
	// a XOR bitwise operator in the blend mode. The resulting image will contain
	// a row of voxels for each x,y pixel, packed in the color bits. To reconstruct
	// the voxels from there, we just have to check which bits are set to 1 and
	// dump a voxel (given as a pair of min/max points) to the output array.

	// Note the implementation of this method is self-contained and therefore
	// we're allocating and deallocating resources each time we voxelize. This
	// should be refactored in case of a continuous voxelization.

	glewInit();

	// clamp to the limits of this implementation 
	// (128 bits as 4 x 32 bit color channels)
	resX = std::max( 1, std::min( 128, resX ) );
	resY = std::max( 1, std::min( 128, resY ) );
	resZ = std::max( 1, std::min( 128, resZ ) );

	struct Vertex {
		float x,y,z,w;
	};
	typedef GLuint Index;
	int numIndices = 0;

	// create resources
	GLuint vbo, ibo;
	GLuint renderTarget;
	GLuint program, vs, fs;
	GLuint bitmaskTex;
	GLuint fbo, rbo;

	MBoundingBox bounds;

	{ // create shader program

		{ // Vertex shader
			vs  = glCreateShader( GL_VERTEX_SHADER );

			const char* shader = "varying float depth; \r\n\
								 uniform float nearClipPlane;\r\n\
								 uniform float farClipPlane;\r\n\
								 void main() { \r\n\
									gl_Position = ftransform(); \r\n\
									vec4 transformed = gl_ModelViewMatrix * gl_Vertex;\r\n\
									depth = (-transformed.z / transformed.w ) / ( farClipPlane - nearClipPlane );\r\n\
								 }";
			glShaderSource( vs, 1, &shader, NULL );
			glCompileShader( vs );

			{
				char log[512];
				GLsizei loglength;
				glGetShaderInfoLog( vs, 512, &loglength, log );
				if ( loglength > 0 ) {
					std::cerr << "OpenGL shader compiler: " << log << std::endl;
				}
			}
		}

		{ // pixel shader

			fs  = glCreateShader( GL_FRAGMENT_SHADER );

			const char* shader = "uniform sampler1D bitmask; \r\n\
								 varying float depth; \r\n\
								 void main() { \r\n\
									gl_FragColor = texture1D( bitmask, depth );\r\n\
								  }";
			glShaderSource( fs, 1, &shader, NULL );
			glCompileShader( fs );

			{
				char log[512];
				GLsizei loglength;
				glGetShaderInfoLog( fs, 512, &loglength, log );
				if ( loglength > 0 ) {
					std::cerr << "OpenGL shader compiler: " << log << std::endl;
				}
			}
		}
		
		program = glCreateProgram();
		glAttachShader( program, vs );
		glAttachShader( program, fs );
		
		glLinkProgram( program );
	}

	{ // Generate destination texture and frame buffer object

		glGenTextures( 1, &renderTarget );
		glBindTexture( GL_TEXTURE_2D, renderTarget );
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, resX, resY, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, NULL );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );	
		glBindTexture( GL_TEXTURE_2D, 0 );

		// create a framebuffer object, you need to delete them when program exits.
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		// create a renderbuffer object to store depth info
		// NOTE: A depth renderable image should be attached the FBO for depth test.
		// If we don't attach a depth renderable image to the FBO, then
		// the rendering output will be corrupted because of missing depth test.
		// If you also need stencil test for your rendering, then you must
		// attach additional image to the stencil attachement point, too.
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, resX, resY);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// attach a texture to FBO color attachment point
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget, 0);

		// attach a renderbuffer to depth attachment point
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo );


		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(status != GL_FRAMEBUFFER_COMPLETE ){
			std::cerr << "error setting up frame buffer object" << std::endl;
		}
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}

	// map Maya mesh to OpenGL
	{
		{ // copy vertices

			glGenBuffers( 1, &vbo );
			glBindBuffer( GL_ARRAY_BUFFER, vbo );
			glBufferData( GL_ARRAY_BUFFER, mesh.numVertices() * sizeof( Vertex ), NULL, GL_STATIC_DRAW );
			Vertex* glVertices = (Vertex*)glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
			if ( glVertices == NULL ) return false;

			MFloatPointArray vertices;
			mesh.getPoints( vertices, MSpace::kWorld );		

			bounds.clear();

			for( unsigned int i = 0; i < vertices.length(); i++ ) {
				MFloatPoint& worldPoint =  vertices[ i ];
				glVertices[ i ].x =  worldPoint.x;
				glVertices[ i ].y =  worldPoint.y;
				glVertices[ i ].z =  worldPoint.z;
				glVertices[ i ].w =  worldPoint.w;
				bounds.expand( worldPoint );
			}

			// commit data
			glUnmapBuffer( GL_ARRAY_BUFFER );
			
		}

		{ // copy indices
			MIntArray triangleCounts, triVertices;
			mesh.getTriangles( triangleCounts, triVertices );

			glGenBuffers( 1, &ibo );
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
			numIndices = (int)triVertices.length();
			glBufferData( GL_ELEMENT_ARRAY_BUFFER, numIndices * 3 * sizeof( Index ), NULL, GL_STATIC_DRAW );
			Index* indices = (Index*)glMapBuffer( GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY );

			for( unsigned int i = 0; i < triVertices.length(); i++ ) {
				indices[ i ] = triVertices[ i ];
			}

			// commit data
			glUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );
		}
	}

	{ // create bit mask lookup texture for the fragment shader

		GLuint* lookup = (GLuint*)calloc( 4 * resZ, sizeof(GLuint) );
		for( int i = 1; i < resZ; i++ ) {
			if ( i < 32 ) {
				lookup[ 4 * i + 3 ] = ( 1U << std::min( 32, i ) ) - 1;
			} else if( i == 32 ) {
				lookup[ 4 * i + 3 ] = 0xFFFFFFFF;
			} else if ( i < 64 ) {
				lookup[ 4 * i + 3 ] = 0xFFFFFFFF;
				lookup[ 4 * i + 2 ] = ( 1U << std::min( 32, ( i - 32 ) ) ) - 1;
			} else if( i == 64 ) {
				lookup[ 4 * i + 3 ] = 0xFFFFFFFF;
				lookup[ 4 * i + 2 ] = 0xFFFFFFFF;
			} else if ( i < 96 ) {
				lookup[ 4 * i + 3 ] = 0xFFFFFFFF;
				lookup[ 4 * i + 2 ] = 0xFFFFFFFF;
				lookup[ 4 * i + 1 ] = ( 1U << std::min( 32, ( i - 64 ) ) ) - 1;
			} else if( i == 96 ) {
				lookup[ 4 * i + 3 ] = 0xFFFFFFFF;
				lookup[ 4 * i + 2 ] = 0xFFFFFFFF;
				lookup[ 4 * i + 1 ] = 0xFFFFFFFF;
			} else {
				lookup[ 4 * i + 3 ] = 0xFFFFFFFF;
				lookup[ 4 * i + 2 ] = 0xFFFFFFFF;
				lookup[ 4 * i + 1 ] = 0xFFFFFFFF;
				lookup[ 4 * i + 0 ] = ( 1U << std::min( 32, ( i - 96 ) ) ) - 1;
			}
		}	

		glClampColorARB( GL_CLAMP_VERTEX_COLOR_ARB  , GL_FALSE );
		glClampColorARB( GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE );
		glClampColorARB( GL_CLAMP_READ_COLOR_ARB    , GL_FALSE );


		glGenTextures( 1, &bitmaskTex );
		glBindTexture( GL_TEXTURE_1D, bitmaskTex );
		glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA32UI, resZ, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, lookup ); 		
		free( lookup );

		GLenum errCode;
		const GLubyte *errString;

		if ((errCode = glGetError()) != GL_NO_ERROR) {
			errString = gluErrorString(errCode);
			fprintf (stderr, "OpenGL Error: %s\n", errString);
		}
	}


	{ // setup modelView/projection matrices

		const float epsilon = 1e-2f;

		glMatrixMode( GL_PROJECTION );			
		glPushMatrix();							
		glLoadIdentity();	
		glOrtho( -bounds.width() / 2, bounds.width() / 2,
				 -bounds.height() / 2, bounds.height() / 2,
				 0, bounds.max().z - bounds.min().z );
				 /*bounds.min().z, bounds.max().z );*/

		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();				
		glLoadIdentity();
		//glScaled( 1.0 / bounds.width(), 1.0 / bounds.height(), 1.0 / bounds.depth() );
		gluLookAt( bounds.center().x, bounds.center().y, bounds.max().z,
				   bounds.center().x, bounds.center().y, bounds.center().z,
				   0, 1, 0 );
	}

	// Render ////////////////////////////////////////////////////////////////////////

	// set state and shader input variables
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 4, GL_FLOAT, 0, BUFFER_OFFSET(0) );
	glEnableClientState( GL_INDEX_ARRAY );
	glIndexPointer( GL_INT, 0, NULL );
	glEnable( GL_TEXTURE_1D );

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				// Set The Clear Color To Medium Blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And Depth Buffer
	
	glDisable( GL_DEPTH_TEST );

	glUseProgram( program );

	// feed shader variables
	
	int bitmaskHandler = glGetUniformLocation( program, "bitmask" );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_1D, bitmaskTex );
	glUniform1i( bitmaskHandler, 0 );

	int nearClipHandle = glGetUniformLocation( program, "nearClipPlane" );
	glUniform1f( nearClipHandle, 0.0f);

	int farClipHandle = glGetUniformLocation( program, "farClipPlane" );
	glUniform1f( farClipHandle, (float)bounds.depth() );

	// set blending mode
	glLogicOp( GL_XOR );
	glEnable( GL_COLOR_LOGIC_OP );

	glPushAttrib( GL_VIEWPORT_BIT );
	glViewport( 0, 0, resX, resY );

	// render geometry
	glDrawElements( GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, NULL );

	glPopAttrib();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLenum errCode;
	const GLubyte *errString;

	if ((errCode = glGetError()) != GL_NO_ERROR) {
		errString = gluErrorString(errCode);
		fprintf (stderr, "OpenGL Error: %s\n", errString);
	}

	// gather the resulting texture data
	glBindTexture( GL_TEXTURE_2D, renderTarget );
	unsigned int* data = (unsigned int*)malloc( 4 * resX * resY * sizeof(unsigned int) );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, data );

	if ((errCode = glGetError()) != GL_NO_ERROR) {
		errString = gluErrorString(errCode);
		fprintf (stderr, "OpenGL Error: %s\n", errString);
	}

	{ // dump voxels

		voxels.clear();

		const float deltaX = (float)bounds.width()  / resX;
		const float deltaY = (float)bounds.height() / resY;
		const float deltaZ = (float)bounds.depth()  / resZ;

		// due to the way the bits are constructed, the
		// resulting boxes are shifted half a voxel - we'll
		// compensate that shift while dumping the geometry.
		const MPoint halfVoxel( 0, 0, -deltaZ * 0.5f );

		for( int y = 0; y < resY; y++ ) {
			for( int x = 0; x < resX; x++ ) {
				
				for( int i = 3; i >= 0; i-- ) {
					unsigned int col = data[ 4 * ( x + y * resX ) + i ];
					
					if ( col == 0 ) continue;
					
					for( int z = 0; z < 32; z++ ) { // unpack color data

						if ( ( col & ( 1 << z ) ) != 0 ) { // if the z-th bit is set, create a voxel
							
							MPoint bbMin( bounds.min().x + x * deltaX, bounds.min().y + y * deltaY, bounds.max().z - ( 32 * ( 3 - i ) + z ) * deltaZ );
							MPoint bbMax( bbMin.x + deltaX, bbMin.y + deltaY, bbMin.z + deltaZ );
							bbMin += halfVoxel;
							bbMax += halfVoxel;
							voxels.append( bbMin );
							voxels.append( bbMax );
						}
					}
				}
			}
		}
	}

	free( data );

	// restore state


	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_INDEX_ARRAY );
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
	glEnable( GL_DEPTH_TEST );
	glUseProgram( 0 );
	glDisable( GL_TEXTURE_1D );
	glDisable( GL_COLOR_LOGIC_OP );

	// dispose resources /////////////////////////////////////////////////////////////

	glDeleteBuffers( 1, &vbo );
	glDeleteBuffers( 1, &ibo );
	glDeleteTextures( 1, &renderTarget );
	glDeleteShader( fs );
	glDeleteProgram( program );
	glDeleteFramebuffers( 1, &fbo );
	glDeleteRenderbuffers( 1, &rbo );

	return true;
}

// this random distribution is far from good, but it'll do for our purposes
static inline double random01() {
	return (double)rand() / RAND_MAX;
}

bool VoxelSampler::SampleVoxels( const MPointArray& voxels, int numSamples,
								 MPointArray& samples ) {

	srand( (unsigned int)time(0) );

	if ( voxels.length() == 0 || numSamples <= 0 ) return false;

	// sample voxels assuming they're the same size. If they were not, we would
	// sample them according to their volume by finding the common denominator
	// and building an array so that each voxel index would appear a proportional
	// number of times, then choosing a random element each time within that array.

	unsigned int numVoxels = voxels.length() / 2; // (min,max), (min,max)...

	samples.clear();

	for( int i = 0; i < numSamples; i++ ) {
		unsigned int voxelIndex = (unsigned int)( random01() * numVoxels );

		// sample voxel
		const MPoint& bbMin = voxels[ 2 * voxelIndex ];
		const MPoint& bbMax = voxels[ 2 * voxelIndex + 1 ];

		// recalculating this every time would be unnecessary as every voxel will
		// be the same dimensions but it's left for illustration purposes
		const double voxelsWidth  = bbMax.x - bbMin.x;
		const double voxelsHeight = bbMax.y - bbMin.y;
		const double voxelsDepth  = bbMax.z - bbMin.z;

		MPoint sample(	bbMin.x + voxelsWidth * random01(), 
						bbMin.y + voxelsHeight * random01(),
						bbMin.z + voxelsDepth * random01() );				

		samples.append( sample );
	}

	return true;
}
