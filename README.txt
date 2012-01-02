Description:
	
	Maya plugin implementing utility nodes to voxelize and sample geometry.	
	
	Visit http://www.joesfer.com/?p=84 for further information.

License:

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