#include "Test.hpp"
#include "Log.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include "ResourceManager.hpp"
#include "TextureManager.hpp"
#include "Platform.hpp"
#include "FileManager.hpp"
#include "ShaderManager.hpp"
#include "Image.hpp"
#include "Camera.hpp"
#include "AnimationManager.hpp"
#include "StoryboardManager.hpp"
#include "GameObjectManager.hpp"
#include "Metrics.hpp"
#include "Time.hpp"
#include "StateMachine.hpp"
#include "LayerManager.hpp"
#include "TouchInput.hpp"
#include "Engine.hpp"
#include "DebugRenderer.hpp"
#include "Property.hpp"
#include "FontManager.hpp"
#include "SoundManager.hpp"
#include "EffectManager.hpp"
#include "ParticleManager.hpp"

#include "BlurEffect.hpp"
#include "RippleEffect.hpp"
#include "MorphEffect.hpp"

#include "json.h"

// Unit test state machines
#include "unittest1.h"
#include "unittest2a.h"
#include "unittest2b.h"
#include "unittest2c.h"
#include "unittest3a.h"
#include "unittest3b.h"
#include "unittest4.h"
#include "unittest5.h"



namespace Z
{



bool TestHandles()
{
    bool rval = true;

    RETAILMSG(ZONE_INFO, "TestHandles()");
    
    Handle<UINT32> h1, h2;
    
    RETAILMSG(ZONE_INFO, "h1 = %d, sizeof(h1) = %d, sizeof(Handle<UINT32>) = %d",
              (unsigned long)h1, sizeof(h1), sizeof(Handle<UINT32>));
    

    h1.Init(24);
    
    RETAILMSG(ZONE_INFO, "h1 = 0x%x, h1.GetHandle() = 0x%x, h1.GetToken() = 0x%x, h1.GetIndex() = 0x%x",
              (UINT32)h1, h1.GetHandle(), h1.GetToken(), h1.GetIndex() );
 
    RETAILMSG(ZONE_INFO, "h1 %s == h2",
              h1 == h2 ? "DOES" : "DOES NOT");

    RETAILMSG(ZONE_INFO, "h1 %s != h2",
              h1 != h2 ? "DOES" : "DOES NOT");

    RETAILMSG(ZONE_INFO, "h2 = 0x%x, h2.GetHandle() = 0x%x, h2.GetToken() = 0x%x, h2.GetIndex() = 0x%x",
              (UINT32)h2, h2.GetHandle(), h2.GetToken(), h2.GetIndex() );
    
    RETAILMSG(ZONE_INFO, "h2.IsNull() = %d", h2.IsNull());

    h2 = h1;
    
    RETAILMSG(ZONE_INFO, "h1 %s == h2",
              h1 == h2 ? "DOES" : "DOES NOT");
    
    RETAILMSG(ZONE_INFO, "h1 %s != h2",
              h1 != h2 ? "DOES" : "DOES NOT");
    
    RETAILMSG(ZONE_INFO, "h2 = 0x%x, h2.GetHandle() = 0x%x, h2.GetToken() = 0x%x, h2.GetIndex() = 0x%x",
              (UINT32)h2, h2.GetHandle(), h2.GetToken(), h2.GetIndex() );

    RETAILMSG(ZONE_INFO, "h2.IsNull() = %d", h2.IsNull());

Exit:
    return rval;
}



bool TestResourceManager()
{
    bool rval = true;

    
    #define objectManager ((ResourceManager<Object>&)ResourceManager<Object>::Instance())
    
    objectManager.Init();
    

    RETAILMSG(ZONE_INFO, "ObjectManager(0x%x).Count(): %d", &objectManager, objectManager.Count() );

    Object* pObject = Object::Create();
    Handle<Object> hObject;
    RETAILMSG(ZONE_INFO, "pObject->GetRefCount(): %d", pObject->GetRefCount());

    objectManager.Add( "/Objects/Object1", pObject, &hObject  );

    RETAILMSG(ZONE_INFO, "ObjectManager.Count(): %d", objectManager.Count());
    RETAILMSG(ZONE_INFO, "hObject.GetHandle(): 0x%x", hObject.GetHandle());
    RETAILMSG(ZONE_INFO, "hObject.GetIndex():  0x%x", hObject.GetIndex());
    RETAILMSG(ZONE_INFO, "hObject.GetToken():  0x%x", hObject.GetToken());
    RETAILMSG(ZONE_INFO, "hObject:             0x%x", (UINT32)hObject);
    
    Handle<Object> hObjectNull;
    objectManager.Get( "undefined", &hObjectNull );
    RETAILMSG(ZONE_INFO, "hObjectNull.IsNull() = %d", hObjectNull.IsNull());
    
    Handle<Object> hObject2;
    objectManager.Get( "/Objects/Object1", &hObject2 );
    RETAILMSG(ZONE_INFO, "hObject2.IsNull()    = %d", hObject2.IsNull());
    RETAILMSG(ZONE_INFO, "hObject2:            0x%x", (UINT32)hObject2);

    if (FAILED(objectManager.Remove( hObject2 )))
    {
        RETAILMSG(ZONE_INFO, "ERROR: objectManager.Remove( 0x%x ) failed.", (UINT32)hObject2);
    }
    RETAILMSG(ZONE_INFO, "ObjectManager.Count(): %d", objectManager.Count());

    RETAILMSG(ZONE_INFO, "pObject->GetRefCount(): %d", pObject->GetRefCount());
    
    objectManager.Release( hObject2 );

    // By now the refcount should be one
    RETAILMSG(ZONE_INFO, "pObject->GetRefCount(): %d", pObject->GetRefCount());

    objectManager.Release( hObject );
    //SAFE_RELEASE(pObject);
    
    /*

    //
    // Add a bunch of Objects
    //
    for (int i = 0; i < 10; ++i)
    {
        string name = "/Objects/Object" + i;
        Handle<Object> handle;
        
        objectManager.Add( name.c_str(), Object::Create(), NULL );
    }

    
    //
    // Create a bunch of Handles
    // 
    Handle<Object> hObjects[20];
    for (int i = 0; i < 20; ++i)
    {
        Handle<Object> *phObject = &hObjects[i];

        *phObject = objectManager.OpenHandle("/Objects/Object0");
    }

    for (int i = 0; i < 20; ++i)
    {
        Handle<Object> *phObject = &hObjects[i];

        RETAILMSG(ZONE_INFO, "hObject[%d] = 0x%x", i, phObject->GetHandle());
    }

    for (int i = 0; i < 20; ++i)
    {
        Handle<Object> *phObject = &hObjects[i];
        
        objectManager.CloseHandle( *phObject );
    }
*/
 
    objectManager.Shutdown();
    
Exit:
    return rval;
}




bool TestTextureManager()
{
    bool rval = true;
    
    TextureMan.Init( "/app/settings/Textures.xml" );
    
Exit:
    return rval;
}



bool TestFileManager()
{
    bool rval = true;

//    RESULT          OpenFile    ( IN    const string& pFilename, INOUT HFile* pHandle, IN const char* pMode = "rb+"        );
//    RESULT          GetFileSize ( IN    HFile handle, INOUT UINT32* pFilesize                                              );
//    RESULT          ReadFile    ( IN    HFile handle, INOUT BYTE* pBuffer, IN UINT32 numBytes, INOUT UINT32* pBytesRead    );
//    RESULT          WriteFile   ( IN    HFile handle, INOUT BYTE* pBuffer, IN UINT32 numBytes, INOUT UINT32* pBytesWritten );
//    RESULT          CloseFile   ( IN    HFile handle                                                                       );

    HFile hFile;

    FileMan.Init();
    
    
    // Test invalid arguments
    RESULT result = S_OK;
    result = FileMan.OpenFile( "", &hFile );
    result = FileMan.OpenFile( "/some/file", NULL );
    result = FileMan.OpenFile( "/some/file", &hFile );
    result = FileMan.OpenFile( "/some/file", &hFile, (FileManager::FILE_MODE)-1 );
    
    result = FileMan.OpenFile( "/app/settings/settings.xml", &hFile );

    // Get the file size
    UINT32 size;
    result = FileMan.GetFileSize( hFile, &size );
    if (S_OK != result)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileMan.GetFileSize( 0x%x ): return error 0x%x",
                  (UINT32)hFile, result);
    }
    else 
    {
        RETAILMSG(ZONE_INFO, "/app/settings/settings.xml size = %d bytes", size);
    }

    
    //
    // Read file into a buffer
    //
    UINT32 numBytesRead = 0;
    BYTE* pBuffer= new BYTE[ size ];
    result = FileMan.ReadFile( hFile, pBuffer, size, &numBytesRead );
    
    //
    // Print contents of file
    //
    RETAILMSG(ZONE_INFO, "File = \"%s\"", pBuffer);
    
    //
    // Open file and write to it
    //
    HFile hFileOut;
    UINT32 numBytesWritten;
    result = FileMan.OpenFile( "/storage/settings/settings_copy.xml", &hFileOut, FileManager::WRITE );
    
    result = FileMan.WriteFile( hFileOut, pBuffer, size, &numBytesWritten );
    assert( numBytesWritten == size );
    
    delete[] pBuffer;
    result = FileMan.CloseFile( hFile );
    
    //
    // Test closed handle
    //
    result = FileMan.GetFileSize( hFile, &size );
    result = FileMan.ReadFile( hFile, pBuffer, size, &numBytesRead );
    
    
Exit:
    return rval;
}




bool TestShaderManager()
{
    bool rval = true;
    
    RESULT  result = S_OK;
    HShader hDefaultShader;
    
    ShaderMan.Init( "/app/settings/shaders.xml" );
    
    // Create a shader effect
//    result = ShaderMan.CreateShader( "default", "/app/shaders/DefaultEffect_vs", "/app/shaders/DefaultEffect_fs", &hDefaultShader );
    result = ShaderMan.Get( "default", &hDefaultShader );
      
    
Exit:
    return rval;
}



bool TestImage()
{
    bool    rval        = true;
    RESULT  result      = S_OK;
    string  filename    = "/app/textures/texture_atlas0.png";
    HFile   hFile;
    UINT32  fileSize;
    UINT32  numBytesRead;
    BYTE*   pBuffer;
    BYTE*   pBufferRGBA;
    UINT32  pBufferRGBASize;
    
    //
    // Load the source bitmap
    //
    CHR( FileMan.OpenFile( filename, &hFile ) );
    CHR( FileMan.GetFileSize( hFile, &fileSize ) );
    pBuffer = new BYTE[fileSize];
    CHR( FileMan.ReadFile( hFile, pBuffer, fileSize, &numBytesRead ) );
    assert( fileSize == numBytesRead );

    //
    // Convert to RGBA
    //
    result = Image::ConvertPNGToRGBA(pBuffer, fileSize, &pBufferRGBA, &pBufferRGBASize );
    
    // Debug:
    FileMan.Print();
    
    
Exit:
    SAFE_ARRAY_DELETE(pBuffer);
    SAFE_ARRAY_DELETE(pBufferRGBA);
    FileMan.CloseFile( hFile );
    return rval;
}



bool TestCamera()
{
    bool rval = true;
    
    Camera camera;
    
    vec3 position   = vec3(0.0f, 0.0f, 0.0f);
    vec3 lookAt     = vec3(0.0f, 0.0f, -1.0f);
    camera.SetPosition  ( position );
    camera.SetLookAt    ( lookAt   );
    camera.SetProjection( 0.53f, 2.0/3.0, 1.0f, 1000.0f);

//    vec3 up         = camera.GetUpVector();
//    vec3 right      = camera.GetRightVector();
//    vec3 ahead      = camera.GetAheadVector();
//    vec3 eye        = camera.GetEyePt();
    
    mat4 projection = camera.GetProjectionMatrix();
//    mat4 view       = camera.GetWorldMatrix();
    
Exit:
    return rval;
}


bool TestRender()
{
    bool rval = true;
    
    
Exit:
    return rval;
}


/*
bool TestAnimation()
{
    bool rval = true;
    
    RESULT      result = S_OK;
    HAnimation  hAnimation1;
    HAnimation  hAnimation2;
    
    // Get an animation instance
    result  = AnimationMan.GetCopy( "default", &hAnimation1 );
    result  = AnimationMan.GetCopy( "default", &hAnimation2 );
    
    assert(hAnimation1 != hAnimation2);

    float targetValue = 0.0f;


//    AnimationTarget animationTarget;
//    animationTarget.BindTo( &targetValue );
//    result  = AnimationMan.BindTo( hAnimation1, &animationTarget );
    result = AnimationMan.BindTo( hAnimation1, &targetValue );
    
    
    result = AnimationMan.Start( hAnimation1 );
    result = AnimationMan.Pause( hAnimation1 );
    result = AnimationMan.Stop ( hAnimation1 );
    result = AnimationMan.Start( hAnimation1 );
    result = AnimationMan.Start( hAnimation2 );
    
    
    // For tick 0...n
    //      update animation
    //      apply animation to objet
    
    
    UINT32 start = Platform::GetTickCount();
    while(Platform::GetTickCount() < start + 5000)
    {
        AnimationMan.Update( Platform::GetTickCount() );
//        RETAILMSG(ZONE_INFO, "targetValue = %4.4f", targetValue);
    }
    
    
   
    // Release the animation instance
    CHR(AnimationMan.Release( hAnimation1 ));
    CHR(AnimationMan.Release( hAnimation2 ));
    
Exit:
    return rval;
}
*/



bool TestGameObject()
{
    bool    rval     = true;
    RESULT  result   = S_OK;
    char    name[MAX_NAME];
    HSprite hSprite;
    
    const UINT32 NUM_SPRITES = 200;

    //
    // Add a background Sprite while we're at it
    //
//    GameObject* pGO = new GameObject( GO_TYPE_SPRITE );
//    pGO->Init( "background", NULL );
//    result = SpriteMan.GetHandle( "background", &hSprite );
//    result = pGO->SetSprite( hSprite );
//    result = GOMan.Add( pGO->GetName(), pGO, NULL );
//    result = pGO->SetOpacity( 0.20f );
    
    // Create a GameObject
    // Bind sprite frames to the GO
    // Bind animation controller to the GO

    HGameObject goHandles[NUM_SPRITES];
    HStoryboard acHandles[NUM_SPRITES];
    
    for (int i = 0; i < NUM_SPRITES; ++i)
    {
        GameObject* pGO = new GameObject( GO_TYPE_SPRITE );
        sprintf(name, "SpriteTest%d", i);
        pGO->Init(name, NULL, "");

        // The GameObject copies and owns the Sprite handle.
        // No need for us to close it.
        result = SpriteMan.GetCopy( "MarineFace", &hSprite );
        result = pGO->SetSprite( hSprite );
       
        result = GOMan.Add( pGO->GetName(), pGO, &goHandles[i] );
        
        result = StoryboardMan.GetCopy( "SpriteAnimation", &acHandles[i] );
        result = StoryboardMan.BindTo( acHandles[i], goHandles[i] );
//        result = pGO->SetStoryboard( acHandles[i] );
    }

    // GO creation can be expensive, so create all of them before
    // starting the animations, or the animations will be out-of-sync.
    for (int i = 0; i < NUM_SPRITES; ++i)
    {
        result = StoryboardMan.Start( acHandles[i] );
    }

    
Exit:
    return rval;
}


bool TestMetrics()
{
    bool rval = true;
    
    UINT64 uint64Val;
    double doubleVal;
    
    Metrics::Set( METRIC_RUNTIME_SECONDS,   &(uint64Val = 10)      );
    Metrics::Set( METRIC_FPS_AVG,           &(doubleVal = 30.25)   );
    Metrics::Set( METRIC_FPS_LOW,           &(doubleVal = 20)      );
    Metrics::Set( METRIC_FPS_HIGH,          &(doubleVal = 60)      );
    Metrics::Set( METRIC_RAM_LOW,           &(uint64Val = 10)      );
    Metrics::Set( METRIC_RAM_HIGH,          &(uint64Val = 50)      );
    Metrics::Set( METRIC_TEXTURE_CHANGES,   &(uint64Val = 0xFFFFFFF) );
    
    RETAILMSG(ZONE_INFO, "METRIC_RUNTIME_SECONDS    = %d", *(UINT64*)Metrics::Get(METRIC_RUNTIME_SECONDS));
    RETAILMSG(ZONE_INFO, "METRIC_FPS_AVG            = %f", *(double*)Metrics::Get(METRIC_FPS_AVG));
    RETAILMSG(ZONE_INFO, "METRIC_FPS_LOW            = %f", *(double*)Metrics::Get(METRIC_FPS_LOW));
    RETAILMSG(ZONE_INFO, "METRIC_FPS_HIGH           = %f", *(double*)Metrics::Get(METRIC_FPS_HIGH));
    RETAILMSG(ZONE_INFO, "METRIC_RAM_LOW            = %d", *(UINT64*)Metrics::Get(METRIC_RAM_LOW));
    RETAILMSG(ZONE_INFO, "METRIC_RAM_HIGH           = %d", *(UINT64*)Metrics::Get(METRIC_RAM_HIGH));
    RETAILMSG(ZONE_INFO, "METRIC_TEXTURE_CHANGES    = %d", *(UINT64*)Metrics::Get(METRIC_TEXTURE_CHANGES));
    
    
Exit:
    return rval;
}



bool TestGameTime()
{
    double dtime    = 0.0;
    double dtime2   = 0.0;
    UINT64 time     = 0;
    UINT64 time2    = 0;
    
    
    time    =  Platform::GetTickCount();
    time2   =  GameTime.GetTime();
    
    dtime   = GameTime.GetTimeDouble();
    RETAILMSG(ZONE_INFO, "Platform time: %lld, game time: %lld, dtime: %f, dtime2: %f", time, time2, dtime, dtime2);

    Platform::Sleep( 1000 );
    
    dtime2  = GameTime.GetTimeDouble();
    RETAILMSG(ZONE_INFO, "Platform time: %lld, game time: %lld, dtime: %f, dtime2: %f", time, time2, dtime, dtime2);
    
    for(int i = 0; i < 10; ++i)
    {
        Platform::Sleep( 33 );
        RETAILMSG(ZONE_INFO, "dtime2: %8.8f", GameTime.GetTimeDouble());
    }
    
    
    // Test Pause/Resume
    time  = Platform::GetTickCount();
    time2 = GameTime.GetTime();
    dtime = GameTime.GetTimeDouble();
    RETAILMSG(ZONE_INFO, "Time before Pause: %lld, %lld, %f", time, time2, dtime);
    GameTime.Pause();
    
    Platform::Sleep( 5000 );
    
    GameTime.Resume();
    time  = Platform::GetTickCount();
    time2 = GameTime.GetTime();
    dtime = GameTime.GetTimeDouble();
    RETAILMSG(ZONE_INFO, "Time after  Pause: %lld, %lld, %f", time, time2, dtime);
    
    
    return true;
}



bool TestStateMachine()
{
    RESULT result = S_OK;
    
    //StateMachine* pStateMachine = NULL;
    
    // TODO: Create a StateMachineManager
    // TODO: add to a Game Object
    // TODO: Create a StateMachine
    // TODO: push onto Game Object
    
    //Create unit test game objects
    GameObject* unittest1   = new GameObject( GO_TYPE_ANY ); unittest1->Init("UnitTest1");
    GameObject* unittest2   = new GameObject( GO_TYPE_ANY ); unittest2->Init("UnitTest2");
    GameObject* unittest3a  = new GameObject( GO_TYPE_ANY ); unittest3a->Init("UnitTest3a");
    GameObject* unittest3b  = new GameObject( GO_TYPE_ANY ); unittest3b->Init("UnitTest3b");
    GameObject* unittest4   = new GameObject( GO_TYPE_ANY ); unittest4->Init("UnitTest4");
    GameObject* unittest5a  = new GameObject( GO_TYPE_ANY ); unittest5a->Init("UnitTest5a");
    GameObject* unittest5b  = new GameObject( GO_TYPE_ANY ); unittest5b->Init("UnitTest5b");
/*    
    D3DXVECTOR3 pos(0.0f, 0.0f, 0.0f);
    unittest1->CreateBody( 100, pos );
    unittest2->CreateBody( 100, pos );
    unittest3a->CreateBody( 100, pos );
    unittest3b->CreateBody( 100, pos );
    unittest4->CreateBody( 100, pos );
    unittest5a->CreateBody( 100, pos );
    unittest5b->CreateBody( 100, pos );
*/    
    unittest1->CreateStateMachineManager();
    unittest2->CreateStateMachineManager();
    unittest3a->CreateStateMachineManager();
    unittest3b->CreateStateMachineManager();
    unittest4->CreateStateMachineManager();
    unittest5a->CreateStateMachineManager();
    unittest5b->CreateStateMachineManager();
    
    result = GOMan.Add( unittest1->GetName(), unittest1  );
    result = GOMan.Add( unittest2->GetName(), unittest2  );
    result = GOMan.Add( unittest3a->GetName(), unittest3a );
    result = GOMan.Add( unittest3b->GetName(), unittest3b );
    result = GOMan.Add( unittest4->GetName(), unittest4  );
    result = GOMan.Add( unittest5a->GetName(), unittest5a );
    result = GOMan.Add( unittest5b->GetName(), unittest5b );
    
    //Give the unit test game objects a state machine
    unittest1->GetStateMachineManager()->PushStateMachine( *new UnitTest1( *unittest1 ), STATE_MACHINE_QUEUE_0, true );
    unittest2->GetStateMachineManager()->PushStateMachine( *new UnitTest2a( *unittest2 ), STATE_MACHINE_QUEUE_0, true );
    unittest3a->GetStateMachineManager()->PushStateMachine( *new UnitTest3a( *unittest3a ), STATE_MACHINE_QUEUE_0, true );
    unittest3b->GetStateMachineManager()->PushStateMachine( *new UnitTest3b( *unittest3b ), STATE_MACHINE_QUEUE_0, true );
    unittest4->GetStateMachineManager()->PushStateMachine( *new UnitTest4( *unittest4 ), STATE_MACHINE_QUEUE_0, true );
    unittest5a->GetStateMachineManager()->PushStateMachine( *new UnitTest5( *unittest5a ), STATE_MACHINE_QUEUE_0, true );
    unittest5b->GetStateMachineManager()->PushStateMachine( *new UnitTest5( *unittest5b ), STATE_MACHINE_QUEUE_0, true );
    
    
    
    return true;
}


bool TestLayer()
{
    // Open LayerSettings.xaml
    // For each Layer, create and init
    //      Layer has:
    //          Sprite
    //          Z-order
    //          Position
    //          Opacity
    //          IsVisible
    
    // LayerManager adds methods: 
    //      SetVisible( bool )
    
    RESULT      result;
    HLayer      hBackgroundLayer;
    HLayer      hGameLayer;
    HSprite     hBackgroundSprite;
    HSprite     hSprite;
    HGameObject hGO;
    

    //
    // Create the background and game layers
    //
    Layer* pLayer = new Layer();
    result = pLayer->Init("background");
    result = LayerMan.Add( pLayer->GetName(), pLayer, &hBackgroundLayer );
    
    pLayer = new Layer();
    result = pLayer->Init("gamelayer");
    result = LayerMan.Add( pLayer->GetName(), pLayer, &hGameLayer );
    
    //
    // Add a background Sprite
    //
    result = SpriteMan.GetCopy( "background", &hBackgroundSprite );
    result = LayerMan.AddToLayer( hBackgroundLayer, hBackgroundSprite );
    
    //
    // Add a Game Object
    //
    GameObject* pGO = new GameObject( GO_TYPE_SPRITE );
    pGO->Init( "Player1", NULL );
    result = SpriteMan.GetCopy( "MarineFace", &hSprite );
    result = pGO->SetSprite( hSprite );
    result = GOMan.Add( pGO->GetName(), pGO, &hGO );
    result = LayerMan.AddToLayer( hGameLayer, hSprite );
    
    LayerMan.Draw();
    
    
//    LayerMan.SetVisible( hBackgroundLayer, false );
    
    return true;
}


bool TestColumn()
{
    // Create a Column (GameObject; overrides Update() and Draw() to work collectively on its three Sprites)
    // Add ColumnStateMachine to Column
    //  StateCreate:
    //      Pick three random Sprites
    //      Add to slots
    //      Set position
    //      Create SparkleAnimation
    //      BindSparkleAnimation to Column
    //      Add to GOMan
    //      goto StatePlayable
    //  StatePlayable:
    //      Draw scaled and positioned off to one side
    //      goto StatePlaying
    //  StatePlaying:
    //      If movement message, set position
    
    return true;
}

/*
bool TestTouch()
{

    // Create GameObject to listen for touch events
    GameObject* pGO = new GameObject( GO_TYPE_TRIGGER );
    pGO->Init( "TouchEventListener", NULL );


    // Register for touch events
    TouchScreen.AddListener( pGO->GetID(), MSG_TouchBegin );
    
    // Deregister for touch events
    TouchScreen.RemoveListener( pGO->GetID(), MSG_TouchBegin );
    
Exit:
    return true;
}
*/


bool TestBoundingBox()
{
    AABB bounds;

    //
    // Create a Sprite
    //
    HSprite hSprite;
    SpriteMan.GetCopy( "MarineFace", &hSprite );

    bounds = SpriteMan.GetBounds( hSprite );
    RETAILMSG(1, "AABB = (%2.2f, %2.2f, %2.2f), (%2.2f, %2.2f, %2.2f)",
        bounds.GetMin().x, bounds.GetMin().y, bounds.GetMin().z,
        bounds.GetMax().x, bounds.GetMax().y, bounds.GetMax().z);
    
    
    // Scale
    SpriteMan.SetScale( hSprite, 2.0f );
    bounds = SpriteMan.GetBounds( hSprite );
    RETAILMSG(1, "AABB = (%2.2f, %2.2f, %2.2f), (%2.2f, %2.2f, %2.2f)",
        bounds.GetMin().x, bounds.GetMin().y, bounds.GetMin().z,
        bounds.GetMax().x, bounds.GetMax().y, bounds.GetMax().z);
        
    // BUG BUG: AABB totally broken for rotation!
    // Rotate
    SpriteMan.SetRotation( hSprite, vec3(0, 0, 45) );
    bounds = SpriteMan.GetBounds( hSprite );
    RETAILMSG(1, "AABB = (%2.2f, %2.2f, %2.2f), (%2.2f, %2.2f, %2.2f)",
        bounds.GetMin().x, bounds.GetMin().y, bounds.GetMin().z,
        bounds.GetMax().x, bounds.GetMax().y, bounds.GetMax().z);

    // Translate
    SpriteMan.SetPosition( hSprite, vec3(100, 100, 100) );
    bounds = SpriteMan.GetBounds( hSprite );
    RETAILMSG(1, "AABB = (%2.2f, %2.2f, %2.2f), (%2.2f, %2.2f, %2.2f)",
        bounds.GetMin().x, bounds.GetMin().y, bounds.GetMin().z,
        bounds.GetMax().x, bounds.GetMax().y, bounds.GetMax().z);




    //
    // Create a GameObject with a Sprite
    //
    GameObject* pGO = new GameObject();
    pGO->SetSprite( hSprite );
    

    bounds = pGO->GetBounds();
    RETAILMSG(1, "AABB = (%2.2f, %2.2f, %2.2f), (%2.2f, %2.2f, %2.2f)",
        bounds.GetMin().x, bounds.GetMin().y, bounds.GetMin().z,
        bounds.GetMax().x, bounds.GetMax().y, bounds.GetMax().z);
    
    
    // Scale
    pGO->SetScale( 2.0f );
    bounds = pGO->GetBounds();
    RETAILMSG(1, "AABB = (%2.2f, %2.2f, %2.2f), (%2.2f, %2.2f, %2.2f)",
        bounds.GetMin().x, bounds.GetMin().y, bounds.GetMin().z,
        bounds.GetMax().x, bounds.GetMax().y, bounds.GetMax().z);
        

    // Rotate
    pGO->SetRotation( vec3(0, 0, 45) );
    bounds = pGO->GetBounds();
    RETAILMSG(1, "AABB = (%2.2f, %2.2f, %2.2f), (%2.2f, %2.2f, %2.2f)",
        bounds.GetMin().x, bounds.GetMin().y, bounds.GetMin().z,
        bounds.GetMax().x, bounds.GetMax().y, bounds.GetMax().z);

    // Translate
    pGO->SetPosition( vec3(100, 100, 100) );
    bounds = pGO->GetBounds();
    RETAILMSG(1, "AABB = (%2.2f, %2.2f, %2.2f), (%2.2f, %2.2f, %2.2f)",
        bounds.GetMin().x, bounds.GetMin().y, bounds.GetMin().z,
        bounds.GetMax().x, bounds.GetMax().y, bounds.GetMax().z);



    SpriteMan.Release( hSprite );
    delete pGO;

    
    return true;
}



bool TestDebugRender()
{
    DebugRender.Line( vec3(0,0,0), vec3(0, 1, 0) );
    DebugRender.Draw();
    
    return true;
}



bool TestGameAnimations()
{
    RESULT rval = S_OK;

    HGameObject hBrick1;
    HGameObject hBrick2;
    HGameObject hBrick3;
    HGameObject hBrick4;

    HStoryboard hEraseBrick1;
    HStoryboard hEraseBrick2;
    HStoryboard hEraseBrick3;
    HStoryboard hEraseBrick4;


    QuadraticEaseInInterpolator quadratic;
    float progress = 0.0f;
    while( progress <= 1.0 )
    {
        float f = quadratic(0.0f, 1.0f, progress);
        printf("quadratic easing %f = %f\n", progress, f);
        
        vec3 v3 = quadratic(vec3(0,1,2), vec3(1,2,3), progress);
        printf("quadratic easing %f = (%f, %f, %f)\n", progress, v3.x, v3.y, v3.z);

        progress += 0.01;
    }


    rval = GOMan.Create( "Brick1", &hBrick1, "block_yellow", "", "default", "", GO_TYPE_SPRITE, vec3(64,0,0),  1.0 );
    rval = GOMan.Create( "Brick2", &hBrick2, "block_red",    "", "default", "", GO_TYPE_SPRITE, vec3(128,0,0), 1.0 );
    rval = GOMan.Create( "Brick3", &hBrick3, "block_blue",   "", "default", "", GO_TYPE_SPRITE, vec3(192,0,0), 1.0 );
    rval = GOMan.Create( "Brick4", &hBrick4, "block_green",  "", "default", "", GO_TYPE_SPRITE, vec3(256,0,0), 1.0 );

    rval = StoryboardMan.GetCopy( "EraseBrick", &hEraseBrick1 );
    rval = StoryboardMan.GetCopy( "EraseBrick", &hEraseBrick2 );
    rval = StoryboardMan.GetCopy( "EraseBrick", &hEraseBrick3 );
    rval = StoryboardMan.GetCopy( "EraseBrick", &hEraseBrick4 );
    
    rval = StoryboardMan.BindTo( hEraseBrick1,  hBrick1 );
    rval = StoryboardMan.BindTo( hEraseBrick2,  hBrick2 );
    rval = StoryboardMan.BindTo( hEraseBrick3,  hBrick3 );
    rval = StoryboardMan.BindTo( hEraseBrick4,  hBrick4 );
    
    rval = GOMan.Release( hBrick1 );
    rval = GOMan.Release( hBrick2 );
    rval = GOMan.Release( hBrick3 );
    rval = GOMan.Release( hBrick4 );
    
    rval = StoryboardMan.Start( hEraseBrick1 );
    rval = StoryboardMan.Start( hEraseBrick2 );
    rval = StoryboardMan.Start( hEraseBrick3 );
    rval = StoryboardMan.Start( hEraseBrick4 );

    for (UINT64 time = 0; time < 500; ++time)
    {
        StoryboardMan.Update( GameTime.GetTime() );
        
        Renderer.BeginFrame();
        Renderer.Clear( 1.0, 1.0, 1.0, 1.0 );
        SpriteMan.BeginBatch();
        GOMan.Draw( GO_TYPE_SPRITE );
        SpriteMan.EndBatch();
        Renderer.EndFrame();
    }
    
    rval = StoryboardMan.Release( hEraseBrick1 );
    rval = StoryboardMan.Release( hEraseBrick2 );
    rval = StoryboardMan.Release( hEraseBrick3 );
    rval = StoryboardMan.Release( hEraseBrick4 );

    rval = GOMan.Release( hBrick1 );
    rval = GOMan.Release( hBrick2 );
    rval = GOMan.Release( hBrick3 );
    rval = GOMan.Release( hBrick4 );

    return true;
}



bool TestStoryboard()
{
    // Create a Storyboard that loops forever.
    // Create a Storyboard that reverses once.
    // Create a Storyboard that reverses forever.
    // Create a Storyboard that self-deletes.

    RESULT rval;
    
    HStoryboard hLoopForever;
    HStoryboard hReverseOnce;
    HStoryboard hReverseForever;
    HStoryboard hSelfDelete;
    
    HGameObject hBrick1;
    HGameObject hBrick2;
    HGameObject hBrick3;
    HGameObject hBrick4;


    rval = GOMan.Create( "Brick1", &hBrick1, "block_yellow", "", "default", "", GO_TYPE_SPRITE, vec3(0,0,0),   1.0 );
    rval = GOMan.Create( "Brick2", &hBrick2, "block_red",    "", "default", "", GO_TYPE_SPRITE, vec3(64,0,0),  1.0 );
    rval = GOMan.Create( "Brick3", &hBrick3, "block_blue",   "", "default", "", GO_TYPE_SPRITE, vec3(128,0,0), 1.0 );
    rval = GOMan.Create( "Brick4", &hBrick4, "block_green",  "", "default", "", GO_TYPE_SPRITE, vec3(192,0,0), 1.0 );
    
    rval = StoryboardMan.GetCopy( "LoopForever",    &hLoopForever );
    rval = StoryboardMan.GetCopy( "ReverseOnce",    &hReverseOnce );
    rval = StoryboardMan.GetCopy( "ReverseForever", &hReverseForever );
    rval = StoryboardMan.GetCopy( "SelfDelete",     &hSelfDelete );
    
    rval = StoryboardMan.BindTo( hLoopForever,      hBrick1 );
    rval = StoryboardMan.BindTo( hReverseOnce,      hBrick2 );
    rval = StoryboardMan.BindTo( hReverseForever,   hBrick3 );
    rval = StoryboardMan.BindTo( hSelfDelete,       hBrick4 );
    
    rval = StoryboardMan.Start( hLoopForever    );
    rval = StoryboardMan.Start( hReverseOnce    );
    rval = StoryboardMan.Start( hReverseForever );
    rval = StoryboardMan.Start( hSelfDelete     );

    for (UINT64 time = 0; time < 1000; ++time)
    {
        StoryboardMan.Update( GameTime.GetTime() );
        
        Renderer.BeginFrame();
        Renderer.Clear( 1.0, 1.0, 1.0, 1.0 );
        SpriteMan.BeginBatch();
        GOMan.Draw( GO_TYPE_SPRITE );
        SpriteMan.EndBatch();
        Renderer.EndFrame();
    }
    
    rval = StoryboardMan.Remove( hLoopForever   );
    rval = StoryboardMan.Remove( hReverseOnce   );
    rval = StoryboardMan.Remove( hReverseForever );
    rval = StoryboardMan.Remove( hSelfDelete     );

    rval = GOMan.Release( hBrick1 );
    rval = GOMan.Release( hBrick2 );
    rval = GOMan.Release( hBrick3 );
    rval = GOMan.Release( hBrick4 );

    rval = GOMan.Remove( hBrick1 );
    rval = GOMan.Remove( hBrick2 );
    rval = GOMan.Remove( hBrick3 );
    rval = GOMan.Remove( hBrick4 );

    return true;
}



bool TestStoryboardStress()
{
    const  int NUM_OBJECTS   = 100;
    RESULT rval = S_OK;

    const char* spriteNames[] =
    {
        "MarineFace"
        "block_yellow",
        "block_red",
        "block_blue",
        "block_green",
        "block_purple"
    };

    const char* storyboardNames[]  = 
    {
        "LoopForever",
        "ReverseForever",
        "ReverseOnce",
        "SelfDelete"
    };
    

    HGameObject gameObjects[NUM_OBJECTS];
    HStoryboard storyboards[NUM_OBJECTS];

    for (int i = 0; i < NUM_OBJECTS; ++i)
    {
        const char* pSpriteName = spriteNames[ Platform::Random() % ARRAY_SIZE(spriteNames) ];
        WORLD_POSITION worldPos;
        worldPos.x = Platform::Random() % 640;
        worldPos.y = Platform::Random() % 960;
        worldPos.z = 0;
        
        rval = GOMan.Create( "", &gameObjects[i], pSpriteName, "", "default", "", GO_TYPE_SPRITE, worldPos,   1.0 );
        
        const char* pStoryboardName = storyboardNames[ Platform::Random() % ARRAY_SIZE(storyboardNames) ];
        rval = StoryboardMan.GetCopy( pStoryboardName, &storyboards[i] );
        rval = StoryboardMan.BindTo ( storyboards[i], gameObjects[i] );
    }


    for (int i = 0; i < NUM_OBJECTS; ++i)
    {
        rval = StoryboardMan.Start( storyboards[i] );
    }


    for (UINT64 time = 0; time < 1000; ++time)
    {
        StoryboardMan.Update( GameTime.GetTime() );
        
        Renderer.BeginFrame();
        Renderer.Clear( 1.0, 1.0, 1.0, 1.0 );
        SpriteMan.BeginBatch();
        GOMan.Draw( GO_TYPE_SPRITE );
        SpriteMan.EndBatch();
        Renderer.EndFrame();
    }
    

    for (int i = 0; i < NUM_OBJECTS; ++i)
    {
        StoryboardMan.Remove( storyboards[i] );
        GOMan.Remove( gameObjects[i] );
    }

    return true;
}



bool TestAABB()
{
    AABB bounds = AABB( vec3(100,100,0), vec3(200,200,0) );

    for (UINT64 time = 0; time < 100; ++time)
    {
        Renderer.BeginFrame();
        Renderer.Clear( 1.0, 1.0, 1.0, 1.0 );
        DebugRender.Quad( bounds );
        DebugRender.Draw();
        Renderer.EndFrame();
    }
    
    bounds *= 2.0f;

    for (UINT64 time = 0; time < 100; ++time)
    {
        Renderer.BeginFrame();
        Renderer.Clear( 1.0, 1.0, 1.0, 1.0 );
        DebugRender.Quad( bounds );
        DebugRender.Draw();
        Renderer.EndFrame();
    }

    return true;
}



bool TestFont()
{
    RESULT rval = S_OK;

    HFont hDebugFont;
    HFont hCrackedFont;
    HFont hMarkerFont;
    rval = FontMan.Get( "DebugFont",    &hDebugFont   );
    rval = FontMan.Get( "CrackedFont",  &hCrackedFont );
    rval = FontMan.Get( "MarkerFont",   &hMarkerFont  );

    
    float debugHeight   = FontMan.GetHeight( hDebugFont   );
//    float crackedHeight = FontMan.GetHeight( hCrackedFont );
    float markerHeight  = FontMan.GetHeight( hMarkerFont  );
    
    for (UINT64 time = 0; time < 500; ++time)
    {
        Renderer.BeginFrame();
        Renderer.Clear( 1.0, 1.0, 1.0, 1.0 );
        
        float row = 0;
        rval = FontMan.Draw( vec2(0,row),   "Purple 0.5",       hDebugFont, Color::Purple(), 0.5f, 1.0f, 0, 0, 0 ); row += debugHeight * 0.5f;
        rval = FontMan.Draw( vec2(0,row),   "Red 1.0",          hDebugFont, Color::Red(),    1.0f, 0.7f, 0, 0, 0 ); row += debugHeight * 1.0f;
        rval = FontMan.Draw( vec2(0,row),   "Yellow 2.0",       hDebugFont, Color::Yellow(), 2.0f, 0.5f, 0, 0, 0 ); row += debugHeight * 2.0f;
        rval = FontMan.Draw( vec2(0,row),   "Green 4.0",        hDebugFont, Color::Green(),  4.0f, 0.2f, 0, 0, 0 ); row += debugHeight * 4.0f;


        rval = FontMan.Draw( vec2(0,row),   "00000000000000000",    hMarkerFont, Color::Purple(), 1.0f, 1.0f, 0, 0, 0 );   row += markerHeight;
        rval = FontMan.Draw( vec2(0,row),   "XXXXXXXXXXXXXXXXX",    hMarkerFont, Color::Red(),    1.0f, 1.0f, 75, 0, 0 ); row += markerHeight;
        rval = FontMan.Draw( vec2(0,row),   "YYYYYYYYYYYYYYYYY",    hMarkerFont, Color::Yellow(), 1.0f, 1.0f, 0, 0, -45 ); row += markerHeight;
        rval = FontMan.Draw( vec2(0,row),   "01234567890123456789",    hMarkerFont, Color::Green(),  1.0f, 1.0f, 0, -75, 0 ); row += markerHeight;
/*        
        rval = FontMan.Draw( vec2(0,row),   "Purple 0.5",       hDebugFont, Color::Purple(), 0.5f, 1.0f, 0, 0, 0 );   row += debugHeight;
        rval = FontMan.Draw( vec2(0,row),   "Red 1.0",          hDebugFont, Color::Red(),    1.0f, 0.7f, -45, 0, 0 ); row += debugHeight;
        rval = FontMan.Draw( vec2(0,row),   "Yellow 2.0",       hDebugFont, Color::Yellow(), 2.0f, 0.5f, 0, 0, -45 ); row += debugHeight;
        rval = FontMan.Draw( vec2(0,row),   "Green 4.0",        hDebugFont, Color::Green(),  4.0f, 0.2f, 0, -45, 0 ); row += debugHeight;
        
        FontMan.Draw( vec2(0, 960 - crackedHeight),   "The \"Z engine\" supports",   hCrackedFont);
        FontMan.Draw( vec2(0, 960 - 2*crackedHeight), "font rendering now!",         hCrackedFont);
*/
        
        DebugRender.Draw();
        
        Renderer.EndFrame();
    }
    

    return true;
}



bool TestSound()
{
    HSound hTick;
    HSound hAwesomePowerup;
    HSound hPickup2;
    
//    SoundMan.SetAutoRepeat( hAwesomePowerup, true );
    SoundMan.Play( hTick );
    SoundMan.Play( hAwesomePowerup );
    SoundMan.Play( hPickup2 );

    for (UINT64 time = 0; time < 32; ++time)
    {
        // Test verbose form.
        HSound hSound;
        SoundMan.GetCopy( "Pickup2", &hSound );
        SoundMan.SetDeleteOnFinish( hSound, true );
        SoundMan.Play( hSound );
        Platform::Sleep(100);

        // Test simple form.
        SoundMan.Play("Awwww");
        
        Renderer.BeginFrame();
        Renderer.Clear( 1.0, 1.0, 1.0, 1.0 );

        DebugRender.Draw();
        Renderer.EndFrame();
    }
    

    //
    // Test background music.
    //
    HSound hBGMusic;
    SoundMan.Get( "BGMusic", &hBGMusic );
    SoundMan.Play( hBGMusic );

    return true;
}



bool TestEffect()
{
    RESULT rval = S_OK;

    HSprite hBackground;
    rval = SpriteMan.Get( "background", &hBackground );

    HSprite hSprite;
    rval = SpriteMan.Get( "MarineFace", &hSprite );

    HEffect hDefault;
    HEffect hBlur;
    HEffect hMorph;
    
    rval = Effects.GetCopy( "DefaultEffect", &hDefault );
    rval = Effects.GetCopy( "BlurEffect",    &hBlur    );
    rval = Effects.GetCopy( "MorphEffect",   &hMorph   );
    
    RETAILMSG(ZONE_INFO, "sizeof(h) = %d", sizeof(hMorph));
    
    for (UINT64 time = 0; time < 300; ++time)
    {
        Renderer.PushEffect( hMorph );
        Renderer.BeginFrame();
        Renderer.Clear( 1.0, 1.0, 1.0, 1.0 );

        SpriteMan.BeginBatch();
        SpriteMan.DrawSprite( hBackground, vec3(0,0,0) );
        SpriteMan.DrawSprite( hSprite, vec3(0,0,0) );
        SpriteMan.DrawSprite( hSprite, vec3(0,200,0) );
        SpriteMan.DrawSprite( hSprite, vec3(0,400,0) );
        SpriteMan.EndBatch();
        
        Renderer.PopEffect();
        
        //DebugRender.Draw();
        Renderer.EndFrame();
    }
        
    
    rval = SpriteMan.Release( hBackground );
    rval = SpriteMan.Release( hSprite     );
    rval = Effects.Release( hDefault );
    rval = Effects.Release( hBlur    );
    rval = Effects.Release( hMorph   );
    
    return true;
}




bool TestRandom()
{
    UINT32  a = Platform::Random();
    UINT32  b = Platform::Random((UINT32)0, (UINT32)100);
    UINT32  c = Platform::Random((UINT32)100, (UINT32)200);
    double  x = Platform::RandomDouble(0.0, 100.0);
    double  y = Platform::RandomDouble(100.0, 200.0);

    RETAILMSG(ZONE_INFO, "%d %d %d %f %f", a, b, c, x, y);

    for (int i = 0; i < 100; ++i)
    {
        RETAILMSG(ZONE_INFO, "%d ", Platform::Random(0, 2));
    }

    for (int i = 0; i < 100; ++i)
    {
        RETAILMSG(ZONE_INFO, "%1.4f ", Platform::RandomDouble(0.0, 1.0));
    }

    return true;
}



bool TestProperties()
{
    RippleEffect*   pEffect = NULL;
    IProperty*      pFoo    = NULL;
    HEffect         hEffect;
    RESULT          rval;
    
    rval = Effects.Get( "RippleEffect", &hEffect );
    rval = Effects.GetPointer( hEffect, (IEffect**)&pEffect );
    
    if (pEffect)
    {
        pFoo = pEffect->GetProperty( "Amplitude" );
    
        float f = pFoo->GetFloat();
    
        RETAILMSG(ZONE_INFO, "%f", f);
    }
    
    return true;
}


bool TestProbability()
{
    UINT32 numIterations = 50;

    float  probabilities[] = { 1.0, 0.75, 0.5, 0.25, 0.10, 0.01 };

    for (int i = 0; i < ARRAY_SIZE(probabilities); ++i)
    {
        float  probability  = probabilities[i];
        UINT32 successCount = 0;
    
        for (int j = 0; j < numIterations; ++j)
        {
            if (Platform::RandomDouble(0.0, 1.0) <= probability)
            {
                successCount++;
            }
        }
        
        RETAILMSG(ZONE_INFO, "Probability %2.2f came up %2.2f percent of the time.", probability, ((float)successCount/(float)numIterations) * 100.0f);
    }


    return true;
}


bool TestSpriteFromFile()
{
    RESULT rval = S_OK;

    UINT32 freeBytes, usedBytes;
    HSprite hSprite;
    
    for (int i = 0; i < 100; ++i)
    {
        rval = SpriteMan.CreateFromFile( "/app/textures/explosion.png", &hSprite );
        rval = SpriteMan.Release( hSprite );

        freeBytes = Platform::GetAvailableMemory();
        usedBytes = Platform::GetProcessUsedMemory();
        RETAILMSG(ZONE_INFO, "Free RAM: %d Used RAM: %d numObjects: %d", freeBytes/1048576, usedBytes/1048576, Object::Count());
    }

    return true;
}


bool TestJSON()
{
    RESULT rval = S_OK;

    // Parse a .JSON file
    Json::Value  root;
    Json::Reader reader;

    HFile hJSONFile;
    rval = FileMan.OpenFile( "/app/textures/texture_atlas.json", &hJSONFile );

    size_t fileSize     = 0;
    size_t bytesRead    = 0;
    FileMan.GetFileSize( hJSONFile, &fileSize );
    assert(fileSize);
    const char*  buffer = (char*)new char[fileSize];

    FileMan.ReadFile( hJSONFile, (BYTE*)buffer, fileSize, &bytesRead );
    assert( fileSize == bytesRead );

    std::string* pDocument = new std::string( buffer, bytesRead );
    bool parsingSuccessful = reader.parse( *pDocument, root );
    if ( !parsingSuccessful )
    {
        // report to the user the failure and their locations in the document.
        RETAILMSG(ZONE_INFO, "ERROR: %s", reader.getFormattedErrorMessages().c_str());
    }

    RETAILMSG(ZONE_INFO, "TextureAtlas.filename = \"%s\"", root["meta"]["image"].asCString());

    const Json::Value textures = root["frames"];

    Json::Value::Members memberNames = textures.getMemberNames();
    Json::Value::Members::iterator pMemberName;
    for (pMemberName = memberNames.begin(); pMemberName != memberNames.end(); ++pMemberName)
    {
        std::string name = *pMemberName;
        
        RETAILMSG(ZONE_INFO, "Texture = \"%s\"", name.c_str());

        Json::Value texture = textures[name];

        Json::Value frame = texture["frame"];
        Rectangle frameRect;
        frameRect.x      = frame["x"].asFloat();
        frameRect.y      = frame["y"].asFloat();
        frameRect.width  = frame["w"].asFloat();
        frameRect.height = frame["h"].asFloat();

        Json::Value spriteSourceSize = texture["spriteSourceSize"];
        Json::Value sourceSize       = texture["sourceSize"];
        Rectangle   spriteSourceSizeRect;
        Rectangle   sourceSizeRect;
        
        spriteSourceSizeRect.x      = spriteSourceSize["x"].asFloat();
        spriteSourceSizeRect.y      = spriteSourceSize["y"].asFloat();
        spriteSourceSizeRect.width  = spriteSourceSize["w"].asFloat();
        spriteSourceSizeRect.height = spriteSourceSize["h"].asFloat();

        sourceSizeRect.width        = sourceSize["w"].asFloat();
        sourceSizeRect.height       = sourceSize["h"].asFloat();
        
        RETAILMSG(ZONE_INFO, "\tframe = %2.2f x %2.2f @ (%0.2f,%0.2f)",             frameRect.width, frameRect.height, frameRect.x, frameRect.y);
        RETAILMSG(ZONE_INFO, "\trotated = %s",                                      texture["rotated"].asString().c_str());
        RETAILMSG(ZONE_INFO, "\ttrimmed = %s",                                      texture["trimmed"].asString().c_str());
        RETAILMSG(ZONE_INFO, "\tspriteSourceSize = %2.2f x %2.2f @ (%0.2f,%0.2f)",  spriteSourceSizeRect.width, spriteSourceSizeRect.height, spriteSourceSizeRect.x, spriteSourceSizeRect.y);
        RETAILMSG(ZONE_INFO, "\tsourceSize = %2.2f x %2.2f",                        sourceSizeRect.width, sourceSizeRect.height);
    }
 
    FileMan.CloseFile( hJSONFile );
    SAFE_ARRAY_DELETE(buffer);
    SAFE_DELETE(pDocument);
    
    return true;
}


bool TestTexturePacker()
{
    const char* spriteNames[] = 
    {
        "cat1",
        "dog1",
        "fox1",
        "lizard1",
        "rabbit1",
        "bomb",
        "stone",
        "stone_inverse",
        "block_overlay",
        "block_orange",
        "block_green",
        "block_yellow",
        "block_blue",
        "block_white",
        "block_red",
        "swoosh_rotate",
        "swoosh_drop"
    };
    HSprite hSprites[ARRAY_SIZE(spriteNames)];

    HLayer hTestLayer;
    LayerMan.CreateLayer( "TestLayer", &hTestLayer );

    WORLD_POSITION pos(0,0,1.0);

    float maxSpriteHeight = 0.0f;
    for (int i = 0; i < ARRAY_SIZE(spriteNames); ++i)
    {
        SpriteMan.GetCopy( spriteNames[i], &hSprites[i] );

        AABB      spriteBounds;
        spriteBounds = SpriteMan.GetBounds( hSprites[i] );
        maxSpriteHeight = MAX(maxSpriteHeight, spriteBounds.GetHeight());
        
        SpriteMan.SetPosition( hSprites[i], pos );

        pos.x += spriteBounds.GetWidth();
        if (pos.x >= 640)
        {
            pos.x = 0;
            pos.y += maxSpriteHeight;
        }

        LayerMan.AddToLayer( hTestLayer, hSprites[i] ); 
    }


    for (UINT64 time = 0; time < 500; ++time)
    {
        Renderer.BeginFrame();
        Renderer.Clear( 1.0, 1.0, 1.0, 1.0 );
        LayerMan.Draw();
        Renderer.EndFrame();
    }
    
    
    for (int i = 0; i < ARRAY_SIZE(spriteNames); ++i)
    {
        hSprites[i].Release();
    }
    
    LayerMan.Release( hTestLayer );
    assert(0 == hTestLayer.GetRefCount() );

    return true;
}




bool TestParticles()
{
    RESULT result = S_OK;
    
    HLayer hTestLayer;
    Layer* pLayer = new Layer();
    result = pLayer->Init("TestLayer");
    result = LayerMan.Add( pLayer->GetName(), pLayer, &hTestLayer );
    result = LayerMan.SetVisible( hTestLayer, true );
    
    Rectangle screenRect;
    Platform::GetScreenRect( &screenRect );
    vec3 position( screenRect.width/2, screenRect.height/2, 0 );
    
    HParticleEmitter hEmitter;
    result = Particles.GetCopy( "Shock", &hEmitter );
    result = Particles.SetPosition( hEmitter, position );
    result = LayerMan.AddToLayer( hTestLayer, hEmitter );
    result = Particles.Start( hEmitter );

    UINT64 startTimeMS = GameTime.GetTime();
    UINT64 endTimeMS   = startTimeMS + 30000;
    for (UINT64 time = startTimeMS; time < endTimeMS; time = GameTime.GetTime())
    {
        DebugRender.Text(vec3(0,900,0), "TestParticles");
    
        Particles.Update( time );
    
        Renderer.BeginFrame();
        Renderer.Clear( 0.3, 0.3, 0.4, 1.0 );
        LayerMan.Draw();
        DebugRender.Draw();
        DebugRender.Reset();
        Renderer.EndFrame();
        
        if ( !hEmitter.IsValid() || Particles.IsStopped( hEmitter) )
        {
            printf("SPAWN emitter\n");
            result = Particles.GetCopy( "Shock", &hEmitter );
            result = Particles.SetPosition( hEmitter, position );
            result = LayerMan.AddToLayer( hTestLayer, hEmitter );
            result = Particles.Start( hEmitter );
        }
    }
    
    
    return true;
}


bool TestEasing()
{
    HSprite hSprite;
    QuadraticEaseInInterpolator interpolator;

    HLayer hTestLayer;
    Layer* pLayer = new Layer();
    pLayer->Init("TestLayer");
    LayerMan.Add( pLayer->GetName(), pLayer, &hTestLayer );
    LayerMan.SetVisible( hTestLayer, true );

    Sprites.GetCopy( "block_blue", &hSprite );
    LayerMan.AddToLayer( hTestLayer, hSprite );


    float f1 = 1;
    float f2 = 4;

    UINT64 startTimeMS = GameTime.GetTime();
    UINT64 endTimeMS   = startTimeMS + 20000;
    for (UINT64 time = startTimeMS; time < endTimeMS; time = GameTime.GetTime())
    {
        DebugRender.Text(vec3(0,900,0), "TestEasing");
    
        Renderer.BeginFrame();
        Renderer.Clear( 1.0, 1.0, 1.0, 1.0 );

        for (int i = 0; i < 100; ++i)
        {
            float ratio = (float)((time-startTimeMS)/(endTimeMS-startTimeMS));
            float f = interpolator( f1, f2, ratio );
            
            RETAILMSG(ZONE_INFO, "%f = [%f, %f, %f]", f, f1, f2, ratio);
            
            Sprites.SetScale( hSprite, f );
        }


        LayerMan.Draw();
        DebugRender.Draw();
        DebugRender.Reset();
        Renderer.EndFrame();
    }

    return true;
}





bool TestRipple()
{
    HSprite hSprite;
    HEffect hEffect;
    HStoryboard hStoryboard;
    HLayer hTestLayer;

    // Layer
    Layer* pLayer = new Layer();
    pLayer->Init("TestLayer");
    LayerMan.Add( pLayer->GetName(), pLayer, &hTestLayer );
    LayerMan.SetVisible( hTestLayer, true );

    // Sprite
    Sprites.CreateFromFile( "/app/Default@2x.png", &hSprite );
    LayerMan.AddToLayer( hTestLayer, hSprite );

    // Effect
    EffectMan.GetCopy( "RippleEffect", &hEffect );
    LayerMan.SetEffect( hTestLayer, hEffect );
    
    // Storyboard
    StoryboardMan.GetCopy("TestRipple", &hStoryboard);
//    StoryboardMan.GetCopy("AwesomeRipple", &hStoryboard);
    StoryboardMan.BindTo( hStoryboard, hEffect );
    StoryboardMan.SetAutoRepeat( hStoryboard, true );
    StoryboardMan.SetReleaseOnFinish( hStoryboard, false );
    StoryboardMan.Start( hStoryboard );


    UINT64 startTimeMS = GameTime.GetTime();
    UINT64 endTimeMS   = startTimeMS + 60000;
    for (UINT64 time = startTimeMS; time < endTimeMS; time = GameTime.GetTime())
    {
        DebugRender.Text(vec3(0,900,0), "TestRipple");
    
        StoryboardMan.Update( GameTime.GetTime() );

        Renderer.BeginFrame();
        Renderer.Clear( 1.0, 1.0, 1.0, 1.0 );
        LayerMan.Draw();
        DebugRender.Draw();
        DebugRender.Reset();
        Renderer.EndFrame();
    }

    hSprite.Release();
    hTestLayer.Release();
    hEffect.Release();
    hStoryboard.Release();
    
    return true;
}


} // END namespace Z


