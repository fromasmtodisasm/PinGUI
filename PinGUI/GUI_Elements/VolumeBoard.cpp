/**

    PinGUI

    Copyright (c) 2017 Lubomir Barantal <l.pinsius@gmail.com>

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.

**/

#include "VolumeBoard.h"


VolumeBoard::VolumeBoard(PinGUI::Vector2<GUIPos> pos, int* var, int max, std::shared_ptr<GUI_Element> clip,bool* needUpdate):
    _clipBoard(clip),
    _max(max),
    _needUpdate(needUpdate),
    _storageType(PINGUI_INT)
{
    _dataStorage = std::make_shared<ElementDataStorage>(var);

    _position = pos;

    init();
}

VolumeBoard::VolumeBoard(PinGUI::Vector2<GUIPos> pos, float* var, int max, std::shared_ptr<GUI_Element> clip, bool* needUpdate):
    _clipBoard(clip),
    _max(max),
    _needUpdate(needUpdate),
    _storageType(PINGUI_FLOAT)
{
    _dataStorage = std::make_shared<ElementDataStorage>(var);

    _position = pos;

    init();
}

VolumeBoard::~VolumeBoard()
{
}

void VolumeBoard::init(){

    _clipBoard->setNetworking(true);

    //Create sprite
    PinGUI::Rect tmpRect;
    tmpRect.x = _position.x;
    tmpRect.y = _position.y;
    addSprite(tmpRect,SheetManager::getSurface(VOLUME_BACK));

    //calculate the ratio
    calculateRatio();

    //Adding fill sprite
    addFill(tmpRect);

    //Adding Mover sprite
    addMover(tmpRect);
}

float VolumeBoard::getVar(){

    switch(_storageType){

        case PINGUI_INT : {

            return static_cast<float>(_dataStorage->getInt()->getVar());
        }
        case PINGUI_FLOAT : {

            return _dataStorage->getFloat()->getVar();
        }
    }

	return 0.0f;
}

void VolumeBoard::addMover(PinGUI::Rect& tmpRect){

    tmpRect.x += VOLUMEBOARD_MOVER_OFFSET+(getVar()*_ratio);

    tmpRect.y += VOLUMEBOARD_MOVER_OFFSET_Y;
    addSprite(tmpRect,SheetManager::getSurface(VOLUME_MOVER));

    //Now creating collider
    tmpRect.x += VOLUMEBOARD_MOVER_COLLIDER_X;
    tmpRect.y += VOLUMEBOARD_MOVER_COLLIDER_Y;
    tmpRect.w = _SPRITES[VOL_MOVER]->getW()+VOLUMEBOARD_MOVER_COLLIDER_W;
    tmpRect.h = _SPRITES[VOL_MOVER]->getH()+VOLUMEBOARD_MOVER_COLLIDER_H;

    addCollider(tmpRect);

    moveMover();
}

void VolumeBoard::addFill(PinGUI::Rect& tmpRect){

    tmpRect.x = _SPRITES[VOL_BACKGROUND]->getX() + (VOLUMEBOARD_MOVER_OFFSET + 1);
    tmpRect.y = _SPRITES[VOL_BACKGROUND]->getY() + VOLUMEBOARD_FILL_OFFSET;

    addSprite(tmpRect,SheetManager::getSurface(VOLUME_FILL));
}


void VolumeBoard::calculateRatio(){
    _ratio = (float)(VOLUMEBOARD_BAR_LENGTH - VOLUME_MOVER_W)/(float)_max;
}

void VolumeBoard::update(){

    if (needMove()){
        moveMover();
    }
}

bool VolumeBoard::needMove(){

    switch(_storageType){

        case PINGUI_INT : {

            return _dataStorage->getInt()->changed();
        }
        case PINGUI_FLOAT : {

            return _dataStorage->getFloat()->changed();
        }
    }

	return false;
}

void VolumeBoard::moveMover(){

    if (PinGUI::Input_Manager::getCurrentState() != PinGUI::MANIPULATINGMOD || PinGUI::Input_Manager::getManipulatedSprite() != _SPRITES[VOL_MOVER]){
        _SPRITES[VOL_MOVER]->setX(calculatePosition());
    }

    _COLLIDERS[0].rect.x = _SPRITES[VOL_MOVER]->getGUIRect_P()->realRect.x;
    _COLLIDERS[0].realRect.x = _COLLIDERS[0].rect.x;

    *_needUpdate = true;

    float distance = getDistance();

    calculateFill(distance);
    checkMover();
}

void VolumeBoard::moveMover(int distance){

    if (distance==0){

        _SPRITES[VOL_MOVER]->setX(_SPRITES[VOL_BACKGROUND]->getX()+VOLUMEBOARD_MOVER_OFFSET);
    } else if (distance==_max){

        _SPRITES[VOL_MOVER]->setX(_SPRITES[VOL_BACKGROUND]->getX()+VOLUMEBOARD_MOVER_OFFSET+(VOLUMEBOARD_BAR_LENGTH-VOLUME_MOVER_W));
    }

    _COLLIDERS[0].rect.x = _SPRITES[VOL_MOVER]->getX();
    _COLLIDERS[0].realRect.x = _COLLIDERS[0].rect.x;
}

GUIPos VolumeBoard::calculatePosition(){
    return _SPRITES[VOL_BACKGROUND]->getGUIRect_P()->realRect.x + VOLUMEBOARD_MOVER_OFFSET+(getVar() * _ratio);
}

void VolumeBoard::onClick(){

    //Setting the input state
    PinGUI::Input_Manager::setInputState(PinGUI::MANIPULATINGMOD);
    PinGUI::Input_Manager::setState(PinGUI::GUI);
    PinGUI::Input_Manager::createManipulatingModInfo(getSprite(VOL_MOVER)->getX(),getSprite(VOL_MOVER)->getY(),PinGUI::HORIZONTAL);
    PinGUI::Input_Manager::initManipulation(_SPRITES[VOL_MOVER],_needUpdate);
}


bool VolumeBoard::listenForClick(manip_Element manipulatingElement){

    if (!PinGUI::Input_Manager::hasAlreadyClicked()){

        if (PinGUI::Input_Manager::isKeyPressed(SDL_BUTTON_LEFT)){

            PinGUI::Input_Manager::setAlreadyClick(true);

            onClick();

            manipulatingElement = this;
        }

        return true;
    }

    return false;
}

void VolumeBoard::manipulatingMod(manip_Element manipulatingElement){

    if (!PinGUI::Input_Manager::isKeyPressed(SDL_BUTTON_LEFT)){

        modifyVar();
        _COLLIDERS[0] = _SPRITES[VOL_MOVER]->getGUIRect();

        endManipulatingMod(manipulatingElement);

        return;

    } else if (*_needUpdate){

        modifyVar();
    }
}

void VolumeBoard::modifyVar(){

    float distance = getDistance();


    switch(_storageType){

        case PINGUI_INT : {

            auto _var = _dataStorage->getInt()->getVar_P();

            *_var = int(distance/_ratio);

            //Checking the min and max
            if (*_var<=0)
                *_var = 0;
            else if (*_var>_max)
                *_var = _max;

            _dataStorage->getInt()->equalVar();

            break;
        }
        case PINGUI_FLOAT : {

            auto _var = _dataStorage->getFloat()->getVar_P();

            *_var = distance/_ratio;

            //Checking the min and max
            if (*_var<=0)
                *_var = 0;
            else if (*_var>_max)
                *_var = float(_max);

            _dataStorage->getFloat()->equalVar();

            break;
        }
    }

    moveMover();
}

void VolumeBoard::calculateFill(const float& distance){

    if (distance>=0 && distance< _SPRITES[VOL_BACKGROUND]->getGUIRect_P()->realRect.w)
        _SPRITES[VOL_FILL]->setW(int(distance));
    else if (distance<=0){

        _SPRITES[VOL_FILL]->setW(0);
        _COLLIDERS[0] = _SPRITES[VOL_MOVER]->getGUIRect();
    } else{

        _SPRITES[VOL_FILL]->setW(int(_max*_ratio));
        _COLLIDERS[0] = _SPRITES[VOL_MOVER]->getGUIRect();
    }

    if (_SPRITES[VOL_FILL]->getX() != (_SPRITES[VOL_BACKGROUND]->getGUIRect().realRect.x+1))
        _SPRITES[VOL_FILL]->setX(_SPRITES[VOL_BACKGROUND]->getGUIRect().realRect.x+1);

    if (_SPRITES[VOL_FILL]->getW() > (VOLUMEBOARD_BAR_LENGTH - VOLUME_MOVER_W)){

        _SPRITES[VOL_FILL]->setW(VOLUMEBOARD_BAR_LENGTH-2);
    }

    _SPRITES[VOL_FILL]->getGUIRect_P()->realRect.w = _SPRITES[VOL_FILL]->getGUIRect_P()->rect.w;
}

void VolumeBoard::checkMover(){

    float distance = getDistance();

    if (distance<0){

      _SPRITES[VOL_MOVER]->setX(_SPRITES[VOL_BACKGROUND]->getGUIRect_P()->realRect.getX());
    } else if (distance>(VOLUMEBOARD_BAR_LENGTH - VOLUME_MOVER_W)){

        _SPRITES[VOL_MOVER]->setX(_SPRITES[VOL_BACKGROUND]->getGUIRect_P()->realRect.getX()+(VOLUMEBOARD_BAR_LENGTH-VOLUME_MOVER_W));

    }
}

void VolumeBoard::info(){
    std::cout << "Volumeboard element. Address: " << this << std::endl;
}

void VolumeBoard::moveElement(const PinGUI::Vector2<GUIPos>& vect){

    this->GUI_Element::moveElement(vect);
    _clipBoard->moveElement(vect);
}

void VolumeBoard::cropElement(PinGUI::Rect& rect){

    GUI_Element::cropElement(rect);

    _clipBoard->cropElement(rect);
}
