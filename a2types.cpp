#include "a2types.h"

bool IsWarrior(const A2Unit* unit) {
    return (unit->unit_attrs & 4) == 0;
}

bool IsFemale(const A2Unit* unit) {
    return unit->type_id == 34 || unit->type_id == 36;
}
