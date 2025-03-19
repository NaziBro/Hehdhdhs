#include <stdio.h>
#include <jni.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stddef.h>
#include <dlfcn.h>
#include <dirent.h>
#include <stdlib.h>
#include <memory>
#include <string>
#include "Destroy.h"
#include "MCPE/GameData.h"
#include "HackSDK.h"


Destroy::Destroy()
{
	ModuleType = "Item";

	UIType = 2;
	MenuCall = ([=]() {
		mAndroid->Toast("添加冒险破坏NBT成功");
		moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
			addJson();
		});
	});
}

const char* Destroy::GetName()
{
	return "Destroy";
}

void Destroy::addJson(void)
{
	ItemStack* item = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSelectedItem();

	if (item->getId() == 0)return;
	ItemStack old_data(*item);

	std::string idk;
	item->addComponents(*value, idk);

	uint32_t slot = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSelectedItemSlot();

	moduleManager->getModule<HackSDK>()->transferItem(old_data, *item, slot);
}

void Destroy::MinecraftInit()
{
	std::string data = "{\"minecraft:can_destroy\":{\"blocks\":[\"beehive\",\"iron_ore\",\"dirt\",\"observer\",\"cobblestone\",\"leaves\",\"planks\",\"log\",\"gravel\",\"wood\",\"grass\",\"stone\",\"glass\",\"sand\",\"snow\",\"ladder\",\"anvil\",\"slime\",\"turtle_egg\",\"bamboo\",\"bamboo_sapling\",\"lantern\",\"sweet_berry_bush\",\"cactus\",\"fence\",\"torch\",\"fire\",\"water\",\"rail\",\"lever\",\"vine\",\"fence_gate\",\"chest\",\"brewing_stand\",\"cocoa\",\"cobblestone_wall\",\"double_plant\",\"flower_pot\",\"kelp\",\"coral_fan\",\"trapdoor\",\"tripwire_hook\",\"cauldron\",\"hopper\",\"piston\",\"beacon\",\"chorus_plant\",\"chorus_flower\",\"dragon_egg\",\"end_portal\",\"end_rod\",\"skull\",\"frame\",\"shulker_box\",\"seagrass\",\"sea_pickle\",\"conduit\",\"bubble_column\",\"barrier\",\"grindstone\",\"bell\",\"campfire\",\"lectern\",\"cartography_table\",\"stonecutter_block\",\"sponge\",\"lapis_ore\",\"lapis_block\",\"dispenser\",\"sandstone\",\"noteblock\",\"golden_rail\",\"detector_rail\",\"sticky_piston\",\"tallgrass\",\"deadbush\",\"pistonarmCollision\",\"wool\",\"bed\",\"bedrock\",\"yellow_flower\",\"red_flower\",\"brown_mushroom\",\"red_mushroom\",\"gold_block\",\"iron_block\",\"double_stone_slab\",\"stone_slab\",\"brick_block\",\"bookshelf\",\"mossy_cobblestone\",\"obsidian\",\"mob_spawner\",\"oak_stairs\",\"redstone_wire\",\"diamond_ore\",\"diamond_block\",\"crafting_table\",\"wheat\",\"farmland\",\"furnace\",\"lit_furnace\",\"wooden_door\",\"stone_stairs\",\"standing_sign\",\"wall_sign\",\"spruce_standing_sign\",\"spruce_wall_sign\",\"birch_standing_sign\",\"birch_wall_sign\",\"jungle_standing_sign\",\"jungle_wall_sign\",\"acacia_standing_sign\",\"acacia_wall_sign\",\"darkoak_standing_sign\",\"darkoak_wall_sign\",\"stone_pressure_plate\",\"iron_door\",\"wooden_pressure_plate\",\"acacia_pressure_plate\",\"birch_pressure_plate\",\"dark_oak_pressure_plate\",\"jungle_pressure_plate\",\"spruce_pressure_plate\",\"redstone_ore\",\"lit_redstone_ore\",\"unlit_redstone_torch\",\"redstone_torch\",\"stone_button\",\"snow_layer\",\"blue_ice\",\"clay\",\"reeds\",\"dried_kelp_block\",\"nether_brick_fence\",\"pumpkin\",\"carved_pumpkin\",\"netherrack\",\"magma\",\"soul_sand\",\"glowstone\",\"portal\",\"lit_pumpkin\",\"cake\",\"unpowered_repeater\",\"powered_repeater\",\"acacia_trapdoor\",\"birch_trapdoor\",\"dark_oak_trapdoor\",\"jungle_trapdoor\",\"spruce_trapdoor\",\"monster_egg\",\"stonebrick\",\"brown_mushroom_block\",\"red_mushroom_block\",\"iron_bars\",\"glass_pane\",\"melon_block\",\"pumpkin_stem\",\"melon_stem\",\"brick_stairs\",\"mycelium\",\"waterlily\",\"lava_cauldron\",\"end_portal_frame\",\"end_bricks\",\"end_stone\",\"redstone_lamp\",\"lit_redstone_lamp\",\"emerald_ore\",\"emerald_block\",\"spruce_stairs\",\"birch_stairs\",\"jungle_stairs\",\"undyed_shulker_box\",\"wooden_button\",\"acacia_button\",\"birch_button\",\"dark_oak_button\",\"jungle_button\",\"spruce_button\",\"stone_brick_stairs\",\"nether_brick\",\"nether_brick_stairs\",\"red_nether_brick\",\"nether_wart\",\"nether_wart_block\",\"enchanting_table\",\"dropper\",\"command_block\",\"chain_command_block\",\"repeating_command_block\",\"activator_rail\",\"sandstone_stairs\",\"tripwire\",\"carrots\",\"potatoes\",\"trapped_chest\",\"light_weighted_pressure_plate\",\"heavy_weighted_pressure_plate\",\"unpowered_comparator\",\"powered_comparator\",\"daylight_detector\",\"redstone_block\",\"quartz_ore\",\"bone_block\",\"quartz_block\",\"quartz_stairs\",\"purpur_block\",\"purpur_stairs\",\"double_wooden_slab\",\"wooden_slab\",\"stained_hardened_clay\",\"leaves2\",\"log2\",\"acacia_stairs\",\"dark_oak_stairs\",\"iron_trapdoor\",\"hay_block\",\"carpet\",\"hardened_clay\",\"coal_block\",\"packed_ice\",\"standing_banner\",\"wall_banner\",\"daylight_detector_inverted\",\"red_sandstone\",\"red_sandstone_stairs\",\"double_stone_slab2\",\"stone_slab2\",\"double_stone_slab3\",\"stone_slab3\",\"double_stone_slab4\",\"stone_slab4\",\"coral\",\"coral_block\",\"coral_fan_dead\",\"coral_fan_hang\",\"coral_fan_hang2\",\"coral_fan_hang3\",\"spruce_fence_gate\",\"birch_fence_gate\",\"jungle_fence_gate\",\"dark_oak_fence_gate\",\"acacia_fence_gate\",\"spruce_door\",\"birch_door\",\"jungle_door\",\"acacia_door\",\"dark_oak_door\",\"grass_path\",\"prismarine\",\"sealantern\",\"structure_block\",\"white_glazed_terracotta\",\"orange_glazed_terracotta\",\"magenta_glazed_terracotta\",\"light_blue_glazed_terracotta\",\"yellow_glazed_terracotta\",\"lime_glazed_terracotta\",\"pink_glazed_terracotta\",\"gray_glazed_terracotta\",\"silver_glazed_terracotta\",\"cyan_glazed_terracotta\",\"purple_glazed_terracotta\",\"blue_glazed_terracotta\",\"brown_glazed_terracotta\",\"green_glazed_terracotta\",\"red_glazed_terracotta\",\"black_glazed_terracotta\",\"concrete\",\"concretepowder\",\"podzol\",\"beetroot\",\"stained_glass\",\"stained_glass_pane\",\"stripped_spruce_log\",\"stripped_birch_log\",\"stripped_jungle_log\",\"stripped_acacia_log\",\"stripped_dark_oak_log\",\"stripped_oak_log\",\"prismarine_stairs\",\"dark_prismarine_stairs\",\"prismarine_bricks_stairs\",\"granite_stairs\",\"diorite_stairs\",\"andesite_stairs\",\"polished_granite_stairs\",\"polished_diorite_stairs\",\"polished_andesite_stairs\",\"mossy_stone_brick_stairs\",\"smooth_red_sandstone_stairs\",\"smooth_sandstone_stairs\",\"end_brick_stairs\",\"mossy_cobblestone_stairs\",\"normal_stone_stairs\",\"red_nether_brick_stairs\",\"smooth_quartz_stairs\",\"smooth_stone\",\"fletching_table\",\"blast_furnace\",\"lit_blast_furnace\",\"smoker\",\"lit_smoker\",\"smithing_table\",\"barrel\",\"loom\",\"composter\",\"light_block\",\"bee_nest\",\"stickypistonarmcollision\",\"mod_ore\",\"gold_ore\"]},\"minecraft:can_place_on\":{\"blocks\":[\"beehive\",\"iron_ore\",\"dirt\",\"observer\",\"cobblestone\",\"leaves\",\"planks\",\"log\",\"gravel\",\"wood\",\"grass\",\"stone\",\"glass\",\"sand\",\"snow\",\"ladder\",\"anvil\",\"slime\",\"turtle_egg\",\"bamboo\",\"bamboo_sapling\",\"lantern\",\"sweet_berry_bush\",\"cactus\",\"fence\",\"torch\",\"fire\",\"water\",\"rail\",\"lever\",\"vine\",\"fence_gate\",\"chest\",\"brewing_stand\",\"cocoa\",\"cobblestone_wall\",\"double_plant\",\"flower_pot\",\"kelp\",\"coral_fan\",\"trapdoor\",\"tripwire_hook\",\"cauldron\",\"hopper\",\"piston\",\"beacon\",\"chorus_plant\",\"chorus_flower\",\"dragon_egg\",\"end_portal\",\"end_rod\",\"skull\",\"frame\",\"shulker_box\",\"seagrass\",\"sea_pickle\",\"conduit\",\"bubble_column\",\"barrier\",\"grindstone\",\"bell\",\"campfire\",\"lectern\",\"cartography_table\",\"stonecutter_block\",\"sponge\",\"lapis_ore\",\"lapis_block\",\"dispenser\",\"sandstone\",\"noteblock\",\"golden_rail\",\"detector_rail\",\"sticky_piston\",\"tallgrass\",\"deadbush\",\"pistonarmCollision\",\"wool\",\"bed\",\"bedrock\",\"yellow_flower\",\"red_flower\",\"brown_mushroom\",\"red_mushroom\",\"gold_block\",\"iron_block\",\"double_stone_slab\",\"stone_slab\",\"brick_block\",\"bookshelf\",\"mossy_cobblestone\",\"obsidian\",\"mob_spawner\",\"oak_stairs\",\"redstone_wire\",\"diamond_ore\",\"diamond_block\",\"crafting_table\",\"wheat\",\"farmland\",\"furnace\",\"lit_furnace\",\"wooden_door\",\"stone_stairs\",\"standing_sign\",\"wall_sign\",\"spruce_standing_sign\",\"spruce_wall_sign\",\"birch_standing_sign\",\"birch_wall_sign\",\"jungle_standing_sign\",\"jungle_wall_sign\",\"acacia_standing_sign\",\"acacia_wall_sign\",\"darkoak_standing_sign\",\"darkoak_wall_sign\",\"stone_pressure_plate\",\"iron_door\",\"wooden_pressure_plate\",\"acacia_pressure_plate\",\"birch_pressure_plate\",\"dark_oak_pressure_plate\",\"jungle_pressure_plate\",\"spruce_pressure_plate\",\"redstone_ore\",\"lit_redstone_ore\",\"unlit_redstone_torch\",\"redstone_torch\",\"stone_button\",\"snow_layer\",\"blue_ice\",\"clay\",\"reeds\",\"dried_kelp_block\",\"nether_brick_fence\",\"pumpkin\",\"carved_pumpkin\",\"netherrack\",\"magma\",\"soul_sand\",\"glowstone\",\"portal\",\"lit_pumpkin\",\"cake\",\"unpowered_repeater\",\"powered_repeater\",\"acacia_trapdoor\",\"birch_trapdoor\",\"dark_oak_trapdoor\",\"jungle_trapdoor\",\"spruce_trapdoor\",\"monster_egg\",\"stonebrick\",\"brown_mushroom_block\",\"red_mushroom_block\",\"iron_bars\",\"glass_pane\",\"melon_block\",\"pumpkin_stem\",\"melon_stem\",\"brick_stairs\",\"mycelium\",\"waterlily\",\"lava_cauldron\",\"end_portal_frame\",\"end_bricks\",\"end_stone\",\"redstone_lamp\",\"lit_redstone_lamp\",\"emerald_ore\",\"emerald_block\",\"spruce_stairs\",\"birch_stairs\",\"jungle_stairs\",\"undyed_shulker_box\",\"wooden_button\",\"acacia_button\",\"birch_button\",\"dark_oak_button\",\"jungle_button\",\"spruce_button\",\"stone_brick_stairs\",\"nether_brick\",\"nether_brick_stairs\",\"red_nether_brick\",\"nether_wart\",\"nether_wart_block\",\"enchanting_table\",\"dropper\",\"command_block\",\"chain_command_block\",\"repeating_command_block\",\"activator_rail\",\"sandstone_stairs\",\"tripwire\",\"carrots\",\"potatoes\",\"trapped_chest\",\"light_weighted_pressure_plate\",\"heavy_weighted_pressure_plate\",\"unpowered_comparator\",\"powered_comparator\",\"daylight_detector\",\"redstone_block\",\"quartz_ore\",\"bone_block\",\"quartz_block\",\"quartz_stairs\",\"purpur_block\",\"purpur_stairs\",\"double_wooden_slab\",\"wooden_slab\",\"stained_hardened_clay\",\"leaves2\",\"log2\",\"acacia_stairs\",\"dark_oak_stairs\",\"iron_trapdoor\",\"hay_block\",\"carpet\",\"hardened_clay\",\"coal_block\",\"packed_ice\",\"standing_banner\",\"wall_banner\",\"daylight_detector_inverted\",\"red_sandstone\",\"red_sandstone_stairs\",\"double_stone_slab2\",\"stone_slab2\",\"double_stone_slab3\",\"stone_slab3\",\"double_stone_slab4\",\"stone_slab4\",\"coral\",\"coral_block\",\"coral_fan_dead\",\"coral_fan_hang\",\"coral_fan_hang2\",\"coral_fan_hang3\",\"spruce_fence_gate\",\"birch_fence_gate\",\"jungle_fence_gate\",\"dark_oak_fence_gate\",\"acacia_fence_gate\",\"spruce_door\",\"birch_door\",\"jungle_door\",\"acacia_door\",\"dark_oak_door\",\"grass_path\",\"prismarine\",\"sealantern\",\"structure_block\",\"white_glazed_terracotta\",\"orange_glazed_terracotta\",\"magenta_glazed_terracotta\",\"light_blue_glazed_terracotta\",\"yellow_glazed_terracotta\",\"lime_glazed_terracotta\",\"pink_glazed_terracotta\",\"gray_glazed_terracotta\",\"silver_glazed_terracotta\",\"cyan_glazed_terracotta\",\"purple_glazed_terracotta\",\"blue_glazed_terracotta\",\"brown_glazed_terracotta\",\"green_glazed_terracotta\",\"red_glazed_terracotta\",\"black_glazed_terracotta\",\"concrete\",\"concretepowder\",\"podzol\",\"beetroot\",\"stained_glass\",\"stained_glass_pane\",\"stripped_spruce_log\",\"stripped_birch_log\",\"stripped_jungle_log\",\"stripped_acacia_log\",\"stripped_dark_oak_log\",\"stripped_oak_log\",\"prismarine_stairs\",\"dark_prismarine_stairs\",\"prismarine_bricks_stairs\",\"granite_stairs\",\"diorite_stairs\",\"andesite_stairs\",\"polished_granite_stairs\",\"polished_diorite_stairs\",\"polished_andesite_stairs\",\"mossy_stone_brick_stairs\",\"smooth_red_sandstone_stairs\",\"smooth_sandstone_stairs\",\"end_brick_stairs\",\"mossy_cobblestone_stairs\",\"normal_stone_stairs\",\"red_nether_brick_stairs\",\"smooth_quartz_stairs\",\"smooth_stone\",\"fletching_table\",\"blast_furnace\",\"lit_blast_furnace\",\"smoker\",\"lit_smoker\",\"smithing_table\",\"barrel\",\"loom\",\"composter\",\"light_block\",\"bee_nest\",\"stickypistonarmcollision\",\"mod_ore\",\"gold_ore\"]}}";
	MCJson::Reader reader;
	value = new MCJson::Value(0);
	reader.parse(data, *value, false);
}

void Destroy::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".Destroy") {
		moduleManager->executedCMD = true;
		moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
			addJson();
		});
	}
}
