#pragma once

class WeaponAddon
{
public:
			WeaponAddon();
	virtual ~WeaponAddon();
private:
	shared_str bone_target_name;
	Fmatrix bone_offset[2]; // hud,world
	u16 slot_index;
public:
	virtual void save(NET_Packet& output_packet) {};
	virtual void load(IReader& input_packet) {};

	void set_hud_offset(Fmatrix& matrix)
	{
		bone_offset[1] = matrix;
	}

	Fmatrix get_hud_offset()
	{
		return bone_offset[1];
	}

	void set_world_offset(Fmatrix& matrix)
	{
		bone_offset[2] = matrix;
	}

	Fmatrix get_world_offset()
	{
		return bone_offset[2];
	}
};

// Подствольник будет грузится по старой схеме
class WAddonScope : public WeaponAddon
{
public:
	u8 current_mark;
	bool bNVenabled; // Мейби сделать возможность настраивать яркость?
	virtual void save(NET_Packet& output_packet);
	virtual void load(IReader& input_packet);
};

class WAddonSilencer : public WeaponAddon
{
public:
	u16 durable;
};