.686
.xmm
.model flat, c

DATA segment align(32)
F76543210 real4 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0
F88888888 real4 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0
F00000000 real4 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
F3210 real4 0.0, 1.0, 2.0, 3.0
DATA ends

.code

projx32 PROC param:dword
;			for (int iy=iy0; iy<iy1; iy++) {
;				const int ifpidx = iy * ixdimp;
;				const float fyoff = iy * fsin + foffset;
;				for (int ix=0; ix<ixdimp; ix++) {
;					int ix0 = (int)(ix * fcos + fyoff);
;					if (ix0 < 0) continue;
;					if (ix0 >= ixdimpg) continue;
;					ifp[ifpidx + ix] += ipgp[ix0];
;				}
;			}

;local valiables
	local ixdimp :dword
	local ixdimp4 :dword
	local iy0 :dword
	local iy1 :dword
;	local pmxcsr :dword
;	local smxcsr :dword

;store registers
	push esi
	push edi
	push ebx
	push ebp
;	stmxcsr smxcsr

;get pointer to args
	mov esi, param ;arg #1

;load valiables	and constants
;	mov ixdimpg, [esi + 12]
	mov eax, [esi + 16]
	mov ixdimp, eax
;	mov eax, ixdimp
	shl eax, 2
	mov ixdimp4, eax
;	mov ifp, [esi + 20]
;	mov igp, [esi + 24]
	mov eax, [esi + 32]	;iy0
	mov iy0, eax
	mov eax, [esi + 36]	;iy1
	mov iy1, eax

;sse rounding mode RC=00B (MXCSR[14:13])
;	stmxcsr pmxcsr
;	and pmxcsr, 0FFFF9FFFh
;	or pmxcsr,  000000000h
;	ldmxcsr pmxcsr

;jump to AVX routine
;	mov eax, [esi + 28]	; AVX flag
;	and eax, 000000001h
;	jnz USEAVX

;SSE
	mov eax, [esi] ; &fcos
	movss xmm0, real4 ptr [eax]
	shufps xmm0, xmm0, 0
	
	mov eax, [esi + 4]; &fsin
	movss xmm1, real4 ptr [eax]
	shufps xmm1, xmm1, 0

	mov eax, [esi + 8]; &foffset
	movss xmm7, real4 ptr [eax]
	shufps xmm7, xmm7, 0

	movaps xmm6, F3210

	mov eax, iy1 ; iy1
	dec eax
	mov ecx, ixdimp
	imul ecx
	shl eax, 2 ; ixy = ixdimp * (iy1 - 1) * 4
	add eax, [esi + 20]; ixy += ifp
	mov edi, eax

	mov ecx, [esi + 12]; ixdimpg
	mov esi, [esi + 24]; igp

	mov edx, iy1; iy<==iy1
	dec edx
	mov eax, 0
LOOPY:
	mov ebx, ixdimp ; ix<==ixdimp
	dec ebx
	cvtsi2ss xmm3, edx	; xmm3<==iy
	shufps xmm3, xmm3, 0	; xmm3<==iy, iy, iy, iy
	movaps xmm5, xmm1	; xmm5<==fsin, fsin, fsin, fsin
	mulps xmm5, xmm3	; iy * fsin for each float
	addps xmm5, xmm7	; + foffset for each float
LOOPX:
	cvtsi2ss xmm2, ebx	; xmm2<==ix
	shufps xmm2, xmm2, 0	; xmm2<==ix, ix, ix, ix
	subps xmm2, xmm6	; xmm2<==ix-3, ix-2, ix-2, ix
	movaps xmm4, xmm0	; xmm4<==fcos, fcos, fcos, fcos
	mulps xmm4, xmm2	; (ix-n) * fcos
	addps xmm4, xmm5	; (ix-n) * fcos + foffset
	cvttps2dq xmm4, xmm4	; xmm4 float*4 to integer32*4
	movd eax, xmm4	; lower 4 bytes to eax
;	pextrd eax, xmm4, 0	; SSE4.1
	cmp eax, ecx	; ix<=>ixdimpg
	jae LOOPXSKIP1	; ix0 >= ixdimp * DBPT_GINTP or ix0 < 0
	mov eax, [esi + eax * 4]	; eax<==igp[ix * DBPT_GINTP]
	add [edi + ebx * 4], eax	; ifp[ix] += eax
LOOPXSKIP1:
	dec ebx ; ix--
	jl LOOPYEND ; ix < 0
	psrldq xmm4, 4	; shift right by 4 bytes (integer32)
	movd eax, xmm4
;	pextrd eax, xmm4, 1 ; SSE4.1
	cmp eax, ecx; ixdimpg
	jae LOOPXSKIP2 ; ix0 >= ixdimp * DBPT_GINTP or ix0 < 0
	mov eax, [esi + eax * 4]
	add [edi + ebx * 4], eax
LOOPXSKIP2:
	dec ebx ; ix--
	jl LOOPYEND ; ix < 0
	psrldq xmm4, 4
	movd eax, xmm4
;	pextrd eax, xmm4, 2 ; SSE4.1
	cmp eax, ecx; ixdimpg
	jae LOOPXSKIP3 ; ix0 >= ixdimp * DBPT_GINTP or ix0 < 0
	mov eax, [esi + eax * 4]
	add [edi + ebx * 4], eax
LOOPXSKIP3:
	dec ebx ; ix--
	jl LOOPYEND ; ix < 0
	psrldq xmm4, 4
	movd eax, xmm4
;	pextrd eax, xmm4, 3 ; SSE4.1
	cmp eax, ecx; ixdimpg
	jae LOOPXEND ; ix0 >= ixdimp * DBPT_GINTP or ix0 < 0
	mov eax, [esi + eax * 4]
	add [edi + ebx * 4], eax
LOOPXEND:
	dec ebx
	jge LOOPX ; ix >= 0
LOOPYEND:
	sub edi, ixdimp4
	dec edx
	cmp edx, iy0
	jge LOOPY ; iy >= iy0

;	ldmxcsr smxcsr
	pop ebp
	pop ebx
	pop edi
	pop esi
	ret

projx32 ENDP

end
