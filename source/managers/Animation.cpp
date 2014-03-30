#include "Animation.hpp"
#include "AnimationManager.hpp"

namespace Z
{



KEYFRAME_TYPE_MAP Animation::s_keyFrameTypeMap[] =
{
    { "float",      KEYFRAME_TYPE_FLOAT     },
    { "uint32",     KEYFRAME_TYPE_UINT32    },
    { "vec2",       KEYFRAME_TYPE_VEC2      },
    { "vec3",       KEYFRAME_TYPE_VEC3      },
    { "vec4",       KEYFRAME_TYPE_VEC4      },
    { "color",      KEYFRAME_TYPE_COLOR     },
};



// ============================================================================
//
//  Animation Implementation
//
// ============================================================================


#pragma mark Animation Implementation

Animation::Animation() :
    m_isBoundToProperty(false),
    m_propertyName(""),
    m_propertyType(PROPERTY_UNKNOWN),
    m_pTargetProperty(NULL),
    m_pCallbackOnFinished(NULL),
    m_autoRepeat(false),
    m_autoReverse(false),
    m_relativeToCurrentState(false),
    m_deleteOnFinish(true),
    m_durationMS(0),
    m_startTimeMS(0),
    m_keyFrameType(KEYFRAME_TYPE_UNKNOWN),
    m_pKeyFrames(NULL),
    m_numKeyFrames(0),
    m_currKeyFrame(0),
    m_nextKeyFrame(1),
    m_direction(DIRECTION_FORWARD),
    m_interpolatorType(INTERPOLATOR_TYPE_UNKNOWN),
    m_pInterpolator(NULL),
    m_isStarted(false),
    m_isPaused(false)
{
    RETAILMSG(ZONE_OBJECT, "Animation( %4d )", m_ID);
}



Animation::~Animation()
{
    DEBUGMSG(ZONE_OBJECT, "\t~Animation( %4d, \"%s\" )", m_ID, m_name.c_str());
    
    SAFE_DELETE(m_pCallbackOnFinished);
    SAFE_DELETE(m_pInterpolator);
    SAFE_DELETE(m_pTargetProperty);
    SAFE_ARRAY_DELETE(m_pKeyFrames);
}



Animation*
Animation::Clone() const
{
    return new Animation(*this);
}


Animation::Animation( const Animation& rhs ) : 
    Object(),
    m_pKeyFrames(NULL),
    m_pTargetProperty(NULL),
    m_pInterpolator(NULL),
    m_pCallbackOnFinished(NULL)
{
    *this = rhs;
}


Animation& 
Animation::operator=( const Animation& rhs )
{
    // Avoid self-assignment.
    if (this == &rhs)
        return *this;
    

    // Explicitly DO NOT copy the base class state.
    // We DO NOT want to copy the Object::m_RefCount, m_ID, or m_name from the copied Object.


    // SHALLOW COPY:
    m_isBoundToProperty         = rhs.m_isBoundToProperty;
    m_propertyName              = rhs.m_propertyName;
    m_propertyType              = rhs.m_propertyType;
    m_autoRepeat                = rhs.m_autoRepeat;
    m_autoReverse               = rhs.m_autoReverse;
    m_relativeToCurrentState    = rhs.m_relativeToCurrentState;
    m_deleteOnFinish            = rhs.m_deleteOnFinish;
    m_durationMS                = rhs.m_durationMS;
    m_startTimeMS               = rhs.m_startTimeMS;
    m_keyFrameType              = rhs.m_keyFrameType;
    m_numKeyFrames              = rhs.m_numKeyFrames;
    m_currKeyFrame              = rhs.m_currKeyFrame;
    m_nextKeyFrame              = rhs.m_nextKeyFrame;
    m_direction                 = rhs.m_direction;
    m_interpolatorType          = rhs.m_interpolatorType;
    m_isStarted                 = rhs.m_isStarted;
    m_isPaused                  = rhs.m_isPaused;

    SAFE_ARRAY_DELETE(m_pKeyFrames);
    SAFE_DELETE(m_pInterpolator);
    SAFE_DELETE(m_pTargetProperty);
    SAFE_DELETE(m_pCallbackOnFinished);

    // DEEP COPY: m_pKeyFrames
    if (rhs.m_numKeyFrames)
    {
        m_pKeyFrames = new KeyFrame[rhs.m_numKeyFrames];
        DEBUGCHK(m_pKeyFrames);
        memcpy(m_pKeyFrames, rhs.m_pKeyFrames, sizeof(KeyFrame)*rhs.m_numKeyFrames);
    }
    
    
    // DEEP COPY: m_pInterpolator
    switch (rhs.m_interpolatorType)
    {
        case INTERPOLATOR_TYPE_LINEAR:
            m_pInterpolator = new LinearInterpolator();
            break;
        case INTERPOLATOR_TYPE_QUADRATIC_IN:
            m_pInterpolator = new QuadraticEaseInInterpolator();
            break;
        case INTERPOLATOR_TYPE_QUADRATIC_OUT:
            m_pInterpolator = new QuadraticEaseOutInterpolator();
            break;
        case INTERPOLATOR_TYPE_QUADRATIC_INOUT:
            m_pInterpolator = new QuadraticEaseInOutInterpolator();
            break;

        case INTERPOLATOR_TYPE_ELASTIC_IN:
            m_pInterpolator = new ElasticEaseInInterpolator();
            break;
        
        default:
            DEBUGCHK(0);
    }
    
    // DEEP COPY: m_pTargetProperty
    if (rhs.m_pTargetProperty)
    {
        m_pTargetProperty = rhs.m_pTargetProperty->Clone();
    }
    
    
    // DEEP COPY: m_pCallbackOnFinished
    if (rhs.m_pCallbackOnFinished)
    {
        m_pCallbackOnFinished = rhs.m_pCallbackOnFinished->Clone();
    }
    
    // Give it a new name with a random suffix
    char instanceName[MAX_NAME];
    sprintf(instanceName, "%s_%X", rhs.m_name.c_str(), (unsigned int)Platform::Random());
    m_name = string(instanceName);
    
    // Reset state
    m_isStarted    = false;
    m_isPaused     = false;
    m_currKeyFrame = 0;
    m_nextKeyFrame = 0;
    
    return *this;
}






RESULT
Animation::Init ( 
        IN const string&            name, 
        IN const string&            propertyName, 
        IN       PropertyType       propertyType, 
        IN       InterpolatorType   interpolatorType, 
        IN       KeyFrameType       keyFrameType, 
        IN const KeyFrame*          pKeyFrames, 
        IN       UINT8              numKeyFrames, 
        IN       bool               isRelative 
    )
{
    RESULT rval = S_OK;
    
    if (!pKeyFrames || !numKeyFrames)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Animation::Init(): must pass pKeyFrames, numKeyFrames.");
        rval = E_INVALID_DATA;
        goto Exit;
    }
    

    if (name == "")
    {
        char randomName[MAX_NAME];
        sprintf(randomName, "Animation_%X", (unsigned int)Platform::Random());
        m_name = randomName;
    }
    else 
    {
        m_name = name;
    }

    m_propertyName = propertyName;
    
    switch (propertyType) 
    {
        case PROPERTY_FLOAT:
        case PROPERTY_UINT32:
        case PROPERTY_BOOL:
        case PROPERTY_VEC2:
        case PROPERTY_VEC3:
        case PROPERTY_VEC4:
        case PROPERTY_IVEC2:
        case PROPERTY_IVEC3:
        case PROPERTY_IVEC4:
            m_propertyType = propertyType;
            break;
        default:
            RETAILMSG(ZONE_ERROR, "ERROR: Animation::Init(): unknown property type [%d]", propertyType);
            rval = E_INVALID_DATA;
            goto Exit;
    }
    
    
    switch (interpolatorType) 
    {
        case INTERPOLATOR_TYPE_LINEAR:
        case INTERPOLATOR_TYPE_QUADRATIC_IN:
        case INTERPOLATOR_TYPE_QUADRATIC_OUT:
        case INTERPOLATOR_TYPE_QUADRATIC_INOUT:
        case INTERPOLATOR_TYPE_ELASTIC_IN:
            m_interpolatorType = interpolatorType;
            break;
        default:
            RETAILMSG(ZONE_ERROR, "ERROR: Animation::Init(): unknown interpolator type [%d]", interpolatorType);
            rval = E_INVALID_DATA;
            goto Exit;
    }
    

    switch (keyFrameType) 
    {
        case KEYFRAME_TYPE_FLOAT:
        case KEYFRAME_TYPE_UINT32:
        case KEYFRAME_TYPE_VEC2:
        case KEYFRAME_TYPE_VEC3:
        case KEYFRAME_TYPE_VEC4:
        case KEYFRAME_TYPE_COLOR:
            m_keyFrameType = keyFrameType;
            break;
        default:
            RETAILMSG(ZONE_ERROR, "ERROR: Animation::Init(): unknown keyframe type [%d]", keyFrameType);
            DEBUGCHK(0);
            rval = E_INVALID_DATA;
            goto Exit;
    }

    m_pKeyFrames             = new KeyFrame[ numKeyFrames ];   
    CPR(m_pKeyFrames);
    memcpy( m_pKeyFrames, pKeyFrames, numKeyFrames*sizeof(KeyFrame) );

    m_numKeyFrames           = numKeyFrames;
    m_relativeToCurrentState = isRelative;
    m_durationMS             = m_pKeyFrames[ m_numKeyFrames-1 ].GetTimeMS();

    RETAILMSG(ZONE_ANIMATION, "Animation[%4d]: %2d frames %d MSec \"%-32s\"", m_ID, m_numKeyFrames, m_durationMS, m_name.c_str());
    
Exit:
    return rval;
}



RESULT
Animation::Init( const string& name, const Settings* pSettings, const string& settingsPath )
{
    RESULT      rval        = S_OK;
    char        path[MAX_PATH];
    string      type;
    string      interpolator;
    string      propertyType;
    
    RETAILMSG(ZONE_OBJECT, "Animation[%4d]::Init( %s )", m_ID, name.c_str());
    
    m_name = name;
    
    if (!pSettings)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Animation::Init(): NULL settings");
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    //
    // Get the KeyFrame type
    //
    type = pSettings->GetString( settingsPath + ".Type" );
    for (int i = 0; i < ARRAY_SIZE(s_keyFrameTypeMap); ++i)
    {
        if ( !strcasecmp( type.c_str(), s_keyFrameTypeMap[i].name ) )
        {
            m_keyFrameType = s_keyFrameTypeMap[i].type;
            break;
        }
    }
    if (KEYFRAME_TYPE_UNKNOWN == m_keyFrameType)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Animation::Init(): unknown keyframe type [%s]", type.c_str());
//        DEBUGCHK(0);
        rval = E_INVALID_DATA;
        goto Exit;
    }
    
    //
    // Get the Interpolator type
    //
    interpolator = pSettings->GetString( settingsPath + ".Interpolator" );
    if ("Linear" == interpolator)
    {
        m_interpolatorType  = INTERPOLATOR_TYPE_LINEAR;
        m_pInterpolator     = new LinearInterpolator();
    }
    else if ("QuadraticEaseIn" == interpolator)
    {
        m_interpolatorType  = INTERPOLATOR_TYPE_QUADRATIC_IN;
        m_pInterpolator     = new QuadraticEaseInInterpolator();
    }
    else if ("QuadraticEaseOut" == interpolator)
    {
        m_interpolatorType  = INTERPOLATOR_TYPE_QUADRATIC_OUT;
        m_pInterpolator     = new QuadraticEaseOutInterpolator();
    }
    else if ("QuadraticEaseInOut" == interpolator)
    {
        m_interpolatorType  = INTERPOLATOR_TYPE_QUADRATIC_INOUT;
        m_pInterpolator     = new QuadraticEaseInOutInterpolator();
    }
    else if ("ElasticIn" == interpolator)
    {
        m_interpolatorType  = INTERPOLATOR_TYPE_ELASTIC_IN;
        m_pInterpolator     = new ElasticEaseInInterpolator();
    }
    else
    {
        // Interpolator type is optional (it may have been specified by the parent Storyboard).
        // Default to Linear rather than failing.
        m_interpolatorType  = INTERPOLATOR_TYPE_LINEAR;
        m_pInterpolator     = new LinearInterpolator();
    }

    
    //
    // Get the Property type
    // TODO: totally redundant with KeyFrameType; collapse them.
    //
    type            = pSettings->GetString( settingsPath + ".Type" );
    m_propertyType  = PropertyTypeFromName( type );
    if (PROPERTY_UNKNOWN == m_propertyType)
    {
        RETAILMSG(ZONE_ANIMATION, "ERROR: Animation::Init(): unknown PropertyType [%s]", propertyType.c_str());
        rval = E_INVALID_DATA;
        goto Exit;
    }


    //
    // Get the Property name
    //
    m_propertyName = pSettings->GetString( settingsPath + ".Property" );
    if ("" == m_propertyName)
    {
        RETAILMSG(ZONE_ANIMATION, "ERROR: Animation::Init(): .Property not specified");
        rval = E_INVALID_DATA;
        goto Exit;
    }


    //
    // Is Animation relative to absolute?
    //
    m_relativeToCurrentState = pSettings->GetBool( settingsPath + ".RelativeToObject", m_relativeToCurrentState );
    

    //
    // Create the KeyFrames
    //
    m_numKeyFrames  = pSettings->GetInt( settingsPath + "/KeyFrames.NumKeyFrames" );
    m_pKeyFrames    = new KeyFrame[m_numKeyFrames];
    DEBUGCHK(m_pKeyFrames);
    memset(m_pKeyFrames, 0, sizeof(KeyFrame)*m_numKeyFrames);
    
    for (int i = 0; i < m_numKeyFrames; ++i)
    {
        sprintf(path, "%s/KeyFrames/KeyFrame%d", settingsPath.c_str(), i);
        DEBUGMSG(ZONE_ANIMATION | ZONE_VERBOSE, "Creating [%s]", path);
        string settingsPath(path);
        
        UINT64 keyframeTime = pSettings->GetInt( settingsPath + ".TimeMS", 0 );
        m_pKeyFrames[i].SetTimeMS( keyframeTime );

        m_durationMS = MAX(m_durationMS, keyframeTime);
        
        switch (m_keyFrameType)
        {
            case KEYFRAME_TYPE_UINT32:
                m_pKeyFrames[i].SetIntValue( pSettings->GetInt( settingsPath + ".Value" ) );
                break;
            case KEYFRAME_TYPE_FLOAT:
                m_pKeyFrames[i].SetFloatValue( pSettings->GetFloat( settingsPath + ".Value" ) );
                break;
            case KEYFRAME_TYPE_VEC2:
                m_pKeyFrames[i].SetVec2Value( pSettings->GetVec2( settingsPath + ".Value" ) );
                break;
            case KEYFRAME_TYPE_VEC3:
                m_pKeyFrames[i].SetVec3Value( pSettings->GetVec3( settingsPath + ".Value" ) );
                break;
            case KEYFRAME_TYPE_VEC4:
                m_pKeyFrames[i].SetVec4Value( pSettings->GetVec4( settingsPath + ".Value" ) );
                break;
            case KEYFRAME_TYPE_COLOR:
                m_pKeyFrames[i].SetColorValue( pSettings->GetColor( settingsPath + ".Value" ) );
                break;
            default:
                DEBUGCHK(0);
        }
    }
    
    
    RETAILMSG(ZONE_ANIMATION, "Animation[%4d]: %2d frames %d MSec \"%-32s\" %s: %s", m_ID, m_numKeyFrames, m_durationMS, m_name.c_str(), interpolator.c_str(), type.c_str());
    
Exit:
    return rval;
}



RESULT
Animation::Start( )
{ 
    DEBUGMSG(ZONE_ANIMATION, "START Animation \"%s\"", m_name.c_str());

    m_isStarted     = true;  
    m_isPaused      = false; 
    
    m_currKeyFrame  = 0;
//    m_nextKeyFrame  = 0;
    m_nextKeyFrame  = 1;
    m_startTimeMS   = GameTime.GetTime(); // TODO: defer to first Update() call???  The StateMachines have too much latency for short animations. :-(
    
    return S_OK;
}



RESULT
Animation::Pause( )
{ 
    DEBUGMSG(ZONE_ANIMATION, "PAUSE Animation \"%s\"", m_name.c_str());

    m_isPaused      = true; 

    return S_OK;
}



RESULT
Animation::Stop( )
{ 
    DEBUGMSG(ZONE_ANIMATION, "STOP Animation \"%s\"", m_name.c_str());

    if (!m_isStarted && !m_isPaused)
    {
//        RETAILMSG(ZONE_WARN, "WARNING: Animation \"%s\" already stopped.", m_name.c_str());
        return E_INVALID_OPERATION;
    }

    m_isStarted     = false;  
    m_isPaused      = false; 
    m_currKeyFrame  = 0;
    m_nextKeyFrame  = 0;
    
    if (m_pCallbackOnFinished)
    {
        m_pCallbackOnFinished->Invoke();
    }
    
//    if (m_deleteOnFinish)
    if (m_deleteOnFinish && m_refCount > 0)
    {
        DEBUGMSG(ZONE_ANIMATION, "Animation::Stop(): \"%s\" DELETE on finish.", m_name.c_str());
        AnimationMan.ReleaseOnNextFrame( this );
    }
    
    // TODO: release our m_pTargetProperty ?  Or keep in in case we're started again in the future?

    return S_OK;
}



// TODO: Break this function up; it's too long.
RESULT
Animation::Update( UINT64 currentMS )
{
    RESULT rval = S_OK;
    UINT64 elapsedAnimationMS;
    

    if (!m_isStarted || !m_isBoundToProperty)
    {
        return E_NOTHING_TO_DO;
    }
    

    if ( NULL == m_pTargetValue && ( NULL == m_pTargetProperty || m_pTargetProperty->IsNull()) )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Animation::Update(): NULL target.");
        DEBUGCHK(0);
        return E_ACCESS_DENIED;
    }
    
    
    // 
    // Special case: one keyframe
    //
    if (1 == m_numKeyFrames)
    {
        UpdateTarget( &m_pKeyFrames[0], &m_pKeyFrames[0], 1.0 );
        this->Stop();
        
        return S_OK;
    }
    
    
    //
    // Is the animation ticking forwards or backwards?
    //
    if (DIRECTION_FORWARD == m_direction)
    {
        elapsedAnimationMS = currentMS - m_startTimeMS;
    }
    else 
    {
        // Run the animation clock backwards
        if ( currentMS < m_startTimeMS + m_durationMS )
        {
            elapsedAnimationMS = (m_startTimeMS + m_durationMS) - currentMS;
        }
        else
        {
            // Prevent underflow of elapsedAnimationMS (it's UNSIGNED). 
            elapsedAnimationMS = 0;
        }
    }
    
    
    //
    // Repeat, reverse, or stop and delete the animation as needed.
    //
    if ((elapsedAnimationMS >= m_durationMS) || (0 == elapsedAnimationMS && m_direction == DIRECTION_REVERSE))
    {
        DEBUGMSG(ZONE_ANIMATION, "Animation::Update(): END of \"%s\" start: %llu current: %llu elapsed: %llu duration: %llu", 
            m_name.c_str(), m_startTimeMS, currentMS, elapsedAnimationMS, m_durationMS);
    
        // TODO: This causes Animations to DRIFT when there are a ton of them, because
        // Animations started at the same moment don't reset at the same moment!
        m_startTimeMS      = GameTime.GetTime();


        //
        // Set target property to the final keyframe to ensure the animation completes before repeating.
        // This is important for UINT32 animations like SpriteFrame, where otherwise the final frame might not display.
        //
        if (m_direction == DIRECTION_FORWARD)
        {
            UpdateTarget( &m_pKeyFrames[m_numKeyFrames-1], &m_pKeyFrames[m_numKeyFrames-1], 1.0f );
        }
        else 
        {
            UpdateTarget( &m_pKeyFrames[0], &m_pKeyFrames[0], 0.0f );
        }

        if (m_autoReverse && m_direction == DIRECTION_FORWARD)
        {
            DEBUGMSG(ZONE_ANIMATION, "Animation::Update(): REVERSE \"%s\"", m_name.c_str());
            m_direction = DIRECTION_REVERSE;

            return S_OK;
        }
        
        if (m_autoReverse && m_direction == DIRECTION_REVERSE && m_autoRepeat)
        {
            DEBUGMSG(ZONE_ANIMATION, "Animation::Update(): FORWARD \"%s\"", m_name.c_str());
            m_direction = DIRECTION_FORWARD;
            
            m_currKeyFrame = 0;
            m_nextKeyFrame = MIN(m_currKeyFrame+1, m_numKeyFrames-1);

            return S_OK;
        }

        
        if (m_autoRepeat)
        {
            DEBUGMSG(ZONE_ANIMATION, "Animation::Update(): REPEAT \"%s\"", m_name.c_str());

            elapsedAnimationMS = elapsedAnimationMS % m_durationMS;

            m_currKeyFrame = 0;
            m_nextKeyFrame = MIN(m_currKeyFrame+1, m_numKeyFrames-1);
        }
        else 
        {
            DEBUGMSG(ZONE_ANIMATION, "Animation::Update(): \"%s\" STOP", m_name.c_str());

            this->Stop();

            return S_OK;
        }
    }
    
    
    //
    // Find current pair of keyframes.
    //
    while ( m_currKeyFrame     < m_numKeyFrames &&
            m_nextKeyFrame     < m_numKeyFrames &&
            elapsedAnimationMS >= m_pKeyFrames[m_nextKeyFrame].GetTimeMS() )
    {
        m_currKeyFrame++;
        m_nextKeyFrame = MIN(m_currKeyFrame+1, m_numKeyFrames-1);
    }
    DEBUGCHK(m_currKeyFrame <= m_nextKeyFrame);
    

    //
    // Handle the case where time is ticking backwards.
    //
    if (elapsedAnimationMS < m_pKeyFrames[m_currKeyFrame].GetTimeMS())
    {
        if (m_currKeyFrame > 0)
            --m_currKeyFrame;
            
        m_nextKeyFrame = MIN(m_currKeyFrame+1, m_numKeyFrames-1);
    }

    // Bounds check.
///////////////    DEBUGCHK( m_pKeyFrames[m_currKeyFrame].GetTimeMS() <= elapsedAnimationMS && elapsedAnimationMS < m_pKeyFrames[m_nextKeyFrame].GetTimeMS() );
    
    //
    // Interpolate between two KeyFrames.
    //
    double    progress;
    KeyFrame* pFrame1                   = &m_pKeyFrames[m_currKeyFrame];
    KeyFrame* pFrame2                   = &m_pKeyFrames[m_nextKeyFrame];
    UINT32    intervalBetweenFramesMS   = pFrame2->GetTimeMS() - pFrame1->GetTimeMS();
    
    if (intervalBetweenFramesMS > 0)
    {
        UINT64 elapsedFrameMS   = elapsedAnimationMS - pFrame1->GetTimeMS();
        progress = (double)elapsedFrameMS / (double)intervalBetweenFramesMS;
        progress = MIN(progress, 1.0f);
    }
    else 
    {
        progress = 0.0;
    }
    
    
    DEBUGMSG(ZONE_ANIMATION | ZONE_VERBOSE, "[%llu of %llu] \"%s\" frames[%d, %d] progress: %2.3f", elapsedAnimationMS, m_durationMS, m_name.c_str(), m_currKeyFrame, m_nextKeyFrame, progress);
    
    
    UpdateTarget( pFrame1, pFrame2, progress );


Exit:
    return rval;
}




RESULT
Animation::UpdateTarget( KeyFrame* pFrame1, KeyFrame* pFrame2, float progress )
{
    RESULT rval = S_OK;

    CPR(pFrame1);
    CPR(pFrame2);

    if (!m_isBoundToProperty)
    {
        DEBUGCHK(0);
        rval = E_INVALID_OPERATION;
        goto Exit;
    }

    DEBUGCHK(m_pTargetProperty);
    switch (m_keyFrameType)
    {
        case KEYFRAME_TYPE_UINT32:
        {
            UINT32 value = (*m_pInterpolator)( (UINT32)pFrame1->GetIntValue(), (UINT32)pFrame2->GetIntValue(), progress );
            
            value += m_startingValue.GetIntValue();
            
            m_pTargetProperty->SetInteger( value );
        }
        break;
 
        case KEYFRAME_TYPE_FLOAT:
        {
            float value = (*m_pInterpolator)( (float)pFrame1->GetFloatValue(), (float)pFrame2->GetFloatValue(), progress );
            
            value += m_startingValue.GetFloatValue();
            
            m_pTargetProperty->SetFloat( value );
        }
        break;
            
        case KEYFRAME_TYPE_VEC2:
        {
            vec2 value = (*m_pInterpolator)( (vec2)pFrame1->GetVec2Value(), (vec2)pFrame2->GetVec2Value(), progress );
            
            value += m_startingValue.GetVec2Value();
            
            m_pTargetProperty->SetVec2( value );
        }
        break;
            
        case KEYFRAME_TYPE_VEC3:
        {
            vec3 value = (*m_pInterpolator)( (vec3)pFrame1->GetVec3Value(), (vec3)pFrame2->GetVec3Value(), progress );
            
            value += m_startingValue.GetVec3Value();
            
            m_pTargetProperty->SetVec3( value );
        }
        break;
            
        case KEYFRAME_TYPE_VEC4:
        {
            vec4 value = (*m_pInterpolator)( (vec4)pFrame1->GetVec4Value(), (vec4)pFrame2->GetVec4Value(), progress );
            
            value += m_startingValue.GetVec4Value();
            
            m_pTargetProperty->SetVec4( value );
        }
        break;
            
        case KEYFRAME_TYPE_COLOR:
        {
            Color value = (*m_pInterpolator)( (Color)pFrame1->GetColorValue(), (Color)pFrame2->GetColorValue(), progress );
            
            value += m_startingValue.GetColorValue();
            
            m_pTargetProperty->SetColor( value );
        }
        break;
            
        default:
            DEBUGCHK(0);
            break;
    }

Exit:
    return rval;
}



RESULT
Animation::SetInterpolatorType( InterpolatorType type )
{
    RESULT rval = S_OK;

    SAFE_DELETE(m_pInterpolator);
    m_interpolatorType = type;

    switch (m_interpolatorType)
    {
        case INTERPOLATOR_TYPE_LINEAR:
            m_pInterpolator = new LinearInterpolator();
            break;
        case INTERPOLATOR_TYPE_QUADRATIC_IN:
            m_pInterpolator = new QuadraticEaseInInterpolator();
            break;
        case INTERPOLATOR_TYPE_QUADRATIC_OUT:
            m_pInterpolator = new QuadraticEaseOutInterpolator();
            break;
        case INTERPOLATOR_TYPE_QUADRATIC_INOUT:
            m_pInterpolator = new QuadraticEaseInOutInterpolator();
            break;

        case INTERPOLATOR_TYPE_ELASTIC_IN:
            m_pInterpolator = new ElasticEaseInInterpolator();
            break;
        
        default:
            DEBUGCHK(0);
    }
    
    return rval;
}



RESULT
Animation::BindTo( IProperty& property )
{
    RESULT rval = S_OK;
   
    if (property.IsNull())
    {
        SAFE_DELETE(m_pTargetProperty);
        m_isBoundToProperty = false;
        Stop();
        
        goto Exit;
    }

    
    if (property.GetType() != m_propertyType)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Animation::BindTo( \"%s\", 0x%x ): target Property type 0x%x incorrect for this Animation",
                  m_name.c_str(), (UINT32)&property, property.GetType());
        rval = E_INVALID_OPERATION;
        goto Exit;
    }
    

    SAFE_DELETE(m_pTargetProperty);
    m_pTargetProperty   = property.Clone();
    m_isBoundToProperty = true;    
    
    if (m_relativeToCurrentState)
    {
        // Save the target Property into our m_startingValue.
        // Keyframes will be interpolated, then added to m_startingValue to produce the final result.
        switch (m_keyFrameType)
        {
            case KEYFRAME_TYPE_UINT32:
                m_startingValue.SetIntValue( m_pTargetProperty->GetInteger() );
                break;
            case KEYFRAME_TYPE_FLOAT:
                m_startingValue.SetFloatValue( m_pTargetProperty->GetFloat() );
                break;
            case KEYFRAME_TYPE_VEC2:
                m_startingValue.SetVec2Value( m_pTargetProperty->GetVec2() );
                break;
            case KEYFRAME_TYPE_VEC3:
                m_startingValue.SetVec3Value( m_pTargetProperty->GetVec3() );
                break;
            case KEYFRAME_TYPE_VEC4:
                m_startingValue.SetVec4Value( m_pTargetProperty->GetVec4() );
                break;
            case KEYFRAME_TYPE_COLOR:
                m_startingValue.SetColorValue( m_pTargetProperty->GetColor() );
                break;
            default:
                DEBUGCHK(0);
                break;
        }
    }


Exit:
    return rval;
}



RESULT
Animation::CallbackOnFinished( ICallback& callback )
{
    RESULT rval = S_OK;
    
    if (callback.IsNull())
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Animation::CallbackOnFinished( \"%s\", 0x%x ): Callback is NULL",
                  m_name.c_str(), (UINT32)&callback);
        
        SAFE_DELETE(m_pCallbackOnFinished);
        
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    SAFE_DELETE(m_pCallbackOnFinished);
    m_pCallbackOnFinished = callback.Clone();
    
Exit:
    return rval;
}



} // END namespace Z


