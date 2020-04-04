#include "stdafx.h"
#include "Weapon.h"
#include "HUDManager.h"
#include "ui/UIWindow.h"
#include "ui/UIXmlInit.h"

extern CUIXml* pWpnScopeXml = NULL;

void createWpnScopeXML()
{
	if (!pWpnScopeXml)
	{
		pWpnScopeXml = xr_new<CUIXml>();
		pWpnScopeXml->Load(CONFIG_PATH, UI_PATH, "scopes.xml");
	}
}

void CWeapon::LoadCurrentScopeParams(LPCSTR section)
{
	shared_str scope_tex_name = "none";
	bScopeIsHasTexture = false;
	if (pSettings->line_exist(section, "scope_texture"))
	{
		scope_tex_name = pSettings->r_string(section, "scope_texture");
		if (xr_strcmp(scope_tex_name, "none") != 0)
			bScopeIsHasTexture = true;
	}

	Load3DScopeParams(section);

	m_zoom_params.m_fScopeZoomFactor = pSettings->r_float(section, "scope_zoom_factor");

	if (bScopeIsHasTexture || bIsSecondVPZoomPresent())
	{
		if (bIsSecondVPZoomPresent())
			bNVsecondVPavaible = !!pSettings->line_exist(section, "scope_nightvision");

		m_zoom_params.m_sUseZoomPostprocess = READ_IF_EXISTS(pSettings, r_string, section, "scope_nightvision", 0);
		m_zoom_params.m_bUseDynamicZoom = READ_IF_EXISTS(pSettings, r_bool, section, "scope_dynamic_zoom", FALSE);

		if (m_zoom_params.m_bUseDynamicZoom)
		{
			m_fZoomStepCount = READ_IF_EXISTS(pSettings, r_u8, section, "scope_zoom_steps", 3.0f);
			m_fZoomMinKoeff = READ_IF_EXISTS(pSettings, r_float, section, "min_zoom_k", 0.3f);
		}

		m_zoom_params.m_sUseBinocularVision = READ_IF_EXISTS(pSettings, r_string, section, "scope_alive_detector", 0);
	}
	else
	{
		bNVsecondVPavaible = false;
		bNVsecondVPstatus = false;
	}

	m_fScopeInertionFactor = READ_IF_EXISTS(pSettings, r_float, section, "scope_inertion_factor", m_fControlInertionFactor);

	m_fRTZoomFactor = m_zoom_params.m_fScopeZoomFactor;

	if (m_UIScope)
	{
		xr_delete(m_UIScope);
	}

	if (bScopeIsHasTexture)
	{
		m_UIScope = xr_new<CUIWindow>();
		createWpnScopeXML();
		CUIXmlInit::InitWindow(*pWpnScopeXml, scope_tex_name.c_str(), 0, m_UIScope);
	}
}

void CWeapon::Load3DScopeParams(LPCSTR section)
{
	m_zoom_params.m_fSecondVPFovFactor = READ_IF_EXISTS(pSettings, r_float, section, "3d_fov", 0.0f);
	m_zoom_params.m_f3dZoomFactor = READ_IF_EXISTS(pSettings, r_float, section, "3d_zoom_factor", 100.0f);
}

bool CWeapon::bReloadSectionScope(LPCSTR section)
{
	if (!pSettings->line_exist(section, "scopes"))
		return false;

	if (pSettings->r_string(section, "scopes") == NULL)
		return false;

	if (xr_strcmp(pSettings->r_string(section, "scopes"), "none") == 0)
		return false;

	return true;
}

bool CWeapon::bLoadAltScopesParams(LPCSTR section)
{
	if (!pSettings->line_exist(section, "scopes"))
		return false;

	if (pSettings->r_string(section, "scopes") == NULL)
		return false;

	if (xr_strcmp(pSettings->r_string(section, "scopes"), "none") == 0)
		return false;

	if (m_eScopeStatus == ALife::eAddonAttachable)
	{
		LPCSTR str = pSettings->r_string(section, "scopes");
		for (int i = 0, count = _GetItemCount(str); i < count; ++i)
		{
			string128 scope_section;
			_GetItem(str, i, scope_section);
			m_scopes.push_back(scope_section);
		}
	}
	else if (m_eScopeStatus == ALife::eAddonPermanent)
	{
		LoadCurrentScopeParams(section);
	}

	return true;
}

void CWeapon::LoadOriginalScopesParams(LPCSTR section)
{
	if (m_eScopeStatus == ALife::eAddonAttachable)
	{
		if (pSettings->line_exist(section, "scopes_sect"))
		{
			LPCSTR str = pSettings->r_string(section, "scopes_sect");
			for (int i = 0, count = _GetItemCount(str); i < count; ++i)
			{
				string128						scope_section;
				_GetItem(str, i, scope_section);
				m_scopes.push_back(scope_section);
			}
		}
		else
		{
			m_scopes.push_back(section);
		}
	}
	else if (m_eScopeStatus == ALife::eAddonPermanent)
	{
		LoadCurrentScopeParams(section);
	}
}

void CWeapon::UpdateAltScope()
{
	if (m_eScopeStatus != ALife::eAddonAttachable || !bUseAltScope)
		return;

	shared_str sectionNeedLoad;

	sectionNeedLoad = IsScopeAttached() ? GetNameWithAttachment() : m_section_id;

	if (!pSettings->section_exist(sectionNeedLoad))
		return;

	shared_str vis = pSettings->r_string(sectionNeedLoad, "visual");

	if (vis != cNameVisual())
	{
		cNameVisual_set(vis);
	}

	shared_str new_hud = pSettings->r_string(sectionNeedLoad, "hud");
	if (new_hud != hud_sect)
	{
		hud_sect = new_hud;
	}
}