#pragma once

#include "StateMachine.hpp"


namespace Z
{


template<typename TYPE>
class StateMachineFactory
{
public:
    static TYPE* Create( HGameObject hGameObject )
    {
        return new TYPE( hGameObject );
    }


protected:
    StateMachineFactory();
    StateMachineFactory( const StateMachineFactory& rhs );
    StateMachineFactory& operator=( const StateMachineFactory& rhs );
    virtual ~StateMachineFactory();

};



class StateMachineFactoryManager
{

    typedef StateMachine*(*SM_FACTORY_METHOD)( HGameObject hGameObject );
    typedef struct 
    {
        const char*         name;
        SM_FACTORY_METHOD   factoryMethod;
    } StateMachineFactoryItem;


public:
    static StateMachine* Create( IN const string& stateMachineName, IN HGameObject hGameObject )
    {
        StateMachine*            pStateMachine = NULL;
        StateMachineFactoryItem* pFactory = NULL;
        int                      i = 0;
        
        do 
        {
            pFactory = &s_stateMachineFactories[i++];
        
            if (stateMachineName == pFactory->name)
            {
                DEBUGMSG(ZONE_STATEMACHINE | ZONE_VERBOSE, "StateMachineFactoryManager::Create( \"%s\" )", stateMachineName.c_str());
                
                pStateMachine = pFactory->factoryMethod( hGameObject );
                break;
            }
        } while (pFactory->factoryMethod != NULL);
        
        if (!pStateMachine)
        {
            RETAILMSG(ZONE_ERROR, "StateMachineFactoryManager::Create( \"%s\" ): not found", stateMachineName.c_str());
        }
        
        return pStateMachine;
    }
    
    
protected:
    StateMachineFactoryManager();
    StateMachineFactoryManager( const StateMachineFactoryManager& rhs );
    StateMachineFactoryManager& operator=( const StateMachineFactoryManager& rhs );
    virtual ~StateMachineFactoryManager();


protected:
    static StateMachineFactoryItem s_stateMachineFactories[];
};


#define StateMachines StateMachineFactoryManager

} // END namespace Z

