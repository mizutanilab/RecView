;ml64.exe

.data
align 16
F3210 real4 0.0, 1.0, 2.0, 3.0

.code

projx64 PROC
;	mov r10, rcx ; the first arg = rcx
;	mov r10, rdx ; the 2nd arg = rdx
;	mov rax, r10 ; rax to return result

;			for (int iy=0; iy<ixdimp; iy++) {
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
	local pmxcsr :dword
;store registers
	push rsi
	push rdi
;get pointer to args
	mov rsi, rcx	; arg #1
;load valiables	
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
	
;	mov ixdimpg, [rsi + 24]
	mov r10, [rsi + 32]	; ixdimp
;	mov ifp, [rsi + 40]
;	mov igp, [rsi + 48]

;init params
	mov r9, r10
	shl r9, 2	; r9<==ixdimp * 4

	mov rax, r10; iy = ixdimp
	mov rcx, rax
	dec rax
	imul rcx
	shl rax, 2	; ixy = ixdimp * (ixdimp - 1) * 4
	add rax, [rsi + 40]	; ixy += ifp
	mov rdi, rax

;sse rounding mode RC=01B
;	stmxcsr pmxcsr
;	and pmxcsr, 0FFFF9FFFh
;	or pmxcsr,  000002000h
;	ldmxcsr pmxcsr

;start process
	mov rcx, [rsi + 24]	; ixdimpg
	mov rsi, [rsi + 48]	; igp

	mov rdx, r10	; iy<==ixdimp
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
	jge LOOPY	; iy >= 0

	pop rdi
	pop rsi
	ret

projx64 ENDP

end
