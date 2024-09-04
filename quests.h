#include <memory>
#include <string>
#include <unordered_map>

// Map of "mob type" to lowercase mob name. Mob type is: `face << 8 | type_id`.
// This format is used by the "kill N monsters" quests.
extern std::unique_ptr<std::unordered_map<int, std::string>> mob_names;

// Initializes `mob_names`. Defined in `inn.cpp` because it has the necessary structures.
// If `mob_names` are already initialized, does nothing.
void InitializeMobNames();

// Quest filter. Player ID -> filter string.
extern std::unordered_map<short, std::string> quest_filter_per_player;

// Initializes `quest_filter_per_player` by pre-allocating all possible player IDs.
void InitializeQuestFilter();