.section ".text.boot"

.globl _start

// Kernel entry point
// r15 -> should be at 0x8000
// r0, r1, r2 = arguments from gpu
_start:
    // Setup stack -----------------------------------------------------------
    mov sp, #0x8000
    
    // Clear out bss ---------------------------------------------------------
	ldr r4, =__bss_start
	ldr r9, =__bss_end
	mov r5, #0
	mov r6, #0
	mov r7, #0
	mov r8, #0
	b       2f
 
1:
	// store multiple at r4.
	stmia r4!, {r5-r8}
 
	// If we are still below bss_end, loop.
2:
	cmp r4, r9
	blo 1b
    
    // Enable VFP ------------------------------------------------------------
    // r4 = Access Control Register
    mrc p15, #0, r4, c1, c0, #2
    // enable full access for p10,11
    orr r4, r4, #(0xf << 20)
    // ccess Control Register = r4
    mcr p15, #0, r4, c1, c0, #2
    mov r4, #0
    // flush prefetch buffer because of FMXR below
    mcr p15, #0, r4, c7, c5, #4
    // and CP 10 & 11 were only just enabled
    // Enable VFP itself
    mov r4,#0x40000000
    // FPEXC = r4
    fmxr FPEXC, r4
    
    // Enable L1 cache -------------------------------------------------------
    mrc p15,0,r4,c1,c0,0 // R4 = System Control Register

    // Enable caches and branch prediction
    orr r4,#0x4    // SCTLR_ENABLE_BRANCH_PREDICTION
    orr r4,#0x800  // SCTLR_ENABLE_DATA_CACHE
    orr r4,#0x1000 // SCTLR_ENABLE_INSTRUCTION_CACHE
    
    mcr p15,0,r4,c1,c0,0 // System Control Register = R4
    
    // Call kernel_main ------------------------------------------------------
    ldr r3, =kernel_main
	blx r3
	
	// halt
halt:
    wfe
    b halt