/*
 *  ParticleEmitter.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 7/26/11.
 *  Copyright 2011 Sean Kelleher. All rights reserved.
 *
 */

#include "ParticleEmitter.hpp"
#include "Util.hpp"
#include "Engine.hpp"
#include "TextureManager.hpp"
#include "ParticleManager.hpp"


namespace Z 
{

//----------------------------------------------------------------------------
// We expose the following named properties for animation and scripting.
//----------------------------------------------------------------------------
static const NamedProperty s_propertyTable[] =
{
    DECLARE_PROPERTY( ParticleEmitter, PROPERTY_VEC3,  Position  ),
    DECLARE_PROPERTY( ParticleEmitter, PROPERTY_VEC3,  Rotation  ),
    DECLARE_PROPERTY( ParticleEmitter, PROPERTY_FLOAT, Scale     ),
    DECLARE_PROPERTY( ParticleEmitter, PROPERTY_FLOAT, Opacity   ),
//    DECLARE_PROPERTY( ParticleEmitter, PROPERTY_BOOL,  Visible   ),    // asserts that SetVisible() is NULL ??
    NULL,
};
DECLARE_PROPERTY_SET( ParticleEmitter, s_propertyTable );



// ============================================================================
//
//  ParticleEmitter Implementation
//
// ============================================================================


#pragma mark ParticleEmitter Implementation

ParticleEmitter::ParticleEmitter() :
    m_pVertices(NULL),
    m_numActiveParticles(0),
    m_isVisible(true),
    m_isStarted(false),
    m_isPaused(false),
    m_deleteOnFinish(false),
    m_durationMS(0),
    m_previousFrameMS(0),
    m_fEmitCounter(0),
    m_fScale(1.0f),
    m_fOpacity(1.0f),
    m_isShadowEnabled(false),
    m_vSourcePosition(0,0,0),
    m_vSourcePositionVariance(0,0,0),
    m_fSpeed(0),
    m_fSpeedVariance(0),
    m_fParticleLifeSpan(0),
    m_fParticleLifeSpanVariance(0),
    m_fAngle(0),
    m_fAngleVariance(0),
    m_vGravity(0,0,0),
    m_fRadialAcceleration(0),
    m_fTangentialAcceleration(0),
    m_fRadialAccelVariance(0),
    m_fTangentialAccellVariance(0),
    m_startColor(Color::White()),
    m_startColorVariance(Color::Clear()),
    m_finishColor(Color::White()),
    m_finishColorVariance(Color::Clear()),
    m_maxParticles(0),
    m_fStartParticleSize(0),
    m_fStartParticleSizeVariance(0),
    m_fFinishParticleSize(0),
    m_fFinishParticleSizeVariance(0),
    m_fDurationSec(0),
    m_startTimeMS(0),
    m_emitterType(PARTICLE_EMITTER_TYPE_UNKNOWN),
    m_fMaxRadius(0),
    m_fMaxRadiusVariance(0),
    m_fMinRadius(0),
    m_fRotatePerSecond(0),
    m_fRotatePerSecondVariance(0),
    m_blendFuncSource(0),
    m_blendFuncDestination(0),
    m_fRotationStart(0),
    m_fRotationStartVariance(0),
    m_fRotationEnd(0),
    m_fRotationEndVariance(0),
    m_fEmitPerSecond(0)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "ParticleEmitter( %4d )", m_ID);

    m_bounds.SetMin( vec3( 0.0f, 0.0f, 0.0f ) );
    m_bounds.SetMax( vec3( 0.0f, 0.0f, 0.0f ) );
}


ParticleEmitter::~ParticleEmitter()
{
    RETAILMSG(ZONE_PARTICLES, "\t~ParticleEmitter( %4d )", m_ID);

    ParticleMan.Stop( this );

    SAFE_ARRAY_DELETE(m_pVertices);

    m_activeParticles.clear();
    m_numActiveParticles = 0;

    if ( !m_hEffect.IsNull() )
    {
        Effects.Release(m_hEffect);
    }

    if ( !m_hTexture.IsNull() )
    {
        Textures.Release(m_hTexture);
    }
}



ParticleEmitter*
ParticleEmitter::Clone() const
{
    return new ParticleEmitter(*this);
}


ParticleEmitter::ParticleEmitter( const ParticleEmitter& rhs ) : 
    Object(),
    m_pVertices(NULL)
{
    *this = rhs;
}


ParticleEmitter& 
ParticleEmitter::operator=( const ParticleEmitter& rhs )
{
    // Avoid self-assignment.
    if (this == &rhs)
        return *this;
    

    // Explicitly DO NOT copy the base class state.
    // We DO NOT want to copy the Object::m_RefCount, m_ID, or m_name from the copied Object.


    // SHALLOW COPY:
    m_deleteOnFinish                = rhs.m_deleteOnFinish;
    m_isVisible                     = rhs.m_isVisible;
    m_fScale                        = rhs.m_fScale;
    m_fOpacity                      = rhs.m_fOpacity;
    m_bounds                        = rhs.m_bounds;
    m_isShadowEnabled               = rhs.m_isShadowEnabled;
    m_durationMS                    = rhs.m_durationMS;
    m_fEmitCounter                  = rhs.m_fEmitCounter;
    m_numActiveParticles            = rhs.m_numActiveParticles;
    
    m_textureFilename               = rhs.m_textureFilename; 
    m_vSourcePosition               = rhs.m_vSourcePosition;
    m_vSourcePositionVariance       = rhs.m_vSourcePositionVariance;
    m_fSpeed                        = rhs.m_fSpeed;
    m_fSpeedVariance                = rhs.m_fSpeedVariance;
    m_fParticleLifeSpan             = rhs.m_fParticleLifeSpan;
    m_fParticleLifeSpanVariance     = rhs.m_fParticleLifeSpanVariance;
    m_fAngle                        = rhs.m_fAngle;
    m_fAngleVariance                = rhs.m_fAngleVariance;
    m_vGravity                      = rhs.m_vGravity;
    m_fRadialAcceleration           = rhs.m_fRadialAcceleration;
    m_fRadialAccelVariance          = rhs.m_fRadialAccelVariance;
    m_fTangentialAcceleration       = rhs.m_fTangentialAcceleration;
    m_fTangentialAccellVariance     = rhs.m_fTangentialAccellVariance;
    m_startColor                    = rhs.m_startColor;
    m_startColorVariance            = rhs.m_startColorVariance;
    m_finishColor                   = rhs.m_finishColor;
    m_finishColorVariance           = rhs.m_finishColorVariance;
    m_maxParticles                  = rhs.m_maxParticles;
    m_fStartParticleSize            = rhs.m_fStartParticleSize;
    m_fStartParticleSizeVariance    = rhs.m_fStartParticleSizeVariance;
    m_fFinishParticleSize           = rhs.m_fFinishParticleSize;
    m_fFinishParticleSizeVariance   = rhs.m_fFinishParticleSizeVariance;
    m_fDurationSec                  = rhs.m_fDurationSec;
    m_emitterType                   = rhs.m_emitterType;
    m_fMaxRadius                    = rhs.m_fMaxRadius;
    m_fMaxRadiusVariance            = rhs.m_fMaxRadiusVariance;
    m_fMinRadius                    = rhs.m_fMinRadius;
    m_fRotatePerSecond              = rhs.m_fRotatePerSecond;
    m_fRotatePerSecondVariance      = rhs.m_fRotatePerSecondVariance;
    m_blendFuncSource               = rhs.m_blendFuncSource;
    m_blendFuncDestination          = rhs.m_blendFuncDestination;
    m_fRotationStart                = rhs.m_fRotationStart;
    m_fRotationStartVariance        = rhs.m_fRotationStartVariance;
    m_fRotationEnd                  = rhs.m_fRotationEnd;
    m_fRotationEndVariance          = rhs.m_fRotationEndVariance;
    m_fEmitPerSecond                = rhs.m_fEmitPerSecond;

    
    // DEEP COPY: m_hEffect
    if (!rhs.m_hEffect.IsNull())
    {
        m_hEffect                   = rhs.m_hEffect;
        Effects.AddRef( m_hEffect );
    }


    // DEEP COPY: m_hTexture
    if (!rhs.m_hTexture.IsNull())
    {
        m_hTexture                   = rhs.m_hTexture;
        Textures.AddRef( m_hTexture );
    }

    
    // DEEP COPY: m_pVertices
    SAFE_ARRAY_DELETE(m_pVertices);
    m_pVertices                     = new Vertex[m_maxParticles * VERTS_PER_PARTICLE];
//    memcpy( m_pVertices, rhs.m_pVertices, m_maxParticles*VERTS_PER_PARTICLE*sizeof(Vertex) );


    // DEEP COPY: m_activeParticles
//    m_activeParticles               = rhs.m_activeParticles;
    

    // Give it a new name with a random suffix
    char instanceName[MAX_NAME];
    sprintf(instanceName, "%s_%X", rhs.m_name.c_str(), (unsigned int)Platform::Random());
    m_name = string(instanceName);
    
    // Reset state
    m_numActiveParticles = 0;
    m_isStarted          = false;
    m_isPaused           = false;
    m_startTimeMS        = 0;
    m_previousFrameMS    = 0;
    m_fEmitCounter       = 0.0f;
    
    return *this;
}








RESULT
ParticleEmitter::InitFromFile( const string& filename )
{
    RESULT      rval        = S_OK;
    
    RETAILMSG(ZONE_OBJECT, "ParticleEmitter[%4d]::Init( %s )", m_ID, filename.c_str());


    Settings settings;
    if ( FAILED(settings.Read( filename )) )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleEmitter::InitFromFile( %s ): failed to load .PEX file", filename.c_str() );
        return E_UNEXPECTED;
    }
    

    m_name                          = filename;
    m_textureFilename               = settings.GetString( "/particleEmitterConfig/texture.name" );
    m_vSourcePosition.x             = settings.GetFloat ( "/particleEmitterConfig/sourcePosition.x" );
    m_vSourcePosition.y             = settings.GetFloat ( "/particleEmitterConfig/sourcePosition.y" );
    m_vSourcePosition.z             = 0.0f;                                                                             // UNSUPPORTED by Particle Designer
    m_vSourcePositionVariance.x     = settings.GetFloat ( "/particleEmitterConfig/sourcePositionVariance.x" );
    m_vSourcePositionVariance.y     = settings.GetFloat ( "/particleEmitterConfig/sourcePositionVariance.y" );
    m_vSourcePositionVariance.z     = 0.0f;                                                                             // UNSUPPORTED by Particle Designer
    m_fSpeed                        = settings.GetFloat ( "/particleEmitterConfig/speed.value" );
    m_fSpeedVariance                = settings.GetFloat ( "/particleEmitterConfig/speedVariance.value" );
    m_fParticleLifeSpan             = settings.GetFloat ( "/particleEmitterConfig/particleLifeSpan.value" );
    m_fParticleLifeSpanVariance     = settings.GetFloat ( "/particleEmitterConfig/particleLifespanVariance.value" );    // spelling is correct
    m_fAngle                        = settings.GetFloat ( "/particleEmitterConfig/angle.value" );
    m_fAngleVariance                = settings.GetFloat ( "/particleEmitterConfig/angleVariance.value" );
    m_vGravity.x                    = settings.GetFloat ( "/particleEmitterConfig/gravity.x" );
    m_vGravity.y                    = settings.GetFloat ( "/particleEmitterConfig/gravity.y" );
    m_vGravity.z                    = 0.0f;                                                                             // UNSUPPORTED by Particle Designer
    m_fRadialAcceleration           = settings.GetFloat ( "/particleEmitterConfig/radialAcceleration.value" );
    m_fRadialAccelVariance          = settings.GetFloat ( "/particleEmitterConfig/radialAccelVariange.value" );
    m_fTangentialAcceleration       = settings.GetFloat ( "/particleEmitterConfig/tangentialAcceleration.value" );
    m_fTangentialAccellVariance     = settings.GetFloat ( "/particleEmitterConfig/tangentialAccelerationVariance.value" );
    m_startColor.floats.r           = settings.GetFloat ( "/particleEmitterConfig/startColor.red" );
    m_startColor.floats.g           = settings.GetFloat ( "/particleEmitterConfig/startColor.green" );
    m_startColor.floats.b           = settings.GetFloat ( "/particleEmitterConfig/startColor.blue" );
    m_startColor.floats.a           = settings.GetFloat ( "/particleEmitterConfig/startColor.alpha" );
    m_startColorVariance.floats.r   = settings.GetFloat ( "/particleEmitterConfig/startColorVariance.red" );
    m_startColorVariance.floats.g   = settings.GetFloat ( "/particleEmitterConfig/startColorVariance.green" );
    m_startColorVariance.floats.b   = settings.GetFloat ( "/particleEmitterConfig/startColorVariance.blue" );
    m_startColorVariance.floats.a   = settings.GetFloat ( "/particleEmitterConfig/startColorVariance.alpha" );
    m_finishColor.floats.r          = settings.GetFloat ( "/particleEmitterConfig/finishColor.red" );
    m_finishColor.floats.g          = settings.GetFloat ( "/particleEmitterConfig/finishColor.green" );
    m_finishColor.floats.b          = settings.GetFloat ( "/particleEmitterConfig/finishColor.blue" );
    m_finishColor.floats.a          = settings.GetFloat ( "/particleEmitterConfig/finishColor.alpha" );
    m_finishColorVariance.floats.r  = settings.GetFloat ( "/particleEmitterConfig/finishColorVariance.red" );
    m_finishColorVariance.floats.g  = settings.GetFloat ( "/particleEmitterConfig/finishColorVariance.green" );
    m_finishColorVariance.floats.b  = settings.GetFloat ( "/particleEmitterConfig/finishColorVariance.blue" );
    m_finishColorVariance.floats.a  = settings.GetFloat ( "/particleEmitterConfig/finishColorVariance.alpha" );
    m_maxParticles                  = settings.GetInt   ( "/particleEmitterConfig/maxParticles.value" );
    m_fStartParticleSize            = settings.GetFloat ( "/particleEmitterConfig/startParticleSize.value" );
    m_fStartParticleSizeVariance    = settings.GetFloat ( "/particleEmitterConfig/startParticleSizeVariance.value" );
    m_fFinishParticleSize           = settings.GetFloat ( "/particleEmitterConfig/finishParticleSize.value" );
    m_fFinishParticleSizeVariance   = settings.GetFloat ( "/particleEmitterConfig/finishParticleSizeVariance.value" );
    m_fDurationSec                  = settings.GetFloat ( "/particleEmitterConfig/duration.value" );
    m_emitterType = (ParticleEmitterType)settings.GetInt( "/particleEmitterConfig/emitterType.value" );
    m_fMaxRadius                    = settings.GetFloat ( "/particleEmitterConfig/maxRadius.value" );
    m_fMaxRadiusVariance            = settings.GetFloat ( "/particleEmitterConfig/maxRadiusVariance.value" );
    m_fMinRadius                    = settings.GetFloat ( "/particleEmitterConfig/minRadius.value" );
    m_fRotatePerSecond              = settings.GetFloat ( "/particleEmitterConfig/rotatePerSecond.value" );
    m_fRotatePerSecondVariance      = settings.GetFloat ( "/particleEmitterConfig/rotatePerSecondVariance.value" );
    m_blendFuncSource               = settings.GetInt   ( "/particleEmitterConfig/blendFuncSource.value" );             // HACK HACK: this is a GL_xxx #define.
    m_blendFuncDestination          = settings.GetInt   ( "/particleEmitterConfig/blendFuncDestination.value" );        // HACK HACK: this is a GL_xxx #define.
    m_fRotationStart                = settings.GetFloat ( "/particleEmitterConfig/rotationStart.value" );
    m_fRotationStartVariance        = settings.GetFloat ( "/particleEmitterConfig/rotationStartVariance.value" );
    m_fRotationEnd                  = settings.GetFloat ( "/particleEmitterConfig/rotationEnd.value" );
    m_fRotationEndVariance          = settings.GetFloat ( "/particleEmitterConfig/rotationEndVariance.value" );
    
    m_durationMS                    = m_fDurationSec*1000;

    if (m_durationMS < 33)
    {
        RETAILMSG(ZONE_ERROR, "ParticleEmitter %s duration %dms is LESS than one frame!!! Rounding up.", filename.c_str(), m_durationMS);
        m_durationMS = 33;
    }

	// Calculate the emission rate
	m_fEmitPerSecond                = m_maxParticles / (m_fParticleLifeSpan + m_fParticleLifeSpanVariance);

#ifdef USE_POINT_SPRITES

    // Try loading the texture file directly (can't use point sprites with a texture atlas).
    string textureFilename = "/app/particles/" + m_textureFilename;
    if (FAILED(TextureMan.CreateFromFile( textureFilename, &m_hTexture )))
    {
        RETAILMSG(ZONE_WARN, "ERROR: ParticleEmitter::Init( %s ): failed to load texture \"%s\"", filename.c_str(), textureFilename.c_str());
        rval = E_FILE_NOT_FOUND;
    }

#else

    // Strip ".png" from the texture name, and try to fetch it from an existing TextureAtlas.
    string textureName = m_textureFilename;
    size_t pos = textureName.find(".png");
    if (pos != string::npos)
        textureName.replace(pos, 4, "");

    if (FAILED(TextureMan.Get( textureName, &m_hTexture )))
    {
        RETAILMSG(ZONE_WARN, "WARNING: ParticleEmitter::Init( %s ): texture \"%s\" was not found in a TextureAtlas. Loading from \"%s\" instead", 
            filename.c_str(), textureName.c_str(), m_textureFilename.c_str());

        // Try loading the texture file directly.
        string textureFilename = "/app/particles/" + m_textureFilename;
        if (FAILED(TextureMan.CreateFromFile( textureFilename, &m_hTexture )))
        {
            RETAILMSG(ZONE_WARN, "ERROR: ParticleEmitter::Init( %s ): failed to load texture \"%s\"", filename.c_str(), textureFilename.c_str());
            rval = E_FILE_NOT_FOUND;
            goto Exit;
        }
    }

#endif // USE_POINT_SPRITES


    CHR(InitParticles());
    
    
Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleEmitter::Init( \"%s\" ): failed", filename.c_str());
    }
    return rval;
}



RESULT
ParticleEmitter::InitParticles()
{
    RESULT rval = S_OK;

    // Create the array of Particles.
    m_activeParticles.clear();
    m_activeParticles.resize( m_maxParticles );
    
    // Create the array of particle vertices.
    // TODO: Create a Vertex Buffer Object?
    SAFE_ARRAY_DELETE(m_pVertices);
    m_pVertices = new Vertex[m_maxParticles * VERTS_PER_PARTICLE];
    DEBUGCHK(m_pVertices != NULL);

#ifdef USE_POINT_SPRITES
    
    // TODO: measure perf of point sprites vs quad sprites on iPhone, see where the curves cross.

    for (int i = 0; i < m_maxParticles; ++i)
    {
        m_pVertices[i].position.x = 0;
        m_pVertices[i].position.y = 0;
        m_pVertices[i].position.z = 1.0f;
        m_pVertices[i].color      = Color::Red();
    }
    
#else    
    
    // The advantage over point sprites is that quads work with texture atlasses.
    Rectangle   spriteRect  = { -m_fStartParticleSize/2.0, -m_fStartParticleSize/2.0, m_fStartParticleSize, m_fStartParticleSize };
    TextureInfo texInfo;
    CHR(TextureMan.GetInfo( m_hTexture, &texInfo ));

    for (int i = 0; i < m_maxParticles; ++i)
    {
        Util::CreateTriangleList( &spriteRect, 1, 1, &m_pVertices[ i * VERTS_PER_PARTICLE], texInfo.uStart, texInfo.uEnd, texInfo.vStart, texInfo.vEnd, m_startColor );
    }

#endif // USE_POINT_SPRITES

    
Exit:
    return rval;
}


RESULT
ParticleEmitter::SpawnParticle()
{
    RESULT rval = S_OK;
	
    // Don't spawn beyond the maximum number of particles.
    if (m_numActiveParticles >= m_maxParticles)
        return S_OK;
        
    CHR(InitParticle( &m_activeParticles[ m_numActiveParticles ] ));
	
    m_numActiveParticles++;
    
Exit:	
	return rval;
}



RESULT
ParticleEmitter::InitParticle( Particle* pParticle )
{
    // Copyright (c) 2010 71Squared
    //
    // Permission is hereby granted, free of charge, to any person obtaining a copy
    // of this software and associated documentation files (the "Software"), to deal
    // in the Software without restriction, including without limitation the rights
    // to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    // copies of the Software, and to permit persons to whom the Software is
    // furnished to do so, subject to the following conditions:
    //
    // The above copyright notice and this permission notice shall be included in
    // all copies or substantial portions of the Software.

    
    RESULT rval = S_OK;

    if (NULL == pParticle)
    {
        return E_NULL_POINTER;
    }
    
    memset(pParticle, 0, sizeof(Particle));
	
	// Init the position of the particle.  This is based on the source position of the particle emitter
	// plus a configured variance.  The RANDOM_MINUS_1_TO_1 macro allows the number to be both positive
	// and negative.
    pParticle->vPosition.x          = m_vSourcePosition.x + m_vSourcePositionVariance.x * Platform::RandomDouble(-1.0, 1.0);
    pParticle->vPosition.y          = m_vSourcePosition.y + m_vSourcePositionVariance.y * Platform::RandomDouble(-1.0, 1.0);
    pParticle->vPosition.z          = 0.0;
    pParticle->vStartPosition.x     = m_vSourcePosition.x;
    pParticle->vStartPosition.y     = m_vSourcePosition.y;
    pParticle->vStartPosition.z     = 0.0;
	
    // Init the direction of the particle.  The newAngle is calculated using the angle passed in and the
    // angle variance.
    float   newAngle                = RADIANS(m_fAngle + m_fAngleVariance * Platform::RandomDouble(-1.0, 1.0));
    vec3    vector                  = vec3(cosf(newAngle), sinf(newAngle), 0.0);
    float   vectorSpeed             = m_fSpeed + m_fSpeedVariance * Platform::RandomDouble(-1.0, 1.0);
    pParticle->vVelocity            = vector * vectorSpeed;
	
    // Set the default diameter of the particle from the source position.
    pParticle->fRadius              = m_fMaxRadius + m_fMaxRadiusVariance  * Platform::RandomDouble(-1.0, 1.0);
    
    pParticle->fRadiusDelta         = (m_fMaxRadius / m_fParticleLifeSpan) * (1.0 / MAXIMUM_UPDATE_RATE_HZ);
    pParticle->fAngle               = RADIANS(m_fAngle + m_fAngleVariance  * Platform::RandomDouble(-1.0, 1.0));
    pParticle->fDegreesPerSecond    = RADIANS(m_fRotatePerSecond + m_fRotatePerSecondVariance * Platform::RandomDouble(-1.0, 1.0));
    
    pParticle->fRadialAcceleration  = m_fRadialAcceleration;
    pParticle->fTangentialAcceleration = m_fTangentialAcceleration;
	
    // Calculate the particles life span using the life span and variance passed in.
    pParticle->fTimeToLiveSec       = MAX(0, m_fParticleLifeSpan + m_fParticleLifeSpanVariance * Platform::RandomDouble(-1.0, 1.0));
    
    // Calculate the particle size using the start and finish particle sizes
    float particleStartSize         = m_fStartParticleSize  + m_fStartParticleSizeVariance  * Platform::RandomDouble(-1.0, 1.0);
    float particleFinishSize        = m_fFinishParticleSize + m_fFinishParticleSizeVariance * Platform::RandomDouble(-1.0, 1.0);
    pParticle->fParticleSizeDelta   = ((particleFinishSize - particleStartSize) / pParticle->fTimeToLiveSec) * (1.0 / MAXIMUM_UPDATE_RATE_HZ);
    pParticle->fParticleSize        = MAX(0, particleStartSize);
	
    // Calculate the color the particle should have when it starts its life.  All the elements
    // of the start color passed in along with the variance are used to calculate the star color.
    Color start                     = Color::Clear();
    start.floats.r                  = m_startColor.floats.r + m_startColorVariance.floats.r * Platform::RandomDouble(-1.0, 1.0);
    start.floats.g                  = m_startColor.floats.g + m_startColorVariance.floats.g * Platform::RandomDouble(-1.0, 1.0);
    start.floats.b                  = m_startColor.floats.b + m_startColorVariance.floats.b * Platform::RandomDouble(-1.0, 1.0);
    start.floats.a                  = m_startColor.floats.a + m_startColorVariance.floats.a * Platform::RandomDouble(-1.0, 1.0);
	
    // Calculate the color the particle should be when its life is over.  This is done the same
    // way as the start color above.
    Color end                       = Color::Clear();
    end.floats.r                    = m_finishColor.floats.r + m_finishColorVariance.floats.r * Platform::RandomDouble(-1.0, 1.0);
    end.floats.g                    = m_finishColor.floats.g + m_finishColorVariance.floats.g * Platform::RandomDouble(-1.0, 1.0);
    end.floats.b                    = m_finishColor.floats.b + m_finishColorVariance.floats.b * Platform::RandomDouble(-1.0, 1.0);
    end.floats.a                    = m_finishColor.floats.a + m_finishColorVariance.floats.a * Platform::RandomDouble(-1.0, 1.0);
	
    // Calculate the delta which is to be applied to the particles color during each cycle of its
    // life.  The delta calculation uses the life span of the particle to make sure that the 
    // particles color will transition from the start to end color during its life time.  As the game
    // loop is using a fixed delta value we can calculate the delta color once saving cycles in the 
    // update method.
    pParticle->color = start;
    pParticle->deltaColor.floats.r  = ((end.floats.r - start.floats.r) / pParticle->fTimeToLiveSec) * (1.0 / MAXIMUM_UPDATE_RATE_HZ);
    pParticle->deltaColor.floats.g  = ((end.floats.g - start.floats.g) / pParticle->fTimeToLiveSec) * (1.0 / MAXIMUM_UPDATE_RATE_HZ);
    pParticle->deltaColor.floats.b  = ((end.floats.b - start.floats.b) / pParticle->fTimeToLiveSec) * (1.0 / MAXIMUM_UPDATE_RATE_HZ);
    pParticle->deltaColor.floats.a  = ((end.floats.a - start.floats.a) / pParticle->fTimeToLiveSec) * (1.0 / MAXIMUM_UPDATE_RATE_HZ);


Exit:
    return rval;
}



RESULT
ParticleEmitter::Start( )
{ 
    m_isStarted          = true;
    m_isVisible          = true;
    m_startTimeMS        = 0;
    m_previousFrameMS    = 0;
    m_numActiveParticles = 0;
    m_fEmitCounter       = 0.0f;
    
    ///m_previousFrameMS = m_startTimeMS = GameTime.GetTime();
    // NO: defer until the first update, so that even very-short-duration emitters
    // have a chance to run.
    // Otherwise, it may be many milliseconds between ::Start() and ::Update(),
    // and the emitter with stop before it has spawned a single particle.

    DEBUGMSG(ZONE_PARTICLES, "START ParticleEmitter \"%s\"", m_name.c_str());

    InitParticles();

    return S_OK; 
}




RESULT
ParticleEmitter::Stop( )
{
    RESULT rval = S_OK;

    DEBUGMSG(ZONE_PARTICLES, "STOP ParticleEmitter \"%s\"", m_name.c_str());


    if (!m_isStarted && !m_isPaused)
    {
        RETAILMSG(ZONE_WARN, "WARNING: ParticleEmitter \"%s\" already stopped.", m_name.c_str());
        return E_INVALID_OPERATION;
    }


    m_isStarted             = false;
    m_isPaused              = false;
    m_isVisible             = false;
    m_startTimeMS           = 0;
    m_previousFrameMS       = 0;
    ////m_fEmitCounter          = 0.0f;
  
    if (m_deleteOnFinish && m_refCount > 0)
    //if (m_refCount > 0)
    {
        DEBUGMSG(ZONE_PARTICLES, "ParticleEmitter::Stop(): \"%s\" DELETE on finish.", m_name.c_str());
        ParticleMan.ReleaseOnNextFrame( this );
    }

Exit:
    return rval;
}



RESULT
ParticleEmitter::Update( UINT64 elapsedMS )
{
    RESULT rval = S_OK;
    
    if (!m_isStarted || m_isPaused)
        return S_OK;
    
    // Start the emitter's life-clock on first ::Update().
    if (0 == m_startTimeMS)
    {
        m_startTimeMS = elapsedMS;
        
        // HACK: simulate one standard frame so we always emit some particles, even if the emitter's duration is less than a single frame:
        m_previousFrameMS = elapsedMS - 33;
    }

    // How many milliseconds have elapsed since the previous call to Update()?
    UINT64 deltaMS       = elapsedMS - m_previousFrameMS;
    float  deltaSec      = ((double)deltaMS)/1000.0;
    m_previousFrameMS    = elapsedMS;


    // As long as the emitter lifetime has not expired, spawn particles.
    UINT64 timeSinceStartMS  = elapsedMS - m_startTimeMS;
    double timeSinceStartSec = ((double)timeSinceStartMS)/1000.0;

    DEBUGMSG(ZONE_PARTICLES | ZONE_VERBOSE, "ParticleEmitter::Update( \"%s\" ): %d particles deltaMS: %llu deltaSecs: %2.2f totalMS: %llu totalSec: %2.2f duration: %2.2f",
        m_name.c_str(), m_numActiveParticles, deltaMS, deltaSec, timeSinceStartMS, timeSinceStartSec, m_fDurationSec);


    if ( timeSinceStartSec <= m_fDurationSec || m_fDurationSec == -1.0 )
    {
        if (m_fEmitPerSecond > 0.0) 
        {
            float rate      = 1.0f/m_fEmitPerSecond;
            m_fEmitCounter += deltaSec;
            
            while (m_numActiveParticles < m_maxParticles && m_fEmitCounter > rate) 
            {
                SpawnParticle();
                m_fEmitCounter -= rate;
            }
        }
    }
	
    //
    // TODO: custom ParticleController/ParticleAnimator object.  Instances for gravity, bezier path, etc.
    // Unity does this.
    //

#ifndef USE_POINT_SPRITES
    // Save the particle's texture atlas coordinates; we'll use them in the update loop below.
    TextureInfo texInfo;
    CHR(TextureMan.GetInfo( m_hTexture, &texInfo ));
#endif

    //
    // Update each Particle
    //
    for (int i = 0; i < m_numActiveParticles; ++i)
    {
        Particle* pParticle = &m_activeParticles[i];
        
        /*
            struct Particle:

            vec3    startPosition;
            vec3    position;
            vec3    velocity;
            Color   color;
            Color   deltaColor;
            float   fRadialAcceleration;
            float   fTangentialAcceleration;
            float   fRadius;
            float   fRadiusDelta;
            float   fAngle;
            float   fDegreesPerSecond;
            float   fParticleSize;
            float   fParticleSizeDelta;
            float   fTimeToLiveSec;
        */


        // Age the Particle.
        pParticle->fTimeToLiveSec -= deltaSec;

        if (pParticle->fTimeToLiveSec <= 0.0)
        {
            // Particle has died.
            // Copy the last live particle into its slot, overwriting the dead particle.
            // This keeps active particles at the front of the list.
            m_activeParticles[i] = m_activeParticles[m_numActiveParticles-1];
            m_numActiveParticles--;
            
            
            if (0 == m_numActiveParticles)
            {
                DEBUGMSG(ZONE_PARTICLES, "ParticleEmitter::Update( \"%s\" ): last Particle expired, STOP", m_name.c_str());
                rval = E_NOTHING_TO_DO;
                goto Exit;
            }
        }


        // Update Particle position.
        switch ( m_emitterType )
        {
            case PARTICLE_EMITTER_TYPE_RADIAL:
            {
                // Update the angle of the particle from the sourcePosition and the radius.  This is only
                // done if the particles are rotating
                pParticle->fAngle  += pParticle->fDegreesPerSecond * deltaSec;
                pParticle->fRadius -= pParticle->fRadiusDelta;
                
                vec3 tmp;
                tmp.x = m_vSourcePosition.x - cosf(pParticle->fAngle) * pParticle->fRadius;
                tmp.y = m_vSourcePosition.y - sinf(pParticle->fAngle) * pParticle->fRadius;
                tmp.z = 0.0;
                pParticle->vPosition = tmp;

                if (pParticle->fRadius < m_fMinRadius)
                {
                    pParticle->fTimeToLiveSec = 0;
                }
            } 
            break;

            case PARTICLE_EMITTER_TYPE_GRAVITY:
            {
                vec3 offset, radial, tangential;
                
                // Move particle back to the coordinate space of the emitter for the calculations that are to follow.
                // We apply gravity and radial/tangential acceleration in emitter space, then move the particle
                // back to world space.
                pParticle->vPosition -= pParticle->vStartPosition;
                
                if (pParticle->vPosition.x || pParticle->vPosition.y || pParticle->vPosition.z)
                {
                    tangential = radial = pParticle->vPosition.Normalized();
                }
                
                radial         *= pParticle->fRadialAcceleration;
                
                float newy      = tangential.x;
                tangential.x    = -tangential.y;
                tangential.y    = newy;
                tangential     *= pParticle->fTangentialAcceleration;
                
                offset         = (radial + tangential) + m_vGravity;
                offset        *= deltaSec;
                
                pParticle->vVelocity = pParticle->vVelocity + offset;
                offset = pParticle->vVelocity * deltaSec;
                
                pParticle->vPosition += offset;
                
                // Move the particle back to world space.
                pParticle->vPosition += pParticle->vStartPosition;
            }
            break;
            
            default:
                DEBUGCHK(0);
        } // END: switch( m_emitterType )

        
        // Update the particle color
        pParticle->color.floats.r += pParticle->deltaColor.floats.r;
        pParticle->color.floats.g += pParticle->deltaColor.floats.g;
        pParticle->color.floats.b += pParticle->deltaColor.floats.b;
        pParticle->color.floats.a += pParticle->deltaColor.floats.a;
        
     
        // Update the size
        pParticle->fParticleSize += pParticle->fParticleSizeDelta;
        pParticle->fParticleSize  = MAX(0, pParticle->fParticleSize);
                    
#ifdef USE_POINT_SPRITES        

        // Update the particle's point sprite.
        m_pVertices[i].x = pParticle->vPosition.x;
        m_pVertices[i].y = pParticle->vPosition.y;
        
        // Place the color of the current particle into the color array
        m_pVertices[i].color = pParticle->color;

#else
        // Update the particle's position/scale/color.
        Rectangle spriteRect = { 0, 0, 1.0, 1.0 };
        
        spriteRect.width    =  pParticle->fParticleSize;
        spriteRect.height   =  pParticle->fParticleSize;
        spriteRect.x        += pParticle->vPosition.x - (spriteRect.width/2.0);
        spriteRect.y        += pParticle->vPosition.y - (spriteRect.height/2.0);

        Util::CreateTriangleList( &spriteRect, 1, 1, &m_pVertices[ i * VERTS_PER_PARTICLE ], texInfo.uStart, texInfo.uEnd, texInfo.vStart, texInfo.vEnd, pParticle->color );

#endif // USE_POINT_SPRITES
    }

    // Are we done emitting particles?
    if ( timeSinceStartSec >= m_fDurationSec && m_fDurationSec != -1.0 && 0 == m_numActiveParticles )
    {
        rval = E_NOTHING_TO_DO;
    }


Exit:
    return rval;
}




//
// IDrawable
// 

RESULT  
ParticleEmitter::Draw( const mat4& matParentWorld )
{
    RESULT  rval = S_OK;
    mat4    world;

    if (0 == m_numActiveParticles)
    {
        return S_OK;
    }


    //
    // TODO: needs to take advantage of batching! For performance, and so that draw order is correct when overlapping Sprites!
    //

//    CHR(Renderer.PushEffect( m_hEffect ));
    CHR(Renderer.SetModelViewMatrix( matParentWorld ));
    CHR(Renderer.SetTexture( 0, m_hTexture ));
    CHR(Renderer.SetGlobalColor( Color(1, 1, 1, m_fOpacity )));
    CHR(Renderer.SetBlendFunctions( m_blendFuncSource, m_blendFuncDestination ));
    
#ifdef USE_POINT_SPRITES    
    CHR(Renderer.DrawPointSprites( m_pVertices, m_numActiveParticles, m_fStartParticleSize /* HACK: until we add point size to Vertex */ ));
#else
    CHR(Renderer.DrawTriangleList( m_pVertices, m_numActiveParticles * VERTS_PER_PARTICLE ));
#endif

Exit:    
//    IGNOREHR(Renderer.PopEffect());

    // Premultiplied alpha, which is the default for iPhone.
    // TODO: Push/Pop or Get/SetBlendFunctions so we can restore previous. Current code breaks on iOS simulator.
    IGNOREHR(Renderer.SetBlendFunctions( GL_ONE, GL_ONE_MINUS_SRC_ALPHA ));
    IGNOREHR(Renderer.SetGlobalColor( Color::White() ));
    
    return rval;
}



RESULT
ParticleEmitter::SetEffect( HEffect hEffect )
{
    RESULT rval = S_OK;

    if (m_hEffect != hEffect)
    {
        if (!m_hEffect.IsNull())
            CHR(Effects.Release(m_hEffect));
            
        m_hEffect = hEffect;
        
        if (!m_hEffect.IsNull())
            CHR(Effects.AddRef( m_hEffect ));
    }
    
Exit:
    return rval;
}



IProperty*
ParticleEmitter::GetProperty( const string& propertyName ) const
{
    return s_properties.Get( this, propertyName );
}




} // END namespace Z


