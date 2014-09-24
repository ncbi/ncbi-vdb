	.section .text
.LNDBG_TX:
# -- Machine type IA32
# mark_description "Intel(R) C++ Compiler for applications running on IA-32, Version 10.1    Build 20081024 %s";
# mark_description "-I/home/yaschenk/devel/internal/asm-trace/sra/search/linux -I/home/yaschenk/devel/internal/asm-trace/sra/sea";
# mark_description "rch/unix -I/home/yaschenk/devel/internal/asm-trace/sra/search -I/home/yaschenk/devel/internal/asm-trace/itf ";
# mark_description "-I/home/yaschenk/devel/internal/asm-trace/inc/icc/i386 -I/home/yaschenk/devel/internal/asm-trace/inc/icc -I/";
# mark_description "home/yaschenk/devel/internal/asm-trace/inc/gcc/i386 -I/home/yaschenk/devel/internal/asm-trace/inc/gcc -I/hom";
# mark_description "e/yaschenk/devel/internal/asm-trace/inc/linux -I/home/yaschenk/devel/internal/asm-trace/inc/unix -I/home/rod";
# mark_description "armer/devel/internal/asm-trace/inc -c -S -o nucstrstr.pic.s -fPIC -g -Wall -D_DEBUGGING -DLINUX -DUNIX -D_GN";
# mark_description "U_SOURCE -D_REENTRANT -D_FILE_OFFSET_BITS=64 -MD";
	.file "nucstrstr.c"
	.data
	.text
..TXTST0:
# -- Begin  uint16_lsbit
# mark_begin;
       .align    2,0x90
uint16_lsbit:
# parameter 1(self): 8 + %ebp
..B1.1:                         # Preds ..B1.0
..___tag_value_uint16_lsbit.2:                                  #
..LN1:
        pushl     %ebp                                          #38.1
        movl      %esp, %ebp                                    #38.1
..___tag_value_uint16_lsbit.7:                                  #
        pushl     %esi                                          #38.1
..LN3:
        movzwl    8(%ebp), %eax                                 #40.0
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #40.0
        movw      %ax, -4(%ebp)                                 #40.0
..LN5:
        movswl    -4(%ebp), %eax                                #49.12
        leave                                                   #49.12
..___tag_value_uint16_lsbit.11:                                 #
        ret                                                     #49.12
        .align    2,0x90
..___tag_value_uint16_lsbit.12:                                 #
                                # LOE
# mark_end;
	.type	uint16_lsbit,@function
	.size	uint16_lsbit,.-uint16_lsbit
.LNuint16_lsbit:
	.data
# -- End  uint16_lsbit
	.data
	.text
# -- Begin  uint128_shr
# mark_begin;
       .align    2,0x90
uint128_shr:
# parameter 1(self): 8 + %ebp
# parameter 2(i): 12 + %ebp
..B2.1:                         # Preds ..B2.0
..___tag_value_uint128_shr.15:                                  #
..LN7:
        pushl     %ebp                                          #295.1
        movl      %esp, %ebp                                    #295.1
..___tag_value_uint128_shr.20:                                  #
..LN9:
        movl      8(%ebp), %edx                                 #296.0
        movl      12(%ebp), %ecx                                #296.0
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #296.0
..LN11:
        leave                                                   #310.1
..___tag_value_uint128_shr.24:                                  #
        ret                                                     #310.1
        .align    2,0x90
..___tag_value_uint128_shr.25:                                  #
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
# parameter 1(self): 8 + %ebp
..B3.1:                         # Preds ..B3.0
..___tag_value_uint128_bswap.28:                                #
..LN13:
        pushl     %ebp                                          #333.1
        movl      %esp, %ebp                                    #333.1
..___tag_value_uint128_bswap.33:                                #
..LN15:
        movl      8(%ebp), %ecx                                 #334.0
# Begin ASM
        mov (%ecx), %eax;mov 12(%ecx), %edx;bswap %eax;bswap %edx;mov %eax, 12(%ecx);mov %edx, (%ecx);mov 4(%ecx), %eax;mov 8(%ecx), %edx;bswap %eax;bswap %edx;mov %eax, 8(%ecx);mov %edx, 4(%ecx);
# End ASM                                                       #334.0
..LN17:
        leave                                                   #352.1
..___tag_value_uint128_bswap.37:                                #
        ret                                                     #352.1
        .align    2,0x90
..___tag_value_uint128_bswap.38:                                #
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
# parameter 1(to): 8 + %ebp
# parameter 2(from): 12 + %ebp
..B4.1:                         # Preds ..B4.0
..___tag_value_uint128_bswap_copy.41:                           #
..LN19:
        pushl     %ebp                                          #356.1
        movl      %esp, %ebp                                    #356.1
..___tag_value_uint128_bswap_copy.46:                           #
..LN21:
        movl      8(%ebp), %ecx                                 #357.0
        movl      12(%ebp), %edx                                #357.0
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #357.0
..LN23:
        leave                                                   #377.1
..___tag_value_uint128_bswap_copy.50:                           #
        ret                                                     #377.1
        .align    2,0x90
..___tag_value_uint128_bswap_copy.51:                           #
                                # LOE
# mark_end;
	.type	uint128_bswap_copy,@function
	.size	uint128_bswap_copy,.-uint128_bswap_copy
.LNuint128_bswap_copy:
	.data
# -- End  uint128_bswap_copy
	.data
	.text
# -- Begin  prime_buffer_2na
# mark_begin;
       .align    2,0x90
prime_buffer_2na:
# parameter 1(src): 8 + %ebx
# parameter 2(ignore): 12 + %ebx
..B5.1:                         # Preds ..B5.0
..___tag_value_prime_buffer_2na.54:                             #
..LN25:
        pushl     %ebx                                          #2129.1
..___tag_value_prime_buffer_2na.59:                             #
        movl      %esp, %ebx                                    #2129.1
..___tag_value_prime_buffer_2na.60:                             #
        andl      $-16, %esp                                    #2129.1
        pushl     %ebp                                          #2129.1
        pushl     %ebp                                          #2129.1
        movl      4(%ebx), %ebp                                 #2129.1
        movl      %ebp, 4(%esp)                                 #2129.1
        movl      %esp, %ebp                                    #2129.1
..___tag_value_prime_buffer_2na.62:                             #
        subl      $24, %esp                                     #2129.1
..LN27:
        movl      8(%ebx), %eax                                 #2132.23
..LN29:
        testb     $15, %al                                      #2132.29
..LN31:
        jne       ..B5.3        # Prob 50%                      #2132.37
                                # LOE
..B5.2:                         # Preds ..B5.1
..LN33:
        movl      8(%ebx), %eax                                 #2133.54
        movdqa    (%eax), %xmm0                                 #2133.54
..LN35:
        movdqa    %xmm0, -24(%ebp)                              #2133.9
        jmp       ..B5.4        # Prob 100%                     #2133.9
                                # LOE
..B5.3:                         # Preds ..B5.1
..LN37:
        movl      8(%ebx), %eax                                 #2135.55
        movdqu    (%eax), %xmm0                                 #2135.55
..LN39:
        movdqa    %xmm0, -24(%ebp)                              #2135.9
                                # LOE
..B5.4:                         # Preds ..B5.2 ..B5.3
..LN41:
        movdqa    -24(%ebp), %xmm0                              #2136.12
        leave                                                   #2136.12
..___tag_value_prime_buffer_2na.65:                             #
        movl      %ebx, %esp                                    #2136.12
        popl      %ebx                                          #2136.12
..___tag_value_prime_buffer_2na.66:                             #
        ret                                                     #2136.12
        .align    2,0x90
..___tag_value_prime_buffer_2na.68:                             #
                                # LOE
# mark_end;
	.type	prime_buffer_2na,@function
	.size	prime_buffer_2na,.-prime_buffer_2na
.LNprime_buffer_2na:
	.data
# -- End  prime_buffer_2na
	.data
	.align 32
	.align 32
expand_2na:
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
	.type	expand_2na,@object
	.size	expand_2na,512
	.data
	.text
# -- Begin  prime_buffer_4na
# mark_begin;
       .align    2,0x90
prime_buffer_4na:
# parameter 1(src): 8 + %ebx
# parameter 2(ignore): 12 + %ebx
..B6.1:                         # Preds ..B6.0
..___tag_value_prime_buffer_4na.71:                             #
..LN43:
        pushl     %ebx                                          #3990.1
..___tag_value_prime_buffer_4na.76:                             #
        movl      %esp, %ebx                                    #3990.1
..___tag_value_prime_buffer_4na.77:                             #
        andl      $-16, %esp                                    #3990.1
        pushl     %ebp                                          #3990.1
        pushl     %ebp                                          #3990.1
        movl      4(%ebx), %ebp                                 #3990.1
        movl      %ebp, 4(%esp)                                 #3990.1
        movl      %esp, %ebp                                    #3990.1
..___tag_value_prime_buffer_4na.79:                             #
        subl      $40, %esp                                     #3990.1
        call      ..L81         # Prob 100%                     #3990.1
..L81:                                                          #
        popl      %eax                                          #3990.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L81], %eax      #3990.1
..LN45:
        movl      8(%ebx), %edx                                 #3996.34
        movzbl    (%edx), %edx                                  #3996.34
        movzbl    %dl, %edx                                     #3996.34
..LN47:
        movzwl    expand_2na@GOTOFF(%eax,%edx,2), %edx          #3996.21
..LN49:
        movw      %dx, -40(%ebp)                                #3996.5
..LN51:
        movl      8(%ebx), %edx                                 #3997.34
        movzbl    1(%edx), %edx                                 #3997.34
        movzbl    %dl, %edx                                     #3997.34
..LN53:
        movzwl    expand_2na@GOTOFF(%eax,%edx,2), %edx          #3997.21
..LN55:
        movw      %dx, -38(%ebp)                                #3997.5
..LN57:
        movl      8(%ebx), %edx                                 #3998.34
        movzbl    2(%edx), %edx                                 #3998.34
        movzbl    %dl, %edx                                     #3998.34
..LN59:
        movzwl    expand_2na@GOTOFF(%eax,%edx,2), %edx          #3998.21
..LN61:
        movw      %dx, -36(%ebp)                                #3998.5
..LN63:
        movl      8(%ebx), %edx                                 #3999.34
        movzbl    3(%edx), %edx                                 #3999.34
        movzbl    %dl, %edx                                     #3999.34
..LN65:
        movzwl    expand_2na@GOTOFF(%eax,%edx,2), %edx          #3999.21
..LN67:
        movw      %dx, -34(%ebp)                                #3999.5
..LN69:
        movl      8(%ebx), %edx                                 #4000.34
        movzbl    4(%edx), %edx                                 #4000.34
        movzbl    %dl, %edx                                     #4000.34
..LN71:
        movzwl    expand_2na@GOTOFF(%eax,%edx,2), %edx          #4000.21
..LN73:
        movw      %dx, -32(%ebp)                                #4000.5
..LN75:
        movl      8(%ebx), %edx                                 #4001.34
        movzbl    5(%edx), %edx                                 #4001.34
        movzbl    %dl, %edx                                     #4001.34
..LN77:
        movzwl    expand_2na@GOTOFF(%eax,%edx,2), %edx          #4001.21
..LN79:
        movw      %dx, -30(%ebp)                                #4001.5
..LN81:
        movl      8(%ebx), %edx                                 #4002.34
        movzbl    6(%edx), %edx                                 #4002.34
        movzbl    %dl, %edx                                     #4002.34
..LN83:
        movzwl    expand_2na@GOTOFF(%eax,%edx,2), %edx          #4002.21
..LN85:
        movw      %dx, -28(%ebp)                                #4002.5
..LN87:
        movl      8(%ebx), %edx                                 #4003.34
        movzbl    7(%edx), %edx                                 #4003.34
        movzbl    %dl, %edx                                     #4003.34
..LN89:
        movzwl    expand_2na@GOTOFF(%eax,%edx,2), %eax          #4003.21
..LN91:
        movw      %ax, -26(%ebp)                                #4003.5
..LN93:
        movdqu    -40(%ebp), %xmm0                              #4005.51
..LN95:
        movdqa    %xmm0, -24(%ebp)                              #4005.5
..LN97:
        movdqa    -24(%ebp), %xmm0                              #4006.12
        leave                                                   #4006.12
..___tag_value_prime_buffer_4na.83:                             #
        movl      %ebx, %esp                                    #4006.12
        popl      %ebx                                          #4006.12
..___tag_value_prime_buffer_4na.84:                             #
        ret                                                     #4006.12
        .align    2,0x90
..___tag_value_prime_buffer_4na.86:                             #
                                # LOE
# mark_end;
	.type	prime_buffer_4na,@function
	.size	prime_buffer_4na,.-prime_buffer_4na
.LNprime_buffer_4na:
	.data
# -- End  prime_buffer_4na
	.section .rodata, "a"
	.align 4
	.align 4
__$U0:
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
	.type	__$U0,@object
	.size	__$U0,21
	.space 3	# pad
_2__STRING.1:
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
	.type	_2__STRING.1,@object
	.size	_2__STRING.1,12
_2__STRING.0:
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
	.type	_2__STRING.0,@object
	.size	_2__STRING.0,54
	.space 2	# pad
_2__STRING.2:
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
	.type	_2__STRING.2,@object
	.size	_2__STRING.2,51
	.data
	.text
# -- Begin  NucStrFastaExprAlloc
# mark_begin;
       .align    2,0x90
NucStrFastaExprAlloc:
# parameter 1(sz): 8 + %ebp
..B7.1:                         # Preds ..B7.0
..___tag_value_NucStrFastaExprAlloc.89:                         #
..LN99:
        pushl     %ebp                                          #790.1
        movl      %esp, %ebp                                    #790.1
..___tag_value_NucStrFastaExprAlloc.94:                         #
        subl      $24, %esp                                     #790.1
        movl      %ebx, -4(%ebp)                                #790.1
..___tag_value_NucStrFastaExprAlloc.97:                         #
        call      ..L98         # Prob 100%                     #790.1
..L98:                                                          #
        popl      %eax                                          #790.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L98], %eax      #790.1
        movl      %eax, -12(%ebp)                               #790.1
..LN101:
        addl      $0, %esp                                      #791.19
..LN103:
        movl      8(%ebp), %edx                                 #791.28
..LN105:
        addl      $16, %edx                                     #791.33
        movl      %edx, (%esp)                                  #791.33
..LN107:
        movl      %eax, %ebx                                    #791.19
        call      malloc@PLT                                    #791.19
                                # LOE eax
..B7.11:                        # Preds ..B7.1
        popl      %ecx                                          #791.19
        movl      %eax, -8(%ebp)                                #791.19
                                # LOE
..B7.2:                         # Preds ..B7.11
..LN109:
        movl      -8(%ebp), %eax                                #791.17
        movl      %eax, -16(%ebp)                               #791.17
..LN111:
        movl      -16(%ebp), %eax                               #792.10
..LN113:
        testl     %eax, %eax                                    #792.19
        je        ..B7.8        # Prob 50%                      #792.19
                                # LOE
..B7.3:                         # Preds ..B7.2
..LN115:
        movl      -16(%ebp), %eax                               #795.28
..LN117:
        addl      $15, %eax                                     #795.36
..LN119:
        andl      $-16, %eax                                    #795.43
..LN121:
        movl      %eax, -20(%ebp)                               #794.28
..LN123:
        movl      -20(%ebp), %eax                               #796.9
..LN125:
        movl      -16(%ebp), %edx                               #796.26
..LN127:
        movl      %edx, 8(%eax)                                 #796.9
..LN129:
        movl      -20(%ebp), %eax                               #797.9
        testb     $15, %al                                      #797.9
        je        ..B7.5        # Prob 50%                      #797.9
..___tag_value_NucStrFastaExprAlloc.99:                         #
                                # LOE
..B7.4:                         # Preds ..B7.3
        addl      $-16, %esp                                    #797.9
        movl      -12(%ebp), %eax                               #797.9
        lea       _2__STRING.0@GOTOFF(%eax), %eax               #797.9
        movl      %eax, (%esp)                                  #797.9
        movl      -12(%ebp), %eax                               #797.9
        lea       _2__STRING.1@GOTOFF(%eax), %eax               #797.9
        movl      %eax, 4(%esp)                                 #797.9
        movl      $797, 8(%esp)                                 #797.9
        movl      -12(%ebp), %eax                               #797.9
        lea       __$U0@GOTOFF(%eax), %eax                      #797.9
        movl      %eax, 12(%esp)                                #797.9
        movl      -12(%ebp), %eax                               #797.9
        movl      %eax, %ebx                                    #797.9
        call      __assert_fail@PLT                             #797.9
                                # LOE
..B7.12:                        # Preds ..B7.4
        addl      $16, %esp                                     #797.9
..___tag_value_NucStrFastaExprAlloc.100:                        #
                                # LOE
..B7.5:                         # Preds ..B7.3
..LN131:
        movl      -20(%ebp), %eax                               #798.9
        testb     $15, %al                                      #798.9
        je        ..B7.7        # Prob 50%                      #798.9
..___tag_value_NucStrFastaExprAlloc.101:                        #
                                # LOE
..B7.6:                         # Preds ..B7.5
        addl      $-16, %esp                                    #798.9
        movl      -12(%ebp), %eax                               #798.9
        lea       _2__STRING.2@GOTOFF(%eax), %eax               #798.9
        movl      %eax, (%esp)                                  #798.9
        movl      -12(%ebp), %eax                               #798.9
        lea       _2__STRING.1@GOTOFF(%eax), %eax               #798.9
        movl      %eax, 4(%esp)                                 #798.9
        movl      $798, 8(%esp)                                 #798.9
        movl      -12(%ebp), %eax                               #798.9
        lea       __$U0@GOTOFF(%eax), %eax                      #798.9
        movl      %eax, 12(%esp)                                #798.9
        movl      -12(%ebp), %eax                               #798.9
        movl      %eax, %ebx                                    #798.9
        call      __assert_fail@PLT                             #798.9
                                # LOE
..B7.13:                        # Preds ..B7.6
        addl      $16, %esp                                     #798.9
..___tag_value_NucStrFastaExprAlloc.102:                        #
                                # LOE
..B7.7:                         # Preds ..B7.5
..LN133:
        movl      -20(%ebp), %eax                               #799.16
        movl      -4(%ebp), %ebx                                #799.16
..___tag_value_NucStrFastaExprAlloc.103:                        #
        leave                                                   #799.16
..___tag_value_NucStrFastaExprAlloc.105:                        #
        ret                                                     #799.16
..___tag_value_NucStrFastaExprAlloc.106:                        #
                                # LOE
..B7.8:                         # Preds ..B7.2
..LN135:
        xorl      %eax, %eax                                    #801.12
        movl      -4(%ebp), %ebx                                #801.12
..___tag_value_NucStrFastaExprAlloc.108:                        #
        leave                                                   #801.12
..___tag_value_NucStrFastaExprAlloc.110:                        #
        ret                                                     #801.12
        .align    2,0x90
..___tag_value_NucStrFastaExprAlloc.111:                        #
                                # LOE
# mark_end;
	.type	NucStrFastaExprAlloc,@function
	.size	NucStrFastaExprAlloc,.-NucStrFastaExprAlloc
.LNNucStrFastaExprAlloc:
	.data
# -- End  NucStrFastaExprAlloc
	.bss
	.align 32
	.align 32
fasta_2na_map:
	.type	fasta_2na_map,@object
	.size	fasta_2na_map,128
	.space 128	# pad
	.data
	.text
# -- Begin  NucStrFastaExprMake2
# mark_begin;
       .align    2,0x90
NucStrFastaExprMake2:
# parameter 1(expr): 8 + %ebp
# parameter 2(positional): 12 + %ebp
# parameter 3(fasta): 16 + %ebp
# parameter 4(size): 20 + %ebp
..B8.1:                         # Preds ..B8.0
..___tag_value_NucStrFastaExprMake2.114:                        #
..LN137:
        pushl     %ebp                                          #838.1
        movl      %esp, %ebp                                    #838.1
..___tag_value_NucStrFastaExprMake2.119:                        #
        subl      $60, %esp                                     #838.1
        movl      %ebx, -4(%ebp)                                #838.1
..___tag_value_NucStrFastaExprMake2.122:                        #
..LN139:
        movl      20(%ebp), %eax                                #844.10
..LN141:
        cmpl      $61, %eax                                     #844.17
        jbe       ..B8.3        # Prob 50%                      #844.17
                                # LOE
..B8.2:                         # Preds ..B8.1
..LN143:
        movl      $7, %eax                                      #845.16
        movl      -4(%ebp), %ebx                                #845.16
..___tag_value_NucStrFastaExprMake2.123:                        #
        leave                                                   #845.16
..___tag_value_NucStrFastaExprMake2.125:                        #
        ret                                                     #845.16
..___tag_value_NucStrFastaExprMake2.126:                        #
                                # LOE
..B8.3:                         # Preds ..B8.1
..LN145:
        pushl     %edi                                          #847.9
..LN147:
        movl      $144, (%esp)                                  #847.32
..LN149:
        call      NucStrFastaExprAlloc                          #847.9
                                # LOE eax
..B8.48:                        # Preds ..B8.3
        popl      %ecx                                          #847.9
        movl      %eax, -16(%ebp)                               #847.9
                                # LOE
..B8.4:                         # Preds ..B8.48
        call      ..L128        # Prob 100%                     #
..L128:                                                         #
        popl      %eax                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L128], %eax     #
        movl      %eax, -12(%ebp)                               #
..LN151:
        movl      -16(%ebp), %eax                               #847.5
        movl      %eax, -20(%ebp)                               #847.5
..LN153:
        movl      -20(%ebp), %eax                               #848.10
..LN155:
        testl     %eax, %eax                                    #848.15
        jne       ..B8.6        # Prob 50%                      #848.15
                                # LOE
..B8.5:                         # Preds ..B8.4
..LN157:
        movl      -12(%ebp), %eax                               #849.16
        movl      %eax, %ebx                                    #849.16
        call      __errno_location@PLT                          #849.16
                                # LOE eax
..B8.49:                        # Preds ..B8.5
        movl      (%eax), %eax                                  #849.16
        movl      -4(%ebp), %ebx                                #849.16
..___tag_value_NucStrFastaExprMake2.129:                        #
        leave                                                   #849.16
..___tag_value_NucStrFastaExprMake2.131:                        #
        ret                                                     #849.16
..___tag_value_NucStrFastaExprMake2.132:                        #
                                # LOE
..B8.6:                         # Preds ..B8.4
..LN159:
        movl      8(%ebp), %eax                                 #851.5
..LN161:
        movl      -20(%ebp), %edx                               #851.14
..LN163:
        movl      %edx, (%eax)                                  #851.5
..LN165:
        movl      -20(%ebp), %eax                               #852.5
..LN167:
        movl      20(%ebp), %edx                                #852.25
..LN169:
        movl      %edx, 4(%eax)                                 #852.5
..LN171:
        movl      $0, -28(%ebp)                                 #856.11
..LN173:
        movl      -28(%ebp), %eax                               #856.18
..LN175:
        movl      20(%ebp), %edx                                #856.22
        cmpl      %edx, %eax                                    #856.22
        jb        ..B8.9        # Prob 50%                      #856.22
        jmp       ..B8.17       # Prob 100%                     #856.22
                                # LOE
..B8.7:                         # Preds ..B8.13 ..B8.14 ..B8.15 ..B8.16 ..B8.12
                                #      
..LN177:
        incl      -28(%ebp)                                     #856.31
..LN179:
        movl      -28(%ebp), %eax                               #856.18
..LN181:
        movl      20(%ebp), %edx                                #856.22
        cmpl      %edx, %eax                                    #856.22
        jae       ..B8.17       # Prob 50%                      #856.22
                                # LOE
..B8.9:                         # Preds ..B8.6 ..B8.7
..LN183:
        movl      16(%ebp), %eax                                #858.48
..LN185:
        movl      -28(%ebp), %edx                               #858.56
..LN187:
        movzbl    (%edx,%eax), %eax                             #858.48
        movsbl    %al, %eax                                     #858.48
..LN189:
        movl      -12(%ebp), %edx                               #858.24
        movzbl    fasta_2na_map@GOTOFF(%eax,%edx), %eax         #858.24
..LN191:
        movb      %al, -8(%ebp)                                 #858.22
..LN193:
        movl      -28(%ebp), %eax                               #859.18
..LN195:
        andl      $3, %eax                                      #859.22
        movl      %eax, -24(%ebp)                               #859.22
        je        ..B8.13       # Prob 50%                      #859.22
                                # LOE
..B8.10:                        # Preds ..B8.9
        movl      -24(%ebp), %eax                               #859.22
        cmpl      $1, %eax                                      #859.22
        je        ..B8.14       # Prob 50%                      #859.22
                                # LOE
..B8.11:                        # Preds ..B8.10
        movl      -24(%ebp), %eax                               #859.22
        cmpl      $2, %eax                                      #859.22
        je        ..B8.15       # Prob 50%                      #859.22
                                # LOE
..B8.12:                        # Preds ..B8.11
        movl      -24(%ebp), %eax                               #859.22
        cmpl      $3, %eax                                      #859.22
        je        ..B8.16       # Prob 50%                      #859.22
        jmp       ..B8.7        # Prob 100%                     #859.22
                                # LOE
..B8.13:                        # Preds ..B8.9
..LN197:
        movzbl    -8(%ebp), %eax                                #862.38
..LN199:
        shll      $6, %eax                                      #862.46
..LN201:
        movl      -28(%ebp), %edx                               #862.27
..LN203:
        shrl      $2, %edx                                      #862.32
..LN205:
        movb      %al, -60(%ebp,%edx)                           #862.13
..LN207:
        movl      -28(%ebp), %eax                               #863.24
..LN209:
        shrl      $2, %eax                                      #863.29
..LN211:
        movb      $192, -44(%ebp,%eax)                          #863.13
        jmp       ..B8.7        # Prob 100%                     #863.13
                                # LOE
..B8.14:                        # Preds ..B8.10
..LN213:
        movl      -28(%ebp), %eax                               #866.27
..LN215:
        shrl      $2, %eax                                      #866.32
..LN217:
        movzbl    -60(%ebp,%eax), %eax                          #866.13
..LN219:
        movzbl    -8(%ebp), %edx                                #866.39
..LN221:
        shll      $4, %edx                                      #866.47
..LN223:
        orl       %edx, %eax                                    #866.13
..LN225:
        movl      -28(%ebp), %edx                               #866.27
..LN227:
        shrl      $2, %edx                                      #866.32
..LN229:
        movb      %al, -60(%ebp,%edx)                           #866.13
..LN231:
        movl      -28(%ebp), %eax                               #867.24
..LN233:
        shrl      $2, %eax                                      #867.29
..LN235:
        movzbl    -44(%ebp,%eax), %eax                          #867.13
        orl       $48, %eax                                     #867.13
..LN237:
        movl      -28(%ebp), %edx                               #867.24
..LN239:
        shrl      $2, %edx                                      #867.29
..LN241:
        movb      %al, -44(%ebp,%edx)                           #867.13
        jmp       ..B8.7        # Prob 100%                     #867.13
                                # LOE
..B8.15:                        # Preds ..B8.11
..LN243:
        movl      -28(%ebp), %eax                               #870.27
..LN245:
        shrl      $2, %eax                                      #870.32
..LN247:
        movzbl    -60(%ebp,%eax), %eax                          #870.13
..LN249:
        movzbl    -8(%ebp), %edx                                #870.39
..LN251:
        shll      $2, %edx                                      #870.47
..LN253:
        orl       %edx, %eax                                    #870.13
..LN255:
        movl      -28(%ebp), %edx                               #870.27
..LN257:
        shrl      $2, %edx                                      #870.32
..LN259:
        movb      %al, -60(%ebp,%edx)                           #870.13
..LN261:
        movl      -28(%ebp), %eax                               #871.24
..LN263:
        shrl      $2, %eax                                      #871.29
..LN265:
        movzbl    -44(%ebp,%eax), %eax                          #871.13
        orl       $12, %eax                                     #871.13
..LN267:
        movl      -28(%ebp), %edx                               #871.24
..LN269:
        shrl      $2, %edx                                      #871.29
..LN271:
        movb      %al, -44(%ebp,%edx)                           #871.13
        jmp       ..B8.7        # Prob 100%                     #871.13
                                # LOE
..B8.16:                        # Preds ..B8.12
..LN273:
        movl      -28(%ebp), %eax                               #874.27
..LN275:
        shrl      $2, %eax                                      #874.32
..LN277:
        movzbl    -60(%ebp,%eax), %eax                          #874.13
..LN279:
        movzbl    -8(%ebp), %edx                                #874.39
..LN281:
        orl       %edx, %eax                                    #874.13
..LN283:
        movl      -28(%ebp), %edx                               #874.27
..LN285:
        shrl      $2, %edx                                      #874.32
..LN287:
        movb      %al, -60(%ebp,%edx)                           #874.13
..LN289:
        movl      -28(%ebp), %eax                               #875.24
..LN291:
        shrl      $2, %eax                                      #875.29
..LN293:
        movzbl    -44(%ebp,%eax), %eax                          #875.13
        orl       $3, %eax                                      #875.13
..LN295:
        movl      -28(%ebp), %edx                               #875.24
..LN297:
        shrl      $2, %edx                                      #875.29
..LN299:
        movb      %al, -44(%ebp,%edx)                           #875.13
        jmp       ..B8.7        # Prob 100%                     #875.13
                                # LOE
..B8.17:                        # Preds ..B8.7 ..B8.6
..LN301:
        movl      -28(%ebp), %eax                               #881.17
..LN303:
        addl      $3, %eax                                      #881.21
..LN305:
        shrl      $2, %eax                                      #881.28
..LN307:
        movl      %eax, -28(%ebp)                               #881.11
..LN309:
        movl      -28(%ebp), %eax                               #881.31
..LN311:
        cmpl      $16, %eax                                     #881.35
        jae       ..B8.20       # Prob 50%                      #881.35
                                # LOE
..B8.19:                        # Preds ..B8.17 ..B8.19
..LN313:
        movl      -28(%ebp), %eax                               #883.23
..LN315:
        xorl      %edx, %edx                                    #883.9
        movb      %dl, -60(%ebp,%eax)                           #883.9
..LN317:
        movl      -28(%ebp), %eax                               #884.20
..LN319:
        movb      %dl, -44(%ebp,%eax)                           #884.9
..LN321:
        incl      -28(%ebp)                                     #881.42
..LN323:
        movl      -28(%ebp), %eax                               #881.31
..LN325:
        cmpl      $16, %eax                                     #881.35
        jb        ..B8.19       # Prob 50%                      #881.35
                                # LOE
..B8.20:                        # Preds ..B8.19 ..B8.17
..LN327:
        movl      12(%ebp), %eax                                #890.10
        testl     %eax, %eax                                    #890.10
        je        ..B8.22       # Prob 50%                      #890.10
                                # LOE
..B8.21:                        # Preds ..B8.20
..LN329:
        movl      -20(%ebp), %eax                               #892.9
        movl      $9, (%eax)                                    #892.9
        jmp       ..B8.31       # Prob 100%                     #892.9
                                # LOE
..B8.22:                        # Preds ..B8.20
..LN331:
        movl      20(%ebp), %eax                                #896.15
..LN333:
        cmpl      $2, %eax                                      #896.22
        jae       ..B8.24       # Prob 50%                      #896.22
                                # LOE
..B8.23:                        # Preds ..B8.22
..LN335:
        movzbl    -60(%ebp), %eax                               #898.29
..LN337:
        movb      %al, -59(%ebp)                                #898.9
..LN339:
        movzwl    -60(%ebp), %eax                               #899.29
..LN341:
        movw      %ax, -58(%ebp)                                #899.9
..LN343:
        movl      -60(%ebp), %eax                               #900.29
..LN345:
        movl      %eax, -56(%ebp)                               #900.9
..LN347:
        movl      -60(%ebp), %eax                               #901.29
        movl      -56(%ebp), %edx                               #901.29
..LN349:
        movl      %eax, -52(%ebp)                               #901.9
        movl      %edx, -48(%ebp)                               #901.9
..LN351:
        movzbl    -44(%ebp), %eax                               #903.26
..LN353:
        movb      %al, -43(%ebp)                                #903.9
..LN355:
        movzwl    -44(%ebp), %eax                               #904.26
..LN357:
        movw      %ax, -42(%ebp)                                #904.9
..LN359:
        movl      -44(%ebp), %eax                               #905.26
..LN361:
        movl      %eax, -40(%ebp)                               #905.9
..LN363:
        movl      -44(%ebp), %eax                               #906.26
        movl      -40(%ebp), %edx                               #906.26
..LN365:
        movl      %eax, -36(%ebp)                               #906.9
        movl      %edx, -32(%ebp)                               #906.9
..LN367:
        movl      -20(%ebp), %eax                               #908.9
        movl      $2, (%eax)                                    #908.9
        jmp       ..B8.31       # Prob 100%                     #908.9
                                # LOE
..B8.24:                        # Preds ..B8.22
..LN369:
        movl      20(%ebp), %eax                                #910.15
..LN371:
        cmpl      $6, %eax                                      #910.22
        jae       ..B8.26       # Prob 50%                      #910.22
                                # LOE
..B8.25:                        # Preds ..B8.24
..LN373:
        movzwl    -60(%ebp), %eax                               #912.29
..LN375:
        movw      %ax, -58(%ebp)                                #912.9
..LN377:
        movl      -60(%ebp), %eax                               #913.29
..LN379:
        movl      %eax, -56(%ebp)                               #913.9
..LN381:
        movl      -60(%ebp), %eax                               #914.29
        movl      -56(%ebp), %edx                               #914.29
..LN383:
        movl      %eax, -52(%ebp)                               #914.9
        movl      %edx, -48(%ebp)                               #914.9
..LN385:
        movzwl    -44(%ebp), %eax                               #916.26
..LN387:
        movw      %ax, -42(%ebp)                                #916.9
..LN389:
        movl      -44(%ebp), %eax                               #917.26
..LN391:
        movl      %eax, -40(%ebp)                               #917.9
..LN393:
        movl      -44(%ebp), %eax                               #918.26
        movl      -40(%ebp), %edx                               #918.26
..LN395:
        movl      %eax, -36(%ebp)                               #918.9
        movl      %edx, -32(%ebp)                               #918.9
..LN397:
        movl      -20(%ebp), %eax                               #920.9
        movl      $3, (%eax)                                    #920.9
        jmp       ..B8.31       # Prob 100%                     #920.9
                                # LOE
..B8.26:                        # Preds ..B8.24
..LN399:
        movl      20(%ebp), %eax                                #922.15
..LN401:
        cmpl      $14, %eax                                     #922.22
        jae       ..B8.28       # Prob 50%                      #922.22
                                # LOE
..B8.27:                        # Preds ..B8.26
..LN403:
        movl      -60(%ebp), %eax                               #924.29
..LN405:
        movl      %eax, -56(%ebp)                               #924.9
..LN407:
        movl      -60(%ebp), %eax                               #925.29
        movl      -56(%ebp), %edx                               #925.29
..LN409:
        movl      %eax, -52(%ebp)                               #925.9
        movl      %edx, -48(%ebp)                               #925.9
..LN411:
        movl      -44(%ebp), %eax                               #927.26
..LN413:
        movl      %eax, -40(%ebp)                               #927.9
..LN415:
        movl      -44(%ebp), %eax                               #928.26
        movl      -40(%ebp), %edx                               #928.26
..LN417:
        movl      %eax, -36(%ebp)                               #928.9
        movl      %edx, -32(%ebp)                               #928.9
..LN419:
        movl      -20(%ebp), %eax                               #930.9
        movl      $4, (%eax)                                    #930.9
        jmp       ..B8.31       # Prob 100%                     #930.9
                                # LOE
..B8.28:                        # Preds ..B8.26
..LN421:
        movl      20(%ebp), %eax                                #932.15
..LN423:
        cmpl      $30, %eax                                     #932.22
        jae       ..B8.30       # Prob 50%                      #932.22
                                # LOE
..B8.29:                        # Preds ..B8.28
..LN425:
        movl      -60(%ebp), %eax                               #934.29
        movl      -56(%ebp), %edx                               #934.29
..LN427:
        movl      %eax, -52(%ebp)                               #934.9
        movl      %edx, -48(%ebp)                               #934.9
..LN429:
        movl      -44(%ebp), %eax                               #935.26
        movl      -40(%ebp), %edx                               #935.26
..LN431:
        movl      %eax, -36(%ebp)                               #935.9
        movl      %edx, -32(%ebp)                               #935.9
..LN433:
        movl      -20(%ebp), %eax                               #937.9
        movl      $0, (%eax)                                    #937.9
        jmp       ..B8.31       # Prob 100%                     #937.9
                                # LOE
..B8.30:                        # Preds ..B8.28
..LN435:
        movl      -20(%ebp), %eax                               #941.9
        movl      $5, (%eax)                                    #941.9
                                # LOE
..B8.31:                        # Preds ..B8.21 ..B8.23 ..B8.25 ..B8.27 ..B8.29
                                #       ..B8.30
..LN437:
        movl      -20(%ebp), %eax                               #944.5
..LN439:
        movl      -60(%ebp), %edx                               #944.42
        movl      %edx, 16(%eax)                                #944.42
        movl      -56(%ebp), %edx                               #944.42
        movl      %edx, 20(%eax)                                #944.42
        movl      -52(%ebp), %edx                               #944.42
        movl      %edx, 24(%eax)                                #944.42
        movl      -48(%ebp), %edx                               #944.42
        movl      %edx, 28(%eax)                                #944.42
..LN441:
        movl      -20(%ebp), %eax                               #945.5
..LN443:
        movl      -44(%ebp), %edx                               #945.39
        movl      %edx, 32(%eax)                                #945.39
        movl      -40(%ebp), %edx                               #945.39
        movl      %edx, 36(%eax)                                #945.39
        movl      -36(%ebp), %edx                               #945.39
        movl      %edx, 40(%eax)                                #945.39
        movl      -32(%ebp), %edx                               #945.39
        movl      %edx, 44(%eax)                                #945.39
..LN445:
        pushl     %edi                                          #948.5
..LN447:
        lea       -60(%ebp), %eax                               #948.23
        movl      %eax, (%esp)                                  #948.23
..LN449:
        call      uint128_bswap                                 #948.5
                                # LOE
..B8.50:                        # Preds ..B8.31
        popl      %ecx                                          #948.5
                                # LOE
..B8.32:                        # Preds ..B8.50
..LN451:
        pushl     %edi                                          #949.5
..LN453:
        lea       -44(%ebp), %eax                               #949.23
        movl      %eax, (%esp)                                  #949.23
..LN455:
        call      uint128_bswap                                 #949.5
                                # LOE
..B8.51:                        # Preds ..B8.32
        popl      %ecx                                          #949.5
                                # LOE
..B8.33:                        # Preds ..B8.51
..LN457:
        addl      $-8, %esp                                     #952.5
..LN459:
        lea       -60(%ebp), %eax                               #952.21
        movl      %eax, (%esp)                                  #952.21
..LN461:
        movl      $2, 4(%esp)                                   #952.34
..LN463:
        call      uint128_shr                                   #952.5
                                # LOE
..B8.52:                        # Preds ..B8.33
        addl      $8, %esp                                      #952.5
                                # LOE
..B8.34:                        # Preds ..B8.52
..LN465:
        addl      $-8, %esp                                     #953.5
..LN467:
        lea       -44(%ebp), %eax                               #953.21
        movl      %eax, (%esp)                                  #953.21
..LN469:
        movl      $2, 4(%esp)                                   #953.31
..LN471:
        call      uint128_shr                                   #953.5
                                # LOE
..B8.53:                        # Preds ..B8.34
        addl      $8, %esp                                      #953.5
                                # LOE
..B8.35:                        # Preds ..B8.53
..LN473:
        addl      $-8, %esp                                     #956.5
..LN475:
        movl      -20(%ebp), %eax                               #956.28
..LN477:
        addl      $48, %eax                                     #956.5
        movl      %eax, (%esp)                                  #956.5
..LN479:
        lea       -60(%ebp), %eax                               #956.70
        movl      %eax, 4(%esp)                                 #956.70
..LN481:
        call      uint128_bswap_copy                            #956.5
                                # LOE
..B8.54:                        # Preds ..B8.35
        addl      $8, %esp                                      #956.5
                                # LOE
..B8.36:                        # Preds ..B8.54
..LN483:
        addl      $-8, %esp                                     #957.5
..LN485:
        movl      -20(%ebp), %eax                               #957.28
..LN487:
        addl      $64, %eax                                     #957.5
        movl      %eax, (%esp)                                  #957.5
..LN489:
        lea       -44(%ebp), %eax                               #957.67
        movl      %eax, 4(%esp)                                 #957.67
..LN491:
        call      uint128_bswap_copy                            #957.5
                                # LOE
..B8.55:                        # Preds ..B8.36
        addl      $8, %esp                                      #957.5
                                # LOE
..B8.37:                        # Preds ..B8.55
..LN493:
        addl      $-8, %esp                                     #959.5
..LN495:
        lea       -60(%ebp), %eax                               #959.21
        movl      %eax, (%esp)                                  #959.21
..LN497:
        movl      $2, 4(%esp)                                   #959.34
..LN499:
        call      uint128_shr                                   #959.5
                                # LOE
..B8.56:                        # Preds ..B8.37
        addl      $8, %esp                                      #959.5
                                # LOE
..B8.38:                        # Preds ..B8.56
..LN501:
        addl      $-8, %esp                                     #960.5
..LN503:
        lea       -44(%ebp), %eax                               #960.21
        movl      %eax, (%esp)                                  #960.21
..LN505:
        movl      $2, 4(%esp)                                   #960.31
..LN507:
        call      uint128_shr                                   #960.5
                                # LOE
..B8.57:                        # Preds ..B8.38
        addl      $8, %esp                                      #960.5
                                # LOE
..B8.39:                        # Preds ..B8.57
..LN509:
        addl      $-8, %esp                                     #962.5
..LN511:
        movl      -20(%ebp), %eax                               #962.28
..LN513:
        addl      $80, %eax                                     #962.5
        movl      %eax, (%esp)                                  #962.5
..LN515:
        lea       -60(%ebp), %eax                               #962.70
        movl      %eax, 4(%esp)                                 #962.70
..LN517:
        call      uint128_bswap_copy                            #962.5
                                # LOE
..B8.58:                        # Preds ..B8.39
        addl      $8, %esp                                      #962.5
                                # LOE
..B8.40:                        # Preds ..B8.58
..LN519:
        addl      $-8, %esp                                     #963.5
..LN521:
        movl      -20(%ebp), %eax                               #963.28
..LN523:
        addl      $96, %eax                                     #963.5
        movl      %eax, (%esp)                                  #963.5
..LN525:
        lea       -44(%ebp), %eax                               #963.67
        movl      %eax, 4(%esp)                                 #963.67
..LN527:
        call      uint128_bswap_copy                            #963.5
                                # LOE
..B8.59:                        # Preds ..B8.40
        addl      $8, %esp                                      #963.5
                                # LOE
..B8.41:                        # Preds ..B8.59
..LN529:
        addl      $-8, %esp                                     #965.5
..LN531:
        lea       -60(%ebp), %eax                               #965.21
        movl      %eax, (%esp)                                  #965.21
..LN533:
        movl      $2, 4(%esp)                                   #965.34
..LN535:
        call      uint128_shr                                   #965.5
                                # LOE
..B8.60:                        # Preds ..B8.41
        addl      $8, %esp                                      #965.5
                                # LOE
..B8.42:                        # Preds ..B8.60
..LN537:
        addl      $-8, %esp                                     #966.5
..LN539:
        lea       -44(%ebp), %eax                               #966.21
        movl      %eax, (%esp)                                  #966.21
..LN541:
        movl      $2, 4(%esp)                                   #966.31
..LN543:
        call      uint128_shr                                   #966.5
                                # LOE
..B8.61:                        # Preds ..B8.42
        addl      $8, %esp                                      #966.5
                                # LOE
..B8.43:                        # Preds ..B8.61
..LN545:
        addl      $-8, %esp                                     #968.5
..LN547:
        movl      -20(%ebp), %eax                               #968.28
..LN549:
        addl      $112, %eax                                    #968.5
        movl      %eax, (%esp)                                  #968.5
..LN551:
        lea       -60(%ebp), %eax                               #968.70
        movl      %eax, 4(%esp)                                 #968.70
..LN553:
        call      uint128_bswap_copy                            #968.5
                                # LOE
..B8.62:                        # Preds ..B8.43
        addl      $8, %esp                                      #968.5
                                # LOE
..B8.44:                        # Preds ..B8.62
..LN555:
        addl      $-8, %esp                                     #969.5
..LN557:
        movl      -20(%ebp), %eax                               #969.28
..LN559:
        addl      $128, %eax                                    #969.5
        movl      %eax, (%esp)                                  #969.5
..LN561:
        lea       -44(%ebp), %eax                               #969.67
        movl      %eax, 4(%esp)                                 #969.67
..LN563:
        call      uint128_bswap_copy                            #969.5
                                # LOE
..B8.63:                        # Preds ..B8.44
        addl      $8, %esp                                      #969.5
                                # LOE
..B8.45:                        # Preds ..B8.63
..LN565:
        xorl      %eax, %eax                                    #1011.12
        movl      -4(%ebp), %ebx                                #1011.12
..___tag_value_NucStrFastaExprMake2.134:                        #
        leave                                                   #1011.12
..___tag_value_NucStrFastaExprMake2.136:                        #
        ret                                                     #1011.12
        .align    2,0x90
..___tag_value_NucStrFastaExprMake2.137:                        #
                                # LOE
# mark_end;
	.type	NucStrFastaExprMake2,@function
	.size	NucStrFastaExprMake2,.-NucStrFastaExprMake2
.LNNucStrFastaExprMake2:
	.data
# -- End  NucStrFastaExprMake2
	.bss
	.align 32
fasta_4na_map:
	.type	fasta_4na_map,@object
	.size	fasta_4na_map,128
	.space 128	# pad
	.data
	.text
# -- Begin  NucStrFastaExprMake4
# mark_begin;
       .align    2,0x90
NucStrFastaExprMake4:
# parameter 1(expr): 8 + %ebp
# parameter 2(positional): 12 + %ebp
# parameter 3(fasta): 16 + %ebp
# parameter 4(size): 20 + %ebp
..B9.1:                         # Preds ..B9.0
..___tag_value_NucStrFastaExprMake4.140:                        #
..LN567:
        pushl     %ebp                                          #1017.1
        movl      %esp, %ebp                                    #1017.1
..___tag_value_NucStrFastaExprMake4.145:                        #
        subl      $60, %esp                                     #1017.1
        movl      %ebx, -4(%ebp)                                #1017.1
..___tag_value_NucStrFastaExprMake4.148:                        #
..LN569:
        movl      20(%ebp), %eax                                #1023.10
..LN571:
        cmpl      $29, %eax                                     #1023.17
        jbe       ..B9.3        # Prob 50%                      #1023.17
                                # LOE
..B9.2:                         # Preds ..B9.1
..LN573:
        movl      $7, %eax                                      #1024.16
        movl      -4(%ebp), %ebx                                #1024.16
..___tag_value_NucStrFastaExprMake4.149:                        #
        leave                                                   #1024.16
..___tag_value_NucStrFastaExprMake4.151:                        #
        ret                                                     #1024.16
..___tag_value_NucStrFastaExprMake4.152:                        #
                                # LOE
..B9.3:                         # Preds ..B9.1
..LN575:
        pushl     %edi                                          #1026.9
..LN577:
        movl      $144, (%esp)                                  #1026.32
..LN579:
        call      NucStrFastaExprAlloc                          #1026.9
                                # LOE eax
..B9.46:                        # Preds ..B9.3
        popl      %ecx                                          #1026.9
        movl      %eax, -12(%ebp)                               #1026.9
                                # LOE
..B9.4:                         # Preds ..B9.46
        call      ..L154        # Prob 100%                     #
..L154:                                                         #
        popl      %eax                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L154], %eax     #
        movl      %eax, -8(%ebp)                                #
..LN581:
        movl      -12(%ebp), %eax                               #1026.5
        movl      %eax, -16(%ebp)                               #1026.5
..LN583:
        movl      -16(%ebp), %eax                               #1027.10
..LN585:
        testl     %eax, %eax                                    #1027.15
        jne       ..B9.6        # Prob 50%                      #1027.15
                                # LOE
..B9.5:                         # Preds ..B9.4
..LN587:
        movl      -8(%ebp), %eax                                #1028.16
        movl      %eax, %ebx                                    #1028.16
        call      __errno_location@PLT                          #1028.16
                                # LOE eax
..B9.47:                        # Preds ..B9.5
        movl      (%eax), %eax                                  #1028.16
        movl      -4(%ebp), %ebx                                #1028.16
..___tag_value_NucStrFastaExprMake4.155:                        #
        leave                                                   #1028.16
..___tag_value_NucStrFastaExprMake4.157:                        #
        ret                                                     #1028.16
..___tag_value_NucStrFastaExprMake4.158:                        #
                                # LOE
..B9.6:                         # Preds ..B9.4
..LN589:
        movl      8(%ebp), %eax                                 #1030.5
..LN591:
        movl      -16(%ebp), %edx                               #1030.14
..LN593:
        movl      %edx, (%eax)                                  #1030.5
..LN595:
        movl      -16(%ebp), %eax                               #1031.5
..LN597:
        movl      20(%ebp), %edx                                #1031.25
..LN599:
        movl      %edx, 4(%eax)                                 #1031.5
..LN601:
        movl      $0, -28(%ebp)                                 #1035.11
..LN603:
        movl      -28(%ebp), %eax                               #1035.18
..LN605:
        movl      20(%ebp), %edx                                #1035.22
        cmpl      %edx, %eax                                    #1035.22
        jb        ..B9.9        # Prob 50%                      #1035.22
        jmp       ..B9.17       # Prob 100%                     #1035.22
                                # LOE
..B9.7:                         # Preds ..B9.13 ..B9.14 ..B9.15 ..B9.16 ..B9.12
                                #      
..LN607:
        incl      -28(%ebp)                                     #1035.31
..LN609:
        movl      -28(%ebp), %eax                               #1035.18
..LN611:
        movl      20(%ebp), %edx                                #1035.22
        cmpl      %edx, %eax                                    #1035.22
        jae       ..B9.17       # Prob 50%                      #1035.22
                                # LOE
..B9.9:                         # Preds ..B9.6 ..B9.7
..LN613:
        movl      16(%ebp), %eax                                #1037.49
..LN615:
        movl      -28(%ebp), %edx                               #1037.57
..LN617:
        movzbl    (%edx,%eax), %eax                             #1037.49
        movsbl    %al, %eax                                     #1037.49
..LN619:
        movl      -8(%ebp), %edx                                #1037.25
        movzbl    fasta_4na_map@GOTOFF(%eax,%edx), %eax         #1037.25
        movsbl    %al, %eax                                     #1037.25
..LN621:
        movw      %ax, -20(%ebp)                                #1037.23
..LN623:
        movl      -28(%ebp), %eax                               #1038.18
..LN625:
        andl      $3, %eax                                      #1038.22
        movl      %eax, -24(%ebp)                               #1038.22
        je        ..B9.13       # Prob 50%                      #1038.22
                                # LOE
..B9.10:                        # Preds ..B9.9
        movl      -24(%ebp), %eax                               #1038.22
        cmpl      $1, %eax                                      #1038.22
        je        ..B9.14       # Prob 50%                      #1038.22
                                # LOE
..B9.11:                        # Preds ..B9.10
        movl      -24(%ebp), %eax                               #1038.22
        cmpl      $2, %eax                                      #1038.22
        je        ..B9.15       # Prob 50%                      #1038.22
                                # LOE
..B9.12:                        # Preds ..B9.11
        movl      -24(%ebp), %eax                               #1038.22
        cmpl      $3, %eax                                      #1038.22
        je        ..B9.16       # Prob 50%                      #1038.22
        jmp       ..B9.7        # Prob 100%                     #1038.22
                                # LOE
..B9.13:                        # Preds ..B9.9
..LN627:
        movzwl    -20(%ebp), %eax                               #1041.38
..LN629:
        shll      $4, %eax                                      #1041.46
..LN631:
        movl      -28(%ebp), %edx                               #1041.27
..LN633:
        shrl      $2, %edx                                      #1041.32
..LN635:
        movw      %ax, -60(%ebp,%edx,2)                         #1041.13
..LN637:
        movl      -28(%ebp), %eax                               #1042.24
..LN639:
        shrl      $2, %eax                                      #1042.29
..LN641:
        movw      $240, -44(%ebp,%eax,2)                        #1042.13
        jmp       ..B9.7        # Prob 100%                     #1042.13
                                # LOE
..B9.14:                        # Preds ..B9.10
..LN643:
        movl      -28(%ebp), %eax                               #1045.27
..LN645:
        shrl      $2, %eax                                      #1045.32
..LN647:
        movzwl    -60(%ebp,%eax,2), %eax                        #1045.13
..LN649:
        movzwl    -20(%ebp), %edx                               #1045.39
..LN651:
        orl       %edx, %eax                                    #1045.13
..LN653:
        movl      -28(%ebp), %edx                               #1045.27
..LN655:
        shrl      $2, %edx                                      #1045.32
..LN657:
        movw      %ax, -60(%ebp,%edx,2)                         #1045.13
..LN659:
        movl      -28(%ebp), %eax                               #1046.24
..LN661:
        shrl      $2, %eax                                      #1046.29
..LN663:
        movzwl    -44(%ebp,%eax,2), %eax                        #1046.13
        orl       $15, %eax                                     #1046.13
..LN665:
        movl      -28(%ebp), %edx                               #1046.24
..LN667:
        shrl      $2, %edx                                      #1046.29
..LN669:
        movw      %ax, -44(%ebp,%edx,2)                         #1046.13
        jmp       ..B9.7        # Prob 100%                     #1046.13
                                # LOE
..B9.15:                        # Preds ..B9.11
..LN671:
        movl      -28(%ebp), %eax                               #1049.27
..LN673:
        shrl      $2, %eax                                      #1049.32
..LN675:
        movzwl    -60(%ebp,%eax,2), %eax                        #1049.13
..LN677:
        movzwl    -20(%ebp), %edx                               #1049.39
..LN679:
        shll      $12, %edx                                     #1049.47
..LN681:
        orl       %edx, %eax                                    #1049.13
..LN683:
        movl      -28(%ebp), %edx                               #1049.27
..LN685:
        shrl      $2, %edx                                      #1049.32
..LN687:
        movw      %ax, -60(%ebp,%edx,2)                         #1049.13
..LN689:
        movl      -28(%ebp), %eax                               #1050.24
..LN691:
        shrl      $2, %eax                                      #1050.29
..LN693:
        movzwl    -44(%ebp,%eax,2), %eax                        #1050.13
        orl       $-4096, %eax                                  #1050.13
..LN695:
        movl      -28(%ebp), %edx                               #1050.24
..LN697:
        shrl      $2, %edx                                      #1050.29
..LN699:
        movw      %ax, -44(%ebp,%edx,2)                         #1050.13
        jmp       ..B9.7        # Prob 100%                     #1050.13
                                # LOE
..B9.16:                        # Preds ..B9.12
..LN701:
        movl      -28(%ebp), %eax                               #1053.27
..LN703:
        shrl      $2, %eax                                      #1053.32
..LN705:
        movzwl    -60(%ebp,%eax,2), %eax                        #1053.13
..LN707:
        movzwl    -20(%ebp), %edx                               #1053.39
..LN709:
        shll      $8, %edx                                      #1053.47
..LN711:
        orl       %edx, %eax                                    #1053.13
..LN713:
        movl      -28(%ebp), %edx                               #1053.27
..LN715:
        shrl      $2, %edx                                      #1053.32
..LN717:
        movw      %ax, -60(%ebp,%edx,2)                         #1053.13
..LN719:
        movl      -28(%ebp), %eax                               #1054.24
..LN721:
        shrl      $2, %eax                                      #1054.29
..LN723:
        movzwl    -44(%ebp,%eax,2), %eax                        #1054.13
        orl       $-61696, %eax                                 #1054.13
..LN725:
        movl      -28(%ebp), %edx                               #1054.24
..LN727:
        shrl      $2, %edx                                      #1054.29
..LN729:
        movw      %ax, -44(%ebp,%edx,2)                         #1054.13
        jmp       ..B9.7        # Prob 100%                     #1054.13
                                # LOE
..B9.17:                        # Preds ..B9.7 ..B9.6
..LN731:
        movl      -28(%ebp), %eax                               #1060.17
..LN733:
        addl      $3, %eax                                      #1060.21
..LN735:
        shrl      $2, %eax                                      #1060.28
..LN737:
        movl      %eax, -28(%ebp)                               #1060.11
..LN739:
        movl      -28(%ebp), %eax                               #1060.31
..LN741:
        cmpl      $8, %eax                                      #1060.35
        jae       ..B9.20       # Prob 50%                      #1060.35
                                # LOE
..B9.19:                        # Preds ..B9.17 ..B9.19
..LN743:
        movl      -28(%ebp), %eax                               #1062.23
..LN745:
        xorl      %edx, %edx                                    #1062.9
        movw      %dx, -60(%ebp,%eax,2)                         #1062.9
..LN747:
        movl      -28(%ebp), %eax                               #1063.20
..LN749:
        movw      %dx, -44(%ebp,%eax,2)                         #1063.9
..LN751:
        incl      -28(%ebp)                                     #1060.41
..LN753:
        movl      -28(%ebp), %eax                               #1060.31
..LN755:
        cmpl      $8, %eax                                      #1060.35
        jb        ..B9.19       # Prob 50%                      #1060.35
                                # LOE
..B9.20:                        # Preds ..B9.19 ..B9.17
..LN757:
        movl      12(%ebp), %eax                                #1068.10
        testl     %eax, %eax                                    #1068.10
        je        ..B9.22       # Prob 50%                      #1068.10
                                # LOE
..B9.21:                        # Preds ..B9.20
..LN759:
        movl      -16(%ebp), %eax                               #1070.9
        movl      $10, (%eax)                                   #1070.9
        jmp       ..B9.29       # Prob 100%                     #1070.9
                                # LOE
..B9.22:                        # Preds ..B9.20
..LN761:
        movl      20(%ebp), %eax                                #1072.15
..LN763:
        cmpl      $2, %eax                                      #1072.22
        jae       ..B9.24       # Prob 50%                      #1072.22
                                # LOE
..B9.23:                        # Preds ..B9.22
..LN765:
        movzwl    -60(%ebp), %eax                               #1074.29
..LN767:
        movw      %ax, -58(%ebp)                                #1074.9
..LN769:
        movl      -60(%ebp), %eax                               #1075.29
..LN771:
        movl      %eax, -56(%ebp)                               #1075.9
..LN773:
        movl      -60(%ebp), %eax                               #1076.29
        movl      -56(%ebp), %edx                               #1076.29
..LN775:
        movl      %eax, -52(%ebp)                               #1076.9
        movl      %edx, -48(%ebp)                               #1076.9
..LN777:
        movzwl    -44(%ebp), %eax                               #1078.26
..LN779:
        movw      %ax, -42(%ebp)                                #1078.9
..LN781:
        movl      -44(%ebp), %eax                               #1079.26
..LN783:
        movl      %eax, -40(%ebp)                               #1079.9
..LN785:
        movl      -44(%ebp), %eax                               #1080.26
        movl      -40(%ebp), %edx                               #1080.26
..LN787:
        movl      %eax, -36(%ebp)                               #1080.9
        movl      %edx, -32(%ebp)                               #1080.9
..LN789:
        movl      -16(%ebp), %eax                               #1082.9
        movl      $6, (%eax)                                    #1082.9
        jmp       ..B9.29       # Prob 100%                     #1082.9
                                # LOE
..B9.24:                        # Preds ..B9.22
..LN791:
        movl      20(%ebp), %eax                                #1084.15
..LN793:
        cmpl      $6, %eax                                      #1084.22
        jae       ..B9.26       # Prob 50%                      #1084.22
                                # LOE
..B9.25:                        # Preds ..B9.24
..LN795:
        movl      -60(%ebp), %eax                               #1086.29
..LN797:
        movl      %eax, -56(%ebp)                               #1086.9
..LN799:
        movl      -60(%ebp), %eax                               #1087.29
        movl      -56(%ebp), %edx                               #1087.29
..LN801:
        movl      %eax, -52(%ebp)                               #1087.9
        movl      %edx, -48(%ebp)                               #1087.9
..LN803:
        movl      -44(%ebp), %eax                               #1089.26
..LN805:
        movl      %eax, -40(%ebp)                               #1089.9
..LN807:
        movl      -44(%ebp), %eax                               #1090.26
        movl      -40(%ebp), %edx                               #1090.26
..LN809:
        movl      %eax, -36(%ebp)                               #1090.9
        movl      %edx, -32(%ebp)                               #1090.9
..LN811:
        movl      -16(%ebp), %eax                               #1092.9
        movl      $7, (%eax)                                    #1092.9
        jmp       ..B9.29       # Prob 100%                     #1092.9
                                # LOE
..B9.26:                        # Preds ..B9.24
..LN813:
        movl      20(%ebp), %eax                                #1094.15
..LN815:
        cmpl      $14, %eax                                     #1094.22
        jae       ..B9.28       # Prob 50%                      #1094.22
                                # LOE
..B9.27:                        # Preds ..B9.26
..LN817:
        movl      -60(%ebp), %eax                               #1096.29
        movl      -56(%ebp), %edx                               #1096.29
..LN819:
        movl      %eax, -52(%ebp)                               #1096.9
        movl      %edx, -48(%ebp)                               #1096.9
..LN821:
        movl      -44(%ebp), %eax                               #1097.26
        movl      -40(%ebp), %edx                               #1097.26
..LN823:
        movl      %eax, -36(%ebp)                               #1097.9
        movl      %edx, -32(%ebp)                               #1097.9
..LN825:
        movl      -16(%ebp), %eax                               #1099.9
        movl      $1, (%eax)                                    #1099.9
        jmp       ..B9.29       # Prob 100%                     #1099.9
                                # LOE
..B9.28:                        # Preds ..B9.26
..LN827:
        movl      -16(%ebp), %eax                               #1103.9
        movl      $8, (%eax)                                    #1103.9
                                # LOE
..B9.29:                        # Preds ..B9.21 ..B9.23 ..B9.25 ..B9.27 ..B9.28
                                #      
..LN829:
        movl      -16(%ebp), %eax                               #1106.5
..LN831:
        movl      -60(%ebp), %edx                               #1106.42
        movl      %edx, 16(%eax)                                #1106.42
        movl      -56(%ebp), %edx                               #1106.42
        movl      %edx, 20(%eax)                                #1106.42
        movl      -52(%ebp), %edx                               #1106.42
        movl      %edx, 24(%eax)                                #1106.42
        movl      -48(%ebp), %edx                               #1106.42
        movl      %edx, 28(%eax)                                #1106.42
..LN833:
        movl      -16(%ebp), %eax                               #1107.5
..LN835:
        movl      -44(%ebp), %edx                               #1107.39
        movl      %edx, 32(%eax)                                #1107.39
        movl      -40(%ebp), %edx                               #1107.39
        movl      %edx, 36(%eax)                                #1107.39
        movl      -36(%ebp), %edx                               #1107.39
        movl      %edx, 40(%eax)                                #1107.39
        movl      -32(%ebp), %edx                               #1107.39
        movl      %edx, 44(%eax)                                #1107.39
..LN837:
        pushl     %edi                                          #1110.5
..LN839:
        lea       -60(%ebp), %eax                               #1110.23
        movl      %eax, (%esp)                                  #1110.23
..LN841:
        call      uint128_bswap                                 #1110.5
                                # LOE
..B9.48:                        # Preds ..B9.29
        popl      %ecx                                          #1110.5
                                # LOE
..B9.30:                        # Preds ..B9.48
..LN843:
        pushl     %edi                                          #1111.5
..LN845:
        lea       -44(%ebp), %eax                               #1111.23
        movl      %eax, (%esp)                                  #1111.23
..LN847:
        call      uint128_bswap                                 #1111.5
                                # LOE
..B9.49:                        # Preds ..B9.30
        popl      %ecx                                          #1111.5
                                # LOE
..B9.31:                        # Preds ..B9.49
..LN849:
        addl      $-8, %esp                                     #1114.5
..LN851:
        lea       -60(%ebp), %eax                               #1114.21
        movl      %eax, (%esp)                                  #1114.21
..LN853:
        movl      $4, 4(%esp)                                   #1114.34
..LN855:
        call      uint128_shr                                   #1114.5
                                # LOE
..B9.50:                        # Preds ..B9.31
        addl      $8, %esp                                      #1114.5
                                # LOE
..B9.32:                        # Preds ..B9.50
..LN857:
        addl      $-8, %esp                                     #1115.5
..LN859:
        lea       -44(%ebp), %eax                               #1115.21
        movl      %eax, (%esp)                                  #1115.21
..LN861:
        movl      $4, 4(%esp)                                   #1115.31
..LN863:
        call      uint128_shr                                   #1115.5
                                # LOE
..B9.51:                        # Preds ..B9.32
        addl      $8, %esp                                      #1115.5
                                # LOE
..B9.33:                        # Preds ..B9.51
..LN865:
        addl      $-8, %esp                                     #1118.5
..LN867:
        movl      -16(%ebp), %eax                               #1118.28
..LN869:
        addl      $48, %eax                                     #1118.5
        movl      %eax, (%esp)                                  #1118.5
..LN871:
        lea       -60(%ebp), %eax                               #1118.70
        movl      %eax, 4(%esp)                                 #1118.70
..LN873:
        call      uint128_bswap_copy                            #1118.5
                                # LOE
..B9.52:                        # Preds ..B9.33
        addl      $8, %esp                                      #1118.5
                                # LOE
..B9.34:                        # Preds ..B9.52
..LN875:
        addl      $-8, %esp                                     #1119.5
..LN877:
        movl      -16(%ebp), %eax                               #1119.28
..LN879:
        addl      $64, %eax                                     #1119.5
        movl      %eax, (%esp)                                  #1119.5
..LN881:
        lea       -44(%ebp), %eax                               #1119.67
        movl      %eax, 4(%esp)                                 #1119.67
..LN883:
        call      uint128_bswap_copy                            #1119.5
                                # LOE
..B9.53:                        # Preds ..B9.34
        addl      $8, %esp                                      #1119.5
                                # LOE
..B9.35:                        # Preds ..B9.53
..LN885:
        addl      $-8, %esp                                     #1121.5
..LN887:
        lea       -60(%ebp), %eax                               #1121.21
        movl      %eax, (%esp)                                  #1121.21
..LN889:
        movl      $4, 4(%esp)                                   #1121.34
..LN891:
        call      uint128_shr                                   #1121.5
                                # LOE
..B9.54:                        # Preds ..B9.35
        addl      $8, %esp                                      #1121.5
                                # LOE
..B9.36:                        # Preds ..B9.54
..LN893:
        addl      $-8, %esp                                     #1122.5
..LN895:
        lea       -44(%ebp), %eax                               #1122.21
        movl      %eax, (%esp)                                  #1122.21
..LN897:
        movl      $4, 4(%esp)                                   #1122.31
..LN899:
        call      uint128_shr                                   #1122.5
                                # LOE
..B9.55:                        # Preds ..B9.36
        addl      $8, %esp                                      #1122.5
                                # LOE
..B9.37:                        # Preds ..B9.55
..LN901:
        addl      $-8, %esp                                     #1124.5
..LN903:
        movl      -16(%ebp), %eax                               #1124.28
..LN905:
        addl      $80, %eax                                     #1124.5
        movl      %eax, (%esp)                                  #1124.5
..LN907:
        lea       -60(%ebp), %eax                               #1124.70
        movl      %eax, 4(%esp)                                 #1124.70
..LN909:
        call      uint128_bswap_copy                            #1124.5
                                # LOE
..B9.56:                        # Preds ..B9.37
        addl      $8, %esp                                      #1124.5
                                # LOE
..B9.38:                        # Preds ..B9.56
..LN911:
        addl      $-8, %esp                                     #1125.5
..LN913:
        movl      -16(%ebp), %eax                               #1125.28
..LN915:
        addl      $96, %eax                                     #1125.5
        movl      %eax, (%esp)                                  #1125.5
..LN917:
        lea       -44(%ebp), %eax                               #1125.67
        movl      %eax, 4(%esp)                                 #1125.67
..LN919:
        call      uint128_bswap_copy                            #1125.5
                                # LOE
..B9.57:                        # Preds ..B9.38
        addl      $8, %esp                                      #1125.5
                                # LOE
..B9.39:                        # Preds ..B9.57
..LN921:
        addl      $-8, %esp                                     #1127.5
..LN923:
        lea       -60(%ebp), %eax                               #1127.21
        movl      %eax, (%esp)                                  #1127.21
..LN925:
        movl      $4, 4(%esp)                                   #1127.34
..LN927:
        call      uint128_shr                                   #1127.5
                                # LOE
..B9.58:                        # Preds ..B9.39
        addl      $8, %esp                                      #1127.5
                                # LOE
..B9.40:                        # Preds ..B9.58
..LN929:
        addl      $-8, %esp                                     #1128.5
..LN931:
        lea       -44(%ebp), %eax                               #1128.21
        movl      %eax, (%esp)                                  #1128.21
..LN933:
        movl      $4, 4(%esp)                                   #1128.31
..LN935:
        call      uint128_shr                                   #1128.5
                                # LOE
..B9.59:                        # Preds ..B9.40
        addl      $8, %esp                                      #1128.5
                                # LOE
..B9.41:                        # Preds ..B9.59
..LN937:
        addl      $-8, %esp                                     #1130.5
..LN939:
        movl      -16(%ebp), %eax                               #1130.28
..LN941:
        addl      $112, %eax                                    #1130.5
        movl      %eax, (%esp)                                  #1130.5
..LN943:
        lea       -60(%ebp), %eax                               #1130.70
        movl      %eax, 4(%esp)                                 #1130.70
..LN945:
        call      uint128_bswap_copy                            #1130.5
                                # LOE
..B9.60:                        # Preds ..B9.41
        addl      $8, %esp                                      #1130.5
                                # LOE
..B9.42:                        # Preds ..B9.60
..LN947:
        addl      $-8, %esp                                     #1131.5
..LN949:
        movl      -16(%ebp), %eax                               #1131.28
..LN951:
        addl      $128, %eax                                    #1131.5
        movl      %eax, (%esp)                                  #1131.5
..LN953:
        lea       -44(%ebp), %eax                               #1131.67
        movl      %eax, 4(%esp)                                 #1131.67
..LN955:
        call      uint128_bswap_copy                            #1131.5
                                # LOE
..B9.61:                        # Preds ..B9.42
        addl      $8, %esp                                      #1131.5
                                # LOE
..B9.43:                        # Preds ..B9.61
..LN957:
        xorl      %eax, %eax                                    #1173.12
        movl      -4(%ebp), %ebx                                #1173.12
..___tag_value_NucStrFastaExprMake4.160:                        #
        leave                                                   #1173.12
..___tag_value_NucStrFastaExprMake4.162:                        #
        ret                                                     #1173.12
        .align    2,0x90
..___tag_value_NucStrFastaExprMake4.163:                        #
                                # LOE
# mark_end;
	.type	NucStrFastaExprMake4,@function
	.size	NucStrFastaExprMake4,.-NucStrFastaExprMake4
.LNNucStrFastaExprMake4:
	.data
# -- End  NucStrFastaExprMake4
	.data
	.text
# -- Begin  nss_sob
# mark_begin;
       .align    2,0x90
nss_sob:
# parameter 1(p): 8 + %ebp
# parameter 2(end): 12 + %ebp
..B10.1:                        # Preds ..B10.0
..___tag_value_nss_sob.166:                                     #
..LN959:
        pushl     %ebp                                          #1181.1
        movl      %esp, %ebp                                    #1181.1
..___tag_value_nss_sob.171:                                     #
        subl      $8, %esp                                      #1181.1
        movl      %ebx, -4(%ebp)                                #1181.1
..___tag_value_nss_sob.174:                                     #
..LN961:
        movl      8(%ebp), %eax                                 #1182.13
..LN963:
        movl      12(%ebp), %edx                                #1182.17
        cmpl      %edx, %eax                                    #1182.17
        jae       ..B10.6       # Prob 50%                      #1182.17
                                # LOE
..B10.2:                        # Preds ..B10.1
        call      ..L175        # Prob 100%                     #
..L175:                                                         #
        popl      %eax                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L175], %eax     #
        movl      %eax, -8(%ebp)                                #
..LN965:
        movl      %eax, %ebx                                    #1182.24
        call      __ctype_b_loc@PLT                             #1182.24
                                # LOE eax
..B10.9:                        # Preds ..B10.2
        movl      8(%ebp), %edx                                 #1182.24
        movzbl    (%edx), %edx                                  #1182.24
        movzbl    %dl, %edx                                     #1182.24
        movl      (%eax), %eax                                  #1182.24
        movzwl    (%eax,%edx,2), %eax                           #1182.24
        testl     $8192, %eax                                   #1182.24
        jne       ..B10.5       # Prob 50%                      #1182.24
        jmp       ..B10.6       # Prob 100%                     #1182.24
                                # LOE
..B10.3:                        # Preds ..B10.5
        movl      -8(%ebp), %eax                                #1182.24
        movl      %eax, %ebx                                    #1182.24
        call      __ctype_b_loc@PLT                             #1182.24
                                # LOE eax
..B10.10:                       # Preds ..B10.3
        movl      8(%ebp), %edx                                 #1182.24
        movzbl    (%edx), %edx                                  #1182.24
        movzbl    %dl, %edx                                     #1182.24
        movl      (%eax), %eax                                  #1182.24
        movzwl    (%eax,%edx,2), %eax                           #1182.24
        testl     $8192, %eax                                   #1182.24
        je        ..B10.6       # Prob 50%                      #1182.24
                                # LOE
..B10.5:                        # Preds ..B10.9 ..B10.10
..LN967:
        incl      8(%ebp)                                       #1183.12
..LN969:
        movl      8(%ebp), %eax                                 #1182.13
..LN971:
        movl      12(%ebp), %edx                                #1182.17
        cmpl      %edx, %eax                                    #1182.17
        jb        ..B10.3       # Prob 50%                      #1182.17
                                # LOE
..B10.6:                        # Preds ..B10.10 ..B10.9 ..B10.5 ..B10.1
..LN973:
        movl      8(%ebp), %eax                                 #1184.12
        movl      -4(%ebp), %ebx                                #1184.12
..___tag_value_nss_sob.176:                                     #
        leave                                                   #1184.12
..___tag_value_nss_sob.178:                                     #
        ret                                                     #1184.12
        .align    2,0x90
..___tag_value_nss_sob.179:                                     #
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
# parameter 1(p): 8 + %ebp
# parameter 2(end): 12 + %ebp
# parameter 3(expr): 16 + %ebp
# parameter 4(status): 20 + %ebp
# parameter 5(positional): 24 + %ebp
..B11.1:                        # Preds ..B11.0
..___tag_value_nss_FASTA_expr.182:                              #
..LN975:
        pushl     %ebp                                          #1192.1
        movl      %esp, %ebp                                    #1192.1
..___tag_value_nss_FASTA_expr.187:                              #
        subl      $24, %esp                                     #1192.1
..LN977:
        movl      8(%ebp), %eax                                 #1193.10
..LN979:
        movl      12(%ebp), %edx                                #1193.15
        cmpl      %edx, %eax                                    #1193.15
        jb        ..B11.3       # Prob 50%                      #1193.15
                                # LOE
..B11.2:                        # Preds ..B11.1
..LN981:
        movl      20(%ebp), %eax                                #1194.9
        movl      $22, (%eax)                                   #1194.9
        jmp       ..B11.17      # Prob 100%                     #1194.9
                                # LOE
..B11.3:                        # Preds ..B11.1
        call      ..L190        # Prob 100%                     #
..L190:                                                         #
        popl      %eax                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L190], %eax     #
        movl      %eax, -4(%ebp)                                #
..LN983:
        movl      8(%ebp), %edx                                 #1197.30
..LN985:
        movl      %edx, -16(%ebp)                               #1197.27
..LN987:
        movl      $0, -12(%ebp)                                 #1199.22
..LN989:
        lea       fasta_2na_map@GOTOFF(%eax), %eax              #1200.27
        movl      %eax, -8(%ebp)                                #1200.27
                                # LOE
..B11.4:                        # Preds ..B11.9 ..B11.3
..LN991:
        movl      8(%ebp), %eax                                 #1203.20
        movzbl    (%eax), %eax                                  #1203.20
        movsbl    %al, %eax                                     #1203.20
..LN993:
        testl     %eax, %eax                                    #1203.24
        jl        ..B11.10      # Prob 50%                      #1203.24
                                # LOE
..B11.5:                        # Preds ..B11.4
..LN995:
        movl      8(%ebp), %eax                                 #1206.45
        movzbl    (%eax), %eax                                  #1206.45
        movzbl    %al, %eax                                     #1206.45
..LN997:
        movl      -8(%ebp), %edx                                #1206.18
        movzbl    (%eax,%edx), %eax                             #1206.18
        movsbl    %al, %eax                                     #1206.18
..LN999:
        testl     %eax, %eax                                    #1206.51
        jge       ..B11.9       # Prob 50%                      #1206.51
                                # LOE
..B11.6:                        # Preds ..B11.5
..LN1001:
        movl      -4(%ebp), %eax                                #1208.29
        lea       fasta_4na_map@GOTOFF(%eax), %eax              #1208.29
..LN1003:
        movl      -8(%ebp), %edx                                #1208.22
..LN1005:
        cmpl      %eax, %edx                                    #1208.29
        je        ..B11.10      # Prob 50%                      #1208.29
                                # LOE
..B11.7:                        # Preds ..B11.6
..LN1007:
        movl      8(%ebp), %eax                                 #1210.59
        movzbl    (%eax), %eax                                  #1210.59
        movzbl    %al, %eax                                     #1210.59
..LN1009:
        movl      -4(%ebp), %edx                                #1210.22
        movzbl    fasta_4na_map@GOTOFF(%eax,%edx), %eax         #1210.22
        movsbl    %al, %eax                                     #1210.22
..LN1011:
        testl     %eax, %eax                                    #1210.65
        jl        ..B11.10      # Prob 50%                      #1210.65
                                # LOE
..B11.8:                        # Preds ..B11.7
..LN1013:
        movl      $1, -12(%ebp)                                 #1212.17
..LN1015:
        movl      -4(%ebp), %eax                                #1213.23
        lea       fasta_4na_map@GOTOFF(%eax), %eax              #1213.23
..LN1017:
        movl      %eax, -8(%ebp)                                #1213.17
                                # LOE
..B11.9:                        # Preds ..B11.8 ..B11.5
..LN1019:
        movl      8(%ebp), %eax                                 #1216.20
        incl      %eax                                          #1216.20
        movl      %eax, 8(%ebp)                                 #1216.20
..LN1021:
        movl      12(%ebp), %edx                                #1216.24
        cmpl      %edx, %eax                                    #1216.24
        jb        ..B11.4       # Prob 50%                      #1216.24
                                # LOE
..B11.10:                       # Preds ..B11.7 ..B11.6 ..B11.4 ..B11.9
..LN1023:
        movl      8(%ebp), %eax                                 #1218.14
..LN1025:
        movl      -16(%ebp), %edx                               #1218.19
        cmpl      %edx, %eax                                    #1218.19
        ja        ..B11.12      # Prob 50%                      #1218.19
                                # LOE
..B11.11:                       # Preds ..B11.10
..LN1027:
        movl      20(%ebp), %eax                                #1219.13
        movl      $22, (%eax)                                   #1219.13
        jmp       ..B11.17      # Prob 100%                     #1219.13
                                # LOE
..B11.12:                       # Preds ..B11.10
..LN1029:
        movl      -12(%ebp), %eax                               #1220.19
..LN1031:
        testl     %eax, %eax                                    #1220.27
        jne       ..B11.15      # Prob 50%                      #1220.27
                                # LOE
..B11.13:                       # Preds ..B11.12
..LN1033:
        addl      $-16, %esp                                    #1221.24
..LN1035:
        movl      16(%ebp), %eax                                #1221.47
        movl      %eax, (%esp)                                  #1221.47
..LN1037:
        movl      24(%ebp), %eax                                #1221.53
        movl      %eax, 4(%esp)                                 #1221.53
..LN1039:
        movl      -16(%ebp), %eax                               #1221.65
        movl      %eax, 8(%esp)                                 #1221.65
..LN1041:
        movl      -16(%ebp), %eax                               #1221.76
        negl      %eax                                          #1221.76
        addl      8(%ebp), %eax                                 #1221.76
        movl      %eax, 12(%esp)                                #1221.76
..LN1043:
        call      NucStrFastaExprMake2                          #1221.24
                                # LOE eax
..B11.20:                       # Preds ..B11.13
        addl      $16, %esp                                     #1221.24
        movl      %eax, -24(%ebp)                               #1221.24
                                # LOE
..B11.14:                       # Preds ..B11.20
..LN1045:
        movl      20(%ebp), %eax                                #1221.13
        movl      -24(%ebp), %edx                               #1221.13
        movl      %edx, (%eax)                                  #1221.13
        jmp       ..B11.17      # Prob 100%                     #1221.13
                                # LOE
..B11.15:                       # Preds ..B11.12
..LN1047:
        addl      $-16, %esp                                    #1223.24
..LN1049:
        movl      16(%ebp), %eax                                #1223.47
        movl      %eax, (%esp)                                  #1223.47
..LN1051:
        movl      24(%ebp), %eax                                #1223.53
        movl      %eax, 4(%esp)                                 #1223.53
..LN1053:
        movl      -16(%ebp), %eax                               #1223.65
        movl      %eax, 8(%esp)                                 #1223.65
..LN1055:
        movl      -16(%ebp), %eax                               #1223.76
        negl      %eax                                          #1223.76
        addl      8(%ebp), %eax                                 #1223.76
        movl      %eax, 12(%esp)                                #1223.76
..LN1057:
        call      NucStrFastaExprMake4                          #1223.24
                                # LOE eax
..B11.21:                       # Preds ..B11.15
        addl      $16, %esp                                     #1223.24
        movl      %eax, -20(%ebp)                               #1223.24
                                # LOE
..B11.16:                       # Preds ..B11.21
..LN1059:
        movl      20(%ebp), %eax                                #1223.13
        movl      -20(%ebp), %edx                               #1223.13
        movl      %edx, (%eax)                                  #1223.13
                                # LOE
..B11.17:                       # Preds ..B11.2 ..B11.11 ..B11.14 ..B11.16
..LN1061:
        movl      8(%ebp), %eax                                 #1226.12
        leave                                                   #1226.12
..___tag_value_nss_FASTA_expr.192:                              #
        ret                                                     #1226.12
        .align    2,0x90
..___tag_value_nss_FASTA_expr.193:                              #
                                # LOE
# mark_end;
	.type	nss_FASTA_expr,@function
	.size	nss_FASTA_expr,.-nss_FASTA_expr
.LNnss_FASTA_expr:
	.data
# -- End  nss_FASTA_expr
	.section .rodata, "a"
	.align 4
__$U1:
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
	.type	__$U1,@object
	.size	__$U1,15
	.space 1	# pad
_2__STRING.3:
	.byte	112
	.byte	32
	.byte	60
	.byte	32
	.byte	101
	.byte	110
	.byte	100
	.byte	0
	.type	_2__STRING.3,@object
	.size	_2__STRING.3,8
	.text
# -- Begin  nss_fasta_expr
# mark_begin;
       .align    2,0x90
nss_fasta_expr:
# parameter 1(p): 8 + %ebp
# parameter 2(end): 12 + %ebp
# parameter 3(expr): 16 + %ebp
# parameter 4(status): 20 + %ebp
# parameter 5(positional): 24 + %ebp
..B12.1:                        # Preds ..B12.0
..___tag_value_nss_fasta_expr.196:                              #
..LN1063:
        pushl     %ebp                                          #1234.1
        movl      %esp, %ebp                                    #1234.1
..___tag_value_nss_fasta_expr.201:                              #
        subl      $28, %esp                                     #1234.1
        movl      %ebx, -4(%ebp)                                #1234.1
..___tag_value_nss_fasta_expr.204:                              #
..LN1065:
        movl      8(%ebp), %eax                                 #1235.5
        movl      12(%ebp), %edx                                #1235.5
        cmpl      %edx, %eax                                    #1235.5
        jb        ..B12.3       # Prob 50%                      #1235.5
..___tag_value_nss_fasta_expr.205:                              #
                                # LOE
..B12.2:                        # Preds ..B12.1
        call      ..L206        # Prob 100%                     #
..L206:                                                         #
        popl      %eax                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L206], %eax     #
        addl      $-16, %esp                                    #1235.5
        lea       _2__STRING.3@GOTOFF(%eax), %edx               #1235.5
        movl      %edx, (%esp)                                  #1235.5
        lea       _2__STRING.1@GOTOFF(%eax), %edx               #1235.5
        movl      %edx, 4(%esp)                                 #1235.5
        movl      $1235, 8(%esp)                                #1235.5
        lea       __$U1@GOTOFF(%eax), %edx                      #1235.5
        movl      %edx, 12(%esp)                                #1235.5
        movl      %eax, %ebx                                    #1235.5
        call      __assert_fail@PLT                             #1235.5
                                # LOE
..B12.26:                       # Preds ..B12.2
        addl      $16, %esp                                     #1235.5
..___tag_value_nss_fasta_expr.207:                              #
                                # LOE
..B12.3:                        # Preds ..B12.1
..LN1067:
        movl      8(%ebp), %eax                                 #1236.16
        movzbl    (%eax), %eax                                  #1236.16
        movsbl    %al, %eax                                     #1236.16
        movl      %eax, -8(%ebp)                                #1236.16
        cmpl      $39, %eax                                     #1236.16
        je        ..B12.5       # Prob 50%                      #1236.16
                                # LOE
..B12.4:                        # Preds ..B12.3
        movl      -8(%ebp), %eax                                #1236.16
        cmpl      $34, %eax                                     #1236.16
        je        ..B12.13      # Prob 50%                      #1236.16
        jmp       ..B12.21      # Prob 100%                     #1236.16
                                # LOE
..B12.5:                        # Preds ..B12.3
..LN1069:
        addl      $-20, %esp                                    #1239.13
..LN1071:
        movl      8(%ebp), %eax                                 #1239.30
        incl      %eax                                          #1239.30
        movl      %eax, (%esp)                                  #1239.30
..LN1073:
        movl      12(%ebp), %eax                                #1239.37
        movl      %eax, 4(%esp)                                 #1239.37
..LN1075:
        movl      16(%ebp), %eax                                #1239.42
        movl      %eax, 8(%esp)                                 #1239.42
..LN1077:
        movl      20(%ebp), %eax                                #1239.48
        movl      %eax, 12(%esp)                                #1239.48
..LN1079:
        movl      24(%ebp), %eax                                #1239.56
        movl      %eax, 16(%esp)                                #1239.56
..LN1081:
        call      nss_FASTA_expr                                #1239.13
                                # LOE eax
..B12.27:                       # Preds ..B12.5
        addl      $20, %esp                                     #1239.13
        movl      %eax, -12(%ebp)                               #1239.13
                                # LOE
..B12.6:                        # Preds ..B12.27
..LN1083:
        movl      -12(%ebp), %eax                               #1239.9
        movl      %eax, 8(%ebp)                                 #1239.9
..LN1085:
        movl      20(%ebp), %eax                                #1240.16
        movl      (%eax), %eax                                  #1240.16
..LN1087:
        testl     %eax, %eax                                    #1240.26
        jne       ..B12.23      # Prob 50%                      #1240.26
                                # LOE
..B12.7:                        # Preds ..B12.6
..LN1089:
        movl      8(%ebp), %eax                                 #1240.33
..LN1091:
        movl      12(%ebp), %edx                                #1240.38
        cmpl      %edx, %eax                                    #1240.38
        je        ..B12.12      # Prob 50%                      #1240.38
                                # LOE
..B12.8:                        # Preds ..B12.7
..LN1093:
        movl      8(%ebp), %eax                                 #1240.47
        movzbl    (%eax), %eax                                  #1240.47
        movsbl    %al, %eax                                     #1240.47
..LN1095:
        cmpl      $39, %eax                                     #1240.55
        jne       ..B12.10      # Prob 50%                      #1240.55
                                # LOE
..B12.9:                        # Preds ..B12.8
        movl      $0, -24(%ebp)                                 #1240.55
        jmp       ..B12.11      # Prob 100%                     #1240.55
                                # LOE
..B12.10:                       # Preds ..B12.8
        movl      $1, -24(%ebp)                                 #1240.55
                                # LOE
..B12.11:                       # Preds ..B12.10 ..B12.9
..LN1097:
        movl      -24(%ebp), %eax                               #1240.47
        incl      8(%ebp)                                       #1240.47
..LN1099:
        testl     %eax, %eax                                    #1240.55
        je        ..B12.23      # Prob 100%                     #1240.55
                                # LOE
..B12.12:                       # Preds ..B12.7 ..B12.11
..LN1101:
        movl      20(%ebp), %eax                                #1241.13
        movl      $22, (%eax)                                   #1241.13
        jmp       ..B12.23      # Prob 100%                     #1241.13
                                # LOE
..B12.13:                       # Preds ..B12.4
..LN1103:
        addl      $-20, %esp                                    #1244.13
..LN1105:
        movl      8(%ebp), %eax                                 #1244.30
        incl      %eax                                          #1244.30
        movl      %eax, (%esp)                                  #1244.30
..LN1107:
        movl      12(%ebp), %eax                                #1244.37
        movl      %eax, 4(%esp)                                 #1244.37
..LN1109:
        movl      16(%ebp), %eax                                #1244.42
        movl      %eax, 8(%esp)                                 #1244.42
..LN1111:
        movl      20(%ebp), %eax                                #1244.48
        movl      %eax, 12(%esp)                                #1244.48
..LN1113:
        movl      24(%ebp), %eax                                #1244.56
        movl      %eax, 16(%esp)                                #1244.56
..LN1115:
        call      nss_FASTA_expr                                #1244.13
                                # LOE eax
..B12.28:                       # Preds ..B12.13
        addl      $20, %esp                                     #1244.13
        movl      %eax, -20(%ebp)                               #1244.13
                                # LOE
..B12.14:                       # Preds ..B12.28
..LN1117:
        movl      -20(%ebp), %eax                               #1244.9
        movl      %eax, 8(%ebp)                                 #1244.9
..LN1119:
        movl      20(%ebp), %eax                                #1245.16
        movl      (%eax), %eax                                  #1245.16
..LN1121:
        testl     %eax, %eax                                    #1245.26
        jne       ..B12.23      # Prob 50%                      #1245.26
                                # LOE
..B12.15:                       # Preds ..B12.14
..LN1123:
        movl      8(%ebp), %eax                                 #1245.33
..LN1125:
        movl      12(%ebp), %edx                                #1245.38
        cmpl      %edx, %eax                                    #1245.38
        je        ..B12.20      # Prob 50%                      #1245.38
                                # LOE
..B12.16:                       # Preds ..B12.15
..LN1127:
        movl      8(%ebp), %eax                                 #1245.47
        movzbl    (%eax), %eax                                  #1245.47
        movsbl    %al, %eax                                     #1245.47
..LN1129:
        cmpl      $34, %eax                                     #1245.55
        jne       ..B12.18      # Prob 50%                      #1245.55
                                # LOE
..B12.17:                       # Preds ..B12.16
        movl      $0, -28(%ebp)                                 #1245.55
        jmp       ..B12.19      # Prob 100%                     #1245.55
                                # LOE
..B12.18:                       # Preds ..B12.16
        movl      $1, -28(%ebp)                                 #1245.55
                                # LOE
..B12.19:                       # Preds ..B12.18 ..B12.17
..LN1131:
        movl      -28(%ebp), %eax                               #1245.47
        incl      8(%ebp)                                       #1245.47
..LN1133:
        testl     %eax, %eax                                    #1245.55
        je        ..B12.23      # Prob 100%                     #1245.55
                                # LOE
..B12.20:                       # Preds ..B12.15 ..B12.19
..LN1135:
        movl      20(%ebp), %eax                                #1246.13
        movl      $22, (%eax)                                   #1246.13
        jmp       ..B12.23      # Prob 100%                     #1246.13
                                # LOE
..B12.21:                       # Preds ..B12.4
..LN1137:
        addl      $-20, %esp                                    #1249.16
..LN1139:
        movl      8(%ebp), %eax                                 #1249.33
        movl      %eax, (%esp)                                  #1249.33
..LN1141:
        movl      12(%ebp), %eax                                #1249.36
        movl      %eax, 4(%esp)                                 #1249.36
..LN1143:
        movl      16(%ebp), %eax                                #1249.41
        movl      %eax, 8(%esp)                                 #1249.41
..LN1145:
        movl      20(%ebp), %eax                                #1249.47
        movl      %eax, 12(%esp)                                #1249.47
..LN1147:
        movl      24(%ebp), %eax                                #1249.55
        movl      %eax, 16(%esp)                                #1249.55
..LN1149:
        call      nss_FASTA_expr                                #1249.16
                                # LOE eax
..B12.29:                       # Preds ..B12.21
        addl      $20, %esp                                     #1249.16
        movl      %eax, -16(%ebp)                               #1249.16
                                # LOE
..B12.22:                       # Preds ..B12.29
        movl      -16(%ebp), %eax                               #1249.16
        movl      -4(%ebp), %ebx                                #1249.16
..___tag_value_nss_fasta_expr.208:                              #
        leave                                                   #1249.16
..___tag_value_nss_fasta_expr.210:                              #
        ret                                                     #1249.16
..___tag_value_nss_fasta_expr.211:                              #
                                # LOE
..B12.23:                       # Preds ..B12.12 ..B12.11 ..B12.6 ..B12.20 ..B12.19
                                #       ..B12.14
..LN1151:
        movl      8(%ebp), %eax                                 #1252.12
        movl      -4(%ebp), %ebx                                #1252.12
..___tag_value_nss_fasta_expr.213:                              #
        leave                                                   #1252.12
..___tag_value_nss_fasta_expr.215:                              #
        ret                                                     #1252.12
        .align    2,0x90
..___tag_value_nss_fasta_expr.216:                              #
                                # LOE
# mark_end;
	.type	nss_fasta_expr,@function
	.size	nss_fasta_expr,.-nss_fasta_expr
.LNnss_fasta_expr:
	.data
# -- End  nss_fasta_expr
	.section .rodata, "a"
	.align 4
__$U2:
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
	.type	__$U2,@object
	.size	__$U2,17
	.text
# -- Begin  nss_primary_expr
# mark_begin;
       .align    2,0x90
nss_primary_expr:
# parameter 1(p): 8 + %ebp
# parameter 2(end): 12 + %ebp
# parameter 3(expr): 16 + %ebp
# parameter 4(status): 20 + %ebp
# parameter 5(positional): 24 + %ebp
..B13.1:                        # Preds ..B13.0
..___tag_value_nss_primary_expr.219:                            #
..LN1153:
        pushl     %ebp                                          #1291.1
        movl      %esp, %ebp                                    #1291.1
..___tag_value_nss_primary_expr.224:                            #
        subl      $52, %esp                                     #1291.1
        movl      %ebx, -4(%ebp)                                #1291.1
..___tag_value_nss_primary_expr.227:                            #
        call      ..L228        # Prob 100%                     #1291.1
..L228:                                                         #
        popl      %eax                                          #1291.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L228], %eax     #1291.1
        movl      %eax, -8(%ebp)                                #1291.1
..LN1155:
        movl      8(%ebp), %eax                                 #1294.5
        movl      12(%ebp), %edx                                #1294.5
        cmpl      %edx, %eax                                    #1294.5
        jb        ..B13.3       # Prob 50%                      #1294.5
..___tag_value_nss_primary_expr.229:                            #
                                # LOE
..B13.2:                        # Preds ..B13.1
        addl      $-16, %esp                                    #1294.5
        movl      -8(%ebp), %eax                                #1294.5
        lea       _2__STRING.3@GOTOFF(%eax), %eax               #1294.5
        movl      %eax, (%esp)                                  #1294.5
        movl      -8(%ebp), %eax                                #1294.5
        lea       _2__STRING.1@GOTOFF(%eax), %eax               #1294.5
        movl      %eax, 4(%esp)                                 #1294.5
        movl      $1294, 8(%esp)                                #1294.5
        movl      -8(%ebp), %eax                                #1294.5
        lea       __$U2@GOTOFF(%eax), %eax                      #1294.5
        movl      %eax, 12(%esp)                                #1294.5
        movl      -8(%ebp), %eax                                #1294.5
        movl      %eax, %ebx                                    #1294.5
        call      __assert_fail@PLT                             #1294.5
                                # LOE
..B13.38:                       # Preds ..B13.2
        addl      $16, %esp                                     #1294.5
..___tag_value_nss_primary_expr.230:                            #
                                # LOE
..B13.3:                        # Preds ..B13.1
..LN1157:
        movl      8(%ebp), %eax                                 #1295.16
        movzbl    (%eax), %eax                                  #1295.16
        movsbl    %al, %eax                                     #1295.16
        movl      %eax, -12(%ebp)                               #1295.16
        cmpl      $94, %eax                                     #1295.16
        je        ..B13.5       # Prob 50%                      #1295.16
                                # LOE
..B13.4:                        # Preds ..B13.3
        movl      -12(%ebp), %eax                               #1295.16
        cmpl      $40, %eax                                     #1295.16
        je        ..B13.12      # Prob 50%                      #1295.16
        jmp       ..B13.25      # Prob 100%                     #1295.16
                                # LOE
..B13.5:                        # Preds ..B13.3
..LN1159:
        pushl     %edi                                          #1298.13
..LN1161:
        movl      $12, (%esp)                                   #1298.22
..LN1163:
        movl      -8(%ebp), %eax                                #1298.13
        movl      %eax, %ebx                                    #1298.13
        call      malloc@PLT                                    #1298.13
                                # LOE eax
..B13.39:                       # Preds ..B13.5
        popl      %ecx                                          #1298.13
        movl      %eax, -16(%ebp)                               #1298.13
                                # LOE
..B13.6:                        # Preds ..B13.39
..LN1165:
        movl      -16(%ebp), %eax                               #1298.9
        movl      %eax, -20(%ebp)                               #1298.9
..LN1167:
        movl      -20(%ebp), %eax                               #1299.14
..LN1169:
        testl     %eax, %eax                                    #1299.19
        jne       ..B13.8       # Prob 50%                      #1299.19
                                # LOE
..B13.7:                        # Preds ..B13.6
..LN1171:
        movl      -8(%ebp), %eax                                #1300.24
        movl      %eax, %ebx                                    #1300.24
        call      __errno_location@PLT                          #1300.24
                                # LOE eax
..B13.40:                       # Preds ..B13.7
..LN1173:
        movl      20(%ebp), %edx                                #1300.13
..LN1175:
        movl      (%eax), %eax                                  #1300.24
..LN1177:
        movl      %eax, (%edx)                                  #1300.13
        jmp       ..B13.11      # Prob 100%                     #1300.13
                                # LOE
..B13.8:                        # Preds ..B13.6
..LN1179:
        movl      -20(%ebp), %eax                               #1303.13
        movl      $12, (%eax)                                   #1303.13
..LN1181:
        movl      -20(%ebp), %eax                               #1304.13
        movl      $14, 4(%eax)                                  #1304.13
..LN1183:
        movl      -20(%ebp), %eax                               #1305.13
        movl      $0, 8(%eax)                                   #1305.13
..LN1185:
        movl      16(%ebp), %eax                                #1306.13
..LN1187:
        movl      -20(%ebp), %edx                               #1306.22
..LN1189:
        movl      %edx, (%eax)                                  #1306.13
..LN1191:
        addl      $-8, %esp                                     #1308.17
..LN1193:
        movl      8(%ebp), %eax                                 #1308.27
        incl      %eax                                          #1308.27
        movl      %eax, (%esp)                                  #1308.27
..LN1195:
        movl      12(%ebp), %eax                                #1308.34
        movl      %eax, 4(%esp)                                 #1308.34
..LN1197:
        call      nss_sob                                       #1308.17
                                # LOE eax
..B13.41:                       # Preds ..B13.8
        addl      $8, %esp                                      #1308.17
        movl      %eax, -36(%ebp)                               #1308.17
                                # LOE
..B13.9:                        # Preds ..B13.41
..LN1199:
        movl      -36(%ebp), %eax                               #1308.13
        movl      %eax, 8(%ebp)                                 #1308.13
..LN1201:
        addl      $-20, %esp                                    #1309.17
        movl      8(%ebp), %eax                                 #1309.17
        movl      %eax, (%esp)                                  #1309.17
        movl      12(%ebp), %eax                                #1309.17
        movl      %eax, 4(%esp)                                 #1309.17
        movl      -20(%ebp), %eax                               #1309.17
        addl      $8, %eax                                      #1309.17
        movl      %eax, 8(%esp)                                 #1309.17
        movl      20(%ebp), %eax                                #1309.17
        movl      %eax, 12(%esp)                                #1309.17
        movl      24(%ebp), %eax                                #1309.17
        movl      %eax, 16(%esp)                                #1309.17
        call      nss_fasta_expr                                #1309.17
                                # LOE eax
..B13.42:                       # Preds ..B13.9
        addl      $20, %esp                                     #1309.17
        movl      %eax, -32(%ebp)                               #1309.17
                                # LOE
..B13.10:                       # Preds ..B13.42
..LN1203:
        movl      -32(%ebp), %eax                               #1309.13
        movl      %eax, 8(%ebp)                                 #1309.13
                                # LOE
..B13.11:                       # Preds ..B13.40 ..B13.10
..LN1205:
        movl      8(%ebp), %eax                                 #1311.16
        movl      -4(%ebp), %ebx                                #1311.16
..___tag_value_nss_primary_expr.231:                            #
        leave                                                   #1311.16
..___tag_value_nss_primary_expr.233:                            #
        ret                                                     #1311.16
..___tag_value_nss_primary_expr.234:                            #
                                # LOE
..B13.12:                       # Preds ..B13.4
..LN1207:
        pushl     %edi                                          #1313.13
..LN1209:
        movl      $12, (%esp)                                   #1313.22
..LN1211:
        movl      -8(%ebp), %eax                                #1313.13
        movl      %eax, %ebx                                    #1313.13
        call      malloc@PLT                                    #1313.13
                                # LOE eax
..B13.43:                       # Preds ..B13.12
        popl      %ecx                                          #1313.13
        movl      %eax, -28(%ebp)                               #1313.13
                                # LOE
..B13.13:                       # Preds ..B13.43
..LN1213:
        movl      -28(%ebp), %eax                               #1313.9
        movl      %eax, -20(%ebp)                               #1313.9
..LN1215:
        movl      -20(%ebp), %eax                               #1314.14
..LN1217:
        testl     %eax, %eax                                    #1314.19
        jne       ..B13.15      # Prob 50%                      #1314.19
                                # LOE
..B13.14:                       # Preds ..B13.13
..LN1219:
        movl      -8(%ebp), %eax                                #1315.24
        movl      %eax, %ebx                                    #1315.24
        call      __errno_location@PLT                          #1315.24
                                # LOE eax
..B13.44:                       # Preds ..B13.14
..LN1221:
        movl      20(%ebp), %edx                                #1315.13
..LN1223:
        movl      (%eax), %eax                                  #1315.24
..LN1225:
        movl      %eax, (%edx)                                  #1315.13
        jmp       ..B13.24      # Prob 100%                     #1315.13
                                # LOE
..B13.15:                       # Preds ..B13.13
..LN1227:
        movl      -20(%ebp), %eax                               #1318.13
        movl      $12, (%eax)                                   #1318.13
..LN1229:
        movl      -20(%ebp), %eax                               #1319.13
        movl      $0, 4(%eax)                                   #1319.13
..LN1231:
        movl      16(%ebp), %eax                                #1320.13
..LN1233:
        movl      -20(%ebp), %edx                               #1320.22
..LN1235:
        movl      %edx, (%eax)                                  #1320.13
..LN1237:
        addl      $-20, %esp                                    #1322.17
..LN1239:
        movl      8(%ebp), %eax                                 #1322.28
        incl      %eax                                          #1322.28
        movl      %eax, (%esp)                                  #1322.28
..LN1241:
        movl      12(%ebp), %eax                                #1322.35
        movl      %eax, 4(%esp)                                 #1322.35
..LN1243:
        movl      -20(%ebp), %eax                               #1322.42
        addl      $8, %eax                                      #1322.42
        movl      %eax, 8(%esp)                                 #1322.42
..LN1245:
        movl      20(%ebp), %eax                                #1322.59
        movl      %eax, 12(%esp)                                #1322.59
..LN1247:
        movl      24(%ebp), %eax                                #1322.67
        movl      %eax, 16(%esp)                                #1322.67
..LN1249:
        call      nss_expr                                      #1322.17
                                # LOE eax
..B13.45:                       # Preds ..B13.15
        addl      $20, %esp                                     #1322.17
        movl      %eax, -40(%ebp)                               #1322.17
                                # LOE
..B13.16:                       # Preds ..B13.45
..LN1251:
        movl      -40(%ebp), %eax                               #1322.13
        movl      %eax, 8(%ebp)                                 #1322.13
..LN1253:
        movl      20(%ebp), %eax                                #1323.20
        movl      (%eax), %eax                                  #1323.20
..LN1255:
        testl     %eax, %eax                                    #1323.30
        jne       ..B13.24      # Prob 50%                      #1323.30
                                # LOE
..B13.17:                       # Preds ..B13.16
..LN1257:
        movl      -20(%ebp), %eax                               #1325.22
        movl      8(%eax), %eax                                 #1325.22
..LN1259:
        testl     %eax, %eax                                    #1325.41
        je        ..B13.23      # Prob 50%                      #1325.41
                                # LOE
..B13.18:                       # Preds ..B13.17
..LN1261:
        movl      8(%ebp), %eax                                 #1325.49
..LN1263:
        movl      12(%ebp), %edx                                #1325.54
        cmpl      %edx, %eax                                    #1325.54
        je        ..B13.23      # Prob 50%                      #1325.54
                                # LOE
..B13.19:                       # Preds ..B13.18
..LN1265:
        movl      8(%ebp), %eax                                 #1325.63
        movzbl    (%eax), %eax                                  #1325.63
        movsbl    %al, %eax                                     #1325.63
..LN1267:
        cmpl      $41, %eax                                     #1325.71
        jne       ..B13.21      # Prob 50%                      #1325.71
                                # LOE
..B13.20:                       # Preds ..B13.19
        movl      $0, -52(%ebp)                                 #1325.71
        jmp       ..B13.22      # Prob 100%                     #1325.71
                                # LOE
..B13.21:                       # Preds ..B13.19
        movl      $1, -52(%ebp)                                 #1325.71
                                # LOE
..B13.22:                       # Preds ..B13.21 ..B13.20
..LN1269:
        movl      -52(%ebp), %eax                               #1325.63
        incl      8(%ebp)                                       #1325.63
..LN1271:
        testl     %eax, %eax                                    #1325.71
        je        ..B13.24      # Prob 100%                     #1325.71
                                # LOE
..B13.23:                       # Preds ..B13.17 ..B13.18 ..B13.22
..LN1273:
        movl      20(%ebp), %eax                                #1326.21
        movl      $22, (%eax)                                   #1326.21
                                # LOE
..B13.24:                       # Preds ..B13.44 ..B13.23 ..B13.22 ..B13.16
..LN1275:
        movl      8(%ebp), %eax                                 #1329.16
        movl      -4(%ebp), %ebx                                #1329.16
..___tag_value_nss_primary_expr.236:                            #
        leave                                                   #1329.16
..___tag_value_nss_primary_expr.238:                            #
        ret                                                     #1329.16
..___tag_value_nss_primary_expr.239:                            #
                                # LOE
..B13.25:                       # Preds ..B13.4
..LN1277:
        addl      $-20, %esp                                    #1332.9
        movl      8(%ebp), %eax                                 #1332.9
        movl      %eax, (%esp)                                  #1332.9
        movl      12(%ebp), %eax                                #1332.9
        movl      %eax, 4(%esp)                                 #1332.9
        movl      16(%ebp), %eax                                #1332.9
        movl      %eax, 8(%esp)                                 #1332.9
        movl      20(%ebp), %eax                                #1332.9
        movl      %eax, 12(%esp)                                #1332.9
        movl      24(%ebp), %eax                                #1332.9
        movl      %eax, 16(%esp)                                #1332.9
        call      nss_fasta_expr                                #1332.9
                                # LOE eax
..B13.46:                       # Preds ..B13.25
        addl      $20, %esp                                     #1332.9
        movl      %eax, -24(%ebp)                               #1332.9
                                # LOE
..B13.26:                       # Preds ..B13.46
..LN1279:
        movl      -24(%ebp), %eax                               #1332.5
        movl      %eax, 8(%ebp)                                 #1332.5
..LN1281:
        movl      20(%ebp), %eax                                #1333.12
        movl      (%eax), %eax                                  #1333.12
..LN1283:
        testl     %eax, %eax                                    #1333.22
        jne       ..B13.35      # Prob 50%                      #1333.22
                                # LOE
..B13.27:                       # Preds ..B13.26
..LN1285:
        movl      8(%ebp), %eax                                 #1333.27
..LN1287:
        movl      12(%ebp), %edx                                #1333.31
        cmpl      %edx, %eax                                    #1333.31
        jae       ..B13.35      # Prob 50%                      #1333.31
                                # LOE
..B13.28:                       # Preds ..B13.27
..LN1289:
        addl      $-8, %esp                                     #1335.13
..LN1291:
        movl      8(%ebp), %eax                                 #1335.23
        movl      %eax, (%esp)                                  #1335.23
..LN1293:
        movl      12(%ebp), %eax                                #1335.26
        movl      %eax, 4(%esp)                                 #1335.26
..LN1295:
        call      nss_sob                                       #1335.13
                                # LOE eax
..B13.47:                       # Preds ..B13.28
        addl      $8, %esp                                      #1335.13
        movl      %eax, -44(%ebp)                               #1335.13
                                # LOE
..B13.29:                       # Preds ..B13.47
..LN1297:
        movl      -44(%ebp), %eax                               #1335.9
        movl      %eax, 8(%ebp)                                 #1335.9
..LN1299:
        movl      8(%ebp), %eax                                 #1336.14
..LN1301:
        movl      12(%ebp), %edx                                #1336.18
        cmpl      %edx, %eax                                    #1336.18
        jae       ..B13.35      # Prob 50%                      #1336.18
                                # LOE
..B13.30:                       # Preds ..B13.29
..LN1303:
        movl      8(%ebp), %eax                                 #1336.27
        movzbl    (%eax), %eax                                  #1336.27
        movsbl    %al, %eax                                     #1336.27
..LN1305:
        cmpl      $36, %eax                                     #1336.32
        jne       ..B13.35      # Prob 50%                      #1336.32
                                # LOE
..B13.31:                       # Preds ..B13.30
..LN1307:
        incl      8(%ebp)                                       #1338.16
..LN1309:
        pushl     %edi                                          #1340.17
..LN1311:
        movl      $12, (%esp)                                   #1340.26
..LN1313:
        movl      -8(%ebp), %eax                                #1340.17
        movl      %eax, %ebx                                    #1340.17
        call      malloc@PLT                                    #1340.17
                                # LOE eax
..B13.48:                       # Preds ..B13.31
        popl      %ecx                                          #1340.17
        movl      %eax, -48(%ebp)                               #1340.17
                                # LOE
..B13.32:                       # Preds ..B13.48
..LN1315:
        movl      -48(%ebp), %eax                               #1340.13
        movl      %eax, -20(%ebp)                               #1340.13
..LN1317:
        movl      -20(%ebp), %eax                               #1341.18
..LN1319:
        testl     %eax, %eax                                    #1341.23
        jne       ..B13.34      # Prob 50%                      #1341.23
                                # LOE
..B13.33:                       # Preds ..B13.32
..LN1321:
        movl      -8(%ebp), %eax                                #1342.28
        movl      %eax, %ebx                                    #1342.28
        call      __errno_location@PLT                          #1342.28
                                # LOE eax
..B13.49:                       # Preds ..B13.33
..LN1323:
        movl      20(%ebp), %edx                                #1342.17
..LN1325:
        movl      (%eax), %eax                                  #1342.28
..LN1327:
        movl      %eax, (%edx)                                  #1342.17
        jmp       ..B13.35      # Prob 100%                     #1342.17
                                # LOE
..B13.34:                       # Preds ..B13.32
..LN1329:
        movl      -20(%ebp), %eax                               #1345.17
        movl      $12, (%eax)                                   #1345.17
..LN1331:
        movl      -20(%ebp), %eax                               #1346.17
        movl      $15, 4(%eax)                                  #1346.17
..LN1333:
        movl      16(%ebp), %eax                                #1347.37
..LN1335:
        movl      -20(%ebp), %edx                               #1347.17
..LN1337:
        movl      (%eax), %eax                                  #1347.37
..LN1339:
        movl      %eax, 8(%edx)                                 #1347.17
..LN1341:
        movl      16(%ebp), %eax                                #1348.17
..LN1343:
        movl      -20(%ebp), %edx                               #1348.26
..LN1345:
        movl      %edx, (%eax)                                  #1348.17
                                # LOE
..B13.35:                       # Preds ..B13.49 ..B13.34 ..B13.30 ..B13.29 ..B13.27
                                #       ..B13.26
..LN1347:
        movl      8(%ebp), %eax                                 #1353.12
        movl      -4(%ebp), %ebx                                #1353.12
..___tag_value_nss_primary_expr.241:                            #
        leave                                                   #1353.12
..___tag_value_nss_primary_expr.243:                            #
        ret                                                     #1353.12
        .align    2,0x90
..___tag_value_nss_primary_expr.244:                            #
                                # LOE
# mark_end;
	.type	nss_primary_expr,@function
	.size	nss_primary_expr,.-nss_primary_expr
.LNnss_primary_expr:
	.data
# -- End  nss_primary_expr
	.section .rodata, "a"
	.align 4
__$U3:
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
	.type	__$U3,@object
	.size	__$U3,15
	.space 1	# pad
_2__STRING.4:
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
	.type	_2__STRING.4,@object
	.size	_2__STRING.4,48
	.text
# -- Begin  nss_unary_expr
# mark_begin;
       .align    2,0x90
nss_unary_expr:
# parameter 1(p): 8 + %ebp
# parameter 2(end): 12 + %ebp
# parameter 3(expr): 16 + %ebp
# parameter 4(status): 20 + %ebp
# parameter 5(positional): 24 + %ebp
..B14.1:                        # Preds ..B14.0
..___tag_value_nss_unary_expr.247:                              #
..LN1349:
        pushl     %ebp                                          #1361.1
        movl      %esp, %ebp                                    #1361.1
..___tag_value_nss_unary_expr.252:                              #
        subl      $28, %esp                                     #1361.1
        movl      %ebx, -4(%ebp)                                #1361.1
..___tag_value_nss_unary_expr.255:                              #
        call      ..L256        # Prob 100%                     #1361.1
..L256:                                                         #
        popl      %eax                                          #1361.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L256], %eax     #1361.1
        movl      %eax, -8(%ebp)                                #1361.1
..LN1351:
        movl      8(%ebp), %eax                                 #1362.5
        movl      12(%ebp), %edx                                #1362.5
        cmpl      %edx, %eax                                    #1362.5
        jb        ..B14.3       # Prob 50%                      #1362.5
..___tag_value_nss_unary_expr.257:                              #
                                # LOE
..B14.2:                        # Preds ..B14.1
        addl      $-16, %esp                                    #1362.5
        movl      -8(%ebp), %eax                                #1362.5
        lea       _2__STRING.3@GOTOFF(%eax), %eax               #1362.5
        movl      %eax, (%esp)                                  #1362.5
        movl      -8(%ebp), %eax                                #1362.5
        lea       _2__STRING.1@GOTOFF(%eax), %eax               #1362.5
        movl      %eax, 4(%esp)                                 #1362.5
        movl      $1362, 8(%esp)                                #1362.5
        movl      -8(%ebp), %eax                                #1362.5
        lea       __$U3@GOTOFF(%eax), %eax                      #1362.5
        movl      %eax, 12(%esp)                                #1362.5
        movl      -8(%ebp), %eax                                #1362.5
        movl      %eax, %ebx                                    #1362.5
        call      __assert_fail@PLT                             #1362.5
                                # LOE
..B14.19:                       # Preds ..B14.2
        addl      $16, %esp                                     #1362.5
..___tag_value_nss_unary_expr.258:                              #
                                # LOE
..B14.3:                        # Preds ..B14.1
..LN1353:
        movl      8(%ebp), %eax                                 #1363.12
        movzbl    (%eax), %eax                                  #1363.12
        movsbl    %al, %eax                                     #1363.12
..LN1355:
        cmpl      $33, %eax                                     #1363.17
        je        ..B14.6       # Prob 50%                      #1363.17
                                # LOE
..B14.4:                        # Preds ..B14.3
..LN1357:
        addl      $-20, %esp                                    #1364.16
..LN1359:
        movl      8(%ebp), %eax                                 #1364.35
        movl      %eax, (%esp)                                  #1364.35
..LN1361:
        movl      12(%ebp), %eax                                #1364.38
        movl      %eax, 4(%esp)                                 #1364.38
..LN1363:
        movl      16(%ebp), %eax                                #1364.43
        movl      %eax, 8(%esp)                                 #1364.43
..LN1365:
        movl      20(%ebp), %eax                                #1364.49
        movl      %eax, 12(%esp)                                #1364.49
..LN1367:
        movl      24(%ebp), %eax                                #1364.57
        movl      %eax, 16(%esp)                                #1364.57
..LN1369:
        call      nss_primary_expr                              #1364.16
                                # LOE eax
..B14.20:                       # Preds ..B14.4
        addl      $20, %esp                                     #1364.16
        movl      %eax, -16(%ebp)                               #1364.16
                                # LOE
..B14.5:                        # Preds ..B14.20
        movl      -16(%ebp), %eax                               #1364.16
        movl      -4(%ebp), %ebx                                #1364.16
..___tag_value_nss_unary_expr.259:                              #
        leave                                                   #1364.16
..___tag_value_nss_unary_expr.261:                              #
        ret                                                     #1364.16
..___tag_value_nss_unary_expr.262:                              #
                                # LOE
..B14.6:                        # Preds ..B14.3
..LN1371:
        addl      $-8, %esp                                     #1374.9
..LN1373:
        movl      8(%ebp), %eax                                 #1374.19
        incl      %eax                                          #1374.19
        movl      %eax, (%esp)                                  #1374.19
..LN1375:
        movl      12(%ebp), %eax                                #1374.26
        movl      %eax, 4(%esp)                                 #1374.26
..LN1377:
        call      nss_sob                                       #1374.9
                                # LOE eax
..B14.21:                       # Preds ..B14.6
        addl      $8, %esp                                      #1374.9
        movl      %eax, -12(%ebp)                               #1374.9
                                # LOE
..B14.7:                        # Preds ..B14.21
..LN1379:
        movl      -12(%ebp), %eax                               #1374.5
        movl      %eax, 8(%ebp)                                 #1374.5
..LN1381:
        movl      8(%ebp), %eax                                 #1375.10
..LN1383:
        movl      12(%ebp), %edx                                #1375.15
        cmpl      %edx, %eax                                    #1375.15
        jne       ..B14.9       # Prob 50%                      #1375.15
                                # LOE
..B14.8:                        # Preds ..B14.7
..LN1385:
        movl      20(%ebp), %eax                                #1376.9
        movl      $22, (%eax)                                   #1376.9
        jmp       ..B14.16      # Prob 100%                     #1376.9
                                # LOE
..B14.9:                        # Preds ..B14.7
..LN1387:
        pushl     %edi                                          #1379.25
..LN1389:
        movl      $12, (%esp)                                   #1379.34
..LN1391:
        movl      -8(%ebp), %eax                                #1379.25
        movl      %eax, %ebx                                    #1379.25
        call      malloc@PLT                                    #1379.25
                                # LOE eax
..B14.22:                       # Preds ..B14.9
        popl      %ecx                                          #1379.25
        movl      %eax, -20(%ebp)                               #1379.25
                                # LOE
..B14.10:                       # Preds ..B14.22
..LN1393:
        movl      -20(%ebp), %eax                               #1379.23
        movl      %eax, -24(%ebp)                               #1379.23
..LN1395:
        movl      -24(%ebp), %eax                               #1380.14
..LN1397:
        testl     %eax, %eax                                    #1380.19
        jne       ..B14.12      # Prob 50%                      #1380.19
                                # LOE
..B14.11:                       # Preds ..B14.10
..LN1399:
        movl      -8(%ebp), %eax                                #1381.24
        movl      %eax, %ebx                                    #1381.24
        call      __errno_location@PLT                          #1381.24
                                # LOE eax
..B14.23:                       # Preds ..B14.11
..LN1401:
        movl      20(%ebp), %edx                                #1381.13
..LN1403:
        movl      (%eax), %eax                                  #1381.24
..LN1405:
        movl      %eax, (%edx)                                  #1381.13
        jmp       ..B14.16      # Prob 100%                     #1381.13
                                # LOE
..B14.12:                       # Preds ..B14.10
..LN1407:
        movl      -24(%ebp), %eax                               #1384.13
        movl      $12, (%eax)                                   #1384.13
..LN1409:
        movl      -24(%ebp), %eax                               #1385.13
        movl      $13, 4(%eax)                                  #1385.13
..LN1411:
        movl      -24(%ebp), %eax                               #1386.13
        movl      $0, 8(%eax)                                   #1386.13
..LN1413:
        movl      16(%ebp), %eax                                #1387.13
..LN1415:
        movl      -24(%ebp), %edx                               #1387.22
..LN1417:
        movl      %edx, (%eax)                                  #1387.13
..LN1419:
        addl      $-20, %esp                                    #1389.17
..LN1421:
        movl      8(%ebp), %eax                                 #1389.34
        movl      %eax, (%esp)                                  #1389.34
..LN1423:
        movl      12(%ebp), %eax                                #1389.37
        movl      %eax, 4(%esp)                                 #1389.37
..LN1425:
        movl      -24(%ebp), %eax                               #1389.44
        addl      $8, %eax                                      #1389.44
        movl      %eax, 8(%esp)                                 #1389.44
..LN1427:
        movl      20(%ebp), %eax                                #1389.61
        movl      %eax, 12(%esp)                                #1389.61
..LN1429:
        movl      24(%ebp), %eax                                #1389.69
        movl      %eax, 16(%esp)                                #1389.69
..LN1431:
        call      nss_unary_expr                                #1389.17
                                # LOE eax
..B14.24:                       # Preds ..B14.12
        addl      $20, %esp                                     #1389.17
        movl      %eax, -28(%ebp)                               #1389.17
                                # LOE
..B14.13:                       # Preds ..B14.24
..LN1433:
        movl      -28(%ebp), %eax                               #1389.13
        movl      %eax, 8(%ebp)                                 #1389.13
..LN1435:
        movl      20(%ebp), %eax                                #1390.13
        movl      (%eax), %eax                                  #1390.13
        testl     %eax, %eax                                    #1390.13
        jne       ..B14.16      # Prob 50%                      #1390.13
                                # LOE
..B14.14:                       # Preds ..B14.13
        movl      -24(%ebp), %eax                               #1390.13
        movl      8(%eax), %eax                                 #1390.13
        testl     %eax, %eax                                    #1390.13
        jne       ..B14.16      # Prob 50%                      #1390.13
..___tag_value_nss_unary_expr.264:                              #
                                # LOE
..B14.15:                       # Preds ..B14.14
        addl      $-16, %esp                                    #1390.13
        movl      -8(%ebp), %eax                                #1390.13
        lea       _2__STRING.4@GOTOFF(%eax), %eax               #1390.13
        movl      %eax, (%esp)                                  #1390.13
        movl      -8(%ebp), %eax                                #1390.13
        lea       _2__STRING.1@GOTOFF(%eax), %eax               #1390.13
        movl      %eax, 4(%esp)                                 #1390.13
        movl      $1390, 8(%esp)                                #1390.13
        movl      -8(%ebp), %eax                                #1390.13
        lea       __$U3@GOTOFF(%eax), %eax                      #1390.13
        movl      %eax, 12(%esp)                                #1390.13
        movl      -8(%ebp), %eax                                #1390.13
        movl      %eax, %ebx                                    #1390.13
        call      __assert_fail@PLT                             #1390.13
                                # LOE
..B14.25:                       # Preds ..B14.15
        addl      $16, %esp                                     #1390.13
..___tag_value_nss_unary_expr.265:                              #
                                # LOE
..B14.16:                       # Preds ..B14.23 ..B14.8 ..B14.14 ..B14.13
..LN1437:
        movl      8(%ebp), %eax                                 #1393.12
        movl      -4(%ebp), %ebx                                #1393.12
..___tag_value_nss_unary_expr.266:                              #
        leave                                                   #1393.12
..___tag_value_nss_unary_expr.268:                              #
        ret                                                     #1393.12
        .align    2,0x90
..___tag_value_nss_unary_expr.269:                              #
                                # LOE
# mark_end;
	.type	nss_unary_expr,@function
	.size	nss_unary_expr,.-nss_unary_expr
.LNnss_unary_expr:
	.data
# -- End  nss_unary_expr
	.section .rodata, "a"
	.align 4
__$U4:
	.byte	110
	.byte	115
	.byte	115
	.byte	95
	.byte	101
	.byte	120
	.byte	112
	.byte	114
	.byte	0
	.type	__$U4,@object
	.size	__$U4,9
	.space 3	# pad
_2__STRING.5:
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
	.type	_2__STRING.5,@object
	.size	_2__STRING.5,22
	.space 2	# pad
_2__STRING.6:
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
	.type	_2__STRING.6,@object
	.size	_2__STRING.6,53
	.text
# -- Begin  nss_expr
# mark_begin;
       .align    2,0x90
nss_expr:
# parameter 1(p): 8 + %ebp
# parameter 2(end): 12 + %ebp
# parameter 3(expr): 16 + %ebp
# parameter 4(status): 20 + %ebp
# parameter 5(positional): 24 + %ebp
..B15.1:                        # Preds ..B15.0
..___tag_value_nss_expr.272:                                    #
..LN1439:
        pushl     %ebp                                          #1406.1
        movl      %esp, %ebp                                    #1406.1
..___tag_value_nss_expr.277:                                    #
        subl      $48, %esp                                     #1406.1
        movl      %ebx, -4(%ebp)                                #1406.1
..___tag_value_nss_expr.280:                                    #
..LN1441:
        movl      16(%ebp), %eax                                #1407.5
        movl      $0, (%eax)                                    #1407.5
..LN1443:
        addl      $0, %esp                                      #1409.9
..LN1445:
        movl      8(%ebp), %eax                                 #1409.19
        movl      %eax, (%esp)                                  #1409.19
..LN1447:
        movl      12(%ebp), %eax                                #1409.22
        movl      %eax, 4(%esp)                                 #1409.22
..LN1449:
        call      nss_sob                                       #1409.9
                                # LOE eax
..B15.32:                       # Preds ..B15.1
        addl      $8, %esp                                      #1409.9
        movl      %eax, -8(%ebp)                                #1409.9
                                # LOE
..B15.2:                        # Preds ..B15.32
..LN1451:
        movl      -8(%ebp), %eax                                #1409.5
        movl      %eax, 8(%ebp)                                 #1409.5
..LN1453:
        movl      8(%ebp), %eax                                 #1410.10
..LN1455:
        movl      12(%ebp), %edx                                #1410.15
        cmpl      %edx, %eax                                    #1410.15
        je        ..B15.29      # Prob 50%                      #1410.15
                                # LOE
..B15.3:                        # Preds ..B15.2
..LN1457:
        addl      $-20, %esp                                    #1412.13
..LN1459:
        movl      8(%ebp), %eax                                 #1412.30
        movl      %eax, (%esp)                                  #1412.30
..LN1461:
        movl      12(%ebp), %eax                                #1412.33
        movl      %eax, 4(%esp)                                 #1412.33
..LN1463:
        movl      16(%ebp), %eax                                #1412.38
        movl      %eax, 8(%esp)                                 #1412.38
..LN1465:
        movl      20(%ebp), %eax                                #1412.44
        movl      %eax, 12(%esp)                                #1412.44
..LN1467:
        movl      24(%ebp), %eax                                #1412.52
        movl      %eax, 16(%esp)                                #1412.52
..LN1469:
        call      nss_unary_expr                                #1412.13
                                # LOE eax
..B15.33:                       # Preds ..B15.3
        addl      $20, %esp                                     #1412.13
        movl      %eax, -12(%ebp)                               #1412.13
                                # LOE
..B15.4:                        # Preds ..B15.33
..LN1471:
        movl      -12(%ebp), %eax                               #1412.9
        movl      %eax, 8(%ebp)                                 #1412.9
..LN1473:
        movl      20(%ebp), %eax                                #1413.16
        movl      (%eax), %eax                                  #1413.16
..LN1475:
        testl     %eax, %eax                                    #1413.26
        jne       ..B15.29      # Prob 50%                      #1413.26
                                # LOE
..B15.5:                        # Preds ..B15.4
..LN1477:
        addl      $-8, %esp                                     #1415.17
..LN1479:
        movl      8(%ebp), %eax                                 #1415.27
        movl      %eax, (%esp)                                  #1415.27
..LN1481:
        movl      12(%ebp), %eax                                #1415.30
        movl      %eax, 4(%esp)                                 #1415.30
..LN1483:
        call      nss_sob                                       #1415.17
                                # LOE eax
..B15.34:                       # Preds ..B15.5
        addl      $8, %esp                                      #1415.17
        movl      %eax, -16(%ebp)                               #1415.17
                                # LOE
..B15.6:                        # Preds ..B15.34
..LN1485:
        movl      -16(%ebp), %eax                               #1415.13
        movl      %eax, 8(%ebp)                                 #1415.13
..LN1487:
        movl      8(%ebp), %eax                                 #1416.18
..LN1489:
        movl      12(%ebp), %edx                                #1416.23
        cmpl      %edx, %eax                                    #1416.23
        je        ..B15.29      # Prob 50%                      #1416.23
                                # LOE
..B15.7:                        # Preds ..B15.6
        call      ..L281        # Prob 100%                     #
..L281:                                                         #
        popl      %eax                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L281], %eax     #
        movl      %eax, -20(%ebp)                               #
..LN1491:
        movl      16(%ebp), %eax                                #1420.17
        movl      (%eax), %eax                                  #1420.17
        testl     %eax, %eax                                    #1420.17
        jne       ..B15.9       # Prob 50%                      #1420.17
..___tag_value_nss_expr.282:                                    #
                                # LOE
..B15.8:                        # Preds ..B15.7
        addl      $-16, %esp                                    #1420.17
        movl      -20(%ebp), %eax                               #1420.17
        lea       _2__STRING.5@GOTOFF(%eax), %eax               #1420.17
        movl      %eax, (%esp)                                  #1420.17
        movl      -20(%ebp), %eax                               #1420.17
        lea       _2__STRING.1@GOTOFF(%eax), %eax               #1420.17
        movl      %eax, 4(%esp)                                 #1420.17
        movl      $1420, 8(%esp)                                #1420.17
        movl      -20(%ebp), %eax                               #1420.17
        lea       __$U4@GOTOFF(%eax), %eax                      #1420.17
        movl      %eax, 12(%esp)                                #1420.17
        movl      -20(%ebp), %eax                               #1420.17
        movl      %eax, %ebx                                    #1420.17
        call      __assert_fail@PLT                             #1420.17
                                # LOE
..B15.35:                       # Preds ..B15.8
        addl      $16, %esp                                     #1420.17
..___tag_value_nss_expr.283:                                    #
                                # LOE
..B15.9:                        # Preds ..B15.7
..LN1493:
        movl      8(%ebp), %eax                                 #1422.28
        movzbl    (%eax), %eax                                  #1422.28
        movsbl    %al, %eax                                     #1422.28
        movl      %eax, -24(%ebp)                               #1422.28
        incl      8(%ebp)                                       #1422.28
        cmpl      $41, %eax                                     #1422.28
        je        ..B15.12      # Prob 50%                      #1422.28
                                # LOE
..B15.10:                       # Preds ..B15.9
        movl      -24(%ebp), %eax                               #1422.28
        cmpl      $38, %eax                                     #1422.28
        je        ..B15.13      # Prob 50%                      #1422.28
                                # LOE
..B15.11:                       # Preds ..B15.10
        movl      -24(%ebp), %eax                               #1422.28
        cmpl      $124, %eax                                    #1422.28
        je        ..B15.17      # Prob 50%                      #1422.28
        jmp       ..B15.21      # Prob 100%                     #1422.28
                                # LOE
..B15.12:                       # Preds ..B15.9
..LN1495:
        movl      8(%ebp), %eax                                 #1425.28
..LN1497:
        decl      %eax                                          #1425.32
        movl      -4(%ebp), %ebx                                #1425.32
..___tag_value_nss_expr.284:                                    #
        leave                                                   #1425.32
..___tag_value_nss_expr.286:                                    #
        ret                                                     #1425.32
..___tag_value_nss_expr.287:                                    #
                                # LOE
..B15.13:                       # Preds ..B15.10
..LN1499:
        movl      8(%ebp), %eax                                 #1427.26
..LN1501:
        movl      12(%ebp), %edx                                #1427.30
        cmpl      %edx, %eax                                    #1427.30
        jae       ..B15.16      # Prob 50%                      #1427.30
                                # LOE
..B15.14:                       # Preds ..B15.13
..LN1503:
        movl      8(%ebp), %eax                                 #1427.39
        movzbl    (%eax), %eax                                  #1427.39
        movsbl    %al, %eax                                     #1427.39
..LN1505:
        cmpl      $38, %eax                                     #1427.44
        jne       ..B15.16      # Prob 50%                      #1427.44
                                # LOE
..B15.15:                       # Preds ..B15.14
..LN1507:
        incl      8(%ebp)                                       #1428.28
                                # LOE
..B15.16:                       # Preds ..B15.15 ..B15.14 ..B15.13
..LN1509:
        movl      $16, -36(%ebp)                                #1429.21
        jmp       ..B15.22      # Prob 100%                     #1429.21
                                # LOE
..B15.17:                       # Preds ..B15.11
..LN1511:
        movl      8(%ebp), %eax                                 #1432.26
..LN1513:
        movl      12(%ebp), %edx                                #1432.30
        cmpl      %edx, %eax                                    #1432.30
        jae       ..B15.20      # Prob 50%                      #1432.30
                                # LOE
..B15.18:                       # Preds ..B15.17
..LN1515:
        movl      8(%ebp), %eax                                 #1432.39
        movzbl    (%eax), %eax                                  #1432.39
        movsbl    %al, %eax                                     #1432.39
..LN1517:
        cmpl      $124, %eax                                    #1432.44
        jne       ..B15.20      # Prob 50%                      #1432.44
                                # LOE
..B15.19:                       # Preds ..B15.18
..LN1519:
        incl      8(%ebp)                                       #1433.28
                                # LOE
..B15.20:                       # Preds ..B15.19 ..B15.18 ..B15.17
..LN1521:
        movl      $17, -36(%ebp)                                #1434.21
        jmp       ..B15.22      # Prob 100%                     #1434.21
                                # LOE
..B15.21:                       # Preds ..B15.11
..LN1523:
        movl      20(%ebp), %eax                                #1438.21
        movl      $22, (%eax)                                   #1438.21
..LN1525:
        movl      8(%ebp), %eax                                 #1439.28
..LN1527:
        decl      %eax                                          #1439.32
        movl      -4(%ebp), %ebx                                #1439.32
..___tag_value_nss_expr.289:                                    #
        leave                                                   #1439.32
..___tag_value_nss_expr.291:                                    #
        ret                                                     #1439.32
..___tag_value_nss_expr.292:                                    #
                                # LOE
..B15.22:                       # Preds ..B15.16 ..B15.20
..LN1529:
        pushl     %edi                                          #1451.21
..LN1531:
        movl      $16, (%esp)                                   #1451.30
..LN1533:
        movl      -20(%ebp), %eax                               #1451.21
        movl      %eax, %ebx                                    #1451.21
        call      malloc@PLT                                    #1451.21
                                # LOE eax
..B15.36:                       # Preds ..B15.22
        popl      %ecx                                          #1451.21
        movl      %eax, -28(%ebp)                               #1451.21
                                # LOE
..B15.23:                       # Preds ..B15.36
..LN1535:
        movl      -28(%ebp), %eax                               #1451.17
        movl      %eax, -32(%ebp)                               #1451.17
..LN1537:
        movl      -32(%ebp), %eax                               #1452.22
..LN1539:
        testl     %eax, %eax                                    #1452.27
        jne       ..B15.25      # Prob 50%                      #1452.27
                                # LOE
..B15.24:                       # Preds ..B15.23
..LN1541:
        movl      -20(%ebp), %eax                               #1454.32
        movl      %eax, %ebx                                    #1454.32
        call      __errno_location@PLT                          #1454.32
                                # LOE eax
..B15.37:                       # Preds ..B15.24
..LN1543:
        movl      20(%ebp), %edx                                #1454.21
..LN1545:
        movl      (%eax), %eax                                  #1454.32
..LN1547:
        movl      %eax, (%edx)                                  #1454.21
..LN1549:
        movl      8(%ebp), %eax                                 #1455.28
        movl      -4(%ebp), %ebx                                #1455.28
..___tag_value_nss_expr.294:                                    #
        leave                                                   #1455.28
..___tag_value_nss_expr.296:                                    #
        ret                                                     #1455.28
..___tag_value_nss_expr.297:                                    #
                                # LOE
..B15.25:                       # Preds ..B15.23
..LN1551:
        movl      -32(%ebp), %eax                               #1458.17
        movl      $11, (%eax)                                   #1458.17
..LN1553:
        movl      -32(%ebp), %eax                               #1459.17
..LN1555:
        movl      -36(%ebp), %edx                               #1459.37
..LN1557:
        movl      %edx, 4(%eax)                                 #1459.17
..LN1559:
        movl      16(%ebp), %eax                                #1460.41
..LN1561:
        movl      -32(%ebp), %edx                               #1460.17
..LN1563:
        movl      (%eax), %eax                                  #1460.41
..LN1565:
        movl      %eax, 8(%edx)                                 #1460.17
..LN1567:
        movl      16(%ebp), %eax                                #1461.17
..LN1569:
        movl      -32(%ebp), %edx                               #1461.26
..LN1571:
        movl      %edx, (%eax)                                  #1461.17
..LN1573:
        addl      $-20, %esp                                    #1464.21
..LN1575:
        movl      8(%ebp), %eax                                 #1464.32
        movl      %eax, (%esp)                                  #1464.32
..LN1577:
        movl      12(%ebp), %eax                                #1464.35
        movl      %eax, 4(%esp)                                 #1464.35
..LN1579:
        movl      -32(%ebp), %eax                               #1464.42
        addl      $12, %eax                                     #1464.42
        movl      %eax, 8(%esp)                                 #1464.42
..LN1581:
        movl      20(%ebp), %eax                                #1464.64
        movl      %eax, 12(%esp)                                #1464.64
..LN1583:
        movl      24(%ebp), %eax                                #1464.72
        movl      %eax, 16(%esp)                                #1464.72
..LN1585:
        call      nss_expr                                      #1464.21
                                # LOE eax
..B15.38:                       # Preds ..B15.25
        addl      $20, %esp                                     #1464.21
        movl      %eax, -40(%ebp)                               #1464.21
                                # LOE
..B15.26:                       # Preds ..B15.38
..LN1587:
        movl      -40(%ebp), %eax                               #1464.17
        movl      %eax, 8(%ebp)                                 #1464.17
..LN1589:
        movl      20(%ebp), %eax                                #1465.17
        movl      (%eax), %eax                                  #1465.17
        testl     %eax, %eax                                    #1465.17
        jne       ..B15.29      # Prob 50%                      #1465.17
                                # LOE
..B15.27:                       # Preds ..B15.26
        movl      -32(%ebp), %eax                               #1465.17
        movl      12(%eax), %eax                                #1465.17
        testl     %eax, %eax                                    #1465.17
        jne       ..B15.29      # Prob 50%                      #1465.17
..___tag_value_nss_expr.299:                                    #
                                # LOE
..B15.28:                       # Preds ..B15.27
        addl      $-16, %esp                                    #1465.17
        movl      -20(%ebp), %eax                               #1465.17
        lea       _2__STRING.6@GOTOFF(%eax), %eax               #1465.17
        movl      %eax, (%esp)                                  #1465.17
        movl      -20(%ebp), %eax                               #1465.17
        lea       _2__STRING.1@GOTOFF(%eax), %eax               #1465.17
        movl      %eax, 4(%esp)                                 #1465.17
        movl      $1465, 8(%esp)                                #1465.17
        movl      -20(%ebp), %eax                               #1465.17
        lea       __$U4@GOTOFF(%eax), %eax                      #1465.17
        movl      %eax, 12(%esp)                                #1465.17
        movl      -20(%ebp), %eax                               #1465.17
        movl      %eax, %ebx                                    #1465.17
        call      __assert_fail@PLT                             #1465.17
                                # LOE
..B15.39:                       # Preds ..B15.28
        addl      $16, %esp                                     #1465.17
..___tag_value_nss_expr.300:                                    #
                                # LOE
..B15.29:                       # Preds ..B15.27 ..B15.26 ..B15.6 ..B15.4 ..B15.2
                                #      
..LN1591:
        movl      8(%ebp), %eax                                 #1470.12
        movl      -4(%ebp), %ebx                                #1470.12
..___tag_value_nss_expr.301:                                    #
        leave                                                   #1470.12
..___tag_value_nss_expr.303:                                    #
        ret                                                     #1470.12
        .align    2,0x90
..___tag_value_nss_expr.304:                                    #
                                # LOE
# mark_end;
	.type	nss_expr,@function
	.size	nss_expr,.-nss_expr
.LNnss_expr:
	.data
# -- End  nss_expr
	.section .rodata, "a"
	.align 4
_2__STRING.7:
	.byte	65
	.byte	67
	.byte	71
	.byte	84
	.byte	0
	.type	_2__STRING.7,@object
	.size	_2__STRING.7,5
	.space 3	# pad
_2__STRING.8:
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
	.type	_2__STRING.8,@object
	.size	_2__STRING.8,17
	.text
# -- Begin  NucStrstrInit
# mark_begin;
       .align    2,0x90
NucStrstrInit:
..B16.1:                        # Preds ..B16.0
..___tag_value_NucStrstrInit.307:                               #
..LN1593:
        pushl     %ebp                                          #1484.1
        movl      %esp, %ebp                                    #1484.1
..___tag_value_NucStrstrInit.312:                               #
        subl      $64, %esp                                     #1484.1
        movl      %ebx, -12(%ebp)                               #1484.1
..___tag_value_NucStrstrInit.315:                               #
        call      ..L316        # Prob 100%                     #1484.1
..L316:                                                         #
        popl      %eax                                          #1484.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L316], %eax     #1484.1
        movl      %eax, -32(%ebp)                               #1484.1
..LN1595:
        lea       _2__STRING.7@GOTOFF(%eax), %edx               #1487.29
        movl      %edx, -52(%ebp)                               #1487.29
..LN1597:
        lea       _2__STRING.8@GOTOFF(%eax), %edx               #1488.25
        movl      %edx, -48(%ebp)                               #1488.25
..LN1599:
        addl      $0, %esp                                      #1491.5
..LN1601:
        lea       fasta_2na_map@GOTOFF(%eax), %edx              #1491.14
        movl      %edx, (%esp)                                  #1491.14
..LN1603:
        movl      $-1, 4(%esp)                                  #1491.29
..LN1605:
        movl      $128, 8(%esp)                                 #1491.33
..LN1607:
        movl      %eax, %ebx                                    #1491.5
        call      memset@PLT                                    #1491.5
                                # LOE
..B16.20:                       # Preds ..B16.1
        addl      $12, %esp                                     #1491.5
                                # LOE
..B16.2:                        # Preds ..B16.20
..LN1609:
        addl      $-12, %esp                                    #1492.5
..LN1611:
        movl      -32(%ebp), %eax                               #1492.14
        lea       fasta_4na_map@GOTOFF(%eax), %eax              #1492.14
        movl      %eax, (%esp)                                  #1492.14
..LN1613:
        movl      $-1, 4(%esp)                                  #1492.29
..LN1615:
        movl      $128, 8(%esp)                                 #1492.33
..LN1617:
        movl      -32(%ebp), %eax                               #1492.5
        movl      %eax, %ebx                                    #1492.5
        call      memset@PLT                                    #1492.5
                                # LOE
..B16.21:                       # Preds ..B16.2
        addl      $12, %esp                                     #1492.5
                                # LOE
..B16.3:                        # Preds ..B16.21
..LN1619:
        movl      $0, -44(%ebp)                                 #1495.11
..LN1621:
        movl      -52(%ebp), %eax                               #1495.22
..LN1623:
        movl      %eax, -40(%ebp)                               #1495.18
..LN1625:
        movl      -40(%ebp), %eax                               #1495.31
        movzbl    (%eax), %eax                                  #1495.31
        movsbl    %al, %eax                                     #1495.31
..LN1627:
        testl     %eax, %eax                                    #1495.42
        je        ..B16.7       # Prob 50%                      #1495.42
                                # LOE
..B16.5:                        # Preds ..B16.3 ..B16.6
..LN1629:
        movl      -40(%ebp), %eax                               #1497.14
        movzbl    (%eax), %eax                                  #1497.14
        movsbl    %al, %eax                                     #1497.14
..LN1631:
        movl      %eax, -36(%ebp)                               #1497.9
..LN1633:
        movl      -44(%ebp), %eax                               #1498.78
        movsbl    %al, %eax                                     #1498.78
        movl      %eax, -28(%ebp)                               #1498.78
..LN1635:
        pushl     %edi                                          #1498.48
..LN1637:
        movl      -36(%ebp), %eax                               #1498.58
        movl      %eax, (%esp)                                  #1498.58
..LN1639:
        movl      -32(%ebp), %eax                               #1498.48
        movl      %eax, %ebx                                    #1498.48
        call      tolower@PLT                                   #1498.48
                                # LOE eax
..B16.22:                       # Preds ..B16.5
        popl      %ecx                                          #1498.48
        movl      %eax, -24(%ebp)                               #1498.48
                                # LOE
..B16.6:                        # Preds ..B16.22
..LN1641:
        movl      -32(%ebp), %eax                               #1498.32
        movl      -28(%ebp), %edx                               #1498.32
        movl      -24(%ebp), %ecx                               #1498.32
        movb      %dl, fasta_2na_map@GOTOFF(%ecx,%eax)          #1498.32
..LN1643:
        movl      -36(%ebp), %eax                               #1498.25
..LN1645:
        movl      -32(%ebp), %edx                               #1498.9
        movl      -28(%ebp), %ecx                               #1498.9
        movb      %cl, fasta_2na_map@GOTOFF(%eax,%edx)          #1498.9
..LN1647:
        incl      -44(%ebp)                                     #1495.48
..LN1649:
        incl      -40(%ebp)                                     #1495.54
..LN1651:
        movl      -40(%ebp), %eax                               #1495.31
        movzbl    (%eax), %eax                                  #1495.31
        movsbl    %al, %eax                                     #1495.31
..LN1653:
        testl     %eax, %eax                                    #1495.42
        jne       ..B16.5       # Prob 50%                      #1495.42
                                # LOE
..B16.7:                        # Preds ..B16.6 ..B16.3
..LN1655:
        movl      $0, -44(%ebp)                                 #1502.11
..LN1657:
        movl      -48(%ebp), %eax                               #1502.22
..LN1659:
        movl      %eax, -40(%ebp)                               #1502.18
..LN1661:
        movl      -40(%ebp), %eax                               #1502.31
        movzbl    (%eax), %eax                                  #1502.31
        movsbl    %al, %eax                                     #1502.31
..LN1663:
        testl     %eax, %eax                                    #1502.42
        je        ..B16.11      # Prob 50%                      #1502.42
                                # LOE
..B16.9:                        # Preds ..B16.7 ..B16.10
..LN1665:
        movl      -40(%ebp), %eax                               #1504.14
        movzbl    (%eax), %eax                                  #1504.14
        movsbl    %al, %eax                                     #1504.14
..LN1667:
        movl      %eax, -36(%ebp)                               #1504.9
..LN1669:
        movl      -44(%ebp), %eax                               #1505.78
        movsbl    %al, %eax                                     #1505.78
        movl      %eax, -20(%ebp)                               #1505.78
..LN1671:
        pushl     %edi                                          #1505.48
..LN1673:
        movl      -36(%ebp), %eax                               #1505.58
        movl      %eax, (%esp)                                  #1505.58
..LN1675:
        movl      -32(%ebp), %eax                               #1505.48
        movl      %eax, %ebx                                    #1505.48
        call      tolower@PLT                                   #1505.48
                                # LOE eax
..B16.23:                       # Preds ..B16.9
        popl      %ecx                                          #1505.48
        movl      %eax, -16(%ebp)                               #1505.48
                                # LOE
..B16.10:                       # Preds ..B16.23
..LN1677:
        movl      -32(%ebp), %eax                               #1505.32
        movl      -20(%ebp), %edx                               #1505.32
        movl      -16(%ebp), %ecx                               #1505.32
        movb      %dl, fasta_4na_map@GOTOFF(%ecx,%eax)          #1505.32
..LN1679:
        movl      -36(%ebp), %eax                               #1505.25
..LN1681:
        movl      -32(%ebp), %edx                               #1505.9
        movl      -20(%ebp), %ecx                               #1505.9
        movb      %cl, fasta_4na_map@GOTOFF(%eax,%edx)          #1505.9
..LN1683:
        incl      -44(%ebp)                                     #1502.48
..LN1685:
        incl      -40(%ebp)                                     #1502.54
..LN1687:
        movl      -40(%ebp), %eax                               #1502.31
        movzbl    (%eax), %eax                                  #1502.31
        movsbl    %al, %eax                                     #1502.31
..LN1689:
        testl     %eax, %eax                                    #1502.42
        jne       ..B16.9       # Prob 50%                      #1502.42
                                # LOE
..B16.11:                       # Preds ..B16.10 ..B16.7
..LN1691:
        movl      $0, -44(%ebp)                                 #1510.11
..LN1693:
        movl      -44(%ebp), %eax                               #1510.18
..LN1695:
        cmpl      $256, %eax                                    #1510.22
        jae       ..B16.17      # Prob 50%                      #1510.22
                                # LOE
..B16.13:                       # Preds ..B16.11 ..B16.16
..LN1697:
        movl      -44(%ebp), %eax                               #1511.28
        movl      -32(%ebp), %edx                               #1511.28
        movzwl    expand_2na@GOTOFF(%edx,%eax,2), %eax          #1511.28
        movw      %ax, -8(%ebp)                                 #1511.28
        xorl      %eax, %eax                                    #1511.28
        testl     %eax, %eax                                    #1511.28
        jne       ..B16.15      # Prob 50%                      #1511.28
                                # LOE
..B16.14:                       # Preds ..B16.13
..LN1699:
        movzwl    -8(%ebp), %eax                                #1511.0
# Begin ASM
        rorw $8, %ax
# End ASM                                                       #1511.0
        movw      %ax, -4(%ebp)                                 #1511.0
        jmp       ..B16.16      # Prob 100%                     #1511.0
                                # LOE
..B16.15:                       # Preds ..B16.13
..LN1701:
        movzwl    -8(%ebp), %eax                                #1511.28
        shrl      $8, %eax                                      #1511.28
        movzbl    %al, %eax                                     #1511.28
        movzwl    -8(%ebp), %edx                                #1511.28
        movzbl    %dl, %edx                                     #1511.28
        shll      $8, %edx                                      #1511.28
        orl       %edx, %eax                                    #1511.28
        movw      %ax, -4(%ebp)                                 #1511.28
                                # LOE
..B16.16:                       # Preds ..B16.15 ..B16.14
..LN1703:
        movl      -44(%ebp), %eax                               #1511.22
..LN1705:
        movzwl    -4(%ebp), %edx                                #1511.28
..LN1707:
        movl      -32(%ebp), %ecx                               #1511.9
        movw      %dx, expand_2na@GOTOFF(%ecx,%eax,2)           #1511.9
..LN1709:
        incl      -44(%ebp)                                     #1510.30
..LN1711:
        movl      -44(%ebp), %eax                               #1510.18
..LN1713:
        cmpl      $256, %eax                                    #1510.22
        jb        ..B16.13      # Prob 50%                      #1510.22
                                # LOE
..B16.17:                       # Preds ..B16.16 ..B16.11
..LN1715:
        movl      -12(%ebp), %ebx                               #1513.1
..___tag_value_NucStrstrInit.317:                               #
        leave                                                   #1513.1
..___tag_value_NucStrstrInit.319:                               #
        ret                                                     #1513.1
        .align    2,0x90
..___tag_value_NucStrstrInit.320:                               #
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
# parameter 1(nss): 8 + %ebp
# parameter 2(positional): 12 + %ebp
# parameter 3(query): 16 + %ebp
# parameter 4(len): 20 + %ebp
..B17.1:                        # Preds ..B17.0
..___tag_value_NucStrstrMake.323:                               #
..LN1717:
        pushl     %ebp                                          #1539.1
        movl      %esp, %ebp                                    #1539.1
..___tag_value_NucStrstrMake.328:                               #
        subl      $20, %esp                                     #1539.1
        movl      %ebx, -4(%ebp)                                #1539.1
..___tag_value_NucStrstrMake.331:                               #
..LN1719:
        movl      8(%ebp), %eax                                 #1540.10
..LN1721:
        testl     %eax, %eax                                    #1540.17
        je        ..B17.14      # Prob 50%                      #1540.17
                                # LOE
..B17.2:                        # Preds ..B17.1
..LN1723:
        movl      16(%ebp), %eax                                #1542.14
..LN1725:
        testl     %eax, %eax                                    #1542.23
        je        ..B17.13      # Prob 50%                      #1542.23
                                # LOE
..B17.3:                        # Preds ..B17.2
..LN1727:
        movl      20(%ebp), %eax                                #1542.31
..LN1729:
        testl     %eax, %eax                                    #1542.38
        je        ..B17.13      # Prob 50%                      #1542.38
                                # LOE
..B17.4:                        # Preds ..B17.3
        call      ..L332        # Prob 100%                     #
..L332:                                                         #
        popl      %eax                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L332], %eax     #
        movl      %eax, -12(%ebp)                               #
..LN1731:
        movl      $0, -20(%ebp)                                 #1544.24
..LN1733:
        movzbl    fasta_2na_map@GOTOFF(%eax), %eax              #1547.18
        movsbl    %al, %eax                                     #1547.18
..LN1735:
        testl     %eax, %eax                                    #1547.41
        jne       ..B17.6       # Prob 50%                      #1547.41
                                # LOE
..B17.5:                        # Preds ..B17.4
..LN1737:
        call      NucStrstrInit                                 #1548.17
                                # LOE
..B17.6:                        # Preds ..B17.5 ..B17.4
..LN1739:
        movl      20(%ebp), %eax                                #1550.27
..LN1741:
        addl      16(%ebp), %eax                                #1550.19
..LN1743:
        movl      %eax, -16(%ebp)                               #1550.13
..LN1745:
        addl      $-20, %esp                                    #1551.21
..LN1747:
        movl      16(%ebp), %eax                                #1551.32
        movl      %eax, (%esp)                                  #1551.32
..LN1749:
        movl      -16(%ebp), %eax                               #1551.39
        movl      %eax, 4(%esp)                                 #1551.39
..LN1751:
        movl      8(%ebp), %eax                                 #1551.44
        movl      %eax, 8(%esp)                                 #1551.44
..LN1753:
        lea       -20(%ebp), %eax                               #1551.49
        movl      %eax, 12(%esp)                                #1551.49
..LN1755:
        movl      12(%ebp), %eax                                #1551.59
        movl      %eax, 16(%esp)                                #1551.59
..LN1757:
        call      nss_expr                                      #1551.21
                                # LOE eax
..B17.17:                       # Preds ..B17.6
        addl      $20, %esp                                     #1551.21
        movl      %eax, -8(%ebp)                                #1551.21
                                # LOE
..B17.7:                        # Preds ..B17.17
..LN1759:
        movl      -8(%ebp), %eax                                #1551.13
        movl      %eax, 16(%ebp)                                #1551.13
..LN1761:
        movl      -20(%ebp), %eax                               #1552.18
..LN1763:
        testl     %eax, %eax                                    #1552.28
        jne       ..B17.11      # Prob 50%                      #1552.28
                                # LOE
..B17.8:                        # Preds ..B17.7
..LN1765:
        movl      16(%ebp), %eax                                #1554.22
..LN1767:
        movl      -16(%ebp), %edx                               #1554.31
        cmpl      %edx, %eax                                    #1554.31
        jne       ..B17.10      # Prob 50%                      #1554.31
                                # LOE
..B17.9:                        # Preds ..B17.8
..LN1769:
        xorl      %eax, %eax                                    #1555.28
        movl      -4(%ebp), %ebx                                #1555.28
..___tag_value_NucStrstrMake.333:                               #
        leave                                                   #1555.28
..___tag_value_NucStrstrMake.335:                               #
        ret                                                     #1555.28
..___tag_value_NucStrstrMake.336:                               #
                                # LOE
..B17.10:                       # Preds ..B17.8
..LN1771:
        movl      $22, -20(%ebp)                                #1557.17
                                # LOE
..B17.11:                       # Preds ..B17.10 ..B17.7
..LN1773:
        pushl     %edi                                          #1560.13
..LN1775:
        movl      8(%ebp), %eax                                 #1560.32
        movl      (%eax), %eax                                  #1560.32
        movl      %eax, (%esp)                                  #1560.32
..LN1777:
        movl      -12(%ebp), %eax                               #1560.13
        movl      %eax, %ebx                                    #1560.13
        call      NucStrstrWhack@PLT                            #1560.13
                                # LOE
..B17.18:                       # Preds ..B17.11
        popl      %ecx                                          #1560.13
                                # LOE
..B17.12:                       # Preds ..B17.18
..LN1779:
        movl      8(%ebp), %eax                                 #1561.13
        movl      $0, (%eax)                                    #1561.13
..LN1781:
        movl      -20(%ebp), %eax                               #1562.20
        movl      -4(%ebp), %ebx                                #1562.20
..___tag_value_NucStrstrMake.338:                               #
        leave                                                   #1562.20
..___tag_value_NucStrstrMake.340:                               #
        ret                                                     #1562.20
..___tag_value_NucStrstrMake.341:                               #
                                # LOE
..B17.13:                       # Preds ..B17.3 ..B17.2
..LN1783:
        movl      8(%ebp), %eax                                 #1565.9
        movl      $0, (%eax)                                    #1565.9
                                # LOE
..B17.14:                       # Preds ..B17.13 ..B17.1
..LN1785:
        movl      $22, %eax                                     #1567.12
        movl      -4(%ebp), %ebx                                #1567.12
..___tag_value_NucStrstrMake.343:                               #
        leave                                                   #1567.12
..___tag_value_NucStrstrMake.345:                               #
        ret                                                     #1567.12
        .align    2,0x90
..___tag_value_NucStrstrMake.346:                               #
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
# parameter 1(self): 8 + %ebp
..B18.1:                        # Preds ..B18.0
..___tag_value_NucStrstrWhack.349:                              #
..LN1787:
        pushl     %ebp                                          #1574.1
        movl      %esp, %ebp                                    #1574.1
..___tag_value_NucStrstrWhack.354:                              #
        subl      $12, %esp                                     #1574.1
        movl      %ebx, -4(%ebp)                                #1574.1
..___tag_value_NucStrstrWhack.357:                              #
..LN1789:
        movl      8(%ebp), %eax                                 #1575.10
..LN1791:
        testl     %eax, %eax                                    #1575.18
        je        ..B18.22      # Prob 50%                      #1575.18
                                # LOE
..B18.2:                        # Preds ..B18.1
        call      ..L358        # Prob 100%                     #
..L358:                                                         #
        popl      %eax                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L358], %eax     #
        movl      %eax, -12(%ebp)                               #
..LN1793:
        movl      8(%ebp), %eax                                 #1577.18
        movl      (%eax), %eax                                  #1577.18
        movl      %eax, -8(%ebp)                                #1577.18
..LN1795:
        cmpl      $12, %eax                                     #1577.9
        ja        ..B18.21      # Prob 50%                      #1577.9
                                # LOE
..B18.3:                        # Preds ..B18.2
        movl      -12(%ebp), %eax                               #1577.9
        movl      -8(%ebp), %edx                                #1577.9
        movl      ..1..TPKT.18_0@GOTOFF(%eax,%edx,4), %eax      #1577.9
        jmp       *%eax                                         #1577.9
                                # LOE
..1.18_0.TAG.0a:
..1.18_0.TAG.09:
..1.18_0.TAG.08:
..1.18_0.TAG.07:
..1.18_0.TAG.06:
..1.18_0.TAG.05:
..1.18_0.TAG.04:
..1.18_0.TAG.03:
..1.18_0.TAG.02:
..1.18_0.TAG.01:
..1.18_0.TAG.00:
..B18.15:                       # Preds ..B18.3 ..B18.3 ..B18.3 ..B18.3 ..B18.3
                                #       ..B18.3 ..B18.3 ..B18.3 ..B18.3 ..B18.3
                                #       ..B18.3
..LN1797:
        movl      8(%ebp), %eax                                 #1593.20
        movl      8(%eax), %eax                                 #1593.20
..LN1799:
        movl      %eax, 8(%ebp)                                 #1593.13
        jmp       ..B18.21      # Prob 100%                     #1593.13
                                # LOE
..1.18_0.TAG.0b:
..B18.17:                       # Preds ..B18.3
..LN1801:
        pushl     %edi                                          #1597.13
..LN1803:
        movl      8(%ebp), %eax                                 #1597.30
        movl      8(%eax), %eax                                 #1597.30
        movl      %eax, (%esp)                                  #1597.30
..LN1805:
        movl      -12(%ebp), %eax                               #1597.13
        movl      %eax, %ebx                                    #1597.13
        call      NucStrstrWhack@PLT                            #1597.13
                                # LOE
..B18.25:                       # Preds ..B18.17
        popl      %ecx                                          #1597.13
                                # LOE
..B18.18:                       # Preds ..B18.25
..LN1807:
        pushl     %edi                                          #1598.13
..LN1809:
        movl      8(%ebp), %eax                                 #1598.30
        movl      12(%eax), %eax                                #1598.30
        movl      %eax, (%esp)                                  #1598.30
..LN1811:
        movl      -12(%ebp), %eax                               #1598.13
        movl      %eax, %ebx                                    #1598.13
        call      NucStrstrWhack@PLT                            #1598.13
                                # LOE
..B18.26:                       # Preds ..B18.18
        popl      %ecx                                          #1598.13
        jmp       ..B18.21      # Prob 100%                     #1598.13
                                # LOE
..1.18_0.TAG.0c:
..B18.20:                       # Preds ..B18.3
..LN1813:
        pushl     %edi                                          #1601.13
..LN1815:
        movl      8(%ebp), %eax                                 #1601.30
        movl      8(%eax), %eax                                 #1601.30
        movl      %eax, (%esp)                                  #1601.30
..LN1817:
        movl      -12(%ebp), %eax                               #1601.13
        movl      %eax, %ebx                                    #1601.13
        call      NucStrstrWhack@PLT                            #1601.13
                                # LOE
..B18.27:                       # Preds ..B18.20
        popl      %ecx                                          #1601.13
                                # LOE
..B18.21:                       # Preds ..B18.27 ..B18.26 ..B18.15 ..B18.2
..LN1819:
        pushl     %edi                                          #1604.9
..LN1821:
        movl      8(%ebp), %eax                                 #1604.16
        movl      %eax, (%esp)                                  #1604.16
..LN1823:
        movl      -12(%ebp), %eax                               #1604.9
        movl      %eax, %ebx                                    #1604.9
        call      free@PLT                                      #1604.9
                                # LOE
..B18.28:                       # Preds ..B18.21
        popl      %ecx                                          #1604.9
                                # LOE
..B18.22:                       # Preds ..B18.28 ..B18.1
..LN1825:
        movl      -4(%ebp), %ebx                                #1606.1
..___tag_value_NucStrstrWhack.359:                              #
        leave                                                   #1606.1
..___tag_value_NucStrstrWhack.361:                              #
        ret                                                     #1606.1
        .align    2,0x90
..___tag_value_NucStrstrWhack.362:                              #
                                # LOE
# mark_end;
	.type	NucStrstrWhack,@function
	.size	NucStrstrWhack,.-NucStrstrWhack
.LNNucStrstrWhack:
	.section .data1, "wa"
	.align 4
	.align 4
..1..TPKT.18_0:
	.long	..1.18_0.TAG.00
	.long	..1.18_0.TAG.01
	.long	..1.18_0.TAG.02
	.long	..1.18_0.TAG.03
	.long	..1.18_0.TAG.04
	.long	..1.18_0.TAG.05
	.long	..1.18_0.TAG.06
	.long	..1.18_0.TAG.07
	.long	..1.18_0.TAG.08
	.long	..1.18_0.TAG.09
	.long	..1.18_0.TAG.0a
	.long	..1.18_0.TAG.0b
	.long	..1.18_0.TAG.0c
	.data
# -- End  NucStrstrWhack
	.section .rodata, "a"
	.align 4
__$U5:
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
	.type	__$U5,@object
	.size	__$U5,11
	.space 1	# pad
_2__STRING.9:
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
	.type	_2__STRING.9,@object
	.size	_2__STRING.9,12
	.data
	.text
# -- Begin  eval_2na_8
# mark_begin;
       .align    2,0x90
eval_2na_8:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B19.1:                        # Preds ..B19.0
..___tag_value_eval_2na_8.365:                                  #
..LN1827:
        pushl     %ebx                                          #2196.1
..___tag_value_eval_2na_8.370:                                  #
        movl      %esp, %ebx                                    #2196.1
..___tag_value_eval_2na_8.371:                                  #
        andl      $-16, %esp                                    #2196.1
        pushl     %ebp                                          #2196.1
        pushl     %ebp                                          #2196.1
        movl      4(%ebx), %ebp                                 #2196.1
        movl      %ebp, 4(%esp)                                 #2196.1
        movl      %esp, %ebp                                    #2196.1
..___tag_value_eval_2na_8.373:                                  #
        subl      $280, %esp                                    #2196.1
        movl      %ebx, -280(%ebp)                              #2196.1
..LN1829:
        movl      8(%ebx), %eax                                 #2226.25
        movl      4(%eax), %eax                                 #2226.25
..LN1831:
        movl      %eax, -4(%ebp)                                #2226.23
..LN1833:
        movl      20(%ebx), %eax                                #2229.5
        movl      -4(%ebp), %edx                                #2229.5
        cmpl      %edx, %eax                                    #2229.5
        jae       ..B19.3       # Prob 50%                      #2229.5
                                # LOE
..B19.2:                        # Preds ..B19.1
        call      ..L375        # Prob 100%                     #
..L375:                                                         #
        popl      %eax                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L375], %eax     #
        addl      $-16, %esp                                    #2229.5
        lea       _2__STRING.9@GOTOFF(%eax), %edx               #2229.5
        movl      %edx, (%esp)                                  #2229.5
        lea       _2__STRING.1@GOTOFF(%eax), %edx               #2229.5
        movl      %edx, 4(%esp)                                 #2229.5
        movl      $2229, 8(%esp)                                #2229.5
        lea       __$U5@GOTOFF(%eax), %edx                      #2229.5
        movl      %edx, 12(%esp)                                #2229.5
        movl      %eax, %ebx                                    #2229.5
..___tag_value_eval_2na_8.376:                                  #2229.5
        call      __assert_fail@PLT                             #2229.5
        movl      -280(%ebp), %ebx                              #2229.5
..___tag_value_eval_2na_8.377:                                  #
                                # LOE
..B19.43:                       # Preds ..B19.2
        addl      $16, %esp                                     #2229.5
                                # LOE
..B19.3:                        # Preds ..B19.1
..LN1835:
        movl      16(%ebx), %eax                                #2233.12
..LN1837:
        addl      20(%ebx), %eax                                #2233.5
        movl      %eax, 20(%ebx)                                #2233.5
..LN1839:
        movl      16(%ebx), %eax                                #2236.42
..LN1841:
        shrl      $2, %eax                                      #2236.49
..LN1843:
        addl      12(%ebx), %eax                                #2236.30
..LN1845:
        movl      %eax, -36(%ebp)                               #2236.5
..LN1847:
        movl      -4(%ebp), %eax                                #2239.18
        negl      %eax                                          #2239.18
        addl      20(%ebx), %eax                                #2239.18
..LN1849:
        movl      %eax, -32(%ebp)                               #2239.5
..LN1851:
        movl      20(%ebx), %eax                                #2242.44
..LN1853:
        addl      $3, %eax                                      #2242.50
..LN1855:
        shrl      $2, %eax                                      #2242.57
..LN1857:
        addl      12(%ebx), %eax                                #2242.30
..LN1859:
        movl      %eax, -28(%ebp)                               #2242.5
..LN1861:
        addl      $-8, %esp                                     #2245.14
..LN1863:
        movl      -36(%ebp), %eax                               #2245.33
        movl      %eax, (%esp)                                  #2245.33
..LN1865:
        movl      -28(%ebp), %eax                               #2245.38
        movl      %eax, 4(%esp)                                 #2245.38
..LN1867:
        call      prime_buffer_2na                              #2245.14
                                # LOE xmm0
..B19.44:                       # Preds ..B19.3
        addl      $8, %esp                                      #2245.14
        movdqa    %xmm0, -88(%ebp)                              #2245.14
                                # LOE
..B19.4:                        # Preds ..B19.44
..LN1869:
        movdqa    -88(%ebp), %xmm0                              #2245.5
        movdqa    %xmm0, -232(%ebp)                             #2245.5
..LN1871:
        addl      $16, -36(%ebp)                                #2246.5
..LN1873:
        movl      8(%ebx), %eax                                 #2256.5
        movdqa    16(%eax), %xmm0                               #2256.5
        movdqa    %xmm0, -216(%ebp)                             #2256.5
        movl      8(%ebx), %eax                                 #2256.5
        movdqa    32(%eax), %xmm0                               #2256.5
        movdqa    %xmm0, -200(%ebp)                             #2256.5
        movl      8(%ebx), %eax                                 #2256.5
        movdqa    48(%eax), %xmm0                               #2256.5
        movdqa    %xmm0, -184(%ebp)                             #2256.5
        movl      8(%ebx), %eax                                 #2256.5
        movdqa    64(%eax), %xmm0                               #2256.5
        movdqa    %xmm0, -168(%ebp)                             #2256.5
        movl      8(%ebx), %eax                                 #2256.5
        movdqa    80(%eax), %xmm0                               #2256.5
        movdqa    %xmm0, -152(%ebp)                             #2256.5
        movl      8(%ebx), %eax                                 #2256.5
        movdqa    96(%eax), %xmm0                               #2256.5
        movdqa    %xmm0, -136(%ebp)                             #2256.5
        movl      8(%ebx), %eax                                 #2256.5
        movdqa    112(%eax), %xmm0                              #2256.5
        movdqa    %xmm0, -120(%ebp)                             #2256.5
        movl      8(%ebx), %eax                                 #2256.5
        movdqa    128(%eax), %xmm0                              #2256.5
        movdqa    %xmm0, -104(%ebp)                             #2256.5
..LN1875:
        xorl      %eax, %eax                                    #2259.15
        movl      %eax, -24(%ebp)                               #2259.15
..LN1877:
        movl      %eax, -20(%ebp)                               #2259.10
..LN1879:
        movl      %eax, -16(%ebp)                               #2259.5
..LN1881:
        movl      $1, -12(%ebp)                                 #2263.5
..LN1883:
        movl      16(%ebx), %eax                                #2268.14
..LN1885:
        andl      $3, %eax                                      #2268.20
        movl      %eax, -8(%ebp)                                #2268.20
        je        ..B19.12      # Prob 50%                      #2268.20
                                # LOE
..B19.5:                        # Preds ..B19.4
        movl      -8(%ebp), %eax                                #2268.20
        cmpl      $1, %eax                                      #2268.20
        je        ..B19.13      # Prob 50%                      #2268.20
                                # LOE
..B19.6:                        # Preds ..B19.5
        movl      -8(%ebp), %eax                                #2268.20
        cmpl      $2, %eax                                      #2268.20
        je        ..B19.14      # Prob 50%                      #2268.20
                                # LOE
..B19.7:                        # Preds ..B19.6
        movl      -8(%ebp), %eax                                #2268.20
        cmpl      $3, %eax                                      #2268.20
        je        ..B19.15      # Prob 50%                      #2268.20
                                # LOE
..B19.8:                        # Preds ..B19.39 ..B19.7
..LN1887:
        movl      $1, %eax                                      #2273.9
        testl     %eax, %eax                                    #2273.9
        je        ..B19.40      # Prob 100%                     #2273.9
                                # LOE
..B19.9:                        # Preds ..B19.8
..LN1889:
        movl      $1, -12(%ebp)                                 #2275.13
                                # LOE
..B19.10:                       # Preds ..B19.34 ..B19.9
..LN1891:
        movl      $1, %eax                                      #2278.13
        testl     %eax, %eax                                    #2278.13
        je        ..B19.36      # Prob 100%                     #2278.13
                                # LOE
..B19.12:                       # Preds ..B19.4 ..B19.10
..LN1893:
        movdqa    -232(%ebp), %xmm0                             #2285.38
..LN1895:
        movdqa    -200(%ebp), %xmm1                             #2285.46
..LN1897:
        pand      %xmm1, %xmm0                                  #2285.22
..LN1899:
        movdqa    %xmm0, -248(%ebp)                             #2285.17
..LN1901:
        movdqa    -248(%ebp), %xmm0                             #2286.22
        movdqa    -216(%ebp), %xmm1                             #2286.22
        pcmpeqb   %xmm1, %xmm0                                  #2286.22
..LN1903:
        movdqa    %xmm0, -248(%ebp)                             #2286.17
..LN1905:
        movdqa    -248(%ebp), %xmm0                             #2287.42
..LN1907:
        pmovmskb  %xmm0, %eax                                   #2287.22
..LN1909:
        movl      %eax, -16(%ebp)                               #2287.17
                                # LOE
..B19.13:                       # Preds ..B19.5 ..B19.12
..LN1911:
        movdqa    -232(%ebp), %xmm0                             #2291.38
..LN1913:
        movdqa    -168(%ebp), %xmm1                             #2291.46
..LN1915:
        pand      %xmm1, %xmm0                                  #2291.22
..LN1917:
        movdqa    %xmm0, -248(%ebp)                             #2291.17
..LN1919:
        movdqa    -248(%ebp), %xmm0                             #2292.22
        movdqa    -184(%ebp), %xmm1                             #2292.22
        pcmpeqb   %xmm1, %xmm0                                  #2292.22
..LN1921:
        movdqa    %xmm0, -248(%ebp)                             #2292.17
..LN1923:
        movdqa    -248(%ebp), %xmm0                             #2293.42
..LN1925:
        pmovmskb  %xmm0, %eax                                   #2293.22
..LN1927:
        movl      %eax, -20(%ebp)                               #2293.17
                                # LOE
..B19.14:                       # Preds ..B19.6 ..B19.13
..LN1929:
        movdqa    -232(%ebp), %xmm0                             #2297.38
..LN1931:
        movdqa    -136(%ebp), %xmm1                             #2297.46
..LN1933:
        pand      %xmm1, %xmm0                                  #2297.22
..LN1935:
        movdqa    %xmm0, -248(%ebp)                             #2297.17
..LN1937:
        movdqa    -248(%ebp), %xmm0                             #2298.22
        movdqa    -152(%ebp), %xmm1                             #2298.22
        pcmpeqb   %xmm1, %xmm0                                  #2298.22
..LN1939:
        movdqa    %xmm0, -248(%ebp)                             #2298.17
..LN1941:
        movdqa    -248(%ebp), %xmm0                             #2299.42
..LN1943:
        pmovmskb  %xmm0, %eax                                   #2299.22
..LN1945:
        movl      %eax, -24(%ebp)                               #2299.17
                                # LOE
..B19.15:                       # Preds ..B19.7 ..B19.14
..LN1947:
        movdqa    -232(%ebp), %xmm0                             #2303.38
..LN1949:
        movdqa    -104(%ebp), %xmm1                             #2303.46
..LN1951:
        pand      %xmm1, %xmm0                                  #2303.22
..LN1953:
        movdqa    %xmm0, -248(%ebp)                             #2303.17
..LN1955:
        movdqa    -248(%ebp), %xmm0                             #2304.22
        movdqa    -120(%ebp), %xmm1                             #2304.22
        pcmpeqb   %xmm1, %xmm0                                  #2304.22
..LN1957:
        movdqa    %xmm0, -248(%ebp)                             #2304.17
..LN1959:
        movdqa    -248(%ebp), %xmm0                             #2305.42
..LN1961:
        pmovmskb  %xmm0, %eax                                   #2305.22
..LN1963:
        movl      %eax, -40(%ebp)                               #2305.17
..LN1965:
        movl      16(%ebx), %eax                                #2310.17
        andl      $-4, %eax                                     #2310.17
        movl      %eax, 16(%ebx)                                #2310.17
..LN1967:
        movl      -20(%ebp), %eax                               #2313.29
        orl       -16(%ebp), %eax                               #2313.29
..LN1969:
        orl       -24(%ebp), %eax                               #2313.34
..LN1971:
        orl       -40(%ebp), %eax                               #2313.39
..LN1973:
        je        ..B19.32      # Prob 50%                      #2313.47
                                # LOE
..B19.16:                       # Preds ..B19.15
..LN1975:
        pushl     %edi                                          #2331.30
..LN1977:
        movl      -16(%ebp), %eax                               #2331.58
        movw      %ax, (%esp)                                   #2331.58
..LN1979:
        call      uint16_lsbit                                  #2331.30
                                # LOE eax
..B19.45:                       # Preds ..B19.16
        popl      %ecx                                          #2331.30
        movl      %eax, -56(%ebp)                               #2331.30
                                # LOE
..B19.17:                       # Preds ..B19.45
        movl      -56(%ebp), %eax                               #2331.30
        movswl    %ax, %eax                                     #2331.30
..LN1981:
        movl      %eax, -72(%ebp)                               #2331.28
..LN1983:
        pushl     %edi                                          #2332.30
..LN1985:
        movl      -20(%ebp), %eax                               #2332.58
        movw      %ax, (%esp)                                   #2332.58
..LN1987:
        call      uint16_lsbit                                  #2332.30
                                # LOE eax
..B19.46:                       # Preds ..B19.17
        popl      %ecx                                          #2332.30
        movl      %eax, -52(%ebp)                               #2332.30
                                # LOE
..B19.18:                       # Preds ..B19.46
        movl      -52(%ebp), %eax                               #2332.30
        movswl    %ax, %eax                                     #2332.30
..LN1989:
        movl      %eax, -68(%ebp)                               #2332.28
..LN1991:
        pushl     %edi                                          #2333.30
..LN1993:
        movl      -24(%ebp), %eax                               #2333.58
        movw      %ax, (%esp)                                   #2333.58
..LN1995:
        call      uint16_lsbit                                  #2333.30
                                # LOE eax
..B19.47:                       # Preds ..B19.18
        popl      %ecx                                          #2333.30
        movl      %eax, -48(%ebp)                               #2333.30
                                # LOE
..B19.19:                       # Preds ..B19.47
        movl      -48(%ebp), %eax                               #2333.30
        movswl    %ax, %eax                                     #2333.30
..LN1997:
        movl      %eax, -64(%ebp)                               #2333.28
..LN1999:
        pushl     %edi                                          #2334.30
..LN2001:
        movl      -40(%ebp), %eax                               #2334.58
        movw      %ax, (%esp)                                   #2334.58
..LN2003:
        call      uint16_lsbit                                  #2334.30
                                # LOE eax
..B19.48:                       # Preds ..B19.19
        popl      %ecx                                          #2334.30
        movl      %eax, -44(%ebp)                               #2334.30
                                # LOE
..B19.20:                       # Preds ..B19.48
        movl      -44(%ebp), %eax                               #2334.30
        movswl    %ax, %eax                                     #2334.30
..LN2005:
        movl      %eax, -60(%ebp)                               #2334.28
..LN2007:
        shll      $2, -72(%ebp)                                 #2339.34
..LN2009:
        movl      -68(%ebp), %eax                               #2340.28
..LN2011:
        lea       1(,%eax,4), %eax                              #2340.40
..LN2013:
        movl      %eax, -68(%ebp)                               #2340.21
..LN2015:
        movl      -64(%ebp), %eax                               #2341.28
..LN2017:
        lea       2(,%eax,4), %eax                              #2341.40
..LN2019:
        movl      %eax, -64(%ebp)                               #2341.21
..LN2021:
        movl      -60(%ebp), %eax                               #2342.28
..LN2023:
        lea       3(,%eax,4), %eax                              #2342.40
..LN2025:
        movl      %eax, -60(%ebp)                               #2342.21
..LN2027:
        movl      -16(%ebp), %eax                               #2346.26
..LN2029:
        testl     %eax, %eax                                    #2346.32
        je        ..B19.23      # Prob 50%                      #2346.32
                                # LOE
..B19.21:                       # Preds ..B19.20
..LN2031:
        movl      -72(%ebp), %eax                               #2346.43
        addl      16(%ebx), %eax                                #2346.43
..LN2033:
        movl      -32(%ebp), %edx                               #2346.49
        cmpl      %edx, %eax                                    #2346.49
        ja        ..B19.23      # Prob 50%                      #2346.49
                                # LOE
..B19.22:                       # Preds ..B19.21
..LN2035:
        movl      $1, %eax                                      #2346.63
        leave                                                   #2346.63
..___tag_value_eval_2na_8.379:                                  #
        movl      %ebx, %esp                                    #2346.63
        popl      %ebx                                          #2346.63
..___tag_value_eval_2na_8.380:                                  #
        ret                                                     #2346.63
..___tag_value_eval_2na_8.382:                                  #
                                # LOE
..B19.23:                       # Preds ..B19.21 ..B19.20
..LN2037:
        movl      -20(%ebp), %eax                               #2347.26
..LN2039:
        testl     %eax, %eax                                    #2347.32
        je        ..B19.26      # Prob 50%                      #2347.32
                                # LOE
..B19.24:                       # Preds ..B19.23
..LN2041:
        movl      -68(%ebp), %eax                               #2347.43
        addl      16(%ebx), %eax                                #2347.43
..LN2043:
        movl      -32(%ebp), %edx                               #2347.49
        cmpl      %edx, %eax                                    #2347.49
        ja        ..B19.26      # Prob 50%                      #2347.49
                                # LOE
..B19.25:                       # Preds ..B19.24
..LN2045:
        movl      $1, %eax                                      #2347.63
        leave                                                   #2347.63
..___tag_value_eval_2na_8.386:                                  #
        movl      %ebx, %esp                                    #2347.63
        popl      %ebx                                          #2347.63
..___tag_value_eval_2na_8.387:                                  #
        ret                                                     #2347.63
..___tag_value_eval_2na_8.389:                                  #
                                # LOE
..B19.26:                       # Preds ..B19.24 ..B19.23
..LN2047:
        movl      -24(%ebp), %eax                               #2348.26
..LN2049:
        testl     %eax, %eax                                    #2348.32
        je        ..B19.29      # Prob 50%                      #2348.32
                                # LOE
..B19.27:                       # Preds ..B19.26
..LN2051:
        movl      -64(%ebp), %eax                               #2348.43
        addl      16(%ebx), %eax                                #2348.43
..LN2053:
        movl      -32(%ebp), %edx                               #2348.49
        cmpl      %edx, %eax                                    #2348.49
        ja        ..B19.29      # Prob 50%                      #2348.49
                                # LOE
..B19.28:                       # Preds ..B19.27
..LN2055:
        movl      $1, %eax                                      #2348.63
        leave                                                   #2348.63
..___tag_value_eval_2na_8.393:                                  #
        movl      %ebx, %esp                                    #2348.63
        popl      %ebx                                          #2348.63
..___tag_value_eval_2na_8.394:                                  #
        ret                                                     #2348.63
..___tag_value_eval_2na_8.396:                                  #
                                # LOE
..B19.29:                       # Preds ..B19.27 ..B19.26
..LN2057:
        movl      -40(%ebp), %eax                               #2349.26
..LN2059:
        testl     %eax, %eax                                    #2349.32
        je        ..B19.32      # Prob 50%                      #2349.32
                                # LOE
..B19.30:                       # Preds ..B19.29
..LN2061:
        movl      -60(%ebp), %eax                               #2349.43
        addl      16(%ebx), %eax                                #2349.43
..LN2063:
        movl      -32(%ebp), %edx                               #2349.49
        cmpl      %edx, %eax                                    #2349.49
        ja        ..B19.32      # Prob 50%                      #2349.49
                                # LOE
..B19.31:                       # Preds ..B19.30
..LN2065:
        movl      $1, %eax                                      #2349.63
        leave                                                   #2349.63
..___tag_value_eval_2na_8.400:                                  #
        movl      %ebx, %esp                                    #2349.63
        popl      %ebx                                          #2349.63
..___tag_value_eval_2na_8.401:                                  #
        ret                                                     #2349.63
..___tag_value_eval_2na_8.403:                                  #
                                # LOE
..B19.32:                       # Preds ..B19.30 ..B19.29 ..B19.15
..LN2067:
        movl      16(%ebx), %eax                                #2354.17
        addl      $4, %eax                                      #2354.17
        movl      %eax, 16(%ebx)                                #2354.17
..LN2069:
        movl      16(%ebx), %eax                                #2357.22
..LN2071:
        movl      -32(%ebp), %edx                               #2357.28
        cmpl      %edx, %eax                                    #2357.28
        jbe       ..B19.34      # Prob 50%                      #2357.28
                                # LOE
..B19.33:                       # Preds ..B19.32
..LN2073:
        xorl      %eax, %eax                                    #2358.28
        leave                                                   #2358.28
..___tag_value_eval_2na_8.407:                                  #
        movl      %ebx, %esp                                    #2358.28
        popl      %ebx                                          #2358.28
..___tag_value_eval_2na_8.408:                                  #
        ret                                                     #2358.28
..___tag_value_eval_2na_8.410:                                  #
                                # LOE
..B19.34:                       # Preds ..B19.32
..LN2075:
        movl      -12(%ebp), %eax                               #2361.25
        decl      %eax                                          #2361.25
        movl      %eax, -12(%ebp)                               #2361.25
..LN2077:
        jne       ..B19.10      # Prob 50%                      #2361.39
                                # LOE
..B19.36:                       # Preds ..B19.34 ..B19.10
..LN2079:
        movl      -36(%ebp), %eax                               #2382.18
..LN2081:
        movl      -28(%ebp), %edx                               #2382.25
        cmpl      %edx, %eax                                    #2382.25
        jae       ..B19.40      # Prob 50%                      #2382.25
                                # LOE
..B19.37:                       # Preds ..B19.36
..LN2083:
        movl      16(%ebx), %eax                                #2386.13
        addl      $60, %eax                                     #2386.13
        movl      %eax, 16(%ebx)                                #2386.13
..LN2085:
        movl      16(%ebx), %eax                                #2387.18
..LN2087:
        movl      -32(%ebp), %edx                               #2387.24
        cmpl      %edx, %eax                                    #2387.24
        ja        ..B19.40      # Prob 50%                      #2387.24
                                # LOE
..B19.38:                       # Preds ..B19.37
..LN2089:
        addl      $-8, %esp                                     #2392.22
..LN2091:
        movl      -36(%ebp), %eax                               #2392.41
        movl      %eax, (%esp)                                  #2392.41
..LN2093:
        movl      -28(%ebp), %eax                               #2392.46
        movl      %eax, 4(%esp)                                 #2392.46
..LN2095:
        call      prime_buffer_2na                              #2392.22
                                # LOE xmm0
..B19.49:                       # Preds ..B19.38
        addl      $8, %esp                                      #2392.22
        movdqa    %xmm0, -264(%ebp)                             #2392.22
                                # LOE
..B19.39:                       # Preds ..B19.49
..LN2097:
        movdqa    -264(%ebp), %xmm0                             #2392.13
        movdqa    %xmm0, -232(%ebp)                             #2392.13
..LN2099:
        addl      $16, -36(%ebp)                                #2470.13
        jmp       ..B19.8       # Prob 100%                     #2470.13
                                # LOE
..B19.40:                       # Preds ..B19.37 ..B19.36 ..B19.8
..LN2101:
        xorl      %eax, %eax                                    #2483.12
        leave                                                   #2483.12
..___tag_value_eval_2na_8.414:                                  #
        movl      %ebx, %esp                                    #2483.12
        popl      %ebx                                          #2483.12
..___tag_value_eval_2na_8.415:                                  #
        ret                                                     #2483.12
        .align    2,0x90
..___tag_value_eval_2na_8.417:                                  #
                                # LOE
# mark_end;
	.type	eval_2na_8,@function
	.size	eval_2na_8,.-eval_2na_8
.LNeval_2na_8:
	.data
# -- End  eval_2na_8
	.section .rodata, "a"
	.align 4
__$U6:
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
	.type	__$U6,@object
	.size	__$U6,12
	.text
# -- Begin  eval_2na_16
# mark_begin;
       .align    2,0x90
eval_2na_16:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B20.1:                        # Preds ..B20.0
..___tag_value_eval_2na_16.420:                                 #
..LN2103:
        pushl     %ebx                                          #2489.1
..___tag_value_eval_2na_16.425:                                 #
        movl      %esp, %ebx                                    #2489.1
..___tag_value_eval_2na_16.426:                                 #
        andl      $-16, %esp                                    #2489.1
        pushl     %ebp                                          #2489.1
        pushl     %ebp                                          #2489.1
        movl      4(%ebx), %ebp                                 #2489.1
        movl      %ebp, 4(%esp)                                 #2489.1
        movl      %esp, %ebp                                    #2489.1
..___tag_value_eval_2na_16.428:                                 #
        subl      $296, %esp                                    #2489.1
        movl      %ebx, -296(%ebp)                              #2489.1
..LN2105:
        movl      8(%ebx), %eax                                 #2519.25
        movl      4(%eax), %eax                                 #2519.25
..LN2107:
        movl      %eax, -12(%ebp)                               #2519.23
..LN2109:
        movl      20(%ebx), %eax                                #2522.5
        movl      -12(%ebp), %edx                               #2522.5
        cmpl      %edx, %eax                                    #2522.5
        jae       ..B20.3       # Prob 50%                      #2522.5
                                # LOE
..B20.2:                        # Preds ..B20.1
        call      ..L430        # Prob 100%                     #
..L430:                                                         #
        popl      %eax                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L430], %eax     #
        addl      $-16, %esp                                    #2522.5
        lea       _2__STRING.9@GOTOFF(%eax), %edx               #2522.5
        movl      %edx, (%esp)                                  #2522.5
        lea       _2__STRING.1@GOTOFF(%eax), %edx               #2522.5
        movl      %edx, 4(%esp)                                 #2522.5
        movl      $2522, 8(%esp)                                #2522.5
        lea       __$U6@GOTOFF(%eax), %edx                      #2522.5
        movl      %edx, 12(%esp)                                #2522.5
        movl      %eax, %ebx                                    #2522.5
..___tag_value_eval_2na_16.431:                                 #2522.5
        call      __assert_fail@PLT                             #2522.5
        movl      -296(%ebp), %ebx                              #2522.5
..___tag_value_eval_2na_16.432:                                 #
                                # LOE
..B20.49:                       # Preds ..B20.2
        addl      $16, %esp                                     #2522.5
                                # LOE
..B20.3:                        # Preds ..B20.1
..LN2111:
        movl      16(%ebx), %eax                                #2526.12
..LN2113:
        addl      20(%ebx), %eax                                #2526.5
        movl      %eax, 20(%ebx)                                #2526.5
..LN2115:
        movl      16(%ebx), %eax                                #2529.42
..LN2117:
        shrl      $2, %eax                                      #2529.49
..LN2119:
        addl      12(%ebx), %eax                                #2529.30
..LN2121:
        movl      %eax, -48(%ebp)                               #2529.5
..LN2123:
        movl      -12(%ebp), %eax                               #2532.18
        negl      %eax                                          #2532.18
        addl      20(%ebx), %eax                                #2532.18
..LN2125:
        movl      %eax, -44(%ebp)                               #2532.5
..LN2127:
        movl      20(%ebx), %eax                                #2535.44
..LN2129:
        addl      $3, %eax                                      #2535.50
..LN2131:
        shrl      $2, %eax                                      #2535.57
..LN2133:
        addl      12(%ebx), %eax                                #2535.30
..LN2135:
        movl      %eax, -40(%ebp)                               #2535.5
..LN2137:
        addl      $-8, %esp                                     #2538.14
..LN2139:
        movl      -48(%ebp), %eax                               #2538.33
        movl      %eax, (%esp)                                  #2538.33
..LN2141:
        movl      -40(%ebp), %eax                               #2538.38
        movl      %eax, 4(%esp)                                 #2538.38
..LN2143:
        call      prime_buffer_2na                              #2538.14
                                # LOE xmm0
..B20.50:                       # Preds ..B20.3
        addl      $8, %esp                                      #2538.14
        movdqa    %xmm0, -104(%ebp)                             #2538.14
                                # LOE
..B20.4:                        # Preds ..B20.50
..LN2145:
        movdqa    -104(%ebp), %xmm0                             #2538.5
        movdqa    %xmm0, -248(%ebp)                             #2538.5
..LN2147:
        addl      $16, -48(%ebp)                                #2539.5
..LN2149:
        movl      -48(%ebp), %eax                               #2541.9
..LN2151:
        movl      %eax, -36(%ebp)                               #2541.5
..LN2153:
        movl      -48(%ebp), %eax                               #2544.10
..LN2155:
        movl      -40(%ebp), %edx                               #2544.16
        cmpl      %edx, %eax                                    #2544.16
        jae       ..B20.6       # Prob 50%                      #2544.16
                                # LOE
..B20.5:                        # Preds ..B20.4
..LN2157:
        movl      -48(%ebp), %eax                               #2545.24
        movzbl    -1(%eax), %eax                                #2545.24
        movzbl    %al, %eax                                     #2545.24
..LN2159:
        shll      $8, %eax                                      #2545.38
..LN2161:
        movl      %eax, -56(%ebp)                               #2545.9
                                # LOE
..B20.6:                        # Preds ..B20.5 ..B20.4
..LN2163:
        movl      8(%ebx), %eax                                 #2549.5
        movdqa    16(%eax), %xmm0                               #2549.5
        movdqa    %xmm0, -232(%ebp)                             #2549.5
        movl      8(%ebx), %eax                                 #2549.5
        movdqa    32(%eax), %xmm0                               #2549.5
        movdqa    %xmm0, -216(%ebp)                             #2549.5
        movl      8(%ebx), %eax                                 #2549.5
        movdqa    48(%eax), %xmm0                               #2549.5
        movdqa    %xmm0, -200(%ebp)                             #2549.5
        movl      8(%ebx), %eax                                 #2549.5
        movdqa    64(%eax), %xmm0                               #2549.5
        movdqa    %xmm0, -184(%ebp)                             #2549.5
        movl      8(%ebx), %eax                                 #2549.5
        movdqa    80(%eax), %xmm0                               #2549.5
        movdqa    %xmm0, -168(%ebp)                             #2549.5
        movl      8(%ebx), %eax                                 #2549.5
        movdqa    96(%eax), %xmm0                               #2549.5
        movdqa    %xmm0, -152(%ebp)                             #2549.5
        movl      8(%ebx), %eax                                 #2549.5
        movdqa    112(%eax), %xmm0                              #2549.5
        movdqa    %xmm0, -136(%ebp)                             #2549.5
        movl      8(%ebx), %eax                                 #2549.5
        movdqa    128(%eax), %xmm0                              #2549.5
        movdqa    %xmm0, -120(%ebp)                             #2549.5
..LN2165:
        xorl      %eax, %eax                                    #2552.15
        movl      %eax, -32(%ebp)                               #2552.15
..LN2167:
        movl      %eax, -28(%ebp)                               #2552.10
..LN2169:
        movl      %eax, -24(%ebp)                               #2552.5
..LN2171:
        movl      $2, -20(%ebp)                                 #2556.5
..LN2173:
        movl      16(%ebx), %eax                                #2561.14
..LN2175:
        andl      $3, %eax                                      #2561.20
        movl      %eax, -16(%ebp)                               #2561.20
        je        ..B20.14      # Prob 50%                      #2561.20
                                # LOE
..B20.7:                        # Preds ..B20.6
        movl      -16(%ebp), %eax                               #2561.20
        cmpl      $1, %eax                                      #2561.20
        je        ..B20.15      # Prob 50%                      #2561.20
                                # LOE
..B20.8:                        # Preds ..B20.7
        movl      -16(%ebp), %eax                               #2561.20
        cmpl      $2, %eax                                      #2561.20
        je        ..B20.16      # Prob 50%                      #2561.20
                                # LOE
..B20.9:                        # Preds ..B20.8
        movl      -16(%ebp), %eax                               #2561.20
        cmpl      $3, %eax                                      #2561.20
        je        ..B20.17      # Prob 50%                      #2561.20
                                # LOE
..B20.10:                       # Preds ..B20.45 ..B20.44 ..B20.9
..LN2177:
        movl      $1, %eax                                      #2566.9
        testl     %eax, %eax                                    #2566.9
        je        ..B20.46      # Prob 100%                     #2566.9
                                # LOE
..B20.11:                       # Preds ..B20.10
..LN2179:
        movl      $2, -20(%ebp)                                 #2568.13
                                # LOE
..B20.12:                       # Preds ..B20.39 ..B20.11
..LN2181:
        movl      $1, %eax                                      #2571.13
        testl     %eax, %eax                                    #2571.13
        je        ..B20.41      # Prob 100%                     #2571.13
                                # LOE
..B20.14:                       # Preds ..B20.6 ..B20.12
..LN2183:
        movdqa    -248(%ebp), %xmm0                             #2578.38
..LN2185:
        movdqa    -216(%ebp), %xmm1                             #2578.46
..LN2187:
        pand      %xmm1, %xmm0                                  #2578.22
..LN2189:
        movdqa    %xmm0, -264(%ebp)                             #2578.17
..LN2191:
        movdqa    -264(%ebp), %xmm0                             #2579.22
        movdqa    -232(%ebp), %xmm1                             #2579.22
        pcmpeqw   %xmm1, %xmm0                                  #2579.22
..LN2193:
        movdqa    %xmm0, -264(%ebp)                             #2579.17
..LN2195:
        movdqa    -264(%ebp), %xmm0                             #2580.42
..LN2197:
        pmovmskb  %xmm0, %eax                                   #2580.22
..LN2199:
        movl      %eax, -24(%ebp)                               #2580.17
                                # LOE
..B20.15:                       # Preds ..B20.7 ..B20.14
..LN2201:
        movdqa    -248(%ebp), %xmm0                             #2584.38
..LN2203:
        movdqa    -184(%ebp), %xmm1                             #2584.46
..LN2205:
        pand      %xmm1, %xmm0                                  #2584.22
..LN2207:
        movdqa    %xmm0, -264(%ebp)                             #2584.17
..LN2209:
        movdqa    -264(%ebp), %xmm0                             #2585.22
        movdqa    -200(%ebp), %xmm1                             #2585.22
        pcmpeqw   %xmm1, %xmm0                                  #2585.22
..LN2211:
        movdqa    %xmm0, -264(%ebp)                             #2585.17
..LN2213:
        movdqa    -264(%ebp), %xmm0                             #2586.42
..LN2215:
        pmovmskb  %xmm0, %eax                                   #2586.22
..LN2217:
        movl      %eax, -28(%ebp)                               #2586.17
                                # LOE
..B20.16:                       # Preds ..B20.8 ..B20.15
..LN2219:
        movdqa    -248(%ebp), %xmm0                             #2590.38
..LN2221:
        movdqa    -152(%ebp), %xmm1                             #2590.46
..LN2223:
        pand      %xmm1, %xmm0                                  #2590.22
..LN2225:
        movdqa    %xmm0, -264(%ebp)                             #2590.17
..LN2227:
        movdqa    -264(%ebp), %xmm0                             #2591.22
        movdqa    -168(%ebp), %xmm1                             #2591.22
        pcmpeqw   %xmm1, %xmm0                                  #2591.22
..LN2229:
        movdqa    %xmm0, -264(%ebp)                             #2591.17
..LN2231:
        movdqa    -264(%ebp), %xmm0                             #2592.42
..LN2233:
        pmovmskb  %xmm0, %eax                                   #2592.22
..LN2235:
        movl      %eax, -32(%ebp)                               #2592.17
                                # LOE
..B20.17:                       # Preds ..B20.9 ..B20.16
..LN2237:
        movdqa    -248(%ebp), %xmm0                             #2596.38
..LN2239:
        movdqa    -120(%ebp), %xmm1                             #2596.46
..LN2241:
        pand      %xmm1, %xmm0                                  #2596.22
..LN2243:
        movdqa    %xmm0, -264(%ebp)                             #2596.17
..LN2245:
        movdqa    -264(%ebp), %xmm0                             #2597.22
        movdqa    -136(%ebp), %xmm1                             #2597.22
        pcmpeqw   %xmm1, %xmm0                                  #2597.22
..LN2247:
        movdqa    %xmm0, -264(%ebp)                             #2597.17
..LN2249:
        movdqa    -264(%ebp), %xmm0                             #2598.42
..LN2251:
        pmovmskb  %xmm0, %eax                                   #2598.22
..LN2253:
        movl      %eax, -52(%ebp)                               #2598.17
..LN2255:
        movl      16(%ebx), %eax                                #2603.17
        andl      $-4, %eax                                     #2603.17
        movl      %eax, 16(%ebx)                                #2603.17
..LN2257:
        movl      -28(%ebp), %eax                               #2606.29
        orl       -24(%ebp), %eax                               #2606.29
..LN2259:
        orl       -32(%ebp), %eax                               #2606.34
..LN2261:
        orl       -52(%ebp), %eax                               #2606.39
..LN2263:
        je        ..B20.34      # Prob 50%                      #2606.47
                                # LOE
..B20.18:                       # Preds ..B20.17
..LN2265:
        pushl     %edi                                          #2624.30
..LN2267:
        movl      -24(%ebp), %eax                               #2624.58
        movw      %ax, (%esp)                                   #2624.58
..LN2269:
        call      uint16_lsbit                                  #2624.30
                                # LOE eax
..B20.51:                       # Preds ..B20.18
        popl      %ecx                                          #2624.30
        movl      %eax, -72(%ebp)                               #2624.30
                                # LOE
..B20.19:                       # Preds ..B20.51
        movl      -72(%ebp), %eax                               #2624.30
        movswl    %ax, %eax                                     #2624.30
..LN2271:
        movl      %eax, -88(%ebp)                               #2624.28
..LN2273:
        pushl     %edi                                          #2625.30
..LN2275:
        movl      -28(%ebp), %eax                               #2625.58
        movw      %ax, (%esp)                                   #2625.58
..LN2277:
        call      uint16_lsbit                                  #2625.30
                                # LOE eax
..B20.52:                       # Preds ..B20.19
        popl      %ecx                                          #2625.30
        movl      %eax, -68(%ebp)                               #2625.30
                                # LOE
..B20.20:                       # Preds ..B20.52
        movl      -68(%ebp), %eax                               #2625.30
        movswl    %ax, %eax                                     #2625.30
..LN2279:
        movl      %eax, -84(%ebp)                               #2625.28
..LN2281:
        pushl     %edi                                          #2626.30
..LN2283:
        movl      -32(%ebp), %eax                               #2626.58
        movw      %ax, (%esp)                                   #2626.58
..LN2285:
        call      uint16_lsbit                                  #2626.30
                                # LOE eax
..B20.53:                       # Preds ..B20.20
        popl      %ecx                                          #2626.30
        movl      %eax, -64(%ebp)                               #2626.30
                                # LOE
..B20.21:                       # Preds ..B20.53
        movl      -64(%ebp), %eax                               #2626.30
        movswl    %ax, %eax                                     #2626.30
..LN2287:
        movl      %eax, -80(%ebp)                               #2626.28
..LN2289:
        pushl     %edi                                          #2627.30
..LN2291:
        movl      -52(%ebp), %eax                               #2627.58
        movw      %ax, (%esp)                                   #2627.58
..LN2293:
        call      uint16_lsbit                                  #2627.30
                                # LOE eax
..B20.54:                       # Preds ..B20.21
        popl      %ecx                                          #2627.30
        movl      %eax, -60(%ebp)                               #2627.30
                                # LOE
..B20.22:                       # Preds ..B20.54
        movl      -60(%ebp), %eax                               #2627.30
        movswl    %ax, %eax                                     #2627.30
..LN2295:
        movl      %eax, -76(%ebp)                               #2627.28
..LN2297:
        shll      $2, -88(%ebp)                                 #2632.34
..LN2299:
        movl      -84(%ebp), %eax                               #2633.28
..LN2301:
        lea       1(,%eax,4), %eax                              #2633.40
..LN2303:
        movl      %eax, -84(%ebp)                               #2633.21
..LN2305:
        movl      -80(%ebp), %eax                               #2634.28
..LN2307:
        lea       2(,%eax,4), %eax                              #2634.40
..LN2309:
        movl      %eax, -80(%ebp)                               #2634.21
..LN2311:
        movl      -76(%ebp), %eax                               #2635.28
..LN2313:
        lea       3(,%eax,4), %eax                              #2635.40
..LN2315:
        movl      %eax, -76(%ebp)                               #2635.21
..LN2317:
        movl      -24(%ebp), %eax                               #2639.26
..LN2319:
        testl     %eax, %eax                                    #2639.32
        je        ..B20.25      # Prob 50%                      #2639.32
                                # LOE
..B20.23:                       # Preds ..B20.22
..LN2321:
        movl      -88(%ebp), %eax                               #2639.43
        addl      16(%ebx), %eax                                #2639.43
..LN2323:
        movl      -44(%ebp), %edx                               #2639.49
        cmpl      %edx, %eax                                    #2639.49
        ja        ..B20.25      # Prob 50%                      #2639.49
                                # LOE
..B20.24:                       # Preds ..B20.23
..LN2325:
        movl      $1, %eax                                      #2639.63
        leave                                                   #2639.63
..___tag_value_eval_2na_16.434:                                 #
        movl      %ebx, %esp                                    #2639.63
        popl      %ebx                                          #2639.63
..___tag_value_eval_2na_16.435:                                 #
        ret                                                     #2639.63
..___tag_value_eval_2na_16.437:                                 #
                                # LOE
..B20.25:                       # Preds ..B20.23 ..B20.22
..LN2327:
        movl      -28(%ebp), %eax                               #2640.26
..LN2329:
        testl     %eax, %eax                                    #2640.32
        je        ..B20.28      # Prob 50%                      #2640.32
                                # LOE
..B20.26:                       # Preds ..B20.25
..LN2331:
        movl      -84(%ebp), %eax                               #2640.43
        addl      16(%ebx), %eax                                #2640.43
..LN2333:
        movl      -44(%ebp), %edx                               #2640.49
        cmpl      %edx, %eax                                    #2640.49
        ja        ..B20.28      # Prob 50%                      #2640.49
                                # LOE
..B20.27:                       # Preds ..B20.26
..LN2335:
        movl      $1, %eax                                      #2640.63
        leave                                                   #2640.63
..___tag_value_eval_2na_16.441:                                 #
        movl      %ebx, %esp                                    #2640.63
        popl      %ebx                                          #2640.63
..___tag_value_eval_2na_16.442:                                 #
        ret                                                     #2640.63
..___tag_value_eval_2na_16.444:                                 #
                                # LOE
..B20.28:                       # Preds ..B20.26 ..B20.25
..LN2337:
        movl      -32(%ebp), %eax                               #2641.26
..LN2339:
        testl     %eax, %eax                                    #2641.32
        je        ..B20.31      # Prob 50%                      #2641.32
                                # LOE
..B20.29:                       # Preds ..B20.28
..LN2341:
        movl      -80(%ebp), %eax                               #2641.43
        addl      16(%ebx), %eax                                #2641.43
..LN2343:
        movl      -44(%ebp), %edx                               #2641.49
        cmpl      %edx, %eax                                    #2641.49
        ja        ..B20.31      # Prob 50%                      #2641.49
                                # LOE
..B20.30:                       # Preds ..B20.29
..LN2345:
        movl      $1, %eax                                      #2641.63
        leave                                                   #2641.63
..___tag_value_eval_2na_16.448:                                 #
        movl      %ebx, %esp                                    #2641.63
        popl      %ebx                                          #2641.63
..___tag_value_eval_2na_16.449:                                 #
        ret                                                     #2641.63
..___tag_value_eval_2na_16.451:                                 #
                                # LOE
..B20.31:                       # Preds ..B20.29 ..B20.28
..LN2347:
        movl      -52(%ebp), %eax                               #2642.26
..LN2349:
        testl     %eax, %eax                                    #2642.32
        je        ..B20.34      # Prob 50%                      #2642.32
                                # LOE
..B20.32:                       # Preds ..B20.31
..LN2351:
        movl      -76(%ebp), %eax                               #2642.43
        addl      16(%ebx), %eax                                #2642.43
..LN2353:
        movl      -44(%ebp), %edx                               #2642.49
        cmpl      %edx, %eax                                    #2642.49
        ja        ..B20.34      # Prob 50%                      #2642.49
                                # LOE
..B20.33:                       # Preds ..B20.32
..LN2355:
        movl      $1, %eax                                      #2642.63
        leave                                                   #2642.63
..___tag_value_eval_2na_16.455:                                 #
        movl      %ebx, %esp                                    #2642.63
        popl      %ebx                                          #2642.63
..___tag_value_eval_2na_16.456:                                 #
        ret                                                     #2642.63
..___tag_value_eval_2na_16.458:                                 #
                                # LOE
..B20.34:                       # Preds ..B20.32 ..B20.31 ..B20.17
..LN2357:
        movl      16(%ebx), %eax                                #2647.17
        addl      $4, %eax                                      #2647.17
        movl      %eax, 16(%ebx)                                #2647.17
..LN2359:
        movl      16(%ebx), %eax                                #2650.22
..LN2361:
        movl      -44(%ebp), %edx                               #2650.28
        cmpl      %edx, %eax                                    #2650.28
        jbe       ..B20.36      # Prob 50%                      #2650.28
                                # LOE
..B20.35:                       # Preds ..B20.34
..LN2363:
        xorl      %eax, %eax                                    #2651.28
        leave                                                   #2651.28
..___tag_value_eval_2na_16.462:                                 #
        movl      %ebx, %esp                                    #2651.28
        popl      %ebx                                          #2651.28
..___tag_value_eval_2na_16.463:                                 #
        ret                                                     #2651.28
..___tag_value_eval_2na_16.465:                                 #
                                # LOE
..B20.36:                       # Preds ..B20.34
..LN2365:
        movl      -20(%ebp), %eax                               #2654.25
        decl      %eax                                          #2654.25
        movl      %eax, -20(%ebp)                               #2654.25
..LN2367:
        je        ..B20.41      # Prob 50%                      #2654.39
                                # LOE
..B20.37:                       # Preds ..B20.36
..LN2369:
        movdqa    -248(%ebp), %xmm0                             #2659.43
..LN2371:
        psrldq    $1, %xmm0                                     #2659.26
..LN2373:
        movdqa    %xmm0, -248(%ebp)                             #2659.17
..LN2375:
        movl      -36(%ebp), %eax                               #2662.22
..LN2377:
        movl      -40(%ebp), %edx                               #2662.26
        cmpl      %edx, %eax                                    #2662.26
        jae       ..B20.39      # Prob 50%                      #2662.26
                                # LOE
..B20.38:                       # Preds ..B20.37
..LN2379:
        sarl      $8, -56(%ebp)                                 #2664.21
..LN2381:
        movl      -36(%ebp), %eax                               #2665.37
        movzbl    (%eax), %eax                                  #2665.37
        movzbl    %al, %eax                                     #2665.37
..LN2383:
        shll      $8, %eax                                      #2665.48
..LN2385:
        orl       -56(%ebp), %eax                               #2665.21
        movl      %eax, -56(%ebp)                               #2665.21
..LN2387:
        movdqa    -248(%ebp), %xmm0                             #2666.49
..LN2389:
        movl      -56(%ebp), %eax                               #2666.57
..LN2391:
        pinsrw    $7, %eax, %xmm0                               #2666.30
..LN2393:
        movdqa    %xmm0, -248(%ebp)                             #2666.21
                                # LOE
..B20.39:                       # Preds ..B20.38 ..B20.37
..LN2395:
        incl      -36(%ebp)                                     #2670.20
        jmp       ..B20.12      # Prob 100%                     #2670.20
                                # LOE
..B20.41:                       # Preds ..B20.36 ..B20.12
..LN2397:
        movl      -48(%ebp), %eax                               #2675.18
..LN2399:
        movl      -40(%ebp), %edx                               #2675.25
        cmpl      %edx, %eax                                    #2675.25
        jae       ..B20.46      # Prob 50%                      #2675.25
                                # LOE
..B20.42:                       # Preds ..B20.41
..LN2401:
        movl      16(%ebx), %eax                                #2679.13
        addl      $56, %eax                                     #2679.13
        movl      %eax, 16(%ebx)                                #2679.13
..LN2403:
        movl      16(%ebx), %eax                                #2680.18
..LN2405:
        movl      -44(%ebp), %edx                               #2680.24
        cmpl      %edx, %eax                                    #2680.24
        ja        ..B20.46      # Prob 50%                      #2680.24
                                # LOE
..B20.43:                       # Preds ..B20.42
..LN2407:
        addl      $-8, %esp                                     #2685.22
..LN2409:
        movl      -48(%ebp), %eax                               #2685.41
        movl      %eax, (%esp)                                  #2685.41
..LN2411:
        movl      -40(%ebp), %eax                               #2685.46
        movl      %eax, 4(%esp)                                 #2685.46
..LN2413:
        call      prime_buffer_2na                              #2685.22
                                # LOE xmm0
..B20.55:                       # Preds ..B20.43
        addl      $8, %esp                                      #2685.22
        movdqa    %xmm0, -280(%ebp)                             #2685.22
                                # LOE
..B20.44:                       # Preds ..B20.55
..LN2415:
        movdqa    -280(%ebp), %xmm0                             #2685.13
        movdqa    %xmm0, -248(%ebp)                             #2685.13
..LN2417:
        addl      $16, -48(%ebp)                                #2763.13
..LN2419:
        movl      -48(%ebp), %eax                               #2765.17
..LN2421:
        movl      %eax, -36(%ebp)                               #2765.13
..LN2423:
        movl      -48(%ebp), %eax                               #2766.18
..LN2425:
        movl      -40(%ebp), %edx                               #2766.24
        cmpl      %edx, %eax                                    #2766.24
        jae       ..B20.10      # Prob 50%                      #2766.24
                                # LOE
..B20.45:                       # Preds ..B20.44
..LN2427:
        movl      -48(%ebp), %eax                               #2767.32
        movzbl    -1(%eax), %eax                                #2767.32
        movzbl    %al, %eax                                     #2767.32
..LN2429:
        shll      $8, %eax                                      #2767.46
..LN2431:
        movl      %eax, -56(%ebp)                               #2767.17
        jmp       ..B20.10      # Prob 100%                     #2767.17
                                # LOE
..B20.46:                       # Preds ..B20.42 ..B20.41 ..B20.10
..LN2433:
        xorl      %eax, %eax                                    #2776.12
        leave                                                   #2776.12
..___tag_value_eval_2na_16.469:                                 #
        movl      %ebx, %esp                                    #2776.12
        popl      %ebx                                          #2776.12
..___tag_value_eval_2na_16.470:                                 #
        ret                                                     #2776.12
        .align    2,0x90
..___tag_value_eval_2na_16.472:                                 #
                                # LOE
# mark_end;
	.type	eval_2na_16,@function
	.size	eval_2na_16,.-eval_2na_16
.LNeval_2na_16:
	.data
# -- End  eval_2na_16
	.section .rodata, "a"
	.align 4
__$U7:
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
	.type	__$U7,@object
	.size	__$U7,12
	.text
# -- Begin  eval_2na_32
# mark_begin;
       .align    2,0x90
eval_2na_32:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B21.1:                        # Preds ..B21.0
..___tag_value_eval_2na_32.475:                                 #
..LN2435:
        pushl     %ebx                                          #2782.1
..___tag_value_eval_2na_32.480:                                 #
        movl      %esp, %ebx                                    #2782.1
..___tag_value_eval_2na_32.481:                                 #
        andl      $-16, %esp                                    #2782.1
        pushl     %ebp                                          #2782.1
        pushl     %ebp                                          #2782.1
        movl      4(%ebx), %ebp                                 #2782.1
        movl      %ebp, 4(%esp)                                 #2782.1
        movl      %esp, %ebp                                    #2782.1
..___tag_value_eval_2na_32.483:                                 #
        subl      $296, %esp                                    #2782.1
        movl      %ebx, -296(%ebp)                              #2782.1
..LN2437:
        movl      8(%ebx), %eax                                 #2812.25
        movl      4(%eax), %eax                                 #2812.25
..LN2439:
        movl      %eax, -12(%ebp)                               #2812.23
..LN2441:
        movl      20(%ebx), %eax                                #2815.5
        movl      -12(%ebp), %edx                               #2815.5
        cmpl      %edx, %eax                                    #2815.5
        jae       ..B21.3       # Prob 50%                      #2815.5
                                # LOE
..B21.2:                        # Preds ..B21.1
        call      ..L485        # Prob 100%                     #
..L485:                                                         #
        popl      %eax                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L485], %eax     #
        addl      $-16, %esp                                    #2815.5
        lea       _2__STRING.9@GOTOFF(%eax), %edx               #2815.5
        movl      %edx, (%esp)                                  #2815.5
        lea       _2__STRING.1@GOTOFF(%eax), %edx               #2815.5
        movl      %edx, 4(%esp)                                 #2815.5
        movl      $2815, 8(%esp)                                #2815.5
        lea       __$U7@GOTOFF(%eax), %edx                      #2815.5
        movl      %edx, 12(%esp)                                #2815.5
        movl      %eax, %ebx                                    #2815.5
..___tag_value_eval_2na_32.486:                                 #2815.5
        call      __assert_fail@PLT                             #2815.5
        movl      -296(%ebp), %ebx                              #2815.5
..___tag_value_eval_2na_32.487:                                 #
                                # LOE
..B21.49:                       # Preds ..B21.2
        addl      $16, %esp                                     #2815.5
                                # LOE
..B21.3:                        # Preds ..B21.1
..LN2443:
        movl      16(%ebx), %eax                                #2819.12
..LN2445:
        addl      20(%ebx), %eax                                #2819.5
        movl      %eax, 20(%ebx)                                #2819.5
..LN2447:
        movl      16(%ebx), %eax                                #2822.42
..LN2449:
        shrl      $2, %eax                                      #2822.49
..LN2451:
        addl      12(%ebx), %eax                                #2822.30
..LN2453:
        movl      %eax, -48(%ebp)                               #2822.5
..LN2455:
        movl      -12(%ebp), %eax                               #2825.18
        negl      %eax                                          #2825.18
        addl      20(%ebx), %eax                                #2825.18
..LN2457:
        movl      %eax, -44(%ebp)                               #2825.5
..LN2459:
        movl      20(%ebx), %eax                                #2828.44
..LN2461:
        addl      $3, %eax                                      #2828.50
..LN2463:
        shrl      $2, %eax                                      #2828.57
..LN2465:
        addl      12(%ebx), %eax                                #2828.30
..LN2467:
        movl      %eax, -40(%ebp)                               #2828.5
..LN2469:
        addl      $-8, %esp                                     #2831.14
..LN2471:
        movl      -48(%ebp), %eax                               #2831.33
        movl      %eax, (%esp)                                  #2831.33
..LN2473:
        movl      -40(%ebp), %eax                               #2831.38
        movl      %eax, 4(%esp)                                 #2831.38
..LN2475:
        call      prime_buffer_2na                              #2831.14
                                # LOE xmm0
..B21.50:                       # Preds ..B21.3
        addl      $8, %esp                                      #2831.14
        movdqa    %xmm0, -104(%ebp)                             #2831.14
                                # LOE
..B21.4:                        # Preds ..B21.50
..LN2477:
        movdqa    -104(%ebp), %xmm0                             #2831.5
        movdqa    %xmm0, -248(%ebp)                             #2831.5
..LN2479:
        addl      $16, -48(%ebp)                                #2832.5
..LN2481:
        movl      -48(%ebp), %eax                               #2834.9
..LN2483:
        movl      %eax, -36(%ebp)                               #2834.5
..LN2485:
        movl      -48(%ebp), %eax                               #2837.10
..LN2487:
        movl      -40(%ebp), %edx                               #2837.16
        cmpl      %edx, %eax                                    #2837.16
        jae       ..B21.6       # Prob 50%                      #2837.16
                                # LOE
..B21.5:                        # Preds ..B21.4
..LN2489:
        movl      -48(%ebp), %eax                               #2838.24
        movzbl    -1(%eax), %eax                                #2838.24
        movzbl    %al, %eax                                     #2838.24
..LN2491:
        shll      $8, %eax                                      #2838.38
..LN2493:
        movl      %eax, -56(%ebp)                               #2838.9
                                # LOE
..B21.6:                        # Preds ..B21.5 ..B21.4
..LN2495:
        movl      8(%ebx), %eax                                 #2842.5
        movdqa    16(%eax), %xmm0                               #2842.5
        movdqa    %xmm0, -232(%ebp)                             #2842.5
        movl      8(%ebx), %eax                                 #2842.5
        movdqa    32(%eax), %xmm0                               #2842.5
        movdqa    %xmm0, -216(%ebp)                             #2842.5
        movl      8(%ebx), %eax                                 #2842.5
        movdqa    48(%eax), %xmm0                               #2842.5
        movdqa    %xmm0, -200(%ebp)                             #2842.5
        movl      8(%ebx), %eax                                 #2842.5
        movdqa    64(%eax), %xmm0                               #2842.5
        movdqa    %xmm0, -184(%ebp)                             #2842.5
        movl      8(%ebx), %eax                                 #2842.5
        movdqa    80(%eax), %xmm0                               #2842.5
        movdqa    %xmm0, -168(%ebp)                             #2842.5
        movl      8(%ebx), %eax                                 #2842.5
        movdqa    96(%eax), %xmm0                               #2842.5
        movdqa    %xmm0, -152(%ebp)                             #2842.5
        movl      8(%ebx), %eax                                 #2842.5
        movdqa    112(%eax), %xmm0                              #2842.5
        movdqa    %xmm0, -136(%ebp)                             #2842.5
        movl      8(%ebx), %eax                                 #2842.5
        movdqa    128(%eax), %xmm0                              #2842.5
        movdqa    %xmm0, -120(%ebp)                             #2842.5
..LN2497:
        xorl      %eax, %eax                                    #2845.15
        movl      %eax, -32(%ebp)                               #2845.15
..LN2499:
        movl      %eax, -28(%ebp)                               #2845.10
..LN2501:
        movl      %eax, -24(%ebp)                               #2845.5
..LN2503:
        movl      $4, -20(%ebp)                                 #2849.5
..LN2505:
        movl      16(%ebx), %eax                                #2854.14
..LN2507:
        andl      $3, %eax                                      #2854.20
        movl      %eax, -16(%ebp)                               #2854.20
        je        ..B21.14      # Prob 50%                      #2854.20
                                # LOE
..B21.7:                        # Preds ..B21.6
        movl      -16(%ebp), %eax                               #2854.20
        cmpl      $1, %eax                                      #2854.20
        je        ..B21.15      # Prob 50%                      #2854.20
                                # LOE
..B21.8:                        # Preds ..B21.7
        movl      -16(%ebp), %eax                               #2854.20
        cmpl      $2, %eax                                      #2854.20
        je        ..B21.16      # Prob 50%                      #2854.20
                                # LOE
..B21.9:                        # Preds ..B21.8
        movl      -16(%ebp), %eax                               #2854.20
        cmpl      $3, %eax                                      #2854.20
        je        ..B21.17      # Prob 50%                      #2854.20
                                # LOE
..B21.10:                       # Preds ..B21.45 ..B21.44 ..B21.9
..LN2509:
        movl      $1, %eax                                      #2859.9
        testl     %eax, %eax                                    #2859.9
        je        ..B21.46      # Prob 100%                     #2859.9
                                # LOE
..B21.11:                       # Preds ..B21.10
..LN2511:
        movl      $4, -20(%ebp)                                 #2861.13
                                # LOE
..B21.12:                       # Preds ..B21.39 ..B21.11
..LN2513:
        movl      $1, %eax                                      #2864.13
        testl     %eax, %eax                                    #2864.13
        je        ..B21.41      # Prob 100%                     #2864.13
                                # LOE
..B21.14:                       # Preds ..B21.6 ..B21.12
..LN2515:
        movdqa    -248(%ebp), %xmm0                             #2871.38
..LN2517:
        movdqa    -216(%ebp), %xmm1                             #2871.46
..LN2519:
        pand      %xmm1, %xmm0                                  #2871.22
..LN2521:
        movdqa    %xmm0, -264(%ebp)                             #2871.17
..LN2523:
        movdqa    -264(%ebp), %xmm0                             #2872.22
        movdqa    -232(%ebp), %xmm1                             #2872.22
        pcmpeqd   %xmm1, %xmm0                                  #2872.22
..LN2525:
        movdqa    %xmm0, -264(%ebp)                             #2872.17
..LN2527:
        movdqa    -264(%ebp), %xmm0                             #2873.42
..LN2529:
        pmovmskb  %xmm0, %eax                                   #2873.22
..LN2531:
        movl      %eax, -24(%ebp)                               #2873.17
                                # LOE
..B21.15:                       # Preds ..B21.7 ..B21.14
..LN2533:
        movdqa    -248(%ebp), %xmm0                             #2877.38
..LN2535:
        movdqa    -184(%ebp), %xmm1                             #2877.46
..LN2537:
        pand      %xmm1, %xmm0                                  #2877.22
..LN2539:
        movdqa    %xmm0, -264(%ebp)                             #2877.17
..LN2541:
        movdqa    -264(%ebp), %xmm0                             #2878.22
        movdqa    -200(%ebp), %xmm1                             #2878.22
        pcmpeqd   %xmm1, %xmm0                                  #2878.22
..LN2543:
        movdqa    %xmm0, -264(%ebp)                             #2878.17
..LN2545:
        movdqa    -264(%ebp), %xmm0                             #2879.42
..LN2547:
        pmovmskb  %xmm0, %eax                                   #2879.22
..LN2549:
        movl      %eax, -28(%ebp)                               #2879.17
                                # LOE
..B21.16:                       # Preds ..B21.8 ..B21.15
..LN2551:
        movdqa    -248(%ebp), %xmm0                             #2883.38
..LN2553:
        movdqa    -152(%ebp), %xmm1                             #2883.46
..LN2555:
        pand      %xmm1, %xmm0                                  #2883.22
..LN2557:
        movdqa    %xmm0, -264(%ebp)                             #2883.17
..LN2559:
        movdqa    -264(%ebp), %xmm0                             #2884.22
        movdqa    -168(%ebp), %xmm1                             #2884.22
        pcmpeqd   %xmm1, %xmm0                                  #2884.22
..LN2561:
        movdqa    %xmm0, -264(%ebp)                             #2884.17
..LN2563:
        movdqa    -264(%ebp), %xmm0                             #2885.42
..LN2565:
        pmovmskb  %xmm0, %eax                                   #2885.22
..LN2567:
        movl      %eax, -32(%ebp)                               #2885.17
                                # LOE
..B21.17:                       # Preds ..B21.9 ..B21.16
..LN2569:
        movdqa    -248(%ebp), %xmm0                             #2889.38
..LN2571:
        movdqa    -120(%ebp), %xmm1                             #2889.46
..LN2573:
        pand      %xmm1, %xmm0                                  #2889.22
..LN2575:
        movdqa    %xmm0, -264(%ebp)                             #2889.17
..LN2577:
        movdqa    -264(%ebp), %xmm0                             #2890.22
        movdqa    -136(%ebp), %xmm1                             #2890.22
        pcmpeqd   %xmm1, %xmm0                                  #2890.22
..LN2579:
        movdqa    %xmm0, -264(%ebp)                             #2890.17
..LN2581:
        movdqa    -264(%ebp), %xmm0                             #2891.42
..LN2583:
        pmovmskb  %xmm0, %eax                                   #2891.22
..LN2585:
        movl      %eax, -52(%ebp)                               #2891.17
..LN2587:
        movl      16(%ebx), %eax                                #2896.17
        andl      $-4, %eax                                     #2896.17
        movl      %eax, 16(%ebx)                                #2896.17
..LN2589:
        movl      -28(%ebp), %eax                               #2899.29
        orl       -24(%ebp), %eax                               #2899.29
..LN2591:
        orl       -32(%ebp), %eax                               #2899.34
..LN2593:
        orl       -52(%ebp), %eax                               #2899.39
..LN2595:
        je        ..B21.34      # Prob 50%                      #2899.47
                                # LOE
..B21.18:                       # Preds ..B21.17
..LN2597:
        pushl     %edi                                          #2917.30
..LN2599:
        movl      -24(%ebp), %eax                               #2917.58
        movw      %ax, (%esp)                                   #2917.58
..LN2601:
        call      uint16_lsbit                                  #2917.30
                                # LOE eax
..B21.51:                       # Preds ..B21.18
        popl      %ecx                                          #2917.30
        movl      %eax, -72(%ebp)                               #2917.30
                                # LOE
..B21.19:                       # Preds ..B21.51
        movl      -72(%ebp), %eax                               #2917.30
        movswl    %ax, %eax                                     #2917.30
..LN2603:
        movl      %eax, -88(%ebp)                               #2917.28
..LN2605:
        pushl     %edi                                          #2918.30
..LN2607:
        movl      -28(%ebp), %eax                               #2918.58
        movw      %ax, (%esp)                                   #2918.58
..LN2609:
        call      uint16_lsbit                                  #2918.30
                                # LOE eax
..B21.52:                       # Preds ..B21.19
        popl      %ecx                                          #2918.30
        movl      %eax, -68(%ebp)                               #2918.30
                                # LOE
..B21.20:                       # Preds ..B21.52
        movl      -68(%ebp), %eax                               #2918.30
        movswl    %ax, %eax                                     #2918.30
..LN2611:
        movl      %eax, -84(%ebp)                               #2918.28
..LN2613:
        pushl     %edi                                          #2919.30
..LN2615:
        movl      -32(%ebp), %eax                               #2919.58
        movw      %ax, (%esp)                                   #2919.58
..LN2617:
        call      uint16_lsbit                                  #2919.30
                                # LOE eax
..B21.53:                       # Preds ..B21.20
        popl      %ecx                                          #2919.30
        movl      %eax, -64(%ebp)                               #2919.30
                                # LOE
..B21.21:                       # Preds ..B21.53
        movl      -64(%ebp), %eax                               #2919.30
        movswl    %ax, %eax                                     #2919.30
..LN2619:
        movl      %eax, -80(%ebp)                               #2919.28
..LN2621:
        pushl     %edi                                          #2920.30
..LN2623:
        movl      -52(%ebp), %eax                               #2920.58
        movw      %ax, (%esp)                                   #2920.58
..LN2625:
        call      uint16_lsbit                                  #2920.30
                                # LOE eax
..B21.54:                       # Preds ..B21.21
        popl      %ecx                                          #2920.30
        movl      %eax, -60(%ebp)                               #2920.30
                                # LOE
..B21.22:                       # Preds ..B21.54
        movl      -60(%ebp), %eax                               #2920.30
        movswl    %ax, %eax                                     #2920.30
..LN2627:
        movl      %eax, -76(%ebp)                               #2920.28
..LN2629:
        shll      $2, -88(%ebp)                                 #2925.34
..LN2631:
        movl      -84(%ebp), %eax                               #2926.28
..LN2633:
        lea       1(,%eax,4), %eax                              #2926.40
..LN2635:
        movl      %eax, -84(%ebp)                               #2926.21
..LN2637:
        movl      -80(%ebp), %eax                               #2927.28
..LN2639:
        lea       2(,%eax,4), %eax                              #2927.40
..LN2641:
        movl      %eax, -80(%ebp)                               #2927.21
..LN2643:
        movl      -76(%ebp), %eax                               #2928.28
..LN2645:
        lea       3(,%eax,4), %eax                              #2928.40
..LN2647:
        movl      %eax, -76(%ebp)                               #2928.21
..LN2649:
        movl      -24(%ebp), %eax                               #2932.26
..LN2651:
        testl     %eax, %eax                                    #2932.32
        je        ..B21.25      # Prob 50%                      #2932.32
                                # LOE
..B21.23:                       # Preds ..B21.22
..LN2653:
        movl      -88(%ebp), %eax                               #2932.43
        addl      16(%ebx), %eax                                #2932.43
..LN2655:
        movl      -44(%ebp), %edx                               #2932.49
        cmpl      %edx, %eax                                    #2932.49
        ja        ..B21.25      # Prob 50%                      #2932.49
                                # LOE
..B21.24:                       # Preds ..B21.23
..LN2657:
        movl      $1, %eax                                      #2932.63
        leave                                                   #2932.63
..___tag_value_eval_2na_32.489:                                 #
        movl      %ebx, %esp                                    #2932.63
        popl      %ebx                                          #2932.63
..___tag_value_eval_2na_32.490:                                 #
        ret                                                     #2932.63
..___tag_value_eval_2na_32.492:                                 #
                                # LOE
..B21.25:                       # Preds ..B21.23 ..B21.22
..LN2659:
        movl      -28(%ebp), %eax                               #2933.26
..LN2661:
        testl     %eax, %eax                                    #2933.32
        je        ..B21.28      # Prob 50%                      #2933.32
                                # LOE
..B21.26:                       # Preds ..B21.25
..LN2663:
        movl      -84(%ebp), %eax                               #2933.43
        addl      16(%ebx), %eax                                #2933.43
..LN2665:
        movl      -44(%ebp), %edx                               #2933.49
        cmpl      %edx, %eax                                    #2933.49
        ja        ..B21.28      # Prob 50%                      #2933.49
                                # LOE
..B21.27:                       # Preds ..B21.26
..LN2667:
        movl      $1, %eax                                      #2933.63
        leave                                                   #2933.63
..___tag_value_eval_2na_32.496:                                 #
        movl      %ebx, %esp                                    #2933.63
        popl      %ebx                                          #2933.63
..___tag_value_eval_2na_32.497:                                 #
        ret                                                     #2933.63
..___tag_value_eval_2na_32.499:                                 #
                                # LOE
..B21.28:                       # Preds ..B21.26 ..B21.25
..LN2669:
        movl      -32(%ebp), %eax                               #2934.26
..LN2671:
        testl     %eax, %eax                                    #2934.32
        je        ..B21.31      # Prob 50%                      #2934.32
                                # LOE
..B21.29:                       # Preds ..B21.28
..LN2673:
        movl      -80(%ebp), %eax                               #2934.43
        addl      16(%ebx), %eax                                #2934.43
..LN2675:
        movl      -44(%ebp), %edx                               #2934.49
        cmpl      %edx, %eax                                    #2934.49
        ja        ..B21.31      # Prob 50%                      #2934.49
                                # LOE
..B21.30:                       # Preds ..B21.29
..LN2677:
        movl      $1, %eax                                      #2934.63
        leave                                                   #2934.63
..___tag_value_eval_2na_32.503:                                 #
        movl      %ebx, %esp                                    #2934.63
        popl      %ebx                                          #2934.63
..___tag_value_eval_2na_32.504:                                 #
        ret                                                     #2934.63
..___tag_value_eval_2na_32.506:                                 #
                                # LOE
..B21.31:                       # Preds ..B21.29 ..B21.28
..LN2679:
        movl      -52(%ebp), %eax                               #2935.26
..LN2681:
        testl     %eax, %eax                                    #2935.32
        je        ..B21.34      # Prob 50%                      #2935.32
                                # LOE
..B21.32:                       # Preds ..B21.31
..LN2683:
        movl      -76(%ebp), %eax                               #2935.43
        addl      16(%ebx), %eax                                #2935.43
..LN2685:
        movl      -44(%ebp), %edx                               #2935.49
        cmpl      %edx, %eax                                    #2935.49
        ja        ..B21.34      # Prob 50%                      #2935.49
                                # LOE
..B21.33:                       # Preds ..B21.32
..LN2687:
        movl      $1, %eax                                      #2935.63
        leave                                                   #2935.63
..___tag_value_eval_2na_32.510:                                 #
        movl      %ebx, %esp                                    #2935.63
        popl      %ebx                                          #2935.63
..___tag_value_eval_2na_32.511:                                 #
        ret                                                     #2935.63
..___tag_value_eval_2na_32.513:                                 #
                                # LOE
..B21.34:                       # Preds ..B21.32 ..B21.31 ..B21.17
..LN2689:
        movl      16(%ebx), %eax                                #2940.17
        addl      $4, %eax                                      #2940.17
        movl      %eax, 16(%ebx)                                #2940.17
..LN2691:
        movl      16(%ebx), %eax                                #2943.22
..LN2693:
        movl      -44(%ebp), %edx                               #2943.28
        cmpl      %edx, %eax                                    #2943.28
        jbe       ..B21.36      # Prob 50%                      #2943.28
                                # LOE
..B21.35:                       # Preds ..B21.34
..LN2695:
        xorl      %eax, %eax                                    #2944.28
        leave                                                   #2944.28
..___tag_value_eval_2na_32.517:                                 #
        movl      %ebx, %esp                                    #2944.28
        popl      %ebx                                          #2944.28
..___tag_value_eval_2na_32.518:                                 #
        ret                                                     #2944.28
..___tag_value_eval_2na_32.520:                                 #
                                # LOE
..B21.36:                       # Preds ..B21.34
..LN2697:
        movl      -20(%ebp), %eax                               #2947.25
        decl      %eax                                          #2947.25
        movl      %eax, -20(%ebp)                               #2947.25
..LN2699:
        je        ..B21.41      # Prob 50%                      #2947.39
                                # LOE
..B21.37:                       # Preds ..B21.36
..LN2701:
        movdqa    -248(%ebp), %xmm0                             #2952.43
..LN2703:
        psrldq    $1, %xmm0                                     #2952.26
..LN2705:
        movdqa    %xmm0, -248(%ebp)                             #2952.17
..LN2707:
        movl      -36(%ebp), %eax                               #2955.22
..LN2709:
        movl      -40(%ebp), %edx                               #2955.26
        cmpl      %edx, %eax                                    #2955.26
        jae       ..B21.39      # Prob 50%                      #2955.26
                                # LOE
..B21.38:                       # Preds ..B21.37
..LN2711:
        sarl      $8, -56(%ebp)                                 #2957.21
..LN2713:
        movl      -36(%ebp), %eax                               #2958.37
        movzbl    (%eax), %eax                                  #2958.37
        movzbl    %al, %eax                                     #2958.37
..LN2715:
        shll      $8, %eax                                      #2958.48
..LN2717:
        orl       -56(%ebp), %eax                               #2958.21
        movl      %eax, -56(%ebp)                               #2958.21
..LN2719:
        movdqa    -248(%ebp), %xmm0                             #2959.49
..LN2721:
        movl      -56(%ebp), %eax                               #2959.57
..LN2723:
        pinsrw    $7, %eax, %xmm0                               #2959.30
..LN2725:
        movdqa    %xmm0, -248(%ebp)                             #2959.21
                                # LOE
..B21.39:                       # Preds ..B21.38 ..B21.37
..LN2727:
        incl      -36(%ebp)                                     #2963.20
        jmp       ..B21.12      # Prob 100%                     #2963.20
                                # LOE
..B21.41:                       # Preds ..B21.36 ..B21.12
..LN2729:
        movl      -48(%ebp), %eax                               #2968.18
..LN2731:
        movl      -40(%ebp), %edx                               #2968.25
        cmpl      %edx, %eax                                    #2968.25
        jae       ..B21.46      # Prob 50%                      #2968.25
                                # LOE
..B21.42:                       # Preds ..B21.41
..LN2733:
        movl      16(%ebx), %eax                                #2972.13
        addl      $48, %eax                                     #2972.13
        movl      %eax, 16(%ebx)                                #2972.13
..LN2735:
        movl      16(%ebx), %eax                                #2973.18
..LN2737:
        movl      -44(%ebp), %edx                               #2973.24
        cmpl      %edx, %eax                                    #2973.24
        ja        ..B21.46      # Prob 50%                      #2973.24
                                # LOE
..B21.43:                       # Preds ..B21.42
..LN2739:
        addl      $-8, %esp                                     #2978.22
..LN2741:
        movl      -48(%ebp), %eax                               #2978.41
        movl      %eax, (%esp)                                  #2978.41
..LN2743:
        movl      -40(%ebp), %eax                               #2978.46
        movl      %eax, 4(%esp)                                 #2978.46
..LN2745:
        call      prime_buffer_2na                              #2978.22
                                # LOE xmm0
..B21.55:                       # Preds ..B21.43
        addl      $8, %esp                                      #2978.22
        movdqa    %xmm0, -280(%ebp)                             #2978.22
                                # LOE
..B21.44:                       # Preds ..B21.55
..LN2747:
        movdqa    -280(%ebp), %xmm0                             #2978.13
        movdqa    %xmm0, -248(%ebp)                             #2978.13
..LN2749:
        addl      $16, -48(%ebp)                                #3056.13
..LN2751:
        movl      -48(%ebp), %eax                               #3058.17
..LN2753:
        movl      %eax, -36(%ebp)                               #3058.13
..LN2755:
        movl      -48(%ebp), %eax                               #3059.18
..LN2757:
        movl      -40(%ebp), %edx                               #3059.24
        cmpl      %edx, %eax                                    #3059.24
        jae       ..B21.10      # Prob 50%                      #3059.24
                                # LOE
..B21.45:                       # Preds ..B21.44
..LN2759:
        movl      -48(%ebp), %eax                               #3060.32
        movzbl    -1(%eax), %eax                                #3060.32
        movzbl    %al, %eax                                     #3060.32
..LN2761:
        shll      $8, %eax                                      #3060.46
..LN2763:
        movl      %eax, -56(%ebp)                               #3060.17
        jmp       ..B21.10      # Prob 100%                     #3060.17
                                # LOE
..B21.46:                       # Preds ..B21.42 ..B21.41 ..B21.10
..LN2765:
        xorl      %eax, %eax                                    #3069.12
        leave                                                   #3069.12
..___tag_value_eval_2na_32.524:                                 #
        movl      %ebx, %esp                                    #3069.12
        popl      %ebx                                          #3069.12
..___tag_value_eval_2na_32.525:                                 #
        ret                                                     #3069.12
        .align    2,0x90
..___tag_value_eval_2na_32.527:                                 #
                                # LOE
# mark_end;
	.type	eval_2na_32,@function
	.size	eval_2na_32,.-eval_2na_32
.LNeval_2na_32:
	.data
# -- End  eval_2na_32
	.section .rodata, "a"
	.align 4
__$U8:
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
	.type	__$U8,@object
	.size	__$U8,12
	.text
# -- Begin  eval_2na_64
# mark_begin;
       .align    2,0x90
eval_2na_64:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B22.1:                        # Preds ..B22.0
..___tag_value_eval_2na_64.530:                                 #
..LN2767:
        pushl     %ebx                                          #3075.1
..___tag_value_eval_2na_64.535:                                 #
        movl      %esp, %ebx                                    #3075.1
..___tag_value_eval_2na_64.536:                                 #
        andl      $-16, %esp                                    #3075.1
        pushl     %ebp                                          #3075.1
        pushl     %ebp                                          #3075.1
        movl      4(%ebx), %ebp                                 #3075.1
        movl      %ebp, 4(%esp)                                 #3075.1
        movl      %esp, %ebp                                    #3075.1
..___tag_value_eval_2na_64.538:                                 #
        subl      $296, %esp                                    #3075.1
        movl      %ebx, -296(%ebp)                              #3075.1
..LN2769:
        movl      8(%ebx), %eax                                 #3106.25
        movl      4(%eax), %eax                                 #3106.25
..LN2771:
        movl      %eax, -12(%ebp)                               #3106.23
..LN2773:
        movl      20(%ebx), %eax                                #3109.5
        movl      -12(%ebp), %edx                               #3109.5
        cmpl      %edx, %eax                                    #3109.5
        jae       ..B22.3       # Prob 50%                      #3109.5
                                # LOE
..B22.2:                        # Preds ..B22.1
        call      ..L540        # Prob 100%                     #
..L540:                                                         #
        popl      %eax                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L540], %eax     #
        addl      $-16, %esp                                    #3109.5
        lea       _2__STRING.9@GOTOFF(%eax), %edx               #3109.5
        movl      %edx, (%esp)                                  #3109.5
        lea       _2__STRING.1@GOTOFF(%eax), %edx               #3109.5
        movl      %edx, 4(%esp)                                 #3109.5
        movl      $3109, 8(%esp)                                #3109.5
        lea       __$U8@GOTOFF(%eax), %edx                      #3109.5
        movl      %edx, 12(%esp)                                #3109.5
        movl      %eax, %ebx                                    #3109.5
..___tag_value_eval_2na_64.541:                                 #3109.5
        call      __assert_fail@PLT                             #3109.5
        movl      -296(%ebp), %ebx                              #3109.5
..___tag_value_eval_2na_64.542:                                 #
                                # LOE
..B22.49:                       # Preds ..B22.2
        addl      $16, %esp                                     #3109.5
                                # LOE
..B22.3:                        # Preds ..B22.1
..LN2775:
        movl      16(%ebx), %eax                                #3113.12
..LN2777:
        addl      20(%ebx), %eax                                #3113.5
        movl      %eax, 20(%ebx)                                #3113.5
..LN2779:
        movl      16(%ebx), %eax                                #3116.42
..LN2781:
        shrl      $2, %eax                                      #3116.49
..LN2783:
        addl      12(%ebx), %eax                                #3116.30
..LN2785:
        movl      %eax, -48(%ebp)                               #3116.5
..LN2787:
        movl      -12(%ebp), %eax                               #3119.18
        negl      %eax                                          #3119.18
        addl      20(%ebx), %eax                                #3119.18
..LN2789:
        movl      %eax, -44(%ebp)                               #3119.5
..LN2791:
        movl      20(%ebx), %eax                                #3122.44
..LN2793:
        addl      $3, %eax                                      #3122.50
..LN2795:
        shrl      $2, %eax                                      #3122.57
..LN2797:
        addl      12(%ebx), %eax                                #3122.30
..LN2799:
        movl      %eax, -40(%ebp)                               #3122.5
..LN2801:
        addl      $-8, %esp                                     #3125.14
..LN2803:
        movl      -48(%ebp), %eax                               #3125.33
        movl      %eax, (%esp)                                  #3125.33
..LN2805:
        movl      -40(%ebp), %eax                               #3125.38
        movl      %eax, 4(%esp)                                 #3125.38
..LN2807:
        call      prime_buffer_2na                              #3125.14
                                # LOE xmm0
..B22.50:                       # Preds ..B22.3
        addl      $8, %esp                                      #3125.14
        movdqa    %xmm0, -104(%ebp)                             #3125.14
                                # LOE
..B22.4:                        # Preds ..B22.50
..LN2809:
        movdqa    -104(%ebp), %xmm0                             #3125.5
        movdqa    %xmm0, -248(%ebp)                             #3125.5
..LN2811:
        addl      $16, -48(%ebp)                                #3126.5
..LN2813:
        movl      -48(%ebp), %eax                               #3128.9
..LN2815:
        movl      %eax, -36(%ebp)                               #3128.5
..LN2817:
        movl      -48(%ebp), %eax                               #3131.10
..LN2819:
        movl      -40(%ebp), %edx                               #3131.16
        cmpl      %edx, %eax                                    #3131.16
        jae       ..B22.6       # Prob 50%                      #3131.16
                                # LOE
..B22.5:                        # Preds ..B22.4
..LN2821:
        movl      -48(%ebp), %eax                               #3132.24
        movzbl    -1(%eax), %eax                                #3132.24
        movzbl    %al, %eax                                     #3132.24
..LN2823:
        shll      $8, %eax                                      #3132.38
..LN2825:
        movl      %eax, -56(%ebp)                               #3132.9
                                # LOE
..B22.6:                        # Preds ..B22.5 ..B22.4
..LN2827:
        movl      8(%ebx), %eax                                 #3136.5
        movdqa    16(%eax), %xmm0                               #3136.5
        movdqa    %xmm0, -232(%ebp)                             #3136.5
        movl      8(%ebx), %eax                                 #3136.5
        movdqa    32(%eax), %xmm0                               #3136.5
        movdqa    %xmm0, -216(%ebp)                             #3136.5
        movl      8(%ebx), %eax                                 #3136.5
        movdqa    48(%eax), %xmm0                               #3136.5
        movdqa    %xmm0, -200(%ebp)                             #3136.5
        movl      8(%ebx), %eax                                 #3136.5
        movdqa    64(%eax), %xmm0                               #3136.5
        movdqa    %xmm0, -184(%ebp)                             #3136.5
        movl      8(%ebx), %eax                                 #3136.5
        movdqa    80(%eax), %xmm0                               #3136.5
        movdqa    %xmm0, -168(%ebp)                             #3136.5
        movl      8(%ebx), %eax                                 #3136.5
        movdqa    96(%eax), %xmm0                               #3136.5
        movdqa    %xmm0, -152(%ebp)                             #3136.5
        movl      8(%ebx), %eax                                 #3136.5
        movdqa    112(%eax), %xmm0                              #3136.5
        movdqa    %xmm0, -136(%ebp)                             #3136.5
        movl      8(%ebx), %eax                                 #3136.5
        movdqa    128(%eax), %xmm0                              #3136.5
        movdqa    %xmm0, -120(%ebp)                             #3136.5
..LN2829:
        xorl      %eax, %eax                                    #3139.15
        movl      %eax, -32(%ebp)                               #3139.15
..LN2831:
        movl      %eax, -28(%ebp)                               #3139.10
..LN2833:
        movl      %eax, -24(%ebp)                               #3139.5
..LN2835:
        movl      $8, -20(%ebp)                                 #3143.5
..LN2837:
        movl      16(%ebx), %eax                                #3148.14
..LN2839:
        andl      $3, %eax                                      #3148.20
        movl      %eax, -16(%ebp)                               #3148.20
        je        ..B22.14      # Prob 50%                      #3148.20
                                # LOE
..B22.7:                        # Preds ..B22.6
        movl      -16(%ebp), %eax                               #3148.20
        cmpl      $1, %eax                                      #3148.20
        je        ..B22.15      # Prob 50%                      #3148.20
                                # LOE
..B22.8:                        # Preds ..B22.7
        movl      -16(%ebp), %eax                               #3148.20
        cmpl      $2, %eax                                      #3148.20
        je        ..B22.16      # Prob 50%                      #3148.20
                                # LOE
..B22.9:                        # Preds ..B22.8
        movl      -16(%ebp), %eax                               #3148.20
        cmpl      $3, %eax                                      #3148.20
        je        ..B22.17      # Prob 50%                      #3148.20
                                # LOE
..B22.10:                       # Preds ..B22.45 ..B22.44 ..B22.9
..LN2841:
        movl      $1, %eax                                      #3153.9
        testl     %eax, %eax                                    #3153.9
        je        ..B22.46      # Prob 100%                     #3153.9
                                # LOE
..B22.11:                       # Preds ..B22.10
..LN2843:
        movl      $8, -20(%ebp)                                 #3155.13
                                # LOE
..B22.12:                       # Preds ..B22.39 ..B22.11
..LN2845:
        movl      $1, %eax                                      #3158.13
        testl     %eax, %eax                                    #3158.13
        je        ..B22.41      # Prob 100%                     #3158.13
                                # LOE
..B22.14:                       # Preds ..B22.6 ..B22.12
..LN2847:
        movdqa    -248(%ebp), %xmm0                             #3165.38
..LN2849:
        movdqa    -216(%ebp), %xmm1                             #3165.46
..LN2851:
        pand      %xmm1, %xmm0                                  #3165.22
..LN2853:
        movdqa    %xmm0, -264(%ebp)                             #3165.17
..LN2855:
        movdqa    -264(%ebp), %xmm0                             #3166.22
        movdqa    -232(%ebp), %xmm1                             #3166.22
        pcmpeqd   %xmm1, %xmm0                                  #3166.22
..LN2857:
        movdqa    %xmm0, -264(%ebp)                             #3166.17
..LN2859:
        movdqa    -264(%ebp), %xmm0                             #3167.42
..LN2861:
        pmovmskb  %xmm0, %eax                                   #3167.22
..LN2863:
        movl      %eax, -24(%ebp)                               #3167.17
..LN2865:
        movl      -24(%ebp), %eax                               #3168.17
        andl      $3855, %eax                                   #3168.17
        shll      $4, %eax                                      #3168.17
        andl      -24(%ebp), %eax                               #3168.17
        movl      %eax, -24(%ebp)                               #3168.17
        movl      -24(%ebp), %eax                               #3168.17
        sarl      $4, %eax                                      #3168.17
        orl       -24(%ebp), %eax                               #3168.17
        movl      %eax, -24(%ebp)                               #3168.17
                                # LOE
..B22.15:                       # Preds ..B22.7 ..B22.14
..LN2867:
        movdqa    -248(%ebp), %xmm0                             #3171.38
..LN2869:
        movdqa    -184(%ebp), %xmm1                             #3171.46
..LN2871:
        pand      %xmm1, %xmm0                                  #3171.22
..LN2873:
        movdqa    %xmm0, -264(%ebp)                             #3171.17
..LN2875:
        movdqa    -264(%ebp), %xmm0                             #3172.22
        movdqa    -200(%ebp), %xmm1                             #3172.22
        pcmpeqd   %xmm1, %xmm0                                  #3172.22
..LN2877:
        movdqa    %xmm0, -264(%ebp)                             #3172.17
..LN2879:
        movdqa    -264(%ebp), %xmm0                             #3173.42
..LN2881:
        pmovmskb  %xmm0, %eax                                   #3173.22
..LN2883:
        movl      %eax, -28(%ebp)                               #3173.17
..LN2885:
        movl      -28(%ebp), %eax                               #3174.17
        andl      $3855, %eax                                   #3174.17
        shll      $4, %eax                                      #3174.17
        andl      -28(%ebp), %eax                               #3174.17
        movl      %eax, -28(%ebp)                               #3174.17
        movl      -28(%ebp), %eax                               #3174.17
        sarl      $4, %eax                                      #3174.17
        orl       -28(%ebp), %eax                               #3174.17
        movl      %eax, -28(%ebp)                               #3174.17
                                # LOE
..B22.16:                       # Preds ..B22.8 ..B22.15
..LN2887:
        movdqa    -248(%ebp), %xmm0                             #3177.38
..LN2889:
        movdqa    -152(%ebp), %xmm1                             #3177.46
..LN2891:
        pand      %xmm1, %xmm0                                  #3177.22
..LN2893:
        movdqa    %xmm0, -264(%ebp)                             #3177.17
..LN2895:
        movdqa    -264(%ebp), %xmm0                             #3178.22
        movdqa    -168(%ebp), %xmm1                             #3178.22
        pcmpeqd   %xmm1, %xmm0                                  #3178.22
..LN2897:
        movdqa    %xmm0, -264(%ebp)                             #3178.17
..LN2899:
        movdqa    -264(%ebp), %xmm0                             #3179.42
..LN2901:
        pmovmskb  %xmm0, %eax                                   #3179.22
..LN2903:
        movl      %eax, -32(%ebp)                               #3179.17
..LN2905:
        movl      -32(%ebp), %eax                               #3180.17
        andl      $3855, %eax                                   #3180.17
        shll      $4, %eax                                      #3180.17
        andl      -32(%ebp), %eax                               #3180.17
        movl      %eax, -32(%ebp)                               #3180.17
        movl      -32(%ebp), %eax                               #3180.17
        sarl      $4, %eax                                      #3180.17
        orl       -32(%ebp), %eax                               #3180.17
        movl      %eax, -32(%ebp)                               #3180.17
                                # LOE
..B22.17:                       # Preds ..B22.9 ..B22.16
..LN2907:
        movdqa    -248(%ebp), %xmm0                             #3183.38
..LN2909:
        movdqa    -120(%ebp), %xmm1                             #3183.46
..LN2911:
        pand      %xmm1, %xmm0                                  #3183.22
..LN2913:
        movdqa    %xmm0, -264(%ebp)                             #3183.17
..LN2915:
        movdqa    -264(%ebp), %xmm0                             #3184.22
        movdqa    -136(%ebp), %xmm1                             #3184.22
        pcmpeqd   %xmm1, %xmm0                                  #3184.22
..LN2917:
        movdqa    %xmm0, -264(%ebp)                             #3184.17
..LN2919:
        movdqa    -264(%ebp), %xmm0                             #3185.42
..LN2921:
        pmovmskb  %xmm0, %eax                                   #3185.22
..LN2923:
        movl      %eax, -52(%ebp)                               #3185.17
..LN2925:
        movl      -52(%ebp), %eax                               #3186.17
        andl      $3855, %eax                                   #3186.17
        shll      $4, %eax                                      #3186.17
        andl      -52(%ebp), %eax                               #3186.17
        movl      %eax, -52(%ebp)                               #3186.17
        movl      -52(%ebp), %eax                               #3186.17
        sarl      $4, %eax                                      #3186.17
        orl       -52(%ebp), %eax                               #3186.17
        movl      %eax, -52(%ebp)                               #3186.17
..LN2927:
        movl      16(%ebx), %eax                                #3190.17
        andl      $-4, %eax                                     #3190.17
        movl      %eax, 16(%ebx)                                #3190.17
..LN2929:
        movl      -28(%ebp), %eax                               #3193.29
        orl       -24(%ebp), %eax                               #3193.29
..LN2931:
        orl       -32(%ebp), %eax                               #3193.34
..LN2933:
        orl       -52(%ebp), %eax                               #3193.39
..LN2935:
        je        ..B22.34      # Prob 50%                      #3193.47
                                # LOE
..B22.18:                       # Preds ..B22.17
..LN2937:
        pushl     %edi                                          #3211.30
..LN2939:
        movl      -24(%ebp), %eax                               #3211.58
        movw      %ax, (%esp)                                   #3211.58
..LN2941:
        call      uint16_lsbit                                  #3211.30
                                # LOE eax
..B22.51:                       # Preds ..B22.18
        popl      %ecx                                          #3211.30
        movl      %eax, -72(%ebp)                               #3211.30
                                # LOE
..B22.19:                       # Preds ..B22.51
        movl      -72(%ebp), %eax                               #3211.30
        movswl    %ax, %eax                                     #3211.30
..LN2943:
        movl      %eax, -88(%ebp)                               #3211.28
..LN2945:
        pushl     %edi                                          #3212.30
..LN2947:
        movl      -28(%ebp), %eax                               #3212.58
        movw      %ax, (%esp)                                   #3212.58
..LN2949:
        call      uint16_lsbit                                  #3212.30
                                # LOE eax
..B22.52:                       # Preds ..B22.19
        popl      %ecx                                          #3212.30
        movl      %eax, -68(%ebp)                               #3212.30
                                # LOE
..B22.20:                       # Preds ..B22.52
        movl      -68(%ebp), %eax                               #3212.30
        movswl    %ax, %eax                                     #3212.30
..LN2951:
        movl      %eax, -84(%ebp)                               #3212.28
..LN2953:
        pushl     %edi                                          #3213.30
..LN2955:
        movl      -32(%ebp), %eax                               #3213.58
        movw      %ax, (%esp)                                   #3213.58
..LN2957:
        call      uint16_lsbit                                  #3213.30
                                # LOE eax
..B22.53:                       # Preds ..B22.20
        popl      %ecx                                          #3213.30
        movl      %eax, -64(%ebp)                               #3213.30
                                # LOE
..B22.21:                       # Preds ..B22.53
        movl      -64(%ebp), %eax                               #3213.30
        movswl    %ax, %eax                                     #3213.30
..LN2959:
        movl      %eax, -80(%ebp)                               #3213.28
..LN2961:
        pushl     %edi                                          #3214.30
..LN2963:
        movl      -52(%ebp), %eax                               #3214.58
        movw      %ax, (%esp)                                   #3214.58
..LN2965:
        call      uint16_lsbit                                  #3214.30
                                # LOE eax
..B22.54:                       # Preds ..B22.21
        popl      %ecx                                          #3214.30
        movl      %eax, -60(%ebp)                               #3214.30
                                # LOE
..B22.22:                       # Preds ..B22.54
        movl      -60(%ebp), %eax                               #3214.30
        movswl    %ax, %eax                                     #3214.30
..LN2967:
        movl      %eax, -76(%ebp)                               #3214.28
..LN2969:
        shll      $2, -88(%ebp)                                 #3219.34
..LN2971:
        movl      -84(%ebp), %eax                               #3220.28
..LN2973:
        lea       1(,%eax,4), %eax                              #3220.40
..LN2975:
        movl      %eax, -84(%ebp)                               #3220.21
..LN2977:
        movl      -80(%ebp), %eax                               #3221.28
..LN2979:
        lea       2(,%eax,4), %eax                              #3221.40
..LN2981:
        movl      %eax, -80(%ebp)                               #3221.21
..LN2983:
        movl      -76(%ebp), %eax                               #3222.28
..LN2985:
        lea       3(,%eax,4), %eax                              #3222.40
..LN2987:
        movl      %eax, -76(%ebp)                               #3222.21
..LN2989:
        movl      -24(%ebp), %eax                               #3226.26
..LN2991:
        testl     %eax, %eax                                    #3226.32
        je        ..B22.25      # Prob 50%                      #3226.32
                                # LOE
..B22.23:                       # Preds ..B22.22
..LN2993:
        movl      -88(%ebp), %eax                               #3226.43
        addl      16(%ebx), %eax                                #3226.43
..LN2995:
        movl      -44(%ebp), %edx                               #3226.49
        cmpl      %edx, %eax                                    #3226.49
        ja        ..B22.25      # Prob 50%                      #3226.49
                                # LOE
..B22.24:                       # Preds ..B22.23
..LN2997:
        movl      $1, %eax                                      #3226.63
        leave                                                   #3226.63
..___tag_value_eval_2na_64.544:                                 #
        movl      %ebx, %esp                                    #3226.63
        popl      %ebx                                          #3226.63
..___tag_value_eval_2na_64.545:                                 #
        ret                                                     #3226.63
..___tag_value_eval_2na_64.547:                                 #
                                # LOE
..B22.25:                       # Preds ..B22.23 ..B22.22
..LN2999:
        movl      -28(%ebp), %eax                               #3227.26
..LN3001:
        testl     %eax, %eax                                    #3227.32
        je        ..B22.28      # Prob 50%                      #3227.32
                                # LOE
..B22.26:                       # Preds ..B22.25
..LN3003:
        movl      -84(%ebp), %eax                               #3227.43
        addl      16(%ebx), %eax                                #3227.43
..LN3005:
        movl      -44(%ebp), %edx                               #3227.49
        cmpl      %edx, %eax                                    #3227.49
        ja        ..B22.28      # Prob 50%                      #3227.49
                                # LOE
..B22.27:                       # Preds ..B22.26
..LN3007:
        movl      $1, %eax                                      #3227.63
        leave                                                   #3227.63
..___tag_value_eval_2na_64.551:                                 #
        movl      %ebx, %esp                                    #3227.63
        popl      %ebx                                          #3227.63
..___tag_value_eval_2na_64.552:                                 #
        ret                                                     #3227.63
..___tag_value_eval_2na_64.554:                                 #
                                # LOE
..B22.28:                       # Preds ..B22.26 ..B22.25
..LN3009:
        movl      -32(%ebp), %eax                               #3228.26
..LN3011:
        testl     %eax, %eax                                    #3228.32
        je        ..B22.31      # Prob 50%                      #3228.32
                                # LOE
..B22.29:                       # Preds ..B22.28
..LN3013:
        movl      -80(%ebp), %eax                               #3228.43
        addl      16(%ebx), %eax                                #3228.43
..LN3015:
        movl      -44(%ebp), %edx                               #3228.49
        cmpl      %edx, %eax                                    #3228.49
        ja        ..B22.31      # Prob 50%                      #3228.49
                                # LOE
..B22.30:                       # Preds ..B22.29
..LN3017:
        movl      $1, %eax                                      #3228.63
        leave                                                   #3228.63
..___tag_value_eval_2na_64.558:                                 #
        movl      %ebx, %esp                                    #3228.63
        popl      %ebx                                          #3228.63
..___tag_value_eval_2na_64.559:                                 #
        ret                                                     #3228.63
..___tag_value_eval_2na_64.561:                                 #
                                # LOE
..B22.31:                       # Preds ..B22.29 ..B22.28
..LN3019:
        movl      -52(%ebp), %eax                               #3229.26
..LN3021:
        testl     %eax, %eax                                    #3229.32
        je        ..B22.34      # Prob 50%                      #3229.32
                                # LOE
..B22.32:                       # Preds ..B22.31
..LN3023:
        movl      -76(%ebp), %eax                               #3229.43
        addl      16(%ebx), %eax                                #3229.43
..LN3025:
        movl      -44(%ebp), %edx                               #3229.49
        cmpl      %edx, %eax                                    #3229.49
        ja        ..B22.34      # Prob 50%                      #3229.49
                                # LOE
..B22.33:                       # Preds ..B22.32
..LN3027:
        movl      $1, %eax                                      #3229.63
        leave                                                   #3229.63
..___tag_value_eval_2na_64.565:                                 #
        movl      %ebx, %esp                                    #3229.63
        popl      %ebx                                          #3229.63
..___tag_value_eval_2na_64.566:                                 #
        ret                                                     #3229.63
..___tag_value_eval_2na_64.568:                                 #
                                # LOE
..B22.34:                       # Preds ..B22.32 ..B22.31 ..B22.17
..LN3029:
        movl      16(%ebx), %eax                                #3234.17
        addl      $4, %eax                                      #3234.17
        movl      %eax, 16(%ebx)                                #3234.17
..LN3031:
        movl      16(%ebx), %eax                                #3237.22
..LN3033:
        movl      -44(%ebp), %edx                               #3237.28
        cmpl      %edx, %eax                                    #3237.28
        jbe       ..B22.36      # Prob 50%                      #3237.28
                                # LOE
..B22.35:                       # Preds ..B22.34
..LN3035:
        xorl      %eax, %eax                                    #3238.28
        leave                                                   #3238.28
..___tag_value_eval_2na_64.572:                                 #
        movl      %ebx, %esp                                    #3238.28
        popl      %ebx                                          #3238.28
..___tag_value_eval_2na_64.573:                                 #
        ret                                                     #3238.28
..___tag_value_eval_2na_64.575:                                 #
                                # LOE
..B22.36:                       # Preds ..B22.34
..LN3037:
        movl      -20(%ebp), %eax                               #3241.25
        decl      %eax                                          #3241.25
        movl      %eax, -20(%ebp)                               #3241.25
..LN3039:
        je        ..B22.41      # Prob 50%                      #3241.39
                                # LOE
..B22.37:                       # Preds ..B22.36
..LN3041:
        movdqa    -248(%ebp), %xmm0                             #3246.43
..LN3043:
        psrldq    $1, %xmm0                                     #3246.26
..LN3045:
        movdqa    %xmm0, -248(%ebp)                             #3246.17
..LN3047:
        movl      -36(%ebp), %eax                               #3249.22
..LN3049:
        movl      -40(%ebp), %edx                               #3249.26
        cmpl      %edx, %eax                                    #3249.26
        jae       ..B22.39      # Prob 50%                      #3249.26
                                # LOE
..B22.38:                       # Preds ..B22.37
..LN3051:
        sarl      $8, -56(%ebp)                                 #3251.21
..LN3053:
        movl      -36(%ebp), %eax                               #3252.37
        movzbl    (%eax), %eax                                  #3252.37
        movzbl    %al, %eax                                     #3252.37
..LN3055:
        shll      $8, %eax                                      #3252.48
..LN3057:
        orl       -56(%ebp), %eax                               #3252.21
        movl      %eax, -56(%ebp)                               #3252.21
..LN3059:
        movdqa    -248(%ebp), %xmm0                             #3253.49
..LN3061:
        movl      -56(%ebp), %eax                               #3253.57
..LN3063:
        pinsrw    $7, %eax, %xmm0                               #3253.30
..LN3065:
        movdqa    %xmm0, -248(%ebp)                             #3253.21
                                # LOE
..B22.39:                       # Preds ..B22.38 ..B22.37
..LN3067:
        incl      -36(%ebp)                                     #3257.20
        jmp       ..B22.12      # Prob 100%                     #3257.20
                                # LOE
..B22.41:                       # Preds ..B22.36 ..B22.12
..LN3069:
        movl      -48(%ebp), %eax                               #3262.18
..LN3071:
        movl      -40(%ebp), %edx                               #3262.25
        cmpl      %edx, %eax                                    #3262.25
        jae       ..B22.46      # Prob 50%                      #3262.25
                                # LOE
..B22.42:                       # Preds ..B22.41
..LN3073:
        movl      16(%ebx), %eax                                #3266.13
        addl      $32, %eax                                     #3266.13
        movl      %eax, 16(%ebx)                                #3266.13
..LN3075:
        movl      16(%ebx), %eax                                #3267.18
..LN3077:
        movl      -44(%ebp), %edx                               #3267.24
        cmpl      %edx, %eax                                    #3267.24
        ja        ..B22.46      # Prob 50%                      #3267.24
                                # LOE
..B22.43:                       # Preds ..B22.42
..LN3079:
        addl      $-8, %esp                                     #3272.22
..LN3081:
        movl      -48(%ebp), %eax                               #3272.41
        movl      %eax, (%esp)                                  #3272.41
..LN3083:
        movl      -40(%ebp), %eax                               #3272.46
        movl      %eax, 4(%esp)                                 #3272.46
..LN3085:
        call      prime_buffer_2na                              #3272.22
                                # LOE xmm0
..B22.55:                       # Preds ..B22.43
        addl      $8, %esp                                      #3272.22
        movdqa    %xmm0, -280(%ebp)                             #3272.22
                                # LOE
..B22.44:                       # Preds ..B22.55
..LN3087:
        movdqa    -280(%ebp), %xmm0                             #3272.13
        movdqa    %xmm0, -248(%ebp)                             #3272.13
..LN3089:
        addl      $16, -48(%ebp)                                #3350.13
..LN3091:
        movl      -48(%ebp), %eax                               #3352.17
..LN3093:
        movl      %eax, -36(%ebp)                               #3352.13
..LN3095:
        movl      -48(%ebp), %eax                               #3353.18
..LN3097:
        movl      -40(%ebp), %edx                               #3353.24
        cmpl      %edx, %eax                                    #3353.24
        jae       ..B22.10      # Prob 50%                      #3353.24
                                # LOE
..B22.45:                       # Preds ..B22.44
..LN3099:
        movl      -48(%ebp), %eax                               #3354.32
        movzbl    -1(%eax), %eax                                #3354.32
        movzbl    %al, %eax                                     #3354.32
..LN3101:
        shll      $8, %eax                                      #3354.46
..LN3103:
        movl      %eax, -56(%ebp)                               #3354.17
        jmp       ..B22.10      # Prob 100%                     #3354.17
                                # LOE
..B22.46:                       # Preds ..B22.42 ..B22.41 ..B22.10
..LN3105:
        xorl      %eax, %eax                                    #3363.12
        leave                                                   #3363.12
..___tag_value_eval_2na_64.579:                                 #
        movl      %ebx, %esp                                    #3363.12
        popl      %ebx                                          #3363.12
..___tag_value_eval_2na_64.580:                                 #
        ret                                                     #3363.12
        .align    2,0x90
..___tag_value_eval_2na_64.582:                                 #
                                # LOE
# mark_end;
	.type	eval_2na_64,@function
	.size	eval_2na_64,.-eval_2na_64
.LNeval_2na_64:
	.data
# -- End  eval_2na_64
	.section .rodata, "a"
	.align 4
__$U9:
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
	.type	__$U9,@object
	.size	__$U9,13
	.text
# -- Begin  eval_2na_128
# mark_begin;
       .align    2,0x90
eval_2na_128:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B23.1:                        # Preds ..B23.0
..___tag_value_eval_2na_128.585:                                #
..LN3107:
        pushl     %ebx                                          #3369.1
..___tag_value_eval_2na_128.590:                                #
        movl      %esp, %ebx                                    #3369.1
..___tag_value_eval_2na_128.591:                                #
        andl      $-16, %esp                                    #3369.1
        pushl     %ebp                                          #3369.1
        pushl     %ebp                                          #3369.1
        movl      4(%ebx), %ebp                                 #3369.1
        movl      %ebp, 4(%esp)                                 #3369.1
        movl      %esp, %ebp                                    #3369.1
..___tag_value_eval_2na_128.593:                                #
        subl      $264, %esp                                    #3369.1
        movl      %ebx, -264(%ebp)                              #3369.1
..LN3109:
        movl      8(%ebx), %eax                                 #3399.25
        movl      4(%eax), %eax                                 #3399.25
..LN3111:
        movl      %eax, -8(%ebp)                                #3399.23
..LN3113:
        movl      20(%ebx), %eax                                #3402.5
        movl      -8(%ebp), %edx                                #3402.5
        cmpl      %edx, %eax                                    #3402.5
        jae       ..B23.3       # Prob 50%                      #3402.5
                                # LOE
..B23.2:                        # Preds ..B23.1
        call      ..L595        # Prob 100%                     #
..L595:                                                         #
        popl      %eax                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L595], %eax     #
        addl      $-16, %esp                                    #3402.5
        lea       _2__STRING.9@GOTOFF(%eax), %edx               #3402.5
        movl      %edx, (%esp)                                  #3402.5
        lea       _2__STRING.1@GOTOFF(%eax), %edx               #3402.5
        movl      %edx, 4(%esp)                                 #3402.5
        movl      $3402, 8(%esp)                                #3402.5
        lea       __$U9@GOTOFF(%eax), %edx                      #3402.5
        movl      %edx, 12(%esp)                                #3402.5
        movl      %eax, %ebx                                    #3402.5
..___tag_value_eval_2na_128.596:                                #3402.5
        call      __assert_fail@PLT                             #3402.5
        movl      -264(%ebp), %ebx                              #3402.5
..___tag_value_eval_2na_128.597:                                #
                                # LOE
..B23.44:                       # Preds ..B23.2
        addl      $16, %esp                                     #3402.5
                                # LOE
..B23.3:                        # Preds ..B23.1
..LN3115:
        movl      16(%ebx), %eax                                #3406.12
..LN3117:
        addl      20(%ebx), %eax                                #3406.5
        movl      %eax, 20(%ebx)                                #3406.5
..LN3119:
        movl      16(%ebx), %eax                                #3409.42
..LN3121:
        shrl      $2, %eax                                      #3409.49
..LN3123:
        addl      12(%ebx), %eax                                #3409.30
..LN3125:
        movl      %eax, -44(%ebp)                               #3409.5
..LN3127:
        movl      -8(%ebp), %eax                                #3412.18
        negl      %eax                                          #3412.18
        addl      20(%ebx), %eax                                #3412.18
..LN3129:
        movl      %eax, -40(%ebp)                               #3412.5
..LN3131:
        movl      20(%ebx), %eax                                #3415.44
..LN3133:
        addl      $3, %eax                                      #3415.50
..LN3135:
        shrl      $2, %eax                                      #3415.57
..LN3137:
        addl      12(%ebx), %eax                                #3415.30
..LN3139:
        movl      %eax, -36(%ebp)                               #3415.5
..LN3141:
        addl      $-8, %esp                                     #3418.14
..LN3143:
        movl      -44(%ebp), %eax                               #3418.33
        movl      %eax, (%esp)                                  #3418.33
..LN3145:
        movl      -36(%ebp), %eax                               #3418.38
        movl      %eax, 4(%esp)                                 #3418.38
..LN3147:
        call      prime_buffer_2na                              #3418.14
                                # LOE xmm0
..B23.45:                       # Preds ..B23.3
        addl      $8, %esp                                      #3418.14
        movdqa    %xmm0, -72(%ebp)                              #3418.14
                                # LOE
..B23.4:                        # Preds ..B23.45
..LN3149:
        movdqa    -72(%ebp), %xmm0                              #3418.5
        movdqa    %xmm0, -216(%ebp)                             #3418.5
..LN3151:
        addl      $16, -44(%ebp)                                #3419.5
..LN3153:
        movl      -44(%ebp), %eax                               #3421.9
..LN3155:
        movl      %eax, -32(%ebp)                               #3421.5
..LN3157:
        movl      -44(%ebp), %eax                               #3424.10
..LN3159:
        movl      -36(%ebp), %edx                               #3424.16
        cmpl      %edx, %eax                                    #3424.16
        jae       ..B23.6       # Prob 50%                      #3424.16
                                # LOE
..B23.5:                        # Preds ..B23.4
..LN3161:
        movl      -44(%ebp), %eax                               #3425.24
        movzbl    -1(%eax), %eax                                #3425.24
        movzbl    %al, %eax                                     #3425.24
..LN3163:
        shll      $8, %eax                                      #3425.38
..LN3165:
        movl      %eax, -52(%ebp)                               #3425.9
                                # LOE
..B23.6:                        # Preds ..B23.5 ..B23.4
..LN3167:
        movl      8(%ebx), %eax                                 #3429.5
        movdqa    16(%eax), %xmm0                               #3429.5
        movdqa    %xmm0, -200(%ebp)                             #3429.5
        movl      8(%ebx), %eax                                 #3429.5
        movdqa    32(%eax), %xmm0                               #3429.5
        movdqa    %xmm0, -184(%ebp)                             #3429.5
        movl      8(%ebx), %eax                                 #3429.5
        movdqa    48(%eax), %xmm0                               #3429.5
        movdqa    %xmm0, -168(%ebp)                             #3429.5
        movl      8(%ebx), %eax                                 #3429.5
        movdqa    64(%eax), %xmm0                               #3429.5
        movdqa    %xmm0, -152(%ebp)                             #3429.5
        movl      8(%ebx), %eax                                 #3429.5
        movdqa    80(%eax), %xmm0                               #3429.5
        movdqa    %xmm0, -136(%ebp)                             #3429.5
        movl      8(%ebx), %eax                                 #3429.5
        movdqa    96(%eax), %xmm0                               #3429.5
        movdqa    %xmm0, -120(%ebp)                             #3429.5
        movl      8(%ebx), %eax                                 #3429.5
        movdqa    112(%eax), %xmm0                              #3429.5
        movdqa    %xmm0, -104(%ebp)                             #3429.5
        movl      8(%ebx), %eax                                 #3429.5
        movdqa    128(%eax), %xmm0                              #3429.5
        movdqa    %xmm0, -88(%ebp)                              #3429.5
..LN3169:
        xorl      %eax, %eax                                    #3432.15
        movl      %eax, -28(%ebp)                               #3432.15
..LN3171:
        movl      %eax, -24(%ebp)                               #3432.10
..LN3173:
        movl      %eax, -20(%ebp)                               #3432.5
..LN3175:
        movl      16(%ebx), %eax                                #3434.27
        negl      %eax                                          #3434.27
..LN3177:
        movl      -40(%ebp), %edx                               #3434.20
..LN3179:
        lea       7(%edx,%eax), %eax                            #3434.33
..LN3181:
        shrl      $2, %eax                                      #3434.40
..LN3183:
        movl      %eax, -16(%ebp)                               #3434.5
..LN3185:
        movl      16(%ebx), %eax                                #3441.14
..LN3187:
        andl      $3, %eax                                      #3441.20
        movl      %eax, -12(%ebp)                               #3441.20
        je        ..B23.14      # Prob 50%                      #3441.20
                                # LOE
..B23.7:                        # Preds ..B23.6
        movl      -12(%ebp), %eax                               #3441.20
        cmpl      $1, %eax                                      #3441.20
        je        ..B23.15      # Prob 50%                      #3441.20
                                # LOE
..B23.8:                        # Preds ..B23.7
        movl      -12(%ebp), %eax                               #3441.20
        cmpl      $2, %eax                                      #3441.20
        je        ..B23.16      # Prob 50%                      #3441.20
                                # LOE
..B23.9:                        # Preds ..B23.8
        movl      -12(%ebp), %eax                               #3441.20
        cmpl      $3, %eax                                      #3441.20
        je        ..B23.17      # Prob 50%                      #3441.20
                                # LOE
..B23.10:                       # Preds ..B23.40 ..B23.39 ..B23.9
..LN3189:
        movl      $1, %eax                                      #3446.9
        testl     %eax, %eax                                    #3446.9
        je        ..B23.41      # Prob 100%                     #3446.9
                                # LOE
..B23.11:                       # Preds ..B23.10
..LN3191:
        movl      $16, -16(%ebp)                                #3448.13
                                # LOE
..B23.12:                       # Preds ..B23.34 ..B23.11
..LN3193:
        movl      $1, %eax                                      #3451.13
        testl     %eax, %eax                                    #3451.13
        je        ..B23.36      # Prob 100%                     #3451.13
                                # LOE
..B23.14:                       # Preds ..B23.6 ..B23.12
..LN3195:
        movdqa    -216(%ebp), %xmm0                             #3458.38
..LN3197:
        movdqa    -184(%ebp), %xmm1                             #3458.46
..LN3199:
        pand      %xmm1, %xmm0                                  #3458.22
..LN3201:
        movdqa    %xmm0, -232(%ebp)                             #3458.17
..LN3203:
        movdqa    -232(%ebp), %xmm0                             #3459.22
        movdqa    -200(%ebp), %xmm1                             #3459.22
        pcmpeqd   %xmm1, %xmm0                                  #3459.22
..LN3205:
        movdqa    %xmm0, -232(%ebp)                             #3459.17
..LN3207:
        movdqa    -232(%ebp), %xmm0                             #3460.42
..LN3209:
        pmovmskb  %xmm0, %eax                                   #3460.22
..LN3211:
        movl      %eax, -20(%ebp)                               #3460.17
..LN3213:
        movl      -20(%ebp), %eax                               #3461.17
        incl      %eax                                          #3461.17
        sarl      $16, %eax                                     #3461.17
        negl      %eax                                          #3461.17
        movl      %eax, -20(%ebp)                               #3461.17
                                # LOE
..B23.15:                       # Preds ..B23.7 ..B23.14
..LN3215:
        movdqa    -216(%ebp), %xmm0                             #3464.38
..LN3217:
        movdqa    -152(%ebp), %xmm1                             #3464.46
..LN3219:
        pand      %xmm1, %xmm0                                  #3464.22
..LN3221:
        movdqa    %xmm0, -232(%ebp)                             #3464.17
..LN3223:
        movdqa    -232(%ebp), %xmm0                             #3465.22
        movdqa    -168(%ebp), %xmm1                             #3465.22
        pcmpeqd   %xmm1, %xmm0                                  #3465.22
..LN3225:
        movdqa    %xmm0, -232(%ebp)                             #3465.17
..LN3227:
        movdqa    -232(%ebp), %xmm0                             #3466.42
..LN3229:
        pmovmskb  %xmm0, %eax                                   #3466.22
..LN3231:
        movl      %eax, -24(%ebp)                               #3466.17
..LN3233:
        movl      -24(%ebp), %eax                               #3467.17
        incl      %eax                                          #3467.17
        sarl      $16, %eax                                     #3467.17
        negl      %eax                                          #3467.17
        movl      %eax, -24(%ebp)                               #3467.17
                                # LOE
..B23.16:                       # Preds ..B23.8 ..B23.15
..LN3235:
        movdqa    -216(%ebp), %xmm0                             #3470.38
..LN3237:
        movdqa    -120(%ebp), %xmm1                             #3470.46
..LN3239:
        pand      %xmm1, %xmm0                                  #3470.22
..LN3241:
        movdqa    %xmm0, -232(%ebp)                             #3470.17
..LN3243:
        movdqa    -232(%ebp), %xmm0                             #3471.22
        movdqa    -136(%ebp), %xmm1                             #3471.22
        pcmpeqd   %xmm1, %xmm0                                  #3471.22
..LN3245:
        movdqa    %xmm0, -232(%ebp)                             #3471.17
..LN3247:
        movdqa    -232(%ebp), %xmm0                             #3472.42
..LN3249:
        pmovmskb  %xmm0, %eax                                   #3472.22
..LN3251:
        movl      %eax, -28(%ebp)                               #3472.17
..LN3253:
        movl      -28(%ebp), %eax                               #3473.17
        incl      %eax                                          #3473.17
        sarl      $16, %eax                                     #3473.17
        negl      %eax                                          #3473.17
        movl      %eax, -28(%ebp)                               #3473.17
                                # LOE
..B23.17:                       # Preds ..B23.9 ..B23.16
..LN3255:
        movdqa    -216(%ebp), %xmm0                             #3476.38
..LN3257:
        movdqa    -88(%ebp), %xmm1                              #3476.46
..LN3259:
        pand      %xmm1, %xmm0                                  #3476.22
..LN3261:
        movdqa    %xmm0, -232(%ebp)                             #3476.17
..LN3263:
        movdqa    -232(%ebp), %xmm0                             #3477.22
        movdqa    -104(%ebp), %xmm1                             #3477.22
        pcmpeqd   %xmm1, %xmm0                                  #3477.22
..LN3265:
        movdqa    %xmm0, -232(%ebp)                             #3477.17
..LN3267:
        movdqa    -232(%ebp), %xmm0                             #3478.42
..LN3269:
        pmovmskb  %xmm0, %eax                                   #3478.22
..LN3271:
        movl      %eax, -48(%ebp)                               #3478.17
..LN3273:
        movl      -48(%ebp), %eax                               #3479.17
        incl      %eax                                          #3479.17
        sarl      $16, %eax                                     #3479.17
        negl      %eax                                          #3479.17
        movl      %eax, -48(%ebp)                               #3479.17
..LN3275:
        movl      16(%ebx), %eax                                #3483.17
        andl      $-4, %eax                                     #3483.17
        movl      %eax, 16(%ebx)                                #3483.17
..LN3277:
        movl      -24(%ebp), %eax                               #3486.29
        orl       -20(%ebp), %eax                               #3486.29
..LN3279:
        orl       -28(%ebp), %eax                               #3486.34
..LN3281:
        orl       -48(%ebp), %eax                               #3486.39
..LN3283:
        je        ..B23.29      # Prob 50%                      #3486.47
                                # LOE
..B23.18:                       # Preds ..B23.17
..LN3285:
        movl      16(%ebx), %eax                                #3489.37
        negl      %eax                                          #3489.37
        addl      -40(%ebp), %eax                               #3489.37
        movl      %eax, -56(%ebp)                               #3489.37
        cmpl      $2, %eax                                      #3489.37
        je        ..B23.22      # Prob 50%                      #3489.37
                                # LOE
..B23.19:                       # Preds ..B23.18
        movl      -56(%ebp), %eax                               #3489.37
        cmpl      $1, %eax                                      #3489.37
        je        ..B23.24      # Prob 50%                      #3489.37
                                # LOE
..B23.20:                       # Preds ..B23.19
        movl      -56(%ebp), %eax                               #3489.37
        testl     %eax, %eax                                    #3489.37
        je        ..B23.26      # Prob 50%                      #3489.37
                                # LOE
..B23.21:                       # Preds ..B23.20
..LN3287:
        movl      $1, %eax                                      #3492.32
        leave                                                   #3492.32
..___tag_value_eval_2na_128.599:                                #
        movl      %ebx, %esp                                    #3492.32
        popl      %ebx                                          #3492.32
..___tag_value_eval_2na_128.600:                                #
        ret                                                     #3492.32
..___tag_value_eval_2na_128.602:                                #
                                # LOE
..B23.22:                       # Preds ..B23.18
..LN3289:
        movl      -28(%ebp), %eax                               #3494.30
..LN3291:
        testl     %eax, %eax                                    #3494.36
        je        ..B23.24      # Prob 50%                      #3494.36
                                # LOE
..B23.23:                       # Preds ..B23.22
..LN3293:
        movl      $1, %eax                                      #3494.47
        leave                                                   #3494.47
..___tag_value_eval_2na_128.606:                                #
        movl      %ebx, %esp                                    #3494.47
        popl      %ebx                                          #3494.47
..___tag_value_eval_2na_128.607:                                #
        ret                                                     #3494.47
..___tag_value_eval_2na_128.609:                                #
                                # LOE
..B23.24:                       # Preds ..B23.22 ..B23.19
..LN3295:
        movl      -24(%ebp), %eax                               #3496.30
..LN3297:
        testl     %eax, %eax                                    #3496.36
        je        ..B23.26      # Prob 50%                      #3496.36
                                # LOE
..B23.25:                       # Preds ..B23.24
..LN3299:
        movl      $1, %eax                                      #3496.47
        leave                                                   #3496.47
..___tag_value_eval_2na_128.613:                                #
        movl      %ebx, %esp                                    #3496.47
        popl      %ebx                                          #3496.47
..___tag_value_eval_2na_128.614:                                #
        ret                                                     #3496.47
..___tag_value_eval_2na_128.616:                                #
                                # LOE
..B23.26:                       # Preds ..B23.24 ..B23.20
..LN3301:
        movl      -20(%ebp), %eax                               #3498.30
..LN3303:
        testl     %eax, %eax                                    #3498.36
        je        ..B23.28      # Prob 50%                      #3498.36
                                # LOE
..B23.27:                       # Preds ..B23.26
..LN3305:
        movl      $1, %eax                                      #3498.47
        leave                                                   #3498.47
..___tag_value_eval_2na_128.620:                                #
        movl      %ebx, %esp                                    #3498.47
        popl      %ebx                                          #3498.47
..___tag_value_eval_2na_128.621:                                #
        ret                                                     #3498.47
..___tag_value_eval_2na_128.623:                                #
                                # LOE
..B23.28:                       # Preds ..B23.26
..LN3307:
        xorl      %eax, %eax                                    #3500.28
        leave                                                   #3500.28
..___tag_value_eval_2na_128.627:                                #
        movl      %ebx, %esp                                    #3500.28
        popl      %ebx                                          #3500.28
..___tag_value_eval_2na_128.628:                                #
        ret                                                     #3500.28
..___tag_value_eval_2na_128.630:                                #
                                # LOE
..B23.29:                       # Preds ..B23.17
..LN3309:
        movl      16(%ebx), %eax                                #3527.17
        addl      $4, %eax                                      #3527.17
        movl      %eax, 16(%ebx)                                #3527.17
..LN3311:
        movl      16(%ebx), %eax                                #3530.22
..LN3313:
        movl      -40(%ebp), %edx                               #3530.28
        cmpl      %edx, %eax                                    #3530.28
        jbe       ..B23.31      # Prob 50%                      #3530.28
                                # LOE
..B23.30:                       # Preds ..B23.29
..LN3315:
        xorl      %eax, %eax                                    #3531.28
        leave                                                   #3531.28
..___tag_value_eval_2na_128.634:                                #
        movl      %ebx, %esp                                    #3531.28
        popl      %ebx                                          #3531.28
..___tag_value_eval_2na_128.635:                                #
        ret                                                     #3531.28
..___tag_value_eval_2na_128.637:                                #
                                # LOE
..B23.31:                       # Preds ..B23.29
..LN3317:
        movl      -16(%ebp), %eax                               #3534.25
        decl      %eax                                          #3534.25
        movl      %eax, -16(%ebp)                               #3534.25
..LN3319:
        je        ..B23.36      # Prob 50%                      #3534.39
                                # LOE
..B23.32:                       # Preds ..B23.31
..LN3321:
        movdqa    -216(%ebp), %xmm0                             #3539.43
..LN3323:
        psrldq    $1, %xmm0                                     #3539.26
..LN3325:
        movdqa    %xmm0, -216(%ebp)                             #3539.17
..LN3327:
        movl      -32(%ebp), %eax                               #3542.22
..LN3329:
        movl      -36(%ebp), %edx                               #3542.26
        cmpl      %edx, %eax                                    #3542.26
        jae       ..B23.34      # Prob 50%                      #3542.26
                                # LOE
..B23.33:                       # Preds ..B23.32
..LN3331:
        sarl      $8, -52(%ebp)                                 #3544.21
..LN3333:
        movl      -32(%ebp), %eax                               #3545.37
        movzbl    (%eax), %eax                                  #3545.37
        movzbl    %al, %eax                                     #3545.37
..LN3335:
        shll      $8, %eax                                      #3545.48
..LN3337:
        orl       -52(%ebp), %eax                               #3545.21
        movl      %eax, -52(%ebp)                               #3545.21
..LN3339:
        movdqa    -216(%ebp), %xmm0                             #3546.49
..LN3341:
        movl      -52(%ebp), %eax                               #3546.57
..LN3343:
        pinsrw    $7, %eax, %xmm0                               #3546.30
..LN3345:
        movdqa    %xmm0, -216(%ebp)                             #3546.21
                                # LOE
..B23.34:                       # Preds ..B23.33 ..B23.32
..LN3347:
        incl      -32(%ebp)                                     #3550.20
        jmp       ..B23.12      # Prob 100%                     #3550.20
                                # LOE
..B23.36:                       # Preds ..B23.31 ..B23.12
..LN3349:
        movl      -44(%ebp), %eax                               #3555.18
..LN3351:
        movl      -36(%ebp), %edx                               #3555.25
        cmpl      %edx, %eax                                    #3555.25
        jae       ..B23.41      # Prob 50%                      #3555.25
                                # LOE
..B23.37:                       # Preds ..B23.36
..LN3353:
        movl      16(%ebx), %eax                                #3560.18
..LN3355:
        movl      -40(%ebp), %edx                               #3560.24
        cmpl      %edx, %eax                                    #3560.24
        ja        ..B23.41      # Prob 50%                      #3560.24
                                # LOE
..B23.38:                       # Preds ..B23.37
..LN3357:
        addl      $-8, %esp                                     #3565.22
..LN3359:
        movl      -44(%ebp), %eax                               #3565.41
        movl      %eax, (%esp)                                  #3565.41
..LN3361:
        movl      -36(%ebp), %eax                               #3565.46
        movl      %eax, 4(%esp)                                 #3565.46
..LN3363:
        call      prime_buffer_2na                              #3565.22
                                # LOE xmm0
..B23.46:                       # Preds ..B23.38
        addl      $8, %esp                                      #3565.22
        movdqa    %xmm0, -248(%ebp)                             #3565.22
                                # LOE
..B23.39:                       # Preds ..B23.46
..LN3365:
        movdqa    -248(%ebp), %xmm0                             #3565.13
        movdqa    %xmm0, -216(%ebp)                             #3565.13
..LN3367:
        addl      $16, -44(%ebp)                                #3643.13
..LN3369:
        movl      -44(%ebp), %eax                               #3645.17
..LN3371:
        movl      %eax, -32(%ebp)                               #3645.13
..LN3373:
        movl      -44(%ebp), %eax                               #3646.18
..LN3375:
        movl      -36(%ebp), %edx                               #3646.24
        cmpl      %edx, %eax                                    #3646.24
        jae       ..B23.10      # Prob 50%                      #3646.24
                                # LOE
..B23.40:                       # Preds ..B23.39
..LN3377:
        movl      -44(%ebp), %eax                               #3647.32
        movzbl    -1(%eax), %eax                                #3647.32
        movzbl    %al, %eax                                     #3647.32
..LN3379:
        shll      $8, %eax                                      #3647.46
..LN3381:
        movl      %eax, -52(%ebp)                               #3647.17
        jmp       ..B23.10      # Prob 100%                     #3647.17
                                # LOE
..B23.41:                       # Preds ..B23.37 ..B23.36 ..B23.10
..LN3383:
        xorl      %eax, %eax                                    #3656.12
        leave                                                   #3656.12
..___tag_value_eval_2na_128.641:                                #
        movl      %ebx, %esp                                    #3656.12
        popl      %ebx                                          #3656.12
..___tag_value_eval_2na_128.642:                                #
        ret                                                     #3656.12
        .align    2,0x90
..___tag_value_eval_2na_128.644:                                #
                                # LOE
# mark_end;
	.type	eval_2na_128,@function
	.size	eval_2na_128,.-eval_2na_128
.LNeval_2na_128:
	.data
# -- End  eval_2na_128
	.section .rodata, "a"
	.align 4
__$Ua:
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
	.type	__$Ua,@object
	.size	__$Ua,13
	.text
# -- Begin  eval_2na_pos
# mark_begin;
       .align    2,0x90
eval_2na_pos:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B24.1:                        # Preds ..B24.0
..___tag_value_eval_2na_pos.647:                                #
..LN3385:
        pushl     %ebx                                          #3662.1
..___tag_value_eval_2na_pos.652:                                #
        movl      %esp, %ebx                                    #3662.1
..___tag_value_eval_2na_pos.653:                                #
        andl      $-16, %esp                                    #3662.1
        pushl     %ebp                                          #3662.1
        pushl     %ebp                                          #3662.1
        movl      4(%ebx), %ebp                                 #3662.1
        movl      %ebp, 4(%esp)                                 #3662.1
        movl      %esp, %ebp                                    #3662.1
..___tag_value_eval_2na_pos.655:                                #
        subl      $264, %esp                                    #3662.1
        movl      %ebx, -264(%ebp)                              #3662.1
..LN3387:
        movl      8(%ebx), %eax                                 #3698.25
        movl      4(%eax), %eax                                 #3698.25
..LN3389:
        movl      %eax, -4(%ebp)                                #3698.23
..LN3391:
        movl      20(%ebx), %eax                                #3701.5
        movl      -4(%ebp), %edx                                #3701.5
        cmpl      %edx, %eax                                    #3701.5
        jae       ..B24.3       # Prob 50%                      #3701.5
                                # LOE
..B24.2:                        # Preds ..B24.1
        call      ..L657        # Prob 100%                     #
..L657:                                                         #
        popl      %eax                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L657], %eax     #
        addl      $-16, %esp                                    #3701.5
        lea       _2__STRING.9@GOTOFF(%eax), %edx               #3701.5
        movl      %edx, (%esp)                                  #3701.5
        lea       _2__STRING.1@GOTOFF(%eax), %edx               #3701.5
        movl      %edx, 4(%esp)                                 #3701.5
        movl      $3701, 8(%esp)                                #3701.5
        lea       __$Ua@GOTOFF(%eax), %edx                      #3701.5
        movl      %edx, 12(%esp)                                #3701.5
        movl      %eax, %ebx                                    #3701.5
..___tag_value_eval_2na_pos.658:                                #3701.5
        call      __assert_fail@PLT                             #3701.5
        movl      -264(%ebp), %ebx                              #3701.5
..___tag_value_eval_2na_pos.659:                                #
                                # LOE
..B24.57:                       # Preds ..B24.2
        addl      $16, %esp                                     #3701.5
                                # LOE
..B24.3:                        # Preds ..B24.1
..LN3393:
        movl      16(%ebx), %eax                                #3705.12
..LN3395:
        addl      20(%ebx), %eax                                #3705.5
        movl      %eax, 20(%ebx)                                #3705.5
..LN3397:
        movl      16(%ebx), %eax                                #3709.13
..LN3399:
        movl      %eax, -44(%ebp)                               #3709.5
..LN3401:
        movl      16(%ebx), %eax                                #3713.42
..LN3403:
        shrl      $2, %eax                                      #3713.49
..LN3405:
        addl      12(%ebx), %eax                                #3713.30
..LN3407:
        movl      %eax, -40(%ebp)                               #3713.5
..LN3409:
        movl      -4(%ebp), %eax                                #3716.18
        negl      %eax                                          #3716.18
        addl      20(%ebx), %eax                                #3716.18
..LN3411:
        movl      %eax, -36(%ebp)                               #3716.5
..LN3413:
        movl      20(%ebx), %eax                                #3719.44
..LN3415:
        addl      $3, %eax                                      #3719.50
..LN3417:
        shrl      $2, %eax                                      #3719.57
..LN3419:
        addl      12(%ebx), %eax                                #3719.30
..LN3421:
        movl      %eax, -32(%ebp)                               #3719.5
..LN3423:
        addl      $-8, %esp                                     #3722.14
..LN3425:
        movl      -40(%ebp), %eax                               #3722.33
        movl      %eax, (%esp)                                  #3722.33
..LN3427:
        movl      -32(%ebp), %eax                               #3722.38
        movl      %eax, 4(%esp)                                 #3722.38
..LN3429:
        call      prime_buffer_2na                              #3722.14
                                # LOE xmm0
..B24.58:                       # Preds ..B24.3
        addl      $8, %esp                                      #3722.14
        movdqa    %xmm0, -72(%ebp)                              #3722.14
                                # LOE
..B24.4:                        # Preds ..B24.58
..LN3431:
        movdqa    -72(%ebp), %xmm0                              #3722.5
        movdqa    %xmm0, -216(%ebp)                             #3722.5
..LN3433:
        addl      $16, -40(%ebp)                                #3723.5
..LN3435:
        movl      -40(%ebp), %eax                               #3725.9
..LN3437:
        movl      %eax, -28(%ebp)                               #3725.5
..LN3439:
        movl      -40(%ebp), %eax                               #3728.10
..LN3441:
        movl      -32(%ebp), %edx                               #3728.16
        cmpl      %edx, %eax                                    #3728.16
        jae       ..B24.6       # Prob 50%                      #3728.16
                                # LOE
..B24.5:                        # Preds ..B24.4
..LN3443:
        movl      -40(%ebp), %eax                               #3729.24
        movzbl    -1(%eax), %eax                                #3729.24
        movzbl    %al, %eax                                     #3729.24
..LN3445:
        shll      $8, %eax                                      #3729.38
..LN3447:
        movl      %eax, -52(%ebp)                               #3729.9
                                # LOE
..B24.6:                        # Preds ..B24.5 ..B24.4
..LN3449:
        movl      8(%ebx), %eax                                 #3733.5
        movdqa    16(%eax), %xmm0                               #3733.5
        movdqa    %xmm0, -200(%ebp)                             #3733.5
        movl      8(%ebx), %eax                                 #3733.5
        movdqa    32(%eax), %xmm0                               #3733.5
        movdqa    %xmm0, -184(%ebp)                             #3733.5
        movl      8(%ebx), %eax                                 #3733.5
        movdqa    48(%eax), %xmm0                               #3733.5
        movdqa    %xmm0, -168(%ebp)                             #3733.5
        movl      8(%ebx), %eax                                 #3733.5
        movdqa    64(%eax), %xmm0                               #3733.5
        movdqa    %xmm0, -152(%ebp)                             #3733.5
        movl      8(%ebx), %eax                                 #3733.5
        movdqa    80(%eax), %xmm0                               #3733.5
        movdqa    %xmm0, -136(%ebp)                             #3733.5
        movl      8(%ebx), %eax                                 #3733.5
        movdqa    96(%eax), %xmm0                               #3733.5
        movdqa    %xmm0, -120(%ebp)                             #3733.5
        movl      8(%ebx), %eax                                 #3733.5
        movdqa    112(%eax), %xmm0                              #3733.5
        movdqa    %xmm0, -104(%ebp)                             #3733.5
        movl      8(%ebx), %eax                                 #3733.5
        movdqa    128(%eax), %xmm0                              #3733.5
        movdqa    %xmm0, -88(%ebp)                              #3733.5
..LN3451:
        xorl      %eax, %eax                                    #3736.15
        movl      %eax, -24(%ebp)                               #3736.15
..LN3453:
        movl      %eax, -20(%ebp)                               #3736.10
..LN3455:
        movl      %eax, -16(%ebp)                               #3736.5
..LN3457:
        movl      16(%ebx), %eax                                #3738.27
        negl      %eax                                          #3738.27
..LN3459:
        movl      -36(%ebp), %edx                               #3738.20
..LN3461:
        lea       7(%edx,%eax), %eax                            #3738.33
..LN3463:
        shrl      $2, %eax                                      #3738.40
..LN3465:
        movl      %eax, -12(%ebp)                               #3738.5
..LN3467:
        movl      16(%ebx), %eax                                #3745.14
..LN3469:
        andl      $3, %eax                                      #3745.20
        movl      %eax, -8(%ebp)                                #3745.20
        je        ..B24.14      # Prob 50%                      #3745.20
                                # LOE
..B24.7:                        # Preds ..B24.6
        movl      -8(%ebp), %eax                                #3745.20
        cmpl      $1, %eax                                      #3745.20
        je        ..B24.15      # Prob 50%                      #3745.20
                                # LOE
..B24.8:                        # Preds ..B24.7
        movl      -8(%ebp), %eax                                #3745.20
        cmpl      $2, %eax                                      #3745.20
        je        ..B24.16      # Prob 50%                      #3745.20
                                # LOE
..B24.9:                        # Preds ..B24.8
        movl      -8(%ebp), %eax                                #3745.20
        cmpl      $3, %eax                                      #3745.20
        je        ..B24.17      # Prob 50%                      #3745.20
                                # LOE
..B24.10:                       # Preds ..B24.53 ..B24.52 ..B24.9
..LN3471:
        movl      $1, %eax                                      #3750.9
        testl     %eax, %eax                                    #3750.9
        je        ..B24.54      # Prob 100%                     #3750.9
                                # LOE
..B24.11:                       # Preds ..B24.10
..LN3473:
        movl      $16, -12(%ebp)                                #3752.13
                                # LOE
..B24.12:                       # Preds ..B24.47 ..B24.11
..LN3475:
        movl      $1, %eax                                      #3755.13
        testl     %eax, %eax                                    #3755.13
        je        ..B24.49      # Prob 100%                     #3755.13
                                # LOE
..B24.14:                       # Preds ..B24.6 ..B24.12
..LN3477:
        movdqa    -216(%ebp), %xmm0                             #3762.38
..LN3479:
        movdqa    -184(%ebp), %xmm1                             #3762.46
..LN3481:
        pand      %xmm1, %xmm0                                  #3762.22
..LN3483:
        movdqa    %xmm0, -232(%ebp)                             #3762.17
..LN3485:
        movdqa    -232(%ebp), %xmm0                             #3763.22
        movdqa    -200(%ebp), %xmm1                             #3763.22
        pcmpeqd   %xmm1, %xmm0                                  #3763.22
..LN3487:
        movdqa    %xmm0, -232(%ebp)                             #3763.17
..LN3489:
        movdqa    -232(%ebp), %xmm0                             #3764.42
..LN3491:
        pmovmskb  %xmm0, %eax                                   #3764.22
..LN3493:
        movl      %eax, -16(%ebp)                               #3764.17
..LN3495:
        movl      -16(%ebp), %eax                               #3765.17
        incl      %eax                                          #3765.17
        sarl      $16, %eax                                     #3765.17
        negl      %eax                                          #3765.17
        movl      %eax, -16(%ebp)                               #3765.17
                                # LOE
..B24.15:                       # Preds ..B24.7 ..B24.14
..LN3497:
        movdqa    -216(%ebp), %xmm0                             #3768.38
..LN3499:
        movdqa    -152(%ebp), %xmm1                             #3768.46
..LN3501:
        pand      %xmm1, %xmm0                                  #3768.22
..LN3503:
        movdqa    %xmm0, -232(%ebp)                             #3768.17
..LN3505:
        movdqa    -232(%ebp), %xmm0                             #3769.22
        movdqa    -168(%ebp), %xmm1                             #3769.22
        pcmpeqd   %xmm1, %xmm0                                  #3769.22
..LN3507:
        movdqa    %xmm0, -232(%ebp)                             #3769.17
..LN3509:
        movdqa    -232(%ebp), %xmm0                             #3770.42
..LN3511:
        pmovmskb  %xmm0, %eax                                   #3770.22
..LN3513:
        movl      %eax, -20(%ebp)                               #3770.17
..LN3515:
        movl      -20(%ebp), %eax                               #3771.17
        incl      %eax                                          #3771.17
        sarl      $16, %eax                                     #3771.17
        negl      %eax                                          #3771.17
        movl      %eax, -20(%ebp)                               #3771.17
                                # LOE
..B24.16:                       # Preds ..B24.8 ..B24.15
..LN3517:
        movdqa    -216(%ebp), %xmm0                             #3774.38
..LN3519:
        movdqa    -120(%ebp), %xmm1                             #3774.46
..LN3521:
        pand      %xmm1, %xmm0                                  #3774.22
..LN3523:
        movdqa    %xmm0, -232(%ebp)                             #3774.17
..LN3525:
        movdqa    -232(%ebp), %xmm0                             #3775.22
        movdqa    -136(%ebp), %xmm1                             #3775.22
        pcmpeqd   %xmm1, %xmm0                                  #3775.22
..LN3527:
        movdqa    %xmm0, -232(%ebp)                             #3775.17
..LN3529:
        movdqa    -232(%ebp), %xmm0                             #3776.42
..LN3531:
        pmovmskb  %xmm0, %eax                                   #3776.22
..LN3533:
        movl      %eax, -24(%ebp)                               #3776.17
..LN3535:
        movl      -24(%ebp), %eax                               #3777.17
        incl      %eax                                          #3777.17
        sarl      $16, %eax                                     #3777.17
        negl      %eax                                          #3777.17
        movl      %eax, -24(%ebp)                               #3777.17
                                # LOE
..B24.17:                       # Preds ..B24.9 ..B24.16
..LN3537:
        movdqa    -216(%ebp), %xmm0                             #3780.38
..LN3539:
        movdqa    -88(%ebp), %xmm1                              #3780.46
..LN3541:
        pand      %xmm1, %xmm0                                  #3780.22
..LN3543:
        movdqa    %xmm0, -232(%ebp)                             #3780.17
..LN3545:
        movdqa    -232(%ebp), %xmm0                             #3781.22
        movdqa    -104(%ebp), %xmm1                             #3781.22
        pcmpeqd   %xmm1, %xmm0                                  #3781.22
..LN3547:
        movdqa    %xmm0, -232(%ebp)                             #3781.17
..LN3549:
        movdqa    -232(%ebp), %xmm0                             #3782.42
..LN3551:
        pmovmskb  %xmm0, %eax                                   #3782.22
..LN3553:
        movl      %eax, -48(%ebp)                               #3782.17
..LN3555:
        movl      -48(%ebp), %eax                               #3783.17
        incl      %eax                                          #3783.17
        sarl      $16, %eax                                     #3783.17
        negl      %eax                                          #3783.17
        movl      %eax, -48(%ebp)                               #3783.17
..LN3557:
        movl      16(%ebx), %eax                                #3787.17
        andl      $-4, %eax                                     #3787.17
        movl      %eax, 16(%ebx)                                #3787.17
..LN3559:
        movl      -20(%ebp), %eax                               #3790.29
        orl       -16(%ebp), %eax                               #3790.29
..LN3561:
        orl       -24(%ebp), %eax                               #3790.34
..LN3563:
        orl       -48(%ebp), %eax                               #3790.39
..LN3565:
        je        ..B24.42      # Prob 50%                      #3790.47
                                # LOE
..B24.18:                       # Preds ..B24.17
..LN3567:
        movl      16(%ebx), %eax                                #3793.37
        negl      %eax                                          #3793.37
        addl      -36(%ebp), %eax                               #3793.37
        movl      %eax, -56(%ebp)                               #3793.37
        je        ..B24.21      # Prob 50%                      #3793.37
                                # LOE
..B24.19:                       # Preds ..B24.18
        movl      -56(%ebp), %eax                               #3793.37
        cmpl      $1, %eax                                      #3793.37
        je        ..B24.23      # Prob 50%                      #3793.37
                                # LOE
..B24.20:                       # Preds ..B24.19
        movl      -56(%ebp), %eax                               #3793.37
        cmpl      $2, %eax                                      #3793.37
        je        ..B24.27      # Prob 50%                      #3793.37
        jmp       ..B24.33      # Prob 100%                     #3793.37
                                # LOE
..B24.21:                       # Preds ..B24.18
..LN3569:
        movl      -16(%ebp), %eax                               #3796.30
..LN3571:
        testl     %eax, %eax                                    #3796.36
        je        ..B24.41      # Prob 50%                      #3796.36
                                # LOE
..B24.22:                       # Preds ..B24.21
..LN3573:
        movl      -44(%ebp), %eax                               #3796.53
        negl      %eax                                          #3796.53
..LN3575:
        movl      16(%ebx), %edx                                #3796.47
..LN3577:
        lea       1(%edx,%eax), %eax                            #3796.61
        leave                                                   #3796.61
..___tag_value_eval_2na_pos.661:                                #
        movl      %ebx, %esp                                    #3796.61
        popl      %ebx                                          #3796.61
..___tag_value_eval_2na_pos.662:                                #
        ret                                                     #3796.61
..___tag_value_eval_2na_pos.664:                                #
                                # LOE
..B24.23:                       # Preds ..B24.19
..LN3579:
        movl      -16(%ebp), %eax                               #3799.30
..LN3581:
        testl     %eax, %eax                                    #3799.36
        je        ..B24.25      # Prob 50%                      #3799.36
                                # LOE
..B24.24:                       # Preds ..B24.23
..LN3583:
        movl      -44(%ebp), %eax                               #3799.53
        negl      %eax                                          #3799.53
..LN3585:
        movl      16(%ebx), %edx                                #3799.47
..LN3587:
        lea       1(%edx,%eax), %eax                            #3799.61
        leave                                                   #3799.61
..___tag_value_eval_2na_pos.668:                                #
        movl      %ebx, %esp                                    #3799.61
        popl      %ebx                                          #3799.61
..___tag_value_eval_2na_pos.669:                                #
        ret                                                     #3799.61
..___tag_value_eval_2na_pos.671:                                #
                                # LOE
..B24.25:                       # Preds ..B24.23
..LN3589:
        movl      -20(%ebp), %eax                               #3800.30
..LN3591:
        testl     %eax, %eax                                    #3800.36
        je        ..B24.41      # Prob 50%                      #3800.36
                                # LOE
..B24.26:                       # Preds ..B24.25
..LN3593:
        movl      -44(%ebp), %eax                               #3800.53
        negl      %eax                                          #3800.53
..LN3595:
        movl      16(%ebx), %edx                                #3800.47
..LN3597:
        lea       2(%edx,%eax), %eax                            #3800.61
        leave                                                   #3800.61
..___tag_value_eval_2na_pos.675:                                #
        movl      %ebx, %esp                                    #3800.61
        popl      %ebx                                          #3800.61
..___tag_value_eval_2na_pos.676:                                #
        ret                                                     #3800.61
..___tag_value_eval_2na_pos.678:                                #
                                # LOE
..B24.27:                       # Preds ..B24.20
..LN3599:
        movl      -16(%ebp), %eax                               #3803.30
..LN3601:
        testl     %eax, %eax                                    #3803.36
        je        ..B24.29      # Prob 50%                      #3803.36
                                # LOE
..B24.28:                       # Preds ..B24.27
..LN3603:
        movl      -44(%ebp), %eax                               #3803.53
        negl      %eax                                          #3803.53
..LN3605:
        movl      16(%ebx), %edx                                #3803.47
..LN3607:
        lea       1(%edx,%eax), %eax                            #3803.61
        leave                                                   #3803.61
..___tag_value_eval_2na_pos.682:                                #
        movl      %ebx, %esp                                    #3803.61
        popl      %ebx                                          #3803.61
..___tag_value_eval_2na_pos.683:                                #
        ret                                                     #3803.61
..___tag_value_eval_2na_pos.685:                                #
                                # LOE
..B24.29:                       # Preds ..B24.27
..LN3609:
        movl      -20(%ebp), %eax                               #3804.30
..LN3611:
        testl     %eax, %eax                                    #3804.36
        je        ..B24.31      # Prob 50%                      #3804.36
                                # LOE
..B24.30:                       # Preds ..B24.29
..LN3613:
        movl      -44(%ebp), %eax                               #3804.53
        negl      %eax                                          #3804.53
..LN3615:
        movl      16(%ebx), %edx                                #3804.47
..LN3617:
        lea       2(%edx,%eax), %eax                            #3804.61
        leave                                                   #3804.61
..___tag_value_eval_2na_pos.689:                                #
        movl      %ebx, %esp                                    #3804.61
        popl      %ebx                                          #3804.61
..___tag_value_eval_2na_pos.690:                                #
        ret                                                     #3804.61
..___tag_value_eval_2na_pos.692:                                #
                                # LOE
..B24.31:                       # Preds ..B24.29
..LN3619:
        movl      -24(%ebp), %eax                               #3805.30
..LN3621:
        testl     %eax, %eax                                    #3805.36
        je        ..B24.41      # Prob 50%                      #3805.36
                                # LOE
..B24.32:                       # Preds ..B24.31
..LN3623:
        movl      -44(%ebp), %eax                               #3805.53
        negl      %eax                                          #3805.53
..LN3625:
        movl      16(%ebx), %edx                                #3805.47
..LN3627:
        lea       3(%edx,%eax), %eax                            #3805.61
        leave                                                   #3805.61
..___tag_value_eval_2na_pos.696:                                #
        movl      %ebx, %esp                                    #3805.61
        popl      %ebx                                          #3805.61
..___tag_value_eval_2na_pos.697:                                #
        ret                                                     #3805.61
..___tag_value_eval_2na_pos.699:                                #
                                # LOE
..B24.33:                       # Preds ..B24.20
..LN3629:
        movl      -16(%ebp), %eax                               #3808.30
..LN3631:
        testl     %eax, %eax                                    #3808.36
        je        ..B24.35      # Prob 50%                      #3808.36
                                # LOE
..B24.34:                       # Preds ..B24.33
..LN3633:
        movl      -44(%ebp), %eax                               #3808.53
        negl      %eax                                          #3808.53
..LN3635:
        movl      16(%ebx), %edx                                #3808.47
..LN3637:
        lea       1(%edx,%eax), %eax                            #3808.61
        leave                                                   #3808.61
..___tag_value_eval_2na_pos.703:                                #
        movl      %ebx, %esp                                    #3808.61
        popl      %ebx                                          #3808.61
..___tag_value_eval_2na_pos.704:                                #
        ret                                                     #3808.61
..___tag_value_eval_2na_pos.706:                                #
                                # LOE
..B24.35:                       # Preds ..B24.33
..LN3639:
        movl      -20(%ebp), %eax                               #3809.30
..LN3641:
        testl     %eax, %eax                                    #3809.36
        je        ..B24.37      # Prob 50%                      #3809.36
                                # LOE
..B24.36:                       # Preds ..B24.35
..LN3643:
        movl      -44(%ebp), %eax                               #3809.53
        negl      %eax                                          #3809.53
..LN3645:
        movl      16(%ebx), %edx                                #3809.47
..LN3647:
        lea       2(%edx,%eax), %eax                            #3809.61
        leave                                                   #3809.61
..___tag_value_eval_2na_pos.710:                                #
        movl      %ebx, %esp                                    #3809.61
        popl      %ebx                                          #3809.61
..___tag_value_eval_2na_pos.711:                                #
        ret                                                     #3809.61
..___tag_value_eval_2na_pos.713:                                #
                                # LOE
..B24.37:                       # Preds ..B24.35
..LN3649:
        movl      -24(%ebp), %eax                               #3810.30
..LN3651:
        testl     %eax, %eax                                    #3810.36
        je        ..B24.39      # Prob 50%                      #3810.36
                                # LOE
..B24.38:                       # Preds ..B24.37
..LN3653:
        movl      -44(%ebp), %eax                               #3810.53
        negl      %eax                                          #3810.53
..LN3655:
        movl      16(%ebx), %edx                                #3810.47
..LN3657:
        lea       3(%edx,%eax), %eax                            #3810.61
        leave                                                   #3810.61
..___tag_value_eval_2na_pos.717:                                #
        movl      %ebx, %esp                                    #3810.61
        popl      %ebx                                          #3810.61
..___tag_value_eval_2na_pos.718:                                #
        ret                                                     #3810.61
..___tag_value_eval_2na_pos.720:                                #
                                # LOE
..B24.39:                       # Preds ..B24.37
..LN3659:
        movl      -48(%ebp), %eax                               #3811.30
..LN3661:
        testl     %eax, %eax                                    #3811.36
        je        ..B24.41      # Prob 50%                      #3811.36
                                # LOE
..B24.40:                       # Preds ..B24.39
..LN3663:
        movl      -44(%ebp), %eax                               #3811.53
        negl      %eax                                          #3811.53
..LN3665:
        movl      16(%ebx), %edx                                #3811.47
..LN3667:
        lea       4(%edx,%eax), %eax                            #3811.61
        leave                                                   #3811.61
..___tag_value_eval_2na_pos.724:                                #
        movl      %ebx, %esp                                    #3811.61
        popl      %ebx                                          #3811.61
..___tag_value_eval_2na_pos.725:                                #
        ret                                                     #3811.61
..___tag_value_eval_2na_pos.727:                                #
                                # LOE
..B24.41:                       # Preds ..B24.21 ..B24.25 ..B24.31 ..B24.39
..LN3669:
        xorl      %eax, %eax                                    #3813.28
        leave                                                   #3813.28
..___tag_value_eval_2na_pos.731:                                #
        movl      %ebx, %esp                                    #3813.28
        popl      %ebx                                          #3813.28
..___tag_value_eval_2na_pos.732:                                #
        ret                                                     #3813.28
..___tag_value_eval_2na_pos.734:                                #
                                # LOE
..B24.42:                       # Preds ..B24.17
..LN3671:
        movl      16(%ebx), %eax                                #3853.17
        addl      $4, %eax                                      #3853.17
        movl      %eax, 16(%ebx)                                #3853.17
..LN3673:
        movl      16(%ebx), %eax                                #3856.22
..LN3675:
        movl      -36(%ebp), %edx                               #3856.28
        cmpl      %edx, %eax                                    #3856.28
        jbe       ..B24.44      # Prob 50%                      #3856.28
                                # LOE
..B24.43:                       # Preds ..B24.42
..LN3677:
        xorl      %eax, %eax                                    #3857.28
        leave                                                   #3857.28
..___tag_value_eval_2na_pos.738:                                #
        movl      %ebx, %esp                                    #3857.28
        popl      %ebx                                          #3857.28
..___tag_value_eval_2na_pos.739:                                #
        ret                                                     #3857.28
..___tag_value_eval_2na_pos.741:                                #
                                # LOE
..B24.44:                       # Preds ..B24.42
..LN3679:
        movl      -12(%ebp), %eax                               #3860.25
        decl      %eax                                          #3860.25
        movl      %eax, -12(%ebp)                               #3860.25
..LN3681:
        je        ..B24.49      # Prob 50%                      #3860.39
                                # LOE
..B24.45:                       # Preds ..B24.44
..LN3683:
        movdqa    -216(%ebp), %xmm0                             #3865.43
..LN3685:
        psrldq    $1, %xmm0                                     #3865.26
..LN3687:
        movdqa    %xmm0, -216(%ebp)                             #3865.17
..LN3689:
        movl      -28(%ebp), %eax                               #3868.22
..LN3691:
        movl      -32(%ebp), %edx                               #3868.26
        cmpl      %edx, %eax                                    #3868.26
        jae       ..B24.47      # Prob 50%                      #3868.26
                                # LOE
..B24.46:                       # Preds ..B24.45
..LN3693:
        sarl      $8, -52(%ebp)                                 #3870.21
..LN3695:
        movl      -28(%ebp), %eax                               #3871.37
        movzbl    (%eax), %eax                                  #3871.37
        movzbl    %al, %eax                                     #3871.37
..LN3697:
        shll      $8, %eax                                      #3871.48
..LN3699:
        orl       -52(%ebp), %eax                               #3871.21
        movl      %eax, -52(%ebp)                               #3871.21
..LN3701:
        movdqa    -216(%ebp), %xmm0                             #3872.49
..LN3703:
        movl      -52(%ebp), %eax                               #3872.57
..LN3705:
        pinsrw    $7, %eax, %xmm0                               #3872.30
..LN3707:
        movdqa    %xmm0, -216(%ebp)                             #3872.21
                                # LOE
..B24.47:                       # Preds ..B24.46 ..B24.45
..LN3709:
        incl      -28(%ebp)                                     #3876.20
        jmp       ..B24.12      # Prob 100%                     #3876.20
                                # LOE
..B24.49:                       # Preds ..B24.44 ..B24.12
..LN3711:
        movl      -40(%ebp), %eax                               #3881.18
..LN3713:
        movl      -32(%ebp), %edx                               #3881.25
        cmpl      %edx, %eax                                    #3881.25
        jae       ..B24.54      # Prob 50%                      #3881.25
                                # LOE
..B24.50:                       # Preds ..B24.49
..LN3715:
        movl      16(%ebx), %eax                                #3886.18
..LN3717:
        movl      -36(%ebp), %edx                               #3886.24
        cmpl      %edx, %eax                                    #3886.24
        ja        ..B24.54      # Prob 50%                      #3886.24
                                # LOE
..B24.51:                       # Preds ..B24.50
..LN3719:
        addl      $-8, %esp                                     #3891.22
..LN3721:
        movl      -40(%ebp), %eax                               #3891.41
        movl      %eax, (%esp)                                  #3891.41
..LN3723:
        movl      -32(%ebp), %eax                               #3891.46
        movl      %eax, 4(%esp)                                 #3891.46
..LN3725:
        call      prime_buffer_2na                              #3891.22
                                # LOE xmm0
..B24.59:                       # Preds ..B24.51
        addl      $8, %esp                                      #3891.22
        movdqa    %xmm0, -248(%ebp)                             #3891.22
                                # LOE
..B24.52:                       # Preds ..B24.59
..LN3727:
        movdqa    -248(%ebp), %xmm0                             #3891.13
        movdqa    %xmm0, -216(%ebp)                             #3891.13
..LN3729:
        addl      $16, -40(%ebp)                                #3969.13
..LN3731:
        movl      -40(%ebp), %eax                               #3971.17
..LN3733:
        movl      %eax, -28(%ebp)                               #3971.13
..LN3735:
        movl      -40(%ebp), %eax                               #3972.18
..LN3737:
        movl      -32(%ebp), %edx                               #3972.24
        cmpl      %edx, %eax                                    #3972.24
        jae       ..B24.10      # Prob 50%                      #3972.24
                                # LOE
..B24.53:                       # Preds ..B24.52
..LN3739:
        movl      -40(%ebp), %eax                               #3973.32
        movzbl    -1(%eax), %eax                                #3973.32
        movzbl    %al, %eax                                     #3973.32
..LN3741:
        shll      $8, %eax                                      #3973.46
..LN3743:
        movl      %eax, -52(%ebp)                               #3973.17
        jmp       ..B24.10      # Prob 100%                     #3973.17
                                # LOE
..B24.54:                       # Preds ..B24.50 ..B24.49 ..B24.10
..LN3745:
        xorl      %eax, %eax                                    #3983.12
        leave                                                   #3983.12
..___tag_value_eval_2na_pos.745:                                #
        movl      %ebx, %esp                                    #3983.12
        popl      %ebx                                          #3983.12
..___tag_value_eval_2na_pos.746:                                #
        ret                                                     #3983.12
        .align    2,0x90
..___tag_value_eval_2na_pos.748:                                #
                                # LOE
# mark_end;
	.type	eval_2na_pos,@function
	.size	eval_2na_pos,.-eval_2na_pos
.LNeval_2na_pos:
	.data
# -- End  eval_2na_pos
	.section .rodata, "a"
	.align 4
__$Ub:
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
	.type	__$Ub,@object
	.size	__$Ub,12
	.text
# -- Begin  eval_4na_16
# mark_begin;
       .align    2,0x90
eval_4na_16:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B25.1:                        # Preds ..B25.0
..___tag_value_eval_4na_16.751:                                 #
..LN3747:
        pushl     %ebx                                          #4057.1
..___tag_value_eval_4na_16.756:                                 #
        movl      %esp, %ebx                                    #4057.1
..___tag_value_eval_4na_16.757:                                 #
        andl      $-16, %esp                                    #4057.1
        pushl     %ebp                                          #4057.1
        pushl     %ebp                                          #4057.1
        movl      4(%ebx), %ebp                                 #4057.1
        movl      %ebp, 4(%esp)                                 #4057.1
        movl      %esp, %ebp                                    #4057.1
..___tag_value_eval_4na_16.759:                                 #
        subl      $296, %esp                                    #4057.1
        movl      %ebx, -296(%ebp)                              #4057.1
..LN3749:
        movl      8(%ebx), %eax                                 #4083.25
        movl      4(%eax), %eax                                 #4083.25
..LN3751:
        movl      %eax, -4(%ebp)                                #4083.23
..LN3753:
        movl      20(%ebx), %eax                                #4086.5
        movl      -4(%ebp), %edx                                #4086.5
        cmpl      %edx, %eax                                    #4086.5
        jae       ..B25.3       # Prob 50%                      #4086.5
                                # LOE
..B25.2:                        # Preds ..B25.1
        call      ..L761        # Prob 100%                     #
..L761:                                                         #
        popl      %eax                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L761], %eax     #
        addl      $-16, %esp                                    #4086.5
        lea       _2__STRING.9@GOTOFF(%eax), %edx               #4086.5
        movl      %edx, (%esp)                                  #4086.5
        lea       _2__STRING.1@GOTOFF(%eax), %edx               #4086.5
        movl      %edx, 4(%esp)                                 #4086.5
        movl      $4086, 8(%esp)                                #4086.5
        lea       __$Ub@GOTOFF(%eax), %edx                      #4086.5
        movl      %edx, 12(%esp)                                #4086.5
        movl      %eax, %ebx                                    #4086.5
..___tag_value_eval_4na_16.762:                                 #4086.5
        call      __assert_fail@PLT                             #4086.5
        movl      -296(%ebp), %ebx                              #4086.5
..___tag_value_eval_4na_16.763:                                 #
                                # LOE
..B25.43:                       # Preds ..B25.2
        addl      $16, %esp                                     #4086.5
                                # LOE
..B25.3:                        # Preds ..B25.1
..LN3755:
        movl      16(%ebx), %eax                                #4090.12
..LN3757:
        addl      20(%ebx), %eax                                #4090.5
        movl      %eax, 20(%ebx)                                #4090.5
..LN3759:
        movl      16(%ebx), %eax                                #4093.42
..LN3761:
        shrl      $2, %eax                                      #4093.49
..LN3763:
        addl      12(%ebx), %eax                                #4093.30
..LN3765:
        movl      %eax, -36(%ebp)                               #4093.5
..LN3767:
        movl      -4(%ebp), %eax                                #4096.18
        negl      %eax                                          #4096.18
        addl      20(%ebx), %eax                                #4096.18
..LN3769:
        movl      %eax, -32(%ebp)                               #4096.5
..LN3771:
        movl      20(%ebx), %eax                                #4099.44
..LN3773:
        addl      $3, %eax                                      #4099.50
..LN3775:
        shrl      $2, %eax                                      #4099.57
..LN3777:
        addl      12(%ebx), %eax                                #4099.30
..LN3779:
        movl      %eax, -28(%ebp)                               #4099.5
..LN3781:
        addl      $-8, %esp                                     #4102.14
..LN3783:
        movl      -36(%ebp), %eax                               #4102.33
        movl      %eax, (%esp)                                  #4102.33
..LN3785:
        movl      -28(%ebp), %eax                               #4102.38
        movl      %eax, 4(%esp)                                 #4102.38
..LN3787:
        call      prime_buffer_4na                              #4102.14
                                # LOE xmm0
..B25.44:                       # Preds ..B25.3
        addl      $8, %esp                                      #4102.14
        movdqa    %xmm0, -88(%ebp)                              #4102.14
                                # LOE
..B25.4:                        # Preds ..B25.44
..LN3789:
        movdqa    -88(%ebp), %xmm0                              #4102.5
        movdqa    %xmm0, -232(%ebp)                             #4102.5
..LN3791:
        addl      $8, -36(%ebp)                                 #4103.5
..LN3793:
        movl      8(%ebx), %eax                                 #4109.5
        movdqa    16(%eax), %xmm0                               #4109.5
        movdqa    %xmm0, -216(%ebp)                             #4109.5
        movl      8(%ebx), %eax                                 #4109.5
        movdqa    32(%eax), %xmm0                               #4109.5
        movdqa    %xmm0, -200(%ebp)                             #4109.5
        movl      8(%ebx), %eax                                 #4109.5
        movdqa    48(%eax), %xmm0                               #4109.5
        movdqa    %xmm0, -184(%ebp)                             #4109.5
        movl      8(%ebx), %eax                                 #4109.5
        movdqa    64(%eax), %xmm0                               #4109.5
        movdqa    %xmm0, -168(%ebp)                             #4109.5
        movl      8(%ebx), %eax                                 #4109.5
        movdqa    80(%eax), %xmm0                               #4109.5
        movdqa    %xmm0, -152(%ebp)                             #4109.5
        movl      8(%ebx), %eax                                 #4109.5
        movdqa    96(%eax), %xmm0                               #4109.5
        movdqa    %xmm0, -136(%ebp)                             #4109.5
        movl      8(%ebx), %eax                                 #4109.5
        movdqa    112(%eax), %xmm0                              #4109.5
        movdqa    %xmm0, -120(%ebp)                             #4109.5
        movl      8(%ebx), %eax                                 #4109.5
        movdqa    128(%eax), %xmm0                              #4109.5
        movdqa    %xmm0, -104(%ebp)                             #4109.5
..LN3795:
        xorl      %eax, %eax                                    #4112.15
        movl      %eax, -24(%ebp)                               #4112.15
..LN3797:
        movl      %eax, -20(%ebp)                               #4112.10
..LN3799:
        movl      %eax, -16(%ebp)                               #4112.5
..LN3801:
        movl      $1, -12(%ebp)                                 #4118.5
..LN3803:
        movl      16(%ebx), %eax                                #4123.14
..LN3805:
        andl      $3, %eax                                      #4123.20
        movl      %eax, -8(%ebp)                                #4123.20
        je        ..B25.12      # Prob 50%                      #4123.20
                                # LOE
..B25.5:                        # Preds ..B25.4
        movl      -8(%ebp), %eax                                #4123.20
        cmpl      $1, %eax                                      #4123.20
        je        ..B25.13      # Prob 50%                      #4123.20
                                # LOE
..B25.6:                        # Preds ..B25.5
        movl      -8(%ebp), %eax                                #4123.20
        cmpl      $2, %eax                                      #4123.20
        je        ..B25.14      # Prob 50%                      #4123.20
                                # LOE
..B25.7:                        # Preds ..B25.6
        movl      -8(%ebp), %eax                                #4123.20
        cmpl      $3, %eax                                      #4123.20
        je        ..B25.15      # Prob 50%                      #4123.20
                                # LOE
..B25.8:                        # Preds ..B25.39 ..B25.7
..LN3807:
        movl      $1, %eax                                      #4128.9
        testl     %eax, %eax                                    #4128.9
        je        ..B25.40      # Prob 100%                     #4128.9
                                # LOE
..B25.9:                        # Preds ..B25.8
..LN3809:
        movl      $1, -12(%ebp)                                 #4130.13
                                # LOE
..B25.10:                       # Preds ..B25.34 ..B25.9
..LN3811:
        movl      $1, %eax                                      #4133.13
        testl     %eax, %eax                                    #4133.13
        je        ..B25.36      # Prob 100%                     #4133.13
                                # LOE
..B25.12:                       # Preds ..B25.4 ..B25.10
..LN3813:
        movdqa    -232(%ebp), %xmm0                             #4140.38
..LN3815:
        movdqa    -216(%ebp), %xmm1                             #4140.46
..LN3817:
        pand      %xmm1, %xmm0                                  #4140.22
..LN3819:
        movdqa    %xmm0, -264(%ebp)                             #4140.17
..LN3821:
        movdqa    -232(%ebp), %xmm0                             #4141.38
..LN3823:
        movdqa    -200(%ebp), %xmm1                             #4141.46
..LN3825:
        pand      %xmm1, %xmm0                                  #4141.22
..LN3827:
        movdqa    %xmm0, -248(%ebp)                             #4141.17
..LN3829:
        movdqa    -264(%ebp), %xmm0                             #4142.22
        movdqa    -248(%ebp), %xmm1                             #4142.22
        pcmpeqw   %xmm1, %xmm0                                  #4142.22
..LN3831:
        movdqa    %xmm0, -264(%ebp)                             #4142.17
..LN3833:
        movdqa    -264(%ebp), %xmm0                             #4143.42
..LN3835:
        pmovmskb  %xmm0, %eax                                   #4143.22
..LN3837:
        movl      %eax, -16(%ebp)                               #4143.17
                                # LOE
..B25.13:                       # Preds ..B25.5 ..B25.12
..LN3839:
        movdqa    -232(%ebp), %xmm0                             #4147.38
..LN3841:
        movdqa    -184(%ebp), %xmm1                             #4147.46
..LN3843:
        pand      %xmm1, %xmm0                                  #4147.22
..LN3845:
        movdqa    %xmm0, -264(%ebp)                             #4147.17
..LN3847:
        movdqa    -232(%ebp), %xmm0                             #4148.38
..LN3849:
        movdqa    -168(%ebp), %xmm1                             #4148.46
..LN3851:
        pand      %xmm1, %xmm0                                  #4148.22
..LN3853:
        movdqa    %xmm0, -248(%ebp)                             #4148.17
..LN3855:
        movdqa    -264(%ebp), %xmm0                             #4149.22
        movdqa    -248(%ebp), %xmm1                             #4149.22
        pcmpeqw   %xmm1, %xmm0                                  #4149.22
..LN3857:
        movdqa    %xmm0, -264(%ebp)                             #4149.17
..LN3859:
        movdqa    -264(%ebp), %xmm0                             #4150.42
..LN3861:
        pmovmskb  %xmm0, %eax                                   #4150.22
..LN3863:
        movl      %eax, -20(%ebp)                               #4150.17
                                # LOE
..B25.14:                       # Preds ..B25.6 ..B25.13
..LN3865:
        movdqa    -232(%ebp), %xmm0                             #4154.38
..LN3867:
        movdqa    -152(%ebp), %xmm1                             #4154.46
..LN3869:
        pand      %xmm1, %xmm0                                  #4154.22
..LN3871:
        movdqa    %xmm0, -264(%ebp)                             #4154.17
..LN3873:
        movdqa    -232(%ebp), %xmm0                             #4155.38
..LN3875:
        movdqa    -136(%ebp), %xmm1                             #4155.46
..LN3877:
        pand      %xmm1, %xmm0                                  #4155.22
..LN3879:
        movdqa    %xmm0, -248(%ebp)                             #4155.17
..LN3881:
        movdqa    -264(%ebp), %xmm0                             #4156.22
        movdqa    -248(%ebp), %xmm1                             #4156.22
        pcmpeqw   %xmm1, %xmm0                                  #4156.22
..LN3883:
        movdqa    %xmm0, -264(%ebp)                             #4156.17
..LN3885:
        movdqa    -264(%ebp), %xmm0                             #4157.42
..LN3887:
        pmovmskb  %xmm0, %eax                                   #4157.22
..LN3889:
        movl      %eax, -24(%ebp)                               #4157.17
                                # LOE
..B25.15:                       # Preds ..B25.7 ..B25.14
..LN3891:
        movdqa    -232(%ebp), %xmm0                             #4161.38
..LN3893:
        movdqa    -120(%ebp), %xmm1                             #4161.46
..LN3895:
        pand      %xmm1, %xmm0                                  #4161.22
..LN3897:
        movdqa    %xmm0, -264(%ebp)                             #4161.17
..LN3899:
        movdqa    -232(%ebp), %xmm0                             #4162.38
..LN3901:
        movdqa    -104(%ebp), %xmm1                             #4162.46
..LN3903:
        pand      %xmm1, %xmm0                                  #4162.22
..LN3905:
        movdqa    %xmm0, -248(%ebp)                             #4162.17
..LN3907:
        movdqa    -264(%ebp), %xmm0                             #4163.22
        movdqa    -248(%ebp), %xmm1                             #4163.22
        pcmpeqw   %xmm1, %xmm0                                  #4163.22
..LN3909:
        movdqa    %xmm0, -264(%ebp)                             #4163.17
..LN3911:
        movdqa    -264(%ebp), %xmm0                             #4164.42
..LN3913:
        pmovmskb  %xmm0, %eax                                   #4164.22
..LN3915:
        movl      %eax, -40(%ebp)                               #4164.17
..LN3917:
        movl      16(%ebx), %eax                                #4169.17
        andl      $-4, %eax                                     #4169.17
        movl      %eax, 16(%ebx)                                #4169.17
..LN3919:
        movl      -20(%ebp), %eax                               #4172.29
        orl       -16(%ebp), %eax                               #4172.29
..LN3921:
        orl       -24(%ebp), %eax                               #4172.34
..LN3923:
        orl       -40(%ebp), %eax                               #4172.39
..LN3925:
        je        ..B25.32      # Prob 50%                      #4172.47
                                # LOE
..B25.16:                       # Preds ..B25.15
..LN3927:
        pushl     %edi                                          #4190.30
..LN3929:
        movl      -16(%ebp), %eax                               #4190.58
        movw      %ax, (%esp)                                   #4190.58
..LN3931:
        call      uint16_lsbit                                  #4190.30
                                # LOE eax
..B25.45:                       # Preds ..B25.16
        popl      %ecx                                          #4190.30
        movl      %eax, -56(%ebp)                               #4190.30
                                # LOE
..B25.17:                       # Preds ..B25.45
        movl      -56(%ebp), %eax                               #4190.30
        movswl    %ax, %eax                                     #4190.30
..LN3933:
        movl      %eax, -72(%ebp)                               #4190.28
..LN3935:
        pushl     %edi                                          #4191.30
..LN3937:
        movl      -20(%ebp), %eax                               #4191.58
        movw      %ax, (%esp)                                   #4191.58
..LN3939:
        call      uint16_lsbit                                  #4191.30
                                # LOE eax
..B25.46:                       # Preds ..B25.17
        popl      %ecx                                          #4191.30
        movl      %eax, -52(%ebp)                               #4191.30
                                # LOE
..B25.18:                       # Preds ..B25.46
        movl      -52(%ebp), %eax                               #4191.30
        movswl    %ax, %eax                                     #4191.30
..LN3941:
        movl      %eax, -68(%ebp)                               #4191.28
..LN3943:
        pushl     %edi                                          #4192.30
..LN3945:
        movl      -24(%ebp), %eax                               #4192.58
        movw      %ax, (%esp)                                   #4192.58
..LN3947:
        call      uint16_lsbit                                  #4192.30
                                # LOE eax
..B25.47:                       # Preds ..B25.18
        popl      %ecx                                          #4192.30
        movl      %eax, -48(%ebp)                               #4192.30
                                # LOE
..B25.19:                       # Preds ..B25.47
        movl      -48(%ebp), %eax                               #4192.30
        movswl    %ax, %eax                                     #4192.30
..LN3949:
        movl      %eax, -64(%ebp)                               #4192.28
..LN3951:
        pushl     %edi                                          #4193.30
..LN3953:
        movl      -40(%ebp), %eax                               #4193.58
        movw      %ax, (%esp)                                   #4193.58
..LN3955:
        call      uint16_lsbit                                  #4193.30
                                # LOE eax
..B25.48:                       # Preds ..B25.19
        popl      %ecx                                          #4193.30
        movl      %eax, -44(%ebp)                               #4193.30
                                # LOE
..B25.20:                       # Preds ..B25.48
        movl      -44(%ebp), %eax                               #4193.30
        movswl    %ax, %eax                                     #4193.30
..LN3957:
        movl      %eax, -60(%ebp)                               #4193.28
..LN3959:
        movl      -72(%ebp), %eax                               #4198.28
..LN3961:
        addl      %eax, %eax                                    #4198.34
..LN3963:
        movl      %eax, -72(%ebp)                               #4198.21
..LN3965:
        movl      -68(%ebp), %eax                               #4199.28
..LN3967:
        lea       1(%eax,%eax), %eax                            #4199.40
..LN3969:
        movl      %eax, -68(%ebp)                               #4199.21
..LN3971:
        movl      -64(%ebp), %eax                               #4200.28
..LN3973:
        lea       2(%eax,%eax), %eax                            #4200.40
..LN3975:
        movl      %eax, -64(%ebp)                               #4200.21
..LN3977:
        movl      -60(%ebp), %eax                               #4201.28
..LN3979:
        lea       3(%eax,%eax), %eax                            #4201.40
..LN3981:
        movl      %eax, -60(%ebp)                               #4201.21
..LN3983:
        movl      -16(%ebp), %eax                               #4205.26
..LN3985:
        testl     %eax, %eax                                    #4205.32
        je        ..B25.23      # Prob 50%                      #4205.32
                                # LOE
..B25.21:                       # Preds ..B25.20
..LN3987:
        movl      -72(%ebp), %eax                               #4205.43
        addl      16(%ebx), %eax                                #4205.43
..LN3989:
        movl      -32(%ebp), %edx                               #4205.49
        cmpl      %edx, %eax                                    #4205.49
        ja        ..B25.23      # Prob 50%                      #4205.49
                                # LOE
..B25.22:                       # Preds ..B25.21
..LN3991:
        movl      $1, %eax                                      #4205.63
        leave                                                   #4205.63
..___tag_value_eval_4na_16.765:                                 #
        movl      %ebx, %esp                                    #4205.63
        popl      %ebx                                          #4205.63
..___tag_value_eval_4na_16.766:                                 #
        ret                                                     #4205.63
..___tag_value_eval_4na_16.768:                                 #
                                # LOE
..B25.23:                       # Preds ..B25.21 ..B25.20
..LN3993:
        movl      -20(%ebp), %eax                               #4206.26
..LN3995:
        testl     %eax, %eax                                    #4206.32
        je        ..B25.26      # Prob 50%                      #4206.32
                                # LOE
..B25.24:                       # Preds ..B25.23
..LN3997:
        movl      -68(%ebp), %eax                               #4206.43
        addl      16(%ebx), %eax                                #4206.43
..LN3999:
        movl      -32(%ebp), %edx                               #4206.49
        cmpl      %edx, %eax                                    #4206.49
        ja        ..B25.26      # Prob 50%                      #4206.49
                                # LOE
..B25.25:                       # Preds ..B25.24
..LN4001:
        movl      $1, %eax                                      #4206.63
        leave                                                   #4206.63
..___tag_value_eval_4na_16.772:                                 #
        movl      %ebx, %esp                                    #4206.63
        popl      %ebx                                          #4206.63
..___tag_value_eval_4na_16.773:                                 #
        ret                                                     #4206.63
..___tag_value_eval_4na_16.775:                                 #
                                # LOE
..B25.26:                       # Preds ..B25.24 ..B25.23
..LN4003:
        movl      -24(%ebp), %eax                               #4207.26
..LN4005:
        testl     %eax, %eax                                    #4207.32
        je        ..B25.29      # Prob 50%                      #4207.32
                                # LOE
..B25.27:                       # Preds ..B25.26
..LN4007:
        movl      -64(%ebp), %eax                               #4207.43
        addl      16(%ebx), %eax                                #4207.43
..LN4009:
        movl      -32(%ebp), %edx                               #4207.49
        cmpl      %edx, %eax                                    #4207.49
        ja        ..B25.29      # Prob 50%                      #4207.49
                                # LOE
..B25.28:                       # Preds ..B25.27
..LN4011:
        movl      $1, %eax                                      #4207.63
        leave                                                   #4207.63
..___tag_value_eval_4na_16.779:                                 #
        movl      %ebx, %esp                                    #4207.63
        popl      %ebx                                          #4207.63
..___tag_value_eval_4na_16.780:                                 #
        ret                                                     #4207.63
..___tag_value_eval_4na_16.782:                                 #
                                # LOE
..B25.29:                       # Preds ..B25.27 ..B25.26
..LN4013:
        movl      -40(%ebp), %eax                               #4208.26
..LN4015:
        testl     %eax, %eax                                    #4208.32
        je        ..B25.32      # Prob 50%                      #4208.32
                                # LOE
..B25.30:                       # Preds ..B25.29
..LN4017:
        movl      -60(%ebp), %eax                               #4208.43
        addl      16(%ebx), %eax                                #4208.43
..LN4019:
        movl      -32(%ebp), %edx                               #4208.49
        cmpl      %edx, %eax                                    #4208.49
        ja        ..B25.32      # Prob 50%                      #4208.49
                                # LOE
..B25.31:                       # Preds ..B25.30
..LN4021:
        movl      $1, %eax                                      #4208.63
        leave                                                   #4208.63
..___tag_value_eval_4na_16.786:                                 #
        movl      %ebx, %esp                                    #4208.63
        popl      %ebx                                          #4208.63
..___tag_value_eval_4na_16.787:                                 #
        ret                                                     #4208.63
..___tag_value_eval_4na_16.789:                                 #
                                # LOE
..B25.32:                       # Preds ..B25.30 ..B25.29 ..B25.15
..LN4023:
        movl      16(%ebx), %eax                                #4213.17
        addl      $4, %eax                                      #4213.17
        movl      %eax, 16(%ebx)                                #4213.17
..LN4025:
        movl      16(%ebx), %eax                                #4216.22
..LN4027:
        movl      -32(%ebp), %edx                               #4216.28
        cmpl      %edx, %eax                                    #4216.28
        jbe       ..B25.34      # Prob 50%                      #4216.28
                                # LOE
..B25.33:                       # Preds ..B25.32
..LN4029:
        xorl      %eax, %eax                                    #4217.28
        leave                                                   #4217.28
..___tag_value_eval_4na_16.793:                                 #
        movl      %ebx, %esp                                    #4217.28
        popl      %ebx                                          #4217.28
..___tag_value_eval_4na_16.794:                                 #
        ret                                                     #4217.28
..___tag_value_eval_4na_16.796:                                 #
                                # LOE
..B25.34:                       # Preds ..B25.32
..LN4031:
        movl      -12(%ebp), %eax                               #4220.25
        decl      %eax                                          #4220.25
        movl      %eax, -12(%ebp)                               #4220.25
..LN4033:
        jne       ..B25.10      # Prob 50%                      #4220.39
                                # LOE
..B25.36:                       # Preds ..B25.34 ..B25.10
..LN4035:
        movl      -36(%ebp), %eax                               #4237.18
..LN4037:
        movl      -28(%ebp), %edx                               #4237.25
        cmpl      %edx, %eax                                    #4237.25
        jae       ..B25.40      # Prob 50%                      #4237.25
                                # LOE
..B25.37:                       # Preds ..B25.36
..LN4039:
        movl      16(%ebx), %eax                                #4241.13
        addl      $28, %eax                                     #4241.13
        movl      %eax, 16(%ebx)                                #4241.13
..LN4041:
        movl      16(%ebx), %eax                                #4242.18
..LN4043:
        movl      -32(%ebp), %edx                               #4242.24
        cmpl      %edx, %eax                                    #4242.24
        ja        ..B25.40      # Prob 50%                      #4242.24
                                # LOE
..B25.38:                       # Preds ..B25.37
..LN4045:
        addl      $-8, %esp                                     #4247.22
..LN4047:
        movl      -36(%ebp), %eax                               #4247.41
        movl      %eax, (%esp)                                  #4247.41
..LN4049:
        movl      -28(%ebp), %eax                               #4247.46
        movl      %eax, 4(%esp)                                 #4247.46
..LN4051:
        call      prime_buffer_4na                              #4247.22
                                # LOE xmm0
..B25.49:                       # Preds ..B25.38
        addl      $8, %esp                                      #4247.22
        movdqa    %xmm0, -280(%ebp)                             #4247.22
                                # LOE
..B25.39:                       # Preds ..B25.49
..LN4053:
        movdqa    -280(%ebp), %xmm0                             #4247.13
        movdqa    %xmm0, -232(%ebp)                             #4247.13
..LN4055:
        addl      $8, -36(%ebp)                                 #4262.13
        jmp       ..B25.8       # Prob 100%                     #4262.13
                                # LOE
..B25.40:                       # Preds ..B25.37 ..B25.36 ..B25.8
..LN4057:
        xorl      %eax, %eax                                    #4273.12
        leave                                                   #4273.12
..___tag_value_eval_4na_16.800:                                 #
        movl      %ebx, %esp                                    #4273.12
        popl      %ebx                                          #4273.12
..___tag_value_eval_4na_16.801:                                 #
        ret                                                     #4273.12
        .align    2,0x90
..___tag_value_eval_4na_16.803:                                 #
                                # LOE
# mark_end;
	.type	eval_4na_16,@function
	.size	eval_4na_16,.-eval_4na_16
.LNeval_4na_16:
	.data
# -- End  eval_4na_16
	.section .rodata, "a"
	.align 4
__$Uc:
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
	.type	__$Uc,@object
	.size	__$Uc,12
	.text
# -- Begin  eval_4na_32
# mark_begin;
       .align    2,0x90
eval_4na_32:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B26.1:                        # Preds ..B26.0
..___tag_value_eval_4na_32.806:                                 #
..LN4059:
        pushl     %ebx                                          #4279.1
..___tag_value_eval_4na_32.811:                                 #
        movl      %esp, %ebx                                    #4279.1
..___tag_value_eval_4na_32.812:                                 #
        andl      $-16, %esp                                    #4279.1
        pushl     %ebp                                          #4279.1
        pushl     %ebp                                          #4279.1
        movl      4(%ebx), %ebp                                 #4279.1
        movl      %ebp, 4(%esp)                                 #4279.1
        movl      %esp, %ebp                                    #4279.1
..___tag_value_eval_4na_32.814:                                 #
        subl      $312, %esp                                    #4279.1
        movl      %ebx, -312(%ebp)                              #4279.1
        call      ..L816        # Prob 100%                     #4279.1
..L816:                                                         #
        popl      %eax                                          #4279.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L816], %eax     #4279.1
        movl      %eax, -12(%ebp)                               #4279.1
..LN4061:
        movl      8(%ebx), %eax                                 #4305.25
        movl      4(%eax), %eax                                 #4305.25
..LN4063:
        movl      %eax, -16(%ebp)                               #4305.23
..LN4065:
        movl      20(%ebx), %eax                                #4308.5
        movl      -16(%ebp), %edx                               #4308.5
        cmpl      %edx, %eax                                    #4308.5
        jae       ..B26.3       # Prob 50%                      #4308.5
                                # LOE
..B26.2:                        # Preds ..B26.1
        addl      $-16, %esp                                    #4308.5
        movl      -12(%ebp), %eax                               #4308.5
        lea       _2__STRING.9@GOTOFF(%eax), %eax               #4308.5
        movl      %eax, (%esp)                                  #4308.5
        movl      -12(%ebp), %eax                               #4308.5
        lea       _2__STRING.1@GOTOFF(%eax), %eax               #4308.5
        movl      %eax, 4(%esp)                                 #4308.5
        movl      $4308, 8(%esp)                                #4308.5
        movl      -12(%ebp), %eax                               #4308.5
        lea       __$Uc@GOTOFF(%eax), %eax                      #4308.5
        movl      %eax, 12(%esp)                                #4308.5
        movl      -12(%ebp), %eax                               #4308.5
        movl      %eax, %ebx                                    #4308.5
..___tag_value_eval_4na_32.817:                                 #4308.5
        call      __assert_fail@PLT                             #4308.5
        movl      -312(%ebp), %ebx                              #4308.5
..___tag_value_eval_4na_32.818:                                 #
                                # LOE
..B26.46:                       # Preds ..B26.2
        addl      $16, %esp                                     #4308.5
                                # LOE
..B26.3:                        # Preds ..B26.1
..LN4067:
        movl      16(%ebx), %eax                                #4312.12
..LN4069:
        addl      20(%ebx), %eax                                #4312.5
        movl      %eax, 20(%ebx)                                #4312.5
..LN4071:
        movl      16(%ebx), %eax                                #4315.42
..LN4073:
        shrl      $2, %eax                                      #4315.49
..LN4075:
        addl      12(%ebx), %eax                                #4315.30
..LN4077:
        movl      %eax, -52(%ebp)                               #4315.5
..LN4079:
        movl      -16(%ebp), %eax                               #4318.18
        negl      %eax                                          #4318.18
        addl      20(%ebx), %eax                                #4318.18
..LN4081:
        movl      %eax, -48(%ebp)                               #4318.5
..LN4083:
        movl      20(%ebx), %eax                                #4321.44
..LN4085:
        addl      $3, %eax                                      #4321.50
..LN4087:
        shrl      $2, %eax                                      #4321.57
..LN4089:
        addl      12(%ebx), %eax                                #4321.30
..LN4091:
        movl      %eax, -44(%ebp)                               #4321.5
..LN4093:
        addl      $-8, %esp                                     #4324.14
..LN4095:
        movl      -52(%ebp), %eax                               #4324.33
        movl      %eax, (%esp)                                  #4324.33
..LN4097:
        movl      -44(%ebp), %eax                               #4324.38
        movl      %eax, 4(%esp)                                 #4324.38
..LN4099:
        call      prime_buffer_4na                              #4324.14
                                # LOE xmm0
..B26.47:                       # Preds ..B26.3
        addl      $8, %esp                                      #4324.14
        movdqa    %xmm0, -104(%ebp)                             #4324.14
                                # LOE
..B26.4:                        # Preds ..B26.47
..LN4101:
        movdqa    -104(%ebp), %xmm0                             #4324.5
        movdqa    %xmm0, -248(%ebp)                             #4324.5
..LN4103:
        addl      $8, -52(%ebp)                                 #4325.5
..LN4105:
        movl      -52(%ebp), %eax                               #4327.9
..LN4107:
        movl      %eax, -40(%ebp)                               #4327.5
..LN4109:
        movl      8(%ebx), %eax                                 #4331.5
        movdqa    16(%eax), %xmm0                               #4331.5
        movdqa    %xmm0, -232(%ebp)                             #4331.5
        movl      8(%ebx), %eax                                 #4331.5
        movdqa    32(%eax), %xmm0                               #4331.5
        movdqa    %xmm0, -216(%ebp)                             #4331.5
        movl      8(%ebx), %eax                                 #4331.5
        movdqa    48(%eax), %xmm0                               #4331.5
        movdqa    %xmm0, -200(%ebp)                             #4331.5
        movl      8(%ebx), %eax                                 #4331.5
        movdqa    64(%eax), %xmm0                               #4331.5
        movdqa    %xmm0, -184(%ebp)                             #4331.5
        movl      8(%ebx), %eax                                 #4331.5
        movdqa    80(%eax), %xmm0                               #4331.5
        movdqa    %xmm0, -168(%ebp)                             #4331.5
        movl      8(%ebx), %eax                                 #4331.5
        movdqa    96(%eax), %xmm0                               #4331.5
        movdqa    %xmm0, -152(%ebp)                             #4331.5
        movl      8(%ebx), %eax                                 #4331.5
        movdqa    112(%eax), %xmm0                              #4331.5
        movdqa    %xmm0, -136(%ebp)                             #4331.5
        movl      8(%ebx), %eax                                 #4331.5
        movdqa    128(%eax), %xmm0                              #4331.5
        movdqa    %xmm0, -120(%ebp)                             #4331.5
..LN4111:
        xorl      %eax, %eax                                    #4334.15
        movl      %eax, -36(%ebp)                               #4334.15
..LN4113:
        movl      %eax, -32(%ebp)                               #4334.10
..LN4115:
        movl      %eax, -28(%ebp)                               #4334.5
..LN4117:
        movl      $2, -24(%ebp)                                 #4340.5
..LN4119:
        movl      16(%ebx), %eax                                #4345.14
..LN4121:
        andl      $3, %eax                                      #4345.20
        movl      %eax, -20(%ebp)                               #4345.20
        je        ..B26.12      # Prob 50%                      #4345.20
                                # LOE
..B26.5:                        # Preds ..B26.4
        movl      -20(%ebp), %eax                               #4345.20
        cmpl      $1, %eax                                      #4345.20
        je        ..B26.13      # Prob 50%                      #4345.20
                                # LOE
..B26.6:                        # Preds ..B26.5
        movl      -20(%ebp), %eax                               #4345.20
        cmpl      $2, %eax                                      #4345.20
        je        ..B26.14      # Prob 50%                      #4345.20
                                # LOE
..B26.7:                        # Preds ..B26.6
        movl      -20(%ebp), %eax                               #4345.20
        cmpl      $3, %eax                                      #4345.20
        je        ..B26.15      # Prob 50%                      #4345.20
                                # LOE
..B26.8:                        # Preds ..B26.42 ..B26.7
..LN4123:
        movl      $1, %eax                                      #4350.9
        testl     %eax, %eax                                    #4350.9
        je        ..B26.43      # Prob 100%                     #4350.9
                                # LOE
..B26.9:                        # Preds ..B26.8
..LN4125:
        movl      $2, -24(%ebp)                                 #4352.13
                                # LOE
..B26.10:                       # Preds ..B26.37 ..B26.9
..LN4127:
        movl      $1, %eax                                      #4355.13
        testl     %eax, %eax                                    #4355.13
        je        ..B26.39      # Prob 100%                     #4355.13
                                # LOE
..B26.12:                       # Preds ..B26.4 ..B26.10
..LN4129:
        movdqa    -248(%ebp), %xmm0                             #4362.38
..LN4131:
        movdqa    -232(%ebp), %xmm1                             #4362.46
..LN4133:
        pand      %xmm1, %xmm0                                  #4362.22
..LN4135:
        movdqa    %xmm0, -280(%ebp)                             #4362.17
..LN4137:
        movdqa    -248(%ebp), %xmm0                             #4363.38
..LN4139:
        movdqa    -216(%ebp), %xmm1                             #4363.46
..LN4141:
        pand      %xmm1, %xmm0                                  #4363.22
..LN4143:
        movdqa    %xmm0, -264(%ebp)                             #4363.17
..LN4145:
        movdqa    -280(%ebp), %xmm0                             #4364.22
        movdqa    -264(%ebp), %xmm1                             #4364.22
        pcmpeqd   %xmm1, %xmm0                                  #4364.22
..LN4147:
        movdqa    %xmm0, -280(%ebp)                             #4364.17
..LN4149:
        movdqa    -280(%ebp), %xmm0                             #4365.42
..LN4151:
        pmovmskb  %xmm0, %eax                                   #4365.22
..LN4153:
        movl      %eax, -28(%ebp)                               #4365.17
                                # LOE
..B26.13:                       # Preds ..B26.5 ..B26.12
..LN4155:
        movdqa    -248(%ebp), %xmm0                             #4369.38
..LN4157:
        movdqa    -200(%ebp), %xmm1                             #4369.46
..LN4159:
        pand      %xmm1, %xmm0                                  #4369.22
..LN4161:
        movdqa    %xmm0, -280(%ebp)                             #4369.17
..LN4163:
        movdqa    -248(%ebp), %xmm0                             #4370.38
..LN4165:
        movdqa    -184(%ebp), %xmm1                             #4370.46
..LN4167:
        pand      %xmm1, %xmm0                                  #4370.22
..LN4169:
        movdqa    %xmm0, -264(%ebp)                             #4370.17
..LN4171:
        movdqa    -280(%ebp), %xmm0                             #4371.22
        movdqa    -264(%ebp), %xmm1                             #4371.22
        pcmpeqd   %xmm1, %xmm0                                  #4371.22
..LN4173:
        movdqa    %xmm0, -280(%ebp)                             #4371.17
..LN4175:
        movdqa    -280(%ebp), %xmm0                             #4372.42
..LN4177:
        pmovmskb  %xmm0, %eax                                   #4372.22
..LN4179:
        movl      %eax, -32(%ebp)                               #4372.17
                                # LOE
..B26.14:                       # Preds ..B26.6 ..B26.13
..LN4181:
        movdqa    -248(%ebp), %xmm0                             #4376.38
..LN4183:
        movdqa    -168(%ebp), %xmm1                             #4376.46
..LN4185:
        pand      %xmm1, %xmm0                                  #4376.22
..LN4187:
        movdqa    %xmm0, -280(%ebp)                             #4376.17
..LN4189:
        movdqa    -248(%ebp), %xmm0                             #4377.38
..LN4191:
        movdqa    -152(%ebp), %xmm1                             #4377.46
..LN4193:
        pand      %xmm1, %xmm0                                  #4377.22
..LN4195:
        movdqa    %xmm0, -264(%ebp)                             #4377.17
..LN4197:
        movdqa    -280(%ebp), %xmm0                             #4378.22
        movdqa    -264(%ebp), %xmm1                             #4378.22
        pcmpeqd   %xmm1, %xmm0                                  #4378.22
..LN4199:
        movdqa    %xmm0, -280(%ebp)                             #4378.17
..LN4201:
        movdqa    -280(%ebp), %xmm0                             #4379.42
..LN4203:
        pmovmskb  %xmm0, %eax                                   #4379.22
..LN4205:
        movl      %eax, -36(%ebp)                               #4379.17
                                # LOE
..B26.15:                       # Preds ..B26.7 ..B26.14
..LN4207:
        movdqa    -248(%ebp), %xmm0                             #4383.38
..LN4209:
        movdqa    -136(%ebp), %xmm1                             #4383.46
..LN4211:
        pand      %xmm1, %xmm0                                  #4383.22
..LN4213:
        movdqa    %xmm0, -280(%ebp)                             #4383.17
..LN4215:
        movdqa    -248(%ebp), %xmm0                             #4384.38
..LN4217:
        movdqa    -120(%ebp), %xmm1                             #4384.46
..LN4219:
        pand      %xmm1, %xmm0                                  #4384.22
..LN4221:
        movdqa    %xmm0, -264(%ebp)                             #4384.17
..LN4223:
        movdqa    -280(%ebp), %xmm0                             #4385.22
        movdqa    -264(%ebp), %xmm1                             #4385.22
        pcmpeqd   %xmm1, %xmm0                                  #4385.22
..LN4225:
        movdqa    %xmm0, -280(%ebp)                             #4385.17
..LN4227:
        movdqa    -280(%ebp), %xmm0                             #4386.42
..LN4229:
        pmovmskb  %xmm0, %eax                                   #4386.22
..LN4231:
        movl      %eax, -56(%ebp)                               #4386.17
..LN4233:
        movl      16(%ebx), %eax                                #4391.17
        andl      $-4, %eax                                     #4391.17
        movl      %eax, 16(%ebx)                                #4391.17
..LN4235:
        movl      -32(%ebp), %eax                               #4394.29
        orl       -28(%ebp), %eax                               #4394.29
..LN4237:
        orl       -36(%ebp), %eax                               #4394.34
..LN4239:
        orl       -56(%ebp), %eax                               #4394.39
..LN4241:
        je        ..B26.32      # Prob 50%                      #4394.47
                                # LOE
..B26.16:                       # Preds ..B26.15
..LN4243:
        pushl     %edi                                          #4412.30
..LN4245:
        movl      -28(%ebp), %eax                               #4412.58
        movw      %ax, (%esp)                                   #4412.58
..LN4247:
        call      uint16_lsbit                                  #4412.30
                                # LOE eax
..B26.48:                       # Preds ..B26.16
        popl      %ecx                                          #4412.30
        movl      %eax, -72(%ebp)                               #4412.30
                                # LOE
..B26.17:                       # Preds ..B26.48
        movl      -72(%ebp), %eax                               #4412.30
        movswl    %ax, %eax                                     #4412.30
..LN4249:
        movl      %eax, -88(%ebp)                               #4412.28
..LN4251:
        pushl     %edi                                          #4413.30
..LN4253:
        movl      -32(%ebp), %eax                               #4413.58
        movw      %ax, (%esp)                                   #4413.58
..LN4255:
        call      uint16_lsbit                                  #4413.30
                                # LOE eax
..B26.49:                       # Preds ..B26.17
        popl      %ecx                                          #4413.30
        movl      %eax, -68(%ebp)                               #4413.30
                                # LOE
..B26.18:                       # Preds ..B26.49
        movl      -68(%ebp), %eax                               #4413.30
        movswl    %ax, %eax                                     #4413.30
..LN4257:
        movl      %eax, -84(%ebp)                               #4413.28
..LN4259:
        pushl     %edi                                          #4414.30
..LN4261:
        movl      -36(%ebp), %eax                               #4414.58
        movw      %ax, (%esp)                                   #4414.58
..LN4263:
        call      uint16_lsbit                                  #4414.30
                                # LOE eax
..B26.50:                       # Preds ..B26.18
        popl      %ecx                                          #4414.30
        movl      %eax, -64(%ebp)                               #4414.30
                                # LOE
..B26.19:                       # Preds ..B26.50
        movl      -64(%ebp), %eax                               #4414.30
        movswl    %ax, %eax                                     #4414.30
..LN4265:
        movl      %eax, -80(%ebp)                               #4414.28
..LN4267:
        pushl     %edi                                          #4415.30
..LN4269:
        movl      -56(%ebp), %eax                               #4415.58
        movw      %ax, (%esp)                                   #4415.58
..LN4271:
        call      uint16_lsbit                                  #4415.30
                                # LOE eax
..B26.51:                       # Preds ..B26.19
        popl      %ecx                                          #4415.30
        movl      %eax, -60(%ebp)                               #4415.30
                                # LOE
..B26.20:                       # Preds ..B26.51
        movl      -60(%ebp), %eax                               #4415.30
        movswl    %ax, %eax                                     #4415.30
..LN4273:
        movl      %eax, -76(%ebp)                               #4415.28
..LN4275:
        movl      -88(%ebp), %eax                               #4420.28
..LN4277:
        addl      %eax, %eax                                    #4420.34
..LN4279:
        movl      %eax, -88(%ebp)                               #4420.21
..LN4281:
        movl      -84(%ebp), %eax                               #4421.28
..LN4283:
        lea       1(%eax,%eax), %eax                            #4421.40
..LN4285:
        movl      %eax, -84(%ebp)                               #4421.21
..LN4287:
        movl      -80(%ebp), %eax                               #4422.28
..LN4289:
        lea       2(%eax,%eax), %eax                            #4422.40
..LN4291:
        movl      %eax, -80(%ebp)                               #4422.21
..LN4293:
        movl      -76(%ebp), %eax                               #4423.28
..LN4295:
        lea       3(%eax,%eax), %eax                            #4423.40
..LN4297:
        movl      %eax, -76(%ebp)                               #4423.21
..LN4299:
        movl      -28(%ebp), %eax                               #4427.26
..LN4301:
        testl     %eax, %eax                                    #4427.32
        je        ..B26.23      # Prob 50%                      #4427.32
                                # LOE
..B26.21:                       # Preds ..B26.20
..LN4303:
        movl      -88(%ebp), %eax                               #4427.43
        addl      16(%ebx), %eax                                #4427.43
..LN4305:
        movl      -48(%ebp), %edx                               #4427.49
        cmpl      %edx, %eax                                    #4427.49
        ja        ..B26.23      # Prob 50%                      #4427.49
                                # LOE
..B26.22:                       # Preds ..B26.21
..LN4307:
        movl      $1, %eax                                      #4427.63
        leave                                                   #4427.63
..___tag_value_eval_4na_32.820:                                 #
        movl      %ebx, %esp                                    #4427.63
        popl      %ebx                                          #4427.63
..___tag_value_eval_4na_32.821:                                 #
        ret                                                     #4427.63
..___tag_value_eval_4na_32.823:                                 #
                                # LOE
..B26.23:                       # Preds ..B26.21 ..B26.20
..LN4309:
        movl      -32(%ebp), %eax                               #4428.26
..LN4311:
        testl     %eax, %eax                                    #4428.32
        je        ..B26.26      # Prob 50%                      #4428.32
                                # LOE
..B26.24:                       # Preds ..B26.23
..LN4313:
        movl      -84(%ebp), %eax                               #4428.43
        addl      16(%ebx), %eax                                #4428.43
..LN4315:
        movl      -48(%ebp), %edx                               #4428.49
        cmpl      %edx, %eax                                    #4428.49
        ja        ..B26.26      # Prob 50%                      #4428.49
                                # LOE
..B26.25:                       # Preds ..B26.24
..LN4317:
        movl      $1, %eax                                      #4428.63
        leave                                                   #4428.63
..___tag_value_eval_4na_32.827:                                 #
        movl      %ebx, %esp                                    #4428.63
        popl      %ebx                                          #4428.63
..___tag_value_eval_4na_32.828:                                 #
        ret                                                     #4428.63
..___tag_value_eval_4na_32.830:                                 #
                                # LOE
..B26.26:                       # Preds ..B26.24 ..B26.23
..LN4319:
        movl      -36(%ebp), %eax                               #4429.26
..LN4321:
        testl     %eax, %eax                                    #4429.32
        je        ..B26.29      # Prob 50%                      #4429.32
                                # LOE
..B26.27:                       # Preds ..B26.26
..LN4323:
        movl      -80(%ebp), %eax                               #4429.43
        addl      16(%ebx), %eax                                #4429.43
..LN4325:
        movl      -48(%ebp), %edx                               #4429.49
        cmpl      %edx, %eax                                    #4429.49
        ja        ..B26.29      # Prob 50%                      #4429.49
                                # LOE
..B26.28:                       # Preds ..B26.27
..LN4327:
        movl      $1, %eax                                      #4429.63
        leave                                                   #4429.63
..___tag_value_eval_4na_32.834:                                 #
        movl      %ebx, %esp                                    #4429.63
        popl      %ebx                                          #4429.63
..___tag_value_eval_4na_32.835:                                 #
        ret                                                     #4429.63
..___tag_value_eval_4na_32.837:                                 #
                                # LOE
..B26.29:                       # Preds ..B26.27 ..B26.26
..LN4329:
        movl      -56(%ebp), %eax                               #4430.26
..LN4331:
        testl     %eax, %eax                                    #4430.32
        je        ..B26.32      # Prob 50%                      #4430.32
                                # LOE
..B26.30:                       # Preds ..B26.29
..LN4333:
        movl      -76(%ebp), %eax                               #4430.43
        addl      16(%ebx), %eax                                #4430.43
..LN4335:
        movl      -48(%ebp), %edx                               #4430.49
        cmpl      %edx, %eax                                    #4430.49
        ja        ..B26.32      # Prob 50%                      #4430.49
                                # LOE
..B26.31:                       # Preds ..B26.30
..LN4337:
        movl      $1, %eax                                      #4430.63
        leave                                                   #4430.63
..___tag_value_eval_4na_32.841:                                 #
        movl      %ebx, %esp                                    #4430.63
        popl      %ebx                                          #4430.63
..___tag_value_eval_4na_32.842:                                 #
        ret                                                     #4430.63
..___tag_value_eval_4na_32.844:                                 #
                                # LOE
..B26.32:                       # Preds ..B26.30 ..B26.29 ..B26.15
..LN4339:
        movl      16(%ebx), %eax                                #4435.17
        addl      $4, %eax                                      #4435.17
        movl      %eax, 16(%ebx)                                #4435.17
..LN4341:
        movl      16(%ebx), %eax                                #4438.22
..LN4343:
        movl      -48(%ebp), %edx                               #4438.28
        cmpl      %edx, %eax                                    #4438.28
        jbe       ..B26.34      # Prob 50%                      #4438.28
                                # LOE
..B26.33:                       # Preds ..B26.32
..LN4345:
        xorl      %eax, %eax                                    #4439.28
        leave                                                   #4439.28
..___tag_value_eval_4na_32.848:                                 #
        movl      %ebx, %esp                                    #4439.28
        popl      %ebx                                          #4439.28
..___tag_value_eval_4na_32.849:                                 #
        ret                                                     #4439.28
..___tag_value_eval_4na_32.851:                                 #
                                # LOE
..B26.34:                       # Preds ..B26.32
..LN4347:
        movl      -24(%ebp), %eax                               #4442.25
        decl      %eax                                          #4442.25
        movl      %eax, -24(%ebp)                               #4442.25
..LN4349:
        je        ..B26.39      # Prob 50%                      #4442.39
                                # LOE
..B26.35:                       # Preds ..B26.34
..LN4351:
        movdqa    -248(%ebp), %xmm0                             #4447.43
..LN4353:
        psrldq    $2, %xmm0                                     #4447.26
..LN4355:
        movdqa    %xmm0, -248(%ebp)                             #4447.17
..LN4357:
        movl      -40(%ebp), %eax                               #4450.22
..LN4359:
        movl      -44(%ebp), %edx                               #4450.26
        cmpl      %edx, %eax                                    #4450.26
        jae       ..B26.37      # Prob 50%                      #4450.26
                                # LOE
..B26.36:                       # Preds ..B26.35
..LN4361:
        movdqa    -248(%ebp), %xmm0                             #4451.49
..LN4363:
        movl      -40(%ebp), %eax                               #4451.72
        movzbl    (%eax), %eax                                  #4451.72
        movzbl    %al, %eax                                     #4451.72
..LN4365:
        movl      -12(%ebp), %edx                               #4451.57
        movzwl    expand_2na@GOTOFF(%edx,%eax,2), %eax          #4451.57
        movzwl    %ax, %eax                                     #4451.57
..LN4367:
        pinsrw    $7, %eax, %xmm0                               #4451.30
..LN4369:
        movdqa    %xmm0, -248(%ebp)                             #4451.21
                                # LOE
..B26.37:                       # Preds ..B26.36 ..B26.35
..LN4371:
        incl      -40(%ebp)                                     #4454.20
        jmp       ..B26.10      # Prob 100%                     #4454.20
                                # LOE
..B26.39:                       # Preds ..B26.34 ..B26.10
..LN4373:
        movl      -52(%ebp), %eax                               #4459.18
..LN4375:
        movl      -44(%ebp), %edx                               #4459.25
        cmpl      %edx, %eax                                    #4459.25
        jae       ..B26.43      # Prob 50%                      #4459.25
                                # LOE
..B26.40:                       # Preds ..B26.39
..LN4377:
        movl      16(%ebx), %eax                                #4463.13
        addl      $24, %eax                                     #4463.13
        movl      %eax, 16(%ebx)                                #4463.13
..LN4379:
        movl      16(%ebx), %eax                                #4464.18
..LN4381:
        movl      -48(%ebp), %edx                               #4464.24
        cmpl      %edx, %eax                                    #4464.24
        ja        ..B26.43      # Prob 50%                      #4464.24
                                # LOE
..B26.41:                       # Preds ..B26.40
..LN4383:
        addl      $-8, %esp                                     #4469.22
..LN4385:
        movl      -52(%ebp), %eax                               #4469.41
        movl      %eax, (%esp)                                  #4469.41
..LN4387:
        movl      -44(%ebp), %eax                               #4469.46
        movl      %eax, 4(%esp)                                 #4469.46
..LN4389:
        call      prime_buffer_4na                              #4469.22
                                # LOE xmm0
..B26.52:                       # Preds ..B26.41
        addl      $8, %esp                                      #4469.22
        movdqa    %xmm0, -296(%ebp)                             #4469.22
                                # LOE
..B26.42:                       # Preds ..B26.52
..LN4391:
        movdqa    -296(%ebp), %xmm0                             #4469.13
        movdqa    %xmm0, -248(%ebp)                             #4469.13
..LN4393:
        addl      $8, -52(%ebp)                                 #4484.13
..LN4395:
        movl      -52(%ebp), %eax                               #4486.17
..LN4397:
        movl      %eax, -40(%ebp)                               #4486.13
        jmp       ..B26.8       # Prob 100%                     #4486.13
                                # LOE
..B26.43:                       # Preds ..B26.40 ..B26.39 ..B26.8
..LN4399:
        xorl      %eax, %eax                                    #4495.12
        leave                                                   #4495.12
..___tag_value_eval_4na_32.855:                                 #
        movl      %ebx, %esp                                    #4495.12
        popl      %ebx                                          #4495.12
..___tag_value_eval_4na_32.856:                                 #
        ret                                                     #4495.12
        .align    2,0x90
..___tag_value_eval_4na_32.858:                                 #
                                # LOE
# mark_end;
	.type	eval_4na_32,@function
	.size	eval_4na_32,.-eval_4na_32
.LNeval_4na_32:
	.data
# -- End  eval_4na_32
	.section .rodata, "a"
	.align 4
__$Ud:
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
	.type	__$Ud,@object
	.size	__$Ud,12
	.text
# -- Begin  eval_4na_64
# mark_begin;
       .align    2,0x90
eval_4na_64:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B27.1:                        # Preds ..B27.0
..___tag_value_eval_4na_64.861:                                 #
..LN4401:
        pushl     %ebx                                          #4501.1
..___tag_value_eval_4na_64.866:                                 #
        movl      %esp, %ebx                                    #4501.1
..___tag_value_eval_4na_64.867:                                 #
        andl      $-16, %esp                                    #4501.1
        pushl     %ebp                                          #4501.1
        pushl     %ebp                                          #4501.1
        movl      4(%ebx), %ebp                                 #4501.1
        movl      %ebp, 4(%esp)                                 #4501.1
        movl      %esp, %ebp                                    #4501.1
..___tag_value_eval_4na_64.869:                                 #
        subl      $312, %esp                                    #4501.1
        movl      %ebx, -312(%ebp)                              #4501.1
        call      ..L871        # Prob 100%                     #4501.1
..L871:                                                         #
        popl      %eax                                          #4501.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L871], %eax     #4501.1
        movl      %eax, -12(%ebp)                               #4501.1
..LN4403:
        movl      8(%ebx), %eax                                 #4528.25
        movl      4(%eax), %eax                                 #4528.25
..LN4405:
        movl      %eax, -16(%ebp)                               #4528.23
..LN4407:
        movl      20(%ebx), %eax                                #4531.5
        movl      -16(%ebp), %edx                               #4531.5
        cmpl      %edx, %eax                                    #4531.5
        jae       ..B27.3       # Prob 50%                      #4531.5
                                # LOE
..B27.2:                        # Preds ..B27.1
        addl      $-16, %esp                                    #4531.5
        movl      -12(%ebp), %eax                               #4531.5
        lea       _2__STRING.9@GOTOFF(%eax), %eax               #4531.5
        movl      %eax, (%esp)                                  #4531.5
        movl      -12(%ebp), %eax                               #4531.5
        lea       _2__STRING.1@GOTOFF(%eax), %eax               #4531.5
        movl      %eax, 4(%esp)                                 #4531.5
        movl      $4531, 8(%esp)                                #4531.5
        movl      -12(%ebp), %eax                               #4531.5
        lea       __$Ud@GOTOFF(%eax), %eax                      #4531.5
        movl      %eax, 12(%esp)                                #4531.5
        movl      -12(%ebp), %eax                               #4531.5
        movl      %eax, %ebx                                    #4531.5
..___tag_value_eval_4na_64.872:                                 #4531.5
        call      __assert_fail@PLT                             #4531.5
        movl      -312(%ebp), %ebx                              #4531.5
..___tag_value_eval_4na_64.873:                                 #
                                # LOE
..B27.46:                       # Preds ..B27.2
        addl      $16, %esp                                     #4531.5
                                # LOE
..B27.3:                        # Preds ..B27.1
..LN4409:
        movl      16(%ebx), %eax                                #4535.12
..LN4411:
        addl      20(%ebx), %eax                                #4535.5
        movl      %eax, 20(%ebx)                                #4535.5
..LN4413:
        movl      16(%ebx), %eax                                #4538.42
..LN4415:
        shrl      $2, %eax                                      #4538.49
..LN4417:
        addl      12(%ebx), %eax                                #4538.30
..LN4419:
        movl      %eax, -52(%ebp)                               #4538.5
..LN4421:
        movl      -16(%ebp), %eax                               #4541.18
        negl      %eax                                          #4541.18
        addl      20(%ebx), %eax                                #4541.18
..LN4423:
        movl      %eax, -48(%ebp)                               #4541.5
..LN4425:
        movl      20(%ebx), %eax                                #4544.44
..LN4427:
        addl      $3, %eax                                      #4544.50
..LN4429:
        shrl      $2, %eax                                      #4544.57
..LN4431:
        addl      12(%ebx), %eax                                #4544.30
..LN4433:
        movl      %eax, -44(%ebp)                               #4544.5
..LN4435:
        addl      $-8, %esp                                     #4547.14
..LN4437:
        movl      -52(%ebp), %eax                               #4547.33
        movl      %eax, (%esp)                                  #4547.33
..LN4439:
        movl      -44(%ebp), %eax                               #4547.38
        movl      %eax, 4(%esp)                                 #4547.38
..LN4441:
        call      prime_buffer_4na                              #4547.14
                                # LOE xmm0
..B27.47:                       # Preds ..B27.3
        addl      $8, %esp                                      #4547.14
        movdqa    %xmm0, -104(%ebp)                             #4547.14
                                # LOE
..B27.4:                        # Preds ..B27.47
..LN4443:
        movdqa    -104(%ebp), %xmm0                             #4547.5
        movdqa    %xmm0, -248(%ebp)                             #4547.5
..LN4445:
        addl      $8, -52(%ebp)                                 #4548.5
..LN4447:
        movl      -52(%ebp), %eax                               #4550.9
..LN4449:
        movl      %eax, -40(%ebp)                               #4550.5
..LN4451:
        movl      8(%ebx), %eax                                 #4554.5
        movdqa    16(%eax), %xmm0                               #4554.5
        movdqa    %xmm0, -232(%ebp)                             #4554.5
        movl      8(%ebx), %eax                                 #4554.5
        movdqa    32(%eax), %xmm0                               #4554.5
        movdqa    %xmm0, -216(%ebp)                             #4554.5
        movl      8(%ebx), %eax                                 #4554.5
        movdqa    48(%eax), %xmm0                               #4554.5
        movdqa    %xmm0, -200(%ebp)                             #4554.5
        movl      8(%ebx), %eax                                 #4554.5
        movdqa    64(%eax), %xmm0                               #4554.5
        movdqa    %xmm0, -184(%ebp)                             #4554.5
        movl      8(%ebx), %eax                                 #4554.5
        movdqa    80(%eax), %xmm0                               #4554.5
        movdqa    %xmm0, -168(%ebp)                             #4554.5
        movl      8(%ebx), %eax                                 #4554.5
        movdqa    96(%eax), %xmm0                               #4554.5
        movdqa    %xmm0, -152(%ebp)                             #4554.5
        movl      8(%ebx), %eax                                 #4554.5
        movdqa    112(%eax), %xmm0                              #4554.5
        movdqa    %xmm0, -136(%ebp)                             #4554.5
        movl      8(%ebx), %eax                                 #4554.5
        movdqa    128(%eax), %xmm0                              #4554.5
        movdqa    %xmm0, -120(%ebp)                             #4554.5
..LN4453:
        xorl      %eax, %eax                                    #4557.15
        movl      %eax, -36(%ebp)                               #4557.15
..LN4455:
        movl      %eax, -32(%ebp)                               #4557.10
..LN4457:
        movl      %eax, -28(%ebp)                               #4557.5
..LN4459:
        movl      $4, -24(%ebp)                                 #4563.5
..LN4461:
        movl      16(%ebx), %eax                                #4568.14
..LN4463:
        andl      $3, %eax                                      #4568.20
        movl      %eax, -20(%ebp)                               #4568.20
        je        ..B27.12      # Prob 50%                      #4568.20
                                # LOE
..B27.5:                        # Preds ..B27.4
        movl      -20(%ebp), %eax                               #4568.20
        cmpl      $1, %eax                                      #4568.20
        je        ..B27.13      # Prob 50%                      #4568.20
                                # LOE
..B27.6:                        # Preds ..B27.5
        movl      -20(%ebp), %eax                               #4568.20
        cmpl      $2, %eax                                      #4568.20
        je        ..B27.14      # Prob 50%                      #4568.20
                                # LOE
..B27.7:                        # Preds ..B27.6
        movl      -20(%ebp), %eax                               #4568.20
        cmpl      $3, %eax                                      #4568.20
        je        ..B27.15      # Prob 50%                      #4568.20
                                # LOE
..B27.8:                        # Preds ..B27.42 ..B27.7
..LN4465:
        movl      $1, %eax                                      #4573.9
        testl     %eax, %eax                                    #4573.9
        je        ..B27.43      # Prob 100%                     #4573.9
                                # LOE
..B27.9:                        # Preds ..B27.8
..LN4467:
        movl      $4, -24(%ebp)                                 #4575.13
                                # LOE
..B27.10:                       # Preds ..B27.37 ..B27.9
..LN4469:
        movl      $1, %eax                                      #4578.13
        testl     %eax, %eax                                    #4578.13
        je        ..B27.39      # Prob 100%                     #4578.13
                                # LOE
..B27.12:                       # Preds ..B27.4 ..B27.10
..LN4471:
        movdqa    -248(%ebp), %xmm0                             #4585.38
..LN4473:
        movdqa    -232(%ebp), %xmm1                             #4585.46
..LN4475:
        pand      %xmm1, %xmm0                                  #4585.22
..LN4477:
        movdqa    %xmm0, -280(%ebp)                             #4585.17
..LN4479:
        movdqa    -248(%ebp), %xmm0                             #4586.38
..LN4481:
        movdqa    -216(%ebp), %xmm1                             #4586.46
..LN4483:
        pand      %xmm1, %xmm0                                  #4586.22
..LN4485:
        movdqa    %xmm0, -264(%ebp)                             #4586.17
..LN4487:
        movdqa    -280(%ebp), %xmm0                             #4587.22
        movdqa    -264(%ebp), %xmm1                             #4587.22
        pcmpeqd   %xmm1, %xmm0                                  #4587.22
..LN4489:
        movdqa    %xmm0, -280(%ebp)                             #4587.17
..LN4491:
        movdqa    -280(%ebp), %xmm0                             #4588.42
..LN4493:
        pmovmskb  %xmm0, %eax                                   #4588.22
..LN4495:
        movl      %eax, -28(%ebp)                               #4588.17
..LN4497:
        movl      -28(%ebp), %eax                               #4589.17
        andl      $3855, %eax                                   #4589.17
        shll      $4, %eax                                      #4589.17
        andl      -28(%ebp), %eax                               #4589.17
        movl      %eax, -28(%ebp)                               #4589.17
        movl      -28(%ebp), %eax                               #4589.17
        sarl      $4, %eax                                      #4589.17
        orl       -28(%ebp), %eax                               #4589.17
        movl      %eax, -28(%ebp)                               #4589.17
                                # LOE
..B27.13:                       # Preds ..B27.5 ..B27.12
..LN4499:
        movdqa    -248(%ebp), %xmm0                             #4592.38
..LN4501:
        movdqa    -200(%ebp), %xmm1                             #4592.46
..LN4503:
        pand      %xmm1, %xmm0                                  #4592.22
..LN4505:
        movdqa    %xmm0, -280(%ebp)                             #4592.17
..LN4507:
        movdqa    -248(%ebp), %xmm0                             #4593.38
..LN4509:
        movdqa    -184(%ebp), %xmm1                             #4593.46
..LN4511:
        pand      %xmm1, %xmm0                                  #4593.22
..LN4513:
        movdqa    %xmm0, -264(%ebp)                             #4593.17
..LN4515:
        movdqa    -280(%ebp), %xmm0                             #4594.22
        movdqa    -264(%ebp), %xmm1                             #4594.22
        pcmpeqd   %xmm1, %xmm0                                  #4594.22
..LN4517:
        movdqa    %xmm0, -280(%ebp)                             #4594.17
..LN4519:
        movdqa    -280(%ebp), %xmm0                             #4595.42
..LN4521:
        pmovmskb  %xmm0, %eax                                   #4595.22
..LN4523:
        movl      %eax, -32(%ebp)                               #4595.17
..LN4525:
        movl      -32(%ebp), %eax                               #4596.17
        andl      $3855, %eax                                   #4596.17
        shll      $4, %eax                                      #4596.17
        andl      -32(%ebp), %eax                               #4596.17
        movl      %eax, -32(%ebp)                               #4596.17
        movl      -32(%ebp), %eax                               #4596.17
        sarl      $4, %eax                                      #4596.17
        orl       -32(%ebp), %eax                               #4596.17
        movl      %eax, -32(%ebp)                               #4596.17
                                # LOE
..B27.14:                       # Preds ..B27.6 ..B27.13
..LN4527:
        movdqa    -248(%ebp), %xmm0                             #4599.38
..LN4529:
        movdqa    -168(%ebp), %xmm1                             #4599.46
..LN4531:
        pand      %xmm1, %xmm0                                  #4599.22
..LN4533:
        movdqa    %xmm0, -280(%ebp)                             #4599.17
..LN4535:
        movdqa    -248(%ebp), %xmm0                             #4600.38
..LN4537:
        movdqa    -152(%ebp), %xmm1                             #4600.46
..LN4539:
        pand      %xmm1, %xmm0                                  #4600.22
..LN4541:
        movdqa    %xmm0, -264(%ebp)                             #4600.17
..LN4543:
        movdqa    -280(%ebp), %xmm0                             #4601.22
        movdqa    -264(%ebp), %xmm1                             #4601.22
        pcmpeqd   %xmm1, %xmm0                                  #4601.22
..LN4545:
        movdqa    %xmm0, -280(%ebp)                             #4601.17
..LN4547:
        movdqa    -280(%ebp), %xmm0                             #4602.42
..LN4549:
        pmovmskb  %xmm0, %eax                                   #4602.22
..LN4551:
        movl      %eax, -36(%ebp)                               #4602.17
..LN4553:
        movl      -36(%ebp), %eax                               #4603.17
        andl      $3855, %eax                                   #4603.17
        shll      $4, %eax                                      #4603.17
        andl      -36(%ebp), %eax                               #4603.17
        movl      %eax, -36(%ebp)                               #4603.17
        movl      -36(%ebp), %eax                               #4603.17
        sarl      $4, %eax                                      #4603.17
        orl       -36(%ebp), %eax                               #4603.17
        movl      %eax, -36(%ebp)                               #4603.17
                                # LOE
..B27.15:                       # Preds ..B27.7 ..B27.14
..LN4555:
        movdqa    -248(%ebp), %xmm0                             #4606.38
..LN4557:
        movdqa    -136(%ebp), %xmm1                             #4606.46
..LN4559:
        pand      %xmm1, %xmm0                                  #4606.22
..LN4561:
        movdqa    %xmm0, -280(%ebp)                             #4606.17
..LN4563:
        movdqa    -248(%ebp), %xmm0                             #4607.38
..LN4565:
        movdqa    -120(%ebp), %xmm1                             #4607.46
..LN4567:
        pand      %xmm1, %xmm0                                  #4607.22
..LN4569:
        movdqa    %xmm0, -264(%ebp)                             #4607.17
..LN4571:
        movdqa    -280(%ebp), %xmm0                             #4608.22
        movdqa    -264(%ebp), %xmm1                             #4608.22
        pcmpeqd   %xmm1, %xmm0                                  #4608.22
..LN4573:
        movdqa    %xmm0, -280(%ebp)                             #4608.17
..LN4575:
        movdqa    -280(%ebp), %xmm0                             #4609.42
..LN4577:
        pmovmskb  %xmm0, %eax                                   #4609.22
..LN4579:
        movl      %eax, -56(%ebp)                               #4609.17
..LN4581:
        movl      -56(%ebp), %eax                               #4610.17
        andl      $3855, %eax                                   #4610.17
        shll      $4, %eax                                      #4610.17
        andl      -56(%ebp), %eax                               #4610.17
        movl      %eax, -56(%ebp)                               #4610.17
        movl      -56(%ebp), %eax                               #4610.17
        sarl      $4, %eax                                      #4610.17
        orl       -56(%ebp), %eax                               #4610.17
        movl      %eax, -56(%ebp)                               #4610.17
..LN4583:
        movl      16(%ebx), %eax                                #4614.17
        andl      $-4, %eax                                     #4614.17
        movl      %eax, 16(%ebx)                                #4614.17
..LN4585:
        movl      -32(%ebp), %eax                               #4617.29
        orl       -28(%ebp), %eax                               #4617.29
..LN4587:
        orl       -36(%ebp), %eax                               #4617.34
..LN4589:
        orl       -56(%ebp), %eax                               #4617.39
..LN4591:
        je        ..B27.32      # Prob 50%                      #4617.47
                                # LOE
..B27.16:                       # Preds ..B27.15
..LN4593:
        pushl     %edi                                          #4635.30
..LN4595:
        movl      -28(%ebp), %eax                               #4635.58
        movw      %ax, (%esp)                                   #4635.58
..LN4597:
        call      uint16_lsbit                                  #4635.30
                                # LOE eax
..B27.48:                       # Preds ..B27.16
        popl      %ecx                                          #4635.30
        movl      %eax, -72(%ebp)                               #4635.30
                                # LOE
..B27.17:                       # Preds ..B27.48
        movl      -72(%ebp), %eax                               #4635.30
        movswl    %ax, %eax                                     #4635.30
..LN4599:
        movl      %eax, -88(%ebp)                               #4635.28
..LN4601:
        pushl     %edi                                          #4636.30
..LN4603:
        movl      -32(%ebp), %eax                               #4636.58
        movw      %ax, (%esp)                                   #4636.58
..LN4605:
        call      uint16_lsbit                                  #4636.30
                                # LOE eax
..B27.49:                       # Preds ..B27.17
        popl      %ecx                                          #4636.30
        movl      %eax, -68(%ebp)                               #4636.30
                                # LOE
..B27.18:                       # Preds ..B27.49
        movl      -68(%ebp), %eax                               #4636.30
        movswl    %ax, %eax                                     #4636.30
..LN4607:
        movl      %eax, -84(%ebp)                               #4636.28
..LN4609:
        pushl     %edi                                          #4637.30
..LN4611:
        movl      -36(%ebp), %eax                               #4637.58
        movw      %ax, (%esp)                                   #4637.58
..LN4613:
        call      uint16_lsbit                                  #4637.30
                                # LOE eax
..B27.50:                       # Preds ..B27.18
        popl      %ecx                                          #4637.30
        movl      %eax, -64(%ebp)                               #4637.30
                                # LOE
..B27.19:                       # Preds ..B27.50
        movl      -64(%ebp), %eax                               #4637.30
        movswl    %ax, %eax                                     #4637.30
..LN4615:
        movl      %eax, -80(%ebp)                               #4637.28
..LN4617:
        pushl     %edi                                          #4638.30
..LN4619:
        movl      -56(%ebp), %eax                               #4638.58
        movw      %ax, (%esp)                                   #4638.58
..LN4621:
        call      uint16_lsbit                                  #4638.30
                                # LOE eax
..B27.51:                       # Preds ..B27.19
        popl      %ecx                                          #4638.30
        movl      %eax, -60(%ebp)                               #4638.30
                                # LOE
..B27.20:                       # Preds ..B27.51
        movl      -60(%ebp), %eax                               #4638.30
        movswl    %ax, %eax                                     #4638.30
..LN4623:
        movl      %eax, -76(%ebp)                               #4638.28
..LN4625:
        movl      -88(%ebp), %eax                               #4643.28
..LN4627:
        addl      %eax, %eax                                    #4643.34
..LN4629:
        movl      %eax, -88(%ebp)                               #4643.21
..LN4631:
        movl      -84(%ebp), %eax                               #4644.28
..LN4633:
        lea       1(%eax,%eax), %eax                            #4644.40
..LN4635:
        movl      %eax, -84(%ebp)                               #4644.21
..LN4637:
        movl      -80(%ebp), %eax                               #4645.28
..LN4639:
        lea       2(%eax,%eax), %eax                            #4645.40
..LN4641:
        movl      %eax, -80(%ebp)                               #4645.21
..LN4643:
        movl      -76(%ebp), %eax                               #4646.28
..LN4645:
        lea       3(%eax,%eax), %eax                            #4646.40
..LN4647:
        movl      %eax, -76(%ebp)                               #4646.21
..LN4649:
        movl      -28(%ebp), %eax                               #4650.26
..LN4651:
        testl     %eax, %eax                                    #4650.32
        je        ..B27.23      # Prob 50%                      #4650.32
                                # LOE
..B27.21:                       # Preds ..B27.20
..LN4653:
        movl      -88(%ebp), %eax                               #4650.43
        addl      16(%ebx), %eax                                #4650.43
..LN4655:
        movl      -48(%ebp), %edx                               #4650.49
        cmpl      %edx, %eax                                    #4650.49
        ja        ..B27.23      # Prob 50%                      #4650.49
                                # LOE
..B27.22:                       # Preds ..B27.21
..LN4657:
        movl      $1, %eax                                      #4650.63
        leave                                                   #4650.63
..___tag_value_eval_4na_64.875:                                 #
        movl      %ebx, %esp                                    #4650.63
        popl      %ebx                                          #4650.63
..___tag_value_eval_4na_64.876:                                 #
        ret                                                     #4650.63
..___tag_value_eval_4na_64.878:                                 #
                                # LOE
..B27.23:                       # Preds ..B27.21 ..B27.20
..LN4659:
        movl      -32(%ebp), %eax                               #4651.26
..LN4661:
        testl     %eax, %eax                                    #4651.32
        je        ..B27.26      # Prob 50%                      #4651.32
                                # LOE
..B27.24:                       # Preds ..B27.23
..LN4663:
        movl      -84(%ebp), %eax                               #4651.43
        addl      16(%ebx), %eax                                #4651.43
..LN4665:
        movl      -48(%ebp), %edx                               #4651.49
        cmpl      %edx, %eax                                    #4651.49
        ja        ..B27.26      # Prob 50%                      #4651.49
                                # LOE
..B27.25:                       # Preds ..B27.24
..LN4667:
        movl      $1, %eax                                      #4651.63
        leave                                                   #4651.63
..___tag_value_eval_4na_64.882:                                 #
        movl      %ebx, %esp                                    #4651.63
        popl      %ebx                                          #4651.63
..___tag_value_eval_4na_64.883:                                 #
        ret                                                     #4651.63
..___tag_value_eval_4na_64.885:                                 #
                                # LOE
..B27.26:                       # Preds ..B27.24 ..B27.23
..LN4669:
        movl      -36(%ebp), %eax                               #4652.26
..LN4671:
        testl     %eax, %eax                                    #4652.32
        je        ..B27.29      # Prob 50%                      #4652.32
                                # LOE
..B27.27:                       # Preds ..B27.26
..LN4673:
        movl      -80(%ebp), %eax                               #4652.43
        addl      16(%ebx), %eax                                #4652.43
..LN4675:
        movl      -48(%ebp), %edx                               #4652.49
        cmpl      %edx, %eax                                    #4652.49
        ja        ..B27.29      # Prob 50%                      #4652.49
                                # LOE
..B27.28:                       # Preds ..B27.27
..LN4677:
        movl      $1, %eax                                      #4652.63
        leave                                                   #4652.63
..___tag_value_eval_4na_64.889:                                 #
        movl      %ebx, %esp                                    #4652.63
        popl      %ebx                                          #4652.63
..___tag_value_eval_4na_64.890:                                 #
        ret                                                     #4652.63
..___tag_value_eval_4na_64.892:                                 #
                                # LOE
..B27.29:                       # Preds ..B27.27 ..B27.26
..LN4679:
        movl      -56(%ebp), %eax                               #4653.26
..LN4681:
        testl     %eax, %eax                                    #4653.32
        je        ..B27.32      # Prob 50%                      #4653.32
                                # LOE
..B27.30:                       # Preds ..B27.29
..LN4683:
        movl      -76(%ebp), %eax                               #4653.43
        addl      16(%ebx), %eax                                #4653.43
..LN4685:
        movl      -48(%ebp), %edx                               #4653.49
        cmpl      %edx, %eax                                    #4653.49
        ja        ..B27.32      # Prob 50%                      #4653.49
                                # LOE
..B27.31:                       # Preds ..B27.30
..LN4687:
        movl      $1, %eax                                      #4653.63
        leave                                                   #4653.63
..___tag_value_eval_4na_64.896:                                 #
        movl      %ebx, %esp                                    #4653.63
        popl      %ebx                                          #4653.63
..___tag_value_eval_4na_64.897:                                 #
        ret                                                     #4653.63
..___tag_value_eval_4na_64.899:                                 #
                                # LOE
..B27.32:                       # Preds ..B27.30 ..B27.29 ..B27.15
..LN4689:
        movl      16(%ebx), %eax                                #4658.17
        addl      $4, %eax                                      #4658.17
        movl      %eax, 16(%ebx)                                #4658.17
..LN4691:
        movl      16(%ebx), %eax                                #4661.22
..LN4693:
        movl      -48(%ebp), %edx                               #4661.28
        cmpl      %edx, %eax                                    #4661.28
        jbe       ..B27.34      # Prob 50%                      #4661.28
                                # LOE
..B27.33:                       # Preds ..B27.32
..LN4695:
        xorl      %eax, %eax                                    #4662.28
        leave                                                   #4662.28
..___tag_value_eval_4na_64.903:                                 #
        movl      %ebx, %esp                                    #4662.28
        popl      %ebx                                          #4662.28
..___tag_value_eval_4na_64.904:                                 #
        ret                                                     #4662.28
..___tag_value_eval_4na_64.906:                                 #
                                # LOE
..B27.34:                       # Preds ..B27.32
..LN4697:
        movl      -24(%ebp), %eax                               #4665.25
        decl      %eax                                          #4665.25
        movl      %eax, -24(%ebp)                               #4665.25
..LN4699:
        je        ..B27.39      # Prob 50%                      #4665.39
                                # LOE
..B27.35:                       # Preds ..B27.34
..LN4701:
        movdqa    -248(%ebp), %xmm0                             #4670.43
..LN4703:
        psrldq    $2, %xmm0                                     #4670.26
..LN4705:
        movdqa    %xmm0, -248(%ebp)                             #4670.17
..LN4707:
        movl      -40(%ebp), %eax                               #4673.22
..LN4709:
        movl      -44(%ebp), %edx                               #4673.26
        cmpl      %edx, %eax                                    #4673.26
        jae       ..B27.37      # Prob 50%                      #4673.26
                                # LOE
..B27.36:                       # Preds ..B27.35
..LN4711:
        movdqa    -248(%ebp), %xmm0                             #4674.49
..LN4713:
        movl      -40(%ebp), %eax                               #4674.72
        movzbl    (%eax), %eax                                  #4674.72
        movzbl    %al, %eax                                     #4674.72
..LN4715:
        movl      -12(%ebp), %edx                               #4674.57
        movzwl    expand_2na@GOTOFF(%edx,%eax,2), %eax          #4674.57
        movzwl    %ax, %eax                                     #4674.57
..LN4717:
        pinsrw    $7, %eax, %xmm0                               #4674.30
..LN4719:
        movdqa    %xmm0, -248(%ebp)                             #4674.21
                                # LOE
..B27.37:                       # Preds ..B27.36 ..B27.35
..LN4721:
        incl      -40(%ebp)                                     #4677.20
        jmp       ..B27.10      # Prob 100%                     #4677.20
                                # LOE
..B27.39:                       # Preds ..B27.34 ..B27.10
..LN4723:
        movl      -52(%ebp), %eax                               #4682.18
..LN4725:
        movl      -44(%ebp), %edx                               #4682.25
        cmpl      %edx, %eax                                    #4682.25
        jae       ..B27.43      # Prob 50%                      #4682.25
                                # LOE
..B27.40:                       # Preds ..B27.39
..LN4727:
        movl      16(%ebx), %eax                                #4686.13
        addl      $16, %eax                                     #4686.13
        movl      %eax, 16(%ebx)                                #4686.13
..LN4729:
        movl      16(%ebx), %eax                                #4687.18
..LN4731:
        movl      -48(%ebp), %edx                               #4687.24
        cmpl      %edx, %eax                                    #4687.24
        ja        ..B27.43      # Prob 50%                      #4687.24
                                # LOE
..B27.41:                       # Preds ..B27.40
..LN4733:
        addl      $-8, %esp                                     #4692.22
..LN4735:
        movl      -52(%ebp), %eax                               #4692.41
        movl      %eax, (%esp)                                  #4692.41
..LN4737:
        movl      -44(%ebp), %eax                               #4692.46
        movl      %eax, 4(%esp)                                 #4692.46
..LN4739:
        call      prime_buffer_4na                              #4692.22
                                # LOE xmm0
..B27.52:                       # Preds ..B27.41
        addl      $8, %esp                                      #4692.22
        movdqa    %xmm0, -296(%ebp)                             #4692.22
                                # LOE
..B27.42:                       # Preds ..B27.52
..LN4741:
        movdqa    -296(%ebp), %xmm0                             #4692.13
        movdqa    %xmm0, -248(%ebp)                             #4692.13
..LN4743:
        addl      $8, -52(%ebp)                                 #4707.13
..LN4745:
        movl      -52(%ebp), %eax                               #4709.17
..LN4747:
        movl      %eax, -40(%ebp)                               #4709.13
        jmp       ..B27.8       # Prob 100%                     #4709.13
                                # LOE
..B27.43:                       # Preds ..B27.40 ..B27.39 ..B27.8
..LN4749:
        xorl      %eax, %eax                                    #4718.12
        leave                                                   #4718.12
..___tag_value_eval_4na_64.910:                                 #
        movl      %ebx, %esp                                    #4718.12
        popl      %ebx                                          #4718.12
..___tag_value_eval_4na_64.911:                                 #
        ret                                                     #4718.12
        .align    2,0x90
..___tag_value_eval_4na_64.913:                                 #
                                # LOE
# mark_end;
	.type	eval_4na_64,@function
	.size	eval_4na_64,.-eval_4na_64
.LNeval_4na_64:
	.data
# -- End  eval_4na_64
	.section .rodata, "a"
	.align 4
__$Ue:
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
	.type	__$Ue,@object
	.size	__$Ue,13
	.text
# -- Begin  eval_4na_128
# mark_begin;
       .align    2,0x90
eval_4na_128:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B28.1:                        # Preds ..B28.0
..___tag_value_eval_4na_128.916:                                #
..LN4751:
        pushl     %ebx                                          #4724.1
..___tag_value_eval_4na_128.921:                                #
        movl      %esp, %ebx                                    #4724.1
..___tag_value_eval_4na_128.922:                                #
        andl      $-16, %esp                                    #4724.1
        pushl     %ebp                                          #4724.1
        pushl     %ebp                                          #4724.1
        movl      4(%ebx), %ebp                                 #4724.1
        movl      %ebp, 4(%esp)                                 #4724.1
        movl      %esp, %ebp                                    #4724.1
..___tag_value_eval_4na_128.924:                                #
        subl      $280, %esp                                    #4724.1
        movl      %ebx, -280(%ebp)                              #4724.1
        call      ..L926        # Prob 100%                     #4724.1
..L926:                                                         #
        popl      %eax                                          #4724.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L926], %eax     #4724.1
        movl      %eax, -8(%ebp)                                #4724.1
..LN4753:
        movl      8(%ebx), %eax                                 #4750.25
        movl      4(%eax), %eax                                 #4750.25
..LN4755:
        movl      %eax, -12(%ebp)                               #4750.23
..LN4757:
        movl      20(%ebx), %eax                                #4753.5
        movl      -12(%ebp), %edx                               #4753.5
        cmpl      %edx, %eax                                    #4753.5
        jae       ..B28.3       # Prob 50%                      #4753.5
                                # LOE
..B28.2:                        # Preds ..B28.1
        addl      $-16, %esp                                    #4753.5
        movl      -8(%ebp), %eax                                #4753.5
        lea       _2__STRING.9@GOTOFF(%eax), %eax               #4753.5
        movl      %eax, (%esp)                                  #4753.5
        movl      -8(%ebp), %eax                                #4753.5
        lea       _2__STRING.1@GOTOFF(%eax), %eax               #4753.5
        movl      %eax, 4(%esp)                                 #4753.5
        movl      $4753, 8(%esp)                                #4753.5
        movl      -8(%ebp), %eax                                #4753.5
        lea       __$Ue@GOTOFF(%eax), %eax                      #4753.5
        movl      %eax, 12(%esp)                                #4753.5
        movl      -8(%ebp), %eax                                #4753.5
        movl      %eax, %ebx                                    #4753.5
..___tag_value_eval_4na_128.927:                                #4753.5
        call      __assert_fail@PLT                             #4753.5
        movl      -280(%ebp), %ebx                              #4753.5
..___tag_value_eval_4na_128.928:                                #
                                # LOE
..B28.41:                       # Preds ..B28.2
        addl      $16, %esp                                     #4753.5
                                # LOE
..B28.3:                        # Preds ..B28.1
..LN4759:
        movl      16(%ebx), %eax                                #4757.12
..LN4761:
        addl      20(%ebx), %eax                                #4757.5
        movl      %eax, 20(%ebx)                                #4757.5
..LN4763:
        movl      16(%ebx), %eax                                #4760.42
..LN4765:
        shrl      $2, %eax                                      #4760.49
..LN4767:
        addl      12(%ebx), %eax                                #4760.30
..LN4769:
        movl      %eax, -48(%ebp)                               #4760.5
..LN4771:
        movl      -12(%ebp), %eax                               #4763.18
        negl      %eax                                          #4763.18
        addl      20(%ebx), %eax                                #4763.18
..LN4773:
        movl      %eax, -44(%ebp)                               #4763.5
..LN4775:
        movl      20(%ebx), %eax                                #4766.44
..LN4777:
        addl      $3, %eax                                      #4766.50
..LN4779:
        shrl      $2, %eax                                      #4766.57
..LN4781:
        addl      12(%ebx), %eax                                #4766.30
..LN4783:
        movl      %eax, -40(%ebp)                               #4766.5
..LN4785:
        addl      $-8, %esp                                     #4769.14
..LN4787:
        movl      -48(%ebp), %eax                               #4769.33
        movl      %eax, (%esp)                                  #4769.33
..LN4789:
        movl      -40(%ebp), %eax                               #4769.38
        movl      %eax, 4(%esp)                                 #4769.38
..LN4791:
        call      prime_buffer_4na                              #4769.14
                                # LOE xmm0
..B28.42:                       # Preds ..B28.3
        addl      $8, %esp                                      #4769.14
        movdqa    %xmm0, -72(%ebp)                              #4769.14
                                # LOE
..B28.4:                        # Preds ..B28.42
..LN4793:
        movdqa    -72(%ebp), %xmm0                              #4769.5
        movdqa    %xmm0, -216(%ebp)                             #4769.5
..LN4795:
        addl      $8, -48(%ebp)                                 #4770.5
..LN4797:
        movl      -48(%ebp), %eax                               #4772.9
..LN4799:
        movl      %eax, -36(%ebp)                               #4772.5
..LN4801:
        movl      8(%ebx), %eax                                 #4776.5
        movdqa    16(%eax), %xmm0                               #4776.5
        movdqa    %xmm0, -200(%ebp)                             #4776.5
        movl      8(%ebx), %eax                                 #4776.5
        movdqa    32(%eax), %xmm0                               #4776.5
        movdqa    %xmm0, -184(%ebp)                             #4776.5
        movl      8(%ebx), %eax                                 #4776.5
        movdqa    48(%eax), %xmm0                               #4776.5
        movdqa    %xmm0, -168(%ebp)                             #4776.5
        movl      8(%ebx), %eax                                 #4776.5
        movdqa    64(%eax), %xmm0                               #4776.5
        movdqa    %xmm0, -152(%ebp)                             #4776.5
        movl      8(%ebx), %eax                                 #4776.5
        movdqa    80(%eax), %xmm0                               #4776.5
        movdqa    %xmm0, -136(%ebp)                             #4776.5
        movl      8(%ebx), %eax                                 #4776.5
        movdqa    96(%eax), %xmm0                               #4776.5
        movdqa    %xmm0, -120(%ebp)                             #4776.5
        movl      8(%ebx), %eax                                 #4776.5
        movdqa    112(%eax), %xmm0                              #4776.5
        movdqa    %xmm0, -104(%ebp)                             #4776.5
        movl      8(%ebx), %eax                                 #4776.5
        movdqa    128(%eax), %xmm0                              #4776.5
        movdqa    %xmm0, -88(%ebp)                              #4776.5
..LN4803:
        xorl      %eax, %eax                                    #4779.15
        movl      %eax, -32(%ebp)                               #4779.15
..LN4805:
        movl      %eax, -28(%ebp)                               #4779.10
..LN4807:
        movl      %eax, -24(%ebp)                               #4779.5
..LN4809:
        movl      16(%ebx), %eax                                #4781.27
        negl      %eax                                          #4781.27
..LN4811:
        movl      -44(%ebp), %edx                               #4781.20
..LN4813:
        lea       7(%edx,%eax), %eax                            #4781.33
..LN4815:
        shrl      $2, %eax                                      #4781.40
..LN4817:
        movl      %eax, -20(%ebp)                               #4781.5
..LN4819:
        movl      16(%ebx), %eax                                #4790.14
..LN4821:
        andl      $3, %eax                                      #4790.20
        movl      %eax, -16(%ebp)                               #4790.20
        je        ..B28.12      # Prob 50%                      #4790.20
                                # LOE
..B28.5:                        # Preds ..B28.4
        movl      -16(%ebp), %eax                               #4790.20
        cmpl      $1, %eax                                      #4790.20
        je        ..B28.13      # Prob 50%                      #4790.20
                                # LOE
..B28.6:                        # Preds ..B28.5
        movl      -16(%ebp), %eax                               #4790.20
        cmpl      $2, %eax                                      #4790.20
        je        ..B28.14      # Prob 50%                      #4790.20
                                # LOE
..B28.7:                        # Preds ..B28.6
        movl      -16(%ebp), %eax                               #4790.20
        cmpl      $3, %eax                                      #4790.20
        je        ..B28.15      # Prob 50%                      #4790.20
                                # LOE
..B28.8:                        # Preds ..B28.37 ..B28.7
..LN4823:
        movl      $1, %eax                                      #4795.9
        testl     %eax, %eax                                    #4795.9
        je        ..B28.38      # Prob 100%                     #4795.9
                                # LOE
..B28.9:                        # Preds ..B28.8
..LN4825:
        movl      $8, -20(%ebp)                                 #4797.13
                                # LOE
..B28.10:                       # Preds ..B28.32 ..B28.9
..LN4827:
        movl      $1, %eax                                      #4800.13
        testl     %eax, %eax                                    #4800.13
        je        ..B28.34      # Prob 100%                     #4800.13
                                # LOE
..B28.12:                       # Preds ..B28.4 ..B28.10
..LN4829:
        movdqa    -216(%ebp), %xmm0                             #4807.38
..LN4831:
        movdqa    -200(%ebp), %xmm1                             #4807.46
..LN4833:
        pand      %xmm1, %xmm0                                  #4807.22
..LN4835:
        movdqa    %xmm0, -248(%ebp)                             #4807.17
..LN4837:
        movdqa    -216(%ebp), %xmm0                             #4808.38
..LN4839:
        movdqa    -184(%ebp), %xmm1                             #4808.46
..LN4841:
        pand      %xmm1, %xmm0                                  #4808.22
..LN4843:
        movdqa    %xmm0, -232(%ebp)                             #4808.17
..LN4845:
        movdqa    -248(%ebp), %xmm0                             #4809.22
        movdqa    -232(%ebp), %xmm1                             #4809.22
        pcmpeqd   %xmm1, %xmm0                                  #4809.22
..LN4847:
        movdqa    %xmm0, -248(%ebp)                             #4809.17
..LN4849:
        movdqa    -248(%ebp), %xmm0                             #4810.42
..LN4851:
        pmovmskb  %xmm0, %eax                                   #4810.22
..LN4853:
        movl      %eax, -24(%ebp)                               #4810.17
..LN4855:
        movl      -24(%ebp), %eax                               #4811.17
        incl      %eax                                          #4811.17
        sarl      $16, %eax                                     #4811.17
        negl      %eax                                          #4811.17
        movl      %eax, -24(%ebp)                               #4811.17
                                # LOE
..B28.13:                       # Preds ..B28.5 ..B28.12
..LN4857:
        movdqa    -216(%ebp), %xmm0                             #4814.38
..LN4859:
        movdqa    -168(%ebp), %xmm1                             #4814.46
..LN4861:
        pand      %xmm1, %xmm0                                  #4814.22
..LN4863:
        movdqa    %xmm0, -248(%ebp)                             #4814.17
..LN4865:
        movdqa    -216(%ebp), %xmm0                             #4815.38
..LN4867:
        movdqa    -152(%ebp), %xmm1                             #4815.46
..LN4869:
        pand      %xmm1, %xmm0                                  #4815.22
..LN4871:
        movdqa    %xmm0, -232(%ebp)                             #4815.17
..LN4873:
        movdqa    -248(%ebp), %xmm0                             #4816.22
        movdqa    -232(%ebp), %xmm1                             #4816.22
        pcmpeqd   %xmm1, %xmm0                                  #4816.22
..LN4875:
        movdqa    %xmm0, -248(%ebp)                             #4816.17
..LN4877:
        movdqa    -248(%ebp), %xmm0                             #4817.42
..LN4879:
        pmovmskb  %xmm0, %eax                                   #4817.22
..LN4881:
        movl      %eax, -28(%ebp)                               #4817.17
..LN4883:
        movl      -28(%ebp), %eax                               #4818.17
        incl      %eax                                          #4818.17
        sarl      $16, %eax                                     #4818.17
        negl      %eax                                          #4818.17
        movl      %eax, -28(%ebp)                               #4818.17
                                # LOE
..B28.14:                       # Preds ..B28.6 ..B28.13
..LN4885:
        movdqa    -216(%ebp), %xmm0                             #4821.38
..LN4887:
        movdqa    -136(%ebp), %xmm1                             #4821.46
..LN4889:
        pand      %xmm1, %xmm0                                  #4821.22
..LN4891:
        movdqa    %xmm0, -248(%ebp)                             #4821.17
..LN4893:
        movdqa    -216(%ebp), %xmm0                             #4822.38
..LN4895:
        movdqa    -120(%ebp), %xmm1                             #4822.46
..LN4897:
        pand      %xmm1, %xmm0                                  #4822.22
..LN4899:
        movdqa    %xmm0, -232(%ebp)                             #4822.17
..LN4901:
        movdqa    -248(%ebp), %xmm0                             #4823.22
        movdqa    -232(%ebp), %xmm1                             #4823.22
        pcmpeqd   %xmm1, %xmm0                                  #4823.22
..LN4903:
        movdqa    %xmm0, -248(%ebp)                             #4823.17
..LN4905:
        movdqa    -248(%ebp), %xmm0                             #4824.42
..LN4907:
        pmovmskb  %xmm0, %eax                                   #4824.22
..LN4909:
        movl      %eax, -32(%ebp)                               #4824.17
..LN4911:
        movl      -32(%ebp), %eax                               #4825.17
        incl      %eax                                          #4825.17
        sarl      $16, %eax                                     #4825.17
        negl      %eax                                          #4825.17
        movl      %eax, -32(%ebp)                               #4825.17
                                # LOE
..B28.15:                       # Preds ..B28.7 ..B28.14
..LN4913:
        movdqa    -216(%ebp), %xmm0                             #4828.38
..LN4915:
        movdqa    -104(%ebp), %xmm1                             #4828.46
..LN4917:
        pand      %xmm1, %xmm0                                  #4828.22
..LN4919:
        movdqa    %xmm0, -248(%ebp)                             #4828.17
..LN4921:
        movdqa    -216(%ebp), %xmm0                             #4829.38
..LN4923:
        movdqa    -88(%ebp), %xmm1                              #4829.46
..LN4925:
        pand      %xmm1, %xmm0                                  #4829.22
..LN4927:
        movdqa    %xmm0, -232(%ebp)                             #4829.17
..LN4929:
        movdqa    -248(%ebp), %xmm0                             #4830.22
        movdqa    -232(%ebp), %xmm1                             #4830.22
        pcmpeqd   %xmm1, %xmm0                                  #4830.22
..LN4931:
        movdqa    %xmm0, -248(%ebp)                             #4830.17
..LN4933:
        movdqa    -248(%ebp), %xmm0                             #4831.42
..LN4935:
        pmovmskb  %xmm0, %eax                                   #4831.22
..LN4937:
        movl      %eax, -52(%ebp)                               #4831.17
..LN4939:
        movl      -52(%ebp), %eax                               #4832.17
        incl      %eax                                          #4832.17
        sarl      $16, %eax                                     #4832.17
        negl      %eax                                          #4832.17
        movl      %eax, -52(%ebp)                               #4832.17
..LN4941:
        movl      16(%ebx), %eax                                #4836.17
        andl      $-4, %eax                                     #4836.17
        movl      %eax, 16(%ebx)                                #4836.17
..LN4943:
        movl      -28(%ebp), %eax                               #4839.29
        orl       -24(%ebp), %eax                               #4839.29
..LN4945:
        orl       -32(%ebp), %eax                               #4839.34
..LN4947:
        orl       -52(%ebp), %eax                               #4839.39
..LN4949:
        je        ..B28.27      # Prob 50%                      #4839.47
                                # LOE
..B28.16:                       # Preds ..B28.15
..LN4951:
        movl      16(%ebx), %eax                                #4842.37
        negl      %eax                                          #4842.37
        addl      -44(%ebp), %eax                               #4842.37
        movl      %eax, -56(%ebp)                               #4842.37
        cmpl      $2, %eax                                      #4842.37
        je        ..B28.20      # Prob 50%                      #4842.37
                                # LOE
..B28.17:                       # Preds ..B28.16
        movl      -56(%ebp), %eax                               #4842.37
        cmpl      $1, %eax                                      #4842.37
        je        ..B28.22      # Prob 50%                      #4842.37
                                # LOE
..B28.18:                       # Preds ..B28.17
        movl      -56(%ebp), %eax                               #4842.37
        testl     %eax, %eax                                    #4842.37
        je        ..B28.24      # Prob 50%                      #4842.37
                                # LOE
..B28.19:                       # Preds ..B28.18
..LN4953:
        movl      $1, %eax                                      #4845.32
        leave                                                   #4845.32
..___tag_value_eval_4na_128.930:                                #
        movl      %ebx, %esp                                    #4845.32
        popl      %ebx                                          #4845.32
..___tag_value_eval_4na_128.931:                                #
        ret                                                     #4845.32
..___tag_value_eval_4na_128.933:                                #
                                # LOE
..B28.20:                       # Preds ..B28.16
..LN4955:
        movl      -32(%ebp), %eax                               #4847.30
..LN4957:
        testl     %eax, %eax                                    #4847.36
        je        ..B28.22      # Prob 50%                      #4847.36
                                # LOE
..B28.21:                       # Preds ..B28.20
..LN4959:
        movl      $1, %eax                                      #4847.47
        leave                                                   #4847.47
..___tag_value_eval_4na_128.937:                                #
        movl      %ebx, %esp                                    #4847.47
        popl      %ebx                                          #4847.47
..___tag_value_eval_4na_128.938:                                #
        ret                                                     #4847.47
..___tag_value_eval_4na_128.940:                                #
                                # LOE
..B28.22:                       # Preds ..B28.20 ..B28.17
..LN4961:
        movl      -28(%ebp), %eax                               #4849.30
..LN4963:
        testl     %eax, %eax                                    #4849.36
        je        ..B28.24      # Prob 50%                      #4849.36
                                # LOE
..B28.23:                       # Preds ..B28.22
..LN4965:
        movl      $1, %eax                                      #4849.47
        leave                                                   #4849.47
..___tag_value_eval_4na_128.944:                                #
        movl      %ebx, %esp                                    #4849.47
        popl      %ebx                                          #4849.47
..___tag_value_eval_4na_128.945:                                #
        ret                                                     #4849.47
..___tag_value_eval_4na_128.947:                                #
                                # LOE
..B28.24:                       # Preds ..B28.22 ..B28.18
..LN4967:
        movl      -24(%ebp), %eax                               #4851.30
..LN4969:
        testl     %eax, %eax                                    #4851.36
        je        ..B28.26      # Prob 50%                      #4851.36
                                # LOE
..B28.25:                       # Preds ..B28.24
..LN4971:
        movl      $1, %eax                                      #4851.47
        leave                                                   #4851.47
..___tag_value_eval_4na_128.951:                                #
        movl      %ebx, %esp                                    #4851.47
        popl      %ebx                                          #4851.47
..___tag_value_eval_4na_128.952:                                #
        ret                                                     #4851.47
..___tag_value_eval_4na_128.954:                                #
                                # LOE
..B28.26:                       # Preds ..B28.24
..LN4973:
        xorl      %eax, %eax                                    #4853.28
        leave                                                   #4853.28
..___tag_value_eval_4na_128.958:                                #
        movl      %ebx, %esp                                    #4853.28
        popl      %ebx                                          #4853.28
..___tag_value_eval_4na_128.959:                                #
        ret                                                     #4853.28
..___tag_value_eval_4na_128.961:                                #
                                # LOE
..B28.27:                       # Preds ..B28.15
..LN4975:
        movl      16(%ebx), %eax                                #4880.17
        addl      $4, %eax                                      #4880.17
        movl      %eax, 16(%ebx)                                #4880.17
..LN4977:
        movl      16(%ebx), %eax                                #4883.22
..LN4979:
        movl      -44(%ebp), %edx                               #4883.28
        cmpl      %edx, %eax                                    #4883.28
        jbe       ..B28.29      # Prob 50%                      #4883.28
                                # LOE
..B28.28:                       # Preds ..B28.27
..LN4981:
        xorl      %eax, %eax                                    #4884.28
        leave                                                   #4884.28
..___tag_value_eval_4na_128.965:                                #
        movl      %ebx, %esp                                    #4884.28
        popl      %ebx                                          #4884.28
..___tag_value_eval_4na_128.966:                                #
        ret                                                     #4884.28
..___tag_value_eval_4na_128.968:                                #
                                # LOE
..B28.29:                       # Preds ..B28.27
..LN4983:
        movl      -20(%ebp), %eax                               #4887.25
        decl      %eax                                          #4887.25
        movl      %eax, -20(%ebp)                               #4887.25
..LN4985:
        je        ..B28.34      # Prob 50%                      #4887.39
                                # LOE
..B28.30:                       # Preds ..B28.29
..LN4987:
        movdqa    -216(%ebp), %xmm0                             #4892.43
..LN4989:
        psrldq    $2, %xmm0                                     #4892.26
..LN4991:
        movdqa    %xmm0, -216(%ebp)                             #4892.17
..LN4993:
        movl      -36(%ebp), %eax                               #4895.22
..LN4995:
        movl      -40(%ebp), %edx                               #4895.26
        cmpl      %edx, %eax                                    #4895.26
        jae       ..B28.32      # Prob 50%                      #4895.26
                                # LOE
..B28.31:                       # Preds ..B28.30
..LN4997:
        movdqa    -216(%ebp), %xmm0                             #4896.49
..LN4999:
        movl      -36(%ebp), %eax                               #4896.72
        movzbl    (%eax), %eax                                  #4896.72
        movzbl    %al, %eax                                     #4896.72
..LN5001:
        movl      -8(%ebp), %edx                                #4896.57
        movzwl    expand_2na@GOTOFF(%edx,%eax,2), %eax          #4896.57
        movzwl    %ax, %eax                                     #4896.57
..LN5003:
        pinsrw    $7, %eax, %xmm0                               #4896.30
..LN5005:
        movdqa    %xmm0, -216(%ebp)                             #4896.21
                                # LOE
..B28.32:                       # Preds ..B28.31 ..B28.30
..LN5007:
        incl      -36(%ebp)                                     #4899.20
        jmp       ..B28.10      # Prob 100%                     #4899.20
                                # LOE
..B28.34:                       # Preds ..B28.29 ..B28.10
..LN5009:
        movl      -48(%ebp), %eax                               #4904.18
..LN5011:
        movl      -40(%ebp), %edx                               #4904.25
        cmpl      %edx, %eax                                    #4904.25
        jae       ..B28.38      # Prob 50%                      #4904.25
                                # LOE
..B28.35:                       # Preds ..B28.34
..LN5013:
        movl      16(%ebx), %eax                                #4909.18
..LN5015:
        movl      -44(%ebp), %edx                               #4909.24
        cmpl      %edx, %eax                                    #4909.24
        ja        ..B28.38      # Prob 50%                      #4909.24
                                # LOE
..B28.36:                       # Preds ..B28.35
..LN5017:
        addl      $-8, %esp                                     #4914.22
..LN5019:
        movl      -48(%ebp), %eax                               #4914.41
        movl      %eax, (%esp)                                  #4914.41
..LN5021:
        movl      -40(%ebp), %eax                               #4914.46
        movl      %eax, 4(%esp)                                 #4914.46
..LN5023:
        call      prime_buffer_4na                              #4914.22
                                # LOE xmm0
..B28.43:                       # Preds ..B28.36
        addl      $8, %esp                                      #4914.22
        movdqa    %xmm0, -264(%ebp)                             #4914.22
                                # LOE
..B28.37:                       # Preds ..B28.43
..LN5025:
        movdqa    -264(%ebp), %xmm0                             #4914.13
        movdqa    %xmm0, -216(%ebp)                             #4914.13
..LN5027:
        addl      $8, -48(%ebp)                                 #4929.13
..LN5029:
        movl      -48(%ebp), %eax                               #4931.17
..LN5031:
        movl      %eax, -36(%ebp)                               #4931.13
        jmp       ..B28.8       # Prob 100%                     #4931.13
                                # LOE
..B28.38:                       # Preds ..B28.35 ..B28.34 ..B28.8
..LN5033:
        xorl      %eax, %eax                                    #4940.12
        leave                                                   #4940.12
..___tag_value_eval_4na_128.972:                                #
        movl      %ebx, %esp                                    #4940.12
        popl      %ebx                                          #4940.12
..___tag_value_eval_4na_128.973:                                #
        ret                                                     #4940.12
        .align    2,0x90
..___tag_value_eval_4na_128.975:                                #
                                # LOE
# mark_end;
	.type	eval_4na_128,@function
	.size	eval_4na_128,.-eval_4na_128
.LNeval_4na_128:
	.data
# -- End  eval_4na_128
	.section .rodata, "a"
	.align 4
__$Uf:
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
	.type	__$Uf,@object
	.size	__$Uf,13
	.text
# -- Begin  eval_4na_pos
# mark_begin;
       .align    2,0x90
eval_4na_pos:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B29.1:                        # Preds ..B29.0
..___tag_value_eval_4na_pos.978:                                #
..LN5035:
        pushl     %ebx                                          #4946.1
..___tag_value_eval_4na_pos.983:                                #
        movl      %esp, %ebx                                    #4946.1
..___tag_value_eval_4na_pos.984:                                #
        andl      $-16, %esp                                    #4946.1
        pushl     %ebp                                          #4946.1
        pushl     %ebp                                          #4946.1
        movl      4(%ebx), %ebp                                 #4946.1
        movl      %ebp, 4(%esp)                                 #4946.1
        movl      %esp, %ebp                                    #4946.1
..___tag_value_eval_4na_pos.986:                                #
        subl      $280, %esp                                    #4946.1
        movl      %ebx, -280(%ebp)                              #4946.1
        call      ..L988        # Prob 100%                     #4946.1
..L988:                                                         #
        popl      %eax                                          #4946.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L988], %eax     #4946.1
        movl      %eax, -4(%ebp)                                #4946.1
..LN5037:
        movl      8(%ebx), %eax                                 #4978.25
        movl      4(%eax), %eax                                 #4978.25
..LN5039:
        movl      %eax, -8(%ebp)                                #4978.23
..LN5041:
        movl      20(%ebx), %eax                                #4981.5
        movl      -8(%ebp), %edx                                #4981.5
        cmpl      %edx, %eax                                    #4981.5
        jae       ..B29.3       # Prob 50%                      #4981.5
                                # LOE
..B29.2:                        # Preds ..B29.1
        addl      $-16, %esp                                    #4981.5
        movl      -4(%ebp), %eax                                #4981.5
        lea       _2__STRING.9@GOTOFF(%eax), %eax               #4981.5
        movl      %eax, (%esp)                                  #4981.5
        movl      -4(%ebp), %eax                                #4981.5
        lea       _2__STRING.1@GOTOFF(%eax), %eax               #4981.5
        movl      %eax, 4(%esp)                                 #4981.5
        movl      $4981, 8(%esp)                                #4981.5
        movl      -4(%ebp), %eax                                #4981.5
        lea       __$Uf@GOTOFF(%eax), %eax                      #4981.5
        movl      %eax, 12(%esp)                                #4981.5
        movl      -4(%ebp), %eax                                #4981.5
        movl      %eax, %ebx                                    #4981.5
..___tag_value_eval_4na_pos.989:                                #4981.5
        call      __assert_fail@PLT                             #4981.5
        movl      -280(%ebp), %ebx                              #4981.5
..___tag_value_eval_4na_pos.990:                                #
                                # LOE
..B29.54:                       # Preds ..B29.2
        addl      $16, %esp                                     #4981.5
                                # LOE
..B29.3:                        # Preds ..B29.1
..LN5043:
        movl      16(%ebx), %eax                                #4985.12
..LN5045:
        addl      20(%ebx), %eax                                #4985.5
        movl      %eax, 20(%ebx)                                #4985.5
..LN5047:
        movl      16(%ebx), %eax                                #4989.13
..LN5049:
        movl      %eax, -48(%ebp)                               #4989.5
..LN5051:
        movl      16(%ebx), %eax                                #4993.42
..LN5053:
        shrl      $2, %eax                                      #4993.49
..LN5055:
        addl      12(%ebx), %eax                                #4993.30
..LN5057:
        movl      %eax, -44(%ebp)                               #4993.5
..LN5059:
        movl      -8(%ebp), %eax                                #4996.18
        negl      %eax                                          #4996.18
        addl      20(%ebx), %eax                                #4996.18
..LN5061:
        movl      %eax, -40(%ebp)                               #4996.5
..LN5063:
        movl      20(%ebx), %eax                                #4999.44
..LN5065:
        addl      $3, %eax                                      #4999.50
..LN5067:
        shrl      $2, %eax                                      #4999.57
..LN5069:
        addl      12(%ebx), %eax                                #4999.30
..LN5071:
        movl      %eax, -36(%ebp)                               #4999.5
..LN5073:
        addl      $-8, %esp                                     #5002.14
..LN5075:
        movl      -44(%ebp), %eax                               #5002.33
        movl      %eax, (%esp)                                  #5002.33
..LN5077:
        movl      -36(%ebp), %eax                               #5002.38
        movl      %eax, 4(%esp)                                 #5002.38
..LN5079:
        call      prime_buffer_4na                              #5002.14
                                # LOE xmm0
..B29.55:                       # Preds ..B29.3
        addl      $8, %esp                                      #5002.14
        movdqa    %xmm0, -72(%ebp)                              #5002.14
                                # LOE
..B29.4:                        # Preds ..B29.55
..LN5081:
        movdqa    -72(%ebp), %xmm0                              #5002.5
        movdqa    %xmm0, -216(%ebp)                             #5002.5
..LN5083:
        addl      $8, -44(%ebp)                                 #5003.5
..LN5085:
        movl      -44(%ebp), %eax                               #5005.9
..LN5087:
        movl      %eax, -32(%ebp)                               #5005.5
..LN5089:
        movl      8(%ebx), %eax                                 #5009.5
        movdqa    16(%eax), %xmm0                               #5009.5
        movdqa    %xmm0, -200(%ebp)                             #5009.5
        movl      8(%ebx), %eax                                 #5009.5
        movdqa    32(%eax), %xmm0                               #5009.5
        movdqa    %xmm0, -184(%ebp)                             #5009.5
        movl      8(%ebx), %eax                                 #5009.5
        movdqa    48(%eax), %xmm0                               #5009.5
        movdqa    %xmm0, -168(%ebp)                             #5009.5
        movl      8(%ebx), %eax                                 #5009.5
        movdqa    64(%eax), %xmm0                               #5009.5
        movdqa    %xmm0, -152(%ebp)                             #5009.5
        movl      8(%ebx), %eax                                 #5009.5
        movdqa    80(%eax), %xmm0                               #5009.5
        movdqa    %xmm0, -136(%ebp)                             #5009.5
        movl      8(%ebx), %eax                                 #5009.5
        movdqa    96(%eax), %xmm0                               #5009.5
        movdqa    %xmm0, -120(%ebp)                             #5009.5
        movl      8(%ebx), %eax                                 #5009.5
        movdqa    112(%eax), %xmm0                              #5009.5
        movdqa    %xmm0, -104(%ebp)                             #5009.5
        movl      8(%ebx), %eax                                 #5009.5
        movdqa    128(%eax), %xmm0                              #5009.5
        movdqa    %xmm0, -88(%ebp)                              #5009.5
..LN5091:
        xorl      %eax, %eax                                    #5012.15
        movl      %eax, -28(%ebp)                               #5012.15
..LN5093:
        movl      %eax, -24(%ebp)                               #5012.10
..LN5095:
        movl      %eax, -20(%ebp)                               #5012.5
..LN5097:
        movl      16(%ebx), %eax                                #5014.27
        negl      %eax                                          #5014.27
..LN5099:
        movl      -40(%ebp), %edx                               #5014.20
..LN5101:
        lea       7(%edx,%eax), %eax                            #5014.33
..LN5103:
        shrl      $2, %eax                                      #5014.40
..LN5105:
        movl      %eax, -16(%ebp)                               #5014.5
..LN5107:
        movl      16(%ebx), %eax                                #5023.14
..LN5109:
        andl      $3, %eax                                      #5023.20
        movl      %eax, -12(%ebp)                               #5023.20
        je        ..B29.12      # Prob 50%                      #5023.20
                                # LOE
..B29.5:                        # Preds ..B29.4
        movl      -12(%ebp), %eax                               #5023.20
        cmpl      $1, %eax                                      #5023.20
        je        ..B29.13      # Prob 50%                      #5023.20
                                # LOE
..B29.6:                        # Preds ..B29.5
        movl      -12(%ebp), %eax                               #5023.20
        cmpl      $2, %eax                                      #5023.20
        je        ..B29.14      # Prob 50%                      #5023.20
                                # LOE
..B29.7:                        # Preds ..B29.6
        movl      -12(%ebp), %eax                               #5023.20
        cmpl      $3, %eax                                      #5023.20
        je        ..B29.15      # Prob 50%                      #5023.20
                                # LOE
..B29.8:                        # Preds ..B29.50 ..B29.7
..LN5111:
        movl      $1, %eax                                      #5028.9
        testl     %eax, %eax                                    #5028.9
        je        ..B29.51      # Prob 100%                     #5028.9
                                # LOE
..B29.9:                        # Preds ..B29.8
..LN5113:
        movl      $8, -16(%ebp)                                 #5030.13
                                # LOE
..B29.10:                       # Preds ..B29.45 ..B29.9
..LN5115:
        movl      $1, %eax                                      #5033.13
        testl     %eax, %eax                                    #5033.13
        je        ..B29.47      # Prob 100%                     #5033.13
                                # LOE
..B29.12:                       # Preds ..B29.4 ..B29.10
..LN5117:
        movdqa    -216(%ebp), %xmm0                             #5040.38
..LN5119:
        movdqa    -200(%ebp), %xmm1                             #5040.46
..LN5121:
        pand      %xmm1, %xmm0                                  #5040.22
..LN5123:
        movdqa    %xmm0, -248(%ebp)                             #5040.17
..LN5125:
        movdqa    -216(%ebp), %xmm0                             #5041.38
..LN5127:
        movdqa    -184(%ebp), %xmm1                             #5041.46
..LN5129:
        pand      %xmm1, %xmm0                                  #5041.22
..LN5131:
        movdqa    %xmm0, -232(%ebp)                             #5041.17
..LN5133:
        movdqa    -248(%ebp), %xmm0                             #5042.22
        movdqa    -232(%ebp), %xmm1                             #5042.22
        pcmpeqd   %xmm1, %xmm0                                  #5042.22
..LN5135:
        movdqa    %xmm0, -248(%ebp)                             #5042.17
..LN5137:
        movdqa    -248(%ebp), %xmm0                             #5043.42
..LN5139:
        pmovmskb  %xmm0, %eax                                   #5043.22
..LN5141:
        movl      %eax, -20(%ebp)                               #5043.17
..LN5143:
        movl      -20(%ebp), %eax                               #5044.17
        incl      %eax                                          #5044.17
        sarl      $16, %eax                                     #5044.17
        negl      %eax                                          #5044.17
        movl      %eax, -20(%ebp)                               #5044.17
                                # LOE
..B29.13:                       # Preds ..B29.5 ..B29.12
..LN5145:
        movdqa    -216(%ebp), %xmm0                             #5047.38
..LN5147:
        movdqa    -168(%ebp), %xmm1                             #5047.46
..LN5149:
        pand      %xmm1, %xmm0                                  #5047.22
..LN5151:
        movdqa    %xmm0, -248(%ebp)                             #5047.17
..LN5153:
        movdqa    -216(%ebp), %xmm0                             #5048.38
..LN5155:
        movdqa    -152(%ebp), %xmm1                             #5048.46
..LN5157:
        pand      %xmm1, %xmm0                                  #5048.22
..LN5159:
        movdqa    %xmm0, -232(%ebp)                             #5048.17
..LN5161:
        movdqa    -248(%ebp), %xmm0                             #5049.22
        movdqa    -232(%ebp), %xmm1                             #5049.22
        pcmpeqd   %xmm1, %xmm0                                  #5049.22
..LN5163:
        movdqa    %xmm0, -248(%ebp)                             #5049.17
..LN5165:
        movdqa    -248(%ebp), %xmm0                             #5050.42
..LN5167:
        pmovmskb  %xmm0, %eax                                   #5050.22
..LN5169:
        movl      %eax, -24(%ebp)                               #5050.17
..LN5171:
        movl      -24(%ebp), %eax                               #5051.17
        incl      %eax                                          #5051.17
        sarl      $16, %eax                                     #5051.17
        negl      %eax                                          #5051.17
        movl      %eax, -24(%ebp)                               #5051.17
                                # LOE
..B29.14:                       # Preds ..B29.6 ..B29.13
..LN5173:
        movdqa    -216(%ebp), %xmm0                             #5054.38
..LN5175:
        movdqa    -136(%ebp), %xmm1                             #5054.46
..LN5177:
        pand      %xmm1, %xmm0                                  #5054.22
..LN5179:
        movdqa    %xmm0, -248(%ebp)                             #5054.17
..LN5181:
        movdqa    -216(%ebp), %xmm0                             #5055.38
..LN5183:
        movdqa    -120(%ebp), %xmm1                             #5055.46
..LN5185:
        pand      %xmm1, %xmm0                                  #5055.22
..LN5187:
        movdqa    %xmm0, -232(%ebp)                             #5055.17
..LN5189:
        movdqa    -248(%ebp), %xmm0                             #5056.22
        movdqa    -232(%ebp), %xmm1                             #5056.22
        pcmpeqd   %xmm1, %xmm0                                  #5056.22
..LN5191:
        movdqa    %xmm0, -248(%ebp)                             #5056.17
..LN5193:
        movdqa    -248(%ebp), %xmm0                             #5057.42
..LN5195:
        pmovmskb  %xmm0, %eax                                   #5057.22
..LN5197:
        movl      %eax, -28(%ebp)                               #5057.17
..LN5199:
        movl      -28(%ebp), %eax                               #5058.17
        incl      %eax                                          #5058.17
        sarl      $16, %eax                                     #5058.17
        negl      %eax                                          #5058.17
        movl      %eax, -28(%ebp)                               #5058.17
                                # LOE
..B29.15:                       # Preds ..B29.7 ..B29.14
..LN5201:
        movdqa    -216(%ebp), %xmm0                             #5061.38
..LN5203:
        movdqa    -104(%ebp), %xmm1                             #5061.46
..LN5205:
        pand      %xmm1, %xmm0                                  #5061.22
..LN5207:
        movdqa    %xmm0, -248(%ebp)                             #5061.17
..LN5209:
        movdqa    -216(%ebp), %xmm0                             #5062.38
..LN5211:
        movdqa    -88(%ebp), %xmm1                              #5062.46
..LN5213:
        pand      %xmm1, %xmm0                                  #5062.22
..LN5215:
        movdqa    %xmm0, -232(%ebp)                             #5062.17
..LN5217:
        movdqa    -248(%ebp), %xmm0                             #5063.22
        movdqa    -232(%ebp), %xmm1                             #5063.22
        pcmpeqd   %xmm1, %xmm0                                  #5063.22
..LN5219:
        movdqa    %xmm0, -248(%ebp)                             #5063.17
..LN5221:
        movdqa    -248(%ebp), %xmm0                             #5064.42
..LN5223:
        pmovmskb  %xmm0, %eax                                   #5064.22
..LN5225:
        movl      %eax, -52(%ebp)                               #5064.17
..LN5227:
        movl      -52(%ebp), %eax                               #5065.17
        incl      %eax                                          #5065.17
        sarl      $16, %eax                                     #5065.17
        negl      %eax                                          #5065.17
        movl      %eax, -52(%ebp)                               #5065.17
..LN5229:
        movl      16(%ebx), %eax                                #5069.17
        andl      $-4, %eax                                     #5069.17
        movl      %eax, 16(%ebx)                                #5069.17
..LN5231:
        movl      -24(%ebp), %eax                               #5072.29
        orl       -20(%ebp), %eax                               #5072.29
..LN5233:
        orl       -28(%ebp), %eax                               #5072.34
..LN5235:
        orl       -52(%ebp), %eax                               #5072.39
..LN5237:
        je        ..B29.40      # Prob 50%                      #5072.47
                                # LOE
..B29.16:                       # Preds ..B29.15
..LN5239:
        movl      16(%ebx), %eax                                #5075.37
        negl      %eax                                          #5075.37
        addl      -40(%ebp), %eax                               #5075.37
        movl      %eax, -56(%ebp)                               #5075.37
        je        ..B29.19      # Prob 50%                      #5075.37
                                # LOE
..B29.17:                       # Preds ..B29.16
        movl      -56(%ebp), %eax                               #5075.37
        cmpl      $1, %eax                                      #5075.37
        je        ..B29.21      # Prob 50%                      #5075.37
                                # LOE
..B29.18:                       # Preds ..B29.17
        movl      -56(%ebp), %eax                               #5075.37
        cmpl      $2, %eax                                      #5075.37
        je        ..B29.25      # Prob 50%                      #5075.37
        jmp       ..B29.31      # Prob 100%                     #5075.37
                                # LOE
..B29.19:                       # Preds ..B29.16
..LN5241:
        movl      -20(%ebp), %eax                               #5078.30
..LN5243:
        testl     %eax, %eax                                    #5078.36
        je        ..B29.39      # Prob 50%                      #5078.36
                                # LOE
..B29.20:                       # Preds ..B29.19
..LN5245:
        movl      -48(%ebp), %eax                               #5078.53
        negl      %eax                                          #5078.53
..LN5247:
        movl      16(%ebx), %edx                                #5078.47
..LN5249:
        lea       1(%edx,%eax), %eax                            #5078.61
        leave                                                   #5078.61
..___tag_value_eval_4na_pos.992:                                #
        movl      %ebx, %esp                                    #5078.61
        popl      %ebx                                          #5078.61
..___tag_value_eval_4na_pos.993:                                #
        ret                                                     #5078.61
..___tag_value_eval_4na_pos.995:                                #
                                # LOE
..B29.21:                       # Preds ..B29.17
..LN5251:
        movl      -20(%ebp), %eax                               #5081.30
..LN5253:
        testl     %eax, %eax                                    #5081.36
        je        ..B29.23      # Prob 50%                      #5081.36
                                # LOE
..B29.22:                       # Preds ..B29.21
..LN5255:
        movl      -48(%ebp), %eax                               #5081.53
        negl      %eax                                          #5081.53
..LN5257:
        movl      16(%ebx), %edx                                #5081.47
..LN5259:
        lea       1(%edx,%eax), %eax                            #5081.61
        leave                                                   #5081.61
..___tag_value_eval_4na_pos.999:                                #
        movl      %ebx, %esp                                    #5081.61
        popl      %ebx                                          #5081.61
..___tag_value_eval_4na_pos.1000:                               #
        ret                                                     #5081.61
..___tag_value_eval_4na_pos.1002:                               #
                                # LOE
..B29.23:                       # Preds ..B29.21
..LN5261:
        movl      -24(%ebp), %eax                               #5082.30
..LN5263:
        testl     %eax, %eax                                    #5082.36
        je        ..B29.39      # Prob 50%                      #5082.36
                                # LOE
..B29.24:                       # Preds ..B29.23
..LN5265:
        movl      -48(%ebp), %eax                               #5082.53
        negl      %eax                                          #5082.53
..LN5267:
        movl      16(%ebx), %edx                                #5082.47
..LN5269:
        lea       2(%edx,%eax), %eax                            #5082.61
        leave                                                   #5082.61
..___tag_value_eval_4na_pos.1006:                               #
        movl      %ebx, %esp                                    #5082.61
        popl      %ebx                                          #5082.61
..___tag_value_eval_4na_pos.1007:                               #
        ret                                                     #5082.61
..___tag_value_eval_4na_pos.1009:                               #
                                # LOE
..B29.25:                       # Preds ..B29.18
..LN5271:
        movl      -20(%ebp), %eax                               #5085.30
..LN5273:
        testl     %eax, %eax                                    #5085.36
        je        ..B29.27      # Prob 50%                      #5085.36
                                # LOE
..B29.26:                       # Preds ..B29.25
..LN5275:
        movl      -48(%ebp), %eax                               #5085.53
        negl      %eax                                          #5085.53
..LN5277:
        movl      16(%ebx), %edx                                #5085.47
..LN5279:
        lea       1(%edx,%eax), %eax                            #5085.61
        leave                                                   #5085.61
..___tag_value_eval_4na_pos.1013:                               #
        movl      %ebx, %esp                                    #5085.61
        popl      %ebx                                          #5085.61
..___tag_value_eval_4na_pos.1014:                               #
        ret                                                     #5085.61
..___tag_value_eval_4na_pos.1016:                               #
                                # LOE
..B29.27:                       # Preds ..B29.25
..LN5281:
        movl      -24(%ebp), %eax                               #5086.30
..LN5283:
        testl     %eax, %eax                                    #5086.36
        je        ..B29.29      # Prob 50%                      #5086.36
                                # LOE
..B29.28:                       # Preds ..B29.27
..LN5285:
        movl      -48(%ebp), %eax                               #5086.53
        negl      %eax                                          #5086.53
..LN5287:
        movl      16(%ebx), %edx                                #5086.47
..LN5289:
        lea       2(%edx,%eax), %eax                            #5086.61
        leave                                                   #5086.61
..___tag_value_eval_4na_pos.1020:                               #
        movl      %ebx, %esp                                    #5086.61
        popl      %ebx                                          #5086.61
..___tag_value_eval_4na_pos.1021:                               #
        ret                                                     #5086.61
..___tag_value_eval_4na_pos.1023:                               #
                                # LOE
..B29.29:                       # Preds ..B29.27
..LN5291:
        movl      -28(%ebp), %eax                               #5087.30
..LN5293:
        testl     %eax, %eax                                    #5087.36
        je        ..B29.39      # Prob 50%                      #5087.36
                                # LOE
..B29.30:                       # Preds ..B29.29
..LN5295:
        movl      -48(%ebp), %eax                               #5087.53
        negl      %eax                                          #5087.53
..LN5297:
        movl      16(%ebx), %edx                                #5087.47
..LN5299:
        lea       3(%edx,%eax), %eax                            #5087.61
        leave                                                   #5087.61
..___tag_value_eval_4na_pos.1027:                               #
        movl      %ebx, %esp                                    #5087.61
        popl      %ebx                                          #5087.61
..___tag_value_eval_4na_pos.1028:                               #
        ret                                                     #5087.61
..___tag_value_eval_4na_pos.1030:                               #
                                # LOE
..B29.31:                       # Preds ..B29.18
..LN5301:
        movl      -20(%ebp), %eax                               #5090.30
..LN5303:
        testl     %eax, %eax                                    #5090.36
        je        ..B29.33      # Prob 50%                      #5090.36
                                # LOE
..B29.32:                       # Preds ..B29.31
..LN5305:
        movl      -48(%ebp), %eax                               #5090.53
        negl      %eax                                          #5090.53
..LN5307:
        movl      16(%ebx), %edx                                #5090.47
..LN5309:
        lea       1(%edx,%eax), %eax                            #5090.61
        leave                                                   #5090.61
..___tag_value_eval_4na_pos.1034:                               #
        movl      %ebx, %esp                                    #5090.61
        popl      %ebx                                          #5090.61
..___tag_value_eval_4na_pos.1035:                               #
        ret                                                     #5090.61
..___tag_value_eval_4na_pos.1037:                               #
                                # LOE
..B29.33:                       # Preds ..B29.31
..LN5311:
        movl      -24(%ebp), %eax                               #5091.30
..LN5313:
        testl     %eax, %eax                                    #5091.36
        je        ..B29.35      # Prob 50%                      #5091.36
                                # LOE
..B29.34:                       # Preds ..B29.33
..LN5315:
        movl      -48(%ebp), %eax                               #5091.53
        negl      %eax                                          #5091.53
..LN5317:
        movl      16(%ebx), %edx                                #5091.47
..LN5319:
        lea       2(%edx,%eax), %eax                            #5091.61
        leave                                                   #5091.61
..___tag_value_eval_4na_pos.1041:                               #
        movl      %ebx, %esp                                    #5091.61
        popl      %ebx                                          #5091.61
..___tag_value_eval_4na_pos.1042:                               #
        ret                                                     #5091.61
..___tag_value_eval_4na_pos.1044:                               #
                                # LOE
..B29.35:                       # Preds ..B29.33
..LN5321:
        movl      -28(%ebp), %eax                               #5092.30
..LN5323:
        testl     %eax, %eax                                    #5092.36
        je        ..B29.37      # Prob 50%                      #5092.36
                                # LOE
..B29.36:                       # Preds ..B29.35
..LN5325:
        movl      -48(%ebp), %eax                               #5092.53
        negl      %eax                                          #5092.53
..LN5327:
        movl      16(%ebx), %edx                                #5092.47
..LN5329:
        lea       3(%edx,%eax), %eax                            #5092.61
        leave                                                   #5092.61
..___tag_value_eval_4na_pos.1048:                               #
        movl      %ebx, %esp                                    #5092.61
        popl      %ebx                                          #5092.61
..___tag_value_eval_4na_pos.1049:                               #
        ret                                                     #5092.61
..___tag_value_eval_4na_pos.1051:                               #
                                # LOE
..B29.37:                       # Preds ..B29.35
..LN5331:
        movl      -52(%ebp), %eax                               #5093.30
..LN5333:
        testl     %eax, %eax                                    #5093.36
        je        ..B29.39      # Prob 50%                      #5093.36
                                # LOE
..B29.38:                       # Preds ..B29.37
..LN5335:
        movl      -48(%ebp), %eax                               #5093.53
        negl      %eax                                          #5093.53
..LN5337:
        movl      16(%ebx), %edx                                #5093.47
..LN5339:
        lea       4(%edx,%eax), %eax                            #5093.61
        leave                                                   #5093.61
..___tag_value_eval_4na_pos.1055:                               #
        movl      %ebx, %esp                                    #5093.61
        popl      %ebx                                          #5093.61
..___tag_value_eval_4na_pos.1056:                               #
        ret                                                     #5093.61
..___tag_value_eval_4na_pos.1058:                               #
                                # LOE
..B29.39:                       # Preds ..B29.19 ..B29.23 ..B29.29 ..B29.37
..LN5341:
        xorl      %eax, %eax                                    #5095.28
        leave                                                   #5095.28
..___tag_value_eval_4na_pos.1062:                               #
        movl      %ebx, %esp                                    #5095.28
        popl      %ebx                                          #5095.28
..___tag_value_eval_4na_pos.1063:                               #
        ret                                                     #5095.28
..___tag_value_eval_4na_pos.1065:                               #
                                # LOE
..B29.40:                       # Preds ..B29.15
..LN5343:
        movl      16(%ebx), %eax                                #5135.17
        addl      $4, %eax                                      #5135.17
        movl      %eax, 16(%ebx)                                #5135.17
..LN5345:
        movl      16(%ebx), %eax                                #5138.22
..LN5347:
        movl      -40(%ebp), %edx                               #5138.28
        cmpl      %edx, %eax                                    #5138.28
        jbe       ..B29.42      # Prob 50%                      #5138.28
                                # LOE
..B29.41:                       # Preds ..B29.40
..LN5349:
        xorl      %eax, %eax                                    #5139.28
        leave                                                   #5139.28
..___tag_value_eval_4na_pos.1069:                               #
        movl      %ebx, %esp                                    #5139.28
        popl      %ebx                                          #5139.28
..___tag_value_eval_4na_pos.1070:                               #
        ret                                                     #5139.28
..___tag_value_eval_4na_pos.1072:                               #
                                # LOE
..B29.42:                       # Preds ..B29.40
..LN5351:
        movl      -16(%ebp), %eax                               #5142.25
        decl      %eax                                          #5142.25
        movl      %eax, -16(%ebp)                               #5142.25
..LN5353:
        je        ..B29.47      # Prob 50%                      #5142.39
                                # LOE
..B29.43:                       # Preds ..B29.42
..LN5355:
        movdqa    -216(%ebp), %xmm0                             #5147.43
..LN5357:
        psrldq    $2, %xmm0                                     #5147.26
..LN5359:
        movdqa    %xmm0, -216(%ebp)                             #5147.17
..LN5361:
        movl      -32(%ebp), %eax                               #5150.22
..LN5363:
        movl      -36(%ebp), %edx                               #5150.26
        cmpl      %edx, %eax                                    #5150.26
        jae       ..B29.45      # Prob 50%                      #5150.26
                                # LOE
..B29.44:                       # Preds ..B29.43
..LN5365:
        movdqa    -216(%ebp), %xmm0                             #5151.49
..LN5367:
        movl      -32(%ebp), %eax                               #5151.72
        movzbl    (%eax), %eax                                  #5151.72
        movzbl    %al, %eax                                     #5151.72
..LN5369:
        movl      -4(%ebp), %edx                                #5151.57
        movzwl    expand_2na@GOTOFF(%edx,%eax,2), %eax          #5151.57
        movzwl    %ax, %eax                                     #5151.57
..LN5371:
        pinsrw    $7, %eax, %xmm0                               #5151.30
..LN5373:
        movdqa    %xmm0, -216(%ebp)                             #5151.21
                                # LOE
..B29.45:                       # Preds ..B29.44 ..B29.43
..LN5375:
        incl      -32(%ebp)                                     #5154.20
        jmp       ..B29.10      # Prob 100%                     #5154.20
                                # LOE
..B29.47:                       # Preds ..B29.42 ..B29.10
..LN5377:
        movl      -44(%ebp), %eax                               #5159.18
..LN5379:
        movl      -36(%ebp), %edx                               #5159.25
        cmpl      %edx, %eax                                    #5159.25
        jae       ..B29.51      # Prob 50%                      #5159.25
                                # LOE
..B29.48:                       # Preds ..B29.47
..LN5381:
        movl      16(%ebx), %eax                                #5164.18
..LN5383:
        movl      -40(%ebp), %edx                               #5164.24
        cmpl      %edx, %eax                                    #5164.24
        ja        ..B29.51      # Prob 50%                      #5164.24
                                # LOE
..B29.49:                       # Preds ..B29.48
..LN5385:
        addl      $-8, %esp                                     #5169.22
..LN5387:
        movl      -44(%ebp), %eax                               #5169.41
        movl      %eax, (%esp)                                  #5169.41
..LN5389:
        movl      -36(%ebp), %eax                               #5169.46
        movl      %eax, 4(%esp)                                 #5169.46
..LN5391:
        call      prime_buffer_4na                              #5169.22
                                # LOE xmm0
..B29.56:                       # Preds ..B29.49
        addl      $8, %esp                                      #5169.22
        movdqa    %xmm0, -264(%ebp)                             #5169.22
                                # LOE
..B29.50:                       # Preds ..B29.56
..LN5393:
        movdqa    -264(%ebp), %xmm0                             #5169.13
        movdqa    %xmm0, -216(%ebp)                             #5169.13
..LN5395:
        addl      $8, -44(%ebp)                                 #5184.13
..LN5397:
        movl      -44(%ebp), %eax                               #5186.17
..LN5399:
        movl      %eax, -32(%ebp)                               #5186.13
        jmp       ..B29.8       # Prob 100%                     #5186.13
                                # LOE
..B29.51:                       # Preds ..B29.48 ..B29.47 ..B29.8
..LN5401:
        xorl      %eax, %eax                                    #5196.12
        leave                                                   #5196.12
..___tag_value_eval_4na_pos.1076:                               #
        movl      %ebx, %esp                                    #5196.12
        popl      %ebx                                          #5196.12
..___tag_value_eval_4na_pos.1077:                               #
        ret                                                     #5196.12
        .align    2,0x90
..___tag_value_eval_4na_pos.1079:                               #
                                # LOE
# mark_end;
	.type	eval_4na_pos,@function
	.size	eval_4na_pos,.-eval_4na_pos
.LNeval_4na_pos:
	.data
# -- End  eval_4na_pos
	.data
	.text
# -- Begin  NucStrstrSearch
# mark_begin;
       .align    2,0x90
	.globl NucStrstrSearch
NucStrstrSearch:
# parameter 1(self): 8 + %ebp
# parameter 2(ncbi2na): 12 + %ebp
# parameter 3(pos): 16 + %ebp
# parameter 4(len): 20 + %ebp
# parameter 5(selflen): 24 + %ebp
..B30.1:                        # Preds ..B30.0
..___tag_value_NucStrstrSearch.1082:                            #
..LN5403:
        pushl     %ebp                                          #5217.1
        movl      %esp, %ebp                                    #5217.1
..___tag_value_NucStrstrSearch.1087:                            #
        subl      $92, %esp                                     #5217.1
        movl      %ebx, -4(%ebp)                                #5217.1
..___tag_value_NucStrstrSearch.1090:                            #
..LN5405:
        movl      8(%ebp), %eax                                 #5218.10
..LN5407:
        testl     %eax, %eax                                    #5218.18
        je        ..B30.111     # Prob 50%                      #5218.18
                                # LOE
..B30.2:                        # Preds ..B30.1
..LN5409:
        movl      12(%ebp), %eax                                #5218.26
..LN5411:
        testl     %eax, %eax                                    #5218.37
        je        ..B30.111     # Prob 50%                      #5218.37
                                # LOE
..B30.3:                        # Preds ..B30.2
..LN5413:
        movl      20(%ebp), %eax                                #5218.45
..LN5415:
        testl     %eax, %eax                                    #5218.52
        je        ..B30.111     # Prob 50%                      #5218.52
                                # LOE
..B30.4:                        # Preds ..B30.3
..LN5417:
        movl      8(%ebp), %eax                                 #5223.18
        movl      (%eax), %eax                                  #5223.18
        movl      %eax, -8(%ebp)                                #5223.18
..LN5419:
        cmpl      $12, %eax                                     #5223.9
        ja        ..B30.111     # Prob 50%                      #5223.9
                                # LOE
..B30.5:                        # Preds ..B30.4
        call      ..L1091       # Prob 100%                     #
..L1091:                                                        #
        popl      %eax                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L1091], %eax    #
        movl      %eax, -12(%ebp)                               #
        movl      -8(%ebp), %edx                                #5223.9
        movl      ..1..TPKT.30_0@GOTOFF(%eax,%edx,4), %eax      #5223.9
        jmp       *%eax                                         #5223.9
                                # LOE
..1.30_0.TAG.00:
..B30.7:                        # Preds ..B30.5
..LN5421:
        movl      8(%ebp), %eax                                 #5226.24
..LN5423:
        movl      20(%ebp), %edx                                #5226.18
..LN5425:
        movl      4(%eax), %eax                                 #5226.24
        cmpl      %eax, %edx                                    #5226.24
        jae       ..B30.9       # Prob 50%                      #5226.24
                                # LOE
..B30.8:                        # Preds ..B30.7
..LN5427:
        xorl      %eax, %eax                                    #5226.54
        movl      -4(%ebp), %ebx                                #5226.54
..___tag_value_NucStrstrSearch.1092:                            #
        leave                                                   #5226.54
..___tag_value_NucStrstrSearch.1094:                            #
        ret                                                     #5226.54
..___tag_value_NucStrstrSearch.1095:                            #
                                # LOE
..B30.9:                        # Preds ..B30.7
..LN5429:
        movl      24(%ebp), %eax                                #5227.9
        testl     %eax, %eax                                    #5227.9
        je        ..B30.11      # Prob 50%                      #5227.9
                                # LOE
..B30.10:                       # Preds ..B30.9
..LN5431:
        movl      8(%ebp), %eax                                 #5227.27
..LN5433:
        movl      24(%ebp), %edx                                #5227.18
..LN5435:
        movl      4(%eax), %eax                                 #5227.27
..LN5437:
        movl      %eax, (%edx)                                  #5227.18
                                # LOE
..B30.11:                       # Preds ..B30.10 ..B30.9
..LN5439:
        addl      $-16, %esp                                    #5228.20
..LN5441:
        movl      8(%ebp), %eax                                 #5228.36
        movl      %eax, (%esp)                                  #5228.36
..LN5443:
        movl      12(%ebp), %eax                                #5228.51
        movl      %eax, 4(%esp)                                 #5228.51
..LN5445:
        movl      16(%ebp), %eax                                #5228.60
        movl      %eax, 8(%esp)                                 #5228.60
..LN5447:
        movl      20(%ebp), %eax                                #5228.65
        movl      %eax, 12(%esp)                                #5228.65
..LN5449:
        call      eval_2na_64                                   #5228.20
                                # LOE eax
..B30.114:                      # Preds ..B30.11
        addl      $16, %esp                                     #5228.20
        movl      %eax, -80(%ebp)                               #5228.20
                                # LOE
..B30.12:                       # Preds ..B30.114
        movl      -80(%ebp), %eax                               #5228.20
        movl      -4(%ebp), %ebx                                #5228.20
..___tag_value_NucStrstrSearch.1097:                            #
        leave                                                   #5228.20
..___tag_value_NucStrstrSearch.1099:                            #
        ret                                                     #5228.20
..___tag_value_NucStrstrSearch.1100:                            #
                                # LOE
..1.30_0.TAG.01:
..B30.14:                       # Preds ..B30.5
..LN5451:
        movl      8(%ebp), %eax                                 #5230.24
..LN5453:
        movl      20(%ebp), %edx                                #5230.18
..LN5455:
        movl      4(%eax), %eax                                 #5230.24
        cmpl      %eax, %edx                                    #5230.24
        jae       ..B30.16      # Prob 50%                      #5230.24
                                # LOE
..B30.15:                       # Preds ..B30.14
..LN5457:
        xorl      %eax, %eax                                    #5230.54
        movl      -4(%ebp), %ebx                                #5230.54
..___tag_value_NucStrstrSearch.1102:                            #
        leave                                                   #5230.54
..___tag_value_NucStrstrSearch.1104:                            #
        ret                                                     #5230.54
..___tag_value_NucStrstrSearch.1105:                            #
                                # LOE
..B30.16:                       # Preds ..B30.14
..LN5459:
        movl      24(%ebp), %eax                                #5231.9
        testl     %eax, %eax                                    #5231.9
        je        ..B30.18      # Prob 50%                      #5231.9
                                # LOE
..B30.17:                       # Preds ..B30.16
..LN5461:
        movl      8(%ebp), %eax                                 #5231.27
..LN5463:
        movl      24(%ebp), %edx                                #5231.18
..LN5465:
        movl      4(%eax), %eax                                 #5231.27
..LN5467:
        movl      %eax, (%edx)                                  #5231.18
                                # LOE
..B30.18:                       # Preds ..B30.17 ..B30.16
..LN5469:
        addl      $-16, %esp                                    #5232.20
..LN5471:
        movl      8(%ebp), %eax                                 #5232.36
        movl      %eax, (%esp)                                  #5232.36
..LN5473:
        movl      12(%ebp), %eax                                #5232.51
        movl      %eax, 4(%esp)                                 #5232.51
..LN5475:
        movl      16(%ebp), %eax                                #5232.60
        movl      %eax, 8(%esp)                                 #5232.60
..LN5477:
        movl      20(%ebp), %eax                                #5232.65
        movl      %eax, 12(%esp)                                #5232.65
..LN5479:
        call      eval_4na_64                                   #5232.20
                                # LOE eax
..B30.115:                      # Preds ..B30.18
        addl      $16, %esp                                     #5232.20
        movl      %eax, -76(%ebp)                               #5232.20
                                # LOE
..B30.19:                       # Preds ..B30.115
        movl      -76(%ebp), %eax                               #5232.20
        movl      -4(%ebp), %ebx                                #5232.20
..___tag_value_NucStrstrSearch.1107:                            #
        leave                                                   #5232.20
..___tag_value_NucStrstrSearch.1109:                            #
        ret                                                     #5232.20
..___tag_value_NucStrstrSearch.1110:                            #
                                # LOE
..1.30_0.TAG.02:
..B30.21:                       # Preds ..B30.5
..LN5481:
        movl      8(%ebp), %eax                                 #5235.24
..LN5483:
        movl      20(%ebp), %edx                                #5235.18
..LN5485:
        movl      4(%eax), %eax                                 #5235.24
        cmpl      %eax, %edx                                    #5235.24
        jae       ..B30.23      # Prob 50%                      #5235.24
                                # LOE
..B30.22:                       # Preds ..B30.21
..LN5487:
        xorl      %eax, %eax                                    #5235.54
        movl      -4(%ebp), %ebx                                #5235.54
..___tag_value_NucStrstrSearch.1112:                            #
        leave                                                   #5235.54
..___tag_value_NucStrstrSearch.1114:                            #
        ret                                                     #5235.54
..___tag_value_NucStrstrSearch.1115:                            #
                                # LOE
..B30.23:                       # Preds ..B30.21
..LN5489:
        movl      24(%ebp), %eax                                #5236.9
        testl     %eax, %eax                                    #5236.9
        je        ..B30.25      # Prob 50%                      #5236.9
                                # LOE
..B30.24:                       # Preds ..B30.23
..LN5491:
        movl      8(%ebp), %eax                                 #5236.27
..LN5493:
        movl      24(%ebp), %edx                                #5236.18
..LN5495:
        movl      4(%eax), %eax                                 #5236.27
..LN5497:
        movl      %eax, (%edx)                                  #5236.18
                                # LOE
..B30.25:                       # Preds ..B30.24 ..B30.23
..LN5499:
        addl      $-16, %esp                                    #5237.20
..LN5501:
        movl      8(%ebp), %eax                                 #5237.35
        movl      %eax, (%esp)                                  #5237.35
..LN5503:
        movl      12(%ebp), %eax                                #5237.50
        movl      %eax, 4(%esp)                                 #5237.50
..LN5505:
        movl      16(%ebp), %eax                                #5237.59
        movl      %eax, 8(%esp)                                 #5237.59
..LN5507:
        movl      20(%ebp), %eax                                #5237.64
        movl      %eax, 12(%esp)                                #5237.64
..LN5509:
        call      eval_2na_8                                    #5237.20
                                # LOE eax
..B30.116:                      # Preds ..B30.25
        addl      $16, %esp                                     #5237.20
        movl      %eax, -72(%ebp)                               #5237.20
                                # LOE
..B30.26:                       # Preds ..B30.116
        movl      -72(%ebp), %eax                               #5237.20
        movl      -4(%ebp), %ebx                                #5237.20
..___tag_value_NucStrstrSearch.1117:                            #
        leave                                                   #5237.20
..___tag_value_NucStrstrSearch.1119:                            #
        ret                                                     #5237.20
..___tag_value_NucStrstrSearch.1120:                            #
                                # LOE
..1.30_0.TAG.03:
..B30.28:                       # Preds ..B30.5
..LN5511:
        movl      8(%ebp), %eax                                 #5239.24
..LN5513:
        movl      20(%ebp), %edx                                #5239.18
..LN5515:
        movl      4(%eax), %eax                                 #5239.24
        cmpl      %eax, %edx                                    #5239.24
        jae       ..B30.30      # Prob 50%                      #5239.24
                                # LOE
..B30.29:                       # Preds ..B30.28
..LN5517:
        xorl      %eax, %eax                                    #5239.54
        movl      -4(%ebp), %ebx                                #5239.54
..___tag_value_NucStrstrSearch.1122:                            #
        leave                                                   #5239.54
..___tag_value_NucStrstrSearch.1124:                            #
        ret                                                     #5239.54
..___tag_value_NucStrstrSearch.1125:                            #
                                # LOE
..B30.30:                       # Preds ..B30.28
..LN5519:
        movl      24(%ebp), %eax                                #5240.9
        testl     %eax, %eax                                    #5240.9
        je        ..B30.32      # Prob 50%                      #5240.9
                                # LOE
..B30.31:                       # Preds ..B30.30
..LN5521:
        movl      8(%ebp), %eax                                 #5240.27
..LN5523:
        movl      24(%ebp), %edx                                #5240.18
..LN5525:
        movl      4(%eax), %eax                                 #5240.27
..LN5527:
        movl      %eax, (%edx)                                  #5240.18
                                # LOE
..B30.32:                       # Preds ..B30.31 ..B30.30
..LN5529:
        addl      $-16, %esp                                    #5241.20
..LN5531:
        movl      8(%ebp), %eax                                 #5241.36
        movl      %eax, (%esp)                                  #5241.36
..LN5533:
        movl      12(%ebp), %eax                                #5241.51
        movl      %eax, 4(%esp)                                 #5241.51
..LN5535:
        movl      16(%ebp), %eax                                #5241.60
        movl      %eax, 8(%esp)                                 #5241.60
..LN5537:
        movl      20(%ebp), %eax                                #5241.65
        movl      %eax, 12(%esp)                                #5241.65
..LN5539:
        call      eval_2na_16                                   #5241.20
                                # LOE eax
..B30.117:                      # Preds ..B30.32
        addl      $16, %esp                                     #5241.20
        movl      %eax, -68(%ebp)                               #5241.20
                                # LOE
..B30.33:                       # Preds ..B30.117
        movl      -68(%ebp), %eax                               #5241.20
        movl      -4(%ebp), %ebx                                #5241.20
..___tag_value_NucStrstrSearch.1127:                            #
        leave                                                   #5241.20
..___tag_value_NucStrstrSearch.1129:                            #
        ret                                                     #5241.20
..___tag_value_NucStrstrSearch.1130:                            #
                                # LOE
..1.30_0.TAG.04:
..B30.35:                       # Preds ..B30.5
..LN5541:
        movl      8(%ebp), %eax                                 #5243.24
..LN5543:
        movl      20(%ebp), %edx                                #5243.18
..LN5545:
        movl      4(%eax), %eax                                 #5243.24
        cmpl      %eax, %edx                                    #5243.24
        jae       ..B30.37      # Prob 50%                      #5243.24
                                # LOE
..B30.36:                       # Preds ..B30.35
..LN5547:
        xorl      %eax, %eax                                    #5243.54
        movl      -4(%ebp), %ebx                                #5243.54
..___tag_value_NucStrstrSearch.1132:                            #
        leave                                                   #5243.54
..___tag_value_NucStrstrSearch.1134:                            #
        ret                                                     #5243.54
..___tag_value_NucStrstrSearch.1135:                            #
                                # LOE
..B30.37:                       # Preds ..B30.35
..LN5549:
        movl      24(%ebp), %eax                                #5244.9
        testl     %eax, %eax                                    #5244.9
        je        ..B30.39      # Prob 50%                      #5244.9
                                # LOE
..B30.38:                       # Preds ..B30.37
..LN5551:
        movl      8(%ebp), %eax                                 #5244.27
..LN5553:
        movl      24(%ebp), %edx                                #5244.18
..LN5555:
        movl      4(%eax), %eax                                 #5244.27
..LN5557:
        movl      %eax, (%edx)                                  #5244.18
                                # LOE
..B30.39:                       # Preds ..B30.38 ..B30.37
..LN5559:
        addl      $-16, %esp                                    #5245.20
..LN5561:
        movl      8(%ebp), %eax                                 #5245.36
        movl      %eax, (%esp)                                  #5245.36
..LN5563:
        movl      12(%ebp), %eax                                #5245.51
        movl      %eax, 4(%esp)                                 #5245.51
..LN5565:
        movl      16(%ebp), %eax                                #5245.60
        movl      %eax, 8(%esp)                                 #5245.60
..LN5567:
        movl      20(%ebp), %eax                                #5245.65
        movl      %eax, 12(%esp)                                #5245.65
..LN5569:
        call      eval_2na_32                                   #5245.20
                                # LOE eax
..B30.118:                      # Preds ..B30.39
        addl      $16, %esp                                     #5245.20
        movl      %eax, -64(%ebp)                               #5245.20
                                # LOE
..B30.40:                       # Preds ..B30.118
        movl      -64(%ebp), %eax                               #5245.20
        movl      -4(%ebp), %ebx                                #5245.20
..___tag_value_NucStrstrSearch.1137:                            #
        leave                                                   #5245.20
..___tag_value_NucStrstrSearch.1139:                            #
        ret                                                     #5245.20
..___tag_value_NucStrstrSearch.1140:                            #
                                # LOE
..1.30_0.TAG.05:
..B30.42:                       # Preds ..B30.5
..LN5571:
        movl      8(%ebp), %eax                                 #5247.24
..LN5573:
        movl      20(%ebp), %edx                                #5247.18
..LN5575:
        movl      4(%eax), %eax                                 #5247.24
        cmpl      %eax, %edx                                    #5247.24
        jae       ..B30.44      # Prob 50%                      #5247.24
                                # LOE
..B30.43:                       # Preds ..B30.42
..LN5577:
        xorl      %eax, %eax                                    #5247.54
        movl      -4(%ebp), %ebx                                #5247.54
..___tag_value_NucStrstrSearch.1142:                            #
        leave                                                   #5247.54
..___tag_value_NucStrstrSearch.1144:                            #
        ret                                                     #5247.54
..___tag_value_NucStrstrSearch.1145:                            #
                                # LOE
..B30.44:                       # Preds ..B30.42
..LN5579:
        movl      24(%ebp), %eax                                #5248.9
        testl     %eax, %eax                                    #5248.9
        je        ..B30.46      # Prob 50%                      #5248.9
                                # LOE
..B30.45:                       # Preds ..B30.44
..LN5581:
        movl      8(%ebp), %eax                                 #5248.27
..LN5583:
        movl      24(%ebp), %edx                                #5248.18
..LN5585:
        movl      4(%eax), %eax                                 #5248.27
..LN5587:
        movl      %eax, (%edx)                                  #5248.18
                                # LOE
..B30.46:                       # Preds ..B30.45 ..B30.44
..LN5589:
        addl      $-16, %esp                                    #5249.20
..LN5591:
        movl      8(%ebp), %eax                                 #5249.37
        movl      %eax, (%esp)                                  #5249.37
..LN5593:
        movl      12(%ebp), %eax                                #5249.52
        movl      %eax, 4(%esp)                                 #5249.52
..LN5595:
        movl      16(%ebp), %eax                                #5249.61
        movl      %eax, 8(%esp)                                 #5249.61
..LN5597:
        movl      20(%ebp), %eax                                #5249.66
        movl      %eax, 12(%esp)                                #5249.66
..LN5599:
        call      eval_2na_128                                  #5249.20
                                # LOE eax
..B30.119:                      # Preds ..B30.46
        addl      $16, %esp                                     #5249.20
        movl      %eax, -60(%ebp)                               #5249.20
                                # LOE
..B30.47:                       # Preds ..B30.119
        movl      -60(%ebp), %eax                               #5249.20
        movl      -4(%ebp), %ebx                                #5249.20
..___tag_value_NucStrstrSearch.1147:                            #
        leave                                                   #5249.20
..___tag_value_NucStrstrSearch.1149:                            #
        ret                                                     #5249.20
..___tag_value_NucStrstrSearch.1150:                            #
                                # LOE
..1.30_0.TAG.06:
..B30.49:                       # Preds ..B30.5
..LN5601:
        movl      8(%ebp), %eax                                 #5251.24
..LN5603:
        movl      20(%ebp), %edx                                #5251.18
..LN5605:
        movl      4(%eax), %eax                                 #5251.24
        cmpl      %eax, %edx                                    #5251.24
        jae       ..B30.51      # Prob 50%                      #5251.24
                                # LOE
..B30.50:                       # Preds ..B30.49
..LN5607:
        xorl      %eax, %eax                                    #5251.54
        movl      -4(%ebp), %ebx                                #5251.54
..___tag_value_NucStrstrSearch.1152:                            #
        leave                                                   #5251.54
..___tag_value_NucStrstrSearch.1154:                            #
        ret                                                     #5251.54
..___tag_value_NucStrstrSearch.1155:                            #
                                # LOE
..B30.51:                       # Preds ..B30.49
..LN5609:
        movl      24(%ebp), %eax                                #5252.9
        testl     %eax, %eax                                    #5252.9
        je        ..B30.53      # Prob 50%                      #5252.9
                                # LOE
..B30.52:                       # Preds ..B30.51
..LN5611:
        movl      8(%ebp), %eax                                 #5252.27
..LN5613:
        movl      24(%ebp), %edx                                #5252.18
..LN5615:
        movl      4(%eax), %eax                                 #5252.27
..LN5617:
        movl      %eax, (%edx)                                  #5252.18
                                # LOE
..B30.53:                       # Preds ..B30.52 ..B30.51
..LN5619:
        addl      $-16, %esp                                    #5253.20
..LN5621:
        movl      8(%ebp), %eax                                 #5253.36
        movl      %eax, (%esp)                                  #5253.36
..LN5623:
        movl      12(%ebp), %eax                                #5253.51
        movl      %eax, 4(%esp)                                 #5253.51
..LN5625:
        movl      16(%ebp), %eax                                #5253.60
        movl      %eax, 8(%esp)                                 #5253.60
..LN5627:
        movl      20(%ebp), %eax                                #5253.65
        movl      %eax, 12(%esp)                                #5253.65
..LN5629:
        call      eval_4na_16                                   #5253.20
                                # LOE eax
..B30.120:                      # Preds ..B30.53
        addl      $16, %esp                                     #5253.20
        movl      %eax, -56(%ebp)                               #5253.20
                                # LOE
..B30.54:                       # Preds ..B30.120
        movl      -56(%ebp), %eax                               #5253.20
        movl      -4(%ebp), %ebx                                #5253.20
..___tag_value_NucStrstrSearch.1157:                            #
        leave                                                   #5253.20
..___tag_value_NucStrstrSearch.1159:                            #
        ret                                                     #5253.20
..___tag_value_NucStrstrSearch.1160:                            #
                                # LOE
..1.30_0.TAG.07:
..B30.56:                       # Preds ..B30.5
..LN5631:
        movl      8(%ebp), %eax                                 #5255.24
..LN5633:
        movl      20(%ebp), %edx                                #5255.18
..LN5635:
        movl      4(%eax), %eax                                 #5255.24
        cmpl      %eax, %edx                                    #5255.24
        jae       ..B30.58      # Prob 50%                      #5255.24
                                # LOE
..B30.57:                       # Preds ..B30.56
..LN5637:
        xorl      %eax, %eax                                    #5255.54
        movl      -4(%ebp), %ebx                                #5255.54
..___tag_value_NucStrstrSearch.1162:                            #
        leave                                                   #5255.54
..___tag_value_NucStrstrSearch.1164:                            #
        ret                                                     #5255.54
..___tag_value_NucStrstrSearch.1165:                            #
                                # LOE
..B30.58:                       # Preds ..B30.56
..LN5639:
        movl      24(%ebp), %eax                                #5256.9
        testl     %eax, %eax                                    #5256.9
        je        ..B30.60      # Prob 50%                      #5256.9
                                # LOE
..B30.59:                       # Preds ..B30.58
..LN5641:
        movl      8(%ebp), %eax                                 #5256.27
..LN5643:
        movl      24(%ebp), %edx                                #5256.18
..LN5645:
        movl      4(%eax), %eax                                 #5256.27
..LN5647:
        movl      %eax, (%edx)                                  #5256.18
                                # LOE
..B30.60:                       # Preds ..B30.59 ..B30.58
..LN5649:
        addl      $-16, %esp                                    #5257.20
..LN5651:
        movl      8(%ebp), %eax                                 #5257.36
        movl      %eax, (%esp)                                  #5257.36
..LN5653:
        movl      12(%ebp), %eax                                #5257.51
        movl      %eax, 4(%esp)                                 #5257.51
..LN5655:
        movl      16(%ebp), %eax                                #5257.60
        movl      %eax, 8(%esp)                                 #5257.60
..LN5657:
        movl      20(%ebp), %eax                                #5257.65
        movl      %eax, 12(%esp)                                #5257.65
..LN5659:
        call      eval_4na_32                                   #5257.20
                                # LOE eax
..B30.121:                      # Preds ..B30.60
        addl      $16, %esp                                     #5257.20
        movl      %eax, -52(%ebp)                               #5257.20
                                # LOE
..B30.61:                       # Preds ..B30.121
        movl      -52(%ebp), %eax                               #5257.20
        movl      -4(%ebp), %ebx                                #5257.20
..___tag_value_NucStrstrSearch.1167:                            #
        leave                                                   #5257.20
..___tag_value_NucStrstrSearch.1169:                            #
        ret                                                     #5257.20
..___tag_value_NucStrstrSearch.1170:                            #
                                # LOE
..1.30_0.TAG.08:
..B30.63:                       # Preds ..B30.5
..LN5661:
        movl      8(%ebp), %eax                                 #5259.24
..LN5663:
        movl      20(%ebp), %edx                                #5259.18
..LN5665:
        movl      4(%eax), %eax                                 #5259.24
        cmpl      %eax, %edx                                    #5259.24
        jae       ..B30.65      # Prob 50%                      #5259.24
                                # LOE
..B30.64:                       # Preds ..B30.63
..LN5667:
        xorl      %eax, %eax                                    #5259.54
        movl      -4(%ebp), %ebx                                #5259.54
..___tag_value_NucStrstrSearch.1172:                            #
        leave                                                   #5259.54
..___tag_value_NucStrstrSearch.1174:                            #
        ret                                                     #5259.54
..___tag_value_NucStrstrSearch.1175:                            #
                                # LOE
..B30.65:                       # Preds ..B30.63
..LN5669:
        movl      24(%ebp), %eax                                #5260.9
        testl     %eax, %eax                                    #5260.9
        je        ..B30.67      # Prob 50%                      #5260.9
                                # LOE
..B30.66:                       # Preds ..B30.65
..LN5671:
        movl      8(%ebp), %eax                                 #5260.27
..LN5673:
        movl      24(%ebp), %edx                                #5260.18
..LN5675:
        movl      4(%eax), %eax                                 #5260.27
..LN5677:
        movl      %eax, (%edx)                                  #5260.18
                                # LOE
..B30.67:                       # Preds ..B30.66 ..B30.65
..LN5679:
        addl      $-16, %esp                                    #5261.20
..LN5681:
        movl      8(%ebp), %eax                                 #5261.37
        movl      %eax, (%esp)                                  #5261.37
..LN5683:
        movl      12(%ebp), %eax                                #5261.52
        movl      %eax, 4(%esp)                                 #5261.52
..LN5685:
        movl      16(%ebp), %eax                                #5261.61
        movl      %eax, 8(%esp)                                 #5261.61
..LN5687:
        movl      20(%ebp), %eax                                #5261.66
        movl      %eax, 12(%esp)                                #5261.66
..LN5689:
        call      eval_4na_128                                  #5261.20
                                # LOE eax
..B30.122:                      # Preds ..B30.67
        addl      $16, %esp                                     #5261.20
        movl      %eax, -48(%ebp)                               #5261.20
                                # LOE
..B30.68:                       # Preds ..B30.122
        movl      -48(%ebp), %eax                               #5261.20
        movl      -4(%ebp), %ebx                                #5261.20
..___tag_value_NucStrstrSearch.1177:                            #
        leave                                                   #5261.20
..___tag_value_NucStrstrSearch.1179:                            #
        ret                                                     #5261.20
..___tag_value_NucStrstrSearch.1180:                            #
                                # LOE
..1.30_0.TAG.09:
..B30.70:                       # Preds ..B30.5
..LN5691:
        movl      8(%ebp), %eax                                 #5264.24
..LN5693:
        movl      20(%ebp), %edx                                #5264.18
..LN5695:
        movl      4(%eax), %eax                                 #5264.24
        cmpl      %eax, %edx                                    #5264.24
        jae       ..B30.72      # Prob 50%                      #5264.24
                                # LOE
..B30.71:                       # Preds ..B30.70
..LN5697:
        xorl      %eax, %eax                                    #5264.54
        movl      -4(%ebp), %ebx                                #5264.54
..___tag_value_NucStrstrSearch.1182:                            #
        leave                                                   #5264.54
..___tag_value_NucStrstrSearch.1184:                            #
        ret                                                     #5264.54
..___tag_value_NucStrstrSearch.1185:                            #
                                # LOE
..B30.72:                       # Preds ..B30.70
..LN5699:
        movl      24(%ebp), %eax                                #5265.9
        testl     %eax, %eax                                    #5265.9
        je        ..B30.74      # Prob 50%                      #5265.9
                                # LOE
..B30.73:                       # Preds ..B30.72
..LN5701:
        movl      8(%ebp), %eax                                 #5265.27
..LN5703:
        movl      24(%ebp), %edx                                #5265.18
..LN5705:
        movl      4(%eax), %eax                                 #5265.27
..LN5707:
        movl      %eax, (%edx)                                  #5265.18
                                # LOE
..B30.74:                       # Preds ..B30.73 ..B30.72
..LN5709:
        addl      $-16, %esp                                    #5266.20
..LN5711:
        movl      8(%ebp), %eax                                 #5266.37
        movl      %eax, (%esp)                                  #5266.37
..LN5713:
        movl      12(%ebp), %eax                                #5266.52
        movl      %eax, 4(%esp)                                 #5266.52
..LN5715:
        movl      16(%ebp), %eax                                #5266.61
        movl      %eax, 8(%esp)                                 #5266.61
..LN5717:
        movl      20(%ebp), %eax                                #5266.66
        movl      %eax, 12(%esp)                                #5266.66
..LN5719:
        call      eval_2na_pos                                  #5266.20
                                # LOE eax
..B30.123:                      # Preds ..B30.74
        addl      $16, %esp                                     #5266.20
        movl      %eax, -44(%ebp)                               #5266.20
                                # LOE
..B30.75:                       # Preds ..B30.123
        movl      -44(%ebp), %eax                               #5266.20
        movl      -4(%ebp), %ebx                                #5266.20
..___tag_value_NucStrstrSearch.1187:                            #
        leave                                                   #5266.20
..___tag_value_NucStrstrSearch.1189:                            #
        ret                                                     #5266.20
..___tag_value_NucStrstrSearch.1190:                            #
                                # LOE
..1.30_0.TAG.0a:
..B30.77:                       # Preds ..B30.5
..LN5721:
        movl      8(%ebp), %eax                                 #5268.24
..LN5723:
        movl      20(%ebp), %edx                                #5268.18
..LN5725:
        movl      4(%eax), %eax                                 #5268.24
        cmpl      %eax, %edx                                    #5268.24
        jae       ..B30.79      # Prob 50%                      #5268.24
                                # LOE
..B30.78:                       # Preds ..B30.77
..LN5727:
        xorl      %eax, %eax                                    #5268.54
        movl      -4(%ebp), %ebx                                #5268.54
..___tag_value_NucStrstrSearch.1192:                            #
        leave                                                   #5268.54
..___tag_value_NucStrstrSearch.1194:                            #
        ret                                                     #5268.54
..___tag_value_NucStrstrSearch.1195:                            #
                                # LOE
..B30.79:                       # Preds ..B30.77
..LN5729:
        movl      24(%ebp), %eax                                #5269.9
        testl     %eax, %eax                                    #5269.9
        je        ..B30.81      # Prob 50%                      #5269.9
                                # LOE
..B30.80:                       # Preds ..B30.79
..LN5731:
        movl      8(%ebp), %eax                                 #5269.27
..LN5733:
        movl      24(%ebp), %edx                                #5269.18
..LN5735:
        movl      4(%eax), %eax                                 #5269.27
..LN5737:
        movl      %eax, (%edx)                                  #5269.18
                                # LOE
..B30.81:                       # Preds ..B30.80 ..B30.79
..LN5739:
        addl      $-16, %esp                                    #5270.20
..LN5741:
        movl      8(%ebp), %eax                                 #5270.37
        movl      %eax, (%esp)                                  #5270.37
..LN5743:
        movl      12(%ebp), %eax                                #5270.52
        movl      %eax, 4(%esp)                                 #5270.52
..LN5745:
        movl      16(%ebp), %eax                                #5270.61
        movl      %eax, 8(%esp)                                 #5270.61
..LN5747:
        movl      20(%ebp), %eax                                #5270.66
        movl      %eax, 12(%esp)                                #5270.66
..LN5749:
        call      eval_4na_pos                                  #5270.20
                                # LOE eax
..B30.124:                      # Preds ..B30.81
        addl      $16, %esp                                     #5270.20
        movl      %eax, -40(%ebp)                               #5270.20
                                # LOE
..B30.82:                       # Preds ..B30.124
        movl      -40(%ebp), %eax                               #5270.20
        movl      -4(%ebp), %ebx                                #5270.20
..___tag_value_NucStrstrSearch.1197:                            #
        leave                                                   #5270.20
..___tag_value_NucStrstrSearch.1199:                            #
        ret                                                     #5270.20
..___tag_value_NucStrstrSearch.1200:                            #
                                # LOE
..1.30_0.TAG.0b:
..B30.84:                       # Preds ..B30.5
..LN5751:
        addl      $-20, %esp                                    #5272.21
..LN5753:
        movl      8(%ebp), %eax                                 #5272.39
        movl      8(%eax), %eax                                 #5272.39
        movl      %eax, (%esp)                                  #5272.39
..LN5755:
        movl      12(%ebp), %eax                                #5272.63
        movl      %eax, 4(%esp)                                 #5272.63
..LN5757:
        movl      16(%ebp), %eax                                #5272.72
        movl      %eax, 8(%esp)                                 #5272.72
..LN5759:
        movl      20(%ebp), %eax                                #5272.77
        movl      %eax, 12(%esp)                                #5272.77
..LN5761:
        movl      24(%ebp), %eax                                #5272.82
        movl      %eax, 16(%esp)                                #5272.82
..LN5763:
        movl      -12(%ebp), %eax                               #5272.21
        movl      %eax, %ebx                                    #5272.21
        call      NucStrstrSearch@PLT                           #5272.21
                                # LOE eax
..B30.125:                      # Preds ..B30.84
        addl      $20, %esp                                     #5272.21
        movl      %eax, -24(%ebp)                               #5272.21
                                # LOE
..B30.85:                       # Preds ..B30.125
..LN5765:
        movl      -24(%ebp), %eax                               #5272.13
        movl      %eax, -28(%ebp)                               #5272.13
..LN5767:
        movl      8(%ebp), %eax                                 #5273.22
        movl      4(%eax), %eax                                 #5273.22
        movl      %eax, -20(%ebp)                               #5273.22
        cmpl      $17, %eax                                     #5273.22
        je        ..B30.87      # Prob 50%                      #5273.22
                                # LOE
..B30.86:                       # Preds ..B30.85
        movl      -20(%ebp), %eax                               #5273.22
        cmpl      $16, %eax                                     #5273.22
        je        ..B30.89      # Prob 50%                      #5273.22
        jmp       ..B30.91      # Prob 100%                     #5273.22
                                # LOE
..B30.87:                       # Preds ..B30.85
..LN5769:
        movl      -28(%ebp), %eax                               #5276.22
..LN5771:
        testl     %eax, %eax                                    #5276.31
        je        ..B30.91      # Prob 50%                      #5276.31
                                # LOE
..B30.88:                       # Preds ..B30.87
..LN5773:
        movl      -28(%ebp), %eax                               #5277.28
        movl      -4(%ebp), %ebx                                #5277.28
..___tag_value_NucStrstrSearch.1202:                            #
        leave                                                   #5277.28
..___tag_value_NucStrstrSearch.1204:                            #
        ret                                                     #5277.28
..___tag_value_NucStrstrSearch.1205:                            #
                                # LOE
..B30.89:                       # Preds ..B30.86
..LN5775:
        movl      -28(%ebp), %eax                               #5280.22
..LN5777:
        testl     %eax, %eax                                    #5280.31
        jne       ..B30.91      # Prob 50%                      #5280.31
                                # LOE
..B30.90:                       # Preds ..B30.89
..LN5779:
        movl      -28(%ebp), %eax                               #5281.28
        movl      -4(%ebp), %ebx                                #5281.28
..___tag_value_NucStrstrSearch.1207:                            #
        leave                                                   #5281.28
..___tag_value_NucStrstrSearch.1209:                            #
        ret                                                     #5281.28
..___tag_value_NucStrstrSearch.1210:                            #
                                # LOE
..B30.91:                       # Preds ..B30.87 ..B30.89 ..B30.86
..LN5781:
        addl      $-20, %esp                                    #5284.20
..LN5783:
        movl      8(%ebp), %eax                                 #5284.38
        movl      12(%eax), %eax                                #5284.38
        movl      %eax, (%esp)                                  #5284.38
..LN5785:
        movl      12(%ebp), %eax                                #5284.63
        movl      %eax, 4(%esp)                                 #5284.63
..LN5787:
        movl      16(%ebp), %eax                                #5284.72
        movl      %eax, 8(%esp)                                 #5284.72
..LN5789:
        movl      20(%ebp), %eax                                #5284.77
        movl      %eax, 12(%esp)                                #5284.77
..LN5791:
        movl      24(%ebp), %eax                                #5284.82
        movl      %eax, 16(%esp)                                #5284.82
..LN5793:
        movl      -12(%ebp), %eax                               #5284.20
        movl      %eax, %ebx                                    #5284.20
        call      NucStrstrSearch@PLT                           #5284.20
                                # LOE eax
..B30.126:                      # Preds ..B30.91
        addl      $20, %esp                                     #5284.20
        movl      %eax, -36(%ebp)                               #5284.20
                                # LOE
..B30.92:                       # Preds ..B30.126
        movl      -36(%ebp), %eax                               #5284.20
        movl      -4(%ebp), %ebx                                #5284.20
..___tag_value_NucStrstrSearch.1212:                            #
        leave                                                   #5284.20
..___tag_value_NucStrstrSearch.1214:                            #
        ret                                                     #5284.20
..___tag_value_NucStrstrSearch.1215:                            #
                                # LOE
..1.30_0.TAG.0c:
..B30.94:                       # Preds ..B30.5
..LN5795:
        movl      8(%ebp), %eax                                 #5286.22
        movl      4(%eax), %eax                                 #5286.22
        movl      %eax, -16(%ebp)                               #5286.22
        testl     %eax, %eax                                    #5286.22
        je        ..B30.98      # Prob 50%                      #5286.22
                                # LOE
..B30.95:                       # Preds ..B30.94
        movl      -16(%ebp), %eax                               #5286.22
        cmpl      $13, %eax                                     #5286.22
        je        ..B30.98      # Prob 50%                      #5286.22
                                # LOE
..B30.96:                       # Preds ..B30.95
        movl      -16(%ebp), %eax                               #5286.22
        cmpl      $14, %eax                                     #5286.22
        je        ..B30.103     # Prob 50%                      #5286.22
                                # LOE
..B30.97:                       # Preds ..B30.96
        movl      -16(%ebp), %eax                               #5286.22
        cmpl      $15, %eax                                     #5286.22
        je        ..B30.107     # Prob 50%                      #5286.22
        jmp       ..B30.111     # Prob 100%                     #5286.22
                                # LOE
..B30.98:                       # Preds ..B30.94 ..B30.95
..LN5797:
        addl      $-20, %esp                                    #5290.25
..LN5799:
        movl      8(%ebp), %eax                                 #5290.43
        movl      8(%eax), %eax                                 #5290.43
        movl      %eax, (%esp)                                  #5290.43
..LN5801:
        movl      12(%ebp), %eax                                #5290.63
        movl      %eax, 4(%esp)                                 #5290.63
..LN5803:
        movl      16(%ebp), %eax                                #5290.72
        movl      %eax, 8(%esp)                                 #5290.72
..LN5805:
        movl      20(%ebp), %eax                                #5290.77
        movl      %eax, 12(%esp)                                #5290.77
..LN5807:
        movl      24(%ebp), %eax                                #5290.82
        movl      %eax, 16(%esp)                                #5290.82
..LN5809:
        movl      -12(%ebp), %eax                               #5290.25
        movl      %eax, %ebx                                    #5290.25
        call      NucStrstrSearch@PLT                           #5290.25
                                # LOE eax
..B30.127:                      # Preds ..B30.98
        addl      $20, %esp                                     #5290.25
        movl      %eax, -32(%ebp)                               #5290.25
                                # LOE
..B30.99:                       # Preds ..B30.127
..LN5811:
        movl      -32(%ebp), %eax                               #5290.17
        movl      %eax, -28(%ebp)                               #5290.17
..LN5813:
        movl      8(%ebp), %eax                                 #5291.22
        movl      4(%eax), %eax                                 #5291.22
..LN5815:
        testl     %eax, %eax                                    #5291.42
        jne       ..B30.101     # Prob 50%                      #5291.42
                                # LOE
..B30.100:                      # Preds ..B30.99
..LN5817:
        movl      -28(%ebp), %eax                               #5292.28
        movl      -4(%ebp), %ebx                                #5292.28
..___tag_value_NucStrstrSearch.1217:                            #
        leave                                                   #5292.28
..___tag_value_NucStrstrSearch.1219:                            #
        ret                                                     #5292.28
..___tag_value_NucStrstrSearch.1220:                            #
                                # LOE
..B30.101:                      # Preds ..B30.99
..LN5819:
        movl      -28(%ebp), %eax                               #5293.22
..LN5821:
        testl     %eax, %eax                                    #5293.31
        jne       ..B30.111     # Prob 50%                      #5293.31
                                # LOE
..B30.102:                      # Preds ..B30.101
..LN5823:
        movl      $1, %eax                                      #5294.28
        movl      -4(%ebp), %ebx                                #5294.28
..___tag_value_NucStrstrSearch.1222:                            #
        leave                                                   #5294.28
..___tag_value_NucStrstrSearch.1224:                            #
        ret                                                     #5294.28
..___tag_value_NucStrstrSearch.1225:                            #
                                # LOE
..B30.103:                      # Preds ..B30.96
..LN5825:
        movl      8(%ebp), %eax                                 #5297.29
        movl      8(%eax), %eax                                 #5297.29
        movl      4(%eax), %eax                                 #5297.29
..LN5827:
        movl      %eax, -84(%ebp)                               #5297.17
..LN5829:
        movl      -84(%ebp), %eax                               #5298.22
..LN5831:
        movl      20(%ebp), %edx                                #5298.34
        cmpl      %edx, %eax                                    #5298.34
        jbe       ..B30.105     # Prob 50%                      #5298.34
                                # LOE
..B30.104:                      # Preds ..B30.103
..LN5833:
        xorl      %eax, %eax                                    #5299.28
        movl      -4(%ebp), %ebx                                #5299.28
..___tag_value_NucStrstrSearch.1227:                            #
        leave                                                   #5299.28
..___tag_value_NucStrstrSearch.1229:                            #
        ret                                                     #5299.28
..___tag_value_NucStrstrSearch.1230:                            #
                                # LOE
..B30.105:                      # Preds ..B30.103
..LN5835:
        addl      $-20, %esp                                    #5300.24
..LN5837:
        movl      8(%ebp), %eax                                 #5300.42
        movl      8(%eax), %eax                                 #5300.42
        movl      %eax, (%esp)                                  #5300.42
..LN5839:
        movl      12(%ebp), %eax                                #5300.62
        movl      %eax, 4(%esp)                                 #5300.62
..LN5841:
        movl      16(%ebp), %eax                                #5300.71
        movl      %eax, 8(%esp)                                 #5300.71
..LN5843:
        movl      -84(%ebp), %eax                               #5300.76
        movl      %eax, 12(%esp)                                #5300.76
..LN5845:
        movl      24(%ebp), %eax                                #5300.87
        movl      %eax, 16(%esp)                                #5300.87
..LN5847:
        movl      -12(%ebp), %eax                               #5300.24
        movl      %eax, %ebx                                    #5300.24
        call      NucStrstrSearch@PLT                           #5300.24
                                # LOE eax
..B30.128:                      # Preds ..B30.105
        addl      $20, %esp                                     #5300.24
        movl      %eax, -88(%ebp)                               #5300.24
                                # LOE
..B30.106:                      # Preds ..B30.128
        movl      -88(%ebp), %eax                               #5300.24
        movl      -4(%ebp), %ebx                                #5300.24
..___tag_value_NucStrstrSearch.1232:                            #
        leave                                                   #5300.24
..___tag_value_NucStrstrSearch.1234:                            #
        ret                                                     #5300.24
..___tag_value_NucStrstrSearch.1235:                            #
                                # LOE
..B30.107:                      # Preds ..B30.97
..LN5849:
        movl      8(%ebp), %eax                                 #5302.29
        movl      8(%eax), %eax                                 #5302.29
        movl      4(%eax), %eax                                 #5302.29
..LN5851:
        movl      %eax, -84(%ebp)                               #5302.17
..LN5853:
        movl      -84(%ebp), %eax                               #5303.22
..LN5855:
        movl      20(%ebp), %edx                                #5303.34
        cmpl      %edx, %eax                                    #5303.34
        jbe       ..B30.109     # Prob 50%                      #5303.34
                                # LOE
..B30.108:                      # Preds ..B30.107
..LN5857:
        xorl      %eax, %eax                                    #5304.28
        movl      -4(%ebp), %ebx                                #5304.28
..___tag_value_NucStrstrSearch.1237:                            #
        leave                                                   #5304.28
..___tag_value_NucStrstrSearch.1239:                            #
        ret                                                     #5304.28
..___tag_value_NucStrstrSearch.1240:                            #
                                # LOE
..B30.109:                      # Preds ..B30.107
..LN5859:
        addl      $-20, %esp                                    #5305.24
..LN5861:
        movl      8(%ebp), %eax                                 #5305.42
        movl      8(%eax), %eax                                 #5305.42
        movl      %eax, (%esp)                                  #5305.42
..LN5863:
        movl      12(%ebp), %eax                                #5305.62
        movl      %eax, 4(%esp)                                 #5305.62
..LN5865:
        movl      20(%ebp), %eax                                #5306.27
        addl      16(%ebp), %eax                                #5306.27
..LN5867:
        movl      -84(%ebp), %edx                               #5306.33
        negl      %edx                                          #5306.33
        addl      %edx, %eax                                    #5306.33
        movl      %eax, 8(%esp)                                 #5306.33
..LN5869:
        movl      -84(%ebp), %eax                               #5306.44
        movl      %eax, 12(%esp)                                #5306.44
..LN5871:
        movl      24(%ebp), %eax                                #5306.55
        movl      %eax, 16(%esp)                                #5306.55
..LN5873:
        movl      -12(%ebp), %eax                               #5305.24
        movl      %eax, %ebx                                    #5305.24
        call      NucStrstrSearch@PLT                           #5305.24
                                # LOE eax
..B30.129:                      # Preds ..B30.109
        addl      $20, %esp                                     #5305.24
        movl      %eax, -92(%ebp)                               #5305.24
                                # LOE
..B30.110:                      # Preds ..B30.129
        movl      -92(%ebp), %eax                               #5305.24
        movl      -4(%ebp), %ebx                                #5305.24
..___tag_value_NucStrstrSearch.1242:                            #
        leave                                                   #5305.24
..___tag_value_NucStrstrSearch.1244:                            #
        ret                                                     #5305.24
..___tag_value_NucStrstrSearch.1245:                            #
                                # LOE
..B30.111:                      # Preds ..B30.101 ..B30.97 ..B30.4 ..B30.3 ..B30.2
                                #       ..B30.1
..LN5875:
        xorl      %eax, %eax                                    #5311.12
        movl      -4(%ebp), %ebx                                #5311.12
..___tag_value_NucStrstrSearch.1247:                            #
        leave                                                   #5311.12
..___tag_value_NucStrstrSearch.1249:                            #
        ret                                                     #5311.12
        .align    2,0x90
..___tag_value_NucStrstrSearch.1250:                            #
                                # LOE
# mark_end;
	.type	NucStrstrSearch,@function
	.size	NucStrstrSearch,.-NucStrstrSearch
.LNNucStrstrSearch:
	.section .data1, "wa"
	.align 4
..1..TPKT.30_0:
	.long	..1.30_0.TAG.00
	.long	..1.30_0.TAG.01
	.long	..1.30_0.TAG.02
	.long	..1.30_0.TAG.03
	.long	..1.30_0.TAG.04
	.long	..1.30_0.TAG.05
	.long	..1.30_0.TAG.06
	.long	..1.30_0.TAG.07
	.long	..1.30_0.TAG.08
	.long	..1.30_0.TAG.09
	.long	..1.30_0.TAG.0a
	.long	..1.30_0.TAG.0b
	.long	..1.30_0.TAG.0c
	.data
# -- End  NucStrstrSearch
	.data
	.section .note.GNU-stack, ""
// -- Begin DWARF2 SEGMENT .debug_info
	.section .debug_info
.debug_info_seg:
	.align 1
	.4byte 0x00002957
	.2byte 0x0002
	.4byte .debug_abbrev_seg
	.byte 0x04
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
	.8byte 0x6f662072656c6970
	.8byte 0x63696c7070612072
	.8byte 0x7220736e6f697461
	.8byte 0x6f20676e696e6e75
	.8byte 0x2c32332d4149206e
	.8byte 0x6e6f697372655620
	.8byte 0x202020312e303120
	.8byte 0x3220646c69754220
	.8byte 0x2034323031383030
	.8byte 0x65786946200a7325
	.8byte 0x7365676e61522073
	.8byte 0x65766974616c6552
	.2byte 0x000a
//	DW_AT_stmt_list:
	.4byte .debug_line_seg
.DWinfo31:
//	DW_TAG_enumeration_type:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_byte_size:
	.byte 0x04
//	DW_AT_encoding:
	.byte 0x07
//	DW_AT_sibling:
	.4byte 0x0000015b
.DWinfo32:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x726570707553495f
	.byte 0x00
//	DW_AT_const_value:
	.2byte 0x0280
.DWinfo33:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x7265776f6c53495f
	.byte 0x00
//	DW_AT_const_value:
	.2byte 0x0480
.DWinfo34:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x6168706c6153495f
	.byte 0x00
//	DW_AT_const_value:
	.2byte 0x0880
.DWinfo35:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x746967696453495f
	.byte 0x00
//	DW_AT_const_value:
	.2byte 0x1080
.DWinfo36:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x696769647853495f
	.2byte 0x0074
//	DW_AT_const_value:
	.2byte 0x2080
.DWinfo37:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x656361707353495f
	.byte 0x00
//	DW_AT_const_value:
	.2byte 0xc080
	.byte 0x00
.DWinfo38:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x746e69727053495f
	.byte 0x00
//	DW_AT_const_value:
	.2byte 0x8080
	.byte 0x01
.DWinfo39:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x687061726753495f
	.byte 0x00
//	DW_AT_const_value:
	.2byte 0x8080
	.byte 0x02
.DWinfo40:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x6b6e616c6253495f
	.byte 0x00
//	DW_AT_const_value:
	.byte 0x01
.DWinfo41:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x6c72746e6353495f
	.byte 0x00
//	DW_AT_const_value:
	.byte 0x02
.DWinfo42:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x74636e757053495f
	.byte 0x00
//	DW_AT_const_value:
	.byte 0x04
.DWinfo43:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x6d756e6c6153495f
	.byte 0x00
//	DW_AT_const_value:
	.byte 0x08
	.byte 0x00
.DWinfo44:
//	DW_TAG_enumeration_type:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_byte_size:
	.byte 0x04
//	DW_AT_encoding:
	.byte 0x07
//	DW_AT_sibling:
	.4byte 0x00000245
.DWinfo45:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x616e325f65707974
	.4byte 0x0034365f
//	DW_AT_const_value:
	.byte 0x00
.DWinfo46:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x616e345f65707974
	.4byte 0x0034365f
//	DW_AT_const_value:
	.byte 0x01
.DWinfo47:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x616e325f65707974
	.2byte 0x385f
	.byte 0x00
//	DW_AT_const_value:
	.byte 0x02
.DWinfo48:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x616e325f65707974
	.4byte 0x0036315f
//	DW_AT_const_value:
	.byte 0x03
.DWinfo49:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x616e325f65707974
	.4byte 0x0032335f
//	DW_AT_const_value:
	.byte 0x04
.DWinfo50:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x616e325f65707974
	.4byte 0x3832315f
	.byte 0x00
//	DW_AT_const_value:
	.byte 0x05
.DWinfo51:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x616e345f65707974
	.4byte 0x0036315f
//	DW_AT_const_value:
	.byte 0x06
.DWinfo52:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x616e345f65707974
	.4byte 0x0032335f
//	DW_AT_const_value:
	.byte 0x07
.DWinfo53:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x616e345f65707974
	.4byte 0x3832315f
	.byte 0x00
//	DW_AT_const_value:
	.byte 0x08
.DWinfo54:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x616e325f65707974
	.4byte 0x736f705f
	.byte 0x00
//	DW_AT_const_value:
	.byte 0x09
.DWinfo55:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x616e345f65707974
	.4byte 0x736f705f
	.byte 0x00
//	DW_AT_const_value:
	.byte 0x0a
.DWinfo56:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x00504f5f65707974
//	DW_AT_const_value:
	.byte 0x0b
.DWinfo57:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x5058455f65707974
	.2byte 0x0052
//	DW_AT_const_value:
	.byte 0x0c
.DWinfo58:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.4byte 0x4e5f706f
	.2byte 0x544f
	.byte 0x00
//	DW_AT_const_value:
	.byte 0x0d
.DWinfo59:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x00444145485f706f
//	DW_AT_const_value:
	.byte 0x0e
.DWinfo60:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.8byte 0x004c4941545f706f
//	DW_AT_const_value:
	.byte 0x0f
.DWinfo61:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.4byte 0x415f706f
	.2byte 0x444e
	.byte 0x00
//	DW_AT_const_value:
	.byte 0x10
.DWinfo62:
//	DW_TAG_enumerator:
	.byte 0x03
//	DW_AT_name:
	.4byte 0x4f5f706f
	.2byte 0x0052
//	DW_AT_const_value:
	.byte 0x11
	.byte 0x00
.DWinfo63:
//	DW_TAG_typedef:
	.byte 0x04
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
	.4byte 0x00000256
.DWinfo64:
//	DW_TAG_base_type:
	.byte 0x05
//	DW_AT_byte_size:
	.byte 0x02
//	DW_AT_encoding:
	.byte 0x05
//	DW_AT_name:
	.4byte 0x726f6873
	.2byte 0x0074
.DWinfo1:
//	DW_TAG_subprogram:
	.byte 0x06
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
	.4byte 0x00000245
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
	.4byte uint16_lsbit
//	DW_AT_high_pc:
	.4byte .LNuint16_lsbit
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x000002b2
.DWinfo65:
//	DW_TAG_formal_parameter:
	.byte 0x07
//	DW_AT_decl_line:
	.byte 0x25
//	DW_AT_decl_column:
	.byte 0x21
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x000002b2
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x08
.DWinfo66:
//	DW_TAG_variable:
	.byte 0x08
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
	.4byte 0x00000245
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x7c
	.byte 0x00
.DWinfo67:
//	DW_TAG_typedef:
	.byte 0x04
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
	.4byte 0x000002c4
.DWinfo68:
//	DW_TAG_base_type:
	.byte 0x05
//	DW_AT_byte_size:
	.byte 0x02
//	DW_AT_encoding:
	.byte 0x07
//	DW_AT_name:
	.8byte 0x64656e6769736e75
	.4byte 0x6f687320
	.2byte 0x7472
	.byte 0x00
.DWinfo69:
//	DW_TAG_base_type:
	.byte 0x05
//	DW_AT_byte_size:
	.byte 0x00
//	DW_AT_encoding:
	.byte 0x05
//	DW_AT_name:
	.4byte 0x64696f76
	.byte 0x00
.DWinfo2:
//	DW_TAG_subprogram:
	.byte 0x09
//	DW_AT_decl_line:
	.2byte 0x0126
//	DW_AT_decl_column:
	.byte 0x06
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_inline:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x000002d6
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x5f383231746e6975
	.4byte 0x00726873
	.8byte 0x5f383231746e6975
	.4byte 0x00726873
//	DW_AT_low_pc:
	.4byte uint128_shr
//	DW_AT_high_pc:
	.4byte .LNuint128_shr
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x0000032f
.DWinfo70:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0126
//	DW_AT_decl_column:
	.byte 0x1f
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x0000032f
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x08
.DWinfo71:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0126
//	DW_AT_decl_column:
	.byte 0x2e
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x000003a0
//	DW_AT_name:
	.2byte 0x0069
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x0c
	.byte 0x00
.DWinfo72:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x00000334
.DWinfo73:
//	DW_TAG_typedef:
	.byte 0x04
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
	.4byte 0x00000347
.DWinfo74:
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
//	DW_AT_sibling:
	.4byte 0x00000378
.DWinfo75:
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
	.4byte 0x00000378
.DWinfo76:
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
	.4byte 0x00000378
	.byte 0x00
.DWinfo77:
//	DW_TAG_typedef:
	.byte 0x04
//	DW_AT_decl_line:
	.byte 0x3b
//	DW_AT_decl_column:
	.byte 0x20
//	DW_AT_decl_file:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x745f3436746e6975
	.byte 0x00
//	DW_AT_type:
	.4byte 0x0000038a
.DWinfo78:
//	DW_TAG_base_type:
	.byte 0x05
//	DW_AT_byte_size:
	.byte 0x08
//	DW_AT_encoding:
	.byte 0x07
//	DW_AT_name:
	.8byte 0x64656e6769736e75
	.8byte 0x6f6c20676e6f6c20
	.2byte 0x676e
	.byte 0x00
.DWinfo79:
//	DW_TAG_typedef:
	.byte 0x04
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
	.4byte 0x000003b2
.DWinfo80:
//	DW_TAG_base_type:
	.byte 0x05
//	DW_AT_byte_size:
	.byte 0x04
//	DW_AT_encoding:
	.byte 0x07
//	DW_AT_name:
	.8byte 0x64656e6769736e75
	.4byte 0x746e6920
	.byte 0x00
.DWinfo3:
//	DW_TAG_subprogram:
	.byte 0x09
//	DW_AT_decl_line:
	.2byte 0x014c
//	DW_AT_decl_column:
	.byte 0x06
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_inline:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x000002d6
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
	.4byte uint128_bswap
//	DW_AT_high_pc:
	.4byte .LNuint128_bswap
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x00000409
.DWinfo81:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x014c
//	DW_AT_decl_column:
	.byte 0x21
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x0000032f
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x08
	.byte 0x00
.DWinfo4:
//	DW_TAG_subprogram:
	.byte 0x09
//	DW_AT_decl_line:
	.2byte 0x0163
//	DW_AT_decl_column:
	.byte 0x06
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_inline:
	.byte 0x02
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x000002d6
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
	.4byte uint128_bswap_copy
//	DW_AT_high_pc:
	.4byte .LNuint128_bswap_copy
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x00000469
.DWinfo82:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0163
//	DW_AT_decl_column:
	.byte 0x26
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x0000032f
//	DW_AT_name:
	.2byte 0x6f74
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x08
.DWinfo83:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0163
//	DW_AT_decl_column:
	.byte 0x3b
//	DW_AT_decl_file:
	.byte 0x01
//	DW_AT_type:
	.4byte 0x00000469
//	DW_AT_name:
	.4byte 0x6d6f7266
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x0c
	.byte 0x00
.DWinfo84:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x0000046e
.DWinfo85:
//	DW_TAG_const_type:
	.byte 0x0e
//	DW_AT_type:
	.4byte 0x00000334
.DWinfo86:
//	DW_TAG_typedef:
	.byte 0x04
//	DW_AT_decl_line:
	.byte 0x2b
//	DW_AT_decl_column:
	.byte 0x36
//	DW_AT_decl_file:
	.byte 0x04
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x00693832316d5f5f
//	DW_AT_type:
	.4byte 0x00000484
.DWinfo87:
//	DW_TAG_array_type:
	.byte 0x0f
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x00693832316d5f5f
//	DW_AT_type:
	.4byte 0x0000038a
//	DW_AT_byte_size:
	.byte 0x10
//	DW_AT_sibling:
	.4byte 0x000004a4
.DWinfo88:
//	DW_TAG_subrange_type:
	.byte 0x10
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x00693832316d5f5f
//	DW_AT_upper_bound:
	.byte 0x01
//	DW_AT_lower_bound:
	.byte 0x00
	.byte 0x00
.DWinfo5:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000473
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
	.4byte prime_buffer_2na
//	DW_AT_high_pc:
	.4byte .LNprime_buffer_2na
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x00000517
.DWinfo89:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0850
//	DW_AT_decl_column:
	.byte 0x2b
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000517
//	DW_AT_name:
	.4byte 0x00637273
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x08
.DWinfo90:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0850
//	DW_AT_decl_column:
	.byte 0x3f
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000517
//	DW_AT_name:
	.4byte 0x6f6e6769
	.2byte 0x6572
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x0c
.DWinfo91:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000473
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x68
	.byte 0x00
.DWinfo92:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x0000051c
.DWinfo93:
//	DW_TAG_const_type:
	.byte 0x0e
//	DW_AT_type:
	.4byte 0x00000521
.DWinfo94:
//	DW_TAG_typedef:
	.byte 0x04
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
	.4byte 0x00000532
.DWinfo95:
//	DW_TAG_base_type:
	.byte 0x05
//	DW_AT_byte_size:
	.byte 0x01
//	DW_AT_encoding:
	.byte 0x08
//	DW_AT_name:
	.8byte 0x64656e6769736e75
	.4byte 0x61686320
	.2byte 0x0072
.DWinfo6:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000473
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
	.4byte prime_buffer_4na
//	DW_AT_high_pc:
	.4byte .LNprime_buffer_4na
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x000005c7
.DWinfo96:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0f95
//	DW_AT_decl_column:
	.byte 0x2b
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000517
//	DW_AT_name:
	.4byte 0x00637273
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x08
.DWinfo97:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0f95
//	DW_AT_decl_column:
	.byte 0x3f
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000517
//	DW_AT_name:
	.4byte 0x6f6e6769
	.2byte 0x6572
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x0c
.DWinfo98:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000005c7
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x58
.DWinfo99:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000473
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x68
	.byte 0x00
.DWinfo100:
//	DW_TAG_typedef:
	.byte 0x04
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
	.4byte 0x000005d9
.DWinfo101:
//	DW_TAG_union_type:
	.byte 0x12
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
//	DW_AT_sibling:
	.4byte 0x0000062a
.DWinfo102:
//	DW_TAG_member:
	.byte 0x13
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
	.4byte 0x0000062a
.DWinfo103:
//	DW_TAG_member:
	.byte 0x13
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
	.4byte 0x00000637
.DWinfo104:
//	DW_TAG_member:
	.byte 0x13
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
	.4byte 0x00000644
.DWinfo105:
//	DW_TAG_member:
	.byte 0x13
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
	.4byte 0x00000651
.DWinfo106:
//	DW_TAG_member:
	.byte 0x13
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
	.4byte 0x00000334
	.byte 0x00
.DWinfo107:
//	DW_TAG_array_type:
	.byte 0x14
//	DW_AT_type:
	.4byte 0x00000521
//	DW_AT_byte_size:
	.byte 0x10
//	DW_AT_sibling:
	.4byte 0x00000637
.DWinfo108:
//	DW_TAG_subrange_type:
	.byte 0x15
//	DW_AT_upper_bound:
	.byte 0x0f
	.byte 0x00
.DWinfo109:
//	DW_TAG_array_type:
	.byte 0x14
//	DW_AT_type:
	.4byte 0x000002b2
//	DW_AT_byte_size:
	.byte 0x10
//	DW_AT_sibling:
	.4byte 0x00000644
.DWinfo110:
//	DW_TAG_subrange_type:
	.byte 0x15
//	DW_AT_upper_bound:
	.byte 0x07
	.byte 0x00
.DWinfo111:
//	DW_TAG_array_type:
	.byte 0x14
//	DW_AT_type:
	.4byte 0x000003a0
//	DW_AT_byte_size:
	.byte 0x10
//	DW_AT_sibling:
	.4byte 0x00000651
.DWinfo112:
//	DW_TAG_subrange_type:
	.byte 0x15
//	DW_AT_upper_bound:
	.byte 0x03
	.byte 0x00
.DWinfo113:
//	DW_TAG_array_type:
	.byte 0x14
//	DW_AT_type:
	.4byte 0x00000378
//	DW_AT_byte_size:
	.byte 0x10
//	DW_AT_sibling:
	.4byte 0x0000065e
.DWinfo114:
//	DW_TAG_subrange_type:
	.byte 0x15
//	DW_AT_upper_bound:
	.byte 0x01
	.byte 0x00
.DWinfo115:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x000002d6
.DWinfo7:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x0000065e
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
	.4byte NucStrFastaExprAlloc
//	DW_AT_high_pc:
	.4byte .LNNucStrFastaExprAlloc
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x000006e6
.DWinfo116:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0315
//	DW_AT_decl_column:
	.byte 0x25
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000006e6
//	DW_AT_name:
	.2byte 0x7a73
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x08
.DWinfo117:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x0000065e
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x70
.DWinfo118:
//	DW_TAG_lexical_block:
	.byte 0x16
//	DW_AT_decl_line:
	.2byte 0x0319
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.4byte ..LN121
//	DW_AT_high_pc:
	.4byte ..LN135
.DWinfo119:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000006f6
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x6c
	.byte 0x00
	.byte 0x00
.DWinfo120:
//	DW_TAG_typedef:
	.byte 0x04
//	DW_AT_decl_line:
	.byte 0x20
//	DW_AT_decl_column:
	.byte 0x17
//	DW_AT_decl_file:
	.byte 0x05
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.4byte 0x657a6973
	.2byte 0x745f
	.byte 0x00
//	DW_AT_type:
	.4byte 0x000003b2
.DWinfo121:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x000006fb
.DWinfo122:
//	DW_TAG_typedef:
	.byte 0x17
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
	.4byte 0x00000715
.DWinfo123:
//	DW_TAG_structure_type:
	.byte 0x18
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
//	DW_AT_sibling:
	.4byte 0x00000773
.DWinfo124:
//	DW_TAG_member:
	.byte 0x19
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
	.4byte 0x00000773
.DWinfo125:
//	DW_TAG_member:
	.byte 0x19
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
	.4byte 0x000003a0
.DWinfo126:
//	DW_TAG_member:
	.byte 0x19
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
	.4byte 0x0000078b
.DWinfo127:
//	DW_TAG_member:
	.byte 0x19
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
	.4byte 0x000007bd
	.byte 0x00
.DWinfo128:
//	DW_TAG_typedef:
	.byte 0x04
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
	.4byte 0x00000784
.DWinfo129:
//	DW_TAG_base_type:
	.byte 0x05
//	DW_AT_byte_size:
	.byte 0x04
//	DW_AT_encoding:
	.byte 0x05
//	DW_AT_name:
	.4byte 0x00746e69
.DWinfo130:
//	DW_TAG_union_type:
	.byte 0x1a
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
//	DW_AT_sibling:
	.4byte 0x000007bd
.DWinfo131:
//	DW_TAG_member:
	.byte 0x1b
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
	.4byte 0x0000065e
.DWinfo132:
//	DW_TAG_member:
	.byte 0x1b
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
	.4byte 0x00000378
	.byte 0x00
.DWinfo133:
//	DW_TAG_array_type:
	.byte 0x14
//	DW_AT_type:
	.4byte 0x000007ca
//	DW_AT_byte_size:
	.byte 0x80
//	DW_AT_sibling:
	.4byte 0x000007ca
.DWinfo134:
//	DW_TAG_subrange_type:
	.byte 0x15
//	DW_AT_upper_bound:
	.byte 0x03
	.byte 0x00
.DWinfo135:
//	DW_TAG_structure_type:
	.byte 0x1c
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
//	DW_AT_sibling:
	.4byte 0x000007fb
.DWinfo136:
//	DW_TAG_member:
	.byte 0x19
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
	.4byte 0x000005c7
.DWinfo137:
//	DW_TAG_member:
	.byte 0x19
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
	.4byte 0x000005c7
	.byte 0x00
.DWinfo8:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000784
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
	.4byte NucStrFastaExprMake2
//	DW_AT_high_pc:
	.4byte .LNNucStrFastaExprMake2
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x000008ef
.DWinfo138:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0344
//	DW_AT_decl_column:
	.byte 0x29
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000008ef
//	DW_AT_name:
	.4byte 0x72707865
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x08
.DWinfo139:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0344
//	DW_AT_decl_column:
	.byte 0x33
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000784
//	DW_AT_name:
	.8byte 0x6e6f697469736f70
	.2byte 0x6c61
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x0c
.DWinfo140:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0345
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000a46
//	DW_AT_name:
	.4byte 0x74736166
	.2byte 0x0061
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x10
.DWinfo141:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0345
//	DW_AT_decl_column:
	.byte 0x1f
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000006e6
//	DW_AT_name:
	.4byte 0x657a6973
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x14
.DWinfo142:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000006e6
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x64
.DWinfo143:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000008f4
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x6c
.DWinfo144:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000005c7
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x44
.DWinfo145:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000005c7
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x54
.DWinfo146:
//	DW_TAG_lexical_block:
	.byte 0x16
//	DW_AT_decl_line:
	.2byte 0x0359
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.4byte ..LN191
//	DW_AT_high_pc:
	.4byte ..LN301
.DWinfo147:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000521
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x78
	.byte 0x00
	.byte 0x00
.DWinfo148:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x000008f4
.DWinfo149:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x000008f9
.DWinfo150:
//	DW_TAG_typedef:
	.byte 0x17
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
	.4byte 0x0000090e
.DWinfo151:
//	DW_TAG_typedef:
	.byte 0x04
//	DW_AT_decl_line:
	.byte 0x44
//	DW_AT_decl_column:
	.byte 0x19
//	DW_AT_decl_file:
	.byte 0x06
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x747372745363754e
	.2byte 0x0072
//	DW_AT_type:
	.4byte 0x00000921
.DWinfo152:
//	DW_TAG_union_type:
	.byte 0x1d
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
//	DW_AT_sibling:
	.4byte 0x00000970
.DWinfo153:
//	DW_TAG_member:
	.byte 0x1b
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
	.4byte 0x000006fb
.DWinfo154:
//	DW_TAG_member:
	.byte 0x1b
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
	.4byte 0x00000970
.DWinfo155:
//	DW_TAG_member:
	.byte 0x1b
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
	.4byte 0x000009e3
	.byte 0x00
.DWinfo156:
//	DW_TAG_typedef:
	.byte 0x17
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
	.4byte 0x00000987
.DWinfo157:
//	DW_TAG_structure_type:
	.byte 0x18
//	DW_AT_decl_line:
	.2byte 0x0329
//	DW_AT_decl_column:
	.byte 0x08
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_byte_size:
	.byte 0x10
//	DW_AT_name:
	.8byte 0x704f72745363754e
	.4byte 0x72707845
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x000009e3
.DWinfo158:
//	DW_TAG_member:
	.byte 0x19
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
	.4byte 0x00000773
.DWinfo159:
//	DW_TAG_member:
	.byte 0x19
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
	.4byte 0x00000773
.DWinfo160:
//	DW_TAG_member:
	.byte 0x19
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
	.4byte 0x000008f4
.DWinfo161:
//	DW_TAG_member:
	.byte 0x19
//	DW_AT_decl_line:
	.2byte 0x032e
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_data_member_location:
	.2byte 0x2302
	.byte 0x0c
//	DW_AT_name:
	.4byte 0x68676972
	.2byte 0x0074
//	DW_AT_type:
	.4byte 0x000008f4
	.byte 0x00
.DWinfo162:
//	DW_TAG_typedef:
	.byte 0x17
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
	.4byte 0x000009fb
.DWinfo163:
//	DW_TAG_structure_type:
	.byte 0x18
//	DW_AT_decl_line:
	.2byte 0x0332
//	DW_AT_decl_column:
	.byte 0x08
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_accessibility:
	.byte 0x01
//	DW_AT_byte_size:
	.byte 0x0c
//	DW_AT_name:
	.8byte 0x755372745363754e
	.4byte 0x70784562
	.2byte 0x0072
//	DW_AT_sibling:
	.4byte 0x00000a46
.DWinfo164:
//	DW_TAG_member:
	.byte 0x19
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
	.4byte 0x00000773
.DWinfo165:
//	DW_TAG_member:
	.byte 0x19
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
	.4byte 0x00000773
.DWinfo166:
//	DW_TAG_member:
	.byte 0x19
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
	.4byte 0x000008f4
	.byte 0x00
.DWinfo167:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x00000a4b
.DWinfo168:
//	DW_TAG_const_type:
	.byte 0x0e
//	DW_AT_type:
	.4byte 0x00000a50
.DWinfo169:
//	DW_TAG_base_type:
	.byte 0x05
//	DW_AT_byte_size:
	.byte 0x01
//	DW_AT_encoding:
	.byte 0x06
//	DW_AT_name:
	.4byte 0x72616863
	.byte 0x00
.DWinfo9:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000784
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
	.4byte NucStrFastaExprMake4
//	DW_AT_high_pc:
	.4byte .LNNucStrFastaExprMake4
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x00000b4c
.DWinfo170:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x03f7
//	DW_AT_decl_column:
	.byte 0x29
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000008ef
//	DW_AT_name:
	.4byte 0x72707865
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x08
.DWinfo171:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x03f7
//	DW_AT_decl_column:
	.byte 0x33
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000784
//	DW_AT_name:
	.8byte 0x6e6f697469736f70
	.2byte 0x6c61
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x0c
.DWinfo172:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x03f8
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000a46
//	DW_AT_name:
	.4byte 0x74736166
	.2byte 0x0061
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x10
.DWinfo173:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x03f8
//	DW_AT_decl_column:
	.byte 0x1f
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000006e6
//	DW_AT_name:
	.4byte 0x657a6973
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x14
.DWinfo174:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000006e6
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x64
.DWinfo175:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000008f4
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x70
.DWinfo176:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000005c7
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x44
.DWinfo177:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000005c7
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x54
.DWinfo178:
//	DW_TAG_lexical_block:
	.byte 0x16
//	DW_AT_decl_line:
	.2byte 0x040c
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.4byte ..LN621
//	DW_AT_high_pc:
	.4byte ..LN731
.DWinfo179:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000002b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x6c
	.byte 0x00
	.byte 0x00
.DWinfo10:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000a46
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x00626f735f73736e
	.8byte 0x00626f735f73736e
//	DW_AT_low_pc:
	.4byte nss_sob
//	DW_AT_high_pc:
	.4byte .LNnss_sob
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x00000b94
.DWinfo180:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x049c
//	DW_AT_decl_column:
	.byte 0x23
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000a46
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x08
.DWinfo181:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x049c
//	DW_AT_decl_column:
	.byte 0x32
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000a46
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x0c
	.byte 0x00
.DWinfo11:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000a46
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
	.4byte nss_FASTA_expr
//	DW_AT_high_pc:
	.4byte .LNnss_FASTA_expr
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x00000c69
.DWinfo182:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x04a6
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000a46
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x08
.DWinfo183:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x04a6
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000a46
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x0c
.DWinfo184:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x04a7
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000008ef
//	DW_AT_name:
	.4byte 0x72707865
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x10
.DWinfo185:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x04a7
//	DW_AT_decl_column:
	.byte 0x1d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000c69
//	DW_AT_name:
	.4byte 0x74617473
	.2byte 0x7375
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x14
.DWinfo186:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x04a7
//	DW_AT_decl_column:
	.byte 0x29
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000784
//	DW_AT_name:
	.8byte 0x6e6f697469736f70
	.2byte 0x6c61
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x18
.DWinfo187:
//	DW_TAG_lexical_block:
	.byte 0x16
//	DW_AT_decl_line:
	.2byte 0x04ac
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.4byte ..LN985
//	DW_AT_high_pc:
	.4byte ..LN1061
.DWinfo188:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000a46
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x70
.DWinfo189:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000773
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x74
.DWinfo190:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000c6e
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x78
	.byte 0x00
	.byte 0x00
.DWinfo191:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x00000784
.DWinfo192:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x00000c73
.DWinfo193:
//	DW_TAG_const_type:
	.byte 0x0e
//	DW_AT_type:
	.4byte 0x00000c78
.DWinfo194:
//	DW_TAG_typedef:
	.byte 0x04
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
	.4byte 0x00000a50
.DWinfo12:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000a46
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
	.4byte nss_fasta_expr
//	DW_AT_high_pc:
	.4byte .LNnss_fasta_expr
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x00000d19
.DWinfo195:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x04d0
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000a46
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x08
.DWinfo196:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x04d0
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000a46
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x0c
.DWinfo197:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x04d1
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000008ef
//	DW_AT_name:
	.4byte 0x72707865
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x10
.DWinfo198:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x04d1
//	DW_AT_decl_column:
	.byte 0x1d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000c69
//	DW_AT_name:
	.4byte 0x74617473
	.2byte 0x7375
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x14
.DWinfo199:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x04d1
//	DW_AT_decl_column:
	.byte 0x29
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000784
//	DW_AT_name:
	.8byte 0x6e6f697469736f70
	.2byte 0x6c61
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x18
	.byte 0x00
.DWinfo13:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000a46
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
	.4byte nss_primary_expr
//	DW_AT_high_pc:
	.4byte .LNnss_primary_expr
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x00000dbd
.DWinfo200:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0509
//	DW_AT_decl_column:
	.byte 0x2c
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000a46
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x08
.DWinfo201:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0509
//	DW_AT_decl_column:
	.byte 0x3b
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000a46
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x0c
.DWinfo202:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x050a
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000008ef
//	DW_AT_name:
	.4byte 0x72707865
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x10
.DWinfo203:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x050a
//	DW_AT_decl_column:
	.byte 0x1d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000c69
//	DW_AT_name:
	.4byte 0x74617473
	.2byte 0x7375
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x14
.DWinfo204:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x050a
//	DW_AT_decl_column:
	.byte 0x29
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000784
//	DW_AT_name:
	.8byte 0x6e6f697469736f70
	.2byte 0x6c61
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x18
.DWinfo205:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000008f4
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x6c
	.byte 0x00
.DWinfo14:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000a46
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
	.4byte nss_unary_expr
//	DW_AT_high_pc:
	.4byte .LNnss_unary_expr
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x00000e6b
.DWinfo206:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x054f
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000a46
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x08
.DWinfo207:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x054f
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000a46
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x0c
.DWinfo208:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0550
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000008ef
//	DW_AT_name:
	.4byte 0x72707865
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x10
.DWinfo209:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0550
//	DW_AT_decl_column:
	.byte 0x1d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000c69
//	DW_AT_name:
	.4byte 0x74617473
	.2byte 0x7375
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x14
.DWinfo210:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0550
//	DW_AT_decl_column:
	.byte 0x29
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000784
//	DW_AT_name:
	.8byte 0x6e6f697469736f70
	.2byte 0x6c61
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x18
.DWinfo211:
//	DW_TAG_lexical_block:
	.byte 0x16
//	DW_AT_decl_line:
	.2byte 0x0562
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.4byte ..LN1393
//	DW_AT_high_pc:
	.4byte ..LN1437
.DWinfo212:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000008f4
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x68
	.byte 0x00
	.byte 0x00
.DWinfo15:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000a46
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x727078655f73736e
	.byte 0x00
	.8byte 0x727078655f73736e
	.byte 0x00
//	DW_AT_low_pc:
	.4byte nss_expr
//	DW_AT_high_pc:
	.4byte .LNnss_expr
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x00000f1d
.DWinfo213:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x057c
//	DW_AT_decl_column:
	.byte 0x24
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000a46
//	DW_AT_name:
	.2byte 0x0070
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x08
.DWinfo214:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x057c
//	DW_AT_decl_column:
	.byte 0x33
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000a46
//	DW_AT_name:
	.4byte 0x00646e65
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x0c
.DWinfo215:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x057d
//	DW_AT_decl_column:
	.byte 0x12
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000008ef
//	DW_AT_name:
	.4byte 0x72707865
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x10
.DWinfo216:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x057d
//	DW_AT_decl_column:
	.byte 0x1d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000c69
//	DW_AT_name:
	.4byte 0x74617473
	.2byte 0x7375
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x14
.DWinfo217:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x057d
//	DW_AT_decl_column:
	.byte 0x29
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000784
//	DW_AT_name:
	.8byte 0x6e6f697469736f70
	.2byte 0x6c61
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x18
.DWinfo218:
//	DW_TAG_lexical_block:
	.byte 0x16
//	DW_AT_decl_line:
	.2byte 0x0589
//	DW_AT_decl_column:
	.byte 0x0d
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.4byte ..LN1491
//	DW_AT_high_pc:
	.4byte ..LN1591
.DWinfo219:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000773
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x5c
.DWinfo220:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000008f4
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x60
	.byte 0x00
	.byte 0x00
.DWinfo16:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x000002d6
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
	.4byte NucStrstrInit
//	DW_AT_high_pc:
	.4byte .LNNucStrstrInit
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x00000fdb
.DWinfo221:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x5c
.DWinfo222:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x54
.DWinfo223:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000a46
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x58
.DWinfo224:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000a46
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x4c
.DWinfo225:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000a46
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x50
.DWinfo226:
//	DW_TAG_lexical_block:
	.byte 0x16
//	DW_AT_decl_line:
	.2byte 0x05e7
//	DW_AT_decl_column:
	.byte 0x1c
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.4byte ..LN1699
//	DW_AT_high_pc:
	.4byte ..LN1715
.DWinfo227:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000002c4
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x7c
.DWinfo228:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000002c4
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x78
	.byte 0x00
	.byte 0x00
.DWinfo17:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000784
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
	.4byte NucStrstrMake
//	DW_AT_high_pc:
	.4byte .LNNucStrstrMake
//	DW_AT_external:
	.byte 0x01
//	DW_AT_sibling:
	.4byte 0x0000108d
.DWinfo229:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0601
//	DW_AT_decl_column:
	.byte 0x21
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x0000108d
//	DW_AT_name:
	.4byte 0x0073736e
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x08
.DWinfo230:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0601
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000784
//	DW_AT_name:
	.8byte 0x6e6f697469736f70
	.2byte 0x6c61
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x0c
.DWinfo231:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0602
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00000a46
//	DW_AT_name:
	.4byte 0x72657571
	.2byte 0x0079
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x10
.DWinfo232:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0602
//	DW_AT_decl_column:
	.byte 0x25
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x14
.DWinfo233:
//	DW_TAG_lexical_block:
	.byte 0x16
//	DW_AT_decl_line:
	.2byte 0x0607
//	DW_AT_decl_column:
	.byte 0x09
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.4byte ..LN1731
//	DW_AT_high_pc:
	.4byte ..LN1783
.DWinfo234:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x6c
.DWinfo235:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000a46
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x70
	.byte 0x00
	.byte 0x00
.DWinfo236:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x00001092
.DWinfo237:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x0000090e
.DWinfo18:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x000002d6
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
	.4byte NucStrstrWhack
//	DW_AT_high_pc:
	.4byte .LNNucStrstrWhack
//	DW_AT_external:
	.byte 0x01
//	DW_AT_sibling:
	.4byte 0x000010e0
.DWinfo238:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0625
//	DW_AT_decl_column:
	.byte 0x22
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x00001092
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x08
	.byte 0x00
.DWinfo19:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000784
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
	.4byte eval_2na_8
//	DW_AT_high_pc:
	.4byte .LNeval_2na_8
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x000012f1
.DWinfo239:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0892
//	DW_AT_decl_column:
	.byte 0x29
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012f1
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x08
.DWinfo240:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0893
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012fb
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x0c
.DWinfo241:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0893
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x10
.DWinfo242:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0893
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x14
.DWinfo243:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e987503
.DWinfo244:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e887503
.DWinfo245:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ea87503
.DWinfo246:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ec87503
.DWinfo247:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ee87503
.DWinfo248:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7f887503
.DWinfo249:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7eb87503
.DWinfo250:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ed87503
.DWinfo251:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ef87503
.DWinfo252:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7f987503
.DWinfo253:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x70
.DWinfo254:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x6c
.DWinfo255:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x68
.DWinfo256:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x58
.DWinfo257:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x74
.DWinfo258:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x60
.DWinfo259:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x64
.DWinfo260:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x5c
.DWinfo261:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x7c
.DWinfo262:
//	DW_TAG_lexical_block:
	.byte 0x16
//	DW_AT_decl_line:
	.2byte 0x090a
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.4byte ..LN1981
//	DW_AT_high_pc:
	.4byte ..LN2067
.DWinfo263:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fb87503
.DWinfo264:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fbc7503
.DWinfo265:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x40
.DWinfo266:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x44
	.byte 0x00
	.byte 0x00
.DWinfo267:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x000012f6
.DWinfo268:
//	DW_TAG_const_type:
	.byte 0x0e
//	DW_AT_type:
	.4byte 0x000006fb
.DWinfo269:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x00001300
.DWinfo270:
//	DW_TAG_const_type:
	.byte 0x0e
//	DW_AT_type:
	.4byte 0x000002d6
.DWinfo271:
//	DW_TAG_typedef:
	.byte 0x04
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
	.4byte 0x00000473
.DWinfo20:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000784
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x616e325f6c617665
	.4byte 0x0036315f
	.8byte 0x616e325f6c617665
	.4byte 0x0036315f
//	DW_AT_low_pc:
	.4byte eval_2na_16
//	DW_AT_high_pc:
	.4byte .LNeval_2na_16
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x0000154d
.DWinfo272:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x09b7
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012f1
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x08
.DWinfo273:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x09b8
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012fb
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x0c
.DWinfo274:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x09b8
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x10
.DWinfo275:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x09b8
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x14
.DWinfo276:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e887503
.DWinfo277:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7df87503
.DWinfo278:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e987503
.DWinfo279:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7eb87503
.DWinfo280:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ed87503
.DWinfo281:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ef87503
.DWinfo282:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ea87503
.DWinfo283:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ec87503
.DWinfo284:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ee87503
.DWinfo285:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7f887503
.DWinfo286:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x68
.DWinfo287:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x64
.DWinfo288:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x60
.DWinfo289:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x4c
.DWinfo290:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x6c
.DWinfo291:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x54
.DWinfo292:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x48
.DWinfo293:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x5c
.DWinfo294:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x58
.DWinfo295:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x50
.DWinfo296:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x74
.DWinfo297:
//	DW_TAG_lexical_block:
	.byte 0x16
//	DW_AT_decl_line:
	.2byte 0x0a2f
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.4byte ..LN2271
//	DW_AT_high_pc:
	.4byte ..LN2357
.DWinfo298:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fa87503
.DWinfo299:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fac7503
.DWinfo300:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fb07503
.DWinfo301:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fb47503
	.byte 0x00
	.byte 0x00
.DWinfo21:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000784
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x616e325f6c617665
	.4byte 0x0032335f
	.8byte 0x616e325f6c617665
	.4byte 0x0032335f
//	DW_AT_low_pc:
	.4byte eval_2na_32
//	DW_AT_high_pc:
	.4byte .LNeval_2na_32
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x00001783
.DWinfo302:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0adc
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012f1
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x08
.DWinfo303:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0add
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012fb
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x0c
.DWinfo304:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0add
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x10
.DWinfo305:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0add
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x14
.DWinfo306:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e887503
.DWinfo307:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7df87503
.DWinfo308:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e987503
.DWinfo309:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7eb87503
.DWinfo310:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ed87503
.DWinfo311:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ef87503
.DWinfo312:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ea87503
.DWinfo313:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ec87503
.DWinfo314:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ee87503
.DWinfo315:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7f887503
.DWinfo316:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x68
.DWinfo317:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x64
.DWinfo318:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x60
.DWinfo319:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x4c
.DWinfo320:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x6c
.DWinfo321:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x54
.DWinfo322:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x48
.DWinfo323:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x5c
.DWinfo324:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x58
.DWinfo325:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x50
.DWinfo326:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x74
.DWinfo327:
//	DW_TAG_lexical_block:
	.byte 0x16
//	DW_AT_decl_line:
	.2byte 0x0b54
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.4byte ..LN2603
//	DW_AT_high_pc:
	.4byte ..LN2689
.DWinfo328:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fa87503
.DWinfo329:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fac7503
.DWinfo330:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fb07503
.DWinfo331:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fb47503
	.byte 0x00
	.byte 0x00
.DWinfo22:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000784
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x616e325f6c617665
	.4byte 0x0034365f
	.8byte 0x616e325f6c617665
	.4byte 0x0034365f
//	DW_AT_low_pc:
	.4byte eval_2na_64
//	DW_AT_high_pc:
	.4byte .LNeval_2na_64
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x000019b9
.DWinfo332:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0c01
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012f1
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x08
.DWinfo333:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0c02
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012fb
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x0c
.DWinfo334:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0c02
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x10
.DWinfo335:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0c02
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x14
.DWinfo336:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e887503
.DWinfo337:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7df87503
.DWinfo338:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e987503
.DWinfo339:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7eb87503
.DWinfo340:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ed87503
.DWinfo341:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ef87503
.DWinfo342:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ea87503
.DWinfo343:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ec87503
.DWinfo344:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ee87503
.DWinfo345:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7f887503
.DWinfo346:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x68
.DWinfo347:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x64
.DWinfo348:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x60
.DWinfo349:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x4c
.DWinfo350:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x6c
.DWinfo351:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x54
.DWinfo352:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x48
.DWinfo353:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x5c
.DWinfo354:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x58
.DWinfo355:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x50
.DWinfo356:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x74
.DWinfo357:
//	DW_TAG_lexical_block:
	.byte 0x16
//	DW_AT_decl_line:
	.2byte 0x0c7a
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.4byte ..LN2943
//	DW_AT_high_pc:
	.4byte ..LN3029
.DWinfo358:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fa87503
.DWinfo359:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fac7503
.DWinfo360:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fb07503
.DWinfo361:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fb47503
	.byte 0x00
	.byte 0x00
.DWinfo23:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000784
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
	.4byte eval_2na_128
//	DW_AT_high_pc:
	.4byte .LNeval_2na_128
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x00001b9f
.DWinfo362:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0d27
//	DW_AT_decl_column:
	.byte 0x2b
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012f1
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x08
.DWinfo363:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0d28
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012fb
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x0c
.DWinfo364:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0d28
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x10
.DWinfo365:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0d28
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x14
.DWinfo366:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ea87503
.DWinfo367:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e987503
.DWinfo368:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7eb87503
.DWinfo369:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ed87503
.DWinfo370:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ef87503
.DWinfo371:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7f987503
.DWinfo372:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ec87503
.DWinfo373:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ee87503
.DWinfo374:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7f887503
.DWinfo375:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7fa87503
.DWinfo376:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x6c
.DWinfo377:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x68
.DWinfo378:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x64
.DWinfo379:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x50
.DWinfo380:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x70
.DWinfo381:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x58
.DWinfo382:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x4c
.DWinfo383:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x60
.DWinfo384:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x5c
.DWinfo385:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x54
.DWinfo386:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x78
	.byte 0x00
.DWinfo24:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000784
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
	.4byte eval_2na_pos
//	DW_AT_high_pc:
	.4byte .LNeval_2na_pos
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x00001d98
.DWinfo387:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0e4c
//	DW_AT_decl_column:
	.byte 0x2b
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012f1
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x08
.DWinfo388:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0e4d
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012fb
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x0c
.DWinfo389:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0e4d
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x10
.DWinfo390:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0e4d
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x14
.DWinfo391:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ea87503
.DWinfo392:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e987503
.DWinfo393:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7eb87503
.DWinfo394:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ed87503
.DWinfo395:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ef87503
.DWinfo396:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7f987503
.DWinfo397:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ec87503
.DWinfo398:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ee87503
.DWinfo399:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7f887503
.DWinfo400:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7fa87503
.DWinfo401:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x70
.DWinfo402:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x6c
.DWinfo403:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x68
.DWinfo404:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x50
.DWinfo405:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x74
.DWinfo406:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x5c
.DWinfo407:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x54
.DWinfo408:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x4c
.DWinfo409:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x64
.DWinfo410:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x60
.DWinfo411:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x58
.DWinfo412:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x7c
	.byte 0x00
.DWinfo25:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000784
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x616e345f6c617665
	.4byte 0x0036315f
	.8byte 0x616e345f6c617665
	.4byte 0x0036315f
//	DW_AT_low_pc:
	.4byte eval_4na_16
//	DW_AT_high_pc:
	.4byte .LNeval_4na_16
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x00001fbc
.DWinfo413:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0fd7
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012f1
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x08
.DWinfo414:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0fd8
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012fb
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x0c
.DWinfo415:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0fd8
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x10
.DWinfo416:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x0fd8
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x14
.DWinfo417:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e987503
.DWinfo418:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7df87503
.DWinfo419:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e887503
.DWinfo420:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ea87503
.DWinfo421:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ec87503
.DWinfo422:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ee87503
.DWinfo423:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7f887503
.DWinfo424:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7eb87503
.DWinfo425:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ed87503
.DWinfo426:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ef87503
.DWinfo427:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7f987503
.DWinfo428:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x70
.DWinfo429:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x6c
.DWinfo430:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x68
.DWinfo431:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x58
.DWinfo432:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x74
.DWinfo433:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x60
.DWinfo434:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x64
.DWinfo435:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x5c
.DWinfo436:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x7c
.DWinfo437:
//	DW_TAG_lexical_block:
	.byte 0x16
//	DW_AT_decl_line:
	.2byte 0x104d
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.4byte ..LN3933
//	DW_AT_high_pc:
	.4byte ..LN4023
.DWinfo438:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fb87503
.DWinfo439:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fbc7503
.DWinfo440:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x40
.DWinfo441:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x44
	.byte 0x00
	.byte 0x00
.DWinfo26:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000784
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x616e345f6c617665
	.4byte 0x0032335f
	.8byte 0x616e345f6c617665
	.4byte 0x0032335f
//	DW_AT_low_pc:
	.4byte eval_4na_32
//	DW_AT_high_pc:
	.4byte .LNeval_4na_32
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x000021f1
.DWinfo442:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x10b5
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012f1
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x08
.DWinfo443:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x10b6
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012fb
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x0c
.DWinfo444:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x10b6
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x10
.DWinfo445:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x10b6
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x14
.DWinfo446:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e887503
.DWinfo447:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7de87503
.DWinfo448:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7df87503
.DWinfo449:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e987503
.DWinfo450:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7eb87503
.DWinfo451:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ed87503
.DWinfo452:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ef87503
.DWinfo453:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ea87503
.DWinfo454:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ec87503
.DWinfo455:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ee87503
.DWinfo456:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7f887503
.DWinfo457:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x64
.DWinfo458:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x60
.DWinfo459:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x5c
.DWinfo460:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x48
.DWinfo461:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x68
.DWinfo462:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x50
.DWinfo463:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x58
.DWinfo464:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x54
.DWinfo465:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x4c
.DWinfo466:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x70
.DWinfo467:
//	DW_TAG_lexical_block:
	.byte 0x16
//	DW_AT_decl_line:
	.2byte 0x112b
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.4byte ..LN4249
//	DW_AT_high_pc:
	.4byte ..LN4339
.DWinfo468:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fa87503
.DWinfo469:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fac7503
.DWinfo470:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fb07503
.DWinfo471:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fb47503
	.byte 0x00
	.byte 0x00
.DWinfo27:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000784
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x616e345f6c617665
	.4byte 0x0034365f
	.8byte 0x616e345f6c617665
	.4byte 0x0034365f
//	DW_AT_low_pc:
	.4byte eval_4na_64
//	DW_AT_high_pc:
	.4byte .LNeval_4na_64
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x00002426
.DWinfo472:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x1193
//	DW_AT_decl_column:
	.byte 0x2a
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012f1
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x08
.DWinfo473:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x1194
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012fb
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x0c
.DWinfo474:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x1194
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x10
.DWinfo475:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x1194
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x14
.DWinfo476:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e887503
.DWinfo477:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7de87503
.DWinfo478:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7df87503
.DWinfo479:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e987503
.DWinfo480:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7eb87503
.DWinfo481:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ed87503
.DWinfo482:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ef87503
.DWinfo483:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ea87503
.DWinfo484:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ec87503
.DWinfo485:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ee87503
.DWinfo486:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7f887503
.DWinfo487:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x64
.DWinfo488:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x60
.DWinfo489:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x5c
.DWinfo490:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x48
.DWinfo491:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x68
.DWinfo492:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x50
.DWinfo493:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x58
.DWinfo494:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x54
.DWinfo495:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x4c
.DWinfo496:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x70
.DWinfo497:
//	DW_TAG_lexical_block:
	.byte 0x16
//	DW_AT_decl_line:
	.2byte 0x120a
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.4byte ..LN4599
//	DW_AT_high_pc:
	.4byte ..LN4689
.DWinfo498:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fa87503
.DWinfo499:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fac7503
.DWinfo500:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fb07503
.DWinfo501:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.4byte 0x7fb47503
	.byte 0x00
	.byte 0x00
.DWinfo28:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000784
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
	.4byte eval_4na_128
//	DW_AT_high_pc:
	.4byte .LNeval_4na_128
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x0000260b
.DWinfo502:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x1272
//	DW_AT_decl_column:
	.byte 0x2b
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012f1
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x08
.DWinfo503:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x1273
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012fb
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x0c
.DWinfo504:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x1273
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x10
.DWinfo505:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x1273
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x14
.DWinfo506:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ea87503
.DWinfo507:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e887503
.DWinfo508:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e987503
.DWinfo509:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7eb87503
.DWinfo510:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ed87503
.DWinfo511:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ef87503
.DWinfo512:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7f987503
.DWinfo513:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ec87503
.DWinfo514:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ee87503
.DWinfo515:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7f887503
.DWinfo516:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7fa87503
.DWinfo517:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x68
.DWinfo518:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x64
.DWinfo519:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x60
.DWinfo520:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x4c
.DWinfo521:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x6c
.DWinfo522:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x54
.DWinfo523:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x5c
.DWinfo524:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x58
.DWinfo525:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x50
.DWinfo526:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x74
	.byte 0x00
.DWinfo29:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000784
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
	.4byte eval_4na_pos
//	DW_AT_high_pc:
	.4byte .LNeval_4na_pos
//	DW_AT_external:
	.byte 0x00
//	DW_AT_sibling:
	.4byte 0x00002803
.DWinfo527:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x1350
//	DW_AT_decl_column:
	.byte 0x2b
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012f1
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x08
.DWinfo528:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x1351
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012fb
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x0c
.DWinfo529:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x1351
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x10
.DWinfo530:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x1351
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7302
	.byte 0x14
.DWinfo531:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ea87503
.DWinfo532:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e887503
.DWinfo533:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7e987503
.DWinfo534:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7eb87503
.DWinfo535:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ed87503
.DWinfo536:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ef87503
.DWinfo537:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7f987503
.DWinfo538:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ec87503
.DWinfo539:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7ee87503
.DWinfo540:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7f887503
.DWinfo541:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00001305
//	DW_AT_location:
	.4byte 0x7fa87503
.DWinfo542:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x6c
.DWinfo543:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x68
.DWinfo544:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x64
.DWinfo545:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x4c
.DWinfo546:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x70
.DWinfo547:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x58
.DWinfo548:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x50
.DWinfo549:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x60
.DWinfo550:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x5c
.DWinfo551:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000517
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x54
.DWinfo552:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x78
	.byte 0x00
.DWinfo30:
//	DW_TAG_subprogram:
	.byte 0x09
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
	.4byte 0x00000784
//	DW_AT_prototyped:
	.byte 0x01
//	DW_AT_name:
	.8byte 0x747372745363754e
	.8byte 0x0068637261655372
	.8byte 0x747372745363754e
	.8byte 0x0068637261655372
//	DW_AT_low_pc:
	.4byte NucStrstrSearch
//	DW_AT_high_pc:
	.4byte .LNNucStrstrSearch
//	DW_AT_external:
	.byte 0x01
//	DW_AT_sibling:
	.4byte 0x000028cf
.DWinfo553:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x145f
//	DW_AT_decl_column:
	.byte 0x28
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000028cf
//	DW_AT_name:
	.4byte 0x666c6573
	.byte 0x00
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x08
.DWinfo554:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x1460
//	DW_AT_decl_column:
	.byte 0x11
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000012fb
//	DW_AT_name:
	.8byte 0x00616e326962636e
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x0c
.DWinfo555:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x1460
//	DW_AT_decl_column:
	.byte 0x27
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x00736f70
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x10
.DWinfo556:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x1460
//	DW_AT_decl_column:
	.byte 0x39
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000003b2
//	DW_AT_name:
	.4byte 0x006e656c
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x14
.DWinfo557:
//	DW_TAG_formal_parameter:
	.byte 0x0a
//	DW_AT_decl_line:
	.2byte 0x1460
//	DW_AT_decl_column:
	.byte 0x4e
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_type:
	.4byte 0x000028d9
//	DW_AT_name:
	.8byte 0x006e656c666c6573
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x18
.DWinfo558:
//	DW_TAG_lexical_block:
	.byte 0x16
//	DW_AT_decl_line:
	.2byte 0x1463
//	DW_AT_decl_column:
	.byte 0x05
//	DW_AT_decl_file:
	.byte 0x03
//	DW_AT_low_pc:
	.4byte ..LN5427
//	DW_AT_high_pc:
	.4byte ..LN5875
.DWinfo559:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x00000784
//	DW_AT_location:
	.2byte 0x7502
	.byte 0x64
.DWinfo560:
//	DW_TAG_variable:
	.byte 0x11
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
	.4byte 0x000003b2
//	DW_AT_location:
	.4byte 0x7fac7503
	.byte 0x00
	.byte 0x00
.DWinfo561:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x000028d4
.DWinfo562:
//	DW_TAG_const_type:
	.byte 0x0e
//	DW_AT_type:
	.4byte 0x0000090e
.DWinfo563:
//	DW_TAG_pointer_type:
	.byte 0x0b
//	DW_AT_type:
	.4byte 0x000003b2
.DWinfo564:
//	DW_TAG_variable:
	.byte 0x08
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
	.4byte 0x000028fb
//	DW_AT_location:
	.2byte 0x0305
	.4byte fasta_2na_map
.DWinfo565:
//	DW_TAG_array_type:
	.byte 0x14
//	DW_AT_type:
	.4byte 0x00000c78
//	DW_AT_byte_size:
	.byte 0x80
//	DW_AT_sibling:
	.4byte 0x00002908
.DWinfo566:
//	DW_TAG_subrange_type:
	.byte 0x15
//	DW_AT_upper_bound:
	.byte 0x7f
	.byte 0x00
.DWinfo567:
//	DW_TAG_variable:
	.byte 0x08
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
	.4byte 0x00002925
//	DW_AT_location:
	.2byte 0x0305
	.4byte fasta_4na_map
.DWinfo568:
//	DW_TAG_array_type:
	.byte 0x14
//	DW_AT_type:
	.4byte 0x00000c78
//	DW_AT_byte_size:
	.byte 0x80
//	DW_AT_sibling:
	.4byte 0x00002932
.DWinfo569:
//	DW_TAG_subrange_type:
	.byte 0x15
//	DW_AT_upper_bound:
	.byte 0x7f
	.byte 0x00
.DWinfo570:
//	DW_TAG_variable:
	.byte 0x08
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
	.4byte 0x0000294c
//	DW_AT_location:
	.2byte 0x0305
	.4byte expand_2na
.DWinfo571:
//	DW_TAG_array_type:
	.byte 0x1e
//	DW_AT_type:
	.4byte 0x000002b2
//	DW_AT_byte_size:
	.2byte 0x0200
.DWinfo572:
//	DW_TAG_subrange_type:
	.byte 0x1f
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
	.4byte 0x000026aa
	.2byte 0x0002
	.4byte 0x00000114
	.byte 0x01
	.byte 0x01
	.byte 0xff
	.byte 0x04
	.byte 0x0a
	.8byte 0x0000000101010100
	.byte 0x01
	.8byte 0x6f722f656d6f682f
	.8byte 0x642f72656d726164
	.8byte 0x746e692f6c657665
	.8byte 0x73612f6c616e7265
	.8byte 0x2f65636172742d6d
	.8byte 0x2f6363672f636e69
	.4byte 0x36383369
	.byte 0x00
	.8byte 0x636e692f7273752f
	.4byte 0x6564756c
	.byte 0x00
	.8byte 0x746e692f74706f2f
	.8byte 0x30312f63632f6c65
	.8byte 0x692f3132302e312e
	.4byte 0x756c636e
	.2byte 0x6564
	.byte 0x00
	.8byte 0x6f722f656d6f682f
	.8byte 0x642f72656d726164
	.8byte 0x746e692f6c657665
	.8byte 0x73612f6c616e7265
	.8byte 0x2f65636172742d6d
	.8byte 0x006172732f667469
	.byte 0x00
	.8byte 0x706d692d68637261
	.4byte 0x00682e6c
	.8byte 0x41e904d1e4efaf01
	.8byte 0x682e746e69647473
	.byte 0x00
	.8byte 0x42e104bffdfc8e02
	.8byte 0x747372747363756e
	.4byte 0x00632e72
	.8byte 0x8eec04d283e7bd00
	.byte 0x0a
	.8byte 0x6972746e696d6d65
	.4byte 0x00682e6e
	.8byte 0x7bca04c1ab8cdd03
	.8byte 0x682e666564647473
	.byte 0x00
	.8byte 0x07c404c1ab8cdd03
	.8byte 0x747372747363756e
	.4byte 0x00682e72
	.8byte 0x27d104cb94abbd04
	.byte 0x00
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1
	.2byte 0x2503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5
	.2byte 0x0903
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN7
	.2byte 0xf603
	.byte 0x01
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN9
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN11
	.2byte 0x0e03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN13
	.2byte 0x1703
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN15
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN17
	.2byte 0x1203
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN19
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN21
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN23
	.2byte 0x1403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN25
	.byte 0x04
	.byte 0x03
	.2byte 0xd803
	.byte 0x0d
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN27
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN33
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN37
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN41
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN43
	.2byte 0xbe03
	.byte 0x0e
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN45
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN51
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN57
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN63
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN69
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN75
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN81
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN87
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN93
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN97
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN99
	.2byte 0xf003
	.byte 0x66
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN101
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN111
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN115
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN121
	.byte 0x0a
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN123
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN129
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN131
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN133
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN135
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN137
	.2byte 0x2503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN139
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN143
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN145
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN153
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN157
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN159
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN165
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN171
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN183
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN193
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN197
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN207
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN213
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN231
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN243
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN261
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN273
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN289
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN301
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN313
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN317
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN321
	.2byte 0x7d03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN327
	.2byte 0x0903
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN329
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN331
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN335
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN339
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN343
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN347
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN351
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN355
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN359
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN363
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN367
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN369
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN373
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN377
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN381
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN385
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN389
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN393
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN397
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN399
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN403
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN407
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN411
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN415
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN419
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN421
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN425
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN429
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN433
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN435
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN437
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN441
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN445
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN451
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN457
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN465
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN473
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN483
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN493
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN501
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN509
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN519
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN529
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN537
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN545
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN555
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN565
	.2byte 0x2a03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN567
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN569
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN573
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN575
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN583
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN587
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN589
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN595
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN601
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN613
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN623
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN627
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN637
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN643
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN659
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN671
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN689
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN701
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN719
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN731
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN743
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN747
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN751
	.2byte 0x7d03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN757
	.2byte 0x0803
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN759
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN761
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN765
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN769
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN773
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN777
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN781
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN785
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN789
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN791
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN795
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN799
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN803
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN807
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN811
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN813
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN817
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN821
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN825
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN827
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN829
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN833
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN837
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN843
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN849
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN857
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN865
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN875
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN885
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN893
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN901
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN911
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN921
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN929
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN937
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN947
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN957
	.2byte 0x2a03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN959
	.2byte 0x0803
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN961
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN967
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN969
	.byte 0x0a
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN973
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN975
	.2byte 0x0803
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN977
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN981
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN983
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN987
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN989
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN991
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN995
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1001
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1007
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1013
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1015
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1019
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1023
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1027
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1029
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1033
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1047
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1061
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1063
	.2byte 0x0803
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1065
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1067
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1069
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1085
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1101
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1103
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1119
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1135
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1137
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1151
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1153
	.2byte 0x2703
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1155
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1157
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1159
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1167
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1171
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1179
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1181
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1183
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1185
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1191
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1201
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1205
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1207
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1215
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1219
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1227
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1229
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1231
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1237
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1253
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1257
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1273
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1275
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1277
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1281
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1289
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1299
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1307
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1309
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1317
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1321
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1329
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1331
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1333
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1341
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1347
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1349
	.2byte 0x0803
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1351
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1353
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1357
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1371
	.2byte 0x0a03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1381
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1385
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1387
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1395
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1399
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1407
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1409
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1411
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1413
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1419
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1435
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1437
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1439
	.2byte 0x0d03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1441
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1443
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1453
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1457
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1473
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1477
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1487
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1491
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1493
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1495
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1499
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1507
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1509
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1511
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1519
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1521
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1523
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1525
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1529
	.2byte 0x0c03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1537
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1541
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1549
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1551
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1553
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1559
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1567
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1573
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1589
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1591
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1593
	.2byte 0x0e03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1595
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1597
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1599
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1609
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1619
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1629
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1633
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1647
	.2byte 0x7d03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1655
	.2byte 0x0703
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1665
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1669
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1683
	.2byte 0x7d03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1691
	.2byte 0x0803
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1697
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1709
	.byte 0x0a
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1715
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1717
	.2byte 0x1a03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1719
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1723
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1731
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1733
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1737
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1739
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1745
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1761
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1765
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1769
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1771
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1773
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1779
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1781
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1783
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1785
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1787
	.2byte 0x0703
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1789
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1793
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1797
	.2byte 0x1003
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1801
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1807
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1813
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1819
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1825
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1827
	.2byte 0xce03
	.byte 0x04
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1829
	.2byte 0x1e03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1833
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1835
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1839
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1847
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1851
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1861
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1871
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1873
	.2byte 0x0a03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1875
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1881
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1883
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1887
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1889
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1891
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1893
	.2byte 0x0703
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1901
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1905
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1911
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1919
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1923
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1929
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1937
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1941
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1947
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1955
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1959
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1965
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1967
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1975
	.2byte 0x1203
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1983
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1991
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN1999
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2007
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2009
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2015
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2021
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2027
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2037
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2047
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2057
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2067
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2069
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2073
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2075
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2079
	.2byte 0x1503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2083
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2085
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2089
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2099
	.2byte 0xce03
	.byte 0x00
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2101
	.2byte 0x0d03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2103
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2105
	.2byte 0x1e03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2109
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2111
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2115
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2123
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2127
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2137
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2147
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2149
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2153
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2157
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2163
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2165
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2171
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2173
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2177
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2179
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2181
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2183
	.2byte 0x0703
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2191
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2195
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2201
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2209
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2213
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2219
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2227
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2231
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2237
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2245
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2249
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2255
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2257
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2265
	.2byte 0x1203
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2273
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2281
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2289
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2297
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2299
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2305
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2311
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2317
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2327
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2337
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2347
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2357
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2359
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2363
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2365
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2369
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2375
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2379
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2381
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2387
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2395
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2397
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2401
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2403
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2407
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2417
	.2byte 0xce03
	.byte 0x00
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2419
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2423
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2427
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2433
	.2byte 0x0903
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2435
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2437
	.2byte 0x1e03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2441
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2443
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2447
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2455
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2459
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2469
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2479
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2481
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2485
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2489
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2495
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2497
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2503
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2505
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2509
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2511
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2513
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2515
	.2byte 0x0703
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2523
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2527
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2533
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2541
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2545
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2551
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2559
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2563
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2569
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2577
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2581
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2587
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2589
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2597
	.2byte 0x1203
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2605
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2613
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2621
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2629
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2631
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2637
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2643
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2649
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2659
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2669
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2679
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2689
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2691
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2695
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2697
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2701
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2707
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2711
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2713
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2719
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2727
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2729
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2733
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2735
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2739
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2749
	.2byte 0xce03
	.byte 0x00
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2751
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2755
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2759
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2765
	.2byte 0x0903
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2767
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2769
	.2byte 0x1f03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2773
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2775
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2779
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2787
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2791
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2801
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2811
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2813
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2817
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2821
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2827
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2829
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2835
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2837
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2841
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2843
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2845
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2847
	.2byte 0x0703
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2855
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2859
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2865
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2867
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2875
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2879
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2885
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2887
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2895
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2899
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2905
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2907
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2915
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2919
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2925
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2927
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2929
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2937
	.2byte 0x1203
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2945
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2953
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2961
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2969
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2971
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2977
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2983
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2989
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN2999
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3009
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3019
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3029
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3031
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3035
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3037
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3041
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3047
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3051
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3053
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3059
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3067
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3069
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3073
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3075
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3079
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3089
	.2byte 0xce03
	.byte 0x00
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3091
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3095
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3099
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3105
	.2byte 0x0903
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3107
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3109
	.2byte 0x1e03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3113
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3115
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3119
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3127
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3131
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3141
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3151
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3153
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3157
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3161
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3167
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3169
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3175
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3185
	.2byte 0x0703
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3189
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3191
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3193
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3195
	.2byte 0x0703
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3203
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3207
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3213
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3215
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3223
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3227
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3233
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3235
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3243
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3247
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3253
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3255
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3263
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3267
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3273
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3275
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3277
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3285
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3287
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3289
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3295
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3301
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3307
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3309
	.2byte 0x1b03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3311
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3315
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3317
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3321
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3327
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3331
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3333
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3339
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3347
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3349
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3353
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3357
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3367
	.2byte 0xce03
	.byte 0x00
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3369
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3373
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3377
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3383
	.2byte 0x0903
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3385
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3387
	.2byte 0x2403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3391
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3393
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3397
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3401
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3409
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3413
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3423
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3433
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3435
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3439
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3443
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3449
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3451
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3457
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3467
	.2byte 0x0703
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3471
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3473
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3475
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3477
	.2byte 0x0703
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3485
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3489
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3495
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3497
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3505
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3509
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3515
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3517
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3525
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3529
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3535
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3537
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3545
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3549
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3555
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3557
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3559
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3567
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3569
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3579
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3589
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3599
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3609
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3619
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3629
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3639
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3649
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3659
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3669
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3671
	.2byte 0x2803
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3673
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3677
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3679
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3683
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3689
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3693
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3695
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3701
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3709
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3711
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3715
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3719
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3729
	.2byte 0xce03
	.byte 0x00
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3731
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3735
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3739
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3745
	.2byte 0x0a03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3747
	.2byte 0xca03
	.byte 0x00
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3749
	.2byte 0x1a03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3753
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3755
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3759
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3767
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3771
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3781
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3791
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3793
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3795
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3801
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3803
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3807
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3809
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3811
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3813
	.2byte 0x0703
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3821
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3829
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3833
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3839
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3847
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3855
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3859
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3865
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3873
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3881
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3885
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3891
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3899
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3907
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3911
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3917
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3919
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3927
	.2byte 0x1203
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3935
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3943
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3951
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3959
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3965
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3971
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3977
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3983
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN3993
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4003
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4013
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4023
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4025
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4029
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4031
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4035
	.2byte 0x1103
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4039
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4041
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4045
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4055
	.2byte 0x0f03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4057
	.2byte 0x0b03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4059
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4061
	.2byte 0x1a03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4065
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4067
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4071
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4079
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4083
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4093
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4103
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4105
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4109
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4111
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4117
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4119
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4123
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4125
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4127
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4129
	.2byte 0x0703
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4137
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4145
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4149
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4155
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4163
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4171
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4175
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4181
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4189
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4197
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4201
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4207
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4215
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4223
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4227
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4233
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4235
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4243
	.2byte 0x1203
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4251
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4259
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4267
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4275
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4281
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4287
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4293
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4299
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4309
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4319
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4329
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4339
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4341
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4345
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4347
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4351
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4357
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4361
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4371
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4373
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4377
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4379
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4383
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4393
	.2byte 0x0f03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4395
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4399
	.2byte 0x0903
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4401
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4403
	.2byte 0x1b03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4407
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4409
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4413
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4421
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4425
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4435
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4445
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4447
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4451
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4453
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4459
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4461
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4465
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4467
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4469
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4471
	.2byte 0x0703
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4479
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4487
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4491
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4497
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4499
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4507
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4515
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4519
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4525
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4527
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4535
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4543
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4547
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4553
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4555
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4563
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4571
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4575
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4581
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4583
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4585
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4593
	.2byte 0x1203
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4601
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4609
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4617
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4625
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4631
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4637
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4643
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4649
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4659
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4669
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4679
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4689
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4691
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4695
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4697
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4701
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4707
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4711
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4721
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4723
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4727
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4729
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4733
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4743
	.2byte 0x0f03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4745
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4749
	.2byte 0x0903
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4751
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4753
	.2byte 0x1a03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4757
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4759
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4763
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4771
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4775
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4785
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4795
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4797
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4801
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4803
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4809
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4819
	.2byte 0x0903
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4823
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4825
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4827
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4829
	.2byte 0x0703
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4837
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4845
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4849
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4855
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4857
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4865
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4873
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4877
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4883
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4885
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4893
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4901
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4905
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4911
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4913
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4921
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4929
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4933
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4939
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4941
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4943
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4951
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4953
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4955
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4961
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4967
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4973
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4975
	.2byte 0x1b03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4977
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4981
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4983
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4987
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4993
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN4997
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5007
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5009
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5013
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5017
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5027
	.2byte 0x0f03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5029
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5033
	.2byte 0x0903
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5035
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5037
	.2byte 0x2003
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5041
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5043
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5047
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5051
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5059
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5063
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5073
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5083
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5085
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5089
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5091
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5097
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5107
	.2byte 0x0903
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5111
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5113
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5115
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5117
	.2byte 0x0703
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5125
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5133
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5137
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5143
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5145
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5153
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5161
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5165
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5171
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5173
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5181
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5189
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5193
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5199
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5201
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5209
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5217
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5221
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5227
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5229
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5231
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5239
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5241
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5251
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5261
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5271
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5281
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5291
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5301
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5311
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5321
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5331
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5341
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5343
	.2byte 0x2803
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5345
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5349
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5351
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5355
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5361
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5365
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5375
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5377
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5381
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5385
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5395
	.2byte 0x0f03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5397
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5401
	.2byte 0x0a03
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5403
	.2byte 0x1503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5405
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5417
	.2byte 0x0503
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5421
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5429
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5439
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5451
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5459
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5469
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5481
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5489
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5499
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5511
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5519
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5529
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5541
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5549
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5559
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5571
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5579
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5589
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5601
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5609
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5619
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5631
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5639
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5649
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5661
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5669
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5679
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5691
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5699
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5709
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5721
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5729
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5739
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5751
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5767
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5769
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5773
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5775
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5779
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5781
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5795
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5797
	.2byte 0x0403
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5813
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5817
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5819
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5823
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5825
	.2byte 0x0303
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5829
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5833
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5835
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5849
	.byte 0x0d
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5853
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5857
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5859
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5865
	.byte 0x0c
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5873
	.byte 0x0a
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte ..LN5875
	.2byte 0x0603
	.byte 0x01
	.byte 0x00
	.byte 0x05
	.byte 0x02
	.4byte .LNNucStrstrSearch
	.byte 0x00
	.byte 0x01
	.byte 0x01
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
	.byte 0x04
	.byte 0x01
	.byte 0x32
	.byte 0x0b
	.byte 0x0b
	.byte 0x0b
	.byte 0x3e
	.byte 0x0b
	.byte 0x01
	.byte 0x13
	.2byte 0x0000
	.byte 0x03
	.byte 0x28
	.byte 0x00
	.byte 0x03
	.byte 0x08
	.byte 0x1c
	.byte 0x0d
	.2byte 0x0000
	.byte 0x04
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
	.byte 0x05
	.byte 0x24
	.byte 0x00
	.byte 0x0b
	.byte 0x0b
	.byte 0x3e
	.byte 0x0b
	.byte 0x03
	.byte 0x08
	.2byte 0x0000
	.byte 0x06
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
	.byte 0x01
	.byte 0x13
	.2byte 0x0000
	.byte 0x07
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
	.byte 0x08
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
	.byte 0x09
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
	.byte 0x01
	.byte 0x13
	.2byte 0x0000
	.byte 0x0a
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
	.byte 0x01
	.byte 0x13
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
	.byte 0x01
	.byte 0x13
	.2byte 0x0000
	.byte 0x10
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
	.byte 0x11
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
	.byte 0x12
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
	.byte 0x01
	.byte 0x13
	.2byte 0x0000
	.byte 0x13
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
	.byte 0x14
	.byte 0x01
	.byte 0x01
	.byte 0x49
	.byte 0x13
	.byte 0x0b
	.byte 0x0b
	.byte 0x01
	.byte 0x13
	.2byte 0x0000
	.byte 0x15
	.byte 0x21
	.byte 0x00
	.byte 0x2f
	.byte 0x0b
	.2byte 0x0000
	.byte 0x16
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
	.byte 0x17
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
	.byte 0x18
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
	.byte 0x01
	.byte 0x13
	.2byte 0x0000
	.byte 0x19
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
	.byte 0x1a
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
	.byte 0x01
	.byte 0x13
	.2byte 0x0000
	.byte 0x1b
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
	.byte 0x1c
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
	.byte 0x01
	.byte 0x13
	.2byte 0x0000
	.byte 0x1d
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
	.byte 0x01
	.byte 0x13
	.2byte 0x0000
	.byte 0x1e
	.byte 0x01
	.byte 0x01
	.byte 0x49
	.byte 0x13
	.byte 0x0b
	.byte 0x05
	.2byte 0x0000
	.byte 0x1f
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
	.4byte 0x00000010
	.4byte 0xffffffff
	.byte 0x01
	.byte 0x00
	.byte 0x01
	.byte 0x7c
	.byte 0x08
	.4byte 0x8804040c
	.byte 0x01
	.2byte 0x0000
	.4byte 0x0000001c
	.4byte .debug_frame_seg
	.4byte ..___tag_value_uint16_lsbit.2
	.4byte ..___tag_value_uint16_lsbit.12-..___tag_value_uint16_lsbit.2
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_uint16_lsbit.7-..___tag_value_uint16_lsbit.2
	.byte 0x0c
	.2byte 0x0805
	.byte 0x85
	.byte 0x02
	.byte 0x00
	.4byte 0x0000001c
	.4byte .debug_frame_seg
	.4byte ..___tag_value_uint128_shr.15
	.4byte ..___tag_value_uint128_shr.25-..___tag_value_uint128_shr.15
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_uint128_shr.20-..___tag_value_uint128_shr.15
	.byte 0x0c
	.2byte 0x0805
	.byte 0x85
	.byte 0x02
	.byte 0x00
	.4byte 0x0000001c
	.4byte .debug_frame_seg
	.4byte ..___tag_value_uint128_bswap.28
	.4byte ..___tag_value_uint128_bswap.38-..___tag_value_uint128_bswap.28
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap.33-..___tag_value_uint128_bswap.28
	.byte 0x0c
	.2byte 0x0805
	.byte 0x85
	.byte 0x02
	.byte 0x00
	.4byte 0x0000001c
	.4byte .debug_frame_seg
	.4byte ..___tag_value_uint128_bswap_copy.41
	.4byte ..___tag_value_uint128_bswap_copy.51-..___tag_value_uint128_bswap_copy.41
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap_copy.46-..___tag_value_uint128_bswap_copy.41
	.byte 0x0c
	.2byte 0x0805
	.byte 0x85
	.byte 0x02
	.byte 0x00
	.4byte 0x00000038
	.4byte .debug_frame_seg
	.4byte ..___tag_value_prime_buffer_2na.54
	.4byte ..___tag_value_prime_buffer_2na.68-..___tag_value_prime_buffer_2na.54
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_2na.59-..___tag_value_prime_buffer_2na.54
	.byte 0x0c
	.2byte 0x0804
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_2na.60-..___tag_value_prime_buffer_2na.59
	.byte 0x0c
	.2byte 0x0803
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_2na.62-..___tag_value_prime_buffer_2na.60
	.byte 0x10
	.4byte 0x00750205
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_2na.66-..___tag_value_prime_buffer_2na.62
	.byte 0x0c
	.2byte 0x0404
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000038
	.4byte .debug_frame_seg
	.4byte ..___tag_value_prime_buffer_4na.71
	.4byte ..___tag_value_prime_buffer_4na.86-..___tag_value_prime_buffer_4na.71
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_4na.76-..___tag_value_prime_buffer_4na.71
	.byte 0x0c
	.2byte 0x0804
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_4na.77-..___tag_value_prime_buffer_4na.76
	.byte 0x0c
	.2byte 0x0803
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_4na.79-..___tag_value_prime_buffer_4na.77
	.byte 0x10
	.4byte 0x00750205
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_4na.84-..___tag_value_prime_buffer_4na.79
	.byte 0x0c
	.2byte 0x0404
	.2byte 0x0000
	.byte 0x00
	.4byte 0x0000003c
	.4byte .debug_frame_seg
	.4byte ..___tag_value_NucStrFastaExprAlloc.89
	.4byte ..___tag_value_NucStrFastaExprAlloc.111-..___tag_value_NucStrFastaExprAlloc.89
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprAlloc.94-..___tag_value_NucStrFastaExprAlloc.89
	.byte 0x0c
	.2byte 0x0805
	.byte 0x85
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprAlloc.97-..___tag_value_NucStrFastaExprAlloc.94
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprAlloc.100-..___tag_value_NucStrFastaExprAlloc.97
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprAlloc.102-..___tag_value_NucStrFastaExprAlloc.100
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprAlloc.106-..___tag_value_NucStrFastaExprAlloc.102
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000034
	.4byte .debug_frame_seg
	.4byte ..___tag_value_NucStrFastaExprMake2.114
	.4byte ..___tag_value_NucStrFastaExprMake2.137-..___tag_value_NucStrFastaExprMake2.114
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.119-..___tag_value_NucStrFastaExprMake2.114
	.byte 0x0c
	.2byte 0x0805
	.byte 0x85
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.122-..___tag_value_NucStrFastaExprMake2.119
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.126-..___tag_value_NucStrFastaExprMake2.122
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.132-..___tag_value_NucStrFastaExprMake2.126
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.4byte 0x00000034
	.4byte .debug_frame_seg
	.4byte ..___tag_value_NucStrFastaExprMake4.140
	.4byte ..___tag_value_NucStrFastaExprMake4.163-..___tag_value_NucStrFastaExprMake4.140
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.145-..___tag_value_NucStrFastaExprMake4.140
	.byte 0x0c
	.2byte 0x0805
	.byte 0x85
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.148-..___tag_value_NucStrFastaExprMake4.145
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.152-..___tag_value_NucStrFastaExprMake4.148
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.158-..___tag_value_NucStrFastaExprMake4.152
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.4byte 0x00000024
	.4byte .debug_frame_seg
	.4byte ..___tag_value_nss_sob.166
	.4byte ..___tag_value_nss_sob.179-..___tag_value_nss_sob.166
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_nss_sob.171-..___tag_value_nss_sob.166
	.byte 0x0c
	.2byte 0x0805
	.byte 0x85
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_sob.174-..___tag_value_nss_sob.171
	.byte 0x83
	.byte 0x03
	.2byte 0x0000
	.4byte 0x0000001c
	.4byte .debug_frame_seg
	.4byte ..___tag_value_nss_FASTA_expr.182
	.4byte ..___tag_value_nss_FASTA_expr.193-..___tag_value_nss_FASTA_expr.182
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.187-..___tag_value_nss_FASTA_expr.182
	.byte 0x0c
	.2byte 0x0805
	.byte 0x85
	.byte 0x02
	.byte 0x00
	.4byte 0x00000034
	.4byte .debug_frame_seg
	.4byte ..___tag_value_nss_fasta_expr.196
	.4byte ..___tag_value_nss_fasta_expr.216-..___tag_value_nss_fasta_expr.196
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.201-..___tag_value_nss_fasta_expr.196
	.byte 0x0c
	.2byte 0x0805
	.byte 0x85
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.204-..___tag_value_nss_fasta_expr.201
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.207-..___tag_value_nss_fasta_expr.204
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.211-..___tag_value_nss_fasta_expr.207
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.2byte 0x0000
	.4byte 0x0000003c
	.4byte .debug_frame_seg
	.4byte ..___tag_value_nss_primary_expr.219
	.4byte ..___tag_value_nss_primary_expr.244-..___tag_value_nss_primary_expr.219
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.224-..___tag_value_nss_primary_expr.219
	.byte 0x0c
	.2byte 0x0805
	.byte 0x85
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.227-..___tag_value_nss_primary_expr.224
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.230-..___tag_value_nss_primary_expr.227
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.234-..___tag_value_nss_primary_expr.230
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.239-..___tag_value_nss_primary_expr.234
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x00
	.4byte 0x0000003c
	.4byte .debug_frame_seg
	.4byte ..___tag_value_nss_unary_expr.247
	.4byte ..___tag_value_nss_unary_expr.269-..___tag_value_nss_unary_expr.247
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.252-..___tag_value_nss_unary_expr.247
	.byte 0x0c
	.2byte 0x0805
	.byte 0x85
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.255-..___tag_value_nss_unary_expr.252
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.258-..___tag_value_nss_unary_expr.255
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.262-..___tag_value_nss_unary_expr.258
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.265-..___tag_value_nss_unary_expr.262
	.byte 0x83
	.byte 0x03
	.2byte 0x0000
	.byte 0x00
	.4byte 0x0000004c
	.4byte .debug_frame_seg
	.4byte ..___tag_value_nss_expr.272
	.4byte ..___tag_value_nss_expr.304-..___tag_value_nss_expr.272
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_nss_expr.277-..___tag_value_nss_expr.272
	.byte 0x0c
	.2byte 0x0805
	.byte 0x85
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_expr.280-..___tag_value_nss_expr.277
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_nss_expr.283-..___tag_value_nss_expr.280
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_nss_expr.287-..___tag_value_nss_expr.283
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_nss_expr.292-..___tag_value_nss_expr.287
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_nss_expr.297-..___tag_value_nss_expr.292
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_nss_expr.300-..___tag_value_nss_expr.297
	.byte 0x83
	.byte 0x03
	.byte 0x00
	.4byte 0x00000024
	.4byte .debug_frame_seg
	.4byte ..___tag_value_NucStrstrInit.307
	.4byte ..___tag_value_NucStrstrInit.320-..___tag_value_NucStrstrInit.307
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_NucStrstrInit.312-..___tag_value_NucStrstrInit.307
	.byte 0x0c
	.2byte 0x0805
	.byte 0x85
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrInit.315-..___tag_value_NucStrstrInit.312
	.byte 0x83
	.byte 0x05
	.2byte 0x0000
	.4byte 0x00000034
	.4byte .debug_frame_seg
	.4byte ..___tag_value_NucStrstrMake.323
	.4byte ..___tag_value_NucStrstrMake.346-..___tag_value_NucStrstrMake.323
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.328-..___tag_value_NucStrstrMake.323
	.byte 0x0c
	.2byte 0x0805
	.byte 0x85
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.331-..___tag_value_NucStrstrMake.328
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.336-..___tag_value_NucStrstrMake.331
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.341-..___tag_value_NucStrstrMake.336
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.4byte 0x00000024
	.4byte .debug_frame_seg
	.4byte ..___tag_value_NucStrstrWhack.349
	.4byte ..___tag_value_NucStrstrWhack.362-..___tag_value_NucStrstrWhack.349
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_NucStrstrWhack.354-..___tag_value_NucStrstrWhack.349
	.byte 0x0c
	.2byte 0x0805
	.byte 0x85
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrWhack.357-..___tag_value_NucStrstrWhack.354
	.byte 0x83
	.byte 0x03
	.2byte 0x0000
	.4byte 0x000000c0
	.4byte .debug_frame_seg
	.4byte ..___tag_value_eval_2na_8.365
	.4byte ..___tag_value_eval_2na_8.417-..___tag_value_eval_2na_8.365
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.370-..___tag_value_eval_2na_8.365
	.byte 0x0c
	.2byte 0x0804
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.371-..___tag_value_eval_2na_8.370
	.byte 0x0c
	.2byte 0x0803
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.373-..___tag_value_eval_2na_8.371
	.byte 0x10
	.4byte 0x00750205
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.376-..___tag_value_eval_2na_8.373
	.byte 0x0f
	.4byte 0x7de87506
	.2byte 0x2306
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.377-..___tag_value_eval_2na_8.376
	.byte 0x0c
	.2byte 0x0803
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.380-..___tag_value_eval_2na_8.377
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.382-..___tag_value_eval_2na_8.380
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.387-..___tag_value_eval_2na_8.382
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.389-..___tag_value_eval_2na_8.387
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.394-..___tag_value_eval_2na_8.389
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.396-..___tag_value_eval_2na_8.394
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.401-..___tag_value_eval_2na_8.396
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.403-..___tag_value_eval_2na_8.401
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.408-..___tag_value_eval_2na_8.403
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.410-..___tag_value_eval_2na_8.408
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.415-..___tag_value_eval_2na_8.410
	.byte 0x0c
	.2byte 0x0404
	.2byte 0x0000
	.byte 0x00
	.4byte 0x000000c0
	.4byte .debug_frame_seg
	.4byte ..___tag_value_eval_2na_16.420
	.4byte ..___tag_value_eval_2na_16.472-..___tag_value_eval_2na_16.420
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.425-..___tag_value_eval_2na_16.420
	.byte 0x0c
	.2byte 0x0804
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.426-..___tag_value_eval_2na_16.425
	.byte 0x0c
	.2byte 0x0803
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.428-..___tag_value_eval_2na_16.426
	.byte 0x10
	.4byte 0x00750205
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.431-..___tag_value_eval_2na_16.428
	.byte 0x0f
	.4byte 0x7dd87506
	.2byte 0x2306
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.432-..___tag_value_eval_2na_16.431
	.byte 0x0c
	.2byte 0x0803
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.435-..___tag_value_eval_2na_16.432
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.437-..___tag_value_eval_2na_16.435
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.442-..___tag_value_eval_2na_16.437
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.444-..___tag_value_eval_2na_16.442
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.449-..___tag_value_eval_2na_16.444
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.451-..___tag_value_eval_2na_16.449
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.456-..___tag_value_eval_2na_16.451
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.458-..___tag_value_eval_2na_16.456
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.463-..___tag_value_eval_2na_16.458
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.465-..___tag_value_eval_2na_16.463
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.470-..___tag_value_eval_2na_16.465
	.byte 0x0c
	.2byte 0x0404
	.2byte 0x0000
	.byte 0x00
	.4byte 0x000000c0
	.4byte .debug_frame_seg
	.4byte ..___tag_value_eval_2na_32.475
	.4byte ..___tag_value_eval_2na_32.527-..___tag_value_eval_2na_32.475
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.480-..___tag_value_eval_2na_32.475
	.byte 0x0c
	.2byte 0x0804
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.481-..___tag_value_eval_2na_32.480
	.byte 0x0c
	.2byte 0x0803
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.483-..___tag_value_eval_2na_32.481
	.byte 0x10
	.4byte 0x00750205
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.486-..___tag_value_eval_2na_32.483
	.byte 0x0f
	.4byte 0x7dd87506
	.2byte 0x2306
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.487-..___tag_value_eval_2na_32.486
	.byte 0x0c
	.2byte 0x0803
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.490-..___tag_value_eval_2na_32.487
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.492-..___tag_value_eval_2na_32.490
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.497-..___tag_value_eval_2na_32.492
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.499-..___tag_value_eval_2na_32.497
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.504-..___tag_value_eval_2na_32.499
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.506-..___tag_value_eval_2na_32.504
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.511-..___tag_value_eval_2na_32.506
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.513-..___tag_value_eval_2na_32.511
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.518-..___tag_value_eval_2na_32.513
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.520-..___tag_value_eval_2na_32.518
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.525-..___tag_value_eval_2na_32.520
	.byte 0x0c
	.2byte 0x0404
	.2byte 0x0000
	.byte 0x00
	.4byte 0x000000c0
	.4byte .debug_frame_seg
	.4byte ..___tag_value_eval_2na_64.530
	.4byte ..___tag_value_eval_2na_64.582-..___tag_value_eval_2na_64.530
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.535-..___tag_value_eval_2na_64.530
	.byte 0x0c
	.2byte 0x0804
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.536-..___tag_value_eval_2na_64.535
	.byte 0x0c
	.2byte 0x0803
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.538-..___tag_value_eval_2na_64.536
	.byte 0x10
	.4byte 0x00750205
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.541-..___tag_value_eval_2na_64.538
	.byte 0x0f
	.4byte 0x7dd87506
	.2byte 0x2306
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.542-..___tag_value_eval_2na_64.541
	.byte 0x0c
	.2byte 0x0803
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.545-..___tag_value_eval_2na_64.542
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.547-..___tag_value_eval_2na_64.545
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.552-..___tag_value_eval_2na_64.547
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.554-..___tag_value_eval_2na_64.552
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.559-..___tag_value_eval_2na_64.554
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.561-..___tag_value_eval_2na_64.559
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.566-..___tag_value_eval_2na_64.561
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.568-..___tag_value_eval_2na_64.566
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.573-..___tag_value_eval_2na_64.568
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.575-..___tag_value_eval_2na_64.573
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.580-..___tag_value_eval_2na_64.575
	.byte 0x0c
	.2byte 0x0404
	.2byte 0x0000
	.byte 0x00
	.4byte 0x000000d4
	.4byte .debug_frame_seg
	.4byte ..___tag_value_eval_2na_128.585
	.4byte ..___tag_value_eval_2na_128.644-..___tag_value_eval_2na_128.585
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.590-..___tag_value_eval_2na_128.585
	.byte 0x0c
	.2byte 0x0804
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.591-..___tag_value_eval_2na_128.590
	.byte 0x0c
	.2byte 0x0803
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.593-..___tag_value_eval_2na_128.591
	.byte 0x10
	.4byte 0x00750205
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.596-..___tag_value_eval_2na_128.593
	.byte 0x0f
	.4byte 0x7df87506
	.2byte 0x2306
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.597-..___tag_value_eval_2na_128.596
	.byte 0x0c
	.2byte 0x0803
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.600-..___tag_value_eval_2na_128.597
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.602-..___tag_value_eval_2na_128.600
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.607-..___tag_value_eval_2na_128.602
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.609-..___tag_value_eval_2na_128.607
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.614-..___tag_value_eval_2na_128.609
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.616-..___tag_value_eval_2na_128.614
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.621-..___tag_value_eval_2na_128.616
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.623-..___tag_value_eval_2na_128.621
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.628-..___tag_value_eval_2na_128.623
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.630-..___tag_value_eval_2na_128.628
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.635-..___tag_value_eval_2na_128.630
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.637-..___tag_value_eval_2na_128.635
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.642-..___tag_value_eval_2na_128.637
	.byte 0x0c
	.2byte 0x0404
	.4byte 0x00000160
	.4byte .debug_frame_seg
	.4byte ..___tag_value_eval_2na_pos.647
	.4byte ..___tag_value_eval_2na_pos.748-..___tag_value_eval_2na_pos.647
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.652-..___tag_value_eval_2na_pos.647
	.byte 0x0c
	.2byte 0x0804
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.653-..___tag_value_eval_2na_pos.652
	.byte 0x0c
	.2byte 0x0803
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.655-..___tag_value_eval_2na_pos.653
	.byte 0x10
	.4byte 0x00750205
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.658-..___tag_value_eval_2na_pos.655
	.byte 0x0f
	.4byte 0x7df87506
	.2byte 0x2306
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.659-..___tag_value_eval_2na_pos.658
	.byte 0x0c
	.2byte 0x0803
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.662-..___tag_value_eval_2na_pos.659
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.664-..___tag_value_eval_2na_pos.662
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.669-..___tag_value_eval_2na_pos.664
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.671-..___tag_value_eval_2na_pos.669
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.676-..___tag_value_eval_2na_pos.671
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.678-..___tag_value_eval_2na_pos.676
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.683-..___tag_value_eval_2na_pos.678
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.685-..___tag_value_eval_2na_pos.683
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.690-..___tag_value_eval_2na_pos.685
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.692-..___tag_value_eval_2na_pos.690
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.697-..___tag_value_eval_2na_pos.692
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.699-..___tag_value_eval_2na_pos.697
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.704-..___tag_value_eval_2na_pos.699
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.706-..___tag_value_eval_2na_pos.704
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.711-..___tag_value_eval_2na_pos.706
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.713-..___tag_value_eval_2na_pos.711
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.718-..___tag_value_eval_2na_pos.713
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.720-..___tag_value_eval_2na_pos.718
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.725-..___tag_value_eval_2na_pos.720
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.727-..___tag_value_eval_2na_pos.725
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.732-..___tag_value_eval_2na_pos.727
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.734-..___tag_value_eval_2na_pos.732
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.739-..___tag_value_eval_2na_pos.734
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.741-..___tag_value_eval_2na_pos.739
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.746-..___tag_value_eval_2na_pos.741
	.byte 0x0c
	.2byte 0x0404
	.2byte 0x0000
	.4byte 0x000000c0
	.4byte .debug_frame_seg
	.4byte ..___tag_value_eval_4na_16.751
	.4byte ..___tag_value_eval_4na_16.803-..___tag_value_eval_4na_16.751
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.756-..___tag_value_eval_4na_16.751
	.byte 0x0c
	.2byte 0x0804
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.757-..___tag_value_eval_4na_16.756
	.byte 0x0c
	.2byte 0x0803
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.759-..___tag_value_eval_4na_16.757
	.byte 0x10
	.4byte 0x00750205
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.762-..___tag_value_eval_4na_16.759
	.byte 0x0f
	.4byte 0x7dd87506
	.2byte 0x2306
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.763-..___tag_value_eval_4na_16.762
	.byte 0x0c
	.2byte 0x0803
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.766-..___tag_value_eval_4na_16.763
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.768-..___tag_value_eval_4na_16.766
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.773-..___tag_value_eval_4na_16.768
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.775-..___tag_value_eval_4na_16.773
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.780-..___tag_value_eval_4na_16.775
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.782-..___tag_value_eval_4na_16.780
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.787-..___tag_value_eval_4na_16.782
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.789-..___tag_value_eval_4na_16.787
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.794-..___tag_value_eval_4na_16.789
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.796-..___tag_value_eval_4na_16.794
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.801-..___tag_value_eval_4na_16.796
	.byte 0x0c
	.2byte 0x0404
	.2byte 0x0000
	.byte 0x00
	.4byte 0x000000c0
	.4byte .debug_frame_seg
	.4byte ..___tag_value_eval_4na_32.806
	.4byte ..___tag_value_eval_4na_32.858-..___tag_value_eval_4na_32.806
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.811-..___tag_value_eval_4na_32.806
	.byte 0x0c
	.2byte 0x0804
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.812-..___tag_value_eval_4na_32.811
	.byte 0x0c
	.2byte 0x0803
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.814-..___tag_value_eval_4na_32.812
	.byte 0x10
	.4byte 0x00750205
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.817-..___tag_value_eval_4na_32.814
	.byte 0x0f
	.4byte 0x7dc87506
	.2byte 0x2306
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.818-..___tag_value_eval_4na_32.817
	.byte 0x0c
	.2byte 0x0803
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.821-..___tag_value_eval_4na_32.818
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.823-..___tag_value_eval_4na_32.821
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.828-..___tag_value_eval_4na_32.823
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.830-..___tag_value_eval_4na_32.828
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.835-..___tag_value_eval_4na_32.830
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.837-..___tag_value_eval_4na_32.835
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.842-..___tag_value_eval_4na_32.837
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.844-..___tag_value_eval_4na_32.842
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.849-..___tag_value_eval_4na_32.844
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.851-..___tag_value_eval_4na_32.849
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.856-..___tag_value_eval_4na_32.851
	.byte 0x0c
	.2byte 0x0404
	.2byte 0x0000
	.byte 0x00
	.4byte 0x000000c0
	.4byte .debug_frame_seg
	.4byte ..___tag_value_eval_4na_64.861
	.4byte ..___tag_value_eval_4na_64.913-..___tag_value_eval_4na_64.861
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.866-..___tag_value_eval_4na_64.861
	.byte 0x0c
	.2byte 0x0804
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.867-..___tag_value_eval_4na_64.866
	.byte 0x0c
	.2byte 0x0803
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.869-..___tag_value_eval_4na_64.867
	.byte 0x10
	.4byte 0x00750205
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.872-..___tag_value_eval_4na_64.869
	.byte 0x0f
	.4byte 0x7dc87506
	.2byte 0x2306
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.873-..___tag_value_eval_4na_64.872
	.byte 0x0c
	.2byte 0x0803
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.876-..___tag_value_eval_4na_64.873
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.878-..___tag_value_eval_4na_64.876
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.883-..___tag_value_eval_4na_64.878
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.885-..___tag_value_eval_4na_64.883
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.890-..___tag_value_eval_4na_64.885
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.892-..___tag_value_eval_4na_64.890
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.897-..___tag_value_eval_4na_64.892
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.899-..___tag_value_eval_4na_64.897
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.904-..___tag_value_eval_4na_64.899
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.906-..___tag_value_eval_4na_64.904
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.911-..___tag_value_eval_4na_64.906
	.byte 0x0c
	.2byte 0x0404
	.2byte 0x0000
	.byte 0x00
	.4byte 0x000000d4
	.4byte .debug_frame_seg
	.4byte ..___tag_value_eval_4na_128.916
	.4byte ..___tag_value_eval_4na_128.975-..___tag_value_eval_4na_128.916
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.921-..___tag_value_eval_4na_128.916
	.byte 0x0c
	.2byte 0x0804
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.922-..___tag_value_eval_4na_128.921
	.byte 0x0c
	.2byte 0x0803
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.924-..___tag_value_eval_4na_128.922
	.byte 0x10
	.4byte 0x00750205
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.927-..___tag_value_eval_4na_128.924
	.byte 0x0f
	.4byte 0x7de87506
	.2byte 0x2306
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.928-..___tag_value_eval_4na_128.927
	.byte 0x0c
	.2byte 0x0803
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.931-..___tag_value_eval_4na_128.928
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.933-..___tag_value_eval_4na_128.931
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.938-..___tag_value_eval_4na_128.933
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.940-..___tag_value_eval_4na_128.938
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.945-..___tag_value_eval_4na_128.940
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.947-..___tag_value_eval_4na_128.945
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.952-..___tag_value_eval_4na_128.947
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.954-..___tag_value_eval_4na_128.952
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.959-..___tag_value_eval_4na_128.954
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.961-..___tag_value_eval_4na_128.959
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.966-..___tag_value_eval_4na_128.961
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.968-..___tag_value_eval_4na_128.966
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.973-..___tag_value_eval_4na_128.968
	.byte 0x0c
	.2byte 0x0404
	.4byte 0x00000160
	.4byte .debug_frame_seg
	.4byte ..___tag_value_eval_4na_pos.978
	.4byte ..___tag_value_eval_4na_pos.1079-..___tag_value_eval_4na_pos.978
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.983-..___tag_value_eval_4na_pos.978
	.byte 0x0c
	.2byte 0x0804
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.984-..___tag_value_eval_4na_pos.983
	.byte 0x0c
	.2byte 0x0803
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.986-..___tag_value_eval_4na_pos.984
	.byte 0x10
	.4byte 0x00750205
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.989-..___tag_value_eval_4na_pos.986
	.byte 0x0f
	.4byte 0x7de87506
	.2byte 0x2306
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.990-..___tag_value_eval_4na_pos.989
	.byte 0x0c
	.2byte 0x0803
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.993-..___tag_value_eval_4na_pos.990
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.995-..___tag_value_eval_4na_pos.993
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1000-..___tag_value_eval_4na_pos.995
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1002-..___tag_value_eval_4na_pos.1000
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1007-..___tag_value_eval_4na_pos.1002
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1009-..___tag_value_eval_4na_pos.1007
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1014-..___tag_value_eval_4na_pos.1009
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1016-..___tag_value_eval_4na_pos.1014
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1021-..___tag_value_eval_4na_pos.1016
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1023-..___tag_value_eval_4na_pos.1021
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1028-..___tag_value_eval_4na_pos.1023
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1030-..___tag_value_eval_4na_pos.1028
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1035-..___tag_value_eval_4na_pos.1030
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1037-..___tag_value_eval_4na_pos.1035
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1042-..___tag_value_eval_4na_pos.1037
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1044-..___tag_value_eval_4na_pos.1042
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1049-..___tag_value_eval_4na_pos.1044
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1051-..___tag_value_eval_4na_pos.1049
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1056-..___tag_value_eval_4na_pos.1051
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1058-..___tag_value_eval_4na_pos.1056
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1063-..___tag_value_eval_4na_pos.1058
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1065-..___tag_value_eval_4na_pos.1063
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1070-..___tag_value_eval_4na_pos.1065
	.byte 0x0c
	.2byte 0x0404
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1072-..___tag_value_eval_4na_pos.1070
	.byte 0x0c
	.2byte 0x0803
	.byte 0x10
	.4byte 0x00750205
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.1077-..___tag_value_eval_4na_pos.1072
	.byte 0x0c
	.2byte 0x0404
	.2byte 0x0000
	.4byte 0x0000013c
	.4byte .debug_frame_seg
	.4byte ..___tag_value_NucStrstrSearch.1082
	.4byte ..___tag_value_NucStrstrSearch.1250-..___tag_value_NucStrstrSearch.1082
	.byte 0x04
	.4byte 0x00000000
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1087-..___tag_value_NucStrstrSearch.1082
	.byte 0x0c
	.2byte 0x0805
	.byte 0x85
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1090-..___tag_value_NucStrstrSearch.1087
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1095-..___tag_value_NucStrstrSearch.1090
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1100-..___tag_value_NucStrstrSearch.1095
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1105-..___tag_value_NucStrstrSearch.1100
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1110-..___tag_value_NucStrstrSearch.1105
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1115-..___tag_value_NucStrstrSearch.1110
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1120-..___tag_value_NucStrstrSearch.1115
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1125-..___tag_value_NucStrstrSearch.1120
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1130-..___tag_value_NucStrstrSearch.1125
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1135-..___tag_value_NucStrstrSearch.1130
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1140-..___tag_value_NucStrstrSearch.1135
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1145-..___tag_value_NucStrstrSearch.1140
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1150-..___tag_value_NucStrstrSearch.1145
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1155-..___tag_value_NucStrstrSearch.1150
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1160-..___tag_value_NucStrstrSearch.1155
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1165-..___tag_value_NucStrstrSearch.1160
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1170-..___tag_value_NucStrstrSearch.1165
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1175-..___tag_value_NucStrstrSearch.1170
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1180-..___tag_value_NucStrstrSearch.1175
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1185-..___tag_value_NucStrstrSearch.1180
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1190-..___tag_value_NucStrstrSearch.1185
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1195-..___tag_value_NucStrstrSearch.1190
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1200-..___tag_value_NucStrstrSearch.1195
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1205-..___tag_value_NucStrstrSearch.1200
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1210-..___tag_value_NucStrstrSearch.1205
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1215-..___tag_value_NucStrstrSearch.1210
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1220-..___tag_value_NucStrstrSearch.1215
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1225-..___tag_value_NucStrstrSearch.1220
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1230-..___tag_value_NucStrstrSearch.1225
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1235-..___tag_value_NucStrstrSearch.1230
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1240-..___tag_value_NucStrstrSearch.1235
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.1245-..___tag_value_NucStrstrSearch.1240
	.byte 0x85
	.byte 0x02
	.byte 0x83
	.byte 0x03
	.2byte 0x0000
	.byte 0x00
	.section .text
.LNDBG_TXe:
# End
