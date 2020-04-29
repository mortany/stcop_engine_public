#pragma once


class WeaponAddon
{
public:
			WeaponAddon(LPCSTR section);
	virtual ~WeaponAddon();
	//---------------------------------------------------------------
	bool	   hasIcon;

	shared_str			  bone_target_name;
	shared_str			  addon_section;
	shared_str			  addon_name;

	shared_str			  target_slot;
	shared_str			  require;

	shared_str            hud_visual;
	shared_str            world_visual;

	shared_str            anm_prefix;
	shared_str            anm_suffix;

	// Support vanilla addons & maybe gunslinger
	xr_vector<shared_str> bones_need_enable;
	xr_vector<shared_str> bones_need_disable;
	xr_vector<shared_str> slots_add;

	Fvector bone_offset[2][2]; // hud,world/pos,rot

	// Перетащить это в прицелы, пусть отдельным типом аддонов будет
	Fvector aim_offset[2]; // pos,rot

	IKinematics*		  hud_model;
	IKinematics*		  world_model;

	u16 addon_icon[2]; //x,y
	//---------------------------------------------------------------

	virtual void save(NET_Packet& output_packet) {};
	virtual void load(IReader& input_packet) {};

	// Hud
	void set_h_offset_pos(Fvector& vector) { bone_offset[0][0] = vector; }
	void set_h_offset_rot(Fvector& vector) { bone_offset[0][1] = vector; }

	Fvector get_h_offset_pos() { return bone_offset[0][0]; }
	Fvector get_h_offset_rot() { return bone_offset[0][1]; }

	// World
	void set_w_offset_pos(Fvector& vector) { bone_offset[1][0] = vector; }
	void set_w_offset_rot(Fvector& vector) { bone_offset[1][1] = vector; }

	Fvector get_w_offset_pos() { return bone_offset[1][0]; }
	Fvector get_w_offset_rot() { return bone_offset[1][1]; }

};

class WAddonScope : public WeaponAddon
{
public:

	//u8 current_mark; not used at now
	//bool bNVenabled; // Мейби сделать возможность настраивать яркость?
	//virtual void save(NET_Packet& output_packet);
	//virtual void load(IReader& input_packet);
};

class WAddonSilencer : public WeaponAddon
{
public:
	//u16 durable; not used at now
	//virtual void save(NET_Packet& output_packet);
	//virtual void load(IReader& input_packet);
};

class WAddonLauncher : public WeaponAddon
{
public:
	xr_vector<shared_str> grenade_bones; // for vanilla support
};