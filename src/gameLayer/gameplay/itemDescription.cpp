#include <gameplay/items.h>


const char *itemsDescriptions[] =
{
	"Sturdy. Surprisingly versatile.",                             // stick
	"Soft. Could be a bandage... or fashion.",                     // cloth
	"Sharp. Someone lost a tooth.",                                // fang
	"Brittle. Still has some fight left in it.",                   // bone

	"Shiny and slightly warm.",                                    // copperIngot
	"Heavy. Don’t lick it.",                                       // leadIngot
	"Reliable. For serious crafting.",                             // ironIngot
	"Classy and conductive.",                                      // silverIngot
	"Fancy and overpriced.",                                       // goldIngot

	"Gets the job done.",                                          // copper pickaxe
	"It chops.",                                                   // copper axe
	"Dirt beware.",                                                // copper shovel
	"Heavy hitter.",                                               // lead pickaxe
	"Trees tremble.",                                              // lead axe
	"Might dig more than holes.",                                  // lead shovel
	"Classic miner’s choice.",                                     // iron pickaxe
	"Cuts like a dream.",                                          // iron axe
	"Efficient and stylish.",                                      // iron shovel
	"Almost too pretty to use.",                                   // silver pickaxe
	"Sparkles while chopping.",                                    // silver axe
	"Dig with elegance.",                                          // silver shovel
	"Digs fast, breaks faster.",                                   // gold pickaxe
	"Swing with style.",                                           // gold axe
	"For luxurious holes.",                                        // gold shovel

	"Won’t impress goblins.",                                      // copper sword
	"Packs a punch.",                                              // lead sword
	"Trusty and true.",                                            // iron sword
	"Elegant and sharp.",                                          // silver sword
	"Flashy, but flimsy.",                                         // gold sword

	"For safe training (probably).",                               // trainingScythe
	"For practice, not pride.",                                    // trainingSword
	"Massive but harmless.",                                       // trainingWarHammer
	"Pointy, but padded.",                                         // trainingSpear
	"Won’t cut deep—hopefully.",                                   // trainingKnife
	"All bark, no bite.",                                          // trainingBattleAxe

	"" ,														//"copperWarHammer" ,
	"" ,															//"copperSpear" ,
	"" ,															//"copperKnife" ,
	"" ,														//"copperBattleAxe" ,
	"" ,														//"leadWarHammer" ,
	"" ,															//"leadSpear" ,
	"" ,															//"leadKnife" ,
	"" ,														//"leadBattleAxe" ,
	"" ,														//"silverWarHammer" ,
	"" ,															//"silverSpear" ,
	"" ,															//"silverKnife" ,
	"" ,														//"silverBattleAxe" ,
	"" ,														//"goldWarHammer" ,
	"" ,															//"goldSpear" ,
	"" ,															//"goldKnife" ,
	"" ,														//"goldBattleAxe" ,


	"Hope you brought a sword.",                                   // zombie spawn egg
	"Oinks included.",                                             // pig spawn egg
	"May ignore you.",                                             // cat spawn egg
	"Good luck.",                                                  // goblin spawn egg
	"",																//scare crow

	"Keeps you going.",                                            // apple
	"A bit tart.",                                                 // blackBerrie
	"Nature’s candy.",                                             // blueBerrie
	"Double the fun.",                                             // cherries
	"Why did you eat that?",                                       // chilliPepper
	"Shake it first.",                                             // cocconut
	"Don’t slip on them.",                                         // grapes
	"Pucker up.",                                                  // lime
	"Soft and sweet.",                                             // peach
	"Spiky outside, sweet inside.",                                // pinapple
	"Smells like summer.",                                         // strawberry

	"Better than barefoot.",                                       // leather boots
	"Not just for looks.",                                         // leather ChestPlate
	"Keeps your head warm.",                                       // leather cap
	"Clink with every step.",                                      // copper boots
	"Slightly protective, very loud.",                             // copper ChestPlate
	"Stylish head protection.",                                    // copper cap
	"Don’t try to swim.",                                          // lead boots
	"Heavy-duty defense.",                                         // lead ChestPlate
	"Thicc hat energy.",                                           // lead cap
	"For the serious adventurer.",                                 // iron boots
	"Reliable bodyguard.",                                         // iron ChestPlate
	"Keeps brain safe-ish.",                                       // iron cap
	"Fashion meets function.",                                     // silver boots
	"Gleams in sunlight.",                                         // silver ChestPlate
	"Shiny and snug.",                                             // silver cap
	"Impractical, but fabulous.",                                  // gold boots
	"Flex with protection.",                                       // gold ChestPlate
	"Crown-adjacent.",                                             // gold cap

	"Used to wash paint away from blocks.",                     // soap
	"Blank canvas starter.",                                       // white paint
	"Moody but soft.",                                             // lightGray paint
	"Serious and stormy.",                                         // darkGray paint
	"Embrace the void.",                                           // black paint
	"Dirt, but artsy.",                                            // brown paint
	"Danger? Passion? You decide.",                                // red paint
	"Zesty and bold.",                                             // orange paint
	"Sunshine in a can.",                                          // yellow paint
	"Loud and proud.",                                             // lime paint
	"Nature vibes.",                                               // green paint
	"Somewhere between cool and cooler.",                          // turqoise paint
	"Fresh like ocean breeze.",                                    // cyan paint
	"Classic and calming.",                                        // blue paint
	"Royal pick.",                                                 // purple paint
	"Hot and dramatic.",                                           // magenta paint
	"Loudly lovely.",                                              // pink paint

	"Don’t spend it all at once.",                                 // copper coin
	"Feels richer already.",                                       // silver coin
	"Now you're getting somewhere.",                               // gold coin
	"You're filthy rich.",                                         // diamond coin

	"Basic but useful.",                                           // wooden arrow
	"What could go wrong?",                                        // flaming arrow
	"Pointy and petty.",                                           // goblin arrow
	"Creepy but effective.",                                       // bone arrow
};



std::string Item::getItemDescription()
{


	static_assert(sizeof(itemsDescriptions) / sizeof(itemsDescriptions[0]) == lastItem - ItemsStartPoint, "");

	if (isItem(type))
	{
		return itemsDescriptions[type - ItemsStartPoint];
	}
	else
	{
		return "";
	}

	return "";
}

