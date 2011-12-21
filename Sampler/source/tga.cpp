/*   Class Name: CTGALoader.*/

#include "tga.h"
#include <stdio.h>
#include <malloc.h>
#include <memory.h>

bool LoadTGA( const char* file, int& width, int& height, int& colorMode, bool BGR2RGB, unsigned char** image ) {   
	FILE *pfile;   
	unsigned char tempColor;   
	// This will change the images from BGR to RGB.	
	unsigned char uselessChar; // This will be used to hold char data we dont want.	
	int	 uselessInt;		   // This will be used to hold int data we dont want.   
	long tgaSize;		       // Image size.	
	long index;                // Used in the for loop.   
	unsigned char imageTypeCode;   
	unsigned char bitCount;		
	
	// Open the image and read in binary mode.	
	fopen_s(&pfile, file, "rb");   // check if the file opened.	
	if (!pfile)		return false;	// Read in the two useless values.	
	
	fread(&uselessChar, sizeof(unsigned char), 1, pfile);	
	fread(&uselessChar, sizeof(unsigned char), 1, pfile);		
	
	// Read the image type, 2 is color, 4 is is greyscale.	
	fread(&imageTypeCode, sizeof(unsigned char), 1, pfile);	
	// We only want to be able to read in color or greyscale .tga's.	
	if ((imageTypeCode != 2) && (imageTypeCode != 3))	   {		   
		fclose(pfile);		   
		return false;	   
	}	// Get rid of 13 bytes of useless data.	
	
	fread(&uselessInt, sizeof(int), 1, pfile);	
	fread(&uselessInt, sizeof(int), 1, pfile);	
	fread(&uselessChar, sizeof(unsigned char), 1, pfile);	
	fread(&uselessInt, sizeof(int), 1, pfile);	
	fread(&uselessInt, sizeof(int), 1, pfile);	
	
	// Get the image width and height.	
	fread(&width, sizeof(int), 1, pfile);	
	fread(&height, sizeof(int), 1, pfile);	
	
	// Get the bit count.	
	fread(&bitCount, sizeof(unsigned char), 1, pfile);	
	// Get rid of 1 byte of useless data.   
	fread(&uselessChar, sizeof(unsigned char), 1, pfile);	

	// If the image is RGB then colorMode should be 3 and RGBA would   
	// make colorMode equal to 4.  This will help in our loop when   
	// we must swap the BGR(A) to RGB(A).	
	colorMode = bitCount / 8;   
	
	// Determine the size of the tga image.	
	tgaSize = width * height * colorMode;	
	// Allocate memory for the tga image.	
	unsigned char* img = (unsigned char*)malloc(sizeof(unsigned char)*tgaSize);	
	// Read the image into imageData.	
	fread( img, sizeof(unsigned char), tgaSize, pfile);		

	if ( BGR2RGB ) {
		// This loop will swap the BGR(A) to RGB(A).	
		for (index = 0; index < tgaSize; index += colorMode)	   {		   
			tempColor = img[index];		   
			img[index] = img[index + 2];		   
			img[index + 2] = tempColor;	   
		}	
	}
	
	*image = img;

	// Close the file where your done.	
	fclose(pfile);   
	
	// return true to satisfy our if statement (load successful).	
	return true;
}

bool WriteTGA( int width, int height, const unsigned char *image, bool BGR2RGB, const char *file ) {   
	// To save a screen shot is just like reading in a image.  All you do   
	// is the opposite.  Istead of calling fread to read in data you call   
	// fwrite to save it.   
	
	FILE *pFile;               // The file poshort inter.   
	unsigned char uselessChar; // used for useless char.   
	short int uselessint;      // used for useless short int.   
	unsigned char imageType;   // Type of image we are saving.   
	short int index;                 // used with the for loop.   
	unsigned char bits;		   // Bit depth.   
	long Size;                 // Size of the picture.   
	short int colorMode;   unsigned char tempColors;   
	
	// Open file for output.   
	fopen_s( &pFile, file, "wb");   

	// Check if the file opened or not.   
	if(!pFile) { 
		fclose(pFile); 
		return false; 
	}   
	
	// Set the image type, the color mode, and the bit depth.   
	imageType = 2; 
	colorMode = 4; 
	bits = 32;   
	
	// Set these two to 0.   
	uselessChar = 0; 
	uselessint = 0;   
	
	// Write useless data.   
	fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);   
	fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);   
	// Now image type.   
	fwrite(&imageType, sizeof(unsigned char), 1, pFile);   
	// Write useless data.   
	fwrite(&uselessint, sizeof(short int), 1, pFile);   
	fwrite(&uselessint, sizeof(short int), 1, pFile);   
	fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);   
	fwrite(&uselessint, sizeof(short int), 1, pFile);   
	fwrite(&uselessint, sizeof(short int), 1, pFile);   
	
	// Write the size that you want.   
	fwrite(&width, sizeof(short int), 1, pFile);   
	fwrite(&height, sizeof(short int), 1, pFile);   
	fwrite(&bits, sizeof(unsigned char), 1, pFile);   
	
	// Write useless data.   
	fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);   

	// Get image size.   
	Size = width * height * colorMode;   

	if ( BGR2RGB ) {
		// Now switch image from RGB to BGR.   
		unsigned char* swappedImage = (unsigned char*)malloc( Size );
		memcpy( swappedImage, image, Size );
		for(index = 0; index < Size; index += colorMode)      {         
			tempColors = swappedImage[index];         
			swappedImage[index] = swappedImage[index + 2];         
			swappedImage[index + 2] = tempColors;      
		}   

		// Finally write the image.   
		fwrite(swappedImage, sizeof(unsigned char), Size, pFile);   
		free( swappedImage );
	} else {
		fwrite(image, sizeof(unsigned char), Size, pFile);   
	}
	
	// close the file.   
	fclose(pFile);   
	
	return true;

} 
