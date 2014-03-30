#include "EffectManager.hpp"
#include "OpenGLESEffect.hpp"
#include "BlurEffect.hpp"
#include "RippleEffect.hpp"
#include "MorphEffect.hpp"
#include "ColorEffect.hpp"
#include "GradientEffect.hpp"
#include "DropShadowEffect.hpp"

namespace Z
{



EffectManager::EffectFactoryItem EffectManager::s_EffectFactories[] =
{   
    { "DefaultEffect",      (EFFECT_FACTORY_METHOD)EffectFactory< OpenGLESEffect    >::Create    },
    { "BlurEffect",         (EFFECT_FACTORY_METHOD)EffectFactory< BlurEffect        >::Create    },
    { "RippleEffect",       (EFFECT_FACTORY_METHOD)EffectFactory< RippleEffect      >::Create    },
    { "MorphEffect",        (EFFECT_FACTORY_METHOD)EffectFactory< MorphEffect       >::Create    },
    { "ColorEffect",        (EFFECT_FACTORY_METHOD)EffectFactory< ColorEffect       >::Create    },
    { "GradientEffect",     (EFFECT_FACTORY_METHOD)EffectFactory< GradientEffect    >::Create    },
    { "DropShadowEffect",   (EFFECT_FACTORY_METHOD)EffectFactory< DropShadowEffect  >::Create    },
    { "",                   NULL }
};



} // END namespace Z