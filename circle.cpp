#include "config_new.h"
#include "utils.h"

const char* UnitName(T_UNIT* unit) {
    return unit ? unit->name : "?";
}

const char* PlayerName(T_UNIT* unit) {
    return unit && unit->player ? unit->player->name : "??";
}

namespace circle {

int Circle(T_UNIT* unit) {
    const char* name = unit->player->name;

    if (name == nullptr) {
        return 0;
    }

    char maybe_circle = name[0];
    if (name[0] == '_' || name[0] == '@') {
        maybe_circle = name[1];
    }

    if ('1' >= maybe_circle || maybe_circle >= '9') {
        return 0;
    }

    return static_cast<int>(maybe_circle - '0');
}

double Multiplier(int circle, ServerIDType server_id) {
    switch (server_id) {
        case EASY:      return 0.05 * circle;
        case KIDS:      return 0.10 * circle;
        case NIVAL:     return 0.15 * circle;
        case MEDIUM:    return 0.20 * circle;
        case HARD:      return 0.25 * circle;
        case NIGHTMARE: return 0.30 * circle;
        case QUEST_T1:  return 0.25 * circle;
        case QUEST_T2:  return 0.20 * circle;
        case QUEST_T3:  return 0.15 * circle;
        case QUEST_T4:  return 0.10 * circle;
        default:        return 0.0;
    }
}

int Bonus(int circle, ServerIDType server_id) {
    switch (server_id) {
        case EASY:      return (circle + 1) / 2;
        case KIDS:      return circle;
        case NIVAL:     return circle;
        case MEDIUM:    return circle;
        case HARD:      return 2 * circle;
        case NIGHTMARE: return 3 * circle;
        case QUEST_T1:  return 4 * circle;
        case QUEST_T2:  return 5 * circle;
        case QUEST_T3:  return 6 * circle;
        case QUEST_T4:  return 7 * circle;
        default:        return 0;
    }
}

// Increase damage to units that are going through circles.
int IncreaseDamage(T_UNIT* attacker, T_UNIT* target, int damage) {
    // Not a player's unit --- no changes.
    if (!target || !target->player || target->player->unitType != 0) {
        return damage;
    }

    // On-map effects are not modified.
    if (!attacker || !attacker->player) {
        return damage;
    }

    // PvP is untouched.
    if (attacker->player->unitType == 0) {
        return damage;
    }

    int circle_number = Circle(target);
    if (circle_number == 0) {
        return damage;
    }

    double multiplier = 1 + Multiplier(circle_number, Config::ServerID);
    int bonus = Bonus(circle_number, Config::ServerID);
    int new_damage = static_cast<int>(damage * multiplier + bonus);
    return new_damage;
}

}

int ChangeDamageRegular(T_UNIT* attacker, T_UNIT* target, int damage) {
    return circle::IncreaseDamage(attacker, target, damage);
}

int ChangeDamageSpecial(T_UNIT* attacker, T_UNIT* target, int damage) {
    Printf("[circle/special] 0x%x -> 0x%x (%s/%s -> %s/%s) for %d", attacker, target, UnitName(attacker), PlayerName(attacker), UnitName(target), PlayerName(target), damage);
    // No idea what the original logic is for. Not modfying the damage till we see some usage.
    return damage;
}

int ChangeDamageMagic(T_UNIT* attacker, T_UNIT* target, int damage) {
    return circle::IncreaseDamage(attacker, target, damage);
}

// Address: 00536d81
void __declspec(naked) circle_damage_regular() {
    __asm {
        push DWORD PTR [ebp-0x1c] // Current damage.
        push DWORD PTR [ebp-0x3c] // Attacker.
        push DWORD PTR [ebp+0xc] // Target.
        call ChangeDamageRegular

        // Save new damage.
        mov DWORD PTR [ebp-0x1c], eax

        // Original logic.
        mov ecx,DWORD PTR [ebp+0xc]
        mov edx, 0
        mov dl, BYTE PTR [ecx+0x4c]
        add edx, 0x10

        // Restore original instruction.
        mov ebx, 0x00536d87
        jmp ebx
    }
}

// Address: 00536e1e
void __declspec(naked) circle_damage_special() {
    __asm {
        // Original logic.
        mov cl, BYTE PTR [edx+0x11]
        add eax, ecx

        push eax  // Current damage.
        push DWORD PTR [ebp-0x3c] // Attacker.
        push DWORD PTR [ebp+0xc] // Target.
        call ChangeDamageSpecial

        // Save new damage.
        mov DWORD PTR [ebp-0x1c], eax

        // Restore original instruction.
        mov ebx, 0x00536e26
        jmp ebx
    }
}

// Address: 00536ed8
void __declspec(naked) circle_damage_magic() {
    __asm {
        // Original logic.
        mov dl, BYTE PTR [ecx+0x13]
        add eax, edx

        push eax  // Current damage.
        push DWORD PTR [ebp-0x3c] // Attacker.
        push DWORD PTR [ebp+0xc] // Target.
        call ChangeDamageMagic

        // Save new damage.
        mov DWORD PTR [ebp-0x1c], eax

        // Restore original instruction.
        mov ebx, 0x00536ee0
        jmp ebx
    }
}
