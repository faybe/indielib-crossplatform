/*****************************************************************************************
 * File: IND_SpriterManager.cpp
 * Desc: This class is used for managing Spriter animations.
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

#include "Global.h"
#include "Defines.h"
#include "dependencies/tinyxml/tinyxml.h"
#include "IND_SpriterManager.h"
#include "IND_SpriterEntity.h"
#include "IND_Surface.h"
#include "IND_SurfaceManager.h"
#include "IND_Render.h"
//#ifdef linux
#include <string>
//#endif


// ----- Defines -----

#define MAX_TOKEN 1024



// --------------------------------------------------------------------------------
//							  Initialization / Destruction
// --------------------------------------------------------------------------------

/**
 * Init the manager. returns 1 (true) if successfully initialized.
 * Must be called before using any method.
 */
bool IND_SpriterManager::init(IND_SurfaceManager *pSurfaceManager, IND_Render *pRender) {
	end();
	initVars();
    
    g_debug->header("Initializing SpriterManager", DebugApi::LogHeaderBegin);
    
    // Checking IND_Render
	if (pSurfaceManager->isOK()) {
		g_debug->header("Checking IND_Render", DebugApi::LogHeaderOk);
		_render = pRender;
        
		_ok = true;
        
		g_debug->header("Render OK", DebugApi::LogHeaderEnd);
	} else {
		g_debug->header("Render is not correctly initialized", DebugApi::LogHeaderError);
		_ok = false;
        
        return _ok;
	}
    
    
    
    
	// Checking IND_SurfaceManager
	if (pSurfaceManager->isOK()) {
		g_debug->header("Checking IND_SurfaceManager", DebugApi::LogHeaderOk);
		_surfaceManager = pSurfaceManager;

		_ok = true;
        
		g_debug->header("SurfaceManager OK", DebugApi::LogHeaderEnd);
	} else {
		g_debug->header("SurfaceManager is not correctly initialized", DebugApi::LogHeaderError);
		_ok = false;
	}
    
	return _ok;
}


/**
 * Frees the manager and all the objects that it contains.
 */
void IND_SpriterManager::end() {
	if (_ok) {
		g_debug->header("Finalizing SpriterManager", 5);
		freeVars();
		g_debug->header("SpriterManager finalized", 6);

		_ok = false;
	}
}


// --------------------------------------------------------------------------------
//									Public methods
// --------------------------------------------------------------------------------



/**
 * Returns 1 (true) if the Spriter Entity object passed as a parameter
 * exists and it is successfully eliminated.
 * @param pEn				Pointer to a Spriter Entity object.
 */
bool IND_SpriterManager::remove(IND_SpriterEntity *pEn) {
//	if (remove(pAn, 0))
//		return 1;
// TODO : search for entity and remove from vector



	return 0;
}

bool IND_SpriterManager::addSpriterFile(const char *pSCMLFileName){
	if (parseSpriterData(pSCMLFileName)){
		return 1;
	}
	
	return 0;
}



// --------------------------------------------------------------------------------
//									Private methods
// --------------------------------------------------------------------------------



/**
 * Parses a Spriter SCML animations file using Tinyxml.
 * @param pNewSpriterAnimations		TODO describtion.
 * @param pSCMLFileName				TODO describtion.
 */
bool IND_SpriterManager::parseSpriterData(const char *pSCMLFileName) {
	
    g_debug->header("Start parsing Spriter SGML file", 5);
    
    TiXmlDocument *eXmlDoc = new TiXmlDocument(pSCMLFileName);

	// Fatal error, cannot load
	if (!eXmlDoc->LoadFile()){
		g_debug->header("Not able to load the Spriter SGML file", 2);
		return 0;
	}


	string spriterTopPath;
    string s = string(pSCMLFileName);

	size_t lastPosTemp = s.find_last_of("\\/");

	if(lastPosTemp == string::npos){
		spriterTopPath = "./";
    }
	else{
    	spriterTopPath = s.substr(0, lastPosTemp + 1);
	}
    
	g_debug->header("Top directory : ", 3);
    g_debug->dataChar(spriterTopPath.c_str(), true);

	// Document root
	TiXmlElement *eSpriter_data = 0;
	eSpriter_data = eXmlDoc->FirstChildElement("spriter_data");

	if (!eSpriter_data) {
		g_debug->header("Invalid name for Spriter document root, should be <spriter_data>", 2);
		eXmlDoc->Clear();
		delete eXmlDoc;
		return 0;
	}

	// ----------------- Parse folders and create the images -----------------
    
       
    IND_SpriterEntity *sEnt = IND_SpriterEntity::newSpriterEntity();                        // TODO: remember, the filestructure shouldn't be placed on the spriter entity
                                                                                            //       since it can contain images used by multiple entities... but for now it's ok
    
	TiXmlElement *eFolder = 0;
	eFolder = eSpriter_data->FirstChildElement("folder");

	if (!eFolder) {
		g_debug->header("There are no Spriter resourcefolders to parse", 3);
		eXmlDoc->Clear();
		delete eXmlDoc;
		return 0;
	}

	while (eFolder) {

		TiXmlElement *eFile = 0;
		eFile = eFolder->FirstChildElement("file");

		while (eFile){

			string result = spriterTopPath + string(eFile->Attribute("name"));
			
			//IND_Image *imageTemp = IND_Image::newImage();                                   //
			//if (!_imageManager->add(imageTemp, result.c_str())){                            //
            //    g_debug->header("Unable to add Spriter image", 2);                          // TODO : figure out what is the "right" place to store the images ...
            //    eXmlDoc->Clear();
			//	delete eXmlDoc;
			//	return 0;
			//}
            IND_Surface *surfaceTemp = IND_Surface::newSurface();
            if (!_surfaceManager->add(surfaceTemp, result.c_str(), IND_ALPHA, IND_32)) {
                g_debug->header("Unable to add Spriter surface", 2);
                eXmlDoc->Clear();
                delete eXmlDoc;
                return 0;
            }
            
            
            sEnt->addSurface(toInt(eFolder->Attribute("id")),
                           toInt(eFile->Attribute("id")),
                                 surfaceTemp);                                                // TODO : this is wrong, - we store a ref to an image object located in the imagehandler
                                                                                            //        therefore we have no private ownership =(

			eFile = eFile->NextSiblingElement("file");
		}

		eFolder = eFolder->NextSiblingElement("folder");
	}

	// ----------------- Parse entities -----------------

	TiXmlElement *eEntity = 0;
	eEntity = eSpriter_data->FirstChildElement("entity");

	if (!eEntity) {
		g_debug->header("There are no entities to parse", 2);
		eXmlDoc->Clear();
		delete eXmlDoc;
		return 0;
	}
    
    sEnt->_id = eEntity->Attribute("id");
    sEnt->_name = eEntity->Attribute("name");

	while (eEntity) {

		//IND_SpriterEntity *sEnt = IND_SpriterEntity::newSpriterEntity();                  // TODO use this one, when filestructure is not placed on the entity..

		TiXmlElement *eAnimation = 0;
		eAnimation = eEntity->FirstChildElement("animation");

		while(eAnimation){
			
            Animation *sAnim = sEnt->addAnimation(toInt(eAnimation->Attribute("id")),
                                                        eAnimation->Attribute("name"),
                                                  toInt(eAnimation->Attribute("length")),
                                                        eAnimation->Attribute("looping"),
                                                  toInt(eAnimation->Attribute("loop_to"))
                                                 );
			
			TiXmlElement *eMainline = 0;
			eMainline = eAnimation->FirstChildElement("mainline");
			
			if (!eMainline) {
				g_debug->header("Animation is missing mainline", 2);
				eXmlDoc->Clear();
				delete eXmlDoc;
				return 0;
			}
			
            
            TiXmlElement *eMKey = 0;
			eMKey = eMainline->FirstChildElement("key");

			while (eMKey){
                
                MainlineKey *sMKey = sAnim->getMainline()->addKey(toInt(eMKey->Attribute("id")),
                                                                  toInt(eMKey->Attribute("time"))
                                                                 );
				
				TiXmlElement *eObject_ref = 0;
				eObject_ref = eMKey->FirstChildElement("object_ref");
				
				while (eObject_ref){
                    sMKey->addObjectref(toInt(eObject_ref->Attribute("id")),
                                        toInt(eObject_ref->Attribute("timeline")),
                                        toInt(eObject_ref->Attribute("key")),
                                        toInt(eObject_ref->Attribute("z_index"))
                                       );
					
					eObject_ref = eObject_ref->NextSiblingElement("object_ref");
				}

                
                TiXmlElement *eObject = 0;
				eObject = eMKey->FirstChildElement("object");
				
				while (eObject){
                    sMKey->addObject(  toInt(eObject_ref->Attribute("id")),
                                             eObject_ref->Attribute("object_type"),
                                       toInt(eObject_ref->Attribute("folder")),
                                       toInt(eObject_ref->Attribute("file")),
                                     toFloat(eObject_ref->Attribute("x")),
                                     toFloat(eObject_ref->Attribute("y")),
                                     toFloat(eObject_ref->Attribute("pivot_x")),
                                     toFloat(eObject_ref->Attribute("pivot_y")),
                                     toFloat(eObject_ref->Attribute("angle")),
                                     toFloat(eObject_ref->Attribute("scale_x")),
                                     toFloat(eObject_ref->Attribute("scale_y")),
                                     toFloat(eObject_ref->Attribute("a"))
                                    );
					
					eObject = eObject->NextSiblingElement("object");
				}
                
                
				eMKey = eMKey->NextSiblingElement("key");
                
			}
            
			TiXmlElement *eTimeline = 0;
			eTimeline = eAnimation->FirstChildElement("timeline");

			while (eTimeline){
                
                Timeline *sTimeline = sAnim->addTimeline(toInt(eTimeline->Attribute("id")),
                                                               eTimeline->Attribute("name"),
                                                               eTimeline->Attribute("object_type"),
                                                               eTimeline->Attribute("variable_type"),
                                                               eTimeline->Attribute("usage")
                                                        );
				
				TiXmlElement *eTKey = 0;
				eTKey = eTimeline->FirstChildElement("key");
				
				while (eTKey){
                    
                    TimelineKey *sTKey = sTimeline->addKey(toInt(eTKey->Attribute("id")),
                                                           toInt(eTKey->Attribute("time")),
                                                           toInt(eTKey->Attribute("spin"))
                                                          );
                    

					TiXmlElement *eTimelineObject = 0;
					eTimelineObject = eTKey->FirstChildElement("object");
					
                    while (eTimelineObject) {
                    
                        sTKey->addTimelineObject(  toInt(eTimelineObject->Attribute("folder")),
                                                   toInt(eTimelineObject->Attribute("file")),
                                                 toFloat(eTimelineObject->Attribute("x")),
                                                 toFloat(eTimelineObject->Attribute("y")),
                                                 toFloat(eTimelineObject->Attribute("pivot_x")),
                                                 toFloat(eTimelineObject->Attribute("pivot_y")),
                                                 toFloat(eTimelineObject->Attribute("angle")),
                                                 toFloat(eTimelineObject->Attribute("scale_x")),
                                                 toFloat(eTimelineObject->Attribute("scale_y")),
                                                 toFloat(eTimelineObject->Attribute("a"))
                                                );
                        
                        
                        eTimelineObject = eTimelineObject->NextSiblingElement("object");

                    }
                    
                    
					eTKey = eTKey->NextSiblingElement("key");

				}
				

				eTimeline = eTimeline->NextSiblingElement("timeline");
			}



			eAnimation = eAnimation->NextSiblingElement("animation");
		}


        _listSpriterEntity->push_back(sEnt);

		eEntity = eEntity->NextSiblingElement("entity");

	}


	// Delete our allocated document and return success.
	eXmlDoc->Clear();
	delete eXmlDoc;

    g_debug->header("End parsing Spriter SGML file", 6);
    
	return 1;
}


/**
 * Inserts an animation into the manager.
 * @param pNewAnimation				The animation that is to be inserted into manager.
 */
void IND_SpriterManager::addToList(IND_SpriterEntity *pNewEntity) {
	_listSpriterEntity->push_back(pNewEntity);
}


/**
 * Removes a Spriter animation from the manager.
 * @param pAn					The animation that is to be removed from the manager.
 */
void IND_SpriterManager::delFromlist(IND_SpriterEntity *pEn) {
	//_listSpriterEntity->remove(pEn); //TODO : and we probably have to stop the animation if playing, - also we have to handle that the manager is playlooping the list while we try to delete from it ...
	pEn = NULL;
}


/**
 * Initialization error message.
 */
void IND_SpriterManager::writeMessage() {
	g_debug->header("This operation can not be done", 3);
	g_debug->dataChar("", 1);
	g_debug->header("Invalid Id or not correctly initialized SpriterAnimationManager", 2);
}


/**
 * Init manager variables.
 */
void IND_SpriterManager::initVars() {
	_listSpriterEntity = new vector <IND_SpriterEntity *>;
    _timer = new IND_Timer();
    _timer->start();
    _deltaTime = 0.0;
    _lastTime = 0.0;
}


/**
 * Free manager variables.
 */

void IND_SpriterManager::freeVars() {
	// Releases everything in the anim
/*	list <IND_Animation *>::iterator mAnimationListIter;
	for (mAnimationListIter  = _listAnimations->begin();
	        mAnimationListIter != _listAnimations->end();
	        mAnimationListIter++) {
		// We released the animation(indicates no release pointer (Google Translate)
		remove((*mAnimationListIter), 1);

		g_debug->header("Ok", 6);
	}

	// Clear list
	_listAnimations->clear();

	// Free list
	DISPOSE(_listAnimations);
*/
    _timer->stop();
    DISPOSE(_timer);
}


int IND_SpriterManager::toInt(const char* input) {
        return ( input ) ? atoi(input) : 0;
}
    
float IND_SpriterManager::toFloat(const char* input) {
        return ( input ) ? static_cast<float>(atoi(input)) : 0.f;
}

/* =======================================================================================================
 *                  rendering
 * =======================================================================================================
 */

void IND_SpriterManager::renderEntities() {
    
    double currentTime = _timer->getTicks();
    
    _deltaTime = ( currentTime - _lastTime );
    
    _lastTime = currentTime;
    
    
    for (unsigned i=0; i < _listSpriterEntity->size(); i++) {
        
        updateCurrentTime(_listSpriterEntity->at(i), _deltaTime);
        updateCurrentKey(_listSpriterEntity->at(i));
        
        draw(_listSpriterEntity->at(i));
        
    }

}


void  IND_SpriterManager::draw(IND_SpriterEntity *ent) {
    if (ent->_currentAnimation < 0 || ent->_currentKey < 0 || ent->_currentTime < 0 ){
        return;
    }
    
    std::vector <MainlineObject *> *mainlineObjects = ent->getAnimations()->at(ent->_currentAnimation)->getMainline()->getKeys()->at(ent->_currentKey)->getObjects();
    
    std::vector <MainlineObjectref *> *mainlineObjectrefs = ent->getAnimations()->at(ent->_currentAnimation)->getMainline()->getKeys()->at(ent->_currentKey)->getObjectrefs();
    
    
    for (unsigned i=0; i < mainlineObjects->size(); i++) {
        drawPersistentObject(ent, mainlineObjectrefs->at(i));
    }
    
    
    for (unsigned i=0; i < mainlineObjectrefs->size(); i++) {
        drawTransientObject(ent, mainlineObjectrefs->at(i));
    }
    
}


void  IND_SpriterManager::drawTransientObject(IND_SpriterEntity *ent, MainlineObjectref *mObjectref) {
    TimelineObject *tObject = getTimelineObject(ent, mObjectref->timeline, mObjectref->key);
    
    IND_Surface *surface = getSurface(ent, tObject->folder, tObject->file);
    
    g_debug->header(surface->getTypeString(), DebugApi::LogHeaderError);
    
    IND_Matrix mMatrix = IND_Matrix(); // TODO: do we need this?
    
	int objx = static_cast<int>(tObject->x);
    int objy = static_cast<int>(tObject->y);
    int tempx = objx + 400;
    int tempy = objy <= 0 ? (500 + objy) : (500 - objy);
    
    float axisCalX = (tObject->pivot_x * ((float)surface->getWidth())  * -1.0f );
    float axisCalY = ((1 - tObject->pivot_y) * ((float)surface->getHeight()) * -1.0f );
    
    //TimelineKey *tKey = getAnimations()->at(_currentAnimation)->getTimeLines()->at(mObjectref->timeline)->getKeys()->at(mObjectref->key);
    
    float newangle = (tObject->angle < 0.0f) ? (tObject->angle + 360.0f) : (360.0f - tObject->angle);

    
    //cout << "tObject->x"  << tObject->x << " , tObject->y " << tObject->y << " ";
    //cout << "tObject->pivot_x "  << tObject->pivot_x << " , tObject->pivot_y " << tObject->pivot_y <<  " ";
    //cout << "axisCalX "  << axisCalX << " , axisCalY " << axisCalY << '\n';
    
    
    _render->setTransform2d(tempx,                      // x pos  note: we start in 0,0 (corner of screen)
                            tempy,                      // y pos
                            0.f,                          // Angle x
                            0.f,                          // Angle y
                            newangle,                   // Angle z
                            1.f,                          // Scale x
                            1.f,                          // Scale y
                            static_cast<int>(axisCalX),                   // Axis cal x
                            static_cast<int>(axisCalY),                   // Axis cal y
                            false,                          // Mirror x
                            false,                          // Mirror y
                            0,                          // Width
                            0,                          // Height
                            &mMatrix);                   // Matrix in wich the transformation will be applied (optional)
    
    
    
    // We apply the color, blending and culling transformations.
    _render->setRainbow2d(IND_ALPHA,                    // IND_Type
                          0,                            // Back face culling 0/1 => off / on
                          0,                            // Mirror x
                          0,                            // Mirror y
                          IND_FILTER_LINEAR,            // IND_Filter
                          255,                          // R Component	for tinting
                          255,                          // G Component	for tinting
                          255,                          // B Component	for tinting
                          255,                          // A Component	for tinting
                          0,                            // R Component	for fading to a color
                          0,                            // G Component	for fading to a color
                          0,                            // B Component	for fading to a color
                          255,                          // Amount of fading
                          IND_SRCALPHA,                 // IND_BlendingType (source)
                          IND_INVSRCALPHA);             // IND_BlendingType (destination)
    
    
    
    // Blit the IND_Surface
    _render->blitRegionSurface(surface, 0, 0, surface->getWidth(), surface->getHeight());
}


void  IND_SpriterManager::drawPersistentObject(IND_SpriterEntity *ent, MainlineObjectref *mObjectref) {
    // TODO: MFK, implement this
}


void IND_SpriterManager::drawBone(IND_SpriterEntity *ent, MainlineObjectref *mObjectref) {
    // TODO: support this in a later version
}


void IND_SpriterManager::updateCurrentTime(IND_SpriterEntity *ent, double deltaTime) {
    ent->_currentTime = ent->_currentTime + deltaTime; // TODO: is it ok to do the double to int cast here? ... and what about if we have passed the duration of the animation?
}


void IND_SpriterManager::updateCurrentKey(IND_SpriterEntity *ent) {
    std::vector <MainlineKey *> *mainlineKeys = ent->getAnimations()->at(ent->_currentAnimation)->getMainline()->getKeys();
    
    for (unsigned i=0; i < mainlineKeys->size(); i++) {
        if (mainlineKeys->at(i)->getTime() <= ent->_currentTime) {     // TODO: here again with the messy cast....
            ent->_currentKey = i;
        }
        else {
            cout << "(mainlinkeytime, currentimemessured) : ";
            cout << mainlineKeys->at(i)->getTime();
            cout << " , ";
            cout << ent->_currentTime;
            cout << "  ." << endl;
            
            break;
        }

    }
    

}

TimelineObject* IND_SpriterManager::getTimelineObject(IND_SpriterEntity *ent,int timelineId, int keyId) {
    return ent->getAnimations()->at(ent->_currentAnimation)->getTimeLines()->at(timelineId)->getKeys()->at(keyId)->getObjects()->at(0); // TODO : is there allways one object here? ( we're using an array )
}


IND_Surface* IND_SpriterManager::getSurface(IND_SpriterEntity *ent, int folderId, int fileId) {
    Fileref ref = Fileref(static_cast<unsigned int>(folderId), static_cast<unsigned int>(fileId));
    
    // Find the first matching key. (we should only have one)
    IND_SpriterEntity::SurfaceToFileMap::iterator res = ent->_surfaces->find(ref);
    
    if(res != ent->_surfaces->end()) {
        return res->second;
    }
    else {
        return NULL;
    }
}


