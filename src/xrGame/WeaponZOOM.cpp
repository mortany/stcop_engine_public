#include "stdafx.h"
#include "actor.h"
#include "Weapon.h"
#include "inventory.h"
#include <GamePersistent.h>
#include "Torch.h"
#include "WeaponBinocularsVision.h"
#include <HUDManager.h>

void CWeapon::ZoomDynamicMod(bool bIncrement, bool bForceLimit)
{
	if (!IsScopeAttached())
		return;

	if (!m_zoom_params.m_bUseDynamicZoom)
		return;

	float delta, min_zoom_factor, max_zoom_factor;

	bool bIsSecondZOOM = bIsSecondVPZoomPresent() && psActorFlags.test(AF_3DSCOPE_ENABLE);

	max_zoom_factor = (bIsSecondZOOM ? GetSecondVPZoomFactor() * 100.0f : m_zoom_params.m_fScopeZoomFactor);

	GetZoomData(max_zoom_factor, delta, min_zoom_factor);

	if (bForceLimit)
	{
		if (bIsSecondZOOM)
			m_fSecondRTZoomFactor = (bIncrement ? max_zoom_factor : min_zoom_factor);
		else
			m_fRTZoomFactor = (bIncrement ? max_zoom_factor : min_zoom_factor);
	}
	else
	{
		float f = (bIsSecondZOOM ? m_fSecondRTZoomFactor : GetZoomFactor());

		f -= delta * (bIncrement ? 1.f : -1.f);

		clamp(f, max_zoom_factor, min_zoom_factor);

		if (bIsSecondZOOM)
			m_fSecondRTZoomFactor = f;
		else
			SetZoomFactor(f);
	}
}

void CWeapon::ZoomInc()
{
	ZoomDynamicMod(true, false);
}

void CWeapon::ZoomDec()
{
	ZoomDynamicMod(false, false);
}

float CWeapon::CurrentZoomFactor()
{
	if (psActorFlags.test(AF_3DSCOPE_ENABLE) && IsScopeAttached())
	{
		return bIsSecondVPZoomPresent() ? m_zoom_params.m_f3dZoomFactor : m_zoom_params.m_fScopeZoomFactor;
	}
	else
	{
		return IsScopeAttached() ? m_zoom_params.m_fScopeZoomFactor : m_zoom_params.m_fIronSightZoomFactor;
	}
};

float CWeapon::GetControlInertionFactor() const
{
	float fInertionFactor = inherited::GetControlInertionFactor();
	if (IsScopeAttached() && IsZoomed())
	{
		if (bIsSecondVPZoomPresent() && psActorFlags.test(AF_3DSCOPE_ENABLE))
		{
			if (GetSecondVPFov() < 15.f)
			{
				return 9.5f;
			}
			else if (GetSecondVPFov() < 20.f)
			{
				return 7.5f;
			}
			else if (GetSecondVPFov() < 35.f)
			{
				return 5.5f;
			}
			else if (GetSecondVPFov() < 55.f)
			{
				return 2.5f;
			}
			else
			{
				return m_fScopeInertionFactor;
			}
		}
		else
		{
			return m_fScopeInertionFactor;//m_fScopeInertionFactor;
		}
	}
	return fInertionFactor;
}

void CWeapon::GetZoomData(const float scope_factor, float& delta, float& min_zoom_factor)
{
	float def_fov = bIsSecondVPZoomPresent() ? 75.0f : g_fov;//float(g_fov);
	float delta_factor_total = def_fov - scope_factor;
	VERIFY(delta_factor_total > 0);
	min_zoom_factor = def_fov - delta_factor_total * m_fZoomMinKoeff;
	delta = (delta_factor_total * (1 - m_fZoomMinKoeff)) / m_fZoomStepCount;
}

void CWeapon::OnZoomOut()
{
	if (!bIsSecondVPZoomPresent() && !psActorFlags.test(AF_3DSCOPE_ENABLE))
		m_fRTZoomFactor = GetZoomFactor(); // Сохраняем текущий динамический зум
	m_zoom_params.m_bIsZoomModeNow = false;
	SetZoomFactor(g_fov);
	psActorFlags.set(AF_ZOOM_NEW_FD, false);

	if (GetHUDmode())
		GamePersistent().SetPickableEffectorDOF(false);

	ResetSubStateTime();

	xr_delete(m_zoom_params.m_pVision);
	if (m_zoom_params.m_pNight_vision)
	{
		m_zoom_params.m_pNight_vision->Stop(100000.0f, false);
		xr_delete(m_zoom_params.m_pNight_vision);
	}
}

void CWeapon::OnZoomIn()
{
	m_zoom_params.m_bIsZoomModeNow = true;
	psActorFlags.set(AF_ZOOM_NEW_FD, true);

	if (m_fSecondRTZoomFactor == -1)
		ZoomDynamicMod(true, true);

	if (!m_zoom_params.m_bUseDynamicZoom)
		SetZoomFactor(CurrentZoomFactor());
	else SetZoomFactor(psActorFlags.test(AF_3DSCOPE_ENABLE) ? m_zoom_params.m_f3dZoomFactor : m_fRTZoomFactor);

	if (GetHUDmode())
		GamePersistent().SetPickableEffectorDOF(true);

	if (m_zoom_params.m_sUseBinocularVision.size() && IsScopeAttached() && NULL == m_zoom_params.m_pVision)
		m_zoom_params.m_pVision = xr_new<CBinocularsVision>(m_zoom_params.m_sUseBinocularVision);

	CActor* pA = smart_cast<CActor*>(H_Parent());

	if (pA && IsScopeAttached())
	{
		if (psActorFlags.test(AF_PNV_W_SCOPE_DIS) && UseScopeTexture())
		{
			CTorch* pTorch = smart_cast<CTorch*>(pA->inventory().ItemFromSlot(TORCH_SLOT));
			if (pTorch && pTorch->GetNightVisionStatus())
			{
				OnZoomOut();
			}
		}
		else if (m_zoom_params.m_sUseZoomPostprocess.size() && !psActorFlags.test(AF_3DSCOPE_ENABLE))
		{
			if (NULL == m_zoom_params.m_pNight_vision)
			{
				m_zoom_params.m_pNight_vision = xr_new<CNightVisionEffector>(m_zoom_params.m_sUseZoomPostprocess/*"device_torch"*/);
			}
		}
	}
}

void CWeapon::EnableActorNVisnAfterZoom()
{
	CActor* pA = smart_cast<CActor*>(H_Parent());
	if (IsGameTypeSingle() && !pA)
		pA = g_actor;

	if (pA)
	{
		CTorch* pTorch = smart_cast<CTorch*>(pA->inventory().ItemFromSlot(TORCH_SLOT));
		if (pTorch)
		{
			pTorch->SwitchNightVision(true, false);
			pTorch->GetNightVision()->PlaySounds(CNightVisionEffector::eIdleSound);
		}
	}
}

CUIWindow* CWeapon::ZoomTexture()
{
	if (UseScopeTexture() && (!psActorFlags.test(AF_3DSCOPE_ENABLE) || !bIsSecondVPZoomPresent()))
		return m_UIScope;
	else
		return NULL;
}

ENGINE_API extern float psHUD_FOV_def;

// Получить HUD FOV текущего оружия
float CWeapon::GetHudFov()
{
	// Рассчитываем HUD FOV от бедра (с учётом упирания в стены)
	if (ParentIsActor() && Level().CurrentViewEntity() == H_Parent())
	{
		// Получаем расстояние от камеры до точки в прицеле
		collide::rq_result& RQ = HUD().GetCurrentRayQuery();
		float dist = RQ.range;

		// Интерполируем расстояние в диапазон от 0 (min) до 1 (max)
		clamp(dist, m_nearwall_dist_min, m_nearwall_dist_max);
		float fDistanceMod =
			((dist - m_nearwall_dist_min) / (m_nearwall_dist_max - m_nearwall_dist_min)); // 0.f ... 1.f

		// Рассчитываем базовый HUD FOV от бедра
		float fBaseFov = psHUD_FOV_def + m_hud_fov_add_mod;
		clamp(fBaseFov, 0.0f, FLT_MAX);

		// Плавно высчитываем итоговый FOV от бедра
		float src = m_nearwall_speed_mod * Device.fTimeDelta;
		clamp(src, 0.f, 1.f);

		float fTrgFov = m_nearwall_target_hud_fov + fDistanceMod * (fBaseFov - m_nearwall_target_hud_fov);
		m_nearwall_last_hud_fov = m_nearwall_last_hud_fov * (1 - src) + fTrgFov * src;
	}
	return m_nearwall_last_hud_fov;
}

float CWeapon::GetSecondVPZoomFactor() const
{
	float dist_k = (100.f / m_zoom_params.m_f3dZoomFactor);

	clamp(dist_k, 0.0f, 1.0f);

	float result = (m_zoom_params.m_fSecondVPFovFactor / dist_k);

	return result;
}

// Получить FOV от текущего оружия игрока для второго рендера

float CWeapon::GetSecondVPFov() const
{
	if (m_zoom_params.m_bUseDynamicZoom && bIsSecondVPZoomPresent())
		return (m_fSecondRTZoomFactor / 100.f) * 75.f;//g_fov; 75.f

	return GetSecondVPZoomFactor() * 75.f;//g_fov; 75.f
}

// Обновление необходимости включения второго вьюпорта +SecondVP+
// Вызывается только для активного оружия игрока
void CWeapon::UpdateSecondVP(bool bInGrenade)
{
	bool b_is_active_item = (m_pInventory != NULL) && (m_pInventory->ActiveItem() == this);
	R_ASSERT(ParentIsActor() && b_is_active_item); // Эта функция должна вызываться только для оружия в руках нашего игрока

	CActor* pActor = smart_cast<CActor*>(H_Parent());

	bool bCond_1 = bInZoomRightNow();													// Мы должны целиться

	bool bCond_2 = bIsSecondVPZoomPresent() && psActorFlags.test(AF_3DSCOPE_ENABLE);	// В конфиге должен быть прописан фактор зума для линзы (scope_lense_factor
																						// больше чем 0)
	bool bCond_3 = pActor->cam_Active() == pActor->cam_FirstEye();						// Мы должны быть от 1-го лица	

	Device.m_SecondViewport.SetSVPActive(bCond_1 && bCond_2 && bCond_3 && !bInGrenade);
}