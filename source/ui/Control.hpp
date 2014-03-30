/*
 *  Control.hpp
 *  Critters
 *
 *  Created by Sean Kelleher on 9/1/11.
 *  Copyright 2011 Sean Kelleher. All rights reserved.
 *
 */
#pragma once

#include "Object.hpp"
#include "IDrawable.hpp"
#include "Types.hpp"
//#include "StoryboardManager.hpp"
#include "EffectManager.hpp"


#include <list>
using std::list;



namespace Z
{

// Forward declaration.
class Control;
//typedef Handle<Control> HControl;

   
class Control : virtual public Object, IDrawable
{
public:
    Control();
    virtual ~Control();

    // TODO: smart pointer
    Control*        m_pParentLayer;
    HEffect         m_hEffect;
    //HStoryboard     m_hStoryboard;
    Rectangle       m_frameRect;

    // IDrawable
    bool            m_isVisible;
    float           m_fScale;
    float           m_fOpacity;
    vec3            m_vWorldPosition;
    vec3            m_vRotation;
    Color           m_color;
    bool            m_isShadowEnabled;
    

    typedef list<Control*>                ControlList;
    typedef ControlList::iterator         ControlListIterator;
    
    ControlList     m_childrenList;
    
    
//----------------
// Class data
//----------------

    //
    // Exported Properties for animation/scripting
    //
    static       PropertySet        s_properties;
};


} // END namespace Z
