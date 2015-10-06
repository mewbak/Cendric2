#pragma once

enum class ItemType {
	VOID,
	Equipment_head,
	// ring 1 and 2 are technically the same as types but used to distinguish the equipped items.
	Equipment_ring_1,
	Equipment_ring_2,
	Equipment_weapon,
	Equipment_body,
	Equipment_neck,
	Equipment_back,
	Consumable,
	Quest,
	Document,
	Misc,
	// Gold items are converted to gold at pickup time (examples include cold coin, three gold coins, treasure...)
	Gold,
	MAX
};