#include "stdafx.h"
#include "Weapon.h"
#include "Actor.h"
#include "player_hud.h"
#include "Torch.h"
#include "Inventory.h"
#include "weaponBinocularsVision.h"

bool CWeapon::need_renderable()
{
	return Render->currentViewPort == MAIN_VIEWPORT && !(IsZoomed() && ZoomTexture() && !IsRotatingToZoom());
}

void CWeapon::renderable_Render()
{
	UpdateXForm();

	//нарисовать подсветку

	RenderLight();

	//если мы в режиме снайперки, то сам HUD рисовать не надо
	if (IsZoomed() && !IsRotatingToZoom() && ZoomTexture())
		RenderHud(FALSE);
	else
		RenderHud(TRUE);

	inherited::renderable_Render();
}

extern u32 hud_adj_mode;

void CWeapon::UpdateCL()
{
	inherited::UpdateCL();
	UpdateHUDAddonsVisibility();
	//подсветка от выстрела
	UpdateLight();

	//нарисовать партиклы
	UpdateFlameParticles();
	UpdateFlameParticles2();

	if ((GetNextState() == GetState()) && H_Parent() == Level().CurrentEntity())
	{
		CActor* pActor = smart_cast<CActor*>(H_Parent());
		if (pActor && !pActor->AnyMove() && this == pActor->inventory().ActiveItem())
		{
			if (hud_adj_mode == 0 &&
				GetState() == eIdle &&
				(Device.dwTimeGlobal - m_dw_curr_substate_time > 20000) &&
				!IsZoomed() &&
				g_player_hud->attached_item(1) == NULL)
			{
				if (AllowBore())
					SwitchState(eBore);

				ResetSubStateTime();
			}
		}
	}

	if (m_zoom_params.m_pNight_vision && !need_renderable())
	{
		if (!m_zoom_params.m_pNight_vision->IsActive())
		{
			CActor* pA = smart_cast<CActor*>(H_Parent());
			R_ASSERT(pA);
			CTorch* pTorch = smart_cast<CTorch*>(pA->inventory().ItemFromSlot(TORCH_SLOT));
			if (pTorch && pTorch->GetNightVisionStatus())
			{
				m_bRememberActorNVisnStatus = pTorch->GetNightVisionStatus();
				pTorch->SwitchNightVision(false, false);
			}
			m_zoom_params.m_pNight_vision->Start(m_zoom_params.m_sUseZoomPostprocess, pA, false);
		}

	}
	else if (m_bRememberActorNVisnStatus)
	{
		m_bRememberActorNVisnStatus = false;
		EnableActorNVisnAfterZoom();
	}

	if (m_zoom_params.m_pVision)
		m_zoom_params.m_pVision->Update();
}

bool CWeapon::render_item_ui_query()
{
	bool b_is_active_item = (m_pInventory->ActiveItem() == this);
	bool res = b_is_active_item && IsZoomed() && ZoomHideCrosshair() && ZoomTexture() && !IsRotatingToZoom();
	return res;
}

void CWeapon::render_item_ui()
{
	if (m_zoom_params.m_pVision)
		m_zoom_params.m_pVision->Draw();

	ZoomTexture()->Update();
	ZoomTexture()->Draw();
}

void CWeapon::ForceUpdateFireParticles()
{
	if (!GetHUDmode())
	{//update particlesXFORM real bullet direction

		if (!H_Parent())		return;

		Fvector					p, d;
		smart_cast<CEntity*>(H_Parent())->g_fireParams(this, p, d);

		Fmatrix						_pxf;
		_pxf.k = d;
		_pxf.i.crossproduct(Fvector().set(0.0f, 1.0f, 0.0f), _pxf.k);
		_pxf.j.crossproduct(_pxf.k, _pxf.i);
		_pxf.c = XFORM().c;

		m_current_firedeps.m_FireParticlesXForm.set(_pxf);
	}
}

void CWeapon::UpdateFireDependencies_internal()
{
	if (Device.dwFrame != dwFP_Frame)
	{
		dwFP_Frame = Device.dwFrame;

		UpdateXForm();

		if (GetHUDmode())
		{
			HudItemData()->setup_firedeps(m_current_firedeps);
			VERIFY(_valid(m_current_firedeps.m_FireParticlesXForm));
		}
		else
		{
			// 3rd person or no parent
			Fmatrix& parent = XFORM();
			Fvector& fp = vLoadedFirePoint;
			Fvector& fp2 = vLoadedFirePoint2;
			Fvector& sp = vLoadedShellPoint;

			parent.transform_tiny(m_current_firedeps.vLastFP, fp);
			parent.transform_tiny(m_current_firedeps.vLastFP2, fp2);
			parent.transform_tiny(m_current_firedeps.vLastSP, sp);

			m_current_firedeps.vLastFD.set(0.f, 0.f, 1.f);
			parent.transform_dir(m_current_firedeps.vLastFD);

			m_current_firedeps.m_FireParticlesXForm.set(parent);
			VERIFY(_valid(m_current_firedeps.m_FireParticlesXForm));
		}
	}
}

void CWeapon::UpdateXForm()
{
	if (Device.dwFrame == dwXF_Frame)
		return;

	dwXF_Frame = Device.dwFrame;

	if (!H_Parent())
		return;

	// Get access to entity and its visual
	CEntityAlive* E = smart_cast<CEntityAlive*>(H_Parent());

	if (!E) {
		if (!IsGameTypeSingle())
			UpdatePosition(H_Parent()->XFORM());

		return;
	}

	const CInventoryOwner* parent = smart_cast<const CInventoryOwner*>(E);
	if (parent && parent->use_simplified_visual())
		return;

	if (parent->attached(this))
		return;

	IKinematics* V = smart_cast<IKinematics*>	(E->Visual());
	VERIFY(V);

	// Get matrices
	int						boneL = -1, boneR = -1, boneR2 = -1;

	// this ugly case is possible in case of a CustomMonster, not a Stalker, nor an Actor
	E->g_WeaponBones(boneL, boneR, boneR2);

	if (boneR == -1)		return;

	if ((HandDependence() == hd1Hand) || (GetState() == eReload) || (!E->g_Alive()))
		boneL = boneR2;

	V->CalculateBones();
	Fmatrix& mL = V->LL_GetTransform(u16(boneL));
	Fmatrix& mR = V->LL_GetTransform(u16(boneR));
	// Calculate
	Fmatrix					mRes;
	Fvector					R, D, N;
	D.sub(mL.c, mR.c);

	if (fis_zero(D.magnitude())) {
		mRes.set(E->XFORM());
		mRes.c.set(mR.c);
	}
	else {
		D.normalize();
		R.crossproduct(mR.j, D);

		N.crossproduct(D, R);
		N.normalize();

		mRes.set(R, N, D, mR.c);
		mRes.mulA_43(E->XFORM());
	}

	UpdatePosition(mRes);
}
