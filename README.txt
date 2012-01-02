Description:
	
	Maya plugin implementing utility nodes to voxelize and sample geometry.	
	
	Visit http://www.joesfer.com/?p=84 for further information.

License:

	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	

	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3.0 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Compilation:

	- Clone the git repository into a local folder:

		mkdir <sampler_folder>
		cd <sampler_folder>
		git clone git://github.com/joesfer/Sampler.git 
		
	- Build the plugin using CMake:

		cd <sampler_folder>
		mkdir .build
		cd .build
		
		e.g. To compile for 32-bit maya under Windows using Visual Studio 2008
		
			cmake -G "Visual Studio 9 2008" -DMAYA_ARCH=x86 ..
		
		e.g. Compile for 64-bit maya under Windows using Visual Studio 2008 x64

			cmake -G "Visual Studio 9 2008 Win64" -DMAYA_ARCH=x64 ..
		
		Under windows: cmake will generate a Visual studio solution on .build
		Under linux: cmake will generate a GCC makefile

		Build the plugin using visual studio or make.

		This will find the precompiled renderLib and build the .mll plugin
		under <sampler_folder>/bin

	- Load the .mll file in Maya's plugin manager.
	- Load the provided MEL script for an example on how to use the nodes.