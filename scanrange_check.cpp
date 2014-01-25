#include "srvmgr.h"

int _stdcall get_alt(u_char x, u_char y) {
	__asm {
		mov	eax, 0x6B16A8
		mov	eax, [eax]
		add	eax, 0x944F4
		xor	ecx, ecx
		mov	ch, byte ptr [y]
		mov	cl, byte ptr [x]
		add	eax, ecx
		movsx	eax, byte ptr [eax]
	}
}

int _stdcall test_if_visible(u_char x, u_char y, short vis, u_char x2, u_char y2) {
	int vision;
	vision=(vis >> 1)+(1 << 6);
//	vision+=0.1f*(((vis & 0xFF)*10)>>8);
//	vision*=vision;

	int dx, dy, r, dalt,alt,alt2;
	alt = get_alt(x, y);
	dx = x - x2;
	dy = y - y2;
	r = dx*dx + dy*dy;
	alt2 = get_alt(x2, y2);
	if (alt2 > get_alt(x2+1, y2)) alt2 = get_alt(x2+1, y2);
	if (alt2 > get_alt(x2+1, y2+1)) alt2 = get_alt(x2+1, y2+1);
	if (alt2 > get_alt(x2, y2+1)) alt2 = get_alt(x2, y2+1);

        dalt = alt - alt2;
	log_format("(%d,%d)[%d]->(%d,%d)[%d], r = %d, vis = %d, dalt = %d\n", x,y,alt,x2,y2,alt2,r, vision, dalt);

	return 0;
}

void _declspec(naked) area_cast(void) {
	__asm { //005054FF
		cmp	dword ptr [ebp-0x014], 0
		jz	area_cast_ex

		mov	edx, [ebp-0x010]
		xor	eax, eax
		mov	ax, [edx+13h]
		push	eax
		mov	ecx, [ebp-0x010]
		xor	edx, edx
		mov	dx, [ecx+5]
		push	edx
		mov	ecx, [ebp-0x0C38]
		mov	edx, 0x502AD1
		call	edx	// get char

		test	eax, eax
		jz	area_cast_ex
		mov	[ebp-0x08C], eax
		mov	eax, [ebp-0x08C]

		movsx	edx, word ptr [eax+0A4h]
		and	edx, 0FFh
		imul	edx, 0Ah
		sar	edx, 8

		push	edx

		mov	eax, [ebp-0x08C]
		movsx	ecx, word ptr [eax+0A4h]
		sar	ecx, 8
		push	ecx

		mov	eax, 0x6B16A8
		mov	eax, [eax]
		
		add	eax, 0x944F4

		mov	ecx, [ebp-0x10]
		xor	edx, edx
		mov	dh, byte ptr [ecx + 0x0C]
		mov	dl, byte ptr [ecx + 0x0A]

                add	eax, edx

		movsx	ecx, byte ptr [eax]

		push	ecx

		mov	ecx, [ebp-0x10]
		movzx	edx, byte ptr [ecx + 0x0C]
		push	edx ///to.y
		movzx	edx, byte ptr [ecx + 0x0A]
		push	edx ///to.x

		mov	eax, [ebp-0x08C]
		mov	edx, [eax + 0x10] /// from

		mov	eax, 0x6B16A8
		mov	eax, [eax]
		
		add	eax, 0x944F4

		xor	ecx, ecx
		mov	ch, byte ptr [edx + 1]
		mov	cl, byte ptr [edx]

                add	eax, ecx

		movsx	ecx, byte ptr [eax]
		push	ecx

		movzx	ecx, byte ptr [edx + 1]
		push	ecx ///from.y
		movzx	ecx, byte ptr [edx]
		push	ecx ///from.x

		mov	eax, [ebp - 0x14]
//		mov	ecx, [eax + 0x38] /// player
		mov	ecx, [eax + 0x0A78] /// name
		push	ecx

//		push	eax
		
		push	offset aArea_Cast

		call	log_format

		mov	ecx, [ebp - 0x10]
		mov	eax, [ecx + 0x0A] // coords
		mov	ecx, [ebp - 0x14] // player
		call	vision_of_all
		test	eax, eax
		jnz	skip_na_out

		mov	eax, [ebp - 0x14]
//		mov	ecx, [eax + 0x38] /// player
		mov	ecx, [eax + 0x0A78] /// name
		push	ecx
		push	offset aErrPoint

		call	log_format

skip_na_out:
		
		mov	ecx, [ebp-0x014]
		call	cancel_camp
area_cast_ex:
		mov	edx, 0x050550D
		jmp	edx
	} ///0050550D
}

void _declspec(naked) vision_of_all_2(void) { ///// возвращает 1, если точка видна, 0, если нет
	__asm {
		push	ebp
		mov	ebp, esp
		sub	esp, 0x3C
		mov	[ebp-0x038], ecx /// unitlist
		mov	[ebp-0x03C], eax /// coords
		cmp	dword ptr [ebp-0x038], 0
		jz	loc_5560EC
		mov	eax, [ebp-0x038]
		add	eax, 4
		mov	[ebp-0x024], eax
		jmp	loc_5560F3
// ---------------------------------------------------------------------------

loc_5560EC:				 // CODE XREF: sub_5560D2+Dj
		mov	dword ptr [ebp-0x024], 0

loc_5560F3:				 // CODE XREF: sub_5560D2+18j
		mov	ecx, [ebp-0x024]
		mov	[ebp-0x08], ecx
		mov	edx, [ebp-0x08]
		mov	eax, [edx+4]
		mov	[ebp-0x010], eax
		mov	ecx, [ebp-0x010]
		mov	[ebp-0x04], ecx
		cmp	dword ptr [ebp-0x04], 0
		jz	loc_55613D
		lea	edx, [ebp-0x04]
		mov	[ebp-0x01C], edx
		mov	eax, [ebp-0x08]
		mov	[ebp-0x018], eax
		mov	ecx, [ebp-0x01C]
		mov	edx, [ecx]
		mov	[ebp-0x014], edx
		mov	eax, [ebp-0x01C]
		mov	ecx, [ebp-0x014]
		mov	edx, [ecx]
		mov	[eax], edx
		mov	eax, [ebp-0x014]
		mov	ecx, [eax+8]
		mov	[ebp-0x020], ecx
		mov	edx, [ebp-0x020]
		mov	[ebp-0x0C], edx
		jmp	loc_556144
// ---------------------------------------------------------------------------

loc_55613D:				 // CODE XREF: sub_5560D2+3Aj
		mov	dword ptr [ebp-0x0C], 0

loc_556144:				 // CODE XREF: sub_5560D2+69j
						// sub_5560D2:loc_556197j
		cmp	dword ptr [ebp-0x0C], 0
		jz	loc_556199
		

//		mov	eax, 0x6B16A8
//		mov	eax, [eax]
//		add	eax, 0x944F4

//		lea	edx, [ebp-0x03C]
			
//		xor	ecx, ecx
//		mov	ch, byte ptr [edx + 2]
//		mov	cl, byte ptr [edx]
  //              add	eax, ecx
//		movsx	eax, byte ptr [eax]
//		push	eax	//// altitude

		lea	ecx, [ebp-0x03C]
		movzx	eax, byte ptr [ecx + 2]
		push	eax ///y
		movzx	eax, byte ptr [ecx]
		push	eax ///x

		mov	eax, [ebp-0x0C]
		movzx	ecx, word ptr [eax+0A4h]
		push	ecx

//		mov	eax, 0x6B16A8
//		mov	eax, [eax]
//		add	eax, 0x944F4
//		mov	ecx, [ebp-0x0C]
//		mov	edx, [ecx + 0x10] /// coords
//		xor	ecx, ecx
//		mov	ch, byte ptr [edx + 1]
//		mov	cl, byte ptr [edx]
  //              add	eax, ecx
//		movsx	eax, byte ptr [eax]
//		push	eax	//// altitude

		mov	ecx, [ebp-0x0C]
		mov	edx, [ecx + 0x10] /// coords
		movzx	eax, byte ptr [edx + 1]
		push	eax ///y
		movzx	eax, byte ptr [edx]
		push	eax ///x

		call	test_if_visible
		test	eax, eax
		jnz	loc_55619B

//		mov	ecx, [ebp-0x0C]
//		movsx	eax, word ptr [ecx + 4]
//		push	eax /// ID

//		push	offset alog_vis
//		call	log_format

		cmp	dword ptr [ebp-0x04], 0
		jz	loc_556190
		lea	edx, [ebp-0x04]
		mov	[ebp-0x030], edx
		mov	eax, [ebp-0x08]
		mov	[ebp-0x02C], eax
		mov	ecx, [ebp-0x030]
		mov	edx, [ecx]
		mov	[ebp-0x028], edx
		mov	eax, [ebp-0x030]
		mov	ecx, [ebp-0x028]
		mov	edx, [ecx]
		mov	[eax], edx
		mov	eax, [ebp-0x028]
		mov	ecx, [eax+8]
		mov	[ebp-0x034], ecx
		mov	edx, [ebp-0x034]
		mov	[ebp-0x0C], edx
		jmp	loc_556197
// ---------------------------------------------------------------------------

loc_556190:				 // CODE XREF: sub_5560D2+8Dj
		mov	dword ptr [ebp-0x0C], 0

loc_556197:				 // CODE XREF: sub_5560D2+BCj
		jmp	loc_556144
// ---------------------------------------------------------------------------

loc_556199:				 // CODE XREF: sub_5560D2+76j
		xor	eax, eax

loc_55619B:				 // CODE XREF: sub_5560D2+87j
		mov	esp, ebp
		pop	ebp
		ret
	}
}

void _declspec(naked) vision_of_all(void) { ///// распечатка вида всех игроков
	__asm {///TESTED100%  
		push	ebp
		mov	ebp, esp
		sub	esp, 0x014
		mov	[ebp-0x10], ecx /// player
		mov	[ebp-0x14], eax /// coords
		lea	ecx, [ebp-0x08]
		mov	edx, 0x496DC0
		call	edx	// ??0Iostream_init@@QAE@XZ
						// doubtful name
		mov	ecx, 0x06CDB24
		mov	ecx, [ecx]
		push	ecx
		lea	ecx, [ebp-0x08]
		mov	edx, 0x496DD0
		call	edx

loc_50303D:				 // CODE XREF: sub_50301F+30j
		test	eax, eax
		jz	loc_503051
		mov	[ebp-0x0C], eax

//		mov	ecx, 0x06A8B8C 
//		mov	ecx, [ecx]
//		add	ecx, 0x0A8C4
//		mov	eax, [ebp-0x10]
//		imul	eax, 0x46
//		mov	edx, [ebp-0x0C]
//		movzx	edx, byte ptr [edx + 4]
//		add	eax, edx
//		add	eax, ecx
//		movzx	eax, byte ptr [eax]
//		push	eax

		mov	ecx, 0x06A8B8C 
		mov	ecx, [ecx]
		add	ecx, 0x0A8C4
		mov	edx, [ebp-0x0C]
		movzx	eax, byte ptr [edx + 4]
		imul	eax, 0x46
		mov	edx, [ebp-0x10]
		movzx	edx, byte ptr [edx + 4]
		add	eax, edx
		add	eax, ecx
		movzx	eax, byte ptr [eax]
		test	eax, 0x10
		jz	skip_vis /// только если стоит вид

		mov	eax, [ebp-0x0C]
		movsx	ecx, word ptr [eax+0x04]
		push	ecx

		cmp	dword ptr [eax+0x2C], 0
		jnz	name_AI
		mov	eax, [eax + 0x0A78]
		jmp	name_NOTAI
name_AI:	mov	eax, [eax + 0x18]
name_NOTAI:
		push	eax
		push	offset avisall
		call	log_format

		mov	eax, [ebp-0x14] // unit
		mov	ecx, [ebp-0x0C] 
		mov	ecx, [ecx+24h] /// unitlist
		call	vision_of_all_2
		test	eax, eax
		jnz	loc_503052

skip_vis:

		lea	ecx, [ebp-0x08]
		mov	edx, 0x496E20
		call	edx
		jmp	loc_50303D
// ---------------------------------------------------------------------------

loc_503051:				 // CODE XREF: sub_50301F+20j
		xor	eax, eax
loc_503052:
		mov	esp, ebp
		pop	ebp
		ret
	}
}
