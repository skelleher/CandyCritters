/*
 *  AnimationManager.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 10/9/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "Engine.hpp"
#include "AnimationManager.hpp"
#include "FileManager.hpp"
#include "Log.hpp"
#include "Handle.hpp"
#include "Settings.hpp"
#include "Platform.hpp"
#include "Types.hpp"
#include "Util.hpp"
#include "GameObjectManager.hpp"
#include "StoryboardManager.hpp"


namespace Z
{

   
    
AnimationManager& 
AnimationManager::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new AnimationManager();
    }
    
    return static_cast<AnimationManager&>(*s_pInstance);
}


AnimationManager::AnimationManager()
{
    RETAILMSG(ZONE_VERBOSE, "AnimationManager()");
    
    s_pResourceManagerName = "AnimationManager";
}


AnimationManager::~AnimationManager()
{
    RETAILMSG(ZONE_VERBOSE, "\t~AnimationManager()");
}



RESULT
AnimationManager::Init( IN const string& settingsFilename )
{
    RETAILMSG(ZONE_INFO, "AnimationManager::Init( %s )", settingsFilename.c_str());

    RESULT rval = S_OK;
    char   path[MAX_PATH];
    
    //
    // Create a Settings object and load the file.
    //
    Settings mySettings;
    if ( FAILED(mySettings.Read( settingsFilename )) )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: AnimationManager::Init( %s ): failed to load settings file", settingsFilename.c_str() );
        return E_UNEXPECTED;
    }
    

    //
    // Create each Animation.
    //
    UINT32 numAnimations = mySettings.GetInt("/Animations.NumAnimations");

    for (int i = 0; i < numAnimations; ++i)
    {
        sprintf(path, "/Animations/Animation%d", i);
        //DEBUGMSG(ZONE_INFO, "Loading [%s]", path);

        Animation *pAnimation = NULL;
        CreateAnimation( &mySettings, path, &pAnimation );
        if (!pAnimation)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: AnimationManager::Init( %s ): failed to create Animation", path);
            
            DEBUGCHK(0);
            
            // Continue loading other Animations rather than aborting.
            continue;
        }
        
        //DEBUGMSG(ZONE_ANIMATION, "Created Animation [%s]", pAnimation->GetName().c_str());
        CHR(Add(pAnimation->GetName(), pAnimation));
    }
    
Exit:
    return rval;
}



RESULT
//AnimationManager::CreateAnimation( IN const string& name, IN KeyFrameType keyFrameType, IN InterpolatorType interpolatorType, IN KeyFrame* pKeyFrames, UINT8 numKeyFrames, bool isRelative, INOUT HAnimation* pHandle )
AnimationManager::CreateAnimation( 
        IN const string&            name, 
        IN const string&            propertyName, 
        IN       PropertyType       propertyType, 
        IN       InterpolatorType   interpolatorType, 
        IN       KeyFrameType       keyFrameType, 
        IN       KeyFrame*          pKeyFrames, 
        IN       UINT8              numKeyFrames, 
        IN       bool               isRelative, 
        INOUT    HAnimation*        pHandle
    )
{
    RESULT      rval = S_OK;
    HAnimation  hAnimation;
    
    Animation* pAnimation = new Animation();
    if (!pAnimation)
    {
        rval = E_OUTOFMEMORY;
        goto Exit;
    }

    rval = pAnimation->Init( name, propertyName, propertyType, interpolatorType, keyFrameType, pKeyFrames, numKeyFrames, isRelative );
    if (FAILED(rval))
    {
        SAFE_DELETE(pAnimation);
        goto Exit;
    }
    
    CHR(Add(pAnimation->GetName(), pAnimation, &hAnimation));
    if (pHandle)
    {
        *pHandle = hAnimation;
    }

Exit:
    return rval;
}



RESULT
AnimationManager::CreateAnimation( IN const Settings* pSettings, IN const string& settingsPath, INOUT Animation** ppAnimation )
{
    RESULT rval = S_OK;
    string name;
    
    if (!pSettings || "" == settingsPath || !ppAnimation)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: AnimationManager::CreateAnimation(): invalid argument");
        return E_INVALID_ARG;
    }
    
    Animation *pAnimation = new Animation();
    CPR(pAnimation);

    name = pSettings->GetString( settingsPath + ".Name" );
    
    if ("" == name)
    {
        // Auto-generate the name if needed.
        char instanceName[MAX_PATH];
        sprintf(instanceName, "%s.%s", 
            settingsPath.c_str(), 
            pSettings->GetString( settingsPath + ".Property" ).c_str());
            
        name = string(instanceName);
    }
    
    if (FAILED(pAnimation->Init( name, pSettings, settingsPath )))
    {
        delete pAnimation;
        pAnimation = NULL;
    }

    // Caller must AddRef()
    *ppAnimation = pAnimation;
    
Exit:    
    return rval;
}



RESULT
AnimationManager::ReleaseOnNextFrame( IN HAnimation handle )
{
    RESULT      rval = S_OK;
    Animation*  pAnimation;

    pAnimation = GetObjectPointer( handle );
    if (!pAnimation)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    CHR(ReleaseOnNextFrame( pAnimation ));

Exit:
    return rval;
}



RESULT
AnimationManager::ReleaseOnNextFrame( IN Animation* pAnimation )
{
    RESULT rval = S_OK;
    
    CPR(pAnimation)
    
    m_pendingReleaseAnimationsList.push_back( pAnimation );
    
Exit:
    return rval;
}



RESULT
AnimationManager::BindTo( IN HAnimation handle, IProperty* pProperty )
{
    RESULT rval = S_OK;

    if (pProperty)
    {
        CHR(BindTo( handle, *pProperty ));
    }
    else
    {
        // Bind to "NULL" - will force the Animation to release its target object.
        static Property<Object> nullProperty;
        CHR(BindTo( handle, nullProperty ));
    }
        
Exit:
    return rval;
}



RESULT
AnimationManager::BindTo( IN HAnimation handle, IProperty& property )
{
    RESULT      rval = S_OK;
    Animation*  pAnimation;
    
//    if ( property.IsNull() )
//    {
//        RETAILMSG(ZONE_ERROR, "ERROR: AnimationManager::BindTo( %d, 0x%x ): property IsNull",
//            (UINT32)handle, (UINT32)&property);
//            
//        rval = E_INVALID_ARG;
//        goto Exit;
//    }

    pAnimation = GetObjectPointer( handle );
    if (!pAnimation)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    // If the property is NULL, the Animation will
    // release any target object it was previously bound to.
    CHR(pAnimation->BindTo( property ));
    
Exit:
    return rval;
}



RESULT
AnimationManager::SetAutoRepeat( IN HAnimation handle, bool willAutoRepeat )
{
    RESULT rval = S_OK;

    Animation* pAnimation = GetObjectPointer( handle );
    if (!pAnimation)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

     CHR(pAnimation->SetAutoRepeat( willAutoRepeat ));

Exit:
    return rval;
}



RESULT
AnimationManager::SetAutoReverse( IN HAnimation handle, bool willAutoReverse )
{
    RESULT rval = S_OK;

    Animation* pAnimation = GetObjectPointer( handle );
    if (!pAnimation)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

     CHR(pAnimation->SetAutoReverse( willAutoReverse ));

Exit:
    return rval;
}



RESULT
AnimationManager::SetDeleteOnFinish( IN HAnimation handle, bool willDeleteOnFinish )
{
    RESULT rval = S_OK;

    Animation* pAnimation = GetObjectPointer( handle );
    if (!pAnimation)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

     CHR(pAnimation->SetDeleteOnFinish( willDeleteOnFinish ));

Exit:
    return rval;
}



RESULT
AnimationManager::CallbackOnFinished( IN HAnimation handle, ICallback& callback )
{
    RESULT      rval = S_OK;
    Animation*  pAnimation;
    
    if ( callback.IsNull() )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: AnimationManager::CallbackOnFinished( %d, 0x%x ): callback IsNull",
            (UINT32)handle, (UINT32)&callback);
            
        rval = E_INVALID_ARG;
        goto Exit;
    }

    pAnimation = GetObjectPointer( handle );
    if (!pAnimation)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    CHR(pAnimation->CallbackOnFinished( callback ));
    
Exit:
    return rval;
}



RESULT
AnimationManager::Start( IN HAnimation handle )
{
    RESULT rval = S_OK;
    
    // Get the animation, add it to m_runningAnimationsList if not already started
    Animation* pAnimation = GetObjectPointer( handle );
    if (pAnimation && !pAnimation->IsStarted())
    {
        m_runningAnimationsList.push_back(pAnimation);
    }

    if (pAnimation)
    {
        RETAILMSG(ZONE_ANIMATION | ZONE_VERBOSE, "AnimationManager::Start( \"%s\" )", pAnimation->GetName().c_str());
        pAnimation->Start();
    }
    else 
    {
        rval = E_FAIL;
    }
    
Exit:
    return rval;
}

    

RESULT
AnimationManager::Stop( IN HAnimation handle )
{
    RESULT rval = S_OK;
    
    Animation* pAnimation = GetObjectPointer( handle );
    if (pAnimation && pAnimation->IsStarted())
    {
        RETAILMSG(ZONE_ANIMATION, "AnimationManager::Stop( \"%s\" )", pAnimation->GetName().c_str());
        pAnimation->Stop();

        AnimationListIterator ppAnimation;
        ppAnimation = find( m_runningAnimationsList.begin(), m_runningAnimationsList.end(), pAnimation );
        
        if (ppAnimation != m_runningAnimationsList.end())
        {
            m_runningAnimationsList.erase( ppAnimation );
        }
    }
    else 
    {
        rval = E_FAIL;
    }

Exit:
    return rval;
}



RESULT
AnimationManager::Pause( IN HAnimation handle )
{
    RESULT rval = S_OK;
    
    Animation* pAnimation = GetObjectPointer( handle );
    if (pAnimation && !pAnimation->IsStarted())
    {
        RETAILMSG(ZONE_ANIMATION | ZONE_VERBOSE, "AnimationManager::Pause( \"%s\" )", pAnimation->GetName().c_str());
        pAnimation->Pause();

        AnimationListIterator ppAnimation;
        ppAnimation = find( m_runningAnimationsList.begin(), m_runningAnimationsList.end(), pAnimation );
        
        if (ppAnimation != m_runningAnimationsList.end())
        {
            m_runningAnimationsList.erase( ppAnimation );
        }
    }
    else 
    {
        rval = E_FAIL;
    }

    
Exit:
    return rval;
}



RESULT
AnimationManager::Update( UINT64 elapsedMS )
{
    RESULT rval = S_OK;


    // Release Animations that were marked as done on previous frame.
    AnimationListIterator ppAnimation;
    for (ppAnimation = m_pendingReleaseAnimationsList.begin(); ppAnimation != m_pendingReleaseAnimationsList.end(); ++ppAnimation)
    {
        Animation* pAnimation = *ppAnimation;
        
        DEBUGMSG(ZONE_ANIMATION | ZONE_VERBOSE, "Releasing Animation [%s]", pAnimation->GetName().c_str());
        
        CHR(Release( pAnimation ));
    }
    m_pendingReleaseAnimationsList.clear();
    

    // Update running animation, which in turn will update its bound AnimationTarget
    for (ppAnimation = m_runningAnimationsList.begin(); ppAnimation != m_runningAnimationsList.end(); ++ppAnimation)
    {
        Animation* pAnimation = *ppAnimation;
        
        DEBUGMSG(ZONE_ANIMATION | ZONE_VERBOSE, "Ticking Animation [%s]", pAnimation->GetName().c_str());
        
        pAnimation->Update( elapsedMS );
    }

Exit:
    return rval;
}



RESULT
AnimationManager::Update( IN HAnimation handle, UINT64 elapsedMS )
{
    RESULT rval = S_OK;
    
    Animation* pAnimation = GetObjectPointer( handle );
    if (pAnimation && pAnimation->IsStarted())
    {
        //RETAILMSG(ZONE_ANIMATION | ZONE_VERBOSE, "AnimationManager::Update( \"%s\" )", pAnimation->GetName().c_str());
        
        CHR(pAnimation->Update( elapsedMS ));
    }
    else 
    {
        rval = E_FAIL;
    }
    
Exit:
    return rval;
}




UINT64
AnimationManager::GetDurationMS( IN HAnimation handle )
{
    UINT64 rval = 0;
    
    Animation* pAnimation = GetObjectPointer( handle );
    if (pAnimation)
    {
        rval = pAnimation->GetDurationMS();
    }
    
Exit:
    return rval;
}



UINT8
AnimationManager::GetNumKeyframes( IN HAnimation handle )
{
    UINT8 rval = 0;
    
    Animation* pAnimation = GetObjectPointer( handle );
    if (pAnimation)
    {
        rval = pAnimation->GetNumKeyFrames();
    }
    
Exit:
    return rval;
}



PropertyType 
AnimationManager::GetPropertyType( IN HAnimation handle )
{
    PropertyType rval = PROPERTY_UNKNOWN;
    
    Animation* pAnimation = GetObjectPointer( handle );
    if (pAnimation)
    {
        rval = pAnimation->GetPropertyType();
    }
    
Exit:
    return rval;
}



const string&
AnimationManager::GetPropertyName( IN HAnimation handle )
{
    static string emptystr = "";

    Animation* pAnimation = GetObjectPointer( handle );
    if (pAnimation)
    {
        return pAnimation->GetPropertyName();
    }
    else 
    {
        return emptystr;
    }
}




} // END namespace Z


