/*****************************************************************************************
 * File: IND_ImageManager.cpp
 * Desc: Image manager
 *****************************************************************************************/

/*********************************** The zlib License ************************************
 *
 * Copyright (c) 2013 Indielib-crossplatform Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 *****************************************************************************************/


// ----- Includes -----

#include "Defines.h"
#include "Global.h"
#include "dependencies/FreeImage/Dist/FreeImage.h"
#include "IND_ImageManager.h"
#include "IND_Image.h"
#include "FreeImageHelper.h"

#ifdef PLATFORM_LINUX
#include <string.h>
#include <stdlib.h>
#endif

unsigned int g_ImageManager_AddedCount = 0;
// --------------------------------------------------------------------------------
//							  Initialization / Destruction
// --------------------------------------------------------------------------------

/**
 * Returns 1 (true) if the administrator is successfully initialized.
 * Must be called before using any method.
 */
bool IND_ImageManager::init() {
	end();
	initVars();

	g_debug->header("Initializing ImageManager", DebugApi::LogHeaderBegin);
	g_debug->header("Preparing ImageManager", DebugApi::LogHeaderOk);
	_ok = true;

	FreeImage_Initialise();
	const char* freeImageVer = FreeImage_GetVersion();
	const char* freeImageCopyright = FreeImage_GetCopyrightMessage();
	g_debug->header("Using FreeImage version: ",DebugApi::LogHeaderInfo);
	g_debug->dataChar(freeImageVer, true);
    g_debug->header("Copyright: ", DebugApi::LogHeaderInfo);
    g_debug->dataChar(freeImageCopyright, true);
	g_debug->header("ImageManager Initialised", DebugApi::LogHeaderEnd);
	
	//TODO: REGISTER ERROR HANDLERS FOR FREEIMAGE

	return _ok;
}

/**
 * Frees the manager and all the objects that it contains.
 */
void IND_ImageManager::end() {
	if (_ok) {
		g_debug->header("Finalizing ImageManager", DebugApi::LogHeaderBegin);
		g_debug->header("Freeing images" , DebugApi::LogHeaderBegin);
		freeVars();
		FreeImage_DeInitialise();
		g_debug->header("Images freed", DebugApi::LogHeaderEnd);
		g_debug->header("ImageManager finalized", DebugApi::LogHeaderEnd);

		_ok = false;
	}
}

/**
 Returns state of initialization.
 @return  Will give true if object initialized correctly, false otherwise
 */
bool IND_ImageManager::isOK() const {
    return _ok;
}

// --------------------------------------------------------------------------------
//									Public methods
// --------------------------------------------------------------------------------

/**
 * Returns 1 (true) if the image object passed as a parameter
 * exists and is added successfully to the manager.
 * Supports the following graphics formats:
 * bmp, png, tga, jpg and pcx.
 * @param pNewImage					Pointer no a new image object.
 * @param pName						Image name.
 */
bool IND_ImageManager::add(IND_Image *pNewImage, const char *pName) {
	g_debug->header("Loading Image", DebugApi::LogHeaderBegin);
	
	if(!pName) {
		g_debug->header("Invalid File name provided (null)",DebugApi::LogHeaderError);
		return 0;
	}

	g_debug->header("File name:", DebugApi::LogHeaderInfo);
	g_debug->dataChar(pName, 1);

	if (!_ok) {
		writeMessage();
		return 0;
	}

	// ----- Obtaining and checking file extension -----
	//TODO: Refactor this to use FreeImage getfileType
	char ext [128];
	getExtensionFromName(pName,ext);
	if (checkExtImage(ext)) {
		pNewImage->setExtension(ext);
		g_debug->header("Extension:", DebugApi::LogHeaderInfo);
		g_debug->dataChar(ext, 1);
	} else {
		g_debug->header("Unknown extension", DebugApi::LogHeaderError);
		return 0;
	}

	

	// ----- Load image -----
	FREE_IMAGE_FORMAT imgFormat =  FreeImage_GetFileType(pName, 0);
	if (FIF_UNKNOWN == imgFormat) {
		g_debug->header("Image not found", DebugApi::LogHeaderError);
		return 0;
	}
	FIBITMAP* image = FreeImage_Load(imgFormat, pName, 0);
	if (!image) {
		g_debug->header("Image could not be loaded", DebugApi::LogHeaderError);
		return 0;
	}
	
	// Attributes
	pNewImage->setWidth(FreeImage_GetWidth(image));
	pNewImage->setHeight(FreeImage_GetHeight(image));
	IND_ColorFormat indFormat = FreeImageHelper::calculateINDFormat(image);
	pNewImage->setFormatInt(indFormat);
	pNewImage->setBpp(FreeImage_GetBPP(image));
	pNewImage->setBytespp(pNewImage->getBpp()/8);
	pNewImage->setPointer(FreeImage_GetBits(image));
	pNewImage->setName(pName);
	pNewImage->setFreeImageHandle(image);

	// ----- Puts the object into the manager -----

	addToList(pNewImage);

	// ----- g_debug -----

	g_debug->header("Size:", DebugApi::LogHeaderInfo);
	g_debug->dataInt(pNewImage->getWidth(), 0);
	g_debug->dataChar("x", 0);
	g_debug->dataInt(pNewImage->getHeight(), 1);

	g_debug->header("Bpp:", DebugApi::LogHeaderInfo);
	g_debug->dataInt(pNewImage->getBytespp(), 1);

	g_debug->header("Format:", DebugApi::LogHeaderInfo);
	g_debug->dataChar(pNewImage->getFormatString(), 1);

	g_debug->header("Image loaded", DebugApi::LogHeaderEnd);

	return 1;
}

/**
 * Loads an image. Returns a pointer to a FIBITMAP if the path to the file is correct and if Indielib support the image format.
 * Otherwise null is returned.
 * @param pName						Pointer to name of the image file to load.
 */
FIBITMAP* IND_ImageManager::load(const char *pName) {
		g_debug->header("Loading Image", DebugApi::LogHeaderBegin);
	
	if(!pName) {
		g_debug->header("Invalid File name provided (null)",DebugApi::LogHeaderError);
		return NULL;
	}

	g_debug->header("File name:", DebugApi::LogHeaderInfo);
	g_debug->dataChar(pName, 1);

	if (!_ok) {
		writeMessage();
		return NULL;
	}

	// ----- checking file extension -----
	//TODO: Refactor this to use FreeImage getfileType
	char ext [128];
	getExtensionFromName(pName,ext);
	if (!checkExtImage(ext)){
		g_debug->header("Unknown extension", DebugApi::LogHeaderError);
		return NULL;
	}

	g_debug->header("Extension:", DebugApi::LogHeaderInfo);
	g_debug->dataChar(ext, 1);

	// ----- Load image -----
	FREE_IMAGE_FORMAT imgFormat =  FreeImage_GetFileType(pName, 0);
	if (FIF_UNKNOWN == imgFormat) {
		g_debug->header("Image not found", DebugApi::LogHeaderError);
		return NULL;
	}
	FIBITMAP* image = FreeImage_Load(imgFormat, pName, 0);
	if (!image) {
		g_debug->header("Image could not be loaded", DebugApi::LogHeaderError);
		return NULL;
	}

	return image;
}

/**
 * Returns 1 (true) if the image object passed as a parameter
 * exists and is copied and added successfully to the manager creating a new procedural image.
 * @param pNewImage					Pointer to a new IND_Image object.
 * @param pImageToBeCopied				Pointer to a FIBITMAP, that is going to be cloned and added to the manager.
 */
bool IND_ImageManager::add(IND_Image *pNewImage, FIBITMAP *pImageToBeCopied) {
	if (!_ok || !pImageToBeCopied) {
		g_debug->header("FIBITMAP not supplied", DebugApi::LogHeaderError);
		return 0;
	}

	g_debug->header("Cloning Image", DebugApi::LogHeaderBegin);

	// ----- Image creation using FreeImage -----

	// Clone the old image

	FIBITMAP* pImage = FreeImage_Clone(pImageToBeCopied);
	if (!pImage) {
		g_debug->header("Image could not be cloned", DebugApi::LogHeaderError);
		return 0;
	}
	
	// Attributes
	pNewImage->setWidth(FreeImage_GetWidth(pImage));
	pNewImage->setHeight(FreeImage_GetHeight(pImage));
	IND_ColorFormat indFormat = FreeImageHelper::calculateINDFormat(pImage);
	pNewImage->setFormatInt(indFormat);
	pNewImage->setBpp(FreeImage_GetBPP(pImage));
	pNewImage->setBytespp(pNewImage->getBpp()/8);
	pNewImage->setPointer(FreeImage_GetBits(pImage));
	pNewImage->setName("Image clone-added");
	pNewImage->setFreeImageHandle(pImage);

	// ----- Puts the object into the manager -----

	addToList(pNewImage);

	// ----- g_debug -----

	g_debug->header("Size:", DebugApi::LogHeaderInfo);
	g_debug->dataInt(pNewImage->getWidth(), 0);
	g_debug->dataChar("x", 0);
	g_debug->dataInt(pNewImage->getHeight(), 1);

	g_debug->header("Bpp:", DebugApi::LogHeaderInfo);
	g_debug->dataInt(pNewImage->getBytespp(), 1);

	g_debug->header("Format:", DebugApi::LogHeaderInfo);
	g_debug->dataChar(pNewImage->getFormatString(), 1);

	g_debug->header("Image cloned and added", DebugApi::LogHeaderEnd);

	return 1;
}

/**
 * Returns 1 (true) if the image object passed as a parameter
 * exists and is added successfully to the manager creating a new procedural image.
 * By default, the new image is not initialized to any color, you should used the Clear()
 * method in order to assing a valid colour.
 * @param pNewImage					Pointer to a new image object.
 * @param pWidth					Width of the new image.
 * @param pHeight					Height of the new image.
 * @param pColorFormat				New image format. See ::IND_ColorFormat.
 */
bool IND_ImageManager::add(IND_Image *pNewImage, int pWidth, int pHeight, IND_ColorFormat pColorFormat) {
	g_debug->header("Creating Image", DebugApi::LogHeaderBegin);

	if (!_ok) {
		writeMessage();
		return 0;
	}

	// ----- Creating image using FreeImage -----
	int bpp = defaultBppForColorFormat(pColorFormat);

	FIBITMAP* pImage = FreeImage_Allocate(pWidth, pHeight, bpp);
	if (!pImage) {
		g_debug->header("Image could not be created", DebugApi::LogHeaderError);
		return 0;
	}

	// Attributes
	pNewImage->setWidth(FreeImage_GetWidth(pImage));
	pNewImage->setHeight(FreeImage_GetHeight(pImage));
	IND_ColorFormat indFormat = FreeImageHelper::calculateINDFormat(pImage);
	pNewImage->setFormatInt(indFormat);
	pNewImage->setBpp(FreeImage_GetBPP(pImage));
	pNewImage->setBytespp(pNewImage->getBpp()/8);
	pNewImage->setPointer(FreeImage_GetBits(pImage));
	pNewImage->setFreeImageHandle(pImage);

	// Name
	char name [MAX_TOKEN];	
	sprintf(name,"indielib_img_%i",g_ImageManager_AddedCount);
	pNewImage->setName(name);

	// ----- Puts the object into the manager -----

	addToList(pNewImage);

	g_debug->header("Image Created", DebugApi::LogHeaderEnd);
	return 1;
}

/**
 * Returns 1(true) if the image object passed as parameter exists and it is
 * deleted from the manager successfully.
 * @param pIm						Pointer to an image object.
 */
bool IND_ImageManager::remove(IND_Image *pIm) {
	g_debug->header("Freeing image", DebugApi::LogHeaderBegin);

	if (!_ok || !pIm) {
		writeMessage();
		return 0;
	}

	// Search object
	bool mIs = 0;
	list <IND_Image *>::iterator mImageListIter;
	for (mImageListIter  = _listImages->begin();
	        mImageListIter != _listImages->end();
	        mImageListIter++) {
		if ((*mImageListIter) == pIm) {
			mIs = 1;
			break;
		}
	}

	// Not found
	if (!mIs) {
		writeMessage();
		return 0;
	}

	// ----- Free object -----

	g_debug->header("File name:", DebugApi::LogHeaderInfo);
	g_debug->dataChar(pIm->getName(), 1);

    // Free image
	FreeImage_Unload(pIm->getFreeImageHandle());
    
	// Quit from list
	delFromlist(pIm);

	g_debug->header("Ok", DebugApi::LogHeaderEnd);

	return 1;
}

/**
 * Returns 1 (true) if the image object passed as a parameter
 * exists and is added successfully to the manager creating a new image cloning from a previous one.
 * @param pNewImage					Pointer to a new image object.
 * @param pOldImage					Pointer to the image that is to be cloned.
 */
bool IND_ImageManager::clone(IND_Image *pNewImage, IND_Image *pOldImage) {
	g_debug->header("Cloning Image", DebugApi::LogHeaderBegin);

	if (!_ok || !pNewImage || !pOldImage || !pOldImage->getFreeImageHandle()) {
		writeMessage();

		return 0;
	}

	// ----- Image creation using FreeImage -----

	// Clone the old image

	FIBITMAP* image = FreeImage_Clone(pOldImage->getFreeImageHandle());
	if (!image) {
		g_debug->header("Image could not be cloned", DebugApi::LogHeaderError);
		return 0;
	}
	
	// Attributes

	pNewImage->setWidth(pOldImage->getWidth());
	pNewImage->setHeight(pOldImage->getHeight());
	pNewImage->setBpp(pOldImage->getBpp());
	pNewImage->setBytespp(pOldImage->getBytespp());
	pNewImage->setFormatInt(pOldImage->getFormatInt());
	char ext [128];
	getExtensionFromName(pOldImage->getName(),ext);
	pNewImage->setExtension(ext);
	pNewImage->setPointer(FreeImage_GetBits(image));
	pNewImage->setName("Image cloned");
	pNewImage->setFreeImageHandle(image);

	// ----- Put the object into the manager -----

	addToList(pNewImage);

	// ----- g_debug -----

	g_debug->header("File name:", DebugApi::LogHeaderInfo);
	g_debug->dataChar(pOldImage->getName(), 1);
	g_debug->header("Image cloned", DebugApi::LogHeaderEnd);

	return 1;
}

/**
 * Returns 1 (true) if the image object passed as a parameter exists and
 * is saved in a file with extension and type specified in the name.
 * Supports the following graphics formats:
 * bmp, png, tga, jpg and pcx.
 * @param pIm						Image to take data from
 * @param pName						Image name.
 */
bool IND_ImageManager::save(IND_Image *pIm, const char *pName) {
	g_debug->header("Saving image", DebugApi::LogHeaderBegin);

	if (!_ok || !pIm || !pIm->isImageLoaded()) {
		writeMessage();
		return 0;
	}

    
    // ----- Obtaining and checking file extension -----
    
    char ext [128];
	getExtensionFromName(pName,ext);
	if (checkExtImage(ext)) {
		pIm->setExtension(ext);
	} else {
		g_debug->header("Unknown extension", DebugApi::LogHeaderError);
		return 0;
	}

	g_debug->header("Image type:", DebugApi::LogHeaderInfo);
	g_debug->dataChar(ext, 1);

	
	g_debug->header("File name:", DebugApi::LogHeaderInfo);
	g_debug->dataChar(pName, 1);

	g_debug->header("Size:", DebugApi::LogHeaderInfo);
	g_debug->dataInt(pIm->getWidth(), 0);
	g_debug->dataChar("x", 0);
	g_debug->dataInt(pIm->getHeight(), 1);

	FREE_IMAGE_FORMAT fif =  FreeImage_GetFIFFromFormat(ext);
    
    if(fif != FIF_UNKNOWN) {

        if(!FreeImage_Save(fif, pIm->getFreeImageHandle(), pName)) {
            g_debug->header("Unable to save image file", DebugApi::LogHeaderError);
            return 0;
        }
            

    } else {
        g_debug->header("Unable to get Free Image Format (FIF) from extension", DebugApi::LogHeaderError);
		return 0;
    }
    

	g_debug->header("Ok", DebugApi::LogHeaderEnd);
	return 1;
}

// --------------------------------------------------------------------------------
//									Private methods
// --------------------------------------------------------------------------------

/** @cond DOCUMENT_PRIVATEAPI */

int IND_ImageManager::defaultBppForColorFormat(IND_ColorFormat format) {
	int bpp = 0;
	switch(format) {
		case IND_LUMINANCE:
			bpp = 8;
			break;
		case IND_COLOUR_INDEX:
			bpp = 16;
			break;
		case IND_RGB:
			bpp = 24;
			break;
		case IND_RGBA:
			bpp = 32;
			break;
		case IND_UNKNOWN:
		default:
			bpp = -1;  //Make any method depending on this fail because of bad param
			break;
	}

	return bpp;
}

/*
==================
Obtain file extension
Return "" if the file hasn't got an extension
==================
*/
void IND_ImageManager::getExtensionFromName(const char *pName, char* pExtImage) {
	int length = static_cast<int>(strlen(pName));
    int i;
	// The search starts at the end of the name
	for (i = length; i > -1; i--)
		if (pName [i] == '.') 
			break;

	// Image without extension
	if (pName [i] != '.')
		strcpy (pExtImage, "");

	// Copy the extension
	strcpy(pExtImage, &pName [i + 1]);
}

/*
==================
Check if it is a known extension
==================
*/
bool IND_ImageManager::checkExtImage(const char *pExtImage) {
	for (int i = 0; i < MAX_EXT_IMAGE; i++) {
		if (!strcmp(pExtImage, _supportedExt [i]))
			return 1;
	}

	return 0;
}

/*
==================
Inserts object into the manager
==================
*/
void IND_ImageManager::addToList(IND_Image *pNewImage) {
	_listImages->push_back(pNewImage);
}

/*
==================
Deletes object from the manager
==================
*/
void IND_ImageManager::delFromlist(IND_Image *pIm) {
	_listImages->remove(pIm);
    DISPOSEMANAGED(pIm);
}

/*
==================
Initialization error message
==================
*/
void IND_ImageManager::writeMessage() {
	g_debug->header("This operation can not be done", DebugApi::LogHeaderInfo);
	g_debug->dataChar("", 1);
	g_debug->header("Invalid Id or IND_ImageManager not correctly initialized", DebugApi::LogHeaderError);
}

/*
==================
Init manager vars
==================
*/
void IND_ImageManager::initVars() {
	_listImages = new list <IND_Image *>;
	//TODO: UPDATE THIS WITH SUPPORTED EXTENSIONS DUE TO FREEIMAGE.
	//DO WE REALLY NEED THIS?
	//MFK: MAYBE WE CAN USE THE TWO METHODS i HAVE CREATED IN THE FREEIMAGEHELPER CLASS... 
	// Supported extensions
	_supportedExt [0]  = (char*) "bmp";
	_supportedExt [1]  = (char*) "png";
	_supportedExt [2]  = (char*) "tga";
	_supportedExt [3]  = (char*) "jpg";
	_supportedExt [4]  = (char*) "jpeg";
	_supportedExt [5]  = (char*) "pcx";
}

/*
==================
Free manager memory
==================
*/
void IND_ImageManager::freeVars() {
	// Deletes all the manager entities
	list <IND_Image *>::iterator mImageListIter;
	for (mImageListIter  = _listImages->begin();
	        mImageListIter != _listImages->end();
	        mImageListIter++) {
			FIBITMAP* handle = (*mImageListIter)->getFreeImageHandle();
		if (handle) {
			g_debug->header("Freeing image:", DebugApi::LogHeaderInfo);
			g_debug->dataChar((*mImageListIter)->getName(), 1);

			// Free image
			FreeImage_Unload(handle);
            (*mImageListIter)->setFreeImageHandle(NULL);
            DISPOSEMANAGED((*mImageListIter));
		}
	}

	// Clear list
	_listImages->clear();

	// Free list
	DISPOSE(_listImages);
}

/** @endcond */
