	.section .text
.LNDBG_TX:
# -- Machine type EFI2
# mark_description "Intel(R) C++ Compiler Professional for applications running on Intel(R) 64, Version 11.0    Build 20090131 %";
# mark_description "s";
# mark_description "-I/home/yaschenk/devel/internal/asm-trace/sra/search/linux -I/home/yaschenk/devel/internal/asm-trace/sra/sea";
# mark_description "rch/unix -I/home/yaschenk/devel/internal/asm-trace/sra/search -I/home/yaschenk/devel/internal/asm-trace/itf ";
# mark_description "-I/home/yaschenk/devel/internal/asm-trace/inc/icc/x86_64 -I/home/yaschenk/devel/internal/asm-trace/inc/icc -";
# mark_description "I/home/yaschenk/devel/internal/asm-trace/inc/gcc/x86_64 -I/home/yaschenk/devel/internal/asm-trace/inc/gcc -I";
# mark_description "/home/yaschenk/devel/internal/asm-trace/inc/linux -I/home/yaschenk/devel/internal/asm-trace/inc/unix -I/home";
# mark_description "/yaschenk/devel/internal/asm-trace/inc -c -S -o nucstrstr.pic.s -fPIC -g -Wall -D_DEBUGGING -DLINUX -DUNIX -";
# mark_description "D_GNU_SOURCE -D_REENTRANT -D_FILE_OFFSET_BITS=64 -MD";
	.file "nucstrstr.c"
	.text
..TXTST0:
# -- Begin  uint16_lsbit
# mark_begin;
       .align    2,0x90
uint16_lsbit:
# parameter 1(self): %edi
..B1.1:                         # Preds ..B1.0
..___tag_value_uint16_lsbit.2:                                  #
..LN1:
  .file   1 "/home/yaschenk/devel/internal/asm-trace/inc/gcc/x86_64/arch-impl.h"
   .loc    1  38
        pushq     %rbp                                          #38.1
        movq      %rsp, %rbp                                    #38.1
..___tag_value_uint16_lsbit.3:                                  #
        subq      $16, %rsp                                     #38.1
        movw      %di, -8(%rbp)                                 #38.1
..LN3:
   .loc    1  40
        movzwl    -8(%rbp), %eax                                #40.0
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #40.0
        movw      %ax, -16(%rbp)                                #40.0
..LN5:
   .loc    1  49
        movswl    -16(%rbp), %eax                               #49.12
        leave                                                   #49.12
..___tag_value_uint16_lsbit.7:                                  #
        ret                                                     #49.12
        .align    2,0x90
..___tag_value_uint16_lsbit.8:                                  #
                                # LOE
# mark_end;
	.type	uint16_lsbit,@function
	.size	uint16_lsbit,.-uint16_lsbit
.LNuint16_lsbit:
	.data
  .file   2 "/usr/include/stdint.h"
# -- End  uint16_lsbit
	.text
# -- Begin  uint128_shr
# mark_begin;
       .align    2,0x90
uint128_shr:
# parameter 1(self): %rdi
# parameter 2(i): %esi
..B2.1:                         # Preds ..B2.0
..___tag_value_uint128_shr.11:                                  #
..LN7:
   .loc    1  262
        pushq     %rbp                                          #262.1
        movq      %rsp, %rbp                                    #262.1
..___tag_value_uint128_shr.12:                                  #
        subq      $16, %rsp                                     #262.1
        movq      %rdi, -16(%rbp)                               #262.1
        movl      %esi, -8(%rbp)                                #262.1
..LN9:
   .loc    1  263
        movq      -16(%rbp), %rdi                               #263.0
        movl      -8(%rbp), %esi                                #263.0
        movl      %esi, %ecx                                    #263.0
        movq      8(%rdi), %rax                                 #263.0
        shrdq     %cl, %rax, (%rdi)                             #263.0
        shrq      %cl, %rax                                     #263.0
        movq      %rax, 8(%rdi)                                 #263.0
..LN11:
   .loc    1  274
        leave                                                   #274.1
..___tag_value_uint128_shr.16:                                  #
        ret                                                     #274.1
        .align    2,0x90
..___tag_value_uint128_shr.17:                                  #
                                # LOE
# mark_end;
	.type	uint128_shr,@function
	.size	uint128_shr,.-uint128_shr
.LNuint128_shr:
	.data
# -- End  uint128_shr
	.text
# -- Begin  uint128_bswap
# mark_begin;
       .align    2,0x90
uint128_bswap:
# parameter 1(self): %rdi
..B3.1:                         # Preds ..B3.0
..___tag_value_uint128_bswap.20:                                #
..LN13:
   .loc    1  294
        pushq     %rbp                                          #294.1
        movq      %rsp, %rbp                                    #294.1
..___tag_value_uint128_bswap.21:                                #
        subq      $16, %rsp                                     #294.1
        movq      %rdi, -16(%rbp)                               #294.1
..LN15:
   .loc    1  295
        movq      -16(%rbp), %rdi                               #295.0
        movq      (%rdi), %rax                                  #295.0
        movq      8(%rdi), %rcx                                 #295.0
        bswap     %rax                                          #295.0
        bswap     %rcx                                          #295.0
        movq      %rax, 8(%rdi)                                 #295.0
        movq      %rcx, (%rdi)                                  #295.0
..LN17:
   .loc    1  307
        leave                                                   #307.1
..___tag_value_uint128_bswap.25:                                #
        ret                                                     #307.1
        .align    2,0x90
..___tag_value_uint128_bswap.26:                                #
                                # LOE
# mark_end;
	.type	uint128_bswap,@function
	.size	uint128_bswap,.-uint128_bswap
.LNuint128_bswap:
	.data
# -- End  uint128_bswap
	.text
# -- Begin  uint128_bswap_copy
# mark_begin;
       .align    2,0x90
uint128_bswap_copy:
# parameter 1(to): %rdi
# parameter 2(from): %rsi
..B4.1:                         # Preds ..B4.0
..___tag_value_uint128_bswap_copy.29:                           #
..LN19:
   .loc    1  311
        pushq     %rbp                                          #311.1
        movq      %rsp, %rbp                                    #311.1
..___tag_value_uint128_bswap_copy.30:                           #
        subq      $16, %rsp                                     #311.1
        movq      %rdi, -16(%rbp)                               #311.1
        movq      %rsi, -8(%rbp)                                #311.1
..LN21:
   .loc    1  312
        movq      -16(%rbp), %rdi                               #312.0
        movq      -8(%rbp), %rsi                                #312.0
        movq      (%rsi), %rax                                  #312.0
        movq      8(%rsi), %rcx                                 #312.0
        bswap     %rax                                          #312.0
        bswap     %rcx                                          #312.0
        movq      %rax, 8(%rdi)                                 #312.0
        movq      %rcx, (%rdi)                                  #312.0
..LN23:
   .loc    1  324
        leave                                                   #324.1
..___tag_value_uint128_bswap_copy.34:                           #
        ret                                                     #324.1
        .align    2,0x90
..___tag_value_uint128_bswap_copy.35:                           #
                                # LOE
# mark_end;
	.type	uint128_bswap_copy,@function
	.size	uint128_bswap_copy,.-uint128_bswap_copy
.LNuint128_bswap_copy:
	.data
# -- End  uint128_bswap_copy
	.text
# -- Begin  NucStrFastaExprAlloc
# mark_begin;
       .align    2,0x90
NucStrFastaExprAlloc:
# parameter 1(sz): %rdi
..B5.1:                         # Preds ..B5.0
..___tag_value_NucStrFastaExprAlloc.38:                         #
..LN25:
  .file   3 "nucstrstr.c"
   .loc    3  790
        pushq     %rbp                                          #790.1
        movq      %rsp, %rbp                                    #790.1
..___tag_value_NucStrFastaExprAlloc.39:                         #
        subq      $32, %rsp                                     #790.1
        movq      %rdi, -24(%rbp)                               #790.1
..LN27:
   .loc    3  791
        movq      -24(%rbp), %rax                               #791.28
..LN29:
        addq      $16, %rax                                     #791.33
..LN31:
        movq      %rax, %rdi                                    #791.19
        call      malloc@PLT                                    #791.19
                                # LOE rax
..B5.11:                        # Preds ..B5.1
        movq      %rax, -8(%rbp)                                #791.19
                                # LOE
..B5.2:                         # Preds ..B5.11
..LN33:
        movq      -8(%rbp), %rax                                #791.17
        movq      %rax, -16(%rbp)                               #791.17
..LN35:
   .loc    3  792
        movq      -16(%rbp), %rax                               #792.10
..LN37:
        testq     %rax, %rax                                    #792.19
        je        ..B5.8        # Prob 50%                      #792.19
                                # LOE
..B5.3:                         # Preds ..B5.2
..LN39:
   .loc    3  795
        movq      -16(%rbp), %rax                               #795.28
..LN41:
        addq      $15, %rax                                     #795.36
..LN43:
        andq      $-16, %rax                                    #795.43
..LN45:
   .loc    3  794
        movq      %rax, -32(%rbp)                               #794.28
..LN47:
   .loc    3  796
        movq      -32(%rbp), %rax                               #796.9
..LN49:
        movq      -16(%rbp), %rdx                               #796.26
..LN51:
        movq      %rdx, 8(%rax)                                 #796.9
..LN53:
   .loc    3  797
        movq      -32(%rbp), %rax                               #797.9
        addq      $16, %rax                                     #797.9
        testq     $15, %rax                                     #797.9
        je        ..B5.5        # Prob 50%                      #797.9
                                # LOE
..B5.4:                         # Preds ..B5.3
        lea       _2__STRING.0.0(%rip), %rax                    #797.9
        lea       _2__STRING.1.0(%rip), %rdx                    #797.9
        lea       __$U0.0(%rip), %rcx                           #797.9
        movq      %rax, %rdi                                    #797.9
        movq      %rdx, %rsi                                    #797.9
        movl      $797, %edx                                    #797.9
        call      __assert_fail@PLT                             #797.9
                                # LOE
..B5.5:                         # Preds ..B5.3
..LN55:
   .loc    3  798
        movq      -32(%rbp), %rax                               #798.9
        addq      $32, %rax                                     #798.9
        testq     $15, %rax                                     #798.9
        je        ..B5.7        # Prob 50%                      #798.9
                                # LOE
..B5.6:                         # Preds ..B5.5
        lea       _2__STRING.2.0(%rip), %rax                    #798.9
        lea       _2__STRING.1.0(%rip), %rdx                    #798.9
        lea       __$U0.0(%rip), %rcx                           #798.9
        movq      %rax, %rdi                                    #798.9
        movq      %rdx, %rsi                                    #798.9
        movl      $798, %edx                                    #798.9
        call      __assert_fail@PLT                             #798.9
                                # LOE
..B5.7:                         # Preds ..B5.5
..LN57:
   .loc    3  799
        movq      -32(%rbp), %rax                               #799.16
        leave                                                   #799.16
..___tag_value_NucStrFastaExprAlloc.43:                         #
        ret                                                     #799.16
..___tag_value_NucStrFastaExprAlloc.44:                         #
                                # LOE
..B5.8:                         # Preds ..B5.2
..LN59:
   .loc    3  801
        xorl      %eax, %eax                                    #801.12
        leave                                                   #801.12
..___tag_value_NucStrFastaExprAlloc.46:                         #
        ret                                                     #801.12
        .align    2,0x90
..___tag_value_NucStrFastaExprAlloc.47:                         #
                                # LOE
# mark_end;
	.type	NucStrFastaExprAlloc,@function
	.size	NucStrFastaExprAlloc,.-NucStrFastaExprAlloc
.LNNucStrFastaExprAlloc:
	.data
  .file   4 "/opt/intel/Compiler/11.0/081/include/stddef.h"
# -- End  NucStrFastaExprAlloc
	.text
# -- Begin  NucStrFastaExprMake2
# mark_begin;
       .align    2,0x90
NucStrFastaExprMake2:
# parameter 1(expr): %rdi
# parameter 2(positional): %esi
# parameter 3(fasta): %rdx
# parameter 4(size): %rcx
..B6.1:                         # Preds ..B6.0
..___tag_value_NucStrFastaExprMake2.50:                         #
..LN61:
   .loc    3  838
        pushq     %rbp                                          #838.1
        movq      %rsp, %rbp                                    #838.1
..___tag_value_NucStrFastaExprMake2.51:                         #
        subq      $112, %rsp                                    #838.1
        movq      %rdi, -48(%rbp)                               #838.1
        movl      %esi, -16(%rbp)                               #838.1
        movq      %rdx, -40(%rbp)                               #838.1
        movq      %rcx, -32(%rbp)                               #838.1
..LN63:
   .loc    3  844
        movq      -32(%rbp), %rax                               #844.10
..LN65:
        cmpq      $61, %rax                                     #844.17
        jbe       ..B6.3        # Prob 50%                      #844.17
                                # LOE
..B6.2:                         # Preds ..B6.1
..LN67:
   .loc    3  845
        movl      $7, %eax                                      #845.16
        leave                                                   #845.16
..___tag_value_NucStrFastaExprMake2.55:                         #
        ret                                                     #845.16
..___tag_value_NucStrFastaExprMake2.56:                         #
                                # LOE
..B6.3:                         # Preds ..B6.1
..LN69:
   .loc    3  847
        movl      $144, %edi                                    #847.9
        call      NucStrFastaExprAlloc@PLT                      #847.9
                                # LOE rax
..B6.48:                        # Preds ..B6.3
        movq      %rax, -56(%rbp)                               #847.9
                                # LOE
..B6.4:                         # Preds ..B6.48
..LN71:
        movq      -56(%rbp), %rax                               #847.5
        movq      %rax, -64(%rbp)                               #847.5
..LN73:
   .loc    3  848
        movq      -64(%rbp), %rax                               #848.10
..LN75:
        testq     %rax, %rax                                    #848.15
        jne       ..B6.6        # Prob 50%                      #848.15
                                # LOE
..B6.5:                         # Preds ..B6.4
..LN77:
   .loc    3  849
        call      __errno_location@PLT                          #849.16
                                # LOE rax
..B6.49:                        # Preds ..B6.5
        movl      (%rax), %eax                                  #849.16
        leave                                                   #849.16
..___tag_value_NucStrFastaExprMake2.58:                         #
        ret                                                     #849.16
..___tag_value_NucStrFastaExprMake2.59:                         #
                                # LOE
..B6.6:                         # Preds ..B6.4
..LN79:
   .loc    3  851
        movq      -48(%rbp), %rax                               #851.5
..LN81:
        movq      -64(%rbp), %rdx                               #851.14
..LN83:
        movq      %rdx, (%rax)                                  #851.5
..LN85:
   .loc    3  852
        movq      -32(%rbp), %rax                               #852.38
..LN87:
        movq      -64(%rbp), %rdx                               #852.5
        movl      %eax, 4(%rdx)                                 #852.5
..LN89:
   .loc    3  856
        movq      $0, -80(%rbp)                                 #856.11
..LN91:
        movq      -80(%rbp), %rax                               #856.18
..LN93:
        movq      -32(%rbp), %rdx                               #856.22
        cmpq      %rdx, %rax                                    #856.22
        jb        ..B6.9        # Prob 50%                      #856.22
        jmp       ..B6.17       # Prob 100%                     #856.22
                                # LOE
..B6.7:                         # Preds ..B6.13 ..B6.14 ..B6.15 ..B6.16 ..B6.12
                                #      
..LN95:
        incq      -80(%rbp)                                     #856.31
..LN97:
        movq      -80(%rbp), %rax                               #856.18
..LN99:
        movq      -32(%rbp), %rdx                               #856.22
        cmpq      %rdx, %rax                                    #856.22
        jae       ..B6.17       # Prob 50%                      #856.22
                                # LOE
..B6.9:                         # Preds ..B6.6 ..B6.7
..LN101:
   .loc    3  858
        movq      -40(%rbp), %rax                               #858.48
..LN103:
        movq      -80(%rbp), %rdx                               #858.56
..LN105:
        movzbl    (%rdx,%rax), %eax                             #858.48
        movsbq    %al, %rax                                     #858.48
..LN107:
        movslq    %eax, %rax                                    #858.24
        lea       fasta_2na_map.0(%rip), %rdx                   #858.24
        movzbl    (%rax,%rdx), %eax                             #858.24
        movsbq    %al, %rax                                     #858.24
..LN109:
        movb      %al, -24(%rbp)                                #858.22
..LN111:
   .loc    3  859
        movq      -80(%rbp), %rax                               #859.18
..LN113:
        andq      $3, %rax                                      #859.22
        movq      %rax, -72(%rbp)                               #859.22
        je        ..B6.13       # Prob 50%                      #859.22
                                # LOE
..B6.10:                        # Preds ..B6.9
        movq      -72(%rbp), %rax                               #859.22
        cmpq      $1, %rax                                      #859.22
        je        ..B6.14       # Prob 50%                      #859.22
                                # LOE
..B6.11:                        # Preds ..B6.10
        movq      -72(%rbp), %rax                               #859.22
        cmpq      $2, %rax                                      #859.22
        je        ..B6.15       # Prob 50%                      #859.22
                                # LOE
..B6.12:                        # Preds ..B6.11
        movq      -72(%rbp), %rax                               #859.22
        cmpq      $3, %rax                                      #859.22
        je        ..B6.16       # Prob 50%                      #859.22
        jmp       ..B6.7        # Prob 100%                     #859.22
                                # LOE
..B6.13:                        # Preds ..B6.9
..LN115:
   .loc    3  862
        movzbl    -24(%rbp), %eax                               #862.38
..LN117:
        shll      $6, %eax                                      #862.46
..LN119:
        movq      -80(%rbp), %rdx                               #862.27
..LN121:
        shrq      $2, %rdx                                      #862.32
..LN123:
        movb      %al, -112(%rbp,%rdx)                          #862.13
..LN125:
   .loc    3  863
        movq      -80(%rbp), %rax                               #863.24
..LN127:
        shrq      $2, %rax                                      #863.29
..LN129:
        movb      $192, -96(%rbp,%rax)                          #863.13
        jmp       ..B6.7        # Prob 100%                     #863.13
                                # LOE
..B6.14:                        # Preds ..B6.10
..LN131:
   .loc    3  866
        movq      -80(%rbp), %rax                               #866.27
..LN133:
        shrq      $2, %rax                                      #866.32
..LN135:
        movzbl    -112(%rbp,%rax), %eax                         #866.13
..LN137:
        movzbl    -24(%rbp), %edx                               #866.39
..LN139:
        shll      $4, %edx                                      #866.47
..LN141:
        orl       %edx, %eax                                    #866.13
..LN143:
        movq      -80(%rbp), %rdx                               #866.27
..LN145:
        shrq      $2, %rdx                                      #866.32
..LN147:
        movb      %al, -112(%rbp,%rdx)                          #866.13
..LN149:
   .loc    3  867
        movq      -80(%rbp), %rax                               #867.24
..LN151:
        shrq      $2, %rax                                      #867.29
..LN153:
        movzbl    -96(%rbp,%rax), %eax                          #867.13
        orl       $48, %eax                                     #867.13
..LN155:
        movq      -80(%rbp), %rdx                               #867.24
..LN157:
        shrq      $2, %rdx                                      #867.29
..LN159:
        movb      %al, -96(%rbp,%rdx)                           #867.13
        jmp       ..B6.7        # Prob 100%                     #867.13
                                # LOE
..B6.15:                        # Preds ..B6.11
..LN161:
   .loc    3  870
        movq      -80(%rbp), %rax                               #870.27
..LN163:
        shrq      $2, %rax                                      #870.32
..LN165:
        movzbl    -112(%rbp,%rax), %eax                         #870.13
..LN167:
        movzbl    -24(%rbp), %edx                               #870.39
..LN169:
        shll      $2, %edx                                      #870.47
..LN171:
        orl       %edx, %eax                                    #870.13
..LN173:
        movq      -80(%rbp), %rdx                               #870.27
..LN175:
        shrq      $2, %rdx                                      #870.32
..LN177:
        movb      %al, -112(%rbp,%rdx)                          #870.13
..LN179:
   .loc    3  871
        movq      -80(%rbp), %rax                               #871.24
..LN181:
        shrq      $2, %rax                                      #871.29
..LN183:
        movzbl    -96(%rbp,%rax), %eax                          #871.13
        orl       $12, %eax                                     #871.13
..LN185:
        movq      -80(%rbp), %rdx                               #871.24
..LN187:
        shrq      $2, %rdx                                      #871.29
..LN189:
        movb      %al, -96(%rbp,%rdx)                           #871.13
        jmp       ..B6.7        # Prob 100%                     #871.13
                                # LOE
..B6.16:                        # Preds ..B6.12
..LN191:
   .loc    3  874
        movq      -80(%rbp), %rax                               #874.27
..LN193:
        shrq      $2, %rax                                      #874.32
..LN195:
        movzbl    -112(%rbp,%rax), %eax                         #874.13
..LN197:
        movzbl    -24(%rbp), %edx                               #874.39
..LN199:
        orl       %edx, %eax                                    #874.13
..LN201:
        movq      -80(%rbp), %rdx                               #874.27
..LN203:
        shrq      $2, %rdx                                      #874.32
..LN205:
        movb      %al, -112(%rbp,%rdx)                          #874.13
..LN207:
   .loc    3  875
        movq      -80(%rbp), %rax                               #875.24
..LN209:
        shrq      $2, %rax                                      #875.29
..LN211:
        movzbl    -96(%rbp,%rax), %eax                          #875.13
        orl       $3, %eax                                      #875.13
..LN213:
        movq      -80(%rbp), %rdx                               #875.24
..LN215:
        shrq      $2, %rdx                                      #875.29
..LN217:
        movb      %al, -96(%rbp,%rdx)                           #875.13
        jmp       ..B6.7        # Prob 100%                     #875.13
                                # LOE
..B6.17:                        # Preds ..B6.7 ..B6.6
..LN219:
   .loc    3  881
        movq      -80(%rbp), %rax                               #881.17
..LN221:
        addq      $3, %rax                                      #881.21
..LN223:
        shrq      $2, %rax                                      #881.28
..LN225:
        movq      %rax, -80(%rbp)                               #881.11
..LN227:
        movq      -80(%rbp), %rax                               #881.31
..LN229:
        cmpq      $16, %rax                                     #881.35
        jae       ..B6.20       # Prob 50%                      #881.35
                                # LOE
..B6.19:                        # Preds ..B6.17 ..B6.19
..LN231:
   .loc    3  883
        movq      -80(%rbp), %rax                               #883.23
..LN233:
        xorl      %edx, %edx                                    #883.9
        movb      %dl, -112(%rbp,%rax)                          #883.9
..LN235:
   .loc    3  884
        movq      -80(%rbp), %rax                               #884.20
..LN237:
        movb      %dl, -96(%rbp,%rax)                           #884.9
..LN239:
   .loc    3  881
        incq      -80(%rbp)                                     #881.42
..LN241:
        movq      -80(%rbp), %rax                               #881.31
..LN243:
        cmpq      $16, %rax                                     #881.35
        jb        ..B6.19       # Prob 50%                      #881.35
                                # LOE
..B6.20:                        # Preds ..B6.19 ..B6.17
..LN245:
   .loc    3  890
        movl      -16(%rbp), %eax                               #890.10
        testl     %eax, %eax                                    #890.10
        je        ..B6.22       # Prob 50%                      #890.10
                                # LOE
..B6.21:                        # Preds ..B6.20
..LN247:
   .loc    3  892
        movq      -64(%rbp), %rax                               #892.9
        movl      $9, (%rax)                                    #892.9
        jmp       ..B6.31       # Prob 100%                     #892.9
                                # LOE
..B6.22:                        # Preds ..B6.20
..LN249:
   .loc    3  896
        movq      -32(%rbp), %rax                               #896.15
..LN251:
        cmpq      $2, %rax                                      #896.22
        jae       ..B6.24       # Prob 50%                      #896.22
                                # LOE
..B6.23:                        # Preds ..B6.22
..LN253:
   .loc    3  898
        movzbl    -112(%rbp), %eax                              #898.29
..LN255:
        movb      %al, -111(%rbp)                               #898.9
..LN257:
   .loc    3  899
        movzwl    -112(%rbp), %eax                              #899.29
..LN259:
        movw      %ax, -110(%rbp)                               #899.9
..LN261:
   .loc    3  900
        movl      -112(%rbp), %eax                              #900.29
..LN263:
        movl      %eax, -108(%rbp)                              #900.9
..LN265:
   .loc    3  901
        movq      -112(%rbp), %rax                              #901.29
..LN267:
        movq      %rax, -104(%rbp)                              #901.9
..LN269:
   .loc    3  903
        movzbl    -96(%rbp), %eax                               #903.26
..LN271:
        movb      %al, -95(%rbp)                                #903.9
..LN273:
   .loc    3  904
        movzwl    -96(%rbp), %eax                               #904.26
..LN275:
        movw      %ax, -94(%rbp)                                #904.9
..LN277:
   .loc    3  905
        movl      -96(%rbp), %eax                               #905.26
..LN279:
        movl      %eax, -92(%rbp)                               #905.9
..LN281:
   .loc    3  906
        movq      -96(%rbp), %rax                               #906.26
..LN283:
        movq      %rax, -88(%rbp)                               #906.9
..LN285:
   .loc    3  908
        movq      -64(%rbp), %rax                               #908.9
        movl      $2, (%rax)                                    #908.9
        jmp       ..B6.31       # Prob 100%                     #908.9
                                # LOE
..B6.24:                        # Preds ..B6.22
..LN287:
   .loc    3  910
        movq      -32(%rbp), %rax                               #910.15
..LN289:
        cmpq      $6, %rax                                      #910.22
        jae       ..B6.26       # Prob 50%                      #910.22
                                # LOE
..B6.25:                        # Preds ..B6.24
..LN291:
   .loc    3  912
        movzwl    -112(%rbp), %eax                              #912.29
..LN293:
        movw      %ax, -110(%rbp)                               #912.9
..LN295:
   .loc    3  913
        movl      -112(%rbp), %eax                              #913.29
..LN297:
        movl      %eax, -108(%rbp)                              #913.9
..LN299:
   .loc    3  914
        movq      -112(%rbp), %rax                              #914.29
..LN301:
        movq      %rax, -104(%rbp)                              #914.9
..LN303:
   .loc    3  916
        movzwl    -96(%rbp), %eax                               #916.26
..LN305:
        movw      %ax, -94(%rbp)                                #916.9
..LN307:
   .loc    3  917
        movl      -96(%rbp), %eax                               #917.26
..LN309:
        movl      %eax, -92(%rbp)                               #917.9
..LN311:
   .loc    3  918
        movq      -96(%rbp), %rax                               #918.26
..LN313:
        movq      %rax, -88(%rbp)                               #918.9
..LN315:
   .loc    3  920
        movq      -64(%rbp), %rax                               #920.9
        movl      $3, (%rax)                                    #920.9
        jmp       ..B6.31       # Prob 100%                     #920.9
                                # LOE
..B6.26:                        # Preds ..B6.24
..LN317:
   .loc    3  922
        movq      -32(%rbp), %rax                               #922.15
..LN319:
        cmpq      $14, %rax                                     #922.22
        jae       ..B6.28       # Prob 50%                      #922.22
                                # LOE
..B6.27:                        # Preds ..B6.26
..LN321:
   .loc    3  924
        movl      -112(%rbp), %eax                              #924.29
..LN323:
        movl      %eax, -108(%rbp)                              #924.9
..LN325:
   .loc    3  925
        movq      -112(%rbp), %rax                              #925.29
..LN327:
        movq      %rax, -104(%rbp)                              #925.9
..LN329:
   .loc    3  927
        movl      -96(%rbp), %eax                               #927.26
..LN331:
        movl      %eax, -92(%rbp)                               #927.9
..LN333:
   .loc    3  928
        movq      -96(%rbp), %rax                               #928.26
..LN335:
        movq      %rax, -88(%rbp)                               #928.9
..LN337:
   .loc    3  930
        movq      -64(%rbp), %rax                               #930.9
        movl      $4, (%rax)                                    #930.9
        jmp       ..B6.31       # Prob 100%                     #930.9
                                # LOE
..B6.28:                        # Preds ..B6.26
..LN339:
   .loc    3  932
        movq      -32(%rbp), %rax                               #932.15
..LN341:
        cmpq      $30, %rax                                     #932.22
        jae       ..B6.30       # Prob 50%                      #932.22
                                # LOE
..B6.29:                        # Preds ..B6.28
..LN343:
   .loc    3  934
        movq      -112(%rbp), %rax                              #934.29
..LN345:
        movq      %rax, -104(%rbp)                              #934.9
..LN347:
   .loc    3  935
        movq      -96(%rbp), %rax                               #935.26
..LN349:
        movq      %rax, -88(%rbp)                               #935.9
..LN351:
   .loc    3  937
        movq      -64(%rbp), %rax                               #937.9
        movl      $0, (%rax)                                    #937.9
        jmp       ..B6.31       # Prob 100%                     #937.9
                                # LOE
..B6.30:                        # Preds ..B6.28
..LN353:
   .loc    3  941
        movq      -64(%rbp), %rax                               #941.9
        movl      $5, (%rax)                                    #941.9
                                # LOE
..B6.31:                        # Preds ..B6.21 ..B6.23 ..B6.25 ..B6.27 ..B6.29
                                #       ..B6.30
..LN355:
   .loc    3  944
        movq      -64(%rbp), %rax                               #944.5
..LN357:
        movq      -112(%rbp), %rdx                              #944.42
        movq      %rdx, 16(%rax)                                #944.42
        movq      -104(%rbp), %rdx                              #944.42
        movq      %rdx, 24(%rax)                                #944.42
..LN359:
   .loc    3  945
        movq      -64(%rbp), %rax                               #945.5
..LN361:
        movq      -96(%rbp), %rdx                               #945.39
        movq      %rdx, 32(%rax)                                #945.39
        movq      -88(%rbp), %rdx                               #945.39
        movq      %rdx, 40(%rax)                                #945.39
..LN363:
   .loc    3  948
        lea       -112(%rbp), %rax                              #948.23
..LN365:
        movq      %rax, %rdi                                    #948.5
        call      uint128_bswap@PLT                             #948.5
                                # LOE
..B6.32:                        # Preds ..B6.31
..LN367:
   .loc    3  949
        lea       -96(%rbp), %rax                               #949.23
..LN369:
        movq      %rax, %rdi                                    #949.5
        call      uint128_bswap@PLT                             #949.5
                                # LOE
..B6.33:                        # Preds ..B6.32
..LN371:
   .loc    3  952
        lea       -112(%rbp), %rax                              #952.21
..LN373:
        movq      %rax, %rdi                                    #952.5
        movl      $2, %esi                                      #952.5
        call      uint128_shr@PLT                               #952.5
                                # LOE
..B6.34:                        # Preds ..B6.33
..LN375:
   .loc    3  953
        lea       -96(%rbp), %rax                               #953.21
..LN377:
        movq      %rax, %rdi                                    #953.5
        movl      $2, %esi                                      #953.5
        call      uint128_shr@PLT                               #953.5
                                # LOE
..B6.35:                        # Preds ..B6.34
..LN379:
   .loc    3  956
        movq      -64(%rbp), %rax                               #956.28
..LN381:
        addq      $48, %rax                                     #956.5
..LN383:
        lea       -112(%rbp), %rdx                              #956.70
..LN385:
        movq      %rax, %rdi                                    #956.5
        movq      %rdx, %rsi                                    #956.5
        call      uint128_bswap_copy@PLT                        #956.5
                                # LOE
..B6.36:                        # Preds ..B6.35
..LN387:
   .loc    3  957
        movq      -64(%rbp), %rax                               #957.28
..LN389:
        addq      $64, %rax                                     #957.5
..LN391:
        lea       -96(%rbp), %rdx                               #957.67
..LN393:
        movq      %rax, %rdi                                    #957.5
        movq      %rdx, %rsi                                    #957.5
        call      uint128_bswap_copy@PLT                        #957.5
                                # LOE
..B6.37:                        # Preds ..B6.36
..LN395:
   .loc    3  959
        lea       -112(%rbp), %rax                              #959.21
..LN397:
        movq      %rax, %rdi                                    #959.5
        movl      $2, %esi                                      #959.5
        call      uint128_shr@PLT                               #959.5
                                # LOE
..B6.38:                        # Preds ..B6.37
..LN399:
   .loc    3  960
        lea       -96(%rbp), %rax                               #960.21
..LN401:
        movq      %rax, %rdi                                    #960.5
        movl      $2, %esi                                      #960.5
        call      uint128_shr@PLT                               #960.5
                                # LOE
..B6.39:                        # Preds ..B6.38
..LN403:
   .loc    3  962
        movq      -64(%rbp), %rax                               #962.28
..LN405:
        addq      $80, %rax                                     #962.5
..LN407:
        lea       -112(%rbp), %rdx                              #962.70
..LN409:
        movq      %rax, %rdi                                    #962.5
        movq      %rdx, %rsi                                    #962.5
        call      uint128_bswap_copy@PLT                        #962.5
                                # LOE
..B6.40:                        # Preds ..B6.39
..LN411:
   .loc    3  963
        movq      -64(%rbp), %rax                               #963.28
..LN413:
        addq      $96, %rax                                     #963.5
..LN415:
        lea       -96(%rbp), %rdx                               #963.67
..LN417:
        movq      %rax, %rdi                                    #963.5
        movq      %rdx, %rsi                                    #963.5
        call      uint128_bswap_copy@PLT                        #963.5
                                # LOE
..B6.41:                        # Preds ..B6.40
..LN419:
   .loc    3  965
        lea       -112(%rbp), %rax                              #965.21
..LN421:
        movq      %rax, %rdi                                    #965.5
        movl      $2, %esi                                      #965.5
        call      uint128_shr@PLT                               #965.5
                                # LOE
..B6.42:                        # Preds ..B6.41
..LN423:
   .loc    3  966
        lea       -96(%rbp), %rax                               #966.21
..LN425:
        movq      %rax, %rdi                                    #966.5
        movl      $2, %esi                                      #966.5
        call      uint128_shr@PLT                               #966.5
                                # LOE
..B6.43:                        # Preds ..B6.42
..LN427:
   .loc    3  968
        movq      -64(%rbp), %rax                               #968.28
..LN429:
        addq      $112, %rax                                    #968.5
..LN431:
        lea       -112(%rbp), %rdx                              #968.70
..LN433:
        movq      %rax, %rdi                                    #968.5
        movq      %rdx, %rsi                                    #968.5
        call      uint128_bswap_copy@PLT                        #968.5
                                # LOE
..B6.44:                        # Preds ..B6.43
..LN435:
   .loc    3  969
        movq      -64(%rbp), %rax                               #969.28
..LN437:
        addq      $128, %rax                                    #969.5
..LN439:
        lea       -96(%rbp), %rdx                               #969.67
..LN441:
        movq      %rax, %rdi                                    #969.5
        movq      %rdx, %rsi                                    #969.5
        call      uint128_bswap_copy@PLT                        #969.5
                                # LOE
..B6.45:                        # Preds ..B6.44
..LN443:
   .loc    3  1011
        xorl      %eax, %eax                                    #1011.12
        leave                                                   #1011.12
..___tag_value_NucStrFastaExprMake2.61:                         #
        ret                                                     #1011.12
        .align    2,0x90
..___tag_value_NucStrFastaExprMake2.62:                         #
                                # LOE
# mark_end;
	.type	NucStrFastaExprMake2,@function
	.size	NucStrFastaExprMake2,.-NucStrFastaExprMake2
.LNNucStrFastaExprMake2:
	.data
  .file   5 "/home/yaschenk/devel/internal/asm-trace/itf/sra/nucstrstr.h"
# -- End  NucStrFastaExprMake2
	.text
# -- Begin  NucStrFastaExprMake4
# mark_begin;
       .align    2,0x90
NucStrFastaExprMake4:
# parameter 1(expr): %rdi
# parameter 2(positional): %esi
# parameter 3(fasta): %rdx
# parameter 4(size): %rcx
..B7.1:                         # Preds ..B7.0
..___tag_value_NucStrFastaExprMake4.65:                         #
..LN445:
   .loc    3  1017
        pushq     %rbp                                          #1017.1
        movq      %rsp, %rbp                                    #1017.1
..___tag_value_NucStrFastaExprMake4.66:                         #
        subq      $112, %rsp                                    #1017.1
        movq      %rdi, -40(%rbp)                               #1017.1
        movl      %esi, -16(%rbp)                               #1017.1
        movq      %rdx, -32(%rbp)                               #1017.1
        movq      %rcx, -24(%rbp)                               #1017.1
..LN447:
   .loc    3  1023
        movq      -24(%rbp), %rax                               #1023.10
..LN449:
        cmpq      $29, %rax                                     #1023.17
        jbe       ..B7.3        # Prob 50%                      #1023.17
                                # LOE
..B7.2:                         # Preds ..B7.1
..LN451:
   .loc    3  1024
        movl      $7, %eax                                      #1024.16
        leave                                                   #1024.16
..___tag_value_NucStrFastaExprMake4.70:                         #
        ret                                                     #1024.16
..___tag_value_NucStrFastaExprMake4.71:                         #
                                # LOE
..B7.3:                         # Preds ..B7.1
..LN453:
   .loc    3  1026
        movl      $144, %edi                                    #1026.9
        call      NucStrFastaExprAlloc@PLT                      #1026.9
                                # LOE rax
..B7.46:                        # Preds ..B7.3
        movq      %rax, -56(%rbp)                               #1026.9
                                # LOE
..B7.4:                         # Preds ..B7.46
..LN455:
        movq      -56(%rbp), %rax                               #1026.5
        movq      %rax, -64(%rbp)                               #1026.5
..LN457:
   .loc    3  1027
        movq      -64(%rbp), %rax                               #1027.10
..LN459:
        testq     %rax, %rax                                    #1027.15
        jne       ..B7.6        # Prob 50%                      #1027.15
                                # LOE
..B7.5:                         # Preds ..B7.4
..LN461:
   .loc    3  1028
        call      __errno_location@PLT                          #1028.16
                                # LOE rax
..B7.47:                        # Preds ..B7.5
        movl      (%rax), %eax                                  #1028.16
        leave                                                   #1028.16
..___tag_value_NucStrFastaExprMake4.73:                         #
        ret                                                     #1028.16
..___tag_value_NucStrFastaExprMake4.74:                         #
                                # LOE
..B7.6:                         # Preds ..B7.4
..LN463:
   .loc    3  1030
        movq      -40(%rbp), %rax                               #1030.5
..LN465:
        movq      -64(%rbp), %rdx                               #1030.14
..LN467:
        movq      %rdx, (%rax)                                  #1030.5
..LN469:
   .loc    3  1031
        movq      -24(%rbp), %rax                               #1031.38
..LN471:
        movq      -64(%rbp), %rdx                               #1031.5
        movl      %eax, 4(%rdx)                                 #1031.5
..LN473:
   .loc    3  1035
        movq      $0, -80(%rbp)                                 #1035.11
..LN475:
        movq      -80(%rbp), %rax                               #1035.18
..LN477:
        movq      -24(%rbp), %rdx                               #1035.22
        cmpq      %rdx, %rax                                    #1035.22
        jb        ..B7.9        # Prob 50%                      #1035.22
        jmp       ..B7.17       # Prob 100%                     #1035.22
                                # LOE
..B7.7:                         # Preds ..B7.13 ..B7.14 ..B7.15 ..B7.16 ..B7.12
                                #      
..LN479:
        incq      -80(%rbp)                                     #1035.31
..LN481:
        movq      -80(%rbp), %rax                               #1035.18
..LN483:
        movq      -24(%rbp), %rdx                               #1035.22
        cmpq      %rdx, %rax                                    #1035.22
        jae       ..B7.17       # Prob 50%                      #1035.22
                                # LOE
..B7.9:                         # Preds ..B7.6 ..B7.7
..LN485:
   .loc    3  1037
        movq      -32(%rbp), %rax                               #1037.49
..LN487:
        movq      -80(%rbp), %rdx                               #1037.57
..LN489:
        movzbl    (%rdx,%rax), %eax                             #1037.49
        movsbq    %al, %rax                                     #1037.49
..LN491:
        movslq    %eax, %rax                                    #1037.25
        lea       fasta_4na_map.0(%rip), %rdx                   #1037.25
        movzbl    (%rax,%rdx), %eax                             #1037.25
        movsbq    %al, %rax                                     #1037.25
..LN493:
        movw      %ax, -48(%rbp)                                #1037.23
..LN495:
   .loc    3  1038
        movq      -80(%rbp), %rax                               #1038.18
..LN497:
        andq      $3, %rax                                      #1038.22
        movq      %rax, -72(%rbp)                               #1038.22
        je        ..B7.13       # Prob 50%                      #1038.22
                                # LOE
..B7.10:                        # Preds ..B7.9
        movq      -72(%rbp), %rax                               #1038.22
        cmpq      $1, %rax                                      #1038.22
        je        ..B7.14       # Prob 50%                      #1038.22
                                # LOE
..B7.11:                        # Preds ..B7.10
        movq      -72(%rbp), %rax                               #1038.22
        cmpq      $2, %rax                                      #1038.22
        je        ..B7.15       # Prob 50%                      #1038.22
                                # LOE
..B7.12:                        # Preds ..B7.11
        movq      -72(%rbp), %rax                               #1038.22
        cmpq      $3, %rax                                      #1038.22
        je        ..B7.16       # Prob 50%                      #1038.22
        jmp       ..B7.7        # Prob 100%                     #1038.22
                                # LOE
..B7.13:                        # Preds ..B7.9
..LN499:
   .loc    3  1041
        movzwl    -48(%rbp), %eax                               #1041.38
..LN501:
        shll      $4, %eax                                      #1041.46
..LN503:
        movq      -80(%rbp), %rdx                               #1041.27
..LN505:
        shrq      $2, %rdx                                      #1041.32
..LN507:
        movw      %ax, -112(%rbp,%rdx,2)                        #1041.13
..LN509:
   .loc    3  1042
        movq      -80(%rbp), %rax                               #1042.24
..LN511:
        shrq      $2, %rax                                      #1042.29
..LN513:
        movw      $240, -96(%rbp,%rax,2)                        #1042.13
        jmp       ..B7.7        # Prob 100%                     #1042.13
                                # LOE
..B7.14:                        # Preds ..B7.10
..LN515:
   .loc    3  1045
        movq      -80(%rbp), %rax                               #1045.27
..LN517:
        shrq      $2, %rax                                      #1045.32
..LN519:
        movzwl    -112(%rbp,%rax,2), %eax                       #1045.13
..LN521:
        movzwl    -48(%rbp), %edx                               #1045.39
..LN523:
        orl       %edx, %eax                                    #1045.13
..LN525:
        movq      -80(%rbp), %rdx                               #1045.27
..LN527:
        shrq      $2, %rdx                                      #1045.32
..LN529:
        movw      %ax, -112(%rbp,%rdx,2)                        #1045.13
..LN531:
   .loc    3  1046
        movq      -80(%rbp), %rax                               #1046.24
..LN533:
        shrq      $2, %rax                                      #1046.29
..LN535:
        movzwl    -96(%rbp,%rax,2), %eax                        #1046.13
        orl       $15, %eax                                     #1046.13
..LN537:
        movq      -80(%rbp), %rdx                               #1046.24
..LN539:
        shrq      $2, %rdx                                      #1046.29
..LN541:
        movw      %ax, -96(%rbp,%rdx,2)                         #1046.13
        jmp       ..B7.7        # Prob 100%                     #1046.13
                                # LOE
..B7.15:                        # Preds ..B7.11
..LN543:
   .loc    3  1049
        movq      -80(%rbp), %rax                               #1049.27
..LN545:
        shrq      $2, %rax                                      #1049.32
..LN547:
        movzwl    -112(%rbp,%rax,2), %eax                       #1049.13
..LN549:
        movzwl    -48(%rbp), %edx                               #1049.39
..LN551:
        shll      $12, %edx                                     #1049.47
..LN553:
        orl       %edx, %eax                                    #1049.13
..LN555:
        movq      -80(%rbp), %rdx                               #1049.27
..LN557:
        shrq      $2, %rdx                                      #1049.32
..LN559:
        movw      %ax, -112(%rbp,%rdx,2)                        #1049.13
..LN561:
   .loc    3  1050
        movq      -80(%rbp), %rax                               #1050.24
..LN563:
        shrq      $2, %rax                                      #1050.29
..LN565:
        movzwl    -96(%rbp,%rax,2), %eax                        #1050.13
        orl       $-4096, %eax                                  #1050.13
..LN567:
        movq      -80(%rbp), %rdx                               #1050.24
..LN569:
        shrq      $2, %rdx                                      #1050.29
..LN571:
        movw      %ax, -96(%rbp,%rdx,2)                         #1050.13
        jmp       ..B7.7        # Prob 100%                     #1050.13
                                # LOE
..B7.16:                        # Preds ..B7.12
..LN573:
   .loc    3  1053
        movq      -80(%rbp), %rax                               #1053.27
..LN575:
        shrq      $2, %rax                                      #1053.32
..LN577:
        movzwl    -112(%rbp,%rax,2), %eax                       #1053.13
..LN579:
        movzwl    -48(%rbp), %edx                               #1053.39
..LN581:
        shll      $8, %edx                                      #1053.47
..LN583:
        orl       %edx, %eax                                    #1053.13
..LN585:
        movq      -80(%rbp), %rdx                               #1053.27
..LN587:
        shrq      $2, %rdx                                      #1053.32
..LN589:
        movw      %ax, -112(%rbp,%rdx,2)                        #1053.13
..LN591:
   .loc    3  1054
        movq      -80(%rbp), %rax                               #1054.24
..LN593:
        shrq      $2, %rax                                      #1054.29
..LN595:
        movzwl    -96(%rbp,%rax,2), %eax                        #1054.13
        orl       $-61696, %eax                                 #1054.13
..LN597:
        movq      -80(%rbp), %rdx                               #1054.24
..LN599:
        shrq      $2, %rdx                                      #1054.29
..LN601:
        movw      %ax, -96(%rbp,%rdx,2)                         #1054.13
        jmp       ..B7.7        # Prob 100%                     #1054.13
                                # LOE
..B7.17:                        # Preds ..B7.7 ..B7.6
..LN603:
   .loc    3  1060
        movq      -80(%rbp), %rax                               #1060.17
..LN605:
        addq      $3, %rax                                      #1060.21
..LN607:
        shrq      $2, %rax                                      #1060.28
..LN609:
        movq      %rax, -80(%rbp)                               #1060.11
..LN611:
        movq      -80(%rbp), %rax                               #1060.31
..LN613:
        cmpq      $8, %rax                                      #1060.35
        jae       ..B7.20       # Prob 50%                      #1060.35
                                # LOE
..B7.19:                        # Preds ..B7.17 ..B7.19
..LN615:
   .loc    3  1062
        movq      -80(%rbp), %rax                               #1062.23
..LN617:
        xorl      %edx, %edx                                    #1062.9
        movw      %dx, -112(%rbp,%rax,2)                        #1062.9
..LN619:
   .loc    3  1063
        movq      -80(%rbp), %rax                               #1063.20
..LN621:
        movw      %dx, -96(%rbp,%rax,2)                         #1063.9
..LN623:
   .loc    3  1060
        incq      -80(%rbp)                                     #1060.41
..LN625:
        movq      -80(%rbp), %rax                               #1060.31
..LN627:
        cmpq      $8, %rax                                      #1060.35
        jb        ..B7.19       # Prob 50%                      #1060.35
                                # LOE
..B7.20:                        # Preds ..B7.19 ..B7.17
..LN629:
   .loc    3  1068
        movl      -16(%rbp), %eax                               #1068.10
        testl     %eax, %eax                                    #1068.10
        je        ..B7.22       # Prob 50%                      #1068.10
                                # LOE
..B7.21:                        # Preds ..B7.20
..LN631:
   .loc    3  1070
        movq      -64(%rbp), %rax                               #1070.9
        movl      $10, (%rax)                                   #1070.9
        jmp       ..B7.29       # Prob 100%                     #1070.9
                                # LOE
..B7.22:                        # Preds ..B7.20
..LN633:
   .loc    3  1072
        movq      -24(%rbp), %rax                               #1072.15
..LN635:
        cmpq      $2, %rax                                      #1072.22
        jae       ..B7.24       # Prob 50%                      #1072.22
                                # LOE
..B7.23:                        # Preds ..B7.22
..LN637:
   .loc    3  1074
        movzwl    -112(%rbp), %eax                              #1074.29
..LN639:
        movw      %ax, -110(%rbp)                               #1074.9
..LN641:
   .loc    3  1075
        movl      -112(%rbp), %eax                              #1075.29
..LN643:
        movl      %eax, -108(%rbp)                              #1075.9
..LN645:
   .loc    3  1076
        movq      -112(%rbp), %rax                              #1076.29
..LN647:
        movq      %rax, -104(%rbp)                              #1076.9
..LN649:
   .loc    3  1078
        movzwl    -96(%rbp), %eax                               #1078.26
..LN651:
        movw      %ax, -94(%rbp)                                #1078.9
..LN653:
   .loc    3  1079
        movl      -96(%rbp), %eax                               #1079.26
..LN655:
        movl      %eax, -92(%rbp)                               #1079.9
..LN657:
   .loc    3  1080
        movq      -96(%rbp), %rax                               #1080.26
..LN659:
        movq      %rax, -88(%rbp)                               #1080.9
..LN661:
   .loc    3  1082
        movq      -64(%rbp), %rax                               #1082.9
        movl      $6, (%rax)                                    #1082.9
        jmp       ..B7.29       # Prob 100%                     #1082.9
                                # LOE
..B7.24:                        # Preds ..B7.22
..LN663:
   .loc    3  1084
        movq      -24(%rbp), %rax                               #1084.15
..LN665:
        cmpq      $6, %rax                                      #1084.22
        jae       ..B7.26       # Prob 50%                      #1084.22
                                # LOE
..B7.25:                        # Preds ..B7.24
..LN667:
   .loc    3  1086
        movl      -112(%rbp), %eax                              #1086.29
..LN669:
        movl      %eax, -108(%rbp)                              #1086.9
..LN671:
   .loc    3  1087
        movq      -112(%rbp), %rax                              #1087.29
..LN673:
        movq      %rax, -104(%rbp)                              #1087.9
..LN675:
   .loc    3  1089
        movl      -96(%rbp), %eax                               #1089.26
..LN677:
        movl      %eax, -92(%rbp)                               #1089.9
..LN679:
   .loc    3  1090
        movq      -96(%rbp), %rax                               #1090.26
..LN681:
        movq      %rax, -88(%rbp)                               #1090.9
..LN683:
   .loc    3  1092
        movq      -64(%rbp), %rax                               #1092.9
        movl      $7, (%rax)                                    #1092.9
        jmp       ..B7.29       # Prob 100%                     #1092.9
                                # LOE
..B7.26:                        # Preds ..B7.24
..LN685:
   .loc    3  1094
        movq      -24(%rbp), %rax                               #1094.15
..LN687:
        cmpq      $14, %rax                                     #1094.22
        jae       ..B7.28       # Prob 50%                      #1094.22
                                # LOE
..B7.27:                        # Preds ..B7.26
..LN689:
   .loc    3  1096
        movq      -112(%rbp), %rax                              #1096.29
..LN691:
        movq      %rax, -104(%rbp)                              #1096.9
..LN693:
   .loc    3  1097
        movq      -96(%rbp), %rax                               #1097.26
..LN695:
        movq      %rax, -88(%rbp)                               #1097.9
..LN697:
   .loc    3  1099
        movq      -64(%rbp), %rax                               #1099.9
        movl      $1, (%rax)                                    #1099.9
        jmp       ..B7.29       # Prob 100%                     #1099.9
                                # LOE
..B7.28:                        # Preds ..B7.26
..LN699:
   .loc    3  1103
        movq      -64(%rbp), %rax                               #1103.9
        movl      $8, (%rax)                                    #1103.9
                                # LOE
..B7.29:                        # Preds ..B7.21 ..B7.23 ..B7.25 ..B7.27 ..B7.28
                                #      
..LN701:
   .loc    3  1106
        movq      -64(%rbp), %rax                               #1106.5
..LN703:
        movq      -112(%rbp), %rdx                              #1106.42
        movq      %rdx, 16(%rax)                                #1106.42
        movq      -104(%rbp), %rdx                              #1106.42
        movq      %rdx, 24(%rax)                                #1106.42
..LN705:
   .loc    3  1107
        movq      -64(%rbp), %rax                               #1107.5
..LN707:
        movq      -96(%rbp), %rdx                               #1107.39
        movq      %rdx, 32(%rax)                                #1107.39
        movq      -88(%rbp), %rdx                               #1107.39
        movq      %rdx, 40(%rax)                                #1107.39
..LN709:
   .loc    3  1110
        lea       -112(%rbp), %rax                              #1110.23
..LN711:
        movq      %rax, %rdi                                    #1110.5
        call      uint128_bswap@PLT                             #1110.5
                                # LOE
..B7.30:                        # Preds ..B7.29
..LN713:
   .loc    3  1111
        lea       -96(%rbp), %rax                               #1111.23
..LN715:
        movq      %rax, %rdi                                    #1111.5
        call      uint128_bswap@PLT                             #1111.5
                                # LOE
..B7.31:                        # Preds ..B7.30
..LN717:
   .loc    3  1114
        lea       -112(%rbp), %rax                              #1114.21
..LN719:
        movq      %rax, %rdi                                    #1114.5
        movl      $4, %esi                                      #1114.5
        call      uint128_shr@PLT                               #1114.5
                                # LOE
..B7.32:                        # Preds ..B7.31
..LN721:
   .loc    3  1115
        lea       -96(%rbp), %rax                               #1115.21
..LN723:
        movq      %rax, %rdi                                    #1115.5
        movl      $4, %esi                                      #1115.5
        call      uint128_shr@PLT                               #1115.5
                                # LOE
..B7.33:                        # Preds ..B7.32
..LN725:
   .loc    3  1118
        movq      -64(%rbp), %rax                               #1118.28
..LN727:
        addq      $48, %rax                                     #1118.5
..LN729:
        lea       -112(%rbp), %rdx                              #1118.70
..LN731:
        movq      %rax, %rdi                                    #1118.5
        movq      %rdx, %rsi                                    #1118.5
        call      uint128_bswap_copy@PLT                        #1118.5
                                # LOE
..B7.34:                        # Preds ..B7.33
..LN733:
   .loc    3  1119
        movq      -64(%rbp), %rax                               #1119.28
..LN735:
        addq      $64, %rax                                     #1119.5
..LN737:
        lea       -96(%rbp), %rdx                               #1119.67
..LN739:
        movq      %rax, %rdi                                    #1119.5
        movq      %rdx, %rsi                                    #1119.5
        call      uint128_bswap_copy@PLT                        #1119.5
                                # LOE
..B7.35:                        # Preds ..B7.34
..LN741:
   .loc    3  1121
        lea       -112(%rbp), %rax                              #1121.21
..LN743:
        movq      %rax, %rdi                                    #1121.5
        movl      $4, %esi                                      #1121.5
        call      uint128_shr@PLT                               #1121.5
                                # LOE
..B7.36:                        # Preds ..B7.35
..LN745:
   .loc    3  1122
        lea       -96(%rbp), %rax                               #1122.21
..LN747:
        movq      %rax, %rdi                                    #1122.5
        movl      $4, %esi                                      #1122.5
        call      uint128_shr@PLT                               #1122.5
                                # LOE
..B7.37:                        # Preds ..B7.36
..LN749:
   .loc    3  1124
        movq      -64(%rbp), %rax                               #1124.28
..LN751:
        addq      $80, %rax                                     #1124.5
..LN753:
        lea       -112(%rbp), %rdx                              #1124.70
..LN755:
        movq      %rax, %rdi                                    #1124.5
        movq      %rdx, %rsi                                    #1124.5
        call      uint128_bswap_copy@PLT                        #1124.5
                                # LOE
..B7.38:                        # Preds ..B7.37
..LN757:
   .loc    3  1125
        movq      -64(%rbp), %rax                               #1125.28
..LN759:
        addq      $96, %rax                                     #1125.5
..LN761:
        lea       -96(%rbp), %rdx                               #1125.67
..LN763:
        movq      %rax, %rdi                                    #1125.5
        movq      %rdx, %rsi                                    #1125.5
        call      uint128_bswap_copy@PLT                        #1125.5
                                # LOE
..B7.39:                        # Preds ..B7.38
..LN765:
   .loc    3  1127
        lea       -112(%rbp), %rax                              #1127.21
..LN767:
        movq      %rax, %rdi                                    #1127.5
        movl      $4, %esi                                      #1127.5
        call      uint128_shr@PLT                               #1127.5
                                # LOE
..B7.40:                        # Preds ..B7.39
..LN769:
   .loc    3  1128
        lea       -96(%rbp), %rax                               #1128.21
..LN771:
        movq      %rax, %rdi                                    #1128.5
        movl      $4, %esi                                      #1128.5
        call      uint128_shr@PLT                               #1128.5
                                # LOE
..B7.41:                        # Preds ..B7.40
..LN773:
   .loc    3  1130
        movq      -64(%rbp), %rax                               #1130.28
..LN775:
        addq      $112, %rax                                    #1130.5
..LN777:
        lea       -112(%rbp), %rdx                              #1130.70
..LN779:
        movq      %rax, %rdi                                    #1130.5
        movq      %rdx, %rsi                                    #1130.5
        call      uint128_bswap_copy@PLT                        #1130.5
                                # LOE
..B7.42:                        # Preds ..B7.41
..LN781:
   .loc    3  1131
        movq      -64(%rbp), %rax                               #1131.28
..LN783:
        addq      $128, %rax                                    #1131.5
..LN785:
        lea       -96(%rbp), %rdx                               #1131.67
..LN787:
        movq      %rax, %rdi                                    #1131.5
        movq      %rdx, %rsi                                    #1131.5
        call      uint128_bswap_copy@PLT                        #1131.5
                                # LOE
..B7.43:                        # Preds ..B7.42
..LN789:
   .loc    3  1173
        xorl      %eax, %eax                                    #1173.12
        leave                                                   #1173.12
..___tag_value_NucStrFastaExprMake4.76:                         #
        ret                                                     #1173.12
        .align    2,0x90
..___tag_value_NucStrFastaExprMake4.77:                         #
                                # LOE
# mark_end;
	.type	NucStrFastaExprMake4,@function
	.size	NucStrFastaExprMake4,.-NucStrFastaExprMake4
.LNNucStrFastaExprMake4:
	.data
# -- End  NucStrFastaExprMake4
	.text
# -- Begin  nss_sob
# mark_begin;
       .align    2,0x90
nss_sob:
# parameter 1(p): %rdi
# parameter 2(end): %rsi
..B8.1:                         # Preds ..B8.0
..___tag_value_nss_sob.80:                                      #
..LN791:
   .loc    3  1181
        pushq     %rbp                                          #1181.1
        movq      %rsp, %rbp                                    #1181.1
..___tag_value_nss_sob.81:                                      #
        subq      $16, %rsp                                     #1181.1
        movq      %rdi, -16(%rbp)                               #1181.1
        movq      %rsi, -8(%rbp)                                #1181.1
..LN793:
   .loc    3  1182
        movq      -16(%rbp), %rax                               #1182.13
..LN795:
        movq      -8(%rbp), %rdx                                #1182.17
        cmpq      %rdx, %rax                                    #1182.17
        jae       ..B8.5        # Prob 50%                      #1182.17
                                # LOE
..B8.3:                         # Preds ..B8.1 ..B8.4
..LN797:
        call      __ctype_b_loc@PLT                             #1182.24
                                # LOE rax
..B8.8:                         # Preds ..B8.3
        movq      -16(%rbp), %rdx                               #1182.24
        movzbl    (%rdx), %edx                                  #1182.24
        movzbl    %dl, %edx                                     #1182.24
        movslq    %edx, %rdx                                    #1182.24
        movq      (%rax), %rax                                  #1182.24
        movzwl    (%rax,%rdx,2), %eax                           #1182.24
        testl     $8192, %eax                                   #1182.24
        je        ..B8.5        # Prob 50%                      #1182.24
                                # LOE
..B8.4:                         # Preds ..B8.8
..LN799:
   .loc    3  1183
        incq      -16(%rbp)                                     #1183.12
..LN801:
   .loc    3  1182
        movq      -16(%rbp), %rax                               #1182.13
..LN803:
        movq      -8(%rbp), %rdx                                #1182.17
        cmpq      %rdx, %rax                                    #1182.17
        jb        ..B8.3        # Prob 50%                      #1182.17
                                # LOE
..B8.5:                         # Preds ..B8.4 ..B8.8 ..B8.1
..LN805:
   .loc    3  1184
        movq      -16(%rbp), %rax                               #1184.12
        leave                                                   #1184.12
..___tag_value_nss_sob.85:                                      #
        ret                                                     #1184.12
        .align    2,0x90
..___tag_value_nss_sob.86:                                      #
                                # LOE
# mark_end;
	.type	nss_sob,@function
	.size	nss_sob,.-nss_sob
.LNnss_sob:
	.data
# -- End  nss_sob
	.text
# -- Begin  nss_FASTA_expr
# mark_begin;
       .align    2,0x90
nss_FASTA_expr:
# parameter 1(p): %rdi
# parameter 2(end): %rsi
# parameter 3(expr): %rdx
# parameter 4(status): %rcx
# parameter 5(positional): %r8d
..B9.1:                         # Preds ..B9.0
..___tag_value_nss_FASTA_expr.89:                               #
..LN807:
   .loc    3  1192
        pushq     %rbp                                          #1192.1
        movq      %rsp, %rbp                                    #1192.1
..___tag_value_nss_FASTA_expr.90:                               #
        subq      $80, %rsp                                     #1192.1
        movq      %rbx, -16(%rbp)                               #1192.1
..___tag_value_nss_FASTA_expr.93:                               #
        movq      %rdi, -48(%rbp)                               #1192.1
        movq      %rsi, -40(%rbp)                               #1192.1
        movq      %rdx, -32(%rbp)                               #1192.1
        movq      %rcx, -24(%rbp)                               #1192.1
        movl      %r8d, -8(%rbp)                                #1192.1
..LN809:
   .loc    3  1193
        movq      -48(%rbp), %rax                               #1193.10
..LN811:
        movq      -40(%rbp), %rdx                               #1193.15
        cmpq      %rdx, %rax                                    #1193.15
        jb        ..B9.3        # Prob 50%                      #1193.15
                                # LOE
..B9.2:                         # Preds ..B9.1
..LN813:
   .loc    3  1194
        movq      -24(%rbp), %rax                               #1194.9
        movl      $22, (%rax)                                   #1194.9
        jmp       ..B9.17       # Prob 100%                     #1194.9
                                # LOE
..B9.3:                         # Preds ..B9.1
..LN815:
   .loc    3  1197
        movq      -48(%rbp), %rax                               #1197.30
..LN817:
        movq      %rax, -72(%rbp)                               #1197.27
..LN819:
   .loc    3  1199
        movl      $0, -56(%rbp)                                 #1199.22
..LN821:
   .loc    3  1200
        lea       fasta_2na_map.0(%rip), %rax                   #1200.27
        movq      %rax, -64(%rbp)                               #1200.27
                                # LOE
..B9.4:                         # Preds ..B9.9 ..B9.3
..LN823:
   .loc    3  1203
        movq      -48(%rbp), %rax                               #1203.20
        movzbl    (%rax), %eax                                  #1203.20
        movsbq    %al, %rax                                     #1203.20
..LN825:
        testl     %eax, %eax                                    #1203.24
        jl        ..B9.10       # Prob 50%                      #1203.24
                                # LOE
..B9.5:                         # Preds ..B9.4
..LN827:
   .loc    3  1206
        movq      -48(%rbp), %rax                               #1206.45
        movzbl    (%rax), %eax                                  #1206.45
        movzbl    %al, %eax                                     #1206.45
..LN829:
        movslq    %eax, %rax                                    #1206.18
        movq      -64(%rbp), %rdx                               #1206.18
        movzbl    (%rax,%rdx), %eax                             #1206.18
        movsbq    %al, %rax                                     #1206.18
..LN831:
        testl     %eax, %eax                                    #1206.51
        jge       ..B9.9        # Prob 50%                      #1206.51
                                # LOE
..B9.6:                         # Preds ..B9.5
..LN833:
   .loc    3  1208
        movq      -64(%rbp), %rax                               #1208.22
..LN835:
        lea       fasta_4na_map.0(%rip), %rdx                   #1208.29
        cmpq      %rdx, %rax                                    #1208.29
        je        ..B9.10       # Prob 50%                      #1208.29
                                # LOE
..B9.7:                         # Preds ..B9.6
..LN837:
   .loc    3  1210
        movq      -48(%rbp), %rax                               #1210.59
        movzbl    (%rax), %eax                                  #1210.59
        movzbl    %al, %eax                                     #1210.59
..LN839:
        movslq    %eax, %rax                                    #1210.22
        lea       fasta_4na_map.0(%rip), %rdx                   #1210.22
        movzbl    (%rax,%rdx), %eax                             #1210.22
        movsbq    %al, %rax                                     #1210.22
..LN841:
        testl     %eax, %eax                                    #1210.65
        jl        ..B9.10       # Prob 50%                      #1210.65
                                # LOE
..B9.8:                         # Preds ..B9.7
..LN843:
   .loc    3  1212
        movl      $1, -56(%rbp)                                 #1212.17
..LN845:
   .loc    3  1213
        lea       fasta_4na_map.0(%rip), %rax                   #1213.23
..LN847:
        movq      %rax, -64(%rbp)                               #1213.17
                                # LOE
..B9.9:                         # Preds ..B9.8 ..B9.5
..LN849:
   .loc    3  1216
        movq      -48(%rbp), %rax                               #1216.20
        incq      %rax                                          #1216.20
        movq      %rax, -48(%rbp)                               #1216.20
..LN851:
        movq      -40(%rbp), %rdx                               #1216.24
        cmpq      %rdx, %rax                                    #1216.24
        jb        ..B9.4        # Prob 50%                      #1216.24
                                # LOE
..B9.10:                        # Preds ..B9.7 ..B9.6 ..B9.4 ..B9.9
..LN853:
   .loc    3  1218
        movq      -48(%rbp), %rax                               #1218.14
..LN855:
        movq      -72(%rbp), %rdx                               #1218.19
        cmpq      %rdx, %rax                                    #1218.19
        ja        ..B9.12       # Prob 50%                      #1218.19
                                # LOE
..B9.11:                        # Preds ..B9.10
..LN857:
   .loc    3  1219
        movq      -24(%rbp), %rax                               #1219.13
        movl      $22, (%rax)                                   #1219.13
        jmp       ..B9.17       # Prob 100%                     #1219.13
                                # LOE
..B9.12:                        # Preds ..B9.10
..LN859:
   .loc    3  1220
        movl      -56(%rbp), %eax                               #1220.19
..LN861:
        testl     %eax, %eax                                    #1220.27
        jne       ..B9.15       # Prob 50%                      #1220.27
                                # LOE
..B9.13:                        # Preds ..B9.12
..LN863:
   .loc    3  1221
        movq      -32(%rbp), %rax                               #1221.47
..LN865:
        movl      -8(%rbp), %edx                                #1221.53
..LN867:
        movq      -72(%rbp), %rcx                               #1221.65
..LN869:
        movq      -72(%rbp), %rbx                               #1221.76
        negq      %rbx                                          #1221.76
        addq      -48(%rbp), %rbx                               #1221.76
..LN871:
        movq      %rax, %rdi                                    #1221.24
        movl      %edx, %esi                                    #1221.24
        movq      %rcx, %rdx                                    #1221.24
        movq      %rbx, %rcx                                    #1221.24
        call      NucStrFastaExprMake2@PLT                      #1221.24
                                # LOE eax
..B9.20:                        # Preds ..B9.13
        movl      %eax, -80(%rbp)                               #1221.24
                                # LOE
..B9.14:                        # Preds ..B9.20
..LN873:
        movq      -24(%rbp), %rax                               #1221.13
        movl      -80(%rbp), %edx                               #1221.13
        movl      %edx, (%rax)                                  #1221.13
        jmp       ..B9.17       # Prob 100%                     #1221.13
                                # LOE
..B9.15:                        # Preds ..B9.12
..LN875:
   .loc    3  1223
        movq      -32(%rbp), %rax                               #1223.47
..LN877:
        movl      -8(%rbp), %edx                                #1223.53
..LN879:
        movq      -72(%rbp), %rcx                               #1223.65
..LN881:
        movq      -72(%rbp), %rbx                               #1223.76
        negq      %rbx                                          #1223.76
        addq      -48(%rbp), %rbx                               #1223.76
..LN883:
        movq      %rax, %rdi                                    #1223.24
        movl      %edx, %esi                                    #1223.24
        movq      %rcx, %rdx                                    #1223.24
        movq      %rbx, %rcx                                    #1223.24
        call      NucStrFastaExprMake4@PLT                      #1223.24
                                # LOE eax
..B9.21:                        # Preds ..B9.15
        movl      %eax, -76(%rbp)                               #1223.24
                                # LOE
..B9.16:                        # Preds ..B9.21
..LN885:
        movq      -24(%rbp), %rax                               #1223.13
        movl      -76(%rbp), %edx                               #1223.13
        movl      %edx, (%rax)                                  #1223.13
                                # LOE
..B9.17:                        # Preds ..B9.2 ..B9.11 ..B9.14 ..B9.16
..LN887:
   .loc    3  1226
        movq      -48(%rbp), %rax                               #1226.12
        movq      -16(%rbp), %rbx                               #1226.12
..___tag_value_nss_FASTA_expr.94:                               #
        leave                                                   #1226.12
..___tag_value_nss_FASTA_expr.96:                               #
        ret                                                     #1226.12
        .align    2,0x90
..___tag_value_nss_FASTA_expr.97:                               #
                                # LOE
# mark_end;
	.type	nss_FASTA_expr,@function
	.size	nss_FASTA_expr,.-nss_FASTA_expr
.LNnss_FASTA_expr:
	.data
# -- End  nss_FASTA_expr
	.text
# -- Begin  nss_fasta_expr
# mark_begin;
       .align    2,0x90
nss_fasta_expr:
# parameter 1(p): %rdi
# parameter 2(end): %rsi
# parameter 3(expr): %rdx
# parameter 4(status): %rcx
# parameter 5(positional): %r8d
..B10.1:                        # Preds ..B10.0
..___tag_value_nss_fasta_expr.100:                              #
..LN889:
   .loc    3  1234
        pushq     %rbp                                          #1234.1
        movq      %rsp, %rbp                                    #1234.1
..___tag_value_nss_fasta_expr.101:                              #
        subq      $112, %rsp                                    #1234.1
        movq      %rbx, -16(%rbp)                               #1234.1
..___tag_value_nss_fasta_expr.104:                              #
        movq      %rdi, -48(%rbp)                               #1234.1
        movq      %rsi, -40(%rbp)                               #1234.1
        movq      %rdx, -32(%rbp)                               #1234.1
        movq      %rcx, -24(%rbp)                               #1234.1
        movl      %r8d, -8(%rbp)                                #1234.1
..LN891:
   .loc    3  1235
        movq      -48(%rbp), %rax                               #1235.5
        movq      -40(%rbp), %rdx                               #1235.5
        cmpq      %rdx, %rax                                    #1235.5
        jb        ..B10.3       # Prob 50%                      #1235.5
                                # LOE
..B10.2:                        # Preds ..B10.1
        lea       _2__STRING.3.0(%rip), %rax                    #1235.5
        lea       _2__STRING.1.0(%rip), %rdx                    #1235.5
        lea       __$U1.0(%rip), %rcx                           #1235.5
        movq      %rax, %rdi                                    #1235.5
        movq      %rdx, %rsi                                    #1235.5
        movl      $1235, %edx                                   #1235.5
        call      __assert_fail@PLT                             #1235.5
                                # LOE
..B10.3:                        # Preds ..B10.1
..LN893:
   .loc    3  1236
        movq      -48(%rbp), %rax                               #1236.16
        movzbl    (%rax), %eax                                  #1236.16
        movsbq    %al, %rax                                     #1236.16
        movl      %eax, -56(%rbp)                               #1236.16
        cmpl      $39, %eax                                     #1236.16
        je        ..B10.5       # Prob 50%                      #1236.16
                                # LOE
..B10.4:                        # Preds ..B10.3
        movl      -56(%rbp), %eax                               #1236.16
        cmpl      $34, %eax                                     #1236.16
        je        ..B10.13      # Prob 50%                      #1236.16
        jmp       ..B10.21      # Prob 100%                     #1236.16
                                # LOE
..B10.5:                        # Preds ..B10.3
..LN895:
   .loc    3  1239
        movq      -48(%rbp), %rax                               #1239.30
        incq      %rax                                          #1239.30
..LN897:
        movq      -40(%rbp), %rdx                               #1239.37
..LN899:
        movq      -32(%rbp), %rcx                               #1239.42
..LN901:
        movq      -24(%rbp), %rbx                               #1239.48
..LN903:
        movl      -8(%rbp), %esi                                #1239.56
..LN905:
        movq      %rax, %rdi                                    #1239.13
        movl      %esi, -64(%rbp)                               #1239.13
        movq      %rdx, %rsi                                    #1239.13
        movq      %rcx, %rdx                                    #1239.13
        movq      %rbx, %rcx                                    #1239.13
        movl      -64(%rbp), %eax                               #1239.13
        movl      %eax, %r8d                                    #1239.13
        call      nss_FASTA_expr@PLT                            #1239.13
                                # LOE rax
..B10.26:                       # Preds ..B10.5
        movq      %rax, -72(%rbp)                               #1239.13
                                # LOE
..B10.6:                        # Preds ..B10.26
..LN907:
        movq      -72(%rbp), %rax                               #1239.9
        movq      %rax, -48(%rbp)                               #1239.9
..LN909:
   .loc    3  1240
        movq      -24(%rbp), %rax                               #1240.16
        movl      (%rax), %eax                                  #1240.16
..LN911:
        testl     %eax, %eax                                    #1240.26
        jne       ..B10.23      # Prob 50%                      #1240.26
                                # LOE
..B10.7:                        # Preds ..B10.6
..LN913:
        movq      -48(%rbp), %rax                               #1240.33
..LN915:
        movq      -40(%rbp), %rdx                               #1240.38
        cmpq      %rdx, %rax                                    #1240.38
        je        ..B10.12      # Prob 50%                      #1240.38
                                # LOE
..B10.8:                        # Preds ..B10.7
..LN917:
        movq      -48(%rbp), %rax                               #1240.47
        movzbl    (%rax), %eax                                  #1240.47
        movsbq    %al, %rax                                     #1240.47
..LN919:
        cmpl      $39, %eax                                     #1240.55
        jne       ..B10.10      # Prob 50%                      #1240.55
                                # LOE
..B10.9:                        # Preds ..B10.8
        movl      $0, -108(%rbp)                                #1240.55
        jmp       ..B10.11      # Prob 100%                     #1240.55
                                # LOE
..B10.10:                       # Preds ..B10.8
        movl      $1, -108(%rbp)                                #1240.55
                                # LOE
..B10.11:                       # Preds ..B10.10 ..B10.9
..LN921:
        movl      -108(%rbp), %eax                              #1240.47
        incq      -48(%rbp)                                     #1240.47
..LN923:
        testl     %eax, %eax                                    #1240.55
        je        ..B10.23      # Prob 100%                     #1240.55
                                # LOE
..B10.12:                       # Preds ..B10.7 ..B10.11
..LN925:
   .loc    3  1241
        movq      -24(%rbp), %rax                               #1241.13
        movl      $22, (%rax)                                   #1241.13
        jmp       ..B10.23      # Prob 100%                     #1241.13
                                # LOE
..B10.13:                       # Preds ..B10.4
..LN927:
   .loc    3  1244
        movq      -48(%rbp), %rax                               #1244.30
        incq      %rax                                          #1244.30
..LN929:
        movq      -40(%rbp), %rdx                               #1244.37
..LN931:
        movq      -32(%rbp), %rcx                               #1244.42
..LN933:
        movq      -24(%rbp), %rbx                               #1244.48
..LN935:
        movl      -8(%rbp), %esi                                #1244.56
..LN937:
        movq      %rax, %rdi                                    #1244.13
        movl      %esi, -88(%rbp)                               #1244.13
        movq      %rdx, %rsi                                    #1244.13
        movq      %rcx, %rdx                                    #1244.13
        movq      %rbx, %rcx                                    #1244.13
        movl      -88(%rbp), %eax                               #1244.13
        movl      %eax, %r8d                                    #1244.13
        call      nss_FASTA_expr@PLT                            #1244.13
                                # LOE rax
..B10.27:                       # Preds ..B10.13
        movq      %rax, -104(%rbp)                              #1244.13
                                # LOE
..B10.14:                       # Preds ..B10.27
..LN939:
        movq      -104(%rbp), %rax                              #1244.9
        movq      %rax, -48(%rbp)                               #1244.9
..LN941:
   .loc    3  1245
        movq      -24(%rbp), %rax                               #1245.16
        movl      (%rax), %eax                                  #1245.16
..LN943:
        testl     %eax, %eax                                    #1245.26
        jne       ..B10.23      # Prob 50%                      #1245.26
                                # LOE
..B10.15:                       # Preds ..B10.14
..LN945:
        movq      -48(%rbp), %rax                               #1245.33
..LN947:
        movq      -40(%rbp), %rdx                               #1245.38
        cmpq      %rdx, %rax                                    #1245.38
        je        ..B10.20      # Prob 50%                      #1245.38
                                # LOE
..B10.16:                       # Preds ..B10.15
..LN949:
        movq      -48(%rbp), %rax                               #1245.47
        movzbl    (%rax), %eax                                  #1245.47
        movsbq    %al, %rax                                     #1245.47
..LN951:
        cmpl      $34, %eax                                     #1245.55
        jne       ..B10.18      # Prob 50%                      #1245.55
                                # LOE
..B10.17:                       # Preds ..B10.16
        movl      $0, -112(%rbp)                                #1245.55
        jmp       ..B10.19      # Prob 100%                     #1245.55
                                # LOE
..B10.18:                       # Preds ..B10.16
        movl      $1, -112(%rbp)                                #1245.55
                                # LOE
..B10.19:                       # Preds ..B10.18 ..B10.17
..LN953:
        movl      -112(%rbp), %eax                              #1245.47
        incq      -48(%rbp)                                     #1245.47
..LN955:
        testl     %eax, %eax                                    #1245.55
        je        ..B10.23      # Prob 100%                     #1245.55
                                # LOE
..B10.20:                       # Preds ..B10.15 ..B10.19
..LN957:
   .loc    3  1246
        movq      -24(%rbp), %rax                               #1246.13
        movl      $22, (%rax)                                   #1246.13
        jmp       ..B10.23      # Prob 100%                     #1246.13
                                # LOE
..B10.21:                       # Preds ..B10.4
..LN959:
   .loc    3  1249
        movq      -48(%rbp), %rax                               #1249.33
..LN961:
        movq      -40(%rbp), %rdx                               #1249.36
..LN963:
        movq      -32(%rbp), %rcx                               #1249.41
..LN965:
        movq      -24(%rbp), %rbx                               #1249.47
..LN967:
        movl      -8(%rbp), %esi                                #1249.55
..LN969:
        movq      %rax, %rdi                                    #1249.16
        movl      %esi, -80(%rbp)                               #1249.16
        movq      %rdx, %rsi                                    #1249.16
        movq      %rcx, %rdx                                    #1249.16
        movq      %rbx, %rcx                                    #1249.16
        movl      -80(%rbp), %eax                               #1249.16
        movl      %eax, %r8d                                    #1249.16
        call      nss_FASTA_expr@PLT                            #1249.16
                                # LOE rax
..B10.28:                       # Preds ..B10.21
        movq      %rax, -96(%rbp)                               #1249.16
                                # LOE
..B10.22:                       # Preds ..B10.28
        movq      -96(%rbp), %rax                               #1249.16
        movq      -16(%rbp), %rbx                               #1249.16
..___tag_value_nss_fasta_expr.105:                              #
        leave                                                   #1249.16
..___tag_value_nss_fasta_expr.107:                              #
        ret                                                     #1249.16
..___tag_value_nss_fasta_expr.108:                              #
                                # LOE
..B10.23:                       # Preds ..B10.12 ..B10.11 ..B10.6 ..B10.20 ..B10.19
                                #       ..B10.14
..LN971:
   .loc    3  1252
        movq      -48(%rbp), %rax                               #1252.12
        movq      -16(%rbp), %rbx                               #1252.12
..___tag_value_nss_fasta_expr.110:                              #
        leave                                                   #1252.12
..___tag_value_nss_fasta_expr.112:                              #
        ret                                                     #1252.12
        .align    2,0x90
..___tag_value_nss_fasta_expr.113:                              #
                                # LOE
# mark_end;
	.type	nss_fasta_expr,@function
	.size	nss_fasta_expr,.-nss_fasta_expr
.LNnss_fasta_expr:
	.data
# -- End  nss_fasta_expr
	.text
# -- Begin  nss_primary_expr
# mark_begin;
       .align    2,0x90
nss_primary_expr:
# parameter 1(p): %rdi
# parameter 2(end): %rsi
# parameter 3(expr): %rdx
# parameter 4(status): %rcx
# parameter 5(positional): %r8d
..B11.1:                        # Preds ..B11.0
..___tag_value_nss_primary_expr.116:                            #
..LN973:
   .loc    3  1291
        pushq     %rbp                                          #1291.1
        movq      %rsp, %rbp                                    #1291.1
..___tag_value_nss_primary_expr.117:                            #
        subq      $160, %rsp                                    #1291.1
        movq      %rbx, -16(%rbp)                               #1291.1
..___tag_value_nss_primary_expr.120:                            #
        movq      %rdi, -48(%rbp)                               #1291.1
        movq      %rsi, -40(%rbp)                               #1291.1
        movq      %rdx, -32(%rbp)                               #1291.1
        movq      %rcx, -24(%rbp)                               #1291.1
        movl      %r8d, -8(%rbp)                                #1291.1
..LN975:
   .loc    3  1294
        movq      -48(%rbp), %rax                               #1294.5
        movq      -40(%rbp), %rdx                               #1294.5
        cmpq      %rdx, %rax                                    #1294.5
        jb        ..B11.3       # Prob 50%                      #1294.5
                                # LOE
..B11.2:                        # Preds ..B11.1
        lea       _2__STRING.3.0(%rip), %rax                    #1294.5
        lea       _2__STRING.1.0(%rip), %rdx                    #1294.5
        lea       __$U2.0(%rip), %rcx                           #1294.5
        movq      %rax, %rdi                                    #1294.5
        movq      %rdx, %rsi                                    #1294.5
        movl      $1294, %edx                                   #1294.5
        call      __assert_fail@PLT                             #1294.5
                                # LOE
..B11.3:                        # Preds ..B11.1
..LN977:
   .loc    3  1295
        movq      -48(%rbp), %rax                               #1295.16
        movzbl    (%rax), %eax                                  #1295.16
        movsbq    %al, %rax                                     #1295.16
        movl      %eax, -56(%rbp)                               #1295.16
        cmpl      $94, %eax                                     #1295.16
        je        ..B11.5       # Prob 50%                      #1295.16
                                # LOE
..B11.4:                        # Preds ..B11.3
        movl      -56(%rbp), %eax                               #1295.16
        cmpl      $40, %eax                                     #1295.16
        je        ..B11.12      # Prob 50%                      #1295.16
        jmp       ..B11.25      # Prob 100%                     #1295.16
                                # LOE
..B11.5:                        # Preds ..B11.3
..LN979:
   .loc    3  1298
        movl      $16, %edi                                     #1298.13
        call      malloc@PLT                                    #1298.13
                                # LOE rax
..B11.38:                       # Preds ..B11.5
        movq      %rax, -64(%rbp)                               #1298.13
                                # LOE
..B11.6:                        # Preds ..B11.38
..LN981:
        movq      -64(%rbp), %rax                               #1298.9
        movq      %rax, -72(%rbp)                               #1298.9
..LN983:
   .loc    3  1299
        movq      -72(%rbp), %rax                               #1299.14
..LN985:
        testq     %rax, %rax                                    #1299.19
        jne       ..B11.8       # Prob 50%                      #1299.19
                                # LOE
..B11.7:                        # Preds ..B11.6
..LN987:
   .loc    3  1300
        call      __errno_location@PLT                          #1300.24
                                # LOE rax
..B11.39:                       # Preds ..B11.7
..LN989:
        movq      -24(%rbp), %rdx                               #1300.13
..LN991:
        movl      (%rax), %eax                                  #1300.24
..LN993:
        movl      %eax, (%rdx)                                  #1300.13
        jmp       ..B11.11      # Prob 100%                     #1300.13
                                # LOE
..B11.8:                        # Preds ..B11.6
..LN995:
   .loc    3  1303
        movq      -72(%rbp), %rax                               #1303.13
        movl      $12, (%rax)                                   #1303.13
..LN997:
   .loc    3  1304
        movq      -72(%rbp), %rax                               #1304.13
        movl      $14, 4(%rax)                                  #1304.13
..LN999:
   .loc    3  1305
        movq      -72(%rbp), %rax                               #1305.13
        movq      $0, 8(%rax)                                   #1305.13
..LN1001:
   .loc    3  1306
        movq      -32(%rbp), %rax                               #1306.13
..LN1003:
        movq      -72(%rbp), %rdx                               #1306.22
..LN1005:
        movq      %rdx, (%rax)                                  #1306.13
..LN1007:
   .loc    3  1308
        movq      -48(%rbp), %rax                               #1308.27
        incq      %rax                                          #1308.27
..LN1009:
        movq      -40(%rbp), %rdx                               #1308.34
..LN1011:
        movq      %rax, %rdi                                    #1308.17
        movq      %rdx, %rsi                                    #1308.17
        call      nss_sob@PLT                                   #1308.17
                                # LOE rax
..B11.40:                       # Preds ..B11.8
        movq      %rax, -120(%rbp)                              #1308.17
                                # LOE
..B11.9:                        # Preds ..B11.40
..LN1013:
        movq      -120(%rbp), %rax                              #1308.13
        movq      %rax, -48(%rbp)                               #1308.13
..LN1015:
   .loc    3  1309
        movq      -48(%rbp), %rax                               #1309.17
        movq      -40(%rbp), %rdx                               #1309.17
        movq      -72(%rbp), %rcx                               #1309.17
        addq      $8, %rcx                                      #1309.17
        movq      -24(%rbp), %rbx                               #1309.17
        movl      -8(%rbp), %esi                                #1309.17
        movq      %rax, %rdi                                    #1309.17
        movl      %esi, -88(%rbp)                               #1309.17
        movq      %rdx, %rsi                                    #1309.17
        movq      %rcx, %rdx                                    #1309.17
        movq      %rbx, %rcx                                    #1309.17
        movl      -88(%rbp), %eax                               #1309.17
        movl      %eax, %r8d                                    #1309.17
        call      nss_fasta_expr@PLT                            #1309.17
                                # LOE rax
..B11.41:                       # Preds ..B11.9
        movq      %rax, -112(%rbp)                              #1309.17
                                # LOE
..B11.10:                       # Preds ..B11.41
..LN1017:
        movq      -112(%rbp), %rax                              #1309.13
        movq      %rax, -48(%rbp)                               #1309.13
                                # LOE
..B11.11:                       # Preds ..B11.39 ..B11.10
..LN1019:
   .loc    3  1311
        movq      -48(%rbp), %rax                               #1311.16
        movq      -16(%rbp), %rbx                               #1311.16
..___tag_value_nss_primary_expr.121:                            #
        leave                                                   #1311.16
..___tag_value_nss_primary_expr.123:                            #
        ret                                                     #1311.16
..___tag_value_nss_primary_expr.124:                            #
                                # LOE
..B11.12:                       # Preds ..B11.4
..LN1021:
   .loc    3  1313
        movl      $16, %edi                                     #1313.13
        call      malloc@PLT                                    #1313.13
                                # LOE rax
..B11.42:                       # Preds ..B11.12
        movq      %rax, -104(%rbp)                              #1313.13
                                # LOE
..B11.13:                       # Preds ..B11.42
..LN1023:
        movq      -104(%rbp), %rax                              #1313.9
        movq      %rax, -72(%rbp)                               #1313.9
..LN1025:
   .loc    3  1314
        movq      -72(%rbp), %rax                               #1314.14
..LN1027:
        testq     %rax, %rax                                    #1314.19
        jne       ..B11.15      # Prob 50%                      #1314.19
                                # LOE
..B11.14:                       # Preds ..B11.13
..LN1029:
   .loc    3  1315
        call      __errno_location@PLT                          #1315.24
                                # LOE rax
..B11.43:                       # Preds ..B11.14
..LN1031:
        movq      -24(%rbp), %rdx                               #1315.13
..LN1033:
        movl      (%rax), %eax                                  #1315.24
..LN1035:
        movl      %eax, (%rdx)                                  #1315.13
        jmp       ..B11.24      # Prob 100%                     #1315.13
                                # LOE
..B11.15:                       # Preds ..B11.13
..LN1037:
   .loc    3  1318
        movq      -72(%rbp), %rax                               #1318.13
        movl      $12, (%rax)                                   #1318.13
..LN1039:
   .loc    3  1319
        movq      -72(%rbp), %rax                               #1319.13
        movl      $0, 4(%rax)                                   #1319.13
..LN1041:
   .loc    3  1320
        movq      -32(%rbp), %rax                               #1320.13
..LN1043:
        movq      -72(%rbp), %rdx                               #1320.22
..LN1045:
        movq      %rdx, (%rax)                                  #1320.13
..LN1047:
   .loc    3  1322
        movq      -48(%rbp), %rax                               #1322.28
        incq      %rax                                          #1322.28
..LN1049:
        movq      -40(%rbp), %rdx                               #1322.35
..LN1051:
        movq      -72(%rbp), %rcx                               #1322.42
        addq      $8, %rcx                                      #1322.42
..LN1053:
        movq      -24(%rbp), %rbx                               #1322.59
..LN1055:
        movl      -8(%rbp), %esi                                #1322.67
..LN1057:
        movq      %rax, %rdi                                    #1322.17
        movl      %esi, -128(%rbp)                              #1322.17
        movq      %rdx, %rsi                                    #1322.17
        movq      %rcx, %rdx                                    #1322.17
        movq      %rbx, %rcx                                    #1322.17
        movl      -128(%rbp), %eax                              #1322.17
        movl      %eax, %r8d                                    #1322.17
        call      nss_expr@PLT                                  #1322.17
                                # LOE rax
..B11.44:                       # Preds ..B11.15
        movq      %rax, -136(%rbp)                              #1322.17
                                # LOE
..B11.16:                       # Preds ..B11.44
..LN1059:
        movq      -136(%rbp), %rax                              #1322.13
        movq      %rax, -48(%rbp)                               #1322.13
..LN1061:
   .loc    3  1323
        movq      -24(%rbp), %rax                               #1323.20
        movl      (%rax), %eax                                  #1323.20
..LN1063:
        testl     %eax, %eax                                    #1323.30
        jne       ..B11.24      # Prob 50%                      #1323.30
                                # LOE
..B11.17:                       # Preds ..B11.16
..LN1065:
   .loc    3  1325
        movq      -72(%rbp), %rax                               #1325.22
        movq      8(%rax), %rax                                 #1325.22
..LN1067:
        testq     %rax, %rax                                    #1325.41
        je        ..B11.23      # Prob 50%                      #1325.41
                                # LOE
..B11.18:                       # Preds ..B11.17
..LN1069:
        movq      -48(%rbp), %rax                               #1325.49
..LN1071:
        movq      -40(%rbp), %rdx                               #1325.54
        cmpq      %rdx, %rax                                    #1325.54
        je        ..B11.23      # Prob 50%                      #1325.54
                                # LOE
..B11.19:                       # Preds ..B11.18
..LN1073:
        movq      -48(%rbp), %rax                               #1325.63
        movzbl    (%rax), %eax                                  #1325.63
        movsbq    %al, %rax                                     #1325.63
..LN1075:
        cmpl      $41, %eax                                     #1325.71
        jne       ..B11.21      # Prob 50%                      #1325.71
                                # LOE
..B11.20:                       # Preds ..B11.19
        movl      $0, -152(%rbp)                                #1325.71
        jmp       ..B11.22      # Prob 100%                     #1325.71
                                # LOE
..B11.21:                       # Preds ..B11.19
        movl      $1, -152(%rbp)                                #1325.71
                                # LOE
..B11.22:                       # Preds ..B11.21 ..B11.20
..LN1077:
        movl      -152(%rbp), %eax                              #1325.63
        incq      -48(%rbp)                                     #1325.63
..LN1079:
        testl     %eax, %eax                                    #1325.71
        je        ..B11.24      # Prob 100%                     #1325.71
                                # LOE
..B11.23:                       # Preds ..B11.17 ..B11.18 ..B11.22
..LN1081:
   .loc    3  1326
        movq      -24(%rbp), %rax                               #1326.21
        movl      $22, (%rax)                                   #1326.21
                                # LOE
..B11.24:                       # Preds ..B11.43 ..B11.23 ..B11.22 ..B11.16
..LN1083:
   .loc    3  1329
        movq      -48(%rbp), %rax                               #1329.16
        movq      -16(%rbp), %rbx                               #1329.16
..___tag_value_nss_primary_expr.126:                            #
        leave                                                   #1329.16
..___tag_value_nss_primary_expr.128:                            #
        ret                                                     #1329.16
..___tag_value_nss_primary_expr.129:                            #
                                # LOE
..B11.25:                       # Preds ..B11.4
..LN1085:
   .loc    3  1332
        movq      -48(%rbp), %rax                               #1332.9
        movq      -40(%rbp), %rdx                               #1332.9
        movq      -32(%rbp), %rcx                               #1332.9
        movq      -24(%rbp), %rbx                               #1332.9
        movl      -8(%rbp), %esi                                #1332.9
        movq      %rax, %rdi                                    #1332.9
        movl      %esi, -80(%rbp)                               #1332.9
        movq      %rdx, %rsi                                    #1332.9
        movq      %rcx, %rdx                                    #1332.9
        movq      %rbx, %rcx                                    #1332.9
        movl      -80(%rbp), %eax                               #1332.9
        movl      %eax, %r8d                                    #1332.9
        call      nss_fasta_expr@PLT                            #1332.9
                                # LOE rax
..B11.45:                       # Preds ..B11.25
        movq      %rax, -96(%rbp)                               #1332.9
                                # LOE
..B11.26:                       # Preds ..B11.45
..LN1087:
        movq      -96(%rbp), %rax                               #1332.5
        movq      %rax, -48(%rbp)                               #1332.5
..LN1089:
   .loc    3  1333
        movq      -24(%rbp), %rax                               #1333.12
        movl      (%rax), %eax                                  #1333.12
..LN1091:
        testl     %eax, %eax                                    #1333.22
        jne       ..B11.35      # Prob 50%                      #1333.22
                                # LOE
..B11.27:                       # Preds ..B11.26
..LN1093:
        movq      -48(%rbp), %rax                               #1333.27
..LN1095:
        movq      -40(%rbp), %rdx                               #1333.31
        cmpq      %rdx, %rax                                    #1333.31
        jae       ..B11.35      # Prob 50%                      #1333.31
                                # LOE
..B11.28:                       # Preds ..B11.27
..LN1097:
   .loc    3  1335
        movq      -48(%rbp), %rax                               #1335.23
..LN1099:
        movq      -40(%rbp), %rdx                               #1335.26
..LN1101:
        movq      %rax, %rdi                                    #1335.13
        movq      %rdx, %rsi                                    #1335.13
        call      nss_sob@PLT                                   #1335.13
                                # LOE rax
..B11.46:                       # Preds ..B11.28
        movq      %rax, -144(%rbp)                              #1335.13
                                # LOE
..B11.29:                       # Preds ..B11.46
..LN1103:
        movq      -144(%rbp), %rax                              #1335.9
        movq      %rax, -48(%rbp)                               #1335.9
..LN1105:
   .loc    3  1336
        movq      -48(%rbp), %rax                               #1336.14
..LN1107:
        movq      -40(%rbp), %rdx                               #1336.18
        cmpq      %rdx, %rax                                    #1336.18
        jae       ..B11.35      # Prob 50%                      #1336.18
                                # LOE
..B11.30:                       # Preds ..B11.29
..LN1109:
        movq      -48(%rbp), %rax                               #1336.27
        movzbl    (%rax), %eax                                  #1336.27
        movsbq    %al, %rax                                     #1336.27
..LN1111:
        cmpl      $36, %eax                                     #1336.32
        jne       ..B11.35      # Prob 50%                      #1336.32
                                # LOE
..B11.31:                       # Preds ..B11.30
..LN1113:
   .loc    3  1338
        incq      -48(%rbp)                                     #1338.16
..LN1115:
   .loc    3  1340
        movl      $16, %edi                                     #1340.17
        call      malloc@PLT                                    #1340.17
                                # LOE rax
..B11.47:                       # Preds ..B11.31
        movq      %rax, -160(%rbp)                              #1340.17
                                # LOE
..B11.32:                       # Preds ..B11.47
..LN1117:
        movq      -160(%rbp), %rax                              #1340.13
        movq      %rax, -72(%rbp)                               #1340.13
..LN1119:
   .loc    3  1341
        movq      -72(%rbp), %rax                               #1341.18
..LN1121:
        testq     %rax, %rax                                    #1341.23
        jne       ..B11.34      # Prob 50%                      #1341.23
                                # LOE
..B11.33:                       # Preds ..B11.32
..LN1123:
   .loc    3  1342
        call      __errno_location@PLT                          #1342.28
                                # LOE rax
..B11.48:                       # Preds ..B11.33
..LN1125:
        movq      -24(%rbp), %rdx                               #1342.17
..LN1127:
        movl      (%rax), %eax                                  #1342.28
..LN1129:
        movl      %eax, (%rdx)                                  #1342.17
        jmp       ..B11.35      # Prob 100%                     #1342.17
                                # LOE
..B11.34:                       # Preds ..B11.32
..LN1131:
   .loc    3  1345
        movq      -72(%rbp), %rax                               #1345.17
        movl      $12, (%rax)                                   #1345.17
..LN1133:
   .loc    3  1346
        movq      -72(%rbp), %rax                               #1346.17
        movl      $15, 4(%rax)                                  #1346.17
..LN1135:
   .loc    3  1347
        movq      -32(%rbp), %rax                               #1347.37
..LN1137:
        movq      -72(%rbp), %rdx                               #1347.17
..LN1139:
        movq      (%rax), %rax                                  #1347.37
..LN1141:
        movq      %rax, 8(%rdx)                                 #1347.17
..LN1143:
   .loc    3  1348
        movq      -32(%rbp), %rax                               #1348.17
..LN1145:
        movq      -72(%rbp), %rdx                               #1348.26
..LN1147:
        movq      %rdx, (%rax)                                  #1348.17
                                # LOE
..B11.35:                       # Preds ..B11.48 ..B11.34 ..B11.30 ..B11.29 ..B11.27
                                #       ..B11.26
..LN1149:
   .loc    3  1353
        movq      -48(%rbp), %rax                               #1353.12
        movq      -16(%rbp), %rbx                               #1353.12
..___tag_value_nss_primary_expr.131:                            #
        leave                                                   #1353.12
..___tag_value_nss_primary_expr.133:                            #
        ret                                                     #1353.12
        .align    2,0x90
..___tag_value_nss_primary_expr.134:                            #
                                # LOE
# mark_end;
	.type	nss_primary_expr,@function
	.size	nss_primary_expr,.-nss_primary_expr
.LNnss_primary_expr:
	.data
# -- End  nss_primary_expr
	.text
# -- Begin  nss_unary_expr
# mark_begin;
       .align    2,0x90
nss_unary_expr:
# parameter 1(p): %rdi
# parameter 2(end): %rsi
# parameter 3(expr): %rdx
# parameter 4(status): %rcx
# parameter 5(positional): %r8d
..B12.1:                        # Preds ..B12.0
..___tag_value_nss_unary_expr.137:                              #
..LN1151:
   .loc    3  1361
        pushq     %rbp                                          #1361.1
        movq      %rsp, %rbp                                    #1361.1
..___tag_value_nss_unary_expr.138:                              #
        subq      $112, %rsp                                    #1361.1
        movq      %rbx, -24(%rbp)                               #1361.1
..___tag_value_nss_unary_expr.141:                              #
        movq      %rdi, -56(%rbp)                               #1361.1
        movq      %rsi, -48(%rbp)                               #1361.1
        movq      %rdx, -40(%rbp)                               #1361.1
        movq      %rcx, -32(%rbp)                               #1361.1
        movl      %r8d, -16(%rbp)                               #1361.1
..LN1153:
   .loc    3  1362
        movq      -56(%rbp), %rax                               #1362.5
        movq      -48(%rbp), %rdx                               #1362.5
        cmpq      %rdx, %rax                                    #1362.5
        jb        ..B12.3       # Prob 50%                      #1362.5
                                # LOE
..B12.2:                        # Preds ..B12.1
        lea       _2__STRING.3.0(%rip), %rax                    #1362.5
        lea       _2__STRING.1.0(%rip), %rdx                    #1362.5
        lea       __$U3.0(%rip), %rcx                           #1362.5
        movq      %rax, %rdi                                    #1362.5
        movq      %rdx, %rsi                                    #1362.5
        movl      $1362, %edx                                   #1362.5
        call      __assert_fail@PLT                             #1362.5
                                # LOE
..B12.3:                        # Preds ..B12.1
..LN1155:
   .loc    3  1363
        movq      -56(%rbp), %rax                               #1363.12
        movzbl    (%rax), %eax                                  #1363.12
        movsbq    %al, %rax                                     #1363.12
..LN1157:
        cmpl      $33, %eax                                     #1363.17
        je        ..B12.6       # Prob 50%                      #1363.17
                                # LOE
..B12.4:                        # Preds ..B12.3
..LN1159:
   .loc    3  1364
        movq      -56(%rbp), %rax                               #1364.35
..LN1161:
        movq      -48(%rbp), %rdx                               #1364.38
..LN1163:
        movq      -40(%rbp), %rcx                               #1364.43
..LN1165:
        movq      -32(%rbp), %rbx                               #1364.49
..LN1167:
        movl      -16(%rbp), %esi                               #1364.57
..LN1169:
        movq      %rax, %rdi                                    #1364.16
        movl      %esi, -64(%rbp)                               #1364.16
        movq      %rdx, %rsi                                    #1364.16
        movq      %rcx, %rdx                                    #1364.16
        movq      %rbx, %rcx                                    #1364.16
        movl      -64(%rbp), %eax                               #1364.16
        movl      %eax, %r8d                                    #1364.16
        call      nss_primary_expr@PLT                          #1364.16
                                # LOE rax
..B12.19:                       # Preds ..B12.4
        movq      %rax, -80(%rbp)                               #1364.16
                                # LOE
..B12.5:                        # Preds ..B12.19
        movq      -80(%rbp), %rax                               #1364.16
        movq      -24(%rbp), %rbx                               #1364.16
..___tag_value_nss_unary_expr.142:                              #
        leave                                                   #1364.16
..___tag_value_nss_unary_expr.144:                              #
        ret                                                     #1364.16
..___tag_value_nss_unary_expr.145:                              #
                                # LOE
..B12.6:                        # Preds ..B12.3
..LN1171:
   .loc    3  1374
        movq      -56(%rbp), %rax                               #1374.19
        incq      %rax                                          #1374.19
..LN1173:
        movq      -48(%rbp), %rdx                               #1374.26
..LN1175:
        movq      %rax, %rdi                                    #1374.9
        movq      %rdx, %rsi                                    #1374.9
        call      nss_sob@PLT                                   #1374.9
                                # LOE rax
..B12.20:                       # Preds ..B12.6
        movq      %rax, -72(%rbp)                               #1374.9
                                # LOE
..B12.7:                        # Preds ..B12.20
..LN1177:
        movq      -72(%rbp), %rax                               #1374.5
        movq      %rax, -56(%rbp)                               #1374.5
..LN1179:
   .loc    3  1375
        movq      -56(%rbp), %rax                               #1375.10
..LN1181:
        movq      -48(%rbp), %rdx                               #1375.15
        cmpq      %rdx, %rax                                    #1375.15
        jne       ..B12.9       # Prob 50%                      #1375.15
                                # LOE
..B12.8:                        # Preds ..B12.7
..LN1183:
   .loc    3  1376
        movq      -32(%rbp), %rax                               #1376.9
        movl      $22, (%rax)                                   #1376.9
        jmp       ..B12.16      # Prob 100%                     #1376.9
                                # LOE
..B12.9:                        # Preds ..B12.7
..LN1185:
   .loc    3  1379
        movl      $16, %edi                                     #1379.25
        call      malloc@PLT                                    #1379.25
                                # LOE rax
..B12.21:                       # Preds ..B12.9
        movq      %rax, -88(%rbp)                               #1379.25
                                # LOE
..B12.10:                       # Preds ..B12.21
..LN1187:
        movq      -88(%rbp), %rax                               #1379.23
        movq      %rax, -96(%rbp)                               #1379.23
..LN1189:
   .loc    3  1380
        movq      -96(%rbp), %rax                               #1380.14
..LN1191:
        testq     %rax, %rax                                    #1380.19
        jne       ..B12.12      # Prob 50%                      #1380.19
                                # LOE
..B12.11:                       # Preds ..B12.10
..LN1193:
   .loc    3  1381
        call      __errno_location@PLT                          #1381.24
                                # LOE rax
..B12.22:                       # Preds ..B12.11
..LN1195:
        movq      -32(%rbp), %rdx                               #1381.13
..LN1197:
        movl      (%rax), %eax                                  #1381.24
..LN1199:
        movl      %eax, (%rdx)                                  #1381.13
        jmp       ..B12.16      # Prob 100%                     #1381.13
                                # LOE
..B12.12:                       # Preds ..B12.10
..LN1201:
   .loc    3  1384
        movq      -96(%rbp), %rax                               #1384.13
        movl      $12, (%rax)                                   #1384.13
..LN1203:
   .loc    3  1385
        movq      -96(%rbp), %rax                               #1385.13
        movl      $13, 4(%rax)                                  #1385.13
..LN1205:
   .loc    3  1386
        movq      -96(%rbp), %rax                               #1386.13
        movq      $0, 8(%rax)                                   #1386.13
..LN1207:
   .loc    3  1387
        movq      -40(%rbp), %rax                               #1387.13
..LN1209:
        movq      -96(%rbp), %rdx                               #1387.22
..LN1211:
        movq      %rdx, (%rax)                                  #1387.13
..LN1213:
   .loc    3  1389
        movq      -56(%rbp), %rax                               #1389.34
..LN1215:
        movq      -48(%rbp), %rdx                               #1389.37
..LN1217:
        movq      -96(%rbp), %rcx                               #1389.44
        addq      $8, %rcx                                      #1389.44
..LN1219:
        movq      -32(%rbp), %rbx                               #1389.61
..LN1221:
        movl      -16(%rbp), %esi                               #1389.69
..LN1223:
        movq      %rax, %rdi                                    #1389.17
        movl      %esi, -104(%rbp)                              #1389.17
        movq      %rdx, %rsi                                    #1389.17
        movq      %rcx, %rdx                                    #1389.17
        movq      %rbx, %rcx                                    #1389.17
        movl      -104(%rbp), %eax                              #1389.17
        movl      %eax, %r8d                                    #1389.17
        call      nss_unary_expr@PLT                            #1389.17
                                # LOE rax
..B12.23:                       # Preds ..B12.12
        movq      %rax, -112(%rbp)                              #1389.17
                                # LOE
..B12.13:                       # Preds ..B12.23
..LN1225:
        movq      -112(%rbp), %rax                              #1389.13
        movq      %rax, -56(%rbp)                               #1389.13
..LN1227:
   .loc    3  1390
        movq      -32(%rbp), %rax                               #1390.13
        movl      (%rax), %eax                                  #1390.13
        testl     %eax, %eax                                    #1390.13
        jne       ..B12.16      # Prob 50%                      #1390.13
                                # LOE
..B12.14:                       # Preds ..B12.13
        movq      -96(%rbp), %rax                               #1390.13
        movq      8(%rax), %rax                                 #1390.13
        testq     %rax, %rax                                    #1390.13
        jne       ..B12.16      # Prob 50%                      #1390.13
                                # LOE
..B12.15:                       # Preds ..B12.14
        lea       _2__STRING.4.0(%rip), %rax                    #1390.13
        lea       _2__STRING.1.0(%rip), %rdx                    #1390.13
        lea       __$U3.0(%rip), %rcx                           #1390.13
        movq      %rax, %rdi                                    #1390.13
        movq      %rdx, %rsi                                    #1390.13
        movl      $1390, %edx                                   #1390.13
        call      __assert_fail@PLT                             #1390.13
                                # LOE
..B12.16:                       # Preds ..B12.8 ..B12.22 ..B12.14 ..B12.13
..LN1229:
   .loc    3  1393
        movq      -56(%rbp), %rax                               #1393.12
        movq      -24(%rbp), %rbx                               #1393.12
..___tag_value_nss_unary_expr.147:                              #
        leave                                                   #1393.12
..___tag_value_nss_unary_expr.149:                              #
        ret                                                     #1393.12
        .align    2,0x90
..___tag_value_nss_unary_expr.150:                              #
                                # LOE
# mark_end;
	.type	nss_unary_expr,@function
	.size	nss_unary_expr,.-nss_unary_expr
.LNnss_unary_expr:
	.data
# -- End  nss_unary_expr
	.text
# -- Begin  nss_expr
# mark_begin;
       .align    2,0x90
nss_expr:
# parameter 1(p): %rdi
# parameter 2(end): %rsi
# parameter 3(expr): %rdx
# parameter 4(status): %rcx
# parameter 5(positional): %r8d
..B13.1:                        # Preds ..B13.0
..___tag_value_nss_expr.153:                                    #
..LN1231:
   .loc    3  1406
        pushq     %rbp                                          #1406.1
        movq      %rsp, %rbp                                    #1406.1
..___tag_value_nss_expr.154:                                    #
        subq      $128, %rsp                                    #1406.1
        movq      %rbx, -24(%rbp)                               #1406.1
..___tag_value_nss_expr.157:                                    #
        movq      %rdi, -64(%rbp)                               #1406.1
        movq      %rsi, -56(%rbp)                               #1406.1
        movq      %rdx, -48(%rbp)                               #1406.1
        movq      %rcx, -40(%rbp)                               #1406.1
        movl      %r8d, -16(%rbp)                               #1406.1
..LN1233:
   .loc    3  1407
        movq      -48(%rbp), %rax                               #1407.5
        movq      $0, (%rax)                                    #1407.5
..LN1235:
   .loc    3  1409
        movq      -64(%rbp), %rax                               #1409.19
..LN1237:
        movq      -56(%rbp), %rdx                               #1409.22
..LN1239:
        movq      %rax, %rdi                                    #1409.9
        movq      %rdx, %rsi                                    #1409.9
        call      nss_sob@PLT                                   #1409.9
                                # LOE rax
..B13.32:                       # Preds ..B13.1
        movq      %rax, -32(%rbp)                               #1409.9
                                # LOE
..B13.2:                        # Preds ..B13.32
..LN1241:
        movq      -32(%rbp), %rax                               #1409.5
        movq      %rax, -64(%rbp)                               #1409.5
..LN1243:
   .loc    3  1410
        movq      -64(%rbp), %rax                               #1410.10
..LN1245:
        movq      -56(%rbp), %rdx                               #1410.15
        cmpq      %rdx, %rax                                    #1410.15
        je        ..B13.29      # Prob 50%                      #1410.15
                                # LOE
..B13.3:                        # Preds ..B13.2
..LN1247:
   .loc    3  1412
        movq      -64(%rbp), %rax                               #1412.30
..LN1249:
        movq      -56(%rbp), %rdx                               #1412.33
..LN1251:
        movq      -48(%rbp), %rcx                               #1412.38
..LN1253:
        movq      -40(%rbp), %rbx                               #1412.44
..LN1255:
        movl      -16(%rbp), %esi                               #1412.52
..LN1257:
        movq      %rax, %rdi                                    #1412.13
        movl      %esi, -72(%rbp)                               #1412.13
        movq      %rdx, %rsi                                    #1412.13
        movq      %rcx, %rdx                                    #1412.13
        movq      %rbx, %rcx                                    #1412.13
        movl      -72(%rbp), %eax                               #1412.13
        movl      %eax, %r8d                                    #1412.13
        call      nss_unary_expr@PLT                            #1412.13
                                # LOE rax
..B13.33:                       # Preds ..B13.3
        movq      %rax, -80(%rbp)                               #1412.13
                                # LOE
..B13.4:                        # Preds ..B13.33
..LN1259:
        movq      -80(%rbp), %rax                               #1412.9
        movq      %rax, -64(%rbp)                               #1412.9
..LN1261:
   .loc    3  1413
        movq      -40(%rbp), %rax                               #1413.16
        movl      (%rax), %eax                                  #1413.16
..LN1263:
        testl     %eax, %eax                                    #1413.26
        jne       ..B13.29      # Prob 50%                      #1413.26
                                # LOE
..B13.5:                        # Preds ..B13.4
..LN1265:
   .loc    3  1415
        movq      -64(%rbp), %rax                               #1415.27
..LN1267:
        movq      -56(%rbp), %rdx                               #1415.30
..LN1269:
        movq      %rax, %rdi                                    #1415.17
        movq      %rdx, %rsi                                    #1415.17
        call      nss_sob@PLT                                   #1415.17
                                # LOE rax
..B13.34:                       # Preds ..B13.5
        movq      %rax, -88(%rbp)                               #1415.17
                                # LOE
..B13.6:                        # Preds ..B13.34
..LN1271:
        movq      -88(%rbp), %rax                               #1415.13
        movq      %rax, -64(%rbp)                               #1415.13
..LN1273:
   .loc    3  1416
        movq      -64(%rbp), %rax                               #1416.18
..LN1275:
        movq      -56(%rbp), %rdx                               #1416.23
        cmpq      %rdx, %rax                                    #1416.23
        je        ..B13.29      # Prob 50%                      #1416.23
                                # LOE
..B13.7:                        # Preds ..B13.6
..LN1277:
   .loc    3  1420
        movq      -48(%rbp), %rax                               #1420.17
        movq      (%rax), %rax                                  #1420.17
        testq     %rax, %rax                                    #1420.17
        jne       ..B13.9       # Prob 50%                      #1420.17
                                # LOE
..B13.8:                        # Preds ..B13.7
        lea       _2__STRING.5.0(%rip), %rax                    #1420.17
        lea       _2__STRING.1.0(%rip), %rdx                    #1420.17
        lea       __$U4.0(%rip), %rcx                           #1420.17
        movq      %rax, %rdi                                    #1420.17
        movq      %rdx, %rsi                                    #1420.17
        movl      $1420, %edx                                   #1420.17
        call      __assert_fail@PLT                             #1420.17
                                # LOE
..B13.9:                        # Preds ..B13.7
..LN1279:
   .loc    3  1422
        movq      -64(%rbp), %rax                               #1422.28
        movzbl    (%rax), %eax                                  #1422.28
        movsbq    %al, %rax                                     #1422.28
        movl      %eax, -92(%rbp)                               #1422.28
        incq      -64(%rbp)                                     #1422.28
        cmpl      $41, %eax                                     #1422.28
        je        ..B13.12      # Prob 50%                      #1422.28
                                # LOE
..B13.10:                       # Preds ..B13.9
        movl      -92(%rbp), %eax                               #1422.28
        cmpl      $38, %eax                                     #1422.28
        je        ..B13.13      # Prob 50%                      #1422.28
                                # LOE
..B13.11:                       # Preds ..B13.10
        movl      -92(%rbp), %eax                               #1422.28
        cmpl      $124, %eax                                    #1422.28
        je        ..B13.17      # Prob 50%                      #1422.28
        jmp       ..B13.21      # Prob 100%                     #1422.28
                                # LOE
..B13.12:                       # Preds ..B13.9
..LN1281:
   .loc    3  1425
        movq      -64(%rbp), %rax                               #1425.28
..LN1283:
        decq      %rax                                          #1425.32
        movq      -24(%rbp), %rbx                               #1425.32
..___tag_value_nss_expr.158:                                    #
        leave                                                   #1425.32
..___tag_value_nss_expr.160:                                    #
        ret                                                     #1425.32
..___tag_value_nss_expr.161:                                    #
                                # LOE
..B13.13:                       # Preds ..B13.10
..LN1285:
   .loc    3  1427
        movq      -64(%rbp), %rax                               #1427.26
..LN1287:
        movq      -56(%rbp), %rdx                               #1427.30
        cmpq      %rdx, %rax                                    #1427.30
        jae       ..B13.16      # Prob 50%                      #1427.30
                                # LOE
..B13.14:                       # Preds ..B13.13
..LN1289:
        movq      -64(%rbp), %rax                               #1427.39
        movzbl    (%rax), %eax                                  #1427.39
        movsbq    %al, %rax                                     #1427.39
..LN1291:
        cmpl      $38, %eax                                     #1427.44
        jne       ..B13.16      # Prob 50%                      #1427.44
                                # LOE
..B13.15:                       # Preds ..B13.14
..LN1293:
   .loc    3  1428
        incq      -64(%rbp)                                     #1428.28
                                # LOE
..B13.16:                       # Preds ..B13.15 ..B13.14 ..B13.13
..LN1295:
   .loc    3  1429
        movl      $16, -96(%rbp)                                #1429.21
        jmp       ..B13.22      # Prob 100%                     #1429.21
                                # LOE
..B13.17:                       # Preds ..B13.11
..LN1297:
   .loc    3  1432
        movq      -64(%rbp), %rax                               #1432.26
..LN1299:
        movq      -56(%rbp), %rdx                               #1432.30
        cmpq      %rdx, %rax                                    #1432.30
        jae       ..B13.20      # Prob 50%                      #1432.30
                                # LOE
..B13.18:                       # Preds ..B13.17
..LN1301:
        movq      -64(%rbp), %rax                               #1432.39
        movzbl    (%rax), %eax                                  #1432.39
        movsbq    %al, %rax                                     #1432.39
..LN1303:
        cmpl      $124, %eax                                    #1432.44
        jne       ..B13.20      # Prob 50%                      #1432.44
                                # LOE
..B13.19:                       # Preds ..B13.18
..LN1305:
   .loc    3  1433
        incq      -64(%rbp)                                     #1433.28
                                # LOE
..B13.20:                       # Preds ..B13.19 ..B13.18 ..B13.17
..LN1307:
   .loc    3  1434
        movl      $17, -96(%rbp)                                #1434.21
        jmp       ..B13.22      # Prob 100%                     #1434.21
                                # LOE
..B13.21:                       # Preds ..B13.11
..LN1309:
   .loc    3  1438
        movq      -40(%rbp), %rax                               #1438.21
        movl      $22, (%rax)                                   #1438.21
..LN1311:
   .loc    3  1439
        movq      -64(%rbp), %rax                               #1439.28
..LN1313:
        decq      %rax                                          #1439.32
        movq      -24(%rbp), %rbx                               #1439.32
..___tag_value_nss_expr.163:                                    #
        leave                                                   #1439.32
..___tag_value_nss_expr.165:                                    #
        ret                                                     #1439.32
..___tag_value_nss_expr.166:                                    #
                                # LOE
..B13.22:                       # Preds ..B13.16 ..B13.20
..LN1315:
   .loc    3  1451
        movl      $24, %edi                                     #1451.21
        call      malloc@PLT                                    #1451.21
                                # LOE rax
..B13.35:                       # Preds ..B13.22
        movq      %rax, -104(%rbp)                              #1451.21
                                # LOE
..B13.23:                       # Preds ..B13.35
..LN1317:
        movq      -104(%rbp), %rax                              #1451.17
        movq      %rax, -112(%rbp)                              #1451.17
..LN1319:
   .loc    3  1452
        movq      -112(%rbp), %rax                              #1452.22
..LN1321:
        testq     %rax, %rax                                    #1452.27
        jne       ..B13.25      # Prob 50%                      #1452.27
                                # LOE
..B13.24:                       # Preds ..B13.23
..LN1323:
   .loc    3  1454
        call      __errno_location@PLT                          #1454.32
                                # LOE rax
..B13.36:                       # Preds ..B13.24
..LN1325:
        movq      -40(%rbp), %rdx                               #1454.21
..LN1327:
        movl      (%rax), %eax                                  #1454.32
..LN1329:
        movl      %eax, (%rdx)                                  #1454.21
..LN1331:
   .loc    3  1455
        movq      -64(%rbp), %rax                               #1455.28
        movq      -24(%rbp), %rbx                               #1455.28
..___tag_value_nss_expr.168:                                    #
        leave                                                   #1455.28
..___tag_value_nss_expr.170:                                    #
        ret                                                     #1455.28
..___tag_value_nss_expr.171:                                    #
                                # LOE
..B13.25:                       # Preds ..B13.23
..LN1333:
   .loc    3  1458
        movq      -112(%rbp), %rax                              #1458.17
        movl      $11, (%rax)                                   #1458.17
..LN1335:
   .loc    3  1459
        movq      -112(%rbp), %rax                              #1459.17
..LN1337:
        movl      -96(%rbp), %edx                               #1459.37
..LN1339:
        movl      %edx, 4(%rax)                                 #1459.17
..LN1341:
   .loc    3  1460
        movq      -48(%rbp), %rax                               #1460.41
..LN1343:
        movq      -112(%rbp), %rdx                              #1460.17
..LN1345:
        movq      (%rax), %rax                                  #1460.41
..LN1347:
        movq      %rax, 8(%rdx)                                 #1460.17
..LN1349:
   .loc    3  1461
        movq      -48(%rbp), %rax                               #1461.17
..LN1351:
        movq      -112(%rbp), %rdx                              #1461.26
..LN1353:
        movq      %rdx, (%rax)                                  #1461.17
..LN1355:
   .loc    3  1464
        movq      -64(%rbp), %rax                               #1464.32
..LN1357:
        movq      -56(%rbp), %rdx                               #1464.35
..LN1359:
        movq      -112(%rbp), %rcx                              #1464.42
        addq      $16, %rcx                                     #1464.42
..LN1361:
        movq      -40(%rbp), %rbx                               #1464.64
..LN1363:
        movl      -16(%rbp), %esi                               #1464.72
..LN1365:
        movq      %rax, %rdi                                    #1464.21
        movl      %esi, -120(%rbp)                              #1464.21
        movq      %rdx, %rsi                                    #1464.21
        movq      %rcx, %rdx                                    #1464.21
        movq      %rbx, %rcx                                    #1464.21
        movl      -120(%rbp), %eax                              #1464.21
        movl      %eax, %r8d                                    #1464.21
        call      nss_expr@PLT                                  #1464.21
                                # LOE rax
..B13.37:                       # Preds ..B13.25
        movq      %rax, -128(%rbp)                              #1464.21
                                # LOE
..B13.26:                       # Preds ..B13.37
..LN1367:
        movq      -128(%rbp), %rax                              #1464.17
        movq      %rax, -64(%rbp)                               #1464.17
..LN1369:
   .loc    3  1465
        movq      -40(%rbp), %rax                               #1465.17
        movl      (%rax), %eax                                  #1465.17
        testl     %eax, %eax                                    #1465.17
        jne       ..B13.29      # Prob 50%                      #1465.17
                                # LOE
..B13.27:                       # Preds ..B13.26
        movq      -112(%rbp), %rax                              #1465.17
        movq      16(%rax), %rax                                #1465.17
        testq     %rax, %rax                                    #1465.17
        jne       ..B13.29      # Prob 50%                      #1465.17
                                # LOE
..B13.28:                       # Preds ..B13.27
        lea       _2__STRING.6.0(%rip), %rax                    #1465.17
        lea       _2__STRING.1.0(%rip), %rdx                    #1465.17
        lea       __$U4.0(%rip), %rcx                           #1465.17
        movq      %rax, %rdi                                    #1465.17
        movq      %rdx, %rsi                                    #1465.17
        movl      $1465, %edx                                   #1465.17
        call      __assert_fail@PLT                             #1465.17
                                # LOE
..B13.29:                       # Preds ..B13.27 ..B13.26 ..B13.6 ..B13.4 ..B13.2
                                #      
..LN1371:
   .loc    3  1470
        movq      -64(%rbp), %rax                               #1470.12
        movq      -24(%rbp), %rbx                               #1470.12
..___tag_value_nss_expr.173:                                    #
        leave                                                   #1470.12
..___tag_value_nss_expr.175:                                    #
        ret                                                     #1470.12
        .align    2,0x90
..___tag_value_nss_expr.176:                                    #
                                # LOE
# mark_end;
	.type	nss_expr,@function
	.size	nss_expr,.-nss_expr
.LNnss_expr:
	.data
# -- End  nss_expr
	.text
# -- Begin  NucStrstrInit
# mark_begin;
       .align    2,0x90
NucStrstrInit:
..B14.1:                        # Preds ..B14.0
..___tag_value_NucStrstrInit.179:                               #
..LN1373:
   .loc    3  1484
        pushq     %rbp                                          #1484.1
        movq      %rsp, %rbp                                    #1484.1
..___tag_value_NucStrstrInit.180:                               #
        subq      $64, %rsp                                     #1484.1
..LN1375:
   .loc    3  1487
        lea       _2__STRING.7.0(%rip), %rax                    #1487.29
        movq      %rax, -64(%rbp)                               #1487.29
..LN1377:
   .loc    3  1488
        lea       _2__STRING.8.0(%rip), %rax                    #1488.25
        movq      %rax, -56(%rbp)                               #1488.25
..LN1379:
   .loc    3  1491
        lea       fasta_2na_map.0(%rip), %rax                   #1491.14
..LN1381:
        movq      %rax, %rdi                                    #1491.5
        movl      $-1, %esi                                     #1491.5
        movl      $128, %edx                                    #1491.5
        call      memset@PLT                                    #1491.5
                                # LOE
..B14.2:                        # Preds ..B14.1
..LN1383:
   .loc    3  1492
        lea       fasta_4na_map.0(%rip), %rax                   #1492.14
..LN1385:
        movq      %rax, %rdi                                    #1492.5
        movl      $-1, %esi                                     #1492.5
        movl      $128, %edx                                    #1492.5
        call      memset@PLT                                    #1492.5
                                # LOE
..B14.3:                        # Preds ..B14.2
..LN1387:
   .loc    3  1495
        movl      $0, -40(%rbp)                                 #1495.11
..LN1389:
        movq      -64(%rbp), %rax                               #1495.22
..LN1391:
        movq      %rax, -48(%rbp)                               #1495.18
..LN1393:
        movq      -48(%rbp), %rax                               #1495.31
        movzbl    (%rax), %eax                                  #1495.31
        movsbq    %al, %rax                                     #1495.31
..LN1395:
        testl     %eax, %eax                                    #1495.42
        je        ..B14.7       # Prob 50%                      #1495.42
                                # LOE
..B14.5:                        # Preds ..B14.3 ..B14.6
..LN1397:
   .loc    3  1497
        movq      -48(%rbp), %rax                               #1497.14
        movzbl    (%rax), %eax                                  #1497.14
        movsbq    %al, %rax                                     #1497.14
..LN1399:
        movl      %eax, -36(%rbp)                               #1497.9
..LN1401:
   .loc    3  1498
        movl      -40(%rbp), %eax                               #1498.78
        movl      %eax, -32(%rbp)                               #1498.78
..LN1403:
        movl      -36(%rbp), %eax                               #1498.58
..LN1405:
        movl      %eax, %edi                                    #1498.48
        call      tolower@PLT                                   #1498.48
                                # LOE eax
..B14.20:                       # Preds ..B14.5
        movl      %eax, -28(%rbp)                               #1498.48
                                # LOE
..B14.6:                        # Preds ..B14.20
..LN1407:
        movl      -28(%rbp), %eax                               #1498.32
        movslq    %eax, %rax                                    #1498.32
        lea       fasta_2na_map.0(%rip), %rdx                   #1498.32
        movl      -32(%rbp), %ecx                               #1498.32
        movb      %cl, (%rax,%rdx)                              #1498.32
..LN1409:
        movl      -36(%rbp), %eax                               #1498.25
..LN1411:
        movslq    %eax, %rax                                    #1498.9
        lea       fasta_2na_map.0(%rip), %rdx                   #1498.9
        movl      -32(%rbp), %ecx                               #1498.9
        movb      %cl, (%rax,%rdx)                              #1498.9
..LN1413:
   .loc    3  1495
        incl      -40(%rbp)                                     #1495.48
..LN1415:
        incq      -48(%rbp)                                     #1495.54
..LN1417:
        movq      -48(%rbp), %rax                               #1495.31
        movzbl    (%rax), %eax                                  #1495.31
        movsbq    %al, %rax                                     #1495.31
..LN1419:
        testl     %eax, %eax                                    #1495.42
        jne       ..B14.5       # Prob 50%                      #1495.42
                                # LOE
..B14.7:                        # Preds ..B14.6 ..B14.3
..LN1421:
   .loc    3  1502
        movl      $0, -40(%rbp)                                 #1502.11
..LN1423:
        movq      -56(%rbp), %rax                               #1502.22
..LN1425:
        movq      %rax, -48(%rbp)                               #1502.18
..LN1427:
        movq      -48(%rbp), %rax                               #1502.31
        movzbl    (%rax), %eax                                  #1502.31
        movsbq    %al, %rax                                     #1502.31
..LN1429:
        testl     %eax, %eax                                    #1502.42
        je        ..B14.11      # Prob 50%                      #1502.42
                                # LOE
..B14.9:                        # Preds ..B14.7 ..B14.10
..LN1431:
   .loc    3  1504
        movq      -48(%rbp), %rax                               #1504.14
        movzbl    (%rax), %eax                                  #1504.14
        movsbq    %al, %rax                                     #1504.14
..LN1433:
        movl      %eax, -36(%rbp)                               #1504.9
..LN1435:
   .loc    3  1505
        movl      -40(%rbp), %eax                               #1505.78
        movl      %eax, -24(%rbp)                               #1505.78
..LN1437:
        movl      -36(%rbp), %eax                               #1505.58
..LN1439:
        movl      %eax, %edi                                    #1505.48
        call      tolower@PLT                                   #1505.48
                                # LOE eax
..B14.21:                       # Preds ..B14.9
        movl      %eax, -20(%rbp)                               #1505.48
                                # LOE
..B14.10:                       # Preds ..B14.21
..LN1441:
        movl      -20(%rbp), %eax                               #1505.32
        movslq    %eax, %rax                                    #1505.32
        lea       fasta_4na_map.0(%rip), %rdx                   #1505.32
        movl      -24(%rbp), %ecx                               #1505.32
        movb      %cl, (%rax,%rdx)                              #1505.32
..LN1443:
        movl      -36(%rbp), %eax                               #1505.25
..LN1445:
        movslq    %eax, %rax                                    #1505.9
        lea       fasta_4na_map.0(%rip), %rdx                   #1505.9
        movl      -24(%rbp), %ecx                               #1505.9
        movb      %cl, (%rax,%rdx)                              #1505.9
..LN1447:
   .loc    3  1502
        incl      -40(%rbp)                                     #1502.48
..LN1449:
        incq      -48(%rbp)                                     #1502.54
..LN1451:
        movq      -48(%rbp), %rax                               #1502.31
        movzbl    (%rax), %eax                                  #1502.31
        movsbq    %al, %rax                                     #1502.31
..LN1453:
        testl     %eax, %eax                                    #1502.42
        jne       ..B14.9       # Prob 50%                      #1502.42
                                # LOE
..B14.11:                       # Preds ..B14.10 ..B14.7
..LN1455:
   .loc    3  1510
        movl      $0, -40(%rbp)                                 #1510.11
..LN1457:
        movl      -40(%rbp), %eax                               #1510.18
..LN1459:
        cmpl      $256, %eax                                    #1510.22
        jae       ..B14.17      # Prob 50%                      #1510.22
                                # LOE
..B14.13:                       # Preds ..B14.11 ..B14.16
..LN1461:
   .loc    3  1511
        movl      -40(%rbp), %eax                               #1511.28
        movl      %eax, %eax                                    #1511.28
        lea       expand_2na.0(%rip), %rdx                      #1511.28
        movzwl    (%rdx,%rax,2), %eax                           #1511.28
        movw      %ax, -16(%rbp)                                #1511.28
        xorl      %eax, %eax                                    #1511.28
        testl     %eax, %eax                                    #1511.28
        jne       ..B14.15      # Prob 50%                      #1511.28
                                # LOE
..B14.14:                       # Preds ..B14.13
..LN1463:
        movzwl    -16(%rbp), %eax                               #1511.0
        rorw      $8, %ax                                       #1511.0
        movw      %ax, -12(%rbp)                                #1511.0
        jmp       ..B14.16      # Prob 100%                     #1511.0
                                # LOE
..B14.15:                       # Preds ..B14.13
..LN1465:
        movzwl    -16(%rbp), %eax                               #1511.28
        shrl      $8, %eax                                      #1511.28
        movzbl    %al, %eax                                     #1511.28
        movzwl    -16(%rbp), %edx                               #1511.28
        movzbl    %dl, %edx                                     #1511.28
        shll      $8, %edx                                      #1511.28
        orl       %edx, %eax                                    #1511.28
        movw      %ax, -12(%rbp)                                #1511.28
                                # LOE
..B14.16:                       # Preds ..B14.15 ..B14.14
..LN1467:
        movl      -40(%rbp), %eax                               #1511.22
..LN1469:
        movl      %eax, %eax                                    #1511.9
        lea       expand_2na.0(%rip), %rdx                      #1511.9
..LN1471:
        movzwl    -12(%rbp), %ecx                               #1511.28
..LN1473:
        movw      %cx, (%rdx,%rax,2)                            #1511.9
..LN1475:
   .loc    3  1510
        incl      -40(%rbp)                                     #1510.30
..LN1477:
        movl      -40(%rbp), %eax                               #1510.18
..LN1479:
        cmpl      $256, %eax                                    #1510.22
        jb        ..B14.13      # Prob 50%                      #1510.22
                                # LOE
..B14.17:                       # Preds ..B14.16 ..B14.11
..LN1481:
   .loc    3  1513
        leave                                                   #1513.1
..___tag_value_NucStrstrInit.184:                               #
        ret                                                     #1513.1
        .align    2,0x90
..___tag_value_NucStrstrInit.185:                               #
                                # LOE
# mark_end;
	.type	NucStrstrInit,@function
	.size	NucStrstrInit,.-NucStrstrInit
.LNNucStrstrInit:
	.data
# -- End  NucStrstrInit
	.text
# -- Begin  NucStrstrMake
# mark_begin;
       .align    2,0x90
	.globl NucStrstrMake
NucStrstrMake:
# parameter 1(nss): %rdi
# parameter 2(positional): %esi
# parameter 3(query): %rdx
# parameter 4(len): %ecx
..B15.1:                        # Preds ..B15.0
..___tag_value_NucStrstrMake.188:                               #
..LN1483:
   .loc    3  1539
        pushq     %rbp                                          #1539.1
        movq      %rsp, %rbp                                    #1539.1
..___tag_value_NucStrstrMake.189:                               #
        subq      $80, %rsp                                     #1539.1
        movq      %rbx, -32(%rbp)                               #1539.1
..___tag_value_NucStrstrMake.192:                               #
        movq      %rdi, -48(%rbp)                               #1539.1
        movl      %esi, -24(%rbp)                               #1539.1
        movq      %rdx, -40(%rbp)                               #1539.1
        movl      %ecx, -16(%rbp)                               #1539.1
..LN1485:
   .loc    3  1540
        movq      -48(%rbp), %rax                               #1540.10
..LN1487:
        testq     %rax, %rax                                    #1540.17
        je        ..B15.14      # Prob 50%                      #1540.17
                                # LOE
..B15.2:                        # Preds ..B15.1
..LN1489:
   .loc    3  1542
        movq      -40(%rbp), %rax                               #1542.14
..LN1491:
        testq     %rax, %rax                                    #1542.23
        je        ..B15.13      # Prob 50%                      #1542.23
                                # LOE
..B15.3:                        # Preds ..B15.2
..LN1493:
        movl      -16(%rbp), %eax                               #1542.31
..LN1495:
        testl     %eax, %eax                                    #1542.38
        je        ..B15.13      # Prob 50%                      #1542.38
                                # LOE
..B15.4:                        # Preds ..B15.3
..LN1497:
   .loc    3  1544
        movl      $0, -56(%rbp)                                 #1544.24
..LN1499:
   .loc    3  1547
        lea       fasta_2na_map.0(%rip), %rax                   #1547.18
        movzbl    (%rax), %eax                                  #1547.18
        movsbq    %al, %rax                                     #1547.18
..LN1501:
        testl     %eax, %eax                                    #1547.41
        jne       ..B15.6       # Prob 50%                      #1547.41
                                # LOE
..B15.5:                        # Preds ..B15.4
..LN1503:
   .loc    3  1548
        call      NucStrstrInit@PLT                             #1548.17
                                # LOE
..B15.6:                        # Preds ..B15.5 ..B15.4
..LN1505:
   .loc    3  1550
        movl      -16(%rbp), %eax                               #1550.27
..LN1507:
        movl      %eax, %eax                                    #1550.19
        addq      -40(%rbp), %rax                               #1550.19
..LN1509:
        movq      %rax, -80(%rbp)                               #1550.13
..LN1511:
   .loc    3  1551
        movq      -40(%rbp), %rax                               #1551.32
..LN1513:
        movq      -80(%rbp), %rdx                               #1551.39
..LN1515:
        movq      -48(%rbp), %rcx                               #1551.44
..LN1517:
        lea       -56(%rbp), %rbx                               #1551.49
..LN1519:
        movl      -24(%rbp), %esi                               #1551.59
..LN1521:
        movq      %rax, %rdi                                    #1551.21
        movl      %esi, -64(%rbp)                               #1551.21
        movq      %rdx, %rsi                                    #1551.21
        movq      %rcx, %rdx                                    #1551.21
        movq      %rbx, %rcx                                    #1551.21
        movl      -64(%rbp), %eax                               #1551.21
        movl      %eax, %r8d                                    #1551.21
        call      nss_expr@PLT                                  #1551.21
                                # LOE rax
..B15.17:                       # Preds ..B15.6
        movq      %rax, -72(%rbp)                               #1551.21
                                # LOE
..B15.7:                        # Preds ..B15.17
..LN1523:
        movq      -72(%rbp), %rax                               #1551.13
        movq      %rax, -40(%rbp)                               #1551.13
..LN1525:
   .loc    3  1552
        movl      -56(%rbp), %eax                               #1552.18
..LN1527:
        testl     %eax, %eax                                    #1552.28
        jne       ..B15.11      # Prob 50%                      #1552.28
                                # LOE
..B15.8:                        # Preds ..B15.7
..LN1529:
   .loc    3  1554
        movq      -40(%rbp), %rax                               #1554.22
..LN1531:
        movq      -80(%rbp), %rdx                               #1554.31
        cmpq      %rdx, %rax                                    #1554.31
        jne       ..B15.10      # Prob 50%                      #1554.31
                                # LOE
..B15.9:                        # Preds ..B15.8
..LN1533:
   .loc    3  1555
        xorl      %eax, %eax                                    #1555.28
        movq      -32(%rbp), %rbx                               #1555.28
..___tag_value_NucStrstrMake.193:                               #
        leave                                                   #1555.28
..___tag_value_NucStrstrMake.195:                               #
        ret                                                     #1555.28
..___tag_value_NucStrstrMake.196:                               #
                                # LOE
..B15.10:                       # Preds ..B15.8
..LN1535:
   .loc    3  1557
        movl      $22, -56(%rbp)                                #1557.17
                                # LOE
..B15.11:                       # Preds ..B15.10 ..B15.7
..LN1537:
   .loc    3  1560
        movq      -48(%rbp), %rax                               #1560.32
        movq      (%rax), %rax                                  #1560.32
..LN1539:
        movq      %rax, %rdi                                    #1560.13
        call      NucStrstrWhack@PLT                            #1560.13
                                # LOE
..B15.12:                       # Preds ..B15.11
..LN1541:
   .loc    3  1561
        movq      -48(%rbp), %rax                               #1561.13
        movq      $0, (%rax)                                    #1561.13
..LN1543:
   .loc    3  1562
        movl      -56(%rbp), %eax                               #1562.20
        movq      -32(%rbp), %rbx                               #1562.20
..___tag_value_NucStrstrMake.198:                               #
        leave                                                   #1562.20
..___tag_value_NucStrstrMake.200:                               #
        ret                                                     #1562.20
..___tag_value_NucStrstrMake.201:                               #
                                # LOE
..B15.13:                       # Preds ..B15.3 ..B15.2
..LN1545:
   .loc    3  1565
        movq      -48(%rbp), %rax                               #1565.9
        movq      $0, (%rax)                                    #1565.9
                                # LOE
..B15.14:                       # Preds ..B15.13 ..B15.1
..LN1547:
   .loc    3  1567
        movl      $22, %eax                                     #1567.12
        movq      -32(%rbp), %rbx                               #1567.12
..___tag_value_NucStrstrMake.203:                               #
        leave                                                   #1567.12
..___tag_value_NucStrstrMake.205:                               #
        ret                                                     #1567.12
        .align    2,0x90
..___tag_value_NucStrstrMake.206:                               #
                                # LOE
# mark_end;
	.type	NucStrstrMake,@function
	.size	NucStrstrMake,.-NucStrstrMake
.LNNucStrstrMake:
	.data
# -- End  NucStrstrMake
	.text
# -- Begin  NucStrstrWhack
# mark_begin;
       .align    2,0x90
	.globl NucStrstrWhack
NucStrstrWhack:
# parameter 1(self): %rdi
..B16.1:                        # Preds ..B16.0
..___tag_value_NucStrstrWhack.209:                              #
..LN1549:
   .loc    3  1574
        pushq     %rbp                                          #1574.1
        movq      %rsp, %rbp                                    #1574.1
..___tag_value_NucStrstrWhack.210:                              #
        subq      $16, %rsp                                     #1574.1
        movq      %rdi, -8(%rbp)                                #1574.1
..LN1551:
   .loc    3  1575
        movq      -8(%rbp), %rax                                #1575.10
..LN1553:
        testq     %rax, %rax                                    #1575.18
        je        ..B16.22      # Prob 50%                      #1575.18
                                # LOE
..B16.2:                        # Preds ..B16.1
..LN1555:
   .loc    3  1577
        movq      -8(%rbp), %rax                                #1577.18
        movl      (%rax), %eax                                  #1577.18
        movl      %eax, -16(%rbp)                               #1577.18
..LN1557:
        cmpl      $12, %eax                                     #1577.9
        ja        ..B16.21      # Prob 50%                      #1577.9
                                # LOE
..B16.3:                        # Preds ..B16.2
        movl      -16(%rbp), %eax                               #1577.9
        movl      %eax, %eax                                    #1577.9
        lea       ..1..TPKT.18_0.0.13(%rip), %rdx               #1577.9
        movq      (%rdx,%rax,8), %rax                           #1577.9
        jmp       *%rax                                         #1577.9
                                # LOE
..1.18_0.TAG.0a.0.13:
..1.18_0.TAG.09.0.13:
..1.18_0.TAG.08.0.13:
..1.18_0.TAG.07.0.13:
..1.18_0.TAG.06.0.13:
..1.18_0.TAG.05.0.13:
..1.18_0.TAG.04.0.13:
..1.18_0.TAG.03.0.13:
..1.18_0.TAG.02.0.13:
..1.18_0.TAG.01.0.13:
..1.18_0.TAG.00.0.13:
..B16.15:                       # Preds ..B16.3 ..B16.3 ..B16.3 ..B16.3 ..B16.3
                                #       ..B16.3 ..B16.3 ..B16.3 ..B16.3 ..B16.3
                                #       ..B16.3
..LN1559:
   .loc    3  1593
        movq      -8(%rbp), %rax                                #1593.20
        movq      8(%rax), %rax                                 #1593.20
..LN1561:
        movq      %rax, -8(%rbp)                                #1593.13
        jmp       ..B16.21      # Prob 100%                     #1593.13
                                # LOE
..1.18_0.TAG.0b.0.13:
..B16.17:                       # Preds ..B16.3
..LN1563:
   .loc    3  1597
        movq      -8(%rbp), %rax                                #1597.30
        movq      8(%rax), %rax                                 #1597.30
..LN1565:
        movq      %rax, %rdi                                    #1597.13
        call      NucStrstrWhack@PLT                            #1597.13
                                # LOE
..B16.18:                       # Preds ..B16.17
..LN1567:
   .loc    3  1598
        movq      -8(%rbp), %rax                                #1598.30
        movq      16(%rax), %rax                                #1598.30
..LN1569:
        movq      %rax, %rdi                                    #1598.13
        call      NucStrstrWhack@PLT                            #1598.13
        jmp       ..B16.21      # Prob 100%                     #1598.13
                                # LOE
..1.18_0.TAG.0c.0.13:
..B16.20:                       # Preds ..B16.3
..LN1571:
   .loc    3  1601
        movq      -8(%rbp), %rax                                #1601.30
        movq      8(%rax), %rax                                 #1601.30
..LN1573:
        movq      %rax, %rdi                                    #1601.13
        call      NucStrstrWhack@PLT                            #1601.13
                                # LOE
..B16.21:                       # Preds ..B16.20 ..B16.18 ..B16.15 ..B16.2
..LN1575:
   .loc    3  1604
        movq      -8(%rbp), %rax                                #1604.16
..LN1577:
        movq      %rax, %rdi                                    #1604.9
        call      free@PLT                                      #1604.9
                                # LOE
..B16.22:                       # Preds ..B16.21 ..B16.1
..LN1579:
   .loc    3  1606
        leave                                                   #1606.1
..___tag_value_NucStrstrWhack.214:                              #
        ret                                                     #1606.1
        .align    2,0x90
..___tag_value_NucStrstrWhack.215:                              #
                                # LOE
# mark_end;
	.type	NucStrstrWhack,@function
	.size	NucStrstrWhack,.-NucStrstrWhack
.LNNucStrstrWhack:
	.section .data1, "wa"
	.align 32
	.align 32
..1..TPKT.18_0.0.13:
	.quad	..1.18_0.TAG.00.0.13
	.quad	..1.18_0.TAG.01.0.13
	.quad	..1.18_0.TAG.02.0.13
	.quad	..1.18_0.TAG.03.0.13
	.quad	..1.18_0.TAG.04.0.13
	.quad	..1.18_0.TAG.05.0.13
	.quad	..1.18_0.TAG.06.0.13
	.quad	..1.18_0.TAG.07.0.13
	.quad	..1.18_0.TAG.08.0.13
	.quad	..1.18_0.TAG.09.0.13
	.quad	..1.18_0.TAG.0a.0.13
	.quad	..1.18_0.TAG.0b.0.13
	.quad	..1.18_0.TAG.0c.0.13
	.data
# -- End  NucStrstrWhack
	.text
# -- Begin  prime_buffer_2na
# mark_begin;
       .align    2,0x90
prime_buffer_2na:
# parameter 1(src): %rdi
# parameter 2(ignore): %rsi
..B17.1:                        # Preds ..B17.0
..___tag_value_prime_buffer_2na.218:                            #
..LN1581:
   .loc    3  2129
        pushq     %rbp                                          #2129.1
        movq      %rsp, %rbp                                    #2129.1
..___tag_value_prime_buffer_2na.219:                            #
        subq      $32, %rsp                                     #2129.1
        movq      %rdi, -16(%rbp)                               #2129.1
        movq      %rsi, -8(%rbp)                                #2129.1
..LN1583:
   .loc    3  2132
        movq      -16(%rbp), %rax                               #2132.23
..LN1585:
        testq     $15, %rax                                     #2132.29
..LN1587:
        jne       ..B17.3       # Prob 50%                      #2132.37
                                # LOE
..B17.2:                        # Preds ..B17.1
..LN1589:
   .loc    3  2133
        movq      -16(%rbp), %rax                               #2133.54
        movdqa    (%rax), %xmm0                                 #2133.54
..LN1591:
        movdqa    %xmm0, -32(%rbp)                              #2133.9
        jmp       ..B17.4       # Prob 100%                     #2133.9
                                # LOE
..B17.3:                        # Preds ..B17.1
..LN1593:
   .loc    3  2135
        movq      -16(%rbp), %rax                               #2135.55
        movdqu    (%rax), %xmm0                                 #2135.55
..LN1595:
        movdqa    %xmm0, -32(%rbp)                              #2135.9
                                # LOE
..B17.4:                        # Preds ..B17.2 ..B17.3
..LN1597:
   .loc    3  2136
        movdqa    -32(%rbp), %xmm0                              #2136.12
        leave                                                   #2136.12
..___tag_value_prime_buffer_2na.223:                            #
        ret                                                     #2136.12
        .align    2,0x90
..___tag_value_prime_buffer_2na.224:                            #
                                # LOE
# mark_end;
	.type	prime_buffer_2na,@function
	.size	prime_buffer_2na,.-prime_buffer_2na
.LNprime_buffer_2na:
	.data
  .file   6 "/opt/intel/Compiler/11.0/081/include/emmintrin.h"
# -- End  prime_buffer_2na
	.text
# -- Begin  eval_2na_8
# mark_begin;
       .align    2,0x90
eval_2na_8:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B18.1:                        # Preds ..B18.0
..___tag_value_eval_2na_8.227:                                  #
..LN1599:
   .loc    3  2196
        pushq     %rbp                                          #2196.1
        movq      %rsp, %rbp                                    #2196.1
..___tag_value_eval_2na_8.228:                                  #
        subq      $320, %rsp                                    #2196.1
        movq      %rdi, -48(%rbp)                               #2196.1
        movq      %rsi, -40(%rbp)                               #2196.1
        movl      %edx, -24(%rbp)                               #2196.1
        movl      %ecx, -16(%rbp)                               #2196.1
..LN1601:
   .loc    3  2226
        movq      -48(%rbp), %rax                               #2226.25
        movl      4(%rax), %eax                                 #2226.25
..LN1603:
        movl      %eax, -32(%rbp)                               #2226.23
..LN1605:
   .loc    3  2229
        movl      -16(%rbp), %eax                               #2229.5
        movl      -32(%rbp), %edx                               #2229.5
        cmpl      %edx, %eax                                    #2229.5
        jae       ..B18.3       # Prob 50%                      #2229.5
                                # LOE
..B18.2:                        # Preds ..B18.1
        lea       _2__STRING.9.0(%rip), %rax                    #2229.5
        lea       _2__STRING.1.0(%rip), %rdx                    #2229.5
        lea       __$U5.0(%rip), %rcx                           #2229.5
        movq      %rax, %rdi                                    #2229.5
        movq      %rdx, %rsi                                    #2229.5
        movl      $2229, %edx                                   #2229.5
        call      __assert_fail@PLT                             #2229.5
                                # LOE
..B18.3:                        # Preds ..B18.1
..LN1607:
   .loc    3  2233
        movl      -24(%rbp), %eax                               #2233.12
..LN1609:
        addl      -16(%rbp), %eax                               #2233.5
        movl      %eax, -16(%rbp)                               #2233.5
..LN1611:
   .loc    3  2236
        movl      -24(%rbp), %eax                               #2236.42
..LN1613:
        shrl      $2, %eax                                      #2236.49
..LN1615:
        addq      -40(%rbp), %rax                               #2236.30
..LN1617:
        movq      %rax, -96(%rbp)                               #2236.5
..LN1619:
   .loc    3  2239
        movl      -32(%rbp), %eax                               #2239.18
        negl      %eax                                          #2239.18
        addl      -16(%rbp), %eax                               #2239.18
..LN1621:
        movl      %eax, -76(%rbp)                               #2239.5
..LN1623:
   .loc    3  2242
        movl      -16(%rbp), %eax                               #2242.44
..LN1625:
        addl      $3, %eax                                      #2242.50
..LN1627:
        shrl      $2, %eax                                      #2242.57
..LN1629:
        addq      -40(%rbp), %rax                               #2242.30
..LN1631:
        movq      %rax, -88(%rbp)                               #2242.5
..LN1633:
   .loc    3  2245
        movq      -96(%rbp), %rax                               #2245.33
..LN1635:
        movq      -88(%rbp), %rdx                               #2245.38
..LN1637:
        movq      %rax, %rdi                                    #2245.14
        movq      %rdx, %rsi                                    #2245.14
        call      prime_buffer_2na@PLT                          #2245.14
                                # LOE xmm0
..B18.43:                       # Preds ..B18.3
        movdqa    %xmm0, -144(%rbp)                             #2245.14
                                # LOE
..B18.4:                        # Preds ..B18.43
..LN1639:
        movdqa    -144(%rbp), %xmm0                             #2245.5
        movdqa    %xmm0, -288(%rbp)                             #2245.5
..LN1641:
   .loc    3  2246
        addq      $16, -96(%rbp)                                #2246.5
..LN1643:
   .loc    3  2256
        movq      -48(%rbp), %rax                               #2256.5
        movdqa    16(%rax), %xmm0                               #2256.5
        movdqa    %xmm0, -272(%rbp)                             #2256.5
        movq      -48(%rbp), %rax                               #2256.5
        movdqa    32(%rax), %xmm0                               #2256.5
        movdqa    %xmm0, -256(%rbp)                             #2256.5
        movq      -48(%rbp), %rax                               #2256.5
        movdqa    48(%rax), %xmm0                               #2256.5
        movdqa    %xmm0, -240(%rbp)                             #2256.5
        movq      -48(%rbp), %rax                               #2256.5
        movdqa    64(%rax), %xmm0                               #2256.5
        movdqa    %xmm0, -224(%rbp)                             #2256.5
        movq      -48(%rbp), %rax                               #2256.5
        movdqa    80(%rax), %xmm0                               #2256.5
        movdqa    %xmm0, -208(%rbp)                             #2256.5
        movq      -48(%rbp), %rax                               #2256.5
        movdqa    96(%rax), %xmm0                               #2256.5
        movdqa    %xmm0, -192(%rbp)                             #2256.5
        movq      -48(%rbp), %rax                               #2256.5
        movdqa    112(%rax), %xmm0                              #2256.5
        movdqa    %xmm0, -176(%rbp)                             #2256.5
        movq      -48(%rbp), %rax                               #2256.5
        movdqa    128(%rax), %xmm0                              #2256.5
        movdqa    %xmm0, -160(%rbp)                             #2256.5
..LN1645:
   .loc    3  2259
        xorl      %eax, %eax                                    #2259.15
        movl      %eax, -72(%rbp)                               #2259.15
..LN1647:
        movl      %eax, -68(%rbp)                               #2259.10
..LN1649:
        movl      %eax, -64(%rbp)                               #2259.5
..LN1651:
   .loc    3  2263
        movl      $1, -60(%rbp)                                 #2263.5
..LN1653:
   .loc    3  2268
        movl      -24(%rbp), %eax                               #2268.14
..LN1655:
        andl      $3, %eax                                      #2268.20
        movl      %eax, -56(%rbp)                               #2268.20
        je        ..B18.12      # Prob 50%                      #2268.20
                                # LOE
..B18.5:                        # Preds ..B18.4
        movl      -56(%rbp), %eax                               #2268.20
        cmpl      $1, %eax                                      #2268.20
        je        ..B18.13      # Prob 50%                      #2268.20
                                # LOE
..B18.6:                        # Preds ..B18.5
        movl      -56(%rbp), %eax                               #2268.20
        cmpl      $2, %eax                                      #2268.20
        je        ..B18.14      # Prob 50%                      #2268.20
                                # LOE
..B18.7:                        # Preds ..B18.6
        movl      -56(%rbp), %eax                               #2268.20
        cmpl      $3, %eax                                      #2268.20
        je        ..B18.15      # Prob 50%                      #2268.20
                                # LOE
..B18.8:                        # Preds ..B18.39 ..B18.7
..LN1657:
   .loc    3  2273
        movl      $1, %eax                                      #2273.9
        testl     %eax, %eax                                    #2273.9
        je        ..B18.40      # Prob 100%                     #2273.9
                                # LOE
..B18.9:                        # Preds ..B18.8
..LN1659:
   .loc    3  2275
        movl      $1, -60(%rbp)                                 #2275.13
                                # LOE
..B18.10:                       # Preds ..B18.34 ..B18.9
..LN1661:
   .loc    3  2278
        movl      $1, %eax                                      #2278.13
        testl     %eax, %eax                                    #2278.13
        je        ..B18.36      # Prob 100%                     #2278.13
                                # LOE
..B18.12:                       # Preds ..B18.4 ..B18.10
..LN1663:
   .loc    3  2285
        movdqa    -288(%rbp), %xmm0                             #2285.38
..LN1665:
        movdqa    -256(%rbp), %xmm1                             #2285.46
..LN1667:
        pand      %xmm1, %xmm0                                  #2285.22
..LN1669:
        movdqa    %xmm0, -304(%rbp)                             #2285.17
..LN1671:
   .loc    3  2286
        movdqa    -304(%rbp), %xmm0                             #2286.22
        movdqa    -272(%rbp), %xmm1                             #2286.22
        pcmpeqb   %xmm1, %xmm0                                  #2286.22
..LN1673:
        movdqa    %xmm0, -304(%rbp)                             #2286.17
..LN1675:
   .loc    3  2287
        movdqa    -304(%rbp), %xmm0                             #2287.42
..LN1677:
        pmovmskb  %xmm0, %eax                                   #2287.22
..LN1679:
        movl      %eax, -64(%rbp)                               #2287.17
                                # LOE
..B18.13:                       # Preds ..B18.5 ..B18.12
..LN1681:
   .loc    3  2291
        movdqa    -288(%rbp), %xmm0                             #2291.38
..LN1683:
        movdqa    -224(%rbp), %xmm1                             #2291.46
..LN1685:
        pand      %xmm1, %xmm0                                  #2291.22
..LN1687:
        movdqa    %xmm0, -304(%rbp)                             #2291.17
..LN1689:
   .loc    3  2292
        movdqa    -304(%rbp), %xmm0                             #2292.22
        movdqa    -240(%rbp), %xmm1                             #2292.22
        pcmpeqb   %xmm1, %xmm0                                  #2292.22
..LN1691:
        movdqa    %xmm0, -304(%rbp)                             #2292.17
..LN1693:
   .loc    3  2293
        movdqa    -304(%rbp), %xmm0                             #2293.42
..LN1695:
        pmovmskb  %xmm0, %eax                                   #2293.22
..LN1697:
        movl      %eax, -68(%rbp)                               #2293.17
                                # LOE
..B18.14:                       # Preds ..B18.6 ..B18.13
..LN1699:
   .loc    3  2297
        movdqa    -288(%rbp), %xmm0                             #2297.38
..LN1701:
        movdqa    -192(%rbp), %xmm1                             #2297.46
..LN1703:
        pand      %xmm1, %xmm0                                  #2297.22
..LN1705:
        movdqa    %xmm0, -304(%rbp)                             #2297.17
..LN1707:
   .loc    3  2298
        movdqa    -304(%rbp), %xmm0                             #2298.22
        movdqa    -208(%rbp), %xmm1                             #2298.22
        pcmpeqb   %xmm1, %xmm0                                  #2298.22
..LN1709:
        movdqa    %xmm0, -304(%rbp)                             #2298.17
..LN1711:
   .loc    3  2299
        movdqa    -304(%rbp), %xmm0                             #2299.42
..LN1713:
        pmovmskb  %xmm0, %eax                                   #2299.22
..LN1715:
        movl      %eax, -72(%rbp)                               #2299.17
                                # LOE
..B18.15:                       # Preds ..B18.7 ..B18.14
..LN1717:
   .loc    3  2303
        movdqa    -288(%rbp), %xmm0                             #2303.38
..LN1719:
        movdqa    -160(%rbp), %xmm1                             #2303.46
..LN1721:
        pand      %xmm1, %xmm0                                  #2303.22
..LN1723:
        movdqa    %xmm0, -304(%rbp)                             #2303.17
..LN1725:
   .loc    3  2304
        movdqa    -304(%rbp), %xmm0                             #2304.22
        movdqa    -176(%rbp), %xmm1                             #2304.22
        pcmpeqb   %xmm1, %xmm0                                  #2304.22
..LN1727:
        movdqa    %xmm0, -304(%rbp)                             #2304.17
..LN1729:
   .loc    3  2305
        movdqa    -304(%rbp), %xmm0                             #2305.42
..LN1731:
        pmovmskb  %xmm0, %eax                                   #2305.22
..LN1733:
        movl      %eax, -80(%rbp)                               #2305.17
..LN1735:
   .loc    3  2310
        andl      $-4, -24(%rbp)                                #2310.17
..LN1737:
   .loc    3  2313
        movl      -68(%rbp), %eax                               #2313.29
        orl       -64(%rbp), %eax                               #2313.29
..LN1739:
        orl       -72(%rbp), %eax                               #2313.34
..LN1741:
        orl       -80(%rbp), %eax                               #2313.39
..LN1743:
        je        ..B18.32      # Prob 50%                      #2313.47
                                # LOE
..B18.16:                       # Preds ..B18.15
..LN1745:
   .loc    3  2331
        movl      -64(%rbp), %eax                               #2331.58
..LN1747:
        movl      %eax, %edi                                    #2331.30
        call      uint16_lsbit@PLT                              #2331.30
                                # LOE eax
..B18.44:                       # Preds ..B18.16
        movl      %eax, -112(%rbp)                              #2331.30
                                # LOE
..B18.17:                       # Preds ..B18.44
        movl      -112(%rbp), %eax                              #2331.30
        movswq    %ax, %rax                                     #2331.30
..LN1749:
        movl      %eax, -128(%rbp)                              #2331.28
..LN1751:
   .loc    3  2332
        movl      -68(%rbp), %eax                               #2332.58
..LN1753:
        movl      %eax, %edi                                    #2332.30
        call      uint16_lsbit@PLT                              #2332.30
                                # LOE eax
..B18.45:                       # Preds ..B18.17
        movl      %eax, -108(%rbp)                              #2332.30
                                # LOE
..B18.18:                       # Preds ..B18.45
        movl      -108(%rbp), %eax                              #2332.30
        movswq    %ax, %rax                                     #2332.30
..LN1755:
        movl      %eax, -124(%rbp)                              #2332.28
..LN1757:
   .loc    3  2333
        movl      -72(%rbp), %eax                               #2333.58
..LN1759:
        movl      %eax, %edi                                    #2333.30
        call      uint16_lsbit@PLT                              #2333.30
                                # LOE eax
..B18.46:                       # Preds ..B18.18
        movl      %eax, -104(%rbp)                              #2333.30
                                # LOE
..B18.19:                       # Preds ..B18.46
        movl      -104(%rbp), %eax                              #2333.30
        movswq    %ax, %rax                                     #2333.30
..LN1761:
        movl      %eax, -120(%rbp)                              #2333.28
..LN1763:
   .loc    3  2334
        movl      -80(%rbp), %eax                               #2334.58
..LN1765:
        movl      %eax, %edi                                    #2334.30
        call      uint16_lsbit@PLT                              #2334.30
                                # LOE eax
..B18.47:                       # Preds ..B18.19
        movl      %eax, -100(%rbp)                              #2334.30
                                # LOE
..B18.20:                       # Preds ..B18.47
        movl      -100(%rbp), %eax                              #2334.30
        movswq    %ax, %rax                                     #2334.30
..LN1767:
        movl      %eax, -116(%rbp)                              #2334.28
..LN1769:
   .loc    3  2339
        shll      $2, -128(%rbp)                                #2339.34
..LN1771:
   .loc    3  2340
        movl      -124(%rbp), %eax                              #2340.28
..LN1773:
        shll      $2, %eax                                      #2340.34
..LN1775:
        incl      %eax                                          #2340.40
..LN1777:
        movl      %eax, -124(%rbp)                              #2340.21
..LN1779:
   .loc    3  2341
        movl      -120(%rbp), %eax                              #2341.28
..LN1781:
        shll      $2, %eax                                      #2341.34
..LN1783:
        addl      $2, %eax                                      #2341.40
..LN1785:
        movl      %eax, -120(%rbp)                              #2341.21
..LN1787:
   .loc    3  2342
        movl      -116(%rbp), %eax                              #2342.28
..LN1789:
        shll      $2, %eax                                      #2342.34
..LN1791:
        addl      $3, %eax                                      #2342.40
..LN1793:
        movl      %eax, -116(%rbp)                              #2342.21
..LN1795:
   .loc    3  2346
        movl      -64(%rbp), %eax                               #2346.26
..LN1797:
        testl     %eax, %eax                                    #2346.32
        je        ..B18.23      # Prob 50%                      #2346.32
                                # LOE
..B18.21:                       # Preds ..B18.20
..LN1799:
        movl      -128(%rbp), %eax                              #2346.43
        addl      -24(%rbp), %eax                               #2346.43
..LN1801:
        movl      -76(%rbp), %edx                               #2346.49
        cmpl      %edx, %eax                                    #2346.49
        ja        ..B18.23      # Prob 50%                      #2346.49
                                # LOE
..B18.22:                       # Preds ..B18.21
..LN1803:
        movl      $1, %eax                                      #2346.63
        leave                                                   #2346.63
..___tag_value_eval_2na_8.232:                                  #
        ret                                                     #2346.63
..___tag_value_eval_2na_8.233:                                  #
                                # LOE
..B18.23:                       # Preds ..B18.21 ..B18.20
..LN1805:
   .loc    3  2347
        movl      -68(%rbp), %eax                               #2347.26
..LN1807:
        testl     %eax, %eax                                    #2347.32
        je        ..B18.26      # Prob 50%                      #2347.32
                                # LOE
..B18.24:                       # Preds ..B18.23
..LN1809:
        movl      -124(%rbp), %eax                              #2347.43
        addl      -24(%rbp), %eax                               #2347.43
..LN1811:
        movl      -76(%rbp), %edx                               #2347.49
        cmpl      %edx, %eax                                    #2347.49
        ja        ..B18.26      # Prob 50%                      #2347.49
                                # LOE
..B18.25:                       # Preds ..B18.24
..LN1813:
        movl      $1, %eax                                      #2347.63
        leave                                                   #2347.63
..___tag_value_eval_2na_8.235:                                  #
        ret                                                     #2347.63
..___tag_value_eval_2na_8.236:                                  #
                                # LOE
..B18.26:                       # Preds ..B18.24 ..B18.23
..LN1815:
   .loc    3  2348
        movl      -72(%rbp), %eax                               #2348.26
..LN1817:
        testl     %eax, %eax                                    #2348.32
        je        ..B18.29      # Prob 50%                      #2348.32
                                # LOE
..B18.27:                       # Preds ..B18.26
..LN1819:
        movl      -120(%rbp), %eax                              #2348.43
        addl      -24(%rbp), %eax                               #2348.43
..LN1821:
        movl      -76(%rbp), %edx                               #2348.49
        cmpl      %edx, %eax                                    #2348.49
        ja        ..B18.29      # Prob 50%                      #2348.49
                                # LOE
..B18.28:                       # Preds ..B18.27
..LN1823:
        movl      $1, %eax                                      #2348.63
        leave                                                   #2348.63
..___tag_value_eval_2na_8.238:                                  #
        ret                                                     #2348.63
..___tag_value_eval_2na_8.239:                                  #
                                # LOE
..B18.29:                       # Preds ..B18.27 ..B18.26
..LN1825:
   .loc    3  2349
        movl      -80(%rbp), %eax                               #2349.26
..LN1827:
        testl     %eax, %eax                                    #2349.32
        je        ..B18.32      # Prob 50%                      #2349.32
                                # LOE
..B18.30:                       # Preds ..B18.29
..LN1829:
        movl      -116(%rbp), %eax                              #2349.43
        addl      -24(%rbp), %eax                               #2349.43
..LN1831:
        movl      -76(%rbp), %edx                               #2349.49
        cmpl      %edx, %eax                                    #2349.49
        ja        ..B18.32      # Prob 50%                      #2349.49
                                # LOE
..B18.31:                       # Preds ..B18.30
..LN1833:
        movl      $1, %eax                                      #2349.63
        leave                                                   #2349.63
..___tag_value_eval_2na_8.241:                                  #
        ret                                                     #2349.63
..___tag_value_eval_2na_8.242:                                  #
                                # LOE
..B18.32:                       # Preds ..B18.30 ..B18.29 ..B18.15
..LN1835:
   .loc    3  2354
        addl      $4, -24(%rbp)                                 #2354.17
..LN1837:
   .loc    3  2357
        movl      -24(%rbp), %eax                               #2357.22
..LN1839:
        movl      -76(%rbp), %edx                               #2357.28
        cmpl      %edx, %eax                                    #2357.28
        jbe       ..B18.34      # Prob 50%                      #2357.28
                                # LOE
..B18.33:                       # Preds ..B18.32
..LN1841:
   .loc    3  2358
        xorl      %eax, %eax                                    #2358.28
        leave                                                   #2358.28
..___tag_value_eval_2na_8.244:                                  #
        ret                                                     #2358.28
..___tag_value_eval_2na_8.245:                                  #
                                # LOE
..B18.34:                       # Preds ..B18.32
..LN1843:
   .loc    3  2361
        movl      -60(%rbp), %eax                               #2361.25
        decl      %eax                                          #2361.25
        movl      %eax, -60(%rbp)                               #2361.25
..LN1845:
        jne       ..B18.10      # Prob 50%                      #2361.39
                                # LOE
..B18.36:                       # Preds ..B18.34 ..B18.10
..LN1847:
   .loc    3  2382
        movq      -96(%rbp), %rax                               #2382.18
..LN1849:
        movq      -88(%rbp), %rdx                               #2382.25
        cmpq      %rdx, %rax                                    #2382.25
        jae       ..B18.40      # Prob 50%                      #2382.25
                                # LOE
..B18.37:                       # Preds ..B18.36
..LN1851:
   .loc    3  2386
        addl      $60, -24(%rbp)                                #2386.13
..LN1853:
   .loc    3  2387
        movl      -24(%rbp), %eax                               #2387.18
..LN1855:
        movl      -76(%rbp), %edx                               #2387.24
        cmpl      %edx, %eax                                    #2387.24
        ja        ..B18.40      # Prob 50%                      #2387.24
                                # LOE
..B18.38:                       # Preds ..B18.37
..LN1857:
   .loc    3  2392
        movq      -96(%rbp), %rax                               #2392.41
..LN1859:
        movq      -88(%rbp), %rdx                               #2392.46
..LN1861:
        movq      %rax, %rdi                                    #2392.22
        movq      %rdx, %rsi                                    #2392.22
        call      prime_buffer_2na@PLT                          #2392.22
                                # LOE xmm0
..B18.48:                       # Preds ..B18.38
        movdqa    %xmm0, -320(%rbp)                             #2392.22
                                # LOE
..B18.39:                       # Preds ..B18.48
..LN1863:
        movdqa    -320(%rbp), %xmm0                             #2392.13
        movdqa    %xmm0, -288(%rbp)                             #2392.13
..LN1865:
   .loc    3  2470
        addq      $16, -96(%rbp)                                #2470.13
        jmp       ..B18.8       # Prob 100%                     #2470.13
                                # LOE
..B18.40:                       # Preds ..B18.37 ..B18.36 ..B18.8
..LN1867:
   .loc    3  2483
        xorl      %eax, %eax                                    #2483.12
        leave                                                   #2483.12
..___tag_value_eval_2na_8.247:                                  #
        ret                                                     #2483.12
        .align    2,0x90
..___tag_value_eval_2na_8.248:                                  #
                                # LOE
# mark_end;
	.type	eval_2na_8,@function
	.size	eval_2na_8,.-eval_2na_8
.LNeval_2na_8:
	.data
# -- End  eval_2na_8
	.text
# -- Begin  eval_2na_16
# mark_begin;
       .align    2,0x90
eval_2na_16:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B19.1:                        # Preds ..B19.0
..___tag_value_eval_2na_16.251:                                 #
..LN1869:
   .loc    3  2489
        pushq     %rbp                                          #2489.1
        movq      %rsp, %rbp                                    #2489.1
..___tag_value_eval_2na_16.252:                                 #
        subq      $336, %rsp                                    #2489.1
        movq      %rdi, -48(%rbp)                               #2489.1
        movq      %rsi, -40(%rbp)                               #2489.1
        movl      %edx, -24(%rbp)                               #2489.1
        movl      %ecx, -16(%rbp)                               #2489.1
..LN1871:
   .loc    3  2519
        movq      -48(%rbp), %rax                               #2519.25
        movl      4(%rax), %eax                                 #2519.25
..LN1873:
        movl      %eax, -32(%rbp)                               #2519.23
..LN1875:
   .loc    3  2522
        movl      -16(%rbp), %eax                               #2522.5
        movl      -32(%rbp), %edx                               #2522.5
        cmpl      %edx, %eax                                    #2522.5
        jae       ..B19.3       # Prob 50%                      #2522.5
                                # LOE
..B19.2:                        # Preds ..B19.1
        lea       _2__STRING.9.0(%rip), %rax                    #2522.5
        lea       _2__STRING.1.0(%rip), %rdx                    #2522.5
        lea       __$U6.0(%rip), %rcx                           #2522.5
        movq      %rax, %rdi                                    #2522.5
        movq      %rdx, %rsi                                    #2522.5
        movl      $2522, %edx                                   #2522.5
        call      __assert_fail@PLT                             #2522.5
                                # LOE
..B19.3:                        # Preds ..B19.1
..LN1877:
   .loc    3  2526
        movl      -24(%rbp), %eax                               #2526.12
..LN1879:
        addl      -16(%rbp), %eax                               #2526.5
        movl      %eax, -16(%rbp)                               #2526.5
..LN1881:
   .loc    3  2529
        movl      -24(%rbp), %eax                               #2529.42
..LN1883:
        shrl      $2, %eax                                      #2529.49
..LN1885:
        addq      -40(%rbp), %rax                               #2529.30
..LN1887:
        movq      %rax, -104(%rbp)                              #2529.5
..LN1889:
   .loc    3  2532
        movl      -32(%rbp), %eax                               #2532.18
        negl      %eax                                          #2532.18
        addl      -16(%rbp), %eax                               #2532.18
..LN1891:
        movl      %eax, -76(%rbp)                               #2532.5
..LN1893:
   .loc    3  2535
        movl      -16(%rbp), %eax                               #2535.44
..LN1895:
        addl      $3, %eax                                      #2535.50
..LN1897:
        shrl      $2, %eax                                      #2535.57
..LN1899:
        addq      -40(%rbp), %rax                               #2535.30
..LN1901:
        movq      %rax, -96(%rbp)                               #2535.5
..LN1903:
   .loc    3  2538
        movq      -104(%rbp), %rax                              #2538.33
..LN1905:
        movq      -96(%rbp), %rdx                               #2538.38
..LN1907:
        movq      %rax, %rdi                                    #2538.14
        movq      %rdx, %rsi                                    #2538.14
        call      prime_buffer_2na@PLT                          #2538.14
                                # LOE xmm0
..B19.49:                       # Preds ..B19.3
        movdqa    %xmm0, -160(%rbp)                             #2538.14
                                # LOE
..B19.4:                        # Preds ..B19.49
..LN1909:
        movdqa    -160(%rbp), %xmm0                             #2538.5
        movdqa    %xmm0, -304(%rbp)                             #2538.5
..LN1911:
   .loc    3  2539
        addq      $16, -104(%rbp)                               #2539.5
..LN1913:
   .loc    3  2541
        movq      -104(%rbp), %rax                              #2541.9
..LN1915:
        movq      %rax, -88(%rbp)                               #2541.5
..LN1917:
   .loc    3  2544
        movq      -104(%rbp), %rax                              #2544.10
..LN1919:
        movq      -96(%rbp), %rdx                               #2544.16
        cmpq      %rdx, %rax                                    #2544.16
        jae       ..B19.6       # Prob 50%                      #2544.16
                                # LOE
..B19.5:                        # Preds ..B19.4
..LN1921:
   .loc    3  2545
        movq      -104(%rbp), %rax                              #2545.24
        movzbl    -1(%rax), %eax                                #2545.24
        movzbl    %al, %eax                                     #2545.24
..LN1923:
        shll      $8, %eax                                      #2545.38
..LN1925:
        movl      %eax, -112(%rbp)                              #2545.9
                                # LOE
..B19.6:                        # Preds ..B19.5 ..B19.4
..LN1927:
   .loc    3  2549
        movq      -48(%rbp), %rax                               #2549.5
        movdqa    16(%rax), %xmm0                               #2549.5
        movdqa    %xmm0, -288(%rbp)                             #2549.5
        movq      -48(%rbp), %rax                               #2549.5
        movdqa    32(%rax), %xmm0                               #2549.5
        movdqa    %xmm0, -272(%rbp)                             #2549.5
        movq      -48(%rbp), %rax                               #2549.5
        movdqa    48(%rax), %xmm0                               #2549.5
        movdqa    %xmm0, -256(%rbp)                             #2549.5
        movq      -48(%rbp), %rax                               #2549.5
        movdqa    64(%rax), %xmm0                               #2549.5
        movdqa    %xmm0, -240(%rbp)                             #2549.5
        movq      -48(%rbp), %rax                               #2549.5
        movdqa    80(%rax), %xmm0                               #2549.5
        movdqa    %xmm0, -224(%rbp)                             #2549.5
        movq      -48(%rbp), %rax                               #2549.5
        movdqa    96(%rax), %xmm0                               #2549.5
        movdqa    %xmm0, -208(%rbp)                             #2549.5
        movq      -48(%rbp), %rax                               #2549.5
        movdqa    112(%rax), %xmm0                              #2549.5
        movdqa    %xmm0, -192(%rbp)                             #2549.5
        movq      -48(%rbp), %rax                               #2549.5
        movdqa    128(%rax), %xmm0                              #2549.5
        movdqa    %xmm0, -176(%rbp)                             #2549.5
..LN1929:
   .loc    3  2552
        xorl      %eax, %eax                                    #2552.15
        movl      %eax, -72(%rbp)                               #2552.15
..LN1931:
        movl      %eax, -68(%rbp)                               #2552.10
..LN1933:
        movl      %eax, -64(%rbp)                               #2552.5
..LN1935:
   .loc    3  2556
        movl      $2, -60(%rbp)                                 #2556.5
..LN1937:
   .loc    3  2561
        movl      -24(%rbp), %eax                               #2561.14
..LN1939:
        andl      $3, %eax                                      #2561.20
        movl      %eax, -56(%rbp)                               #2561.20
        je        ..B19.14      # Prob 50%                      #2561.20
                                # LOE
..B19.7:                        # Preds ..B19.6
        movl      -56(%rbp), %eax                               #2561.20
        cmpl      $1, %eax                                      #2561.20
        je        ..B19.15      # Prob 50%                      #2561.20
                                # LOE
..B19.8:                        # Preds ..B19.7
        movl      -56(%rbp), %eax                               #2561.20
        cmpl      $2, %eax                                      #2561.20
        je        ..B19.16      # Prob 50%                      #2561.20
                                # LOE
..B19.9:                        # Preds ..B19.8
        movl      -56(%rbp), %eax                               #2561.20
        cmpl      $3, %eax                                      #2561.20
        je        ..B19.17      # Prob 50%                      #2561.20
                                # LOE
..B19.10:                       # Preds ..B19.45 ..B19.44 ..B19.9
..LN1941:
   .loc    3  2566
        movl      $1, %eax                                      #2566.9
        testl     %eax, %eax                                    #2566.9
        je        ..B19.46      # Prob 100%                     #2566.9
                                # LOE
..B19.11:                       # Preds ..B19.10
..LN1943:
   .loc    3  2568
        movl      $2, -60(%rbp)                                 #2568.13
                                # LOE
..B19.12:                       # Preds ..B19.39 ..B19.11
..LN1945:
   .loc    3  2571
        movl      $1, %eax                                      #2571.13
        testl     %eax, %eax                                    #2571.13
        je        ..B19.41      # Prob 100%                     #2571.13
                                # LOE
..B19.14:                       # Preds ..B19.6 ..B19.12
..LN1947:
   .loc    3  2578
        movdqa    -304(%rbp), %xmm0                             #2578.38
..LN1949:
        movdqa    -272(%rbp), %xmm1                             #2578.46
..LN1951:
        pand      %xmm1, %xmm0                                  #2578.22
..LN1953:
        movdqa    %xmm0, -320(%rbp)                             #2578.17
..LN1955:
   .loc    3  2579
        movdqa    -320(%rbp), %xmm0                             #2579.22
        movdqa    -288(%rbp), %xmm1                             #2579.22
        pcmpeqw   %xmm1, %xmm0                                  #2579.22
..LN1957:
        movdqa    %xmm0, -320(%rbp)                             #2579.17
..LN1959:
   .loc    3  2580
        movdqa    -320(%rbp), %xmm0                             #2580.42
..LN1961:
        pmovmskb  %xmm0, %eax                                   #2580.22
..LN1963:
        movl      %eax, -64(%rbp)                               #2580.17
                                # LOE
..B19.15:                       # Preds ..B19.7 ..B19.14
..LN1965:
   .loc    3  2584
        movdqa    -304(%rbp), %xmm0                             #2584.38
..LN1967:
        movdqa    -240(%rbp), %xmm1                             #2584.46
..LN1969:
        pand      %xmm1, %xmm0                                  #2584.22
..LN1971:
        movdqa    %xmm0, -320(%rbp)                             #2584.17
..LN1973:
   .loc    3  2585
        movdqa    -320(%rbp), %xmm0                             #2585.22
        movdqa    -256(%rbp), %xmm1                             #2585.22
        pcmpeqw   %xmm1, %xmm0                                  #2585.22
..LN1975:
        movdqa    %xmm0, -320(%rbp)                             #2585.17
..LN1977:
   .loc    3  2586
        movdqa    -320(%rbp), %xmm0                             #2586.42
..LN1979:
        pmovmskb  %xmm0, %eax                                   #2586.22
..LN1981:
        movl      %eax, -68(%rbp)                               #2586.17
                                # LOE
..B19.16:                       # Preds ..B19.8 ..B19.15
..LN1983:
   .loc    3  2590
        movdqa    -304(%rbp), %xmm0                             #2590.38
..LN1985:
        movdqa    -208(%rbp), %xmm1                             #2590.46
..LN1987:
        pand      %xmm1, %xmm0                                  #2590.22
..LN1989:
        movdqa    %xmm0, -320(%rbp)                             #2590.17
..LN1991:
   .loc    3  2591
        movdqa    -320(%rbp), %xmm0                             #2591.22
        movdqa    -224(%rbp), %xmm1                             #2591.22
        pcmpeqw   %xmm1, %xmm0                                  #2591.22
..LN1993:
        movdqa    %xmm0, -320(%rbp)                             #2591.17
..LN1995:
   .loc    3  2592
        movdqa    -320(%rbp), %xmm0                             #2592.42
..LN1997:
        pmovmskb  %xmm0, %eax                                   #2592.22
..LN1999:
        movl      %eax, -72(%rbp)                               #2592.17
                                # LOE
..B19.17:                       # Preds ..B19.9 ..B19.16
..LN2001:
   .loc    3  2596
        movdqa    -304(%rbp), %xmm0                             #2596.38
..LN2003:
        movdqa    -176(%rbp), %xmm1                             #2596.46
..LN2005:
        pand      %xmm1, %xmm0                                  #2596.22
..LN2007:
        movdqa    %xmm0, -320(%rbp)                             #2596.17
..LN2009:
   .loc    3  2597
        movdqa    -320(%rbp), %xmm0                             #2597.22
        movdqa    -192(%rbp), %xmm1                             #2597.22
        pcmpeqw   %xmm1, %xmm0                                  #2597.22
..LN2011:
        movdqa    %xmm0, -320(%rbp)                             #2597.17
..LN2013:
   .loc    3  2598
        movdqa    -320(%rbp), %xmm0                             #2598.42
..LN2015:
        pmovmskb  %xmm0, %eax                                   #2598.22
..LN2017:
        movl      %eax, -80(%rbp)                               #2598.17
..LN2019:
   .loc    3  2603
        andl      $-4, -24(%rbp)                                #2603.17
..LN2021:
   .loc    3  2606
        movl      -68(%rbp), %eax                               #2606.29
        orl       -64(%rbp), %eax                               #2606.29
..LN2023:
        orl       -72(%rbp), %eax                               #2606.34
..LN2025:
        orl       -80(%rbp), %eax                               #2606.39
..LN2027:
        je        ..B19.34      # Prob 50%                      #2606.47
                                # LOE
..B19.18:                       # Preds ..B19.17
..LN2029:
   .loc    3  2624
        movl      -64(%rbp), %eax                               #2624.58
..LN2031:
        movl      %eax, %edi                                    #2624.30
        call      uint16_lsbit@PLT                              #2624.30
                                # LOE eax
..B19.50:                       # Preds ..B19.18
        movl      %eax, -128(%rbp)                              #2624.30
                                # LOE
..B19.19:                       # Preds ..B19.50
        movl      -128(%rbp), %eax                              #2624.30
        movswq    %ax, %rax                                     #2624.30
..LN2033:
        movl      %eax, -144(%rbp)                              #2624.28
..LN2035:
   .loc    3  2625
        movl      -68(%rbp), %eax                               #2625.58
..LN2037:
        movl      %eax, %edi                                    #2625.30
        call      uint16_lsbit@PLT                              #2625.30
                                # LOE eax
..B19.51:                       # Preds ..B19.19
        movl      %eax, -124(%rbp)                              #2625.30
                                # LOE
..B19.20:                       # Preds ..B19.51
        movl      -124(%rbp), %eax                              #2625.30
        movswq    %ax, %rax                                     #2625.30
..LN2039:
        movl      %eax, -140(%rbp)                              #2625.28
..LN2041:
   .loc    3  2626
        movl      -72(%rbp), %eax                               #2626.58
..LN2043:
        movl      %eax, %edi                                    #2626.30
        call      uint16_lsbit@PLT                              #2626.30
                                # LOE eax
..B19.52:                       # Preds ..B19.20
        movl      %eax, -120(%rbp)                              #2626.30
                                # LOE
..B19.21:                       # Preds ..B19.52
        movl      -120(%rbp), %eax                              #2626.30
        movswq    %ax, %rax                                     #2626.30
..LN2045:
        movl      %eax, -136(%rbp)                              #2626.28
..LN2047:
   .loc    3  2627
        movl      -80(%rbp), %eax                               #2627.58
..LN2049:
        movl      %eax, %edi                                    #2627.30
        call      uint16_lsbit@PLT                              #2627.30
                                # LOE eax
..B19.53:                       # Preds ..B19.21
        movl      %eax, -116(%rbp)                              #2627.30
                                # LOE
..B19.22:                       # Preds ..B19.53
        movl      -116(%rbp), %eax                              #2627.30
        movswq    %ax, %rax                                     #2627.30
..LN2051:
        movl      %eax, -132(%rbp)                              #2627.28
..LN2053:
   .loc    3  2632
        shll      $2, -144(%rbp)                                #2632.34
..LN2055:
   .loc    3  2633
        movl      -140(%rbp), %eax                              #2633.28
..LN2057:
        shll      $2, %eax                                      #2633.34
..LN2059:
        incl      %eax                                          #2633.40
..LN2061:
        movl      %eax, -140(%rbp)                              #2633.21
..LN2063:
   .loc    3  2634
        movl      -136(%rbp), %eax                              #2634.28
..LN2065:
        shll      $2, %eax                                      #2634.34
..LN2067:
        addl      $2, %eax                                      #2634.40
..LN2069:
        movl      %eax, -136(%rbp)                              #2634.21
..LN2071:
   .loc    3  2635
        movl      -132(%rbp), %eax                              #2635.28
..LN2073:
        shll      $2, %eax                                      #2635.34
..LN2075:
        addl      $3, %eax                                      #2635.40
..LN2077:
        movl      %eax, -132(%rbp)                              #2635.21
..LN2079:
   .loc    3  2639
        movl      -64(%rbp), %eax                               #2639.26
..LN2081:
        testl     %eax, %eax                                    #2639.32
        je        ..B19.25      # Prob 50%                      #2639.32
                                # LOE
..B19.23:                       # Preds ..B19.22
..LN2083:
        movl      -144(%rbp), %eax                              #2639.43
        addl      -24(%rbp), %eax                               #2639.43
..LN2085:
        movl      -76(%rbp), %edx                               #2639.49
        cmpl      %edx, %eax                                    #2639.49
        ja        ..B19.25      # Prob 50%                      #2639.49
                                # LOE
..B19.24:                       # Preds ..B19.23
..LN2087:
        movl      $1, %eax                                      #2639.63
        leave                                                   #2639.63
..___tag_value_eval_2na_16.256:                                 #
        ret                                                     #2639.63
..___tag_value_eval_2na_16.257:                                 #
                                # LOE
..B19.25:                       # Preds ..B19.23 ..B19.22
..LN2089:
   .loc    3  2640
        movl      -68(%rbp), %eax                               #2640.26
..LN2091:
        testl     %eax, %eax                                    #2640.32
        je        ..B19.28      # Prob 50%                      #2640.32
                                # LOE
..B19.26:                       # Preds ..B19.25
..LN2093:
        movl      -140(%rbp), %eax                              #2640.43
        addl      -24(%rbp), %eax                               #2640.43
..LN2095:
        movl      -76(%rbp), %edx                               #2640.49
        cmpl      %edx, %eax                                    #2640.49
        ja        ..B19.28      # Prob 50%                      #2640.49
                                # LOE
..B19.27:                       # Preds ..B19.26
..LN2097:
        movl      $1, %eax                                      #2640.63
        leave                                                   #2640.63
..___tag_value_eval_2na_16.259:                                 #
        ret                                                     #2640.63
..___tag_value_eval_2na_16.260:                                 #
                                # LOE
..B19.28:                       # Preds ..B19.26 ..B19.25
..LN2099:
   .loc    3  2641
        movl      -72(%rbp), %eax                               #2641.26
..LN2101:
        testl     %eax, %eax                                    #2641.32
        je        ..B19.31      # Prob 50%                      #2641.32
                                # LOE
..B19.29:                       # Preds ..B19.28
..LN2103:
        movl      -136(%rbp), %eax                              #2641.43
        addl      -24(%rbp), %eax                               #2641.43
..LN2105:
        movl      -76(%rbp), %edx                               #2641.49
        cmpl      %edx, %eax                                    #2641.49
        ja        ..B19.31      # Prob 50%                      #2641.49
                                # LOE
..B19.30:                       # Preds ..B19.29
..LN2107:
        movl      $1, %eax                                      #2641.63
        leave                                                   #2641.63
..___tag_value_eval_2na_16.262:                                 #
        ret                                                     #2641.63
..___tag_value_eval_2na_16.263:                                 #
                                # LOE
..B19.31:                       # Preds ..B19.29 ..B19.28
..LN2109:
   .loc    3  2642
        movl      -80(%rbp), %eax                               #2642.26
..LN2111:
        testl     %eax, %eax                                    #2642.32
        je        ..B19.34      # Prob 50%                      #2642.32
                                # LOE
..B19.32:                       # Preds ..B19.31
..LN2113:
        movl      -132(%rbp), %eax                              #2642.43
        addl      -24(%rbp), %eax                               #2642.43
..LN2115:
        movl      -76(%rbp), %edx                               #2642.49
        cmpl      %edx, %eax                                    #2642.49
        ja        ..B19.34      # Prob 50%                      #2642.49
                                # LOE
..B19.33:                       # Preds ..B19.32
..LN2117:
        movl      $1, %eax                                      #2642.63
        leave                                                   #2642.63
..___tag_value_eval_2na_16.265:                                 #
        ret                                                     #2642.63
..___tag_value_eval_2na_16.266:                                 #
                                # LOE
..B19.34:                       # Preds ..B19.32 ..B19.31 ..B19.17
..LN2119:
   .loc    3  2647
        addl      $4, -24(%rbp)                                 #2647.17
..LN2121:
   .loc    3  2650
        movl      -24(%rbp), %eax                               #2650.22
..LN2123:
        movl      -76(%rbp), %edx                               #2650.28
        cmpl      %edx, %eax                                    #2650.28
        jbe       ..B19.36      # Prob 50%                      #2650.28
                                # LOE
..B19.35:                       # Preds ..B19.34
..LN2125:
   .loc    3  2651
        xorl      %eax, %eax                                    #2651.28
        leave                                                   #2651.28
..___tag_value_eval_2na_16.268:                                 #
        ret                                                     #2651.28
..___tag_value_eval_2na_16.269:                                 #
                                # LOE
..B19.36:                       # Preds ..B19.34
..LN2127:
   .loc    3  2654
        movl      -60(%rbp), %eax                               #2654.25
        decl      %eax                                          #2654.25
        movl      %eax, -60(%rbp)                               #2654.25
..LN2129:
        je        ..B19.41      # Prob 50%                      #2654.39
                                # LOE
..B19.37:                       # Preds ..B19.36
..LN2131:
   .loc    3  2659
        movdqa    -304(%rbp), %xmm0                             #2659.43
..LN2133:
        psrldq    $1, %xmm0                                     #2659.26
..LN2135:
        movdqa    %xmm0, -304(%rbp)                             #2659.17
..LN2137:
   .loc    3  2662
        movq      -88(%rbp), %rax                               #2662.22
..LN2139:
        movq      -96(%rbp), %rdx                               #2662.26
        cmpq      %rdx, %rax                                    #2662.26
        jae       ..B19.39      # Prob 50%                      #2662.26
                                # LOE
..B19.38:                       # Preds ..B19.37
..LN2141:
   .loc    3  2664
        sarl      $8, -112(%rbp)                                #2664.21
..LN2143:
   .loc    3  2665
        movq      -88(%rbp), %rax                               #2665.37
        movzbl    (%rax), %eax                                  #2665.37
        movzbl    %al, %eax                                     #2665.37
..LN2145:
        shll      $8, %eax                                      #2665.48
..LN2147:
        orl       -112(%rbp), %eax                              #2665.21
        movl      %eax, -112(%rbp)                              #2665.21
..LN2149:
   .loc    3  2666
        movdqa    -304(%rbp), %xmm0                             #2666.49
..LN2151:
        movl      -112(%rbp), %eax                              #2666.57
..LN2153:
        pinsrw    $7, %eax, %xmm0                               #2666.30
..LN2155:
        movdqa    %xmm0, -304(%rbp)                             #2666.21
                                # LOE
..B19.39:                       # Preds ..B19.38 ..B19.37
..LN2157:
   .loc    3  2670
        incq      -88(%rbp)                                     #2670.20
        jmp       ..B19.12      # Prob 100%                     #2670.20
                                # LOE
..B19.41:                       # Preds ..B19.36 ..B19.12
..LN2159:
   .loc    3  2675
        movq      -104(%rbp), %rax                              #2675.18
..LN2161:
        movq      -96(%rbp), %rdx                               #2675.25
        cmpq      %rdx, %rax                                    #2675.25
        jae       ..B19.46      # Prob 50%                      #2675.25
                                # LOE
..B19.42:                       # Preds ..B19.41
..LN2163:
   .loc    3  2679
        addl      $56, -24(%rbp)                                #2679.13
..LN2165:
   .loc    3  2680
        movl      -24(%rbp), %eax                               #2680.18
..LN2167:
        movl      -76(%rbp), %edx                               #2680.24
        cmpl      %edx, %eax                                    #2680.24
        ja        ..B19.46      # Prob 50%                      #2680.24
                                # LOE
..B19.43:                       # Preds ..B19.42
..LN2169:
   .loc    3  2685
        movq      -104(%rbp), %rax                              #2685.41
..LN2171:
        movq      -96(%rbp), %rdx                               #2685.46
..LN2173:
        movq      %rax, %rdi                                    #2685.22
        movq      %rdx, %rsi                                    #2685.22
        call      prime_buffer_2na@PLT                          #2685.22
                                # LOE xmm0
..B19.54:                       # Preds ..B19.43
        movdqa    %xmm0, -336(%rbp)                             #2685.22
                                # LOE
..B19.44:                       # Preds ..B19.54
..LN2175:
        movdqa    -336(%rbp), %xmm0                             #2685.13
        movdqa    %xmm0, -304(%rbp)                             #2685.13
..LN2177:
   .loc    3  2763
        addq      $16, -104(%rbp)                               #2763.13
..LN2179:
   .loc    3  2765
        movq      -104(%rbp), %rax                              #2765.17
..LN2181:
        movq      %rax, -88(%rbp)                               #2765.13
..LN2183:
   .loc    3  2766
        movq      -104(%rbp), %rax                              #2766.18
..LN2185:
        movq      -96(%rbp), %rdx                               #2766.24
        cmpq      %rdx, %rax                                    #2766.24
        jae       ..B19.10      # Prob 50%                      #2766.24
                                # LOE
..B19.45:                       # Preds ..B19.44
..LN2187:
   .loc    3  2767
        movq      -104(%rbp), %rax                              #2767.32
        movzbl    -1(%rax), %eax                                #2767.32
        movzbl    %al, %eax                                     #2767.32
..LN2189:
        shll      $8, %eax                                      #2767.46
..LN2191:
        movl      %eax, -112(%rbp)                              #2767.17
        jmp       ..B19.10      # Prob 100%                     #2767.17
                                # LOE
..B19.46:                       # Preds ..B19.42 ..B19.41 ..B19.10
..LN2193:
   .loc    3  2776
        xorl      %eax, %eax                                    #2776.12
        leave                                                   #2776.12
..___tag_value_eval_2na_16.271:                                 #
        ret                                                     #2776.12
        .align    2,0x90
..___tag_value_eval_2na_16.272:                                 #
                                # LOE
# mark_end;
	.type	eval_2na_16,@function
	.size	eval_2na_16,.-eval_2na_16
.LNeval_2na_16:
	.data
# -- End  eval_2na_16
	.text
# -- Begin  eval_2na_32
# mark_begin;
       .align    2,0x90
eval_2na_32:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B20.1:                        # Preds ..B20.0
..___tag_value_eval_2na_32.275:                                 #
..LN2195:
   .loc    3  2782
        pushq     %rbp                                          #2782.1
        movq      %rsp, %rbp                                    #2782.1
..___tag_value_eval_2na_32.276:                                 #
        subq      $336, %rsp                                    #2782.1
        movq      %rdi, -48(%rbp)                               #2782.1
        movq      %rsi, -40(%rbp)                               #2782.1
        movl      %edx, -24(%rbp)                               #2782.1
        movl      %ecx, -16(%rbp)                               #2782.1
..LN2197:
   .loc    3  2812
        movq      -48(%rbp), %rax                               #2812.25
        movl      4(%rax), %eax                                 #2812.25
..LN2199:
        movl      %eax, -32(%rbp)                               #2812.23
..LN2201:
   .loc    3  2815
        movl      -16(%rbp), %eax                               #2815.5
        movl      -32(%rbp), %edx                               #2815.5
        cmpl      %edx, %eax                                    #2815.5
        jae       ..B20.3       # Prob 50%                      #2815.5
                                # LOE
..B20.2:                        # Preds ..B20.1
        lea       _2__STRING.9.0(%rip), %rax                    #2815.5
        lea       _2__STRING.1.0(%rip), %rdx                    #2815.5
        lea       __$U7.0(%rip), %rcx                           #2815.5
        movq      %rax, %rdi                                    #2815.5
        movq      %rdx, %rsi                                    #2815.5
        movl      $2815, %edx                                   #2815.5
        call      __assert_fail@PLT                             #2815.5
                                # LOE
..B20.3:                        # Preds ..B20.1
..LN2203:
   .loc    3  2819
        movl      -24(%rbp), %eax                               #2819.12
..LN2205:
        addl      -16(%rbp), %eax                               #2819.5
        movl      %eax, -16(%rbp)                               #2819.5
..LN2207:
   .loc    3  2822
        movl      -24(%rbp), %eax                               #2822.42
..LN2209:
        shrl      $2, %eax                                      #2822.49
..LN2211:
        addq      -40(%rbp), %rax                               #2822.30
..LN2213:
        movq      %rax, -104(%rbp)                              #2822.5
..LN2215:
   .loc    3  2825
        movl      -32(%rbp), %eax                               #2825.18
        negl      %eax                                          #2825.18
        addl      -16(%rbp), %eax                               #2825.18
..LN2217:
        movl      %eax, -76(%rbp)                               #2825.5
..LN2219:
   .loc    3  2828
        movl      -16(%rbp), %eax                               #2828.44
..LN2221:
        addl      $3, %eax                                      #2828.50
..LN2223:
        shrl      $2, %eax                                      #2828.57
..LN2225:
        addq      -40(%rbp), %rax                               #2828.30
..LN2227:
        movq      %rax, -96(%rbp)                               #2828.5
..LN2229:
   .loc    3  2831
        movq      -104(%rbp), %rax                              #2831.33
..LN2231:
        movq      -96(%rbp), %rdx                               #2831.38
..LN2233:
        movq      %rax, %rdi                                    #2831.14
        movq      %rdx, %rsi                                    #2831.14
        call      prime_buffer_2na@PLT                          #2831.14
                                # LOE xmm0
..B20.49:                       # Preds ..B20.3
        movdqa    %xmm0, -160(%rbp)                             #2831.14
                                # LOE
..B20.4:                        # Preds ..B20.49
..LN2235:
        movdqa    -160(%rbp), %xmm0                             #2831.5
        movdqa    %xmm0, -304(%rbp)                             #2831.5
..LN2237:
   .loc    3  2832
        addq      $16, -104(%rbp)                               #2832.5
..LN2239:
   .loc    3  2834
        movq      -104(%rbp), %rax                              #2834.9
..LN2241:
        movq      %rax, -88(%rbp)                               #2834.5
..LN2243:
   .loc    3  2837
        movq      -104(%rbp), %rax                              #2837.10
..LN2245:
        movq      -96(%rbp), %rdx                               #2837.16
        cmpq      %rdx, %rax                                    #2837.16
        jae       ..B20.6       # Prob 50%                      #2837.16
                                # LOE
..B20.5:                        # Preds ..B20.4
..LN2247:
   .loc    3  2838
        movq      -104(%rbp), %rax                              #2838.24
        movzbl    -1(%rax), %eax                                #2838.24
        movzbl    %al, %eax                                     #2838.24
..LN2249:
        shll      $8, %eax                                      #2838.38
..LN2251:
        movl      %eax, -112(%rbp)                              #2838.9
                                # LOE
..B20.6:                        # Preds ..B20.5 ..B20.4
..LN2253:
   .loc    3  2842
        movq      -48(%rbp), %rax                               #2842.5
        movdqa    16(%rax), %xmm0                               #2842.5
        movdqa    %xmm0, -288(%rbp)                             #2842.5
        movq      -48(%rbp), %rax                               #2842.5
        movdqa    32(%rax), %xmm0                               #2842.5
        movdqa    %xmm0, -272(%rbp)                             #2842.5
        movq      -48(%rbp), %rax                               #2842.5
        movdqa    48(%rax), %xmm0                               #2842.5
        movdqa    %xmm0, -256(%rbp)                             #2842.5
        movq      -48(%rbp), %rax                               #2842.5
        movdqa    64(%rax), %xmm0                               #2842.5
        movdqa    %xmm0, -240(%rbp)                             #2842.5
        movq      -48(%rbp), %rax                               #2842.5
        movdqa    80(%rax), %xmm0                               #2842.5
        movdqa    %xmm0, -224(%rbp)                             #2842.5
        movq      -48(%rbp), %rax                               #2842.5
        movdqa    96(%rax), %xmm0                               #2842.5
        movdqa    %xmm0, -208(%rbp)                             #2842.5
        movq      -48(%rbp), %rax                               #2842.5
        movdqa    112(%rax), %xmm0                              #2842.5
        movdqa    %xmm0, -192(%rbp)                             #2842.5
        movq      -48(%rbp), %rax                               #2842.5
        movdqa    128(%rax), %xmm0                              #2842.5
        movdqa    %xmm0, -176(%rbp)                             #2842.5
..LN2255:
   .loc    3  2845
        xorl      %eax, %eax                                    #2845.15
        movl      %eax, -72(%rbp)                               #2845.15
..LN2257:
        movl      %eax, -68(%rbp)                               #2845.10
..LN2259:
        movl      %eax, -64(%rbp)                               #2845.5
..LN2261:
   .loc    3  2849
        movl      $4, -60(%rbp)                                 #2849.5
..LN2263:
   .loc    3  2854
        movl      -24(%rbp), %eax                               #2854.14
..LN2265:
        andl      $3, %eax                                      #2854.20
        movl      %eax, -56(%rbp)                               #2854.20
        je        ..B20.14      # Prob 50%                      #2854.20
                                # LOE
..B20.7:                        # Preds ..B20.6
        movl      -56(%rbp), %eax                               #2854.20
        cmpl      $1, %eax                                      #2854.20
        je        ..B20.15      # Prob 50%                      #2854.20
                                # LOE
..B20.8:                        # Preds ..B20.7
        movl      -56(%rbp), %eax                               #2854.20
        cmpl      $2, %eax                                      #2854.20
        je        ..B20.16      # Prob 50%                      #2854.20
                                # LOE
..B20.9:                        # Preds ..B20.8
        movl      -56(%rbp), %eax                               #2854.20
        cmpl      $3, %eax                                      #2854.20
        je        ..B20.17      # Prob 50%                      #2854.20
                                # LOE
..B20.10:                       # Preds ..B20.45 ..B20.44 ..B20.9
..LN2267:
   .loc    3  2859
        movl      $1, %eax                                      #2859.9
        testl     %eax, %eax                                    #2859.9
        je        ..B20.46      # Prob 100%                     #2859.9
                                # LOE
..B20.11:                       # Preds ..B20.10
..LN2269:
   .loc    3  2861
        movl      $4, -60(%rbp)                                 #2861.13
                                # LOE
..B20.12:                       # Preds ..B20.39 ..B20.11
..LN2271:
   .loc    3  2864
        movl      $1, %eax                                      #2864.13
        testl     %eax, %eax                                    #2864.13
        je        ..B20.41      # Prob 100%                     #2864.13
                                # LOE
..B20.14:                       # Preds ..B20.6 ..B20.12
..LN2273:
   .loc    3  2871
        movdqa    -304(%rbp), %xmm0                             #2871.38
..LN2275:
        movdqa    -272(%rbp), %xmm1                             #2871.46
..LN2277:
        pand      %xmm1, %xmm0                                  #2871.22
..LN2279:
        movdqa    %xmm0, -320(%rbp)                             #2871.17
..LN2281:
   .loc    3  2872
        movdqa    -320(%rbp), %xmm0                             #2872.22
        movdqa    -288(%rbp), %xmm1                             #2872.22
        pcmpeqd   %xmm1, %xmm0                                  #2872.22
..LN2283:
        movdqa    %xmm0, -320(%rbp)                             #2872.17
..LN2285:
   .loc    3  2873
        movdqa    -320(%rbp), %xmm0                             #2873.42
..LN2287:
        pmovmskb  %xmm0, %eax                                   #2873.22
..LN2289:
        movl      %eax, -64(%rbp)                               #2873.17
                                # LOE
..B20.15:                       # Preds ..B20.7 ..B20.14
..LN2291:
   .loc    3  2877
        movdqa    -304(%rbp), %xmm0                             #2877.38
..LN2293:
        movdqa    -240(%rbp), %xmm1                             #2877.46
..LN2295:
        pand      %xmm1, %xmm0                                  #2877.22
..LN2297:
        movdqa    %xmm0, -320(%rbp)                             #2877.17
..LN2299:
   .loc    3  2878
        movdqa    -320(%rbp), %xmm0                             #2878.22
        movdqa    -256(%rbp), %xmm1                             #2878.22
        pcmpeqd   %xmm1, %xmm0                                  #2878.22
..LN2301:
        movdqa    %xmm0, -320(%rbp)                             #2878.17
..LN2303:
   .loc    3  2879
        movdqa    -320(%rbp), %xmm0                             #2879.42
..LN2305:
        pmovmskb  %xmm0, %eax                                   #2879.22
..LN2307:
        movl      %eax, -68(%rbp)                               #2879.17
                                # LOE
..B20.16:                       # Preds ..B20.8 ..B20.15
..LN2309:
   .loc    3  2883
        movdqa    -304(%rbp), %xmm0                             #2883.38
..LN2311:
        movdqa    -208(%rbp), %xmm1                             #2883.46
..LN2313:
        pand      %xmm1, %xmm0                                  #2883.22
..LN2315:
        movdqa    %xmm0, -320(%rbp)                             #2883.17
..LN2317:
   .loc    3  2884
        movdqa    -320(%rbp), %xmm0                             #2884.22
        movdqa    -224(%rbp), %xmm1                             #2884.22
        pcmpeqd   %xmm1, %xmm0                                  #2884.22
..LN2319:
        movdqa    %xmm0, -320(%rbp)                             #2884.17
..LN2321:
   .loc    3  2885
        movdqa    -320(%rbp), %xmm0                             #2885.42
..LN2323:
        pmovmskb  %xmm0, %eax                                   #2885.22
..LN2325:
        movl      %eax, -72(%rbp)                               #2885.17
                                # LOE
..B20.17:                       # Preds ..B20.9 ..B20.16
..LN2327:
   .loc    3  2889
        movdqa    -304(%rbp), %xmm0                             #2889.38
..LN2329:
        movdqa    -176(%rbp), %xmm1                             #2889.46
..LN2331:
        pand      %xmm1, %xmm0                                  #2889.22
..LN2333:
        movdqa    %xmm0, -320(%rbp)                             #2889.17
..LN2335:
   .loc    3  2890
        movdqa    -320(%rbp), %xmm0                             #2890.22
        movdqa    -192(%rbp), %xmm1                             #2890.22
        pcmpeqd   %xmm1, %xmm0                                  #2890.22
..LN2337:
        movdqa    %xmm0, -320(%rbp)                             #2890.17
..LN2339:
   .loc    3  2891
        movdqa    -320(%rbp), %xmm0                             #2891.42
..LN2341:
        pmovmskb  %xmm0, %eax                                   #2891.22
..LN2343:
        movl      %eax, -80(%rbp)                               #2891.17
..LN2345:
   .loc    3  2896
        andl      $-4, -24(%rbp)                                #2896.17
..LN2347:
   .loc    3  2899
        movl      -68(%rbp), %eax                               #2899.29
        orl       -64(%rbp), %eax                               #2899.29
..LN2349:
        orl       -72(%rbp), %eax                               #2899.34
..LN2351:
        orl       -80(%rbp), %eax                               #2899.39
..LN2353:
        je        ..B20.34      # Prob 50%                      #2899.47
                                # LOE
..B20.18:                       # Preds ..B20.17
..LN2355:
   .loc    3  2917
        movl      -64(%rbp), %eax                               #2917.58
..LN2357:
        movl      %eax, %edi                                    #2917.30
        call      uint16_lsbit@PLT                              #2917.30
                                # LOE eax
..B20.50:                       # Preds ..B20.18
        movl      %eax, -128(%rbp)                              #2917.30
                                # LOE
..B20.19:                       # Preds ..B20.50
        movl      -128(%rbp), %eax                              #2917.30
        movswq    %ax, %rax                                     #2917.30
..LN2359:
        movl      %eax, -144(%rbp)                              #2917.28
..LN2361:
   .loc    3  2918
        movl      -68(%rbp), %eax                               #2918.58
..LN2363:
        movl      %eax, %edi                                    #2918.30
        call      uint16_lsbit@PLT                              #2918.30
                                # LOE eax
..B20.51:                       # Preds ..B20.19
        movl      %eax, -124(%rbp)                              #2918.30
                                # LOE
..B20.20:                       # Preds ..B20.51
        movl      -124(%rbp), %eax                              #2918.30
        movswq    %ax, %rax                                     #2918.30
..LN2365:
        movl      %eax, -140(%rbp)                              #2918.28
..LN2367:
   .loc    3  2919
        movl      -72(%rbp), %eax                               #2919.58
..LN2369:
        movl      %eax, %edi                                    #2919.30
        call      uint16_lsbit@PLT                              #2919.30
                                # LOE eax
..B20.52:                       # Preds ..B20.20
        movl      %eax, -120(%rbp)                              #2919.30
                                # LOE
..B20.21:                       # Preds ..B20.52
        movl      -120(%rbp), %eax                              #2919.30
        movswq    %ax, %rax                                     #2919.30
..LN2371:
        movl      %eax, -136(%rbp)                              #2919.28
..LN2373:
   .loc    3  2920
        movl      -80(%rbp), %eax                               #2920.58
..LN2375:
        movl      %eax, %edi                                    #2920.30
        call      uint16_lsbit@PLT                              #2920.30
                                # LOE eax
..B20.53:                       # Preds ..B20.21
        movl      %eax, -116(%rbp)                              #2920.30
                                # LOE
..B20.22:                       # Preds ..B20.53
        movl      -116(%rbp), %eax                              #2920.30
        movswq    %ax, %rax                                     #2920.30
..LN2377:
        movl      %eax, -132(%rbp)                              #2920.28
..LN2379:
   .loc    3  2925
        shll      $2, -144(%rbp)                                #2925.34
..LN2381:
   .loc    3  2926
        movl      -140(%rbp), %eax                              #2926.28
..LN2383:
        shll      $2, %eax                                      #2926.34
..LN2385:
        incl      %eax                                          #2926.40
..LN2387:
        movl      %eax, -140(%rbp)                              #2926.21
..LN2389:
   .loc    3  2927
        movl      -136(%rbp), %eax                              #2927.28
..LN2391:
        shll      $2, %eax                                      #2927.34
..LN2393:
        addl      $2, %eax                                      #2927.40
..LN2395:
        movl      %eax, -136(%rbp)                              #2927.21
..LN2397:
   .loc    3  2928
        movl      -132(%rbp), %eax                              #2928.28
..LN2399:
        shll      $2, %eax                                      #2928.34
..LN2401:
        addl      $3, %eax                                      #2928.40
..LN2403:
        movl      %eax, -132(%rbp)                              #2928.21
..LN2405:
   .loc    3  2932
        movl      -64(%rbp), %eax                               #2932.26
..LN2407:
        testl     %eax, %eax                                    #2932.32
        je        ..B20.25      # Prob 50%                      #2932.32
                                # LOE
..B20.23:                       # Preds ..B20.22
..LN2409:
        movl      -144(%rbp), %eax                              #2932.43
        addl      -24(%rbp), %eax                               #2932.43
..LN2411:
        movl      -76(%rbp), %edx                               #2932.49
        cmpl      %edx, %eax                                    #2932.49
        ja        ..B20.25      # Prob 50%                      #2932.49
                                # LOE
..B20.24:                       # Preds ..B20.23
..LN2413:
        movl      $1, %eax                                      #2932.63
        leave                                                   #2932.63
..___tag_value_eval_2na_32.280:                                 #
        ret                                                     #2932.63
..___tag_value_eval_2na_32.281:                                 #
                                # LOE
..B20.25:                       # Preds ..B20.23 ..B20.22
..LN2415:
   .loc    3  2933
        movl      -68(%rbp), %eax                               #2933.26
..LN2417:
        testl     %eax, %eax                                    #2933.32
        je        ..B20.28      # Prob 50%                      #2933.32
                                # LOE
..B20.26:                       # Preds ..B20.25
..LN2419:
        movl      -140(%rbp), %eax                              #2933.43
        addl      -24(%rbp), %eax                               #2933.43
..LN2421:
        movl      -76(%rbp), %edx                               #2933.49
        cmpl      %edx, %eax                                    #2933.49
        ja        ..B20.28      # Prob 50%                      #2933.49
                                # LOE
..B20.27:                       # Preds ..B20.26
..LN2423:
        movl      $1, %eax                                      #2933.63
        leave                                                   #2933.63
..___tag_value_eval_2na_32.283:                                 #
        ret                                                     #2933.63
..___tag_value_eval_2na_32.284:                                 #
                                # LOE
..B20.28:                       # Preds ..B20.26 ..B20.25
..LN2425:
   .loc    3  2934
        movl      -72(%rbp), %eax                               #2934.26
..LN2427:
        testl     %eax, %eax                                    #2934.32
        je        ..B20.31      # Prob 50%                      #2934.32
                                # LOE
..B20.29:                       # Preds ..B20.28
..LN2429:
        movl      -136(%rbp), %eax                              #2934.43
        addl      -24(%rbp), %eax                               #2934.43
..LN2431:
        movl      -76(%rbp), %edx                               #2934.49
        cmpl      %edx, %eax                                    #2934.49
        ja        ..B20.31      # Prob 50%                      #2934.49
                                # LOE
..B20.30:                       # Preds ..B20.29
..LN2433:
        movl      $1, %eax                                      #2934.63
        leave                                                   #2934.63
..___tag_value_eval_2na_32.286:                                 #
        ret                                                     #2934.63
..___tag_value_eval_2na_32.287:                                 #
                                # LOE
..B20.31:                       # Preds ..B20.29 ..B20.28
..LN2435:
   .loc    3  2935
        movl      -80(%rbp), %eax                               #2935.26
..LN2437:
        testl     %eax, %eax                                    #2935.32
        je        ..B20.34      # Prob 50%                      #2935.32
                                # LOE
..B20.32:                       # Preds ..B20.31
..LN2439:
        movl      -132(%rbp), %eax                              #2935.43
        addl      -24(%rbp), %eax                               #2935.43
..LN2441:
        movl      -76(%rbp), %edx                               #2935.49
        cmpl      %edx, %eax                                    #2935.49
        ja        ..B20.34      # Prob 50%                      #2935.49
                                # LOE
..B20.33:                       # Preds ..B20.32
..LN2443:
        movl      $1, %eax                                      #2935.63
        leave                                                   #2935.63
..___tag_value_eval_2na_32.289:                                 #
        ret                                                     #2935.63
..___tag_value_eval_2na_32.290:                                 #
                                # LOE
..B20.34:                       # Preds ..B20.32 ..B20.31 ..B20.17
..LN2445:
   .loc    3  2940
        addl      $4, -24(%rbp)                                 #2940.17
..LN2447:
   .loc    3  2943
        movl      -24(%rbp), %eax                               #2943.22
..LN2449:
        movl      -76(%rbp), %edx                               #2943.28
        cmpl      %edx, %eax                                    #2943.28
        jbe       ..B20.36      # Prob 50%                      #2943.28
                                # LOE
..B20.35:                       # Preds ..B20.34
..LN2451:
   .loc    3  2944
        xorl      %eax, %eax                                    #2944.28
        leave                                                   #2944.28
..___tag_value_eval_2na_32.292:                                 #
        ret                                                     #2944.28
..___tag_value_eval_2na_32.293:                                 #
                                # LOE
..B20.36:                       # Preds ..B20.34
..LN2453:
   .loc    3  2947
        movl      -60(%rbp), %eax                               #2947.25
        decl      %eax                                          #2947.25
        movl      %eax, -60(%rbp)                               #2947.25
..LN2455:
        je        ..B20.41      # Prob 50%                      #2947.39
                                # LOE
..B20.37:                       # Preds ..B20.36
..LN2457:
   .loc    3  2952
        movdqa    -304(%rbp), %xmm0                             #2952.43
..LN2459:
        psrldq    $1, %xmm0                                     #2952.26
..LN2461:
        movdqa    %xmm0, -304(%rbp)                             #2952.17
..LN2463:
   .loc    3  2955
        movq      -88(%rbp), %rax                               #2955.22
..LN2465:
        movq      -96(%rbp), %rdx                               #2955.26
        cmpq      %rdx, %rax                                    #2955.26
        jae       ..B20.39      # Prob 50%                      #2955.26
                                # LOE
..B20.38:                       # Preds ..B20.37
..LN2467:
   .loc    3  2957
        sarl      $8, -112(%rbp)                                #2957.21
..LN2469:
   .loc    3  2958
        movq      -88(%rbp), %rax                               #2958.37
        movzbl    (%rax), %eax                                  #2958.37
        movzbl    %al, %eax                                     #2958.37
..LN2471:
        shll      $8, %eax                                      #2958.48
..LN2473:
        orl       -112(%rbp), %eax                              #2958.21
        movl      %eax, -112(%rbp)                              #2958.21
..LN2475:
   .loc    3  2959
        movdqa    -304(%rbp), %xmm0                             #2959.49
..LN2477:
        movl      -112(%rbp), %eax                              #2959.57
..LN2479:
        pinsrw    $7, %eax, %xmm0                               #2959.30
..LN2481:
        movdqa    %xmm0, -304(%rbp)                             #2959.21
                                # LOE
..B20.39:                       # Preds ..B20.38 ..B20.37
..LN2483:
   .loc    3  2963
        incq      -88(%rbp)                                     #2963.20
        jmp       ..B20.12      # Prob 100%                     #2963.20
                                # LOE
..B20.41:                       # Preds ..B20.36 ..B20.12
..LN2485:
   .loc    3  2968
        movq      -104(%rbp), %rax                              #2968.18
..LN2487:
        movq      -96(%rbp), %rdx                               #2968.25
        cmpq      %rdx, %rax                                    #2968.25
        jae       ..B20.46      # Prob 50%                      #2968.25
                                # LOE
..B20.42:                       # Preds ..B20.41
..LN2489:
   .loc    3  2972
        addl      $48, -24(%rbp)                                #2972.13
..LN2491:
   .loc    3  2973
        movl      -24(%rbp), %eax                               #2973.18
..LN2493:
        movl      -76(%rbp), %edx                               #2973.24
        cmpl      %edx, %eax                                    #2973.24
        ja        ..B20.46      # Prob 50%                      #2973.24
                                # LOE
..B20.43:                       # Preds ..B20.42
..LN2495:
   .loc    3  2978
        movq      -104(%rbp), %rax                              #2978.41
..LN2497:
        movq      -96(%rbp), %rdx                               #2978.46
..LN2499:
        movq      %rax, %rdi                                    #2978.22
        movq      %rdx, %rsi                                    #2978.22
        call      prime_buffer_2na@PLT                          #2978.22
                                # LOE xmm0
..B20.54:                       # Preds ..B20.43
        movdqa    %xmm0, -336(%rbp)                             #2978.22
                                # LOE
..B20.44:                       # Preds ..B20.54
..LN2501:
        movdqa    -336(%rbp), %xmm0                             #2978.13
        movdqa    %xmm0, -304(%rbp)                             #2978.13
..LN2503:
   .loc    3  3056
        addq      $16, -104(%rbp)                               #3056.13
..LN2505:
   .loc    3  3058
        movq      -104(%rbp), %rax                              #3058.17
..LN2507:
        movq      %rax, -88(%rbp)                               #3058.13
..LN2509:
   .loc    3  3059
        movq      -104(%rbp), %rax                              #3059.18
..LN2511:
        movq      -96(%rbp), %rdx                               #3059.24
        cmpq      %rdx, %rax                                    #3059.24
        jae       ..B20.10      # Prob 50%                      #3059.24
                                # LOE
..B20.45:                       # Preds ..B20.44
..LN2513:
   .loc    3  3060
        movq      -104(%rbp), %rax                              #3060.32
        movzbl    -1(%rax), %eax                                #3060.32
        movzbl    %al, %eax                                     #3060.32
..LN2515:
        shll      $8, %eax                                      #3060.46
..LN2517:
        movl      %eax, -112(%rbp)                              #3060.17
        jmp       ..B20.10      # Prob 100%                     #3060.17
                                # LOE
..B20.46:                       # Preds ..B20.42 ..B20.41 ..B20.10
..LN2519:
   .loc    3  3069
        xorl      %eax, %eax                                    #3069.12
        leave                                                   #3069.12
..___tag_value_eval_2na_32.295:                                 #
        ret                                                     #3069.12
        .align    2,0x90
..___tag_value_eval_2na_32.296:                                 #
                                # LOE
# mark_end;
	.type	eval_2na_32,@function
	.size	eval_2na_32,.-eval_2na_32
.LNeval_2na_32:
	.data
# -- End  eval_2na_32
	.text
# -- Begin  eval_2na_64
# mark_begin;
       .align    2,0x90
eval_2na_64:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B21.1:                        # Preds ..B21.0
..___tag_value_eval_2na_64.299:                                 #
..LN2521:
   .loc    3  3075
        pushq     %rbp                                          #3075.1
        movq      %rsp, %rbp                                    #3075.1
..___tag_value_eval_2na_64.300:                                 #
        subq      $336, %rsp                                    #3075.1
        movq      %rdi, -48(%rbp)                               #3075.1
        movq      %rsi, -40(%rbp)                               #3075.1
        movl      %edx, -24(%rbp)                               #3075.1
        movl      %ecx, -16(%rbp)                               #3075.1
..LN2523:
   .loc    3  3106
        movq      -48(%rbp), %rax                               #3106.25
        movl      4(%rax), %eax                                 #3106.25
..LN2525:
        movl      %eax, -32(%rbp)                               #3106.23
..LN2527:
   .loc    3  3109
        movl      -16(%rbp), %eax                               #3109.5
        movl      -32(%rbp), %edx                               #3109.5
        cmpl      %edx, %eax                                    #3109.5
        jae       ..B21.3       # Prob 50%                      #3109.5
                                # LOE
..B21.2:                        # Preds ..B21.1
        lea       _2__STRING.9.0(%rip), %rax                    #3109.5
        lea       _2__STRING.1.0(%rip), %rdx                    #3109.5
        lea       __$U8.0(%rip), %rcx                           #3109.5
        movq      %rax, %rdi                                    #3109.5
        movq      %rdx, %rsi                                    #3109.5
        movl      $3109, %edx                                   #3109.5
        call      __assert_fail@PLT                             #3109.5
                                # LOE
..B21.3:                        # Preds ..B21.1
..LN2529:
   .loc    3  3113
        movl      -24(%rbp), %eax                               #3113.12
..LN2531:
        addl      -16(%rbp), %eax                               #3113.5
        movl      %eax, -16(%rbp)                               #3113.5
..LN2533:
   .loc    3  3116
        movl      -24(%rbp), %eax                               #3116.42
..LN2535:
        shrl      $2, %eax                                      #3116.49
..LN2537:
        addq      -40(%rbp), %rax                               #3116.30
..LN2539:
        movq      %rax, -104(%rbp)                              #3116.5
..LN2541:
   .loc    3  3119
        movl      -32(%rbp), %eax                               #3119.18
        negl      %eax                                          #3119.18
        addl      -16(%rbp), %eax                               #3119.18
..LN2543:
        movl      %eax, -76(%rbp)                               #3119.5
..LN2545:
   .loc    3  3122
        movl      -16(%rbp), %eax                               #3122.44
..LN2547:
        addl      $3, %eax                                      #3122.50
..LN2549:
        shrl      $2, %eax                                      #3122.57
..LN2551:
        addq      -40(%rbp), %rax                               #3122.30
..LN2553:
        movq      %rax, -96(%rbp)                               #3122.5
..LN2555:
   .loc    3  3125
        movq      -104(%rbp), %rax                              #3125.33
..LN2557:
        movq      -96(%rbp), %rdx                               #3125.38
..LN2559:
        movq      %rax, %rdi                                    #3125.14
        movq      %rdx, %rsi                                    #3125.14
        call      prime_buffer_2na@PLT                          #3125.14
                                # LOE xmm0
..B21.49:                       # Preds ..B21.3
        movdqa    %xmm0, -160(%rbp)                             #3125.14
                                # LOE
..B21.4:                        # Preds ..B21.49
..LN2561:
        movdqa    -160(%rbp), %xmm0                             #3125.5
        movdqa    %xmm0, -304(%rbp)                             #3125.5
..LN2563:
   .loc    3  3126
        addq      $16, -104(%rbp)                               #3126.5
..LN2565:
   .loc    3  3128
        movq      -104(%rbp), %rax                              #3128.9
..LN2567:
        movq      %rax, -88(%rbp)                               #3128.5
..LN2569:
   .loc    3  3131
        movq      -104(%rbp), %rax                              #3131.10
..LN2571:
        movq      -96(%rbp), %rdx                               #3131.16
        cmpq      %rdx, %rax                                    #3131.16
        jae       ..B21.6       # Prob 50%                      #3131.16
                                # LOE
..B21.5:                        # Preds ..B21.4
..LN2573:
   .loc    3  3132
        movq      -104(%rbp), %rax                              #3132.24
        movzbl    -1(%rax), %eax                                #3132.24
        movzbl    %al, %eax                                     #3132.24
..LN2575:
        shll      $8, %eax                                      #3132.38
..LN2577:
        movl      %eax, -112(%rbp)                              #3132.9
                                # LOE
..B21.6:                        # Preds ..B21.5 ..B21.4
..LN2579:
   .loc    3  3136
        movq      -48(%rbp), %rax                               #3136.5
        movdqa    16(%rax), %xmm0                               #3136.5
        movdqa    %xmm0, -288(%rbp)                             #3136.5
        movq      -48(%rbp), %rax                               #3136.5
        movdqa    32(%rax), %xmm0                               #3136.5
        movdqa    %xmm0, -272(%rbp)                             #3136.5
        movq      -48(%rbp), %rax                               #3136.5
        movdqa    48(%rax), %xmm0                               #3136.5
        movdqa    %xmm0, -256(%rbp)                             #3136.5
        movq      -48(%rbp), %rax                               #3136.5
        movdqa    64(%rax), %xmm0                               #3136.5
        movdqa    %xmm0, -240(%rbp)                             #3136.5
        movq      -48(%rbp), %rax                               #3136.5
        movdqa    80(%rax), %xmm0                               #3136.5
        movdqa    %xmm0, -224(%rbp)                             #3136.5
        movq      -48(%rbp), %rax                               #3136.5
        movdqa    96(%rax), %xmm0                               #3136.5
        movdqa    %xmm0, -208(%rbp)                             #3136.5
        movq      -48(%rbp), %rax                               #3136.5
        movdqa    112(%rax), %xmm0                              #3136.5
        movdqa    %xmm0, -192(%rbp)                             #3136.5
        movq      -48(%rbp), %rax                               #3136.5
        movdqa    128(%rax), %xmm0                              #3136.5
        movdqa    %xmm0, -176(%rbp)                             #3136.5
..LN2581:
   .loc    3  3139
        xorl      %eax, %eax                                    #3139.15
        movl      %eax, -72(%rbp)                               #3139.15
..LN2583:
        movl      %eax, -68(%rbp)                               #3139.10
..LN2585:
        movl      %eax, -64(%rbp)                               #3139.5
..LN2587:
   .loc    3  3143
        movl      $8, -60(%rbp)                                 #3143.5
..LN2589:
   .loc    3  3148
        movl      -24(%rbp), %eax                               #3148.14
..LN2591:
        andl      $3, %eax                                      #3148.20
        movl      %eax, -56(%rbp)                               #3148.20
        je        ..B21.14      # Prob 50%                      #3148.20
                                # LOE
..B21.7:                        # Preds ..B21.6
        movl      -56(%rbp), %eax                               #3148.20
        cmpl      $1, %eax                                      #3148.20
        je        ..B21.15      # Prob 50%                      #3148.20
                                # LOE
..B21.8:                        # Preds ..B21.7
        movl      -56(%rbp), %eax                               #3148.20
        cmpl      $2, %eax                                      #3148.20
        je        ..B21.16      # Prob 50%                      #3148.20
                                # LOE
..B21.9:                        # Preds ..B21.8
        movl      -56(%rbp), %eax                               #3148.20
        cmpl      $3, %eax                                      #3148.20
        je        ..B21.17      # Prob 50%                      #3148.20
                                # LOE
..B21.10:                       # Preds ..B21.45 ..B21.44 ..B21.9
..LN2593:
   .loc    3  3153
        movl      $1, %eax                                      #3153.9
        testl     %eax, %eax                                    #3153.9
        je        ..B21.46      # Prob 100%                     #3153.9
                                # LOE
..B21.11:                       # Preds ..B21.10
..LN2595:
   .loc    3  3155
        movl      $8, -60(%rbp)                                 #3155.13
                                # LOE
..B21.12:                       # Preds ..B21.39 ..B21.11
..LN2597:
   .loc    3  3158
        movl      $1, %eax                                      #3158.13
        testl     %eax, %eax                                    #3158.13
        je        ..B21.41      # Prob 100%                     #3158.13
                                # LOE
..B21.14:                       # Preds ..B21.6 ..B21.12
..LN2599:
   .loc    3  3165
        movdqa    -304(%rbp), %xmm0                             #3165.38
..LN2601:
        movdqa    -272(%rbp), %xmm1                             #3165.46
..LN2603:
        pand      %xmm1, %xmm0                                  #3165.22
..LN2605:
        movdqa    %xmm0, -320(%rbp)                             #3165.17
..LN2607:
   .loc    3  3166
        movdqa    -320(%rbp), %xmm0                             #3166.22
        movdqa    -288(%rbp), %xmm1                             #3166.22
        pcmpeqd   %xmm1, %xmm0                                  #3166.22
..LN2609:
        movdqa    %xmm0, -320(%rbp)                             #3166.17
..LN2611:
   .loc    3  3167
        movdqa    -320(%rbp), %xmm0                             #3167.42
..LN2613:
        pmovmskb  %xmm0, %eax                                   #3167.22
..LN2615:
        movl      %eax, -64(%rbp)                               #3167.17
..LN2617:
   .loc    3  3168
        movl      -64(%rbp), %eax                               #3168.17
        andl      $3855, %eax                                   #3168.17
        shll      $4, %eax                                      #3168.17
        andl      -64(%rbp), %eax                               #3168.17
        movl      %eax, -64(%rbp)                               #3168.17
        movl      -64(%rbp), %eax                               #3168.17
        sarl      $4, %eax                                      #3168.17
        orl       -64(%rbp), %eax                               #3168.17
        movl      %eax, -64(%rbp)                               #3168.17
                                # LOE
..B21.15:                       # Preds ..B21.7 ..B21.14
..LN2619:
   .loc    3  3171
        movdqa    -304(%rbp), %xmm0                             #3171.38
..LN2621:
        movdqa    -240(%rbp), %xmm1                             #3171.46
..LN2623:
        pand      %xmm1, %xmm0                                  #3171.22
..LN2625:
        movdqa    %xmm0, -320(%rbp)                             #3171.17
..LN2627:
   .loc    3  3172
        movdqa    -320(%rbp), %xmm0                             #3172.22
        movdqa    -256(%rbp), %xmm1                             #3172.22
        pcmpeqd   %xmm1, %xmm0                                  #3172.22
..LN2629:
        movdqa    %xmm0, -320(%rbp)                             #3172.17
..LN2631:
   .loc    3  3173
        movdqa    -320(%rbp), %xmm0                             #3173.42
..LN2633:
        pmovmskb  %xmm0, %eax                                   #3173.22
..LN2635:
        movl      %eax, -68(%rbp)                               #3173.17
..LN2637:
   .loc    3  3174
        movl      -68(%rbp), %eax                               #3174.17
        andl      $3855, %eax                                   #3174.17
        shll      $4, %eax                                      #3174.17
        andl      -68(%rbp), %eax                               #3174.17
        movl      %eax, -68(%rbp)                               #3174.17
        movl      -68(%rbp), %eax                               #3174.17
        sarl      $4, %eax                                      #3174.17
        orl       -68(%rbp), %eax                               #3174.17
        movl      %eax, -68(%rbp)                               #3174.17
                                # LOE
..B21.16:                       # Preds ..B21.8 ..B21.15
..LN2639:
   .loc    3  3177
        movdqa    -304(%rbp), %xmm0                             #3177.38
..LN2641:
        movdqa    -208(%rbp), %xmm1                             #3177.46
..LN2643:
        pand      %xmm1, %xmm0                                  #3177.22
..LN2645:
        movdqa    %xmm0, -320(%rbp)                             #3177.17
..LN2647:
   .loc    3  3178
        movdqa    -320(%rbp), %xmm0                             #3178.22
        movdqa    -224(%rbp), %xmm1                             #3178.22
        pcmpeqd   %xmm1, %xmm0                                  #3178.22
..LN2649:
        movdqa    %xmm0, -320(%rbp)                             #3178.17
..LN2651:
   .loc    3  3179
        movdqa    -320(%rbp), %xmm0                             #3179.42
..LN2653:
        pmovmskb  %xmm0, %eax                                   #3179.22
..LN2655:
        movl      %eax, -72(%rbp)                               #3179.17
..LN2657:
   .loc    3  3180
        movl      -72(%rbp), %eax                               #3180.17
        andl      $3855, %eax                                   #3180.17
        shll      $4, %eax                                      #3180.17
        andl      -72(%rbp), %eax                               #3180.17
        movl      %eax, -72(%rbp)                               #3180.17
        movl      -72(%rbp), %eax                               #3180.17
        sarl      $4, %eax                                      #3180.17
        orl       -72(%rbp), %eax                               #3180.17
        movl      %eax, -72(%rbp)                               #3180.17
                                # LOE
..B21.17:                       # Preds ..B21.9 ..B21.16
..LN2659:
   .loc    3  3183
        movdqa    -304(%rbp), %xmm0                             #3183.38
..LN2661:
        movdqa    -176(%rbp), %xmm1                             #3183.46
..LN2663:
        pand      %xmm1, %xmm0                                  #3183.22
..LN2665:
        movdqa    %xmm0, -320(%rbp)                             #3183.17
..LN2667:
   .loc    3  3184
        movdqa    -320(%rbp), %xmm0                             #3184.22
        movdqa    -192(%rbp), %xmm1                             #3184.22
        pcmpeqd   %xmm1, %xmm0                                  #3184.22
..LN2669:
        movdqa    %xmm0, -320(%rbp)                             #3184.17
..LN2671:
   .loc    3  3185
        movdqa    -320(%rbp), %xmm0                             #3185.42
..LN2673:
        pmovmskb  %xmm0, %eax                                   #3185.22
..LN2675:
        movl      %eax, -80(%rbp)                               #3185.17
..LN2677:
   .loc    3  3186
        movl      -80(%rbp), %eax                               #3186.17
        andl      $3855, %eax                                   #3186.17
        shll      $4, %eax                                      #3186.17
        andl      -80(%rbp), %eax                               #3186.17
        movl      %eax, -80(%rbp)                               #3186.17
        movl      -80(%rbp), %eax                               #3186.17
        sarl      $4, %eax                                      #3186.17
        orl       -80(%rbp), %eax                               #3186.17
        movl      %eax, -80(%rbp)                               #3186.17
..LN2679:
   .loc    3  3190
        andl      $-4, -24(%rbp)                                #3190.17
..LN2681:
   .loc    3  3193
        movl      -68(%rbp), %eax                               #3193.29
        orl       -64(%rbp), %eax                               #3193.29
..LN2683:
        orl       -72(%rbp), %eax                               #3193.34
..LN2685:
        orl       -80(%rbp), %eax                               #3193.39
..LN2687:
        je        ..B21.34      # Prob 50%                      #3193.47
                                # LOE
..B21.18:                       # Preds ..B21.17
..LN2689:
   .loc    3  3211
        movl      -64(%rbp), %eax                               #3211.58
..LN2691:
        movl      %eax, %edi                                    #3211.30
        call      uint16_lsbit@PLT                              #3211.30
                                # LOE eax
..B21.50:                       # Preds ..B21.18
        movl      %eax, -128(%rbp)                              #3211.30
                                # LOE
..B21.19:                       # Preds ..B21.50
        movl      -128(%rbp), %eax                              #3211.30
        movswq    %ax, %rax                                     #3211.30
..LN2693:
        movl      %eax, -144(%rbp)                              #3211.28
..LN2695:
   .loc    3  3212
        movl      -68(%rbp), %eax                               #3212.58
..LN2697:
        movl      %eax, %edi                                    #3212.30
        call      uint16_lsbit@PLT                              #3212.30
                                # LOE eax
..B21.51:                       # Preds ..B21.19
        movl      %eax, -124(%rbp)                              #3212.30
                                # LOE
..B21.20:                       # Preds ..B21.51
        movl      -124(%rbp), %eax                              #3212.30
        movswq    %ax, %rax                                     #3212.30
..LN2699:
        movl      %eax, -140(%rbp)                              #3212.28
..LN2701:
   .loc    3  3213
        movl      -72(%rbp), %eax                               #3213.58
..LN2703:
        movl      %eax, %edi                                    #3213.30
        call      uint16_lsbit@PLT                              #3213.30
                                # LOE eax
..B21.52:                       # Preds ..B21.20
        movl      %eax, -120(%rbp)                              #3213.30
                                # LOE
..B21.21:                       # Preds ..B21.52
        movl      -120(%rbp), %eax                              #3213.30
        movswq    %ax, %rax                                     #3213.30
..LN2705:
        movl      %eax, -136(%rbp)                              #3213.28
..LN2707:
   .loc    3  3214
        movl      -80(%rbp), %eax                               #3214.58
..LN2709:
        movl      %eax, %edi                                    #3214.30
        call      uint16_lsbit@PLT                              #3214.30
                                # LOE eax
..B21.53:                       # Preds ..B21.21
        movl      %eax, -116(%rbp)                              #3214.30
                                # LOE
..B21.22:                       # Preds ..B21.53
        movl      -116(%rbp), %eax                              #3214.30
        movswq    %ax, %rax                                     #3214.30
..LN2711:
        movl      %eax, -132(%rbp)                              #3214.28
..LN2713:
   .loc    3  3219
        shll      $2, -144(%rbp)                                #3219.34
..LN2715:
   .loc    3  3220
        movl      -140(%rbp), %eax                              #3220.28
..LN2717:
        shll      $2, %eax                                      #3220.34
..LN2719:
        incl      %eax                                          #3220.40
..LN2721:
        movl      %eax, -140(%rbp)                              #3220.21
..LN2723:
   .loc    3  3221
        movl      -136(%rbp), %eax                              #3221.28
..LN2725:
        shll      $2, %eax                                      #3221.34
..LN2727:
        addl      $2, %eax                                      #3221.40
..LN2729:
        movl      %eax, -136(%rbp)                              #3221.21
..LN2731:
   .loc    3  3222
        movl      -132(%rbp), %eax                              #3222.28
..LN2733:
        shll      $2, %eax                                      #3222.34
..LN2735:
        addl      $3, %eax                                      #3222.40
..LN2737:
        movl      %eax, -132(%rbp)                              #3222.21
..LN2739:
   .loc    3  3226
        movl      -64(%rbp), %eax                               #3226.26
..LN2741:
        testl     %eax, %eax                                    #3226.32
        je        ..B21.25      # Prob 50%                      #3226.32
                                # LOE
..B21.23:                       # Preds ..B21.22
..LN2743:
        movl      -144(%rbp), %eax                              #3226.43
        addl      -24(%rbp), %eax                               #3226.43
..LN2745:
        movl      -76(%rbp), %edx                               #3226.49
        cmpl      %edx, %eax                                    #3226.49
        ja        ..B21.25      # Prob 50%                      #3226.49
                                # LOE
..B21.24:                       # Preds ..B21.23
..LN2747:
        movl      $1, %eax                                      #3226.63
        leave                                                   #3226.63
..___tag_value_eval_2na_64.304:                                 #
        ret                                                     #3226.63
..___tag_value_eval_2na_64.305:                                 #
                                # LOE
..B21.25:                       # Preds ..B21.23 ..B21.22
..LN2749:
   .loc    3  3227
        movl      -68(%rbp), %eax                               #3227.26
..LN2751:
        testl     %eax, %eax                                    #3227.32
        je        ..B21.28      # Prob 50%                      #3227.32
                                # LOE
..B21.26:                       # Preds ..B21.25
..LN2753:
        movl      -140(%rbp), %eax                              #3227.43
        addl      -24(%rbp), %eax                               #3227.43
..LN2755:
        movl      -76(%rbp), %edx                               #3227.49
        cmpl      %edx, %eax                                    #3227.49
        ja        ..B21.28      # Prob 50%                      #3227.49
                                # LOE
..B21.27:                       # Preds ..B21.26
..LN2757:
        movl      $1, %eax                                      #3227.63
        leave                                                   #3227.63
..___tag_value_eval_2na_64.307:                                 #
        ret                                                     #3227.63
..___tag_value_eval_2na_64.308:                                 #
                                # LOE
..B21.28:                       # Preds ..B21.26 ..B21.25
..LN2759:
   .loc    3  3228
        movl      -72(%rbp), %eax                               #3228.26
..LN2761:
        testl     %eax, %eax                                    #3228.32
        je        ..B21.31      # Prob 50%                      #3228.32
                                # LOE
..B21.29:                       # Preds ..B21.28
..LN2763:
        movl      -136(%rbp), %eax                              #3228.43
        addl      -24(%rbp), %eax                               #3228.43
..LN2765:
        movl      -76(%rbp), %edx                               #3228.49
        cmpl      %edx, %eax                                    #3228.49
        ja        ..B21.31      # Prob 50%                      #3228.49
                                # LOE
..B21.30:                       # Preds ..B21.29
..LN2767:
        movl      $1, %eax                                      #3228.63
        leave                                                   #3228.63
..___tag_value_eval_2na_64.310:                                 #
        ret                                                     #3228.63
..___tag_value_eval_2na_64.311:                                 #
                                # LOE
..B21.31:                       # Preds ..B21.29 ..B21.28
..LN2769:
   .loc    3  3229
        movl      -80(%rbp), %eax                               #3229.26
..LN2771:
        testl     %eax, %eax                                    #3229.32
        je        ..B21.34      # Prob 50%                      #3229.32
                                # LOE
..B21.32:                       # Preds ..B21.31
..LN2773:
        movl      -132(%rbp), %eax                              #3229.43
        addl      -24(%rbp), %eax                               #3229.43
..LN2775:
        movl      -76(%rbp), %edx                               #3229.49
        cmpl      %edx, %eax                                    #3229.49
        ja        ..B21.34      # Prob 50%                      #3229.49
                                # LOE
..B21.33:                       # Preds ..B21.32
..LN2777:
        movl      $1, %eax                                      #3229.63
        leave                                                   #3229.63
..___tag_value_eval_2na_64.313:                                 #
        ret                                                     #3229.63
..___tag_value_eval_2na_64.314:                                 #
                                # LOE
..B21.34:                       # Preds ..B21.32 ..B21.31 ..B21.17
..LN2779:
   .loc    3  3234
        addl      $4, -24(%rbp)                                 #3234.17
..LN2781:
   .loc    3  3237
        movl      -24(%rbp), %eax                               #3237.22
..LN2783:
        movl      -76(%rbp), %edx                               #3237.28
        cmpl      %edx, %eax                                    #3237.28
        jbe       ..B21.36      # Prob 50%                      #3237.28
                                # LOE
..B21.35:                       # Preds ..B21.34
..LN2785:
   .loc    3  3238
        xorl      %eax, %eax                                    #3238.28
        leave                                                   #3238.28
..___tag_value_eval_2na_64.316:                                 #
        ret                                                     #3238.28
..___tag_value_eval_2na_64.317:                                 #
                                # LOE
..B21.36:                       # Preds ..B21.34
..LN2787:
   .loc    3  3241
        movl      -60(%rbp), %eax                               #3241.25
        decl      %eax                                          #3241.25
        movl      %eax, -60(%rbp)                               #3241.25
..LN2789:
        je        ..B21.41      # Prob 50%                      #3241.39
                                # LOE
..B21.37:                       # Preds ..B21.36
..LN2791:
   .loc    3  3246
        movdqa    -304(%rbp), %xmm0                             #3246.43
..LN2793:
        psrldq    $1, %xmm0                                     #3246.26
..LN2795:
        movdqa    %xmm0, -304(%rbp)                             #3246.17
..LN2797:
   .loc    3  3249
        movq      -88(%rbp), %rax                               #3249.22
..LN2799:
        movq      -96(%rbp), %rdx                               #3249.26
        cmpq      %rdx, %rax                                    #3249.26
        jae       ..B21.39      # Prob 50%                      #3249.26
                                # LOE
..B21.38:                       # Preds ..B21.37
..LN2801:
   .loc    3  3251
        sarl      $8, -112(%rbp)                                #3251.21
..LN2803:
   .loc    3  3252
        movq      -88(%rbp), %rax                               #3252.37
        movzbl    (%rax), %eax                                  #3252.37
        movzbl    %al, %eax                                     #3252.37
..LN2805:
        shll      $8, %eax                                      #3252.48
..LN2807:
        orl       -112(%rbp), %eax                              #3252.21
        movl      %eax, -112(%rbp)                              #3252.21
..LN2809:
   .loc    3  3253
        movdqa    -304(%rbp), %xmm0                             #3253.49
..LN2811:
        movl      -112(%rbp), %eax                              #3253.57
..LN2813:
        pinsrw    $7, %eax, %xmm0                               #3253.30
..LN2815:
        movdqa    %xmm0, -304(%rbp)                             #3253.21
                                # LOE
..B21.39:                       # Preds ..B21.38 ..B21.37
..LN2817:
   .loc    3  3257
        incq      -88(%rbp)                                     #3257.20
        jmp       ..B21.12      # Prob 100%                     #3257.20
                                # LOE
..B21.41:                       # Preds ..B21.36 ..B21.12
..LN2819:
   .loc    3  3262
        movq      -104(%rbp), %rax                              #3262.18
..LN2821:
        movq      -96(%rbp), %rdx                               #3262.25
        cmpq      %rdx, %rax                                    #3262.25
        jae       ..B21.46      # Prob 50%                      #3262.25
                                # LOE
..B21.42:                       # Preds ..B21.41
..LN2823:
   .loc    3  3266
        addl      $32, -24(%rbp)                                #3266.13
..LN2825:
   .loc    3  3267
        movl      -24(%rbp), %eax                               #3267.18
..LN2827:
        movl      -76(%rbp), %edx                               #3267.24
        cmpl      %edx, %eax                                    #3267.24
        ja        ..B21.46      # Prob 50%                      #3267.24
                                # LOE
..B21.43:                       # Preds ..B21.42
..LN2829:
   .loc    3  3272
        movq      -104(%rbp), %rax                              #3272.41
..LN2831:
        movq      -96(%rbp), %rdx                               #3272.46
..LN2833:
        movq      %rax, %rdi                                    #3272.22
        movq      %rdx, %rsi                                    #3272.22
        call      prime_buffer_2na@PLT                          #3272.22
                                # LOE xmm0
..B21.54:                       # Preds ..B21.43
        movdqa    %xmm0, -336(%rbp)                             #3272.22
                                # LOE
..B21.44:                       # Preds ..B21.54
..LN2835:
        movdqa    -336(%rbp), %xmm0                             #3272.13
        movdqa    %xmm0, -304(%rbp)                             #3272.13
..LN2837:
   .loc    3  3350
        addq      $16, -104(%rbp)                               #3350.13
..LN2839:
   .loc    3  3352
        movq      -104(%rbp), %rax                              #3352.17
..LN2841:
        movq      %rax, -88(%rbp)                               #3352.13
..LN2843:
   .loc    3  3353
        movq      -104(%rbp), %rax                              #3353.18
..LN2845:
        movq      -96(%rbp), %rdx                               #3353.24
        cmpq      %rdx, %rax                                    #3353.24
        jae       ..B21.10      # Prob 50%                      #3353.24
                                # LOE
..B21.45:                       # Preds ..B21.44
..LN2847:
   .loc    3  3354
        movq      -104(%rbp), %rax                              #3354.32
        movzbl    -1(%rax), %eax                                #3354.32
        movzbl    %al, %eax                                     #3354.32
..LN2849:
        shll      $8, %eax                                      #3354.46
..LN2851:
        movl      %eax, -112(%rbp)                              #3354.17
        jmp       ..B21.10      # Prob 100%                     #3354.17
                                # LOE
..B21.46:                       # Preds ..B21.42 ..B21.41 ..B21.10
..LN2853:
   .loc    3  3363
        xorl      %eax, %eax                                    #3363.12
        leave                                                   #3363.12
..___tag_value_eval_2na_64.319:                                 #
        ret                                                     #3363.12
        .align    2,0x90
..___tag_value_eval_2na_64.320:                                 #
                                # LOE
# mark_end;
	.type	eval_2na_64,@function
	.size	eval_2na_64,.-eval_2na_64
.LNeval_2na_64:
	.data
# -- End  eval_2na_64
	.text
# -- Begin  eval_2na_128
# mark_begin;
       .align    2,0x90
eval_2na_128:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B22.1:                        # Preds ..B22.0
..___tag_value_eval_2na_128.323:                                #
..LN2855:
   .loc    3  3369
        pushq     %rbp                                          #3369.1
        movq      %rsp, %rbp                                    #3369.1
..___tag_value_eval_2na_128.324:                                #
        subq      $304, %rsp                                    #3369.1
        movq      %rdi, -40(%rbp)                               #3369.1
        movq      %rsi, -32(%rbp)                               #3369.1
        movl      %edx, -16(%rbp)                               #3369.1
        movl      %ecx, -8(%rbp)                                #3369.1
..LN2857:
   .loc    3  3399
        movq      -40(%rbp), %rax                               #3399.25
        movl      4(%rax), %eax                                 #3399.25
..LN2859:
        movl      %eax, -24(%rbp)                               #3399.23
..LN2861:
   .loc    3  3402
        movl      -8(%rbp), %eax                                #3402.5
        movl      -24(%rbp), %edx                               #3402.5
        cmpl      %edx, %eax                                    #3402.5
        jae       ..B22.3       # Prob 50%                      #3402.5
                                # LOE
..B22.2:                        # Preds ..B22.1
        lea       _2__STRING.9.0(%rip), %rax                    #3402.5
        lea       _2__STRING.1.0(%rip), %rdx                    #3402.5
        lea       __$U9.0(%rip), %rcx                           #3402.5
        movq      %rax, %rdi                                    #3402.5
        movq      %rdx, %rsi                                    #3402.5
        movl      $3402, %edx                                   #3402.5
        call      __assert_fail@PLT                             #3402.5
                                # LOE
..B22.3:                        # Preds ..B22.1
..LN2863:
   .loc    3  3406
        movl      -16(%rbp), %eax                               #3406.12
..LN2865:
        addl      -8(%rbp), %eax                                #3406.5
        movl      %eax, -8(%rbp)                                #3406.5
..LN2867:
   .loc    3  3409
        movl      -16(%rbp), %eax                               #3409.42
..LN2869:
        shrl      $2, %eax                                      #3409.49
..LN2871:
        addq      -32(%rbp), %rax                               #3409.30
..LN2873:
        movq      %rax, -88(%rbp)                               #3409.5
..LN2875:
   .loc    3  3412
        movl      -24(%rbp), %eax                               #3412.18
        negl      %eax                                          #3412.18
        addl      -8(%rbp), %eax                                #3412.18
..LN2877:
        movl      %eax, -64(%rbp)                               #3412.5
..LN2879:
   .loc    3  3415
        movl      -8(%rbp), %eax                                #3415.44
..LN2881:
        addl      $3, %eax                                      #3415.50
..LN2883:
        shrl      $2, %eax                                      #3415.57
..LN2885:
        addq      -32(%rbp), %rax                               #3415.30
..LN2887:
        movq      %rax, -80(%rbp)                               #3415.5
..LN2889:
   .loc    3  3418
        movq      -88(%rbp), %rax                               #3418.33
..LN2891:
        movq      -80(%rbp), %rdx                               #3418.38
..LN2893:
        movq      %rax, %rdi                                    #3418.14
        movq      %rdx, %rsi                                    #3418.14
        call      prime_buffer_2na@PLT                          #3418.14
                                # LOE xmm0
..B22.44:                       # Preds ..B22.3
        movdqa    %xmm0, -112(%rbp)                             #3418.14
                                # LOE
..B22.4:                        # Preds ..B22.44
..LN2895:
        movdqa    -112(%rbp), %xmm0                             #3418.5
        movdqa    %xmm0, -256(%rbp)                             #3418.5
..LN2897:
   .loc    3  3419
        addq      $16, -88(%rbp)                                #3419.5
..LN2899:
   .loc    3  3421
        movq      -88(%rbp), %rax                               #3421.9
..LN2901:
        movq      %rax, -72(%rbp)                               #3421.5
..LN2903:
   .loc    3  3424
        movq      -88(%rbp), %rax                               #3424.10
..LN2905:
        movq      -80(%rbp), %rdx                               #3424.16
        cmpq      %rdx, %rax                                    #3424.16
        jae       ..B22.6       # Prob 50%                      #3424.16
                                # LOE
..B22.5:                        # Preds ..B22.4
..LN2907:
   .loc    3  3425
        movq      -88(%rbp), %rax                               #3425.24
        movzbl    -1(%rax), %eax                                #3425.24
        movzbl    %al, %eax                                     #3425.24
..LN2909:
        shll      $8, %eax                                      #3425.38
..LN2911:
        movl      %eax, -96(%rbp)                               #3425.9
                                # LOE
..B22.6:                        # Preds ..B22.5 ..B22.4
..LN2913:
   .loc    3  3429
        movq      -40(%rbp), %rax                               #3429.5
        movdqa    16(%rax), %xmm0                               #3429.5
        movdqa    %xmm0, -240(%rbp)                             #3429.5
        movq      -40(%rbp), %rax                               #3429.5
        movdqa    32(%rax), %xmm0                               #3429.5
        movdqa    %xmm0, -224(%rbp)                             #3429.5
        movq      -40(%rbp), %rax                               #3429.5
        movdqa    48(%rax), %xmm0                               #3429.5
        movdqa    %xmm0, -208(%rbp)                             #3429.5
        movq      -40(%rbp), %rax                               #3429.5
        movdqa    64(%rax), %xmm0                               #3429.5
        movdqa    %xmm0, -192(%rbp)                             #3429.5
        movq      -40(%rbp), %rax                               #3429.5
        movdqa    80(%rax), %xmm0                               #3429.5
        movdqa    %xmm0, -176(%rbp)                             #3429.5
        movq      -40(%rbp), %rax                               #3429.5
        movdqa    96(%rax), %xmm0                               #3429.5
        movdqa    %xmm0, -160(%rbp)                             #3429.5
        movq      -40(%rbp), %rax                               #3429.5
        movdqa    112(%rax), %xmm0                              #3429.5
        movdqa    %xmm0, -144(%rbp)                             #3429.5
        movq      -40(%rbp), %rax                               #3429.5
        movdqa    128(%rax), %xmm0                              #3429.5
        movdqa    %xmm0, -128(%rbp)                             #3429.5
..LN2915:
   .loc    3  3432
        xorl      %eax, %eax                                    #3432.15
        movl      %eax, -60(%rbp)                               #3432.15
..LN2917:
        movl      %eax, -56(%rbp)                               #3432.10
..LN2919:
        movl      %eax, -52(%rbp)                               #3432.5
..LN2921:
   .loc    3  3434
        movl      -16(%rbp), %eax                               #3434.27
        negl      %eax                                          #3434.27
..LN2923:
        movl      -64(%rbp), %edx                               #3434.20
..LN2925:
        lea       7(%rdx,%rax), %eax                            #3434.33
..LN2927:
        shrl      $2, %eax                                      #3434.40
..LN2929:
        movl      %eax, -48(%rbp)                               #3434.5
..LN2931:
   .loc    3  3441
        movl      -16(%rbp), %eax                               #3441.14
..LN2933:
        andl      $3, %eax                                      #3441.20
        movl      %eax, -44(%rbp)                               #3441.20
        je        ..B22.14      # Prob 50%                      #3441.20
                                # LOE
..B22.7:                        # Preds ..B22.6
        movl      -44(%rbp), %eax                               #3441.20
        cmpl      $1, %eax                                      #3441.20
        je        ..B22.15      # Prob 50%                      #3441.20
                                # LOE
..B22.8:                        # Preds ..B22.7
        movl      -44(%rbp), %eax                               #3441.20
        cmpl      $2, %eax                                      #3441.20
        je        ..B22.16      # Prob 50%                      #3441.20
                                # LOE
..B22.9:                        # Preds ..B22.8
        movl      -44(%rbp), %eax                               #3441.20
        cmpl      $3, %eax                                      #3441.20
        je        ..B22.17      # Prob 50%                      #3441.20
                                # LOE
..B22.10:                       # Preds ..B22.40 ..B22.39 ..B22.9
..LN2935:
   .loc    3  3446
        movl      $1, %eax                                      #3446.9
        testl     %eax, %eax                                    #3446.9
        je        ..B22.41      # Prob 100%                     #3446.9
                                # LOE
..B22.11:                       # Preds ..B22.10
..LN2937:
   .loc    3  3448
        movl      $16, -48(%rbp)                                #3448.13
                                # LOE
..B22.12:                       # Preds ..B22.34 ..B22.11
..LN2939:
   .loc    3  3451
        movl      $1, %eax                                      #3451.13
        testl     %eax, %eax                                    #3451.13
        je        ..B22.36      # Prob 100%                     #3451.13
                                # LOE
..B22.14:                       # Preds ..B22.6 ..B22.12
..LN2941:
   .loc    3  3458
        movdqa    -256(%rbp), %xmm0                             #3458.38
..LN2943:
        movdqa    -224(%rbp), %xmm1                             #3458.46
..LN2945:
        pand      %xmm1, %xmm0                                  #3458.22
..LN2947:
        movdqa    %xmm0, -288(%rbp)                             #3458.17
..LN2949:
   .loc    3  3459
        movdqa    -288(%rbp), %xmm0                             #3459.22
        movdqa    -240(%rbp), %xmm1                             #3459.22
        pcmpeqd   %xmm1, %xmm0                                  #3459.22
..LN2951:
        movdqa    %xmm0, -288(%rbp)                             #3459.17
..LN2953:
   .loc    3  3460
        movdqa    -288(%rbp), %xmm0                             #3460.42
..LN2955:
        pmovmskb  %xmm0, %eax                                   #3460.22
..LN2957:
        movl      %eax, -52(%rbp)                               #3460.17
..LN2959:
   .loc    3  3461
        movl      -52(%rbp), %eax                               #3461.17
        incl      %eax                                          #3461.17
        sarl      $16, %eax                                     #3461.17
        negl      %eax                                          #3461.17
        movl      %eax, -52(%rbp)                               #3461.17
                                # LOE
..B22.15:                       # Preds ..B22.7 ..B22.14
..LN2961:
   .loc    3  3464
        movdqa    -256(%rbp), %xmm0                             #3464.38
..LN2963:
        movdqa    -192(%rbp), %xmm1                             #3464.46
..LN2965:
        pand      %xmm1, %xmm0                                  #3464.22
..LN2967:
        movdqa    %xmm0, -288(%rbp)                             #3464.17
..LN2969:
   .loc    3  3465
        movdqa    -288(%rbp), %xmm0                             #3465.22
        movdqa    -208(%rbp), %xmm1                             #3465.22
        pcmpeqd   %xmm1, %xmm0                                  #3465.22
..LN2971:
        movdqa    %xmm0, -288(%rbp)                             #3465.17
..LN2973:
   .loc    3  3466
        movdqa    -288(%rbp), %xmm0                             #3466.42
..LN2975:
        pmovmskb  %xmm0, %eax                                   #3466.22
..LN2977:
        movl      %eax, -56(%rbp)                               #3466.17
..LN2979:
   .loc    3  3467
        movl      -56(%rbp), %eax                               #3467.17
        incl      %eax                                          #3467.17
        sarl      $16, %eax                                     #3467.17
        negl      %eax                                          #3467.17
        movl      %eax, -56(%rbp)                               #3467.17
                                # LOE
..B22.16:                       # Preds ..B22.8 ..B22.15
..LN2981:
   .loc    3  3470
        movdqa    -256(%rbp), %xmm0                             #3470.38
..LN2983:
        movdqa    -160(%rbp), %xmm1                             #3470.46
..LN2985:
        pand      %xmm1, %xmm0                                  #3470.22
..LN2987:
        movdqa    %xmm0, -288(%rbp)                             #3470.17
..LN2989:
   .loc    3  3471
        movdqa    -288(%rbp), %xmm0                             #3471.22
        movdqa    -176(%rbp), %xmm1                             #3471.22
        pcmpeqd   %xmm1, %xmm0                                  #3471.22
..LN2991:
        movdqa    %xmm0, -288(%rbp)                             #3471.17
..LN2993:
   .loc    3  3472
        movdqa    -288(%rbp), %xmm0                             #3472.42
..LN2995:
        pmovmskb  %xmm0, %eax                                   #3472.22
..LN2997:
        movl      %eax, -60(%rbp)                               #3472.17
..LN2999:
   .loc    3  3473
        movl      -60(%rbp), %eax                               #3473.17
        incl      %eax                                          #3473.17
        sarl      $16, %eax                                     #3473.17
        negl      %eax                                          #3473.17
        movl      %eax, -60(%rbp)                               #3473.17
                                # LOE
..B22.17:                       # Preds ..B22.9 ..B22.16
..LN3001:
   .loc    3  3476
        movdqa    -256(%rbp), %xmm0                             #3476.38
..LN3003:
        movdqa    -128(%rbp), %xmm1                             #3476.46
..LN3005:
        pand      %xmm1, %xmm0                                  #3476.22
..LN3007:
        movdqa    %xmm0, -288(%rbp)                             #3476.17
..LN3009:
   .loc    3  3477
        movdqa    -288(%rbp), %xmm0                             #3477.22
        movdqa    -144(%rbp), %xmm1                             #3477.22
        pcmpeqd   %xmm1, %xmm0                                  #3477.22
..LN3011:
        movdqa    %xmm0, -288(%rbp)                             #3477.17
..LN3013:
   .loc    3  3478
        movdqa    -288(%rbp), %xmm0                             #3478.42
..LN3015:
        pmovmskb  %xmm0, %eax                                   #3478.22
..LN3017:
        movl      %eax, -92(%rbp)                               #3478.17
..LN3019:
   .loc    3  3479
        movl      -92(%rbp), %eax                               #3479.17
        incl      %eax                                          #3479.17
        sarl      $16, %eax                                     #3479.17
        negl      %eax                                          #3479.17
        movl      %eax, -92(%rbp)                               #3479.17
..LN3021:
   .loc    3  3483
        andl      $-4, -16(%rbp)                                #3483.17
..LN3023:
   .loc    3  3486
        movl      -56(%rbp), %eax                               #3486.29
        orl       -52(%rbp), %eax                               #3486.29
..LN3025:
        orl       -60(%rbp), %eax                               #3486.34
..LN3027:
        orl       -92(%rbp), %eax                               #3486.39
..LN3029:
        je        ..B22.29      # Prob 50%                      #3486.47
                                # LOE
..B22.18:                       # Preds ..B22.17
..LN3031:
   .loc    3  3489
        movl      -16(%rbp), %eax                               #3489.37
        negl      %eax                                          #3489.37
        addl      -64(%rbp), %eax                               #3489.37
        movl      %eax, -272(%rbp)                              #3489.37
        cmpl      $2, %eax                                      #3489.37
        je        ..B22.22      # Prob 50%                      #3489.37
                                # LOE
..B22.19:                       # Preds ..B22.18
        movl      -272(%rbp), %eax                              #3489.37
        cmpl      $1, %eax                                      #3489.37
        je        ..B22.24      # Prob 50%                      #3489.37
                                # LOE
..B22.20:                       # Preds ..B22.19
        movl      -272(%rbp), %eax                              #3489.37
        testl     %eax, %eax                                    #3489.37
        je        ..B22.26      # Prob 50%                      #3489.37
                                # LOE
..B22.21:                       # Preds ..B22.20
..LN3033:
   .loc    3  3492
        movl      $1, %eax                                      #3492.32
        leave                                                   #3492.32
..___tag_value_eval_2na_128.328:                                #
        ret                                                     #3492.32
..___tag_value_eval_2na_128.329:                                #
                                # LOE
..B22.22:                       # Preds ..B22.18
..LN3035:
   .loc    3  3494
        movl      -60(%rbp), %eax                               #3494.30
..LN3037:
        testl     %eax, %eax                                    #3494.36
        je        ..B22.24      # Prob 50%                      #3494.36
                                # LOE
..B22.23:                       # Preds ..B22.22
..LN3039:
        movl      $1, %eax                                      #3494.47
        leave                                                   #3494.47
..___tag_value_eval_2na_128.331:                                #
        ret                                                     #3494.47
..___tag_value_eval_2na_128.332:                                #
                                # LOE
..B22.24:                       # Preds ..B22.22 ..B22.19
..LN3041:
   .loc    3  3496
        movl      -56(%rbp), %eax                               #3496.30
..LN3043:
        testl     %eax, %eax                                    #3496.36
        je        ..B22.26      # Prob 50%                      #3496.36
                                # LOE
..B22.25:                       # Preds ..B22.24
..LN3045:
        movl      $1, %eax                                      #3496.47
        leave                                                   #3496.47
..___tag_value_eval_2na_128.334:                                #
        ret                                                     #3496.47
..___tag_value_eval_2na_128.335:                                #
                                # LOE
..B22.26:                       # Preds ..B22.24 ..B22.20
..LN3047:
   .loc    3  3498
        movl      -52(%rbp), %eax                               #3498.30
..LN3049:
        testl     %eax, %eax                                    #3498.36
        je        ..B22.28      # Prob 50%                      #3498.36
                                # LOE
..B22.27:                       # Preds ..B22.26
..LN3051:
        movl      $1, %eax                                      #3498.47
        leave                                                   #3498.47
..___tag_value_eval_2na_128.337:                                #
        ret                                                     #3498.47
..___tag_value_eval_2na_128.338:                                #
                                # LOE
..B22.28:                       # Preds ..B22.26
..LN3053:
   .loc    3  3500
        xorl      %eax, %eax                                    #3500.28
        leave                                                   #3500.28
..___tag_value_eval_2na_128.340:                                #
        ret                                                     #3500.28
..___tag_value_eval_2na_128.341:                                #
                                # LOE
..B22.29:                       # Preds ..B22.17
..LN3055:
   .loc    3  3527
        addl      $4, -16(%rbp)                                 #3527.17
..LN3057:
   .loc    3  3530
        movl      -16(%rbp), %eax                               #3530.22
..LN3059:
        movl      -64(%rbp), %edx                               #3530.28
        cmpl      %edx, %eax                                    #3530.28
        jbe       ..B22.31      # Prob 50%                      #3530.28
                                # LOE
..B22.30:                       # Preds ..B22.29
..LN3061:
   .loc    3  3531
        xorl      %eax, %eax                                    #3531.28
        leave                                                   #3531.28
..___tag_value_eval_2na_128.343:                                #
        ret                                                     #3531.28
..___tag_value_eval_2na_128.344:                                #
                                # LOE
..B22.31:                       # Preds ..B22.29
..LN3063:
   .loc    3  3534
        movl      -48(%rbp), %eax                               #3534.25
        decl      %eax                                          #3534.25
        movl      %eax, -48(%rbp)                               #3534.25
..LN3065:
        je        ..B22.36      # Prob 50%                      #3534.39
                                # LOE
..B22.32:                       # Preds ..B22.31
..LN3067:
   .loc    3  3539
        movdqa    -256(%rbp), %xmm0                             #3539.43
..LN3069:
        psrldq    $1, %xmm0                                     #3539.26
..LN3071:
        movdqa    %xmm0, -256(%rbp)                             #3539.17
..LN3073:
   .loc    3  3542
        movq      -72(%rbp), %rax                               #3542.22
..LN3075:
        movq      -80(%rbp), %rdx                               #3542.26
        cmpq      %rdx, %rax                                    #3542.26
        jae       ..B22.34      # Prob 50%                      #3542.26
                                # LOE
..B22.33:                       # Preds ..B22.32
..LN3077:
   .loc    3  3544
        sarl      $8, -96(%rbp)                                 #3544.21
..LN3079:
   .loc    3  3545
        movq      -72(%rbp), %rax                               #3545.37
        movzbl    (%rax), %eax                                  #3545.37
        movzbl    %al, %eax                                     #3545.37
..LN3081:
        shll      $8, %eax                                      #3545.48
..LN3083:
        orl       -96(%rbp), %eax                               #3545.21
        movl      %eax, -96(%rbp)                               #3545.21
..LN3085:
   .loc    3  3546
        movdqa    -256(%rbp), %xmm0                             #3546.49
..LN3087:
        movl      -96(%rbp), %eax                               #3546.57
..LN3089:
        pinsrw    $7, %eax, %xmm0                               #3546.30
..LN3091:
        movdqa    %xmm0, -256(%rbp)                             #3546.21
                                # LOE
..B22.34:                       # Preds ..B22.33 ..B22.32
..LN3093:
   .loc    3  3550
        incq      -72(%rbp)                                     #3550.20
        jmp       ..B22.12      # Prob 100%                     #3550.20
                                # LOE
..B22.36:                       # Preds ..B22.31 ..B22.12
..LN3095:
   .loc    3  3555
        movq      -88(%rbp), %rax                               #3555.18
..LN3097:
        movq      -80(%rbp), %rdx                               #3555.25
        cmpq      %rdx, %rax                                    #3555.25
        jae       ..B22.41      # Prob 50%                      #3555.25
                                # LOE
..B22.37:                       # Preds ..B22.36
..LN3099:
   .loc    3  3560
        movl      -16(%rbp), %eax                               #3560.18
..LN3101:
        movl      -64(%rbp), %edx                               #3560.24
        cmpl      %edx, %eax                                    #3560.24
        ja        ..B22.41      # Prob 50%                      #3560.24
                                # LOE
..B22.38:                       # Preds ..B22.37
..LN3103:
   .loc    3  3565
        movq      -88(%rbp), %rax                               #3565.41
..LN3105:
        movq      -80(%rbp), %rdx                               #3565.46
..LN3107:
        movq      %rax, %rdi                                    #3565.22
        movq      %rdx, %rsi                                    #3565.22
        call      prime_buffer_2na@PLT                          #3565.22
                                # LOE xmm0
..B22.45:                       # Preds ..B22.38
        movdqa    %xmm0, -304(%rbp)                             #3565.22
                                # LOE
..B22.39:                       # Preds ..B22.45
..LN3109:
        movdqa    -304(%rbp), %xmm0                             #3565.13
        movdqa    %xmm0, -256(%rbp)                             #3565.13
..LN3111:
   .loc    3  3643
        addq      $16, -88(%rbp)                                #3643.13
..LN3113:
   .loc    3  3645
        movq      -88(%rbp), %rax                               #3645.17
..LN3115:
        movq      %rax, -72(%rbp)                               #3645.13
..LN3117:
   .loc    3  3646
        movq      -88(%rbp), %rax                               #3646.18
..LN3119:
        movq      -80(%rbp), %rdx                               #3646.24
        cmpq      %rdx, %rax                                    #3646.24
        jae       ..B22.10      # Prob 50%                      #3646.24
                                # LOE
..B22.40:                       # Preds ..B22.39
..LN3121:
   .loc    3  3647
        movq      -88(%rbp), %rax                               #3647.32
        movzbl    -1(%rax), %eax                                #3647.32
        movzbl    %al, %eax                                     #3647.32
..LN3123:
        shll      $8, %eax                                      #3647.46
..LN3125:
        movl      %eax, -96(%rbp)                               #3647.17
        jmp       ..B22.10      # Prob 100%                     #3647.17
                                # LOE
..B22.41:                       # Preds ..B22.37 ..B22.36 ..B22.10
..LN3127:
   .loc    3  3656
        xorl      %eax, %eax                                    #3656.12
        leave                                                   #3656.12
..___tag_value_eval_2na_128.346:                                #
        ret                                                     #3656.12
        .align    2,0x90
..___tag_value_eval_2na_128.347:                                #
                                # LOE
# mark_end;
	.type	eval_2na_128,@function
	.size	eval_2na_128,.-eval_2na_128
.LNeval_2na_128:
	.data
# -- End  eval_2na_128
	.text
# -- Begin  eval_2na_pos
# mark_begin;
       .align    2,0x90
eval_2na_pos:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B23.1:                        # Preds ..B23.0
..___tag_value_eval_2na_pos.350:                                #
..LN3129:
   .loc    3  3662
        pushq     %rbp                                          #3662.1
        movq      %rsp, %rbp                                    #3662.1
..___tag_value_eval_2na_pos.351:                                #
        subq      $320, %rsp                                    #3662.1
        movq      %rdi, -48(%rbp)                               #3662.1
        movq      %rsi, -40(%rbp)                               #3662.1
        movl      %edx, -24(%rbp)                               #3662.1
        movl      %ecx, -16(%rbp)                               #3662.1
..LN3131:
   .loc    3  3698
        movq      -48(%rbp), %rax                               #3698.25
        movl      4(%rax), %eax                                 #3698.25
..LN3133:
        movl      %eax, -32(%rbp)                               #3698.23
..LN3135:
   .loc    3  3701
        movl      -16(%rbp), %eax                               #3701.5
        movl      -32(%rbp), %edx                               #3701.5
        cmpl      %edx, %eax                                    #3701.5
        jae       ..B23.3       # Prob 50%                      #3701.5
                                # LOE
..B23.2:                        # Preds ..B23.1
        lea       _2__STRING.9.0(%rip), %rax                    #3701.5
        lea       _2__STRING.1.0(%rip), %rdx                    #3701.5
        lea       __$Ua.0(%rip), %rcx                           #3701.5
        movq      %rax, %rdi                                    #3701.5
        movq      %rdx, %rsi                                    #3701.5
        movl      $3701, %edx                                   #3701.5
        call      __assert_fail@PLT                             #3701.5
                                # LOE
..B23.3:                        # Preds ..B23.1
..LN3137:
   .loc    3  3705
        movl      -24(%rbp), %eax                               #3705.12
..LN3139:
        addl      -16(%rbp), %eax                               #3705.5
        movl      %eax, -16(%rbp)                               #3705.5
..LN3141:
   .loc    3  3709
        movl      -24(%rbp), %eax                               #3709.13
..LN3143:
        movl      %eax, -80(%rbp)                               #3709.5
..LN3145:
   .loc    3  3713
        movl      -24(%rbp), %eax                               #3713.42
..LN3147:
        shrl      $2, %eax                                      #3713.49
..LN3149:
        addq      -40(%rbp), %rax                               #3713.30
..LN3151:
        movq      %rax, -104(%rbp)                              #3713.5
..LN3153:
   .loc    3  3716
        movl      -32(%rbp), %eax                               #3716.18
        negl      %eax                                          #3716.18
        addl      -16(%rbp), %eax                               #3716.18
..LN3155:
        movl      %eax, -76(%rbp)                               #3716.5
..LN3157:
   .loc    3  3719
        movl      -16(%rbp), %eax                               #3719.44
..LN3159:
        addl      $3, %eax                                      #3719.50
..LN3161:
        shrl      $2, %eax                                      #3719.57
..LN3163:
        addq      -40(%rbp), %rax                               #3719.30
..LN3165:
        movq      %rax, -96(%rbp)                               #3719.5
..LN3167:
   .loc    3  3722
        movq      -104(%rbp), %rax                              #3722.33
..LN3169:
        movq      -96(%rbp), %rdx                               #3722.38
..LN3171:
        movq      %rax, %rdi                                    #3722.14
        movq      %rdx, %rsi                                    #3722.14
        call      prime_buffer_2na@PLT                          #3722.14
                                # LOE xmm0
..B23.57:                       # Preds ..B23.3
        movdqa    %xmm0, -128(%rbp)                             #3722.14
                                # LOE
..B23.4:                        # Preds ..B23.57
..LN3173:
        movdqa    -128(%rbp), %xmm0                             #3722.5
        movdqa    %xmm0, -272(%rbp)                             #3722.5
..LN3175:
   .loc    3  3723
        addq      $16, -104(%rbp)                               #3723.5
..LN3177:
   .loc    3  3725
        movq      -104(%rbp), %rax                              #3725.9
..LN3179:
        movq      %rax, -88(%rbp)                               #3725.5
..LN3181:
   .loc    3  3728
        movq      -104(%rbp), %rax                              #3728.10
..LN3183:
        movq      -96(%rbp), %rdx                               #3728.16
        cmpq      %rdx, %rax                                    #3728.16
        jae       ..B23.6       # Prob 50%                      #3728.16
                                # LOE
..B23.5:                        # Preds ..B23.4
..LN3185:
   .loc    3  3729
        movq      -104(%rbp), %rax                              #3729.24
        movzbl    -1(%rax), %eax                                #3729.24
        movzbl    %al, %eax                                     #3729.24
..LN3187:
        shll      $8, %eax                                      #3729.38
..LN3189:
        movl      %eax, -112(%rbp)                              #3729.9
                                # LOE
..B23.6:                        # Preds ..B23.5 ..B23.4
..LN3191:
   .loc    3  3733
        movq      -48(%rbp), %rax                               #3733.5
        movdqa    16(%rax), %xmm0                               #3733.5
        movdqa    %xmm0, -256(%rbp)                             #3733.5
        movq      -48(%rbp), %rax                               #3733.5
        movdqa    32(%rax), %xmm0                               #3733.5
        movdqa    %xmm0, -240(%rbp)                             #3733.5
        movq      -48(%rbp), %rax                               #3733.5
        movdqa    48(%rax), %xmm0                               #3733.5
        movdqa    %xmm0, -224(%rbp)                             #3733.5
        movq      -48(%rbp), %rax                               #3733.5
        movdqa    64(%rax), %xmm0                               #3733.5
        movdqa    %xmm0, -208(%rbp)                             #3733.5
        movq      -48(%rbp), %rax                               #3733.5
        movdqa    80(%rax), %xmm0                               #3733.5
        movdqa    %xmm0, -192(%rbp)                             #3733.5
        movq      -48(%rbp), %rax                               #3733.5
        movdqa    96(%rax), %xmm0                               #3733.5
        movdqa    %xmm0, -176(%rbp)                             #3733.5
        movq      -48(%rbp), %rax                               #3733.5
        movdqa    112(%rax), %xmm0                              #3733.5
        movdqa    %xmm0, -160(%rbp)                             #3733.5
        movq      -48(%rbp), %rax                               #3733.5
        movdqa    128(%rax), %xmm0                              #3733.5
        movdqa    %xmm0, -144(%rbp)                             #3733.5
..LN3193:
   .loc    3  3736
        xorl      %eax, %eax                                    #3736.15
        movl      %eax, -72(%rbp)                               #3736.15
..LN3195:
        movl      %eax, -68(%rbp)                               #3736.10
..LN3197:
        movl      %eax, -64(%rbp)                               #3736.5
..LN3199:
   .loc    3  3738
        movl      -24(%rbp), %eax                               #3738.27
        negl      %eax                                          #3738.27
..LN3201:
        movl      -76(%rbp), %edx                               #3738.20
..LN3203:
        lea       7(%rdx,%rax), %eax                            #3738.33
..LN3205:
        shrl      $2, %eax                                      #3738.40
..LN3207:
        movl      %eax, -60(%rbp)                               #3738.5
..LN3209:
   .loc    3  3745
        movl      -24(%rbp), %eax                               #3745.14
..LN3211:
        andl      $3, %eax                                      #3745.20
        movl      %eax, -56(%rbp)                               #3745.20
        je        ..B23.14      # Prob 50%                      #3745.20
                                # LOE
..B23.7:                        # Preds ..B23.6
        movl      -56(%rbp), %eax                               #3745.20
        cmpl      $1, %eax                                      #3745.20
        je        ..B23.15      # Prob 50%                      #3745.20
                                # LOE
..B23.8:                        # Preds ..B23.7
        movl      -56(%rbp), %eax                               #3745.20
        cmpl      $2, %eax                                      #3745.20
        je        ..B23.16      # Prob 50%                      #3745.20
                                # LOE
..B23.9:                        # Preds ..B23.8
        movl      -56(%rbp), %eax                               #3745.20
        cmpl      $3, %eax                                      #3745.20
        je        ..B23.17      # Prob 50%                      #3745.20
                                # LOE
..B23.10:                       # Preds ..B23.53 ..B23.52 ..B23.9
..LN3213:
   .loc    3  3750
        movl      $1, %eax                                      #3750.9
        testl     %eax, %eax                                    #3750.9
        je        ..B23.54      # Prob 100%                     #3750.9
                                # LOE
..B23.11:                       # Preds ..B23.10
..LN3215:
   .loc    3  3752
        movl      $16, -60(%rbp)                                #3752.13
                                # LOE
..B23.12:                       # Preds ..B23.47 ..B23.11
..LN3217:
   .loc    3  3755
        movl      $1, %eax                                      #3755.13
        testl     %eax, %eax                                    #3755.13
        je        ..B23.49      # Prob 100%                     #3755.13
                                # LOE
..B23.14:                       # Preds ..B23.6 ..B23.12
..LN3219:
   .loc    3  3762
        movdqa    -272(%rbp), %xmm0                             #3762.38
..LN3221:
        movdqa    -240(%rbp), %xmm1                             #3762.46
..LN3223:
        pand      %xmm1, %xmm0                                  #3762.22
..LN3225:
        movdqa    %xmm0, -304(%rbp)                             #3762.17
..LN3227:
   .loc    3  3763
        movdqa    -304(%rbp), %xmm0                             #3763.22
        movdqa    -256(%rbp), %xmm1                             #3763.22
        pcmpeqd   %xmm1, %xmm0                                  #3763.22
..LN3229:
        movdqa    %xmm0, -304(%rbp)                             #3763.17
..LN3231:
   .loc    3  3764
        movdqa    -304(%rbp), %xmm0                             #3764.42
..LN3233:
        pmovmskb  %xmm0, %eax                                   #3764.22
..LN3235:
        movl      %eax, -64(%rbp)                               #3764.17
..LN3237:
   .loc    3  3765
        movl      -64(%rbp), %eax                               #3765.17
        incl      %eax                                          #3765.17
        sarl      $16, %eax                                     #3765.17
        negl      %eax                                          #3765.17
        movl      %eax, -64(%rbp)                               #3765.17
                                # LOE
..B23.15:                       # Preds ..B23.7 ..B23.14
..LN3239:
   .loc    3  3768
        movdqa    -272(%rbp), %xmm0                             #3768.38
..LN3241:
        movdqa    -208(%rbp), %xmm1                             #3768.46
..LN3243:
        pand      %xmm1, %xmm0                                  #3768.22
..LN3245:
        movdqa    %xmm0, -304(%rbp)                             #3768.17
..LN3247:
   .loc    3  3769
        movdqa    -304(%rbp), %xmm0                             #3769.22
        movdqa    -224(%rbp), %xmm1                             #3769.22
        pcmpeqd   %xmm1, %xmm0                                  #3769.22
..LN3249:
        movdqa    %xmm0, -304(%rbp)                             #3769.17
..LN3251:
   .loc    3  3770
        movdqa    -304(%rbp), %xmm0                             #3770.42
..LN3253:
        pmovmskb  %xmm0, %eax                                   #3770.22
..LN3255:
        movl      %eax, -68(%rbp)                               #3770.17
..LN3257:
   .loc    3  3771
        movl      -68(%rbp), %eax                               #3771.17
        incl      %eax                                          #3771.17
        sarl      $16, %eax                                     #3771.17
        negl      %eax                                          #3771.17
        movl      %eax, -68(%rbp)                               #3771.17
                                # LOE
..B23.16:                       # Preds ..B23.8 ..B23.15
..LN3259:
   .loc    3  3774
        movdqa    -272(%rbp), %xmm0                             #3774.38
..LN3261:
        movdqa    -176(%rbp), %xmm1                             #3774.46
..LN3263:
        pand      %xmm1, %xmm0                                  #3774.22
..LN3265:
        movdqa    %xmm0, -304(%rbp)                             #3774.17
..LN3267:
   .loc    3  3775
        movdqa    -304(%rbp), %xmm0                             #3775.22
        movdqa    -192(%rbp), %xmm1                             #3775.22
        pcmpeqd   %xmm1, %xmm0                                  #3775.22
..LN3269:
        movdqa    %xmm0, -304(%rbp)                             #3775.17
..LN3271:
   .loc    3  3776
        movdqa    -304(%rbp), %xmm0                             #3776.42
..LN3273:
        pmovmskb  %xmm0, %eax                                   #3776.22
..LN3275:
        movl      %eax, -72(%rbp)                               #3776.17
..LN3277:
   .loc    3  3777
        movl      -72(%rbp), %eax                               #3777.17
        incl      %eax                                          #3777.17
        sarl      $16, %eax                                     #3777.17
        negl      %eax                                          #3777.17
        movl      %eax, -72(%rbp)                               #3777.17
                                # LOE
..B23.17:                       # Preds ..B23.9 ..B23.16
..LN3279:
   .loc    3  3780
        movdqa    -272(%rbp), %xmm0                             #3780.38
..LN3281:
        movdqa    -144(%rbp), %xmm1                             #3780.46
..LN3283:
        pand      %xmm1, %xmm0                                  #3780.22
..LN3285:
        movdqa    %xmm0, -304(%rbp)                             #3780.17
..LN3287:
   .loc    3  3781
        movdqa    -304(%rbp), %xmm0                             #3781.22
        movdqa    -160(%rbp), %xmm1                             #3781.22
        pcmpeqd   %xmm1, %xmm0                                  #3781.22
..LN3289:
        movdqa    %xmm0, -304(%rbp)                             #3781.17
..LN3291:
   .loc    3  3782
        movdqa    -304(%rbp), %xmm0                             #3782.42
..LN3293:
        pmovmskb  %xmm0, %eax                                   #3782.22
..LN3295:
        movl      %eax, -108(%rbp)                              #3782.17
..LN3297:
   .loc    3  3783
        movl      -108(%rbp), %eax                              #3783.17
        incl      %eax                                          #3783.17
        sarl      $16, %eax                                     #3783.17
        negl      %eax                                          #3783.17
        movl      %eax, -108(%rbp)                              #3783.17
..LN3299:
   .loc    3  3787
        andl      $-4, -24(%rbp)                                #3787.17
..LN3301:
   .loc    3  3790
        movl      -68(%rbp), %eax                               #3790.29
        orl       -64(%rbp), %eax                               #3790.29
..LN3303:
        orl       -72(%rbp), %eax                               #3790.34
..LN3305:
        orl       -108(%rbp), %eax                              #3790.39
..LN3307:
        je        ..B23.42      # Prob 50%                      #3790.47
                                # LOE
..B23.18:                       # Preds ..B23.17
..LN3309:
   .loc    3  3793
        movl      -24(%rbp), %eax                               #3793.37
        negl      %eax                                          #3793.37
        addl      -76(%rbp), %eax                               #3793.37
        movl      %eax, -288(%rbp)                              #3793.37
        je        ..B23.21      # Prob 50%                      #3793.37
                                # LOE
..B23.19:                       # Preds ..B23.18
        movl      -288(%rbp), %eax                              #3793.37
        cmpl      $1, %eax                                      #3793.37
        je        ..B23.23      # Prob 50%                      #3793.37
                                # LOE
..B23.20:                       # Preds ..B23.19
        movl      -288(%rbp), %eax                              #3793.37
        cmpl      $2, %eax                                      #3793.37
        je        ..B23.27      # Prob 50%                      #3793.37
        jmp       ..B23.33      # Prob 100%                     #3793.37
                                # LOE
..B23.21:                       # Preds ..B23.18
..LN3311:
   .loc    3  3796
        movl      -64(%rbp), %eax                               #3796.30
..LN3313:
        testl     %eax, %eax                                    #3796.36
        je        ..B23.41      # Prob 50%                      #3796.36
                                # LOE
..B23.22:                       # Preds ..B23.21
..LN3315:
        movl      -80(%rbp), %eax                               #3796.53
        negl      %eax                                          #3796.53
..LN3317:
        movl      -24(%rbp), %edx                               #3796.47
..LN3319:
        lea       1(%rdx,%rax), %eax                            #3796.61
        leave                                                   #3796.61
..___tag_value_eval_2na_pos.355:                                #
        ret                                                     #3796.61
..___tag_value_eval_2na_pos.356:                                #
                                # LOE
..B23.23:                       # Preds ..B23.19
..LN3321:
   .loc    3  3799
        movl      -64(%rbp), %eax                               #3799.30
..LN3323:
        testl     %eax, %eax                                    #3799.36
        je        ..B23.25      # Prob 50%                      #3799.36
                                # LOE
..B23.24:                       # Preds ..B23.23
..LN3325:
        movl      -80(%rbp), %eax                               #3799.53
        negl      %eax                                          #3799.53
..LN3327:
        movl      -24(%rbp), %edx                               #3799.47
..LN3329:
        lea       1(%rdx,%rax), %eax                            #3799.61
        leave                                                   #3799.61
..___tag_value_eval_2na_pos.358:                                #
        ret                                                     #3799.61
..___tag_value_eval_2na_pos.359:                                #
                                # LOE
..B23.25:                       # Preds ..B23.23
..LN3331:
   .loc    3  3800
        movl      -68(%rbp), %eax                               #3800.30
..LN3333:
        testl     %eax, %eax                                    #3800.36
        je        ..B23.41      # Prob 50%                      #3800.36
                                # LOE
..B23.26:                       # Preds ..B23.25
..LN3335:
        movl      -80(%rbp), %eax                               #3800.53
        negl      %eax                                          #3800.53
..LN3337:
        movl      -24(%rbp), %edx                               #3800.47
..LN3339:
        lea       2(%rdx,%rax), %eax                            #3800.61
        leave                                                   #3800.61
..___tag_value_eval_2na_pos.361:                                #
        ret                                                     #3800.61
..___tag_value_eval_2na_pos.362:                                #
                                # LOE
..B23.27:                       # Preds ..B23.20
..LN3341:
   .loc    3  3803
        movl      -64(%rbp), %eax                               #3803.30
..LN3343:
        testl     %eax, %eax                                    #3803.36
        je        ..B23.29      # Prob 50%                      #3803.36
                                # LOE
..B23.28:                       # Preds ..B23.27
..LN3345:
        movl      -80(%rbp), %eax                               #3803.53
        negl      %eax                                          #3803.53
..LN3347:
        movl      -24(%rbp), %edx                               #3803.47
..LN3349:
        lea       1(%rdx,%rax), %eax                            #3803.61
        leave                                                   #3803.61
..___tag_value_eval_2na_pos.364:                                #
        ret                                                     #3803.61
..___tag_value_eval_2na_pos.365:                                #
                                # LOE
..B23.29:                       # Preds ..B23.27
..LN3351:
   .loc    3  3804
        movl      -68(%rbp), %eax                               #3804.30
..LN3353:
        testl     %eax, %eax                                    #3804.36
        je        ..B23.31      # Prob 50%                      #3804.36
                                # LOE
..B23.30:                       # Preds ..B23.29
..LN3355:
        movl      -80(%rbp), %eax                               #3804.53
        negl      %eax                                          #3804.53
..LN3357:
        movl      -24(%rbp), %edx                               #3804.47
..LN3359:
        lea       2(%rdx,%rax), %eax                            #3804.61
        leave                                                   #3804.61
..___tag_value_eval_2na_pos.367:                                #
        ret                                                     #3804.61
..___tag_value_eval_2na_pos.368:                                #
                                # LOE
..B23.31:                       # Preds ..B23.29
..LN3361:
   .loc    3  3805
        movl      -72(%rbp), %eax                               #3805.30
..LN3363:
        testl     %eax, %eax                                    #3805.36
        je        ..B23.41      # Prob 50%                      #3805.36
                                # LOE
..B23.32:                       # Preds ..B23.31
..LN3365:
        movl      -80(%rbp), %eax                               #3805.53
        negl      %eax                                          #3805.53
..LN3367:
        movl      -24(%rbp), %edx                               #3805.47
..LN3369:
        lea       3(%rdx,%rax), %eax                            #3805.61
        leave                                                   #3805.61
..___tag_value_eval_2na_pos.370:                                #
        ret                                                     #3805.61
..___tag_value_eval_2na_pos.371:                                #
                                # LOE
..B23.33:                       # Preds ..B23.20
..LN3371:
   .loc    3  3808
        movl      -64(%rbp), %eax                               #3808.30
..LN3373:
        testl     %eax, %eax                                    #3808.36
        je        ..B23.35      # Prob 50%                      #3808.36
                                # LOE
..B23.34:                       # Preds ..B23.33
..LN3375:
        movl      -80(%rbp), %eax                               #3808.53
        negl      %eax                                          #3808.53
..LN3377:
        movl      -24(%rbp), %edx                               #3808.47
..LN3379:
        lea       1(%rdx,%rax), %eax                            #3808.61
        leave                                                   #3808.61
..___tag_value_eval_2na_pos.373:                                #
        ret                                                     #3808.61
..___tag_value_eval_2na_pos.374:                                #
                                # LOE
..B23.35:                       # Preds ..B23.33
..LN3381:
   .loc    3  3809
        movl      -68(%rbp), %eax                               #3809.30
..LN3383:
        testl     %eax, %eax                                    #3809.36
        je        ..B23.37      # Prob 50%                      #3809.36
                                # LOE
..B23.36:                       # Preds ..B23.35
..LN3385:
        movl      -80(%rbp), %eax                               #3809.53
        negl      %eax                                          #3809.53
..LN3387:
        movl      -24(%rbp), %edx                               #3809.47
..LN3389:
        lea       2(%rdx,%rax), %eax                            #3809.61
        leave                                                   #3809.61
..___tag_value_eval_2na_pos.376:                                #
        ret                                                     #3809.61
..___tag_value_eval_2na_pos.377:                                #
                                # LOE
..B23.37:                       # Preds ..B23.35
..LN3391:
   .loc    3  3810
        movl      -72(%rbp), %eax                               #3810.30
..LN3393:
        testl     %eax, %eax                                    #3810.36
        je        ..B23.39      # Prob 50%                      #3810.36
                                # LOE
..B23.38:                       # Preds ..B23.37
..LN3395:
        movl      -80(%rbp), %eax                               #3810.53
        negl      %eax                                          #3810.53
..LN3397:
        movl      -24(%rbp), %edx                               #3810.47
..LN3399:
        lea       3(%rdx,%rax), %eax                            #3810.61
        leave                                                   #3810.61
..___tag_value_eval_2na_pos.379:                                #
        ret                                                     #3810.61
..___tag_value_eval_2na_pos.380:                                #
                                # LOE
..B23.39:                       # Preds ..B23.37
..LN3401:
   .loc    3  3811
        movl      -108(%rbp), %eax                              #3811.30
..LN3403:
        testl     %eax, %eax                                    #3811.36
        je        ..B23.41      # Prob 50%                      #3811.36
                                # LOE
..B23.40:                       # Preds ..B23.39
..LN3405:
        movl      -80(%rbp), %eax                               #3811.53
        negl      %eax                                          #3811.53
..LN3407:
        movl      -24(%rbp), %edx                               #3811.47
..LN3409:
        lea       4(%rdx,%rax), %eax                            #3811.61
        leave                                                   #3811.61
..___tag_value_eval_2na_pos.382:                                #
        ret                                                     #3811.61
..___tag_value_eval_2na_pos.383:                                #
                                # LOE
..B23.41:                       # Preds ..B23.21 ..B23.25 ..B23.31 ..B23.39
..LN3411:
   .loc    3  3813
        xorl      %eax, %eax                                    #3813.28
        leave                                                   #3813.28
..___tag_value_eval_2na_pos.385:                                #
        ret                                                     #3813.28
..___tag_value_eval_2na_pos.386:                                #
                                # LOE
..B23.42:                       # Preds ..B23.17
..LN3413:
   .loc    3  3853
        addl      $4, -24(%rbp)                                 #3853.17
..LN3415:
   .loc    3  3856
        movl      -24(%rbp), %eax                               #3856.22
..LN3417:
        movl      -76(%rbp), %edx                               #3856.28
        cmpl      %edx, %eax                                    #3856.28
        jbe       ..B23.44      # Prob 50%                      #3856.28
                                # LOE
..B23.43:                       # Preds ..B23.42
..LN3419:
   .loc    3  3857
        xorl      %eax, %eax                                    #3857.28
        leave                                                   #3857.28
..___tag_value_eval_2na_pos.388:                                #
        ret                                                     #3857.28
..___tag_value_eval_2na_pos.389:                                #
                                # LOE
..B23.44:                       # Preds ..B23.42
..LN3421:
   .loc    3  3860
        movl      -60(%rbp), %eax                               #3860.25
        decl      %eax                                          #3860.25
        movl      %eax, -60(%rbp)                               #3860.25
..LN3423:
        je        ..B23.49      # Prob 50%                      #3860.39
                                # LOE
..B23.45:                       # Preds ..B23.44
..LN3425:
   .loc    3  3865
        movdqa    -272(%rbp), %xmm0                             #3865.43
..LN3427:
        psrldq    $1, %xmm0                                     #3865.26
..LN3429:
        movdqa    %xmm0, -272(%rbp)                             #3865.17
..LN3431:
   .loc    3  3868
        movq      -88(%rbp), %rax                               #3868.22
..LN3433:
        movq      -96(%rbp), %rdx                               #3868.26
        cmpq      %rdx, %rax                                    #3868.26
        jae       ..B23.47      # Prob 50%                      #3868.26
                                # LOE
..B23.46:                       # Preds ..B23.45
..LN3435:
   .loc    3  3870
        sarl      $8, -112(%rbp)                                #3870.21
..LN3437:
   .loc    3  3871
        movq      -88(%rbp), %rax                               #3871.37
        movzbl    (%rax), %eax                                  #3871.37
        movzbl    %al, %eax                                     #3871.37
..LN3439:
        shll      $8, %eax                                      #3871.48
..LN3441:
        orl       -112(%rbp), %eax                              #3871.21
        movl      %eax, -112(%rbp)                              #3871.21
..LN3443:
   .loc    3  3872
        movdqa    -272(%rbp), %xmm0                             #3872.49
..LN3445:
        movl      -112(%rbp), %eax                              #3872.57
..LN3447:
        pinsrw    $7, %eax, %xmm0                               #3872.30
..LN3449:
        movdqa    %xmm0, -272(%rbp)                             #3872.21
                                # LOE
..B23.47:                       # Preds ..B23.46 ..B23.45
..LN3451:
   .loc    3  3876
        incq      -88(%rbp)                                     #3876.20
        jmp       ..B23.12      # Prob 100%                     #3876.20
                                # LOE
..B23.49:                       # Preds ..B23.44 ..B23.12
..LN3453:
   .loc    3  3881
        movq      -104(%rbp), %rax                              #3881.18
..LN3455:
        movq      -96(%rbp), %rdx                               #3881.25
        cmpq      %rdx, %rax                                    #3881.25
        jae       ..B23.54      # Prob 50%                      #3881.25
                                # LOE
..B23.50:                       # Preds ..B23.49
..LN3457:
   .loc    3  3886
        movl      -24(%rbp), %eax                               #3886.18
..LN3459:
        movl      -76(%rbp), %edx                               #3886.24
        cmpl      %edx, %eax                                    #3886.24
        ja        ..B23.54      # Prob 50%                      #3886.24
                                # LOE
..B23.51:                       # Preds ..B23.50
..LN3461:
   .loc    3  3891
        movq      -104(%rbp), %rax                              #3891.41
..LN3463:
        movq      -96(%rbp), %rdx                               #3891.46
..LN3465:
        movq      %rax, %rdi                                    #3891.22
        movq      %rdx, %rsi                                    #3891.22
        call      prime_buffer_2na@PLT                          #3891.22
                                # LOE xmm0
..B23.58:                       # Preds ..B23.51
        movdqa    %xmm0, -320(%rbp)                             #3891.22
                                # LOE
..B23.52:                       # Preds ..B23.58
..LN3467:
        movdqa    -320(%rbp), %xmm0                             #3891.13
        movdqa    %xmm0, -272(%rbp)                             #3891.13
..LN3469:
   .loc    3  3969
        addq      $16, -104(%rbp)                               #3969.13
..LN3471:
   .loc    3  3971
        movq      -104(%rbp), %rax                              #3971.17
..LN3473:
        movq      %rax, -88(%rbp)                               #3971.13
..LN3475:
   .loc    3  3972
        movq      -104(%rbp), %rax                              #3972.18
..LN3477:
        movq      -96(%rbp), %rdx                               #3972.24
        cmpq      %rdx, %rax                                    #3972.24
        jae       ..B23.10      # Prob 50%                      #3972.24
                                # LOE
..B23.53:                       # Preds ..B23.52
..LN3479:
   .loc    3  3973
        movq      -104(%rbp), %rax                              #3973.32
        movzbl    -1(%rax), %eax                                #3973.32
        movzbl    %al, %eax                                     #3973.32
..LN3481:
        shll      $8, %eax                                      #3973.46
..LN3483:
        movl      %eax, -112(%rbp)                              #3973.17
        jmp       ..B23.10      # Prob 100%                     #3973.17
                                # LOE
..B23.54:                       # Preds ..B23.50 ..B23.49 ..B23.10
..LN3485:
   .loc    3  3983
        xorl      %eax, %eax                                    #3983.12
        leave                                                   #3983.12
..___tag_value_eval_2na_pos.391:                                #
        ret                                                     #3983.12
        .align    2,0x90
..___tag_value_eval_2na_pos.392:                                #
                                # LOE
# mark_end;
	.type	eval_2na_pos,@function
	.size	eval_2na_pos,.-eval_2na_pos
.LNeval_2na_pos:
	.data
# -- End  eval_2na_pos
	.text
# -- Begin  prime_buffer_4na
# mark_begin;
       .align    2,0x90
prime_buffer_4na:
# parameter 1(src): %rdi
# parameter 2(ignore): %rsi
..B24.1:                        # Preds ..B24.0
..___tag_value_prime_buffer_4na.395:                            #
..LN3487:
   .loc    3  3990
        pushq     %rbp                                          #3990.1
        movq      %rsp, %rbp                                    #3990.1
..___tag_value_prime_buffer_4na.396:                            #
        subq      $48, %rsp                                     #3990.1
        movq      %rdi, -16(%rbp)                               #3990.1
        movq      %rsi, -8(%rbp)                                #3990.1
..LN3489:
   .loc    3  3996
        movq      -16(%rbp), %rax                               #3996.34
        movzbl    (%rax), %eax                                  #3996.34
        movzbl    %al, %eax                                     #3996.34
..LN3491:
        movslq    %eax, %rax                                    #3996.21
        lea       expand_2na.0(%rip), %rdx                      #3996.21
        movzwl    (%rdx,%rax,2), %eax                           #3996.21
..LN3493:
        movw      %ax, -48(%rbp)                                #3996.5
..LN3495:
   .loc    3  3997
        movq      -16(%rbp), %rax                               #3997.34
        movzbl    1(%rax), %eax                                 #3997.34
        movzbl    %al, %eax                                     #3997.34
..LN3497:
        movslq    %eax, %rax                                    #3997.21
        lea       expand_2na.0(%rip), %rdx                      #3997.21
        movzwl    (%rdx,%rax,2), %eax                           #3997.21
..LN3499:
        movw      %ax, -46(%rbp)                                #3997.5
..LN3501:
   .loc    3  3998
        movq      -16(%rbp), %rax                               #3998.34
        movzbl    2(%rax), %eax                                 #3998.34
        movzbl    %al, %eax                                     #3998.34
..LN3503:
        movslq    %eax, %rax                                    #3998.21
        lea       expand_2na.0(%rip), %rdx                      #3998.21
        movzwl    (%rdx,%rax,2), %eax                           #3998.21
..LN3505:
        movw      %ax, -44(%rbp)                                #3998.5
..LN3507:
   .loc    3  3999
        movq      -16(%rbp), %rax                               #3999.34
        movzbl    3(%rax), %eax                                 #3999.34
        movzbl    %al, %eax                                     #3999.34
..LN3509:
        movslq    %eax, %rax                                    #3999.21
        lea       expand_2na.0(%rip), %rdx                      #3999.21
        movzwl    (%rdx,%rax,2), %eax                           #3999.21
..LN3511:
        movw      %ax, -42(%rbp)                                #3999.5
..LN3513:
   .loc    3  4000
        movq      -16(%rbp), %rax                               #4000.34
        movzbl    4(%rax), %eax                                 #4000.34
        movzbl    %al, %eax                                     #4000.34
..LN3515:
        movslq    %eax, %rax                                    #4000.21
        lea       expand_2na.0(%rip), %rdx                      #4000.21
        movzwl    (%rdx,%rax,2), %eax                           #4000.21
..LN3517:
        movw      %ax, -40(%rbp)                                #4000.5
..LN3519:
   .loc    3  4001
        movq      -16(%rbp), %rax                               #4001.34
        movzbl    5(%rax), %eax                                 #4001.34
        movzbl    %al, %eax                                     #4001.34
..LN3521:
        movslq    %eax, %rax                                    #4001.21
        lea       expand_2na.0(%rip), %rdx                      #4001.21
        movzwl    (%rdx,%rax,2), %eax                           #4001.21
..LN3523:
        movw      %ax, -38(%rbp)                                #4001.5
..LN3525:
   .loc    3  4002
        movq      -16(%rbp), %rax                               #4002.34
        movzbl    6(%rax), %eax                                 #4002.34
        movzbl    %al, %eax                                     #4002.34
..LN3527:
        movslq    %eax, %rax                                    #4002.21
        lea       expand_2na.0(%rip), %rdx                      #4002.21
        movzwl    (%rdx,%rax,2), %eax                           #4002.21
..LN3529:
        movw      %ax, -36(%rbp)                                #4002.5
..LN3531:
   .loc    3  4003
        movq      -16(%rbp), %rax                               #4003.34
        movzbl    7(%rax), %eax                                 #4003.34
        movzbl    %al, %eax                                     #4003.34
..LN3533:
        movslq    %eax, %rax                                    #4003.21
        lea       expand_2na.0(%rip), %rdx                      #4003.21
        movzwl    (%rdx,%rax,2), %eax                           #4003.21
..LN3535:
        movw      %ax, -34(%rbp)                                #4003.5
..LN3537:
   .loc    3  4005
        movdqu    -48(%rbp), %xmm0                              #4005.51
..LN3539:
        movdqa    %xmm0, -32(%rbp)                              #4005.5
..LN3541:
   .loc    3  4006
        movdqa    -32(%rbp), %xmm0                              #4006.12
        leave                                                   #4006.12
..___tag_value_prime_buffer_4na.400:                            #
        ret                                                     #4006.12
        .align    2,0x90
..___tag_value_prime_buffer_4na.401:                            #
                                # LOE
# mark_end;
	.type	prime_buffer_4na,@function
	.size	prime_buffer_4na,.-prime_buffer_4na
.LNprime_buffer_4na:
	.data
# -- End  prime_buffer_4na
	.text
# -- Begin  eval_4na_16
# mark_begin;
       .align    2,0x90
eval_4na_16:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B25.1:                        # Preds ..B25.0
..___tag_value_eval_4na_16.404:                                 #
..LN3543:
   .loc    3  4057
        pushq     %rbp                                          #4057.1
        movq      %rsp, %rbp                                    #4057.1
..___tag_value_eval_4na_16.405:                                 #
        subq      $336, %rsp                                    #4057.1
        movq      %rdi, -48(%rbp)                               #4057.1
        movq      %rsi, -40(%rbp)                               #4057.1
        movl      %edx, -24(%rbp)                               #4057.1
        movl      %ecx, -16(%rbp)                               #4057.1
..LN3545:
   .loc    3  4083
        movq      -48(%rbp), %rax                               #4083.25
        movl      4(%rax), %eax                                 #4083.25
..LN3547:
        movl      %eax, -32(%rbp)                               #4083.23
..LN3549:
   .loc    3  4086
        movl      -16(%rbp), %eax                               #4086.5
        movl      -32(%rbp), %edx                               #4086.5
        cmpl      %edx, %eax                                    #4086.5
        jae       ..B25.3       # Prob 50%                      #4086.5
                                # LOE
..B25.2:                        # Preds ..B25.1
        lea       _2__STRING.9.0(%rip), %rax                    #4086.5
        lea       _2__STRING.1.0(%rip), %rdx                    #4086.5
        lea       __$Ub.0(%rip), %rcx                           #4086.5
        movq      %rax, %rdi                                    #4086.5
        movq      %rdx, %rsi                                    #4086.5
        movl      $4086, %edx                                   #4086.5
        call      __assert_fail@PLT                             #4086.5
                                # LOE
..B25.3:                        # Preds ..B25.1
..LN3551:
   .loc    3  4090
        movl      -24(%rbp), %eax                               #4090.12
..LN3553:
        addl      -16(%rbp), %eax                               #4090.5
        movl      %eax, -16(%rbp)                               #4090.5
..LN3555:
   .loc    3  4093
        movl      -24(%rbp), %eax                               #4093.42
..LN3557:
        shrl      $2, %eax                                      #4093.49
..LN3559:
        addq      -40(%rbp), %rax                               #4093.30
..LN3561:
        movq      %rax, -96(%rbp)                               #4093.5
..LN3563:
   .loc    3  4096
        movl      -32(%rbp), %eax                               #4096.18
        negl      %eax                                          #4096.18
        addl      -16(%rbp), %eax                               #4096.18
..LN3565:
        movl      %eax, -76(%rbp)                               #4096.5
..LN3567:
   .loc    3  4099
        movl      -16(%rbp), %eax                               #4099.44
..LN3569:
        addl      $3, %eax                                      #4099.50
..LN3571:
        shrl      $2, %eax                                      #4099.57
..LN3573:
        addq      -40(%rbp), %rax                               #4099.30
..LN3575:
        movq      %rax, -88(%rbp)                               #4099.5
..LN3577:
   .loc    3  4102
        movq      -96(%rbp), %rax                               #4102.33
..LN3579:
        movq      -88(%rbp), %rdx                               #4102.38
..LN3581:
        movq      %rax, %rdi                                    #4102.14
        movq      %rdx, %rsi                                    #4102.14
        call      prime_buffer_4na@PLT                          #4102.14
                                # LOE xmm0
..B25.43:                       # Preds ..B25.3
        movdqa    %xmm0, -144(%rbp)                             #4102.14
                                # LOE
..B25.4:                        # Preds ..B25.43
..LN3583:
        movdqa    -144(%rbp), %xmm0                             #4102.5
        movdqa    %xmm0, -288(%rbp)                             #4102.5
..LN3585:
   .loc    3  4103
        addq      $8, -96(%rbp)                                 #4103.5
..LN3587:
   .loc    3  4109
        movq      -48(%rbp), %rax                               #4109.5
        movdqa    16(%rax), %xmm0                               #4109.5
        movdqa    %xmm0, -272(%rbp)                             #4109.5
        movq      -48(%rbp), %rax                               #4109.5
        movdqa    32(%rax), %xmm0                               #4109.5
        movdqa    %xmm0, -256(%rbp)                             #4109.5
        movq      -48(%rbp), %rax                               #4109.5
        movdqa    48(%rax), %xmm0                               #4109.5
        movdqa    %xmm0, -240(%rbp)                             #4109.5
        movq      -48(%rbp), %rax                               #4109.5
        movdqa    64(%rax), %xmm0                               #4109.5
        movdqa    %xmm0, -224(%rbp)                             #4109.5
        movq      -48(%rbp), %rax                               #4109.5
        movdqa    80(%rax), %xmm0                               #4109.5
        movdqa    %xmm0, -208(%rbp)                             #4109.5
        movq      -48(%rbp), %rax                               #4109.5
        movdqa    96(%rax), %xmm0                               #4109.5
        movdqa    %xmm0, -192(%rbp)                             #4109.5
        movq      -48(%rbp), %rax                               #4109.5
        movdqa    112(%rax), %xmm0                              #4109.5
        movdqa    %xmm0, -176(%rbp)                             #4109.5
        movq      -48(%rbp), %rax                               #4109.5
        movdqa    128(%rax), %xmm0                              #4109.5
        movdqa    %xmm0, -160(%rbp)                             #4109.5
..LN3589:
   .loc    3  4112
        xorl      %eax, %eax                                    #4112.15
        movl      %eax, -72(%rbp)                               #4112.15
..LN3591:
        movl      %eax, -68(%rbp)                               #4112.10
..LN3593:
        movl      %eax, -64(%rbp)                               #4112.5
..LN3595:
   .loc    3  4118
        movl      $1, -60(%rbp)                                 #4118.5
..LN3597:
   .loc    3  4123
        movl      -24(%rbp), %eax                               #4123.14
..LN3599:
        andl      $3, %eax                                      #4123.20
        movl      %eax, -56(%rbp)                               #4123.20
        je        ..B25.12      # Prob 50%                      #4123.20
                                # LOE
..B25.5:                        # Preds ..B25.4
        movl      -56(%rbp), %eax                               #4123.20
        cmpl      $1, %eax                                      #4123.20
        je        ..B25.13      # Prob 50%                      #4123.20
                                # LOE
..B25.6:                        # Preds ..B25.5
        movl      -56(%rbp), %eax                               #4123.20
        cmpl      $2, %eax                                      #4123.20
        je        ..B25.14      # Prob 50%                      #4123.20
                                # LOE
..B25.7:                        # Preds ..B25.6
        movl      -56(%rbp), %eax                               #4123.20
        cmpl      $3, %eax                                      #4123.20
        je        ..B25.15      # Prob 50%                      #4123.20
                                # LOE
..B25.8:                        # Preds ..B25.39 ..B25.7
..LN3601:
   .loc    3  4128
        movl      $1, %eax                                      #4128.9
        testl     %eax, %eax                                    #4128.9
        je        ..B25.40      # Prob 100%                     #4128.9
                                # LOE
..B25.9:                        # Preds ..B25.8
..LN3603:
   .loc    3  4130
        movl      $1, -60(%rbp)                                 #4130.13
                                # LOE
..B25.10:                       # Preds ..B25.34 ..B25.9
..LN3605:
   .loc    3  4133
        movl      $1, %eax                                      #4133.13
        testl     %eax, %eax                                    #4133.13
        je        ..B25.36      # Prob 100%                     #4133.13
                                # LOE
..B25.12:                       # Preds ..B25.4 ..B25.10
..LN3607:
   .loc    3  4140
        movdqa    -288(%rbp), %xmm0                             #4140.38
..LN3609:
        movdqa    -272(%rbp), %xmm1                             #4140.46
..LN3611:
        pand      %xmm1, %xmm0                                  #4140.22
..LN3613:
        movdqa    %xmm0, -320(%rbp)                             #4140.17
..LN3615:
   .loc    3  4141
        movdqa    -288(%rbp), %xmm0                             #4141.38
..LN3617:
        movdqa    -256(%rbp), %xmm1                             #4141.46
..LN3619:
        pand      %xmm1, %xmm0                                  #4141.22
..LN3621:
        movdqa    %xmm0, -304(%rbp)                             #4141.17
..LN3623:
   .loc    3  4142
        movdqa    -320(%rbp), %xmm0                             #4142.22
        movdqa    -304(%rbp), %xmm1                             #4142.22
        pcmpeqw   %xmm1, %xmm0                                  #4142.22
..LN3625:
        movdqa    %xmm0, -320(%rbp)                             #4142.17
..LN3627:
   .loc    3  4143
        movdqa    -320(%rbp), %xmm0                             #4143.42
..LN3629:
        pmovmskb  %xmm0, %eax                                   #4143.22
..LN3631:
        movl      %eax, -64(%rbp)                               #4143.17
                                # LOE
..B25.13:                       # Preds ..B25.5 ..B25.12
..LN3633:
   .loc    3  4147
        movdqa    -288(%rbp), %xmm0                             #4147.38
..LN3635:
        movdqa    -240(%rbp), %xmm1                             #4147.46
..LN3637:
        pand      %xmm1, %xmm0                                  #4147.22
..LN3639:
        movdqa    %xmm0, -320(%rbp)                             #4147.17
..LN3641:
   .loc    3  4148
        movdqa    -288(%rbp), %xmm0                             #4148.38
..LN3643:
        movdqa    -224(%rbp), %xmm1                             #4148.46
..LN3645:
        pand      %xmm1, %xmm0                                  #4148.22
..LN3647:
        movdqa    %xmm0, -304(%rbp)                             #4148.17
..LN3649:
   .loc    3  4149
        movdqa    -320(%rbp), %xmm0                             #4149.22
        movdqa    -304(%rbp), %xmm1                             #4149.22
        pcmpeqw   %xmm1, %xmm0                                  #4149.22
..LN3651:
        movdqa    %xmm0, -320(%rbp)                             #4149.17
..LN3653:
   .loc    3  4150
        movdqa    -320(%rbp), %xmm0                             #4150.42
..LN3655:
        pmovmskb  %xmm0, %eax                                   #4150.22
..LN3657:
        movl      %eax, -68(%rbp)                               #4150.17
                                # LOE
..B25.14:                       # Preds ..B25.6 ..B25.13
..LN3659:
   .loc    3  4154
        movdqa    -288(%rbp), %xmm0                             #4154.38
..LN3661:
        movdqa    -208(%rbp), %xmm1                             #4154.46
..LN3663:
        pand      %xmm1, %xmm0                                  #4154.22
..LN3665:
        movdqa    %xmm0, -320(%rbp)                             #4154.17
..LN3667:
   .loc    3  4155
        movdqa    -288(%rbp), %xmm0                             #4155.38
..LN3669:
        movdqa    -192(%rbp), %xmm1                             #4155.46
..LN3671:
        pand      %xmm1, %xmm0                                  #4155.22
..LN3673:
        movdqa    %xmm0, -304(%rbp)                             #4155.17
..LN3675:
   .loc    3  4156
        movdqa    -320(%rbp), %xmm0                             #4156.22
        movdqa    -304(%rbp), %xmm1                             #4156.22
        pcmpeqw   %xmm1, %xmm0                                  #4156.22
..LN3677:
        movdqa    %xmm0, -320(%rbp)                             #4156.17
..LN3679:
   .loc    3  4157
        movdqa    -320(%rbp), %xmm0                             #4157.42
..LN3681:
        pmovmskb  %xmm0, %eax                                   #4157.22
..LN3683:
        movl      %eax, -72(%rbp)                               #4157.17
                                # LOE
..B25.15:                       # Preds ..B25.7 ..B25.14
..LN3685:
   .loc    3  4161
        movdqa    -288(%rbp), %xmm0                             #4161.38
..LN3687:
        movdqa    -176(%rbp), %xmm1                             #4161.46
..LN3689:
        pand      %xmm1, %xmm0                                  #4161.22
..LN3691:
        movdqa    %xmm0, -320(%rbp)                             #4161.17
..LN3693:
   .loc    3  4162
        movdqa    -288(%rbp), %xmm0                             #4162.38
..LN3695:
        movdqa    -160(%rbp), %xmm1                             #4162.46
..LN3697:
        pand      %xmm1, %xmm0                                  #4162.22
..LN3699:
        movdqa    %xmm0, -304(%rbp)                             #4162.17
..LN3701:
   .loc    3  4163
        movdqa    -320(%rbp), %xmm0                             #4163.22
        movdqa    -304(%rbp), %xmm1                             #4163.22
        pcmpeqw   %xmm1, %xmm0                                  #4163.22
..LN3703:
        movdqa    %xmm0, -320(%rbp)                             #4163.17
..LN3705:
   .loc    3  4164
        movdqa    -320(%rbp), %xmm0                             #4164.42
..LN3707:
        pmovmskb  %xmm0, %eax                                   #4164.22
..LN3709:
        movl      %eax, -80(%rbp)                               #4164.17
..LN3711:
   .loc    3  4169
        andl      $-4, -24(%rbp)                                #4169.17
..LN3713:
   .loc    3  4172
        movl      -68(%rbp), %eax                               #4172.29
        orl       -64(%rbp), %eax                               #4172.29
..LN3715:
        orl       -72(%rbp), %eax                               #4172.34
..LN3717:
        orl       -80(%rbp), %eax                               #4172.39
..LN3719:
        je        ..B25.32      # Prob 50%                      #4172.47
                                # LOE
..B25.16:                       # Preds ..B25.15
..LN3721:
   .loc    3  4190
        movl      -64(%rbp), %eax                               #4190.58
..LN3723:
        movl      %eax, %edi                                    #4190.30
        call      uint16_lsbit@PLT                              #4190.30
                                # LOE eax
..B25.44:                       # Preds ..B25.16
        movl      %eax, -112(%rbp)                              #4190.30
                                # LOE
..B25.17:                       # Preds ..B25.44
        movl      -112(%rbp), %eax                              #4190.30
        movswq    %ax, %rax                                     #4190.30
..LN3725:
        movl      %eax, -128(%rbp)                              #4190.28
..LN3727:
   .loc    3  4191
        movl      -68(%rbp), %eax                               #4191.58
..LN3729:
        movl      %eax, %edi                                    #4191.30
        call      uint16_lsbit@PLT                              #4191.30
                                # LOE eax
..B25.45:                       # Preds ..B25.17
        movl      %eax, -108(%rbp)                              #4191.30
                                # LOE
..B25.18:                       # Preds ..B25.45
        movl      -108(%rbp), %eax                              #4191.30
        movswq    %ax, %rax                                     #4191.30
..LN3731:
        movl      %eax, -124(%rbp)                              #4191.28
..LN3733:
   .loc    3  4192
        movl      -72(%rbp), %eax                               #4192.58
..LN3735:
        movl      %eax, %edi                                    #4192.30
        call      uint16_lsbit@PLT                              #4192.30
                                # LOE eax
..B25.46:                       # Preds ..B25.18
        movl      %eax, -104(%rbp)                              #4192.30
                                # LOE
..B25.19:                       # Preds ..B25.46
        movl      -104(%rbp), %eax                              #4192.30
        movswq    %ax, %rax                                     #4192.30
..LN3737:
        movl      %eax, -120(%rbp)                              #4192.28
..LN3739:
   .loc    3  4193
        movl      -80(%rbp), %eax                               #4193.58
..LN3741:
        movl      %eax, %edi                                    #4193.30
        call      uint16_lsbit@PLT                              #4193.30
                                # LOE eax
..B25.47:                       # Preds ..B25.19
        movl      %eax, -100(%rbp)                              #4193.30
                                # LOE
..B25.20:                       # Preds ..B25.47
        movl      -100(%rbp), %eax                              #4193.30
        movswq    %ax, %rax                                     #4193.30
..LN3743:
        movl      %eax, -116(%rbp)                              #4193.28
..LN3745:
   .loc    3  4198
        movl      -128(%rbp), %eax                              #4198.28
..LN3747:
        addl      %eax, %eax                                    #4198.34
..LN3749:
        movl      %eax, -128(%rbp)                              #4198.21
..LN3751:
   .loc    3  4199
        movl      -124(%rbp), %eax                              #4199.28
..LN3753:
        lea       1(%rax,%rax), %eax                            #4199.40
..LN3755:
        movl      %eax, -124(%rbp)                              #4199.21
..LN3757:
   .loc    3  4200
        movl      -120(%rbp), %eax                              #4200.28
..LN3759:
        lea       2(%rax,%rax), %eax                            #4200.40
..LN3761:
        movl      %eax, -120(%rbp)                              #4200.21
..LN3763:
   .loc    3  4201
        movl      -116(%rbp), %eax                              #4201.28
..LN3765:
        lea       3(%rax,%rax), %eax                            #4201.40
..LN3767:
        movl      %eax, -116(%rbp)                              #4201.21
..LN3769:
   .loc    3  4205
        movl      -64(%rbp), %eax                               #4205.26
..LN3771:
        testl     %eax, %eax                                    #4205.32
        je        ..B25.23      # Prob 50%                      #4205.32
                                # LOE
..B25.21:                       # Preds ..B25.20
..LN3773:
        movl      -128(%rbp), %eax                              #4205.43
        addl      -24(%rbp), %eax                               #4205.43
..LN3775:
        movl      -76(%rbp), %edx                               #4205.49
        cmpl      %edx, %eax                                    #4205.49
        ja        ..B25.23      # Prob 50%                      #4205.49
                                # LOE
..B25.22:                       # Preds ..B25.21
..LN3777:
        movl      $1, %eax                                      #4205.63
        leave                                                   #4205.63
..___tag_value_eval_4na_16.409:                                 #
        ret                                                     #4205.63
..___tag_value_eval_4na_16.410:                                 #
                                # LOE
..B25.23:                       # Preds ..B25.21 ..B25.20
..LN3779:
   .loc    3  4206
        movl      -68(%rbp), %eax                               #4206.26
..LN3781:
        testl     %eax, %eax                                    #4206.32
        je        ..B25.26      # Prob 50%                      #4206.32
                                # LOE
..B25.24:                       # Preds ..B25.23
..LN3783:
        movl      -124(%rbp), %eax                              #4206.43
        addl      -24(%rbp), %eax                               #4206.43
..LN3785:
        movl      -76(%rbp), %edx                               #4206.49
        cmpl      %edx, %eax                                    #4206.49
        ja        ..B25.26      # Prob 50%                      #4206.49
                                # LOE
..B25.25:                       # Preds ..B25.24
..LN3787:
        movl      $1, %eax                                      #4206.63
        leave                                                   #4206.63
..___tag_value_eval_4na_16.412:                                 #
        ret                                                     #4206.63
..___tag_value_eval_4na_16.413:                                 #
                                # LOE
..B25.26:                       # Preds ..B25.24 ..B25.23
..LN3789:
   .loc    3  4207
        movl      -72(%rbp), %eax                               #4207.26
..LN3791:
        testl     %eax, %eax                                    #4207.32
        je        ..B25.29      # Prob 50%                      #4207.32
                                # LOE
..B25.27:                       # Preds ..B25.26
..LN3793:
        movl      -120(%rbp), %eax                              #4207.43
        addl      -24(%rbp), %eax                               #4207.43
..LN3795:
        movl      -76(%rbp), %edx                               #4207.49
        cmpl      %edx, %eax                                    #4207.49
        ja        ..B25.29      # Prob 50%                      #4207.49
                                # LOE
..B25.28:                       # Preds ..B25.27
..LN3797:
        movl      $1, %eax                                      #4207.63
        leave                                                   #4207.63
..___tag_value_eval_4na_16.415:                                 #
        ret                                                     #4207.63
..___tag_value_eval_4na_16.416:                                 #
                                # LOE
..B25.29:                       # Preds ..B25.27 ..B25.26
..LN3799:
   .loc    3  4208
        movl      -80(%rbp), %eax                               #4208.26
..LN3801:
        testl     %eax, %eax                                    #4208.32
        je        ..B25.32      # Prob 50%                      #4208.32
                                # LOE
..B25.30:                       # Preds ..B25.29
..LN3803:
        movl      -116(%rbp), %eax                              #4208.43
        addl      -24(%rbp), %eax                               #4208.43
..LN3805:
        movl      -76(%rbp), %edx                               #4208.49
        cmpl      %edx, %eax                                    #4208.49
        ja        ..B25.32      # Prob 50%                      #4208.49
                                # LOE
..B25.31:                       # Preds ..B25.30
..LN3807:
        movl      $1, %eax                                      #4208.63
        leave                                                   #4208.63
..___tag_value_eval_4na_16.418:                                 #
        ret                                                     #4208.63
..___tag_value_eval_4na_16.419:                                 #
                                # LOE
..B25.32:                       # Preds ..B25.30 ..B25.29 ..B25.15
..LN3809:
   .loc    3  4213
        addl      $4, -24(%rbp)                                 #4213.17
..LN3811:
   .loc    3  4216
        movl      -24(%rbp), %eax                               #4216.22
..LN3813:
        movl      -76(%rbp), %edx                               #4216.28
        cmpl      %edx, %eax                                    #4216.28
        jbe       ..B25.34      # Prob 50%                      #4216.28
                                # LOE
..B25.33:                       # Preds ..B25.32
..LN3815:
   .loc    3  4217
        xorl      %eax, %eax                                    #4217.28
        leave                                                   #4217.28
..___tag_value_eval_4na_16.421:                                 #
        ret                                                     #4217.28
..___tag_value_eval_4na_16.422:                                 #
                                # LOE
..B25.34:                       # Preds ..B25.32
..LN3817:
   .loc    3  4220
        movl      -60(%rbp), %eax                               #4220.25
        decl      %eax                                          #4220.25
        movl      %eax, -60(%rbp)                               #4220.25
..LN3819:
        jne       ..B25.10      # Prob 50%                      #4220.39
                                # LOE
..B25.36:                       # Preds ..B25.34 ..B25.10
..LN3821:
   .loc    3  4237
        movq      -96(%rbp), %rax                               #4237.18
..LN3823:
        movq      -88(%rbp), %rdx                               #4237.25
        cmpq      %rdx, %rax                                    #4237.25
        jae       ..B25.40      # Prob 50%                      #4237.25
                                # LOE
..B25.37:                       # Preds ..B25.36
..LN3825:
   .loc    3  4241
        addl      $28, -24(%rbp)                                #4241.13
..LN3827:
   .loc    3  4242
        movl      -24(%rbp), %eax                               #4242.18
..LN3829:
        movl      -76(%rbp), %edx                               #4242.24
        cmpl      %edx, %eax                                    #4242.24
        ja        ..B25.40      # Prob 50%                      #4242.24
                                # LOE
..B25.38:                       # Preds ..B25.37
..LN3831:
   .loc    3  4247
        movq      -96(%rbp), %rax                               #4247.41
..LN3833:
        movq      -88(%rbp), %rdx                               #4247.46
..LN3835:
        movq      %rax, %rdi                                    #4247.22
        movq      %rdx, %rsi                                    #4247.22
        call      prime_buffer_4na@PLT                          #4247.22
                                # LOE xmm0
..B25.48:                       # Preds ..B25.38
        movdqa    %xmm0, -336(%rbp)                             #4247.22
                                # LOE
..B25.39:                       # Preds ..B25.48
..LN3837:
        movdqa    -336(%rbp), %xmm0                             #4247.13
        movdqa    %xmm0, -288(%rbp)                             #4247.13
..LN3839:
   .loc    3  4262
        addq      $8, -96(%rbp)                                 #4262.13
        jmp       ..B25.8       # Prob 100%                     #4262.13
                                # LOE
..B25.40:                       # Preds ..B25.37 ..B25.36 ..B25.8
..LN3841:
   .loc    3  4273
        xorl      %eax, %eax                                    #4273.12
        leave                                                   #4273.12
..___tag_value_eval_4na_16.424:                                 #
        ret                                                     #4273.12
        .align    2,0x90
..___tag_value_eval_4na_16.425:                                 #
                                # LOE
# mark_end;
	.type	eval_4na_16,@function
	.size	eval_4na_16,.-eval_4na_16
.LNeval_4na_16:
	.data
# -- End  eval_4na_16
	.text
# -- Begin  eval_4na_32
# mark_begin;
       .align    2,0x90
eval_4na_32:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B26.1:                        # Preds ..B26.0
..___tag_value_eval_4na_32.428:                                 #
..LN3843:
   .loc    3  4279
        pushq     %rbp                                          #4279.1
        movq      %rsp, %rbp                                    #4279.1
..___tag_value_eval_4na_32.429:                                 #
        subq      $336, %rsp                                    #4279.1
        movq      %rdi, -40(%rbp)                               #4279.1
        movq      %rsi, -32(%rbp)                               #4279.1
        movl      %edx, -16(%rbp)                               #4279.1
        movl      %ecx, -8(%rbp)                                #4279.1
..LN3845:
   .loc    3  4305
        movq      -40(%rbp), %rax                               #4305.25
        movl      4(%rax), %eax                                 #4305.25
..LN3847:
        movl      %eax, -24(%rbp)                               #4305.23
..LN3849:
   .loc    3  4308
        movl      -8(%rbp), %eax                                #4308.5
        movl      -24(%rbp), %edx                               #4308.5
        cmpl      %edx, %eax                                    #4308.5
        jae       ..B26.3       # Prob 50%                      #4308.5
                                # LOE
..B26.2:                        # Preds ..B26.1
        lea       _2__STRING.9.0(%rip), %rax                    #4308.5
        lea       _2__STRING.1.0(%rip), %rdx                    #4308.5
        lea       __$Uc.0(%rip), %rcx                           #4308.5
        movq      %rax, %rdi                                    #4308.5
        movq      %rdx, %rsi                                    #4308.5
        movl      $4308, %edx                                   #4308.5
        call      __assert_fail@PLT                             #4308.5
                                # LOE
..B26.3:                        # Preds ..B26.1
..LN3851:
   .loc    3  4312
        movl      -16(%rbp), %eax                               #4312.12
..LN3853:
        addl      -8(%rbp), %eax                                #4312.5
        movl      %eax, -8(%rbp)                                #4312.5
..LN3855:
   .loc    3  4315
        movl      -16(%rbp), %eax                               #4315.42
..LN3857:
        shrl      $2, %eax                                      #4315.49
..LN3859:
        addq      -32(%rbp), %rax                               #4315.30
..LN3861:
        movq      %rax, -96(%rbp)                               #4315.5
..LN3863:
   .loc    3  4318
        movl      -24(%rbp), %eax                               #4318.18
        negl      %eax                                          #4318.18
        addl      -8(%rbp), %eax                                #4318.18
..LN3865:
        movl      %eax, -68(%rbp)                               #4318.5
..LN3867:
   .loc    3  4321
        movl      -8(%rbp), %eax                                #4321.44
..LN3869:
        addl      $3, %eax                                      #4321.50
..LN3871:
        shrl      $2, %eax                                      #4321.57
..LN3873:
        addq      -32(%rbp), %rax                               #4321.30
..LN3875:
        movq      %rax, -88(%rbp)                               #4321.5
..LN3877:
   .loc    3  4324
        movq      -96(%rbp), %rax                               #4324.33
..LN3879:
        movq      -88(%rbp), %rdx                               #4324.38
..LN3881:
        movq      %rax, %rdi                                    #4324.14
        movq      %rdx, %rsi                                    #4324.14
        call      prime_buffer_4na@PLT                          #4324.14
                                # LOE xmm0
..B26.46:                       # Preds ..B26.3
        movdqa    %xmm0, -144(%rbp)                             #4324.14
                                # LOE
..B26.4:                        # Preds ..B26.46
..LN3883:
        movdqa    -144(%rbp), %xmm0                             #4324.5
        movdqa    %xmm0, -288(%rbp)                             #4324.5
..LN3885:
   .loc    3  4325
        addq      $8, -96(%rbp)                                 #4325.5
..LN3887:
   .loc    3  4327
        movq      -96(%rbp), %rax                               #4327.9
..LN3889:
        movq      %rax, -80(%rbp)                               #4327.5
..LN3891:
   .loc    3  4331
        movq      -40(%rbp), %rax                               #4331.5
        movdqa    16(%rax), %xmm0                               #4331.5
        movdqa    %xmm0, -272(%rbp)                             #4331.5
        movq      -40(%rbp), %rax                               #4331.5
        movdqa    32(%rax), %xmm0                               #4331.5
        movdqa    %xmm0, -256(%rbp)                             #4331.5
        movq      -40(%rbp), %rax                               #4331.5
        movdqa    48(%rax), %xmm0                               #4331.5
        movdqa    %xmm0, -240(%rbp)                             #4331.5
        movq      -40(%rbp), %rax                               #4331.5
        movdqa    64(%rax), %xmm0                               #4331.5
        movdqa    %xmm0, -224(%rbp)                             #4331.5
        movq      -40(%rbp), %rax                               #4331.5
        movdqa    80(%rax), %xmm0                               #4331.5
        movdqa    %xmm0, -208(%rbp)                             #4331.5
        movq      -40(%rbp), %rax                               #4331.5
        movdqa    96(%rax), %xmm0                               #4331.5
        movdqa    %xmm0, -192(%rbp)                             #4331.5
        movq      -40(%rbp), %rax                               #4331.5
        movdqa    112(%rax), %xmm0                              #4331.5
        movdqa    %xmm0, -176(%rbp)                             #4331.5
        movq      -40(%rbp), %rax                               #4331.5
        movdqa    128(%rax), %xmm0                              #4331.5
        movdqa    %xmm0, -160(%rbp)                             #4331.5
..LN3893:
   .loc    3  4334
        xorl      %eax, %eax                                    #4334.15
        movl      %eax, -64(%rbp)                               #4334.15
..LN3895:
        movl      %eax, -60(%rbp)                               #4334.10
..LN3897:
        movl      %eax, -56(%rbp)                               #4334.5
..LN3899:
   .loc    3  4340
        movl      $2, -52(%rbp)                                 #4340.5
..LN3901:
   .loc    3  4345
        movl      -16(%rbp), %eax                               #4345.14
..LN3903:
        andl      $3, %eax                                      #4345.20
        movl      %eax, -48(%rbp)                               #4345.20
        je        ..B26.12      # Prob 50%                      #4345.20
                                # LOE
..B26.5:                        # Preds ..B26.4
        movl      -48(%rbp), %eax                               #4345.20
        cmpl      $1, %eax                                      #4345.20
        je        ..B26.13      # Prob 50%                      #4345.20
                                # LOE
..B26.6:                        # Preds ..B26.5
        movl      -48(%rbp), %eax                               #4345.20
        cmpl      $2, %eax                                      #4345.20
        je        ..B26.14      # Prob 50%                      #4345.20
                                # LOE
..B26.7:                        # Preds ..B26.6
        movl      -48(%rbp), %eax                               #4345.20
        cmpl      $3, %eax                                      #4345.20
        je        ..B26.15      # Prob 50%                      #4345.20
                                # LOE
..B26.8:                        # Preds ..B26.42 ..B26.7
..LN3905:
   .loc    3  4350
        movl      $1, %eax                                      #4350.9
        testl     %eax, %eax                                    #4350.9
        je        ..B26.43      # Prob 100%                     #4350.9
                                # LOE
..B26.9:                        # Preds ..B26.8
..LN3907:
   .loc    3  4352
        movl      $2, -52(%rbp)                                 #4352.13
                                # LOE
..B26.10:                       # Preds ..B26.37 ..B26.9
..LN3909:
   .loc    3  4355
        movl      $1, %eax                                      #4355.13
        testl     %eax, %eax                                    #4355.13
        je        ..B26.39      # Prob 100%                     #4355.13
                                # LOE
..B26.12:                       # Preds ..B26.4 ..B26.10
..LN3911:
   .loc    3  4362
        movdqa    -288(%rbp), %xmm0                             #4362.38
..LN3913:
        movdqa    -272(%rbp), %xmm1                             #4362.46
..LN3915:
        pand      %xmm1, %xmm0                                  #4362.22
..LN3917:
        movdqa    %xmm0, -320(%rbp)                             #4362.17
..LN3919:
   .loc    3  4363
        movdqa    -288(%rbp), %xmm0                             #4363.38
..LN3921:
        movdqa    -256(%rbp), %xmm1                             #4363.46
..LN3923:
        pand      %xmm1, %xmm0                                  #4363.22
..LN3925:
        movdqa    %xmm0, -304(%rbp)                             #4363.17
..LN3927:
   .loc    3  4364
        movdqa    -320(%rbp), %xmm0                             #4364.22
        movdqa    -304(%rbp), %xmm1                             #4364.22
        pcmpeqd   %xmm1, %xmm0                                  #4364.22
..LN3929:
        movdqa    %xmm0, -320(%rbp)                             #4364.17
..LN3931:
   .loc    3  4365
        movdqa    -320(%rbp), %xmm0                             #4365.42
..LN3933:
        pmovmskb  %xmm0, %eax                                   #4365.22
..LN3935:
        movl      %eax, -56(%rbp)                               #4365.17
                                # LOE
..B26.13:                       # Preds ..B26.5 ..B26.12
..LN3937:
   .loc    3  4369
        movdqa    -288(%rbp), %xmm0                             #4369.38
..LN3939:
        movdqa    -240(%rbp), %xmm1                             #4369.46
..LN3941:
        pand      %xmm1, %xmm0                                  #4369.22
..LN3943:
        movdqa    %xmm0, -320(%rbp)                             #4369.17
..LN3945:
   .loc    3  4370
        movdqa    -288(%rbp), %xmm0                             #4370.38
..LN3947:
        movdqa    -224(%rbp), %xmm1                             #4370.46
..LN3949:
        pand      %xmm1, %xmm0                                  #4370.22
..LN3951:
        movdqa    %xmm0, -304(%rbp)                             #4370.17
..LN3953:
   .loc    3  4371
        movdqa    -320(%rbp), %xmm0                             #4371.22
        movdqa    -304(%rbp), %xmm1                             #4371.22
        pcmpeqd   %xmm1, %xmm0                                  #4371.22
..LN3955:
        movdqa    %xmm0, -320(%rbp)                             #4371.17
..LN3957:
   .loc    3  4372
        movdqa    -320(%rbp), %xmm0                             #4372.42
..LN3959:
        pmovmskb  %xmm0, %eax                                   #4372.22
..LN3961:
        movl      %eax, -60(%rbp)                               #4372.17
                                # LOE
..B26.14:                       # Preds ..B26.6 ..B26.13
..LN3963:
   .loc    3  4376
        movdqa    -288(%rbp), %xmm0                             #4376.38
..LN3965:
        movdqa    -208(%rbp), %xmm1                             #4376.46
..LN3967:
        pand      %xmm1, %xmm0                                  #4376.22
..LN3969:
        movdqa    %xmm0, -320(%rbp)                             #4376.17
..LN3971:
   .loc    3  4377
        movdqa    -288(%rbp), %xmm0                             #4377.38
..LN3973:
        movdqa    -192(%rbp), %xmm1                             #4377.46
..LN3975:
        pand      %xmm1, %xmm0                                  #4377.22
..LN3977:
        movdqa    %xmm0, -304(%rbp)                             #4377.17
..LN3979:
   .loc    3  4378
        movdqa    -320(%rbp), %xmm0                             #4378.22
        movdqa    -304(%rbp), %xmm1                             #4378.22
        pcmpeqd   %xmm1, %xmm0                                  #4378.22
..LN3981:
        movdqa    %xmm0, -320(%rbp)                             #4378.17
..LN3983:
   .loc    3  4379
        movdqa    -320(%rbp), %xmm0                             #4379.42
..LN3985:
        pmovmskb  %xmm0, %eax                                   #4379.22
..LN3987:
        movl      %eax, -64(%rbp)                               #4379.17
                                # LOE
..B26.15:                       # Preds ..B26.7 ..B26.14
..LN3989:
   .loc    3  4383
        movdqa    -288(%rbp), %xmm0                             #4383.38
..LN3991:
        movdqa    -176(%rbp), %xmm1                             #4383.46
..LN3993:
        pand      %xmm1, %xmm0                                  #4383.22
..LN3995:
        movdqa    %xmm0, -320(%rbp)                             #4383.17
..LN3997:
   .loc    3  4384
        movdqa    -288(%rbp), %xmm0                             #4384.38
..LN3999:
        movdqa    -160(%rbp), %xmm1                             #4384.46
..LN4001:
        pand      %xmm1, %xmm0                                  #4384.22
..LN4003:
        movdqa    %xmm0, -304(%rbp)                             #4384.17
..LN4005:
   .loc    3  4385
        movdqa    -320(%rbp), %xmm0                             #4385.22
        movdqa    -304(%rbp), %xmm1                             #4385.22
        pcmpeqd   %xmm1, %xmm0                                  #4385.22
..LN4007:
        movdqa    %xmm0, -320(%rbp)                             #4385.17
..LN4009:
   .loc    3  4386
        movdqa    -320(%rbp), %xmm0                             #4386.42
..LN4011:
        pmovmskb  %xmm0, %eax                                   #4386.22
..LN4013:
        movl      %eax, -72(%rbp)                               #4386.17
..LN4015:
   .loc    3  4391
        andl      $-4, -16(%rbp)                                #4391.17
..LN4017:
   .loc    3  4394
        movl      -60(%rbp), %eax                               #4394.29
        orl       -56(%rbp), %eax                               #4394.29
..LN4019:
        orl       -64(%rbp), %eax                               #4394.34
..LN4021:
        orl       -72(%rbp), %eax                               #4394.39
..LN4023:
        je        ..B26.32      # Prob 50%                      #4394.47
                                # LOE
..B26.16:                       # Preds ..B26.15
..LN4025:
   .loc    3  4412
        movl      -56(%rbp), %eax                               #4412.58
..LN4027:
        movl      %eax, %edi                                    #4412.30
        call      uint16_lsbit@PLT                              #4412.30
                                # LOE eax
..B26.47:                       # Preds ..B26.16
        movl      %eax, -112(%rbp)                              #4412.30
                                # LOE
..B26.17:                       # Preds ..B26.47
        movl      -112(%rbp), %eax                              #4412.30
        movswq    %ax, %rax                                     #4412.30
..LN4029:
        movl      %eax, -128(%rbp)                              #4412.28
..LN4031:
   .loc    3  4413
        movl      -60(%rbp), %eax                               #4413.58
..LN4033:
        movl      %eax, %edi                                    #4413.30
        call      uint16_lsbit@PLT                              #4413.30
                                # LOE eax
..B26.48:                       # Preds ..B26.17
        movl      %eax, -108(%rbp)                              #4413.30
                                # LOE
..B26.18:                       # Preds ..B26.48
        movl      -108(%rbp), %eax                              #4413.30
        movswq    %ax, %rax                                     #4413.30
..LN4035:
        movl      %eax, -124(%rbp)                              #4413.28
..LN4037:
   .loc    3  4414
        movl      -64(%rbp), %eax                               #4414.58
..LN4039:
        movl      %eax, %edi                                    #4414.30
        call      uint16_lsbit@PLT                              #4414.30
                                # LOE eax
..B26.49:                       # Preds ..B26.18
        movl      %eax, -104(%rbp)                              #4414.30
                                # LOE
..B26.19:                       # Preds ..B26.49
        movl      -104(%rbp), %eax                              #4414.30
        movswq    %ax, %rax                                     #4414.30
..LN4041:
        movl      %eax, -120(%rbp)                              #4414.28
..LN4043:
   .loc    3  4415
        movl      -72(%rbp), %eax                               #4415.58
..LN4045:
        movl      %eax, %edi                                    #4415.30
        call      uint16_lsbit@PLT                              #4415.30
                                # LOE eax
..B26.50:                       # Preds ..B26.19
        movl      %eax, -100(%rbp)                              #4415.30
                                # LOE
..B26.20:                       # Preds ..B26.50
        movl      -100(%rbp), %eax                              #4415.30
        movswq    %ax, %rax                                     #4415.30
..LN4047:
        movl      %eax, -116(%rbp)                              #4415.28
..LN4049:
   .loc    3  4420
        movl      -128(%rbp), %eax                              #4420.28
..LN4051:
        addl      %eax, %eax                                    #4420.34
..LN4053:
        movl      %eax, -128(%rbp)                              #4420.21
..LN4055:
   .loc    3  4421
        movl      -124(%rbp), %eax                              #4421.28
..LN4057:
        lea       1(%rax,%rax), %eax                            #4421.40
..LN4059:
        movl      %eax, -124(%rbp)                              #4421.21
..LN4061:
   .loc    3  4422
        movl      -120(%rbp), %eax                              #4422.28
..LN4063:
        lea       2(%rax,%rax), %eax                            #4422.40
..LN4065:
        movl      %eax, -120(%rbp)                              #4422.21
..LN4067:
   .loc    3  4423
        movl      -116(%rbp), %eax                              #4423.28
..LN4069:
        lea       3(%rax,%rax), %eax                            #4423.40
..LN4071:
        movl      %eax, -116(%rbp)                              #4423.21
..LN4073:
   .loc    3  4427
        movl      -56(%rbp), %eax                               #4427.26
..LN4075:
        testl     %eax, %eax                                    #4427.32
        je        ..B26.23      # Prob 50%                      #4427.32
                                # LOE
..B26.21:                       # Preds ..B26.20
..LN4077:
        movl      -128(%rbp), %eax                              #4427.43
        addl      -16(%rbp), %eax                               #4427.43
..LN4079:
        movl      -68(%rbp), %edx                               #4427.49
        cmpl      %edx, %eax                                    #4427.49
        ja        ..B26.23      # Prob 50%                      #4427.49
                                # LOE
..B26.22:                       # Preds ..B26.21
..LN4081:
        movl      $1, %eax                                      #4427.63
        leave                                                   #4427.63
..___tag_value_eval_4na_32.433:                                 #
        ret                                                     #4427.63
..___tag_value_eval_4na_32.434:                                 #
                                # LOE
..B26.23:                       # Preds ..B26.21 ..B26.20
..LN4083:
   .loc    3  4428
        movl      -60(%rbp), %eax                               #4428.26
..LN4085:
        testl     %eax, %eax                                    #4428.32
        je        ..B26.26      # Prob 50%                      #4428.32
                                # LOE
..B26.24:                       # Preds ..B26.23
..LN4087:
        movl      -124(%rbp), %eax                              #4428.43
        addl      -16(%rbp), %eax                               #4428.43
..LN4089:
        movl      -68(%rbp), %edx                               #4428.49
        cmpl      %edx, %eax                                    #4428.49
        ja        ..B26.26      # Prob 50%                      #4428.49
                                # LOE
..B26.25:                       # Preds ..B26.24
..LN4091:
        movl      $1, %eax                                      #4428.63
        leave                                                   #4428.63
..___tag_value_eval_4na_32.436:                                 #
        ret                                                     #4428.63
..___tag_value_eval_4na_32.437:                                 #
                                # LOE
..B26.26:                       # Preds ..B26.24 ..B26.23
..LN4093:
   .loc    3  4429
        movl      -64(%rbp), %eax                               #4429.26
..LN4095:
        testl     %eax, %eax                                    #4429.32
        je        ..B26.29      # Prob 50%                      #4429.32
                                # LOE
..B26.27:                       # Preds ..B26.26
..LN4097:
        movl      -120(%rbp), %eax                              #4429.43
        addl      -16(%rbp), %eax                               #4429.43
..LN4099:
        movl      -68(%rbp), %edx                               #4429.49
        cmpl      %edx, %eax                                    #4429.49
        ja        ..B26.29      # Prob 50%                      #4429.49
                                # LOE
..B26.28:                       # Preds ..B26.27
..LN4101:
        movl      $1, %eax                                      #4429.63
        leave                                                   #4429.63
..___tag_value_eval_4na_32.439:                                 #
        ret                                                     #4429.63
..___tag_value_eval_4na_32.440:                                 #
                                # LOE
..B26.29:                       # Preds ..B26.27 ..B26.26
..LN4103:
   .loc    3  4430
        movl      -72(%rbp), %eax                               #4430.26
..LN4105:
        testl     %eax, %eax                                    #4430.32
        je        ..B26.32      # Prob 50%                      #4430.32
                                # LOE
..B26.30:                       # Preds ..B26.29
..LN4107:
        movl      -116(%rbp), %eax                              #4430.43
        addl      -16(%rbp), %eax                               #4430.43
..LN4109:
        movl      -68(%rbp), %edx                               #4430.49
        cmpl      %edx, %eax                                    #4430.49
        ja        ..B26.32      # Prob 50%                      #4430.49
                                # LOE
..B26.31:                       # Preds ..B26.30
..LN4111:
        movl      $1, %eax                                      #4430.63
        leave                                                   #4430.63
..___tag_value_eval_4na_32.442:                                 #
        ret                                                     #4430.63
..___tag_value_eval_4na_32.443:                                 #
                                # LOE
..B26.32:                       # Preds ..B26.30 ..B26.29 ..B26.15
..LN4113:
   .loc    3  4435
        addl      $4, -16(%rbp)                                 #4435.17
..LN4115:
   .loc    3  4438
        movl      -16(%rbp), %eax                               #4438.22
..LN4117:
        movl      -68(%rbp), %edx                               #4438.28
        cmpl      %edx, %eax                                    #4438.28
        jbe       ..B26.34      # Prob 50%                      #4438.28
                                # LOE
..B26.33:                       # Preds ..B26.32
..LN4119:
   .loc    3  4439
        xorl      %eax, %eax                                    #4439.28
        leave                                                   #4439.28
..___tag_value_eval_4na_32.445:                                 #
        ret                                                     #4439.28
..___tag_value_eval_4na_32.446:                                 #
                                # LOE
..B26.34:                       # Preds ..B26.32
..LN4121:
   .loc    3  4442
        movl      -52(%rbp), %eax                               #4442.25
        decl      %eax                                          #4442.25
        movl      %eax, -52(%rbp)                               #4442.25
..LN4123:
        je        ..B26.39      # Prob 50%                      #4442.39
                                # LOE
..B26.35:                       # Preds ..B26.34
..LN4125:
   .loc    3  4447
        movdqa    -288(%rbp), %xmm0                             #4447.43
..LN4127:
        psrldq    $2, %xmm0                                     #4447.26
..LN4129:
        movdqa    %xmm0, -288(%rbp)                             #4447.17
..LN4131:
   .loc    3  4450
        movq      -80(%rbp), %rax                               #4450.22
..LN4133:
        movq      -88(%rbp), %rdx                               #4450.26
        cmpq      %rdx, %rax                                    #4450.26
        jae       ..B26.37      # Prob 50%                      #4450.26
                                # LOE
..B26.36:                       # Preds ..B26.35
..LN4135:
   .loc    3  4451
        movdqa    -288(%rbp), %xmm0                             #4451.49
..LN4137:
        movq      -80(%rbp), %rax                               #4451.72
        movzbl    (%rax), %eax                                  #4451.72
        movzbl    %al, %eax                                     #4451.72
..LN4139:
        movslq    %eax, %rax                                    #4451.57
        lea       expand_2na.0(%rip), %rdx                      #4451.57
        movzwl    (%rdx,%rax,2), %eax                           #4451.57
        movzwl    %ax, %eax                                     #4451.57
..LN4141:
        pinsrw    $7, %eax, %xmm0                               #4451.30
..LN4143:
        movdqa    %xmm0, -288(%rbp)                             #4451.21
                                # LOE
..B26.37:                       # Preds ..B26.36 ..B26.35
..LN4145:
   .loc    3  4454
        incq      -80(%rbp)                                     #4454.20
        jmp       ..B26.10      # Prob 100%                     #4454.20
                                # LOE
..B26.39:                       # Preds ..B26.34 ..B26.10
..LN4147:
   .loc    3  4459
        movq      -96(%rbp), %rax                               #4459.18
..LN4149:
        movq      -88(%rbp), %rdx                               #4459.25
        cmpq      %rdx, %rax                                    #4459.25
        jae       ..B26.43      # Prob 50%                      #4459.25
                                # LOE
..B26.40:                       # Preds ..B26.39
..LN4151:
   .loc    3  4463
        addl      $24, -16(%rbp)                                #4463.13
..LN4153:
   .loc    3  4464
        movl      -16(%rbp), %eax                               #4464.18
..LN4155:
        movl      -68(%rbp), %edx                               #4464.24
        cmpl      %edx, %eax                                    #4464.24
        ja        ..B26.43      # Prob 50%                      #4464.24
                                # LOE
..B26.41:                       # Preds ..B26.40
..LN4157:
   .loc    3  4469
        movq      -96(%rbp), %rax                               #4469.41
..LN4159:
        movq      -88(%rbp), %rdx                               #4469.46
..LN4161:
        movq      %rax, %rdi                                    #4469.22
        movq      %rdx, %rsi                                    #4469.22
        call      prime_buffer_4na@PLT                          #4469.22
                                # LOE xmm0
..B26.51:                       # Preds ..B26.41
        movdqa    %xmm0, -336(%rbp)                             #4469.22
                                # LOE
..B26.42:                       # Preds ..B26.51
..LN4163:
        movdqa    -336(%rbp), %xmm0                             #4469.13
        movdqa    %xmm0, -288(%rbp)                             #4469.13
..LN4165:
   .loc    3  4484
        addq      $8, -96(%rbp)                                 #4484.13
..LN4167:
   .loc    3  4486
        movq      -96(%rbp), %rax                               #4486.17
..LN4169:
        movq      %rax, -80(%rbp)                               #4486.13
        jmp       ..B26.8       # Prob 100%                     #4486.13
                                # LOE
..B26.43:                       # Preds ..B26.40 ..B26.39 ..B26.8
..LN4171:
   .loc    3  4495
        xorl      %eax, %eax                                    #4495.12
        leave                                                   #4495.12
..___tag_value_eval_4na_32.448:                                 #
        ret                                                     #4495.12
        .align    2,0x90
..___tag_value_eval_4na_32.449:                                 #
                                # LOE
# mark_end;
	.type	eval_4na_32,@function
	.size	eval_4na_32,.-eval_4na_32
.LNeval_4na_32:
	.data
# -- End  eval_4na_32
	.text
# -- Begin  eval_4na_64
# mark_begin;
       .align    2,0x90
eval_4na_64:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B27.1:                        # Preds ..B27.0
..___tag_value_eval_4na_64.452:                                 #
..LN4173:
   .loc    3  4501
        pushq     %rbp                                          #4501.1
        movq      %rsp, %rbp                                    #4501.1
..___tag_value_eval_4na_64.453:                                 #
        subq      $336, %rsp                                    #4501.1
        movq      %rdi, -40(%rbp)                               #4501.1
        movq      %rsi, -32(%rbp)                               #4501.1
        movl      %edx, -16(%rbp)                               #4501.1
        movl      %ecx, -8(%rbp)                                #4501.1
..LN4175:
   .loc    3  4528
        movq      -40(%rbp), %rax                               #4528.25
        movl      4(%rax), %eax                                 #4528.25
..LN4177:
        movl      %eax, -24(%rbp)                               #4528.23
..LN4179:
   .loc    3  4531
        movl      -8(%rbp), %eax                                #4531.5
        movl      -24(%rbp), %edx                               #4531.5
        cmpl      %edx, %eax                                    #4531.5
        jae       ..B27.3       # Prob 50%                      #4531.5
                                # LOE
..B27.2:                        # Preds ..B27.1
        lea       _2__STRING.9.0(%rip), %rax                    #4531.5
        lea       _2__STRING.1.0(%rip), %rdx                    #4531.5
        lea       __$Ud.0(%rip), %rcx                           #4531.5
        movq      %rax, %rdi                                    #4531.5
        movq      %rdx, %rsi                                    #4531.5
        movl      $4531, %edx                                   #4531.5
        call      __assert_fail@PLT                             #4531.5
                                # LOE
..B27.3:                        # Preds ..B27.1
..LN4181:
   .loc    3  4535
        movl      -16(%rbp), %eax                               #4535.12
..LN4183:
        addl      -8(%rbp), %eax                                #4535.5
        movl      %eax, -8(%rbp)                                #4535.5
..LN4185:
   .loc    3  4538
        movl      -16(%rbp), %eax                               #4538.42
..LN4187:
        shrl      $2, %eax                                      #4538.49
..LN4189:
        addq      -32(%rbp), %rax                               #4538.30
..LN4191:
        movq      %rax, -96(%rbp)                               #4538.5
..LN4193:
   .loc    3  4541
        movl      -24(%rbp), %eax                               #4541.18
        negl      %eax                                          #4541.18
        addl      -8(%rbp), %eax                                #4541.18
..LN4195:
        movl      %eax, -68(%rbp)                               #4541.5
..LN4197:
   .loc    3  4544
        movl      -8(%rbp), %eax                                #4544.44
..LN4199:
        addl      $3, %eax                                      #4544.50
..LN4201:
        shrl      $2, %eax                                      #4544.57
..LN4203:
        addq      -32(%rbp), %rax                               #4544.30
..LN4205:
        movq      %rax, -88(%rbp)                               #4544.5
..LN4207:
   .loc    3  4547
        movq      -96(%rbp), %rax                               #4547.33
..LN4209:
        movq      -88(%rbp), %rdx                               #4547.38
..LN4211:
        movq      %rax, %rdi                                    #4547.14
        movq      %rdx, %rsi                                    #4547.14
        call      prime_buffer_4na@PLT                          #4547.14
                                # LOE xmm0
..B27.46:                       # Preds ..B27.3
        movdqa    %xmm0, -144(%rbp)                             #4547.14
                                # LOE
..B27.4:                        # Preds ..B27.46
..LN4213:
        movdqa    -144(%rbp), %xmm0                             #4547.5
        movdqa    %xmm0, -288(%rbp)                             #4547.5
..LN4215:
   .loc    3  4548
        addq      $8, -96(%rbp)                                 #4548.5
..LN4217:
   .loc    3  4550
        movq      -96(%rbp), %rax                               #4550.9
..LN4219:
        movq      %rax, -80(%rbp)                               #4550.5
..LN4221:
   .loc    3  4554
        movq      -40(%rbp), %rax                               #4554.5
        movdqa    16(%rax), %xmm0                               #4554.5
        movdqa    %xmm0, -272(%rbp)                             #4554.5
        movq      -40(%rbp), %rax                               #4554.5
        movdqa    32(%rax), %xmm0                               #4554.5
        movdqa    %xmm0, -256(%rbp)                             #4554.5
        movq      -40(%rbp), %rax                               #4554.5
        movdqa    48(%rax), %xmm0                               #4554.5
        movdqa    %xmm0, -240(%rbp)                             #4554.5
        movq      -40(%rbp), %rax                               #4554.5
        movdqa    64(%rax), %xmm0                               #4554.5
        movdqa    %xmm0, -224(%rbp)                             #4554.5
        movq      -40(%rbp), %rax                               #4554.5
        movdqa    80(%rax), %xmm0                               #4554.5
        movdqa    %xmm0, -208(%rbp)                             #4554.5
        movq      -40(%rbp), %rax                               #4554.5
        movdqa    96(%rax), %xmm0                               #4554.5
        movdqa    %xmm0, -192(%rbp)                             #4554.5
        movq      -40(%rbp), %rax                               #4554.5
        movdqa    112(%rax), %xmm0                              #4554.5
        movdqa    %xmm0, -176(%rbp)                             #4554.5
        movq      -40(%rbp), %rax                               #4554.5
        movdqa    128(%rax), %xmm0                              #4554.5
        movdqa    %xmm0, -160(%rbp)                             #4554.5
..LN4223:
   .loc    3  4557
        xorl      %eax, %eax                                    #4557.15
        movl      %eax, -64(%rbp)                               #4557.15
..LN4225:
        movl      %eax, -60(%rbp)                               #4557.10
..LN4227:
        movl      %eax, -56(%rbp)                               #4557.5
..LN4229:
   .loc    3  4563
        movl      $4, -52(%rbp)                                 #4563.5
..LN4231:
   .loc    3  4568
        movl      -16(%rbp), %eax                               #4568.14
..LN4233:
        andl      $3, %eax                                      #4568.20
        movl      %eax, -48(%rbp)                               #4568.20
        je        ..B27.12      # Prob 50%                      #4568.20
                                # LOE
..B27.5:                        # Preds ..B27.4
        movl      -48(%rbp), %eax                               #4568.20
        cmpl      $1, %eax                                      #4568.20
        je        ..B27.13      # Prob 50%                      #4568.20
                                # LOE
..B27.6:                        # Preds ..B27.5
        movl      -48(%rbp), %eax                               #4568.20
        cmpl      $2, %eax                                      #4568.20
        je        ..B27.14      # Prob 50%                      #4568.20
                                # LOE
..B27.7:                        # Preds ..B27.6
        movl      -48(%rbp), %eax                               #4568.20
        cmpl      $3, %eax                                      #4568.20
        je        ..B27.15      # Prob 50%                      #4568.20
                                # LOE
..B27.8:                        # Preds ..B27.42 ..B27.7
..LN4235:
   .loc    3  4573
        movl      $1, %eax                                      #4573.9
        testl     %eax, %eax                                    #4573.9
        je        ..B27.43      # Prob 100%                     #4573.9
                                # LOE
..B27.9:                        # Preds ..B27.8
..LN4237:
   .loc    3  4575
        movl      $4, -52(%rbp)                                 #4575.13
                                # LOE
..B27.10:                       # Preds ..B27.37 ..B27.9
..LN4239:
   .loc    3  4578
        movl      $1, %eax                                      #4578.13
        testl     %eax, %eax                                    #4578.13
        je        ..B27.39      # Prob 100%                     #4578.13
                                # LOE
..B27.12:                       # Preds ..B27.4 ..B27.10
..LN4241:
   .loc    3  4585
        movdqa    -288(%rbp), %xmm0                             #4585.38
..LN4243:
        movdqa    -272(%rbp), %xmm1                             #4585.46
..LN4245:
        pand      %xmm1, %xmm0                                  #4585.22
..LN4247:
        movdqa    %xmm0, -320(%rbp)                             #4585.17
..LN4249:
   .loc    3  4586
        movdqa    -288(%rbp), %xmm0                             #4586.38
..LN4251:
        movdqa    -256(%rbp), %xmm1                             #4586.46
..LN4253:
        pand      %xmm1, %xmm0                                  #4586.22
..LN4255:
        movdqa    %xmm0, -304(%rbp)                             #4586.17
..LN4257:
   .loc    3  4587
        movdqa    -320(%rbp), %xmm0                             #4587.22
        movdqa    -304(%rbp), %xmm1                             #4587.22
        pcmpeqd   %xmm1, %xmm0                                  #4587.22
..LN4259:
        movdqa    %xmm0, -320(%rbp)                             #4587.17
..LN4261:
   .loc    3  4588
        movdqa    -320(%rbp), %xmm0                             #4588.42
..LN4263:
        pmovmskb  %xmm0, %eax                                   #4588.22
..LN4265:
        movl      %eax, -56(%rbp)                               #4588.17
..LN4267:
   .loc    3  4589
        movl      -56(%rbp), %eax                               #4589.17
        andl      $3855, %eax                                   #4589.17
        shll      $4, %eax                                      #4589.17
        andl      -56(%rbp), %eax                               #4589.17
        movl      %eax, -56(%rbp)                               #4589.17
        movl      -56(%rbp), %eax                               #4589.17
        sarl      $4, %eax                                      #4589.17
        orl       -56(%rbp), %eax                               #4589.17
        movl      %eax, -56(%rbp)                               #4589.17
                                # LOE
..B27.13:                       # Preds ..B27.5 ..B27.12
..LN4269:
   .loc    3  4592
        movdqa    -288(%rbp), %xmm0                             #4592.38
..LN4271:
        movdqa    -240(%rbp), %xmm1                             #4592.46
..LN4273:
        pand      %xmm1, %xmm0                                  #4592.22
..LN4275:
        movdqa    %xmm0, -320(%rbp)                             #4592.17
..LN4277:
   .loc    3  4593
        movdqa    -288(%rbp), %xmm0                             #4593.38
..LN4279:
        movdqa    -224(%rbp), %xmm1                             #4593.46
..LN4281:
        pand      %xmm1, %xmm0                                  #4593.22
..LN4283:
        movdqa    %xmm0, -304(%rbp)                             #4593.17
..LN4285:
   .loc    3  4594
        movdqa    -320(%rbp), %xmm0                             #4594.22
        movdqa    -304(%rbp), %xmm1                             #4594.22
        pcmpeqd   %xmm1, %xmm0                                  #4594.22
..LN4287:
        movdqa    %xmm0, -320(%rbp)                             #4594.17
..LN4289:
   .loc    3  4595
        movdqa    -320(%rbp), %xmm0                             #4595.42
..LN4291:
        pmovmskb  %xmm0, %eax                                   #4595.22
..LN4293:
        movl      %eax, -60(%rbp)                               #4595.17
..LN4295:
   .loc    3  4596
        movl      -60(%rbp), %eax                               #4596.17
        andl      $3855, %eax                                   #4596.17
        shll      $4, %eax                                      #4596.17
        andl      -60(%rbp), %eax                               #4596.17
        movl      %eax, -60(%rbp)                               #4596.17
        movl      -60(%rbp), %eax                               #4596.17
        sarl      $4, %eax                                      #4596.17
        orl       -60(%rbp), %eax                               #4596.17
        movl      %eax, -60(%rbp)                               #4596.17
                                # LOE
..B27.14:                       # Preds ..B27.6 ..B27.13
..LN4297:
   .loc    3  4599
        movdqa    -288(%rbp), %xmm0                             #4599.38
..LN4299:
        movdqa    -208(%rbp), %xmm1                             #4599.46
..LN4301:
        pand      %xmm1, %xmm0                                  #4599.22
..LN4303:
        movdqa    %xmm0, -320(%rbp)                             #4599.17
..LN4305:
   .loc    3  4600
        movdqa    -288(%rbp), %xmm0                             #4600.38
..LN4307:
        movdqa    -192(%rbp), %xmm1                             #4600.46
..LN4309:
        pand      %xmm1, %xmm0                                  #4600.22
..LN4311:
        movdqa    %xmm0, -304(%rbp)                             #4600.17
..LN4313:
   .loc    3  4601
        movdqa    -320(%rbp), %xmm0                             #4601.22
        movdqa    -304(%rbp), %xmm1                             #4601.22
        pcmpeqd   %xmm1, %xmm0                                  #4601.22
..LN4315:
        movdqa    %xmm0, -320(%rbp)                             #4601.17
..LN4317:
   .loc    3  4602
        movdqa    -320(%rbp), %xmm0                             #4602.42
..LN4319:
        pmovmskb  %xmm0, %eax                                   #4602.22
..LN4321:
        movl      %eax, -64(%rbp)                               #4602.17
..LN4323:
   .loc    3  4603
        movl      -64(%rbp), %eax                               #4603.17
        andl      $3855, %eax                                   #4603.17
        shll      $4, %eax                                      #4603.17
        andl      -64(%rbp), %eax                               #4603.17
        movl      %eax, -64(%rbp)                               #4603.17
        movl      -64(%rbp), %eax                               #4603.17
        sarl      $4, %eax                                      #4603.17
        orl       -64(%rbp), %eax                               #4603.17
        movl      %eax, -64(%rbp)                               #4603.17
                                # LOE
..B27.15:                       # Preds ..B27.7 ..B27.14
..LN4325:
   .loc    3  4606
        movdqa    -288(%rbp), %xmm0                             #4606.38
..LN4327:
        movdqa    -176(%rbp), %xmm1                             #4606.46
..LN4329:
        pand      %xmm1, %xmm0                                  #4606.22
..LN4331:
        movdqa    %xmm0, -320(%rbp)                             #4606.17
..LN4333:
   .loc    3  4607
        movdqa    -288(%rbp), %xmm0                             #4607.38
..LN4335:
        movdqa    -160(%rbp), %xmm1                             #4607.46
..LN4337:
        pand      %xmm1, %xmm0                                  #4607.22
..LN4339:
        movdqa    %xmm0, -304(%rbp)                             #4607.17
..LN4341:
   .loc    3  4608
        movdqa    -320(%rbp), %xmm0                             #4608.22
        movdqa    -304(%rbp), %xmm1                             #4608.22
        pcmpeqd   %xmm1, %xmm0                                  #4608.22
..LN4343:
        movdqa    %xmm0, -320(%rbp)                             #4608.17
..LN4345:
   .loc    3  4609
        movdqa    -320(%rbp), %xmm0                             #4609.42
..LN4347:
        pmovmskb  %xmm0, %eax                                   #4609.22
..LN4349:
        movl      %eax, -72(%rbp)                               #4609.17
..LN4351:
   .loc    3  4610
        movl      -72(%rbp), %eax                               #4610.17
        andl      $3855, %eax                                   #4610.17
        shll      $4, %eax                                      #4610.17
        andl      -72(%rbp), %eax                               #4610.17
        movl      %eax, -72(%rbp)                               #4610.17
        movl      -72(%rbp), %eax                               #4610.17
        sarl      $4, %eax                                      #4610.17
        orl       -72(%rbp), %eax                               #4610.17
        movl      %eax, -72(%rbp)                               #4610.17
..LN4353:
   .loc    3  4614
        andl      $-4, -16(%rbp)                                #4614.17
..LN4355:
   .loc    3  4617
        movl      -60(%rbp), %eax                               #4617.29
        orl       -56(%rbp), %eax                               #4617.29
..LN4357:
        orl       -64(%rbp), %eax                               #4617.34
..LN4359:
        orl       -72(%rbp), %eax                               #4617.39
..LN4361:
        je        ..B27.32      # Prob 50%                      #4617.47
                                # LOE
..B27.16:                       # Preds ..B27.15
..LN4363:
   .loc    3  4635
        movl      -56(%rbp), %eax                               #4635.58
..LN4365:
        movl      %eax, %edi                                    #4635.30
        call      uint16_lsbit@PLT                              #4635.30
                                # LOE eax
..B27.47:                       # Preds ..B27.16
        movl      %eax, -112(%rbp)                              #4635.30
                                # LOE
..B27.17:                       # Preds ..B27.47
        movl      -112(%rbp), %eax                              #4635.30
        movswq    %ax, %rax                                     #4635.30
..LN4367:
        movl      %eax, -128(%rbp)                              #4635.28
..LN4369:
   .loc    3  4636
        movl      -60(%rbp), %eax                               #4636.58
..LN4371:
        movl      %eax, %edi                                    #4636.30
        call      uint16_lsbit@PLT                              #4636.30
                                # LOE eax
..B27.48:                       # Preds ..B27.17
        movl      %eax, -108(%rbp)                              #4636.30
                                # LOE
..B27.18:                       # Preds ..B27.48
        movl      -108(%rbp), %eax                              #4636.30
        movswq    %ax, %rax                                     #4636.30
..LN4373:
        movl      %eax, -124(%rbp)                              #4636.28
..LN4375:
   .loc    3  4637
        movl      -64(%rbp), %eax                               #4637.58
..LN4377:
        movl      %eax, %edi                                    #4637.30
        call      uint16_lsbit@PLT                              #4637.30
                                # LOE eax
..B27.49:                       # Preds ..B27.18
        movl      %eax, -104(%rbp)                              #4637.30
                                # LOE
..B27.19:                       # Preds ..B27.49
        movl      -104(%rbp), %eax                              #4637.30
        movswq    %ax, %rax                                     #4637.30
..LN4379:
        movl      %eax, -120(%rbp)                              #4637.28
..LN4381:
   .loc    3  4638
        movl      -72(%rbp), %eax                               #4638.58
..LN4383:
        movl      %eax, %edi                                    #4638.30
        call      uint16_lsbit@PLT                              #4638.30
                                # LOE eax
..B27.50:                       # Preds ..B27.19
        movl      %eax, -100(%rbp)                              #4638.30
                                # LOE
..B27.20:                       # Preds ..B27.50
        movl      -100(%rbp), %eax                              #4638.30
        movswq    %ax, %rax                                     #4638.30
..LN4385:
        movl      %eax, -116(%rbp)                              #4638.28
..LN4387:
   .loc    3  4643
        movl      -128(%rbp), %eax                              #4643.28
..LN4389:
        addl      %eax, %eax                                    #4643.34
..LN4391:
        movl      %eax, -128(%rbp)                              #4643.21
..LN4393:
   .loc    3  4644
        movl      -124(%rbp), %eax                              #4644.28
..LN4395:
        lea       1(%rax,%rax), %eax                            #4644.40
..LN4397:
        movl      %eax, -124(%rbp)                              #4644.21
..LN4399:
   .loc    3  4645
        movl      -120(%rbp), %eax                              #4645.28
..LN4401:
        lea       2(%rax,%rax), %eax                            #4645.40
..LN4403:
        movl      %eax, -120(%rbp)                              #4645.21
..LN4405:
   .loc    3  4646
        movl      -116(%rbp), %eax                              #4646.28
..LN4407:
        lea       3(%rax,%rax), %eax                            #4646.40
..LN4409:
        movl      %eax, -116(%rbp)                              #4646.21
..LN4411:
   .loc    3  4650
        movl      -56(%rbp), %eax                               #4650.26
..LN4413:
        testl     %eax, %eax                                    #4650.32
        je        ..B27.23      # Prob 50%                      #4650.32
                                # LOE
..B27.21:                       # Preds ..B27.20
..LN4415:
        movl      -128(%rbp), %eax                              #4650.43
        addl      -16(%rbp), %eax                               #4650.43
..LN4417:
        movl      -68(%rbp), %edx                               #4650.49
        cmpl      %edx, %eax                                    #4650.49
        ja        ..B27.23      # Prob 50%                      #4650.49
                                # LOE
..B27.22:                       # Preds ..B27.21
..LN4419:
        movl      $1, %eax                                      #4650.63
        leave                                                   #4650.63
..___tag_value_eval_4na_64.457:                                 #
        ret                                                     #4650.63
..___tag_value_eval_4na_64.458:                                 #
                                # LOE
..B27.23:                       # Preds ..B27.21 ..B27.20
..LN4421:
   .loc    3  4651
        movl      -60(%rbp), %eax                               #4651.26
..LN4423:
        testl     %eax, %eax                                    #4651.32
        je        ..B27.26      # Prob 50%                      #4651.32
                                # LOE
..B27.24:                       # Preds ..B27.23
..LN4425:
        movl      -124(%rbp), %eax                              #4651.43
        addl      -16(%rbp), %eax                               #4651.43
..LN4427:
        movl      -68(%rbp), %edx                               #4651.49
        cmpl      %edx, %eax                                    #4651.49
        ja        ..B27.26      # Prob 50%                      #4651.49
                                # LOE
..B27.25:                       # Preds ..B27.24
..LN4429:
        movl      $1, %eax                                      #4651.63
        leave                                                   #4651.63
..___tag_value_eval_4na_64.460:                                 #
        ret                                                     #4651.63
..___tag_value_eval_4na_64.461:                                 #
                                # LOE
..B27.26:                       # Preds ..B27.24 ..B27.23
..LN4431:
   .loc    3  4652
        movl      -64(%rbp), %eax                               #4652.26
..LN4433:
        testl     %eax, %eax                                    #4652.32
        je        ..B27.29      # Prob 50%                      #4652.32
                                # LOE
..B27.27:                       # Preds ..B27.26
..LN4435:
        movl      -120(%rbp), %eax                              #4652.43
        addl      -16(%rbp), %eax                               #4652.43
..LN4437:
        movl      -68(%rbp), %edx                               #4652.49
        cmpl      %edx, %eax                                    #4652.49
        ja        ..B27.29      # Prob 50%                      #4652.49
                                # LOE
..B27.28:                       # Preds ..B27.27
..LN4439:
        movl      $1, %eax                                      #4652.63
        leave                                                   #4652.63
..___tag_value_eval_4na_64.463:                                 #
        ret                                                     #4652.63
..___tag_value_eval_4na_64.464:                                 #
                                # LOE
..B27.29:                       # Preds ..B27.27 ..B27.26
..LN4441:
   .loc    3  4653
        movl      -72(%rbp), %eax                               #4653.26
..LN4443:
        testl     %eax, %eax                                    #4653.32
        je        ..B27.32      # Prob 50%                      #4653.32
                                # LOE
..B27.30:                       # Preds ..B27.29
..LN4445:
        movl      -116(%rbp), %eax                              #4653.43
        addl      -16(%rbp), %eax                               #4653.43
..LN4447:
        movl      -68(%rbp), %edx                               #4653.49
        cmpl      %edx, %eax                                    #4653.49
        ja        ..B27.32      # Prob 50%                      #4653.49
                                # LOE
..B27.31:                       # Preds ..B27.30
..LN4449:
        movl      $1, %eax                                      #4653.63
        leave                                                   #4653.63
..___tag_value_eval_4na_64.466:                                 #
        ret                                                     #4653.63
..___tag_value_eval_4na_64.467:                                 #
                                # LOE
..B27.32:                       # Preds ..B27.30 ..B27.29 ..B27.15
..LN4451:
   .loc    3  4658
        addl      $4, -16(%rbp)                                 #4658.17
..LN4453:
   .loc    3  4661
        movl      -16(%rbp), %eax                               #4661.22
..LN4455:
        movl      -68(%rbp), %edx                               #4661.28
        cmpl      %edx, %eax                                    #4661.28
        jbe       ..B27.34      # Prob 50%                      #4661.28
                                # LOE
..B27.33:                       # Preds ..B27.32
..LN4457:
   .loc    3  4662
        xorl      %eax, %eax                                    #4662.28
        leave                                                   #4662.28
..___tag_value_eval_4na_64.469:                                 #
        ret                                                     #4662.28
..___tag_value_eval_4na_64.470:                                 #
                                # LOE
..B27.34:                       # Preds ..B27.32
..LN4459:
   .loc    3  4665
        movl      -52(%rbp), %eax                               #4665.25
        decl      %eax                                          #4665.25
        movl      %eax, -52(%rbp)                               #4665.25
..LN4461:
        je        ..B27.39      # Prob 50%                      #4665.39
                                # LOE
..B27.35:                       # Preds ..B27.34
..LN4463:
   .loc    3  4670
        movdqa    -288(%rbp), %xmm0                             #4670.43
..LN4465:
        psrldq    $2, %xmm0                                     #4670.26
..LN4467:
        movdqa    %xmm0, -288(%rbp)                             #4670.17
..LN4469:
   .loc    3  4673
        movq      -80(%rbp), %rax                               #4673.22
..LN4471:
        movq      -88(%rbp), %rdx                               #4673.26
        cmpq      %rdx, %rax                                    #4673.26
        jae       ..B27.37      # Prob 50%                      #4673.26
                                # LOE
..B27.36:                       # Preds ..B27.35
..LN4473:
   .loc    3  4674
        movdqa    -288(%rbp), %xmm0                             #4674.49
..LN4475:
        movq      -80(%rbp), %rax                               #4674.72
        movzbl    (%rax), %eax                                  #4674.72
        movzbl    %al, %eax                                     #4674.72
..LN4477:
        movslq    %eax, %rax                                    #4674.57
        lea       expand_2na.0(%rip), %rdx                      #4674.57
        movzwl    (%rdx,%rax,2), %eax                           #4674.57
        movzwl    %ax, %eax                                     #4674.57
..LN4479:
        pinsrw    $7, %eax, %xmm0                               #4674.30
..LN4481:
        movdqa    %xmm0, -288(%rbp)                             #4674.21
                                # LOE
..B27.37:                       # Preds ..B27.36 ..B27.35
..LN4483:
   .loc    3  4677
        incq      -80(%rbp)                                     #4677.20
        jmp       ..B27.10      # Prob 100%                     #4677.20
                                # LOE
..B27.39:                       # Preds ..B27.34 ..B27.10
..LN4485:
   .loc    3  4682
        movq      -96(%rbp), %rax                               #4682.18
..LN4487:
        movq      -88(%rbp), %rdx                               #4682.25
        cmpq      %rdx, %rax                                    #4682.25
        jae       ..B27.43      # Prob 50%                      #4682.25
                                # LOE
..B27.40:                       # Preds ..B27.39
..LN4489:
   .loc    3  4686
        addl      $16, -16(%rbp)                                #4686.13
..LN4491:
   .loc    3  4687
        movl      -16(%rbp), %eax                               #4687.18
..LN4493:
        movl      -68(%rbp), %edx                               #4687.24
        cmpl      %edx, %eax                                    #4687.24
        ja        ..B27.43      # Prob 50%                      #4687.24
                                # LOE
..B27.41:                       # Preds ..B27.40
..LN4495:
   .loc    3  4692
        movq      -96(%rbp), %rax                               #4692.41
..LN4497:
        movq      -88(%rbp), %rdx                               #4692.46
..LN4499:
        movq      %rax, %rdi                                    #4692.22
        movq      %rdx, %rsi                                    #4692.22
        call      prime_buffer_4na@PLT                          #4692.22
                                # LOE xmm0
..B27.51:                       # Preds ..B27.41
        movdqa    %xmm0, -336(%rbp)                             #4692.22
                                # LOE
..B27.42:                       # Preds ..B27.51
..LN4501:
        movdqa    -336(%rbp), %xmm0                             #4692.13
        movdqa    %xmm0, -288(%rbp)                             #4692.13
..LN4503:
   .loc    3  4707
        addq      $8, -96(%rbp)                                 #4707.13
..LN4505:
   .loc    3  4709
        movq      -96(%rbp), %rax                               #4709.17
..LN4507:
        movq      %rax, -80(%rbp)                               #4709.13
        jmp       ..B27.8       # Prob 100%                     #4709.13
                                # LOE
..B27.43:                       # Preds ..B27.40 ..B27.39 ..B27.8
..LN4509:
   .loc    3  4718
        xorl      %eax, %eax                                    #4718.12
        leave                                                   #4718.12
..___tag_value_eval_4na_64.472:                                 #
        ret                                                     #4718.12
        .align    2,0x90
..___tag_value_eval_4na_64.473:                                 #
                                # LOE
# mark_end;
	.type	eval_4na_64,@function
	.size	eval_4na_64,.-eval_4na_64
.LNeval_4na_64:
	.data
# -- End  eval_4na_64
	.text
# -- Begin  eval_4na_128
# mark_begin;
       .align    2,0x90
eval_4na_128:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B28.1:                        # Preds ..B28.0
..___tag_value_eval_4na_128.476:                                #
..LN4511:
   .loc    3  4724
        pushq     %rbp                                          #4724.1
        movq      %rsp, %rbp                                    #4724.1
..___tag_value_eval_4na_128.477:                                #
        subq      $320, %rsp                                    #4724.1
        movq      %rdi, -40(%rbp)                               #4724.1
        movq      %rsi, -32(%rbp)                               #4724.1
        movl      %edx, -16(%rbp)                               #4724.1
        movl      %ecx, -8(%rbp)                                #4724.1
..LN4513:
   .loc    3  4750
        movq      -40(%rbp), %rax                               #4750.25
        movl      4(%rax), %eax                                 #4750.25
..LN4515:
        movl      %eax, -24(%rbp)                               #4750.23
..LN4517:
   .loc    3  4753
        movl      -8(%rbp), %eax                                #4753.5
        movl      -24(%rbp), %edx                               #4753.5
        cmpl      %edx, %eax                                    #4753.5
        jae       ..B28.3       # Prob 50%                      #4753.5
                                # LOE
..B28.2:                        # Preds ..B28.1
        lea       _2__STRING.9.0(%rip), %rax                    #4753.5
        lea       _2__STRING.1.0(%rip), %rdx                    #4753.5
        lea       __$Ue.0(%rip), %rcx                           #4753.5
        movq      %rax, %rdi                                    #4753.5
        movq      %rdx, %rsi                                    #4753.5
        movl      $4753, %edx                                   #4753.5
        call      __assert_fail@PLT                             #4753.5
                                # LOE
..B28.3:                        # Preds ..B28.1
..LN4519:
   .loc    3  4757
        movl      -16(%rbp), %eax                               #4757.12
..LN4521:
        addl      -8(%rbp), %eax                                #4757.5
        movl      %eax, -8(%rbp)                                #4757.5
..LN4523:
   .loc    3  4760
        movl      -16(%rbp), %eax                               #4760.42
..LN4525:
        shrl      $2, %eax                                      #4760.49
..LN4527:
        addq      -32(%rbp), %rax                               #4760.30
..LN4529:
        movq      %rax, -88(%rbp)                               #4760.5
..LN4531:
   .loc    3  4763
        movl      -24(%rbp), %eax                               #4763.18
        negl      %eax                                          #4763.18
        addl      -8(%rbp), %eax                                #4763.18
..LN4533:
        movl      %eax, -64(%rbp)                               #4763.5
..LN4535:
   .loc    3  4766
        movl      -8(%rbp), %eax                                #4766.44
..LN4537:
        addl      $3, %eax                                      #4766.50
..LN4539:
        shrl      $2, %eax                                      #4766.57
..LN4541:
        addq      -32(%rbp), %rax                               #4766.30
..LN4543:
        movq      %rax, -80(%rbp)                               #4766.5
..LN4545:
   .loc    3  4769
        movq      -88(%rbp), %rax                               #4769.33
..LN4547:
        movq      -80(%rbp), %rdx                               #4769.38
..LN4549:
        movq      %rax, %rdi                                    #4769.14
        movq      %rdx, %rsi                                    #4769.14
        call      prime_buffer_4na@PLT                          #4769.14
                                # LOE xmm0
..B28.41:                       # Preds ..B28.3
        movdqa    %xmm0, -112(%rbp)                             #4769.14
                                # LOE
..B28.4:                        # Preds ..B28.41
..LN4551:
        movdqa    -112(%rbp), %xmm0                             #4769.5
        movdqa    %xmm0, -256(%rbp)                             #4769.5
..LN4553:
   .loc    3  4770
        addq      $8, -88(%rbp)                                 #4770.5
..LN4555:
   .loc    3  4772
        movq      -88(%rbp), %rax                               #4772.9
..LN4557:
        movq      %rax, -72(%rbp)                               #4772.5
..LN4559:
   .loc    3  4776
        movq      -40(%rbp), %rax                               #4776.5
        movdqa    16(%rax), %xmm0                               #4776.5
        movdqa    %xmm0, -240(%rbp)                             #4776.5
        movq      -40(%rbp), %rax                               #4776.5
        movdqa    32(%rax), %xmm0                               #4776.5
        movdqa    %xmm0, -224(%rbp)                             #4776.5
        movq      -40(%rbp), %rax                               #4776.5
        movdqa    48(%rax), %xmm0                               #4776.5
        movdqa    %xmm0, -208(%rbp)                             #4776.5
        movq      -40(%rbp), %rax                               #4776.5
        movdqa    64(%rax), %xmm0                               #4776.5
        movdqa    %xmm0, -192(%rbp)                             #4776.5
        movq      -40(%rbp), %rax                               #4776.5
        movdqa    80(%rax), %xmm0                               #4776.5
        movdqa    %xmm0, -176(%rbp)                             #4776.5
        movq      -40(%rbp), %rax                               #4776.5
        movdqa    96(%rax), %xmm0                               #4776.5
        movdqa    %xmm0, -160(%rbp)                             #4776.5
        movq      -40(%rbp), %rax                               #4776.5
        movdqa    112(%rax), %xmm0                              #4776.5
        movdqa    %xmm0, -144(%rbp)                             #4776.5
        movq      -40(%rbp), %rax                               #4776.5
        movdqa    128(%rax), %xmm0                              #4776.5
        movdqa    %xmm0, -128(%rbp)                             #4776.5
..LN4561:
   .loc    3  4779
        xorl      %eax, %eax                                    #4779.15
        movl      %eax, -60(%rbp)                               #4779.15
..LN4563:
        movl      %eax, -56(%rbp)                               #4779.10
..LN4565:
        movl      %eax, -52(%rbp)                               #4779.5
..LN4567:
   .loc    3  4781
        movl      -16(%rbp), %eax                               #4781.27
        negl      %eax                                          #4781.27
..LN4569:
        movl      -64(%rbp), %edx                               #4781.20
..LN4571:
        lea       7(%rdx,%rax), %eax                            #4781.33
..LN4573:
        shrl      $2, %eax                                      #4781.40
..LN4575:
        movl      %eax, -48(%rbp)                               #4781.5
..LN4577:
   .loc    3  4790
        movl      -16(%rbp), %eax                               #4790.14
..LN4579:
        andl      $3, %eax                                      #4790.20
        movl      %eax, -44(%rbp)                               #4790.20
        je        ..B28.12      # Prob 50%                      #4790.20
                                # LOE
..B28.5:                        # Preds ..B28.4
        movl      -44(%rbp), %eax                               #4790.20
        cmpl      $1, %eax                                      #4790.20
        je        ..B28.13      # Prob 50%                      #4790.20
                                # LOE
..B28.6:                        # Preds ..B28.5
        movl      -44(%rbp), %eax                               #4790.20
        cmpl      $2, %eax                                      #4790.20
        je        ..B28.14      # Prob 50%                      #4790.20
                                # LOE
..B28.7:                        # Preds ..B28.6
        movl      -44(%rbp), %eax                               #4790.20
        cmpl      $3, %eax                                      #4790.20
        je        ..B28.15      # Prob 50%                      #4790.20
                                # LOE
..B28.8:                        # Preds ..B28.37 ..B28.7
..LN4581:
   .loc    3  4795
        movl      $1, %eax                                      #4795.9
        testl     %eax, %eax                                    #4795.9
        je        ..B28.38      # Prob 100%                     #4795.9
                                # LOE
..B28.9:                        # Preds ..B28.8
..LN4583:
   .loc    3  4797
        movl      $8, -48(%rbp)                                 #4797.13
                                # LOE
..B28.10:                       # Preds ..B28.32 ..B28.9
..LN4585:
   .loc    3  4800
        movl      $1, %eax                                      #4800.13
        testl     %eax, %eax                                    #4800.13
        je        ..B28.34      # Prob 100%                     #4800.13
                                # LOE
..B28.12:                       # Preds ..B28.4 ..B28.10
..LN4587:
   .loc    3  4807
        movdqa    -256(%rbp), %xmm0                             #4807.38
..LN4589:
        movdqa    -240(%rbp), %xmm1                             #4807.46
..LN4591:
        pand      %xmm1, %xmm0                                  #4807.22
..LN4593:
        movdqa    %xmm0, -304(%rbp)                             #4807.17
..LN4595:
   .loc    3  4808
        movdqa    -256(%rbp), %xmm0                             #4808.38
..LN4597:
        movdqa    -224(%rbp), %xmm1                             #4808.46
..LN4599:
        pand      %xmm1, %xmm0                                  #4808.22
..LN4601:
        movdqa    %xmm0, -288(%rbp)                             #4808.17
..LN4603:
   .loc    3  4809
        movdqa    -304(%rbp), %xmm0                             #4809.22
        movdqa    -288(%rbp), %xmm1                             #4809.22
        pcmpeqd   %xmm1, %xmm0                                  #4809.22
..LN4605:
        movdqa    %xmm0, -304(%rbp)                             #4809.17
..LN4607:
   .loc    3  4810
        movdqa    -304(%rbp), %xmm0                             #4810.42
..LN4609:
        pmovmskb  %xmm0, %eax                                   #4810.22
..LN4611:
        movl      %eax, -52(%rbp)                               #4810.17
..LN4613:
   .loc    3  4811
        movl      -52(%rbp), %eax                               #4811.17
        incl      %eax                                          #4811.17
        sarl      $16, %eax                                     #4811.17
        negl      %eax                                          #4811.17
        movl      %eax, -52(%rbp)                               #4811.17
                                # LOE
..B28.13:                       # Preds ..B28.5 ..B28.12
..LN4615:
   .loc    3  4814
        movdqa    -256(%rbp), %xmm0                             #4814.38
..LN4617:
        movdqa    -208(%rbp), %xmm1                             #4814.46
..LN4619:
        pand      %xmm1, %xmm0                                  #4814.22
..LN4621:
        movdqa    %xmm0, -304(%rbp)                             #4814.17
..LN4623:
   .loc    3  4815
        movdqa    -256(%rbp), %xmm0                             #4815.38
..LN4625:
        movdqa    -192(%rbp), %xmm1                             #4815.46
..LN4627:
        pand      %xmm1, %xmm0                                  #4815.22
..LN4629:
        movdqa    %xmm0, -288(%rbp)                             #4815.17
..LN4631:
   .loc    3  4816
        movdqa    -304(%rbp), %xmm0                             #4816.22
        movdqa    -288(%rbp), %xmm1                             #4816.22
        pcmpeqd   %xmm1, %xmm0                                  #4816.22
..LN4633:
        movdqa    %xmm0, -304(%rbp)                             #4816.17
..LN4635:
   .loc    3  4817
        movdqa    -304(%rbp), %xmm0                             #4817.42
..LN4637:
        pmovmskb  %xmm0, %eax                                   #4817.22
..LN4639:
        movl      %eax, -56(%rbp)                               #4817.17
..LN4641:
   .loc    3  4818
        movl      -56(%rbp), %eax                               #4818.17
        incl      %eax                                          #4818.17
        sarl      $16, %eax                                     #4818.17
        negl      %eax                                          #4818.17
        movl      %eax, -56(%rbp)                               #4818.17
                                # LOE
..B28.14:                       # Preds ..B28.6 ..B28.13
..LN4643:
   .loc    3  4821
        movdqa    -256(%rbp), %xmm0                             #4821.38
..LN4645:
        movdqa    -176(%rbp), %xmm1                             #4821.46
..LN4647:
        pand      %xmm1, %xmm0                                  #4821.22
..LN4649:
        movdqa    %xmm0, -304(%rbp)                             #4821.17
..LN4651:
   .loc    3  4822
        movdqa    -256(%rbp), %xmm0                             #4822.38
..LN4653:
        movdqa    -160(%rbp), %xmm1                             #4822.46
..LN4655:
        pand      %xmm1, %xmm0                                  #4822.22
..LN4657:
        movdqa    %xmm0, -288(%rbp)                             #4822.17
..LN4659:
   .loc    3  4823
        movdqa    -304(%rbp), %xmm0                             #4823.22
        movdqa    -288(%rbp), %xmm1                             #4823.22
        pcmpeqd   %xmm1, %xmm0                                  #4823.22
..LN4661:
        movdqa    %xmm0, -304(%rbp)                             #4823.17
..LN4663:
   .loc    3  4824
        movdqa    -304(%rbp), %xmm0                             #4824.42
..LN4665:
        pmovmskb  %xmm0, %eax                                   #4824.22
..LN4667:
        movl      %eax, -60(%rbp)                               #4824.17
..LN4669:
   .loc    3  4825
        movl      -60(%rbp), %eax                               #4825.17
        incl      %eax                                          #4825.17
        sarl      $16, %eax                                     #4825.17
        negl      %eax                                          #4825.17
        movl      %eax, -60(%rbp)                               #4825.17
                                # LOE
..B28.15:                       # Preds ..B28.7 ..B28.14
..LN4671:
   .loc    3  4828
        movdqa    -256(%rbp), %xmm0                             #4828.38
..LN4673:
        movdqa    -144(%rbp), %xmm1                             #4828.46
..LN4675:
        pand      %xmm1, %xmm0                                  #4828.22
..LN4677:
        movdqa    %xmm0, -304(%rbp)                             #4828.17
..LN4679:
   .loc    3  4829
        movdqa    -256(%rbp), %xmm0                             #4829.38
..LN4681:
        movdqa    -128(%rbp), %xmm1                             #4829.46
..LN4683:
        pand      %xmm1, %xmm0                                  #4829.22
..LN4685:
        movdqa    %xmm0, -288(%rbp)                             #4829.17
..LN4687:
   .loc    3  4830
        movdqa    -304(%rbp), %xmm0                             #4830.22
        movdqa    -288(%rbp), %xmm1                             #4830.22
        pcmpeqd   %xmm1, %xmm0                                  #4830.22
..LN4689:
        movdqa    %xmm0, -304(%rbp)                             #4830.17
..LN4691:
   .loc    3  4831
        movdqa    -304(%rbp), %xmm0                             #4831.42
..LN4693:
        pmovmskb  %xmm0, %eax                                   #4831.22
..LN4695:
        movl      %eax, -96(%rbp)                               #4831.17
..LN4697:
   .loc    3  4832
        movl      -96(%rbp), %eax                               #4832.17
        incl      %eax                                          #4832.17
        sarl      $16, %eax                                     #4832.17
        negl      %eax                                          #4832.17
        movl      %eax, -96(%rbp)                               #4832.17
..LN4699:
   .loc    3  4836
        andl      $-4, -16(%rbp)                                #4836.17
..LN4701:
   .loc    3  4839
        movl      -56(%rbp), %eax                               #4839.29
        orl       -52(%rbp), %eax                               #4839.29
..LN4703:
        orl       -60(%rbp), %eax                               #4839.34
..LN4705:
        orl       -96(%rbp), %eax                               #4839.39
..LN4707:
        je        ..B28.27      # Prob 50%                      #4839.47
                                # LOE
..B28.16:                       # Preds ..B28.15
..LN4709:
   .loc    3  4842
        movl      -16(%rbp), %eax                               #4842.37
        negl      %eax                                          #4842.37
        addl      -64(%rbp), %eax                               #4842.37
        movl      %eax, -272(%rbp)                              #4842.37
        cmpl      $2, %eax                                      #4842.37
        je        ..B28.20      # Prob 50%                      #4842.37
                                # LOE
..B28.17:                       # Preds ..B28.16
        movl      -272(%rbp), %eax                              #4842.37
        cmpl      $1, %eax                                      #4842.37
        je        ..B28.22      # Prob 50%                      #4842.37
                                # LOE
..B28.18:                       # Preds ..B28.17
        movl      -272(%rbp), %eax                              #4842.37
        testl     %eax, %eax                                    #4842.37
        je        ..B28.24      # Prob 50%                      #4842.37
                                # LOE
..B28.19:                       # Preds ..B28.18
..LN4711:
   .loc    3  4845
        movl      $1, %eax                                      #4845.32
        leave                                                   #4845.32
..___tag_value_eval_4na_128.481:                                #
        ret                                                     #4845.32
..___tag_value_eval_4na_128.482:                                #
                                # LOE
..B28.20:                       # Preds ..B28.16
..LN4713:
   .loc    3  4847
        movl      -60(%rbp), %eax                               #4847.30
..LN4715:
        testl     %eax, %eax                                    #4847.36
        je        ..B28.22      # Prob 50%                      #4847.36
                                # LOE
..B28.21:                       # Preds ..B28.20
..LN4717:
        movl      $1, %eax                                      #4847.47
        leave                                                   #4847.47
..___tag_value_eval_4na_128.484:                                #
        ret                                                     #4847.47
..___tag_value_eval_4na_128.485:                                #
                                # LOE
..B28.22:                       # Preds ..B28.20 ..B28.17
..LN4719:
   .loc    3  4849
        movl      -56(%rbp), %eax                               #4849.30
..LN4721:
        testl     %eax, %eax                                    #4849.36
        je        ..B28.24      # Prob 50%                      #4849.36
                                # LOE
..B28.23:                       # Preds ..B28.22
..LN4723:
        movl      $1, %eax                                      #4849.47
        leave                                                   #4849.47
..___tag_value_eval_4na_128.487:                                #
        ret                                                     #4849.47
..___tag_value_eval_4na_128.488:                                #
                                # LOE
..B28.24:                       # Preds ..B28.22 ..B28.18
..LN4725:
   .loc    3  4851
        movl      -52(%rbp), %eax                               #4851.30
..LN4727:
        testl     %eax, %eax                                    #4851.36
        je        ..B28.26      # Prob 50%                      #4851.36
                                # LOE
..B28.25:                       # Preds ..B28.24
..LN4729:
        movl      $1, %eax                                      #4851.47
        leave                                                   #4851.47
..___tag_value_eval_4na_128.490:                                #
        ret                                                     #4851.47
..___tag_value_eval_4na_128.491:                                #
                                # LOE
..B28.26:                       # Preds ..B28.24
..LN4731:
   .loc    3  4853
        xorl      %eax, %eax                                    #4853.28
        leave                                                   #4853.28
..___tag_value_eval_4na_128.493:                                #
        ret                                                     #4853.28
..___tag_value_eval_4na_128.494:                                #
                                # LOE
..B28.27:                       # Preds ..B28.15
..LN4733:
   .loc    3  4880
        addl      $4, -16(%rbp)                                 #4880.17
..LN4735:
   .loc    3  4883
        movl      -16(%rbp), %eax                               #4883.22
..LN4737:
        movl      -64(%rbp), %edx                               #4883.28
        cmpl      %edx, %eax                                    #4883.28
        jbe       ..B28.29      # Prob 50%                      #4883.28
                                # LOE
..B28.28:                       # Preds ..B28.27
..LN4739:
   .loc    3  4884
        xorl      %eax, %eax                                    #4884.28
        leave                                                   #4884.28
..___tag_value_eval_4na_128.496:                                #
        ret                                                     #4884.28
..___tag_value_eval_4na_128.497:                                #
                                # LOE
..B28.29:                       # Preds ..B28.27
..LN4741:
   .loc    3  4887
        movl      -48(%rbp), %eax                               #4887.25
        decl      %eax                                          #4887.25
        movl      %eax, -48(%rbp)                               #4887.25
..LN4743:
        je        ..B28.34      # Prob 50%                      #4887.39
                                # LOE
..B28.30:                       # Preds ..B28.29
..LN4745:
   .loc    3  4892
        movdqa    -256(%rbp), %xmm0                             #4892.43
..LN4747:
        psrldq    $2, %xmm0                                     #4892.26
..LN4749:
        movdqa    %xmm0, -256(%rbp)                             #4892.17
..LN4751:
   .loc    3  4895
        movq      -72(%rbp), %rax                               #4895.22
..LN4753:
        movq      -80(%rbp), %rdx                               #4895.26
        cmpq      %rdx, %rax                                    #4895.26
        jae       ..B28.32      # Prob 50%                      #4895.26
                                # LOE
..B28.31:                       # Preds ..B28.30
..LN4755:
   .loc    3  4896
        movdqa    -256(%rbp), %xmm0                             #4896.49
..LN4757:
        movq      -72(%rbp), %rax                               #4896.72
        movzbl    (%rax), %eax                                  #4896.72
        movzbl    %al, %eax                                     #4896.72
..LN4759:
        movslq    %eax, %rax                                    #4896.57
        lea       expand_2na.0(%rip), %rdx                      #4896.57
        movzwl    (%rdx,%rax,2), %eax                           #4896.57
        movzwl    %ax, %eax                                     #4896.57
..LN4761:
        pinsrw    $7, %eax, %xmm0                               #4896.30
..LN4763:
        movdqa    %xmm0, -256(%rbp)                             #4896.21
                                # LOE
..B28.32:                       # Preds ..B28.31 ..B28.30
..LN4765:
   .loc    3  4899
        incq      -72(%rbp)                                     #4899.20
        jmp       ..B28.10      # Prob 100%                     #4899.20
                                # LOE
..B28.34:                       # Preds ..B28.29 ..B28.10
..LN4767:
   .loc    3  4904
        movq      -88(%rbp), %rax                               #4904.18
..LN4769:
        movq      -80(%rbp), %rdx                               #4904.25
        cmpq      %rdx, %rax                                    #4904.25
        jae       ..B28.38      # Prob 50%                      #4904.25
                                # LOE
..B28.35:                       # Preds ..B28.34
..LN4771:
   .loc    3  4909
        movl      -16(%rbp), %eax                               #4909.18
..LN4773:
        movl      -64(%rbp), %edx                               #4909.24
        cmpl      %edx, %eax                                    #4909.24
        ja        ..B28.38      # Prob 50%                      #4909.24
                                # LOE
..B28.36:                       # Preds ..B28.35
..LN4775:
   .loc    3  4914
        movq      -88(%rbp), %rax                               #4914.41
..LN4777:
        movq      -80(%rbp), %rdx                               #4914.46
..LN4779:
        movq      %rax, %rdi                                    #4914.22
        movq      %rdx, %rsi                                    #4914.22
        call      prime_buffer_4na@PLT                          #4914.22
                                # LOE xmm0
..B28.42:                       # Preds ..B28.36
        movdqa    %xmm0, -320(%rbp)                             #4914.22
                                # LOE
..B28.37:                       # Preds ..B28.42
..LN4781:
        movdqa    -320(%rbp), %xmm0                             #4914.13
        movdqa    %xmm0, -256(%rbp)                             #4914.13
..LN4783:
   .loc    3  4929
        addq      $8, -88(%rbp)                                 #4929.13
..LN4785:
   .loc    3  4931
        movq      -88(%rbp), %rax                               #4931.17
..LN4787:
        movq      %rax, -72(%rbp)                               #4931.13
        jmp       ..B28.8       # Prob 100%                     #4931.13
                                # LOE
..B28.38:                       # Preds ..B28.35 ..B28.34 ..B28.8
..LN4789:
   .loc    3  4940
        xorl      %eax, %eax                                    #4940.12
        leave                                                   #4940.12
..___tag_value_eval_4na_128.499:                                #
        ret                                                     #4940.12
        .align    2,0x90
..___tag_value_eval_4na_128.500:                                #
                                # LOE
# mark_end;
	.type	eval_4na_128,@function
	.size	eval_4na_128,.-eval_4na_128
.LNeval_4na_128:
	.data
# -- End  eval_4na_128
	.text
# -- Begin  eval_4na_pos
# mark_begin;
       .align    2,0x90
eval_4na_pos:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B29.1:                        # Preds ..B29.0
..___tag_value_eval_4na_pos.503:                                #
..LN4791:
   .loc    3  4946
        pushq     %rbp                                          #4946.1
        movq      %rsp, %rbp                                    #4946.1
..___tag_value_eval_4na_pos.504:                                #
        subq      $336, %rsp                                    #4946.1
        movq      %rdi, -48(%rbp)                               #4946.1
        movq      %rsi, -40(%rbp)                               #4946.1
        movl      %edx, -24(%rbp)                               #4946.1
        movl      %ecx, -16(%rbp)                               #4946.1
..LN4793:
   .loc    3  4978
        movq      -48(%rbp), %rax                               #4978.25
        movl      4(%rax), %eax                                 #4978.25
..LN4795:
        movl      %eax, -32(%rbp)                               #4978.23
..LN4797:
   .loc    3  4981
        movl      -16(%rbp), %eax                               #4981.5
        movl      -32(%rbp), %edx                               #4981.5
        cmpl      %edx, %eax                                    #4981.5
        jae       ..B29.3       # Prob 50%                      #4981.5
                                # LOE
..B29.2:                        # Preds ..B29.1
        lea       _2__STRING.9.0(%rip), %rax                    #4981.5
        lea       _2__STRING.1.0(%rip), %rdx                    #4981.5
        lea       __$Uf.0(%rip), %rcx                           #4981.5
        movq      %rax, %rdi                                    #4981.5
        movq      %rdx, %rsi                                    #4981.5
        movl      $4981, %edx                                   #4981.5
        call      __assert_fail@PLT                             #4981.5
                                # LOE
..B29.3:                        # Preds ..B29.1
..LN4799:
   .loc    3  4985
        movl      -24(%rbp), %eax                               #4985.12
..LN4801:
        addl      -16(%rbp), %eax                               #4985.5
        movl      %eax, -16(%rbp)                               #4985.5
..LN4803:
   .loc    3  4989
        movl      -24(%rbp), %eax                               #4989.13
..LN4805:
        movl      %eax, -80(%rbp)                               #4989.5
..LN4807:
   .loc    3  4993
        movl      -24(%rbp), %eax                               #4993.42
..LN4809:
        shrl      $2, %eax                                      #4993.49
..LN4811:
        addq      -40(%rbp), %rax                               #4993.30
..LN4813:
        movq      %rax, -104(%rbp)                              #4993.5
..LN4815:
   .loc    3  4996
        movl      -32(%rbp), %eax                               #4996.18
        negl      %eax                                          #4996.18
        addl      -16(%rbp), %eax                               #4996.18
..LN4817:
        movl      %eax, -76(%rbp)                               #4996.5
..LN4819:
   .loc    3  4999
        movl      -16(%rbp), %eax                               #4999.44
..LN4821:
        addl      $3, %eax                                      #4999.50
..LN4823:
        shrl      $2, %eax                                      #4999.57
..LN4825:
        addq      -40(%rbp), %rax                               #4999.30
..LN4827:
        movq      %rax, -96(%rbp)                               #4999.5
..LN4829:
   .loc    3  5002
        movq      -104(%rbp), %rax                              #5002.33
..LN4831:
        movq      -96(%rbp), %rdx                               #5002.38
..LN4833:
        movq      %rax, %rdi                                    #5002.14
        movq      %rdx, %rsi                                    #5002.14
        call      prime_buffer_4na@PLT                          #5002.14
                                # LOE xmm0
..B29.54:                       # Preds ..B29.3
        movdqa    %xmm0, -128(%rbp)                             #5002.14
                                # LOE
..B29.4:                        # Preds ..B29.54
..LN4835:
        movdqa    -128(%rbp), %xmm0                             #5002.5
        movdqa    %xmm0, -272(%rbp)                             #5002.5
..LN4837:
   .loc    3  5003
        addq      $8, -104(%rbp)                                #5003.5
..LN4839:
   .loc    3  5005
        movq      -104(%rbp), %rax                              #5005.9
..LN4841:
        movq      %rax, -88(%rbp)                               #5005.5
..LN4843:
   .loc    3  5009
        movq      -48(%rbp), %rax                               #5009.5
        movdqa    16(%rax), %xmm0                               #5009.5
        movdqa    %xmm0, -256(%rbp)                             #5009.5
        movq      -48(%rbp), %rax                               #5009.5
        movdqa    32(%rax), %xmm0                               #5009.5
        movdqa    %xmm0, -240(%rbp)                             #5009.5
        movq      -48(%rbp), %rax                               #5009.5
        movdqa    48(%rax), %xmm0                               #5009.5
        movdqa    %xmm0, -224(%rbp)                             #5009.5
        movq      -48(%rbp), %rax                               #5009.5
        movdqa    64(%rax), %xmm0                               #5009.5
        movdqa    %xmm0, -208(%rbp)                             #5009.5
        movq      -48(%rbp), %rax                               #5009.5
        movdqa    80(%rax), %xmm0                               #5009.5
        movdqa    %xmm0, -192(%rbp)                             #5009.5
        movq      -48(%rbp), %rax                               #5009.5
        movdqa    96(%rax), %xmm0                               #5009.5
        movdqa    %xmm0, -176(%rbp)                             #5009.5
        movq      -48(%rbp), %rax                               #5009.5
        movdqa    112(%rax), %xmm0                              #5009.5
        movdqa    %xmm0, -160(%rbp)                             #5009.5
        movq      -48(%rbp), %rax                               #5009.5
        movdqa    128(%rax), %xmm0                              #5009.5
        movdqa    %xmm0, -144(%rbp)                             #5009.5
..LN4845:
   .loc    3  5012
        xorl      %eax, %eax                                    #5012.15
        movl      %eax, -72(%rbp)                               #5012.15
..LN4847:
        movl      %eax, -68(%rbp)                               #5012.10
..LN4849:
        movl      %eax, -64(%rbp)                               #5012.5
..LN4851:
   .loc    3  5014
        movl      -24(%rbp), %eax                               #5014.27
        negl      %eax                                          #5014.27
..LN4853:
        movl      -76(%rbp), %edx                               #5014.20
..LN4855:
        lea       7(%rdx,%rax), %eax                            #5014.33
..LN4857:
        shrl      $2, %eax                                      #5014.40
..LN4859:
        movl      %eax, -60(%rbp)                               #5014.5
..LN4861:
   .loc    3  5023
        movl      -24(%rbp), %eax                               #5023.14
..LN4863:
        andl      $3, %eax                                      #5023.20
        movl      %eax, -56(%rbp)                               #5023.20
        je        ..B29.12      # Prob 50%                      #5023.20
                                # LOE
..B29.5:                        # Preds ..B29.4
        movl      -56(%rbp), %eax                               #5023.20
        cmpl      $1, %eax                                      #5023.20
        je        ..B29.13      # Prob 50%                      #5023.20
                                # LOE
..B29.6:                        # Preds ..B29.5
        movl      -56(%rbp), %eax                               #5023.20
        cmpl      $2, %eax                                      #5023.20
        je        ..B29.14      # Prob 50%                      #5023.20
                                # LOE
..B29.7:                        # Preds ..B29.6
        movl      -56(%rbp), %eax                               #5023.20
        cmpl      $3, %eax                                      #5023.20
        je        ..B29.15      # Prob 50%                      #5023.20
                                # LOE
..B29.8:                        # Preds ..B29.50 ..B29.7
..LN4865:
   .loc    3  5028
        movl      $1, %eax                                      #5028.9
        testl     %eax, %eax                                    #5028.9
        je        ..B29.51      # Prob 100%                     #5028.9
                                # LOE
..B29.9:                        # Preds ..B29.8
..LN4867:
   .loc    3  5030
        movl      $8, -60(%rbp)                                 #5030.13
                                # LOE
..B29.10:                       # Preds ..B29.45 ..B29.9
..LN4869:
   .loc    3  5033
        movl      $1, %eax                                      #5033.13
        testl     %eax, %eax                                    #5033.13
        je        ..B29.47      # Prob 100%                     #5033.13
                                # LOE
..B29.12:                       # Preds ..B29.4 ..B29.10
..LN4871:
   .loc    3  5040
        movdqa    -272(%rbp), %xmm0                             #5040.38
..LN4873:
        movdqa    -256(%rbp), %xmm1                             #5040.46
..LN4875:
        pand      %xmm1, %xmm0                                  #5040.22
..LN4877:
        movdqa    %xmm0, -320(%rbp)                             #5040.17
..LN4879:
   .loc    3  5041
        movdqa    -272(%rbp), %xmm0                             #5041.38
..LN4881:
        movdqa    -240(%rbp), %xmm1                             #5041.46
..LN4883:
        pand      %xmm1, %xmm0                                  #5041.22
..LN4885:
        movdqa    %xmm0, -304(%rbp)                             #5041.17
..LN4887:
   .loc    3  5042
        movdqa    -320(%rbp), %xmm0                             #5042.22
        movdqa    -304(%rbp), %xmm1                             #5042.22
        pcmpeqd   %xmm1, %xmm0                                  #5042.22
..LN4889:
        movdqa    %xmm0, -320(%rbp)                             #5042.17
..LN4891:
   .loc    3  5043
        movdqa    -320(%rbp), %xmm0                             #5043.42
..LN4893:
        pmovmskb  %xmm0, %eax                                   #5043.22
..LN4895:
        movl      %eax, -64(%rbp)                               #5043.17
..LN4897:
   .loc    3  5044
        movl      -64(%rbp), %eax                               #5044.17
        incl      %eax                                          #5044.17
        sarl      $16, %eax                                     #5044.17
        negl      %eax                                          #5044.17
        movl      %eax, -64(%rbp)                               #5044.17
                                # LOE
..B29.13:                       # Preds ..B29.5 ..B29.12
..LN4899:
   .loc    3  5047
        movdqa    -272(%rbp), %xmm0                             #5047.38
..LN4901:
        movdqa    -224(%rbp), %xmm1                             #5047.46
..LN4903:
        pand      %xmm1, %xmm0                                  #5047.22
..LN4905:
        movdqa    %xmm0, -320(%rbp)                             #5047.17
..LN4907:
   .loc    3  5048
        movdqa    -272(%rbp), %xmm0                             #5048.38
..LN4909:
        movdqa    -208(%rbp), %xmm1                             #5048.46
..LN4911:
        pand      %xmm1, %xmm0                                  #5048.22
..LN4913:
        movdqa    %xmm0, -304(%rbp)                             #5048.17
..LN4915:
   .loc    3  5049
        movdqa    -320(%rbp), %xmm0                             #5049.22
        movdqa    -304(%rbp), %xmm1                             #5049.22
        pcmpeqd   %xmm1, %xmm0                                  #5049.22
..LN4917:
        movdqa    %xmm0, -320(%rbp)                             #5049.17
..LN4919:
   .loc    3  5050
        movdqa    -320(%rbp), %xmm0                             #5050.42
..LN4921:
        pmovmskb  %xmm0, %eax                                   #5050.22
..LN4923:
        movl      %eax, -68(%rbp)                               #5050.17
..LN4925:
   .loc    3  5051
        movl      -68(%rbp), %eax                               #5051.17
        incl      %eax                                          #5051.17
        sarl      $16, %eax                                     #5051.17
        negl      %eax                                          #5051.17
        movl      %eax, -68(%rbp)                               #5051.17
                                # LOE
..B29.14:                       # Preds ..B29.6 ..B29.13
..LN4927:
   .loc    3  5054
        movdqa    -272(%rbp), %xmm0                             #5054.38
..LN4929:
        movdqa    -192(%rbp), %xmm1                             #5054.46
..LN4931:
        pand      %xmm1, %xmm0                                  #5054.22
..LN4933:
        movdqa    %xmm0, -320(%rbp)                             #5054.17
..LN4935:
   .loc    3  5055
        movdqa    -272(%rbp), %xmm0                             #5055.38
..LN4937:
        movdqa    -176(%rbp), %xmm1                             #5055.46
..LN4939:
        pand      %xmm1, %xmm0                                  #5055.22
..LN4941:
        movdqa    %xmm0, -304(%rbp)                             #5055.17
..LN4943:
   .loc    3  5056
        movdqa    -320(%rbp), %xmm0                             #5056.22
        movdqa    -304(%rbp), %xmm1                             #5056.22
        pcmpeqd   %xmm1, %xmm0                                  #5056.22
..LN4945:
        movdqa    %xmm0, -320(%rbp)                             #5056.17
..LN4947:
   .loc    3  5057
        movdqa    -320(%rbp), %xmm0                             #5057.42
..LN4949:
        pmovmskb  %xmm0, %eax                                   #5057.22
..LN4951:
        movl      %eax, -72(%rbp)                               #5057.17
..LN4953:
   .loc    3  5058
        movl      -72(%rbp), %eax                               #5058.17
        incl      %eax                                          #5058.17
        sarl      $16, %eax                                     #5058.17
        negl      %eax                                          #5058.17
        movl      %eax, -72(%rbp)                               #5058.17
                                # LOE
..B29.15:                       # Preds ..B29.7 ..B29.14
..LN4955:
   .loc    3  5061
        movdqa    -272(%rbp), %xmm0                             #5061.38
..LN4957:
        movdqa    -160(%rbp), %xmm1                             #5061.46
..LN4959:
        pand      %xmm1, %xmm0                                  #5061.22
..LN4961:
        movdqa    %xmm0, -320(%rbp)                             #5061.17
..LN4963:
   .loc    3  5062
        movdqa    -272(%rbp), %xmm0                             #5062.38
..LN4965:
        movdqa    -144(%rbp), %xmm1                             #5062.46
..LN4967:
        pand      %xmm1, %xmm0                                  #5062.22
..LN4969:
        movdqa    %xmm0, -304(%rbp)                             #5062.17
..LN4971:
   .loc    3  5063
        movdqa    -320(%rbp), %xmm0                             #5063.22
        movdqa    -304(%rbp), %xmm1                             #5063.22
        pcmpeqd   %xmm1, %xmm0                                  #5063.22
..LN4973:
        movdqa    %xmm0, -320(%rbp)                             #5063.17
..LN4975:
   .loc    3  5064
        movdqa    -320(%rbp), %xmm0                             #5064.42
..LN4977:
        pmovmskb  %xmm0, %eax                                   #5064.22
..LN4979:
        movl      %eax, -112(%rbp)                              #5064.17
..LN4981:
   .loc    3  5065
        movl      -112(%rbp), %eax                              #5065.17
        incl      %eax                                          #5065.17
        sarl      $16, %eax                                     #5065.17
        negl      %eax                                          #5065.17
        movl      %eax, -112(%rbp)                              #5065.17
..LN4983:
   .loc    3  5069
        andl      $-4, -24(%rbp)                                #5069.17
..LN4985:
   .loc    3  5072
        movl      -68(%rbp), %eax                               #5072.29
        orl       -64(%rbp), %eax                               #5072.29
..LN4987:
        orl       -72(%rbp), %eax                               #5072.34
..LN4989:
        orl       -112(%rbp), %eax                              #5072.39
..LN4991:
        je        ..B29.40      # Prob 50%                      #5072.47
                                # LOE
..B29.16:                       # Preds ..B29.15
..LN4993:
   .loc    3  5075
        movl      -24(%rbp), %eax                               #5075.37
        negl      %eax                                          #5075.37
        addl      -76(%rbp), %eax                               #5075.37
        movl      %eax, -288(%rbp)                              #5075.37
        je        ..B29.19      # Prob 50%                      #5075.37
                                # LOE
..B29.17:                       # Preds ..B29.16
        movl      -288(%rbp), %eax                              #5075.37
        cmpl      $1, %eax                                      #5075.37
        je        ..B29.21      # Prob 50%                      #5075.37
                                # LOE
..B29.18:                       # Preds ..B29.17
        movl      -288(%rbp), %eax                              #5075.37
        cmpl      $2, %eax                                      #5075.37
        je        ..B29.25      # Prob 50%                      #5075.37
        jmp       ..B29.31      # Prob 100%                     #5075.37
                                # LOE
..B29.19:                       # Preds ..B29.16
..LN4995:
   .loc    3  5078
        movl      -64(%rbp), %eax                               #5078.30
..LN4997:
        testl     %eax, %eax                                    #5078.36
        je        ..B29.39      # Prob 50%                      #5078.36
                                # LOE
..B29.20:                       # Preds ..B29.19
..LN4999:
        movl      -80(%rbp), %eax                               #5078.53
        negl      %eax                                          #5078.53
..LN5001:
        movl      -24(%rbp), %edx                               #5078.47
..LN5003:
        lea       1(%rdx,%rax), %eax                            #5078.61
        leave                                                   #5078.61
..___tag_value_eval_4na_pos.508:                                #
        ret                                                     #5078.61
..___tag_value_eval_4na_pos.509:                                #
                                # LOE
..B29.21:                       # Preds ..B29.17
..LN5005:
   .loc    3  5081
        movl      -64(%rbp), %eax                               #5081.30
..LN5007:
        testl     %eax, %eax                                    #5081.36
        je        ..B29.23      # Prob 50%                      #5081.36
                                # LOE
..B29.22:                       # Preds ..B29.21
..LN5009:
        movl      -80(%rbp), %eax                               #5081.53
        negl      %eax                                          #5081.53
..LN5011:
        movl      -24(%rbp), %edx                               #5081.47
..LN5013:
        lea       1(%rdx,%rax), %eax                            #5081.61
        leave                                                   #5081.61
..___tag_value_eval_4na_pos.511:                                #
        ret                                                     #5081.61
..___tag_value_eval_4na_pos.512:                                #
                                # LOE
..B29.23:                       # Preds ..B29.21
..LN5015:
   .loc    3  5082
        movl      -68(%rbp), %eax                               #5082.30
..LN5017:
        testl     %eax, %eax                                    #5082.36
        je        ..B29.39      # Prob 50%                      #5082.36
                                # LOE
..B29.24:                       # Preds ..B29.23
..LN5019:
        movl      -80(%rbp), %eax                               #5082.53
        negl      %eax                                          #5082.53
..LN5021:
        movl      -24(%rbp), %edx                               #5082.47
..LN5023:
        lea       2(%rdx,%rax), %eax                            #5082.61
        leave                                                   #5082.61
..___tag_value_eval_4na_pos.514:                                #
        ret                                                     #5082.61
..___tag_value_eval_4na_pos.515:                                #
                                # LOE
..B29.25:                       # Preds ..B29.18
..LN5025:
   .loc    3  5085
        movl      -64(%rbp), %eax                               #5085.30
..LN5027:
        testl     %eax, %eax                                    #5085.36
        je        ..B29.27      # Prob 50%                      #5085.36
                                # LOE
..B29.26:                       # Preds ..B29.25
..LN5029:
        movl      -80(%rbp), %eax                               #5085.53
        negl      %eax                                          #5085.53
..LN5031:
        movl      -24(%rbp), %edx                               #5085.47
..LN5033:
        lea       1(%rdx,%rax), %eax                            #5085.61
        leave                                                   #5085.61
..___tag_value_eval_4na_pos.517:                                #
        ret                                                     #5085.61
..___tag_value_eval_4na_pos.518:                                #
                                # LOE
..B29.27:                       # Preds ..B29.25
..LN5035:
   .loc    3  5086
        movl      -68(%rbp), %eax                               #5086.30
..LN5037:
        testl     %eax, %eax                                    #5086.36
        je        ..B29.29      # Prob 50%                      #5086.36
                                # LOE
..B29.28:                       # Preds ..B29.27
..LN5039:
        movl      -80(%rbp), %eax                               #5086.53
        negl      %eax                                          #5086.53
..LN5041:
        movl      -24(%rbp), %edx                               #5086.47
..LN5043:
        lea       2(%rdx,%rax), %eax                            #5086.61
        leave                                                   #5086.61
..___tag_value_eval_4na_pos.520:                                #
        ret                                                     #5086.61
..___tag_value_eval_4na_pos.521:                                #
                                # LOE
..B29.29:                       # Preds ..B29.27
..LN5045:
   .loc    3  5087
        movl      -72(%rbp), %eax                               #5087.30
..LN5047:
        testl     %eax, %eax                                    #5087.36
        je        ..B29.39      # Prob 50%                      #5087.36
                                # LOE
..B29.30:                       # Preds ..B29.29
..LN5049:
        movl      -80(%rbp), %eax                               #5087.53
        negl      %eax                                          #5087.53
..LN5051:
        movl      -24(%rbp), %edx                               #5087.47
..LN5053:
        lea       3(%rdx,%rax), %eax                            #5087.61
        leave                                                   #5087.61
..___tag_value_eval_4na_pos.523:                                #
        ret                                                     #5087.61
..___tag_value_eval_4na_pos.524:                                #
                                # LOE
..B29.31:                       # Preds ..B29.18
..LN5055:
   .loc    3  5090
        movl      -64(%rbp), %eax                               #5090.30
..LN5057:
        testl     %eax, %eax                                    #5090.36
        je        ..B29.33      # Prob 50%                      #5090.36
                                # LOE
..B29.32:                       # Preds ..B29.31
..LN5059:
        movl      -80(%rbp), %eax                               #5090.53
        negl      %eax                                          #5090.53
..LN5061:
        movl      -24(%rbp), %edx                               #5090.47
..LN5063:
        lea       1(%rdx,%rax), %eax                            #5090.61
        leave                                                   #5090.61
..___tag_value_eval_4na_pos.526:                                #
        ret                                                     #5090.61
..___tag_value_eval_4na_pos.527:                                #
                                # LOE
..B29.33:                       # Preds ..B29.31
..LN5065:
   .loc    3  5091
        movl      -68(%rbp), %eax                               #5091.30
..LN5067:
        testl     %eax, %eax                                    #5091.36
        je        ..B29.35      # Prob 50%                      #5091.36
                                # LOE
..B29.34:                       # Preds ..B29.33
..LN5069:
        movl      -80(%rbp), %eax                               #5091.53
        negl      %eax                                          #5091.53
..LN5071:
        movl      -24(%rbp), %edx                               #5091.47
..LN5073:
        lea       2(%rdx,%rax), %eax                            #5091.61
        leave                                                   #5091.61
..___tag_value_eval_4na_pos.529:                                #
        ret                                                     #5091.61
..___tag_value_eval_4na_pos.530:                                #
                                # LOE
..B29.35:                       # Preds ..B29.33
..LN5075:
   .loc    3  5092
        movl      -72(%rbp), %eax                               #5092.30
..LN5077:
        testl     %eax, %eax                                    #5092.36
        je        ..B29.37      # Prob 50%                      #5092.36
                                # LOE
..B29.36:                       # Preds ..B29.35
..LN5079:
        movl      -80(%rbp), %eax                               #5092.53
        negl      %eax                                          #5092.53
..LN5081:
        movl      -24(%rbp), %edx                               #5092.47
..LN5083:
        lea       3(%rdx,%rax), %eax                            #5092.61
        leave                                                   #5092.61
..___tag_value_eval_4na_pos.532:                                #
        ret                                                     #5092.61
..___tag_value_eval_4na_pos.533:                                #
                                # LOE
..B29.37:                       # Preds ..B29.35
..LN5085:
   .loc    3  5093
        movl      -112(%rbp), %eax                              #5093.30
..LN5087:
        testl     %eax, %eax                                    #5093.36
        je        ..B29.39      # Prob 50%                      #5093.36
                                # LOE
..B29.38:                       # Preds ..B29.37
..LN5089:
        movl      -80(%rbp), %eax                               #5093.53
        negl      %eax                                          #5093.53
..LN5091:
        movl      -24(%rbp), %edx                               #5093.47
..LN5093:
        lea       4(%rdx,%rax), %eax                            #5093.61
        leave                                                   #5093.61
..___tag_value_eval_4na_pos.535:                                #
        ret                                                     #5093.61
..___tag_value_eval_4na_pos.536:                                #
                                # LOE
..B29.39:                       # Preds ..B29.19 ..B29.23 ..B29.29 ..B29.37
..LN5095:
   .loc    3  5095
        xorl      %eax, %eax                                    #5095.28
        leave                                                   #5095.28
..___tag_value_eval_4na_pos.538:                                #
        ret                                                     #5095.28
..___tag_value_eval_4na_pos.539:                                #
                                # LOE
..B29.40:                       # Preds ..B29.15
..LN5097:
   .loc    3  5135
        addl      $4, -24(%rbp)                                 #5135.17
..LN5099:
   .loc    3  5138
        movl      -24(%rbp), %eax                               #5138.22
..LN5101:
        movl      -76(%rbp), %edx                               #5138.28
        cmpl      %edx, %eax                                    #5138.28
        jbe       ..B29.42      # Prob 50%                      #5138.28
                                # LOE
..B29.41:                       # Preds ..B29.40
..LN5103:
   .loc    3  5139
        xorl      %eax, %eax                                    #5139.28
        leave                                                   #5139.28
..___tag_value_eval_4na_pos.541:                                #
        ret                                                     #5139.28
..___tag_value_eval_4na_pos.542:                                #
                                # LOE
..B29.42:                       # Preds ..B29.40
..LN5105:
   .loc    3  5142
        movl      -60(%rbp), %eax                               #5142.25
        decl      %eax                                          #5142.25
        movl      %eax, -60(%rbp)                               #5142.25
..LN5107:
        je        ..B29.47      # Prob 50%                      #5142.39
                                # LOE
..B29.43:                       # Preds ..B29.42
..LN5109:
   .loc    3  5147
        movdqa    -272(%rbp), %xmm0                             #5147.43
..LN5111:
        psrldq    $2, %xmm0                                     #5147.26
..LN5113:
        movdqa    %xmm0, -272(%rbp)                             #5147.17
..LN5115:
   .loc    3  5150
        movq      -88(%rbp), %rax                               #5150.22
..LN5117:
        movq      -96(%rbp), %rdx                               #5150.26
        cmpq      %rdx, %rax                                    #5150.26
        jae       ..B29.45      # Prob 50%                      #5150.26
                                # LOE
..B29.44:                       # Preds ..B29.43
..LN5119:
   .loc    3  5151
        movdqa    -272(%rbp), %xmm0                             #5151.49
..LN5121:
        movq      -88(%rbp), %rax                               #5151.72
        movzbl    (%rax), %eax                                  #5151.72
        movzbl    %al, %eax                                     #5151.72
..LN5123:
        movslq    %eax, %rax                                    #5151.57
        lea       expand_2na.0(%rip), %rdx                      #5151.57
        movzwl    (%rdx,%rax,2), %eax                           #5151.57
        movzwl    %ax, %eax                                     #5151.57
..LN5125:
        pinsrw    $7, %eax, %xmm0                               #5151.30
..LN5127:
        movdqa    %xmm0, -272(%rbp)                             #5151.21
                                # LOE
..B29.45:                       # Preds ..B29.44 ..B29.43
..LN5129:
   .loc    3  5154
        incq      -88(%rbp)                                     #5154.20
        jmp       ..B29.10      # Prob 100%                     #5154.20
                                # LOE
..B29.47:                       # Preds ..B29.42 ..B29.10
..LN5131:
   .loc    3  5159
        movq      -104(%rbp), %rax                              #5159.18
..LN5133:
        movq      -96(%rbp), %rdx                               #5159.25
        cmpq      %rdx, %rax                                    #5159.25
        jae       ..B29.51      # Prob 50%                      #5159.25
                                # LOE
..B29.48:                       # Preds ..B29.47
..LN5135:
   .loc    3  5164
        movl      -24(%rbp), %eax                               #5164.18
..LN5137:
        movl      -76(%rbp), %edx                               #5164.24
        cmpl      %edx, %eax                                    #5164.24
        ja        ..B29.51      # Prob 50%                      #5164.24
                                # LOE
..B29.49:                       # Preds ..B29.48
..LN5139:
   .loc    3  5169
        movq      -104(%rbp), %rax                              #5169.41
..LN5141:
        movq      -96(%rbp), %rdx                               #5169.46
..LN5143:
        movq      %rax, %rdi                                    #5169.22
        movq      %rdx, %rsi                                    #5169.22
        call      prime_buffer_4na@PLT                          #5169.22
                                # LOE xmm0
..B29.55:                       # Preds ..B29.49
        movdqa    %xmm0, -336(%rbp)                             #5169.22
                                # LOE
..B29.50:                       # Preds ..B29.55
..LN5145:
        movdqa    -336(%rbp), %xmm0                             #5169.13
        movdqa    %xmm0, -272(%rbp)                             #5169.13
..LN5147:
   .loc    3  5184
        addq      $8, -104(%rbp)                                #5184.13
..LN5149:
   .loc    3  5186
        movq      -104(%rbp), %rax                              #5186.17
..LN5151:
        movq      %rax, -88(%rbp)                               #5186.13
        jmp       ..B29.8       # Prob 100%                     #5186.13
                                # LOE
..B29.51:                       # Preds ..B29.48 ..B29.47 ..B29.8
..LN5153:
   .loc    3  5196
        xorl      %eax, %eax                                    #5196.12
        leave                                                   #5196.12
..___tag_value_eval_4na_pos.544:                                #
        ret                                                     #5196.12
        .align    2,0x90
..___tag_value_eval_4na_pos.545:                                #
                                # LOE
# mark_end;
	.type	eval_4na_pos,@function
	.size	eval_4na_pos,.-eval_4na_pos
.LNeval_4na_pos:
	.data
# -- End  eval_4na_pos
	.text
# -- Begin  NucStrstrSearch
# mark_begin;
       .align    2,0x90
	.globl NucStrstrSearch
NucStrstrSearch:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
# parameter 5(selflen): %r8
..B30.1:                        # Preds ..B30.0
..___tag_value_NucStrstrSearch.548:                             #
..LN5155:
   .loc    3  5217
        pushq     %rbp                                          #5217.1
        movq      %rsp, %rbp                                    #5217.1
..___tag_value_NucStrstrSearch.549:                             #
        subq      $192, %rsp                                    #5217.1
        movq      %rbx, -32(%rbp)                               #5217.1
..___tag_value_NucStrstrSearch.552:                             #
        movq      %rdi, -56(%rbp)                               #5217.1
        movq      %rsi, -48(%rbp)                               #5217.1
        movl      %edx, -24(%rbp)                               #5217.1
        movl      %ecx, -16(%rbp)                               #5217.1
        movq      %r8, -40(%rbp)                                #5217.1
..LN5157:
   .loc    3  5218
        movq      -56(%rbp), %rax                               #5218.10
..LN5159:
        testq     %rax, %rax                                    #5218.18
        je        ..B30.111     # Prob 50%                      #5218.18
                                # LOE
..B30.2:                        # Preds ..B30.1
..LN5161:
        movq      -48(%rbp), %rax                               #5218.26
..LN5163:
        testq     %rax, %rax                                    #5218.37
        je        ..B30.111     # Prob 50%                      #5218.37
                                # LOE
..B30.3:                        # Preds ..B30.2
..LN5165:
        movl      -16(%rbp), %eax                               #5218.45
..LN5167:
        testl     %eax, %eax                                    #5218.52
        je        ..B30.111     # Prob 50%                      #5218.52
                                # LOE
..B30.4:                        # Preds ..B30.3
..LN5169:
   .loc    3  5223
        movq      -56(%rbp), %rax                               #5223.18
        movl      (%rax), %eax                                  #5223.18
        movl      %eax, -64(%rbp)                               #5223.18
..LN5171:
        cmpl      $12, %eax                                     #5223.9
        ja        ..B30.111     # Prob 50%                      #5223.9
                                # LOE
..B30.5:                        # Preds ..B30.4
        movl      -64(%rbp), %eax                               #5223.9
        movl      %eax, %eax                                    #5223.9
        lea       ..1..TPKT.30_0.0.1(%rip), %rdx                #5223.9
        movq      (%rdx,%rax,8), %rax                           #5223.9
        jmp       *%rax                                         #5223.9
                                # LOE
..1.30_0.TAG.00.0.1:
..B30.7:                        # Preds ..B30.5
..LN5173:
   .loc    3  5226
        movq      -56(%rbp), %rax                               #5226.24
..LN5175:
        movl      -16(%rbp), %edx                               #5226.18
..LN5177:
        movl      4(%rax), %eax                                 #5226.24
        cmpl      %eax, %edx                                    #5226.24
        jae       ..B30.9       # Prob 50%                      #5226.24
                                # LOE
..B30.8:                        # Preds ..B30.7
..LN5179:
        xorl      %eax, %eax                                    #5226.54
        movq      -32(%rbp), %rbx                               #5226.54
..___tag_value_NucStrstrSearch.553:                             #
        leave                                                   #5226.54
..___tag_value_NucStrstrSearch.555:                             #
        ret                                                     #5226.54
..___tag_value_NucStrstrSearch.556:                             #
                                # LOE
..B30.9:                        # Preds ..B30.7
..LN5181:
   .loc    3  5227
        movq      -40(%rbp), %rax                               #5227.9
        testq     %rax, %rax                                    #5227.9
        je        ..B30.11      # Prob 50%                      #5227.9
                                # LOE
..B30.10:                       # Preds ..B30.9
..LN5183:
        movq      -56(%rbp), %rax                               #5227.27
..LN5185:
        movq      -40(%rbp), %rdx                               #5227.18
..LN5187:
        movl      4(%rax), %eax                                 #5227.27
..LN5189:
        movl      %eax, (%rdx)                                  #5227.18
                                # LOE
..B30.11:                       # Preds ..B30.10 ..B30.9
..LN5191:
   .loc    3  5228
        movq      -56(%rbp), %rax                               #5228.36
..LN5193:
        movq      -48(%rbp), %rdx                               #5228.51
..LN5195:
        movl      -24(%rbp), %ecx                               #5228.60
..LN5197:
        movl      -16(%rbp), %ebx                               #5228.65
..LN5199:
        movq      %rax, %rdi                                    #5228.20
        movq      %rdx, %rsi                                    #5228.20
        movl      %ecx, %edx                                    #5228.20
        movl      %ebx, %ecx                                    #5228.20
        call      eval_2na_64@PLT                               #5228.20
                                # LOE eax
..B30.114:                      # Preds ..B30.11
        movl      %eax, -144(%rbp)                              #5228.20
                                # LOE
..B30.12:                       # Preds ..B30.114
        movl      -144(%rbp), %eax                              #5228.20
        movq      -32(%rbp), %rbx                               #5228.20
..___tag_value_NucStrstrSearch.558:                             #
        leave                                                   #5228.20
..___tag_value_NucStrstrSearch.560:                             #
        ret                                                     #5228.20
..___tag_value_NucStrstrSearch.561:                             #
                                # LOE
..1.30_0.TAG.01.0.1:
..B30.14:                       # Preds ..B30.5
..LN5201:
   .loc    3  5230
        movq      -56(%rbp), %rax                               #5230.24
..LN5203:
        movl      -16(%rbp), %edx                               #5230.18
..LN5205:
        movl      4(%rax), %eax                                 #5230.24
        cmpl      %eax, %edx                                    #5230.24
        jae       ..B30.16      # Prob 50%                      #5230.24
                                # LOE
..B30.15:                       # Preds ..B30.14
..LN5207:
        xorl      %eax, %eax                                    #5230.54
        movq      -32(%rbp), %rbx                               #5230.54
..___tag_value_NucStrstrSearch.563:                             #
        leave                                                   #5230.54
..___tag_value_NucStrstrSearch.565:                             #
        ret                                                     #5230.54
..___tag_value_NucStrstrSearch.566:                             #
                                # LOE
..B30.16:                       # Preds ..B30.14
..LN5209:
   .loc    3  5231
        movq      -40(%rbp), %rax                               #5231.9
        testq     %rax, %rax                                    #5231.9
        je        ..B30.18      # Prob 50%                      #5231.9
                                # LOE
..B30.17:                       # Preds ..B30.16
..LN5211:
        movq      -56(%rbp), %rax                               #5231.27
..LN5213:
        movq      -40(%rbp), %rdx                               #5231.18
..LN5215:
        movl      4(%rax), %eax                                 #5231.27
..LN5217:
        movl      %eax, (%rdx)                                  #5231.18
                                # LOE
..B30.18:                       # Preds ..B30.17 ..B30.16
..LN5219:
   .loc    3  5232
        movq      -56(%rbp), %rax                               #5232.36
..LN5221:
        movq      -48(%rbp), %rdx                               #5232.51
..LN5223:
        movl      -24(%rbp), %ecx                               #5232.60
..LN5225:
        movl      -16(%rbp), %ebx                               #5232.65
..LN5227:
        movq      %rax, %rdi                                    #5232.20
        movq      %rdx, %rsi                                    #5232.20
        movl      %ecx, %edx                                    #5232.20
        movl      %ebx, %ecx                                    #5232.20
        call      eval_4na_64@PLT                               #5232.20
                                # LOE eax
..B30.115:                      # Preds ..B30.18
        movl      %eax, -140(%rbp)                              #5232.20
                                # LOE
..B30.19:                       # Preds ..B30.115
        movl      -140(%rbp), %eax                              #5232.20
        movq      -32(%rbp), %rbx                               #5232.20
..___tag_value_NucStrstrSearch.568:                             #
        leave                                                   #5232.20
..___tag_value_NucStrstrSearch.570:                             #
        ret                                                     #5232.20
..___tag_value_NucStrstrSearch.571:                             #
                                # LOE
..1.30_0.TAG.02.0.1:
..B30.21:                       # Preds ..B30.5
..LN5229:
   .loc    3  5235
        movq      -56(%rbp), %rax                               #5235.24
..LN5231:
        movl      -16(%rbp), %edx                               #5235.18
..LN5233:
        movl      4(%rax), %eax                                 #5235.24
        cmpl      %eax, %edx                                    #5235.24
        jae       ..B30.23      # Prob 50%                      #5235.24
                                # LOE
..B30.22:                       # Preds ..B30.21
..LN5235:
        xorl      %eax, %eax                                    #5235.54
        movq      -32(%rbp), %rbx                               #5235.54
..___tag_value_NucStrstrSearch.573:                             #
        leave                                                   #5235.54
..___tag_value_NucStrstrSearch.575:                             #
        ret                                                     #5235.54
..___tag_value_NucStrstrSearch.576:                             #
                                # LOE
..B30.23:                       # Preds ..B30.21
..LN5237:
   .loc    3  5236
        movq      -40(%rbp), %rax                               #5236.9
        testq     %rax, %rax                                    #5236.9
        je        ..B30.25      # Prob 50%                      #5236.9
                                # LOE
..B30.24:                       # Preds ..B30.23
..LN5239:
        movq      -56(%rbp), %rax                               #5236.27
..LN5241:
        movq      -40(%rbp), %rdx                               #5236.18
..LN5243:
        movl      4(%rax), %eax                                 #5236.27
..LN5245:
        movl      %eax, (%rdx)                                  #5236.18
                                # LOE
..B30.25:                       # Preds ..B30.24 ..B30.23
..LN5247:
   .loc    3  5237
        movq      -56(%rbp), %rax                               #5237.35
..LN5249:
        movq      -48(%rbp), %rdx                               #5237.50
..LN5251:
        movl      -24(%rbp), %ecx                               #5237.59
..LN5253:
        movl      -16(%rbp), %ebx                               #5237.64
..LN5255:
        movq      %rax, %rdi                                    #5237.20
        movq      %rdx, %rsi                                    #5237.20
        movl      %ecx, %edx                                    #5237.20
        movl      %ebx, %ecx                                    #5237.20
        call      eval_2na_8@PLT                                #5237.20
                                # LOE eax
..B30.116:                      # Preds ..B30.25
        movl      %eax, -136(%rbp)                              #5237.20
                                # LOE
..B30.26:                       # Preds ..B30.116
        movl      -136(%rbp), %eax                              #5237.20
        movq      -32(%rbp), %rbx                               #5237.20
..___tag_value_NucStrstrSearch.578:                             #
        leave                                                   #5237.20
..___tag_value_NucStrstrSearch.580:                             #
        ret                                                     #5237.20
..___tag_value_NucStrstrSearch.581:                             #
                                # LOE
..1.30_0.TAG.03.0.1:
..B30.28:                       # Preds ..B30.5
..LN5257:
   .loc    3  5239
        movq      -56(%rbp), %rax                               #5239.24
..LN5259:
        movl      -16(%rbp), %edx                               #5239.18
..LN5261:
        movl      4(%rax), %eax                                 #5239.24
        cmpl      %eax, %edx                                    #5239.24
        jae       ..B30.30      # Prob 50%                      #5239.24
                                # LOE
..B30.29:                       # Preds ..B30.28
..LN5263:
        xorl      %eax, %eax                                    #5239.54
        movq      -32(%rbp), %rbx                               #5239.54
..___tag_value_NucStrstrSearch.583:                             #
        leave                                                   #5239.54
..___tag_value_NucStrstrSearch.585:                             #
        ret                                                     #5239.54
..___tag_value_NucStrstrSearch.586:                             #
                                # LOE
..B30.30:                       # Preds ..B30.28
..LN5265:
   .loc    3  5240
        movq      -40(%rbp), %rax                               #5240.9
        testq     %rax, %rax                                    #5240.9
        je        ..B30.32      # Prob 50%                      #5240.9
                                # LOE
..B30.31:                       # Preds ..B30.30
..LN5267:
        movq      -56(%rbp), %rax                               #5240.27
..LN5269:
        movq      -40(%rbp), %rdx                               #5240.18
..LN5271:
        movl      4(%rax), %eax                                 #5240.27
..LN5273:
        movl      %eax, (%rdx)                                  #5240.18
                                # LOE
..B30.32:                       # Preds ..B30.31 ..B30.30
..LN5275:
   .loc    3  5241
        movq      -56(%rbp), %rax                               #5241.36
..LN5277:
        movq      -48(%rbp), %rdx                               #5241.51
..LN5279:
        movl      -24(%rbp), %ecx                               #5241.60
..LN5281:
        movl      -16(%rbp), %ebx                               #5241.65
..LN5283:
        movq      %rax, %rdi                                    #5241.20
        movq      %rdx, %rsi                                    #5241.20
        movl      %ecx, %edx                                    #5241.20
        movl      %ebx, %ecx                                    #5241.20
        call      eval_2na_16@PLT                               #5241.20
                                # LOE eax
..B30.117:                      # Preds ..B30.32
        movl      %eax, -132(%rbp)                              #5241.20
                                # LOE
..B30.33:                       # Preds ..B30.117
        movl      -132(%rbp), %eax                              #5241.20
        movq      -32(%rbp), %rbx                               #5241.20
..___tag_value_NucStrstrSearch.588:                             #
        leave                                                   #5241.20
..___tag_value_NucStrstrSearch.590:                             #
        ret                                                     #5241.20
..___tag_value_NucStrstrSearch.591:                             #
                                # LOE
..1.30_0.TAG.04.0.1:
..B30.35:                       # Preds ..B30.5
..LN5285:
   .loc    3  5243
        movq      -56(%rbp), %rax                               #5243.24
..LN5287:
        movl      -16(%rbp), %edx                               #5243.18
..LN5289:
        movl      4(%rax), %eax                                 #5243.24
        cmpl      %eax, %edx                                    #5243.24
        jae       ..B30.37      # Prob 50%                      #5243.24
                                # LOE
..B30.36:                       # Preds ..B30.35
..LN5291:
        xorl      %eax, %eax                                    #5243.54
        movq      -32(%rbp), %rbx                               #5243.54
..___tag_value_NucStrstrSearch.593:                             #
        leave                                                   #5243.54
..___tag_value_NucStrstrSearch.595:                             #
        ret                                                     #5243.54
..___tag_value_NucStrstrSearch.596:                             #
                                # LOE
..B30.37:                       # Preds ..B30.35
..LN5293:
   .loc    3  5244
        movq      -40(%rbp), %rax                               #5244.9
        testq     %rax, %rax                                    #5244.9
        je        ..B30.39      # Prob 50%                      #5244.9
                                # LOE
..B30.38:                       # Preds ..B30.37
..LN5295:
        movq      -56(%rbp), %rax                               #5244.27
..LN5297:
        movq      -40(%rbp), %rdx                               #5244.18
..LN5299:
        movl      4(%rax), %eax                                 #5244.27
..LN5301:
        movl      %eax, (%rdx)                                  #5244.18
                                # LOE
..B30.39:                       # Preds ..B30.38 ..B30.37
..LN5303:
   .loc    3  5245
        movq      -56(%rbp), %rax                               #5245.36
..LN5305:
        movq      -48(%rbp), %rdx                               #5245.51
..LN5307:
        movl      -24(%rbp), %ecx                               #5245.60
..LN5309:
        movl      -16(%rbp), %ebx                               #5245.65
..LN5311:
        movq      %rax, %rdi                                    #5245.20
        movq      %rdx, %rsi                                    #5245.20
        movl      %ecx, %edx                                    #5245.20
        movl      %ebx, %ecx                                    #5245.20
        call      eval_2na_32@PLT                               #5245.20
                                # LOE eax
..B30.118:                      # Preds ..B30.39
        movl      %eax, -128(%rbp)                              #5245.20
                                # LOE
..B30.40:                       # Preds ..B30.118
        movl      -128(%rbp), %eax                              #5245.20
        movq      -32(%rbp), %rbx                               #5245.20
..___tag_value_NucStrstrSearch.598:                             #
        leave                                                   #5245.20
..___tag_value_NucStrstrSearch.600:                             #
        ret                                                     #5245.20
..___tag_value_NucStrstrSearch.601:                             #
                                # LOE
..1.30_0.TAG.05.0.1:
..B30.42:                       # Preds ..B30.5
..LN5313:
   .loc    3  5247
        movq      -56(%rbp), %rax                               #5247.24
..LN5315:
        movl      -16(%rbp), %edx                               #5247.18
..LN5317:
        movl      4(%rax), %eax                                 #5247.24
        cmpl      %eax, %edx                                    #5247.24
        jae       ..B30.44      # Prob 50%                      #5247.24
                                # LOE
..B30.43:                       # Preds ..B30.42
..LN5319:
        xorl      %eax, %eax                                    #5247.54
        movq      -32(%rbp), %rbx                               #5247.54
..___tag_value_NucStrstrSearch.603:                             #
        leave                                                   #5247.54
..___tag_value_NucStrstrSearch.605:                             #
        ret                                                     #5247.54
..___tag_value_NucStrstrSearch.606:                             #
                                # LOE
..B30.44:                       # Preds ..B30.42
..LN5321:
   .loc    3  5248
        movq      -40(%rbp), %rax                               #5248.9
        testq     %rax, %rax                                    #5248.9
        je        ..B30.46      # Prob 50%                      #5248.9
                                # LOE
..B30.45:                       # Preds ..B30.44
..LN5323:
        movq      -56(%rbp), %rax                               #5248.27
..LN5325:
        movq      -40(%rbp), %rdx                               #5248.18
..LN5327:
        movl      4(%rax), %eax                                 #5248.27
..LN5329:
        movl      %eax, (%rdx)                                  #5248.18
                                # LOE
..B30.46:                       # Preds ..B30.45 ..B30.44
..LN5331:
   .loc    3  5249
        movq      -56(%rbp), %rax                               #5249.37
..LN5333:
        movq      -48(%rbp), %rdx                               #5249.52
..LN5335:
        movl      -24(%rbp), %ecx                               #5249.61
..LN5337:
        movl      -16(%rbp), %ebx                               #5249.66
..LN5339:
        movq      %rax, %rdi                                    #5249.20
        movq      %rdx, %rsi                                    #5249.20
        movl      %ecx, %edx                                    #5249.20
        movl      %ebx, %ecx                                    #5249.20
        call      eval_2na_128@PLT                              #5249.20
                                # LOE eax
..B30.119:                      # Preds ..B30.46
        movl      %eax, -124(%rbp)                              #5249.20
                                # LOE
..B30.47:                       # Preds ..B30.119
        movl      -124(%rbp), %eax                              #5249.20
        movq      -32(%rbp), %rbx                               #5249.20
..___tag_value_NucStrstrSearch.608:                             #
        leave                                                   #5249.20
..___tag_value_NucStrstrSearch.610:                             #
        ret                                                     #5249.20
..___tag_value_NucStrstrSearch.611:                             #
                                # LOE
..1.30_0.TAG.06.0.1:
..B30.49:                       # Preds ..B30.5
..LN5341:
   .loc    3  5251
        movq      -56(%rbp), %rax                               #5251.24
..LN5343:
        movl      -16(%rbp), %edx                               #5251.18
..LN5345:
        movl      4(%rax), %eax                                 #5251.24
        cmpl      %eax, %edx                                    #5251.24
        jae       ..B30.51      # Prob 50%                      #5251.24
                                # LOE
..B30.50:                       # Preds ..B30.49
..LN5347:
        xorl      %eax, %eax                                    #5251.54
        movq      -32(%rbp), %rbx                               #5251.54
..___tag_value_NucStrstrSearch.613:                             #
        leave                                                   #5251.54
..___tag_value_NucStrstrSearch.615:                             #
        ret                                                     #5251.54
..___tag_value_NucStrstrSearch.616:                             #
                                # LOE
..B30.51:                       # Preds ..B30.49
..LN5349:
   .loc    3  5252
        movq      -40(%rbp), %rax                               #5252.9
        testq     %rax, %rax                                    #5252.9
        je        ..B30.53      # Prob 50%                      #5252.9
                                # LOE
..B30.52:                       # Preds ..B30.51
..LN5351:
        movq      -56(%rbp), %rax                               #5252.27
..LN5353:
        movq      -40(%rbp), %rdx                               #5252.18
..LN5355:
        movl      4(%rax), %eax                                 #5252.27
..LN5357:
        movl      %eax, (%rdx)                                  #5252.18
                                # LOE
..B30.53:                       # Preds ..B30.52 ..B30.51
..LN5359:
   .loc    3  5253
        movq      -56(%rbp), %rax                               #5253.36
..LN5361:
        movq      -48(%rbp), %rdx                               #5253.51
..LN5363:
        movl      -24(%rbp), %ecx                               #5253.60
..LN5365:
        movl      -16(%rbp), %ebx                               #5253.65
..LN5367:
        movq      %rax, %rdi                                    #5253.20
        movq      %rdx, %rsi                                    #5253.20
        movl      %ecx, %edx                                    #5253.20
        movl      %ebx, %ecx                                    #5253.20
        call      eval_4na_16@PLT                               #5253.20
                                # LOE eax
..B30.120:                      # Preds ..B30.53
        movl      %eax, -120(%rbp)                              #5253.20
                                # LOE
..B30.54:                       # Preds ..B30.120
        movl      -120(%rbp), %eax                              #5253.20
        movq      -32(%rbp), %rbx                               #5253.20
..___tag_value_NucStrstrSearch.618:                             #
        leave                                                   #5253.20
..___tag_value_NucStrstrSearch.620:                             #
        ret                                                     #5253.20
..___tag_value_NucStrstrSearch.621:                             #
                                # LOE
..1.30_0.TAG.07.0.1:
..B30.56:                       # Preds ..B30.5
..LN5369:
   .loc    3  5255
        movq      -56(%rbp), %rax                               #5255.24
..LN5371:
        movl      -16(%rbp), %edx                               #5255.18
..LN5373:
        movl      4(%rax), %eax                                 #5255.24
        cmpl      %eax, %edx                                    #5255.24
        jae       ..B30.58      # Prob 50%                      #5255.24
                                # LOE
..B30.57:                       # Preds ..B30.56
..LN5375:
        xorl      %eax, %eax                                    #5255.54
        movq      -32(%rbp), %rbx                               #5255.54
..___tag_value_NucStrstrSearch.623:                             #
        leave                                                   #5255.54
..___tag_value_NucStrstrSearch.625:                             #
        ret                                                     #5255.54
..___tag_value_NucStrstrSearch.626:                             #
                                # LOE
..B30.58:                       # Preds ..B30.56
..LN5377:
   .loc    3  5256
        movq      -40(%rbp), %rax                               #5256.9
        testq     %rax, %rax                                    #5256.9
        je        ..B30.60      # Prob 50%                      #5256.9
                                # LOE
..B30.59:                       # Preds ..B30.58
..LN5379:
        movq      -56(%rbp), %rax                               #5256.27
..LN5381:
        movq      -40(%rbp), %rdx                               #5256.18
..LN5383:
        movl      4(%rax), %eax                                 #5256.27
..LN5385:
        movl      %eax, (%rdx)                                  #5256.18
                                # LOE
..B30.60:                       # Preds ..B30.59 ..B30.58
..LN5387:
   .loc    3  5257
        movq      -56(%rbp), %rax                               #5257.36
..LN5389:
        movq      -48(%rbp), %rdx                               #5257.51
..LN5391:
        movl      -24(%rbp), %ecx                               #5257.60
..LN5393:
        movl      -16(%rbp), %ebx                               #5257.65
..LN5395:
        movq      %rax, %rdi                                    #5257.20
        movq      %rdx, %rsi                                    #5257.20
        movl      %ecx, %edx                                    #5257.20
        movl      %ebx, %ecx                                    #5257.20
        call      eval_4na_32@PLT                               #5257.20
                                # LOE eax
..B30.121:                      # Preds ..B30.60
        movl      %eax, -116(%rbp)                              #5257.20
                                # LOE
..B30.61:                       # Preds ..B30.121
        movl      -116(%rbp), %eax                              #5257.20
        movq      -32(%rbp), %rbx                               #5257.20
..___tag_value_NucStrstrSearch.628:                             #
        leave                                                   #5257.20
..___tag_value_NucStrstrSearch.630:                             #
        ret                                                     #5257.20
..___tag_value_NucStrstrSearch.631:                             #
                                # LOE
..1.30_0.TAG.08.0.1:
..B30.63:                       # Preds ..B30.5
..LN5397:
   .loc    3  5259
        movq      -56(%rbp), %rax                               #5259.24
..LN5399:
        movl      -16(%rbp), %edx                               #5259.18
..LN5401:
        movl      4(%rax), %eax                                 #5259.24
        cmpl      %eax, %edx                                    #5259.24
        jae       ..B30.65      # Prob 50%                      #5259.24
                                # LOE
..B30.64:                       # Preds ..B30.63
..LN5403:
        xorl      %eax, %eax                                    #5259.54
        movq      -32(%rbp), %rbx                               #5259.54
..___tag_value_NucStrstrSearch.633:                             #
        leave                                                   #5259.54
..___tag_value_NucStrstrSearch.635:                             #
        ret                                                     #5259.54
..___tag_value_NucStrstrSearch.636:                             #
                                # LOE
..B30.65:                       # Preds ..B30.63
..LN5405:
   .loc    3  5260
        movq      -40(%rbp), %rax                               #5260.9
        testq     %rax, %rax                                    #5260.9
        je        ..B30.67      # Prob 50%                      #5260.9
                                # LOE
..B30.66:                       # Preds ..B30.65
..LN5407:
        movq      -56(%rbp), %rax                               #5260.27
..LN5409:
        movq      -40(%rbp), %rdx                               #5260.18
..LN5411:
        movl      4(%rax), %eax                                 #5260.27
..LN5413:
        movl      %eax, (%rdx)                                  #5260.18
                                # LOE
..B30.67:                       # Preds ..B30.66 ..B30.65
..LN5415:
   .loc    3  5261
        movq      -56(%rbp), %rax                               #5261.37
..LN5417:
        movq      -48(%rbp), %rdx                               #5261.52
..LN5419:
        movl      -24(%rbp), %ecx                               #5261.61
..LN5421:
        movl      -16(%rbp), %ebx                               #5261.66
..LN5423:
        movq      %rax, %rdi                                    #5261.20
        movq      %rdx, %rsi                                    #5261.20
        movl      %ecx, %edx                                    #5261.20
        movl      %ebx, %ecx                                    #5261.20
        call      eval_4na_128@PLT                              #5261.20
                                # LOE eax
..B30.122:                      # Preds ..B30.67
        movl      %eax, -112(%rbp)                              #5261.20
                                # LOE
..B30.68:                       # Preds ..B30.122
        movl      -112(%rbp), %eax                              #5261.20
        movq      -32(%rbp), %rbx                               #5261.20
..___tag_value_NucStrstrSearch.638:                             #
        leave                                                   #5261.20
..___tag_value_NucStrstrSearch.640:                             #
        ret                                                     #5261.20
..___tag_value_NucStrstrSearch.641:                             #
                                # LOE
..1.30_0.TAG.09.0.1:
..B30.70:                       # Preds ..B30.5
..LN5425:
   .loc    3  5264
        movq      -56(%rbp), %rax                               #5264.24
..LN5427:
        movl      -16(%rbp), %edx                               #5264.18
..LN5429:
        movl      4(%rax), %eax                                 #5264.24
        cmpl      %eax, %edx                                    #5264.24
        jae       ..B30.72      # Prob 50%                      #5264.24
                                # LOE
..B30.71:                       # Preds ..B30.70
..LN5431:
        xorl      %eax, %eax                                    #5264.54
        movq      -32(%rbp), %rbx                               #5264.54
..___tag_value_NucStrstrSearch.643:                             #
        leave                                                   #5264.54
..___tag_value_NucStrstrSearch.645:                             #
        ret                                                     #5264.54
..___tag_value_NucStrstrSearch.646:                             #
                                # LOE
..B30.72:                       # Preds ..B30.70
..LN5433:
   .loc    3  5265
        movq      -40(%rbp), %rax                               #5265.9
        testq     %rax, %rax                                    #5265.9
        je        ..B30.74      # Prob 50%                      #5265.9
                                # LOE
..B30.73:                       # Preds ..B30.72
..LN5435:
        movq      -56(%rbp), %rax                               #5265.27
..LN5437:
        movq      -40(%rbp), %rdx                               #5265.18
..LN5439:
        movl      4(%rax), %eax                                 #5265.27
..LN5441:
        movl      %eax, (%rdx)                                  #5265.18
                                # LOE
..B30.74:                       # Preds ..B30.73 ..B30.72
..LN5443:
   .loc    3  5266
        movq      -56(%rbp), %rax                               #5266.37
..LN5445:
        movq      -48(%rbp), %rdx                               #5266.52
..LN5447:
        movl      -24(%rbp), %ecx                               #5266.61
..LN5449:
        movl      -16(%rbp), %ebx                               #5266.66
..LN5451:
        movq      %rax, %rdi                                    #5266.20
        movq      %rdx, %rsi                                    #5266.20
        movl      %ecx, %edx                                    #5266.20
        movl      %ebx, %ecx                                    #5266.20
        call      eval_2na_pos@PLT                              #5266.20
                                # LOE eax
..B30.123:                      # Preds ..B30.74
        movl      %eax, -108(%rbp)                              #5266.20
                                # LOE
..B30.75:                       # Preds ..B30.123
        movl      -108(%rbp), %eax                              #5266.20
        movq      -32(%rbp), %rbx                               #5266.20
..___tag_value_NucStrstrSearch.648:                             #
        leave                                                   #5266.20
..___tag_value_NucStrstrSearch.650:                             #
        ret                                                     #5266.20
..___tag_value_NucStrstrSearch.651:                             #
                                # LOE
..1.30_0.TAG.0a.0.1:
..B30.77:                       # Preds ..B30.5
..LN5453:
   .loc    3  5268
        movq      -56(%rbp), %rax                               #5268.24
..LN5455:
        movl      -16(%rbp), %edx                               #5268.18
..LN5457:
        movl      4(%rax), %eax                                 #5268.24
        cmpl      %eax, %edx                                    #5268.24
        jae       ..B30.79      # Prob 50%                      #5268.24
                                # LOE
..B30.78:                       # Preds ..B30.77
..LN5459:
        xorl      %eax, %eax                                    #5268.54
        movq      -32(%rbp), %rbx                               #5268.54
..___tag_value_NucStrstrSearch.653:                             #
        leave                                                   #5268.54
..___tag_value_NucStrstrSearch.655:                             #
        ret                                                     #5268.54
..___tag_value_NucStrstrSearch.656:                             #
                                # LOE
..B30.79:                       # Preds ..B30.77
..LN5461:
   .loc    3  5269
        movq      -40(%rbp), %rax                               #5269.9
        testq     %rax, %rax                                    #5269.9
        je        ..B30.81      # Prob 50%                      #5269.9
                                # LOE
..B30.80:                       # Preds ..B30.79
..LN5463:
        movq      -56(%rbp), %rax                               #5269.27
..LN5465:
        movq      -40(%rbp), %rdx                               #5269.18
..LN5467:
        movl      4(%rax), %eax                                 #5269.27
..LN5469:
        movl      %eax, (%rdx)                                  #5269.18
                                # LOE
..B30.81:                       # Preds ..B30.80 ..B30.79
..LN5471:
   .loc    3  5270
        movq      -56(%rbp), %rax                               #5270.37
..LN5473:
        movq      -48(%rbp), %rdx                               #5270.52
..LN5475:
        movl      -24(%rbp), %ecx                               #5270.61
..LN5477:
        movl      -16(%rbp), %ebx                               #5270.66
..LN5479:
        movq      %rax, %rdi                                    #5270.20
        movq      %rdx, %rsi                                    #5270.20
        movl      %ecx, %edx                                    #5270.20
        movl      %ebx, %ecx                                    #5270.20
        call      eval_4na_pos@PLT                              #5270.20
                                # LOE eax
..B30.124:                      # Preds ..B30.81
        movl      %eax, -104(%rbp)                              #5270.20
                                # LOE
..B30.82:                       # Preds ..B30.124
        movl      -104(%rbp), %eax                              #5270.20
        movq      -32(%rbp), %rbx                               #5270.20
..___tag_value_NucStrstrSearch.658:                             #
        leave                                                   #5270.20
..___tag_value_NucStrstrSearch.660:                             #
        ret                                                     #5270.20
..___tag_value_NucStrstrSearch.661:                             #
                                # LOE
..1.30_0.TAG.0b.0.1:
..B30.84:                       # Preds ..B30.5
..LN5481:
   .loc    3  5272
        movq      -56(%rbp), %rax                               #5272.39
        movq      8(%rax), %rax                                 #5272.39
..LN5483:
        movq      -48(%rbp), %rdx                               #5272.63
..LN5485:
        movl      -24(%rbp), %ecx                               #5272.72
..LN5487:
        movl      -16(%rbp), %ebx                               #5272.77
..LN5489:
        movq      -40(%rbp), %rsi                               #5272.82
..LN5491:
        movq      %rax, %rdi                                    #5272.21
        movq      %rsi, -96(%rbp)                               #5272.21
        movq      %rdx, %rsi                                    #5272.21
        movl      %ecx, %edx                                    #5272.21
        movl      %ebx, %ecx                                    #5272.21
        movq      -96(%rbp), %rax                               #5272.21
        movq      %rax, %r8                                     #5272.21
        call      NucStrstrSearch@PLT                           #5272.21
                                # LOE eax
..B30.125:                      # Preds ..B30.84
        movl      %eax, -76(%rbp)                               #5272.21
                                # LOE
..B30.85:                       # Preds ..B30.125
..LN5493:
        movl      -76(%rbp), %eax                               #5272.13
        movl      %eax, -80(%rbp)                               #5272.13
..LN5495:
   .loc    3  5273
        movq      -56(%rbp), %rax                               #5273.22
        movl      4(%rax), %eax                                 #5273.22
        movl      %eax, -72(%rbp)                               #5273.22
        cmpl      $17, %eax                                     #5273.22
        je        ..B30.87      # Prob 50%                      #5273.22
                                # LOE
..B30.86:                       # Preds ..B30.85
        movl      -72(%rbp), %eax                               #5273.22
        cmpl      $16, %eax                                     #5273.22
        je        ..B30.89      # Prob 50%                      #5273.22
        jmp       ..B30.91      # Prob 100%                     #5273.22
                                # LOE
..B30.87:                       # Preds ..B30.85
..LN5497:
   .loc    3  5276
        movl      -80(%rbp), %eax                               #5276.22
..LN5499:
        testl     %eax, %eax                                    #5276.31
        je        ..B30.91      # Prob 50%                      #5276.31
                                # LOE
..B30.88:                       # Preds ..B30.87
..LN5501:
   .loc    3  5277
        movl      -80(%rbp), %eax                               #5277.28
        movq      -32(%rbp), %rbx                               #5277.28
..___tag_value_NucStrstrSearch.663:                             #
        leave                                                   #5277.28
..___tag_value_NucStrstrSearch.665:                             #
        ret                                                     #5277.28
..___tag_value_NucStrstrSearch.666:                             #
                                # LOE
..B30.89:                       # Preds ..B30.86
..LN5503:
   .loc    3  5280
        movl      -80(%rbp), %eax                               #5280.22
..LN5505:
        testl     %eax, %eax                                    #5280.31
        jne       ..B30.91      # Prob 50%                      #5280.31
                                # LOE
..B30.90:                       # Preds ..B30.89
..LN5507:
   .loc    3  5281
        movl      -80(%rbp), %eax                               #5281.28
        movq      -32(%rbp), %rbx                               #5281.28
..___tag_value_NucStrstrSearch.668:                             #
        leave                                                   #5281.28
..___tag_value_NucStrstrSearch.670:                             #
        ret                                                     #5281.28
..___tag_value_NucStrstrSearch.671:                             #
                                # LOE
..B30.91:                       # Preds ..B30.87 ..B30.89 ..B30.86
..LN5509:
   .loc    3  5284
        movq      -56(%rbp), %rax                               #5284.38
        movq      16(%rax), %rax                                #5284.38
..LN5511:
        movq      -48(%rbp), %rdx                               #5284.63
..LN5513:
        movl      -24(%rbp), %ecx                               #5284.72
..LN5515:
        movl      -16(%rbp), %ebx                               #5284.77
..LN5517:
        movq      -40(%rbp), %rsi                               #5284.82
..LN5519:
        movq      %rax, %rdi                                    #5284.20
        movq      %rsi, -160(%rbp)                              #5284.20
        movq      %rdx, %rsi                                    #5284.20
        movl      %ecx, %edx                                    #5284.20
        movl      %ebx, %ecx                                    #5284.20
        movq      -160(%rbp), %rax                              #5284.20
        movq      %rax, %r8                                     #5284.20
        call      NucStrstrSearch@PLT                           #5284.20
                                # LOE eax
..B30.126:                      # Preds ..B30.91
        movl      %eax, -88(%rbp)                               #5284.20
                                # LOE
..B30.92:                       # Preds ..B30.126
        movl      -88(%rbp), %eax                               #5284.20
        movq      -32(%rbp), %rbx                               #5284.20
..___tag_value_NucStrstrSearch.673:                             #
        leave                                                   #5284.20
..___tag_value_NucStrstrSearch.675:                             #
        ret                                                     #5284.20
..___tag_value_NucStrstrSearch.676:                             #
                                # LOE
..1.30_0.TAG.0c.0.1:
..B30.94:                       # Preds ..B30.5
..LN5521:
   .loc    3  5286
        movq      -56(%rbp), %rax                               #5286.22
        movl      4(%rax), %eax                                 #5286.22
        movl      %eax, -68(%rbp)                               #5286.22
        testl     %eax, %eax                                    #5286.22
        je        ..B30.98      # Prob 50%                      #5286.22
                                # LOE
..B30.95:                       # Preds ..B30.94
        movl      -68(%rbp), %eax                               #5286.22
        cmpl      $13, %eax                                     #5286.22
        je        ..B30.98      # Prob 50%                      #5286.22
                                # LOE
..B30.96:                       # Preds ..B30.95
        movl      -68(%rbp), %eax                               #5286.22
        cmpl      $14, %eax                                     #5286.22
        je        ..B30.103     # Prob 50%                      #5286.22
                                # LOE
..B30.97:                       # Preds ..B30.96
        movl      -68(%rbp), %eax                               #5286.22
        cmpl      $15, %eax                                     #5286.22
        je        ..B30.107     # Prob 50%                      #5286.22
        jmp       ..B30.111     # Prob 100%                     #5286.22
                                # LOE
..B30.98:                       # Preds ..B30.94 ..B30.95
..LN5523:
   .loc    3  5290
        movq      -56(%rbp), %rax                               #5290.43
        movq      8(%rax), %rax                                 #5290.43
..LN5525:
        movq      -48(%rbp), %rdx                               #5290.63
..LN5527:
        movl      -24(%rbp), %ecx                               #5290.72
..LN5529:
        movl      -16(%rbp), %ebx                               #5290.77
..LN5531:
        movq      -40(%rbp), %rsi                               #5290.82
..LN5533:
        movq      %rax, %rdi                                    #5290.25
        movq      %rsi, -152(%rbp)                              #5290.25
        movq      %rdx, %rsi                                    #5290.25
        movl      %ecx, %edx                                    #5290.25
        movl      %ebx, %ecx                                    #5290.25
        movq      -152(%rbp), %rax                              #5290.25
        movq      %rax, %r8                                     #5290.25
        call      NucStrstrSearch@PLT                           #5290.25
                                # LOE eax
..B30.127:                      # Preds ..B30.98
        movl      %eax, -84(%rbp)                               #5290.25
                                # LOE
..B30.99:                       # Preds ..B30.127
..LN5535:
        movl      -84(%rbp), %eax                               #5290.17
        movl      %eax, -80(%rbp)                               #5290.17
..LN5537:
   .loc    3  5291
        movq      -56(%rbp), %rax                               #5291.22
        movl      4(%rax), %eax                                 #5291.22
..LN5539:
        testl     %eax, %eax                                    #5291.42
        jne       ..B30.101     # Prob 50%                      #5291.42
                                # LOE
..B30.100:                      # Preds ..B30.99
..LN5541:
   .loc    3  5292
        movl      -80(%rbp), %eax                               #5292.28
        movq      -32(%rbp), %rbx                               #5292.28
..___tag_value_NucStrstrSearch.678:                             #
        leave                                                   #5292.28
..___tag_value_NucStrstrSearch.680:                             #
        ret                                                     #5292.28
..___tag_value_NucStrstrSearch.681:                             #
                                # LOE
..B30.101:                      # Preds ..B30.99
..LN5543:
   .loc    3  5293
        movl      -80(%rbp), %eax                               #5293.22
..LN5545:
        testl     %eax, %eax                                    #5293.31
        jne       ..B30.111     # Prob 50%                      #5293.31
                                # LOE
..B30.102:                      # Preds ..B30.101
..LN5547:
   .loc    3  5294
        movl      $1, %eax                                      #5294.28
        movq      -32(%rbp), %rbx                               #5294.28
..___tag_value_NucStrstrSearch.683:                             #
        leave                                                   #5294.28
..___tag_value_NucStrstrSearch.685:                             #
        ret                                                     #5294.28
..___tag_value_NucStrstrSearch.686:                             #
                                # LOE
..B30.103:                      # Preds ..B30.96
..LN5549:
   .loc    3  5297
        movq      -56(%rbp), %rax                               #5297.29
        movq      8(%rax), %rax                                 #5297.29
        movl      4(%rax), %eax                                 #5297.29
..LN5551:
        movl      %eax, -164(%rbp)                              #5297.17
..LN5553:
   .loc    3  5298
        movl      -164(%rbp), %eax                              #5298.22
..LN5555:
        movl      -16(%rbp), %edx                               #5298.34
        cmpl      %edx, %eax                                    #5298.34
        jbe       ..B30.105     # Prob 50%                      #5298.34
                                # LOE
..B30.104:                      # Preds ..B30.103
..LN5557:
   .loc    3  5299
        xorl      %eax, %eax                                    #5299.28
        movq      -32(%rbp), %rbx                               #5299.28
..___tag_value_NucStrstrSearch.688:                             #
        leave                                                   #5299.28
..___tag_value_NucStrstrSearch.690:                             #
        ret                                                     #5299.28
..___tag_value_NucStrstrSearch.691:                             #
                                # LOE
..B30.105:                      # Preds ..B30.103
..LN5559:
   .loc    3  5300
        movq      -56(%rbp), %rax                               #5300.42
        movq      8(%rax), %rax                                 #5300.42
..LN5561:
        movq      -48(%rbp), %rdx                               #5300.62
..LN5563:
        movl      -24(%rbp), %ecx                               #5300.71
..LN5565:
        movl      -164(%rbp), %ebx                              #5300.76
..LN5567:
        movq      -40(%rbp), %rsi                               #5300.87
..LN5569:
        movq      %rax, %rdi                                    #5300.24
        movq      %rsi, -176(%rbp)                              #5300.24
        movq      %rdx, %rsi                                    #5300.24
        movl      %ecx, %edx                                    #5300.24
        movl      %ebx, %ecx                                    #5300.24
        movq      -176(%rbp), %rax                              #5300.24
        movq      %rax, %r8                                     #5300.24
        call      NucStrstrSearch@PLT                           #5300.24
                                # LOE eax
..B30.128:                      # Preds ..B30.105
        movl      %eax, -168(%rbp)                              #5300.24
                                # LOE
..B30.106:                      # Preds ..B30.128
        movl      -168(%rbp), %eax                              #5300.24
        movq      -32(%rbp), %rbx                               #5300.24
..___tag_value_NucStrstrSearch.693:                             #
        leave                                                   #5300.24
..___tag_value_NucStrstrSearch.695:                             #
        ret                                                     #5300.24
..___tag_value_NucStrstrSearch.696:                             #
                                # LOE
..B30.107:                      # Preds ..B30.97
..LN5571:
   .loc    3  5302
        movq      -56(%rbp), %rax                               #5302.29
        movq      8(%rax), %rax                                 #5302.29
        movl      4(%rax), %eax                                 #5302.29
..LN5573:
        movl      %eax, -164(%rbp)                              #5302.17
..LN5575:
   .loc    3  5303
        movl      -164(%rbp), %eax                              #5303.22
..LN5577:
        movl      -16(%rbp), %edx                               #5303.34
        cmpl      %edx, %eax                                    #5303.34
        jbe       ..B30.109     # Prob 50%                      #5303.34
                                # LOE
..B30.108:                      # Preds ..B30.107
..LN5579:
   .loc    3  5304
        xorl      %eax, %eax                                    #5304.28
        movq      -32(%rbp), %rbx                               #5304.28
..___tag_value_NucStrstrSearch.698:                             #
        leave                                                   #5304.28
..___tag_value_NucStrstrSearch.700:                             #
        ret                                                     #5304.28
..___tag_value_NucStrstrSearch.701:                             #
                                # LOE
..B30.109:                      # Preds ..B30.107
..LN5581:
   .loc    3  5305
        movq      -56(%rbp), %rax                               #5305.42
        movq      8(%rax), %rax                                 #5305.42
..LN5583:
        movq      -48(%rbp), %rdx                               #5305.62
..LN5585:
   .loc    3  5306
        movl      -16(%rbp), %ecx                               #5306.27
        addl      -24(%rbp), %ecx                               #5306.27
..LN5587:
        movl      -164(%rbp), %ebx                              #5306.33
        negl      %ebx                                          #5306.33
        addl      %ebx, %ecx                                    #5306.33
..LN5589:
        movl      -164(%rbp), %ebx                              #5306.44
..LN5591:
        movq      -40(%rbp), %rsi                               #5306.55
..LN5593:
   .loc    3  5305
        movq      %rax, %rdi                                    #5305.24
        movq      %rsi, -192(%rbp)                              #5305.24
        movq      %rdx, %rsi                                    #5305.24
        movl      %ecx, %edx                                    #5305.24
        movl      %ebx, %ecx                                    #5305.24
        movq      -192(%rbp), %rax                              #5305.24
        movq      %rax, %r8                                     #5305.24
        call      NucStrstrSearch@PLT                           #5305.24
                                # LOE eax
..B30.129:                      # Preds ..B30.109
        movl      %eax, -184(%rbp)                              #5305.24
                                # LOE
..B30.110:                      # Preds ..B30.129
        movl      -184(%rbp), %eax                              #5305.24
        movq      -32(%rbp), %rbx                               #5305.24
..___tag_value_NucStrstrSearch.703:                             #
        leave                                                   #5305.24
..___tag_value_NucStrstrSearch.705:                             #
        ret                                                     #5305.24
..___tag_value_NucStrstrSearch.706:                             #
                                # LOE
..B30.111:                      # Preds ..B30.101 ..B30.97 ..B30.4 ..B30.3 ..B30.2
                                #       ..B30.1
..LN5595:
   .loc    3  5311
        xorl      %eax, %eax                                    #5311.12
        movq      -32(%rbp), %rbx                               #5311.12
..___tag_value_NucStrstrSearch.708:                             #
        leave                                                   #5311.12
..___tag_value_NucStrstrSearch.710:                             #
        ret                                                     #5311.12
        .align    2,0x90
..___tag_value_NucStrstrSearch.711:                             #
                                # LOE
# mark_end;
	.type	NucStrstrSearch,@function
	.size	NucStrstrSearch,.-NucStrstrSearch
.LNNucStrstrSearch:
	.section .data1, "wa"
	.space 24	# pad
	.align 32
..1..TPKT.30_0.0.1:
	.quad	..1.30_0.TAG.00.0.1
	.quad	..1.30_0.TAG.01.0.1
	.quad	..1.30_0.TAG.02.0.1
	.quad	..1.30_0.TAG.03.0.1
	.quad	..1.30_0.TAG.04.0.1
	.quad	..1.30_0.TAG.05.0.1
	.quad	..1.30_0.TAG.06.0.1
	.quad	..1.30_0.TAG.07.0.1
	.quad	..1.30_0.TAG.08.0.1
	.quad	..1.30_0.TAG.09.0.1
	.quad	..1.30_0.TAG.0a.0.1
	.quad	..1.30_0.TAG.0b.0.1
	.quad	..1.30_0.TAG.0c.0.1
	.data
# -- End  NucStrstrSearch
	.bss
	.align 32
	.align 32
fasta_2na_map.0:
	.type	fasta_2na_map.0,@object
	.size	fasta_2na_map.0,128
	.space 128	# pad
	.align 32
fasta_4na_map.0:
	.type	fasta_4na_map.0,@object
	.size	fasta_4na_map.0,128
	.space 128	# pad
	.data
	.align 32
	.align 32
expand_2na.0:
	.word	4369
	.word	4370
	.word	4372
	.word	4376
	.word	4385
	.word	4386
	.word	4388
	.word	4392
	.word	4417
	.word	4418
	.word	4420
	.word	4424
	.word	4481
	.word	4482
	.word	4484
	.word	4488
	.word	4625
	.word	4626
	.word	4628
	.word	4632
	.word	4641
	.word	4642
	.word	4644
	.word	4648
	.word	4673
	.word	4674
	.word	4676
	.word	4680
	.word	4737
	.word	4738
	.word	4740
	.word	4744
	.word	5137
	.word	5138
	.word	5140
	.word	5144
	.word	5153
	.word	5154
	.word	5156
	.word	5160
	.word	5185
	.word	5186
	.word	5188
	.word	5192
	.word	5249
	.word	5250
	.word	5252
	.word	5256
	.word	6161
	.word	6162
	.word	6164
	.word	6168
	.word	6177
	.word	6178
	.word	6180
	.word	6184
	.word	6209
	.word	6210
	.word	6212
	.word	6216
	.word	6273
	.word	6274
	.word	6276
	.word	6280
	.word	8465
	.word	8466
	.word	8468
	.word	8472
	.word	8481
	.word	8482
	.word	8484
	.word	8488
	.word	8513
	.word	8514
	.word	8516
	.word	8520
	.word	8577
	.word	8578
	.word	8580
	.word	8584
	.word	8721
	.word	8722
	.word	8724
	.word	8728
	.word	8737
	.word	8738
	.word	8740
	.word	8744
	.word	8769
	.word	8770
	.word	8772
	.word	8776
	.word	8833
	.word	8834
	.word	8836
	.word	8840
	.word	9233
	.word	9234
	.word	9236
	.word	9240
	.word	9249
	.word	9250
	.word	9252
	.word	9256
	.word	9281
	.word	9282
	.word	9284
	.word	9288
	.word	9345
	.word	9346
	.word	9348
	.word	9352
	.word	10257
	.word	10258
	.word	10260
	.word	10264
	.word	10273
	.word	10274
	.word	10276
	.word	10280
	.word	10305
	.word	10306
	.word	10308
	.word	10312
	.word	10369
	.word	10370
	.word	10372
	.word	10376
	.word	16657
	.word	16658
	.word	16660
	.word	16664
	.word	16673
	.word	16674
	.word	16676
	.word	16680
	.word	16705
	.word	16706
	.word	16708
	.word	16712
	.word	16769
	.word	16770
	.word	16772
	.word	16776
	.word	16913
	.word	16914
	.word	16916
	.word	16920
	.word	16929
	.word	16930
	.word	16932
	.word	16936
	.word	16961
	.word	16962
	.word	16964
	.word	16968
	.word	17025
	.word	17026
	.word	17028
	.word	17032
	.word	17425
	.word	17426
	.word	17428
	.word	17432
	.word	17441
	.word	17442
	.word	17444
	.word	17448
	.word	17473
	.word	17474
	.word	17476
	.word	17480
	.word	17537
	.word	17538
	.word	17540
	.word	17544
	.word	18449
	.word	18450
	.word	18452
	.word	18456
	.word	18465
	.word	18466
	.word	18468
	.word	18472
	.word	18497
	.word	18498
	.word	18500
	.word	18504
	.word	18561
	.word	18562
	.word	18564
	.word	18568
	.word	33041
	.word	33042
	.word	33044
	.word	33048
	.word	33057
	.word	33058
	.word	33060
	.word	33064
	.word	33089
	.word	33090
	.word	33092
	.word	33096
	.word	33153
	.word	33154
	.word	33156
	.word	33160
	.word	33297
	.word	33298
	.word	33300
	.word	33304
	.word	33313
	.word	33314
	.word	33316
	.word	33320
	.word	33345
	.word	33346
	.word	33348
	.word	33352
	.word	33409
	.word	33410
	.word	33412
	.word	33416
	.word	33809
	.word	33810
	.word	33812
	.word	33816
	.word	33825
	.word	33826
	.word	33828
	.word	33832
	.word	33857
	.word	33858
	.word	33860
	.word	33864
	.word	33921
	.word	33922
	.word	33924
	.word	33928
	.word	34833
	.word	34834
	.word	34836
	.word	34840
	.word	34849
	.word	34850
	.word	34852
	.word	34856
	.word	34881
	.word	34882
	.word	34884
	.word	34888
	.word	34945
	.word	34946
	.word	34948
	.word	34952
	.type	expand_2na.0,@object
	.size	expand_2na.0,512
	.section .rodata, "a"
	.align 4
	.align 4
__$U0.0:
	.byte	78
	.byte	117
	.byte	99
	.byte	83
	.byte	116
	.byte	114
	.byte	70
	.byte	97
	.byte	115
	.byte	116
	.byte	97
	.byte	69
	.byte	120
	.byte	112
	.byte	114
	.byte	65
	.byte	108
	.byte	108
	.byte	111
	.byte	99
	.byte	0
	.type	__$U0.0,@object
	.size	__$U0.0,21
	.space 3	# pad
	.align 4
_2__STRING.1.0:
	.byte	110
	.byte	117
	.byte	99
	.byte	115
	.byte	116
	.byte	114
	.byte	115
	.byte	116
	.byte	114
	.byte	46
	.byte	99
	.byte	0
	.type	_2__STRING.1.0,@object
	.size	_2__STRING.1.0,12
	.align 4
_2__STRING.0.0:
	.byte	40
	.byte	32
	.byte	40
	.byte	32
	.byte	115
	.byte	105
	.byte	122
	.byte	101
	.byte	95
	.byte	116
	.byte	32
	.byte	41
	.byte	32
	.byte	38
	.byte	32
	.byte	101
	.byte	32
	.byte	45
	.byte	62
	.byte	32
	.byte	113
	.byte	117
	.byte	101
	.byte	114
	.byte	121
	.byte	32
	.byte	91
	.byte	32
	.byte	48
	.byte	32
	.byte	93
	.byte	32
	.byte	46
	.byte	32
	.byte	112
	.byte	97
	.byte	116
	.byte	116
	.byte	101
	.byte	114
	.byte	110
	.byte	32
	.byte	38
	.byte	32
	.byte	49
	.byte	53
	.byte	32
	.byte	41
	.byte	32
	.byte	61
	.byte	61
	.byte	32
	.byte	48
	.byte	0
	.type	_2__STRING.0.0,@object
	.size	_2__STRING.0.0,54
	.space 2	# pad
	.align 4
_2__STRING.2.0:
	.byte	40
	.byte	32
	.byte	40
	.byte	32
	.byte	115
	.byte	105
	.byte	122
	.byte	101
	.byte	95
	.byte	116
	.byte	32
	.byte	41
	.byte	32
	.byte	38
	.byte	32
	.byte	101
	.byte	32
	.byte	45
	.byte	62
	.byte	32
	.byte	113
	.byte	117
	.byte	101
	.byte	114
	.byte	121
	.byte	32
	.byte	91
	.byte	32
	.byte	48
	.byte	32
	.byte	93
	.byte	32
	.byte	46
	.byte	32
	.byte	109
	.byte	97
	.byte	115
	.byte	107
	.byte	32
	.byte	38
	.byte	32
	.byte	49
	.byte	53
	.byte	32
	.byte	41
	.byte	32
	.byte	61
	.byte	61
	.byte	32
	.byte	48
	.byte	0
	.type	_2__STRING.2.0,@object
	.size	_2__STRING.2.0,51
	.space 1	# pad
	.align 4
__$U1.0:
	.byte	110
	.byte	115
	.byte	115
	.byte	95
	.byte	102
	.byte	97
	.byte	115
	.byte	116
	.byte	97
	.byte	95
	.byte	101
	.byte	120
	.byte	112
	.byte	114
	.byte	0
	.type	__$U1.0,@object
	.size	__$U1.0,15
	.space 1	# pad
	.align 4
_2__STRING.3.0:
	.byte	112
	.byte	32
	.byte	60
	.byte	32
	.byte	101
	.byte	110
	.byte	100
	.byte	0
	.type	_2__STRING.3.0,@object
	.size	_2__STRING.3.0,8
	.align 4
__$U2.0:
	.byte	110
	.byte	115
	.byte	115
	.byte	95
	.byte	112
	.byte	114
	.byte	105
	.byte	109
	.byte	97
	.byte	114
	.byte	121
	.byte	95
	.byte	101
	.byte	120
	.byte	112
	.byte	114
	.byte	0
	.type	__$U2.0,@object
	.size	__$U2.0,17
	.space 3	# pad
	.align 4
__$U3.0:
	.byte	110
	.byte	115
	.byte	115
	.byte	95
	.byte	117
	.byte	110
	.byte	97
	.byte	114
	.byte	121
	.byte	95
	.byte	101
	.byte	120
	.byte	112
	.byte	114
	.byte	0
	.type	__$U3.0,@object
	.size	__$U3.0,15
	.space 1	# pad
	.align 4
_2__STRING.4.0:
	.byte	42
	.byte	32
	.byte	115
	.byte	116
	.byte	97
	.byte	116
	.byte	117
	.byte	115
	.byte	32
	.byte	33
	.byte	61
	.byte	32
	.byte	48
	.byte	32
	.byte	124
	.byte	124
	.byte	32
	.byte	101
	.byte	32
	.byte	45
	.byte	62
	.byte	32
	.byte	115
	.byte	117
	.byte	98
	.byte	32
	.byte	46
	.byte	32
	.byte	101
	.byte	120
	.byte	112
	.byte	114
	.byte	32
	.byte	33
	.byte	61
	.byte	32
	.byte	40
	.byte	40
	.byte	118
	.byte	111
	.byte	105
	.byte	100
	.byte	32
	.byte	42
	.byte	41
	.byte	48
	.byte	41
	.byte	0
	.type	_2__STRING.4.0,@object
	.size	_2__STRING.4.0,48
	.align 4
__$U4.0:
	.byte	110
	.byte	115
	.byte	115
	.byte	95
	.byte	101
	.byte	120
	.byte	112
	.byte	114
	.byte	0
	.type	__$U4.0,@object
	.size	__$U4.0,9
	.space 3	# pad
	.align 4
_2__STRING.5.0:
	.byte	42
	.byte	32
	.byte	101
	.byte	120
	.byte	112
	.byte	114
	.byte	32
	.byte	33
	.byte	61
	.byte	32
	.byte	40
	.byte	40
	.byte	118
	.byte	111
	.byte	105
	.byte	100
	.byte	32
	.byte	42
	.byte	41
	.byte	48
	.byte	41
	.byte	0
	.type	_2__STRING.5.0,@object
	.size	_2__STRING.5.0,22
	.space 2	# pad
	.align 4
_2__STRING.6.0:
	.byte	42
	.byte	32
	.byte	115
	.byte	116
	.byte	97
	.byte	116
	.byte	117
	.byte	115
	.byte	32
	.byte	33
	.byte	61
	.byte	32
	.byte	48
	.byte	32
	.byte	124
	.byte	124
	.byte	32
	.byte	101
	.byte	32
	.byte	45
	.byte	62
	.byte	32
	.byte	98
	.byte	111
	.byte	111
	.byte	108
	.byte	101
	.byte	97
	.byte	110
	.byte	32
	.byte	46
	.byte	32
	.byte	114
	.byte	105
	.byte	103
	.byte	104
	.byte	116
	.byte	32
	.byte	33
	.byte	61
	.byte	32
	.byte	40
	.byte	40
	.byte	118
	.byte	111
	.byte	105
	.byte	100
	.byte	32
	.byte	42
	.byte	41
	.byte	48
	.byte	41
	.byte	0
	.type	_2__STRING.6.0,@object
	.size	_2__STRING.6.0,53
	.space 3	# pad
	.align 4
_2__STRING.7.0:
	.byte	65
	.byte	67
	.byte	71
	.byte	84
	.byte	0
	.type	_2__STRING.7.0,@object
	.size	_2__STRING.7.0,5
	.space 3	# pad
	.align 4
_2__STRING.8.0:
	.byte	45
	.byte	65
	.byte	67
	.byte	77
	.byte	71
	.byte	82
	.byte	83
	.byte	86
	.byte	84
	.byte	87
	.byte	89
	.byte	72
	.byte	75
	.byte	68
	.byte	66
	.byte	78
	.byte	0
	.type	_2__STRING.8.0,@object
	.size	_2__STRING.8.0,17
	.space 3	# pad
	.align 4
__$U5.0:
	.byte	101
	.byte	118
	.byte	97
	.byte	108
	.byte	95
	.byte	50
	.byte	110
	.byte	97
	.byte	95
	.byte	56
	.byte	0
	.type	__$U5.0,@object
	.size	__$U5.0,11
	.space 1	# pad
	.align 4
_2__STRING.9.0:
	.byte	108
	.byte	101
	.byte	110
	.byte	32
	.byte	62
	.byte	61
	.byte	32
	.byte	113
	.byte	108
	.byte	101
	.byte	110
	.byte	0
	.type	_2__STRING.9.0,@object
	.size	_2__STRING.9.0,12
	.align 4
__$U6.0:
	.byte	101
	.byte	118
	.byte	97
	.byte	108
	.byte	95
	.byte	50
	.byte	110
	.byte	97
	.byte	95
	.byte	49
	.byte	54
	.byte	0
	.type	__$U6.0,@object
	.size	__$U6.0,12
	.align 4
__$U7.0:
	.byte	101
	.byte	118
	.byte	97
	.byte	108
	.byte	95
	.byte	50
	.byte	110
	.byte	97
	.byte	95
	.byte	51
	.byte	50
	.byte	0
	.type	__$U7.0,@object
	.size	__$U7.0,12
	.align 4
__$U8.0:
	.byte	101
	.byte	118
	.byte	97
	.byte	108
	.byte	95
	.byte	50
	.byte	110
	.byte	97
	.byte	95
	.byte	54
	.byte	52
	.byte	0
	.type	__$U8.0,@object
	.size	__$U8.0,12
	.align 4
__$U9.0:
	.byte	101
	.byte	118
	.byte	97
	.byte	108
	.byte	95
	.byte	50
	.byte	110
	.byte	97
	.byte	95
	.byte	49
	.byte	50
	.byte	56
	.byte	0
	.type	__$U9.0,@object
	.size	__$U9.0,13
	.space 3	# pad
	.align 4
__$Ua.0:
	.byte	101
	.byte	118
	.byte	97
	.byte	108
	.byte	95
	.byte	50
	.byte	110
	.byte	97
	.byte	95
	.byte	112
	.byte	111
	.byte	115
	.byte	0
	.type	__$Ua.0,@object
	.size	__$Ua.0,13
	.space 3	# pad
	.align 4
__$Ub.0:
	.byte	101
	.byte	118
	.byte	97
	.byte	108
	.byte	95
	.byte	52
	.byte	110
	.byte	97
	.byte	95
	.byte	49
	.byte	54
	.byte	0
	.type	__$Ub.0,@object
	.size	__$Ub.0,12
	.align 4
__$Uc.0:
	.byte	101
	.byte	118
	.byte	97
	.byte	108
	.byte	95
	.byte	52
	.byte	110
	.byte	97
	.byte	95
	.byte	51
	.byte	50
	.byte	0
	.type	__$Uc.0,@object
	.size	__$Uc.0,12
	.align 4
__$Ud.0:
	.byte	101
	.byte	118
	.byte	97
	.byte	108
	.byte	95
	.byte	52
	.byte	110
	.byte	97
	.byte	95
	.byte	54
	.byte	52
	.byte	0
	.type	__$Ud.0,@object
	.size	__$Ud.0,12
	.align 4
__$Ue.0:
	.byte	101
	.byte	118
	.byte	97
	.byte	108
	.byte	95
	.byte	52
	.byte	110
	.byte	97
	.byte	95
	.byte	49
	.byte	50
	.byte	56
	.byte	0
	.type	__$Ue.0,@object
	.size	__$Ue.0,13
	.space 3	# pad
	.align 4
__$Uf.0:
	.byte	101
	.byte	118
	.byte	97
	.byte	108
	.byte	95
	.byte	52
	.byte	110
	.byte	97
	.byte	95
	.byte	112
	.byte	111
	.byte	115
	.byte	0
	.type	__$Uf.0,@object
	.size	__$Uf.0,13
	.data
	.section .note.GNU-stack, ""
// -- Begin DWARF2 SEGMENT .debug_info
	.section .debug_info
.debug_info_seg:
	.align 1
	.4byte 0x00002915
	.2byte 0x0002
	.4byte .debug_abbrev_seg
	.byte 0x08
.DWinfo0:
//	DW_TAG_compile_unit:
	.byte 0x01
//	DW_AT_comp_dir:
	.8byte 0x6f722f656d6f682f
	.8byte 0x642f72656d726164
	.8byte 0x746e692f6c657665
	.8byte 0x73612f6c616e7265
	.8byte 0x2f65636172742d6d
	.8byte 0x726165732f617273
	.2byte 0x6863
	.byte 0x00
//	DW_AT_language:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x747372747363756e
	.4byte 0x00632e72
//	DW_AT_producer:
	.8byte 0x2952286c65746e49
	.8byte 0x6d6f43202b2b4320
	.8byte 0x72502072656c6970
	.8byte 0x6e6f69737365666f
	.8byte 0x6120726f66206c61
	.8byte 0x69746163696c7070
	.8byte 0x6e6e757220736e6f
	.8byte 0x49206e6f20676e69
	.8byte 0x202952286c65746e
	.8byte 0x73726556202c3436
	.8byte 0x302e3131206e6f69
	.8byte 0x6c69754220202020
	.8byte 0x3130393030322064
	.8byte 0x46200a7325203133
	.8byte 0x6e61522073657869
	.8byte 0x74616c6552736567
	.4byte 0x0a657669
	.byte 0x00
//	DW_AT_stmt_list:
	.4byte .debug_line_seg
.DWinfo31:
//	DW_TAG_typedef:
	.byte 0x02
//	DW_AT_decl_line:
	.byte 0x26
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x00745f3631746e69
//	DW_AT_type:
	.4byte 0x00002266
.DWinfo30:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x145f
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x747372745363754e
	.8byte 0x0068637261655372
	.8byte 0x747372745363754e
	.8byte 0x0068637261655372
//	DW_AT_low_pc:
	.8byte NucStrstrSearch
//	DW_AT_high_pc:
	.8byte .LNNucStrstrSearch
//	DW_AT_external:
	.byte 0x01
.DWinfo32:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x145f
//	DW_AT_decl_column:
	.byte 0x28
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002889
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x48
.DWinfo33:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x1460
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x0000286d
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo34:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x1460
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
.DWinfo35:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x1460
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo36:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x1460
//	DW_AT_decl_column:
	.byte 0x4e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002893
//	DW_AT_name:
	.8byte 0x006e656c666c6573
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo37:
//	DW_TAG_lexical_block:
	.byte 0x05
//	DW_AT_decl_line:
	.2byte 0x1463
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.8byte ..LN5179
//	DW_AT_high_pc:
	.8byte ..LN5595
.DWinfo38:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1464
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6e756f66
	.2byte 0x0064
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fb07603
.DWinfo39:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1465
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x656c5f6174736166
	.2byte 0x006e
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.4byte 0x7edc7603
	.byte 0x00
	.byte 0x00
.DWinfo29:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x1350
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x616e345f6c617665
	.4byte 0x736f705f
	.byte 0x00
	.8byte 0x616e345f6c617665
	.4byte 0x736f705f
	.byte 0x00
//	DW_AT_low_pc:
	.8byte eval_4na_pos
//	DW_AT_high_pc:
	.8byte .LNeval_4na_pos
//	DW_AT_external:
	.byte 0x00
.DWinfo40:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x1350
//	DW_AT_decl_column:
	.byte 0x2b
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002863
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo41:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x1351
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x0000286d
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo42:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x1351
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
.DWinfo43:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x1351
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo44:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1360
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x66667562
	.2byte 0x7265
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7df07603
.DWinfo45:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1360
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6972
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7dc07603
.DWinfo46:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1360
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6a72
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7dd07603
.DWinfo47:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1361
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3070
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e807603
.DWinfo48:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1361
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3170
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ea07603
.DWinfo49:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1361
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3270
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ec07603
.DWinfo50:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1361
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3370
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ee07603
.DWinfo51:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1361
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x306d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e907603
.DWinfo52:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1361
//	DW_AT_decl_column:
	.byte 0x22
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x316d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7eb07603
.DWinfo53:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1361
//	DW_AT_decl_column:
	.byte 0x26
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x326d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ed07603
.DWinfo54:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1361
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x336d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ef07603
.DWinfo55:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1364
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6172
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x40
.DWinfo56:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1364
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6272
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fbc7603
.DWinfo57:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1364
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6372
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fb87603
.DWinfo58:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1364
//	DW_AT_decl_column:
	.byte 0x15
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6472
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f907603
.DWinfo59:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1367
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x737361705f6d756e
	.2byte 0x7365
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x44
.DWinfo60:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1367
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x706f7473
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.4byte 0x7fb47603
.DWinfo61:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x136b
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x72617473
	.2byte 0x0074
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.4byte 0x7fb07603
.DWinfo62:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x136f
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa87603
.DWinfo63:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1371
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa07603
.DWinfo64:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1371
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00637273
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7f987603
.DWinfo65:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1372
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6e656c71
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
	.byte 0x00
.DWinfo28:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x1272
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x616e345f6c617665
	.4byte 0x3832315f
	.byte 0x00
	.8byte 0x616e345f6c617665
	.4byte 0x3832315f
	.byte 0x00
//	DW_AT_low_pc:
	.8byte eval_4na_128
//	DW_AT_high_pc:
	.8byte .LNeval_4na_128
//	DW_AT_external:
	.byte 0x00
.DWinfo66:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x1272
//	DW_AT_decl_column:
	.byte 0x2b
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002863
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo67:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x1273
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x0000286d
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
.DWinfo68:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x1273
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo69:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x1273
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x78
.DWinfo70:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1281
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x66667562
	.2byte 0x7265
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e807603
.DWinfo71:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1281
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6972
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7dd07603
.DWinfo72:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1281
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6a72
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7de07603
.DWinfo73:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1282
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3070
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e907603
.DWinfo74:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1282
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3170
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7eb07603
.DWinfo75:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1282
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3270
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ed07603
.DWinfo76:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1282
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3370
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ef07603
.DWinfo77:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1282
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x306d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ea07603
.DWinfo78:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1282
//	DW_AT_decl_column:
	.byte 0x22
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x316d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ec07603
.DWinfo79:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1282
//	DW_AT_decl_column:
	.byte 0x26
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x326d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ee07603
.DWinfo80:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1282
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x336d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7f807603
.DWinfo81:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1285
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6172
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x4c
.DWinfo82:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1285
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6272
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x48
.DWinfo83:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1285
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6372
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x44
.DWinfo84:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1285
//	DW_AT_decl_column:
	.byte 0x15
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6472
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fa07603
.DWinfo85:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1288
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x737361705f6d756e
	.2byte 0x7365
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo86:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1288
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x706f7473
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x40
.DWinfo87:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x128b
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fb87603
.DWinfo88:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x128d
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fb07603
.DWinfo89:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x128d
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00637273
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa87603
.DWinfo90:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x128e
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6e656c71
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
	.byte 0x00
.DWinfo27:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x1193
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x616e345f6c617665
	.4byte 0x0034365f
	.8byte 0x616e345f6c617665
	.4byte 0x0034365f
//	DW_AT_low_pc:
	.8byte eval_4na_64
//	DW_AT_high_pc:
	.8byte .LNeval_4na_64
//	DW_AT_external:
	.byte 0x00
.DWinfo91:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x1193
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002863
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo92:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x1194
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x0000286d
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
.DWinfo93:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x1194
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo94:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x1194
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x78
.DWinfo95:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11a3
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x66667562
	.2byte 0x7265
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7de07603
.DWinfo96:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11a3
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6972
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7dc07603
.DWinfo97:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11a3
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6a72
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7dd07603
.DWinfo98:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11a4
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3070
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7df07603
.DWinfo99:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11a4
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3170
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e907603
.DWinfo100:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11a4
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3270
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7eb07603
.DWinfo101:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11a4
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3370
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ed07603
.DWinfo102:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11a4
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x306d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e807603
.DWinfo103:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11a4
//	DW_AT_decl_column:
	.byte 0x22
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x316d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ea07603
.DWinfo104:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11a4
//	DW_AT_decl_column:
	.byte 0x26
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x326d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ec07603
.DWinfo105:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11a4
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x336d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ee07603
.DWinfo106:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11a7
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6172
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x48
.DWinfo107:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11a7
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6272
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x44
.DWinfo108:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11a7
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6372
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x40
.DWinfo109:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11a7
//	DW_AT_decl_column:
	.byte 0x15
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6472
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fb87603
.DWinfo110:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11aa
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x737361705f6d756e
	.2byte 0x7365
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x4c
.DWinfo111:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11aa
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x706f7473
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.4byte 0x7fbc7603
.DWinfo112:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11ad
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fb07603
.DWinfo113:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11af
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa87603
.DWinfo114:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11af
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00637273
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa07603
.DWinfo115:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x11b0
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6e656c71
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
.DWinfo116:
//	DW_TAG_lexical_block:
	.byte 0x05
//	DW_AT_decl_line:
	.2byte 0x120a
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.8byte ..LN4367
//	DW_AT_high_pc:
	.8byte ..LN4451
.DWinfo117:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x121b
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6166
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f807603
.DWinfo118:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x121c
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6266
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f847603
.DWinfo119:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x121d
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6366
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f887603
.DWinfo120:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x121e
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6466
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f8c7603
	.byte 0x00
	.byte 0x00
.DWinfo26:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x10b5
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x616e345f6c617665
	.4byte 0x0032335f
	.8byte 0x616e345f6c617665
	.4byte 0x0032335f
//	DW_AT_low_pc:
	.8byte eval_4na_32
//	DW_AT_high_pc:
	.8byte .LNeval_4na_32
//	DW_AT_external:
	.byte 0x00
.DWinfo121:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x10b5
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002863
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo122:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x10b6
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x0000286d
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
.DWinfo123:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x10b6
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo124:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x10b6
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x78
.DWinfo125:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10c4
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x66667562
	.2byte 0x7265
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7de07603
.DWinfo126:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10c4
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6972
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7dc07603
.DWinfo127:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10c4
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6a72
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7dd07603
.DWinfo128:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10c5
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3070
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7df07603
.DWinfo129:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10c5
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3170
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e907603
.DWinfo130:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10c5
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3270
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7eb07603
.DWinfo131:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10c5
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3370
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ed07603
.DWinfo132:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10c5
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x306d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e807603
.DWinfo133:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10c5
//	DW_AT_decl_column:
	.byte 0x22
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x316d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ea07603
.DWinfo134:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10c5
//	DW_AT_decl_column:
	.byte 0x26
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x326d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ec07603
.DWinfo135:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10c5
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x336d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ee07603
.DWinfo136:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10c8
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6172
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x48
.DWinfo137:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10c8
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6272
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x44
.DWinfo138:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10c8
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6372
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x40
.DWinfo139:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10c8
//	DW_AT_decl_column:
	.byte 0x15
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6472
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fb87603
.DWinfo140:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10cb
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x737361705f6d756e
	.2byte 0x7365
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x4c
.DWinfo141:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10cb
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x706f7473
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.4byte 0x7fbc7603
.DWinfo142:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10ce
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fb07603
.DWinfo143:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10d0
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa87603
.DWinfo144:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10d0
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00637273
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa07603
.DWinfo145:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x10d1
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6e656c71
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
.DWinfo146:
//	DW_TAG_lexical_block:
	.byte 0x05
//	DW_AT_decl_line:
	.2byte 0x112b
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.8byte ..LN4029
//	DW_AT_high_pc:
	.8byte ..LN4113
.DWinfo147:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x113c
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6166
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f807603
.DWinfo148:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x113d
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6266
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f847603
.DWinfo149:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x113e
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6366
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f887603
.DWinfo150:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x113f
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6466
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f8c7603
	.byte 0x00
	.byte 0x00
.DWinfo25:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x0fd7
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x616e345f6c617665
	.4byte 0x0036315f
	.8byte 0x616e345f6c617665
	.4byte 0x0036315f
//	DW_AT_low_pc:
	.8byte eval_4na_16
//	DW_AT_high_pc:
	.8byte .LNeval_4na_16
//	DW_AT_external:
	.byte 0x00
.DWinfo151:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0fd7
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002863
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo152:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0fd8
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x0000286d
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo153:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0fd8
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
.DWinfo154:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0fd8
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo155:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0fe6
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x66667562
	.2byte 0x7265
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7de07603
.DWinfo156:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0fe6
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6972
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7dc07603
.DWinfo157:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0fe6
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6a72
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7dd07603
.DWinfo158:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0fe7
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3070
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7df07603
.DWinfo159:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0fe7
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3170
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e907603
.DWinfo160:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0fe7
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3270
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7eb07603
.DWinfo161:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0fe7
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3370
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ed07603
.DWinfo162:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0fe7
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x306d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e807603
.DWinfo163:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0fe7
//	DW_AT_decl_column:
	.byte 0x22
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x316d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ea07603
.DWinfo164:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0fe7
//	DW_AT_decl_column:
	.byte 0x26
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x326d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ec07603
.DWinfo165:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0fe7
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x336d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ee07603
.DWinfo166:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0fea
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6172
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x40
.DWinfo167:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0fea
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6272
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fbc7603
.DWinfo168:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0fea
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6372
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fb87603
.DWinfo169:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0fea
//	DW_AT_decl_column:
	.byte 0x15
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6472
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fb07603
.DWinfo170:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0fed
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x737361705f6d756e
	.2byte 0x7365
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x44
.DWinfo171:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0fed
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x706f7473
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.4byte 0x7fb47603
.DWinfo172:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0ff2
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa87603
.DWinfo173:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0ff2
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00637273
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa07603
.DWinfo174:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0ff3
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6e656c71
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
.DWinfo175:
//	DW_TAG_lexical_block:
	.byte 0x05
//	DW_AT_decl_line:
	.2byte 0x104d
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.8byte ..LN3725
//	DW_AT_high_pc:
	.8byte ..LN3809
.DWinfo176:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x105e
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6166
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f807603
.DWinfo177:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x105f
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6266
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f847603
.DWinfo178:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1060
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6366
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f887603
.DWinfo179:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x1061
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6466
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f8c7603
	.byte 0x00
	.byte 0x00
.DWinfo24:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x0f95
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x0000279f
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x75625f656d697270
	.8byte 0x616e345f72656666
	.byte 0x00
	.8byte 0x75625f656d697270
	.8byte 0x616e345f72656666
	.byte 0x00
//	DW_AT_low_pc:
	.8byte prime_buffer_4na
//	DW_AT_high_pc:
	.8byte .LNprime_buffer_4na
//	DW_AT_external:
	.byte 0x00
.DWinfo180:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0f95
//	DW_AT_decl_column:
	.byte 0x2b
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_name:
	.4byte 0x00637273
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo181:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0f95
//	DW_AT_decl_column:
	.byte 0x3f
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_name:
	.4byte 0x6f6e6769
	.2byte 0x6572
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x78
.DWinfo182:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0f97
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00706d74
//	DW_AT_type:
	.4byte 0x00002574
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo183:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0f98
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x66667562
	.2byte 0x7265
	.byte 0x00
//	DW_AT_type:
	.4byte 0x0000279f
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
	.byte 0x00
.DWinfo23:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x0e4c
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x616e325f6c617665
	.4byte 0x736f705f
	.byte 0x00
	.8byte 0x616e325f6c617665
	.4byte 0x736f705f
	.byte 0x00
//	DW_AT_low_pc:
	.8byte eval_2na_pos
//	DW_AT_high_pc:
	.8byte .LNeval_2na_pos
//	DW_AT_external:
	.byte 0x00
.DWinfo184:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0e4c
//	DW_AT_decl_column:
	.byte 0x2b
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002863
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo185:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0e4d
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x0000286d
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo186:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0e4d
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
.DWinfo187:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0e4d
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo188:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e5c
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x66667562
	.2byte 0x7265
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7df07603
.DWinfo189:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e5c
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6972
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7dd07603
.DWinfo190:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e5d
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3070
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e807603
.DWinfo191:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e5d
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3170
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ea07603
.DWinfo192:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e5d
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3270
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ec07603
.DWinfo193:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e5d
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3370
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ee07603
.DWinfo194:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e5d
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x306d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e907603
.DWinfo195:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e5d
//	DW_AT_decl_column:
	.byte 0x22
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x316d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7eb07603
.DWinfo196:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e5d
//	DW_AT_decl_column:
	.byte 0x26
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x326d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ed07603
.DWinfo197:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e5d
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x336d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ef07603
.DWinfo198:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e60
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6172
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x40
.DWinfo199:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e60
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6272
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fbc7603
.DWinfo200:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e60
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6372
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fb87603
.DWinfo201:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e60
//	DW_AT_decl_column:
	.byte 0x15
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6472
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f947603
.DWinfo202:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e63
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x737361705f6d756e
	.2byte 0x7365
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x44
.DWinfo203:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e63
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x706f7473
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.4byte 0x7fb47603
.DWinfo204:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e67
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x72617473
	.2byte 0x0074
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.4byte 0x7fb07603
.DWinfo205:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e6d
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6d616c73
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f907603
.DWinfo206:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e6e
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa87603
.DWinfo207:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e71
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa07603
.DWinfo208:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e71
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00637273
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7f987603
.DWinfo209:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0e72
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6e656c71
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
	.byte 0x00
.DWinfo22:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x0d27
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x616e325f6c617665
	.4byte 0x3832315f
	.byte 0x00
	.8byte 0x616e325f6c617665
	.4byte 0x3832315f
	.byte 0x00
//	DW_AT_low_pc:
	.8byte eval_2na_128
//	DW_AT_high_pc:
	.8byte .LNeval_2na_128
//	DW_AT_external:
	.byte 0x00
.DWinfo210:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0d27
//	DW_AT_decl_column:
	.byte 0x2b
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002863
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo211:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0d28
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x0000286d
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
.DWinfo212:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0d28
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo213:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0d28
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x78
.DWinfo214:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d36
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x66667562
	.2byte 0x7265
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e807603
.DWinfo215:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d36
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6972
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7de07603
.DWinfo216:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d37
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3070
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e907603
.DWinfo217:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d37
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3170
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7eb07603
.DWinfo218:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d37
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3270
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ed07603
.DWinfo219:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d37
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3370
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ef07603
.DWinfo220:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d37
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x306d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ea07603
.DWinfo221:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d37
//	DW_AT_decl_column:
	.byte 0x22
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x316d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ec07603
.DWinfo222:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d37
//	DW_AT_decl_column:
	.byte 0x26
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x326d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ee07603
.DWinfo223:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d37
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x336d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7f807603
.DWinfo224:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d3a
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6172
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x4c
.DWinfo225:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d3a
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6272
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x48
.DWinfo226:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d3a
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6372
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x44
.DWinfo227:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d3a
//	DW_AT_decl_column:
	.byte 0x15
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6472
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fa47603
.DWinfo228:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d3d
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x737361705f6d756e
	.2byte 0x7365
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo229:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d3d
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x706f7473
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x40
.DWinfo230:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d42
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6d616c73
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fa07603
.DWinfo231:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d43
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fb87603
.DWinfo232:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d46
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fb07603
.DWinfo233:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d46
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00637273
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa87603
.DWinfo234:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0d47
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6e656c71
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
	.byte 0x00
.DWinfo21:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x0c01
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x616e325f6c617665
	.4byte 0x0034365f
	.8byte 0x616e325f6c617665
	.4byte 0x0034365f
//	DW_AT_low_pc:
	.8byte eval_2na_64
//	DW_AT_high_pc:
	.8byte .LNeval_2na_64
//	DW_AT_external:
	.byte 0x00
.DWinfo235:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0c01
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002863
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo236:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0c02
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x0000286d
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo237:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0c02
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
.DWinfo238:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0c02
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo239:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c11
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x66667562
	.2byte 0x7265
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7dd07603
.DWinfo240:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c11
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6972
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7dc07603
.DWinfo241:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c12
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3070
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7de07603
.DWinfo242:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c12
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3170
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e807603
.DWinfo243:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c12
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3270
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ea07603
.DWinfo244:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c12
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3370
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ec07603
.DWinfo245:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c12
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x306d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7df07603
.DWinfo246:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c12
//	DW_AT_decl_column:
	.byte 0x22
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x316d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e907603
.DWinfo247:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c12
//	DW_AT_decl_column:
	.byte 0x26
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x326d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7eb07603
.DWinfo248:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c12
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x336d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ed07603
.DWinfo249:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c15
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6172
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x40
.DWinfo250:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c15
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6272
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fbc7603
.DWinfo251:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c15
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6372
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fb87603
.DWinfo252:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c15
//	DW_AT_decl_column:
	.byte 0x15
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6472
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fb07603
.DWinfo253:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c18
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x737361705f6d756e
	.2byte 0x7365
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x44
.DWinfo254:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c18
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x706f7473
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.4byte 0x7fb47603
.DWinfo255:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c1d
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6d616c73
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f907603
.DWinfo256:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c1e
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa87603
.DWinfo257:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c21
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa07603
.DWinfo258:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c21
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00637273
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7f987603
.DWinfo259:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c22
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6e656c71
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
.DWinfo260:
//	DW_TAG_lexical_block:
	.byte 0x05
//	DW_AT_decl_line:
	.2byte 0x0c7a
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.8byte ..LN2693
//	DW_AT_high_pc:
	.8byte ..LN2779
.DWinfo261:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c8b
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6166
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7ef07603
.DWinfo262:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c8c
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6266
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7ef47603
.DWinfo263:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c8d
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6366
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7ef87603
.DWinfo264:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0c8e
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6466
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7efc7603
	.byte 0x00
	.byte 0x00
.DWinfo20:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x0adc
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x616e325f6c617665
	.4byte 0x0032335f
	.8byte 0x616e325f6c617665
	.4byte 0x0032335f
//	DW_AT_low_pc:
	.8byte eval_2na_32
//	DW_AT_high_pc:
	.8byte .LNeval_2na_32
//	DW_AT_external:
	.byte 0x00
.DWinfo265:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0adc
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002863
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo266:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0add
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x0000286d
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo267:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0add
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
.DWinfo268:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0add
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo269:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0aeb
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x66667562
	.2byte 0x7265
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7dd07603
.DWinfo270:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0aeb
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6972
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7dc07603
.DWinfo271:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0aec
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3070
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7de07603
.DWinfo272:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0aec
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3170
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e807603
.DWinfo273:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0aec
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3270
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ea07603
.DWinfo274:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0aec
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3370
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ec07603
.DWinfo275:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0aec
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x306d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7df07603
.DWinfo276:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0aec
//	DW_AT_decl_column:
	.byte 0x22
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x316d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e907603
.DWinfo277:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0aec
//	DW_AT_decl_column:
	.byte 0x26
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x326d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7eb07603
.DWinfo278:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0aec
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x336d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ed07603
.DWinfo279:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0aef
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6172
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x40
.DWinfo280:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0aef
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6272
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fbc7603
.DWinfo281:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0aef
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6372
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fb87603
.DWinfo282:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0aef
//	DW_AT_decl_column:
	.byte 0x15
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6472
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fb07603
.DWinfo283:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0af2
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x737361705f6d756e
	.2byte 0x7365
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x44
.DWinfo284:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0af2
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x706f7473
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.4byte 0x7fb47603
.DWinfo285:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0af7
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6d616c73
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f907603
.DWinfo286:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0af8
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa87603
.DWinfo287:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0afb
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa07603
.DWinfo288:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0afb
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00637273
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7f987603
.DWinfo289:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0afc
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6e656c71
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
.DWinfo290:
//	DW_TAG_lexical_block:
	.byte 0x05
//	DW_AT_decl_line:
	.2byte 0x0b54
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.8byte ..LN2359
//	DW_AT_high_pc:
	.8byte ..LN2445
.DWinfo291:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0b65
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6166
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7ef07603
.DWinfo292:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0b66
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6266
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7ef47603
.DWinfo293:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0b67
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6366
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7ef87603
.DWinfo294:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0b68
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6466
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7efc7603
	.byte 0x00
	.byte 0x00
.DWinfo19:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x09b7
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x616e325f6c617665
	.4byte 0x0036315f
	.8byte 0x616e325f6c617665
	.4byte 0x0036315f
//	DW_AT_low_pc:
	.8byte eval_2na_16
//	DW_AT_high_pc:
	.8byte .LNeval_2na_16
//	DW_AT_external:
	.byte 0x00
.DWinfo295:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x09b7
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002863
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo296:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x09b8
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x0000286d
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo297:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x09b8
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
.DWinfo298:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x09b8
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo299:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09c6
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x66667562
	.2byte 0x7265
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7dd07603
.DWinfo300:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09c6
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6972
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7dc07603
.DWinfo301:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09c7
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3070
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7de07603
.DWinfo302:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09c7
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3170
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e807603
.DWinfo303:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09c7
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3270
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ea07603
.DWinfo304:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09c7
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3370
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ec07603
.DWinfo305:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09c7
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x306d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7df07603
.DWinfo306:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09c7
//	DW_AT_decl_column:
	.byte 0x22
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x316d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e907603
.DWinfo307:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09c7
//	DW_AT_decl_column:
	.byte 0x26
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x326d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7eb07603
.DWinfo308:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09c7
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x336d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ed07603
.DWinfo309:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09ca
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6172
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x40
.DWinfo310:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09ca
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6272
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fbc7603
.DWinfo311:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09ca
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6372
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fb87603
.DWinfo312:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09ca
//	DW_AT_decl_column:
	.byte 0x15
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6472
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fb07603
.DWinfo313:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09cd
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x737361705f6d756e
	.2byte 0x7365
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x44
.DWinfo314:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09cd
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x706f7473
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.4byte 0x7fb47603
.DWinfo315:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09d2
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6d616c73
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f907603
.DWinfo316:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09d3
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa87603
.DWinfo317:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09d6
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa07603
.DWinfo318:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09d6
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00637273
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7f987603
.DWinfo319:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x09d7
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6e656c71
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
.DWinfo320:
//	DW_TAG_lexical_block:
	.byte 0x05
//	DW_AT_decl_line:
	.2byte 0x0a2f
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.8byte ..LN2033
//	DW_AT_high_pc:
	.8byte ..LN2119
.DWinfo321:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0a40
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6166
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7ef07603
.DWinfo322:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0a41
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6266
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7ef47603
.DWinfo323:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0a42
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6366
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7ef87603
.DWinfo324:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0a43
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6466
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7efc7603
	.byte 0x00
	.byte 0x00
.DWinfo18:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x0892
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x616e325f6c617665
	.2byte 0x385f
	.byte 0x00
	.8byte 0x616e325f6c617665
	.2byte 0x385f
	.byte 0x00
//	DW_AT_low_pc:
	.8byte eval_2na_8
//	DW_AT_high_pc:
	.8byte .LNeval_2na_8
//	DW_AT_external:
	.byte 0x00
.DWinfo325:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0892
//	DW_AT_decl_column:
	.byte 0x29
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002863
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo326:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0893
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x0000286d
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo327:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0893
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
.DWinfo328:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0893
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo329:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08a1
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x66667562
	.2byte 0x7265
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7de07603
.DWinfo330:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08a1
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6972
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7dd07603
.DWinfo331:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08a2
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3070
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7df07603
.DWinfo332:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08a2
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3170
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e907603
.DWinfo333:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08a2
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3270
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7eb07603
.DWinfo334:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08a2
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x3370
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ed07603
.DWinfo335:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08a2
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x306d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7e807603
.DWinfo336:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08a2
//	DW_AT_decl_column:
	.byte 0x22
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x316d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ea07603
.DWinfo337:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08a2
//	DW_AT_decl_column:
	.byte 0x26
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x326d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ec07603
.DWinfo338:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08a2
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x336d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002877
//	DW_AT_location:
	.4byte 0x7ee07603
.DWinfo339:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08a5
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6172
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x40
.DWinfo340:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08a5
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6272
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fbc7603
.DWinfo341:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08a5
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6372
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fb87603
.DWinfo342:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08a5
//	DW_AT_decl_column:
	.byte 0x15
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6472
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7fb07603
.DWinfo343:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08a8
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x737361705f6d756e
	.2byte 0x7365
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x44
.DWinfo344:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08a8
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x706f7473
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.4byte 0x7fb47603
.DWinfo345:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08b1
//	DW_AT_decl_column:
	.byte 0x14
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa87603
.DWinfo346:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08b1
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00637273
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_location:
	.4byte 0x7fa07603
.DWinfo347:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x08b2
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6e656c71
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
.DWinfo348:
//	DW_TAG_lexical_block:
	.byte 0x05
//	DW_AT_decl_line:
	.2byte 0x090a
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.8byte ..LN1749
//	DW_AT_high_pc:
	.8byte ..LN1835
.DWinfo349:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x091b
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6166
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f807603
.DWinfo350:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x091c
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6266
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f847603
.DWinfo351:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x091d
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6366
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f887603
.DWinfo352:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x091e
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6466
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.4byte 0x7f8c7603
	.byte 0x00
	.byte 0x00
.DWinfo16:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x0625
//	DW_AT_decl_column:
	.byte 0x06
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x000022ea
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x747372745363754e
	.4byte 0x61685772
	.2byte 0x6b63
	.byte 0x00
	.8byte 0x747372745363754e
	.4byte 0x61685772
	.2byte 0x6b63
	.byte 0x00
//	DW_AT_low_pc:
	.8byte NucStrstrWhack
//	DW_AT_high_pc:
	.8byte .LNNucStrstrWhack
//	DW_AT_external:
	.byte 0x01
.DWinfo353:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0625
//	DW_AT_decl_column:
	.byte 0x22
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x0000279a
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x78
	.byte 0x00
.DWinfo15:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x0601
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x747372745363754e
	.4byte 0x6b614d72
	.2byte 0x0065
	.8byte 0x747372745363754e
	.4byte 0x6b614d72
	.2byte 0x0065
//	DW_AT_low_pc:
	.8byte NucStrstrMake
//	DW_AT_high_pc:
	.8byte .LNNucStrstrMake
//	DW_AT_external:
	.byte 0x01
.DWinfo354:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0601
//	DW_AT_decl_column:
	.byte 0x21
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002795
//	DW_AT_name:
	.4byte 0x0073736e
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo355:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0601
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_name:
	.8byte 0x6e6f697469736f70
	.2byte 0x6c61
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
.DWinfo356:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0602
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_name:
	.4byte 0x72657571
	.2byte 0x0079
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo357:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0602
//	DW_AT_decl_column:
	.byte 0x25
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo358:
//	DW_TAG_lexical_block:
	.byte 0x05
//	DW_AT_decl_line:
	.2byte 0x0607
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.8byte ..LN1497
//	DW_AT_high_pc:
	.8byte ..LN1545
.DWinfo359:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0608
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x74617473
	.2byte 0x7375
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x48
.DWinfo360:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0609
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_location:
	.4byte 0x7fb07603
	.byte 0x00
	.byte 0x00
.DWinfo14:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x05cb
//	DW_AT_decl_column:
	.byte 0x06
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x000022ea
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x747372745363754e
	.4byte 0x696e4972
	.2byte 0x0074
	.8byte 0x747372745363754e
	.4byte 0x696e4972
	.2byte 0x0074
//	DW_AT_low_pc:
	.8byte NucStrstrInit
//	DW_AT_high_pc:
	.8byte .LNNucStrstrInit
//	DW_AT_external:
	.byte 0x00
.DWinfo361:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x05cd
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x6863
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x5c
.DWinfo362:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x05ce
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0069
//	DW_AT_type:
	.4byte 0x000023c1
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo363:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x05cf
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo364:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x05cf
//	DW_AT_decl_column:
	.byte 0x15
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x40
.DWinfo365:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x05d0
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x00616e346962636e
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x48
.DWinfo366:
//	DW_TAG_lexical_block:
	.byte 0x05
//	DW_AT_decl_line:
	.2byte 0x05e7
//	DW_AT_decl_column:
	.byte 0x1c
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.8byte ..LN1463
//	DW_AT_high_pc:
	.8byte ..LN1481
.DWinfo367:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x05e7
//	DW_AT_decl_column:
	.byte 0x1c
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00765f5f
//	DW_AT_type:
	.4byte 0x000022d8
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x74
.DWinfo368:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x05e7
//	DW_AT_decl_column:
	.byte 0x1c
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x00785f5f
//	DW_AT_type:
	.4byte 0x000022d8
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
	.byte 0x00
	.byte 0x00
.DWinfo13:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x057c
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x727078655f73736e
	.byte 0x00
	.8byte 0x727078655f73736e
	.byte 0x00
//	DW_AT_low_pc:
	.8byte nss_expr
//	DW_AT_high_pc:
	.8byte .LNnss_expr
//	DW_AT_external:
	.byte 0x00
.DWinfo369:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x057c
//	DW_AT_decl_column:
	.byte 0x24
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x40
.DWinfo370:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x057c
//	DW_AT_decl_column:
	.byte 0x33
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x48
.DWinfo371:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x057d
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002619
//	DW_AT_name:
	.4byte 0x72707865
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo372:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x057d
//	DW_AT_decl_column:
	.byte 0x1d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002776
//	DW_AT_name:
	.4byte 0x74617473
	.2byte 0x7375
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo373:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x057d
//	DW_AT_decl_column:
	.byte 0x29
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_name:
	.8byte 0x6e6f697469736f70
	.2byte 0x6c61
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo374:
//	DW_TAG_lexical_block:
	.byte 0x05
//	DW_AT_decl_line:
	.2byte 0x0589
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.8byte ..LN1277
//	DW_AT_high_pc:
	.8byte ..LN1371
.DWinfo375:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x058a
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x706f
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000024f8
//	DW_AT_location:
	.4byte 0x7fa07603
.DWinfo376:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x058b
//	DW_AT_decl_column:
	.byte 0x1d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0065
//	DW_AT_type:
	.4byte 0x0000261e
//	DW_AT_location:
	.4byte 0x7f907603
	.byte 0x00
	.byte 0x00
.DWinfo12:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x054f
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x72616e755f73736e
	.4byte 0x78655f79
	.2byte 0x7270
	.byte 0x00
	.8byte 0x72616e755f73736e
	.4byte 0x78655f79
	.2byte 0x7270
	.byte 0x00
//	DW_AT_low_pc:
	.8byte nss_unary_expr
//	DW_AT_high_pc:
	.8byte .LNnss_unary_expr
//	DW_AT_external:
	.byte 0x00
.DWinfo377:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x054f
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x48
.DWinfo378:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x054f
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo379:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0550
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002619
//	DW_AT_name:
	.4byte 0x72707865
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo380:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0550
//	DW_AT_decl_column:
	.byte 0x1d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002776
//	DW_AT_name:
	.4byte 0x74617473
	.2byte 0x7375
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
.DWinfo381:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0550
//	DW_AT_decl_column:
	.byte 0x29
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_name:
	.8byte 0x6e6f697469736f70
	.2byte 0x6c61
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo382:
//	DW_TAG_lexical_block:
	.byte 0x05
//	DW_AT_decl_line:
	.2byte 0x0562
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.8byte ..LN1187
//	DW_AT_high_pc:
	.8byte ..LN1229
.DWinfo383:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0563
//	DW_AT_decl_column:
	.byte 0x15
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0065
//	DW_AT_type:
	.4byte 0x0000261e
//	DW_AT_location:
	.4byte 0x7fa07603
	.byte 0x00
	.byte 0x00
.DWinfo11:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x0509
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x6d6972705f73736e
	.8byte 0x727078655f797261
	.byte 0x00
	.8byte 0x6d6972705f73736e
	.8byte 0x727078655f797261
	.byte 0x00
//	DW_AT_low_pc:
	.8byte nss_primary_expr
//	DW_AT_high_pc:
	.8byte .LNnss_primary_expr
//	DW_AT_external:
	.byte 0x00
.DWinfo384:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0509
//	DW_AT_decl_column:
	.byte 0x2c
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo385:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0509
//	DW_AT_decl_column:
	.byte 0x3b
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo386:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x050a
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002619
//	DW_AT_name:
	.4byte 0x72707865
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
.DWinfo387:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x050a
//	DW_AT_decl_column:
	.byte 0x1d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002776
//	DW_AT_name:
	.4byte 0x74617473
	.2byte 0x7375
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
.DWinfo388:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x050a
//	DW_AT_decl_column:
	.byte 0x29
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_name:
	.8byte 0x6e6f697469736f70
	.2byte 0x6c61
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x78
.DWinfo389:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x050c
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0065
//	DW_AT_type:
	.4byte 0x0000261e
//	DW_AT_location:
	.4byte 0x7fb87603
	.byte 0x00
.DWinfo10:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x04d0
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x747361665f73736e
	.4byte 0x78655f61
	.2byte 0x7270
	.byte 0x00
	.8byte 0x747361665f73736e
	.4byte 0x78655f61
	.2byte 0x7270
	.byte 0x00
//	DW_AT_low_pc:
	.8byte nss_fasta_expr
//	DW_AT_high_pc:
	.8byte .LNnss_fasta_expr
//	DW_AT_external:
	.byte 0x00
.DWinfo390:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x04d0
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo391:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x04d0
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo392:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x04d1
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002619
//	DW_AT_name:
	.4byte 0x72707865
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
.DWinfo393:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x04d1
//	DW_AT_decl_column:
	.byte 0x1d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002776
//	DW_AT_name:
	.4byte 0x74617473
	.2byte 0x7375
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
.DWinfo394:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x04d1
//	DW_AT_decl_column:
	.byte 0x29
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_name:
	.8byte 0x6e6f697469736f70
	.2byte 0x6c61
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x78
	.byte 0x00
.DWinfo9:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x04a6
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x545341465f73736e
	.4byte 0x78655f41
	.2byte 0x7270
	.byte 0x00
	.8byte 0x545341465f73736e
	.4byte 0x78655f41
	.2byte 0x7270
	.byte 0x00
//	DW_AT_low_pc:
	.8byte nss_FASTA_expr
//	DW_AT_high_pc:
	.8byte .LNnss_FASTA_expr
//	DW_AT_external:
	.byte 0x00
.DWinfo395:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x04a6
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo396:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x04a6
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo397:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x04a7
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002619
//	DW_AT_name:
	.4byte 0x72707865
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
.DWinfo398:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x04a7
//	DW_AT_decl_column:
	.byte 0x1d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002776
//	DW_AT_name:
	.4byte 0x74617473
	.2byte 0x7375
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
.DWinfo399:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x04a7
//	DW_AT_decl_column:
	.byte 0x29
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_name:
	.8byte 0x6e6f697469736f70
	.2byte 0x6c61
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x78
.DWinfo400:
//	DW_TAG_lexical_block:
	.byte 0x05
//	DW_AT_decl_line:
	.2byte 0x04ac
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.8byte ..LN817
//	DW_AT_high_pc:
	.8byte ..LN887
.DWinfo401:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x04ad
//	DW_AT_decl_column:
	.byte 0x15
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x72617473
	.2byte 0x0074
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_location:
	.4byte 0x7fb87603
.DWinfo402:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x04af
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x65707974
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000024f8
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x48
.DWinfo403:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x04b0
//	DW_AT_decl_column:
	.byte 0x17
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x0070616d
//	DW_AT_type:
	.4byte 0x0000277b
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x40
	.byte 0x00
	.byte 0x00
.DWinfo8:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x049c
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x00626f735f73736e
	.8byte 0x00626f735f73736e
//	DW_AT_low_pc:
	.8byte nss_sob
//	DW_AT_high_pc:
	.8byte .LNnss_sob
//	DW_AT_external:
	.byte 0x00
.DWinfo404:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x049c
//	DW_AT_decl_column:
	.byte 0x23
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo405:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x049c
//	DW_AT_decl_column:
	.byte 0x32
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x78
	.byte 0x00
.DWinfo7:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x03f7
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x614672745363754e
	.8byte 0x4d72707845617473
	.4byte 0x34656b61
	.byte 0x00
	.8byte 0x614672745363754e
	.8byte 0x4d72707845617473
	.4byte 0x34656b61
	.byte 0x00
//	DW_AT_low_pc:
	.8byte NucStrFastaExprMake4
//	DW_AT_high_pc:
	.8byte .LNNucStrFastaExprMake4
//	DW_AT_external:
	.byte 0x00
.DWinfo406:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x03f7
//	DW_AT_decl_column:
	.byte 0x29
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002619
//	DW_AT_name:
	.4byte 0x72707865
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo407:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x03f7
//	DW_AT_decl_column:
	.byte 0x33
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_name:
	.8byte 0x6e6f697469736f70
	.2byte 0x6c61
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo408:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x03f8
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_name:
	.4byte 0x74736166
	.2byte 0x0061
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
.DWinfo409:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x03f8
//	DW_AT_decl_column:
	.byte 0x1f
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x0000246f
//	DW_AT_name:
	.4byte 0x657a6973
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
.DWinfo410:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x03fa
//	DW_AT_decl_column:
	.byte 0x0c
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0069
//	DW_AT_type:
	.4byte 0x0000246f
//	DW_AT_location:
	.4byte 0x7fb07603
.DWinfo411:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x03fb
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0065
//	DW_AT_type:
	.4byte 0x0000261e
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x40
.DWinfo412:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x03fc
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x006e726574746170
//	DW_AT_type:
	.4byte 0x00002574
//	DW_AT_location:
	.4byte 0x7f907603
.DWinfo413:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x03fc
//	DW_AT_decl_column:
	.byte 0x17
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6b73616d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002574
//	DW_AT_location:
	.4byte 0x7fa07603
.DWinfo414:
//	DW_TAG_lexical_block:
	.byte 0x05
//	DW_AT_decl_line:
	.2byte 0x040c
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.8byte ..LN493
//	DW_AT_high_pc:
	.8byte ..LN603
.DWinfo415:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x040d
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x65736162
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000022c6
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
	.byte 0x00
	.byte 0x00
.DWinfo6:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x0344
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x614672745363754e
	.8byte 0x4d72707845617473
	.4byte 0x32656b61
	.byte 0x00
	.8byte 0x614672745363754e
	.8byte 0x4d72707845617473
	.4byte 0x32656b61
	.byte 0x00
//	DW_AT_low_pc:
	.8byte NucStrFastaExprMake2
//	DW_AT_high_pc:
	.8byte .LNNucStrFastaExprMake2
//	DW_AT_external:
	.byte 0x00
.DWinfo416:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0344
//	DW_AT_decl_column:
	.byte 0x29
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002619
//	DW_AT_name:
	.4byte 0x72707865
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x50
.DWinfo417:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0344
//	DW_AT_decl_column:
	.byte 0x33
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002509
//	DW_AT_name:
	.8byte 0x6e6f697469736f70
	.2byte 0x6c61
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo418:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0345
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002764
//	DW_AT_name:
	.4byte 0x74736166
	.2byte 0x0061
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x58
.DWinfo419:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0345
//	DW_AT_decl_column:
	.byte 0x1f
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x0000246f
//	DW_AT_name:
	.4byte 0x657a6973
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
.DWinfo420:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0347
//	DW_AT_decl_column:
	.byte 0x0c
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0069
//	DW_AT_type:
	.4byte 0x0000246f
//	DW_AT_location:
	.4byte 0x7fb07603
.DWinfo421:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0348
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0065
//	DW_AT_type:
	.4byte 0x0000261e
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x40
.DWinfo422:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0349
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x006e726574746170
//	DW_AT_type:
	.4byte 0x00002574
//	DW_AT_location:
	.4byte 0x7f907603
.DWinfo423:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0349
//	DW_AT_decl_column:
	.byte 0x17
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6b73616d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002574
//	DW_AT_location:
	.4byte 0x7fa07603
.DWinfo424:
//	DW_TAG_lexical_block:
	.byte 0x05
//	DW_AT_decl_line:
	.2byte 0x0359
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.8byte ..LN109
//	DW_AT_high_pc:
	.8byte ..LN219
.DWinfo425:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x035a
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x65736162
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000025dc
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
	.byte 0x00
	.byte 0x00
.DWinfo4:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x0136
//	DW_AT_decl_column:
	.byte 0x06
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_inline:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x000022ea
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x5f383231746e6975
	.8byte 0x6f635f7061777362
	.2byte 0x7970
	.byte 0x00
	.8byte 0x5f383231746e6975
	.8byte 0x6f635f7061777362
	.2byte 0x7970
	.byte 0x00
//	DW_AT_low_pc:
	.8byte uint128_bswap_copy
//	DW_AT_high_pc:
	.8byte .LNuint128_bswap_copy
//	DW_AT_external:
	.byte 0x00
.DWinfo426:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0136
//	DW_AT_decl_column:
	.byte 0x26
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002347
//	DW_AT_name:
	.2byte 0x6f74
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo427:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0136
//	DW_AT_decl_column:
	.byte 0x3b
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x000023d1
//	DW_AT_name:
	.4byte 0x6d6f7266
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x78
	.byte 0x00
.DWinfo3:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x0125
//	DW_AT_decl_column:
	.byte 0x06
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_inline:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x000022ea
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x5f383231746e6975
	.4byte 0x61777362
	.2byte 0x0070
	.8byte 0x5f383231746e6975
	.4byte 0x61777362
	.2byte 0x0070
//	DW_AT_low_pc:
	.8byte uint128_bswap
//	DW_AT_high_pc:
	.8byte .LNuint128_bswap
//	DW_AT_external:
	.byte 0x00
.DWinfo428:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0125
//	DW_AT_decl_column:
	.byte 0x21
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002347
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
	.byte 0x00
.DWinfo429:
//	DW_TAG_base_type:
	.byte 0x07
//	DW_AT_byte_size:
	.byte 0x02
//	DW_AT_encoding:
	.byte 0x05
//	DW_AT_name:
	.4byte 0x726f6873
	.2byte 0x0074
.DWinfo1:
//	DW_TAG_subprogram:
	.byte 0x08
//	DW_AT_decl_line:
	.byte 0x25
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_inline:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x000000d5
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x6c5f3631746e6975
	.4byte 0x74696273
	.byte 0x00
	.8byte 0x6c5f3631746e6975
	.4byte 0x74696273
	.byte 0x00
//	DW_AT_low_pc:
	.8byte uint16_lsbit
//	DW_AT_high_pc:
	.8byte .LNuint16_lsbit
//	DW_AT_external:
	.byte 0x00
.DWinfo430:
//	DW_TAG_formal_parameter:
	.byte 0x09
//	DW_AT_decl_line:
	.byte 0x25
//	DW_AT_decl_column:
	.byte 0x21
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x000022c6
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x78
.DWinfo431:
//	DW_TAG_variable:
	.byte 0x0a
//	DW_AT_decl_line:
	.byte 0x27
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x006e7472
//	DW_AT_type:
	.4byte 0x000000d5
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
	.byte 0x00
.DWinfo432:
//	DW_TAG_typedef:
	.byte 0x02
//	DW_AT_decl_line:
	.byte 0x32
//	DW_AT_decl_column:
	.byte 0x1c
//	DW_AT_decl_file:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x745f3631746e6975
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000022d8
.DWinfo433:
//	DW_TAG_base_type:
	.byte 0x07
//	DW_AT_byte_size:
	.byte 0x02
//	DW_AT_encoding:
	.byte 0x07
//	DW_AT_name:
	.8byte 0x64656e6769736e75
	.4byte 0x6f687320
	.2byte 0x7472
	.byte 0x00
.DWinfo434:
//	DW_TAG_base_type:
	.byte 0x07
//	DW_AT_byte_size:
	.byte 0x00
//	DW_AT_encoding:
	.byte 0x05
//	DW_AT_name:
	.4byte 0x64696f76
	.byte 0x00
.DWinfo2:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x0105
//	DW_AT_decl_column:
	.byte 0x06
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_inline:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x000022ea
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x5f383231746e6975
	.4byte 0x00726873
	.8byte 0x5f383231746e6975
	.4byte 0x00726873
//	DW_AT_low_pc:
	.8byte uint128_shr
//	DW_AT_high_pc:
	.8byte .LNuint128_shr
//	DW_AT_external:
	.byte 0x00
.DWinfo435:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0105
//	DW_AT_decl_column:
	.byte 0x1f
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00002347
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo436:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0105
//	DW_AT_decl_column:
	.byte 0x2e
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x000023af
//	DW_AT_name:
	.2byte 0x0069
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x78
	.byte 0x00
.DWinfo437:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x0000234c
.DWinfo438:
//	DW_TAG_typedef:
	.byte 0x02
//	DW_AT_decl_line:
	.byte 0x63
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x5f383231746e6975
	.2byte 0x0074
//	DW_AT_type:
	.4byte 0x0000235f
.DWinfo439:
//	DW_TAG_structure_type:
	.byte 0x0c
//	DW_AT_decl_line:
	.byte 0x64
//	DW_AT_decl_column:
	.byte 0x08
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_byte_size:
	.byte 0x10
//	DW_AT_name:
	.8byte 0x5f383231746e6975
	.2byte 0x0074
.DWinfo440:
//	DW_TAG_member:
	.byte 0x0d
//	DW_AT_decl_line:
	.byte 0x66
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x00
//	DW_AT_name:
	.2byte 0x6f6c
	.byte 0x00
//	DW_AT_type:
	.4byte 0x0000238c
.DWinfo441:
//	DW_TAG_member:
	.byte 0x0d
//	DW_AT_decl_line:
	.byte 0x67
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x08
//	DW_AT_name:
	.2byte 0x6968
	.byte 0x00
//	DW_AT_type:
	.4byte 0x0000238c
	.byte 0x00
.DWinfo442:
//	DW_TAG_typedef:
	.byte 0x02
//	DW_AT_decl_line:
	.byte 0x38
//	DW_AT_decl_column:
	.byte 0x1b
//	DW_AT_decl_file:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x745f3436746e6975
	.byte 0x00
//	DW_AT_type:
	.4byte 0x0000239e
.DWinfo443:
//	DW_TAG_base_type:
	.byte 0x07
//	DW_AT_byte_size:
	.byte 0x08
//	DW_AT_encoding:
	.byte 0x07
//	DW_AT_name:
	.8byte 0x64656e6769736e75
	.4byte 0x6e6f6c20
	.2byte 0x0067
.DWinfo444:
//	DW_TAG_typedef:
	.byte 0x02
//	DW_AT_decl_line:
	.byte 0x34
//	DW_AT_decl_column:
	.byte 0x17
//	DW_AT_decl_file:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x745f3233746e6975
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023c1
.DWinfo445:
//	DW_TAG_base_type:
	.byte 0x07
//	DW_AT_byte_size:
	.byte 0x04
//	DW_AT_encoding:
	.byte 0x07
//	DW_AT_name:
	.8byte 0x64656e6769736e75
	.4byte 0x746e6920
	.byte 0x00
.DWinfo446:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x000023d6
.DWinfo447:
//	DW_TAG_const_type:
	.byte 0x0e
//	DW_AT_type:
	.4byte 0x0000234c
.DWinfo448:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x000022ea
.DWinfo5:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x0315
//	DW_AT_decl_column:
	.byte 0x07
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x00
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x000023db
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x614672745363754e
	.8byte 0x4172707845617473
	.4byte 0x636f6c6c
	.byte 0x00
	.8byte 0x614672745363754e
	.8byte 0x4172707845617473
	.4byte 0x636f6c6c
	.byte 0x00
//	DW_AT_low_pc:
	.8byte NucStrFastaExprAlloc
//	DW_AT_high_pc:
	.8byte .LNNucStrFastaExprAlloc
//	DW_AT_external:
	.byte 0x00
.DWinfo449:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0315
//	DW_AT_decl_column:
	.byte 0x25
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x0000246f
//	DW_AT_name:
	.2byte 0x7a73
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x68
.DWinfo450:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0317
//	DW_AT_decl_column:
	.byte 0x0b
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x6574756f
	.2byte 0x0072
//	DW_AT_type:
	.4byte 0x000023db
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo451:
//	DW_TAG_lexical_block:
	.byte 0x05
//	DW_AT_decl_line:
	.2byte 0x0319
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.8byte ..LN45
//	DW_AT_high_pc:
	.8byte ..LN59
.DWinfo452:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x031a
//	DW_AT_decl_column:
	.byte 0x1a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.2byte 0x0065
//	DW_AT_type:
	.4byte 0x0000247f
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
	.byte 0x00
	.byte 0x00
.DWinfo453:
//	DW_TAG_typedef:
	.byte 0x02
//	DW_AT_decl_line:
	.byte 0x32
//	DW_AT_decl_column:
	.byte 0x17
//	DW_AT_decl_file:
	.byte 0x04
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x657a6973
	.2byte 0x745f
	.byte 0x00
//	DW_AT_type:
	.4byte 0x0000239e
.DWinfo454:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x00002484
.DWinfo455:
//	DW_TAG_typedef:
	.byte 0x0f
//	DW_AT_decl_line:
	.2byte 0x02fa
//	DW_AT_decl_column:
	.byte 0x20
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x614672745363754e
	.8byte 0x0072707845617473
//	DW_AT_type:
	.4byte 0x0000249e
.DWinfo456:
//	DW_TAG_structure_type:
	.byte 0x10
//	DW_AT_decl_line:
	.2byte 0x02fb
//	DW_AT_decl_column:
	.byte 0x08
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_byte_size:
	.byte 0x90
//	DW_AT_name:
	.8byte 0x614672745363754e
	.8byte 0x0072707845617473
.DWinfo457:
//	DW_TAG_member:
	.byte 0x11
//	DW_AT_decl_line:
	.2byte 0x02fd
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x00
//	DW_AT_name:
	.4byte 0x65707974
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000024f8
.DWinfo458:
//	DW_TAG_member:
	.byte 0x11
//	DW_AT_decl_line:
	.2byte 0x02fe
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x04
//	DW_AT_name:
	.4byte 0x657a6973
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000023af
.DWinfo459:
//	DW_TAG_member:
	.byte 0x11
//	DW_AT_decl_line:
	.2byte 0x0307
//	DW_AT_decl_column:
	.byte 0x07
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x08
//	DW_AT_name:
	.2byte 0x0075
//	DW_AT_type:
	.4byte 0x00002510
.DWinfo460:
//	DW_TAG_member:
	.byte 0x11
//	DW_AT_decl_line:
	.2byte 0x030d
//	DW_AT_decl_column:
	.byte 0x07
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x10
//	DW_AT_name:
	.4byte 0x72657571
	.2byte 0x0079
//	DW_AT_type:
	.4byte 0x0000253e
	.byte 0x00
.DWinfo461:
//	DW_TAG_typedef:
	.byte 0x02
//	DW_AT_decl_line:
	.byte 0x27
//	DW_AT_decl_column:
	.byte 0x0f
//	DW_AT_decl_file:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x00745f3233746e69
//	DW_AT_type:
	.4byte 0x00002509
.DWinfo462:
//	DW_TAG_base_type:
	.byte 0x07
//	DW_AT_byte_size:
	.byte 0x04
//	DW_AT_encoding:
	.byte 0x05
//	DW_AT_name:
	.4byte 0x00746e69
.DWinfo463:
//	DW_TAG_union_type:
	.byte 0x12
//	DW_AT_decl_line:
	.2byte 0x0301
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_byte_size:
	.byte 0x08
.DWinfo464:
//	DW_TAG_member:
	.byte 0x13
//	DW_AT_decl_line:
	.2byte 0x0305
//	DW_AT_decl_column:
	.byte 0x0f
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x00
//	DW_AT_name:
	.4byte 0x6574756f
	.2byte 0x0072
//	DW_AT_type:
	.4byte 0x000023db
.DWinfo465:
//	DW_TAG_member:
	.byte 0x13
//	DW_AT_decl_line:
	.2byte 0x0306
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x00
//	DW_AT_name:
	.4byte 0x67696c61
	.2byte 0x006e
//	DW_AT_type:
	.4byte 0x0000238c
	.byte 0x00
.DWinfo466:
//	DW_TAG_array_type:
	.byte 0x14
//	DW_AT_type:
	.4byte 0x00002547
//	DW_AT_byte_size:
	.byte 0x80
.DWinfo467:
//	DW_TAG_subrange_type:
	.byte 0x15
//	DW_AT_upper_bound:
	.byte 0x03
	.byte 0x00
.DWinfo468:
//	DW_TAG_structure_type:
	.byte 0x16
//	DW_AT_decl_line:
	.2byte 0x030a
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_byte_size:
	.byte 0x20
.DWinfo469:
//	DW_TAG_member:
	.byte 0x11
//	DW_AT_decl_line:
	.2byte 0x030b
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x00
//	DW_AT_name:
	.8byte 0x006e726574746170
//	DW_AT_type:
	.4byte 0x00002574
.DWinfo470:
//	DW_TAG_member:
	.byte 0x11
//	DW_AT_decl_line:
	.2byte 0x030c
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x10
//	DW_AT_name:
	.4byte 0x6b73616d
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002574
	.byte 0x00
.DWinfo471:
//	DW_TAG_typedef:
	.byte 0x02
//	DW_AT_decl_line:
	.byte 0x48
//	DW_AT_decl_column:
	.byte 0x03
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x745f74617063756e
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002586
.DWinfo472:
//	DW_TAG_union_type:
	.byte 0x17
//	DW_AT_decl_line:
	.byte 0x42
//	DW_AT_decl_column:
	.byte 0x01
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_byte_size:
	.byte 0x10
.DWinfo473:
//	DW_TAG_member:
	.byte 0x18
//	DW_AT_decl_line:
	.byte 0x43
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x00
//	DW_AT_name:
	.2byte 0x0062
//	DW_AT_type:
	.4byte 0x000025d3
.DWinfo474:
//	DW_TAG_member:
	.byte 0x18
//	DW_AT_decl_line:
	.byte 0x44
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x00
//	DW_AT_name:
	.2byte 0x0077
//	DW_AT_type:
	.4byte 0x000025fe
.DWinfo475:
//	DW_TAG_member:
	.byte 0x18
//	DW_AT_decl_line:
	.byte 0x45
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x00
//	DW_AT_name:
	.2byte 0x0069
//	DW_AT_type:
	.4byte 0x00002607
.DWinfo476:
//	DW_TAG_member:
	.byte 0x18
//	DW_AT_decl_line:
	.byte 0x46
//	DW_AT_decl_column:
	.byte 0x0e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x00
//	DW_AT_name:
	.2byte 0x006c
//	DW_AT_type:
	.4byte 0x00002610
.DWinfo477:
//	DW_TAG_member:
	.byte 0x18
//	DW_AT_decl_line:
	.byte 0x47
//	DW_AT_decl_column:
	.byte 0x0f
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x00
//	DW_AT_name:
	.2byte 0x0073
//	DW_AT_type:
	.4byte 0x0000234c
	.byte 0x00
.DWinfo478:
//	DW_TAG_array_type:
	.byte 0x14
//	DW_AT_type:
	.4byte 0x000025dc
//	DW_AT_byte_size:
	.byte 0x10
.DWinfo479:
//	DW_TAG_subrange_type:
	.byte 0x15
//	DW_AT_upper_bound:
	.byte 0x0f
	.byte 0x00
.DWinfo480:
//	DW_TAG_typedef:
	.byte 0x02
//	DW_AT_decl_line:
	.byte 0x31
//	DW_AT_decl_column:
	.byte 0x18
//	DW_AT_decl_file:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x00745f38746e6975
//	DW_AT_type:
	.4byte 0x000025ed
.DWinfo481:
//	DW_TAG_base_type:
	.byte 0x07
//	DW_AT_byte_size:
	.byte 0x01
//	DW_AT_encoding:
	.byte 0x08
//	DW_AT_name:
	.8byte 0x64656e6769736e75
	.4byte 0x61686320
	.2byte 0x0072
.DWinfo482:
//	DW_TAG_array_type:
	.byte 0x14
//	DW_AT_type:
	.4byte 0x000022c6
//	DW_AT_byte_size:
	.byte 0x10
.DWinfo483:
//	DW_TAG_subrange_type:
	.byte 0x15
//	DW_AT_upper_bound:
	.byte 0x07
	.byte 0x00
.DWinfo484:
//	DW_TAG_array_type:
	.byte 0x14
//	DW_AT_type:
	.4byte 0x000023af
//	DW_AT_byte_size:
	.byte 0x10
.DWinfo485:
//	DW_TAG_subrange_type:
	.byte 0x15
//	DW_AT_upper_bound:
	.byte 0x03
	.byte 0x00
.DWinfo486:
//	DW_TAG_array_type:
	.byte 0x14
//	DW_AT_type:
	.4byte 0x0000238c
//	DW_AT_byte_size:
	.byte 0x10
.DWinfo487:
//	DW_TAG_subrange_type:
	.byte 0x15
//	DW_AT_upper_bound:
	.byte 0x01
	.byte 0x00
.DWinfo488:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x0000261e
.DWinfo489:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x00002623
.DWinfo490:
//	DW_TAG_typedef:
	.byte 0x0f
//	DW_AT_decl_line:
	.2byte 0x02df
//	DW_AT_decl_column:
	.byte 0x13
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x784572745363754e
	.2byte 0x7270
	.byte 0x00
//	DW_AT_type:
	.4byte 0x00002638
.DWinfo491:
//	DW_TAG_typedef:
	.byte 0x02
//	DW_AT_decl_line:
	.byte 0x44
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x05
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x747372745363754e
	.2byte 0x0072
//	DW_AT_type:
	.4byte 0x0000264b
.DWinfo492:
//	DW_TAG_union_type:
	.byte 0x19
//	DW_AT_decl_line:
	.2byte 0x0339
//	DW_AT_decl_column:
	.byte 0x08
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_byte_size:
	.byte 0x90
//	DW_AT_name:
	.8byte 0x747372745363754e
	.2byte 0x0072
.DWinfo493:
//	DW_TAG_member:
	.byte 0x13
//	DW_AT_decl_line:
	.2byte 0x033b
//	DW_AT_decl_column:
	.byte 0x15
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x00
//	DW_AT_name:
	.4byte 0x74736166
	.2byte 0x0061
//	DW_AT_type:
	.4byte 0x00002484
.DWinfo494:
//	DW_TAG_member:
	.byte 0x13
//	DW_AT_decl_line:
	.2byte 0x033c
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x00
//	DW_AT_name:
	.8byte 0x006e61656c6f6f62
//	DW_AT_type:
	.4byte 0x00002696
.DWinfo495:
//	DW_TAG_member:
	.byte 0x13
//	DW_AT_decl_line:
	.2byte 0x033d
//	DW_AT_decl_column:
	.byte 0x13
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x00
//	DW_AT_name:
	.4byte 0x00627573
//	DW_AT_type:
	.4byte 0x00002705
	.byte 0x00
.DWinfo496:
//	DW_TAG_typedef:
	.byte 0x0f
//	DW_AT_decl_line:
	.2byte 0x0328
//	DW_AT_decl_column:
	.byte 0x1d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x704f72745363754e
	.4byte 0x72707845
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000026ad
.DWinfo497:
//	DW_TAG_structure_type:
	.byte 0x10
//	DW_AT_decl_line:
	.2byte 0x0329
//	DW_AT_decl_column:
	.byte 0x08
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_byte_size:
	.byte 0x18
//	DW_AT_name:
	.8byte 0x704f72745363754e
	.4byte 0x72707845
	.byte 0x00
.DWinfo498:
//	DW_TAG_member:
	.byte 0x11
//	DW_AT_decl_line:
	.2byte 0x032b
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x00
//	DW_AT_name:
	.4byte 0x65707974
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000024f8
.DWinfo499:
//	DW_TAG_member:
	.byte 0x11
//	DW_AT_decl_line:
	.2byte 0x032c
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x04
//	DW_AT_name:
	.2byte 0x706f
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000024f8
.DWinfo500:
//	DW_TAG_member:
	.byte 0x11
//	DW_AT_decl_line:
	.2byte 0x032d
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x08
//	DW_AT_name:
	.4byte 0x7466656c
	.byte 0x00
//	DW_AT_type:
	.4byte 0x0000261e
.DWinfo501:
//	DW_TAG_member:
	.byte 0x11
//	DW_AT_decl_line:
	.2byte 0x032e
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x10
//	DW_AT_name:
	.4byte 0x68676972
	.2byte 0x0074
//	DW_AT_type:
	.4byte 0x0000261e
	.byte 0x00
.DWinfo502:
//	DW_TAG_typedef:
	.byte 0x0f
//	DW_AT_decl_line:
	.2byte 0x0331
//	DW_AT_decl_column:
	.byte 0x1e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x755372745363754e
	.4byte 0x70784562
	.2byte 0x0072
//	DW_AT_type:
	.4byte 0x0000271d
.DWinfo503:
//	DW_TAG_structure_type:
	.byte 0x10
//	DW_AT_decl_line:
	.2byte 0x0332
//	DW_AT_decl_column:
	.byte 0x08
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_byte_size:
	.byte 0x10
//	DW_AT_name:
	.8byte 0x755372745363754e
	.4byte 0x70784562
	.2byte 0x0072
.DWinfo504:
//	DW_TAG_member:
	.byte 0x11
//	DW_AT_decl_line:
	.2byte 0x0334
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x00
//	DW_AT_name:
	.4byte 0x65707974
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000024f8
.DWinfo505:
//	DW_TAG_member:
	.byte 0x11
//	DW_AT_decl_line:
	.2byte 0x0335
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x04
//	DW_AT_name:
	.2byte 0x706f
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000024f8
.DWinfo506:
//	DW_TAG_member:
	.byte 0x11
//	DW_AT_decl_line:
	.2byte 0x0336
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x08
//	DW_AT_name:
	.4byte 0x72707865
	.byte 0x00
//	DW_AT_type:
	.4byte 0x0000261e
	.byte 0x00
.DWinfo507:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x00002769
.DWinfo508:
//	DW_TAG_const_type:
	.byte 0x0e
//	DW_AT_type:
	.4byte 0x0000276e
.DWinfo509:
//	DW_TAG_base_type:
	.byte 0x07
//	DW_AT_byte_size:
	.byte 0x01
//	DW_AT_encoding:
	.byte 0x06
//	DW_AT_name:
	.4byte 0x72616863
	.byte 0x00
.DWinfo510:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x00002509
.DWinfo511:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x00002780
.DWinfo512:
//	DW_TAG_const_type:
	.byte 0x0e
//	DW_AT_type:
	.4byte 0x00002785
.DWinfo513:
//	DW_TAG_typedef:
	.byte 0x02
//	DW_AT_decl_line:
	.byte 0x25
//	DW_AT_decl_column:
	.byte 0x16
//	DW_AT_decl_file:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x38746e69
	.2byte 0x745f
	.byte 0x00
//	DW_AT_type:
	.4byte 0x0000276e
.DWinfo514:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x0000279a
.DWinfo515:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x00002638
.DWinfo516:
//	DW_TAG_typedef:
	.byte 0x02
//	DW_AT_decl_line:
	.byte 0x2b
//	DW_AT_decl_column:
	.byte 0x36
//	DW_AT_decl_file:
	.byte 0x06
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x00693832316d5f5f
//	DW_AT_type:
	.4byte 0x000027b0
.DWinfo517:
//	DW_TAG_array_type:
	.byte 0x1a
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x00693832316d5f5f
//	DW_AT_type:
	.4byte 0x000027cc
//	DW_AT_byte_size:
	.byte 0x10
.DWinfo518:
//	DW_TAG_subrange_type:
	.byte 0x1b
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x00693832316d5f5f
//	DW_AT_upper_bound:
	.byte 0x01
//	DW_AT_lower_bound:
	.byte 0x00
	.byte 0x00
.DWinfo519:
//	DW_TAG_base_type:
	.byte 0x07
//	DW_AT_byte_size:
	.byte 0x08
//	DW_AT_encoding:
	.byte 0x07
//	DW_AT_name:
	.8byte 0x64656e6769736e75
	.8byte 0x6f6c20676e6f6c20
	.2byte 0x676e
	.byte 0x00
.DWinfo17:
//	DW_TAG_subprogram:
	.byte 0x03
//	DW_AT_decl_line:
	.2byte 0x0850
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_inline:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x0000279f
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x75625f656d697270
	.8byte 0x616e325f72656666
	.byte 0x00
	.8byte 0x75625f656d697270
	.8byte 0x616e325f72656666
	.byte 0x00
//	DW_AT_low_pc:
	.8byte prime_buffer_2na
//	DW_AT_high_pc:
	.8byte .LNprime_buffer_2na
//	DW_AT_external:
	.byte 0x00
.DWinfo520:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0850
//	DW_AT_decl_column:
	.byte 0x2b
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_name:
	.4byte 0x00637273
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x70
.DWinfo521:
//	DW_TAG_formal_parameter:
	.byte 0x04
//	DW_AT_decl_line:
	.2byte 0x0850
//	DW_AT_decl_column:
	.byte 0x3f
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00002859
//	DW_AT_name:
	.4byte 0x6f6e6769
	.2byte 0x6572
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x78
.DWinfo522:
//	DW_TAG_variable:
	.byte 0x06
//	DW_AT_decl_line:
	.2byte 0x0852
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x66667562
	.2byte 0x7265
	.byte 0x00
//	DW_AT_type:
	.4byte 0x0000279f
//	DW_AT_location:
	.2byte 0x7602
	.byte 0x60
	.byte 0x00
.DWinfo523:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x0000285e
.DWinfo524:
//	DW_TAG_const_type:
	.byte 0x0e
//	DW_AT_type:
	.4byte 0x000025dc
.DWinfo525:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x00002868
.DWinfo526:
//	DW_TAG_const_type:
	.byte 0x0e
//	DW_AT_type:
	.4byte 0x00002484
.DWinfo527:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x00002872
.DWinfo528:
//	DW_TAG_const_type:
	.byte 0x0e
//	DW_AT_type:
	.4byte 0x000022ea
.DWinfo529:
//	DW_TAG_typedef:
	.byte 0x02
//	DW_AT_decl_line:
	.byte 0x40
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x745f67657263756e
	.byte 0x00
//	DW_AT_type:
	.4byte 0x0000279f
.DWinfo530:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x0000288e
.DWinfo531:
//	DW_TAG_const_type:
	.byte 0x0e
//	DW_AT_type:
	.4byte 0x00002638
.DWinfo532:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x000023c1
.DWinfo533:
//	DW_TAG_variable:
	.byte 0x0a
//	DW_AT_decl_line:
	.byte 0x5a
//	DW_AT_decl_column:
	.byte 0x0f
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x6e325f6174736166
	.4byte 0x616d5f61
	.2byte 0x0070
//	DW_AT_type:
	.4byte 0x000028b9
//	DW_AT_location:
	.2byte 0x0309
	.8byte fasta_2na_map.0
.DWinfo534:
//	DW_TAG_array_type:
	.byte 0x14
//	DW_AT_type:
	.4byte 0x00002785
//	DW_AT_byte_size:
	.byte 0x80
.DWinfo535:
//	DW_TAG_subrange_type:
	.byte 0x15
//	DW_AT_upper_bound:
	.byte 0x7f
	.byte 0x00
.DWinfo536:
//	DW_TAG_variable:
	.byte 0x0a
//	DW_AT_decl_line:
	.byte 0x5b
//	DW_AT_decl_column:
	.byte 0x0f
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x6e345f6174736166
	.4byte 0x616d5f61
	.2byte 0x0070
//	DW_AT_type:
	.4byte 0x000028e3
//	DW_AT_location:
	.2byte 0x0309
	.8byte fasta_4na_map.0
.DWinfo537:
//	DW_TAG_array_type:
	.byte 0x14
//	DW_AT_type:
	.4byte 0x00002785
//	DW_AT_byte_size:
	.byte 0x80
.DWinfo538:
//	DW_TAG_subrange_type:
	.byte 0x15
//	DW_AT_upper_bound:
	.byte 0x7f
	.byte 0x00
.DWinfo539:
//	DW_TAG_variable:
	.byte 0x0a
//	DW_AT_decl_line:
	.byte 0x5c
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x325f646e61707865
	.2byte 0x616e
	.byte 0x00
//	DW_AT_type:
	.4byte 0x0000290a
//	DW_AT_location:
	.2byte 0x0309
	.8byte expand_2na.0
.DWinfo540:
//	DW_TAG_array_type:
	.byte 0x1c
//	DW_AT_type:
	.4byte 0x000022c6
//	DW_AT_byte_size:
	.2byte 0x0200
.DWinfo541:
//	DW_TAG_subrange_type:
	.byte 0x1d
//	DW_AT_upper_bound:
	.2byte 0x00ff
	.byte 0x00
	.byte 0x00
	.byte 0x00
	.byte 0x00
	.byte 0x00
// -- Begin DWARF2 SEGMENT .debug_line
	.section .debug_line
.debug_line_seg:
	.align 1
// -- Begin DWARF2 SEGMENT .debug_abbrev
	.section .debug_abbrev
.debug_abbrev_seg:
	.align 1
	.byte 0x01
	.byte 0x11
	.byte 0x01
	.byte 0x1b
	.byte 0x08
	.byte 0x13
	.byte 0x0b
	.byte 0x03
	.byte 0x08
	.byte 0x25
	.byte 0x08
	.byte 0x10
	.byte 0x06
	.2byte 0x0000
	.byte 0x02
	.byte 0x16
	.byte 0x00
	.byte 0x3b
	.byte 0x0b
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x32
	.byte 0x0b
	.byte 0x03
	.byte 0x08
	.byte 0x49
	.byte 0x13
	.2byte 0x0000
	.byte 0x03
	.byte 0x2e
	.byte 0x01
	.byte 0x3b
	.byte 0x05
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x20
	.byte 0x0b
	.byte 0x32
	.byte 0x0b
	.byte 0x49
	.byte 0x13
	.byte 0x27
	.byte 0x0c
	.byte 0x03
	.byte 0x08
	.2byte 0x4087
	.byte 0x08
	.byte 0x11
	.byte 0x01
	.byte 0x12
	.byte 0x01
	.byte 0x3f
	.byte 0x0c
	.2byte 0x0000
	.byte 0x04
	.byte 0x05
	.byte 0x00
	.byte 0x3b
	.byte 0x05
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x49
	.byte 0x13
	.byte 0x03
	.byte 0x08
	.byte 0x02
	.byte 0x0a
	.2byte 0x0000
	.byte 0x05
	.byte 0x0b
	.byte 0x01
	.byte 0x3b
	.byte 0x05
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x11
	.byte 0x01
	.byte 0x12
	.byte 0x01
	.2byte 0x0000
	.byte 0x06
	.byte 0x34
	.byte 0x00
	.byte 0x3b
	.byte 0x05
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x32
	.byte 0x0b
	.byte 0x03
	.byte 0x08
	.byte 0x49
	.byte 0x13
	.byte 0x02
	.byte 0x0a
	.2byte 0x0000
	.byte 0x07
	.byte 0x24
	.byte 0x00
	.byte 0x0b
	.byte 0x0b
	.byte 0x3e
	.byte 0x0b
	.byte 0x03
	.byte 0x08
	.2byte 0x0000
	.byte 0x08
	.byte 0x2e
	.byte 0x01
	.byte 0x3b
	.byte 0x0b
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x20
	.byte 0x0b
	.byte 0x32
	.byte 0x0b
	.byte 0x49
	.byte 0x13
	.byte 0x27
	.byte 0x0c
	.byte 0x03
	.byte 0x08
	.2byte 0x4087
	.byte 0x08
	.byte 0x11
	.byte 0x01
	.byte 0x12
	.byte 0x01
	.byte 0x3f
	.byte 0x0c
	.2byte 0x0000
	.byte 0x09
	.byte 0x05
	.byte 0x00
	.byte 0x3b
	.byte 0x0b
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x49
	.byte 0x13
	.byte 0x03
	.byte 0x08
	.byte 0x02
	.byte 0x0a
	.2byte 0x0000
	.byte 0x0a
	.byte 0x34
	.byte 0x00
	.byte 0x3b
	.byte 0x0b
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x32
	.byte 0x0b
	.byte 0x03
	.byte 0x08
	.byte 0x49
	.byte 0x13
	.byte 0x02
	.byte 0x0a
	.2byte 0x0000
	.byte 0x0b
	.byte 0x0f
	.byte 0x00
	.byte 0x49
	.byte 0x13
	.2byte 0x0000
	.byte 0x0c
	.byte 0x13
	.byte 0x01
	.byte 0x3b
	.byte 0x0b
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x32
	.byte 0x0b
	.byte 0x0b
	.byte 0x0b
	.byte 0x03
	.byte 0x08
	.2byte 0x0000
	.byte 0x0d
	.byte 0x0d
	.byte 0x00
	.byte 0x3b
	.byte 0x0b
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x38
	.byte 0x0a
	.byte 0x03
	.byte 0x08
	.byte 0x49
	.byte 0x13
	.2byte 0x0000
	.byte 0x0e
	.byte 0x26
	.byte 0x00
	.byte 0x49
	.byte 0x13
	.2byte 0x0000
	.byte 0x0f
	.byte 0x16
	.byte 0x00
	.byte 0x3b
	.byte 0x05
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x32
	.byte 0x0b
	.byte 0x03
	.byte 0x08
	.byte 0x49
	.byte 0x13
	.2byte 0x0000
	.byte 0x10
	.byte 0x13
	.byte 0x01
	.byte 0x3b
	.byte 0x05
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x32
	.byte 0x0b
	.byte 0x0b
	.byte 0x0b
	.byte 0x03
	.byte 0x08
	.2byte 0x0000
	.byte 0x11
	.byte 0x0d
	.byte 0x00
	.byte 0x3b
	.byte 0x05
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x38
	.byte 0x0a
	.byte 0x03
	.byte 0x08
	.byte 0x49
	.byte 0x13
	.2byte 0x0000
	.byte 0x12
	.byte 0x17
	.byte 0x01
	.byte 0x3b
	.byte 0x05
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x32
	.byte 0x0b
	.byte 0x0b
	.byte 0x0b
	.2byte 0x0000
	.byte 0x13
	.byte 0x0d
	.byte 0x00
	.byte 0x3b
	.byte 0x05
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x32
	.byte 0x0b
	.byte 0x38
	.byte 0x0a
	.byte 0x03
	.byte 0x08
	.byte 0x49
	.byte 0x13
	.2byte 0x0000
	.byte 0x14
	.byte 0x01
	.byte 0x01
	.byte 0x49
	.byte 0x13
	.byte 0x0b
	.byte 0x0b
	.2byte 0x0000
	.byte 0x15
	.byte 0x21
	.byte 0x00
	.byte 0x2f
	.byte 0x0b
	.2byte 0x0000
	.byte 0x16
	.byte 0x13
	.byte 0x01
	.byte 0x3b
	.byte 0x05
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x32
	.byte 0x0b
	.byte 0x0b
	.byte 0x0b
	.2byte 0x0000
	.byte 0x17
	.byte 0x17
	.byte 0x01
	.byte 0x3b
	.byte 0x0b
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x32
	.byte 0x0b
	.byte 0x0b
	.byte 0x0b
	.2byte 0x0000
	.byte 0x18
	.byte 0x0d
	.byte 0x00
	.byte 0x3b
	.byte 0x0b
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x32
	.byte 0x0b
	.byte 0x38
	.byte 0x0a
	.byte 0x03
	.byte 0x08
	.byte 0x49
	.byte 0x13
	.2byte 0x0000
	.byte 0x19
	.byte 0x17
	.byte 0x01
	.byte 0x3b
	.byte 0x05
	.byte 0x39
	.byte 0x0b
	.byte 0x3a
	.byte 0x0b
	.byte 0x32
	.byte 0x0b
	.byte 0x0b
	.byte 0x0b
	.byte 0x03
	.byte 0x08
	.2byte 0x0000
	.byte 0x1a
	.byte 0x01
	.byte 0x01
	.byte 0x32
	.byte 0x0b
	.byte 0x03
	.byte 0x08
	.byte 0x49
	.byte 0x13
	.byte 0x0b
	.byte 0x0b
	.2byte 0x0000
	.byte 0x1b
	.byte 0x21
	.byte 0x00
	.byte 0x32
	.byte 0x0b
	.byte 0x03
	.byte 0x08
	.byte 0x2f
	.byte 0x0b
	.byte 0x22
	.byte 0x0b
	.2byte 0x0000
	.byte 0x1c
	.byte 0x01
	.byte 0x01
	.byte 0x49
	.byte 0x13
	.byte 0x0b
	.byte 0x05
	.2byte 0x0000
	.byte 0x1d
	.byte 0x21
	.byte 0x00
	.byte 0x2f
	.byte 0x05
	.2byte 0x0000
	.byte 0x00
// -- Begin DWARF2 SEGMENT .debug_frame
	.section .debug_frame
.debug_frame_seg:
	.align 1
	.4byte 0x00000014
	.4byte 0xffffffff
	.byte 0x01
	.byte 0x00
	.byte 0x01
	.byte 0x78
	.byte 0x10
	.4byte 0x9008070c
	.byte 0x01
	.4byte 0x00000000
	.2byte 0x0000
	.4byte 0x00000024
	.4byte .debug_frame_seg
	.8byte ..___tag_value_uint16_lsbit.2
	.4byte ..___tag_value_uint16_lsbit.8-..___tag_value_uint16_lsbit.2
	.byte 0x04
	.4byte ..___tag_value_uint16_lsbit.3-..___tag_value_uint16_lsbit.2
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_uint16_lsbit.7-..___tag_value_uint16_lsbit.3
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.4byte 0x00000024
	.4byte .debug_frame_seg
	.8byte ..___tag_value_uint128_shr.11
	.4byte ..___tag_value_uint128_shr.17-..___tag_value_uint128_shr.11
	.byte 0x04
	.4byte ..___tag_value_uint128_shr.12-..___tag_value_uint128_shr.11
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_uint128_shr.16-..___tag_value_uint128_shr.12
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.4byte 0x00000024
	.4byte .debug_frame_seg
	.8byte ..___tag_value_uint128_bswap.20
	.4byte ..___tag_value_uint128_bswap.26-..___tag_value_uint128_bswap.20
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap.21-..___tag_value_uint128_bswap.20
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap.25-..___tag_value_uint128_bswap.21
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.4byte 0x00000024
	.4byte .debug_frame_seg
	.8byte ..___tag_value_uint128_bswap_copy.29
	.4byte ..___tag_value_uint128_bswap_copy.35-..___tag_value_uint128_bswap_copy.29
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap_copy.30-..___tag_value_uint128_bswap_copy.29
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap_copy.34-..___tag_value_uint128_bswap_copy.30
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.4byte 0x00000034
	.4byte .debug_frame_seg
	.8byte ..___tag_value_NucStrFastaExprAlloc.38
	.4byte ..___tag_value_NucStrFastaExprAlloc.47-..___tag_value_NucStrFastaExprAlloc.38
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprAlloc.39-..___tag_value_NucStrFastaExprAlloc.38
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprAlloc.43-..___tag_value_NucStrFastaExprAlloc.39
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprAlloc.44-..___tag_value_NucStrFastaExprAlloc.43
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprAlloc.46-..___tag_value_NucStrFastaExprAlloc.44
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000044
	.4byte .debug_frame_seg
	.8byte ..___tag_value_NucStrFastaExprMake2.50
	.4byte ..___tag_value_NucStrFastaExprMake2.62-..___tag_value_NucStrFastaExprMake2.50
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.51-..___tag_value_NucStrFastaExprMake2.50
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.55-..___tag_value_NucStrFastaExprMake2.51
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.56-..___tag_value_NucStrFastaExprMake2.55
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.58-..___tag_value_NucStrFastaExprMake2.56
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.59-..___tag_value_NucStrFastaExprMake2.58
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.61-..___tag_value_NucStrFastaExprMake2.59
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.4byte 0x00000044
	.4byte .debug_frame_seg
	.8byte ..___tag_value_NucStrFastaExprMake4.65
	.4byte ..___tag_value_NucStrFastaExprMake4.77-..___tag_value_NucStrFastaExprMake4.65
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.66-..___tag_value_NucStrFastaExprMake4.65
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.70-..___tag_value_NucStrFastaExprMake4.66
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.71-..___tag_value_NucStrFastaExprMake4.70
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.73-..___tag_value_NucStrFastaExprMake4.71
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.74-..___tag_value_NucStrFastaExprMake4.73
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.76-..___tag_value_NucStrFastaExprMake4.74
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.4byte 0x00000024
	.4byte .debug_frame_seg
	.8byte ..___tag_value_nss_sob.80
	.4byte ..___tag_value_nss_sob.86-..___tag_value_nss_sob.80
	.byte 0x04
	.4byte ..___tag_value_nss_sob.81-..___tag_value_nss_sob.80
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_sob.85-..___tag_value_nss_sob.81
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.4byte 0x00000034
	.4byte .debug_frame_seg
	.8byte ..___tag_value_nss_FASTA_expr.89
	.4byte ..___tag_value_nss_FASTA_expr.97-..___tag_value_nss_FASTA_expr.89
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.90-..___tag_value_nss_FASTA_expr.89
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.93-..___tag_value_nss_FASTA_expr.90
	.byte 0x83
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.94-..___tag_value_nss_FASTA_expr.93
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.96-..___tag_value_nss_FASTA_expr.94
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.byte 0x00
	.4byte 0x0000004c
	.4byte .debug_frame_seg
	.8byte ..___tag_value_nss_fasta_expr.100
	.4byte ..___tag_value_nss_fasta_expr.113-..___tag_value_nss_fasta_expr.100
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.101-..___tag_value_nss_fasta_expr.100
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.104-..___tag_value_nss_fasta_expr.101
	.byte 0x83
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.105-..___tag_value_nss_fasta_expr.104
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.107-..___tag_value_nss_fasta_expr.105
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.108-..___tag_value_nss_fasta_expr.107
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.110-..___tag_value_nss_fasta_expr.108
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.112-..___tag_value_nss_fasta_expr.110
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.4byte 0x00000064
	.4byte .debug_frame_seg
	.8byte ..___tag_value_nss_primary_expr.116
	.4byte ..___tag_value_nss_primary_expr.134-..___tag_value_nss_primary_expr.116
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.117-..___tag_value_nss_primary_expr.116
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.120-..___tag_value_nss_primary_expr.117
	.byte 0x83
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.121-..___tag_value_nss_primary_expr.120
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.123-..___tag_value_nss_primary_expr.121
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.124-..___tag_value_nss_primary_expr.123
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.126-..___tag_value_nss_primary_expr.124
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.128-..___tag_value_nss_primary_expr.126
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.129-..___tag_value_nss_primary_expr.128
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.131-..___tag_value_nss_primary_expr.129
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.133-..___tag_value_nss_primary_expr.131
	.byte 0x09
	.2byte 0x0606
	.byte 0x00
	.4byte 0x0000004c
	.4byte .debug_frame_seg
	.8byte ..___tag_value_nss_unary_expr.137
	.4byte ..___tag_value_nss_unary_expr.150-..___tag_value_nss_unary_expr.137
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.138-..___tag_value_nss_unary_expr.137
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.141-..___tag_value_nss_unary_expr.138
	.byte 0x83
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.142-..___tag_value_nss_unary_expr.141
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.144-..___tag_value_nss_unary_expr.142
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.145-..___tag_value_nss_unary_expr.144
	.byte 0x83
	.byte 0x05
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.147-..___tag_value_nss_unary_expr.145
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.149-..___tag_value_nss_unary_expr.147
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.4byte 0x0000007c
	.4byte .debug_frame_seg
	.8byte ..___tag_value_nss_expr.153
	.4byte ..___tag_value_nss_expr.176-..___tag_value_nss_expr.153
	.byte 0x04
	.4byte ..___tag_value_nss_expr.154-..___tag_value_nss_expr.153
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_expr.157-..___tag_value_nss_expr.154
	.byte 0x83
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_nss_expr.158-..___tag_value_nss_expr.157
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_expr.160-..___tag_value_nss_expr.158
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_expr.161-..___tag_value_nss_expr.160
	.byte 0x83
	.byte 0x05
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_expr.163-..___tag_value_nss_expr.161
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_expr.165-..___tag_value_nss_expr.163
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_expr.166-..___tag_value_nss_expr.165
	.byte 0x83
	.byte 0x05
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_expr.168-..___tag_value_nss_expr.166
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_expr.170-..___tag_value_nss_expr.168
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_expr.171-..___tag_value_nss_expr.170
	.byte 0x83
	.byte 0x05
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_expr.173-..___tag_value_nss_expr.171
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_expr.175-..___tag_value_nss_expr.173
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000024
	.4byte .debug_frame_seg
	.8byte ..___tag_value_NucStrstrInit.179
	.4byte ..___tag_value_NucStrstrInit.185-..___tag_value_NucStrstrInit.179
	.byte 0x04
	.4byte ..___tag_value_NucStrstrInit.180-..___tag_value_NucStrstrInit.179
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrInit.184-..___tag_value_NucStrstrInit.180
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.4byte 0x00000064
	.4byte .debug_frame_seg
	.8byte ..___tag_value_NucStrstrMake.188
	.4byte ..___tag_value_NucStrstrMake.206-..___tag_value_NucStrstrMake.188
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.189-..___tag_value_NucStrstrMake.188
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.192-..___tag_value_NucStrstrMake.189
	.byte 0x83
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.193-..___tag_value_NucStrstrMake.192
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.195-..___tag_value_NucStrstrMake.193
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.196-..___tag_value_NucStrstrMake.195
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.198-..___tag_value_NucStrstrMake.196
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.200-..___tag_value_NucStrstrMake.198
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.201-..___tag_value_NucStrstrMake.200
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.203-..___tag_value_NucStrstrMake.201
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.205-..___tag_value_NucStrstrMake.203
	.byte 0x09
	.2byte 0x0606
	.byte 0x00
	.4byte 0x00000024
	.4byte .debug_frame_seg
	.8byte ..___tag_value_NucStrstrWhack.209
	.4byte ..___tag_value_NucStrstrWhack.215-..___tag_value_NucStrstrWhack.209
	.byte 0x04
	.4byte ..___tag_value_NucStrstrWhack.210-..___tag_value_NucStrstrWhack.209
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrWhack.214-..___tag_value_NucStrstrWhack.210
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.4byte 0x00000024
	.4byte .debug_frame_seg
	.8byte ..___tag_value_prime_buffer_2na.218
	.4byte ..___tag_value_prime_buffer_2na.224-..___tag_value_prime_buffer_2na.218
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_2na.219-..___tag_value_prime_buffer_2na.218
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_2na.223-..___tag_value_prime_buffer_2na.219
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.4byte 0x00000074
	.4byte .debug_frame_seg
	.8byte ..___tag_value_eval_2na_8.227
	.4byte ..___tag_value_eval_2na_8.248-..___tag_value_eval_2na_8.227
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.228-..___tag_value_eval_2na_8.227
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.232-..___tag_value_eval_2na_8.228
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.233-..___tag_value_eval_2na_8.232
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.235-..___tag_value_eval_2na_8.233
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.236-..___tag_value_eval_2na_8.235
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.238-..___tag_value_eval_2na_8.236
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.239-..___tag_value_eval_2na_8.238
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.241-..___tag_value_eval_2na_8.239
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.242-..___tag_value_eval_2na_8.241
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.244-..___tag_value_eval_2na_8.242
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.245-..___tag_value_eval_2na_8.244
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.247-..___tag_value_eval_2na_8.245
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000074
	.4byte .debug_frame_seg
	.8byte ..___tag_value_eval_2na_16.251
	.4byte ..___tag_value_eval_2na_16.272-..___tag_value_eval_2na_16.251
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.252-..___tag_value_eval_2na_16.251
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.256-..___tag_value_eval_2na_16.252
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.257-..___tag_value_eval_2na_16.256
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.259-..___tag_value_eval_2na_16.257
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.260-..___tag_value_eval_2na_16.259
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.262-..___tag_value_eval_2na_16.260
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.263-..___tag_value_eval_2na_16.262
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.265-..___tag_value_eval_2na_16.263
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.266-..___tag_value_eval_2na_16.265
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.268-..___tag_value_eval_2na_16.266
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.269-..___tag_value_eval_2na_16.268
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.271-..___tag_value_eval_2na_16.269
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000074
	.4byte .debug_frame_seg
	.8byte ..___tag_value_eval_2na_32.275
	.4byte ..___tag_value_eval_2na_32.296-..___tag_value_eval_2na_32.275
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.276-..___tag_value_eval_2na_32.275
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.280-..___tag_value_eval_2na_32.276
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.281-..___tag_value_eval_2na_32.280
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.283-..___tag_value_eval_2na_32.281
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.284-..___tag_value_eval_2na_32.283
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.286-..___tag_value_eval_2na_32.284
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.287-..___tag_value_eval_2na_32.286
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.289-..___tag_value_eval_2na_32.287
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.290-..___tag_value_eval_2na_32.289
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.292-..___tag_value_eval_2na_32.290
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.293-..___tag_value_eval_2na_32.292
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.295-..___tag_value_eval_2na_32.293
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000074
	.4byte .debug_frame_seg
	.8byte ..___tag_value_eval_2na_64.299
	.4byte ..___tag_value_eval_2na_64.320-..___tag_value_eval_2na_64.299
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.300-..___tag_value_eval_2na_64.299
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.304-..___tag_value_eval_2na_64.300
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.305-..___tag_value_eval_2na_64.304
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.307-..___tag_value_eval_2na_64.305
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.308-..___tag_value_eval_2na_64.307
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.310-..___tag_value_eval_2na_64.308
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.311-..___tag_value_eval_2na_64.310
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.313-..___tag_value_eval_2na_64.311
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.314-..___tag_value_eval_2na_64.313
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.316-..___tag_value_eval_2na_64.314
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.317-..___tag_value_eval_2na_64.316
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.319-..___tag_value_eval_2na_64.317
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x0000007c
	.4byte .debug_frame_seg
	.8byte ..___tag_value_eval_2na_128.323
	.4byte ..___tag_value_eval_2na_128.347-..___tag_value_eval_2na_128.323
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.324-..___tag_value_eval_2na_128.323
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.328-..___tag_value_eval_2na_128.324
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.329-..___tag_value_eval_2na_128.328
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.331-..___tag_value_eval_2na_128.329
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.332-..___tag_value_eval_2na_128.331
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.334-..___tag_value_eval_2na_128.332
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.335-..___tag_value_eval_2na_128.334
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.337-..___tag_value_eval_2na_128.335
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.338-..___tag_value_eval_2na_128.337
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.340-..___tag_value_eval_2na_128.338
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.341-..___tag_value_eval_2na_128.340
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.343-..___tag_value_eval_2na_128.341
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.344-..___tag_value_eval_2na_128.343
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.346-..___tag_value_eval_2na_128.344
	.byte 0x09
	.2byte 0x0606
	.4byte 0x000000dc
	.4byte .debug_frame_seg
	.8byte ..___tag_value_eval_2na_pos.350
	.4byte ..___tag_value_eval_2na_pos.392-..___tag_value_eval_2na_pos.350
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.351-..___tag_value_eval_2na_pos.350
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.355-..___tag_value_eval_2na_pos.351
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.356-..___tag_value_eval_2na_pos.355
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.358-..___tag_value_eval_2na_pos.356
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.359-..___tag_value_eval_2na_pos.358
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.361-..___tag_value_eval_2na_pos.359
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.362-..___tag_value_eval_2na_pos.361
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.364-..___tag_value_eval_2na_pos.362
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.365-..___tag_value_eval_2na_pos.364
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.367-..___tag_value_eval_2na_pos.365
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.368-..___tag_value_eval_2na_pos.367
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.370-..___tag_value_eval_2na_pos.368
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.371-..___tag_value_eval_2na_pos.370
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.373-..___tag_value_eval_2na_pos.371
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.374-..___tag_value_eval_2na_pos.373
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.376-..___tag_value_eval_2na_pos.374
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.377-..___tag_value_eval_2na_pos.376
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.379-..___tag_value_eval_2na_pos.377
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.380-..___tag_value_eval_2na_pos.379
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.382-..___tag_value_eval_2na_pos.380
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.383-..___tag_value_eval_2na_pos.382
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.385-..___tag_value_eval_2na_pos.383
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.386-..___tag_value_eval_2na_pos.385
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.388-..___tag_value_eval_2na_pos.386
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.389-..___tag_value_eval_2na_pos.388
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.391-..___tag_value_eval_2na_pos.389
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.4byte 0x00000024
	.4byte .debug_frame_seg
	.8byte ..___tag_value_prime_buffer_4na.395
	.4byte ..___tag_value_prime_buffer_4na.401-..___tag_value_prime_buffer_4na.395
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_4na.396-..___tag_value_prime_buffer_4na.395
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_4na.400-..___tag_value_prime_buffer_4na.396
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.4byte 0x00000074
	.4byte .debug_frame_seg
	.8byte ..___tag_value_eval_4na_16.404
	.4byte ..___tag_value_eval_4na_16.425-..___tag_value_eval_4na_16.404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.405-..___tag_value_eval_4na_16.404
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.409-..___tag_value_eval_4na_16.405
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.410-..___tag_value_eval_4na_16.409
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.412-..___tag_value_eval_4na_16.410
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.413-..___tag_value_eval_4na_16.412
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.415-..___tag_value_eval_4na_16.413
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.416-..___tag_value_eval_4na_16.415
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.418-..___tag_value_eval_4na_16.416
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.419-..___tag_value_eval_4na_16.418
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.421-..___tag_value_eval_4na_16.419
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.422-..___tag_value_eval_4na_16.421
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.424-..___tag_value_eval_4na_16.422
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000074
	.4byte .debug_frame_seg
	.8byte ..___tag_value_eval_4na_32.428
	.4byte ..___tag_value_eval_4na_32.449-..___tag_value_eval_4na_32.428
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.429-..___tag_value_eval_4na_32.428
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.433-..___tag_value_eval_4na_32.429
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.434-..___tag_value_eval_4na_32.433
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.436-..___tag_value_eval_4na_32.434
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.437-..___tag_value_eval_4na_32.436
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.439-..___tag_value_eval_4na_32.437
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.440-..___tag_value_eval_4na_32.439
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.442-..___tag_value_eval_4na_32.440
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.443-..___tag_value_eval_4na_32.442
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.445-..___tag_value_eval_4na_32.443
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.446-..___tag_value_eval_4na_32.445
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.448-..___tag_value_eval_4na_32.446
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000074
	.4byte .debug_frame_seg
	.8byte ..___tag_value_eval_4na_64.452
	.4byte ..___tag_value_eval_4na_64.473-..___tag_value_eval_4na_64.452
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.453-..___tag_value_eval_4na_64.452
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.457-..___tag_value_eval_4na_64.453
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.458-..___tag_value_eval_4na_64.457
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.460-..___tag_value_eval_4na_64.458
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.461-..___tag_value_eval_4na_64.460
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.463-..___tag_value_eval_4na_64.461
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.464-..___tag_value_eval_4na_64.463
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.466-..___tag_value_eval_4na_64.464
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.467-..___tag_value_eval_4na_64.466
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.469-..___tag_value_eval_4na_64.467
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.470-..___tag_value_eval_4na_64.469
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.472-..___tag_value_eval_4na_64.470
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x0000007c
	.4byte .debug_frame_seg
	.8byte ..___tag_value_eval_4na_128.476
	.4byte ..___tag_value_eval_4na_128.500-..___tag_value_eval_4na_128.476
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.477-..___tag_value_eval_4na_128.476
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.481-..___tag_value_eval_4na_128.477
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.482-..___tag_value_eval_4na_128.481
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.484-..___tag_value_eval_4na_128.482
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.485-..___tag_value_eval_4na_128.484
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.487-..___tag_value_eval_4na_128.485
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.488-..___tag_value_eval_4na_128.487
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.490-..___tag_value_eval_4na_128.488
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.491-..___tag_value_eval_4na_128.490
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.493-..___tag_value_eval_4na_128.491
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.494-..___tag_value_eval_4na_128.493
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.496-..___tag_value_eval_4na_128.494
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.497-..___tag_value_eval_4na_128.496
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.499-..___tag_value_eval_4na_128.497
	.byte 0x09
	.2byte 0x0606
	.4byte 0x000000dc
	.4byte .debug_frame_seg
	.8byte ..___tag_value_eval_4na_pos.503
	.4byte ..___tag_value_eval_4na_pos.545-..___tag_value_eval_4na_pos.503
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.504-..___tag_value_eval_4na_pos.503
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.508-..___tag_value_eval_4na_pos.504
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.509-..___tag_value_eval_4na_pos.508
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.511-..___tag_value_eval_4na_pos.509
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.512-..___tag_value_eval_4na_pos.511
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.514-..___tag_value_eval_4na_pos.512
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.515-..___tag_value_eval_4na_pos.514
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.517-..___tag_value_eval_4na_pos.515
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.518-..___tag_value_eval_4na_pos.517
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.520-..___tag_value_eval_4na_pos.518
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.521-..___tag_value_eval_4na_pos.520
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.523-..___tag_value_eval_4na_pos.521
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.524-..___tag_value_eval_4na_pos.523
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.526-..___tag_value_eval_4na_pos.524
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.527-..___tag_value_eval_4na_pos.526
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.529-..___tag_value_eval_4na_pos.527
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.530-..___tag_value_eval_4na_pos.529
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.532-..___tag_value_eval_4na_pos.530
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.533-..___tag_value_eval_4na_pos.532
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.535-..___tag_value_eval_4na_pos.533
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.536-..___tag_value_eval_4na_pos.535
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.538-..___tag_value_eval_4na_pos.536
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.539-..___tag_value_eval_4na_pos.538
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.541-..___tag_value_eval_4na_pos.539
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.542-..___tag_value_eval_4na_pos.541
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.544-..___tag_value_eval_4na_pos.542
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.4byte 0x0000033c
	.4byte .debug_frame_seg
	.8byte ..___tag_value_NucStrstrSearch.548
	.4byte ..___tag_value_NucStrstrSearch.711-..___tag_value_NucStrstrSearch.548
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.549-..___tag_value_NucStrstrSearch.548
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.552-..___tag_value_NucStrstrSearch.549
	.byte 0x83
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.553-..___tag_value_NucStrstrSearch.552
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.555-..___tag_value_NucStrstrSearch.553
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.556-..___tag_value_NucStrstrSearch.555
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.558-..___tag_value_NucStrstrSearch.556
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.560-..___tag_value_NucStrstrSearch.558
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.561-..___tag_value_NucStrstrSearch.560
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.563-..___tag_value_NucStrstrSearch.561
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.565-..___tag_value_NucStrstrSearch.563
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.566-..___tag_value_NucStrstrSearch.565
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.568-..___tag_value_NucStrstrSearch.566
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.570-..___tag_value_NucStrstrSearch.568
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.571-..___tag_value_NucStrstrSearch.570
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.573-..___tag_value_NucStrstrSearch.571
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.575-..___tag_value_NucStrstrSearch.573
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.576-..___tag_value_NucStrstrSearch.575
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.578-..___tag_value_NucStrstrSearch.576
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.580-..___tag_value_NucStrstrSearch.578
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.581-..___tag_value_NucStrstrSearch.580
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.583-..___tag_value_NucStrstrSearch.581
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.585-..___tag_value_NucStrstrSearch.583
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.586-..___tag_value_NucStrstrSearch.585
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.588-..___tag_value_NucStrstrSearch.586
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.590-..___tag_value_NucStrstrSearch.588
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.591-..___tag_value_NucStrstrSearch.590
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.593-..___tag_value_NucStrstrSearch.591
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.595-..___tag_value_NucStrstrSearch.593
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.596-..___tag_value_NucStrstrSearch.595
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.598-..___tag_value_NucStrstrSearch.596
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.600-..___tag_value_NucStrstrSearch.598
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.601-..___tag_value_NucStrstrSearch.600
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.603-..___tag_value_NucStrstrSearch.601
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.605-..___tag_value_NucStrstrSearch.603
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.606-..___tag_value_NucStrstrSearch.605
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.608-..___tag_value_NucStrstrSearch.606
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.610-..___tag_value_NucStrstrSearch.608
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.611-..___tag_value_NucStrstrSearch.610
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.613-..___tag_value_NucStrstrSearch.611
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.615-..___tag_value_NucStrstrSearch.613
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.616-..___tag_value_NucStrstrSearch.615
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.618-..___tag_value_NucStrstrSearch.616
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.620-..___tag_value_NucStrstrSearch.618
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.621-..___tag_value_NucStrstrSearch.620
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.623-..___tag_value_NucStrstrSearch.621
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.625-..___tag_value_NucStrstrSearch.623
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.626-..___tag_value_NucStrstrSearch.625
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.628-..___tag_value_NucStrstrSearch.626
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.630-..___tag_value_NucStrstrSearch.628
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.631-..___tag_value_NucStrstrSearch.630
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.633-..___tag_value_NucStrstrSearch.631
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.635-..___tag_value_NucStrstrSearch.633
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.636-..___tag_value_NucStrstrSearch.635
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.638-..___tag_value_NucStrstrSearch.636
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.640-..___tag_value_NucStrstrSearch.638
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.641-..___tag_value_NucStrstrSearch.640
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.643-..___tag_value_NucStrstrSearch.641
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.645-..___tag_value_NucStrstrSearch.643
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.646-..___tag_value_NucStrstrSearch.645
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.648-..___tag_value_NucStrstrSearch.646
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.650-..___tag_value_NucStrstrSearch.648
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.651-..___tag_value_NucStrstrSearch.650
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.653-..___tag_value_NucStrstrSearch.651
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.655-..___tag_value_NucStrstrSearch.653
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.656-..___tag_value_NucStrstrSearch.655
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.658-..___tag_value_NucStrstrSearch.656
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.660-..___tag_value_NucStrstrSearch.658
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.661-..___tag_value_NucStrstrSearch.660
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.663-..___tag_value_NucStrstrSearch.661
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.665-..___tag_value_NucStrstrSearch.663
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.666-..___tag_value_NucStrstrSearch.665
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.668-..___tag_value_NucStrstrSearch.666
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.670-..___tag_value_NucStrstrSearch.668
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.671-..___tag_value_NucStrstrSearch.670
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.673-..___tag_value_NucStrstrSearch.671
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.675-..___tag_value_NucStrstrSearch.673
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.676-..___tag_value_NucStrstrSearch.675
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.678-..___tag_value_NucStrstrSearch.676
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.680-..___tag_value_NucStrstrSearch.678
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.681-..___tag_value_NucStrstrSearch.680
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.683-..___tag_value_NucStrstrSearch.681
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.685-..___tag_value_NucStrstrSearch.683
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.686-..___tag_value_NucStrstrSearch.685
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.688-..___tag_value_NucStrstrSearch.686
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.690-..___tag_value_NucStrstrSearch.688
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.691-..___tag_value_NucStrstrSearch.690
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.693-..___tag_value_NucStrstrSearch.691
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.695-..___tag_value_NucStrstrSearch.693
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.696-..___tag_value_NucStrstrSearch.695
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.698-..___tag_value_NucStrstrSearch.696
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.700-..___tag_value_NucStrstrSearch.698
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.701-..___tag_value_NucStrstrSearch.700
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.703-..___tag_value_NucStrstrSearch.701
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.705-..___tag_value_NucStrstrSearch.703
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.706-..___tag_value_NucStrstrSearch.705
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.708-..___tag_value_NucStrstrSearch.706
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.710-..___tag_value_NucStrstrSearch.708
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
// -- Begin DWARF2 SEGMENT .eh_frame
	.section .eh_frame,"a",@progbits
.eh_frame_seg:
	.align 1
	.4byte 0x00000014
	.4byte 0x00000000
	.byte 0x01
	.2byte 0x527a
	.byte 0x00
	.byte 0x01
	.byte 0x78
	.byte 0x10
	.byte 0x01
	.4byte 0x08070c1b
	.2byte 0x0190
	.2byte 0x0000
	.4byte 0x00000024
	.4byte 0x0000001c
	.4byte ..___tag_value_uint16_lsbit.2-.
	.4byte ..___tag_value_uint16_lsbit.8-..___tag_value_uint16_lsbit.2
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_uint16_lsbit.3-..___tag_value_uint16_lsbit.2
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_uint16_lsbit.7-..___tag_value_uint16_lsbit.3
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.byte 0x00
	.4byte 0x00000024
	.4byte 0x00000044
	.4byte ..___tag_value_uint128_shr.11-.
	.4byte ..___tag_value_uint128_shr.17-..___tag_value_uint128_shr.11
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_uint128_shr.12-..___tag_value_uint128_shr.11
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_uint128_shr.16-..___tag_value_uint128_shr.12
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.byte 0x00
	.4byte 0x00000024
	.4byte 0x0000006c
	.4byte ..___tag_value_uint128_bswap.20-.
	.4byte ..___tag_value_uint128_bswap.26-..___tag_value_uint128_bswap.20
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap.21-..___tag_value_uint128_bswap.20
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap.25-..___tag_value_uint128_bswap.21
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.byte 0x00
	.4byte 0x00000024
	.4byte 0x00000094
	.4byte ..___tag_value_uint128_bswap_copy.29-.
	.4byte ..___tag_value_uint128_bswap_copy.35-..___tag_value_uint128_bswap_copy.29
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap_copy.30-..___tag_value_uint128_bswap_copy.29
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap_copy.34-..___tag_value_uint128_bswap_copy.30
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.byte 0x00
	.4byte 0x00000034
	.4byte 0x000000bc
	.4byte ..___tag_value_NucStrFastaExprAlloc.38-.
	.4byte ..___tag_value_NucStrFastaExprAlloc.47-..___tag_value_NucStrFastaExprAlloc.38
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprAlloc.39-..___tag_value_NucStrFastaExprAlloc.38
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprAlloc.43-..___tag_value_NucStrFastaExprAlloc.39
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprAlloc.44-..___tag_value_NucStrFastaExprAlloc.43
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprAlloc.46-..___tag_value_NucStrFastaExprAlloc.44
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.4byte 0x00000044
	.4byte 0x000000f4
	.4byte ..___tag_value_NucStrFastaExprMake2.50-.
	.4byte ..___tag_value_NucStrFastaExprMake2.62-..___tag_value_NucStrFastaExprMake2.50
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.51-..___tag_value_NucStrFastaExprMake2.50
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.55-..___tag_value_NucStrFastaExprMake2.51
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.56-..___tag_value_NucStrFastaExprMake2.55
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.58-..___tag_value_NucStrFastaExprMake2.56
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.59-..___tag_value_NucStrFastaExprMake2.58
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.61-..___tag_value_NucStrFastaExprMake2.59
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000044
	.4byte 0x0000013c
	.4byte ..___tag_value_NucStrFastaExprMake4.65-.
	.4byte ..___tag_value_NucStrFastaExprMake4.77-..___tag_value_NucStrFastaExprMake4.65
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.66-..___tag_value_NucStrFastaExprMake4.65
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.70-..___tag_value_NucStrFastaExprMake4.66
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.71-..___tag_value_NucStrFastaExprMake4.70
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.73-..___tag_value_NucStrFastaExprMake4.71
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.74-..___tag_value_NucStrFastaExprMake4.73
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.76-..___tag_value_NucStrFastaExprMake4.74
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000024
	.4byte 0x00000184
	.4byte ..___tag_value_nss_sob.80-.
	.4byte ..___tag_value_nss_sob.86-..___tag_value_nss_sob.80
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_nss_sob.81-..___tag_value_nss_sob.80
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_sob.85-..___tag_value_nss_sob.81
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.byte 0x00
	.4byte 0x00000034
	.4byte 0x000001ac
	.4byte ..___tag_value_nss_FASTA_expr.89-.
	.4byte ..___tag_value_nss_FASTA_expr.97-..___tag_value_nss_FASTA_expr.89
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.90-..___tag_value_nss_FASTA_expr.89
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.93-..___tag_value_nss_FASTA_expr.90
	.byte 0x83
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.94-..___tag_value_nss_FASTA_expr.93
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.96-..___tag_value_nss_FASTA_expr.94
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.4byte 0x0000004c
	.4byte 0x000001e4
	.4byte ..___tag_value_nss_fasta_expr.100-.
	.4byte ..___tag_value_nss_fasta_expr.113-..___tag_value_nss_fasta_expr.100
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.101-..___tag_value_nss_fasta_expr.100
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.104-..___tag_value_nss_fasta_expr.101
	.byte 0x83
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.105-..___tag_value_nss_fasta_expr.104
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.107-..___tag_value_nss_fasta_expr.105
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.108-..___tag_value_nss_fasta_expr.107
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.110-..___tag_value_nss_fasta_expr.108
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.112-..___tag_value_nss_fasta_expr.110
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.byte 0x00
	.4byte 0x00000064
	.4byte 0x00000234
	.4byte ..___tag_value_nss_primary_expr.116-.
	.4byte ..___tag_value_nss_primary_expr.134-..___tag_value_nss_primary_expr.116
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.117-..___tag_value_nss_primary_expr.116
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.120-..___tag_value_nss_primary_expr.117
	.byte 0x83
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.121-..___tag_value_nss_primary_expr.120
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.123-..___tag_value_nss_primary_expr.121
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.124-..___tag_value_nss_primary_expr.123
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.126-..___tag_value_nss_primary_expr.124
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.128-..___tag_value_nss_primary_expr.126
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.129-..___tag_value_nss_primary_expr.128
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.131-..___tag_value_nss_primary_expr.129
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.133-..___tag_value_nss_primary_expr.131
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.4byte 0x0000004c
	.4byte 0x0000029c
	.4byte ..___tag_value_nss_unary_expr.137-.
	.4byte ..___tag_value_nss_unary_expr.150-..___tag_value_nss_unary_expr.137
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.138-..___tag_value_nss_unary_expr.137
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.141-..___tag_value_nss_unary_expr.138
	.byte 0x83
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.142-..___tag_value_nss_unary_expr.141
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.144-..___tag_value_nss_unary_expr.142
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.145-..___tag_value_nss_unary_expr.144
	.byte 0x83
	.byte 0x05
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.147-..___tag_value_nss_unary_expr.145
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.149-..___tag_value_nss_unary_expr.147
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.byte 0x00
	.4byte 0x0000007c
	.4byte 0x000002ec
	.4byte ..___tag_value_nss_expr.153-.
	.4byte ..___tag_value_nss_expr.176-..___tag_value_nss_expr.153
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_nss_expr.154-..___tag_value_nss_expr.153
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_expr.157-..___tag_value_nss_expr.154
	.byte 0x83
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_nss_expr.158-..___tag_value_nss_expr.157
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_expr.160-..___tag_value_nss_expr.158
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_expr.161-..___tag_value_nss_expr.160
	.byte 0x83
	.byte 0x05
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_expr.163-..___tag_value_nss_expr.161
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_expr.165-..___tag_value_nss_expr.163
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_expr.166-..___tag_value_nss_expr.165
	.byte 0x83
	.byte 0x05
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_expr.168-..___tag_value_nss_expr.166
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_expr.170-..___tag_value_nss_expr.168
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_expr.171-..___tag_value_nss_expr.170
	.byte 0x83
	.byte 0x05
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_expr.173-..___tag_value_nss_expr.171
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_expr.175-..___tag_value_nss_expr.173
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000024
	.4byte 0x0000036c
	.4byte ..___tag_value_NucStrstrInit.179-.
	.4byte ..___tag_value_NucStrstrInit.185-..___tag_value_NucStrstrInit.179
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_NucStrstrInit.180-..___tag_value_NucStrstrInit.179
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrInit.184-..___tag_value_NucStrstrInit.180
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.byte 0x00
	.4byte 0x00000064
	.4byte 0x00000394
	.4byte ..___tag_value_NucStrstrMake.188-.
	.4byte ..___tag_value_NucStrstrMake.206-..___tag_value_NucStrstrMake.188
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.189-..___tag_value_NucStrstrMake.188
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.192-..___tag_value_NucStrstrMake.189
	.byte 0x83
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.193-..___tag_value_NucStrstrMake.192
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.195-..___tag_value_NucStrstrMake.193
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.196-..___tag_value_NucStrstrMake.195
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.198-..___tag_value_NucStrstrMake.196
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.200-..___tag_value_NucStrstrMake.198
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.201-..___tag_value_NucStrstrMake.200
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.203-..___tag_value_NucStrstrMake.201
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.205-..___tag_value_NucStrstrMake.203
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.4byte 0x00000024
	.4byte 0x000003fc
	.4byte ..___tag_value_NucStrstrWhack.209-.
	.4byte ..___tag_value_NucStrstrWhack.215-..___tag_value_NucStrstrWhack.209
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_NucStrstrWhack.210-..___tag_value_NucStrstrWhack.209
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrWhack.214-..___tag_value_NucStrstrWhack.210
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.byte 0x00
	.4byte 0x00000024
	.4byte 0x00000424
	.4byte ..___tag_value_prime_buffer_2na.218-.
	.4byte ..___tag_value_prime_buffer_2na.224-..___tag_value_prime_buffer_2na.218
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_2na.219-..___tag_value_prime_buffer_2na.218
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_2na.223-..___tag_value_prime_buffer_2na.219
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.byte 0x00
	.4byte 0x0000006c
	.4byte 0x0000044c
	.4byte ..___tag_value_eval_2na_8.227-.
	.4byte ..___tag_value_eval_2na_8.248-..___tag_value_eval_2na_8.227
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.228-..___tag_value_eval_2na_8.227
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.232-..___tag_value_eval_2na_8.228
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.233-..___tag_value_eval_2na_8.232
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.235-..___tag_value_eval_2na_8.233
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.236-..___tag_value_eval_2na_8.235
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.238-..___tag_value_eval_2na_8.236
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.239-..___tag_value_eval_2na_8.238
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.241-..___tag_value_eval_2na_8.239
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.242-..___tag_value_eval_2na_8.241
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.244-..___tag_value_eval_2na_8.242
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.245-..___tag_value_eval_2na_8.244
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.247-..___tag_value_eval_2na_8.245
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.4byte 0x0000006c
	.4byte 0x000004bc
	.4byte ..___tag_value_eval_2na_16.251-.
	.4byte ..___tag_value_eval_2na_16.272-..___tag_value_eval_2na_16.251
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.252-..___tag_value_eval_2na_16.251
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.256-..___tag_value_eval_2na_16.252
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.257-..___tag_value_eval_2na_16.256
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.259-..___tag_value_eval_2na_16.257
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.260-..___tag_value_eval_2na_16.259
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.262-..___tag_value_eval_2na_16.260
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.263-..___tag_value_eval_2na_16.262
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.265-..___tag_value_eval_2na_16.263
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.266-..___tag_value_eval_2na_16.265
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.268-..___tag_value_eval_2na_16.266
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.269-..___tag_value_eval_2na_16.268
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.271-..___tag_value_eval_2na_16.269
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.4byte 0x0000006c
	.4byte 0x0000052c
	.4byte ..___tag_value_eval_2na_32.275-.
	.4byte ..___tag_value_eval_2na_32.296-..___tag_value_eval_2na_32.275
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.276-..___tag_value_eval_2na_32.275
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.280-..___tag_value_eval_2na_32.276
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.281-..___tag_value_eval_2na_32.280
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.283-..___tag_value_eval_2na_32.281
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.284-..___tag_value_eval_2na_32.283
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.286-..___tag_value_eval_2na_32.284
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.287-..___tag_value_eval_2na_32.286
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.289-..___tag_value_eval_2na_32.287
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.290-..___tag_value_eval_2na_32.289
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.292-..___tag_value_eval_2na_32.290
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.293-..___tag_value_eval_2na_32.292
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.295-..___tag_value_eval_2na_32.293
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.4byte 0x0000006c
	.4byte 0x0000059c
	.4byte ..___tag_value_eval_2na_64.299-.
	.4byte ..___tag_value_eval_2na_64.320-..___tag_value_eval_2na_64.299
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.300-..___tag_value_eval_2na_64.299
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.304-..___tag_value_eval_2na_64.300
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.305-..___tag_value_eval_2na_64.304
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.307-..___tag_value_eval_2na_64.305
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.308-..___tag_value_eval_2na_64.307
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.310-..___tag_value_eval_2na_64.308
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.311-..___tag_value_eval_2na_64.310
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.313-..___tag_value_eval_2na_64.311
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.314-..___tag_value_eval_2na_64.313
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.316-..___tag_value_eval_2na_64.314
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.317-..___tag_value_eval_2na_64.316
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.319-..___tag_value_eval_2na_64.317
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.4byte 0x0000007c
	.4byte 0x0000060c
	.4byte ..___tag_value_eval_2na_128.323-.
	.4byte ..___tag_value_eval_2na_128.347-..___tag_value_eval_2na_128.323
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.324-..___tag_value_eval_2na_128.323
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.328-..___tag_value_eval_2na_128.324
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.329-..___tag_value_eval_2na_128.328
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.331-..___tag_value_eval_2na_128.329
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.332-..___tag_value_eval_2na_128.331
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.334-..___tag_value_eval_2na_128.332
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.335-..___tag_value_eval_2na_128.334
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.337-..___tag_value_eval_2na_128.335
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.338-..___tag_value_eval_2na_128.337
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.340-..___tag_value_eval_2na_128.338
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.341-..___tag_value_eval_2na_128.340
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.343-..___tag_value_eval_2na_128.341
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.344-..___tag_value_eval_2na_128.343
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.346-..___tag_value_eval_2na_128.344
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.byte 0x00
	.4byte 0x000000d4
	.4byte 0x0000068c
	.4byte ..___tag_value_eval_2na_pos.350-.
	.4byte ..___tag_value_eval_2na_pos.392-..___tag_value_eval_2na_pos.350
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.351-..___tag_value_eval_2na_pos.350
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.355-..___tag_value_eval_2na_pos.351
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.356-..___tag_value_eval_2na_pos.355
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.358-..___tag_value_eval_2na_pos.356
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.359-..___tag_value_eval_2na_pos.358
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.361-..___tag_value_eval_2na_pos.359
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.362-..___tag_value_eval_2na_pos.361
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.364-..___tag_value_eval_2na_pos.362
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.365-..___tag_value_eval_2na_pos.364
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.367-..___tag_value_eval_2na_pos.365
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.368-..___tag_value_eval_2na_pos.367
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.370-..___tag_value_eval_2na_pos.368
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.371-..___tag_value_eval_2na_pos.370
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.373-..___tag_value_eval_2na_pos.371
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.374-..___tag_value_eval_2na_pos.373
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.376-..___tag_value_eval_2na_pos.374
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.377-..___tag_value_eval_2na_pos.376
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.379-..___tag_value_eval_2na_pos.377
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.380-..___tag_value_eval_2na_pos.379
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.382-..___tag_value_eval_2na_pos.380
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.383-..___tag_value_eval_2na_pos.382
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.385-..___tag_value_eval_2na_pos.383
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.386-..___tag_value_eval_2na_pos.385
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.388-..___tag_value_eval_2na_pos.386
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.389-..___tag_value_eval_2na_pos.388
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.391-..___tag_value_eval_2na_pos.389
	.byte 0x09
	.2byte 0x0606
	.byte 0x00
	.4byte 0x00000024
	.4byte 0x00000764
	.4byte ..___tag_value_prime_buffer_4na.395-.
	.4byte ..___tag_value_prime_buffer_4na.401-..___tag_value_prime_buffer_4na.395
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_4na.396-..___tag_value_prime_buffer_4na.395
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_4na.400-..___tag_value_prime_buffer_4na.396
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.byte 0x00
	.4byte 0x0000006c
	.4byte 0x0000078c
	.4byte ..___tag_value_eval_4na_16.404-.
	.4byte ..___tag_value_eval_4na_16.425-..___tag_value_eval_4na_16.404
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.405-..___tag_value_eval_4na_16.404
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.409-..___tag_value_eval_4na_16.405
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.410-..___tag_value_eval_4na_16.409
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.412-..___tag_value_eval_4na_16.410
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.413-..___tag_value_eval_4na_16.412
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.415-..___tag_value_eval_4na_16.413
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.416-..___tag_value_eval_4na_16.415
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.418-..___tag_value_eval_4na_16.416
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.419-..___tag_value_eval_4na_16.418
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.421-..___tag_value_eval_4na_16.419
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.422-..___tag_value_eval_4na_16.421
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.424-..___tag_value_eval_4na_16.422
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.4byte 0x0000006c
	.4byte 0x000007fc
	.4byte ..___tag_value_eval_4na_32.428-.
	.4byte ..___tag_value_eval_4na_32.449-..___tag_value_eval_4na_32.428
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.429-..___tag_value_eval_4na_32.428
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.433-..___tag_value_eval_4na_32.429
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.434-..___tag_value_eval_4na_32.433
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.436-..___tag_value_eval_4na_32.434
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.437-..___tag_value_eval_4na_32.436
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.439-..___tag_value_eval_4na_32.437
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.440-..___tag_value_eval_4na_32.439
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.442-..___tag_value_eval_4na_32.440
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.443-..___tag_value_eval_4na_32.442
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.445-..___tag_value_eval_4na_32.443
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.446-..___tag_value_eval_4na_32.445
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.448-..___tag_value_eval_4na_32.446
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.4byte 0x0000006c
	.4byte 0x0000086c
	.4byte ..___tag_value_eval_4na_64.452-.
	.4byte ..___tag_value_eval_4na_64.473-..___tag_value_eval_4na_64.452
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.453-..___tag_value_eval_4na_64.452
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.457-..___tag_value_eval_4na_64.453
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.458-..___tag_value_eval_4na_64.457
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.460-..___tag_value_eval_4na_64.458
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.461-..___tag_value_eval_4na_64.460
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.463-..___tag_value_eval_4na_64.461
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.464-..___tag_value_eval_4na_64.463
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.466-..___tag_value_eval_4na_64.464
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.467-..___tag_value_eval_4na_64.466
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.469-..___tag_value_eval_4na_64.467
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.470-..___tag_value_eval_4na_64.469
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.472-..___tag_value_eval_4na_64.470
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.4byte 0x0000007c
	.4byte 0x000008dc
	.4byte ..___tag_value_eval_4na_128.476-.
	.4byte ..___tag_value_eval_4na_128.500-..___tag_value_eval_4na_128.476
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.477-..___tag_value_eval_4na_128.476
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.481-..___tag_value_eval_4na_128.477
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.482-..___tag_value_eval_4na_128.481
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.484-..___tag_value_eval_4na_128.482
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.485-..___tag_value_eval_4na_128.484
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.487-..___tag_value_eval_4na_128.485
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.488-..___tag_value_eval_4na_128.487
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.490-..___tag_value_eval_4na_128.488
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.491-..___tag_value_eval_4na_128.490
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.493-..___tag_value_eval_4na_128.491
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.494-..___tag_value_eval_4na_128.493
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.496-..___tag_value_eval_4na_128.494
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.497-..___tag_value_eval_4na_128.496
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.499-..___tag_value_eval_4na_128.497
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.byte 0x00
	.4byte 0x000000d4
	.4byte 0x0000095c
	.4byte ..___tag_value_eval_4na_pos.503-.
	.4byte ..___tag_value_eval_4na_pos.545-..___tag_value_eval_4na_pos.503
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.504-..___tag_value_eval_4na_pos.503
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.508-..___tag_value_eval_4na_pos.504
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.509-..___tag_value_eval_4na_pos.508
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.511-..___tag_value_eval_4na_pos.509
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.512-..___tag_value_eval_4na_pos.511
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.514-..___tag_value_eval_4na_pos.512
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.515-..___tag_value_eval_4na_pos.514
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.517-..___tag_value_eval_4na_pos.515
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.518-..___tag_value_eval_4na_pos.517
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.520-..___tag_value_eval_4na_pos.518
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.521-..___tag_value_eval_4na_pos.520
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.523-..___tag_value_eval_4na_pos.521
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.524-..___tag_value_eval_4na_pos.523
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.526-..___tag_value_eval_4na_pos.524
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.527-..___tag_value_eval_4na_pos.526
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.529-..___tag_value_eval_4na_pos.527
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.530-..___tag_value_eval_4na_pos.529
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.532-..___tag_value_eval_4na_pos.530
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.533-..___tag_value_eval_4na_pos.532
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.535-..___tag_value_eval_4na_pos.533
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.536-..___tag_value_eval_4na_pos.535
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.538-..___tag_value_eval_4na_pos.536
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.539-..___tag_value_eval_4na_pos.538
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.541-..___tag_value_eval_4na_pos.539
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.542-..___tag_value_eval_4na_pos.541
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.544-..___tag_value_eval_4na_pos.542
	.byte 0x09
	.2byte 0x0606
	.byte 0x00
	.4byte 0x0000033c
	.4byte 0x00000a34
	.4byte ..___tag_value_NucStrstrSearch.548-.
	.4byte ..___tag_value_NucStrstrSearch.711-..___tag_value_NucStrstrSearch.548
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.549-..___tag_value_NucStrstrSearch.548
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.552-..___tag_value_NucStrstrSearch.549
	.byte 0x83
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.553-..___tag_value_NucStrstrSearch.552
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.555-..___tag_value_NucStrstrSearch.553
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.556-..___tag_value_NucStrstrSearch.555
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.558-..___tag_value_NucStrstrSearch.556
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.560-..___tag_value_NucStrstrSearch.558
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.561-..___tag_value_NucStrstrSearch.560
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.563-..___tag_value_NucStrstrSearch.561
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.565-..___tag_value_NucStrstrSearch.563
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.566-..___tag_value_NucStrstrSearch.565
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.568-..___tag_value_NucStrstrSearch.566
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.570-..___tag_value_NucStrstrSearch.568
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.571-..___tag_value_NucStrstrSearch.570
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.573-..___tag_value_NucStrstrSearch.571
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.575-..___tag_value_NucStrstrSearch.573
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.576-..___tag_value_NucStrstrSearch.575
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.578-..___tag_value_NucStrstrSearch.576
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.580-..___tag_value_NucStrstrSearch.578
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.581-..___tag_value_NucStrstrSearch.580
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.583-..___tag_value_NucStrstrSearch.581
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.585-..___tag_value_NucStrstrSearch.583
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.586-..___tag_value_NucStrstrSearch.585
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.588-..___tag_value_NucStrstrSearch.586
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.590-..___tag_value_NucStrstrSearch.588
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.591-..___tag_value_NucStrstrSearch.590
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.593-..___tag_value_NucStrstrSearch.591
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.595-..___tag_value_NucStrstrSearch.593
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.596-..___tag_value_NucStrstrSearch.595
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.598-..___tag_value_NucStrstrSearch.596
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.600-..___tag_value_NucStrstrSearch.598
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.601-..___tag_value_NucStrstrSearch.600
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.603-..___tag_value_NucStrstrSearch.601
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.605-..___tag_value_NucStrstrSearch.603
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.606-..___tag_value_NucStrstrSearch.605
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.608-..___tag_value_NucStrstrSearch.606
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.610-..___tag_value_NucStrstrSearch.608
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.611-..___tag_value_NucStrstrSearch.610
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.613-..___tag_value_NucStrstrSearch.611
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.615-..___tag_value_NucStrstrSearch.613
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.616-..___tag_value_NucStrstrSearch.615
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.618-..___tag_value_NucStrstrSearch.616
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.620-..___tag_value_NucStrstrSearch.618
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.621-..___tag_value_NucStrstrSearch.620
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.623-..___tag_value_NucStrstrSearch.621
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.625-..___tag_value_NucStrstrSearch.623
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.626-..___tag_value_NucStrstrSearch.625
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.628-..___tag_value_NucStrstrSearch.626
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.630-..___tag_value_NucStrstrSearch.628
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.631-..___tag_value_NucStrstrSearch.630
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.633-..___tag_value_NucStrstrSearch.631
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.635-..___tag_value_NucStrstrSearch.633
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.636-..___tag_value_NucStrstrSearch.635
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.638-..___tag_value_NucStrstrSearch.636
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.640-..___tag_value_NucStrstrSearch.638
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.641-..___tag_value_NucStrstrSearch.640
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.643-..___tag_value_NucStrstrSearch.641
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.645-..___tag_value_NucStrstrSearch.643
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.646-..___tag_value_NucStrstrSearch.645
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.648-..___tag_value_NucStrstrSearch.646
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.650-..___tag_value_NucStrstrSearch.648
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.651-..___tag_value_NucStrstrSearch.650
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.653-..___tag_value_NucStrstrSearch.651
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.655-..___tag_value_NucStrstrSearch.653
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.656-..___tag_value_NucStrstrSearch.655
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.658-..___tag_value_NucStrstrSearch.656
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.660-..___tag_value_NucStrstrSearch.658
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.661-..___tag_value_NucStrstrSearch.660
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.663-..___tag_value_NucStrstrSearch.661
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.665-..___tag_value_NucStrstrSearch.663
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.666-..___tag_value_NucStrstrSearch.665
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.668-..___tag_value_NucStrstrSearch.666
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.670-..___tag_value_NucStrstrSearch.668
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.671-..___tag_value_NucStrstrSearch.670
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.673-..___tag_value_NucStrstrSearch.671
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.675-..___tag_value_NucStrstrSearch.673
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.676-..___tag_value_NucStrstrSearch.675
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.678-..___tag_value_NucStrstrSearch.676
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.680-..___tag_value_NucStrstrSearch.678
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.681-..___tag_value_NucStrstrSearch.680
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.683-..___tag_value_NucStrstrSearch.681
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.685-..___tag_value_NucStrstrSearch.683
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.686-..___tag_value_NucStrstrSearch.685
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.688-..___tag_value_NucStrstrSearch.686
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.690-..___tag_value_NucStrstrSearch.688
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.691-..___tag_value_NucStrstrSearch.690
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.693-..___tag_value_NucStrstrSearch.691
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.695-..___tag_value_NucStrstrSearch.693
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.696-..___tag_value_NucStrstrSearch.695
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.698-..___tag_value_NucStrstrSearch.696
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.700-..___tag_value_NucStrstrSearch.698
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.701-..___tag_value_NucStrstrSearch.700
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.703-..___tag_value_NucStrstrSearch.701
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.705-..___tag_value_NucStrstrSearch.703
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.706-..___tag_value_NucStrstrSearch.705
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.708-..___tag_value_NucStrstrSearch.706
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.710-..___tag_value_NucStrstrSearch.708
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.section .text
.LNDBG_TXe:
# End
