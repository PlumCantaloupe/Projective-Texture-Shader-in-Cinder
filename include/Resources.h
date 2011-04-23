#pragma once
#include "cinder/CinderResources.h"

//shaders
#define BBlender_VERT	CINDER_RESOURCE( shaders/, BasicBlender_vert.glsl, 100, GLSL )
#define BBlender_FRAG	CINDER_RESOURCE( shaders/, BasicBlender_frag.glsl, 101, GLSL )

#define PROJ_VERT		CINDER_RESOURCE( shaders/, basicTexture_proj_vert.glsl, 102, GLSL )
#define PROJ_FRAG		CINDER_RESOURCE( shaders/, basicTexture_proj_frag.glsl, 103, GLSL )

//textures
#define GRASS_TEX_REP	CINDER_RESOURCE( textures/, grass.jpg, 104, IMAGE )
#define TROLL_TEX       CINDER_RESOURCE( textures/, trollface.jpg, 105, IMAGE )

