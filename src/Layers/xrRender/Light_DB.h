#pragma once

#include "light.h"
#include "light_package.h"

struct LDBViewPortBuffers
{
	light_Package			rawPackage_;
	light_Package			rawPackageDeffered_;
};

class	CLight_DB
{
private:
	xr_vector<ref_light>	v_static;
	xr_vector<ref_light>	v_hemi;
public:
	ref_light				sun_original;
	ref_light				sun_adapted;

	LDBViewPortBuffers		ldbViewPortBuffer1;
	LDBViewPortBuffers		ldbViewPortBuffer2;
	LDBViewPortBuffers*		ldbTargetViewPortBuffer;

public:
	void					add_light			(light*		L	);

	void					Load				(IReader*	fs	);
#if RENDER != R_R1
	void					LoadHemi			();
#endif
	void					Unload				();

	light*					Create				();
	void					Update				();

	CLight_DB				();
	~CLight_DB				();
};
