#pragma once

#include "Types.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include "Settings.hpp"
#include "Time.hpp"
#include "Property.hpp"
#include "Callback.hpp"
#include "GameObject.hpp"   // TODO: should be able to yank this, if we get the include order right.


namespace Z
{


//=============================================================================
//
// KeyFrame Interpolators.
//
//=============================================================================

typedef enum
{
    INTERPOLATOR_TYPE_UNKNOWN = 0,
    INTERPOLATOR_TYPE_LINEAR,
    INTERPOLATOR_TYPE_QUADRATIC_IN,
    INTERPOLATOR_TYPE_QUADRATIC_OUT,
    INTERPOLATOR_TYPE_QUADRATIC_INOUT,

    INTERPOLATOR_TYPE_ELASTIC_IN,
} InterpolatorType;


class IInterpolator
{
public:
	virtual float   operator()( float  start, float  end, float ratio )    = 0;
	virtual UINT32  operator()( UINT32 start, UINT32 end, float ratio )    = 0;
	virtual vec2    operator()( vec2   start, vec2   end, float ratio )    = 0;
	virtual vec3    operator()( vec3   start, vec3   end, float ratio )    = 0;
	virtual vec4    operator()( vec4   start, vec4   end, float ratio )    = 0;
	virtual Color   operator()( Color  start, Color  end, float ratio )    = 0;
    
    virtual ~IInterpolator() {};
};


class LinearInterpolator : public IInterpolator
{
public:
	virtual float operator()( float start, float end, float ratio )
	{
		return ((1.0f - ratio) * start) + (ratio * end);
	}
    
	virtual UINT32 operator()( UINT32 start, UINT32 end, float ratio )
	{
		return floorf(((1.0f - ratio) * start)) + ceilf((ratio * end));
	}

	virtual vec3   operator()( vec3 start, vec3 end, float ratio )
	{
        vec3 rval(0,0,0);
        
        start *= (1.0f - ratio);
        end   *= ratio;
        rval   = start + end;
                
		return rval;
	}

	virtual vec2   operator()( vec2 start, vec2 end, float ratio )
	{
        vec2 rval(0,0);
        
        start *= (1.0f - ratio);
        end   *= ratio;
        rval   = start + end;
        
		return rval;
	}
    
	virtual vec4   operator()( vec4 start, vec4 end, float ratio )
	{
        vec4 rval(0,0,0,0);
        
        start *= (1.0f - ratio);
        end   *= ratio;
        rval   = start + end;
                
		return rval;
	}

	virtual Color  operator()( Color start, Color end, float ratio )
	{
        Color rval(0.0f, 0.0f, 0.0f, 0.0f);
        
        start *= (1.0f - ratio);
        end   *= ratio;
        rval   = start + end;

		return rval;
	}


    virtual ~LinearInterpolator() {};
};



class QuadraticEaseInInterpolator : public IInterpolator
{
public:
	virtual float operator()( float start, float end, float ratio )
	{
        float change = end-start;
        return (change * ratio * ratio) + start;
	}
    
	virtual UINT32 operator()( UINT32 start, UINT32 end, float ratio )
	{
        float change = end-start;
        return floorf(change * ratio * ratio) + start;
	}

	virtual vec3   operator()( vec3 start, vec3 end, float ratio )
	{
        vec3 rval(0,0,0);
        
        rval.x = (*this)(start.x, end.x, ratio);
        rval.y = (*this)(start.y, end.y, ratio);
        rval.z = (*this)(start.z, end.z, ratio);

		return rval;
	}

	virtual vec2   operator()( vec2 start, vec2 end, float ratio )
	{
        vec2 rval(0,0);
        
        rval.x = (*this)(start.x, end.x, ratio);
        rval.y = (*this)(start.y, end.y, ratio);

		return rval;
	}
    
	virtual vec4   operator()( vec4 start, vec4 end, float ratio )
	{
        vec4 rval(0,0,0,0);
        
        rval.x = (*this)(start.x, end.x, ratio);
        rval.y = (*this)(start.y, end.y, ratio);
        rval.z = (*this)(start.z, end.z, ratio);
        rval.w = (*this)(start.w, end.w, ratio);

		return rval;
	}

	virtual Color  operator()( Color start, Color end, float ratio )
	{
        Color rval(0.0f, 0.0f, 0.0f, 0.0f);
        
        rval.floats.r = (*this)(start.floats.r, end.floats.r, ratio);
        rval.floats.g = (*this)(start.floats.g, end.floats.g, ratio);
        rval.floats.b = (*this)(start.floats.b, end.floats.b, ratio);
        rval.floats.a = (*this)(start.floats.a, end.floats.a, ratio);

		return rval;
	}


    virtual ~QuadraticEaseInInterpolator() {};
};



class QuadraticEaseOutInterpolator : public IInterpolator
{
public:
	virtual float operator()( float start, float end, float ratio )
	{
        float change = end-start;
        return -change * (ratio * (ratio - 2.0f)) + start;
	}
    
	virtual UINT32 operator()( UINT32 start, UINT32 end, float ratio )
	{
        UINT32 change = end-start;
        return -change * (ratio * (ratio - 2)) + start;
	}

	virtual vec3   operator()( vec3 start, vec3 end, float ratio )
	{
        vec3 rval(0,0,0);
        
        rval.x = (*this)( start.x, end.x, ratio );
        rval.y = (*this)( start.y, end.y, ratio );
        rval.z = (*this)( start.z, end.z, ratio );
                
		return rval;
	}

	virtual vec2   operator()( vec2 start, vec2 end, float ratio )
	{
        vec2 rval(0,0);
        
        rval.x = (*this)( start.x, end.x, ratio );
        rval.y = (*this)( start.y, end.y, ratio );
        
		return rval;
	}
    
	virtual vec4   operator()( vec4 start, vec4 end, float ratio )
	{
        vec4 rval(0,0,0,0);
        
        rval.x = (*this)( start.x, end.x, ratio );
        rval.y = (*this)( start.y, end.y, ratio );
        rval.z = (*this)( start.z, end.z, ratio );
        rval.w = (*this)( start.w, end.w, ratio );
                
		return rval;
	}

	virtual Color  operator()( Color start, Color end, float ratio )
	{
        Color rval(0.0f, 0.0f, 0.0f, 0.0f);
        
        rval.floats.r = (*this)( start.floats.r, end.floats.r, ratio );
        rval.floats.g = (*this)( start.floats.g, end.floats.g, ratio );
        rval.floats.b = (*this)( start.floats.b, end.floats.b, ratio );
        rval.floats.a = (*this)( start.floats.a, end.floats.a, ratio );

		return rval;
	}


    virtual ~QuadraticEaseOutInterpolator() {};
};



class QuadraticEaseInOutInterpolator : public IInterpolator
{
public:
	virtual float operator()( float start, float end, float ratio )
	{
        float change = end-start;
    
        ratio *= 2.0f;
        if (ratio < 1.0f) return (change/2.0f * ratio * ratio) + start;
        ratio -= 1.0;
        return -change/2.0f * (ratio * (ratio - 2.0f) - 1.0f) + start;
	}
    
	virtual UINT32 operator()( UINT32 start, UINT32 end, float ratio )
	{
        UINT32 change = end-start;
    
        ratio *= 2.0f;
        if (ratio < 1.0f) return (UINT32)(change/2.0f * ratio * ratio + start);
        ratio -= 1.0;
        return (UINT32)(-change/2.0f * (ratio * (ratio - 2.0f) - 1.0f) + start);
	}

	virtual vec3   operator()( vec3 start, vec3 end, float ratio )
	{
        vec3 rval(0,0,0);
        
        rval.x = (*this)( start.x, end.x, ratio );
        rval.y = (*this)( start.y, end.y, ratio );
        rval.z = (*this)( start.z, end.z, ratio );
                
		return rval;
	}

	virtual vec2   operator()( vec2 start, vec2 end, float ratio )
	{
        vec2 rval(0,0);
        
        rval.x = (*this)( start.x, end.x, ratio );
        rval.y = (*this)( start.y, end.y, ratio );
        
		return rval;
	}
    
	virtual vec4   operator()( vec4 start, vec4 end, float ratio )
	{
        vec4 rval(0,0,0,0);
        
        rval.x = (*this)( start.x, end.x, ratio );
        rval.y = (*this)( start.y, end.y, ratio );
        rval.z = (*this)( start.z, end.z, ratio );
        rval.w = (*this)( start.w, end.w, ratio );
                
		return rval;
	}

	virtual Color  operator()( Color start, Color end, float ratio )
	{
        Color rval(0.0f, 0.0f, 0.0f, 0.0f);
        
        rval.floats.r = (*this)( start.floats.r, end.floats.r, ratio );
        rval.floats.g = (*this)( start.floats.g, end.floats.g, ratio );
        rval.floats.b = (*this)( start.floats.b, end.floats.b, ratio );
        rval.floats.a = (*this)( start.floats.a, end.floats.a, ratio );

		return rval;
	}


    virtual ~QuadraticEaseInOutInterpolator() {};
};




class ElasticEaseInInterpolator : public IInterpolator
{
public:
	virtual float operator()( float start, float end, float ratio )
	{
        float change = end-start;
        float s = 0;
        float duration = 500;           // HACK HACK
        float period = change * 0.3;    // HACK HACK
        float amplitude = 64.0;         // HACK HACK
        
        if (ratio == 0 ) return start;

        if (ratio == 1 ) return start + end;


        if (amplitude < abs(change))
        {
            amplitude = change;
            s = period/4;
        }
        else
        {
            s = period/(2*PI) * asin(change/amplitude);
        }
        
        return -(amplitude*pow(2,10*(ratio -= 1)) * sin( (ratio*duration-s)*(2*PI)/period )) + start;
    }
    
	virtual UINT32 operator()( UINT32 start, UINT32 end, float ratio )
	{
        float change = end-start;
        return floorf(change * ratio * ratio) + start;
	}

	virtual vec3   operator()( vec3 start, vec3 end, float ratio )
	{
        vec3 rval(0,0,0);

        rval.x = (*this)(start.x, end.x, ratio);
        rval.y = (*this)(start.y, end.y, ratio);
        rval.z = (*this)(start.z, end.z, ratio);

		return rval;
	}

	virtual vec2   operator()( vec2 start, vec2 end, float ratio )
	{
        vec2 rval(0,0);
        
        float changeX = end.x-start.x;
        float changeY = end.y-start.y;

        rval.x = (changeX * ratio * ratio) + start.x;
        rval.y = (changeY * ratio * ratio) + start.y;

		return rval;
	}
    
	virtual vec4   operator()( vec4 start, vec4 end, float ratio )
	{
        vec4 rval(0,0,0,0);
        
        float changeX = end.x-start.x;
        float changeY = end.y-start.y;
        float changeZ = end.z-start.z;
        float changeW = end.w-start.w;

        rval.x = (changeX * ratio * ratio) + start.x;
        rval.y = (changeY * ratio * ratio) + start.y;
        rval.z = (changeZ * ratio * ratio) + start.z;
        rval.w = (changeW * ratio * ratio) + start.w;

		return rval;
	}

	virtual Color  operator()( Color start, Color end, float ratio )
	{
        Color rval(0.0f, 0.0f, 0.0f, 0.0f);
        
        float changeR = end.floats.r-start.floats.r;
        float changeG = end.floats.g-start.floats.g;
        float changeB = end.floats.b-start.floats.b;
        float changeA = end.floats.a-start.floats.a;

        rval.floats.r = (changeR * ratio * ratio) + start.floats.r;
        rval.floats.g = (changeG * ratio * ratio) + start.floats.g;
        rval.floats.b = (changeB * ratio * ratio) + start.floats.b;
        rval.floats.a = (changeA * ratio * ratio) + start.floats.a;

		return rval;
	}


    virtual ~ElasticEaseInInterpolator() {};
};



//=============================================================================
//
// A KeyFrame instance.
//
//=============================================================================

class KeyFrame
{
public:
    KeyFrame() :
        m_timeMS(0),
        m_intValue(0),
        m_vec4Value(0,0,0,0),
        m_colorValue(0,0,0,0)
    {}

public:
    inline void     SetTimeMS( UINT32 timeMS )          { m_timeMS = timeMS;            }
    inline UINT32   GetTimeMS( )                        { return m_timeMS;              }
    
    inline void     SetIntValue( UINT32 intValue )      { m_intValue = intValue;        }
    inline UINT32   GetIntValue( )                      { return m_intValue;            }

    inline void     SetFloatValue( float floatValue )   { m_floatValue = floatValue;    }
    inline float    GetFloatValue( )                    { return m_floatValue;          }

    inline void     SetVec4Value( vec4 vec4Value )      { m_vec4Value = vec4Value;      }
    inline vec4     GetVec4Value( )                     { return m_vec4Value;           }

    inline void     SetVec3Value( vec3 vec3Value )      { m_vec4Value = vec4(vec3Value.x, vec3Value.y, vec3Value.z, 0.0);     }
    inline vec3     GetVec3Value( )                     { return vec3(m_vec4Value.x, m_vec4Value.y, m_vec4Value.z);           }

    inline void     SetVec2Value( vec2 vec2Value )      { m_vec4Value = vec4(vec2Value.x, vec2Value.y, 0.0, 0.0);             }
    inline vec2     GetVec2Value( )                     { return vec2(m_vec4Value.x, m_vec4Value.y);                          }
    
    inline void     SetColorValue( Color colorValue )   { m_colorValue = colorValue;    }
    inline Color    GetColorValue( )                    { return m_colorValue;          }

protected:
    UINT32  m_timeMS;
    
    union 
    {
        UINT32  m_intValue;
        float   m_floatValue;
    };
    
    vec4    m_vec4Value;
    Color   m_colorValue;
};


typedef enum
{
    KEYFRAME_TYPE_UNKNOWN = 0,
    KEYFRAME_TYPE_UINT32,
    KEYFRAME_TYPE_FLOAT,
    KEYFRAME_TYPE_VEC2,
    KEYFRAME_TYPE_VEC3,
    KEYFRAME_TYPE_VEC4,
    KEYFRAME_TYPE_COLOR,
} KeyFrameType;


typedef struct
{
    const char*     name;
    KeyFrameType    type;
} KEYFRAME_TYPE_MAP;


typedef enum 
{
    DIRECTION_FORWARD = 1,
    DIRECTION_REVERSE = -1
} KeyFrameDirection;



//=============================================================================
//
// An Animation instance.
// Animations target a single value, e.g. m_fScale or m_vWorldPos.
//  - or -
// Animations target a single Property, e.g. pGameObject->GetProperty( "Scale" ).
//
// Multiple Animations are grouped together in a Storyboard,
// which controls several properties of a single target object.
//
//=============================================================================
class Animation : virtual public Object
{
public:
    Animation();
    virtual ~Animation();
    Animation* Clone() const;
    
    
    RESULT          Init                        ( 
                                                    IN const string&            name, 
                                                    IN const string&            propertyName, 
                                                    IN       PropertyType       propertyType, 
                                                    IN       InterpolatorType   interpolatorType, 
                                                    IN       KeyFrameType       keyFrameType, 
                                                    IN const KeyFrame*          pKeyFrames, 
                                                    IN       UINT8              numKeyFrames, 
                                                    IN       bool               isRelative 
                                                );
                                                    
    RESULT          Init                        ( IN const string& name, IN const Settings* pSettings, IN const string& settingsPath );
    
//    RESULT          BindTo                      ( void*      pValue   );
    RESULT          BindTo                      ( IProperty& property );

    RESULT          CallbackOnFinished          ( ICallback& callback );
    
    RESULT          Start                       ( );
    RESULT          Stop                        ( );
    RESULT          Pause                       ( );
    
    bool            IsStarted                   ( ) const                           { return m_isStarted;  }
    bool            IsStopped                   ( ) const                           { return !m_isStarted; }
    bool            IsPaused                    ( ) const                           { return m_isPaused;   }
    
    RESULT          Update                      ( UINT64 elapsedMS );

    RESULT          SetAutoRepeat               ( bool willAutoRepeat           )   { m_autoRepeat              = willAutoRepeat;           return S_OK; }
    RESULT          SetAutoReverse              ( bool willAutoReverse          )   { m_autoReverse             = willAutoReverse;          return S_OK; }
    RESULT          SetDeleteOnFinish           ( bool willDeleteOnFinish       )   { m_deleteOnFinish          = willDeleteOnFinish;       return S_OK; }
    RESULT          SetRelativeToCurrentState   ( bool isRelativeToCurrentState )   { m_relativeToCurrentState  = isRelativeToCurrentState; return S_OK; }
    RESULT          SetInterpolatorType         ( InterpolatorType type         ); //   { m_interpolatorType        = type;                     return S_OK; }

    UINT64          GetDurationMS               ( )                                 { return m_durationMS;              }
    UINT8           GetNumKeyFrames             ( )                                 { return m_numKeyFrames;            }
    bool            GetAutoRepeat               ( )                                 { return m_autoRepeat;              }
    bool            GetAutoReverse              ( )                                 { return m_autoReverse;             }
    bool            GetDeleteOnFinish           ( )                                 { return m_deleteOnFinish;          }
    bool            GetRelativeToCurrentState   ( )                                 { return m_relativeToCurrentState;  }
    InterpolatorType GetInterpolatorType        ( )                                 { return m_interpolatorType;        }
    
    PropertyType    GetPropertyType             ( )                                 { return m_propertyType;            }
    const string&   GetPropertyName             ( )                                 { return m_propertyName;            }
    
    RESULT          SetStoryboard               ( IN HStoryboard hStoryboard )      { m_hStoryboard = hStoryboard; return S_OK; }
    HStoryboard     GetStoryboard               ( )                                 { return m_hStoryboard;                     }
    

protected:
    Animation( const Animation& rhs );
    Animation& operator=( const Animation& rhs );

    RESULT          UpdateTarget                ( KeyFrame* pFrame1, KeyFrame* pFrame2, float progress );
    
protected:
    bool                    m_isStarted;
    bool                    m_isPaused;
    bool                    m_autoRepeat;
    bool                    m_autoReverse;
    bool                    m_deleteOnFinish;
    bool                    m_relativeToCurrentState;   // Animation keyframes are relative to the starting value of the animation target.
    
    UINT64                  m_durationMS;
    UINT64                  m_startTimeMS;
    KeyFrame                m_startingValue;
    
    KeyFrameType            m_keyFrameType;
    KeyFrame*               m_pKeyFrames;               // TODO: clones of an Animation template should SHARE KeyFrames to cut down on memory.
    UINT8                   m_numKeyFrames;
    UINT8                   m_currKeyFrame;
    UINT8                   m_nextKeyFrame;
    KeyFrameDirection       m_direction;
    
    InterpolatorType        m_interpolatorType;
    IInterpolator*          m_pInterpolator;

    bool                    m_isBoundToProperty;
    string                  m_propertyName;
    PropertyType            m_propertyType;
    IProperty*              m_pTargetProperty;
    void*                   m_pTargetValue;
    
    ICallback*              m_pCallbackOnFinished;
    
    HStoryboard             m_hStoryboard;
    
protected:
    static KEYFRAME_TYPE_MAP            s_keyFrameTypeMap[];
};

typedef Handle<Animation> HAnimation;



} // END namespace Z



