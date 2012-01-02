Description:
	
	Maya plugin implementing utility nodes to voxelize and sample geometry.	
	
	Visit http://www.joesfer.com/?p=84 for further information.

License:

	This software is released under the MIT license: http://www.opensource.org/licenses/MIT	

	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	
	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to
	use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
	of the Software, and to permit persons to whom the Software is furnished to do
	so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.

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