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

#include "HorizontalScroller.h"



HorizontalScroller::HorizontalScroller(PinGUI::Vector2<GUIPos> pos,int value, bool* update, std::vector<std::shared_ptr<GUI_Element>>* _ELEMENTS):
    Scroller(pos,update)
{
    PinGUI::Rect tmpRect;
    tmpRect.initPos(pos);

    addSprite(tmpRect,SheetManager::createWindowScroller(value,PinGUI::HORIZONTAL));

    _collidable = false;
}

HorizontalScroller::~HorizontalScroller()
{
    //dtor
}

void HorizontalScroller::createArrows(std::vector<std::shared_ptr<GUI_Element>>* _ELEMENTS){

    PinGUI::basicPointer f1(boost::bind(&HorizontalScroller::incScroller, this));

    PinGUI::basicPointer f2(boost::bind(&HorizontalScroller::decScroller, this));

    PinGUI::Rect tmpRect(getSprite()->getX() + PINGUI_WINDOW_LINE_W,
						 getSprite()->getY() + PINGUI_WINDOW_LINE_H,
						 WINDOW_ARROW_WIDTH,
					   	 WINDOW_ARROW_HEIGHT);

    //Right arrow
    auto tmpArrow = std::make_shared<Window_Arrow>(tmpRect,SheetManager::getSurface(BOARD),LEFT);

    tmpArrow->setClickFunction(f2);
    _ARROWS.push_back(tmpArrow);

    //Left arrow
    tmpRect.x +=  getSprite()->getW() - WINDOW_ARROW_WIDTH - PINGUI_WINDOW_LINE_W;

    tmpArrow = std::make_shared<Window_Arrow>(tmpRect,SheetManager::getSurface(BOARD),RIGHT);

    tmpArrow->setClickFunction(f1);
    _ARROWS.push_back(tmpArrow);

    for (const auto& arrow : _ARROWS)
        _ELEMENTS->push_back(arrow);
}

void HorizontalScroller::calculateRatio(const int& totalValue){

    int numOfManipulationPixels =  int(_ARROWS[1]->getSprite()->getX() - (getSprite(1)->getX() + getSprite(1)->getW()));

    _ratio = int(static_cast<float>(totalValue)/numOfManipulationPixels);

    initNormalizer(totalValue,numOfManipulationPixels);
}

void HorizontalScroller::manageScrollerFill(const int& value){

    getSprite(1)->setW(value);
    getSprite(1)->setH(PINGUI_WINDOW_DEFAULT_SCROLLER_H - (PINGUI_WINDOW_LINE_H*2));
    getSprite(1)->setX((_ARROWS[0]->getSprite()->getX()+_ARROWS[0]->getSprite()->getW()));
    _oldPos = getSprite(1)->getX();

    getSprite(1)->getGUIRect_P()->realRect = getSprite(1)->getGUIRect_P()->rect;
}

int HorizontalScroller::calculateScrollerSize(const int& value, int& totalValue){

    //Percentage of window viewport
    float windowPercentage = static_cast<float>(value)/static_cast<float>(totalValue);

    //Now need the possible length of scroller fill
    int tmpLength = 0;

    tmpLength = int(_ARROWS[1]->getSprite()->getX()-(_ARROWS[0]->getSprite()->getX()-WINDOW_ARROW_WIDTH));

   return int(tmpLength*windowPercentage);
}

void HorizontalScroller::modifyArrowPos(){

    _ARROWS[1]->setX(getSprite()->getX() + getSprite()->getW() - WINDOW_ARROW_WIDTH);
    _ARROWS[1]->getSprite(-1)->offsetToRect(*(_ARROWS[1]->getCollider()));
}

void HorizontalScroller::onClick(){

    //Setting the input state
    PinGUI::Input_Manager::setInputState(PinGUI::MANIPULATINGMOD);
    PinGUI::Input_Manager::setState(PinGUI::GUI);
    PinGUI::Input_Manager::createManipulatingModInfo(getSprite(1)->getX(),getSprite(1)->getY(),PinGUI::HORIZONTAL);
    PinGUI::Input_Manager::initManipulation(_SPRITES[1],_update);
}

void HorizontalScroller::checkLimits(){

    if (getSprite(1)->getX()<(_ARROWS[0]->getSprite()->getX()+_ARROWS[0]->getSprite()->getW())){

        getSprite(1)->setX(_ARROWS[0]->getSprite()->getX()+_ARROWS[0]->getSprite()->getW());
    } else if (getSprite(1)->getX() + getSprite(1)->getW() > _ARROWS[1]->getSprite()->getX()){

        getSprite(1)->setX(_ARROWS[1]->getSprite()->getX() - getSprite(1)->getW());
    }
}

void HorizontalScroller::decide(){

    if (getSprite(1)->getX()!=_oldPos){

        int diff = 0;

        int newDiff = getDiff();

        if (newDiff > _oldDiff)
            diff -= (newDiff - _oldDiff);
        else
            diff =  _oldDiff -newDiff;

        diff *= _ratio ;

         if (diff<0)
            diff += useNormalizer(PLUS,diff);
        else
            diff += useNormalizer(MINUS,diff);

        PinGUI::Vector2<GUIPos> tmpVec(float(diff),0.0f);
        _cameraRoll.exec(tmpVec);

        _oldPos = getSprite(1)->getX();
        _oldDiff = newDiff;
    }
}

void HorizontalScroller::initOldPos(){
    _oldPos = getSprite(1)->getX();
}

void HorizontalScroller::incScroller(){

	if (_SPRITES.size()>1) {

		getSprite(1)->setX(getSprite(1)->getX() + WINDOW_SCROLLER_ARROW_MOVEMENT);

		manageFunctionality();
	}
}

void HorizontalScroller::reloadScroller(int value, int totalValue){

	GUIPos oldPos = getSprite(1)->getX();

	deleteSprite(1);

	loadScrollMover(value, totalValue);

	getSprite(1)->setX(oldPos);
	_oldDiff = 0.0f;

	decide();
	checkLimits();
}

void HorizontalScroller::decScroller(){

	if (_SPRITES.size()>1) {

		getSprite(1)->setX(getSprite(1)->getX() - WINDOW_SCROLLER_ARROW_MOVEMENT);

		manageFunctionality();
	}
}

void HorizontalScroller::moveOldPos(const PinGUI::Vector2<GUIPos>& vect){
    _oldPos += vect.x;
}

int HorizontalScroller::useNormalizer(CHANGER change, int diff){

    int numOfPxs = diff*-1;

    switch(change){

        case PLUS : {

            if (_NORMALIZER._PLUS > 0){

                if (_NORMALIZER._PLUS - numOfPxs >=0 ){

                    _NORMALIZER._PLUS -= numOfPxs;
                    _NORMALIZER._MINUS += numOfPxs;

                    return (-numOfPxs);

                } else {

                    int tmp = _NORMALIZER._PLUS;

                    _NORMALIZER._MINUS += _NORMALIZER._PLUS;
                    _NORMALIZER._PLUS -= _NORMALIZER._PLUS;

                    return (-tmp);
                }
            }

            break;
        }
        case MINUS : {


            if (_NORMALIZER._MINUS > 0){

                if (_NORMALIZER._MINUS + numOfPxs >=0 ){

                    _NORMALIZER._MINUS += numOfPxs;
                    _NORMALIZER._PLUS -= numOfPxs;

                    return -numOfPxs;
                } else {

                    int tmp = _NORMALIZER._MINUS;

                    _NORMALIZER._PLUS += _NORMALIZER._MINUS;
                    _NORMALIZER._MINUS -= _NORMALIZER._MINUS;

                    return tmp;
                }
            }
            break;
        }
    }
    return 0;
}

int HorizontalScroller::getDiff(){

    return int(getSprite(1)->getX() - (_ARROWS[0]->getSprite()->getX()+_ARROWS[0]->getSprite()->getW()));
}
