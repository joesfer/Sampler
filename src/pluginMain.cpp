/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the MIT license: http://www.opensource.org/licenses/MIT	
	================================================================================
*/

#include "VoxelSamplerNode.h"
#include "VoxelShape.h"
#include "VoxelShapeUI.h"
#include "SamplePreviewShape.h"
#include "SamplePreviewShapeUI.h"
#include "RaySampler.h"

#include <maya/MFnPlugin.h>

MStatus initializePlugin( MObject obj )
//
//	Description:
//		this method is called when the plug-in is loaded into Maya.  It 
//		registers all of the services that this plug-in provides with 
//		Maya.
//
//	Arguments:
//		obj - a handle to the plug-in object (use MFnPlugin to access it)
//
{ 
	MStatus   status;
	MFnPlugin plugin( obj, "Jose Esteve - www.joesfer.com", "2011", "Any");

	status = plugin.registerData( VoxelPreviewDataWrapper::typeName, VoxelPreviewDataWrapper::id, VoxelPreviewDataWrapper::creator, MPxData::kGeometryData );
	if (!status) {
		status.perror("registerData");
		return status;
	}

	status = plugin.registerShape( VoxelShape::typeName, VoxelShape::id, 
								   VoxelShape::creator, VoxelShape::initialize, 
								   VoxelShapeUI::creator );
	if (!status) {
		status.perror("registerShape");
		return status;
	}

	status = plugin.registerData( SamplePreviewData::typeName, SamplePreviewData::id, SamplePreviewData::creator, MPxData::kGeometryData );
	if (!status) {
		status.perror("registerData");
		return status;
	}

	status = plugin.registerShape( SampleShape::typeName, SampleShape::id, 
		SampleShape::creator, SampleShape::initialize, 
		SampleShapeUI::creator );
	if (!status) {
		status.perror("registerShape");
		return status;
	}

	status = plugin.registerNode( "VoxelSampler", 
								  VoxelSampler::id, 
								  VoxelSampler::creator,
								  VoxelSampler::initialize );
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	status = plugin.registerNode( "RaySampler", 
								  RaySampler::id, 
								  RaySampler::creator,
								  RaySampler::initialize );
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
//
//	Description:
//		this method is called when the plug-in is unloaded from Maya. It 
//		deregisters all of the services that it was providing.
//
//	Arguments:
//		obj - a handle to the plug-in object (use MFnPlugin to access it)
//
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterData( VoxelPreviewDataWrapper::id );
	if (!status) {
		status.perror("deregisterData");
		return status;
	}

	status = plugin.deregisterData( SamplePreviewData::id );
	if (!status) {
		status.perror("deregisterData");
		return status;
	}

	status = plugin.deregisterNode( VoxelSampler::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	status = plugin.deregisterNode( RaySampler::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}


	return status;
}
