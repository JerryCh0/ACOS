	.section	__TEXT,__text,regular,pure_instructions
	.macosx_version_min 10, 11
	.globl	_main
	.align	4, 0x90
_main:                                  ## @main
	.cfi_startproc
## BB#0:
	pushq	%rbp
Ltmp0:
	.cfi_def_cfa_offset 16
Ltmp1:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp2:
	.cfi_def_cfa_register %rbp
	subq	$96, %rsp
	movl	$0, -4(%rbp)
	movl	%edi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	cmpl	$2, -8(%rbp)
	je	LBB0_2
## BB#1:
	leaq	L_.str(%rip), %rsi
	movq	___stderrp@GOTPCREL(%rip), %rax
	movq	(%rax), %rdi
	movq	-16(%rbp), %rax
	movq	(%rax), %rdx
	movb	$0, %al
	callq	_fprintf
	movl	$1, %edi
	movl	%eax, -36(%rbp)         ## 4-byte Spill
	callq	_exit
LBB0_2:
	leaq	-24(%rbp), %rdi
	callq	_pipe
	cmpl	$-1, %eax
	jne	LBB0_4
## BB#3:
	leaq	L_.str1(%rip), %rdi
	callq	_perror
	movl	$1, %edi
	callq	_exit
LBB0_4:
	callq	_fork
	movl	%eax, -28(%rbp)
	cmpl	$-1, -28(%rbp)
	jne	LBB0_6
## BB#5:
	leaq	L_.str2(%rip), %rdi
	callq	_perror
	movl	$1, %edi
	callq	_exit
LBB0_6:
	cmpl	$0, -28(%rbp)
	jne	LBB0_11
## BB#7:
	movl	-20(%rbp), %edi
	callq	_close
	movl	%eax, -40(%rbp)         ## 4-byte Spill
LBB0_8:                                 ## =>This Inner Loop Header: Depth=1
	leaq	-29(%rbp), %rsi
	movl	$1, %eax
	movl	%eax, %edx
	movl	-24(%rbp), %edi
	callq	_read
	cmpq	$0, %rax
	jle	LBB0_10
## BB#9:                                ##   in Loop: Header=BB0_8 Depth=1
	movl	$1, %edi
	leaq	-29(%rbp), %rsi
	movl	$1, %eax
	movl	%eax, %edx
	callq	_write
	movq	%rax, -48(%rbp)         ## 8-byte Spill
	jmp	LBB0_8
LBB0_10:
	movl	$1, %edi
	leaq	L_.str3(%rip), %rsi
	movl	$1, %eax
	movl	%eax, %edx
	callq	_write
	movl	-24(%rbp), %edi
	movq	%rax, -56(%rbp)         ## 8-byte Spill
	callq	_close
	xorl	%edi, %edi
	movl	%eax, -60(%rbp)         ## 4-byte Spill
	callq	__exit
LBB0_11:
	movl	-24(%rbp), %edi
	callq	_close
	movl	-20(%rbp), %edi
	movq	-16(%rbp), %rcx
	movq	8(%rcx), %rsi
	movq	-16(%rbp), %rcx
	movq	8(%rcx), %rcx
	movl	%edi, -64(%rbp)         ## 4-byte Spill
	movq	%rcx, %rdi
	movl	%eax, -68(%rbp)         ## 4-byte Spill
	movq	%rsi, -80(%rbp)         ## 8-byte Spill
	callq	_strlen
	movl	-64(%rbp), %edi         ## 4-byte Reload
	movq	-80(%rbp), %rsi         ## 8-byte Reload
	movq	%rax, %rdx
	callq	_write
	movl	-20(%rbp), %edi
	movq	%rax, -88(%rbp)         ## 8-byte Spill
	callq	_close
	xorl	%edi, %edi
                                        ## kill: RDI<def> EDI<kill>
	movl	%eax, -92(%rbp)         ## 4-byte Spill
	callq	_wait
	xorl	%edi, %edi
	movl	%eax, -96(%rbp)         ## 4-byte Spill
	callq	_exit
	.cfi_endproc

	.section	__TEXT,__cstring,cstring_literals
L_.str:                                 ## @.str
	.asciz	"Usage: %s <string>\n"

L_.str1:                                ## @.str1
	.asciz	"pipe"

L_.str2:                                ## @.str2
	.asciz	"fork"

L_.str3:                                ## @.str3
	.asciz	"\n"


.subsections_via_symbols
