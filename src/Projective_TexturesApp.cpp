#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/gl/Light.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/DisplayList.h"
#include "cinder/gl/Material.h"
#include "cinder/ImageIo.h"

#include "cinder/params/Params.h"

#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static const Vec3f	CAM_POSITION_INIT( 0.0f, 0.0f, -21.0f);
static const Vec3f	LIGHT_POSITION_INIT( 0.0f, 4.0f, 0.0f );

class Water_Refraction_Test : public AppBasic 
{
public:
    Water_Refraction_Test();
    virtual ~Water_Refraction_Test();
    void prepareSettings( Settings *settings );
    
    void setup();
    void update();
    void draw();
    
    void mouseDown( MouseEvent event );	
    void keyDown( app::KeyEvent event ); 
	
    void drawTestObjects();
	
    void initFBOs();
    void initShaders();	
    void renderScreenSpace();
    void renderSceneToFBO();
	
protected:
	
    //debug
    cinder::params::InterfaceGl mParams;
    bool				mShowParams;
    float				mCurrFramerate;
    bool				mLightingOn;
    bool				mViewFromLight;
	
    //objects
    gl::DisplayList		mTorus, mBoard, mBox, mSphere;
	
    //camera
    CameraPersp			*mCam;
    Vec3f				mEye;
    Vec3f				mCenter;
    Vec3f				mUp;
    float				mCameraDistance;
	
    //light
    gl::Light			*mLight;
    gl::Light			*mLightRef;
	
    //
    gl::Fbo				mScreenSpace1;
	
    //
    gl::GlslProg		mBasicBlender;
    gl::GlslProg        mProjShader;
    
    gl::Texture         mGrassTex;
    gl::Texture         mTrollTex;
	
    //matrices required for texture projection
	Matrix44f			mTexGenMat;
	Matrix44f			mInvViewMat;
    
    //
    float mWobbleStartRad;
    Vec2f mWobbleAmp;
    Vec2f mWobbleFreq;
	
public:
};

Water_Refraction_Test::Water_Refraction_Test()
{}

Water_Refraction_Test::~Water_Refraction_Test()
{
	delete mCam;
	delete mLight;
	delete mLightRef;
}

void Water_Refraction_Test::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 720, 486 );		
	settings->setFrameRate( 60.0f );			//the more the merrier!
	settings->setResizable( false );			//this isn't going to be resizable
	
	//make sure secondary screen isn't blacked out as well when in fullscreen mode ( do wish it could accept keyboard focus though :(
	//settings->enableSecondaryDisplayBlanking( false );
}

void Water_Refraction_Test::setup()
{
	glEnable( GL_LIGHTING );
	glEnable( GL_DEPTH_TEST );
	glEnable(GL_RESCALE_NORMAL);
    //	glEnable(GL_POLYGON_SMOOTH);
    //	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    //	glEnable(GL_COLOR_MATERIAL);
	
	mParams = params::InterfaceGl( "3D_Scene_Base", Vec2i( 225, 250 ) );
	mParams.addParam( "Framerate", &mCurrFramerate, "", true );
	mParams.addParam( "Eye Distance", &mCameraDistance, "min=-100.0 max=-5.0 step=1.0 keyIncr== keyDecr=-");
	mParams.addParam( "Light Frustum", &mLightingOn, "key=l");
	mParams.addParam( "Show/Hide Params", &mShowParams, "key=x");
	
	mCurrFramerate = 0.0f;
	mLightingOn = true;
	mViewFromLight = false;
	mShowParams = true;
	
	//create camera
	mCameraDistance = CAM_POSITION_INIT.z;
	mEye		= Vec3f(CAM_POSITION_INIT.x, CAM_POSITION_INIT.y, CAM_POSITION_INIT.z);
	mCenter		= Vec3f::zero();
	mUp			= Vec3f::yAxis();
	
	mCam = new CameraPersp( getWindowWidth(), getWindowHeight(), 180.0f );
	mCam->lookAt(mEye, mCenter, mUp);
	mCam->setPerspective( 45.0f, getWindowAspectRatio(), 1.0f, 50.0f );
	gl::setMatrices( *mCam );
	
	//create light
	mLight = new gl::Light( gl::Light::DIRECTIONAL, 0 );
	mLight->lookAt( Vec3f(LIGHT_POSITION_INIT.x, LIGHT_POSITION_INIT.y * -1, LIGHT_POSITION_INIT.z), Vec3f( 0, 0, 0 ) );
	mLight->setAmbient( Color( 1.0f, 1.0f, 1.0f ) );
	mLight->setDiffuse( Color( 1.0f, 1.0f, 1.0f ) );
	mLight->setSpecular( Color( 1.0f, 1.0f, 1.0f ) );
	mLight->setShadowParams( 100.0f, 1.0f, 20.0f );
	mLight->update( *mCam );
	mLight->enable();
	
	//create light ref
	mLightRef = new gl::Light( gl::Light::DIRECTIONAL, 0 );
	mLightRef->lookAt( LIGHT_POSITION_INIT, Vec3f( 0, 0, 0 ) );
	mLightRef->setShadowParams( 100.0f, 1.0f, 20.0f );
	
	//DEBUG Test objects
	ci::ColorA pink( CM_RGB, 0.84f, 0.49f, 0.50f, 1.0f );
	ci::ColorA green( CM_RGB, 0.39f, 0.78f, 0.64f, 1.0f );
	ci::ColorA blue( CM_RGB, 0.32f, 0.59f, 0.81f, 1.0f );
	ci::ColorA orange( CM_RGB, 0.77f, 0.35f, 0.35f, 1.0f );
	
	gl::Material torusMaterial;
	torusMaterial.setSpecular( ColorA( 1.0, 1.0, 1.0, 1.0 ) );
	torusMaterial.setDiffuse( pink );
	torusMaterial.setAmbient( ColorA( 0.3, 0.3, 0.3, 1.0 ) );
	torusMaterial.setShininess( 25.0f );
	
	gl::Material boardMaterial;
	boardMaterial.setSpecular( ColorA( 0.0, 0.0, 0.0, 0.0 ) );
	boardMaterial.setAmbient( ColorA( 0.3, 0.3, 0.3, 1.0 ) );
	boardMaterial.setDiffuse( green );	
	boardMaterial.setShininess( 0.0f );
	
	gl::Material boxMaterial;
	boxMaterial.setSpecular( ColorA( 0.0, 0.0, 0.0, 0.0 ) );
	boxMaterial.setAmbient( ColorA( 0.3, 0.3, 0.3, 1.0 ) );
	boxMaterial.setDiffuse( blue );	
	boxMaterial.setShininess( 0.0f );
	
	gl::Material sphereMaterial;
	sphereMaterial.setSpecular( ColorA( 1.0, 1.0, 1.0, 1.0 ) );
	sphereMaterial.setAmbient( ColorA( 0.3, 0.3, 0.3, 1.0 ) );
	sphereMaterial.setDiffuse( orange ) ;	
	sphereMaterial.setShininess( 35.0f );	
	
	mTorus = gl::DisplayList( GL_COMPILE );
	mTorus.newList();
	gl::drawTorus( 1.0f, 0.3f, 32, 64 );
	mTorus.endList();
	mTorus.setMaterial( torusMaterial );
	
	mBoard = gl::DisplayList( GL_COMPILE );
	mBoard.newList();
	gl::drawCube( Vec3f( 0.0f, 0.0f, 0.0f ), Vec3f( 10.0f, 0.1f, 10.0f ) );
	mBoard.endList();
	mBoard.setMaterial( boardMaterial );
	
	mBox = gl::DisplayList( GL_COMPILE );
	mBox.newList();
	gl::drawCube( Vec3f( 0.0f, 0.0f, 0.0f ), Vec3f( 1.0f, 1.0f, 1.0f ) );
	mBox.endList();
	mBox.setMaterial( boxMaterial );
	
	mSphere = gl::DisplayList( GL_COMPILE );
	mSphere.newList();
	gl::drawSphere( Vec3f::zero(), 0.8f, 30 );
	mSphere.endList();
	mSphere.setMaterial( sphereMaterial );
	
	initFBOs();
	initShaders();
	
	mWobbleStartRad = 0.0f;
	mWobbleAmp = Vec2f(0.01f, 0.02f);
	mWobbleFreq = Vec2f(0.2f, 0.8f);
    
    //init textures
    mGrassTex	= gl::Texture( loadImage( loadResource( GRASS_TEX_REP ) ) );
    mTrollTex	= gl::Texture( loadImage( loadResource( TROLL_TEX ) ) );
}

void Water_Refraction_Test::update()
{
    mTexGenMat = mLight->getShadowCamera().getModelViewMatrix();
	mInvViewMat = mCam->getInverseModelViewMatrix();
    
	mCurrFramerate = getAverageFps();
	mWobbleStartRad += 0.05f;
}

void Water_Refraction_Test::draw()
{
	glClearColor( 0.5f, 0.5f, 0.5f, 1 );
	glClearDepth(1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_LIGHTING );
    
	mLight->update( *mCam );
	
	renderSceneToFBO();
	renderScreenSpace();
	
	if (mShowParams)
		params::InterfaceGl::draw();
}

void Water_Refraction_Test::mouseDown( MouseEvent event )
{}

void Water_Refraction_Test::keyDown( app::KeyEvent event ) 
{
	switch ( event.getCode() ) 
	{
		case 273:
			//up
		{
			Vec3f lightPos = mLight->getPosition();
			if ( lightPos.y > 0 )
				lightPos.y *= -1;
			lightPos = lightPos + Vec3f(0, 0.0, 0.1);		
			mLight->lookAt( lightPos, Vec3f::zero() );
			//mLight->update( *mCam );
			
			lightPos = mLightRef->getPosition() + Vec3f(0, 0.0, 0.1);
			mLightRef->lookAt( lightPos, Vec3f::zero() );
			mLightRef->update( *mCam );
		}
			break;
		case 274:
			//down
		{
			Vec3f lightPos = mLight->getPosition();
			if ( lightPos.y > 0 )
				lightPos.y *= -1;
			lightPos = lightPos + Vec3f(0, 0.0, -0.1);		
			mLight->lookAt( lightPos, Vec3f::zero() );
			//mLight->update( *mCam );
			
			lightPos = mLightRef->getPosition() + Vec3f(0, 0.0, -0.1);	
			mLightRef->lookAt( lightPos, Vec3f::zero() );
			mLightRef->update( *mCam );
		}
			break;
		case 276:
			//left
		{
			Vec3f lightPos = mLight->getPosition();
			if ( lightPos.y > 0 )
				lightPos.y *= -1;
			lightPos = lightPos + Vec3f(0.1, 0, 0);		
			mLight->lookAt( lightPos, Vec3f::zero() );
			//mLight->update( *mCam );
			
			lightPos = mLightRef->getPosition() + Vec3f(0.1, 0, 0);
			mLightRef->lookAt( lightPos, Vec3f::zero() );
			mLightRef->update( *mCam );
		}
			break;
		case 275:
			//right
		{
			Vec3f lightPos = mLight->getPosition();
			if ( lightPos.y > 0 )
				lightPos.y *= -1;
			lightPos = lightPos + Vec3f(-0.1, 0, 0);		
			mLight->lookAt( lightPos, Vec3f::zero() );
			//mLight->update( *mCam );
			
			lightPos = mLightRef->getPosition() + Vec3f(-0.1, 0, 0);	
			mLightRef->lookAt( lightPos, Vec3f::zero() );
			mLightRef->update( *mCam );
		}
			break;
		case 119:
			//W
		{
			mEye = mCam->getEyePoint();
			mEye = Quatf( Vec3f(1, 0, 0), -0.03f ) * mEye;
			mCam->lookAt( mEye, Vec3f::zero() );
			//mLight->update( *mCam );
			mLightRef->update( *mCam );
		}
			break;
		case 97:
			//A
		{
			mEye = mCam->getEyePoint();
			mEye = Quatf( Vec3f(0, 1, 0), 0.03f ) * mEye;
			mCam->lookAt( mEye, Vec3f::zero() );
			//mLight->update( *mCam );
			mLightRef->update( *mCam );
		}
			break;	
		case 115:
			//S
		{
			mEye = mCam->getEyePoint();
			mEye = Quatf( Vec3f(1, 0, 0), 0.03f ) * mEye;
			mCam->lookAt( mEye, Vec3f::zero() );
			//mLight->update( *mCam );
			mLightRef->update( *mCam );
		}
			break;
		case 100:
			//D
		{
			mEye = mCam->getEyePoint();
			mEye = Quatf( Vec3f(0, 1, 0), -0.03f ) * mEye;
			mCam->lookAt( mEye, Vec3f::zero() );
			//mLight->update( *mCam );
			mLightRef->update( *mCam );
		}
			break;
		case 48:
			//0
		{
			//reset everything
			
		}
			break;	
		default:
			break;
	}
}

void Water_Refraction_Test::drawTestObjects()
{
    
    mTrollTex.bind(1);
    mGrassTex.bind(0);
    
    mProjShader.bind();
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    mProjShader.uniform("diffuseTex", 0);
    mProjShader.uniform("projMap", 1);
    mProjShader.uniform("TexGenMat", mTexGenMat);
    mProjShader.uniform("InvViewMat", mInvViewMat);
    mProjShader.uniform("alpha", 1.0f );
    
    //glColor3f(1.0, 0.2, 0.2);
	gl::pushMatrices();
	glTranslatef(-2.0f, -1.0f, 0.0f);
	glRotated(90.0f, 1, 0, 0);
    mTorus.draw();
	gl::popMatrices();
	
	//glColor3f(0.4, 1.0, 0.2);
	gl::pushMatrices();
	glTranslatef(0.0f, -1.35f, 0.0f);
    mBoard.draw();
	gl::popMatrices();
	
	//glColor3f(0.8, 0.5, 0.2);
	gl::pushMatrices();
	glTranslatef(0.4f, -0.3f, 0.5f);
	glScalef(2.0f, 2.0f, 2.0f);
    mBox.draw();
	gl::popMatrices();
	
	//glColor3f(0.3, 0.5, 0.9);
	gl::pushMatrices();
	glTranslatef(0.1f, -0.56f, -1.25f);
    mSphere.draw();
	gl::popMatrices();
    
    mProjShader.unbind();
    
    mTrollTex.unbind(1);
    mGrassTex.unbind(0);
}

void Water_Refraction_Test::renderScreenSpace()
{
	//render out main scene
	
	// use the scene we rendered into the FBO as a texture
	//glEnable( GL_TEXTURE_2D ); //not necessary as using shaders to texture ...
	
	// show the FBO texture in the upper left corner
	gl::setMatricesWindow( getWindowSize() );
	
	mScreenSpace1.getTexture(0).bind(0);
	
	mBasicBlender.bind();
	
	mBasicBlender.uniform("baseTex", 0 );
	mBasicBlender.uniform("StartRad", mWobbleStartRad );
	mBasicBlender.uniform("Freq", mWobbleFreq );
	mBasicBlender.uniform("Amplitude", mWobbleAmp );
	
	gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
	
	mBasicBlender.unbind();
	
	mScreenSpace1.getTexture(0).unbind(0);
	
	//glDisable(GL_TEXTURE_2D);
}

void Water_Refraction_Test::initShaders()
{
	mBasicBlender		= gl::GlslProg( loadResource( BBlender_VERT ), loadResource( BBlender_FRAG ) );
    mProjShader         = gl::GlslProg( loadResource( PROJ_VERT ),	loadResource( PROJ_FRAG ) );
}

void Water_Refraction_Test::initFBOs()
{		
	gl::Fbo::Format format;
	//format.setDepthInternalFormat( GL_DEPTH_COMPONENT32 );
	format.setColorInternalFormat( GL_RGBA16F_ARB );
	format.setSamples( 4 ); // enable 4x antialiasing
	//init screen space render
	mScreenSpace1	= gl::Fbo( getWindowWidth(), getWindowHeight(), format );
	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );	
}

void Water_Refraction_Test::renderSceneToFBO()
{
	//render out main scene to FBO
	mScreenSpace1.bindFramebuffer();
	
	glClearColor( 0.5f, 0.5f, 0.5f, 1 );
	glClearDepth(1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	mEye = mCam->getEyePoint();
	mEye.normalize();
	mEye = mEye * abs(mCameraDistance);
	mCam->lookAt( mEye, mCenter, mUp );
	gl::setMatrices( *mCam );
	mLight->update( *mCam );
	
    if (mLightingOn)
	{
        glDisable( GL_LIGHTING );
		glPushMatrix();
		glScalef(1, -1, 1); //cam is updide down for light for some reason ...
		glColor3f( 1.0f, 1.0f, 0.1f );
        gl::drawFrustum( mLight->getShadowCamera() );
		glColor3f( 1.0f, 1.0f, 1.0f );
		glPopMatrix();
        glEnable( GL_LIGHTING );
	}
    
	drawTestObjects();
	
	mScreenSpace1.unbindFramebuffer();
	
	glDisable(GL_LIGHTING);
}

CINDER_APP_BASIC( Water_Refraction_Test, RendererGl )
