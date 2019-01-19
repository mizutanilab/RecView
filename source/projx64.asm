;ml64.exe

DATA segment align(32)
F76543210 real4 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0
F88888888 real4 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0
F00000000 real4 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
F3210 real4 0.0, 1.0, 2.0, 3.0
DATA ends

.code

projx64 PROC
;	mov r10, rcx ; the first arg = rcx
;	mov r10, rdx ; the 2nd arg = rdx
;	mov rax, r10 ; rax to return result

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
;	local pmxcsr :dword
;	local smxcsr :dword

;store registers
	push rbx
	push rbp
	push rsi
	push rdi

;get pointer to args
	mov rsi, rcx	; arg #1

;init params
;	mov ixdimpg, [rsi + 24]
	mov r10, [rsi + 32]	; ixdimp
	mov r9, r10
	shl r9, 2	; r9<==ixdimp * 4
;	mov ifp, [rsi + 40]
;	mov igp, [rsi + 48]
	mov r12, [rsi + 64]	;iy0
	mov r13, [rsi + 72]	;iy1

;sse rounding mode RC=00B (MXCSR[14:13])
;	stmxcsr pmxcsr
;	and pmxcsr, 0FFFF9FFFh
;	ldmxcsr pmxcsr

;jump to AVX routine
	mov rax, [rsi + 56]	; AVX flag
	and rax, 000000001h
	jnz USEAVX

;load valiables	
	mov rax, r13; iy = iy1
	dec rax
	mov rcx, r10; ix = ixdimp
	imul rcx
	shl rax, 2	; ixy = ixdimp * (iy1 - 1) * 4
	add rax, [rsi + 40]	; ixy += ifp
	mov rdi, rax

	mov rax, [rsi]	; &fcos
	movss xmm0, real4 ptr [rax]
	shufps xmm0, xmm0, 0
	
	mov rax, [rsi + 8]	; &fsin
	movss xmm1, real4 ptr [rax]
	shufps xmm1, xmm1, 0

	mov rax, [rsi + 16]	; &foffset
	movss xmm7, real4 ptr [rax]
	shufps xmm7, xmm7, 0

	movaps xmm6, F3210
	
;start process
	mov rcx, [rsi + 24]	; ixdimpg
	mov rsi, [rsi + 48]	; igp

	mov rdx, r13	; iy<==iy1
	dec rdx
	mov rax, 0
LOOPY:
	mov rbx, r10	; ix<==ixdimp
	dec rbx
	cvtsi2ss xmm3, rdx	; xmm3<==iy
	shufps xmm3, xmm3, 0	; xmm3<==iy, iy, iy, iy
	movaps xmm5, xmm1	; xmm5<==fsin, fsin, fsin, fsin
	mulps xmm5, xmm3	; iy * fsin for each float
	addps xmm5, xmm7	; + foffset for each float
LOOPX:
	cvtsi2ss xmm2, rbx	; xmm2<==ix
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
	mov eax, [rsi + rax * 4]	; eax<==igp[ix * DBPT_GINTP]
	add [rdi + rbx * 4], eax	; ifp[ix] += eax
LOOPXSKIP1:
	dec rbx	; ix--
	jl LOOPYEND	; ix < 0
	psrldq xmm4, 4	; shift right by 4 bytes (integer32)
	movd eax, xmm4
;	pextrd eax, xmm4, 1	; SSE4.1
	cmp eax, ecx; ixdimpg
	jae LOOPXSKIP2	; ix0 >= ixdimp * DBPT_GINTP or ix0 < 0
	mov eax, [rsi + rax * 4]
	add [rdi + rbx * 4], eax
LOOPXSKIP2:
	dec rbx	; ix--
	jl LOOPYEND	; ix < 0
	psrldq xmm4, 4
	movd eax, xmm4
;	pextrd eax, xmm4, 2	; SSE4.1
	cmp eax, ecx; ixdimpg
	jae LOOPXSKIP3	; ix0 >= ixdimp * DBPT_GINTP or ix0 < 0
	mov eax, [rsi + rax * 4]
	add [rdi + rbx * 4], eax
LOOPXSKIP3:
	dec rbx	; ix--
	jl LOOPYEND	; ix < 0
	psrldq xmm4, 4
	movd eax, xmm4
;	pextrd eax, xmm4, 3	; SSE4.1
	cmp eax, ecx; ixdimpg
	jae LOOPXEND	; ix0 >= ixdimp * DBPT_GINTP or ix0 < 0
	mov eax, [rsi + rax * 4]
	add [rdi + rbx * 4], eax
LOOPXEND:
	dec rbx	; ix--
	jge LOOPX	; ix >= 0
LOOPYEND:
	sub rdi, r9 ; ixdimp*4
	dec rdx	; iy--
	cmp rdx, r12
	jge LOOPY	; iy >= iy0

;	ldmxcsr smxcsr
	pop rdi
	pop rsi
	pop rbp
	pop rbx
	ret

USEAVX:
;load valiables	
	mov rax, [rsi]	; &fcos
	vbroadcastss ymm0, real4 ptr [rax]
	mov r8, [rsi + 8]	; &fsin
	mov r11, [rsi + 16]	; &foffset

	mov rcx, [rsi + 24]	; ixdimpg
	vcvtsi2ss xmm6, xmm6, rcx	; xmm6<==ixdimpg
	vbroadcastss ymm6, xmm6	; ymm6<==ixdimpg, ixdimpg, ixdimpg, ixdimpg
	vcvtsi2ss xmm1, xmm1, r10	; xmm1<==ixdimp
	vbroadcastss ymm1, xmm1	; ymm1<==ixdimp, ixdimp, ixdimp, ixdimp
	
	mov rax, r12; iy = iy0
	mov rcx, r10; ix = ixdimp
	imul rcx
	shl rax, 2	; ixy = ixdimp * iy0 * 4
	add rax, [rsi + 40]	; ixy += ifp
	mov rdi, rax
;	mov rdi, [rsi + 40]	; ifp
	mov rsi, [rsi + 48]	; igp

	mov rdx, r12	; iy<==iy0
ALOOPY:
	mov rbx, 0	; ix<==0
	vmovaps ymm2, F76543210	; reset ix
	vcvtsi2ss xmm3, xmm3, rdx	; xmm3<==iy
	vbroadcastss ymm3, xmm3	; xmm3<==iy, iy, iy, iy
	vbroadcastss ymm5, real4 ptr [r8]
	vmulps ymm5, ymm5, ymm3	; iy * fsin for each float
	vbroadcastss ymm7, real4 ptr [r11]	; ymm7<==foffset
	vaddps ymm5, ymm5, ymm7	; ymm5<==iy * fsin + foffset
ALOOPX:
	vmulps ymm4, ymm0, ymm2	; (ix+n) * fcos
	vaddps ymm4, ymm4, ymm5	; (ix+n) * fcos + foffset
	vcmpltps ymm7, ymm4, ymm6	; ymm7[i:i]=1 if (ymm4 < ixdimpg)
	vcmpgeps ymm3, ymm4, F00000000	; ymm3[i:i]=1 if (ymm4 >= 0)
	vpand ymm7, ymm7, ymm3
	vpxor ymm3, ymm3, ymm3	; clear ymm3
	vcvttps2dq ymm4, ymm4	; ymm4 float*8 to integer32*8
	vpgatherdd ymm3, [rsi + ymm4 * 4], ymm7	; load [rsi+ymm4*4] if ymm7=1
	vcmpltps ymm7, ymm2, ymm1	; ymm7[i:i]=1 if (ymm2 < ixdimp)
	vpmaskmovd ymm4, ymm7, [rdi + rbx * 4]
	vpaddd ymm4, ymm3, ymm4
	vpmaskmovd [rdi + rbx * 4], ymm7, ymm4

	vaddps ymm2, ymm2, F88888888	; ymm2 + 8.0
	add rbx, 8
	cmp rbx, r10
	jnae ALOOPX	; ix < ixdimp
ALOOPYEND2:
	add rdi, r9 ; +ixdimp*4
	inc rdx	; iy++
	cmp rdx, r13
	jnae ALOOPY	; iy < iy1

;	ldmxcsr smxcsr
	pop rdi
	pop rsi
	pop rbp
	pop rbx
	ret

projx64 ENDP

end
