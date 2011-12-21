#pragma once

bool LoadTGA( const char *file, int& width, int& height, int& colormode, bool BGR2RGB, unsigned char** image );		// Load a tga image.      
bool WriteTGA( int width, int height, const unsigned char* image, bool BGR2RGB, const char *file ); // Write a tga file.      


