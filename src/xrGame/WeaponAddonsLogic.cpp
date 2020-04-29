#pragma once

#include "stdafx.h"
#include "Weapon.h"
#include "WeaponAddons.h"

void CWeapon::UpdateAddonsHudParams()
{

}


void CWeapon::UpdateAddonsTransform(bool for_hud)
{

}

WeaponAddon::WeaponAddon(LPCSTR section)
{
	addon_section	 =	section;
	addon_name		 =	READ_IF_EXISTS(pSettings, r_string, section, "addon_name", 0); //Если addon_name == NULL то аддон нельзя снять

	bone_target_name =	pSettings->r_string(section, "target_bone");
	target_slot		 =  pSettings->r_string(section, "slot_type");
	require			 =  READ_IF_EXISTS(pSettings, r_string, section, "require_addon", 0); // Если NULL то требований нет

	hud_visual		 =  READ_IF_EXISTS(pSettings, r_string, section, "hud_visual", 0);    // Если NULL то визуала нет, нужно для ванильных & stcop аддонов
	world_visual	 =  READ_IF_EXISTS(pSettings, r_string, section, "world_visual", 0);

	hasIcon			 =  READ_IF_EXISTS(pSettings, r_bool, section, "icon_enable", false);
	addon_icon[0]	 =  READ_IF_EXISTS(pSettings, r_u16, section, "icon_x", 0);
	addon_icon[1]	 =  READ_IF_EXISTS(pSettings, r_u16, section, "icon_y", 0);

	anm_prefix		 =  READ_IF_EXISTS(pSettings, r_string, section, "anm_prefix", 0); // Для использования кастомных анимаций, пока так пусть будет
	anm_suffix		 =  READ_IF_EXISTS(pSettings, r_string, section, "anm_suffix", 0);
}

WeaponAddon::~WeaponAddon()
{

}