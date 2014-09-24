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
# mark_description "/yaschenk/devel/internal/asm-trace/inc -c -S -o nucstrstr.pic.s -fPIC -O3 -unroll -xW -vec_report5 -DNDEBUG ";
# mark_description "-D_PROFILING -p -DLINUX -DUNIX -D_GNU_SOURCE -D_REENTRANT -D_FILE_OFFSET_BITS=64 -MD";
	.file "nucstrstr.c"
	.text
..TXTST0:
# -- Begin  NucStrstrSearch
# mark_begin;
       .align    16,0x90
	.globl NucStrstrSearch
NucStrstrSearch:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
# parameter 5(selflen): %r8
..B1.1:                         # Preds ..B1.0
..___tag_value_NucStrstrSearch.1:                               #5217.1
..LN1:
  .file   1 "nucstrstr.c"
   .loc    1  5217
        pushq     %rbp                                          #5217.1
        movq      %rsp, %rbp                                    #5217.1
..___tag_value_NucStrstrSearch.2:                               #
        subq      $48, %rsp                                     #5217.1
        movq      %r15, -32(%rbp)                               #5217.1
..___tag_value_NucStrstrSearch.4:                               #
        movq      %r14, -24(%rbp)                               #5217.1
..___tag_value_NucStrstrSearch.5:                               #
        movq      %r13, -16(%rbp)                               #5217.1
..___tag_value_NucStrstrSearch.6:                               #
        movq      %r12, -48(%rbp)                               #5217.1
..___tag_value_NucStrstrSearch.7:                               #
        movq      %rbx, -40(%rbp)                               #5217.1
..___tag_value_NucStrstrSearch.8:                               #
        movq      %r8, %r15                                     #5217.1
        movl      %ecx, %r14d                                   #5217.1
        movl      %edx, %r13d                                   #5217.1
        movq      %rsi, %r12                                    #5217.1
        movq      %rdi, %rbx                                    #5217.1
        lea       _gprof_pack0(%rip), %rdx                      #5217.1
        call      mcount@PLT                                    #5217.1
                                # LOE rbx r12 r15 r13d r14d
..B1.117:                       # Preds ..B1.1
..LN3:
   .loc    1  5218
        testq     %rbx, %rbx                                    #5218.18
        je        ..B1.107      # Prob 6%                       #5218.18
                                # LOE rbx r12 r15 r13d r14d
..B1.2:                         # Preds ..B1.117
..LN5:
        testq     %r12, %r12                                    #5218.37
        je        ..B1.107      # Prob 6%                       #5218.37
                                # LOE rbx r12 r15 r13d r14d
..B1.4:                         # Preds ..B1.2 ..B1.14 ..B1.17
..LN7:
        testl     %r14d, %r14d                                  #5218.52
        je        ..B1.107      # Prob 28%                      #5218.52
                                # LOE rbx r12 r15 r13d r14d
..B1.7:                         # Preds ..B1.4 ..B1.31
..LN9:
   .loc    1  5223
        movl      (%rbx), %eax                                  #5223.18
..LN11:
        cmpl      $12, %eax                                     #5223.9
        ja        ..B1.107      # Prob 28%                      #5223.9
                                # LOE rbx r12 r15 eax r13d r14d
..B1.8:                         # Preds ..B1.7
        movl      %eax, %eax                                    #5223.9
        lea       ..1..TPKT.30_0.0.1(%rip), %rdx                #5223.9
        movq      (%rdx,%rax,8), %rcx                           #5223.9
        jmp       *%rcx                                         #5223.9
                                # LOE rbx r12 r15 r13d r14d
..1.30_0.TAG.0c.0.1:
..B1.10:                        # Preds ..B1.8
..LN13:
   .loc    1  5286
        movl      4(%rbx), %eax                                 #5286.22
        testl     %eax, %eax                                    #5286.22
        je        ..B1.20       # Prob 20%                      #5286.22
                                # LOE rbx r12 r15 eax r13d r14d
..B1.11:                        # Preds ..B1.10
        cmpl      $13, %eax                                     #5286.22
        je        ..B1.20       # Prob 25%                      #5286.22
                                # LOE rbx r12 r15 eax r13d r14d
..B1.12:                        # Preds ..B1.11
        cmpl      $14, %eax                                     #5286.22
        jne       ..B1.15       # Prob 67%                      #5286.22
                                # LOE rbx r12 r15 eax r13d r14d
..B1.13:                        # Preds ..B1.12
..LN15:
   .loc    1  5297
        movq      8(%rbx), %rbx                                 #5297.29
        movl      4(%rbx), %eax                                 #5297.29
..LN17:
   .loc    1  5298
        cmpl      %r14d, %eax                                   #5298.34
        ja        ..B1.107      # Prob 12%                      #5298.34
                                # LOE rbx r12 r15 eax r13d
..B1.14:                        # Preds ..B1.13
..LN19:
   .loc    1  5297
        movl      %eax, %r14d                                   #5297.17
..LN21:
   .loc    1  5218
        testq     %rbx, %rbx                                    #5218.18
        jne       ..B1.4        # Prob 94%                      #5218.18
        jmp       ..B1.107      # Prob 100%                     #5218.18
                                # LOE rbx r12 r15 r13d r14d
..B1.15:                        # Preds ..B1.12
..LN23:
   .loc    1  5286
        cmpl      $15, %eax                                     #5286.22
        jne       ..B1.107      # Prob 50%                      #5286.22
                                # LOE rbx r12 r15 r13d r14d
..B1.16:                        # Preds ..B1.15
..LN25:
   .loc    1  5302
        movq      8(%rbx), %rbx                                 #5302.29
        movl      4(%rbx), %eax                                 #5302.29
..LN27:
   .loc    1  5303
        cmpl      %r14d, %eax                                   #5303.34
        ja        ..B1.107      # Prob 12%                      #5303.34
                                # LOE rbx r12 r15 eax r13d r14d
..B1.17:                        # Preds ..B1.16
..LN29:
   .loc    1  5306
        addl      %r14d, %r13d                                  #5306.27
..LN31:
        subl      %eax, %r13d                                   #5306.33
..LN33:
   .loc    1  5302
        movl      %eax, %r14d                                   #5302.17
..LN35:
   .loc    1  5218
        testq     %rbx, %rbx                                    #5218.18
        jne       ..B1.4        # Prob 94%                      #5218.18
        jmp       ..B1.107      # Prob 100%                     #5218.18
                                # LOE rbx r12 r15 r13d r14d
..B1.20:                        # Preds ..B1.10 ..B1.11
..LN37:
   .loc    1  5290
        movq      8(%rbx), %rdi                                 #5290.25
        movq      %r12, %rsi                                    #5290.25
        movl      %r13d, %edx                                   #5290.25
        movl      %r14d, %ecx                                   #5290.25
        movq      %r15, %r8                                     #5290.25
        call      NucStrstrSearch@PLT                           #5290.25
                                # LOE rbx eax
..B1.21:                        # Preds ..B1.20
..LN39:
   .loc    1  5291
        movl      4(%rbx), %edx                                 #5291.22
..LN41:
        testl     %edx, %edx                                    #5291.42
        je        ..B1.56       # Prob 28%                      #5291.42
                                # LOE eax
..B1.22:                        # Preds ..B1.21
..LN43:
   .loc    1  5293
        testl     %eax, %eax                                    #5293.31
        jne       ..B1.107      # Prob 50%                      #5293.31
                                # LOE
..B1.23:                        # Preds ..B1.22
..LN45:
   .loc    1  5294
        movq      -40(%rbp), %rbx                               #5294.28
..___tag_value_NucStrstrSearch.9:                               #
        movq      -48(%rbp), %r12                               #5294.28
..___tag_value_NucStrstrSearch.10:                              #
        movq      -16(%rbp), %r13                               #5294.28
..___tag_value_NucStrstrSearch.11:                              #
        movq      -24(%rbp), %r14                               #5294.28
..___tag_value_NucStrstrSearch.12:                              #
        movl      $1, %eax                                      #5294.28
        movq      -32(%rbp), %r15                               #5294.28
..___tag_value_NucStrstrSearch.13:                              #
        movq      %rbp, %rsp                                    #5294.28
        popq      %rbp                                          #5294.28
..___tag_value_NucStrstrSearch.14:                              #
        ret                                                     #5294.28
..___tag_value_NucStrstrSearch.15:                              #
                                # LOE
..1.30_0.TAG.0b.0.1:
..B1.26:                        # Preds ..B1.8
..LN47:
   .loc    1  5272
        movq      8(%rbx), %rdi                                 #5272.21
        movq      %r12, %rsi                                    #5272.21
        movl      %r13d, %edx                                   #5272.21
        movl      %r14d, %ecx                                   #5272.21
        movq      %r15, %r8                                     #5272.21
        call      NucStrstrSearch@PLT                           #5272.21
                                # LOE rbx r12 r15 eax r13d r14d
..B1.27:                        # Preds ..B1.26
..LN49:
   .loc    1  5273
        movl      4(%rbx), %edx                                 #5273.22
        cmpl      $17, %edx                                     #5273.22
        jne       ..B1.29       # Prob 67%                      #5273.22
                                # LOE rbx r12 r15 eax edx r13d r14d
..B1.28:                        # Preds ..B1.27
..LN51:
   .loc    1  5276
        testl     %eax, %eax                                    #5276.31
        jne       ..B1.56       # Prob 12%                      #5276.31
        jmp       ..B1.31       # Prob 100%                     #5276.31
                                # LOE rbx r12 r15 eax r13d r14d
..B1.29:                        # Preds ..B1.27
..LN53:
   .loc    1  5273
        cmpl      $16, %edx                                     #5273.22
        jne       ..B1.31       # Prob 50%                      #5273.22
                                # LOE rbx r12 r15 eax r13d r14d
..B1.30:                        # Preds ..B1.29
..LN55:
   .loc    1  5280
        testl     %eax, %eax                                    #5280.31
        je        ..B1.56       # Prob 12%                      #5280.31
                                # LOE rbx r12 r15 eax r13d r14d
..B1.31:                        # Preds ..B1.28 ..B1.30 ..B1.29
..LN57:
   .loc    1  5284
        movq      16(%rbx), %rbx                                #5284.38
..LN59:
   .loc    1  5218
        testq     %rbx, %rbx                                    #5218.18
        jne       ..B1.7        # Prob 94%                      #5218.18
        jmp       ..B1.107      # Prob 100%                     #5218.18
                                # LOE rbx r12 r15 r13d r14d
..1.30_0.TAG.0a.0.1:
..B1.34:                        # Preds ..B1.8
..LN61:
   .loc    1  5268
        movl      4(%rbx), %eax                                 #5268.24
        cmpl      %eax, %r14d                                   #5268.24
        jb        ..B1.107      # Prob 28%                      #5268.24
                                # LOE rbx r12 r15 eax r13d r14d
..B1.35:                        # Preds ..B1.34
..LN63:
   .loc    1  5269
        testq     %r15, %r15                                    #5269.9
        je        ..B1.37       # Prob 12%                      #5269.9
                                # LOE rbx r12 r15 eax r13d r14d
..B1.36:                        # Preds ..B1.35
..LN65:
        movl      %eax, (%r15)                                  #5269.18
                                # LOE rbx r12 r13d r14d
..B1.37:                        # Preds ..B1.35 ..B1.36
..LN67:
   .loc    1  5270
        movq      %rbx, %rdi                                    #5270.20
        movq      %r12, %rsi                                    #5270.20
        movl      %r13d, %edx                                   #5270.20
        movl      %r14d, %ecx                                   #5270.20
        call      eval_4na_pos@PLT                              #5270.20
                                # LOE eax
..B1.120:                       # Preds ..B1.37
        movq      -40(%rbp), %rbx                               #5270.20
..___tag_value_NucStrstrSearch.21:                              #
        movq      -48(%rbp), %r12                               #5270.20
..___tag_value_NucStrstrSearch.22:                              #
        movq      -16(%rbp), %r13                               #5270.20
..___tag_value_NucStrstrSearch.23:                              #
        movq      -24(%rbp), %r14                               #5270.20
..___tag_value_NucStrstrSearch.24:                              #
        movq      -32(%rbp), %r15                               #5270.20
..___tag_value_NucStrstrSearch.25:                              #
        movq      %rbp, %rsp                                    #5270.20
        popq      %rbp                                          #5270.20
..___tag_value_NucStrstrSearch.26:                              #
        ret                                                     #5270.20
..___tag_value_NucStrstrSearch.27:                              #
                                # LOE
..1.30_0.TAG.09.0.1:
..B1.40:                        # Preds ..B1.8
..LN69:
   .loc    1  5264
        movl      4(%rbx), %eax                                 #5264.24
        cmpl      %eax, %r14d                                   #5264.24
        jb        ..B1.107      # Prob 28%                      #5264.24
                                # LOE rbx r12 r15 eax r13d r14d
..B1.41:                        # Preds ..B1.40
..LN71:
   .loc    1  5265
        testq     %r15, %r15                                    #5265.9
        je        ..B1.43       # Prob 12%                      #5265.9
                                # LOE rbx r12 r15 eax r13d r14d
..B1.42:                        # Preds ..B1.41
..LN73:
        movl      %eax, (%r15)                                  #5265.18
                                # LOE rbx r12 r13d r14d
..B1.43:                        # Preds ..B1.41 ..B1.42
..LN75:
   .loc    1  5266
        movq      %rbx, %rdi                                    #5266.20
        movq      %r12, %rsi                                    #5266.20
        movl      %r13d, %edx                                   #5266.20
        movl      %r14d, %ecx                                   #5266.20
        call      eval_2na_pos@PLT                              #5266.20
                                # LOE eax
..B1.121:                       # Preds ..B1.43
        movq      -40(%rbp), %rbx                               #5266.20
..___tag_value_NucStrstrSearch.33:                              #
        movq      -48(%rbp), %r12                               #5266.20
..___tag_value_NucStrstrSearch.34:                              #
        movq      -16(%rbp), %r13                               #5266.20
..___tag_value_NucStrstrSearch.35:                              #
        movq      -24(%rbp), %r14                               #5266.20
..___tag_value_NucStrstrSearch.36:                              #
        movq      -32(%rbp), %r15                               #5266.20
..___tag_value_NucStrstrSearch.37:                              #
        movq      %rbp, %rsp                                    #5266.20
        popq      %rbp                                          #5266.20
..___tag_value_NucStrstrSearch.38:                              #
        ret                                                     #5266.20
..___tag_value_NucStrstrSearch.39:                              #
                                # LOE
..1.30_0.TAG.08.0.1:
..B1.46:                        # Preds ..B1.8
..LN77:
   .loc    1  5259
        movl      4(%rbx), %eax                                 #5259.24
        cmpl      %eax, %r14d                                   #5259.24
        jb        ..B1.107      # Prob 28%                      #5259.24
                                # LOE rbx r12 r15 eax r13d r14d
..B1.47:                        # Preds ..B1.46
..LN79:
   .loc    1  5260
        testq     %r15, %r15                                    #5260.9
        je        ..B1.49       # Prob 12%                      #5260.9
                                # LOE rbx r12 r15 eax r13d r14d
..B1.48:                        # Preds ..B1.47
..LN81:
        movl      %eax, (%r15)                                  #5260.18
                                # LOE rbx r12 r13d r14d
..B1.49:                        # Preds ..B1.47 ..B1.48
..LN83:
   .loc    1  5261
        movq      %rbx, %rdi                                    #5261.20
        movq      %r12, %rsi                                    #5261.20
        movl      %r13d, %edx                                   #5261.20
        movl      %r14d, %ecx                                   #5261.20
        call      eval_4na_128@PLT                              #5261.20
                                # LOE eax
..B1.122:                       # Preds ..B1.49
        movq      -40(%rbp), %rbx                               #5261.20
..___tag_value_NucStrstrSearch.45:                              #
        movq      -48(%rbp), %r12                               #5261.20
..___tag_value_NucStrstrSearch.46:                              #
        movq      -16(%rbp), %r13                               #5261.20
..___tag_value_NucStrstrSearch.47:                              #
        movq      -24(%rbp), %r14                               #5261.20
..___tag_value_NucStrstrSearch.48:                              #
        movq      -32(%rbp), %r15                               #5261.20
..___tag_value_NucStrstrSearch.49:                              #
        movq      %rbp, %rsp                                    #5261.20
        popq      %rbp                                          #5261.20
..___tag_value_NucStrstrSearch.50:                              #
        ret                                                     #5261.20
..___tag_value_NucStrstrSearch.51:                              #
                                # LOE
..1.30_0.TAG.07.0.1:
..B1.52:                        # Preds ..B1.8
..LN85:
   .loc    1  5255
        movl      4(%rbx), %eax                                 #5255.24
        cmpl      %eax, %r14d                                   #5255.24
        jb        ..B1.107      # Prob 28%                      #5255.24
                                # LOE rbx r12 r15 eax r13d r14d
..B1.53:                        # Preds ..B1.52
..LN87:
   .loc    1  5256
        testq     %r15, %r15                                    #5256.9
        je        ..B1.55       # Prob 12%                      #5256.9
                                # LOE rbx r12 r15 eax r13d r14d
..B1.54:                        # Preds ..B1.53
..LN89:
        movl      %eax, (%r15)                                  #5256.18
                                # LOE rbx r12 r13d r14d
..B1.55:                        # Preds ..B1.53 ..B1.54
..LN91:
   .loc    1  5257
        movq      %rbx, %rdi                                    #5257.20
        movq      %r12, %rsi                                    #5257.20
        movl      %r13d, %edx                                   #5257.20
        movl      %r14d, %ecx                                   #5257.20
        call      eval_4na_32@PLT                               #5257.20
                                # LOE eax
..B1.56:                        # Preds ..B1.28 ..B1.30 ..B1.21 ..B1.55
        movq      -40(%rbp), %rbx                               #5257.20
..___tag_value_NucStrstrSearch.57:                              #
        movq      -48(%rbp), %r12                               #5257.20
..___tag_value_NucStrstrSearch.58:                              #
        movq      -16(%rbp), %r13                               #5257.20
..___tag_value_NucStrstrSearch.59:                              #
        movq      -24(%rbp), %r14                               #5257.20
..___tag_value_NucStrstrSearch.60:                              #
        movq      -32(%rbp), %r15                               #5257.20
..___tag_value_NucStrstrSearch.61:                              #
        movq      %rbp, %rsp                                    #5257.20
        popq      %rbp                                          #5257.20
..___tag_value_NucStrstrSearch.62:                              #
        ret                                                     #5257.20
..___tag_value_NucStrstrSearch.63:                              #
                                # LOE
..1.30_0.TAG.06.0.1:
..B1.59:                        # Preds ..B1.8
..LN93:
   .loc    1  5251
        movl      4(%rbx), %eax                                 #5251.24
        cmpl      %eax, %r14d                                   #5251.24
        jb        ..B1.107      # Prob 28%                      #5251.24
                                # LOE rbx r12 r15 eax r13d r14d
..B1.60:                        # Preds ..B1.59
..LN95:
   .loc    1  5252
        testq     %r15, %r15                                    #5252.9
        je        ..B1.62       # Prob 12%                      #5252.9
                                # LOE rbx r12 r15 eax r13d r14d
..B1.61:                        # Preds ..B1.60
..LN97:
        movl      %eax, (%r15)                                  #5252.18
                                # LOE rbx r12 r13d r14d
..B1.62:                        # Preds ..B1.60 ..B1.61
..LN99:
   .loc    1  5253
        movq      %rbx, %rdi                                    #5253.20
        movq      %r12, %rsi                                    #5253.20
        movl      %r13d, %edx                                   #5253.20
        movl      %r14d, %ecx                                   #5253.20
        call      eval_4na_16@PLT                               #5253.20
                                # LOE eax
..B1.63:                        # Preds ..B1.62
        movq      -40(%rbp), %rbx                               #5253.20
..___tag_value_NucStrstrSearch.69:                              #
        movq      -48(%rbp), %r12                               #5253.20
..___tag_value_NucStrstrSearch.70:                              #
        movq      -16(%rbp), %r13                               #5253.20
..___tag_value_NucStrstrSearch.71:                              #
        movq      -24(%rbp), %r14                               #5253.20
..___tag_value_NucStrstrSearch.72:                              #
        movq      -32(%rbp), %r15                               #5253.20
..___tag_value_NucStrstrSearch.73:                              #
        movq      %rbp, %rsp                                    #5253.20
        popq      %rbp                                          #5253.20
..___tag_value_NucStrstrSearch.74:                              #
        ret                                                     #5253.20
..___tag_value_NucStrstrSearch.75:                              #
                                # LOE
..1.30_0.TAG.05.0.1:
..B1.66:                        # Preds ..B1.8
..LN101:
   .loc    1  5247
        movl      4(%rbx), %eax                                 #5247.24
        cmpl      %eax, %r14d                                   #5247.24
        jb        ..B1.107      # Prob 28%                      #5247.24
                                # LOE rbx r12 r15 eax r13d r14d
..B1.67:                        # Preds ..B1.66
..LN103:
   .loc    1  5248
        testq     %r15, %r15                                    #5248.9
        je        ..B1.69       # Prob 12%                      #5248.9
                                # LOE rbx r12 r15 eax r13d r14d
..B1.68:                        # Preds ..B1.67
..LN105:
        movl      %eax, (%r15)                                  #5248.18
                                # LOE rbx r12 r13d r14d
..B1.69:                        # Preds ..B1.67 ..B1.68
..LN107:
   .loc    1  5249
        movq      %rbx, %rdi                                    #5249.20
        movq      %r12, %rsi                                    #5249.20
        movl      %r13d, %edx                                   #5249.20
        movl      %r14d, %ecx                                   #5249.20
        call      eval_2na_128@PLT                              #5249.20
                                # LOE eax
..B1.125:                       # Preds ..B1.69
        movq      -40(%rbp), %rbx                               #5249.20
..___tag_value_NucStrstrSearch.81:                              #
        movq      -48(%rbp), %r12                               #5249.20
..___tag_value_NucStrstrSearch.82:                              #
        movq      -16(%rbp), %r13                               #5249.20
..___tag_value_NucStrstrSearch.83:                              #
        movq      -24(%rbp), %r14                               #5249.20
..___tag_value_NucStrstrSearch.84:                              #
        movq      -32(%rbp), %r15                               #5249.20
..___tag_value_NucStrstrSearch.85:                              #
        movq      %rbp, %rsp                                    #5249.20
        popq      %rbp                                          #5249.20
..___tag_value_NucStrstrSearch.86:                              #
        ret                                                     #5249.20
..___tag_value_NucStrstrSearch.87:                              #
                                # LOE
..1.30_0.TAG.04.0.1:
..B1.72:                        # Preds ..B1.8
..LN109:
   .loc    1  5243
        movl      4(%rbx), %eax                                 #5243.24
        cmpl      %eax, %r14d                                   #5243.24
        jb        ..B1.107      # Prob 28%                      #5243.24
                                # LOE rbx r12 r15 eax r13d r14d
..B1.73:                        # Preds ..B1.72
..LN111:
   .loc    1  5244
        testq     %r15, %r15                                    #5244.9
        je        ..B1.75       # Prob 12%                      #5244.9
                                # LOE rbx r12 r15 eax r13d r14d
..B1.74:                        # Preds ..B1.73
..LN113:
        movl      %eax, (%r15)                                  #5244.18
                                # LOE rbx r12 r13d r14d
..B1.75:                        # Preds ..B1.73 ..B1.74
..LN115:
   .loc    1  5245
        movq      %rbx, %rdi                                    #5245.20
        movq      %r12, %rsi                                    #5245.20
        movl      %r13d, %edx                                   #5245.20
        movl      %r14d, %ecx                                   #5245.20
        call      eval_2na_32@PLT                               #5245.20
                                # LOE eax
..B1.76:                        # Preds ..B1.75
        movq      -40(%rbp), %rbx                               #5245.20
..___tag_value_NucStrstrSearch.93:                              #
        movq      -48(%rbp), %r12                               #5245.20
..___tag_value_NucStrstrSearch.94:                              #
        movq      -16(%rbp), %r13                               #5245.20
..___tag_value_NucStrstrSearch.95:                              #
        movq      -24(%rbp), %r14                               #5245.20
..___tag_value_NucStrstrSearch.96:                              #
        movq      -32(%rbp), %r15                               #5245.20
..___tag_value_NucStrstrSearch.97:                              #
        movq      %rbp, %rsp                                    #5245.20
        popq      %rbp                                          #5245.20
..___tag_value_NucStrstrSearch.98:                              #
        ret                                                     #5245.20
..___tag_value_NucStrstrSearch.99:                              #
                                # LOE
..1.30_0.TAG.03.0.1:
..B1.79:                        # Preds ..B1.8
..LN117:
   .loc    1  5239
        movl      4(%rbx), %eax                                 #5239.24
        cmpl      %eax, %r14d                                   #5239.24
        jb        ..B1.107      # Prob 28%                      #5239.24
                                # LOE rbx r12 r15 eax r13d r14d
..B1.80:                        # Preds ..B1.79
..LN119:
   .loc    1  5240
        testq     %r15, %r15                                    #5240.9
        je        ..B1.82       # Prob 12%                      #5240.9
                                # LOE rbx r12 r15 eax r13d r14d
..B1.81:                        # Preds ..B1.80
..LN121:
        movl      %eax, (%r15)                                  #5240.18
                                # LOE rbx r12 r13d r14d
..B1.82:                        # Preds ..B1.80 ..B1.81
..LN123:
   .loc    1  5241
        movq      %rbx, %rdi                                    #5241.20
        movq      %r12, %rsi                                    #5241.20
        movl      %r13d, %edx                                   #5241.20
        movl      %r14d, %ecx                                   #5241.20
        call      eval_2na_16@PLT                               #5241.20
                                # LOE eax
..B1.83:                        # Preds ..B1.82
        movq      -40(%rbp), %rbx                               #5241.20
..___tag_value_NucStrstrSearch.105:                             #
        movq      -48(%rbp), %r12                               #5241.20
..___tag_value_NucStrstrSearch.106:                             #
        movq      -16(%rbp), %r13                               #5241.20
..___tag_value_NucStrstrSearch.107:                             #
        movq      -24(%rbp), %r14                               #5241.20
..___tag_value_NucStrstrSearch.108:                             #
        movq      -32(%rbp), %r15                               #5241.20
..___tag_value_NucStrstrSearch.109:                             #
        movq      %rbp, %rsp                                    #5241.20
        popq      %rbp                                          #5241.20
..___tag_value_NucStrstrSearch.110:                             #
        ret                                                     #5241.20
..___tag_value_NucStrstrSearch.111:                             #
                                # LOE
..1.30_0.TAG.02.0.1:
..B1.86:                        # Preds ..B1.8
..LN125:
   .loc    1  5235
        movl      4(%rbx), %eax                                 #5235.24
        cmpl      %eax, %r14d                                   #5235.24
        jb        ..B1.107      # Prob 28%                      #5235.24
                                # LOE rbx r12 r15 eax r13d r14d
..B1.87:                        # Preds ..B1.86
..LN127:
   .loc    1  5236
        testq     %r15, %r15                                    #5236.9
        je        ..B1.89       # Prob 12%                      #5236.9
                                # LOE rbx r12 r15 eax r13d r14d
..B1.88:                        # Preds ..B1.87
..LN129:
        movl      %eax, (%r15)                                  #5236.18
                                # LOE rbx r12 r13d r14d
..B1.89:                        # Preds ..B1.87 ..B1.88
..LN131:
   .loc    1  5237
        movq      %rbx, %rdi                                    #5237.20
        movq      %r12, %rsi                                    #5237.20
        movl      %r13d, %edx                                   #5237.20
        movl      %r14d, %ecx                                   #5237.20
        call      eval_2na_8@PLT                                #5237.20
                                # LOE eax
..B1.90:                        # Preds ..B1.89
        movq      -40(%rbp), %rbx                               #5237.20
..___tag_value_NucStrstrSearch.117:                             #
        movq      -48(%rbp), %r12                               #5237.20
..___tag_value_NucStrstrSearch.118:                             #
        movq      -16(%rbp), %r13                               #5237.20
..___tag_value_NucStrstrSearch.119:                             #
        movq      -24(%rbp), %r14                               #5237.20
..___tag_value_NucStrstrSearch.120:                             #
        movq      -32(%rbp), %r15                               #5237.20
..___tag_value_NucStrstrSearch.121:                             #
        movq      %rbp, %rsp                                    #5237.20
        popq      %rbp                                          #5237.20
..___tag_value_NucStrstrSearch.122:                             #
        ret                                                     #5237.20
..___tag_value_NucStrstrSearch.123:                             #
                                # LOE
..1.30_0.TAG.01.0.1:
..B1.93:                        # Preds ..B1.8
..LN133:
   .loc    1  5230
        movl      4(%rbx), %eax                                 #5230.24
        cmpl      %eax, %r14d                                   #5230.24
        jb        ..B1.107      # Prob 28%                      #5230.24
                                # LOE rbx r12 r15 eax r13d r14d
..B1.94:                        # Preds ..B1.93
..LN135:
   .loc    1  5231
        testq     %r15, %r15                                    #5231.9
        je        ..B1.96       # Prob 12%                      #5231.9
                                # LOE rbx r12 r15 eax r13d r14d
..B1.95:                        # Preds ..B1.94
..LN137:
        movl      %eax, (%r15)                                  #5231.18
                                # LOE rbx r12 r13d r14d
..B1.96:                        # Preds ..B1.94 ..B1.95
..LN139:
   .loc    1  5232
        movq      %rbx, %rdi                                    #5232.20
        movq      %r12, %rsi                                    #5232.20
        movl      %r13d, %edx                                   #5232.20
        movl      %r14d, %ecx                                   #5232.20
        call      eval_4na_64@PLT                               #5232.20
                                # LOE eax
..B1.97:                        # Preds ..B1.96
        movq      -40(%rbp), %rbx                               #5232.20
..___tag_value_NucStrstrSearch.129:                             #
        movq      -48(%rbp), %r12                               #5232.20
..___tag_value_NucStrstrSearch.130:                             #
        movq      -16(%rbp), %r13                               #5232.20
..___tag_value_NucStrstrSearch.131:                             #
        movq      -24(%rbp), %r14                               #5232.20
..___tag_value_NucStrstrSearch.132:                             #
        movq      -32(%rbp), %r15                               #5232.20
..___tag_value_NucStrstrSearch.133:                             #
        movq      %rbp, %rsp                                    #5232.20
        popq      %rbp                                          #5232.20
..___tag_value_NucStrstrSearch.134:                             #
        ret                                                     #5232.20
..___tag_value_NucStrstrSearch.135:                             #
                                # LOE
..1.30_0.TAG.00.0.1:
..B1.100:                       # Preds ..B1.8
..LN141:
   .loc    1  5226
        movl      4(%rbx), %eax                                 #5226.24
        cmpl      %eax, %r14d                                   #5226.24
        jb        ..B1.107      # Prob 28%                      #5226.24
                                # LOE rbx r12 r15 eax r13d r14d
..B1.101:                       # Preds ..B1.100
..LN143:
   .loc    1  5227
        testq     %r15, %r15                                    #5227.9
        je        ..B1.103      # Prob 12%                      #5227.9
                                # LOE rbx r12 r15 eax r13d r14d
..B1.102:                       # Preds ..B1.101
..LN145:
        movl      %eax, (%r15)                                  #5227.18
                                # LOE rbx r12 r13d r14d
..B1.103:                       # Preds ..B1.101 ..B1.102
..LN147:
   .loc    1  5228
        movq      %rbx, %rdi                                    #5228.20
        movq      %r12, %rsi                                    #5228.20
        movl      %r13d, %edx                                   #5228.20
        movl      %r14d, %ecx                                   #5228.20
        call      eval_2na_64@PLT                               #5228.20
                                # LOE eax
..B1.104:                       # Preds ..B1.103
        movq      -40(%rbp), %rbx                               #5228.20
..___tag_value_NucStrstrSearch.141:                             #
        movq      -48(%rbp), %r12                               #5228.20
..___tag_value_NucStrstrSearch.142:                             #
        movq      -16(%rbp), %r13                               #5228.20
..___tag_value_NucStrstrSearch.143:                             #
        movq      -24(%rbp), %r14                               #5228.20
..___tag_value_NucStrstrSearch.144:                             #
        movq      -32(%rbp), %r15                               #5228.20
..___tag_value_NucStrstrSearch.145:                             #
        movq      %rbp, %rsp                                    #5228.20
        popq      %rbp                                          #5228.20
..___tag_value_NucStrstrSearch.146:                             #
        ret                                                     #5228.20
..___tag_value_NucStrstrSearch.147:                             #
                                # LOE
..B1.107:                       # Preds ..B1.13 ..B1.16 ..B1.17 ..B1.14 ..B1.7
                                #       ..B1.31 ..B1.15 ..B1.4 ..B1.100 ..B1.93
                                #       ..B1.86 ..B1.79 ..B1.72 ..B1.66 ..B1.59
                                #       ..B1.52 ..B1.46 ..B1.40 ..B1.34 ..B1.22
                                #       ..B1.2 ..B1.117
..LN149:
   .loc    1  5311
        movq      -40(%rbp), %rbx                               #5311.12
..___tag_value_NucStrstrSearch.153:                             #
        movq      -48(%rbp), %r12                               #5311.12
..___tag_value_NucStrstrSearch.154:                             #
        movq      -16(%rbp), %r13                               #5311.12
..___tag_value_NucStrstrSearch.155:                             #
        movq      -24(%rbp), %r14                               #5311.12
..___tag_value_NucStrstrSearch.156:                             #
        xorl      %eax, %eax                                    #5311.12
        movq      -32(%rbp), %r15                               #5311.12
..___tag_value_NucStrstrSearch.157:                             #
        movq      %rbp, %rsp                                    #5311.12
        popq      %rbp                                          #5311.12
..___tag_value_NucStrstrSearch.158:                             #
        ret                                                     #5311.12
        .align    16,0x90
..___tag_value_NucStrstrSearch.159:                             #
                                # LOE
# mark_end;
	.type	NucStrstrSearch,@function
	.size	NucStrstrSearch,.-NucStrstrSearch
.LNNucStrstrSearch:
	.section .data1, "wa"
	.align 32
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
	.text
# -- Begin  eval_4na_pos
# mark_begin;
       .align    16,0x90
eval_4na_pos:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B2.1:                         # Preds ..B2.0
..___tag_value_eval_4na_pos.160:                                #4946.1
..LN151:
   .loc    1  4946
        pushq     %rbp                                          #4946.1
        movq      %rsp, %rbp                                    #4946.1
..___tag_value_eval_4na_pos.161:                                #
        subq      $176, %rsp                                    #4946.1
        movq      %r15, -48(%rbp)                               #4946.1
..___tag_value_eval_4na_pos.163:                                #
        movq      %r14, -40(%rbp)                               #4946.1
..___tag_value_eval_4na_pos.164:                                #
        movq      %r13, -32(%rbp)                               #4946.1
..___tag_value_eval_4na_pos.165:                                #
        movq      %r12, -24(%rbp)                               #4946.1
..___tag_value_eval_4na_pos.166:                                #
        movq      %rbx, -16(%rbp)                               #4946.1
..___tag_value_eval_4na_pos.167:                                #
        movl      %edx, %r14d                                   #4946.1
        movq      %rsi, %r15                                    #4946.1
        movq      %rdi, %rbx                                    #4946.1
        lea       _gprof_pack1(%rip), %rdx                      #4946.1
        movl      %ecx, -168(%rbp)                              #4946.1
        call      mcount@PLT                                    #4946.1
                                # LOE rbx r15 r14d
..B2.50:                        # Preds ..B2.1
        movl      -168(%rbp), %ecx                              #
        movl      %r14d, -8(%rbp)                               #4946.1
..LN153:
   .loc    1  4985
        lea       (%r14,%rcx), %r12d                            #4985.5
..LN155:
   .loc    1  4993
        movl      %r14d, %r13d                                  #4993.49
        shrl      $2, %r13d                                     #4993.49
..LN157:
   .loc    1  4996
        subl      4(%rbx), %r12d                                #4996.12
..LN159:
   .loc    1  4993
        lea       (%r15,%r13), %rdi                             #4993.30
..LN161:
   .loc    1  4999
        lea       3(%r14,%rcx), %esi                            #4999.50
..LN163:
        shrl      $2, %esi                                      #4999.57
..LN165:
        addq      %r15, %rsi                                    #4999.30
        movq      %rsi, -176(%rbp)                              #4999.30
..LN167:
   .loc    1  5002
        call      prime_buffer_4na@PLT                          #5002.14
                                # LOE rbx r13 r14 r15 r12d r14d r14b xmm0
..B2.51:                        # Preds ..B2.50
..LN169:
   .loc    1  5009
        movdqa    16(%rbx), %xmm8                               #5009.5
        movdqa    32(%rbx), %xmm7                               #5009.5
        movdqa    48(%rbx), %xmm6                               #5009.5
        movdqa    64(%rbx), %xmm5                               #5009.5
        movdqa    80(%rbx), %xmm4                               #5009.5
        movdqa    96(%rbx), %xmm3                               #5009.5
        movdqa    112(%rbx), %xmm2                              #5009.5
        movdqa    128(%rbx), %xmm1                              #5009.5
..LN171:
   .loc    1  5003
        lea       8(%r15,%r13), %r13                            #5003.5
        movq      %r13, %rdx                                    #5003.5
..LN173:
   .loc    1  5012
        xorl      %edi, %edi                                    #5012.15
..LN175:
        xorl      %r8d, %r8d                                    #5012.10
..LN177:
        xorl      %esi, %esi                                    #5012.5
..LN179:
   .loc    1  5014
        movl      %r12d, %eax                                   #5014.20
        subl      %r14d, %eax                                   #5014.20
..LN181:
        addl      $7, %eax                                      #5014.33
..LN183:
        shrl      $2, %eax                                      #5014.40
..LN185:
   .loc    1  5023
        movl      %r14d, %ecx                                   #5023.20
        andl      $3, %ecx                                      #5023.20
        je        ..B2.45       # Prob 20%                      #5023.20
                                # LOE rdx r13 r14 eax ecx esi edi r8d r12d r14d r14b xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.2:                         # Preds ..B2.51
        cmpl      $1, %ecx                                      #5023.20
        je        ..B2.8        # Prob 25%                      #5023.20
                                # LOE rdx r13 r14 eax ecx esi edi r8d r12d r14d r14b xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.3:                         # Preds ..B2.2
        cmpl      $2, %ecx                                      #5023.20
        je        ..B2.7        # Prob 33%                      #5023.20
                                # LOE rdx r13 r14 eax ecx esi edi r8d r12d r14d r14b xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.4:                         # Preds ..B2.3
        cmpl      $3, %ecx                                      #5023.20
        je        ..B2.6        # Prob 50%                      #5023.20
                                # LOE rdx r13 r14 eax esi edi r8d r12d r14d r14b xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.5:                         # Preds ..B2.4
        movq      -176(%rbp), %rbx                              #
        jmp       ..B2.16       # Prob 100%                     #
                                # LOE rdx rbx r13 r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.6:                         # Preds ..B2.4
        movq      -176(%rbp), %rbx                              #
        jmp       ..B2.11       # Prob 100%                     #
                                # LOE rdx rbx r13 eax esi edi r8d r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.7:                         # Preds ..B2.3
        movq      -176(%rbp), %rbx                              #
        jmp       ..B2.10       # Prob 100%                     #
                                # LOE rdx rbx r13 eax esi r8d r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.8:                         # Preds ..B2.2
        movq      -176(%rbp), %rbx                              #
                                # LOE rdx rbx r13 eax esi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.9:                         # Preds ..B2.8 ..B2.20
..LN187:
   .loc    1  5047
        movdqa    %xmm0, %xmm10                                 #5047.22
..LN189:
   .loc    1  5048
        movdqa    %xmm0, %xmm9                                  #5048.22
..LN191:
   .loc    1  5047
        pand      %xmm6, %xmm10                                 #5047.22
..LN193:
   .loc    1  5048
        pand      %xmm5, %xmm9                                  #5048.22
..LN195:
   .loc    1  5049
        pcmpeqd   %xmm9, %xmm10                                 #5049.22
..LN197:
   .loc    1  5050
        pmovmskb  %xmm10, %r8d                                  #5050.22
..LN199:
   .loc    1  5051
        incl      %r8d                                          #5051.17
        shrl      $16, %r8d                                     #5051.17
        negl      %r8d                                          #5051.17
                                # LOE rdx rbx r13 eax esi r8d r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.10:                        # Preds ..B2.7 ..B2.9
..LN201:
   .loc    1  5054
        movdqa    %xmm0, %xmm10                                 #5054.22
..LN203:
   .loc    1  5055
        movdqa    %xmm0, %xmm9                                  #5055.22
..LN205:
   .loc    1  5054
        pand      %xmm4, %xmm10                                 #5054.22
..LN207:
   .loc    1  5055
        pand      %xmm3, %xmm9                                  #5055.22
..LN209:
   .loc    1  5056
        pcmpeqd   %xmm9, %xmm10                                 #5056.22
..LN211:
   .loc    1  5057
        pmovmskb  %xmm10, %edi                                  #5057.22
..LN213:
   .loc    1  5058
        incl      %edi                                          #5058.17
        shrl      $16, %edi                                     #5058.17
        negl      %edi                                          #5058.17
                                # LOE rdx rbx r13 eax esi edi r8d r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.11:                        # Preds ..B2.6 ..B2.10
..LN215:
   .loc    1  5061
        movdqa    %xmm0, %xmm10                                 #5061.22
..LN217:
   .loc    1  5062
        movdqa    %xmm0, %xmm9                                  #5062.22
..LN219:
   .loc    1  5069
        andl      $-4, %r14d                                    #5069.17
..LN221:
   .loc    1  5061
        pand      %xmm2, %xmm10                                 #5061.22
..LN223:
   .loc    1  5062
        pand      %xmm1, %xmm9                                  #5062.22
..LN225:
   .loc    1  5063
        pcmpeqd   %xmm9, %xmm10                                 #5063.22
..LN227:
   .loc    1  5064
        pmovmskb  %xmm10, %ecx                                  #5064.22
..LN229:
   .loc    1  5072
        movl      %esi, %r9d                                    #5072.29
        orl       %r8d, %r9d                                    #5072.29
..LN231:
   .loc    1  5065
        incl      %ecx                                          #5065.17
        shrl      $16, %ecx                                     #5065.17
        negl      %ecx                                          #5065.17
..LN233:
   .loc    1  5072
        orl       %edi, %r9d                                    #5072.34
..LN235:
        orl       %ecx, %r9d                                    #5072.39
..LN237:
        jne       ..B2.21       # Prob 20%                      #5072.47
                                # LOE rdx rbx r13 eax ecx esi edi r8d r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.12:                        # Preds ..B2.11
..LN239:
   .loc    1  5135
        addl      $4, %r14d                                     #5135.17
..LN241:
   .loc    1  5138
        cmpl      %r12d, %r14d                                  #5138.28
        ja        ..B2.44       # Prob 20%                      #5138.28
                                # LOE rdx rbx r13 eax r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.13:                        # Preds ..B2.12
..LN243:
   .loc    1  5142
        decl      %eax                                          #5142.25
..LN245:
        jne       ..B2.17       # Prob 50%                      #5142.39
                                # LOE rdx rbx r13 eax r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.14:                        # Preds ..B2.13
..LN247:
   .loc    1  5159
        cmpq      %rbx, %r13                                    #5159.25
        jae       ..B2.44       # Prob 4%                       #5159.25
                                # LOE rbx r13 r12d r14d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.15:                        # Preds ..B2.14
..LN249:
   .loc    1  5169
        movdqa    %xmm1, -176(%rbp)                             #5169.22
        movdqa    %xmm2, -160(%rbp)                             #5169.22
        movdqa    %xmm3, -144(%rbp)                             #5169.22
        movdqa    %xmm4, -128(%rbp)                             #5169.22
        movdqa    %xmm5, -112(%rbp)                             #5169.22
        movdqa    %xmm6, -96(%rbp)                              #5169.22
        movdqa    %xmm7, -80(%rbp)                              #5169.22
        movdqa    %xmm8, -64(%rbp)                              #5169.22
        movq      %r13, %rdi                                    #5169.22
        movq      %rbx, %rsi                                    #5169.22
        call      prime_buffer_4na@PLT                          #5169.22
                                # LOE rbx r13 r12d r14d xmm0
..B2.52:                        # Preds ..B2.15
        movdqa    -64(%rbp), %xmm8                              #
        movdqa    -80(%rbp), %xmm7                              #
        movdqa    -96(%rbp), %xmm6                              #
        movdqa    -112(%rbp), %xmm5                             #
..LN251:
   .loc    1  5184
        addq      $8, %r13                                      #5184.13
        movq      %r13, %rdx                                    #5184.13
        movdqa    -128(%rbp), %xmm4                             #
        movdqa    -144(%rbp), %xmm3                             #
        movdqa    -160(%rbp), %xmm2                             #
        movdqa    -176(%rbp), %xmm1                             #
                                # LOE rdx rbx r13 r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.16:                        # Preds ..B2.5 ..B2.52
..LN253:
   .loc    1  5030
        movl      $8, %eax                                      #5030.13
        jmp       ..B2.20       # Prob 100%                     #5030.13
                                # LOE rdx rbx r13 eax r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.17:                        # Preds ..B2.13
..LN255:
   .loc    1  5147
        psrldq    $2, %xmm0                                     #5147.26
..LN257:
   .loc    1  5150
        cmpq      %rbx, %rdx                                    #5150.26
        jae       ..B2.19       # Prob 19%                      #5150.26
                                # LOE rdx rbx r13 eax r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.18:                        # Preds ..B2.17
..LN259:
   .loc    1  5151
        movzbl    (%rdx), %esi                                  #5151.72
..LN261:
        lea       expand_2na.0(%rip), %rcx                      #5151.57
        movzwl    (%rcx,%rsi,2), %edi                           #5151.57
..LN263:
        pinsrw    $7, %edi, %xmm0                               #5151.30
                                # LOE rdx rbx r13 eax r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.19:                        # Preds ..B2.18 ..B2.17
..LN265:
   .loc    1  5154
        incq      %rdx                                          #5154.20
                                # LOE rdx rbx r13 eax r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.20:                        # Preds ..B2.16 ..B2.19 ..B2.45
..LN267:
   .loc    1  5040
        movdqa    %xmm0, %xmm10                                 #5040.22
..LN269:
   .loc    1  5041
        movdqa    %xmm0, %xmm9                                  #5041.22
..LN271:
   .loc    1  5040
        pand      %xmm8, %xmm10                                 #5040.22
..LN273:
   .loc    1  5041
        pand      %xmm7, %xmm9                                  #5041.22
..LN275:
   .loc    1  5042
        pcmpeqd   %xmm9, %xmm10                                 #5042.22
..LN277:
   .loc    1  5043
        pmovmskb  %xmm10, %esi                                  #5043.22
..LN279:
   .loc    1  5044
        incl      %esi                                          #5044.17
        shrl      $16, %esi                                     #5044.17
        negl      %esi                                          #5044.17
        jmp       ..B2.9        # Prob 100%                     #5044.17
                                # LOE rdx rbx r13 eax esi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.21:                        # Preds ..B2.11                 # Infreq
..LN281:
   .loc    1  5075
        subl      %r14d, %r12d                                  #5075.30
..LN283:
        je        ..B2.42       # Prob 25%                      #5075.37
                                # LOE ecx esi edi r8d r12d r14d
..B2.22:                        # Preds ..B2.21                 # Infreq
        cmpl      $1, %r12d                                     #5075.37
        jne       ..B2.27       # Prob 67%                      #5075.37
                                # LOE ecx esi edi r8d r12d r14d
..B2.23:                        # Preds ..B2.22                 # Infreq
..LN285:
   .loc    1  5081
        testl     %esi, %esi                                    #5081.36
        jne       ..B2.43       # Prob 28%                      #5081.36
                                # LOE r8d r14d
..B2.24:                        # Preds ..B2.23                 # Infreq
..LN287:
   .loc    1  5082
        testl     %r8d, %r8d                                    #5082.36
        je        ..B2.44       # Prob 50%                      #5082.36
                                # LOE r14d
..B2.25:                        # Preds ..B2.35 ..B2.29 ..B2.24 # Infreq
..LN289:
        subl      -8(%rbp), %r14d                               #5082.47
..LN291:
        movq      -16(%rbp), %rbx                               #5082.61
..___tag_value_eval_4na_pos.168:                                #
        addl      $2, %r14d                                     #5082.61
        movl      %r14d, %eax                                   #5082.61
        movq      -24(%rbp), %r12                               #5082.61
..___tag_value_eval_4na_pos.169:                                #
        movq      -32(%rbp), %r13                               #5082.61
..___tag_value_eval_4na_pos.170:                                #
        movq      -40(%rbp), %r14                               #5082.61
..___tag_value_eval_4na_pos.171:                                #
        movq      -48(%rbp), %r15                               #5082.61
..___tag_value_eval_4na_pos.172:                                #
        movq      %rbp, %rsp                                    #5082.61
        popq      %rbp                                          #5082.61
..___tag_value_eval_4na_pos.173:                                #
        ret                                                     #5082.61
..___tag_value_eval_4na_pos.174:                                #
                                # LOE
..B2.27:                        # Preds ..B2.22                 # Infreq
..LN293:
   .loc    1  5075
        cmpl      $2, %r12d                                     #5075.37
        jne       ..B2.34       # Prob 50%                      #5075.37
                                # LOE ecx esi edi r8d r14d
..B2.28:                        # Preds ..B2.27                 # Infreq
..LN295:
   .loc    1  5085
        testl     %esi, %esi                                    #5085.36
        jne       ..B2.43       # Prob 28%                      #5085.36
                                # LOE edi r8d r14d
..B2.29:                        # Preds ..B2.28                 # Infreq
..LN297:
   .loc    1  5086
        testl     %r8d, %r8d                                    #5086.36
        jne       ..B2.25       # Prob 28%                      #5086.36
                                # LOE edi r14d
..B2.30:                        # Preds ..B2.29                 # Infreq
..LN299:
   .loc    1  5087
        testl     %edi, %edi                                    #5087.36
        je        ..B2.44       # Prob 50%                      #5087.36
                                # LOE r14d
..B2.31:                        # Preds ..B2.36 ..B2.30         # Infreq
..LN301:
        subl      -8(%rbp), %r14d                               #5087.47
..LN303:
        movq      -16(%rbp), %rbx                               #5087.61
..___tag_value_eval_4na_pos.180:                                #
        addl      $3, %r14d                                     #5087.61
        movl      %r14d, %eax                                   #5087.61
        movq      -24(%rbp), %r12                               #5087.61
..___tag_value_eval_4na_pos.181:                                #
        movq      -32(%rbp), %r13                               #5087.61
..___tag_value_eval_4na_pos.182:                                #
        movq      -40(%rbp), %r14                               #5087.61
..___tag_value_eval_4na_pos.183:                                #
        movq      -48(%rbp), %r15                               #5087.61
..___tag_value_eval_4na_pos.184:                                #
        movq      %rbp, %rsp                                    #5087.61
        popq      %rbp                                          #5087.61
..___tag_value_eval_4na_pos.185:                                #
        ret                                                     #5087.61
..___tag_value_eval_4na_pos.186:                                #
                                # LOE
..B2.34:                        # Preds ..B2.27                 # Infreq
..LN305:
   .loc    1  5090
        testl     %esi, %esi                                    #5090.36
        jne       ..B2.43       # Prob 28%                      #5090.36
                                # LOE ecx edi r8d r14d
..B2.35:                        # Preds ..B2.34                 # Infreq
..LN307:
   .loc    1  5091
        testl     %r8d, %r8d                                    #5091.36
        jne       ..B2.25       # Prob 28%                      #5091.36
                                # LOE ecx edi r14d
..B2.36:                        # Preds ..B2.35                 # Infreq
..LN309:
   .loc    1  5092
        testl     %edi, %edi                                    #5092.36
        jne       ..B2.31       # Prob 28%                      #5092.36
                                # LOE ecx r14d
..B2.37:                        # Preds ..B2.36                 # Infreq
..LN311:
   .loc    1  5093
        testl     %ecx, %ecx                                    #5093.36
        je        ..B2.44       # Prob 50%                      #5093.36
                                # LOE r14d
..B2.38:                        # Preds ..B2.37                 # Infreq
..LN313:
        subl      -8(%rbp), %r14d                               #5093.47
..LN315:
        movq      -16(%rbp), %rbx                               #5093.61
..___tag_value_eval_4na_pos.192:                                #
        addl      $4, %r14d                                     #5093.61
        movl      %r14d, %eax                                   #5093.61
        movq      -24(%rbp), %r12                               #5093.61
..___tag_value_eval_4na_pos.193:                                #
        movq      -32(%rbp), %r13                               #5093.61
..___tag_value_eval_4na_pos.194:                                #
        movq      -40(%rbp), %r14                               #5093.61
..___tag_value_eval_4na_pos.195:                                #
        movq      -48(%rbp), %r15                               #5093.61
..___tag_value_eval_4na_pos.196:                                #
        movq      %rbp, %rsp                                    #5093.61
        popq      %rbp                                          #5093.61
..___tag_value_eval_4na_pos.197:                                #
        ret                                                     #5093.61
..___tag_value_eval_4na_pos.198:                                #
                                # LOE
..B2.42:                        # Preds ..B2.21                 # Infreq
..LN317:
   .loc    1  5078
        testl     %esi, %esi                                    #5078.36
        je        ..B2.44       # Prob 50%                      #5078.36
                                # LOE r14d
..B2.43:                        # Preds ..B2.34 ..B2.28 ..B2.23 ..B2.42 # Infreq
..LN319:
        subl      -8(%rbp), %r14d                               #5078.47
..LN321:
        movq      -16(%rbp), %rbx                               #5078.61
..___tag_value_eval_4na_pos.204:                                #
        incl      %r14d                                         #5078.61
        movl      %r14d, %eax                                   #5078.61
        movq      -24(%rbp), %r12                               #5078.61
..___tag_value_eval_4na_pos.205:                                #
        movq      -32(%rbp), %r13                               #5078.61
..___tag_value_eval_4na_pos.206:                                #
        movq      -40(%rbp), %r14                               #5078.61
..___tag_value_eval_4na_pos.207:                                #
        movq      -48(%rbp), %r15                               #5078.61
..___tag_value_eval_4na_pos.208:                                #
        movq      %rbp, %rsp                                    #5078.61
        popq      %rbp                                          #5078.61
..___tag_value_eval_4na_pos.209:                                #
        ret                                                     #5078.61
..___tag_value_eval_4na_pos.210:                                #
                                # LOE
..B2.44:                        # Preds ..B2.14 ..B2.12 ..B2.37 ..B2.30 ..B2.24
                                #       ..B2.42                 # Infreq
..LN323:
   .loc    1  5095
        movq      -16(%rbp), %rbx                               #5095.28
..___tag_value_eval_4na_pos.216:                                #
        movq      -24(%rbp), %r12                               #5095.28
..___tag_value_eval_4na_pos.217:                                #
        movq      -32(%rbp), %r13                               #5095.28
..___tag_value_eval_4na_pos.218:                                #
        movq      -40(%rbp), %r14                               #5095.28
..___tag_value_eval_4na_pos.219:                                #
        xorl      %eax, %eax                                    #5095.28
        movq      -48(%rbp), %r15                               #5095.28
..___tag_value_eval_4na_pos.220:                                #
        movq      %rbp, %rsp                                    #5095.28
        popq      %rbp                                          #5095.28
..___tag_value_eval_4na_pos.221:                                #
        ret                                                     #5095.28
..___tag_value_eval_4na_pos.222:                                #
                                # LOE
..B2.45:                        # Preds ..B2.51                 # Infreq
        movq      -176(%rbp), %rbx                              #
        jmp       ..B2.20       # Prob 100%                     #
        .align    16,0x90
..___tag_value_eval_4na_pos.228:                                #
                                # LOE rdx rbx r13 eax r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
# mark_end;
	.type	eval_4na_pos,@function
	.size	eval_4na_pos,.-eval_4na_pos
.LNeval_4na_pos:
	.data
# -- End  eval_4na_pos
	.text
# -- Begin  prime_buffer_4na
# mark_begin;
       .align    16,0x90
prime_buffer_4na:
# parameter 1(src): %rdi
# parameter 2(ignore): %rsi
..B3.1:                         # Preds ..B3.0
..___tag_value_prime_buffer_4na.229:                            #3990.1
..LN325:
   .loc    1  3990
        pushq     %rbp                                          #3990.1
        movq      %rsp, %rbp                                    #3990.1
..___tag_value_prime_buffer_4na.230:                            #
        subq      $32, %rsp                                     #3990.1
        movq      %r14, -16(%rbp)                               #3990.1
..___tag_value_prime_buffer_4na.232:                            #
        movq      %rdi, %r14                                    #3990.1
        lea       _gprof_pack2(%rip), %rdx                      #3990.1
        call      mcount@PLT                                    #3990.1
                                # LOE rbx r12 r13 r14 r15
..B3.4:                         # Preds ..B3.1
..LN327:
   .loc    1  3996
        movzbl    (%r14), %edx                                  #3996.34
..LN329:
   .loc    1  3997
        movzbl    1(%r14), %esi                                 #3997.34
..LN331:
   .loc    1  3998
        movzbl    2(%r14), %r8d                                 #3998.34
..LN333:
   .loc    1  3999
        movzbl    3(%r14), %r10d                                #3999.34
..LN335:
   .loc    1  3996
        lea       expand_2na.0(%rip), %rax                      #3996.21
        movzwl    (%rax,%rdx,2), %ecx                           #3996.21
..LN337:
   .loc    1  3997
        movzwl    (%rax,%rsi,2), %edi                           #3997.21
..LN339:
   .loc    1  3998
        movzwl    (%rax,%r8,2), %r9d                            #3998.21
..LN341:
   .loc    1  3999
        movzwl    (%rax,%r10,2), %r11d                          #3999.21
..LN343:
   .loc    1  4000
        movzbl    4(%r14), %edx                                 #4000.34
..LN345:
   .loc    1  4001
        movzbl    5(%r14), %esi                                 #4001.34
..LN347:
   .loc    1  4002
        movzbl    6(%r14), %r8d                                 #4002.34
..LN349:
   .loc    1  4003
        movzbl    7(%r14), %r14d                                #4003.34
..LN351:
   .loc    1  3996
        movw      %cx, -32(%rbp)                                #3996.5
..LN353:
   .loc    1  4000
        movzwl    (%rax,%rdx,2), %ecx                           #4000.21
..LN355:
   .loc    1  3997
        movw      %di, -30(%rbp)                                #3997.5
..LN357:
   .loc    1  4001
        movzwl    (%rax,%rsi,2), %edi                           #4001.21
..LN359:
   .loc    1  3998
        movw      %r9w, -28(%rbp)                               #3998.5
..LN361:
   .loc    1  4002
        movzwl    (%rax,%r8,2), %r9d                            #4002.21
..LN363:
   .loc    1  4003
        movzwl    (%rax,%r14,2), %eax                           #4003.21
..LN365:
   .loc    1  4006
        movq      -16(%rbp), %r14                               #4006.12
..___tag_value_prime_buffer_4na.233:                            #
..LN367:
   .loc    1  3999
        movw      %r11w, -26(%rbp)                              #3999.5
..LN369:
   .loc    1  4000
        movw      %cx, -24(%rbp)                                #4000.5
..LN371:
   .loc    1  4001
        movw      %di, -22(%rbp)                                #4001.5
..LN373:
   .loc    1  4002
        movw      %r9w, -20(%rbp)                               #4002.5
..LN375:
   .loc    1  4003
        movw      %ax, -18(%rbp)                                #4003.5
..LN377:
   .loc    1  4005
        movdqu    -32(%rbp), %xmm0                              #4005.51
..LN379:
   .loc    1  4006
        movq      %rbp, %rsp                                    #4006.12
        popq      %rbp                                          #4006.12
..___tag_value_prime_buffer_4na.234:                            #
        ret                                                     #4006.12
        .align    16,0x90
..___tag_value_prime_buffer_4na.235:                            #
                                # LOE
# mark_end;
	.type	prime_buffer_4na,@function
	.size	prime_buffer_4na,.-prime_buffer_4na
.LNprime_buffer_4na:
	.data
# -- End  prime_buffer_4na
	.text
# -- Begin  eval_2na_pos
# mark_begin;
       .align    16,0x90
eval_2na_pos:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B4.1:                         # Preds ..B4.0
..___tag_value_eval_2na_pos.236:                                #3662.1
..LN381:
   .loc    1  3662
        pushq     %rbp                                          #3662.1
        movq      %rsp, %rbp                                    #3662.1
..___tag_value_eval_2na_pos.237:                                #
        subq      $176, %rsp                                    #3662.1
        movq      %r15, -48(%rbp)                               #3662.1
..___tag_value_eval_2na_pos.239:                                #
        movq      %r14, -40(%rbp)                               #3662.1
..___tag_value_eval_2na_pos.240:                                #
        movq      %r13, -32(%rbp)                               #3662.1
..___tag_value_eval_2na_pos.241:                                #
        movq      %r12, -24(%rbp)                               #3662.1
..___tag_value_eval_2na_pos.242:                                #
        movq      %rbx, -16(%rbp)                               #3662.1
..___tag_value_eval_2na_pos.243:                                #
        movl      %ecx, %r13d                                   #3662.1
        movl      %edx, %r12d                                   #3662.1
        movq      %rsi, %rbx                                    #3662.1
        movq      %rdi, %r15                                    #3662.1
        lea       _gprof_pack3(%rip), %rdx                      #3662.1
        call      mcount@PLT                                    #3662.1
                                # LOE rbx r15 r12d r13d
..B4.53:                        # Preds ..B4.1
        movl      %r12d, -8(%rbp)                               #3662.1
..LN383:
   .loc    1  3705
        lea       (%r12,%r13), %r14d                            #3705.5
..LN385:
   .loc    1  3716
        subl      4(%r15), %r14d                                #3716.12
..LN387:
   .loc    1  3713
        movl      %r12d, %eax                                   #3713.49
        shrl      $2, %eax                                      #3713.49
..LN389:
        movq      %rax, -176(%rbp)                              #3713.30
        lea       (%rbx,%rax), %rdi                             #3713.30
..LN391:
   .loc    1  3719
        lea       3(%r12,%r13), %r13d                           #3719.50
..LN393:
        shrl      $2, %r13d                                     #3719.57
..LN395:
        addq      %rbx, %r13                                    #3719.30
..LN397:
   .loc    1  3722
        movq      %r13, %rsi                                    #3722.14
        call      prime_buffer_2na@PLT                          #3722.14
                                # LOE rbx r12 r13 r15 r12d r14d r12b xmm0
..B4.54:                        # Preds ..B4.53
..LN399:
   .loc    1  3723
        movq      -176(%rbp), %rax                              #3723.5
        lea       16(%rbx,%rax), %r8                            #3723.5
        movq      %r8, %rdi                                     #3723.5
..LN401:
   .loc    1  3728
        cmpq      %r13, %r8                                     #3728.16
..LN403:
   .loc    1  3733
..LN405:
   .loc    1  3728
        jae       ..B4.3        # Prob 12%                      #3728.16
                                # LOE rdi r8 r12 r13 r15 ebx r12d r14d r12b xmm0
..B4.2:                         # Preds ..B4.54
..LN407:
   .loc    1  3729
        movzbl    -1(%r8), %ebx                                 #3729.24
..LN409:
        shll      $8, %ebx                                      #3729.38
                                # LOE rdi r8 r12 r13 r15 ebx r12d r14d r12b xmm0
..B4.3:                         # Preds ..B4.2 ..B4.54
..LN411:
   .loc    1  3733
        movdqa    16(%r15), %xmm8                               #3733.5
        movdqa    32(%r15), %xmm7                               #3733.5
        movdqa    48(%r15), %xmm6                               #3733.5
        movdqa    64(%r15), %xmm5                               #3733.5
        movdqa    80(%r15), %xmm4                               #3733.5
        movdqa    96(%r15), %xmm3                               #3733.5
        movdqa    112(%r15), %xmm2                              #3733.5
        movdqa    128(%r15), %xmm1                              #3733.5
..LN413:
   .loc    1  3736
        xorl      %ecx, %ecx                                    #3736.15
..LN415:
        xorl      %esi, %esi                                    #3736.10
..LN417:
        xorl      %edx, %edx                                    #3736.5
..LN419:
   .loc    1  3738
        movl      %r14d, %eax                                   #3738.20
        subl      %r12d, %eax                                   #3738.20
..LN421:
        addl      $7, %eax                                      #3738.33
..LN423:
        shrl      $2, %eax                                      #3738.40
..LN425:
   .loc    1  3745
        movl      %r12d, %r9d                                   #3745.20
        andl      $3, %r9d                                      #3745.20
        je        ..B4.48       # Prob 20%                      #3745.20
                                # LOE rdi r8 r12 r13 eax edx ecx ebx esi r9d r12d r14d r12b xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.4:                         # Preds ..B4.3
        cmpl      $1, %r9d                                      #3745.20
        je        ..B4.10       # Prob 25%                      #3745.20
                                # LOE rdi r8 r12 r13 eax edx ecx ebx esi r9d r12d r14d r12b xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.5:                         # Preds ..B4.4
        cmpl      $2, %r9d                                      #3745.20
        je        ..B4.9        # Prob 33%                      #3745.20
                                # LOE rdi r8 r12 r13 eax edx ecx ebx esi r9d r12d r14d r12b xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.6:                         # Preds ..B4.5
        cmpl      $3, %r9d                                      #3745.20
        je        ..B4.8        # Prob 50%                      #3745.20
                                # LOE rdi r8 r12 r13 eax edx ecx ebx esi r12d r14d r12b xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.7:                         # Preds ..B4.6
        movq      %r13, %r15                                    #
        movq      %r8, %r13                                     #
        jmp       ..B4.19       # Prob 100%                     #
                                # LOE rdi r13 r15 ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.8:                         # Preds ..B4.6
        movq      %r13, %r15                                    #
        movq      %r8, %r13                                     #
        jmp       ..B4.13       # Prob 100%                     #
                                # LOE rdi r13 r15 eax edx ecx ebx esi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.9:                         # Preds ..B4.5
        movq      %r13, %r15                                    #
        movq      %r8, %r13                                     #
        jmp       ..B4.12       # Prob 100%                     #
                                # LOE rdi r13 r15 eax edx ebx esi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.10:                        # Preds ..B4.4
        movq      %r13, %r15                                    #
        movq      %r8, %r13                                     #
                                # LOE rdi r13 r15 eax edx ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.11:                        # Preds ..B4.10 ..B4.23
..LN427:
   .loc    1  3768
        movdqa    %xmm0, %xmm9                                  #3768.22
        pand      %xmm5, %xmm9                                  #3768.22
..LN429:
   .loc    1  3769
        pcmpeqd   %xmm6, %xmm9                                  #3769.22
..LN431:
   .loc    1  3770
        pmovmskb  %xmm9, %esi                                   #3770.22
..LN433:
   .loc    1  3771
        incl      %esi                                          #3771.17
        shrl      $16, %esi                                     #3771.17
        negl      %esi                                          #3771.17
                                # LOE rdi r13 r15 eax edx ebx esi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.12:                        # Preds ..B4.9 ..B4.11
..LN435:
   .loc    1  3774
        movdqa    %xmm0, %xmm9                                  #3774.22
        pand      %xmm3, %xmm9                                  #3774.22
..LN437:
   .loc    1  3775
        pcmpeqd   %xmm4, %xmm9                                  #3775.22
..LN439:
   .loc    1  3776
        pmovmskb  %xmm9, %ecx                                   #3776.22
..LN441:
   .loc    1  3777
        incl      %ecx                                          #3777.17
        shrl      $16, %ecx                                     #3777.17
        negl      %ecx                                          #3777.17
                                # LOE rdi r13 r15 eax edx ecx ebx esi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.13:                        # Preds ..B4.8 ..B4.12
..LN443:
   .loc    1  3780
        movdqa    %xmm0, %xmm9                                  #3780.22
..LN445:
   .loc    1  3787
        andl      $-4, %r12d                                    #3787.17
..LN447:
   .loc    1  3790
        movl      %edx, %r9d                                    #3790.29
..LN449:
   .loc    1  3780
        pand      %xmm1, %xmm9                                  #3780.22
..LN451:
   .loc    1  3781
        pcmpeqd   %xmm2, %xmm9                                  #3781.22
..LN453:
   .loc    1  3782
        pmovmskb  %xmm9, %r8d                                   #3782.22
..LN455:
   .loc    1  3790
        orl       %esi, %r9d                                    #3790.29
..LN457:
        orl       %ecx, %r9d                                    #3790.34
..LN459:
   .loc    1  3783
        incl      %r8d                                          #3783.17
        shrl      $16, %r8d                                     #3783.17
        negl      %r8d                                          #3783.17
..LN461:
   .loc    1  3790
        orl       %r8d, %r9d                                    #3790.39
..LN463:
        jne       ..B4.24       # Prob 20%                      #3790.47
                                # LOE rdi r13 r15 eax edx ecx ebx esi r8d r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.14:                        # Preds ..B4.13
..LN465:
   .loc    1  3853
        addl      $4, %r12d                                     #3853.17
..LN467:
   .loc    1  3856
        cmpl      %r14d, %r12d                                  #3856.28
        ja        ..B4.47       # Prob 20%                      #3856.28
                                # LOE rdi r13 r15 eax ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.15:                        # Preds ..B4.14
..LN469:
   .loc    1  3860
        decl      %eax                                          #3860.25
..LN471:
        jne       ..B4.20       # Prob 50%                      #3860.39
                                # LOE rdi r13 r15 eax ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.16:                        # Preds ..B4.15
..LN473:
   .loc    1  3881
        cmpq      %r15, %r13                                    #3881.25
        jae       ..B4.47       # Prob 4%                       #3881.25
                                # LOE r13 r15 ebx r12d r14d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.17:                        # Preds ..B4.16
..LN475:
   .loc    1  3891
        movdqa    %xmm1, -176(%rbp)                             #3891.22
        movdqa    %xmm2, -160(%rbp)                             #3891.22
        movdqa    %xmm3, -144(%rbp)                             #3891.22
        movdqa    %xmm4, -128(%rbp)                             #3891.22
        movdqa    %xmm5, -112(%rbp)                             #3891.22
        movdqa    %xmm6, -96(%rbp)                              #3891.22
        movdqa    %xmm7, -80(%rbp)                              #3891.22
        movdqa    %xmm8, -64(%rbp)                              #3891.22
        movq      %r13, %rdi                                    #3891.22
        movq      %r15, %rsi                                    #3891.22
        call      prime_buffer_2na@PLT                          #3891.22
                                # LOE r13 r15 ebx r12d r14d xmm0
..B4.55:                        # Preds ..B4.17
        movdqa    -64(%rbp), %xmm8                              #
        movdqa    -80(%rbp), %xmm7                              #
        movdqa    -96(%rbp), %xmm6                              #
        movdqa    -112(%rbp), %xmm5                             #
..LN477:
   .loc    1  3969
        addq      $16, %r13                                     #3969.13
        movq      %r13, %rdi                                    #3969.13
        movdqa    -128(%rbp), %xmm4                             #
        movdqa    -144(%rbp), %xmm3                             #
        movdqa    -160(%rbp), %xmm2                             #
        movdqa    -176(%rbp), %xmm1                             #
..LN479:
   .loc    1  3972
        cmpq      %r15, %r13                                    #3972.24
        jae       ..B4.19       # Prob 12%                      #3972.24
                                # LOE rdi r13 r15 ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.18:                        # Preds ..B4.55
..LN481:
   .loc    1  3973
        movzbl    -1(%r13), %ebx                                #3973.32
..LN483:
        shll      $8, %ebx                                      #3973.46
                                # LOE rdi r13 r15 ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.19:                        # Preds ..B4.55 ..B4.7 ..B4.18
..LN485:
   .loc    1  3752
        movl      $16, %eax                                     #3752.13
        jmp       ..B4.23       # Prob 100%                     #3752.13
                                # LOE rdi r13 r15 eax ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.20:                        # Preds ..B4.15
..LN487:
   .loc    1  3865
        psrldq    $1, %xmm0                                     #3865.26
..LN489:
   .loc    1  3868
        cmpq      %r15, %rdi                                    #3868.26
        jae       ..B4.22       # Prob 19%                      #3868.26
                                # LOE rdi r13 r15 eax ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.21:                        # Preds ..B4.20
..LN491:
   .loc    1  3871
        movzbl    (%rdi), %edx                                  #3871.37
..LN493:
   .loc    1  3870
        sarl      $8, %ebx                                      #3870.21
..LN495:
   .loc    1  3871
        shll      $8, %edx                                      #3871.48
..LN497:
        orl       %edx, %ebx                                    #3871.21
..LN499:
   .loc    1  3872
        pinsrw    $7, %ebx, %xmm0                               #3872.30
                                # LOE rdi r13 r15 eax ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.22:                        # Preds ..B4.21 ..B4.20
..LN501:
   .loc    1  3876
        incq      %rdi                                          #3876.20
                                # LOE rdi r13 r15 eax ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.23:                        # Preds ..B4.19 ..B4.22 ..B4.48
..LN503:
   .loc    1  3762
        movdqa    %xmm0, %xmm9                                  #3762.22
        pand      %xmm7, %xmm9                                  #3762.22
..LN505:
   .loc    1  3763
        pcmpeqd   %xmm8, %xmm9                                  #3763.22
..LN507:
   .loc    1  3764
        pmovmskb  %xmm9, %edx                                   #3764.22
..LN509:
   .loc    1  3765
        incl      %edx                                          #3765.17
        shrl      $16, %edx                                     #3765.17
        negl      %edx                                          #3765.17
        jmp       ..B4.11       # Prob 100%                     #3765.17
                                # LOE rdi r13 r15 eax edx ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.24:                        # Preds ..B4.13                 # Infreq
..LN511:
   .loc    1  3793
        subl      %r12d, %r14d                                  #3793.30
..LN513:
        je        ..B4.45       # Prob 25%                      #3793.37
                                # LOE edx ecx esi r8d r12d r14d
..B4.25:                        # Preds ..B4.24                 # Infreq
        cmpl      $1, %r14d                                     #3793.37
        jne       ..B4.30       # Prob 67%                      #3793.37
                                # LOE edx ecx esi r8d r12d r14d
..B4.26:                        # Preds ..B4.25                 # Infreq
..LN515:
   .loc    1  3799
        testl     %edx, %edx                                    #3799.36
        jne       ..B4.46       # Prob 28%                      #3799.36
                                # LOE esi r12d
..B4.27:                        # Preds ..B4.26                 # Infreq
..LN517:
   .loc    1  3800
        testl     %esi, %esi                                    #3800.36
        je        ..B4.47       # Prob 50%                      #3800.36
                                # LOE r12d
..B4.28:                        # Preds ..B4.38 ..B4.32 ..B4.27 # Infreq
..LN519:
        subl      -8(%rbp), %r12d                               #3800.47
..LN521:
        movq      -16(%rbp), %rbx                               #3800.61
..___tag_value_eval_2na_pos.244:                                #
        addl      $2, %r12d                                     #3800.61
        movl      %r12d, %eax                                   #3800.61
        movq      -24(%rbp), %r12                               #3800.61
..___tag_value_eval_2na_pos.245:                                #
        movq      -32(%rbp), %r13                               #3800.61
..___tag_value_eval_2na_pos.246:                                #
        movq      -40(%rbp), %r14                               #3800.61
..___tag_value_eval_2na_pos.247:                                #
        movq      -48(%rbp), %r15                               #3800.61
..___tag_value_eval_2na_pos.248:                                #
        movq      %rbp, %rsp                                    #3800.61
        popq      %rbp                                          #3800.61
..___tag_value_eval_2na_pos.249:                                #
        ret                                                     #3800.61
..___tag_value_eval_2na_pos.250:                                #
                                # LOE
..B4.30:                        # Preds ..B4.25                 # Infreq
..LN523:
   .loc    1  3793
        cmpl      $2, %r14d                                     #3793.37
        jne       ..B4.37       # Prob 50%                      #3793.37
                                # LOE edx ecx esi r8d r12d
..B4.31:                        # Preds ..B4.30                 # Infreq
..LN525:
   .loc    1  3803
        testl     %edx, %edx                                    #3803.36
        jne       ..B4.46       # Prob 28%                      #3803.36
                                # LOE ecx esi r12d
..B4.32:                        # Preds ..B4.31                 # Infreq
..LN527:
   .loc    1  3804
        testl     %esi, %esi                                    #3804.36
        jne       ..B4.28       # Prob 28%                      #3804.36
                                # LOE ecx r12d
..B4.33:                        # Preds ..B4.32                 # Infreq
..LN529:
   .loc    1  3805
        testl     %ecx, %ecx                                    #3805.36
        je        ..B4.47       # Prob 50%                      #3805.36
                                # LOE r12d
..B4.34:                        # Preds ..B4.39 ..B4.33         # Infreq
..LN531:
        subl      -8(%rbp), %r12d                               #3805.47
..LN533:
        movq      -16(%rbp), %rbx                               #3805.61
..___tag_value_eval_2na_pos.256:                                #
        addl      $3, %r12d                                     #3805.61
        movl      %r12d, %eax                                   #3805.61
        movq      -24(%rbp), %r12                               #3805.61
..___tag_value_eval_2na_pos.257:                                #
        movq      -32(%rbp), %r13                               #3805.61
..___tag_value_eval_2na_pos.258:                                #
        movq      -40(%rbp), %r14                               #3805.61
..___tag_value_eval_2na_pos.259:                                #
        movq      -48(%rbp), %r15                               #3805.61
..___tag_value_eval_2na_pos.260:                                #
        movq      %rbp, %rsp                                    #3805.61
        popq      %rbp                                          #3805.61
..___tag_value_eval_2na_pos.261:                                #
        ret                                                     #3805.61
..___tag_value_eval_2na_pos.262:                                #
                                # LOE
..B4.37:                        # Preds ..B4.30                 # Infreq
..LN535:
   .loc    1  3808
        testl     %edx, %edx                                    #3808.36
        jne       ..B4.46       # Prob 28%                      #3808.36
                                # LOE ecx esi r8d r12d
..B4.38:                        # Preds ..B4.37                 # Infreq
..LN537:
   .loc    1  3809
        testl     %esi, %esi                                    #3809.36
        jne       ..B4.28       # Prob 28%                      #3809.36
                                # LOE ecx r8d r12d
..B4.39:                        # Preds ..B4.38                 # Infreq
..LN539:
   .loc    1  3810
        testl     %ecx, %ecx                                    #3810.36
        jne       ..B4.34       # Prob 28%                      #3810.36
                                # LOE r8d r12d
..B4.40:                        # Preds ..B4.39                 # Infreq
..LN541:
   .loc    1  3811
        testl     %r8d, %r8d                                    #3811.36
        je        ..B4.47       # Prob 50%                      #3811.36
                                # LOE r12d
..B4.41:                        # Preds ..B4.40                 # Infreq
..LN543:
        subl      -8(%rbp), %r12d                               #3811.47
..LN545:
        movq      -16(%rbp), %rbx                               #3811.61
..___tag_value_eval_2na_pos.268:                                #
        addl      $4, %r12d                                     #3811.61
        movl      %r12d, %eax                                   #3811.61
        movq      -24(%rbp), %r12                               #3811.61
..___tag_value_eval_2na_pos.269:                                #
        movq      -32(%rbp), %r13                               #3811.61
..___tag_value_eval_2na_pos.270:                                #
        movq      -40(%rbp), %r14                               #3811.61
..___tag_value_eval_2na_pos.271:                                #
        movq      -48(%rbp), %r15                               #3811.61
..___tag_value_eval_2na_pos.272:                                #
        movq      %rbp, %rsp                                    #3811.61
        popq      %rbp                                          #3811.61
..___tag_value_eval_2na_pos.273:                                #
        ret                                                     #3811.61
..___tag_value_eval_2na_pos.274:                                #
                                # LOE
..B4.45:                        # Preds ..B4.24                 # Infreq
..LN547:
   .loc    1  3796
        testl     %edx, %edx                                    #3796.36
        je        ..B4.47       # Prob 50%                      #3796.36
                                # LOE r12d
..B4.46:                        # Preds ..B4.37 ..B4.31 ..B4.26 ..B4.45 # Infreq
..LN549:
        subl      -8(%rbp), %r12d                               #3796.47
..LN551:
        movq      -16(%rbp), %rbx                               #3796.61
..___tag_value_eval_2na_pos.280:                                #
        incl      %r12d                                         #3796.61
        movl      %r12d, %eax                                   #3796.61
        movq      -24(%rbp), %r12                               #3796.61
..___tag_value_eval_2na_pos.281:                                #
        movq      -32(%rbp), %r13                               #3796.61
..___tag_value_eval_2na_pos.282:                                #
        movq      -40(%rbp), %r14                               #3796.61
..___tag_value_eval_2na_pos.283:                                #
        movq      -48(%rbp), %r15                               #3796.61
..___tag_value_eval_2na_pos.284:                                #
        movq      %rbp, %rsp                                    #3796.61
        popq      %rbp                                          #3796.61
..___tag_value_eval_2na_pos.285:                                #
        ret                                                     #3796.61
..___tag_value_eval_2na_pos.286:                                #
                                # LOE
..B4.47:                        # Preds ..B4.14 ..B4.16 ..B4.40 ..B4.33 ..B4.27
                                #       ..B4.45                 # Infreq
..LN553:
   .loc    1  3813
        movq      -16(%rbp), %rbx                               #3813.28
..___tag_value_eval_2na_pos.292:                                #
        movq      -24(%rbp), %r12                               #3813.28
..___tag_value_eval_2na_pos.293:                                #
        movq      -32(%rbp), %r13                               #3813.28
..___tag_value_eval_2na_pos.294:                                #
        movq      -40(%rbp), %r14                               #3813.28
..___tag_value_eval_2na_pos.295:                                #
        xorl      %eax, %eax                                    #3813.28
        movq      -48(%rbp), %r15                               #3813.28
..___tag_value_eval_2na_pos.296:                                #
        movq      %rbp, %rsp                                    #3813.28
        popq      %rbp                                          #3813.28
..___tag_value_eval_2na_pos.297:                                #
        ret                                                     #3813.28
..___tag_value_eval_2na_pos.298:                                #
                                # LOE
..B4.48:                        # Preds ..B4.3                  # Infreq
        movq      %r13, %r15                                    #
        movq      %r8, %r13                                     #
        jmp       ..B4.23       # Prob 100%                     #
        .align    16,0x90
..___tag_value_eval_2na_pos.304:                                #
                                # LOE rdi r13 r15 eax ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
# mark_end;
	.type	eval_2na_pos,@function
	.size	eval_2na_pos,.-eval_2na_pos
.LNeval_2na_pos:
	.data
# -- End  eval_2na_pos
	.text
# -- Begin  prime_buffer_2na
# mark_begin;
       .align    16,0x90
prime_buffer_2na:
# parameter 1(src): %rdi
# parameter 2(ignore): %rsi
..B5.1:                         # Preds ..B5.0
..___tag_value_prime_buffer_2na.305:                            #2129.1
..LN555:
   .loc    1  2129
        pushq     %rbp                                          #2129.1
        movq      %rsp, %rbp                                    #2129.1
..___tag_value_prime_buffer_2na.306:                            #
        subq      $16, %rsp                                     #2129.1
        movq      %r12, -16(%rbp)                               #2129.1
..___tag_value_prime_buffer_2na.308:                            #
        movq      %rdi, %r12                                    #2129.1
        lea       _gprof_pack4(%rip), %rdx                      #2129.1
        call      mcount@PLT                                    #2129.1
                                # LOE rbx r12 r13 r14 r15
..B5.7:                         # Preds ..B5.1
..LN557:
   .loc    1  2132
        testq     $15, %r12                                     #2132.29
..LN559:
        jne       ..B5.3        # Prob 50%                      #2132.37
                                # LOE rbx r12 r13 r14 r15
..B5.2:                         # Preds ..B5.7
..LN561:
   .loc    1  2133
        movdqa    (%r12), %xmm0                                 #2133.54
        jmp       ..B5.4        # Prob 100%                     #2133.54
                                # LOE rbx r13 r14 r15 xmm0
..B5.3:                         # Preds ..B5.7
..LN563:
   .loc    1  2135
        movdqu    (%r12), %xmm0                                 #2135.55
                                # LOE rbx r13 r14 r15 xmm0
..B5.4:                         # Preds ..B5.2 ..B5.3
..LN565:
   .loc    1  2136
        movq      -16(%rbp), %r12                               #2136.12
..___tag_value_prime_buffer_2na.309:                            #
        movq      %rbp, %rsp                                    #2136.12
        popq      %rbp                                          #2136.12
..___tag_value_prime_buffer_2na.310:                            #
        ret                                                     #2136.12
        .align    16,0x90
..___tag_value_prime_buffer_2na.311:                            #
                                # LOE
# mark_end;
	.type	prime_buffer_2na,@function
	.size	prime_buffer_2na,.-prime_buffer_2na
.LNprime_buffer_2na:
	.data
# -- End  prime_buffer_2na
	.text
# -- Begin  eval_4na_128
# mark_begin;
       .align    16,0x90
eval_4na_128:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B6.1:                         # Preds ..B6.0
..___tag_value_eval_4na_128.312:                                #4724.1
..LN567:
   .loc    1  4724
        pushq     %rbp                                          #4724.1
        movq      %rsp, %rbp                                    #4724.1
..___tag_value_eval_4na_128.313:                                #
        subq      $176, %rsp                                    #4724.1
        movq      %r15, -16(%rbp)                               #4724.1
..___tag_value_eval_4na_128.315:                                #
        movq      %r14, -24(%rbp)                               #4724.1
..___tag_value_eval_4na_128.316:                                #
        movq      %r13, -32(%rbp)                               #4724.1
..___tag_value_eval_4na_128.317:                                #
        movq      %r12, -40(%rbp)                               #4724.1
..___tag_value_eval_4na_128.318:                                #
        movq      %rbx, -48(%rbp)                               #4724.1
..___tag_value_eval_4na_128.319:                                #
        movl      %edx, %r14d                                   #4724.1
        movq      %rsi, %r15                                    #4724.1
        movq      %rdi, %rbx                                    #4724.1
        lea       _gprof_pack5(%rip), %rdx                      #4724.1
        movl      %ecx, -168(%rbp)                              #4724.1
        call      mcount@PLT                                    #4724.1
                                # LOE rbx r15 r14d
..B6.37:                        # Preds ..B6.1
        movl      -168(%rbp), %ecx                              #
..LN569:
   .loc    1  4757
        lea       (%r14,%rcx), %r12d                            #4757.5
..LN571:
   .loc    1  4763
        subl      4(%rbx), %r12d                                #4763.12
..LN573:
   .loc    1  4760
        movl      %r14d, %r13d                                  #4760.49
        shrl      $2, %r13d                                     #4760.49
..LN575:
        lea       (%r15,%r13), %rdi                             #4760.30
..LN577:
   .loc    1  4766
        lea       3(%r14,%rcx), %esi                            #4766.50
..LN579:
        shrl      $2, %esi                                      #4766.57
..LN581:
        addq      %r15, %rsi                                    #4766.30
        movq      %rsi, -176(%rbp)                              #4766.30
..LN583:
   .loc    1  4769
        call      prime_buffer_4na@PLT                          #4769.14
                                # LOE rbx r13 r15 r12d r14d xmm0
..B6.38:                        # Preds ..B6.37
..LN585:
   .loc    1  4776
        movdqa    16(%rbx), %xmm8                               #4776.5
        movdqa    32(%rbx), %xmm7                               #4776.5
        movdqa    48(%rbx), %xmm6                               #4776.5
        movdqa    64(%rbx), %xmm5                               #4776.5
        movdqa    80(%rbx), %xmm4                               #4776.5
        movdqa    96(%rbx), %xmm3                               #4776.5
        movdqa    112(%rbx), %xmm2                              #4776.5
        movdqa    128(%rbx), %xmm1                              #4776.5
..LN587:
   .loc    1  4770
        lea       8(%r15,%r13), %r13                            #4770.5
        movq      %r13, %rdx                                    #4770.5
..LN589:
   .loc    1  4779
        xorl      %esi, %esi                                    #4779.15
..LN591:
        xorl      %edi, %edi                                    #4779.10
..LN593:
        xorl      %ecx, %ecx                                    #4779.5
..LN595:
   .loc    1  4781
        movl      %r12d, %eax                                   #4781.20
        subl      %r14d, %eax                                   #4781.20
..LN597:
        addl      $7, %eax                                      #4781.33
..LN599:
        shrl      $2, %eax                                      #4781.40
..LN601:
   .loc    1  4790
        movl      %r14d, %ebx                                   #4790.20
        andl      $3, %ebx                                      #4790.20
        je        ..B6.32       # Prob 20%                      #4790.20
                                # LOE rdx r13 eax ecx ebx esi edi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.2:                         # Preds ..B6.38
        cmpl      $1, %ebx                                      #4790.20
        je        ..B6.8        # Prob 25%                      #4790.20
                                # LOE rdx r13 eax ecx ebx esi edi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.3:                         # Preds ..B6.2
        cmpl      $2, %ebx                                      #4790.20
        je        ..B6.7        # Prob 33%                      #4790.20
                                # LOE rdx r13 eax ecx ebx esi edi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.4:                         # Preds ..B6.3
        cmpl      $3, %ebx                                      #4790.20
        je        ..B6.6        # Prob 50%                      #4790.20
                                # LOE rdx r13 eax ecx esi edi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.5:                         # Preds ..B6.4
        movq      -176(%rbp), %rbx                              #
        jmp       ..B6.16       # Prob 100%                     #
                                # LOE rdx rbx r13 r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.6:                         # Preds ..B6.4
        movq      -176(%rbp), %rbx                              #
        jmp       ..B6.11       # Prob 100%                     #
                                # LOE rdx rbx r13 eax ecx esi edi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.7:                         # Preds ..B6.3
        movq      -176(%rbp), %rbx                              #
        jmp       ..B6.10       # Prob 100%                     #
                                # LOE rdx rbx r13 eax ecx edi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.8:                         # Preds ..B6.2
        movq      -176(%rbp), %rbx                              #
                                # LOE rdx rbx r13 eax ecx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.9:                         # Preds ..B6.8 ..B6.20
..LN603:
   .loc    1  4814
        movdqa    %xmm0, %xmm10                                 #4814.22
..LN605:
   .loc    1  4815
        movdqa    %xmm0, %xmm9                                  #4815.22
..LN607:
   .loc    1  4814
        pand      %xmm6, %xmm10                                 #4814.22
..LN609:
   .loc    1  4815
        pand      %xmm5, %xmm9                                  #4815.22
..LN611:
   .loc    1  4816
        pcmpeqd   %xmm9, %xmm10                                 #4816.22
..LN613:
   .loc    1  4817
        pmovmskb  %xmm10, %edi                                  #4817.22
..LN615:
   .loc    1  4818
        incl      %edi                                          #4818.17
        shrl      $16, %edi                                     #4818.17
        negl      %edi                                          #4818.17
                                # LOE rdx rbx r13 eax ecx edi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.10:                        # Preds ..B6.7 ..B6.9
..LN617:
   .loc    1  4821
        movdqa    %xmm0, %xmm10                                 #4821.22
..LN619:
   .loc    1  4822
        movdqa    %xmm0, %xmm9                                  #4822.22
..LN621:
   .loc    1  4821
        pand      %xmm4, %xmm10                                 #4821.22
..LN623:
   .loc    1  4822
        pand      %xmm3, %xmm9                                  #4822.22
..LN625:
   .loc    1  4823
        pcmpeqd   %xmm9, %xmm10                                 #4823.22
..LN627:
   .loc    1  4824
        pmovmskb  %xmm10, %esi                                  #4824.22
..LN629:
   .loc    1  4825
        incl      %esi                                          #4825.17
        shrl      $16, %esi                                     #4825.17
        negl      %esi                                          #4825.17
                                # LOE rdx rbx r13 eax ecx esi edi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.11:                        # Preds ..B6.6 ..B6.10
..LN631:
   .loc    1  4828
        movdqa    %xmm0, %xmm10                                 #4828.22
..LN633:
   .loc    1  4829
        movdqa    %xmm0, %xmm9                                  #4829.22
..LN635:
   .loc    1  4836
        andl      $-4, %r14d                                    #4836.17
..LN637:
   .loc    1  4828
        pand      %xmm2, %xmm10                                 #4828.22
..LN639:
   .loc    1  4829
        pand      %xmm1, %xmm9                                  #4829.22
..LN641:
   .loc    1  4830
        pcmpeqd   %xmm9, %xmm10                                 #4830.22
..LN643:
   .loc    1  4831
        pmovmskb  %xmm10, %r8d                                  #4831.22
..LN645:
   .loc    1  4839
        movl      %ecx, %r9d                                    #4839.29
        orl       %edi, %r9d                                    #4839.29
..LN647:
   .loc    1  4832
        incl      %r8d                                          #4832.17
        shrl      $16, %r8d                                     #4832.17
        negl      %r8d                                          #4832.17
..LN649:
   .loc    1  4839
        orl       %esi, %r9d                                    #4839.34
..LN651:
        orl       %r8d, %r9d                                    #4839.39
..LN653:
        jne       ..B6.21       # Prob 20%                      #4839.47
                                # LOE rdx rbx r13 eax ecx esi edi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.12:                        # Preds ..B6.11
..LN655:
   .loc    1  4880
        addl      $4, %r14d                                     #4880.17
..LN657:
   .loc    1  4883
        cmpl      %r12d, %r14d                                  #4883.28
        ja        ..B6.29       # Prob 20%                      #4883.28
                                # LOE rdx rbx r13 eax r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.13:                        # Preds ..B6.12
..LN659:
   .loc    1  4887
        decl      %eax                                          #4887.25
..LN661:
        jne       ..B6.17       # Prob 50%                      #4887.39
                                # LOE rdx rbx r13 eax r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.14:                        # Preds ..B6.13
..LN663:
   .loc    1  4904
        cmpq      %rbx, %r13                                    #4904.25
        jae       ..B6.29       # Prob 4%                       #4904.25
                                # LOE rbx r13 r12d r14d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.15:                        # Preds ..B6.14
..LN665:
   .loc    1  4914
        movdqa    %xmm1, -176(%rbp)                             #4914.22
        movdqa    %xmm2, -160(%rbp)                             #4914.22
        movdqa    %xmm3, -144(%rbp)                             #4914.22
        movdqa    %xmm4, -128(%rbp)                             #4914.22
        movdqa    %xmm5, -112(%rbp)                             #4914.22
        movdqa    %xmm6, -96(%rbp)                              #4914.22
        movdqa    %xmm7, -80(%rbp)                              #4914.22
        movdqa    %xmm8, -64(%rbp)                              #4914.22
        movq      %r13, %rdi                                    #4914.22
        movq      %rbx, %rsi                                    #4914.22
        call      prime_buffer_4na@PLT                          #4914.22
                                # LOE rbx r13 r12d r14d xmm0
..B6.39:                        # Preds ..B6.15
        movdqa    -64(%rbp), %xmm8                              #
        movdqa    -80(%rbp), %xmm7                              #
        movdqa    -96(%rbp), %xmm6                              #
        movdqa    -112(%rbp), %xmm5                             #
..LN667:
   .loc    1  4929
        addq      $8, %r13                                      #4929.13
        movq      %r13, %rdx                                    #4929.13
        movdqa    -128(%rbp), %xmm4                             #
        movdqa    -144(%rbp), %xmm3                             #
        movdqa    -160(%rbp), %xmm2                             #
        movdqa    -176(%rbp), %xmm1                             #
                                # LOE rdx rbx r13 r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.16:                        # Preds ..B6.5 ..B6.39
..LN669:
   .loc    1  4797
        movl      $8, %eax                                      #4797.13
        jmp       ..B6.20       # Prob 100%                     #4797.13
                                # LOE rdx rbx r13 eax r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.17:                        # Preds ..B6.13
..LN671:
   .loc    1  4892
        psrldq    $2, %xmm0                                     #4892.26
..LN673:
   .loc    1  4895
        cmpq      %rbx, %rdx                                    #4895.26
        jae       ..B6.19       # Prob 19%                      #4895.26
                                # LOE rdx rbx r13 eax r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.18:                        # Preds ..B6.17
..LN675:
   .loc    1  4896
        movzbl    (%rdx), %esi                                  #4896.72
..LN677:
        lea       expand_2na.0(%rip), %rcx                      #4896.57
        movzwl    (%rcx,%rsi,2), %edi                           #4896.57
..LN679:
        pinsrw    $7, %edi, %xmm0                               #4896.30
                                # LOE rdx rbx r13 eax r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.19:                        # Preds ..B6.18 ..B6.17
..LN681:
   .loc    1  4899
        incq      %rdx                                          #4899.20
                                # LOE rdx rbx r13 eax r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.20:                        # Preds ..B6.16 ..B6.19 ..B6.32
..LN683:
   .loc    1  4807
        movdqa    %xmm0, %xmm10                                 #4807.22
..LN685:
   .loc    1  4808
        movdqa    %xmm0, %xmm9                                  #4808.22
..LN687:
   .loc    1  4807
        pand      %xmm8, %xmm10                                 #4807.22
..LN689:
   .loc    1  4808
        pand      %xmm7, %xmm9                                  #4808.22
..LN691:
   .loc    1  4809
        pcmpeqd   %xmm9, %xmm10                                 #4809.22
..LN693:
   .loc    1  4810
        pmovmskb  %xmm10, %ecx                                  #4810.22
..LN695:
   .loc    1  4811
        incl      %ecx                                          #4811.17
        shrl      $16, %ecx                                     #4811.17
        negl      %ecx                                          #4811.17
        jmp       ..B6.9        # Prob 100%                     #4811.17
                                # LOE rdx rbx r13 eax ecx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.21:                        # Preds ..B6.11                 # Infreq
..LN697:
   .loc    1  4842
        subl      %r14d, %r12d                                  #4842.30
..LN699:
        cmpl      $2, %r12d                                     #4842.37
        je        ..B6.25       # Prob 25%                      #4842.37
                                # LOE ecx esi edi r12d
..B6.22:                        # Preds ..B6.21                 # Infreq
        cmpl      $1, %r12d                                     #4842.37
        je        ..B6.26       # Prob 33%                      #4842.37
                                # LOE ecx edi r12d
..B6.23:                        # Preds ..B6.22                 # Infreq
        testl     %r12d, %r12d                                  #4842.37
        je        ..B6.27       # Prob 50%                      #4842.37
                                # LOE ecx
..B6.24:                        # Preds ..B6.23                 # Infreq
..LN701:
   .loc    1  4845
        movq      -48(%rbp), %rbx                               #4845.32
..___tag_value_eval_4na_128.320:                                #
        movq      -40(%rbp), %r12                               #4845.32
..___tag_value_eval_4na_128.321:                                #
        movq      -32(%rbp), %r13                               #4845.32
..___tag_value_eval_4na_128.322:                                #
        movq      -24(%rbp), %r14                               #4845.32
..___tag_value_eval_4na_128.323:                                #
        movl      $1, %eax                                      #4845.32
        movq      -16(%rbp), %r15                               #4845.32
..___tag_value_eval_4na_128.324:                                #
        movq      %rbp, %rsp                                    #4845.32
        popq      %rbp                                          #4845.32
..___tag_value_eval_4na_128.325:                                #
        ret                                                     #4845.32
..___tag_value_eval_4na_128.326:                                #
                                # LOE
..B6.25:                        # Preds ..B6.21                 # Infreq
..LN703:
   .loc    1  4847
        testl     %esi, %esi                                    #4847.36
        jne       ..B6.28       # Prob 28%                      #4847.36
                                # LOE ecx edi
..B6.26:                        # Preds ..B6.22 ..B6.25         # Infreq
..LN705:
   .loc    1  4849
        testl     %edi, %edi                                    #4849.36
        jne       ..B6.28       # Prob 28%                      #4849.36
                                # LOE ecx
..B6.27:                        # Preds ..B6.26 ..B6.23         # Infreq
..LN707:
   .loc    1  4851
        testl     %ecx, %ecx                                    #4851.36
        je        ..B6.29       # Prob 50%                      #4851.36
                                # LOE
..B6.28:                        # Preds ..B6.25 ..B6.26 ..B6.27 # Infreq
..LN709:
        movq      -48(%rbp), %rbx                               #4851.47
..___tag_value_eval_4na_128.332:                                #
        movq      -40(%rbp), %r12                               #4851.47
..___tag_value_eval_4na_128.333:                                #
        movq      -32(%rbp), %r13                               #4851.47
..___tag_value_eval_4na_128.334:                                #
        movq      -24(%rbp), %r14                               #4851.47
..___tag_value_eval_4na_128.335:                                #
        movl      $1, %eax                                      #4851.47
        movq      -16(%rbp), %r15                               #4851.47
..___tag_value_eval_4na_128.336:                                #
        movq      %rbp, %rsp                                    #4851.47
        popq      %rbp                                          #4851.47
..___tag_value_eval_4na_128.337:                                #
        ret                                                     #4851.47
..___tag_value_eval_4na_128.338:                                #
                                # LOE
..B6.29:                        # Preds ..B6.14 ..B6.12 ..B6.27 # Infreq
..LN711:
   .loc    1  4853
        movq      -48(%rbp), %rbx                               #4853.28
..___tag_value_eval_4na_128.344:                                #
        movq      -40(%rbp), %r12                               #4853.28
..___tag_value_eval_4na_128.345:                                #
        movq      -32(%rbp), %r13                               #4853.28
..___tag_value_eval_4na_128.346:                                #
        movq      -24(%rbp), %r14                               #4853.28
..___tag_value_eval_4na_128.347:                                #
        xorl      %eax, %eax                                    #4853.28
        movq      -16(%rbp), %r15                               #4853.28
..___tag_value_eval_4na_128.348:                                #
        movq      %rbp, %rsp                                    #4853.28
        popq      %rbp                                          #4853.28
..___tag_value_eval_4na_128.349:                                #
        ret                                                     #4853.28
..___tag_value_eval_4na_128.350:                                #
                                # LOE
..B6.32:                        # Preds ..B6.38                 # Infreq
        movq      -176(%rbp), %rbx                              #
        jmp       ..B6.20       # Prob 100%                     #
        .align    16,0x90
..___tag_value_eval_4na_128.356:                                #
                                # LOE rdx rbx r13 eax r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
# mark_end;
	.type	eval_4na_128,@function
	.size	eval_4na_128,.-eval_4na_128
.LNeval_4na_128:
	.data
# -- End  eval_4na_128
	.text
# -- Begin  eval_4na_32
# mark_begin;
       .align    16,0x90
eval_4na_32:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B7.1:                         # Preds ..B7.0
..___tag_value_eval_4na_32.357:                                 #4279.1
..LN713:
   .loc    1  4279
        pushq     %rbp                                          #4279.1
        movq      %rsp, %rbp                                    #4279.1
..___tag_value_eval_4na_32.358:                                 #
        subq      $256, %rsp                                    #4279.1
        movq      %r15, -184(%rbp)                              #4279.1
..___tag_value_eval_4na_32.360:                                 #
        movq      %r14, -24(%rbp)                               #4279.1
..___tag_value_eval_4na_32.361:                                 #
        movq      %r13, -32(%rbp)                               #4279.1
..___tag_value_eval_4na_32.362:                                 #
        movq      %r12, -48(%rbp)                               #4279.1
..___tag_value_eval_4na_32.363:                                 #
        movq      %rbx, -40(%rbp)                               #4279.1
..___tag_value_eval_4na_32.364:                                 #
        movl      %ecx, %r12d                                   #4279.1
        movl      %edx, %r13d                                   #4279.1
        movq      %rsi, %r15                                    #4279.1
        movq      %rdi, %rbx                                    #4279.1
        lea       _gprof_pack6(%rip), %rdx                      #4279.1
        call      mcount@PLT                                    #4279.1
                                # LOE rbx r15 r12d r13d
..B7.45:                        # Preds ..B7.1
..LN715:
   .loc    1  4312
        lea       (%r13,%r12), %eax                             #4312.5
..LN717:
   .loc    1  4318
        subl      4(%rbx), %eax                                 #4318.12
..LN719:
   .loc    1  4324
        movl      %eax, -8(%rbp)                                #4324.14
..LN721:
   .loc    1  4315
        movl      %r13d, %r14d                                  #4315.49
        shrl      $2, %r14d                                     #4315.49
..LN723:
        lea       (%r15,%r14), %rdi                             #4315.30
..LN725:
   .loc    1  4321
        lea       3(%r13,%r12), %esi                            #4321.50
..LN727:
        shrl      $2, %esi                                      #4321.57
..LN729:
        addq      %r15, %rsi                                    #4321.30
        movq      %rsi, -16(%rbp)                               #4321.30
..LN731:
   .loc    1  4324
        call      prime_buffer_4na@PLT                          #4324.14
                                # LOE rbx r14 r15 r13d xmm0
..B7.46:                        # Preds ..B7.45
        movl      -8(%rbp), %eax                                #
..LN733:
   .loc    1  4331
        movdqa    16(%rbx), %xmm7                               #4331.5
        movdqa    32(%rbx), %xmm6                               #4331.5
        movdqa    48(%rbx), %xmm5                               #4331.5
        movdqa    64(%rbx), %xmm4                               #4331.5
        movdqa    80(%rbx), %xmm3                               #4331.5
        movdqa    96(%rbx), %xmm2                               #4331.5
        movdqa    112(%rbx), %xmm1                              #4331.5
        movdqa    128(%rbx), %xmm8                              #4331.5
..LN735:
   .loc    1  4325
        lea       8(%r15,%r14), %r12                            #4325.5
        movq      %r12, %rsi                                    #4325.5
..LN737:
   .loc    1  4334
        xorl      %r15d, %r15d                                  #4334.15
..LN739:
        xorl      %r14d, %r14d                                  #4334.10
..LN741:
        xorl      %ecx, %ecx                                    #4334.5
..LN743:
   .loc    1  4340
        movl      $2, %ebx                                      #4340.5
..LN745:
   .loc    1  4345
        movl      %r13d, %edx                                   #4345.20
        andl      $3, %edx                                      #4345.20
        je        ..B7.37       # Prob 20%                      #4345.20
                                # LOE rax rsi r12 eax edx ecx ebx r13d r14d r15d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.2:                         # Preds ..B7.46
        cmpl      $1, %edx                                      #4345.20
        je        ..B7.8        # Prob 25%                      #4345.20
                                # LOE rax rsi r12 eax edx ecx ebx r13d r14d r15d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.3:                         # Preds ..B7.2
        cmpl      $2, %edx                                      #4345.20
        je        ..B7.7        # Prob 33%                      #4345.20
                                # LOE rax rsi r12 eax edx ecx ebx r13d r14d r15d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.4:                         # Preds ..B7.3
        cmpl      $3, %edx                                      #4345.20
        je        ..B7.6        # Prob 50%                      #4345.20
                                # LOE rax rsi r12 eax ecx ebx r13d r14d r15d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.5:                         # Preds ..B7.4
        movdqa    %xmm8, -80(%rbp)                              #
        movdqa    %xmm1, -64(%rbp)                              #
        movdqa    %xmm2, -176(%rbp)                             #
        movdqa    %xmm3, -128(%rbp)                             #
        movdqa    %xmm4, -112(%rbp)                             #
        movdqa    %xmm5, -160(%rbp)                             #
        movdqa    %xmm6, -144(%rbp)                             #
        movdqa    %xmm7, -96(%rbp)                              #
        movl      %eax, -8(%rbp)                                #
        movl      %r13d, %eax                                   #
        jmp       ..B7.29       # Prob 100%                     #
                                # LOE rsi r12 eax xmm0
..B7.6:                         # Preds ..B7.4
        movdqa    %xmm8, -80(%rbp)                              #
        movdqa    %xmm1, -64(%rbp)                              #
        movdqa    %xmm2, -176(%rbp)                             #
        movdqa    %xmm3, -128(%rbp)                             #
        movdqa    %xmm4, -112(%rbp)                             #
        movdqa    %xmm5, -160(%rbp)                             #
        movdqa    %xmm6, -144(%rbp)                             #
        movdqa    %xmm7, -96(%rbp)                              #
        movl      %eax, -8(%rbp)                                #
        movl      %r13d, %eax                                   #
        jmp       ..B7.11       # Prob 100%                     #
                                # LOE rsi r12 eax ecx ebx r14d r15d xmm0
..B7.7:                         # Preds ..B7.3
        movdqa    %xmm8, -80(%rbp)                              #
        movdqa    %xmm1, -64(%rbp)                              #
        movdqa    %xmm2, -176(%rbp)                             #
        movdqa    %xmm3, -128(%rbp)                             #
        movdqa    %xmm4, -112(%rbp)                             #
        movdqa    %xmm5, -160(%rbp)                             #
        movdqa    %xmm6, -144(%rbp)                             #
        movdqa    %xmm7, -96(%rbp)                              #
        movl      %eax, -8(%rbp)                                #
        movl      %r13d, %eax                                   #
        jmp       ..B7.10       # Prob 100%                     #
                                # LOE rsi r12 eax ecx ebx r14d xmm0
..B7.8:                         # Preds ..B7.2
        movdqa    %xmm8, -80(%rbp)                              #
        movdqa    %xmm1, -64(%rbp)                              #
        movdqa    %xmm2, -176(%rbp)                             #
        movdqa    %xmm3, -128(%rbp)                             #
        movdqa    %xmm4, -112(%rbp)                             #
        movdqa    %xmm5, -160(%rbp)                             #
        movdqa    %xmm6, -144(%rbp)                             #
        movdqa    %xmm7, -96(%rbp)                              #
        movl      %eax, -8(%rbp)                                #
        movl      %r13d, %eax                                   #
                                # LOE rsi r12 eax ecx ebx xmm0
..B7.9:                         # Preds ..B7.8 ..B7.33
..LN747:
   .loc    1  4369
        movdqa    %xmm0, %xmm2                                  #4369.22
..LN749:
   .loc    1  4370
        movdqa    %xmm0, %xmm1                                  #4370.22
..LN751:
   .loc    1  4369
        pand      -160(%rbp), %xmm2                             #4369.22
..LN753:
   .loc    1  4370
        pand      -112(%rbp), %xmm1                             #4370.22
..LN755:
   .loc    1  4371
        pcmpeqd   %xmm1, %xmm2                                  #4371.22
..LN757:
   .loc    1  4372
        pmovmskb  %xmm2, %r14d                                  #4372.22
                                # LOE rsi r12 eax ecx ebx r14d xmm0
..B7.10:                        # Preds ..B7.7 ..B7.9
..LN759:
   .loc    1  4376
        movdqa    %xmm0, %xmm2                                  #4376.22
..LN761:
   .loc    1  4377
        movdqa    %xmm0, %xmm1                                  #4377.22
..LN763:
   .loc    1  4376
        pand      -128(%rbp), %xmm2                             #4376.22
..LN765:
   .loc    1  4377
        pand      -176(%rbp), %xmm1                             #4377.22
..LN767:
   .loc    1  4378
        pcmpeqd   %xmm1, %xmm2                                  #4378.22
..LN769:
   .loc    1  4379
        pmovmskb  %xmm2, %r15d                                  #4379.22
                                # LOE rsi r12 eax ecx ebx r14d r15d xmm0
..B7.11:                        # Preds ..B7.6 ..B7.10
..LN771:
   .loc    1  4383
        movdqa    %xmm0, %xmm2                                  #4383.22
..LN773:
   .loc    1  4384
        movdqa    %xmm0, %xmm1                                  #4384.22
..LN775:
   .loc    1  4383
        pand      -64(%rbp), %xmm2                              #4383.22
..LN777:
   .loc    1  4384
        pand      -80(%rbp), %xmm1                              #4384.22
..LN779:
   .loc    1  4385
        pcmpeqd   %xmm1, %xmm2                                  #4385.22
..LN781:
   .loc    1  4386
        pmovmskb  %xmm2, %edx                                   #4386.22
..LN783:
   .loc    1  4391
        movl      %eax, %r13d                                   #4391.17
        andl      $-4, %r13d                                    #4391.17
..LN785:
   .loc    1  4394
        movl      %ecx, %eax                                    #4394.29
        orl       %r14d, %eax                                   #4394.29
..LN787:
        orl       %r15d, %eax                                   #4394.34
..LN789:
        orl       %edx, %eax                                    #4394.39
..LN791:
        je        ..B7.24       # Prob 78%                      #4394.47
                                # LOE rsi r12 edx ecx ebx r13d r14d r15d xmm0
..B7.12:                        # Preds ..B7.11
..LN793:
   .loc    1  4412
        movdqa    %xmm0, -256(%rbp)                             #4412.30
        movl      %ecx, %edi                                    #4412.30
        movl      %edx, -216(%rbp)                              #4412.30
        movl      %ecx, -208(%rbp)                              #4412.30
        movq      %rsi, -200(%rbp)                              #4412.30
        call      uint16_lsbit@PLT                              #4412.30
                                # LOE r12 eax ebx r13d r14d r15d
..B7.47:                        # Preds ..B7.12
        movq      -200(%rbp), %rsi                              #
        movdqa    -256(%rbp), %xmm0                             #
        movl      -208(%rbp), %ecx                              #
        movl      -216(%rbp), %edx                              #
                                # LOE rdx rcx rsi r12 eax edx ecx ebx r13d r14d r15d dl cl dh ch xmm0
..B7.13:                        # Preds ..B7.47
..LN795:
   .loc    1  4413
        movdqa    %xmm0, -256(%rbp)                             #4413.30
        movl      %r14d, %edi                                   #4413.30
        movl      %edx, -216(%rbp)                              #4413.30
..LN797:
   .loc    1  4412
        movswq    %ax, %rax                                     #4412.30
        movl      %eax, -240(%rbp)                              #4412.30
..LN799:
   .loc    1  4413
        movl      %ecx, -208(%rbp)                              #4413.30
        movq      %rsi, -200(%rbp)                              #4413.30
        call      uint16_lsbit@PLT                              #4413.30
                                # LOE r12 eax ebx r13d r14d r15d
..B7.48:                        # Preds ..B7.13
        movq      -200(%rbp), %rsi                              #
        movdqa    -256(%rbp), %xmm0                             #
        movl      -208(%rbp), %ecx                              #
        movl      -216(%rbp), %edx                              #
                                # LOE rdx rcx rsi r12 eax edx ecx ebx r13d r14d r15d dl cl dh ch xmm0
..B7.14:                        # Preds ..B7.48
..LN801:
   .loc    1  4414
        movdqa    %xmm0, -256(%rbp)                             #4414.30
        movl      %r15d, %edi                                   #4414.30
        movl      %edx, -216(%rbp)                              #4414.30
..LN803:
   .loc    1  4413
        movswq    %ax, %rax                                     #4413.30
        movl      %eax, -224(%rbp)                              #4413.30
..LN805:
   .loc    1  4414
        movl      %ecx, -208(%rbp)                              #4414.30
        movq      %rsi, -200(%rbp)                              #4414.30
        call      uint16_lsbit@PLT                              #4414.30
                                # LOE r12 eax ebx r13d r14d r15d
..B7.49:                        # Preds ..B7.14
        movq      -200(%rbp), %rsi                              #
        movdqa    -256(%rbp), %xmm0                             #
        movl      -208(%rbp), %ecx                              #
        movl      -216(%rbp), %edx                              #
                                # LOE rdx rcx rsi r12 eax edx ecx ebx r13d r14d r15d dl cl dh ch xmm0
..B7.15:                        # Preds ..B7.49
..LN807:
   .loc    1  4415
        movdqa    %xmm0, -256(%rbp)                             #4415.30
        movl      %edx, %edi                                    #4415.30
        movl      %edx, -216(%rbp)                              #4415.30
..LN809:
   .loc    1  4414
        movswq    %ax, %rax                                     #4414.30
        movl      %eax, -232(%rbp)                              #4414.30
..LN811:
   .loc    1  4415
        movl      %ecx, -208(%rbp)                              #4415.30
        movq      %rsi, -200(%rbp)                              #4415.30
        call      uint16_lsbit@PLT                              #4415.30
                                # LOE r12 eax ebx r13d r14d r15d
..B7.50:                        # Preds ..B7.15
        movq      -200(%rbp), %rsi                              #
        movdqa    -256(%rbp), %xmm0                             #
        movl      -208(%rbp), %ecx                              #
        movl      -216(%rbp), %edx                              #
                                # LOE rdx rcx rsi r12 eax edx ecx ebx r13d r14d r15d dl cl dh ch xmm0
..B7.16:                        # Preds ..B7.50
..LN813:
   .loc    1  4422
        movl      -232(%rbp), %r9d                              #4422.40
..LN815:
   .loc    1  4415
        movswq    %ax, %r8                                      #4415.30
..LN817:
   .loc    1  4421
        movl      -224(%rbp), %eax                              #4421.40
..LN819:
   .loc    1  4415
..LN821:
   .loc    1  4421
        lea       1(%rax,%rax), %eax                            #4421.40
..LN823:
   .loc    1  4422
        lea       2(%r9,%r9), %r9d                              #4422.40
..LN825:
   .loc    1  4423
        lea       3(%r8,%r8), %r10d                             #4423.40
..LN827:
   .loc    1  4427
        testl     %ecx, %ecx                                    #4427.32
        je        ..B7.18       # Prob 50%                      #4427.32
                                # LOE rdx rsi r12 eax edx ebx r9d r10d r13d r14d r15d dl dh xmm0
..B7.17:                        # Preds ..B7.16
..LN829:
        movl      -240(%rbp), %ecx                              #4427.43
        lea       (%r13,%rcx,2), %r8d                           #4427.43
..LN831:
        cmpl      -8(%rbp), %r8d                                #4427.49
        jbe       ..B7.38       # Prob 20%                      #4427.49
                                # LOE rdx rsi r12 eax edx ebx r9d r10d r13d r14d r15d dl dh xmm0
..B7.18:                        # Preds ..B7.17 ..B7.16
..LN833:
   .loc    1  4428
        testl     %r14d, %r14d                                  #4428.32
        je        ..B7.20       # Prob 50%                      #4428.32
                                # LOE rdx rsi r12 eax edx ebx r9d r10d r13d r15d dl dh xmm0
..B7.19:                        # Preds ..B7.18
..LN835:
        addl      %r13d, %eax                                   #4428.43
..LN837:
        cmpl      -8(%rbp), %eax                                #4428.49
        jbe       ..B7.38       # Prob 20%                      #4428.49
                                # LOE rdx rsi r12 edx ebx r9d r10d r13d r15d dl dh xmm0
..B7.20:                        # Preds ..B7.19 ..B7.18
..LN839:
   .loc    1  4429
        testl     %r15d, %r15d                                  #4429.32
        je        ..B7.22       # Prob 50%                      #4429.32
                                # LOE rdx rsi r12 edx ebx r9d r10d r13d dl dh xmm0
..B7.21:                        # Preds ..B7.20
..LN841:
        addl      %r13d, %r9d                                   #4429.43
..LN843:
        cmpl      -8(%rbp), %r9d                                #4429.49
        jbe       ..B7.38       # Prob 20%                      #4429.49
                                # LOE rdx rsi r12 edx ebx r10d r13d dl dh xmm0
..B7.22:                        # Preds ..B7.21 ..B7.20
..LN845:
   .loc    1  4430
        testl     %edx, %edx                                    #4430.32
        je        ..B7.24       # Prob 50%                      #4430.32
                                # LOE rsi r12 ebx r10d r13d xmm0
..B7.23:                        # Preds ..B7.22
..LN847:
        addl      %r13d, %r10d                                  #4430.43
..LN849:
        cmpl      -8(%rbp), %r10d                               #4430.49
        jbe       ..B7.38       # Prob 20%                      #4430.49
                                # LOE rsi r12 ebx r13d xmm0
..B7.24:                        # Preds ..B7.23 ..B7.22 ..B7.11
..LN851:
   .loc    1  4435
        lea       4(%r13), %eax                                 #4435.17
..LN853:
   .loc    1  4438
        cmpl      -8(%rbp), %eax                                #4438.28
        ja        ..B7.35       # Prob 20%                      #4438.28
                                # LOE rsi r12 eax ebx r13d xmm0
..B7.25:                        # Preds ..B7.24
..LN855:
   .loc    1  4442
        decl      %ebx                                          #4442.25
..LN857:
        jne       ..B7.30       # Prob 50%                      #4442.39
                                # LOE rsi r12 eax ebx r13d xmm0
..B7.26:                        # Preds ..B7.25
..LN859:
   .loc    1  4459
        cmpq      -16(%rbp), %r12                               #4459.25
        jae       ..B7.35       # Prob 4%                       #4459.25
                                # LOE r12 r13d
..B7.27:                        # Preds ..B7.26
..LN861:
   .loc    1  4463
        lea       28(%r13), %eax                                #4463.13
..LN863:
   .loc    1  4464
        cmpl      -8(%rbp), %eax                                #4464.24
        ja        ..B7.35       # Prob 4%                       #4464.24
                                # LOE r12 eax
..B7.28:                        # Preds ..B7.27
..LN865:
   .loc    1  4469
        movq      -16(%rbp), %rsi                               #4469.22
        movl      %eax, -192(%rbp)                              #4469.22
        movq      %r12, %rdi                                    #4469.22
        call      prime_buffer_4na@PLT                          #4469.22
                                # LOE r12 xmm0
..B7.51:                        # Preds ..B7.28
        movl      -192(%rbp), %eax                              #
..LN867:
   .loc    1  4484
        addq      $8, %r12                                      #4484.13
        movq      %r12, %rsi                                    #4484.13
                                # LOE rsi r12 eax xmm0
..B7.29:                        # Preds ..B7.5 ..B7.51
..LN869:
   .loc    1  4352
        movl      $2, %ebx                                      #4352.13
        jmp       ..B7.33       # Prob 100%                     #4352.13
                                # LOE rsi r12 eax ebx xmm0
..B7.30:                        # Preds ..B7.25
..LN871:
   .loc    1  4450
        cmpq      -16(%rbp), %rsi                               #4450.26
..LN873:
   .loc    1  4447
        psrldq    $2, %xmm0                                     #4447.26
..LN875:
   .loc    1  4450
        jae       ..B7.32       # Prob 19%                      #4450.26
                                # LOE rsi r12 eax ebx xmm0
..B7.31:                        # Preds ..B7.30
..LN877:
   .loc    1  4451
        movzbl    (%rsi), %ecx                                  #4451.72
..LN879:
        lea       expand_2na.0(%rip), %rdx                      #4451.57
        movzwl    (%rdx,%rcx,2), %edi                           #4451.57
..LN881:
        pinsrw    $7, %edi, %xmm0                               #4451.30
                                # LOE rsi r12 eax ebx xmm0
..B7.32:                        # Preds ..B7.31 ..B7.30
..LN883:
   .loc    1  4454
        incq      %rsi                                          #4454.20
                                # LOE rsi r12 eax ebx xmm0
..B7.33:                        # Preds ..B7.29 ..B7.32 ..B7.37
..LN885:
   .loc    1  4362
        movdqa    %xmm0, %xmm2                                  #4362.22
..LN887:
   .loc    1  4363
        movdqa    %xmm0, %xmm1                                  #4363.22
..LN889:
   .loc    1  4362
        pand      -96(%rbp), %xmm2                              #4362.22
..LN891:
   .loc    1  4363
        pand      -144(%rbp), %xmm1                             #4363.22
..LN893:
   .loc    1  4364
        pcmpeqd   %xmm1, %xmm2                                  #4364.22
..LN895:
   .loc    1  4365
        pmovmskb  %xmm2, %ecx                                   #4365.22
        jmp       ..B7.9        # Prob 100%                     #4365.22
                                # LOE rsi r12 eax ecx ebx xmm0
..B7.35:                        # Preds ..B7.24 ..B7.26 ..B7.27 # Infreq
..LN897:
   .loc    1  4495
        movq      -40(%rbp), %rbx                               #4495.12
..___tag_value_eval_4na_32.365:                                 #
        movq      -48(%rbp), %r12                               #4495.12
..___tag_value_eval_4na_32.366:                                 #
        movq      -32(%rbp), %r13                               #4495.12
..___tag_value_eval_4na_32.367:                                 #
        movq      -24(%rbp), %r14                               #4495.12
..___tag_value_eval_4na_32.368:                                 #
        xorl      %eax, %eax                                    #4495.12
        movq      -184(%rbp), %r15                              #4495.12
..___tag_value_eval_4na_32.369:                                 #
        movq      %rbp, %rsp                                    #4495.12
        popq      %rbp                                          #4495.12
..___tag_value_eval_4na_32.370:                                 #
        ret                                                     #4495.12
..___tag_value_eval_4na_32.371:                                 #
                                # LOE
..B7.37:                        # Preds ..B7.46                 # Infreq
        movdqa    %xmm8, -80(%rbp)                              #
        movdqa    %xmm1, -64(%rbp)                              #
        movdqa    %xmm2, -176(%rbp)                             #
        movdqa    %xmm3, -128(%rbp)                             #
        movdqa    %xmm4, -112(%rbp)                             #
        movdqa    %xmm5, -160(%rbp)                             #
        movdqa    %xmm6, -144(%rbp)                             #
        movdqa    %xmm7, -96(%rbp)                              #
        movl      %eax, -8(%rbp)                                #
        movl      %r13d, %eax                                   #
        jmp       ..B7.33       # Prob 100%                     #
                                # LOE rsi r12 eax ebx xmm0
..B7.38:                        # Preds ..B7.17 ..B7.23 ..B7.21 ..B7.19 # Infreq
..LN899:
   .loc    1  4428
        movq      -40(%rbp), %rbx                               #4428.63
..___tag_value_eval_4na_32.377:                                 #
        movq      -48(%rbp), %r12                               #4428.63
..___tag_value_eval_4na_32.378:                                 #
        movq      -32(%rbp), %r13                               #4428.63
..___tag_value_eval_4na_32.379:                                 #
        movq      -24(%rbp), %r14                               #4428.63
..___tag_value_eval_4na_32.380:                                 #
        movl      $1, %eax                                      #4428.63
        movq      -184(%rbp), %r15                              #4428.63
..___tag_value_eval_4na_32.381:                                 #
        movq      %rbp, %rsp                                    #4428.63
        popq      %rbp                                          #4428.63
..___tag_value_eval_4na_32.382:                                 #
        ret                                                     #4428.63
        .align    16,0x90
..___tag_value_eval_4na_32.383:                                 #
                                # LOE
# mark_end;
	.type	eval_4na_32,@function
	.size	eval_4na_32,.-eval_4na_32
.LNeval_4na_32:
	.data
# -- End  eval_4na_32
	.text
# -- Begin  uint16_lsbit
# mark_begin;
       .align    16,0x90
uint16_lsbit:
# parameter 1(self): %edi
..B8.1:                         # Preds ..B8.0
..___tag_value_uint16_lsbit.384:                                #38.1
..LN901:
  .file   2 "/home/yaschenk/devel/internal/asm-trace/inc/gcc/x86_64/arch-impl.h"
   .loc    2  38
        pushq     %rbp                                          #38.1
        movq      %rsp, %rbp                                    #38.1
..___tag_value_uint16_lsbit.385:                                #
        subq      $16, %rsp                                     #38.1
        movq      %rbx, -16(%rbp)                               #38.1
..___tag_value_uint16_lsbit.387:                                #
        movl      %edi, %ebx                                    #38.1
        lea       _gprof_pack7(%rip), %rdx                      #38.1
        call      mcount@PLT                                    #38.1
                                # LOE r12 r13 r14 r15 ebx
..B8.4:                         # Preds ..B8.1
..LN903:
   .loc    2  40
        movl      %ebx, %eax                                    #40.0
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #40.0
..LN905:
   .loc    2  49
        movq      -16(%rbp), %rbx                               #49.12
..___tag_value_uint16_lsbit.388:                                #
        movq      %rbp, %rsp                                    #49.12
        popq      %rbp                                          #49.12
..___tag_value_uint16_lsbit.389:                                #
        ret                                                     #49.12
        .align    16,0x90
..___tag_value_uint16_lsbit.390:                                #
                                # LOE
# mark_end;
	.type	uint16_lsbit,@function
	.size	uint16_lsbit,.-uint16_lsbit
.LNuint16_lsbit:
	.data
# -- End  uint16_lsbit
	.text
# -- Begin  eval_4na_16
# mark_begin;
       .align    16,0x90
eval_4na_16:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B9.1:                         # Preds ..B9.0
..___tag_value_eval_4na_16.391:                                 #4057.1
..LN907:
   .loc    1  4057
        pushq     %rbp                                          #4057.1
        movq      %rsp, %rbp                                    #4057.1
..___tag_value_eval_4na_16.392:                                 #
        subq      $224, %rsp                                    #4057.1
        movq      %r15, -48(%rbp)                               #4057.1
..___tag_value_eval_4na_16.394:                                 #
        movq      %r14, -40(%rbp)                               #4057.1
..___tag_value_eval_4na_16.395:                                 #
        movq      %r13, -32(%rbp)                               #4057.1
..___tag_value_eval_4na_16.396:                                 #
        movq      %r12, -24(%rbp)                               #4057.1
..___tag_value_eval_4na_16.397:                                 #
        movq      %rbx, -56(%rbp)                               #4057.1
..___tag_value_eval_4na_16.398:                                 #
        movl      %ecx, %ebx                                    #4057.1
        movl      %edx, %r12d                                   #4057.1
        movq      %rsi, %r15                                    #4057.1
        movq      %rdi, %r13                                    #4057.1
        lea       _gprof_pack8(%rip), %rdx                      #4057.1
        call      mcount@PLT                                    #4057.1
                                # LOE r13 r15 ebx r12d
..B9.40:                        # Preds ..B9.1
..LN909:
   .loc    1  4090
        lea       (%r12,%rbx), %eax                             #4090.5
..LN911:
   .loc    1  4096
        subl      4(%r13), %eax                                 #4096.12
..LN913:
   .loc    1  4102
        movl      %eax, -224(%rbp)                              #4102.14
..LN915:
   .loc    1  4093
        movl      %r12d, %r14d                                  #4093.49
        shrl      $2, %r14d                                     #4093.49
..LN917:
        lea       (%r15,%r14), %rdi                             #4093.30
..LN919:
   .loc    1  4099
        lea       3(%r12,%rbx), %esi                            #4099.50
..LN921:
        shrl      $2, %esi                                      #4099.57
..LN923:
        addq      %r15, %rsi                                    #4099.30
        movq      %rsi, -16(%rbp)                               #4099.30
..LN925:
   .loc    1  4102
        call      prime_buffer_4na@PLT                          #4102.14
                                # LOE r13 r14 r15 r12d xmm0
..B9.41:                        # Preds ..B9.40
        movl      -224(%rbp), %eax                              #
..LN927:
   .loc    1  4109
        movdqa    16(%r13), %xmm7                               #4109.5
        movdqa    32(%r13), %xmm6                               #4109.5
        movdqa    48(%r13), %xmm5                               #4109.5
        movdqa    64(%r13), %xmm4                               #4109.5
        movdqa    80(%r13), %xmm3                               #4109.5
        movdqa    96(%r13), %xmm2                               #4109.5
        movdqa    112(%r13), %xmm1                              #4109.5
        movdqa    128(%r13), %xmm8                              #4109.5
..LN929:
   .loc    1  4103
        lea       8(%r15,%r14), %rbx                            #4103.5
..LN931:
   .loc    1  4112
        xorl      %r14d, %r14d                                  #4112.15
..LN933:
        xorl      %r15d, %r15d                                  #4112.10
..LN935:
        xorl      %edx, %edx                                    #4112.5
..LN937:
   .loc    1  4123
        movl      %r12d, %ecx                                   #4123.20
        andl      $3, %ecx                                      #4123.20
        je        ..B9.5        # Prob 20%                      #4123.20
                                # LOE rax rbx eax edx ecx r12d r14d r15d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.2:                         # Preds ..B9.41
        cmpl      $1, %ecx                                      #4123.20
        je        ..B9.8        # Prob 25%                      #4123.20
                                # LOE rax rbx eax edx ecx r12d r14d r15d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.3:                         # Preds ..B9.2
        cmpl      $2, %ecx                                      #4123.20
        je        ..B9.7        # Prob 33%                      #4123.20
                                # LOE rax rbx eax edx ecx r12d r14d r15d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.4:                         # Preds ..B9.3
        cmpl      $3, %ecx                                      #4123.20
        je        ..B9.6        # Prob 50%                      #4123.20
                                # LOE rax rbx eax edx r12d r14d r15d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.5:                         # Preds ..B9.41 ..B9.4
        movdqa    %xmm8, -80(%rbp)                              #
        movdqa    %xmm1, -144(%rbp)                             #
        movdqa    %xmm2, -160(%rbp)                             #
        movdqa    %xmm3, -112(%rbp)                             #
        movdqa    %xmm4, -96(%rbp)                              #
        movdqa    %xmm5, -128(%rbp)                             #
        movdqa    %xmm6, -192(%rbp)                             #
        movdqa    %xmm7, -176(%rbp)                             #
        movl      %eax, %r13d                                   #
        jmp       ..B9.28       # Prob 100%                     #
                                # LOE rbx r12d r13d xmm0
..B9.6:                         # Preds ..B9.4
        movdqa    %xmm8, -80(%rbp)                              #
        movdqa    %xmm1, -144(%rbp)                             #
        movdqa    %xmm2, -160(%rbp)                             #
        movdqa    %xmm3, -112(%rbp)                             #
        movdqa    %xmm4, -96(%rbp)                              #
        movdqa    %xmm5, -128(%rbp)                             #
        movdqa    %xmm6, -192(%rbp)                             #
        movdqa    %xmm7, -176(%rbp)                             #
        movl      %eax, %r13d                                   #
        jmp       ..B9.11       # Prob 100%                     #
                                # LOE rbx edx r12d r13d r14d r15d xmm0
..B9.7:                         # Preds ..B9.3
        movdqa    %xmm8, -80(%rbp)                              #
        movdqa    %xmm1, -144(%rbp)                             #
        movdqa    %xmm2, -160(%rbp)                             #
        movdqa    %xmm3, -112(%rbp)                             #
        movdqa    %xmm4, -96(%rbp)                              #
        movdqa    %xmm5, -128(%rbp)                             #
        movdqa    %xmm6, -192(%rbp)                             #
        movdqa    %xmm7, -176(%rbp)                             #
        movl      %eax, %r13d                                   #
        jmp       ..B9.10       # Prob 100%                     #
                                # LOE rbx edx r12d r13d r15d xmm0
..B9.8:                         # Preds ..B9.2
        movdqa    %xmm8, -80(%rbp)                              #
        movdqa    %xmm1, -144(%rbp)                             #
        movdqa    %xmm2, -160(%rbp)                             #
        movdqa    %xmm3, -112(%rbp)                             #
        movdqa    %xmm4, -96(%rbp)                              #
        movdqa    %xmm5, -128(%rbp)                             #
        movdqa    %xmm6, -192(%rbp)                             #
        movdqa    %xmm7, -176(%rbp)                             #
        movl      %eax, %r13d                                   #
                                # LOE rbx edx r12d r13d xmm0
..B9.9:                         # Preds ..B9.8 ..B9.28
..LN939:
   .loc    1  4147
        movdqa    %xmm0, %xmm2                                  #4147.22
..LN941:
   .loc    1  4148
        movdqa    %xmm0, %xmm1                                  #4148.22
..LN943:
   .loc    1  4147
        pand      -128(%rbp), %xmm2                             #4147.22
..LN945:
   .loc    1  4148
        pand      -96(%rbp), %xmm1                              #4148.22
..LN947:
   .loc    1  4149
        pcmpeqw   %xmm1, %xmm2                                  #4149.22
..LN949:
   .loc    1  4150
        pmovmskb  %xmm2, %r15d                                  #4150.22
                                # LOE rbx edx r12d r13d r15d xmm0
..B9.10:                        # Preds ..B9.7 ..B9.9
..LN951:
   .loc    1  4154
        movdqa    %xmm0, %xmm2                                  #4154.22
..LN953:
   .loc    1  4155
        movdqa    %xmm0, %xmm1                                  #4155.22
..LN955:
   .loc    1  4154
        pand      -112(%rbp), %xmm2                             #4154.22
..LN957:
   .loc    1  4155
        pand      -160(%rbp), %xmm1                             #4155.22
..LN959:
   .loc    1  4156
        pcmpeqw   %xmm1, %xmm2                                  #4156.22
..LN961:
   .loc    1  4157
        pmovmskb  %xmm2, %r14d                                  #4157.22
                                # LOE rbx edx r12d r13d r14d r15d xmm0
..B9.11:                        # Preds ..B9.6 ..B9.10
..LN963:
   .loc    1  4161
        movdqa    %xmm0, %xmm1                                  #4161.22
..LN965:
   .loc    1  4162
        pand      -80(%rbp), %xmm0                              #4162.22
..LN967:
   .loc    1  4161
        pand      -144(%rbp), %xmm1                             #4161.22
..LN969:
   .loc    1  4163
        pcmpeqw   %xmm0, %xmm1                                  #4163.22
..LN971:
   .loc    1  4164
        pmovmskb  %xmm1, %ecx                                   #4164.22
        movl      %ecx, -64(%rbp)                               #4164.22
..LN973:
   .loc    1  4169
        andl      $-4, %r12d                                    #4169.17
..LN975:
   .loc    1  4172
        movl      %edx, %esi                                    #4172.29
        orl       %r15d, %esi                                   #4172.29
..LN977:
        orl       %r14d, %esi                                   #4172.34
..LN979:
        orl       %ecx, %esi                                    #4172.39
..LN981:
        je        ..B9.24       # Prob 78%                      #4172.47
                                # LOE rbx edx r12d r13d r14d r15d
..B9.12:                        # Preds ..B9.11
..LN983:
   .loc    1  4190
        movl      %edx, -200(%rbp)                              #4190.30
        movl      %edx, %edi                                    #4190.30
        call      uint16_lsbit@PLT                              #4190.30
                                # LOE rbx eax r12d r13d r14d r15d
..B9.42:                        # Preds ..B9.12
        movl      -200(%rbp), %edx                              #
                                # LOE rdx rbx eax edx r12d r13d r14d r15d dl dh
..B9.13:                        # Preds ..B9.42
..LN985:
   .loc    1  4191
        movl      %edx, -200(%rbp)                              #4191.30
..LN987:
   .loc    1  4190
        movswq    %ax, %rcx                                     #4190.30
        movl      %ecx, -224(%rbp)                              #4190.30
..LN989:
   .loc    1  4191
        movl      %r15d, %edi                                   #4191.30
        call      uint16_lsbit@PLT                              #4191.30
                                # LOE rbx eax r12d r13d r14d r15d
..B9.43:                        # Preds ..B9.13
        movl      -200(%rbp), %edx                              #
                                # LOE rdx rbx eax edx r12d r13d r14d r15d dl dh
..B9.14:                        # Preds ..B9.43
..LN991:
   .loc    1  4192
        movl      %edx, -200(%rbp)                              #4192.30
..LN993:
   .loc    1  4191
        movswq    %ax, %rcx                                     #4191.30
        movl      %ecx, -216(%rbp)                              #4191.30
..LN995:
   .loc    1  4192
        movl      %r14d, %edi                                   #4192.30
        call      uint16_lsbit@PLT                              #4192.30
                                # LOE rbx eax r12d r13d r14d r15d
..B9.44:                        # Preds ..B9.14
        movl      -200(%rbp), %edx                              #
                                # LOE rdx rbx eax edx r12d r13d r14d r15d dl dh
..B9.15:                        # Preds ..B9.44
..LN997:
   .loc    1  4193
        movl      -64(%rbp), %edi                               #4193.30
        movl      %edx, -200(%rbp)                              #4193.30
..LN999:
   .loc    1  4192
        movswq    %ax, %rcx                                     #4192.30
        movl      %ecx, -208(%rbp)                              #4192.30
..LN1001:
   .loc    1  4193
        call      uint16_lsbit@PLT                              #4193.30
                                # LOE rbx eax r12d r13d r14d r15d
..B9.45:                        # Preds ..B9.15
        movl      -200(%rbp), %edx                              #
                                # LOE rdx rbx eax edx r12d r13d r14d r15d dl dh
..B9.16:                        # Preds ..B9.45
..LN1003:
   .loc    1  4199
        movl      -216(%rbp), %esi                              #4199.40
..LN1005:
   .loc    1  4200
        movl      -208(%rbp), %r8d                              #4200.40
..LN1007:
   .loc    1  4193
        movswq    %ax, %rcx                                     #4193.30
..LN1009:
   .loc    1  4199
        lea       1(%rsi,%rsi), %esi                            #4199.40
..LN1011:
   .loc    1  4200
        lea       2(%r8,%r8), %r8d                              #4200.40
..LN1013:
   .loc    1  4201
        lea       3(%rcx,%rcx), %r9d                            #4201.40
..LN1015:
   .loc    1  4205
        testl     %edx, %edx                                    #4205.32
        je        ..B9.18       # Prob 50%                      #4205.32
                                # LOE rbx esi r8d r9d r12d r13d r14d r15d
..B9.17:                        # Preds ..B9.16
..LN1017:
        movl      -224(%rbp), %edx                              #4205.43
        lea       (%r12,%rdx,2), %ecx                           #4205.43
..LN1019:
        cmpl      %ecx, %r13d                                   #4205.49
        jae       ..B9.33       # Prob 20%                      #4205.49
                                # LOE rbx esi r8d r9d r12d r13d r14d r15d
..B9.18:                        # Preds ..B9.17 ..B9.16
..LN1021:
   .loc    1  4206
        testl     %r15d, %r15d                                  #4206.32
        je        ..B9.20       # Prob 50%                      #4206.32
                                # LOE rbx esi r8d r9d r12d r13d r14d
..B9.19:                        # Preds ..B9.18
..LN1023:
        addl      %r12d, %esi                                   #4206.43
..LN1025:
        cmpl      %esi, %r13d                                   #4206.49
        jae       ..B9.33       # Prob 20%                      #4206.49
                                # LOE rbx r8d r9d r12d r13d r14d
..B9.20:                        # Preds ..B9.19 ..B9.18
..LN1027:
   .loc    1  4207
        testl     %r14d, %r14d                                  #4207.32
        je        ..B9.22       # Prob 50%                      #4207.32
                                # LOE rbx r8d r9d r12d r13d
..B9.21:                        # Preds ..B9.20
..LN1029:
        addl      %r12d, %r8d                                   #4207.43
..LN1031:
        cmpl      %r8d, %r13d                                   #4207.49
        jae       ..B9.33       # Prob 20%                      #4207.49
                                # LOE rbx r9d r12d r13d
..B9.22:                        # Preds ..B9.21 ..B9.20
..LN1033:
   .loc    1  4208
        movl      -64(%rbp), %edx                               #4208.32
        testl     %edx, %edx                                    #4208.32
        je        ..B9.24       # Prob 50%                      #4208.32
                                # LOE rbx r9d r12d r13d
..B9.23:                        # Preds ..B9.22
..LN1035:
        addl      %r12d, %r9d                                   #4208.43
..LN1037:
        cmpl      %r9d, %r13d                                   #4208.49
        jae       ..B9.33       # Prob 20%                      #4208.49
                                # LOE rbx r12d r13d
..B9.24:                        # Preds ..B9.23 ..B9.22 ..B9.11
..LN1039:
   .loc    1  4241
        lea       4(%r12), %edx                                 #4241.13
..LN1041:
   .loc    1  4216
        cmpl      %edx, %r13d                                   #4216.28
        jb        ..B9.30       # Prob 20%                      #4216.28
                                # LOE rbx r12d r13d
..B9.25:                        # Preds ..B9.24
..LN1043:
   .loc    1  4237
        cmpq      -16(%rbp), %rbx                               #4237.25
        jae       ..B9.30       # Prob 4%                       #4237.25
                                # LOE rbx r12d r13d
..B9.26:                        # Preds ..B9.25
..LN1045:
   .loc    1  4241
        addl      $32, %r12d                                    #4241.13
..LN1047:
   .loc    1  4242
        cmpl      %r13d, %r12d                                  #4242.24
        ja        ..B9.30       # Prob 4%                       #4242.24
                                # LOE rbx r12d r13d
..B9.27:                        # Preds ..B9.26
..LN1049:
   .loc    1  4247
        movq      -16(%rbp), %rsi                               #4247.22
        movq      %rbx, %rdi                                    #4247.22
        call      prime_buffer_4na@PLT                          #4247.22
                                # LOE rbx r12d r13d xmm0
..B9.46:                        # Preds ..B9.27
..LN1051:
   .loc    1  4262
        addq      $8, %rbx                                      #4262.13
                                # LOE rbx r12d r13d xmm0
..B9.28:                        # Preds ..B9.5 ..B9.46
..LN1053:
   .loc    1  4140
        movdqa    %xmm0, %xmm2                                  #4140.22
..LN1055:
   .loc    1  4141
        movdqa    %xmm0, %xmm1                                  #4141.22
..LN1057:
   .loc    1  4140
        pand      -176(%rbp), %xmm2                             #4140.22
..LN1059:
   .loc    1  4141
        pand      -192(%rbp), %xmm1                             #4141.22
..LN1061:
   .loc    1  4142
        pcmpeqw   %xmm1, %xmm2                                  #4142.22
..LN1063:
   .loc    1  4143
        pmovmskb  %xmm2, %edx                                   #4143.22
        jmp       ..B9.9        # Prob 100%                     #4143.22
                                # LOE rbx edx r12d r13d xmm0
..B9.30:                        # Preds ..B9.24 ..B9.25 ..B9.26 # Infreq
..LN1065:
   .loc    1  4273
        movq      -56(%rbp), %rbx                               #4273.12
..___tag_value_eval_4na_16.399:                                 #
        movq      -24(%rbp), %r12                               #4273.12
..___tag_value_eval_4na_16.400:                                 #
        movq      -32(%rbp), %r13                               #4273.12
..___tag_value_eval_4na_16.401:                                 #
        movq      -40(%rbp), %r14                               #4273.12
..___tag_value_eval_4na_16.402:                                 #
        xorl      %eax, %eax                                    #4273.12
        movq      -48(%rbp), %r15                               #4273.12
..___tag_value_eval_4na_16.403:                                 #
        movq      %rbp, %rsp                                    #4273.12
        popq      %rbp                                          #4273.12
..___tag_value_eval_4na_16.404:                                 #
        ret                                                     #4273.12
..___tag_value_eval_4na_16.405:                                 #
                                # LOE
..B9.33:                        # Preds ..B9.23 ..B9.21 ..B9.19 ..B9.17 # Infreq
..LN1067:
   .loc    1  4205
        movq      -56(%rbp), %rbx                               #4205.63
..___tag_value_eval_4na_16.411:                                 #
        movq      -24(%rbp), %r12                               #4205.63
..___tag_value_eval_4na_16.412:                                 #
        movq      -32(%rbp), %r13                               #4205.63
..___tag_value_eval_4na_16.413:                                 #
        movq      -40(%rbp), %r14                               #4205.63
..___tag_value_eval_4na_16.414:                                 #
        movl      $1, %eax                                      #4205.63
        movq      -48(%rbp), %r15                               #4205.63
..___tag_value_eval_4na_16.415:                                 #
        movq      %rbp, %rsp                                    #4205.63
        popq      %rbp                                          #4205.63
..___tag_value_eval_4na_16.416:                                 #
        ret                                                     #4205.63
        .align    16,0x90
..___tag_value_eval_4na_16.417:                                 #
                                # LOE
# mark_end;
	.type	eval_4na_16,@function
	.size	eval_4na_16,.-eval_4na_16
.LNeval_4na_16:
	.data
# -- End  eval_4na_16
	.text
# -- Begin  eval_2na_128
# mark_begin;
       .align    16,0x90
eval_2na_128:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B10.1:                        # Preds ..B10.0
..___tag_value_eval_2na_128.418:                                #3369.1
..LN1069:
   .loc    1  3369
        pushq     %rbp                                          #3369.1
        movq      %rsp, %rbp                                    #3369.1
..___tag_value_eval_2na_128.419:                                #
        subq      $176, %rsp                                    #3369.1
        movq      %r15, -40(%rbp)                               #3369.1
..___tag_value_eval_2na_128.421:                                #
        movq      %r14, -24(%rbp)                               #3369.1
..___tag_value_eval_2na_128.422:                                #
        movq      %r13, -32(%rbp)                               #3369.1
..___tag_value_eval_2na_128.423:                                #
        movq      %r12, -16(%rbp)                               #3369.1
..___tag_value_eval_2na_128.424:                                #
        movq      %rbx, -48(%rbp)                               #3369.1
..___tag_value_eval_2na_128.425:                                #
        movl      %ecx, %r13d                                   #3369.1
        movl      %edx, %r12d                                   #3369.1
        movq      %rsi, %rbx                                    #3369.1
        movq      %rdi, %r15                                    #3369.1
        lea       _gprof_pack9(%rip), %rdx                      #3369.1
        call      mcount@PLT                                    #3369.1
                                # LOE rbx r15 r12d r13d
..B10.40:                       # Preds ..B10.1
..LN1071:
   .loc    1  3406
        lea       (%r12,%r13), %r14d                            #3406.5
..LN1073:
   .loc    1  3412
        subl      4(%r15), %r14d                                #3412.12
..LN1075:
   .loc    1  3409
        movl      %r12d, %eax                                   #3409.49
        shrl      $2, %eax                                      #3409.49
..LN1077:
        movq      %rax, -176(%rbp)                              #3409.30
        lea       (%rbx,%rax), %rdi                             #3409.30
..LN1079:
   .loc    1  3415
        lea       3(%r12,%r13), %r13d                           #3415.50
..LN1081:
        shrl      $2, %r13d                                     #3415.57
..LN1083:
        addq      %rbx, %r13                                    #3415.30
..LN1085:
   .loc    1  3418
        movq      %r13, %rsi                                    #3418.14
        call      prime_buffer_2na@PLT                          #3418.14
                                # LOE rbx r13 r15 r12d r14d xmm0
..B10.41:                       # Preds ..B10.40
..LN1087:
   .loc    1  3419
        movq      -176(%rbp), %rax                              #3419.5
        lea       16(%rbx,%rax), %r8                            #3419.5
        movq      %r8, %rdi                                     #3419.5
..LN1089:
   .loc    1  3424
        cmpq      %r13, %r8                                     #3424.16
..LN1091:
   .loc    1  3429
..LN1093:
   .loc    1  3424
        jae       ..B10.3       # Prob 12%                      #3424.16
                                # LOE rdi r8 r13 r15 ebx r12d r14d xmm0
..B10.2:                        # Preds ..B10.41
..LN1095:
   .loc    1  3425
        movzbl    -1(%r8), %ebx                                 #3425.24
..LN1097:
        shll      $8, %ebx                                      #3425.38
                                # LOE rdi r8 r13 r15 ebx r12d r14d xmm0
..B10.3:                        # Preds ..B10.2 ..B10.41
..LN1099:
   .loc    1  3429
        movdqa    16(%r15), %xmm8                               #3429.5
        movdqa    32(%r15), %xmm7                               #3429.5
        movdqa    48(%r15), %xmm6                               #3429.5
        movdqa    64(%r15), %xmm5                               #3429.5
        movdqa    80(%r15), %xmm4                               #3429.5
        movdqa    96(%r15), %xmm3                               #3429.5
        movdqa    112(%r15), %xmm2                              #3429.5
        movdqa    128(%r15), %xmm1                              #3429.5
..LN1101:
   .loc    1  3432
        xorl      %ecx, %ecx                                    #3432.15
..LN1103:
        xorl      %esi, %esi                                    #3432.10
..LN1105:
        xorl      %edx, %edx                                    #3432.5
..LN1107:
   .loc    1  3434
        movl      %r14d, %eax                                   #3434.20
        subl      %r12d, %eax                                   #3434.20
..LN1109:
        addl      $7, %eax                                      #3434.33
..LN1111:
        shrl      $2, %eax                                      #3434.40
..LN1113:
   .loc    1  3441
        movl      %r12d, %r9d                                   #3441.20
        andl      $3, %r9d                                      #3441.20
        je        ..B10.35      # Prob 20%                      #3441.20
                                # LOE rdi r8 r13 eax edx ecx ebx esi r9d r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.4:                        # Preds ..B10.3
        cmpl      $1, %r9d                                      #3441.20
        je        ..B10.10      # Prob 25%                      #3441.20
                                # LOE rdi r8 r13 eax edx ecx ebx esi r9d r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.5:                        # Preds ..B10.4
        cmpl      $2, %r9d                                      #3441.20
        je        ..B10.9       # Prob 33%                      #3441.20
                                # LOE rdi r8 r13 eax edx ecx ebx esi r9d r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.6:                        # Preds ..B10.5
        cmpl      $3, %r9d                                      #3441.20
        je        ..B10.8       # Prob 50%                      #3441.20
                                # LOE rdi r8 r13 eax edx ecx ebx esi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.7:                        # Preds ..B10.6
        movq      %r13, %r15                                    #
        movq      %r8, %r13                                     #
        jmp       ..B10.19      # Prob 100%                     #
                                # LOE rdi r13 r15 ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.8:                        # Preds ..B10.6
        movq      %r13, %r15                                    #
        movq      %r8, %r13                                     #
        jmp       ..B10.13      # Prob 100%                     #
                                # LOE rdi r13 r15 eax edx ecx ebx esi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.9:                        # Preds ..B10.5
        movq      %r13, %r15                                    #
        movq      %r8, %r13                                     #
        jmp       ..B10.12      # Prob 100%                     #
                                # LOE rdi r13 r15 eax edx ebx esi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.10:                       # Preds ..B10.4
        movq      %r13, %r15                                    #
        movq      %r8, %r13                                     #
                                # LOE rdi r13 r15 eax edx ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.11:                       # Preds ..B10.10 ..B10.23
..LN1115:
   .loc    1  3464
        movdqa    %xmm0, %xmm9                                  #3464.22
        pand      %xmm5, %xmm9                                  #3464.22
..LN1117:
   .loc    1  3465
        pcmpeqd   %xmm6, %xmm9                                  #3465.22
..LN1119:
   .loc    1  3466
        pmovmskb  %xmm9, %esi                                   #3466.22
..LN1121:
   .loc    1  3467
        incl      %esi                                          #3467.17
        shrl      $16, %esi                                     #3467.17
        negl      %esi                                          #3467.17
                                # LOE rdi r13 r15 eax edx ebx esi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.12:                       # Preds ..B10.9 ..B10.11
..LN1123:
   .loc    1  3470
        movdqa    %xmm0, %xmm9                                  #3470.22
        pand      %xmm3, %xmm9                                  #3470.22
..LN1125:
   .loc    1  3471
        pcmpeqd   %xmm4, %xmm9                                  #3471.22
..LN1127:
   .loc    1  3472
        pmovmskb  %xmm9, %ecx                                   #3472.22
..LN1129:
   .loc    1  3473
        incl      %ecx                                          #3473.17
        shrl      $16, %ecx                                     #3473.17
        negl      %ecx                                          #3473.17
                                # LOE rdi r13 r15 eax edx ecx ebx esi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.13:                       # Preds ..B10.8 ..B10.12
..LN1131:
   .loc    1  3476
        movdqa    %xmm0, %xmm9                                  #3476.22
..LN1133:
   .loc    1  3483
        andl      $-4, %r12d                                    #3483.17
..LN1135:
   .loc    1  3486
        movl      %edx, %r10d                                   #3486.29
..LN1137:
   .loc    1  3476
        pand      %xmm1, %xmm9                                  #3476.22
..LN1139:
   .loc    1  3477
        pcmpeqd   %xmm2, %xmm9                                  #3477.22
..LN1141:
   .loc    1  3478
        pmovmskb  %xmm9, %r9d                                   #3478.22
..LN1143:
   .loc    1  3486
        orl       %esi, %r10d                                   #3486.29
..LN1145:
        orl       %ecx, %r10d                                   #3486.34
..LN1147:
   .loc    1  3479
        incl      %r9d                                          #3479.17
        shrl      $16, %r9d                                     #3479.17
        negl      %r9d                                          #3479.17
..LN1149:
   .loc    1  3486
        orl       %r9d, %r10d                                   #3486.39
..LN1151:
        jne       ..B10.24      # Prob 20%                      #3486.47
                                # LOE rdi r13 r15 eax edx ecx ebx esi r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.14:                       # Preds ..B10.13
..LN1153:
   .loc    1  3527
        addl      $4, %r12d                                     #3527.17
..LN1155:
   .loc    1  3530
        cmpl      %r14d, %r12d                                  #3530.28
        ja        ..B10.32      # Prob 20%                      #3530.28
                                # LOE rdi r13 r15 eax ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.15:                       # Preds ..B10.14
..LN1157:
   .loc    1  3534
        decl      %eax                                          #3534.25
..LN1159:
        jne       ..B10.20      # Prob 50%                      #3534.39
                                # LOE rdi r13 r15 eax ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.16:                       # Preds ..B10.15
..LN1161:
   .loc    1  3555
        cmpq      %r15, %r13                                    #3555.25
        jae       ..B10.32      # Prob 4%                       #3555.25
                                # LOE r13 r15 ebx r12d r14d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.17:                       # Preds ..B10.16
..LN1163:
   .loc    1  3565
        movdqa    %xmm1, -176(%rbp)                             #3565.22
        movdqa    %xmm2, -160(%rbp)                             #3565.22
        movdqa    %xmm3, -144(%rbp)                             #3565.22
        movdqa    %xmm4, -128(%rbp)                             #3565.22
        movdqa    %xmm5, -112(%rbp)                             #3565.22
        movdqa    %xmm6, -96(%rbp)                              #3565.22
        movdqa    %xmm7, -80(%rbp)                              #3565.22
        movdqa    %xmm8, -64(%rbp)                              #3565.22
        movq      %r13, %rdi                                    #3565.22
        movq      %r15, %rsi                                    #3565.22
        call      prime_buffer_2na@PLT                          #3565.22
                                # LOE r13 r15 ebx r12d r14d xmm0
..B10.42:                       # Preds ..B10.17
        movdqa    -64(%rbp), %xmm8                              #
        movdqa    -80(%rbp), %xmm7                              #
        movdqa    -96(%rbp), %xmm6                              #
        movdqa    -112(%rbp), %xmm5                             #
..LN1165:
   .loc    1  3643
        addq      $16, %r13                                     #3643.13
        movq      %r13, %rdi                                    #3643.13
        movdqa    -128(%rbp), %xmm4                             #
        movdqa    -144(%rbp), %xmm3                             #
        movdqa    -160(%rbp), %xmm2                             #
        movdqa    -176(%rbp), %xmm1                             #
..LN1167:
   .loc    1  3646
        cmpq      %r15, %r13                                    #3646.24
        jae       ..B10.19      # Prob 12%                      #3646.24
                                # LOE rdi r13 r15 ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.18:                       # Preds ..B10.42
..LN1169:
   .loc    1  3647
        movzbl    -1(%r13), %ebx                                #3647.32
..LN1171:
        shll      $8, %ebx                                      #3647.46
                                # LOE rdi r13 r15 ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.19:                       # Preds ..B10.42 ..B10.7 ..B10.18
..LN1173:
   .loc    1  3448
        movl      $16, %eax                                     #3448.13
        jmp       ..B10.23      # Prob 100%                     #3448.13
                                # LOE rdi r13 r15 eax ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.20:                       # Preds ..B10.15
..LN1175:
   .loc    1  3539
        psrldq    $1, %xmm0                                     #3539.26
..LN1177:
   .loc    1  3542
        cmpq      %r15, %rdi                                    #3542.26
        jae       ..B10.22      # Prob 19%                      #3542.26
                                # LOE rdi r13 r15 eax ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.21:                       # Preds ..B10.20
..LN1179:
   .loc    1  3545
        movzbl    (%rdi), %edx                                  #3545.37
..LN1181:
   .loc    1  3544
        sarl      $8, %ebx                                      #3544.21
..LN1183:
   .loc    1  3545
        shll      $8, %edx                                      #3545.48
..LN1185:
        orl       %edx, %ebx                                    #3545.21
..LN1187:
   .loc    1  3546
        pinsrw    $7, %ebx, %xmm0                               #3546.30
                                # LOE rdi r13 r15 eax ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.22:                       # Preds ..B10.21 ..B10.20
..LN1189:
   .loc    1  3550
        incq      %rdi                                          #3550.20
                                # LOE rdi r13 r15 eax ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.23:                       # Preds ..B10.19 ..B10.22 ..B10.35
..LN1191:
   .loc    1  3458
        movdqa    %xmm0, %xmm9                                  #3458.22
        pand      %xmm7, %xmm9                                  #3458.22
..LN1193:
   .loc    1  3459
        pcmpeqd   %xmm8, %xmm9                                  #3459.22
..LN1195:
   .loc    1  3460
        pmovmskb  %xmm9, %edx                                   #3460.22
..LN1197:
   .loc    1  3461
        incl      %edx                                          #3461.17
        shrl      $16, %edx                                     #3461.17
        negl      %edx                                          #3461.17
        jmp       ..B10.11      # Prob 100%                     #3461.17
                                # LOE rdi r13 r15 eax edx ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.24:                       # Preds ..B10.13                # Infreq
..LN1199:
   .loc    1  3489
        subl      %r12d, %r14d                                  #3489.30
..LN1201:
        cmpl      $2, %r14d                                     #3489.37
        je        ..B10.28      # Prob 25%                      #3489.37
                                # LOE edx ecx esi r14d
..B10.25:                       # Preds ..B10.24                # Infreq
        cmpl      $1, %r14d                                     #3489.37
        je        ..B10.29      # Prob 33%                      #3489.37
                                # LOE edx esi r14d
..B10.26:                       # Preds ..B10.25                # Infreq
        testl     %r14d, %r14d                                  #3489.37
        je        ..B10.30      # Prob 50%                      #3489.37
                                # LOE edx
..B10.27:                       # Preds ..B10.26                # Infreq
..LN1203:
   .loc    1  3492
        movq      -48(%rbp), %rbx                               #3492.32
..___tag_value_eval_2na_128.426:                                #
        movq      -16(%rbp), %r12                               #3492.32
..___tag_value_eval_2na_128.427:                                #
        movq      -32(%rbp), %r13                               #3492.32
..___tag_value_eval_2na_128.428:                                #
        movq      -24(%rbp), %r14                               #3492.32
..___tag_value_eval_2na_128.429:                                #
        movl      $1, %eax                                      #3492.32
        movq      -40(%rbp), %r15                               #3492.32
..___tag_value_eval_2na_128.430:                                #
        movq      %rbp, %rsp                                    #3492.32
        popq      %rbp                                          #3492.32
..___tag_value_eval_2na_128.431:                                #
        ret                                                     #3492.32
..___tag_value_eval_2na_128.432:                                #
                                # LOE
..B10.28:                       # Preds ..B10.24                # Infreq
..LN1205:
   .loc    1  3494
        testl     %ecx, %ecx                                    #3494.36
        jne       ..B10.31      # Prob 28%                      #3494.36
                                # LOE edx esi
..B10.29:                       # Preds ..B10.28 ..B10.25       # Infreq
..LN1207:
   .loc    1  3496
        testl     %esi, %esi                                    #3496.36
        jne       ..B10.31      # Prob 28%                      #3496.36
                                # LOE edx
..B10.30:                       # Preds ..B10.29 ..B10.26       # Infreq
..LN1209:
   .loc    1  3498
        testl     %edx, %edx                                    #3498.36
        je        ..B10.32      # Prob 50%                      #3498.36
                                # LOE
..B10.31:                       # Preds ..B10.28 ..B10.29 ..B10.30 # Infreq
..LN1211:
        movq      -48(%rbp), %rbx                               #3498.47
..___tag_value_eval_2na_128.438:                                #
        movq      -16(%rbp), %r12                               #3498.47
..___tag_value_eval_2na_128.439:                                #
        movq      -32(%rbp), %r13                               #3498.47
..___tag_value_eval_2na_128.440:                                #
        movq      -24(%rbp), %r14                               #3498.47
..___tag_value_eval_2na_128.441:                                #
        movl      $1, %eax                                      #3498.47
        movq      -40(%rbp), %r15                               #3498.47
..___tag_value_eval_2na_128.442:                                #
        movq      %rbp, %rsp                                    #3498.47
        popq      %rbp                                          #3498.47
..___tag_value_eval_2na_128.443:                                #
        ret                                                     #3498.47
..___tag_value_eval_2na_128.444:                                #
                                # LOE
..B10.32:                       # Preds ..B10.14 ..B10.16 ..B10.30 # Infreq
..LN1213:
   .loc    1  3500
        movq      -48(%rbp), %rbx                               #3500.28
..___tag_value_eval_2na_128.450:                                #
        movq      -16(%rbp), %r12                               #3500.28
..___tag_value_eval_2na_128.451:                                #
        movq      -32(%rbp), %r13                               #3500.28
..___tag_value_eval_2na_128.452:                                #
        movq      -24(%rbp), %r14                               #3500.28
..___tag_value_eval_2na_128.453:                                #
        xorl      %eax, %eax                                    #3500.28
        movq      -40(%rbp), %r15                               #3500.28
..___tag_value_eval_2na_128.454:                                #
        movq      %rbp, %rsp                                    #3500.28
        popq      %rbp                                          #3500.28
..___tag_value_eval_2na_128.455:                                #
        ret                                                     #3500.28
..___tag_value_eval_2na_128.456:                                #
                                # LOE
..B10.35:                       # Preds ..B10.3                 # Infreq
        movq      %r13, %r15                                    #
        movq      %r8, %r13                                     #
        jmp       ..B10.23      # Prob 100%                     #
        .align    16,0x90
..___tag_value_eval_2na_128.462:                                #
                                # LOE rdi r13 r15 eax ebx r12d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
# mark_end;
	.type	eval_2na_128,@function
	.size	eval_2na_128,.-eval_2na_128
.LNeval_2na_128:
	.data
# -- End  eval_2na_128
	.text
# -- Begin  eval_2na_32
# mark_begin;
       .align    16,0x90
eval_2na_32:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B11.1:                        # Preds ..B11.0
..___tag_value_eval_2na_32.463:                                 #2782.1
..LN1215:
   .loc    1  2782
        pushq     %rbp                                          #2782.1
        movq      %rsp, %rbp                                    #2782.1
..___tag_value_eval_2na_32.464:                                 #
        subq      $272, %rsp                                    #2782.1
        movq      %r15, -56(%rbp)                               #2782.1
..___tag_value_eval_2na_32.466:                                 #
        movq      %r14, -48(%rbp)                               #2782.1
..___tag_value_eval_2na_32.467:                                 #
        movq      %r13, -24(%rbp)                               #2782.1
..___tag_value_eval_2na_32.468:                                 #
        movq      %r12, -40(%rbp)                               #2782.1
..___tag_value_eval_2na_32.469:                                 #
        movq      %rbx, -32(%rbp)                               #2782.1
..___tag_value_eval_2na_32.470:                                 #
        movl      %ecx, %r12d                                   #2782.1
        movl      %edx, %ebx                                    #2782.1
        movq      %rsi, %r14                                    #2782.1
        movq      %rdi, %r13                                    #2782.1
        lea       _gprof_pack10(%rip), %rdx                     #2782.1
        call      mcount@PLT                                    #2782.1
                                # LOE r13 r14 ebx r12d
..B11.48:                       # Preds ..B11.1
..LN1217:
   .loc    1  2819
        lea       (%rbx,%r12), %eax                             #2819.5
..LN1219:
   .loc    1  2825
        subl      4(%r13), %eax                                 #2825.12
..LN1221:
   .loc    1  2831
        movl      %eax, -16(%rbp)                               #2831.14
..LN1223:
   .loc    1  2822
        movl      %ebx, %r15d                                   #2822.49
        shrl      $2, %r15d                                     #2822.49
..LN1225:
        lea       (%r14,%r15), %rdi                             #2822.30
..LN1227:
   .loc    1  2828
        lea       3(%rbx,%r12), %r12d                           #2828.50
..LN1229:
        shrl      $2, %r12d                                     #2828.57
..LN1231:
        addq      %r14, %r12                                    #2828.30
..LN1233:
   .loc    1  2831
        movq      %r12, %rsi                                    #2831.14
        call      prime_buffer_2na@PLT                          #2831.14
                                # LOE r12 r13 r14 r15 ebx xmm0
..B11.49:                       # Preds ..B11.48
        movl      -16(%rbp), %eax                               #
..LN1235:
   .loc    1  2832
        lea       16(%r14,%r15), %r15                           #2832.5
        movq      %r15, %rsi                                    #2832.5
..LN1237:
   .loc    1  2837
        cmpq      %r12, %r15                                    #2837.16
..LN1239:
   .loc    1  2842
..LN1241:
   .loc    1  2837
        jae       ..B11.3       # Prob 12%                      #2837.16
                                # LOE rax rsi r12 r13 r15 eax ebx r14d al ah xmm0
..B11.2:                        # Preds ..B11.49
..LN1243:
   .loc    1  2838
        movzbl    -1(%r15), %r14d                               #2838.24
..LN1245:
        shll      $8, %r14d                                     #2838.38
                                # LOE rax rsi r12 r13 r15 eax ebx r14d al ah xmm0
..B11.3:                        # Preds ..B11.2 ..B11.49
..LN1247:
   .loc    1  2842
        movdqa    16(%r13), %xmm7                               #2842.5
        movdqa    32(%r13), %xmm6                               #2842.5
        movdqa    48(%r13), %xmm5                               #2842.5
        movdqa    64(%r13), %xmm4                               #2842.5
        movdqa    80(%r13), %xmm3                               #2842.5
        movdqa    96(%r13), %xmm2                               #2842.5
        movdqa    112(%r13), %xmm1                              #2842.5
        movdqa    128(%r13), %xmm8                              #2842.5
..LN1249:
   .loc    1  2845
        xorl      %edi, %edi                                    #2845.15
..LN1251:
        xorl      %ecx, %ecx                                    #2845.10
..LN1253:
        xorl      %edx, %edx                                    #2845.5
..LN1255:
   .loc    1  2849
        movl      $4, %r13d                                     #2849.5
..LN1257:
   .loc    1  2854
        movl      %ebx, %r8d                                    #2854.20
        andl      $3, %r8d                                      #2854.20
        je        ..B11.40      # Prob 20%                      #2854.20
                                # LOE rax rsi r12 r15 eax edx ecx ebx edi r8d r13d r14d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.4:                        # Preds ..B11.3
        cmpl      $1, %r8d                                      #2854.20
        je        ..B11.10      # Prob 25%                      #2854.20
                                # LOE rax rsi r12 r15 eax edx ecx ebx edi r8d r13d r14d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.5:                        # Preds ..B11.4
        cmpl      $2, %r8d                                      #2854.20
        je        ..B11.9       # Prob 33%                      #2854.20
                                # LOE rax rsi r12 r15 eax edx ecx ebx edi r8d r13d r14d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.6:                        # Preds ..B11.5
        cmpl      $3, %r8d                                      #2854.20
        je        ..B11.8       # Prob 50%                      #2854.20
                                # LOE rax rsi r12 r15 eax edx ecx ebx edi r13d r14d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.7:                        # Preds ..B11.6
        movdqa    %xmm8, -112(%rbp)                             #
        movdqa    %xmm1, -96(%rbp)                              #
        movdqa    %xmm2, -176(%rbp)                             #
        movdqa    %xmm3, -128(%rbp)                             #
        movdqa    %xmm4, -80(%rbp)                              #
        movdqa    %xmm5, -192(%rbp)                             #
        movdqa    %xmm6, -160(%rbp)                             #
        movdqa    %xmm7, -144(%rbp)                             #
        movq      %r12, -64(%rbp)                               #
        movl      %eax, -16(%rbp)                               #
        movl      %ebx, %eax                                    #
        jmp       ..B11.32      # Prob 100%                     #
                                # LOE rsi r15 eax r14d xmm0
..B11.8:                        # Preds ..B11.6
        movdqa    %xmm8, -112(%rbp)                             #
        movdqa    %xmm1, -96(%rbp)                              #
        movdqa    %xmm2, -176(%rbp)                             #
        movdqa    %xmm3, -128(%rbp)                             #
        movdqa    %xmm4, -80(%rbp)                              #
        movdqa    %xmm5, -192(%rbp)                             #
        movdqa    %xmm6, -160(%rbp)                             #
        movdqa    %xmm7, -144(%rbp)                             #
        movq      %r12, -64(%rbp)                               #
        movl      %eax, -16(%rbp)                               #
        movl      %ebx, %eax                                    #
        movl      %edi, %ebx                                    #
        jmp       ..B11.13      # Prob 100%                     #
                                # LOE rsi r15 eax edx ecx ebx r13d r14d xmm0
..B11.9:                        # Preds ..B11.5
        movdqa    %xmm8, -112(%rbp)                             #
        movdqa    %xmm1, -96(%rbp)                              #
        movdqa    %xmm2, -176(%rbp)                             #
        movdqa    %xmm3, -128(%rbp)                             #
        movdqa    %xmm4, -80(%rbp)                              #
        movdqa    %xmm5, -192(%rbp)                             #
        movdqa    %xmm6, -160(%rbp)                             #
        movdqa    %xmm7, -144(%rbp)                             #
        movq      %r12, -64(%rbp)                               #
        movl      %eax, -16(%rbp)                               #
        movl      %ebx, %eax                                    #
        jmp       ..B11.12      # Prob 100%                     #
                                # LOE rsi r15 eax edx ecx r13d r14d xmm0
..B11.10:                       # Preds ..B11.4
        movdqa    %xmm8, -112(%rbp)                             #
        movdqa    %xmm1, -96(%rbp)                              #
        movdqa    %xmm2, -176(%rbp)                             #
        movdqa    %xmm3, -128(%rbp)                             #
        movdqa    %xmm4, -80(%rbp)                              #
        movdqa    %xmm5, -192(%rbp)                             #
        movdqa    %xmm6, -160(%rbp)                             #
        movdqa    %xmm7, -144(%rbp)                             #
        movq      %r12, -64(%rbp)                               #
        movl      %eax, -16(%rbp)                               #
        movl      %ebx, %eax                                    #
                                # LOE rsi r15 eax edx r13d r14d xmm0
..B11.11:                       # Preds ..B11.10 ..B11.36
..LN1259:
   .loc    1  2877
        movdqa    %xmm0, %xmm1                                  #2877.22
        pand      -80(%rbp), %xmm1                              #2877.22
..LN1261:
   .loc    1  2878
        pcmpeqd   -192(%rbp), %xmm1                             #2878.22
..LN1263:
   .loc    1  2879
        pmovmskb  %xmm1, %ecx                                   #2879.22
                                # LOE rsi r15 eax edx ecx r13d r14d xmm0
..B11.12:                       # Preds ..B11.9 ..B11.11
..LN1265:
   .loc    1  2883
        movdqa    %xmm0, %xmm1                                  #2883.22
        pand      -176(%rbp), %xmm1                             #2883.22
..LN1267:
   .loc    1  2884
        pcmpeqd   -128(%rbp), %xmm1                             #2884.22
..LN1269:
   .loc    1  2885
        pmovmskb  %xmm1, %ebx                                   #2885.22
                                # LOE rsi r15 eax edx ecx ebx r13d r14d xmm0
..B11.13:                       # Preds ..B11.8 ..B11.12
..LN1271:
   .loc    1  2889
        movdqa    %xmm0, %xmm1                                  #2889.22
        pand      -112(%rbp), %xmm1                             #2889.22
..LN1273:
   .loc    1  2890
        pcmpeqd   -96(%rbp), %xmm1                              #2890.22
..LN1275:
   .loc    1  2891
        pmovmskb  %xmm1, %r8d                                   #2891.22
..LN1277:
   .loc    1  2896
        movl      %eax, %r12d                                   #2896.17
        andl      $-4, %r12d                                    #2896.17
..LN1279:
   .loc    1  2899
        movl      %edx, %eax                                    #2899.29
        orl       %ecx, %eax                                    #2899.29
..LN1281:
        orl       %ebx, %eax                                    #2899.34
..LN1283:
        orl       %r8d, %eax                                    #2899.39
..LN1285:
        je        ..B11.26      # Prob 78%                      #2899.47
                                # LOE rsi r15 edx ecx ebx r8d r12d r13d r14d xmm0
..B11.14:                       # Preds ..B11.13
..LN1287:
   .loc    1  2917
        movdqa    %xmm0, -272(%rbp)                             #2917.30
        movl      %edx, %edi                                    #2917.30
        movl      %r8d, -232(%rbp)                              #2917.30
        movl      %edx, -224(%rbp)                              #2917.30
        movl      %ecx, -216(%rbp)                              #2917.30
        movq      %rsi, -208(%rbp)                              #2917.30
        call      uint16_lsbit@PLT                              #2917.30
                                # LOE r15 eax ebx r12d r13d r14d
..B11.50:                       # Preds ..B11.14
        movq      -208(%rbp), %rsi                              #
        movdqa    -272(%rbp), %xmm0                             #
        movl      -216(%rbp), %ecx                              #
        movl      -224(%rbp), %edx                              #
        movl      -232(%rbp), %r8d                              #
                                # LOE rdx rcx rsi r8 r15 eax edx ecx ebx r8d r12d r13d r14d dl cl r8b dh ch xmm0
..B11.15:                       # Preds ..B11.50
..LN1289:
   .loc    1  2918
        movdqa    %xmm0, -272(%rbp)                             #2918.30
        movl      %ecx, %edi                                    #2918.30
        movl      %r8d, -232(%rbp)                              #2918.30
..LN1291:
   .loc    1  2917
        movswq    %ax, %rax                                     #2917.30
        movl      %eax, -240(%rbp)                              #2917.30
..LN1293:
   .loc    1  2918
        movl      %edx, -224(%rbp)                              #2918.30
        movl      %ecx, -216(%rbp)                              #2918.30
        movq      %rsi, -208(%rbp)                              #2918.30
        call      uint16_lsbit@PLT                              #2918.30
                                # LOE r15 eax ebx r12d r13d r14d
..B11.51:                       # Preds ..B11.15
        movq      -208(%rbp), %rsi                              #
        movdqa    -272(%rbp), %xmm0                             #
        movl      -216(%rbp), %ecx                              #
        movl      -224(%rbp), %edx                              #
        movl      -232(%rbp), %r8d                              #
                                # LOE rdx rcx rsi r8 r15 eax edx ecx ebx r8d r12d r13d r14d dl cl r8b dh ch xmm0
..B11.16:                       # Preds ..B11.51
..LN1295:
   .loc    1  2919
        movdqa    %xmm0, -272(%rbp)                             #2919.30
        movl      %ebx, %edi                                    #2919.30
        movl      %r8d, -232(%rbp)                              #2919.30
..LN1297:
   .loc    1  2918
        movswq    %ax, %rax                                     #2918.30
        movl      %eax, -248(%rbp)                              #2918.30
..LN1299:
   .loc    1  2919
        movl      %edx, -224(%rbp)                              #2919.30
        movl      %ecx, -216(%rbp)                              #2919.30
        movq      %rsi, -208(%rbp)                              #2919.30
        call      uint16_lsbit@PLT                              #2919.30
                                # LOE r15 eax ebx r12d r13d r14d
..B11.52:                       # Preds ..B11.16
        movq      -208(%rbp), %rsi                              #
        movdqa    -272(%rbp), %xmm0                             #
        movl      -216(%rbp), %ecx                              #
        movl      -224(%rbp), %edx                              #
        movl      -232(%rbp), %r8d                              #
                                # LOE rdx rcx rsi r8 r15 eax edx ecx ebx r8d r12d r13d r14d dl cl r8b dh ch xmm0
..B11.17:                       # Preds ..B11.52
..LN1301:
   .loc    1  2920
        movdqa    %xmm0, -272(%rbp)                             #2920.30
        movl      %r8d, %edi                                    #2920.30
        movl      %r8d, -232(%rbp)                              #2920.30
..LN1303:
   .loc    1  2919
        movswq    %ax, %rax                                     #2919.30
        movl      %eax, -256(%rbp)                              #2919.30
..LN1305:
   .loc    1  2920
        movl      %edx, -224(%rbp)                              #2920.30
        movl      %ecx, -216(%rbp)                              #2920.30
        movq      %rsi, -208(%rbp)                              #2920.30
        call      uint16_lsbit@PLT                              #2920.30
                                # LOE r15 eax ebx r12d r13d r14d
..B11.53:                       # Preds ..B11.17
        movq      -208(%rbp), %rsi                              #
        movdqa    -272(%rbp), %xmm0                             #
        movl      -216(%rbp), %ecx                              #
        movl      -224(%rbp), %edx                              #
        movl      -232(%rbp), %r8d                              #
                                # LOE rdx rcx rsi r8 r15 eax edx ecx ebx r8d r12d r13d r14d dl cl r8b dh ch xmm0
..B11.18:                       # Preds ..B11.53
..LN1307:
   .loc    1  2925
        movl      -240(%rbp), %r9d                              #2925.34
..LN1309:
   .loc    1  2926
        movl      -248(%rbp), %r10d                             #2926.34
..LN1311:
   .loc    1  2927
        movl      -256(%rbp), %r11d                             #2927.34
..LN1313:
   .loc    1  2920
        movswq    %ax, %rax                                     #2920.30
..LN1315:
   .loc    1  2925
        shll      $2, %r9d                                      #2925.34
        movl      %r9d, -240(%rbp)                              #2925.34
..LN1317:
   .loc    1  2926
        shll      $2, %r10d                                     #2926.34
        movl      %r10d, -248(%rbp)                             #2926.34
..LN1319:
   .loc    1  2927
        shll      $2, %r11d                                     #2927.34
        movl      %r11d, -256(%rbp)                             #2927.34
..LN1321:
   .loc    1  2928
        shll      $2, %eax                                      #2928.34
..LN1323:
   .loc    1  2932
        testl     %edx, %edx                                    #2932.32
        je        ..B11.20      # Prob 50%                      #2932.32
                                # LOE rcx rsi r8 r9 r10 r11 r15 eax ecx ebx r8d r9d r10d r11d r12d r13d r14d cl r8b r9b r10b r11b ch xmm0
..B11.19:                       # Preds ..B11.18
..LN1325:
        movl      %r9d, %edx                                    #2932.43
        addl      %r12d, %edx                                   #2932.43
..LN1327:
        cmpl      -16(%rbp), %edx                               #2932.49
        jbe       ..B11.41      # Prob 20%                      #2932.49
                                # LOE rcx rsi r8 r10 r11 r15 eax ecx ebx r8d r10d r11d r12d r13d r14d cl r8b r10b r11b ch xmm0
..B11.20:                       # Preds ..B11.19 ..B11.18
..LN1329:
   .loc    1  2933
        testl     %ecx, %ecx                                    #2933.32
        je        ..B11.22      # Prob 50%                      #2933.32
                                # LOE rsi r8 r10 r11 r15 eax ebx r8d r10d r11d r12d r13d r14d r8b r10b r11b xmm0
..B11.21:                       # Preds ..B11.20
..LN1331:
        movl      %r10d, %edx                                   #2933.43
        lea       1(%r12,%rdx), %ecx                            #2933.43
..LN1333:
        cmpl      -16(%rbp), %ecx                               #2933.49
        jbe       ..B11.41      # Prob 20%                      #2933.49
                                # LOE rsi r8 r11 r15 eax ebx r8d r11d r12d r13d r14d r8b r11b xmm0
..B11.22:                       # Preds ..B11.21 ..B11.20
..LN1335:
   .loc    1  2934
        testl     %ebx, %ebx                                    #2934.32
        je        ..B11.24      # Prob 50%                      #2934.32
                                # LOE rsi r8 r11 r15 eax r8d r11d r12d r13d r14d r8b r11b xmm0
..B11.23:                       # Preds ..B11.22
..LN1337:
        movl      %r11d, %edx                                   #2934.43
        lea       2(%r12,%rdx), %ecx                            #2934.43
..LN1339:
        cmpl      -16(%rbp), %ecx                               #2934.49
        jbe       ..B11.41      # Prob 20%                      #2934.49
                                # LOE rsi r8 r15 eax r8d r12d r13d r14d r8b xmm0
..B11.24:                       # Preds ..B11.23 ..B11.22
..LN1341:
   .loc    1  2935
        testl     %r8d, %r8d                                    #2935.32
        je        ..B11.26      # Prob 50%                      #2935.32
                                # LOE rsi r15 eax r12d r13d r14d xmm0
..B11.25:                       # Preds ..B11.24
..LN1343:
        lea       3(%r12,%rax), %eax                            #2935.43
..LN1345:
        cmpl      -16(%rbp), %eax                               #2935.49
        jbe       ..B11.41      # Prob 20%                      #2935.49
                                # LOE rsi r15 r12d r13d r14d xmm0
..B11.26:                       # Preds ..B11.25 ..B11.24 ..B11.13
..LN1347:
   .loc    1  2940
        lea       4(%r12), %eax                                 #2940.17
..LN1349:
   .loc    1  2943
        cmpl      -16(%rbp), %eax                               #2943.28
        ja        ..B11.38      # Prob 20%                      #2943.28
                                # LOE rsi r15 eax r12d r13d r14d xmm0
..B11.27:                       # Preds ..B11.26
..LN1351:
   .loc    1  2947
        decl      %r13d                                         #2947.25
..LN1353:
        jne       ..B11.33      # Prob 50%                      #2947.39
                                # LOE rsi r15 eax r12d r13d r14d xmm0
..B11.28:                       # Preds ..B11.27
..LN1355:
   .loc    1  2968
        cmpq      -64(%rbp), %r15                               #2968.25
        jae       ..B11.38      # Prob 4%                       #2968.25
                                # LOE r15 r12d r14d
..B11.29:                       # Preds ..B11.28
..LN1357:
   .loc    1  2972
        lea       52(%r12), %eax                                #2972.13
..LN1359:
   .loc    1  2973
        cmpl      -16(%rbp), %eax                               #2973.24
        ja        ..B11.38      # Prob 4%                       #2973.24
                                # LOE r15 eax r14d
..B11.30:                       # Preds ..B11.29
..LN1361:
   .loc    1  2978
        movq      -64(%rbp), %rsi                               #2978.22
        movl      %eax, -200(%rbp)                              #2978.22
        movq      %r15, %rdi                                    #2978.22
        call      prime_buffer_2na@PLT                          #2978.22
                                # LOE r15 r14d xmm0
..B11.54:                       # Preds ..B11.30
        movl      -200(%rbp), %eax                              #
..LN1363:
   .loc    1  3056
        addq      $16, %r15                                     #3056.13
..LN1365:
   .loc    1  3059
        cmpq      -64(%rbp), %r15                               #3059.24
..LN1367:
   .loc    1  3056
        movq      %r15, %rsi                                    #3056.13
..LN1369:
   .loc    1  3059
        jae       ..B11.32      # Prob 12%                      #3059.24
                                # LOE rax rsi r15 eax r14d al ah xmm0
..B11.31:                       # Preds ..B11.54
..LN1371:
   .loc    1  3060
        movzbl    -1(%r15), %r14d                               #3060.32
..LN1373:
        shll      $8, %r14d                                     #3060.46
                                # LOE rsi r15 eax r14d xmm0
..B11.32:                       # Preds ..B11.54 ..B11.7 ..B11.31
..LN1375:
   .loc    1  2861
        movl      $4, %r13d                                     #2861.13
        jmp       ..B11.36      # Prob 100%                     #2861.13
                                # LOE rsi r15 eax r13d r14d xmm0
..B11.33:                       # Preds ..B11.27
..LN1377:
   .loc    1  2955
        cmpq      -64(%rbp), %rsi                               #2955.26
..LN1379:
   .loc    1  2952
        psrldq    $1, %xmm0                                     #2952.26
..LN1381:
   .loc    1  2955
        jae       ..B11.35      # Prob 19%                      #2955.26
                                # LOE rsi r15 eax r13d r14d xmm0
..B11.34:                       # Preds ..B11.33
..LN1383:
   .loc    1  2958
        movzbl    (%rsi), %edx                                  #2958.37
..LN1385:
   .loc    1  2957
        sarl      $8, %r14d                                     #2957.21
..LN1387:
   .loc    1  2958
        shll      $8, %edx                                      #2958.48
..LN1389:
        orl       %edx, %r14d                                   #2958.21
..LN1391:
   .loc    1  2959
        pinsrw    $7, %r14d, %xmm0                              #2959.30
                                # LOE rsi r15 eax r13d r14d xmm0
..B11.35:                       # Preds ..B11.34 ..B11.33
..LN1393:
   .loc    1  2963
        incq      %rsi                                          #2963.20
                                # LOE rsi r15 eax r13d r14d xmm0
..B11.36:                       # Preds ..B11.32 ..B11.35 ..B11.40
..LN1395:
   .loc    1  2871
        movdqa    %xmm0, %xmm1                                  #2871.22
        pand      -160(%rbp), %xmm1                             #2871.22
..LN1397:
   .loc    1  2872
        pcmpeqd   -144(%rbp), %xmm1                             #2872.22
..LN1399:
   .loc    1  2873
        pmovmskb  %xmm1, %edx                                   #2873.22
        jmp       ..B11.11      # Prob 100%                     #2873.22
                                # LOE rsi r15 eax edx r13d r14d xmm0
..B11.38:                       # Preds ..B11.26 ..B11.28 ..B11.29 # Infreq
..LN1401:
   .loc    1  3069
        movq      -32(%rbp), %rbx                               #3069.12
..___tag_value_eval_2na_32.471:                                 #
        movq      -40(%rbp), %r12                               #3069.12
..___tag_value_eval_2na_32.472:                                 #
        movq      -24(%rbp), %r13                               #3069.12
..___tag_value_eval_2na_32.473:                                 #
        movq      -48(%rbp), %r14                               #3069.12
..___tag_value_eval_2na_32.474:                                 #
        xorl      %eax, %eax                                    #3069.12
        movq      -56(%rbp), %r15                               #3069.12
..___tag_value_eval_2na_32.475:                                 #
        movq      %rbp, %rsp                                    #3069.12
        popq      %rbp                                          #3069.12
..___tag_value_eval_2na_32.476:                                 #
        ret                                                     #3069.12
..___tag_value_eval_2na_32.477:                                 #
                                # LOE
..B11.40:                       # Preds ..B11.3                 # Infreq
        movdqa    %xmm8, -112(%rbp)                             #
        movdqa    %xmm1, -96(%rbp)                              #
        movdqa    %xmm2, -176(%rbp)                             #
        movdqa    %xmm3, -128(%rbp)                             #
        movdqa    %xmm4, -80(%rbp)                              #
        movdqa    %xmm5, -192(%rbp)                             #
        movdqa    %xmm6, -160(%rbp)                             #
        movdqa    %xmm7, -144(%rbp)                             #
        movq      %r12, -64(%rbp)                               #
        movl      %eax, -16(%rbp)                               #
        movl      %ebx, %eax                                    #
        jmp       ..B11.36      # Prob 100%                     #
                                # LOE rsi r15 eax r13d r14d xmm0
..B11.41:                       # Preds ..B11.23 ..B11.21 ..B11.19 ..B11.25 # Infreq
..LN1403:
   .loc    1  2935
        movq      -32(%rbp), %rbx                               #2935.63
..___tag_value_eval_2na_32.483:                                 #
        movq      -40(%rbp), %r12                               #2935.63
..___tag_value_eval_2na_32.484:                                 #
        movq      -24(%rbp), %r13                               #2935.63
..___tag_value_eval_2na_32.485:                                 #
        movq      -48(%rbp), %r14                               #2935.63
..___tag_value_eval_2na_32.486:                                 #
        movl      $1, %eax                                      #2935.63
        movq      -56(%rbp), %r15                               #2935.63
..___tag_value_eval_2na_32.487:                                 #
        movq      %rbp, %rsp                                    #2935.63
        popq      %rbp                                          #2935.63
..___tag_value_eval_2na_32.488:                                 #
        ret                                                     #2935.63
        .align    16,0x90
..___tag_value_eval_2na_32.489:                                 #
                                # LOE
# mark_end;
	.type	eval_2na_32,@function
	.size	eval_2na_32,.-eval_2na_32
.LNeval_2na_32:
	.data
# -- End  eval_2na_32
	.text
# -- Begin  eval_2na_16
# mark_begin;
       .align    16,0x90
eval_2na_16:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B12.1:                        # Preds ..B12.0
..___tag_value_eval_2na_16.490:                                 #2489.1
..LN1405:
   .loc    1  2489
        pushq     %rbp                                          #2489.1
        movq      %rsp, %rbp                                    #2489.1
..___tag_value_eval_2na_16.491:                                 #
        subq      $272, %rsp                                    #2489.1
        movq      %r15, -56(%rbp)                               #2489.1
..___tag_value_eval_2na_16.493:                                 #
        movq      %r14, -48(%rbp)                               #2489.1
..___tag_value_eval_2na_16.494:                                 #
        movq      %r13, -24(%rbp)                               #2489.1
..___tag_value_eval_2na_16.495:                                 #
        movq      %r12, -40(%rbp)                               #2489.1
..___tag_value_eval_2na_16.496:                                 #
        movq      %rbx, -32(%rbp)                               #2489.1
..___tag_value_eval_2na_16.497:                                 #
        movl      %ecx, %r12d                                   #2489.1
        movl      %edx, %ebx                                    #2489.1
        movq      %rsi, %r14                                    #2489.1
        movq      %rdi, %r13                                    #2489.1
        lea       _gprof_pack11(%rip), %rdx                     #2489.1
        call      mcount@PLT                                    #2489.1
                                # LOE r13 r14 ebx r12d
..B12.48:                       # Preds ..B12.1
..LN1407:
   .loc    1  2526
        lea       (%rbx,%r12), %eax                             #2526.5
..LN1409:
   .loc    1  2532
        subl      4(%r13), %eax                                 #2532.12
..LN1411:
   .loc    1  2538
        movl      %eax, -16(%rbp)                               #2538.14
..LN1413:
   .loc    1  2529
        movl      %ebx, %r15d                                   #2529.49
        shrl      $2, %r15d                                     #2529.49
..LN1415:
        lea       (%r14,%r15), %rdi                             #2529.30
..LN1417:
   .loc    1  2535
        lea       3(%rbx,%r12), %r12d                           #2535.50
..LN1419:
        shrl      $2, %r12d                                     #2535.57
..LN1421:
        addq      %r14, %r12                                    #2535.30
..LN1423:
   .loc    1  2538
        movq      %r12, %rsi                                    #2538.14
        call      prime_buffer_2na@PLT                          #2538.14
                                # LOE r12 r13 r14 r15 ebx xmm0
..B12.49:                       # Preds ..B12.48
        movl      -16(%rbp), %eax                               #
..LN1425:
   .loc    1  2539
        lea       16(%r14,%r15), %r15                           #2539.5
        movq      %r15, %rsi                                    #2539.5
..LN1427:
   .loc    1  2544
        cmpq      %r12, %r15                                    #2544.16
..LN1429:
   .loc    1  2549
..LN1431:
   .loc    1  2544
        jae       ..B12.3       # Prob 12%                      #2544.16
                                # LOE rax rsi r12 r13 r15 eax ebx r14d al ah xmm0
..B12.2:                        # Preds ..B12.49
..LN1433:
   .loc    1  2545
        movzbl    -1(%r15), %r14d                               #2545.24
..LN1435:
        shll      $8, %r14d                                     #2545.38
                                # LOE rax rsi r12 r13 r15 eax ebx r14d al ah xmm0
..B12.3:                        # Preds ..B12.2 ..B12.49
..LN1437:
   .loc    1  2549
        movdqa    16(%r13), %xmm7                               #2549.5
        movdqa    32(%r13), %xmm6                               #2549.5
        movdqa    48(%r13), %xmm5                               #2549.5
        movdqa    64(%r13), %xmm4                               #2549.5
        movdqa    80(%r13), %xmm3                               #2549.5
        movdqa    96(%r13), %xmm2                               #2549.5
        movdqa    112(%r13), %xmm1                              #2549.5
        movdqa    128(%r13), %xmm8                              #2549.5
..LN1439:
   .loc    1  2552
        xorl      %edi, %edi                                    #2552.15
..LN1441:
        xorl      %ecx, %ecx                                    #2552.10
..LN1443:
        xorl      %edx, %edx                                    #2552.5
..LN1445:
   .loc    1  2556
        movl      $2, %r13d                                     #2556.5
..LN1447:
   .loc    1  2561
        movl      %ebx, %r8d                                    #2561.20
        andl      $3, %r8d                                      #2561.20
        je        ..B12.40      # Prob 20%                      #2561.20
                                # LOE rax rsi r12 r15 eax edx ecx ebx edi r8d r13d r14d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.4:                        # Preds ..B12.3
        cmpl      $1, %r8d                                      #2561.20
        je        ..B12.10      # Prob 25%                      #2561.20
                                # LOE rax rsi r12 r15 eax edx ecx ebx edi r8d r13d r14d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.5:                        # Preds ..B12.4
        cmpl      $2, %r8d                                      #2561.20
        je        ..B12.9       # Prob 33%                      #2561.20
                                # LOE rax rsi r12 r15 eax edx ecx ebx edi r8d r13d r14d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.6:                        # Preds ..B12.5
        cmpl      $3, %r8d                                      #2561.20
        je        ..B12.8       # Prob 50%                      #2561.20
                                # LOE rax rsi r12 r15 eax edx ecx ebx edi r13d r14d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.7:                        # Preds ..B12.6
        movdqa    %xmm8, -112(%rbp)                             #
        movdqa    %xmm1, -96(%rbp)                              #
        movdqa    %xmm2, -176(%rbp)                             #
        movdqa    %xmm3, -128(%rbp)                             #
        movdqa    %xmm4, -80(%rbp)                              #
        movdqa    %xmm5, -192(%rbp)                             #
        movdqa    %xmm6, -160(%rbp)                             #
        movdqa    %xmm7, -144(%rbp)                             #
        movq      %r12, -64(%rbp)                               #
        movl      %eax, -16(%rbp)                               #
        movl      %ebx, %eax                                    #
        jmp       ..B12.32      # Prob 100%                     #
                                # LOE rsi r15 eax r14d xmm0
..B12.8:                        # Preds ..B12.6
        movdqa    %xmm8, -112(%rbp)                             #
        movdqa    %xmm1, -96(%rbp)                              #
        movdqa    %xmm2, -176(%rbp)                             #
        movdqa    %xmm3, -128(%rbp)                             #
        movdqa    %xmm4, -80(%rbp)                              #
        movdqa    %xmm5, -192(%rbp)                             #
        movdqa    %xmm6, -160(%rbp)                             #
        movdqa    %xmm7, -144(%rbp)                             #
        movq      %r12, -64(%rbp)                               #
        movl      %eax, -16(%rbp)                               #
        movl      %ebx, %eax                                    #
        movl      %edi, %ebx                                    #
        jmp       ..B12.13      # Prob 100%                     #
                                # LOE rsi r15 eax edx ecx ebx r13d r14d xmm0
..B12.9:                        # Preds ..B12.5
        movdqa    %xmm8, -112(%rbp)                             #
        movdqa    %xmm1, -96(%rbp)                              #
        movdqa    %xmm2, -176(%rbp)                             #
        movdqa    %xmm3, -128(%rbp)                             #
        movdqa    %xmm4, -80(%rbp)                              #
        movdqa    %xmm5, -192(%rbp)                             #
        movdqa    %xmm6, -160(%rbp)                             #
        movdqa    %xmm7, -144(%rbp)                             #
        movq      %r12, -64(%rbp)                               #
        movl      %eax, -16(%rbp)                               #
        movl      %ebx, %eax                                    #
        jmp       ..B12.12      # Prob 100%                     #
                                # LOE rsi r15 eax edx ecx r13d r14d xmm0
..B12.10:                       # Preds ..B12.4
        movdqa    %xmm8, -112(%rbp)                             #
        movdqa    %xmm1, -96(%rbp)                              #
        movdqa    %xmm2, -176(%rbp)                             #
        movdqa    %xmm3, -128(%rbp)                             #
        movdqa    %xmm4, -80(%rbp)                              #
        movdqa    %xmm5, -192(%rbp)                             #
        movdqa    %xmm6, -160(%rbp)                             #
        movdqa    %xmm7, -144(%rbp)                             #
        movq      %r12, -64(%rbp)                               #
        movl      %eax, -16(%rbp)                               #
        movl      %ebx, %eax                                    #
                                # LOE rsi r15 eax edx r13d r14d xmm0
..B12.11:                       # Preds ..B12.10 ..B12.36
..LN1449:
   .loc    1  2584
        movdqa    %xmm0, %xmm1                                  #2584.22
        pand      -80(%rbp), %xmm1                              #2584.22
..LN1451:
   .loc    1  2585
        pcmpeqw   -192(%rbp), %xmm1                             #2585.22
..LN1453:
   .loc    1  2586
        pmovmskb  %xmm1, %ecx                                   #2586.22
                                # LOE rsi r15 eax edx ecx r13d r14d xmm0
..B12.12:                       # Preds ..B12.9 ..B12.11
..LN1455:
   .loc    1  2590
        movdqa    %xmm0, %xmm1                                  #2590.22
        pand      -176(%rbp), %xmm1                             #2590.22
..LN1457:
   .loc    1  2591
        pcmpeqw   -128(%rbp), %xmm1                             #2591.22
..LN1459:
   .loc    1  2592
        pmovmskb  %xmm1, %ebx                                   #2592.22
                                # LOE rsi r15 eax edx ecx ebx r13d r14d xmm0
..B12.13:                       # Preds ..B12.8 ..B12.12
..LN1461:
   .loc    1  2596
        movdqa    %xmm0, %xmm1                                  #2596.22
        pand      -112(%rbp), %xmm1                             #2596.22
..LN1463:
   .loc    1  2597
        pcmpeqw   -96(%rbp), %xmm1                              #2597.22
..LN1465:
   .loc    1  2598
        pmovmskb  %xmm1, %r8d                                   #2598.22
..LN1467:
   .loc    1  2603
        movl      %eax, %r12d                                   #2603.17
        andl      $-4, %r12d                                    #2603.17
..LN1469:
   .loc    1  2606
        movl      %edx, %eax                                    #2606.29
        orl       %ecx, %eax                                    #2606.29
..LN1471:
        orl       %ebx, %eax                                    #2606.34
..LN1473:
        orl       %r8d, %eax                                    #2606.39
..LN1475:
        je        ..B12.26      # Prob 78%                      #2606.47
                                # LOE rsi r15 edx ecx ebx r8d r12d r13d r14d xmm0
..B12.14:                       # Preds ..B12.13
..LN1477:
   .loc    1  2624
        movdqa    %xmm0, -272(%rbp)                             #2624.30
        movl      %edx, %edi                                    #2624.30
        movl      %r8d, -232(%rbp)                              #2624.30
        movl      %edx, -224(%rbp)                              #2624.30
        movl      %ecx, -216(%rbp)                              #2624.30
        movq      %rsi, -208(%rbp)                              #2624.30
        call      uint16_lsbit@PLT                              #2624.30
                                # LOE r15 eax ebx r12d r13d r14d
..B12.50:                       # Preds ..B12.14
        movq      -208(%rbp), %rsi                              #
        movdqa    -272(%rbp), %xmm0                             #
        movl      -216(%rbp), %ecx                              #
        movl      -224(%rbp), %edx                              #
        movl      -232(%rbp), %r8d                              #
                                # LOE rdx rcx rsi r8 r15 eax edx ecx ebx r8d r12d r13d r14d dl cl r8b dh ch xmm0
..B12.15:                       # Preds ..B12.50
..LN1479:
   .loc    1  2625
        movdqa    %xmm0, -272(%rbp)                             #2625.30
        movl      %ecx, %edi                                    #2625.30
        movl      %r8d, -232(%rbp)                              #2625.30
..LN1481:
   .loc    1  2624
        movswq    %ax, %rax                                     #2624.30
        movl      %eax, -240(%rbp)                              #2624.30
..LN1483:
   .loc    1  2625
        movl      %edx, -224(%rbp)                              #2625.30
        movl      %ecx, -216(%rbp)                              #2625.30
        movq      %rsi, -208(%rbp)                              #2625.30
        call      uint16_lsbit@PLT                              #2625.30
                                # LOE r15 eax ebx r12d r13d r14d
..B12.51:                       # Preds ..B12.15
        movq      -208(%rbp), %rsi                              #
        movdqa    -272(%rbp), %xmm0                             #
        movl      -216(%rbp), %ecx                              #
        movl      -224(%rbp), %edx                              #
        movl      -232(%rbp), %r8d                              #
                                # LOE rdx rcx rsi r8 r15 eax edx ecx ebx r8d r12d r13d r14d dl cl r8b dh ch xmm0
..B12.16:                       # Preds ..B12.51
..LN1485:
   .loc    1  2626
        movdqa    %xmm0, -272(%rbp)                             #2626.30
        movl      %ebx, %edi                                    #2626.30
        movl      %r8d, -232(%rbp)                              #2626.30
..LN1487:
   .loc    1  2625
        movswq    %ax, %rax                                     #2625.30
        movl      %eax, -248(%rbp)                              #2625.30
..LN1489:
   .loc    1  2626
        movl      %edx, -224(%rbp)                              #2626.30
        movl      %ecx, -216(%rbp)                              #2626.30
        movq      %rsi, -208(%rbp)                              #2626.30
        call      uint16_lsbit@PLT                              #2626.30
                                # LOE r15 eax ebx r12d r13d r14d
..B12.52:                       # Preds ..B12.16
        movq      -208(%rbp), %rsi                              #
        movdqa    -272(%rbp), %xmm0                             #
        movl      -216(%rbp), %ecx                              #
        movl      -224(%rbp), %edx                              #
        movl      -232(%rbp), %r8d                              #
                                # LOE rdx rcx rsi r8 r15 eax edx ecx ebx r8d r12d r13d r14d dl cl r8b dh ch xmm0
..B12.17:                       # Preds ..B12.52
..LN1491:
   .loc    1  2627
        movdqa    %xmm0, -272(%rbp)                             #2627.30
        movl      %r8d, %edi                                    #2627.30
        movl      %r8d, -232(%rbp)                              #2627.30
..LN1493:
   .loc    1  2626
        movswq    %ax, %rax                                     #2626.30
        movl      %eax, -256(%rbp)                              #2626.30
..LN1495:
   .loc    1  2627
        movl      %edx, -224(%rbp)                              #2627.30
        movl      %ecx, -216(%rbp)                              #2627.30
        movq      %rsi, -208(%rbp)                              #2627.30
        call      uint16_lsbit@PLT                              #2627.30
                                # LOE r15 eax ebx r12d r13d r14d
..B12.53:                       # Preds ..B12.17
        movq      -208(%rbp), %rsi                              #
        movdqa    -272(%rbp), %xmm0                             #
        movl      -216(%rbp), %ecx                              #
        movl      -224(%rbp), %edx                              #
        movl      -232(%rbp), %r8d                              #
                                # LOE rdx rcx rsi r8 r15 eax edx ecx ebx r8d r12d r13d r14d dl cl r8b dh ch xmm0
..B12.18:                       # Preds ..B12.53
..LN1497:
   .loc    1  2632
        movl      -240(%rbp), %r9d                              #2632.34
..LN1499:
   .loc    1  2633
        movl      -248(%rbp), %r10d                             #2633.34
..LN1501:
   .loc    1  2634
        movl      -256(%rbp), %r11d                             #2634.34
..LN1503:
   .loc    1  2627
        movswq    %ax, %rax                                     #2627.30
..LN1505:
   .loc    1  2632
        shll      $2, %r9d                                      #2632.34
        movl      %r9d, -240(%rbp)                              #2632.34
..LN1507:
   .loc    1  2633
        shll      $2, %r10d                                     #2633.34
        movl      %r10d, -248(%rbp)                             #2633.34
..LN1509:
   .loc    1  2634
        shll      $2, %r11d                                     #2634.34
        movl      %r11d, -256(%rbp)                             #2634.34
..LN1511:
   .loc    1  2635
        shll      $2, %eax                                      #2635.34
..LN1513:
   .loc    1  2639
        testl     %edx, %edx                                    #2639.32
        je        ..B12.20      # Prob 50%                      #2639.32
                                # LOE rcx rsi r8 r9 r10 r11 r15 eax ecx ebx r8d r9d r10d r11d r12d r13d r14d cl r8b r9b r10b r11b ch xmm0
..B12.19:                       # Preds ..B12.18
..LN1515:
        movl      %r9d, %edx                                    #2639.43
        addl      %r12d, %edx                                   #2639.43
..LN1517:
        cmpl      -16(%rbp), %edx                               #2639.49
        jbe       ..B12.41      # Prob 20%                      #2639.49
                                # LOE rcx rsi r8 r10 r11 r15 eax ecx ebx r8d r10d r11d r12d r13d r14d cl r8b r10b r11b ch xmm0
..B12.20:                       # Preds ..B12.19 ..B12.18
..LN1519:
   .loc    1  2640
        testl     %ecx, %ecx                                    #2640.32
        je        ..B12.22      # Prob 50%                      #2640.32
                                # LOE rsi r8 r10 r11 r15 eax ebx r8d r10d r11d r12d r13d r14d r8b r10b r11b xmm0
..B12.21:                       # Preds ..B12.20
..LN1521:
        movl      %r10d, %edx                                   #2640.43
        lea       1(%r12,%rdx), %ecx                            #2640.43
..LN1523:
        cmpl      -16(%rbp), %ecx                               #2640.49
        jbe       ..B12.41      # Prob 20%                      #2640.49
                                # LOE rsi r8 r11 r15 eax ebx r8d r11d r12d r13d r14d r8b r11b xmm0
..B12.22:                       # Preds ..B12.21 ..B12.20
..LN1525:
   .loc    1  2641
        testl     %ebx, %ebx                                    #2641.32
        je        ..B12.24      # Prob 50%                      #2641.32
                                # LOE rsi r8 r11 r15 eax r8d r11d r12d r13d r14d r8b r11b xmm0
..B12.23:                       # Preds ..B12.22
..LN1527:
        movl      %r11d, %edx                                   #2641.43
        lea       2(%r12,%rdx), %ecx                            #2641.43
..LN1529:
        cmpl      -16(%rbp), %ecx                               #2641.49
        jbe       ..B12.41      # Prob 20%                      #2641.49
                                # LOE rsi r8 r15 eax r8d r12d r13d r14d r8b xmm0
..B12.24:                       # Preds ..B12.23 ..B12.22
..LN1531:
   .loc    1  2642
        testl     %r8d, %r8d                                    #2642.32
        je        ..B12.26      # Prob 50%                      #2642.32
                                # LOE rsi r15 eax r12d r13d r14d xmm0
..B12.25:                       # Preds ..B12.24
..LN1533:
        lea       3(%r12,%rax), %eax                            #2642.43
..LN1535:
        cmpl      -16(%rbp), %eax                               #2642.49
        jbe       ..B12.41      # Prob 20%                      #2642.49
                                # LOE rsi r15 r12d r13d r14d xmm0
..B12.26:                       # Preds ..B12.25 ..B12.24 ..B12.13
..LN1537:
   .loc    1  2647
        lea       4(%r12), %eax                                 #2647.17
..LN1539:
   .loc    1  2650
        cmpl      -16(%rbp), %eax                               #2650.28
        ja        ..B12.38      # Prob 20%                      #2650.28
                                # LOE rsi r15 eax r12d r13d r14d xmm0
..B12.27:                       # Preds ..B12.26
..LN1541:
   .loc    1  2654
        decl      %r13d                                         #2654.25
..LN1543:
        jne       ..B12.33      # Prob 50%                      #2654.39
                                # LOE rsi r15 eax r12d r13d r14d xmm0
..B12.28:                       # Preds ..B12.27
..LN1545:
   .loc    1  2675
        cmpq      -64(%rbp), %r15                               #2675.25
        jae       ..B12.38      # Prob 4%                       #2675.25
                                # LOE r15 r12d r14d
..B12.29:                       # Preds ..B12.28
..LN1547:
   .loc    1  2679
        lea       60(%r12), %eax                                #2679.13
..LN1549:
   .loc    1  2680
        cmpl      -16(%rbp), %eax                               #2680.24
        ja        ..B12.38      # Prob 4%                       #2680.24
                                # LOE r15 eax r14d
..B12.30:                       # Preds ..B12.29
..LN1551:
   .loc    1  2685
        movq      -64(%rbp), %rsi                               #2685.22
        movl      %eax, -200(%rbp)                              #2685.22
        movq      %r15, %rdi                                    #2685.22
        call      prime_buffer_2na@PLT                          #2685.22
                                # LOE r15 r14d xmm0
..B12.54:                       # Preds ..B12.30
        movl      -200(%rbp), %eax                              #
..LN1553:
   .loc    1  2763
        addq      $16, %r15                                     #2763.13
..LN1555:
   .loc    1  2766
        cmpq      -64(%rbp), %r15                               #2766.24
..LN1557:
   .loc    1  2763
        movq      %r15, %rsi                                    #2763.13
..LN1559:
   .loc    1  2766
        jae       ..B12.32      # Prob 12%                      #2766.24
                                # LOE rax rsi r15 eax r14d al ah xmm0
..B12.31:                       # Preds ..B12.54
..LN1561:
   .loc    1  2767
        movzbl    -1(%r15), %r14d                               #2767.32
..LN1563:
        shll      $8, %r14d                                     #2767.46
                                # LOE rsi r15 eax r14d xmm0
..B12.32:                       # Preds ..B12.54 ..B12.7 ..B12.31
..LN1565:
   .loc    1  2568
        movl      $2, %r13d                                     #2568.13
        jmp       ..B12.36      # Prob 100%                     #2568.13
                                # LOE rsi r15 eax r13d r14d xmm0
..B12.33:                       # Preds ..B12.27
..LN1567:
   .loc    1  2662
        cmpq      -64(%rbp), %rsi                               #2662.26
..LN1569:
   .loc    1  2659
        psrldq    $1, %xmm0                                     #2659.26
..LN1571:
   .loc    1  2662
        jae       ..B12.35      # Prob 19%                      #2662.26
                                # LOE rsi r15 eax r13d r14d xmm0
..B12.34:                       # Preds ..B12.33
..LN1573:
   .loc    1  2665
        movzbl    (%rsi), %edx                                  #2665.37
..LN1575:
   .loc    1  2664
        sarl      $8, %r14d                                     #2664.21
..LN1577:
   .loc    1  2665
        shll      $8, %edx                                      #2665.48
..LN1579:
        orl       %edx, %r14d                                   #2665.21
..LN1581:
   .loc    1  2666
        pinsrw    $7, %r14d, %xmm0                              #2666.30
                                # LOE rsi r15 eax r13d r14d xmm0
..B12.35:                       # Preds ..B12.34 ..B12.33
..LN1583:
   .loc    1  2670
        incq      %rsi                                          #2670.20
                                # LOE rsi r15 eax r13d r14d xmm0
..B12.36:                       # Preds ..B12.32 ..B12.35 ..B12.40
..LN1585:
   .loc    1  2578
        movdqa    %xmm0, %xmm1                                  #2578.22
        pand      -160(%rbp), %xmm1                             #2578.22
..LN1587:
   .loc    1  2579
        pcmpeqw   -144(%rbp), %xmm1                             #2579.22
..LN1589:
   .loc    1  2580
        pmovmskb  %xmm1, %edx                                   #2580.22
        jmp       ..B12.11      # Prob 100%                     #2580.22
                                # LOE rsi r15 eax edx r13d r14d xmm0
..B12.38:                       # Preds ..B12.26 ..B12.28 ..B12.29 # Infreq
..LN1591:
   .loc    1  2776
        movq      -32(%rbp), %rbx                               #2776.12
..___tag_value_eval_2na_16.498:                                 #
        movq      -40(%rbp), %r12                               #2776.12
..___tag_value_eval_2na_16.499:                                 #
        movq      -24(%rbp), %r13                               #2776.12
..___tag_value_eval_2na_16.500:                                 #
        movq      -48(%rbp), %r14                               #2776.12
..___tag_value_eval_2na_16.501:                                 #
        xorl      %eax, %eax                                    #2776.12
        movq      -56(%rbp), %r15                               #2776.12
..___tag_value_eval_2na_16.502:                                 #
        movq      %rbp, %rsp                                    #2776.12
        popq      %rbp                                          #2776.12
..___tag_value_eval_2na_16.503:                                 #
        ret                                                     #2776.12
..___tag_value_eval_2na_16.504:                                 #
                                # LOE
..B12.40:                       # Preds ..B12.3                 # Infreq
        movdqa    %xmm8, -112(%rbp)                             #
        movdqa    %xmm1, -96(%rbp)                              #
        movdqa    %xmm2, -176(%rbp)                             #
        movdqa    %xmm3, -128(%rbp)                             #
        movdqa    %xmm4, -80(%rbp)                              #
        movdqa    %xmm5, -192(%rbp)                             #
        movdqa    %xmm6, -160(%rbp)                             #
        movdqa    %xmm7, -144(%rbp)                             #
        movq      %r12, -64(%rbp)                               #
        movl      %eax, -16(%rbp)                               #
        movl      %ebx, %eax                                    #
        jmp       ..B12.36      # Prob 100%                     #
                                # LOE rsi r15 eax r13d r14d xmm0
..B12.41:                       # Preds ..B12.23 ..B12.21 ..B12.19 ..B12.25 # Infreq
..LN1593:
   .loc    1  2642
        movq      -32(%rbp), %rbx                               #2642.63
..___tag_value_eval_2na_16.510:                                 #
        movq      -40(%rbp), %r12                               #2642.63
..___tag_value_eval_2na_16.511:                                 #
        movq      -24(%rbp), %r13                               #2642.63
..___tag_value_eval_2na_16.512:                                 #
        movq      -48(%rbp), %r14                               #2642.63
..___tag_value_eval_2na_16.513:                                 #
        movl      $1, %eax                                      #2642.63
        movq      -56(%rbp), %r15                               #2642.63
..___tag_value_eval_2na_16.514:                                 #
        movq      %rbp, %rsp                                    #2642.63
        popq      %rbp                                          #2642.63
..___tag_value_eval_2na_16.515:                                 #
        ret                                                     #2642.63
        .align    16,0x90
..___tag_value_eval_2na_16.516:                                 #
                                # LOE
# mark_end;
	.type	eval_2na_16,@function
	.size	eval_2na_16,.-eval_2na_16
.LNeval_2na_16:
	.data
# -- End  eval_2na_16
	.text
# -- Begin  eval_2na_8
# mark_begin;
       .align    16,0x90
eval_2na_8:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B13.1:                        # Preds ..B13.0
..___tag_value_eval_2na_8.517:                                  #2196.1
..LN1595:
   .loc    1  2196
        pushq     %rbp                                          #2196.1
        movq      %rsp, %rbp                                    #2196.1
..___tag_value_eval_2na_8.518:                                  #
        subq      $224, %rsp                                    #2196.1
        movq      %r15, -48(%rbp)                               #2196.1
..___tag_value_eval_2na_8.520:                                  #
        movq      %r14, -40(%rbp)                               #2196.1
..___tag_value_eval_2na_8.521:                                  #
        movq      %r13, -32(%rbp)                               #2196.1
..___tag_value_eval_2na_8.522:                                  #
        movq      %r12, -24(%rbp)                               #2196.1
..___tag_value_eval_2na_8.523:                                  #
        movq      %rbx, -56(%rbp)                               #2196.1
..___tag_value_eval_2na_8.524:                                  #
        movl      %ecx, %ebx                                    #2196.1
        movl      %edx, %r12d                                   #2196.1
        movq      %rsi, %r15                                    #2196.1
        movq      %rdi, %r13                                    #2196.1
        lea       _gprof_pack12(%rip), %rdx                     #2196.1
        call      mcount@PLT                                    #2196.1
                                # LOE r13 r15 ebx r12d
..B13.40:                       # Preds ..B13.1
..LN1597:
   .loc    1  2233
        lea       (%r12,%rbx), %eax                             #2233.5
..LN1599:
   .loc    1  2239
        subl      4(%r13), %eax                                 #2239.12
..LN1601:
   .loc    1  2245
        movl      %eax, -224(%rbp)                              #2245.14
..LN1603:
   .loc    1  2236
        movl      %r12d, %r14d                                  #2236.49
        shrl      $2, %r14d                                     #2236.49
..LN1605:
        lea       (%r15,%r14), %rdi                             #2236.30
..LN1607:
   .loc    1  2242
        lea       3(%r12,%rbx), %esi                            #2242.50
..LN1609:
        shrl      $2, %esi                                      #2242.57
..LN1611:
        addq      %r15, %rsi                                    #2242.30
        movq      %rsi, -16(%rbp)                               #2242.30
..LN1613:
   .loc    1  2245
        call      prime_buffer_2na@PLT                          #2245.14
                                # LOE r13 r14 r15 r12d xmm0
..B13.41:                       # Preds ..B13.40
        movl      -224(%rbp), %eax                              #
..LN1615:
   .loc    1  2256
        movdqa    16(%r13), %xmm7                               #2256.5
        movdqa    32(%r13), %xmm6                               #2256.5
        movdqa    48(%r13), %xmm5                               #2256.5
        movdqa    64(%r13), %xmm4                               #2256.5
        movdqa    80(%r13), %xmm3                               #2256.5
        movdqa    96(%r13), %xmm2                               #2256.5
        movdqa    112(%r13), %xmm1                              #2256.5
        movdqa    128(%r13), %xmm8                              #2256.5
..LN1617:
   .loc    1  2246
        lea       16(%r15,%r14), %rbx                           #2246.5
..LN1619:
   .loc    1  2259
        xorl      %r15d, %r15d                                  #2259.15
..LN1621:
        xorl      %ecx, %ecx                                    #2259.10
..LN1623:
        xorl      %r14d, %r14d                                  #2259.5
..LN1625:
   .loc    1  2268
        movl      %r12d, %edx                                   #2268.20
        andl      $3, %edx                                      #2268.20
        je        ..B13.5       # Prob 20%                      #2268.20
                                # LOE rax rbx eax edx ecx r12d r14d r15d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B13.2:                        # Preds ..B13.41
        cmpl      $1, %edx                                      #2268.20
        je        ..B13.8       # Prob 25%                      #2268.20
                                # LOE rax rbx eax edx ecx r12d r14d r15d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B13.3:                        # Preds ..B13.2
        cmpl      $2, %edx                                      #2268.20
        je        ..B13.7       # Prob 33%                      #2268.20
                                # LOE rax rbx eax edx ecx r12d r14d r15d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B13.4:                        # Preds ..B13.3
        cmpl      $3, %edx                                      #2268.20
        je        ..B13.6       # Prob 50%                      #2268.20
                                # LOE rax rbx eax ecx r12d r14d r15d al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B13.5:                        # Preds ..B13.41 ..B13.4
        movdqa    %xmm8, -96(%rbp)                              #
        movdqa    %xmm1, -112(%rbp)                             #
        movdqa    %xmm2, -160(%rbp)                             #
        movdqa    %xmm3, -176(%rbp)                             #
        movdqa    %xmm4, -144(%rbp)                             #
        movdqa    %xmm5, -128(%rbp)                             #
        movdqa    %xmm6, -80(%rbp)                              #
        movdqa    %xmm7, -192(%rbp)                             #
        movl      %eax, %r13d                                   #
        jmp       ..B13.28      # Prob 100%                     #
                                # LOE rbx r12d r13d xmm0
..B13.6:                        # Preds ..B13.4
        movdqa    %xmm8, -96(%rbp)                              #
        movdqa    %xmm1, -112(%rbp)                             #
        movdqa    %xmm2, -160(%rbp)                             #
        movdqa    %xmm3, -176(%rbp)                             #
        movdqa    %xmm4, -144(%rbp)                             #
        movdqa    %xmm5, -128(%rbp)                             #
        movdqa    %xmm6, -80(%rbp)                              #
        movdqa    %xmm7, -192(%rbp)                             #
        movl      %eax, %r13d                                   #
        jmp       ..B13.11      # Prob 100%                     #
                                # LOE rbx ecx r12d r13d r14d r15d xmm0
..B13.7:                        # Preds ..B13.3
        movdqa    %xmm8, -96(%rbp)                              #
        movdqa    %xmm1, -112(%rbp)                             #
        movdqa    %xmm2, -160(%rbp)                             #
        movdqa    %xmm3, -176(%rbp)                             #
        movdqa    %xmm4, -144(%rbp)                             #
        movdqa    %xmm5, -128(%rbp)                             #
        movdqa    %xmm6, -80(%rbp)                              #
        movdqa    %xmm7, -192(%rbp)                             #
        movl      %eax, %r13d                                   #
        jmp       ..B13.10      # Prob 100%                     #
                                # LOE rbx ecx r12d r13d r14d xmm0
..B13.8:                        # Preds ..B13.2
        movdqa    %xmm8, -96(%rbp)                              #
        movdqa    %xmm1, -112(%rbp)                             #
        movdqa    %xmm2, -160(%rbp)                             #
        movdqa    %xmm3, -176(%rbp)                             #
        movdqa    %xmm4, -144(%rbp)                             #
        movdqa    %xmm5, -128(%rbp)                             #
        movdqa    %xmm6, -80(%rbp)                              #
        movdqa    %xmm7, -192(%rbp)                             #
        movl      %eax, %r13d                                   #
                                # LOE rbx r12d r13d r14d xmm0
..B13.9:                        # Preds ..B13.8 ..B13.28
..LN1627:
   .loc    1  2291
        movdqa    %xmm0, %xmm1                                  #2291.22
        pand      -144(%rbp), %xmm1                             #2291.22
..LN1629:
   .loc    1  2292
        pcmpeqb   -128(%rbp), %xmm1                             #2292.22
..LN1631:
   .loc    1  2293
        pmovmskb  %xmm1, %ecx                                   #2293.22
                                # LOE rbx ecx r12d r13d r14d xmm0
..B13.10:                       # Preds ..B13.7 ..B13.9
..LN1633:
   .loc    1  2297
        movdqa    %xmm0, %xmm1                                  #2297.22
        pand      -160(%rbp), %xmm1                             #2297.22
..LN1635:
   .loc    1  2298
        pcmpeqb   -176(%rbp), %xmm1                             #2298.22
..LN1637:
   .loc    1  2299
        pmovmskb  %xmm1, %r15d                                  #2299.22
                                # LOE rbx ecx r12d r13d r14d r15d xmm0
..B13.11:                       # Preds ..B13.6 ..B13.10
..LN1639:
   .loc    1  2303
        pand      -96(%rbp), %xmm0                              #2303.22
..LN1641:
   .loc    1  2313
        movl      %r14d, %esi                                   #2313.29
..LN1643:
   .loc    1  2304
        pcmpeqb   -112(%rbp), %xmm0                             #2304.22
..LN1645:
   .loc    1  2305
        pmovmskb  %xmm0, %edx                                   #2305.22
..LN1647:
   .loc    1  2310
        andl      $-4, %r12d                                    #2310.17
..LN1649:
   .loc    1  2305
        movl      %edx, -64(%rbp)                               #2305.22
..LN1651:
   .loc    1  2313
        orl       %ecx, %esi                                    #2313.29
..LN1653:
        orl       %r15d, %esi                                   #2313.34
..LN1655:
        orl       %edx, %esi                                    #2313.39
..LN1657:
        je        ..B13.24      # Prob 78%                      #2313.47
                                # LOE rbx ecx r12d r13d r14d r15d
..B13.12:                       # Preds ..B13.11
..LN1659:
   .loc    1  2331
        movl      %ecx, -208(%rbp)                              #2331.30
        movl      %r14d, %edi                                   #2331.30
        call      uint16_lsbit@PLT                              #2331.30
                                # LOE rbx eax r12d r13d r14d r15d
..B13.42:                       # Preds ..B13.12
        movl      -208(%rbp), %ecx                              #
                                # LOE rcx rbx eax ecx r12d r13d r14d r15d cl ch
..B13.13:                       # Preds ..B13.42
..LN1661:
   .loc    1  2332
        movl      %ecx, -208(%rbp)                              #2332.30
..LN1663:
   .loc    1  2331
        movswq    %ax, %rdx                                     #2331.30
        movl      %edx, -216(%rbp)                              #2331.30
..LN1665:
   .loc    1  2332
        movl      %ecx, %edi                                    #2332.30
        call      uint16_lsbit@PLT                              #2332.30
                                # LOE rbx eax r12d r13d r14d r15d
..B13.43:                       # Preds ..B13.13
        movl      -208(%rbp), %ecx                              #
                                # LOE rcx rbx eax ecx r12d r13d r14d r15d cl ch
..B13.14:                       # Preds ..B13.43
..LN1667:
   .loc    1  2333
        movl      %ecx, -208(%rbp)                              #2333.30
..LN1669:
   .loc    1  2332
        movswq    %ax, %rdx                                     #2332.30
        movl      %edx, -224(%rbp)                              #2332.30
..LN1671:
   .loc    1  2333
        movl      %r15d, %edi                                   #2333.30
        call      uint16_lsbit@PLT                              #2333.30
                                # LOE rbx eax r12d r13d r14d r15d
..B13.44:                       # Preds ..B13.14
        movl      -208(%rbp), %ecx                              #
                                # LOE rcx rbx eax ecx r12d r13d r14d r15d cl ch
..B13.15:                       # Preds ..B13.44
..LN1673:
   .loc    1  2334
        movl      -64(%rbp), %edi                               #2334.30
        movl      %ecx, -208(%rbp)                              #2334.30
..LN1675:
   .loc    1  2333
        movswq    %ax, %rdx                                     #2333.30
..LN1677:
   .loc    1  2334
        movl      %edx, -200(%rbp)                              #2334.30
        call      uint16_lsbit@PLT                              #2334.30
                                # LOE rbx eax r12d r13d r14d r15d
..B13.45:                       # Preds ..B13.15
        movl      -208(%rbp), %ecx                              #
        movl      -200(%rbp), %edx                              #
                                # LOE rdx rcx rbx eax edx ecx r12d r13d r14d r15d dl cl dh ch
..B13.16:                       # Preds ..B13.45
..LN1679:
   .loc    1  2339
        movl      -216(%rbp), %esi                              #2339.34
..LN1681:
   .loc    1  2340
        movl      -224(%rbp), %r8d                              #2340.34
..LN1683:
   .loc    1  2334
        movswq    %ax, %rax                                     #2334.30
..LN1685:
   .loc    1  2339
        shll      $2, %esi                                      #2339.34
..LN1687:
   .loc    1  2340
        shll      $2, %r8d                                      #2340.34
..LN1689:
   .loc    1  2339
        movl      %esi, -216(%rbp)                              #2339.34
..LN1691:
   .loc    1  2340
        movl      %r8d, -224(%rbp)                              #2340.34
..LN1693:
   .loc    1  2341
        shll      $2, %edx                                      #2341.34
..LN1695:
   .loc    1  2342
        shll      $2, %eax                                      #2342.34
..LN1697:
   .loc    1  2346
        testl     %r14d, %r14d                                  #2346.32
        je        ..B13.18      # Prob 50%                      #2346.32
                                # LOE rcx rbx rsi r8 eax edx ecx esi r8d r12d r13d r15d cl sil r8b ch
..B13.17:                       # Preds ..B13.16
..LN1699:
        addl      %r12d, %esi                                   #2346.43
..LN1701:
        cmpl      %esi, %r13d                                   #2346.49
        jae       ..B13.33      # Prob 20%                      #2346.49
                                # LOE rcx rbx r8 eax edx ecx r8d r12d r13d r15d cl r8b ch
..B13.18:                       # Preds ..B13.17 ..B13.16
..LN1703:
   .loc    1  2347
        testl     %ecx, %ecx                                    #2347.32
        je        ..B13.20      # Prob 50%                      #2347.32
                                # LOE rbx r8 eax edx r8d r12d r13d r15d r8b
..B13.19:                       # Preds ..B13.18
..LN1705:
        movl      %r8d, %ecx                                    #2347.43
        lea       1(%r12,%rcx), %esi                            #2347.43
..LN1707:
        cmpl      %esi, %r13d                                   #2347.49
        jae       ..B13.33      # Prob 20%                      #2347.49
                                # LOE rbx eax edx r12d r13d r15d
..B13.20:                       # Preds ..B13.19 ..B13.18
..LN1709:
   .loc    1  2348
        testl     %r15d, %r15d                                  #2348.32
        je        ..B13.22      # Prob 50%                      #2348.32
                                # LOE rbx eax edx r12d r13d
..B13.21:                       # Preds ..B13.20
..LN1711:
        lea       2(%r12,%rdx), %edx                            #2348.43
..LN1713:
        cmpl      %edx, %r13d                                   #2348.49
        jae       ..B13.33      # Prob 20%                      #2348.49
                                # LOE rbx eax r12d r13d
..B13.22:                       # Preds ..B13.21 ..B13.20
..LN1715:
   .loc    1  2349
        movl      -64(%rbp), %edx                               #2349.32
        testl     %edx, %edx                                    #2349.32
        je        ..B13.24      # Prob 50%                      #2349.32
                                # LOE rbx eax r12d r13d
..B13.23:                       # Preds ..B13.22
..LN1717:
        lea       3(%r12,%rax), %edx                            #2349.43
..LN1719:
        cmpl      %edx, %r13d                                   #2349.49
        jae       ..B13.33      # Prob 20%                      #2349.49
                                # LOE rbx r12d r13d
..B13.24:                       # Preds ..B13.23 ..B13.22 ..B13.11
..LN1721:
   .loc    1  2386
        lea       4(%r12), %edx                                 #2386.13
..LN1723:
   .loc    1  2357
        cmpl      %edx, %r13d                                   #2357.28
        jb        ..B13.30      # Prob 20%                      #2357.28
                                # LOE rbx r12d r13d
..B13.25:                       # Preds ..B13.24
..LN1725:
   .loc    1  2382
        cmpq      -16(%rbp), %rbx                               #2382.25
        jae       ..B13.30      # Prob 4%                       #2382.25
                                # LOE rbx r12d r13d
..B13.26:                       # Preds ..B13.25
..LN1727:
   .loc    1  2386
        addl      $64, %r12d                                    #2386.13
..LN1729:
   .loc    1  2387
        cmpl      %r13d, %r12d                                  #2387.24
        ja        ..B13.30      # Prob 4%                       #2387.24
                                # LOE rbx r12d r13d
..B13.27:                       # Preds ..B13.26
..LN1731:
   .loc    1  2392
        movq      -16(%rbp), %rsi                               #2392.22
        movq      %rbx, %rdi                                    #2392.22
        call      prime_buffer_2na@PLT                          #2392.22
                                # LOE rbx r12d r13d xmm0
..B13.46:                       # Preds ..B13.27
..LN1733:
   .loc    1  2470
        addq      $16, %rbx                                     #2470.13
                                # LOE rbx r12d r13d xmm0
..B13.28:                       # Preds ..B13.5 ..B13.46
..LN1735:
   .loc    1  2285
        movdqa    %xmm0, %xmm1                                  #2285.22
        pand      -80(%rbp), %xmm1                              #2285.22
..LN1737:
   .loc    1  2286
        pcmpeqb   -192(%rbp), %xmm1                             #2286.22
..LN1739:
   .loc    1  2287
        pmovmskb  %xmm1, %r14d                                  #2287.22
        jmp       ..B13.9       # Prob 100%                     #2287.22
                                # LOE rbx r12d r13d r14d xmm0
..B13.30:                       # Preds ..B13.24 ..B13.25 ..B13.26 # Infreq
..LN1741:
   .loc    1  2483
        movq      -56(%rbp), %rbx                               #2483.12
..___tag_value_eval_2na_8.525:                                  #
        movq      -24(%rbp), %r12                               #2483.12
..___tag_value_eval_2na_8.526:                                  #
        movq      -32(%rbp), %r13                               #2483.12
..___tag_value_eval_2na_8.527:                                  #
        movq      -40(%rbp), %r14                               #2483.12
..___tag_value_eval_2na_8.528:                                  #
        xorl      %eax, %eax                                    #2483.12
        movq      -48(%rbp), %r15                               #2483.12
..___tag_value_eval_2na_8.529:                                  #
        movq      %rbp, %rsp                                    #2483.12
        popq      %rbp                                          #2483.12
..___tag_value_eval_2na_8.530:                                  #
        ret                                                     #2483.12
..___tag_value_eval_2na_8.531:                                  #
                                # LOE
..B13.33:                       # Preds ..B13.23 ..B13.21 ..B13.19 ..B13.17 # Infreq
..LN1743:
   .loc    1  2346
        movq      -56(%rbp), %rbx                               #2346.63
..___tag_value_eval_2na_8.537:                                  #
        movq      -24(%rbp), %r12                               #2346.63
..___tag_value_eval_2na_8.538:                                  #
        movq      -32(%rbp), %r13                               #2346.63
..___tag_value_eval_2na_8.539:                                  #
        movq      -40(%rbp), %r14                               #2346.63
..___tag_value_eval_2na_8.540:                                  #
        movl      $1, %eax                                      #2346.63
        movq      -48(%rbp), %r15                               #2346.63
..___tag_value_eval_2na_8.541:                                  #
        movq      %rbp, %rsp                                    #2346.63
        popq      %rbp                                          #2346.63
..___tag_value_eval_2na_8.542:                                  #
        ret                                                     #2346.63
        .align    16,0x90
..___tag_value_eval_2na_8.543:                                  #
                                # LOE
# mark_end;
	.type	eval_2na_8,@function
	.size	eval_2na_8,.-eval_2na_8
.LNeval_2na_8:
	.data
# -- End  eval_2na_8
	.text
# -- Begin  eval_4na_64
# mark_begin;
       .align    16,0x90
eval_4na_64:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B14.1:                        # Preds ..B14.0
..___tag_value_eval_4na_64.544:                                 #4501.1
..LN1745:
   .loc    1  4501
        pushq     %rbp                                          #4501.1
        movq      %rsp, %rbp                                    #4501.1
..___tag_value_eval_4na_64.545:                                 #
        subq      $256, %rsp                                    #4501.1
        movq      %r15, -184(%rbp)                              #4501.1
..___tag_value_eval_4na_64.547:                                 #
        movq      %r14, -24(%rbp)                               #4501.1
..___tag_value_eval_4na_64.548:                                 #
        movq      %r13, -32(%rbp)                               #4501.1
..___tag_value_eval_4na_64.549:                                 #
        movq      %r12, -48(%rbp)                               #4501.1
..___tag_value_eval_4na_64.550:                                 #
        movq      %rbx, -40(%rbp)                               #4501.1
..___tag_value_eval_4na_64.551:                                 #
        movl      %ecx, %ebx                                    #4501.1
        movl      %edx, %r12d                                   #4501.1
        movq      %rsi, %r15                                    #4501.1
        movq      %rdi, %r13                                    #4501.1
        lea       _gprof_pack13(%rip), %rdx                     #4501.1
        call      mcount@PLT                                    #4501.1
                                # LOE r13 r15 ebx r12d
..B14.45:                       # Preds ..B14.1
..LN1747:
   .loc    1  4535
        lea       (%r12,%rbx), %ecx                             #4535.5
..LN1749:
   .loc    1  4541
        subl      4(%r13), %ecx                                 #4541.12
..LN1751:
   .loc    1  4547
        movl      %ecx, -8(%rbp)                                #4547.14
..LN1753:
   .loc    1  4538
        movl      %r12d, %r14d                                  #4538.49
        shrl      $2, %r14d                                     #4538.49
..LN1755:
        lea       (%r15,%r14), %rdi                             #4538.30
..LN1757:
   .loc    1  4544
        lea       3(%r12,%rbx), %esi                            #4544.50
..LN1759:
        shrl      $2, %esi                                      #4544.57
..LN1761:
        addq      %r15, %rsi                                    #4544.30
        movq      %rsi, -16(%rbp)                               #4544.30
..LN1763:
   .loc    1  4547
        call      prime_buffer_4na@PLT                          #4547.14
                                # LOE r13 r14 r15 r12d xmm0
..B14.46:                       # Preds ..B14.45
        movl      -8(%rbp), %ecx                                #
..LN1765:
   .loc    1  4554
        movdqa    16(%r13), %xmm7                               #4554.5
        movdqa    32(%r13), %xmm6                               #4554.5
        movdqa    48(%r13), %xmm5                               #4554.5
        movdqa    64(%r13), %xmm4                               #4554.5
        movdqa    80(%r13), %xmm3                               #4554.5
        movdqa    96(%r13), %xmm2                               #4554.5
        movdqa    112(%r13), %xmm1                              #4554.5
        movdqa    128(%r13), %xmm8                              #4554.5
..LN1767:
   .loc    1  4548
        lea       8(%r15,%r14), %rbx                            #4548.5
        movq      %rbx, %rdx                                    #4548.5
..LN1769:
   .loc    1  4557
        xorl      %r15d, %r15d                                  #4557.15
..LN1771:
        xorl      %r14d, %r14d                                  #4557.10
..LN1773:
        xorl      %r13d, %r13d                                  #4557.5
..LN1775:
   .loc    1  4563
        movl      $4, %esi                                      #4563.5
..LN1777:
   .loc    1  4568
        movl      %r12d, %edi                                   #4568.20
        andl      $3, %edi                                      #4568.20
        je        ..B14.37      # Prob 20%                      #4568.20
                                # LOE rdx rcx rbx ecx esi edi r12d r13d r14d r15d cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B14.2:                        # Preds ..B14.46
        cmpl      $1, %edi                                      #4568.20
        je        ..B14.8       # Prob 25%                      #4568.20
                                # LOE rdx rcx rbx ecx esi edi r12d r13d r14d r15d cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B14.3:                        # Preds ..B14.2
        cmpl      $2, %edi                                      #4568.20
        je        ..B14.7       # Prob 33%                      #4568.20
                                # LOE rdx rcx rbx ecx esi edi r12d r13d r14d r15d cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B14.4:                        # Preds ..B14.3
        cmpl      $3, %edi                                      #4568.20
        je        ..B14.6       # Prob 50%                      #4568.20
                                # LOE rdx rcx rbx ecx esi r12d r13d r14d r15d cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B14.5:                        # Preds ..B14.4
        movdqa    %xmm8, -80(%rbp)                              #
        movdqa    %xmm1, -112(%rbp)                             #
        movdqa    %xmm2, -96(%rbp)                              #
        movdqa    %xmm3, -64(%rbp)                              #
        movdqa    %xmm4, -176(%rbp)                             #
        movdqa    %xmm5, -160(%rbp)                             #
        movdqa    %xmm6, -144(%rbp)                             #
        movdqa    %xmm7, -128(%rbp)                             #
        movl      %r12d, %eax                                   #
        movl      %ecx, -8(%rbp)                                #
        jmp       ..B14.29      # Prob 100%                     #
                                # LOE rdx rbx eax xmm0
..B14.6:                        # Preds ..B14.4
        movdqa    %xmm8, -80(%rbp)                              #
        movdqa    %xmm1, -112(%rbp)                             #
        movdqa    %xmm2, -96(%rbp)                              #
        movdqa    %xmm3, -64(%rbp)                              #
        movdqa    %xmm4, -176(%rbp)                             #
        movdqa    %xmm5, -160(%rbp)                             #
        movdqa    %xmm6, -144(%rbp)                             #
        movdqa    %xmm7, -128(%rbp)                             #
        movl      %r12d, %eax                                   #
        movl      %ecx, -8(%rbp)                                #
        jmp       ..B14.11      # Prob 100%                     #
                                # LOE rdx rbx eax esi r13d r14d r15d xmm0
..B14.7:                        # Preds ..B14.3
        movdqa    %xmm8, -80(%rbp)                              #
        movdqa    %xmm1, -112(%rbp)                             #
        movdqa    %xmm2, -96(%rbp)                              #
        movdqa    %xmm3, -64(%rbp)                              #
        movdqa    %xmm4, -176(%rbp)                             #
        movdqa    %xmm5, -160(%rbp)                             #
        movdqa    %xmm6, -144(%rbp)                             #
        movdqa    %xmm7, -128(%rbp)                             #
        movl      %r12d, %eax                                   #
        movl      %ecx, -8(%rbp)                                #
        jmp       ..B14.10      # Prob 100%                     #
                                # LOE rdx rbx eax esi r13d r14d xmm0
..B14.8:                        # Preds ..B14.2
        movdqa    %xmm8, -80(%rbp)                              #
        movdqa    %xmm1, -112(%rbp)                             #
        movdqa    %xmm2, -96(%rbp)                              #
        movdqa    %xmm3, -64(%rbp)                              #
        movdqa    %xmm4, -176(%rbp)                             #
        movdqa    %xmm5, -160(%rbp)                             #
        movdqa    %xmm6, -144(%rbp)                             #
        movdqa    %xmm7, -128(%rbp)                             #
        movl      %r12d, %eax                                   #
        movl      %ecx, -8(%rbp)                                #
                                # LOE rdx rbx eax esi r13d xmm0
..B14.9:                        # Preds ..B14.8 ..B14.33
..LN1779:
   .loc    1  4592
        movdqa    %xmm0, %xmm2                                  #4592.22
..LN1781:
   .loc    1  4593
        movdqa    %xmm0, %xmm1                                  #4593.22
..LN1783:
   .loc    1  4592
        pand      -160(%rbp), %xmm2                             #4592.22
..LN1785:
   .loc    1  4593
        pand      -176(%rbp), %xmm1                             #4593.22
..LN1787:
   .loc    1  4594
        pcmpeqd   %xmm1, %xmm2                                  #4594.22
..LN1789:
   .loc    1  4595
        pmovmskb  %xmm2, %r14d                                  #4595.22
..LN1791:
   .loc    1  4596
        movl      %r14d, %ecx                                   #4596.17
        andl      $3855, %ecx                                   #4596.17
        shll      $4, %ecx                                      #4596.17
        andl      %ecx, %r14d                                   #4596.17
        movl      %r14d, %edi                                   #4596.17
        shrl      $4, %edi                                      #4596.17
        orl       %edi, %r14d                                   #4596.17
                                # LOE rdx rbx eax esi r13d r14d xmm0
..B14.10:                       # Preds ..B14.7 ..B14.9
..LN1793:
   .loc    1  4599
        movdqa    %xmm0, %xmm2                                  #4599.22
..LN1795:
   .loc    1  4600
        movdqa    %xmm0, %xmm1                                  #4600.22
..LN1797:
   .loc    1  4599
        pand      -64(%rbp), %xmm2                              #4599.22
..LN1799:
   .loc    1  4600
        pand      -96(%rbp), %xmm1                              #4600.22
..LN1801:
   .loc    1  4601
        pcmpeqd   %xmm1, %xmm2                                  #4601.22
..LN1803:
   .loc    1  4602
        pmovmskb  %xmm2, %r15d                                  #4602.22
..LN1805:
   .loc    1  4603
        movl      %r15d, %ecx                                   #4603.17
        andl      $3855, %ecx                                   #4603.17
        shll      $4, %ecx                                      #4603.17
        andl      %ecx, %r15d                                   #4603.17
        movl      %r15d, %edi                                   #4603.17
        shrl      $4, %edi                                      #4603.17
        orl       %edi, %r15d                                   #4603.17
                                # LOE rdx rbx eax esi r13d r14d r15d xmm0
..B14.11:                       # Preds ..B14.6 ..B14.10
..LN1807:
   .loc    1  4606
        movdqa    %xmm0, %xmm2                                  #4606.22
..LN1809:
   .loc    1  4607
        movdqa    %xmm0, %xmm1                                  #4607.22
..LN1811:
   .loc    1  4606
        pand      -112(%rbp), %xmm2                             #4606.22
..LN1813:
   .loc    1  4607
        pand      -80(%rbp), %xmm1                              #4607.22
..LN1815:
   .loc    1  4608
        pcmpeqd   %xmm1, %xmm2                                  #4608.22
..LN1817:
   .loc    1  4609
        pmovmskb  %xmm2, %r12d                                  #4609.22
..LN1819:
   .loc    1  4610
        movl      %r12d, %ecx                                   #4610.17
        andl      $3855, %ecx                                   #4610.17
        shll      $4, %ecx                                      #4610.17
        andl      %ecx, %r12d                                   #4610.17
        movl      %r12d, %edi                                   #4610.17
        shrl      $4, %edi                                      #4610.17
        orl       %edi, %r12d                                   #4610.17
..LN1821:
   .loc    1  4614
        movl      %eax, %ecx                                    #4614.17
        andl      $-4, %ecx                                     #4614.17
..LN1823:
   .loc    1  4617
        movl      %r13d, %eax                                   #4617.29
        orl       %r14d, %eax                                   #4617.29
..LN1825:
        orl       %r15d, %eax                                   #4617.34
..LN1827:
        orl       %r12d, %eax                                   #4617.39
..LN1829:
        je        ..B14.24      # Prob 78%                      #4617.47
                                # LOE rdx rbx ecx esi r12d r13d r14d r15d xmm0
..B14.12:                       # Preds ..B14.11
..LN1831:
   .loc    1  4635
        movdqa    %xmm0, -256(%rbp)                             #4635.30
        movl      %r13d, %edi                                   #4635.30
        movl      %esi, -216(%rbp)                              #4635.30
        movl      %ecx, -208(%rbp)                              #4635.30
        movq      %rdx, -200(%rbp)                              #4635.30
        call      uint16_lsbit@PLT                              #4635.30
                                # LOE rbx eax r12d r13d r14d r15d
..B14.47:                       # Preds ..B14.12
        movq      -200(%rbp), %rdx                              #
        movdqa    -256(%rbp), %xmm0                             #
        movl      -208(%rbp), %ecx                              #
        movl      -216(%rbp), %esi                              #
                                # LOE rdx rcx rbx rsi eax ecx esi r12d r13d r14d r15d cl sil ch xmm0
..B14.13:                       # Preds ..B14.47
..LN1833:
   .loc    1  4636
        movdqa    %xmm0, -256(%rbp)                             #4636.30
        movl      %r14d, %edi                                   #4636.30
        movl      %esi, -216(%rbp)                              #4636.30
..LN1835:
   .loc    1  4635
        movswq    %ax, %rax                                     #4635.30
        movl      %eax, -240(%rbp)                              #4635.30
..LN1837:
   .loc    1  4636
        movl      %ecx, -208(%rbp)                              #4636.30
        movq      %rdx, -200(%rbp)                              #4636.30
        call      uint16_lsbit@PLT                              #4636.30
                                # LOE rbx eax r12d r13d r14d r15d
..B14.48:                       # Preds ..B14.13
        movq      -200(%rbp), %rdx                              #
        movdqa    -256(%rbp), %xmm0                             #
        movl      -208(%rbp), %ecx                              #
        movl      -216(%rbp), %esi                              #
                                # LOE rdx rcx rbx rsi eax ecx esi r12d r13d r14d r15d cl sil ch xmm0
..B14.14:                       # Preds ..B14.48
..LN1839:
   .loc    1  4637
        movdqa    %xmm0, -256(%rbp)                             #4637.30
        movl      %r15d, %edi                                   #4637.30
        movl      %esi, -216(%rbp)                              #4637.30
..LN1841:
   .loc    1  4636
        movswq    %ax, %rax                                     #4636.30
        movl      %eax, -232(%rbp)                              #4636.30
..LN1843:
   .loc    1  4637
        movl      %ecx, -208(%rbp)                              #4637.30
        movq      %rdx, -200(%rbp)                              #4637.30
        call      uint16_lsbit@PLT                              #4637.30
                                # LOE rbx eax r12d r13d r14d r15d
..B14.49:                       # Preds ..B14.14
        movq      -200(%rbp), %rdx                              #
        movdqa    -256(%rbp), %xmm0                             #
        movl      -208(%rbp), %ecx                              #
        movl      -216(%rbp), %esi                              #
                                # LOE rdx rcx rbx rsi eax ecx esi r12d r13d r14d r15d cl sil ch xmm0
..B14.15:                       # Preds ..B14.49
..LN1845:
   .loc    1  4638
        movdqa    %xmm0, -256(%rbp)                             #4638.30
        movl      %r12d, %edi                                   #4638.30
        movl      %esi, -216(%rbp)                              #4638.30
..LN1847:
   .loc    1  4637
        movswq    %ax, %rax                                     #4637.30
        movl      %eax, -224(%rbp)                              #4637.30
..LN1849:
   .loc    1  4638
        movl      %ecx, -208(%rbp)                              #4638.30
        movq      %rdx, -200(%rbp)                              #4638.30
        call      uint16_lsbit@PLT                              #4638.30
                                # LOE rbx eax r12d r13d r14d r15d
..B14.50:                       # Preds ..B14.15
        movq      -200(%rbp), %rdx                              #
        movdqa    -256(%rbp), %xmm0                             #
        movl      -208(%rbp), %ecx                              #
        movl      -216(%rbp), %esi                              #
                                # LOE rdx rcx rbx rsi eax ecx esi r12d r13d r14d r15d cl sil ch xmm0
..B14.16:                       # Preds ..B14.50
..LN1851:
   .loc    1  4645
        movl      -224(%rbp), %r8d                              #4645.40
..LN1853:
   .loc    1  4638
        movswq    %ax, %rdi                                     #4638.30
..LN1855:
   .loc    1  4644
        movl      -232(%rbp), %eax                              #4644.40
..LN1857:
   .loc    1  4638
..LN1859:
   .loc    1  4644
        lea       1(%rax,%rax), %eax                            #4644.40
..LN1861:
   .loc    1  4645
        lea       2(%r8,%r8), %r9d                              #4645.40
..LN1863:
   .loc    1  4646
        lea       3(%rdi,%rdi), %r10d                           #4646.40
..LN1865:
   .loc    1  4650
        testl     %r13d, %r13d                                  #4650.32
        je        ..B14.18      # Prob 50%                      #4650.32
                                # LOE rdx rcx rbx rsi eax ecx esi r9d r10d r12d r14d r15d cl sil ch xmm0
..B14.17:                       # Preds ..B14.16
..LN1867:
        movl      -240(%rbp), %edi                              #4650.43
        lea       (%rcx,%rdi,2), %r8d                           #4650.43
..LN1869:
        cmpl      -8(%rbp), %r8d                                #4650.49
        jbe       ..B14.38      # Prob 20%                      #4650.49
                                # LOE rdx rcx rbx rsi eax ecx esi r9d r10d r12d r14d r15d cl sil ch xmm0
..B14.18:                       # Preds ..B14.17 ..B14.16
..LN1871:
   .loc    1  4651
        testl     %r14d, %r14d                                  #4651.32
        je        ..B14.20      # Prob 50%                      #4651.32
                                # LOE rdx rcx rbx rsi eax ecx esi r9d r10d r12d r15d cl sil ch xmm0
..B14.19:                       # Preds ..B14.18
..LN1873:
        addl      %ecx, %eax                                    #4651.43
..LN1875:
        cmpl      -8(%rbp), %eax                                #4651.49
        jbe       ..B14.38      # Prob 20%                      #4651.49
                                # LOE rdx rcx rbx rsi ecx esi r9d r10d r12d r15d cl sil ch xmm0
..B14.20:                       # Preds ..B14.19 ..B14.18
..LN1877:
   .loc    1  4652
        testl     %r15d, %r15d                                  #4652.32
        je        ..B14.22      # Prob 50%                      #4652.32
                                # LOE rdx rcx rbx rsi ecx esi r9d r10d r12d cl sil ch xmm0
..B14.21:                       # Preds ..B14.20
..LN1879:
        addl      %ecx, %r9d                                    #4652.43
..LN1881:
        cmpl      -8(%rbp), %r9d                                #4652.49
        jbe       ..B14.38      # Prob 20%                      #4652.49
                                # LOE rdx rcx rbx rsi ecx esi r10d r12d cl sil ch xmm0
..B14.22:                       # Preds ..B14.21 ..B14.20
..LN1883:
   .loc    1  4653
        testl     %r12d, %r12d                                  #4653.32
        je        ..B14.24      # Prob 50%                      #4653.32
                                # LOE rdx rcx rbx rsi ecx esi r10d cl sil ch xmm0
..B14.23:                       # Preds ..B14.22
..LN1885:
        addl      %ecx, %r10d                                   #4653.43
..LN1887:
        cmpl      -8(%rbp), %r10d                               #4653.49
        jbe       ..B14.38      # Prob 20%                      #4653.49
                                # LOE rdx rbx ecx esi xmm0
..B14.24:                       # Preds ..B14.23 ..B14.22 ..B14.11
..LN1889:
   .loc    1  4658
        lea       4(%rcx), %eax                                 #4658.17
..LN1891:
   .loc    1  4661
        cmpl      -8(%rbp), %eax                                #4661.28
        ja        ..B14.35      # Prob 20%                      #4661.28
                                # LOE rdx rbx eax ecx esi xmm0
..B14.25:                       # Preds ..B14.24
..LN1893:
   .loc    1  4665
        decl      %esi                                          #4665.25
..LN1895:
        jne       ..B14.30      # Prob 50%                      #4665.39
                                # LOE rdx rbx eax ecx esi xmm0
..B14.26:                       # Preds ..B14.25
..LN1897:
   .loc    1  4682
        cmpq      -16(%rbp), %rbx                               #4682.25
        jae       ..B14.35      # Prob 4%                       #4682.25
                                # LOE rbx ecx
..B14.27:                       # Preds ..B14.26
..LN1899:
   .loc    1  4686
        lea       20(%rcx), %eax                                #4686.13
..LN1901:
   .loc    1  4687
        cmpl      -8(%rbp), %eax                                #4687.24
        ja        ..B14.35      # Prob 4%                       #4687.24
                                # LOE rbx eax
..B14.28:                       # Preds ..B14.27
..LN1903:
   .loc    1  4692
        movq      -16(%rbp), %rsi                               #4692.22
        movl      %eax, -192(%rbp)                              #4692.22
        movq      %rbx, %rdi                                    #4692.22
        call      prime_buffer_4na@PLT                          #4692.22
                                # LOE rbx xmm0
..B14.51:                       # Preds ..B14.28
        movl      -192(%rbp), %eax                              #
..LN1905:
   .loc    1  4707
        addq      $8, %rbx                                      #4707.13
        movq      %rbx, %rdx                                    #4707.13
                                # LOE rdx rbx eax xmm0
..B14.29:                       # Preds ..B14.5 ..B14.51
..LN1907:
   .loc    1  4575
        movl      $4, %esi                                      #4575.13
        jmp       ..B14.33      # Prob 100%                     #4575.13
                                # LOE rdx rbx eax esi xmm0
..B14.30:                       # Preds ..B14.25
..LN1909:
   .loc    1  4673
        cmpq      -16(%rbp), %rdx                               #4673.26
..LN1911:
   .loc    1  4670
        psrldq    $2, %xmm0                                     #4670.26
..LN1913:
   .loc    1  4673
        jae       ..B14.32      # Prob 19%                      #4673.26
                                # LOE rdx rbx eax esi xmm0
..B14.31:                       # Preds ..B14.30
..LN1915:
   .loc    1  4674
        movzbl    (%rdx), %edi                                  #4674.72
..LN1917:
        lea       expand_2na.0(%rip), %rcx                      #4674.57
        movzwl    (%rcx,%rdi,2), %r8d                           #4674.57
..LN1919:
        pinsrw    $7, %r8d, %xmm0                               #4674.30
                                # LOE rdx rbx eax esi xmm0
..B14.32:                       # Preds ..B14.31 ..B14.30
..LN1921:
   .loc    1  4677
        incq      %rdx                                          #4677.20
                                # LOE rdx rbx eax esi xmm0
..B14.33:                       # Preds ..B14.29 ..B14.32 ..B14.37
..LN1923:
   .loc    1  4585
        movdqa    %xmm0, %xmm2                                  #4585.22
..LN1925:
   .loc    1  4586
        movdqa    %xmm0, %xmm1                                  #4586.22
..LN1927:
   .loc    1  4585
        pand      -128(%rbp), %xmm2                             #4585.22
..LN1929:
   .loc    1  4586
        pand      -144(%rbp), %xmm1                             #4586.22
..LN1931:
   .loc    1  4587
        pcmpeqd   %xmm1, %xmm2                                  #4587.22
..LN1933:
   .loc    1  4588
        pmovmskb  %xmm2, %r13d                                  #4588.22
..LN1935:
   .loc    1  4589
        movl      %r13d, %ecx                                   #4589.17
        andl      $3855, %ecx                                   #4589.17
        shll      $4, %ecx                                      #4589.17
        andl      %ecx, %r13d                                   #4589.17
        movl      %r13d, %edi                                   #4589.17
        shrl      $4, %edi                                      #4589.17
        orl       %edi, %r13d                                   #4589.17
        jmp       ..B14.9       # Prob 100%                     #4589.17
                                # LOE rdx rbx eax esi r13d xmm0
..B14.35:                       # Preds ..B14.24 ..B14.26 ..B14.27 # Infreq
..LN1937:
   .loc    1  4718
        movq      -40(%rbp), %rbx                               #4718.12
..___tag_value_eval_4na_64.552:                                 #
        movq      -48(%rbp), %r12                               #4718.12
..___tag_value_eval_4na_64.553:                                 #
        movq      -32(%rbp), %r13                               #4718.12
..___tag_value_eval_4na_64.554:                                 #
        movq      -24(%rbp), %r14                               #4718.12
..___tag_value_eval_4na_64.555:                                 #
        xorl      %eax, %eax                                    #4718.12
        movq      -184(%rbp), %r15                              #4718.12
..___tag_value_eval_4na_64.556:                                 #
        movq      %rbp, %rsp                                    #4718.12
        popq      %rbp                                          #4718.12
..___tag_value_eval_4na_64.557:                                 #
        ret                                                     #4718.12
..___tag_value_eval_4na_64.558:                                 #
                                # LOE
..B14.37:                       # Preds ..B14.46                # Infreq
        movdqa    %xmm8, -80(%rbp)                              #
        movdqa    %xmm1, -112(%rbp)                             #
        movdqa    %xmm2, -96(%rbp)                              #
        movdqa    %xmm3, -64(%rbp)                              #
        movdqa    %xmm4, -176(%rbp)                             #
        movdqa    %xmm5, -160(%rbp)                             #
        movdqa    %xmm6, -144(%rbp)                             #
        movdqa    %xmm7, -128(%rbp)                             #
        movl      %r12d, %eax                                   #
        movl      %ecx, -8(%rbp)                                #
        jmp       ..B14.33      # Prob 100%                     #
                                # LOE rdx rbx eax esi xmm0
..B14.38:                       # Preds ..B14.17 ..B14.23 ..B14.21 ..B14.19 # Infreq
..LN1939:
   .loc    1  4651
        movq      -40(%rbp), %rbx                               #4651.63
..___tag_value_eval_4na_64.564:                                 #
        movq      -48(%rbp), %r12                               #4651.63
..___tag_value_eval_4na_64.565:                                 #
        movq      -32(%rbp), %r13                               #4651.63
..___tag_value_eval_4na_64.566:                                 #
        movq      -24(%rbp), %r14                               #4651.63
..___tag_value_eval_4na_64.567:                                 #
        movl      $1, %eax                                      #4651.63
        movq      -184(%rbp), %r15                              #4651.63
..___tag_value_eval_4na_64.568:                                 #
        movq      %rbp, %rsp                                    #4651.63
        popq      %rbp                                          #4651.63
..___tag_value_eval_4na_64.569:                                 #
        ret                                                     #4651.63
        .align    16,0x90
..___tag_value_eval_4na_64.570:                                 #
                                # LOE
# mark_end;
	.type	eval_4na_64,@function
	.size	eval_4na_64,.-eval_4na_64
.LNeval_4na_64:
	.data
# -- End  eval_4na_64
	.text
# -- Begin  eval_2na_64
# mark_begin;
       .align    16,0x90
eval_2na_64:
# parameter 1(self): %rdi
# parameter 2(ncbi2na): %rsi
# parameter 3(pos): %edx
# parameter 4(len): %ecx
..B15.1:                        # Preds ..B15.0
..___tag_value_eval_2na_64.571:                                 #3075.1
..LN1941:
   .loc    1  3075
        pushq     %rbp                                          #3075.1
        movq      %rsp, %rbp                                    #3075.1
..___tag_value_eval_2na_64.572:                                 #
        subq      $272, %rsp                                    #3075.1
        movq      %r15, -56(%rbp)                               #3075.1
..___tag_value_eval_2na_64.574:                                 #
        movq      %r14, -48(%rbp)                               #3075.1
..___tag_value_eval_2na_64.575:                                 #
        movq      %r13, -24(%rbp)                               #3075.1
..___tag_value_eval_2na_64.576:                                 #
        movq      %r12, -40(%rbp)                               #3075.1
..___tag_value_eval_2na_64.577:                                 #
        movq      %rbx, -32(%rbp)                               #3075.1
..___tag_value_eval_2na_64.578:                                 #
        movl      %ecx, %r13d                                   #3075.1
        movl      %edx, %ebx                                    #3075.1
        movq      %rsi, %r14                                    #3075.1
        movq      %rdi, %r12                                    #3075.1
        lea       _gprof_pack14(%rip), %rdx                     #3075.1
        call      mcount@PLT                                    #3075.1
                                # LOE r12 r14 ebx r13d
..B15.48:                       # Preds ..B15.1
..LN1943:
   .loc    1  3113
        lea       (%rbx,%r13), %r8d                             #3113.5
..LN1945:
   .loc    1  3119
        subl      4(%r12), %r8d                                 #3119.12
..LN1947:
   .loc    1  3125
        movl      %r8d, -16(%rbp)                               #3125.14
..LN1949:
   .loc    1  3116
        movl      %ebx, %r15d                                   #3116.49
        shrl      $2, %r15d                                     #3116.49
..LN1951:
        lea       (%r14,%r15), %rdi                             #3116.30
..LN1953:
   .loc    1  3122
        lea       3(%rbx,%r13), %r13d                           #3122.50
..LN1955:
        shrl      $2, %r13d                                     #3122.57
..LN1957:
        addq      %r14, %r13                                    #3122.30
..LN1959:
   .loc    1  3125
        movq      %r13, %rsi                                    #3125.14
        call      prime_buffer_2na@PLT                          #3125.14
                                # LOE r12 r13 r14 r15 ebx xmm0
..B15.49:                       # Preds ..B15.48
        movl      -16(%rbp), %r8d                               #
..LN1961:
   .loc    1  3126
        lea       16(%r14,%r15), %r14                           #3126.5
        movq      %r14, %rcx                                    #3126.5
..LN1963:
   .loc    1  3131
        cmpq      %r13, %r14                                    #3131.16
..LN1965:
   .loc    1  3136
..LN1967:
   .loc    1  3131
        jae       ..B15.3       # Prob 12%                      #3131.16
                                # LOE rcx r8 r12 r13 r14 ebx r8d r15d r8b xmm0
..B15.2:                        # Preds ..B15.49
..LN1969:
   .loc    1  3132
        movzbl    -1(%r14), %r15d                               #3132.24
..LN1971:
        shll      $8, %r15d                                     #3132.38
                                # LOE rcx r8 r12 r13 r14 ebx r8d r15d r8b xmm0
..B15.3:                        # Preds ..B15.2 ..B15.49
..LN1973:
   .loc    1  3136
        movdqa    16(%r12), %xmm7                               #3136.5
        movdqa    32(%r12), %xmm6                               #3136.5
        movdqa    48(%r12), %xmm5                               #3136.5
        movdqa    64(%r12), %xmm4                               #3136.5
        movdqa    80(%r12), %xmm3                               #3136.5
        movdqa    96(%r12), %xmm2                               #3136.5
        movdqa    112(%r12), %xmm1                              #3136.5
        movdqa    128(%r12), %xmm8                              #3136.5
..LN1975:
   .loc    1  3139
        xorl      %esi, %esi                                    #3139.15
..LN1977:
        xorl      %eax, %eax                                    #3139.10
..LN1979:
        xorl      %r12d, %r12d                                  #3139.5
..LN1981:
   .loc    1  3143
        movl      $8, %edx                                      #3143.5
..LN1983:
   .loc    1  3148
        movl      %ebx, %edi                                    #3148.20
        andl      $3, %edi                                      #3148.20
        je        ..B15.40      # Prob 20%                      #3148.20
                                # LOE rcx r8 r13 r14 eax edx ebx esi edi r8d r12d r15d r8b xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B15.4:                        # Preds ..B15.3
        cmpl      $1, %edi                                      #3148.20
        je        ..B15.10      # Prob 25%                      #3148.20
                                # LOE rcx r8 r13 r14 eax edx ebx esi edi r8d r12d r15d r8b xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B15.5:                        # Preds ..B15.4
        cmpl      $2, %edi                                      #3148.20
        je        ..B15.9       # Prob 33%                      #3148.20
                                # LOE rcx r8 r13 r14 eax edx ebx esi edi r8d r12d r15d r8b xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B15.6:                        # Preds ..B15.5
        cmpl      $3, %edi                                      #3148.20
        je        ..B15.8       # Prob 50%                      #3148.20
                                # LOE rcx r8 r13 r14 eax edx ebx esi r8d r12d r15d r8b xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B15.7:                        # Preds ..B15.6
        movdqa    %xmm8, -128(%rbp)                             #
        movdqa    %xmm1, -112(%rbp)                             #
        movdqa    %xmm2, -96(%rbp)                              #
        movdqa    %xmm3, -160(%rbp)                             #
        movdqa    %xmm4, -192(%rbp)                             #
        movdqa    %xmm5, -176(%rbp)                             #
        movdqa    %xmm6, -144(%rbp)                             #
        movdqa    %xmm7, -80(%rbp)                              #
        movl      %ebx, %eax                                    #
        movq      %r13, -64(%rbp)                               #
        movl      %r8d, -16(%rbp)                               #
        jmp       ..B15.32      # Prob 100%                     #
                                # LOE rcx r14 eax r15d xmm0
..B15.8:                        # Preds ..B15.6
        movdqa    %xmm8, -128(%rbp)                             #
        movdqa    %xmm1, -112(%rbp)                             #
        movdqa    %xmm2, -96(%rbp)                              #
        movdqa    %xmm3, -160(%rbp)                             #
        movdqa    %xmm4, -192(%rbp)                             #
        movdqa    %xmm5, -176(%rbp)                             #
        movdqa    %xmm6, -144(%rbp)                             #
        movdqa    %xmm7, -80(%rbp)                              #
        movl      %eax, %edi                                    #
        movq      %r13, -64(%rbp)                               #
        movl      %r8d, -16(%rbp)                               #
        movl      %ebx, %eax                                    #
        movl      %edi, %ebx                                    #
        movl      %esi, %r13d                                   #
        jmp       ..B15.13      # Prob 100%                     #
                                # LOE rcx r14 eax edx ebx r12d r13d r15d xmm0
..B15.9:                        # Preds ..B15.5
        movdqa    %xmm8, -128(%rbp)                             #
        movdqa    %xmm1, -112(%rbp)                             #
        movdqa    %xmm2, -96(%rbp)                              #
        movdqa    %xmm3, -160(%rbp)                             #
        movdqa    %xmm4, -192(%rbp)                             #
        movdqa    %xmm5, -176(%rbp)                             #
        movdqa    %xmm6, -144(%rbp)                             #
        movdqa    %xmm7, -80(%rbp)                              #
        movl      %eax, %esi                                    #
        movq      %r13, -64(%rbp)                               #
        movl      %r8d, -16(%rbp)                               #
        movl      %ebx, %eax                                    #
        movl      %esi, %ebx                                    #
        jmp       ..B15.12      # Prob 100%                     #
                                # LOE rcx r14 eax edx ebx r12d r15d xmm0
..B15.10:                       # Preds ..B15.4
        movdqa    %xmm8, -128(%rbp)                             #
        movdqa    %xmm1, -112(%rbp)                             #
        movdqa    %xmm2, -96(%rbp)                              #
        movdqa    %xmm3, -160(%rbp)                             #
        movdqa    %xmm4, -192(%rbp)                             #
        movdqa    %xmm5, -176(%rbp)                             #
        movdqa    %xmm6, -144(%rbp)                             #
        movdqa    %xmm7, -80(%rbp)                              #
        movl      %ebx, %eax                                    #
        movq      %r13, -64(%rbp)                               #
        movl      %r8d, -16(%rbp)                               #
                                # LOE rcx r14 eax edx r12d r15d xmm0
..B15.11:                       # Preds ..B15.10 ..B15.36
..LN1985:
   .loc    1  3171
        movdqa    %xmm0, %xmm1                                  #3171.22
        pand      -192(%rbp), %xmm1                             #3171.22
..LN1987:
   .loc    1  3172
        pcmpeqd   -176(%rbp), %xmm1                             #3172.22
..LN1989:
   .loc    1  3173
        pmovmskb  %xmm1, %ebx                                   #3173.22
..LN1991:
   .loc    1  3174
        movl      %ebx, %esi                                    #3174.17
        andl      $3855, %esi                                   #3174.17
        shll      $4, %esi                                      #3174.17
        andl      %esi, %ebx                                    #3174.17
        movl      %ebx, %edi                                    #3174.17
        shrl      $4, %edi                                      #3174.17
        orl       %edi, %ebx                                    #3174.17
                                # LOE rcx r14 eax edx ebx r12d r15d xmm0
..B15.12:                       # Preds ..B15.9 ..B15.11
..LN1993:
   .loc    1  3177
        movdqa    %xmm0, %xmm1                                  #3177.22
        pand      -96(%rbp), %xmm1                              #3177.22
..LN1995:
   .loc    1  3178
        pcmpeqd   -160(%rbp), %xmm1                             #3178.22
..LN1997:
   .loc    1  3179
        pmovmskb  %xmm1, %r13d                                  #3179.22
..LN1999:
   .loc    1  3180
        movl      %r13d, %edi                                   #3180.17
        andl      $3855, %edi                                   #3180.17
        shll      $4, %edi                                      #3180.17
        andl      %edi, %r13d                                   #3180.17
        movl      %r13d, %r8d                                   #3180.17
        shrl      $4, %r8d                                      #3180.17
        orl       %r8d, %r13d                                   #3180.17
                                # LOE rcx r14 eax edx ebx r12d r13d r15d xmm0
..B15.13:                       # Preds ..B15.8 ..B15.12
..LN2001:
   .loc    1  3183
        movdqa    %xmm0, %xmm1                                  #3183.22
        pand      -128(%rbp), %xmm1                             #3183.22
..LN2003:
   .loc    1  3184
        pcmpeqd   -112(%rbp), %xmm1                             #3184.22
..LN2005:
   .loc    1  3185
        pmovmskb  %xmm1, %r8d                                   #3185.22
..LN2007:
   .loc    1  3190
        movl      %eax, %esi                                    #3190.17
        andl      $-4, %esi                                     #3190.17
..LN2009:
   .loc    1  3186
        movl      %r8d, %r9d                                    #3186.17
        andl      $3855, %r9d                                   #3186.17
        shll      $4, %r9d                                      #3186.17
        andl      %r9d, %r8d                                    #3186.17
        movl      %r8d, %r10d                                   #3186.17
        shrl      $4, %r10d                                     #3186.17
        orl       %r10d, %r8d                                   #3186.17
..LN2011:
   .loc    1  3193
        movl      %r12d, %r11d                                  #3193.29
        orl       %ebx, %r11d                                   #3193.29
..LN2013:
        orl       %r13d, %r11d                                  #3193.34
..LN2015:
        orl       %r8d, %r11d                                   #3193.39
..LN2017:
        je        ..B15.26      # Prob 78%                      #3193.47
                                # LOE rcx r14 edx ebx esi r8d r12d r13d r15d xmm0
..B15.14:                       # Preds ..B15.13
..LN2019:
   .loc    1  3211
        movdqa    %xmm0, -272(%rbp)                             #3211.30
        movl      %r12d, %edi                                   #3211.30
        movl      %edx, -232(%rbp)                              #3211.30
        movl      %esi, -224(%rbp)                              #3211.30
        movl      %r8d, -216(%rbp)                              #3211.30
        movq      %rcx, -208(%rbp)                              #3211.30
        call      uint16_lsbit@PLT                              #3211.30
                                # LOE r14 eax ebx r12d r13d r15d
..B15.50:                       # Preds ..B15.14
        movq      -208(%rbp), %rcx                              #
        movdqa    -272(%rbp), %xmm0                             #
        movl      -216(%rbp), %r8d                              #
        movl      -224(%rbp), %esi                              #
        movl      -232(%rbp), %edx                              #
                                # LOE rdx rcx rsi r8 r14 eax edx ebx esi r8d r12d r13d r15d dl sil r8b dh xmm0
..B15.15:                       # Preds ..B15.50
..LN2021:
   .loc    1  3212
        movdqa    %xmm0, -272(%rbp)                             #3212.30
        movl      %ebx, %edi                                    #3212.30
        movl      %edx, -232(%rbp)                              #3212.30
..LN2023:
   .loc    1  3211
        movswq    %ax, %r9                                      #3211.30
        movl      %r9d, -240(%rbp)                              #3211.30
..LN2025:
   .loc    1  3212
        movl      %esi, -224(%rbp)                              #3212.30
        movl      %r8d, -216(%rbp)                              #3212.30
        movq      %rcx, -208(%rbp)                              #3212.30
        call      uint16_lsbit@PLT                              #3212.30
                                # LOE r14 eax ebx r12d r13d r15d
..B15.51:                       # Preds ..B15.15
        movq      -208(%rbp), %rcx                              #
        movdqa    -272(%rbp), %xmm0                             #
        movl      -216(%rbp), %r8d                              #
        movl      -224(%rbp), %esi                              #
        movl      -232(%rbp), %edx                              #
                                # LOE rdx rcx rsi r8 r14 eax edx ebx esi r8d r12d r13d r15d dl sil r8b dh xmm0
..B15.16:                       # Preds ..B15.51
..LN2027:
   .loc    1  3213
        movdqa    %xmm0, -272(%rbp)                             #3213.30
        movl      %r13d, %edi                                   #3213.30
        movl      %edx, -232(%rbp)                              #3213.30
..LN2029:
   .loc    1  3212
        movswq    %ax, %r9                                      #3212.30
        movl      %r9d, -248(%rbp)                              #3212.30
..LN2031:
   .loc    1  3213
        movl      %esi, -224(%rbp)                              #3213.30
        movl      %r8d, -216(%rbp)                              #3213.30
        movq      %rcx, -208(%rbp)                              #3213.30
        call      uint16_lsbit@PLT                              #3213.30
                                # LOE r14 eax ebx r12d r13d r15d
..B15.52:                       # Preds ..B15.16
        movq      -208(%rbp), %rcx                              #
        movdqa    -272(%rbp), %xmm0                             #
        movl      -216(%rbp), %r8d                              #
        movl      -224(%rbp), %esi                              #
        movl      -232(%rbp), %edx                              #
                                # LOE rdx rcx rsi r8 r14 eax edx ebx esi r8d r12d r13d r15d dl sil r8b dh xmm0
..B15.17:                       # Preds ..B15.52
..LN2033:
   .loc    1  3214
        movdqa    %xmm0, -272(%rbp)                             #3214.30
        movl      %r8d, %edi                                    #3214.30
        movl      %edx, -232(%rbp)                              #3214.30
..LN2035:
   .loc    1  3213
        movswq    %ax, %r9                                      #3213.30
        movl      %r9d, -256(%rbp)                              #3213.30
..LN2037:
   .loc    1  3214
        movl      %esi, -224(%rbp)                              #3214.30
        movl      %r8d, -216(%rbp)                              #3214.30
        movq      %rcx, -208(%rbp)                              #3214.30
        call      uint16_lsbit@PLT                              #3214.30
                                # LOE r14 eax ebx r12d r13d r15d
..B15.53:                       # Preds ..B15.17
        movq      -208(%rbp), %rcx                              #
        movdqa    -272(%rbp), %xmm0                             #
        movl      -216(%rbp), %r8d                              #
        movl      -224(%rbp), %esi                              #
        movl      -232(%rbp), %edx                              #
                                # LOE rdx rcx rsi r8 r14 eax edx ebx esi r8d r12d r13d r15d dl sil r8b dh xmm0
..B15.18:                       # Preds ..B15.53
..LN2039:
   .loc    1  3219
        movl      -240(%rbp), %r9d                              #3219.34
..LN2041:
   .loc    1  3220
        movl      -248(%rbp), %r10d                             #3220.34
..LN2043:
   .loc    1  3221
        movl      -256(%rbp), %r11d                             #3221.34
..LN2045:
   .loc    1  3214
        movswq    %ax, %rax                                     #3214.30
..LN2047:
   .loc    1  3219
        shll      $2, %r9d                                      #3219.34
        movl      %r9d, -240(%rbp)                              #3219.34
..LN2049:
   .loc    1  3220
        shll      $2, %r10d                                     #3220.34
        movl      %r10d, -248(%rbp)                             #3220.34
..LN2051:
   .loc    1  3221
        shll      $2, %r11d                                     #3221.34
        movl      %r11d, -256(%rbp)                             #3221.34
..LN2053:
   .loc    1  3222
        shll      $2, %eax                                      #3222.34
..LN2055:
   .loc    1  3226
        testl     %r12d, %r12d                                  #3226.32
        je        ..B15.20      # Prob 50%                      #3226.32
                                # LOE rdx rcx rsi r8 r9 r10 r11 r14 eax edx ebx esi r8d r9d r10d r11d r13d r15d dl sil r8b r9b r10b r11b dh xmm0
..B15.19:                       # Preds ..B15.18
..LN2057:
        addl      %esi, %r9d                                    #3226.43
..LN2059:
        cmpl      -16(%rbp), %r9d                               #3226.49
        jbe       ..B15.41      # Prob 20%                      #3226.49
                                # LOE rdx rcx rsi r8 r10 r11 r14 eax edx ebx esi r8d r10d r11d r13d r15d dl sil r8b r10b r11b dh xmm0
..B15.20:                       # Preds ..B15.19 ..B15.18
..LN2061:
   .loc    1  3227
        testl     %ebx, %ebx                                    #3227.32
        je        ..B15.22      # Prob 50%                      #3227.32
                                # LOE rdx rcx rsi r8 r10 r11 r14 eax edx esi r8d r10d r11d r13d r15d dl sil r8b r10b r11b dh xmm0
..B15.21:                       # Preds ..B15.20
..LN2063:
        movl      %r10d, %ebx                                   #3227.43
        lea       1(%rsi,%rbx), %r9d                            #3227.43
..LN2065:
        cmpl      -16(%rbp), %r9d                               #3227.49
        jbe       ..B15.41      # Prob 20%                      #3227.49
                                # LOE rdx rcx rsi r8 r11 r14 eax edx esi r8d r11d r13d r15d dl sil r8b r11b dh xmm0
..B15.22:                       # Preds ..B15.21 ..B15.20
..LN2067:
   .loc    1  3228
        testl     %r13d, %r13d                                  #3228.32
        je        ..B15.24      # Prob 50%                      #3228.32
                                # LOE rdx rcx rsi r8 r11 r14 eax edx esi r8d r11d r15d dl sil r8b r11b dh xmm0
..B15.23:                       # Preds ..B15.22
..LN2069:
        movl      %r11d, %ebx                                   #3228.43
        lea       2(%rsi,%rbx), %r9d                            #3228.43
..LN2071:
        cmpl      -16(%rbp), %r9d                               #3228.49
        jbe       ..B15.41      # Prob 20%                      #3228.49
                                # LOE rdx rcx rsi r8 r14 eax edx esi r8d r15d dl sil r8b dh xmm0
..B15.24:                       # Preds ..B15.23 ..B15.22
..LN2073:
   .loc    1  3229
        testl     %r8d, %r8d                                    #3229.32
        je        ..B15.26      # Prob 50%                      #3229.32
                                # LOE rdx rcx rsi r14 eax edx esi r15d dl sil dh xmm0
..B15.25:                       # Preds ..B15.24
..LN2075:
        lea       3(%rsi,%rax), %eax                            #3229.43
..LN2077:
        cmpl      -16(%rbp), %eax                               #3229.49
        jbe       ..B15.41      # Prob 20%                      #3229.49
                                # LOE rcx r14 edx esi r15d xmm0
..B15.26:                       # Preds ..B15.25 ..B15.24 ..B15.13
..LN2079:
   .loc    1  3234
        lea       4(%rsi), %eax                                 #3234.17
..LN2081:
   .loc    1  3237
        cmpl      -16(%rbp), %eax                               #3237.28
        ja        ..B15.38      # Prob 20%                      #3237.28
                                # LOE rcx r14 eax edx esi r15d xmm0
..B15.27:                       # Preds ..B15.26
..LN2083:
   .loc    1  3241
        decl      %edx                                          #3241.25
..LN2085:
        jne       ..B15.33      # Prob 50%                      #3241.39
                                # LOE rcx r14 eax edx esi r15d xmm0
..B15.28:                       # Preds ..B15.27
..LN2087:
   .loc    1  3262
        cmpq      -64(%rbp), %r14                               #3262.25
        jae       ..B15.38      # Prob 4%                       #3262.25
                                # LOE r14 esi r15d
..B15.29:                       # Preds ..B15.28
..LN2089:
   .loc    1  3266
        lea       36(%rsi), %eax                                #3266.13
..LN2091:
   .loc    1  3267
        cmpl      -16(%rbp), %eax                               #3267.24
        ja        ..B15.38      # Prob 4%                       #3267.24
                                # LOE r14 eax r15d
..B15.30:                       # Preds ..B15.29
..LN2093:
   .loc    1  3272
        movq      -64(%rbp), %rsi                               #3272.22
        movl      %eax, -200(%rbp)                              #3272.22
        movq      %r14, %rdi                                    #3272.22
        call      prime_buffer_2na@PLT                          #3272.22
                                # LOE r14 r15d xmm0
..B15.54:                       # Preds ..B15.30
        movl      -200(%rbp), %eax                              #
..LN2095:
   .loc    1  3350
        addq      $16, %r14                                     #3350.13
..LN2097:
   .loc    1  3353
        cmpq      -64(%rbp), %r14                               #3353.24
..LN2099:
   .loc    1  3350
        movq      %r14, %rcx                                    #3350.13
..LN2101:
   .loc    1  3353
        jae       ..B15.32      # Prob 12%                      #3353.24
                                # LOE rax rcx r14 eax r15d al ah xmm0
..B15.31:                       # Preds ..B15.54
..LN2103:
   .loc    1  3354
        movzbl    -1(%r14), %r15d                               #3354.32
..LN2105:
        shll      $8, %r15d                                     #3354.46
                                # LOE rcx r14 eax r15d xmm0
..B15.32:                       # Preds ..B15.54 ..B15.7 ..B15.31
..LN2107:
   .loc    1  3155
        movl      $8, %edx                                      #3155.13
        jmp       ..B15.36      # Prob 100%                     #3155.13
                                # LOE rcx r14 eax edx r15d xmm0
..B15.33:                       # Preds ..B15.27
..LN2109:
   .loc    1  3249
        cmpq      -64(%rbp), %rcx                               #3249.26
..LN2111:
   .loc    1  3246
        psrldq    $1, %xmm0                                     #3246.26
..LN2113:
   .loc    1  3249
        jae       ..B15.35      # Prob 19%                      #3249.26
                                # LOE rcx r14 eax edx r15d xmm0
..B15.34:                       # Preds ..B15.33
..LN2115:
   .loc    1  3252
        movzbl    (%rcx), %esi                                  #3252.37
..LN2117:
   .loc    1  3251
        sarl      $8, %r15d                                     #3251.21
..LN2119:
   .loc    1  3252
        shll      $8, %esi                                      #3252.48
..LN2121:
        orl       %esi, %r15d                                   #3252.21
..LN2123:
   .loc    1  3253
        pinsrw    $7, %r15d, %xmm0                              #3253.30
                                # LOE rcx r14 eax edx r15d xmm0
..B15.35:                       # Preds ..B15.34 ..B15.33
..LN2125:
   .loc    1  3257
        incq      %rcx                                          #3257.20
                                # LOE rcx r14 eax edx r15d xmm0
..B15.36:                       # Preds ..B15.32 ..B15.35 ..B15.40
..LN2127:
   .loc    1  3165
        movdqa    %xmm0, %xmm1                                  #3165.22
        pand      -144(%rbp), %xmm1                             #3165.22
..LN2129:
   .loc    1  3166
        pcmpeqd   -80(%rbp), %xmm1                              #3166.22
..LN2131:
   .loc    1  3167
        pmovmskb  %xmm1, %r12d                                  #3167.22
..LN2133:
   .loc    1  3168
        movl      %r12d, %esi                                   #3168.17
        andl      $3855, %esi                                   #3168.17
        shll      $4, %esi                                      #3168.17
        andl      %esi, %r12d                                   #3168.17
        movl      %r12d, %edi                                   #3168.17
        shrl      $4, %edi                                      #3168.17
        orl       %edi, %r12d                                   #3168.17
        jmp       ..B15.11      # Prob 100%                     #3168.17
                                # LOE rcx r14 eax edx r12d r15d xmm0
..B15.38:                       # Preds ..B15.26 ..B15.28 ..B15.29 # Infreq
..LN2135:
   .loc    1  3363
        movq      -32(%rbp), %rbx                               #3363.12
..___tag_value_eval_2na_64.579:                                 #
        movq      -40(%rbp), %r12                               #3363.12
..___tag_value_eval_2na_64.580:                                 #
        movq      -24(%rbp), %r13                               #3363.12
..___tag_value_eval_2na_64.581:                                 #
        movq      -48(%rbp), %r14                               #3363.12
..___tag_value_eval_2na_64.582:                                 #
        xorl      %eax, %eax                                    #3363.12
        movq      -56(%rbp), %r15                               #3363.12
..___tag_value_eval_2na_64.583:                                 #
        movq      %rbp, %rsp                                    #3363.12
        popq      %rbp                                          #3363.12
..___tag_value_eval_2na_64.584:                                 #
        ret                                                     #3363.12
..___tag_value_eval_2na_64.585:                                 #
                                # LOE
..B15.40:                       # Preds ..B15.3                 # Infreq
        movdqa    %xmm8, -128(%rbp)                             #
        movdqa    %xmm1, -112(%rbp)                             #
        movdqa    %xmm2, -96(%rbp)                              #
        movdqa    %xmm3, -160(%rbp)                             #
        movdqa    %xmm4, -192(%rbp)                             #
        movdqa    %xmm5, -176(%rbp)                             #
        movdqa    %xmm6, -144(%rbp)                             #
        movdqa    %xmm7, -80(%rbp)                              #
        movl      %ebx, %eax                                    #
        movq      %r13, -64(%rbp)                               #
        movl      %r8d, -16(%rbp)                               #
        jmp       ..B15.36      # Prob 100%                     #
                                # LOE rcx r14 eax edx r15d xmm0
..B15.41:                       # Preds ..B15.23 ..B15.21 ..B15.19 ..B15.25 # Infreq
..LN2137:
   .loc    1  3229
        movq      -32(%rbp), %rbx                               #3229.63
..___tag_value_eval_2na_64.591:                                 #
        movq      -40(%rbp), %r12                               #3229.63
..___tag_value_eval_2na_64.592:                                 #
        movq      -24(%rbp), %r13                               #3229.63
..___tag_value_eval_2na_64.593:                                 #
        movq      -48(%rbp), %r14                               #3229.63
..___tag_value_eval_2na_64.594:                                 #
        movl      $1, %eax                                      #3229.63
        movq      -56(%rbp), %r15                               #3229.63
..___tag_value_eval_2na_64.595:                                 #
        movq      %rbp, %rsp                                    #3229.63
        popq      %rbp                                          #3229.63
..___tag_value_eval_2na_64.596:                                 #
        ret                                                     #3229.63
        .align    16,0x90
..___tag_value_eval_2na_64.597:                                 #
                                # LOE
# mark_end;
	.type	eval_2na_64,@function
	.size	eval_2na_64,.-eval_2na_64
.LNeval_2na_64:
	.data
# -- End  eval_2na_64
	.text
# -- Begin  NucStrstrMake
# mark_begin;
       .align    16,0x90
	.globl NucStrstrMake
NucStrstrMake:
# parameter 1(nss): %rdi
# parameter 2(positional): %esi
# parameter 3(query): %rdx
# parameter 4(len): %ecx
..B16.1:                        # Preds ..B16.0
..___tag_value_NucStrstrMake.598:                               #1539.1
..LN2139:
   .loc    1  1539
        pushq     %rbp                                          #1539.1
        movq      %rsp, %rbp                                    #1539.1
..___tag_value_NucStrstrMake.599:                               #
        subq      $48, %rsp                                     #1539.1
        movq      %r15, -48(%rbp)                               #1539.1
..___tag_value_NucStrstrMake.601:                               #
        movq      %r14, -40(%rbp)                               #1539.1
..___tag_value_NucStrstrMake.602:                               #
        movq      %r13, -32(%rbp)                               #1539.1
..___tag_value_NucStrstrMake.603:                               #
        movq      %r12, -24(%rbp)                               #1539.1
..___tag_value_NucStrstrMake.604:                               #
        movl      %ecx, %r15d                                   #1539.1
        movq      %rdx, %r12                                    #1539.1
        movl      %esi, %r13d                                   #1539.1
        movq      %rdi, %r14                                    #1539.1
        lea       _gprof_pack15(%rip), %rdx                     #1539.1
        call      mcount@PLT                                    #1539.1
                                # LOE rbx r12 r14 r13d r15d
..B16.17:                       # Preds ..B16.1
..LN2141:
   .loc    1  1540
        testq     %r14, %r14                                    #1540.17
        je        ..B16.12      # Prob 6%                       #1540.17
                                # LOE rbx r12 r14 r13d r15d
..B16.2:                        # Preds ..B16.17
..LN2143:
   .loc    1  1542
        testq     %r12, %r12                                    #1542.23
        je        ..B16.11      # Prob 6%                       #1542.23
                                # LOE rbx r12 r14 r13d r15d
..B16.3:                        # Preds ..B16.2
..LN2145:
        testl     %r15d, %r15d                                  #1542.38
        je        ..B16.11      # Prob 28%                      #1542.38
                                # LOE rbx r12 r14 r13d r15d
..B16.4:                        # Preds ..B16.3
..LN2147:
   .loc    1  1544
        movl      $0, -16(%rbp)                                 #1544.24
..LN2149:
   .loc    1  1547
        lea       fasta_2na_map.0(%rip), %rax                   #1547.18
        movsbl    (%rax), %edx                                  #1547.18
..LN2151:
        testl     %edx, %edx                                    #1547.41
        je        ..B16.13      # Prob 5%                       #1547.41
                                # LOE rbx r12 r14 r13d r15d
..B16.5:                        # Preds ..B16.13 ..B16.4
..LN2153:
   .loc    1  1551
        lea       -16(%rbp), %rcx                               #1551.21
..LN2155:
   .loc    1  1550
        movl      %r15d, %r15d                                  #1550.19
        addq      %r12, %r15                                    #1550.19
..LN2157:
   .loc    1  1551
        movq      %r12, %rdi                                    #1551.21
        movq      %r15, %rsi                                    #1551.21
        movq      %r14, %rdx                                    #1551.21
        movl      %r13d, %r8d                                   #1551.21
        call      nss_expr@PLT                                  #1551.21
                                # LOE rax rbx r14 r15
..B16.6:                        # Preds ..B16.5
..LN2159:
   .loc    1  1552
        movl      -16(%rbp), %r12d                              #1552.18
..LN2161:
        testl     %r12d, %r12d                                  #1552.28
        jne       ..B16.9       # Prob 10%                      #1552.28
                                # LOE rax rbx r14 r15 r12d
..B16.7:                        # Preds ..B16.6
..LN2163:
   .loc    1  1554
        cmpq      %r15, %rax                                    #1554.31
        je        ..B16.14      # Prob 12%                      #1554.31
                                # LOE rbx r14
..B16.8:                        # Preds ..B16.7
..LN2165:
   .loc    1  1557
        movl      $22, -16(%rbp)                                #1557.17
        movl      $22, %r12d                                    #1557.17
                                # LOE rbx r14 r12d
..B16.9:                        # Preds ..B16.8 ..B16.6
..LN2167:
   .loc    1  1560
        movq      (%r14), %rdi                                  #1560.13
        call      NucStrstrWhack@PLT                            #1560.13
                                # LOE rbx r14 r12d
..B16.10:                       # Preds ..B16.9
..LN2169:
   .loc    1  1562
        movq      -32(%rbp), %r13                               #1562.20
..___tag_value_NucStrstrMake.605:                               #
        movq      -48(%rbp), %r15                               #1562.20
..___tag_value_NucStrstrMake.606:                               #
..LN2171:
   .loc    1  1561
        movq      $0, (%r14)                                    #1561.13
..LN2173:
   .loc    1  1562
        movl      %r12d, %eax                                   #1562.20
        movq      -24(%rbp), %r12                               #1562.20
..___tag_value_NucStrstrMake.607:                               #
        movq      -40(%rbp), %r14                               #1562.20
..___tag_value_NucStrstrMake.608:                               #
        movq      %rbp, %rsp                                    #1562.20
        popq      %rbp                                          #1562.20
..___tag_value_NucStrstrMake.609:                               #
        ret                                                     #1562.20
..___tag_value_NucStrstrMake.610:                               #
                                # LOE
..B16.11:                       # Preds ..B16.3 ..B16.2
..LN2175:
   .loc    1  1565
        movq      $0, (%r14)                                    #1565.9
                                # LOE rbx
..B16.12:                       # Preds ..B16.17 ..B16.11
..LN2177:
   .loc    1  1567
        movq      -24(%rbp), %r12                               #1567.12
..___tag_value_NucStrstrMake.615:                               #
        movq      -32(%rbp), %r13                               #1567.12
..___tag_value_NucStrstrMake.616:                               #
        movq      -40(%rbp), %r14                               #1567.12
..___tag_value_NucStrstrMake.617:                               #
        movq      -48(%rbp), %r15                               #1567.12
..___tag_value_NucStrstrMake.618:                               #
        movl      $22, %eax                                     #1567.12
        movq      %rbp, %rsp                                    #1567.12
        popq      %rbp                                          #1567.12
..___tag_value_NucStrstrMake.619:                               #
        ret                                                     #1567.12
..___tag_value_NucStrstrMake.620:                               #
                                # LOE
..B16.13:                       # Preds ..B16.4                 # Infreq
..LN2179:
   .loc    1  1548
        call      NucStrstrInit@PLT                             #1548.17
        jmp       ..B16.5       # Prob 100%                     #1548.17
                                # LOE rbx r12 r14 r13d r15d
..B16.14:                       # Preds ..B16.7                 # Infreq
..LN2181:
   .loc    1  1555
        movq      -24(%rbp), %r12                               #1555.28
..___tag_value_NucStrstrMake.625:                               #
        movq      -32(%rbp), %r13                               #1555.28
..___tag_value_NucStrstrMake.626:                               #
        movq      -40(%rbp), %r14                               #1555.28
..___tag_value_NucStrstrMake.627:                               #
        movq      -48(%rbp), %r15                               #1555.28
..___tag_value_NucStrstrMake.628:                               #
        xorl      %eax, %eax                                    #1555.28
        movq      %rbp, %rsp                                    #1555.28
        popq      %rbp                                          #1555.28
..___tag_value_NucStrstrMake.629:                               #
        ret                                                     #1555.28
        .align    16,0x90
..___tag_value_NucStrstrMake.630:                               #
                                # LOE
# mark_end;
	.type	NucStrstrMake,@function
	.size	NucStrstrMake,.-NucStrstrMake
.LNNucStrstrMake:
	.data
# -- End  NucStrstrMake
	.text
# -- Begin  nss_expr
# mark_begin;
       .align    16,0x90
nss_expr:
# parameter 1(p): %rdi
# parameter 2(end): %rsi
# parameter 3(expr): %rdx
# parameter 4(status): %rcx
# parameter 5(positional): %r8d
..B17.1:                        # Preds ..B17.0
..___tag_value_nss_expr.631:                                    #1406.1
..LN2183:
   .loc    1  1406
        pushq     %rbp                                          #1406.1
        movq      %rsp, %rbp                                    #1406.1
..___tag_value_nss_expr.632:                                    #
        subq      $48, %rsp                                     #1406.1
        movq      %r14, -24(%rbp)                               #1406.1
..___tag_value_nss_expr.634:                                    #
        movq      %r13, -32(%rbp)                               #1406.1
..___tag_value_nss_expr.635:                                    #
        movq      %r12, -40(%rbp)                               #1406.1
..___tag_value_nss_expr.636:                                    #
        movq      %rbx, -48(%rbp)                               #1406.1
..___tag_value_nss_expr.637:                                    #
        movq      %rcx, %r14                                    #1406.1
        movl      %r8d, -8(%rbp)                                #1406.1
        movq      %rdx, %r13                                    #1406.1
        lea       _gprof_pack16(%rip), %rdx                     #1406.1
        movq      %rsi, %r12                                    #1406.1
        movq      %rdi, %rbx                                    #1406.1
        call      mcount@PLT                                    #1406.1
                                # LOE rbx r12 r13 r14 r15
..B17.34:                       # Preds ..B17.1
        movq      %r15, -16(%rbp)                               #
..___tag_value_nss_expr.638:                                    #
                                # LOE rbx r12 r13 r14
..B17.2:                        # Preds ..B17.17 ..B17.34
..LN2185:
   .loc    1  1407
        movq      $0, (%r13)                                    #1407.5
..LN2187:
   .loc    1  1409
        movq      %rbx, %rdi                                    #1409.9
        movq      %r12, %rsi                                    #1409.9
        call      nss_sob@PLT                                   #1409.9
                                # LOE rax r12 r13 r14
..B17.27:                       # Preds ..B17.2
..LN2189:
   .loc    1  1410
        cmpq      %r12, %rax                                    #1410.15
        je        ..B17.33      # Prob 16%                      #1410.15
                                # LOE rax r12 r13 r14
..B17.3:                        # Preds ..B17.27
..LN2191:
   .loc    1  1412
        movl      -8(%rbp), %r8d                                #1412.13
        movq      %rax, %rdi                                    #1412.13
        movq      %r12, %rsi                                    #1412.13
        movq      %r13, %rdx                                    #1412.13
        movq      %r14, %rcx                                    #1412.13
        call      nss_unary_expr@PLT                            #1412.13
                                # LOE rax r12 r13 r14
..B17.4:                        # Preds ..B17.3
..LN2193:
   .loc    1  1413
        movl      (%r14), %edx                                  #1413.16
..LN2195:
        testl     %edx, %edx                                    #1413.26
        jne       ..B17.20      # Prob 58%                      #1413.26
                                # LOE rax r12 r13 r14
..B17.5:                        # Preds ..B17.4
..LN2197:
   .loc    1  1415
        movq      %rax, %rdi                                    #1415.17
        movq      %r12, %rsi                                    #1415.17
        call      nss_sob@PLT                                   #1415.17
                                # LOE rax r12 r13 r14
..B17.29:                       # Preds ..B17.5
..LN2199:
   .loc    1  1416
        cmpq      %r12, %rax                                    #1416.23
        je        ..B17.33      # Prob 4%                       #1416.23
                                # LOE rax r12 r13 r14
..B17.6:                        # Preds ..B17.29
..LN2201:
   .loc    1  1422
        movsbl    (%rax), %edx                                  #1422.28
        lea       1(%rax), %rbx                                 #1422.28
        cmpl      $41, %edx                                     #1422.28
        je        ..B17.19      # Prob 25%                      #1422.28
                                # LOE rax rbx r12 r13 r14 edx
..B17.7:                        # Preds ..B17.6
        cmpl      $38, %edx                                     #1422.28
        jne       ..B17.11      # Prob 67%                      #1422.28
                                # LOE rax rbx r12 r13 r14 edx
..B17.8:                        # Preds ..B17.7
..LN2203:
   .loc    1  1427
        cmpq      %r12, %rbx                                    #1427.30
        jae       ..B17.10      # Prob 12%                      #1427.30
                                # LOE rax rbx r12 r13 r14
..B17.9:                        # Preds ..B17.8
..LN2205:
        movsbl    (%rbx), %edx                                  #1427.39
..LN2207:
   .loc    1  1428
        addq      $2, %rax                                      #1428.28
        cmpl      $38, %edx                                     #1428.28
        cmove     %rax, %rbx                                    #1428.28
                                # LOE rbx r12 r13 r14
..B17.10:                       # Preds ..B17.9 ..B17.8
..LN2209:
   .loc    1  1429
        movl      $16, %r15d                                    #1429.21
        jmp       ..B17.15      # Prob 100%                     #1429.21
                                # LOE rbx r12 r13 r14 r15d
..B17.11:                       # Preds ..B17.7
..LN2211:
   .loc    1  1422
        cmpl      $124, %edx                                    #1422.28
        jne       ..B17.18      # Prob 50%                      #1422.28
                                # LOE rax rbx r12 r13 r14
..B17.12:                       # Preds ..B17.11
..LN2213:
   .loc    1  1432
        cmpq      %r12, %rbx                                    #1432.30
        jae       ..B17.14      # Prob 12%                      #1432.30
                                # LOE rax rbx r12 r13 r14
..B17.13:                       # Preds ..B17.12
..LN2215:
        movsbl    (%rbx), %edx                                  #1432.39
..LN2217:
   .loc    1  1433
        addq      $2, %rax                                      #1433.28
        cmpl      $124, %edx                                    #1433.28
        cmove     %rax, %rbx                                    #1433.28
                                # LOE rbx r12 r13 r14
..B17.14:                       # Preds ..B17.13 ..B17.12
..LN2219:
   .loc    1  1434
        movl      $17, %r15d                                    #1434.21
                                # LOE rbx r12 r13 r14 r15d
..B17.15:                       # Preds ..B17.10 ..B17.14
..LN2221:
   .loc    1  1451
        movl      $24, %edi                                     #1451.21
        call      malloc@PLT                                    #1451.21
                                # LOE rax rbx r12 r13 r14 r15d
..B17.16:                       # Preds ..B17.15
..LN2223:
   .loc    1  1452
        testq     %rax, %rax                                    #1452.27
        je        ..B17.24      # Prob 1%                       #1452.27
                                # LOE rax rbx r12 r13 r14 r15d
..B17.17:                       # Preds ..B17.16
..LN2225:
   .loc    1  1458
        movl      $11, (%rax)                                   #1458.17
..LN2227:
   .loc    1  1459
        movl      %r15d, 4(%rax)                                #1459.17
..LN2229:
   .loc    1  1460
        movq      (%r13), %rdx                                  #1460.41
..LN2231:
        movq      %rdx, 8(%rax)                                 #1460.17
..LN2233:
   .loc    1  1461
        movq      %rax, (%r13)                                  #1461.17
..LN2235:
   .loc    1  1464
        lea       16(%rax), %r13                                #1464.42
        jmp       ..B17.2       # Prob 100%                     #1464.42
                                # LOE rbx r12 r13 r14
..B17.18:                       # Preds ..B17.11
        movq      -16(%rbp), %r15                               #
..___tag_value_nss_expr.639:                                    #
..LN2237:
   .loc    1  1439
        movq      -48(%rbp), %rbx                               #1439.32
..___tag_value_nss_expr.640:                                    #
        movq      -40(%rbp), %r12                               #1439.32
..___tag_value_nss_expr.641:                                    #
        movq      -32(%rbp), %r13                               #1439.32
..___tag_value_nss_expr.642:                                    #
..LN2239:
   .loc    1  1438
        movl      $22, (%r14)                                   #1438.21
..LN2241:
   .loc    1  1439
        movq      -24(%rbp), %r14                               #1439.32
..___tag_value_nss_expr.643:                                    #
        movq      %rbp, %rsp                                    #1439.32
        popq      %rbp                                          #1439.32
..___tag_value_nss_expr.644:                                    #
        ret                                                     #1439.32
..___tag_value_nss_expr.645:                                    #
                                # LOE
..B17.19:                       # Preds ..B17.6
        movq      -16(%rbp), %r15                               #
..___tag_value_nss_expr.651:                                    #
..LN2243:
   .loc    1  1425
        movq      -48(%rbp), %rbx                               #1425.32
..___tag_value_nss_expr.652:                                    #
        movq      -40(%rbp), %r12                               #1425.32
..___tag_value_nss_expr.653:                                    #
        movq      -32(%rbp), %r13                               #1425.32
..___tag_value_nss_expr.654:                                    #
        movq      -24(%rbp), %r14                               #1425.32
..___tag_value_nss_expr.655:                                    #
        movq      %rbp, %rsp                                    #1425.32
        popq      %rbp                                          #1425.32
..___tag_value_nss_expr.656:                                    #
        ret                                                     #1425.32
..___tag_value_nss_expr.657:                                    #
                                # LOE
..B17.20:                       # Preds ..B17.4
        movq      -16(%rbp), %r15                               #
..___tag_value_nss_expr.663:                                    #
                                # LOE rax r15
..B17.21:                       # Preds ..B17.33 ..B17.20
..LN2245:
   .loc    1  1470
        movq      -48(%rbp), %rbx                               #1470.12
..___tag_value_nss_expr.665:                                    #
        movq      -40(%rbp), %r12                               #1470.12
..___tag_value_nss_expr.666:                                    #
        movq      -32(%rbp), %r13                               #1470.12
..___tag_value_nss_expr.667:                                    #
        movq      -24(%rbp), %r14                               #1470.12
..___tag_value_nss_expr.668:                                    #
        movq      %rbp, %rsp                                    #1470.12
        popq      %rbp                                          #1470.12
..___tag_value_nss_expr.669:                                    #
        ret                                                     #1470.12
..___tag_value_nss_expr.670:                                    #
                                # LOE
..B17.33:                       # Preds ..B17.29 ..B17.27
        movq      -16(%rbp), %r15                               #
..___tag_value_nss_expr.675:                                    #
        jmp       ..B17.21      # Prob 100%                     #
..___tag_value_nss_expr.676:                                    #
                                # LOE rax r15
..B17.24:                       # Preds ..B17.16                # Infreq
        movq      -16(%rbp), %r15                               #
..___tag_value_nss_expr.677:                                    #
..LN2247:
   .loc    1  1454
        call      __errno_location@PLT                          #1454.32
..___tag_value_nss_expr.678:                                    #
                                # LOE rax rbx r14 r15
..B17.31:                       # Preds ..B17.24                # Infreq
..LN2249:
   .loc    1  1455
        movq      -40(%rbp), %r12                               #1455.28
..___tag_value_nss_expr.679:                                    #
        movq      -32(%rbp), %r13                               #1455.28
..___tag_value_nss_expr.680:                                    #
..LN2251:
   .loc    1  1454
        movl      (%rax), %edx                                  #1454.32
..LN2253:
        movl      %edx, (%r14)                                  #1454.21
..LN2255:
   .loc    1  1455
        movq      %rbx, %rax                                    #1455.28
        movq      -48(%rbp), %rbx                               #1455.28
..___tag_value_nss_expr.681:                                    #
        movq      -24(%rbp), %r14                               #1455.28
..___tag_value_nss_expr.682:                                    #
        movq      %rbp, %rsp                                    #1455.28
        popq      %rbp                                          #1455.28
..___tag_value_nss_expr.683:                                    #
        ret                                                     #1455.28
        .align    16,0x90
..___tag_value_nss_expr.684:                                    #
                                # LOE
# mark_end;
	.type	nss_expr,@function
	.size	nss_expr,.-nss_expr
.LNnss_expr:
	.data
# -- End  nss_expr
	.text
# -- Begin  nss_unary_expr
# mark_begin;
       .align    16,0x90
nss_unary_expr:
# parameter 1(p): %rdi
# parameter 2(end): %rsi
# parameter 3(expr): %rdx
# parameter 4(status): %rcx
# parameter 5(positional): %r8d
..B18.1:                        # Preds ..B18.0
..___tag_value_nss_unary_expr.685:                              #1361.1
..LN2257:
   .loc    1  1361
        pushq     %rbp                                          #1361.1
        movq      %rsp, %rbp                                    #1361.1
..___tag_value_nss_unary_expr.686:                              #
        subq      $48, %rsp                                     #1361.1
        movq      %r15, -48(%rbp)                               #1361.1
..___tag_value_nss_unary_expr.688:                              #
        movq      %r14, -40(%rbp)                               #1361.1
..___tag_value_nss_unary_expr.689:                              #
        movq      %r13, -32(%rbp)                               #1361.1
..___tag_value_nss_unary_expr.690:                              #
        movq      %r12, -24(%rbp)                               #1361.1
..___tag_value_nss_unary_expr.691:                              #
        movq      %rbx, -16(%rbp)                               #1361.1
..___tag_value_nss_unary_expr.692:                              #
        movl      %r8d, %r13d                                   #1361.1
        movq      %rcx, %r15                                    #1361.1
        movq      %rdx, %r12                                    #1361.1
        movq      %rsi, %rbx                                    #1361.1
        movq      %rdi, %r14                                    #1361.1
        lea       _gprof_pack17(%rip), %rdx                     #1361.1
        call      mcount@PLT                                    #1361.1
        jmp       ..B18.2       # Prob 100%                     #1361.1
                                # LOE rbx r12 r14 r15 r13d
..B18.10:                       # Preds ..B18.9
..LN2259:
   .loc    1  1384
        movl      $12, (%rax)                                   #1384.13
..LN2261:
   .loc    1  1385
        movl      $13, 4(%rax)                                  #1385.13
..LN2263:
   .loc    1  1386
        movq      $0, 8(%rax)                                   #1386.13
..LN2265:
   .loc    1  1387
        movq      %rax, (%r12)                                  #1387.13
..LN2267:
   .loc    1  1389
        lea       8(%rax), %r12                                 #1389.44
                                # LOE rbx r12 r14 r15 r13d
..B18.2:                        # Preds ..B18.10 ..B18.1
..LN2269:
   .loc    1  1363
        movsbl    (%r14), %eax                                  #1363.12
..LN2271:
        cmpl      $33, %eax                                     #1363.17
        je        ..B18.5       # Prob 33%                      #1363.17
                                # LOE rbx r12 r14 r15 r13d
..B18.3:                        # Preds ..B18.2
..LN2273:
   .loc    1  1364
        movq      %r14, %rdi                                    #1364.16
        movq      %rbx, %rsi                                    #1364.16
        movq      %r12, %rdx                                    #1364.16
        movq      %r15, %rcx                                    #1364.16
        movl      %r13d, %r8d                                   #1364.16
        call      nss_primary_expr@PLT                          #1364.16
                                # LOE rax
..B18.4:                        # Preds ..B18.3
        movq      -16(%rbp), %rbx                               #1364.16
..___tag_value_nss_unary_expr.693:                              #
        movq      -24(%rbp), %r12                               #1364.16
..___tag_value_nss_unary_expr.694:                              #
        movq      -32(%rbp), %r13                               #1364.16
..___tag_value_nss_unary_expr.695:                              #
        movq      -40(%rbp), %r14                               #1364.16
..___tag_value_nss_unary_expr.696:                              #
        movq      -48(%rbp), %r15                               #1364.16
..___tag_value_nss_unary_expr.697:                              #
        movq      %rbp, %rsp                                    #1364.16
        popq      %rbp                                          #1364.16
..___tag_value_nss_unary_expr.698:                              #
        ret                                                     #1364.16
..___tag_value_nss_unary_expr.699:                              #
                                # LOE
..B18.5:                        # Preds ..B18.2
..LN2275:
   .loc    1  1374
        incq      %r14                                          #1374.19
..LN2277:
        movq      %r14, %rdi                                    #1374.9
        movq      %rbx, %rsi                                    #1374.9
        call      nss_sob@PLT                                   #1374.9
                                # LOE rax rbx r12 r15 r13d
..B18.15:                       # Preds ..B18.5
        movq      %rax, %r14                                    #1374.9
..LN2279:
   .loc    1  1375
        cmpq      %rbx, %r14                                    #1375.15
        jne       ..B18.8       # Prob 68%                      #1375.15
                                # LOE rbx r12 r14 r15 r13d
..B18.6:                        # Preds ..B18.15
..LN2281:
   .loc    1  1376
        movl      $22, (%r15)                                   #1376.9
                                # LOE r14
..B18.7:                        # Preds ..B18.6 ..B18.17
..LN2283:
   .loc    1  1393
        movq      -16(%rbp), %rbx                               #1393.12
..___tag_value_nss_unary_expr.705:                              #
        movq      -24(%rbp), %r12                               #1393.12
..___tag_value_nss_unary_expr.706:                              #
        movq      -32(%rbp), %r13                               #1393.12
..___tag_value_nss_unary_expr.707:                              #
        movq      -48(%rbp), %r15                               #1393.12
..___tag_value_nss_unary_expr.708:                              #
        movq      %r14, %rax                                    #1393.12
        movq      -40(%rbp), %r14                               #1393.12
..___tag_value_nss_unary_expr.709:                              #
        movq      %rbp, %rsp                                    #1393.12
        popq      %rbp                                          #1393.12
..___tag_value_nss_unary_expr.710:                              #
        ret                                                     #1393.12
..___tag_value_nss_unary_expr.711:                              #
                                # LOE
..B18.8:                        # Preds ..B18.15
..LN2285:
   .loc    1  1379
        movl      $16, %edi                                     #1379.25
        call      malloc@PLT                                    #1379.25
                                # LOE rax rbx r12 r14 r15 r13d
..B18.9:                        # Preds ..B18.8
..LN2287:
   .loc    1  1380
        testq     %rax, %rax                                    #1380.19
        jne       ..B18.10      # Prob 99%                      #1380.19
                                # LOE rax rbx r12 r14 r15 r13d
..B18.11:                       # Preds ..B18.9                 # Infreq
..LN2289:
   .loc    1  1381
        call      __errno_location@PLT                          #1381.24
                                # LOE rax r14 r15
..B18.17:                       # Preds ..B18.11                # Infreq
        movl      (%rax), %edx                                  #1381.24
..LN2291:
        movl      %edx, (%r15)                                  #1381.13
        jmp       ..B18.7       # Prob 100%                     #1381.13
        .align    16,0x90
..___tag_value_nss_unary_expr.717:                              #
                                # LOE r14
# mark_end;
	.type	nss_unary_expr,@function
	.size	nss_unary_expr,.-nss_unary_expr
.LNnss_unary_expr:
	.data
# -- End  nss_unary_expr
	.text
# -- Begin  nss_primary_expr
# mark_begin;
       .align    16,0x90
nss_primary_expr:
# parameter 1(p): %rdi
# parameter 2(end): %rsi
# parameter 3(expr): %rdx
# parameter 4(status): %rcx
# parameter 5(positional): %r8d
..B19.1:                        # Preds ..B19.0
..___tag_value_nss_primary_expr.718:                            #1291.1
..LN2293:
   .loc    1  1291
        pushq     %rbp                                          #1291.1
        movq      %rsp, %rbp                                    #1291.1
..___tag_value_nss_primary_expr.719:                            #
        subq      $48, %rsp                                     #1291.1
        movq      %r15, -40(%rbp)                               #1291.1
..___tag_value_nss_primary_expr.721:                            #
        movq      %r14, -32(%rbp)                               #1291.1
..___tag_value_nss_primary_expr.722:                            #
        movq      %r13, -24(%rbp)                               #1291.1
..___tag_value_nss_primary_expr.723:                            #
        movq      %r12, -16(%rbp)                               #1291.1
..___tag_value_nss_primary_expr.724:                            #
        movq      %rbx, -8(%rbp)                                #1291.1
..___tag_value_nss_primary_expr.725:                            #
        movl      %r8d, %ebx                                    #1291.1
        movq      %rcx, %r15                                    #1291.1
        movq      %rdx, %r14                                    #1291.1
        movq      %rsi, %r13                                    #1291.1
        movq      %rdi, %r12                                    #1291.1
        lea       _gprof_pack18(%rip), %rdx                     #1291.1
        call      mcount@PLT                                    #1291.1
                                # LOE r12 r13 r14 r15 ebx
..B19.32:                       # Preds ..B19.1
..LN2295:
   .loc    1  1295
        movsbl    (%r12), %eax                                  #1295.16
        cmpl      $94, %eax                                     #1295.16
        jne       ..B19.8       # Prob 67%                      #1295.16
                                # LOE r12 r13 r14 r15 eax ebx
..B19.2:                        # Preds ..B19.32
..LN2297:
   .loc    1  1298
        movl      $16, %edi                                     #1298.13
        call      malloc@PLT                                    #1298.13
                                # LOE rax r12 r13 r14 r15 ebx
..B19.33:                       # Preds ..B19.2
        movq      %rax, %rdx                                    #1298.13
                                # LOE rdx r12 r13 r14 r15 ebx
..B19.3:                        # Preds ..B19.33
..LN2299:
   .loc    1  1299
        testq     %rdx, %rdx                                    #1299.19
        jne       ..B19.5       # Prob 68%                      #1299.19
                                # LOE rdx r12 r13 r14 r15 ebx
..B19.4:                        # Preds ..B19.3
..LN2301:
   .loc    1  1300
        call      __errno_location@PLT                          #1300.24
                                # LOE rax r12 r15
..B19.34:                       # Preds ..B19.4
        movl      (%rax), %edx                                  #1300.24
..LN2303:
        movl      %edx, (%r15)                                  #1300.13
        jmp       ..B19.7       # Prob 100%                     #1300.13
                                # LOE r12
..B19.5:                        # Preds ..B19.3
..LN2305:
   .loc    1  1303
        movl      $12, (%rdx)                                   #1303.13
..LN2307:
   .loc    1  1308
        movq      %rdx, -48(%rbp)                               #1308.17
..LN2309:
   .loc    1  1306
        movq      %rdx, (%r14)                                  #1306.13
..LN2311:
   .loc    1  1308
        incq      %r12                                          #1308.27
..LN2313:
   .loc    1  1304
        movl      $14, 4(%rdx)                                  #1304.13
..LN2315:
   .loc    1  1308
        movq      %r12, %rdi                                    #1308.17
        movq      %r13, %rsi                                    #1308.17
..LN2317:
   .loc    1  1305
        movq      $0, 8(%rdx)                                   #1305.13
..LN2319:
   .loc    1  1308
        call      nss_sob@PLT                                   #1308.17
                                # LOE rax r13 r15 ebx
..B19.35:                       # Preds ..B19.5
        movq      -48(%rbp), %rdx                               #
..LN2321:
   .loc    1  1309
        addq      $8, %rdx                                      #1309.17
        movq      %rax, %rdi                                    #1309.17
        movq      %r13, %rsi                                    #1309.17
        movq      %r15, %rcx                                    #1309.17
        movl      %ebx, %r8d                                    #1309.17
        call      nss_fasta_expr@PLT                            #1309.17
                                # LOE rax
..B19.36:                       # Preds ..B19.35
        movq      %rax, %r12                                    #1309.17
                                # LOE r12
..B19.7:                        # Preds ..B19.36 ..B19.34
..LN2323:
   .loc    1  1311
        movq      -8(%rbp), %rbx                                #1311.16
..___tag_value_nss_primary_expr.726:                            #
        movq      -24(%rbp), %r13                               #1311.16
..___tag_value_nss_primary_expr.727:                            #
        movq      -32(%rbp), %r14                               #1311.16
..___tag_value_nss_primary_expr.728:                            #
        movq      -40(%rbp), %r15                               #1311.16
..___tag_value_nss_primary_expr.729:                            #
        movq      %r12, %rax                                    #1311.16
        movq      -16(%rbp), %r12                               #1311.16
..___tag_value_nss_primary_expr.730:                            #
        movq      %rbp, %rsp                                    #1311.16
        popq      %rbp                                          #1311.16
..___tag_value_nss_primary_expr.731:                            #
        ret                                                     #1311.16
..___tag_value_nss_primary_expr.732:                            #
                                # LOE
..B19.8:                        # Preds ..B19.32
..LN2325:
   .loc    1  1295
        cmpl      $40, %eax                                     #1295.16
        jne       ..B19.19      # Prob 50%                      #1295.16
                                # LOE r12 r13 r14 r15 ebx
..B19.9:                        # Preds ..B19.8
..LN2327:
   .loc    1  1313
        movl      $16, %edi                                     #1313.13
        call      malloc@PLT                                    #1313.13
                                # LOE rax r12 r13 r14 r15 ebx
..B19.37:                       # Preds ..B19.9
        movq      %rax, %r9                                     #1313.13
                                # LOE r9 r12 r13 r14 r15 ebx
..B19.10:                       # Preds ..B19.37
..LN2329:
   .loc    1  1314
        testq     %r9, %r9                                      #1314.19
        jne       ..B19.12      # Prob 68%                      #1314.19
                                # LOE r9 r12 r13 r14 r15 ebx
..B19.11:                       # Preds ..B19.10
..LN2331:
   .loc    1  1315
        call      __errno_location@PLT                          #1315.24
                                # LOE rax r12 r15
..B19.38:                       # Preds ..B19.11
        movl      (%rax), %edx                                  #1315.24
..LN2333:
        movl      %edx, (%r15)                                  #1315.13
        jmp       ..B19.18      # Prob 100%                     #1315.13
                                # LOE r12
..B19.12:                       # Preds ..B19.10
..LN2335:
   .loc    1  1318
        movl      $12, (%r9)                                    #1318.13
..LN2337:
   .loc    1  1322
        movq      %r9, -48(%rbp)                                #1322.17
..LN2339:
   .loc    1  1320
        movq      %r9, (%r14)                                   #1320.13
..LN2341:
   .loc    1  1322
        incq      %r12                                          #1322.28
..LN2343:
   .loc    1  1319
        movl      $0, 4(%r9)                                    #1319.13
..LN2345:
   .loc    1  1322
        lea       8(%r9), %rdx                                  #1322.42
..LN2347:
        movq      %r12, %rdi                                    #1322.17
        movq      %r13, %rsi                                    #1322.17
        movq      %r15, %rcx                                    #1322.17
        movl      %ebx, %r8d                                    #1322.17
        call      nss_expr@PLT                                  #1322.17
                                # LOE rax r13 r15
..B19.39:                       # Preds ..B19.12
        movq      -48(%rbp), %r9                                #
                                # LOE rax r9 r13 r15
..B19.13:                       # Preds ..B19.39
..LN2349:
   .loc    1  1323
        movl      (%r15), %edx                                  #1323.20
..LN2351:
   .loc    1  1322
        movq      %rax, %r12                                    #1322.17
..LN2353:
   .loc    1  1323
        testl     %edx, %edx                                    #1323.30
        jne       ..B19.18      # Prob 50%                      #1323.30
                                # LOE rax r9 r12 r13 r15
..B19.14:                       # Preds ..B19.13
..LN2355:
   .loc    1  1325
        movq      8(%r9), %rdx                                  #1325.22
..LN2357:
        testq     %rdx, %rdx                                    #1325.41
        je        ..B19.17      # Prob 12%                      #1325.41
                                # LOE rax r12 r13 r15
..B19.15:                       # Preds ..B19.14
..LN2359:
        cmpq      %r13, %rax                                    #1325.54
        je        ..B19.17      # Prob 12%                      #1325.54
                                # LOE rax r12 r15
..B19.16:                       # Preds ..B19.15
..LN2361:
        lea       1(%rax), %r12                                 #1325.63
        movsbl    (%rax), %eax                                  #1325.63
..LN2363:
        cmpl      $41, %eax                                     #1325.71
        je        ..B19.18      # Prob 16%                      #1325.71
                                # LOE r12 r15
..B19.17:                       # Preds ..B19.15 ..B19.14 ..B19.16
..LN2365:
   .loc    1  1326
        movl      $22, (%r15)                                   #1326.21
                                # LOE r12
..B19.18:                       # Preds ..B19.17 ..B19.16 ..B19.38 ..B19.13
..LN2367:
   .loc    1  1329
        movq      -8(%rbp), %rbx                                #1329.16
..___tag_value_nss_primary_expr.738:                            #
        movq      -24(%rbp), %r13                               #1329.16
..___tag_value_nss_primary_expr.739:                            #
        movq      -32(%rbp), %r14                               #1329.16
..___tag_value_nss_primary_expr.740:                            #
        movq      -40(%rbp), %r15                               #1329.16
..___tag_value_nss_primary_expr.741:                            #
        movq      %r12, %rax                                    #1329.16
        movq      -16(%rbp), %r12                               #1329.16
..___tag_value_nss_primary_expr.742:                            #
        movq      %rbp, %rsp                                    #1329.16
        popq      %rbp                                          #1329.16
..___tag_value_nss_primary_expr.743:                            #
        ret                                                     #1329.16
..___tag_value_nss_primary_expr.744:                            #
                                # LOE
..B19.19:                       # Preds ..B19.8
..LN2369:
   .loc    1  1332
        movq      %r12, %rdi                                    #1332.9
        movq      %r13, %rsi                                    #1332.9
        movq      %r14, %rdx                                    #1332.9
        movq      %r15, %rcx                                    #1332.9
        movl      %ebx, %r8d                                    #1332.9
        call      nss_fasta_expr@PLT                            #1332.9
                                # LOE rax r13 r14 r15
..B19.40:                       # Preds ..B19.19
        movq      %rax, %rbx                                    #1332.9
                                # LOE rbx r13 r14 r15
..B19.20:                       # Preds ..B19.40
..LN2371:
   .loc    1  1333
        movl      (%r15), %eax                                  #1333.12
..LN2373:
        testl     %eax, %eax                                    #1333.22
        jne       ..B19.24      # Prob 50%                      #1333.22
                                # LOE rbx r13 r14 r15
..B19.21:                       # Preds ..B19.20
..LN2375:
        cmpq      %r13, %rbx                                    #1333.31
        jae       ..B19.24      # Prob 33%                      #1333.31
                                # LOE rbx r13 r14 r15
..B19.22:                       # Preds ..B19.21
..LN2377:
   .loc    1  1335
        movq      %rbx, %rdi                                    #1335.13
        movq      %r13, %rsi                                    #1335.13
        call      nss_sob@PLT                                   #1335.13
                                # LOE rax r13 r14 r15
..B19.41:                       # Preds ..B19.22
        movq      %rax, %rbx                                    #1335.13
..LN2379:
   .loc    1  1336
        cmpq      %r13, %rbx                                    #1336.18
        jae       ..B19.24      # Prob 12%                      #1336.18
                                # LOE rbx r14 r15
..B19.23:                       # Preds ..B19.41
..LN2381:
        movsbl    (%rbx), %eax                                  #1336.27
..LN2383:
        cmpl      $36, %eax                                     #1336.32
        je        ..B19.26      # Prob 5%                       #1336.32
                                # LOE rbx r14 r15
..B19.24:                       # Preds ..B19.43 ..B19.28 ..B19.23 ..B19.41 ..B19.21
                                #       ..B19.20
..LN2385:
   .loc    1  1353
        movq      -16(%rbp), %r12                               #1353.12
..___tag_value_nss_primary_expr.750:                            #
        movq      -24(%rbp), %r13                               #1353.12
..___tag_value_nss_primary_expr.751:                            #
        movq      -32(%rbp), %r14                               #1353.12
..___tag_value_nss_primary_expr.752:                            #
        movq      -40(%rbp), %r15                               #1353.12
..___tag_value_nss_primary_expr.753:                            #
        movq      %rbx, %rax                                    #1353.12
        movq      -8(%rbp), %rbx                                #1353.12
..___tag_value_nss_primary_expr.754:                            #
        movq      %rbp, %rsp                                    #1353.12
        popq      %rbp                                          #1353.12
..___tag_value_nss_primary_expr.755:                            #
        ret                                                     #1353.12
..___tag_value_nss_primary_expr.756:                            #
                                # LOE
..B19.26:                       # Preds ..B19.23                # Infreq
..LN2387:
   .loc    1  1338
        incq      %rbx                                          #1338.16
..LN2389:
   .loc    1  1340
        movl      $16, %edi                                     #1340.17
        call      malloc@PLT                                    #1340.17
                                # LOE rax rbx r14 r15
..B19.27:                       # Preds ..B19.26                # Infreq
..LN2391:
   .loc    1  1341
        testq     %rax, %rax                                    #1341.23
        je        ..B19.29      # Prob 12%                      #1341.23
                                # LOE rax rbx r14 r15
..B19.28:                       # Preds ..B19.27                # Infreq
..LN2393:
   .loc    1  1347
        movq      (%r14), %rdx                                  #1347.37
..LN2395:
   .loc    1  1345
        movl      $12, (%rax)                                   #1345.17
..LN2397:
   .loc    1  1347
        movq      %rdx, 8(%rax)                                 #1347.17
..LN2399:
   .loc    1  1348
        movq      %rax, (%r14)                                  #1348.17
..LN2401:
   .loc    1  1346
        movl      $15, 4(%rax)                                  #1346.17
        jmp       ..B19.24      # Prob 100%                     #1346.17
                                # LOE rbx
..B19.29:                       # Preds ..B19.27                # Infreq
..LN2403:
   .loc    1  1342
        call      __errno_location@PLT                          #1342.28
                                # LOE rax rbx r15
..B19.43:                       # Preds ..B19.29                # Infreq
        movl      (%rax), %edx                                  #1342.28
..LN2405:
        movl      %edx, (%r15)                                  #1342.17
        jmp       ..B19.24      # Prob 100%                     #1342.17
        .align    16,0x90
..___tag_value_nss_primary_expr.762:                            #
                                # LOE rbx
# mark_end;
	.type	nss_primary_expr,@function
	.size	nss_primary_expr,.-nss_primary_expr
.LNnss_primary_expr:
	.data
# -- End  nss_primary_expr
	.text
# -- Begin  nss_sob
# mark_begin;
       .align    16,0x90
nss_sob:
# parameter 1(p): %rdi
# parameter 2(end): %rsi
..B20.1:                        # Preds ..B20.0
..___tag_value_nss_sob.763:                                     #1181.1
..LN2407:
   .loc    1  1181
        pushq     %rbp                                          #1181.1
        movq      %rsp, %rbp                                    #1181.1
..___tag_value_nss_sob.764:                                     #
        subq      $32, %rsp                                     #1181.1
        movq      %r15, -24(%rbp)                               #1181.1
..___tag_value_nss_sob.766:                                     #
        movq      %r14, -16(%rbp)                               #1181.1
..___tag_value_nss_sob.767:                                     #
        movq      %r13, -8(%rbp)                                #1181.1
..___tag_value_nss_sob.768:                                     #
        movq      %rbx, -32(%rbp)                               #1181.1
..___tag_value_nss_sob.769:                                     #
        movq      %rsi, %rbx                                    #1181.1
        movq      %rdi, %r15                                    #1181.1
        lea       _gprof_pack19(%rip), %rdx                     #1181.1
        call      mcount@PLT                                    #1181.1
                                # LOE rbx r12 r15
..B20.10:                       # Preds ..B20.1
        movq      %r15, %r14                                    #1181.1
..LN2409:
   .loc    1  1182
        xorl      %r13d, %r13d                                  #1182.5
..LN2411:
        cmpq      %rbx, %r15                                    #1182.17
        jae       ..B20.6       # Prob 2%                       #1182.17
                                # LOE rbx r12 r13 r14 r15
..B20.2:                        # Preds ..B20.10
..LN2413:
        call      __ctype_b_loc@PLT                             #1182.24
                                # LOE rax rbx r12 r13 r14 r15
..B20.11:                       # Preds ..B20.2
        movq      (%rax), %rax                                  #1182.24
..LN2415:
   .loc    1  1181
        subq      %r15, %rbx                                    #1181.1
                                # LOE rax rbx r12 r13 r14 r15
..B20.3:                        # Preds ..B20.4 ..B20.11
..LN2417:
   .loc    1  1182
        movzbl    (%r13,%r14), %edx                             #1182.24
        movzwl    (%rax,%rdx,2), %ecx                           #1182.24
        testl     $8192, %ecx                                   #1182.24
        je        ..B20.6       # Prob 20%                      #1182.24
                                # LOE rax rbx r12 r13 r14 r15
..B20.4:                        # Preds ..B20.3
..LN2419:
   .loc    1  1183
        lea       1(%r13,%r14), %r15                            #1183.12
..LN2421:
   .loc    1  1182
        incq      %r13                                          #1182.5
        cmpq      %rbx, %r13                                    #1182.5
        jb        ..B20.3       # Prob 82%                      #1182.5
                                # LOE rax rbx r12 r13 r14 r15
..B20.6:                        # Preds ..B20.3 ..B20.4 ..B20.10
..LN2423:
   .loc    1  1184
        movq      -32(%rbp), %rbx                               #1184.12
..___tag_value_nss_sob.770:                                     #
        movq      -8(%rbp), %r13                                #1184.12
..___tag_value_nss_sob.771:                                     #
        movq      -16(%rbp), %r14                               #1184.12
..___tag_value_nss_sob.772:                                     #
        movq      %r15, %rax                                    #1184.12
        movq      -24(%rbp), %r15                               #1184.12
..___tag_value_nss_sob.773:                                     #
        movq      %rbp, %rsp                                    #1184.12
        popq      %rbp                                          #1184.12
..___tag_value_nss_sob.774:                                     #
        ret                                                     #1184.12
        .align    16,0x90
..___tag_value_nss_sob.775:                                     #
                                # LOE
# mark_end;
	.type	nss_sob,@function
	.size	nss_sob,.-nss_sob
.LNnss_sob:
	.data
# -- End  nss_sob
	.text
# -- Begin  nss_fasta_expr
# mark_begin;
       .align    16,0x90
nss_fasta_expr:
# parameter 1(p): %rdi
# parameter 2(end): %rsi
# parameter 3(expr): %rdx
# parameter 4(status): %rcx
# parameter 5(positional): %r8d
..B21.1:                        # Preds ..B21.0
..___tag_value_nss_fasta_expr.776:                              #1234.1
..LN2425:
   .loc    1  1234
        pushq     %rbp                                          #1234.1
        movq      %rsp, %rbp                                    #1234.1
..___tag_value_nss_fasta_expr.777:                              #
        subq      $48, %rsp                                     #1234.1
        movq      %r15, -24(%rbp)                               #1234.1
..___tag_value_nss_fasta_expr.779:                              #
        movq      %r14, -16(%rbp)                               #1234.1
..___tag_value_nss_fasta_expr.780:                              #
        movq      %r13, -48(%rbp)                               #1234.1
..___tag_value_nss_fasta_expr.781:                              #
        movq      %r12, -40(%rbp)                               #1234.1
..___tag_value_nss_fasta_expr.782:                              #
        movq      %rbx, -32(%rbp)                               #1234.1
..___tag_value_nss_fasta_expr.783:                              #
        movl      %r8d, %r13d                                   #1234.1
        movq      %rcx, %r15                                    #1234.1
        movq      %rdx, %r12                                    #1234.1
        movq      %rsi, %r14                                    #1234.1
        movq      %rdi, %rbx                                    #1234.1
        lea       _gprof_pack20(%rip), %rdx                     #1234.1
        call      mcount@PLT                                    #1234.1
                                # LOE rbx r12 r14 r15 r13d
..B21.20:                       # Preds ..B21.1
..LN2427:
   .loc    1  1236
        movsbl    (%rbx), %eax                                  #1236.16
        cmpl      $39, %eax                                     #1236.16
        jne       ..B21.7       # Prob 67%                      #1236.16
                                # LOE rbx r12 r14 r15 eax r13d
..B21.2:                        # Preds ..B21.20
..LN2429:
   .loc    1  1239
        incq      %rbx                                          #1239.30
..LN2431:
        movq      %rbx, %rdi                                    #1239.13
        movq      %r14, %rsi                                    #1239.13
        movq      %r12, %rdx                                    #1239.13
        movq      %r15, %rcx                                    #1239.13
        movl      %r13d, %r8d                                   #1239.13
        call      nss_FASTA_expr@PLT                            #1239.13
                                # LOE rax r14 r15
..B21.3:                        # Preds ..B21.2
..LN2433:
   .loc    1  1240
        movl      (%r15), %edx                                  #1240.16
..LN2435:
   .loc    1  1239
        movq      %rax, %rcx                                    #1239.13
..LN2437:
   .loc    1  1240
        testl     %edx, %edx                                    #1240.26
        jne       ..B21.13      # Prob 50%                      #1240.26
                                # LOE rax rcx r14 r15
..B21.4:                        # Preds ..B21.3
..LN2439:
        cmpq      %r14, %rax                                    #1240.38
        je        ..B21.12      # Prob 12%                      #1240.38
                                # LOE rax rcx r15
..B21.5:                        # Preds ..B21.4
..LN2441:
        movsbl    (%rax), %edx                                  #1240.47
        lea       1(%rax), %rcx                                 #1240.47
..LN2443:
        cmpl      $39, %edx                                     #1240.55
        jne       ..B21.12      # Prob 84%                      #1240.55
        jmp       ..B21.13      # Prob 100%                     #1240.55
                                # LOE rcx r15
..B21.7:                        # Preds ..B21.20
..LN2445:
   .loc    1  1236
        cmpl      $34, %eax                                     #1236.16
        jne       ..B21.14      # Prob 50%                      #1236.16
                                # LOE rbx r12 r14 r15 r13d
..B21.8:                        # Preds ..B21.7
..LN2447:
   .loc    1  1244
        incq      %rbx                                          #1244.30
..LN2449:
        movq      %rbx, %rdi                                    #1244.13
        movq      %r14, %rsi                                    #1244.13
        movq      %r12, %rdx                                    #1244.13
        movq      %r15, %rcx                                    #1244.13
        movl      %r13d, %r8d                                   #1244.13
        call      nss_FASTA_expr@PLT                            #1244.13
                                # LOE rax r14 r15
..B21.9:                        # Preds ..B21.8
..LN2451:
   .loc    1  1245
        movl      (%r15), %edx                                  #1245.16
..LN2453:
   .loc    1  1244
        movq      %rax, %rcx                                    #1244.13
..LN2455:
   .loc    1  1245
        testl     %edx, %edx                                    #1245.26
        jne       ..B21.13      # Prob 50%                      #1245.26
                                # LOE rax rcx r14 r15
..B21.10:                       # Preds ..B21.9
..LN2457:
        cmpq      %r14, %rax                                    #1245.38
        je        ..B21.12      # Prob 12%                      #1245.38
                                # LOE rax rcx r15
..B21.11:                       # Preds ..B21.10
..LN2459:
        movsbl    (%rax), %edx                                  #1245.47
        lea       1(%rax), %rcx                                 #1245.47
..LN2461:
        cmpl      $34, %edx                                     #1245.55
        je        ..B21.13      # Prob 16%                      #1245.55
                                # LOE rcx r15
..B21.12:                       # Preds ..B21.10 ..B21.4 ..B21.5 ..B21.11
..LN2463:
   .loc    1  1246
        movl      $22, (%r15)                                   #1246.13
                                # LOE rcx
..B21.13:                       # Preds ..B21.5 ..B21.12 ..B21.11 ..B21.3 ..B21.9
                                #      
..LN2465:
   .loc    1  1252
        movq      -32(%rbp), %rbx                               #1252.12
..___tag_value_nss_fasta_expr.784:                              #
        movq      -40(%rbp), %r12                               #1252.12
..___tag_value_nss_fasta_expr.785:                              #
        movq      -48(%rbp), %r13                               #1252.12
..___tag_value_nss_fasta_expr.786:                              #
        movq      -16(%rbp), %r14                               #1252.12
..___tag_value_nss_fasta_expr.787:                              #
        movq      %rcx, %rax                                    #1252.12
        movq      -24(%rbp), %r15                               #1252.12
..___tag_value_nss_fasta_expr.788:                              #
        movq      %rbp, %rsp                                    #1252.12
        popq      %rbp                                          #1252.12
..___tag_value_nss_fasta_expr.789:                              #
        ret                                                     #1252.12
..___tag_value_nss_fasta_expr.790:                              #
                                # LOE
..B21.14:                       # Preds ..B21.7
..LN2467:
   .loc    1  1249
        movq      %rbx, %rdi                                    #1249.16
        movq      %r14, %rsi                                    #1249.16
        movq      %r12, %rdx                                    #1249.16
        movq      %r15, %rcx                                    #1249.16
        movl      %r13d, %r8d                                   #1249.16
        call      nss_FASTA_expr@PLT                            #1249.16
                                # LOE rax
..B21.15:                       # Preds ..B21.14
        movq      -32(%rbp), %rbx                               #1249.16
..___tag_value_nss_fasta_expr.796:                              #
        movq      -40(%rbp), %r12                               #1249.16
..___tag_value_nss_fasta_expr.797:                              #
        movq      -48(%rbp), %r13                               #1249.16
..___tag_value_nss_fasta_expr.798:                              #
        movq      -16(%rbp), %r14                               #1249.16
..___tag_value_nss_fasta_expr.799:                              #
        movq      -24(%rbp), %r15                               #1249.16
..___tag_value_nss_fasta_expr.800:                              #
        movq      %rbp, %rsp                                    #1249.16
        popq      %rbp                                          #1249.16
..___tag_value_nss_fasta_expr.801:                              #
        ret                                                     #1249.16
        .align    16,0x90
..___tag_value_nss_fasta_expr.802:                              #
                                # LOE
# mark_end;
	.type	nss_fasta_expr,@function
	.size	nss_fasta_expr,.-nss_fasta_expr
.LNnss_fasta_expr:
	.data
# -- End  nss_fasta_expr
	.text
# -- Begin  nss_FASTA_expr
# mark_begin;
       .align    16,0x90
nss_FASTA_expr:
# parameter 1(p): %rdi
# parameter 2(end): %rsi
# parameter 3(expr): %rdx
# parameter 4(status): %rcx
# parameter 5(positional): %r8d
..B22.1:                        # Preds ..B22.0
..___tag_value_nss_FASTA_expr.803:                              #1192.1
..LN2469:
   .loc    1  1192
        pushq     %rbp                                          #1192.1
        movq      %rsp, %rbp                                    #1192.1
..___tag_value_nss_FASTA_expr.804:                              #
        subq      $48, %rsp                                     #1192.1
        movq      %r15, -48(%rbp)                               #1192.1
..___tag_value_nss_FASTA_expr.806:                              #
        movq      %r14, -40(%rbp)                               #1192.1
..___tag_value_nss_FASTA_expr.807:                              #
        movq      %r13, -32(%rbp)                               #1192.1
..___tag_value_nss_FASTA_expr.808:                              #
        movq      %r12, -24(%rbp)                               #1192.1
..___tag_value_nss_FASTA_expr.809:                              #
        movq      %rbx, -16(%rbp)                               #1192.1
..___tag_value_nss_FASTA_expr.810:                              #
        movl      %r8d, %r15d                                   #1192.1
        movq      %rcx, %r14                                    #1192.1
        movq      %rdx, %r13                                    #1192.1
        movq      %rsi, %rbx                                    #1192.1
        movq      %rdi, %r12                                    #1192.1
        lea       _gprof_pack21(%rip), %rdx                     #1192.1
        call      mcount@PLT                                    #1192.1
                                # LOE rbx r12 r13 r14 r15d
..B22.24:                       # Preds ..B22.1
..LN2471:
   .loc    1  1193
        cmpq      %rbx, %r12                                    #1193.15
        jae       ..B22.9       # Prob 44%                      #1193.15
                                # LOE rbx r12 r13 r14 r15d
..B22.3:                        # Preds ..B22.24
..LN2473:
   .loc    1  1192
        movq      %r12, %rdx                                    #1192.1
..LN2475:
   .loc    1  1199
        xorl      %eax, %eax                                    #1199.22
..LN2477:
   .loc    1  1200
        lea       fasta_2na_map.0(%rip), %rcx                   #1200.27
                                # LOE rdx rcx rbx r12 r13 r14 eax r15d
..B22.4:                        # Preds ..B22.6 ..B22.3
..LN2479:
   .loc    1  1203
        movsbl    (%r12), %esi                                  #1203.20
..LN2481:
        testl     %esi, %esi                                    #1203.24
        jl        ..B22.8       # Prob 20%                      #1203.24
                                # LOE rdx rcx rbx r12 r13 r14 eax r15d
..B22.5:                        # Preds ..B22.4
..LN2483:
   .loc    1  1206
        movzbl    (%r12), %esi                                  #1206.45
..LN2485:
        movsbl    (%rsi,%rcx), %edi                             #1206.18
..LN2487:
        testl     %edi, %edi                                    #1206.51
        jl        ..B22.16      # Prob 16%                      #1206.51
                                # LOE rdx rcx rbx r12 r13 r14 eax esi r15d
..B22.6:                        # Preds ..B22.18 ..B22.5
..LN2489:
   .loc    1  1216
        incq      %r12                                          #1216.20
..LN2491:
        cmpq      %rbx, %r12                                    #1216.24
        jb        ..B22.4       # Prob 82%                      #1216.24
                                # LOE rdx rcx rbx r12 r13 r14 eax r15d
..B22.8:                        # Preds ..B22.17 ..B22.16 ..B22.4 ..B22.6
..LN2493:
   .loc    1  1218
        cmpq      %rdx, %r12                                    #1218.19
        ja        ..B22.11      # Prob 12%                      #1218.19
                                # LOE rdx r12 r13 r14 eax r15d
..B22.9:                        # Preds ..B22.24 ..B22.8
..LN2495:
   .loc    1  1219
        movl      $22, (%r14)                                   #1219.13
                                # LOE r12
..B22.10:                       # Preds ..B22.9 ..B22.13 ..B22.15
..LN2497:
   .loc    1  1226
        movq      -16(%rbp), %rbx                               #1226.12
..___tag_value_nss_FASTA_expr.811:                              #
        movq      -32(%rbp), %r13                               #1226.12
..___tag_value_nss_FASTA_expr.812:                              #
        movq      -40(%rbp), %r14                               #1226.12
..___tag_value_nss_FASTA_expr.813:                              #
        movq      -48(%rbp), %r15                               #1226.12
..___tag_value_nss_FASTA_expr.814:                              #
        movq      %r12, %rax                                    #1226.12
        movq      -24(%rbp), %r12                               #1226.12
..___tag_value_nss_FASTA_expr.815:                              #
        movq      %rbp, %rsp                                    #1226.12
        popq      %rbp                                          #1226.12
..___tag_value_nss_FASTA_expr.816:                              #
        ret                                                     #1226.12
..___tag_value_nss_FASTA_expr.817:                              #
                                # LOE
..B22.11:                       # Preds ..B22.8                 # Infreq
..LN2499:
   .loc    1  1220
        testl     %eax, %eax                                    #1220.27
        jne       ..B22.14      # Prob 50%                      #1220.27
                                # LOE rdx r12 r13 r14 r15d
..B22.12:                       # Preds ..B22.11                # Infreq
..LN2501:
   .loc    1  1221
        movq      %r12, %rcx                                    #1221.72
        subq      %rdx, %rcx                                    #1221.72
..LN2503:
        movq      %r13, %rdi                                    #1221.24
        movl      %r15d, %esi                                   #1221.24
        call      NucStrFastaExprMake2@PLT                      #1221.24
                                # LOE r12 r14 eax
..B22.13:                       # Preds ..B22.12                # Infreq
..LN2505:
        movl      %eax, (%r14)                                  #1221.13
        jmp       ..B22.10      # Prob 100%                     #1221.13
                                # LOE r12
..B22.14:                       # Preds ..B22.11                # Infreq
..LN2507:
   .loc    1  1223
        movq      %r12, %rcx                                    #1223.72
        subq      %rdx, %rcx                                    #1223.72
..LN2509:
        movq      %r13, %rdi                                    #1223.24
        movl      %r15d, %esi                                   #1223.24
        call      NucStrFastaExprMake4@PLT                      #1223.24
                                # LOE r12 r14 eax
..B22.15:                       # Preds ..B22.14                # Infreq
..LN2511:
        movl      %eax, (%r14)                                  #1223.13
        jmp       ..B22.10      # Prob 100%                     #1223.13
                                # LOE r12
..B22.16:                       # Preds ..B22.5                 # Infreq
..LN2513:
   .loc    1  1208
        lea       fasta_4na_map.0(%rip), %rdi                   #1208.29
        cmpq      %rdi, %rcx                                    #1208.29
        je        ..B22.8       # Prob 20%                      #1208.29
                                # LOE rdx rbx r12 r13 r14 eax esi r15d
..B22.17:                       # Preds ..B22.16                # Infreq
..LN2515:
   .loc    1  1210
        movl      %esi, %esi                                    #1210.22
        lea       fasta_4na_map.0(%rip), %rcx                   #1210.22
        movsbl    (%rcx,%rsi), %edi                             #1210.22
..LN2517:
        testl     %edi, %edi                                    #1210.65
        jl        ..B22.8       # Prob 20%                      #1210.65
                                # LOE rdx rbx r12 r13 r14 eax r15d
..B22.18:                       # Preds ..B22.17                # Infreq
..LN2519:
   .loc    1  1212
        movl      $1, %eax                                      #1212.17
..LN2521:
   .loc    1  1213
        lea       fasta_4na_map.0(%rip), %rcx                   #1213.23
        jmp       ..B22.6       # Prob 100%                     #1213.23
        .align    16,0x90
..___tag_value_nss_FASTA_expr.823:                              #
                                # LOE rdx rcx rbx r12 r13 r14 eax r15d
# mark_end;
	.type	nss_FASTA_expr,@function
	.size	nss_FASTA_expr,.-nss_FASTA_expr
.LNnss_FASTA_expr:
	.data
# -- End  nss_FASTA_expr
	.text
# -- Begin  NucStrFastaExprMake2
# mark_begin;
       .align    16,0x90
NucStrFastaExprMake2:
# parameter 1(expr): %rdi
# parameter 2(positional): %esi
# parameter 3(fasta): %rdx
# parameter 4(size): %rcx
..B23.1:                        # Preds ..B23.0
..___tag_value_NucStrFastaExprMake2.824:                        #838.1
..LN2523:
   .loc    1  838
        pushq     %rbp                                          #838.1
        movq      %rsp, %rbp                                    #838.1
..___tag_value_NucStrFastaExprMake2.825:                        #
        subq      $80, %rsp                                     #838.1
        movq      %r15, -32(%rbp)                               #838.1
..___tag_value_NucStrFastaExprMake2.827:                        #
        movq      %r14, -48(%rbp)                               #838.1
..___tag_value_NucStrFastaExprMake2.828:                        #
        movq      %r13, -40(%rbp)                               #838.1
..___tag_value_NucStrFastaExprMake2.829:                        #
        movq      %r12, -16(%rbp)                               #838.1
..___tag_value_NucStrFastaExprMake2.830:                        #
        movq      %rbx, -24(%rbp)                               #838.1
..___tag_value_NucStrFastaExprMake2.831:                        #
        movq      %rcx, %r13                                    #838.1
        movq      %rdx, %r14                                    #838.1
        movl      %esi, %r12d                                   #838.1
        movq      %rdi, %r15                                    #838.1
        lea       _gprof_pack22(%rip), %rdx                     #838.1
        call      mcount@PLT                                    #838.1
                                # LOE r13 r14 r15 r12d
..B23.60:                       # Preds ..B23.1
..LN2525:
   .loc    1  844
        cmpq      $61, %r13                                     #844.17
        jbe       ..B23.3       # Prob 43%                      #844.17
                                # LOE r13 r14 r15 r12d
..B23.2:                        # Preds ..B23.60
..LN2527:
   .loc    1  845
        movq      -24(%rbp), %rbx                               #845.16
..___tag_value_NucStrFastaExprMake2.832:                        #
        movq      -16(%rbp), %r12                               #845.16
..___tag_value_NucStrFastaExprMake2.833:                        #
        movq      -40(%rbp), %r13                               #845.16
..___tag_value_NucStrFastaExprMake2.834:                        #
        movq      -48(%rbp), %r14                               #845.16
..___tag_value_NucStrFastaExprMake2.835:                        #
        movl      $7, %eax                                      #845.16
        movq      -32(%rbp), %r15                               #845.16
..___tag_value_NucStrFastaExprMake2.836:                        #
        movq      %rbp, %rsp                                    #845.16
        popq      %rbp                                          #845.16
..___tag_value_NucStrFastaExprMake2.837:                        #
        ret                                                     #845.16
..___tag_value_NucStrFastaExprMake2.838:                        #
                                # LOE
..B23.3:                        # Preds ..B23.60
..LN2529:
   .loc    1  847
        movl      $144, %edi                                    #847.9
        call      NucStrFastaExprAlloc@PLT                      #847.9
                                # LOE rax r13 r14 r15 r12d
..B23.61:                       # Preds ..B23.3
        movq      %rax, %rbx                                    #847.9
                                # LOE rbx r13 r14 r15 r12d
..B23.4:                        # Preds ..B23.61
..LN2531:
   .loc    1  848
        testq     %rbx, %rbx                                    #848.15
        je        ..B23.57      # Prob 5%                       #848.15
                                # LOE rbx r13 r14 r15 r12d
..B23.5:                        # Preds ..B23.4
..LN2533:
   .loc    1  851
        movq      %rbx, (%r15)                                  #851.5
..LN2535:
   .loc    1  852
        movl      %r13d, 4(%rbx)                                #852.5
..LN2537:
   .loc    1  856
        xorl      %edi, %edi                                    #856.11
..LN2539:
        testq     %r13, %r13                                    #856.22
        jbe       ..B23.16      # Prob 16%                      #856.22
                                # LOE rbx rdi r13 r14 r12d
..B23.7:                        # Preds ..B23.5 ..B23.14
..LN2541:
   .loc    1  858
        movsbq    (%rdi,%r14), %rdx                             #858.48
..LN2543:
        lea       fasta_2na_map.0(%rip), %rax                   #858.24
        movzbl    (%rax,%rdx), %edx                             #858.24
..LN2545:
   .loc    1  859
        movq      %rdi, %rax                                    #859.22
        andq      $3, %rax                                      #859.22
        je        ..B23.56      # Prob 20%                      #859.22
                                # LOE rax rbx rdi r13 r14 edx r12d
..B23.8:                        # Preds ..B23.7
        cmpq      $1, %rax                                      #859.22
        je        ..B23.13      # Prob 25%                      #859.22
                                # LOE rax rbx rdi r13 r14 edx r12d
..B23.9:                        # Preds ..B23.8
        cmpq      $2, %rax                                      #859.22
        jne       ..B23.11      # Prob 67%                      #859.22
                                # LOE rax rbx rdi r13 r14 edx r12d
..B23.10:                       # Preds ..B23.9
..LN2547:
   .loc    1  870
        movq      %rdi, %rax                                    #870.32
        shrq      $2, %rax                                      #870.32
..LN2549:
        shll      $2, %edx                                      #870.47
..LN2551:
        orb       %dl, -80(%rbp,%rax)                           #870.13
..LN2553:
   .loc    1  871
        orb       $12, -64(%rbp,%rax)                           #871.13
        jmp       ..B23.14      # Prob 100%                     #871.13
                                # LOE rbx rdi r13 r14 r12d
..B23.11:                       # Preds ..B23.9
..LN2555:
   .loc    1  859
        cmpq      $3, %rax                                      #859.22
        jne       ..B23.14      # Prob 50%                      #859.22
                                # LOE rbx rdi r13 r14 edx r12d
..B23.12:                       # Preds ..B23.11
..LN2557:
   .loc    1  874
        movq      %rdi, %rax                                    #874.32
        shrq      $2, %rax                                      #874.32
..LN2559:
        orb       %dl, -80(%rbp,%rax)                           #874.13
..LN2561:
   .loc    1  875
        orb       $3, -64(%rbp,%rax)                            #875.13
        jmp       ..B23.14      # Prob 100%                     #875.13
                                # LOE rbx rdi r13 r14 r12d
..B23.13:                       # Preds ..B23.8
..LN2563:
   .loc    1  866
        movq      %rdi, %rax                                    #866.32
        shrq      $2, %rax                                      #866.32
..LN2565:
        shll      $4, %edx                                      #866.47
..LN2567:
        orb       %dl, -80(%rbp,%rax)                           #866.13
..LN2569:
   .loc    1  867
        orb       $48, -64(%rbp,%rax)                           #867.13
                                # LOE rbx rdi r13 r14 r12d
..B23.14:                       # Preds ..B23.11 ..B23.56 ..B23.13 ..B23.10 ..B23.12
                                #      
..LN2571:
   .loc    1  856
        incq      %rdi                                          #856.31
..LN2573:
        cmpq      %r13, %rdi                                    #856.22
        jb        ..B23.7       # Prob 82%                      #856.22
                                # LOE rbx rdi r13 r14 r12d
..B23.16:                       # Preds ..B23.14 ..B23.5
..LN2575:
   .loc    1  881
        addq      $3, %rdi                                      #881.21
..LN2577:
        shrq      $2, %rdi                                      #881.28
..LN2579:
        cmpq      $16, %rdi                                     #881.35
        jae       ..B23.29      # Prob 50%                      #881.35
                                # LOE rbx rdi r13 r12d
..B23.17:                       # Preds ..B23.16
..LN2581:
   .loc    1  883
        lea       -80(%rbp), %rdx                               #883.9
..LN2583:
   .loc    1  881
        movq      %rdi, %rsi                                    #881.5
        negq      %rsi                                          #881.5
        addq      $16, %rsi                                     #881.5
        lea       (%rdx,%rdi), %rcx                             #881.5
        movq      %rcx, %rdx                                    #881.5
        andq      $15, %rdx                                     #881.5
        movq      %rdx, %rax                                    #881.5
        negq      %rax                                          #881.5
        addq      $16, %rax                                     #881.5
        cmpq      $0, %rdx                                      #881.5
        cmovne    %rax, %rdx                                    #881.5
        lea       16(%rdx), %r8                                 #881.5
        cmpq      %r8, %rsi                                     #881.5
        jl        ..B23.55      # Prob 10%                      #881.5
                                # LOE rdx rcx rbx rsi rdi r13 r12d
..B23.18:                       # Preds ..B23.17
        movq      %rsi, %rax                                    #881.5
        subq      %rdx, %rax                                    #881.5
        andq      $15, %rax                                     #881.5
        negq      %rax                                          #881.5
        addq      %rsi, %rax                                    #881.5
        xorl      %r9d, %r9d                                    #881.5
        testq     %rdx, %rdx                                    #881.5
        jbe       ..B23.22      # Prob 10%                      #881.5
                                # LOE rax rdx rcx rbx rsi rdi r9 r13 r12d
..B23.19:                       # Preds ..B23.18
..LN2585:
   .loc    1  884
        lea       -64(%rbp), %r8                                #884.9
        addq      %rdi, %r8                                     #
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r13 r12d
..B23.20:                       # Preds ..B23.20 ..B23.19
..LN2587:
   .loc    1  883
        xorl      %r10d, %r10d                                  #883.9
        movb      %r10b, (%r9,%rcx)                             #883.9
..LN2589:
   .loc    1  884
        movb      %r10b, (%r9,%r8)                              #884.9
..LN2591:
   .loc    1  881
        incq      %r9                                           #881.5
        cmpq      %rdx, %r9                                     #881.5
        jb        ..B23.20      # Prob 82%                      #881.5
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r13 r12d
..B23.22:                       # Preds ..B23.20 ..B23.18
..LN2593:
   .loc    1  883
        lea       -80(%rbp), %r9                                #883.9
..LN2595:
   .loc    1  884
        lea       -64(%rbp), %r8                                #884.9
        addq      %rdi, %r9                                     #
        addq      %rdi, %r8                                     #
..LN2597:
   .loc    1  883
        pxor      %xmm0, %xmm0                                  #883.29
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r13 r12d xmm0
..B23.23:                       # Preds ..B23.23 ..B23.22
..LN2599:
        movdqa    %xmm0, (%rdx,%r9)                             #883.9
..LN2601:
   .loc    1  884
        movdqa    %xmm0, (%rdx,%r8)                             #884.9
..LN2603:
   .loc    1  881
        addq      $16, %rdx                                     #881.5
        cmpq      %rax, %rdx                                    #881.5
        jb        ..B23.23      # Prob 82%                      #881.5
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r13 r12d xmm0
..B23.25:                       # Preds ..B23.23 ..B23.55
        cmpq      %rsi, %rax                                    #881.5
        jae       ..B23.29      # Prob 10%                      #881.5
                                # LOE rax rcx rbx rsi rdi r13 r12d
..B23.26:                       # Preds ..B23.25
..LN2605:
   .loc    1  884
        lea       -64(%rbp), %rdx                               #884.9
        addq      %rdx, %rdi                                    #
                                # LOE rax rcx rbx rsi rdi r13 r12d
..B23.27:                       # Preds ..B23.27 ..B23.26
..LN2607:
   .loc    1  883
        xorl      %edx, %edx                                    #883.9
        movb      %dl, (%rax,%rcx)                              #883.9
..LN2609:
   .loc    1  884
        movb      %dl, (%rax,%rdi)                              #884.9
..LN2611:
   .loc    1  881
        incq      %rax                                          #881.5
        cmpq      %rsi, %rax                                    #881.5
        jb        ..B23.27      # Prob 82%                      #881.5
                                # LOE rax rcx rbx rsi rdi r13 r12d
..B23.29:                       # Preds ..B23.27 ..B23.25 ..B23.16
..LN2613:
   .loc    1  890
        testl     %r12d, %r12d                                  #890.10
        je        ..B23.31      # Prob 50%                      #890.10
                                # LOE rbx r13
..B23.30:                       # Preds ..B23.29
..LN2615:
   .loc    1  892
        movl      $9, (%rbx)                                    #892.9
        jmp       ..B23.40      # Prob 100%                     #892.9
                                # LOE rbx
..B23.31:                       # Preds ..B23.29
..LN2617:
   .loc    1  896
        cmpq      $2, %r13                                      #896.22
        jae       ..B23.33      # Prob 50%                      #896.22
                                # LOE rbx r13
..B23.32:                       # Preds ..B23.31
..LN2619:
   .loc    1  898
        movzbl    -80(%rbp), %eax                               #898.29
..LN2621:
   .loc    1  903
        movzbl    -64(%rbp), %edi                               #903.26
..LN2623:
   .loc    1  908
        movl      $2, (%rbx)                                    #908.9
..LN2625:
   .loc    1  898
        movb      %al, -79(%rbp)                                #898.9
..LN2627:
   .loc    1  899
        movzwl    -80(%rbp), %edx                               #899.29
..LN2629:
        movw      %dx, -78(%rbp)                                #899.9
..LN2631:
   .loc    1  900
        movl      -80(%rbp), %ecx                               #900.29
..LN2633:
   .loc    1  901
        movl      -80(%rbp), %esi                               #901.29
..LN2635:
   .loc    1  900
        movl      %ecx, -76(%rbp)                               #900.9
..LN2637:
   .loc    1  903
        movb      %dil, -63(%rbp)                               #903.9
..LN2639:
   .loc    1  904
        movzwl    -64(%rbp), %r8d                               #904.26
..LN2641:
        movw      %r8w, -62(%rbp)                               #904.9
..LN2643:
   .loc    1  905
        movl      -64(%rbp), %r9d                               #905.26
..LN2645:
   .loc    1  906
        movl      -64(%rbp), %r10d                              #906.26
..LN2647:
   .loc    1  905
        movl      %r9d, -60(%rbp)                               #905.9
..LN2649:
   .loc    1  900
        movl      %ecx, %ecx                                    #900.9
..LN2651:
   .loc    1  901
        shlq      $32, %rcx                                     #901.29
..LN2653:
   .loc    1  905
        movl      %r9d, %r9d                                    #905.9
..LN2655:
   .loc    1  906
        shlq      $32, %r9                                      #906.26
..LN2657:
   .loc    1  901
        orq       %rsi, %rcx                                    #901.29
..LN2659:
        movq      %rcx, -72(%rbp)                               #901.9
..LN2661:
   .loc    1  906
        orq       %r10, %r9                                     #906.26
..LN2663:
        movq      %r9, -56(%rbp)                                #906.9
        jmp       ..B23.40      # Prob 100%                     #906.9
                                # LOE rbx
..B23.33:                       # Preds ..B23.31
..LN2665:
   .loc    1  910
        cmpq      $6, %r13                                      #910.22
        jae       ..B23.35      # Prob 50%                      #910.22
                                # LOE rbx r13
..B23.34:                       # Preds ..B23.33
..LN2667:
   .loc    1  912
        movzwl    -80(%rbp), %eax                               #912.29
..LN2669:
   .loc    1  916
        movzwl    -64(%rbp), %esi                               #916.26
..LN2671:
   .loc    1  920
        movl      $3, (%rbx)                                    #920.9
..LN2673:
   .loc    1  912
        movw      %ax, -78(%rbp)                                #912.9
..LN2675:
   .loc    1  913
        movl      -80(%rbp), %edx                               #913.29
..LN2677:
   .loc    1  914
        movl      -80(%rbp), %ecx                               #914.29
..LN2679:
   .loc    1  913
        movl      %edx, -76(%rbp)                               #913.9
..LN2681:
   .loc    1  916
        movw      %si, -62(%rbp)                                #916.9
..LN2683:
   .loc    1  917
        movl      -64(%rbp), %edi                               #917.26
..LN2685:
   .loc    1  918
        movl      -64(%rbp), %r8d                               #918.26
..LN2687:
   .loc    1  917
        movl      %edi, -60(%rbp)                               #917.9
..LN2689:
   .loc    1  913
        movl      %edx, %edx                                    #913.9
..LN2691:
   .loc    1  914
        shlq      $32, %rdx                                     #914.29
..LN2693:
   .loc    1  917
        movl      %edi, %edi                                    #917.9
..LN2695:
   .loc    1  918
        shlq      $32, %rdi                                     #918.26
..LN2697:
   .loc    1  914
        orq       %rcx, %rdx                                    #914.29
..LN2699:
        movq      %rdx, -72(%rbp)                               #914.9
..LN2701:
   .loc    1  918
        orq       %r8, %rdi                                     #918.26
..LN2703:
        movq      %rdi, -56(%rbp)                               #918.9
        jmp       ..B23.40      # Prob 100%                     #918.9
                                # LOE rbx
..B23.35:                       # Preds ..B23.33
..LN2705:
   .loc    1  922
        cmpq      $14, %r13                                     #922.22
        jae       ..B23.37      # Prob 50%                      #922.22
                                # LOE rbx r13
..B23.36:                       # Preds ..B23.35
..LN2707:
   .loc    1  924
        movl      -80(%rbp), %eax                               #924.29
..LN2709:
   .loc    1  925
        movl      -80(%rbp), %edx                               #925.29
..LN2711:
   .loc    1  927
        movl      -64(%rbp), %ecx                               #927.26
..LN2713:
   .loc    1  928
        movl      -64(%rbp), %esi                               #928.26
..LN2715:
   .loc    1  924
        movl      %eax, -76(%rbp)                               #924.9
..LN2717:
   .loc    1  930
        movl      $4, (%rbx)                                    #930.9
..LN2719:
   .loc    1  924
        movl      %eax, %eax                                    #924.9
..LN2721:
   .loc    1  925
        shlq      $32, %rax                                     #925.29
..LN2723:
   .loc    1  927
        movl      %ecx, -60(%rbp)                               #927.9
        movl      %ecx, %ecx                                    #927.9
..LN2725:
   .loc    1  928
        shlq      $32, %rcx                                     #928.26
..LN2727:
   .loc    1  925
        orq       %rdx, %rax                                    #925.29
..LN2729:
        movq      %rax, -72(%rbp)                               #925.9
..LN2731:
   .loc    1  928
        orq       %rsi, %rcx                                    #928.26
..LN2733:
        movq      %rcx, -56(%rbp)                               #928.9
        jmp       ..B23.40      # Prob 100%                     #928.9
                                # LOE rbx
..B23.37:                       # Preds ..B23.35
..LN2735:
   .loc    1  932
        cmpq      $30, %r13                                     #932.22
        jae       ..B23.39      # Prob 50%                      #932.22
                                # LOE rbx
..B23.38:                       # Preds ..B23.37
..LN2737:
   .loc    1  934
        movq      -80(%rbp), %rax                               #934.29
..LN2739:
   .loc    1  935
        movq      -64(%rbp), %rdx                               #935.26
..LN2741:
   .loc    1  937
        movl      $0, (%rbx)                                    #937.9
..LN2743:
   .loc    1  934
        movq      %rax, -72(%rbp)                               #934.9
..LN2745:
   .loc    1  935
        movq      %rdx, -56(%rbp)                               #935.9
        jmp       ..B23.40      # Prob 100%                     #935.9
                                # LOE rbx
..B23.39:                       # Preds ..B23.37
..LN2747:
   .loc    1  941
        movl      $5, (%rbx)                                    #941.9
                                # LOE rbx
..B23.40:                       # Preds ..B23.30 ..B23.32 ..B23.34 ..B23.36 ..B23.38
                                #       ..B23.39
..LN2749:
   .loc    1  944
        movq      -80(%rbp), %rax                               #944.42
..LN2751:
   .loc    1  948
        lea       -80(%rbp), %rdi                               #948.5
..LN2753:
   .loc    1  944
        movq      %rax, 16(%rbx)                                #944.42
        movq      -72(%rbp), %rdx                               #944.42
        movq      %rdx, 24(%rbx)                                #944.42
..LN2755:
   .loc    1  945
        movq      -64(%rbp), %rcx                               #945.39
        movq      %rcx, 32(%rbx)                                #945.39
        movq      -56(%rbp), %rsi                               #945.39
        movq      %rsi, 40(%rbx)                                #945.39
..LN2757:
   .loc    1  948
        call      uint128_bswap@PLT                             #948.5
                                # LOE rbx
..B23.41:                       # Preds ..B23.40
..LN2759:
   .loc    1  949
        lea       -64(%rbp), %rdi                               #949.5
        call      uint128_bswap@PLT                             #949.5
                                # LOE rbx
..B23.42:                       # Preds ..B23.41
..LN2761:
   .loc    1  952
        lea       -80(%rbp), %rdi                               #952.5
        movl      $2, %esi                                      #952.5
        call      uint128_shr@PLT                               #952.5
                                # LOE rbx
..B23.43:                       # Preds ..B23.42
..LN2763:
   .loc    1  953
        lea       -64(%rbp), %rdi                               #953.5
        movl      $2, %esi                                      #953.5
        call      uint128_shr@PLT                               #953.5
                                # LOE rbx
..B23.44:                       # Preds ..B23.43
..LN2765:
   .loc    1  956
        lea       -80(%rbp), %rsi                               #956.5
        lea       48(%rbx), %rdi                                #956.5
        call      uint128_bswap_copy@PLT                        #956.5
                                # LOE rbx
..B23.45:                       # Preds ..B23.44
..LN2767:
   .loc    1  957
        lea       -64(%rbp), %rsi                               #957.5
        lea       64(%rbx), %rdi                                #957.5
        call      uint128_bswap_copy@PLT                        #957.5
                                # LOE rbx
..B23.46:                       # Preds ..B23.45
..LN2769:
   .loc    1  959
        lea       -80(%rbp), %rdi                               #959.5
        movl      $2, %esi                                      #959.5
        call      uint128_shr@PLT                               #959.5
                                # LOE rbx
..B23.47:                       # Preds ..B23.46
..LN2771:
   .loc    1  960
        lea       -64(%rbp), %rdi                               #960.5
        movl      $2, %esi                                      #960.5
        call      uint128_shr@PLT                               #960.5
                                # LOE rbx
..B23.48:                       # Preds ..B23.47
..LN2773:
   .loc    1  962
        lea       -80(%rbp), %rsi                               #962.5
        lea       80(%rbx), %rdi                                #962.5
        call      uint128_bswap_copy@PLT                        #962.5
                                # LOE rbx
..B23.49:                       # Preds ..B23.48
..LN2775:
   .loc    1  963
        lea       -64(%rbp), %rsi                               #963.5
        lea       96(%rbx), %rdi                                #963.5
        call      uint128_bswap_copy@PLT                        #963.5
                                # LOE rbx
..B23.50:                       # Preds ..B23.49
..LN2777:
   .loc    1  965
        lea       -80(%rbp), %rdi                               #965.5
        movl      $2, %esi                                      #965.5
        call      uint128_shr@PLT                               #965.5
                                # LOE rbx
..B23.51:                       # Preds ..B23.50
..LN2779:
   .loc    1  966
        lea       -64(%rbp), %rdi                               #966.5
        movl      $2, %esi                                      #966.5
        call      uint128_shr@PLT                               #966.5
                                # LOE rbx
..B23.52:                       # Preds ..B23.51
..LN2781:
   .loc    1  968
        lea       -80(%rbp), %rsi                               #968.5
        lea       112(%rbx), %rdi                               #968.5
        call      uint128_bswap_copy@PLT                        #968.5
                                # LOE rbx
..B23.53:                       # Preds ..B23.52
..LN2783:
   .loc    1  969
        lea       -64(%rbp), %rsi                               #969.5
        addq      $128, %rbx                                    #969.5
        movq      %rbx, %rdi                                    #969.5
        call      uint128_bswap_copy@PLT                        #969.5
                                # LOE
..B23.54:                       # Preds ..B23.53
..LN2785:
   .loc    1  1011
        movq      -24(%rbp), %rbx                               #1011.12
..___tag_value_NucStrFastaExprMake2.844:                        #
        movq      -16(%rbp), %r12                               #1011.12
..___tag_value_NucStrFastaExprMake2.845:                        #
        movq      -40(%rbp), %r13                               #1011.12
..___tag_value_NucStrFastaExprMake2.846:                        #
        movq      -48(%rbp), %r14                               #1011.12
..___tag_value_NucStrFastaExprMake2.847:                        #
        xorl      %eax, %eax                                    #1011.12
        movq      -32(%rbp), %r15                               #1011.12
..___tag_value_NucStrFastaExprMake2.848:                        #
        movq      %rbp, %rsp                                    #1011.12
        popq      %rbp                                          #1011.12
..___tag_value_NucStrFastaExprMake2.849:                        #
        ret                                                     #1011.12
..___tag_value_NucStrFastaExprMake2.850:                        #
                                # LOE
..B23.55:                       # Preds ..B23.17                # Infreq
..LN2787:
   .loc    1  881
        xorl      %eax, %eax                                    #881.5
        jmp       ..B23.25      # Prob 100%                     #881.5
                                # LOE rax rcx rbx rsi rdi r13 r12d
..B23.56:                       # Preds ..B23.7                 # Infreq
..LN2789:
   .loc    1  862
        movq      %rdi, %rax                                    #862.32
        shrq      $2, %rax                                      #862.32
..LN2791:
        shll      $6, %edx                                      #862.46
..LN2793:
        movb      %dl, -80(%rbp,%rax)                           #862.13
..LN2795:
   .loc    1  863
        movb      $192, -64(%rbp,%rax)                          #863.13
        jmp       ..B23.14      # Prob 100%                     #863.13
                                # LOE rbx rdi r13 r14 r12d
..B23.57:                       # Preds ..B23.4                 # Infreq
..LN2797:
   .loc    1  849
        call      __errno_location@PLT                          #849.16
                                # LOE rax
..B23.62:                       # Preds ..B23.57                # Infreq
        movq      -24(%rbp), %rbx                               #849.16
..___tag_value_NucStrFastaExprMake2.856:                        #
        movq      -16(%rbp), %r12                               #849.16
..___tag_value_NucStrFastaExprMake2.857:                        #
        movq      -40(%rbp), %r13                               #849.16
..___tag_value_NucStrFastaExprMake2.858:                        #
        movq      -48(%rbp), %r14                               #849.16
..___tag_value_NucStrFastaExprMake2.859:                        #
        movl      (%rax), %eax                                  #849.16
        movq      -32(%rbp), %r15                               #849.16
..___tag_value_NucStrFastaExprMake2.860:                        #
        movq      %rbp, %rsp                                    #849.16
        popq      %rbp                                          #849.16
..___tag_value_NucStrFastaExprMake2.861:                        #
        ret                                                     #849.16
        .align    16,0x90
..___tag_value_NucStrFastaExprMake2.862:                        #
                                # LOE
# mark_end;
	.type	NucStrFastaExprMake2,@function
	.size	NucStrFastaExprMake2,.-NucStrFastaExprMake2
.LNNucStrFastaExprMake2:
	.data
# -- End  NucStrFastaExprMake2
	.text
# -- Begin  uint128_bswap_copy
# mark_begin;
       .align    16,0x90
uint128_bswap_copy:
# parameter 1(to): %rdi
# parameter 2(from): %rsi
..B24.1:                        # Preds ..B24.0
..___tag_value_uint128_bswap_copy.863:                          #311.1
..LN2799:
   .loc    2  311
        pushq     %rbp                                          #311.1
        movq      %rsp, %rbp                                    #311.1
..___tag_value_uint128_bswap_copy.864:                          #
        subq      $16, %rsp                                     #311.1
        movq      %r13, -16(%rbp)                               #311.1
..___tag_value_uint128_bswap_copy.866:                          #
        movq      %r12, -8(%rbp)                                #311.1
..___tag_value_uint128_bswap_copy.867:                          #
        movq      %rsi, %r13                                    #311.1
        movq      %rdi, %r12                                    #311.1
        lea       _gprof_pack23(%rip), %rdx                     #311.1
        call      mcount@PLT                                    #311.1
                                # LOE rbx r12 r13 r14 r15
..B24.4:                        # Preds ..B24.1
..LN2801:
   .loc    2  312
        movq      %r12, %rdi                                    #312.0
        movq      %r13, %rsi                                    #312.0
        movq      (%rsi), %rax                                  #312.0
        movq      8(%rsi), %rcx                                 #312.0
        bswap     %rax                                          #312.0
        bswap     %rcx                                          #312.0
        movq      %rax, 8(%rdi)                                 #312.0
        movq      %rcx, (%rdi)                                  #312.0
..LN2803:
   .loc    2  324
        movq      -8(%rbp), %r12                                #324.1
..___tag_value_uint128_bswap_copy.868:                          #
        movq      -16(%rbp), %r13                               #324.1
..___tag_value_uint128_bswap_copy.869:                          #
        movq      %rbp, %rsp                                    #324.1
        popq      %rbp                                          #324.1
..___tag_value_uint128_bswap_copy.870:                          #
        ret                                                     #324.1
        .align    16,0x90
..___tag_value_uint128_bswap_copy.871:                          #
                                # LOE
# mark_end;
	.type	uint128_bswap_copy,@function
	.size	uint128_bswap_copy,.-uint128_bswap_copy
.LNuint128_bswap_copy:
	.data
# -- End  uint128_bswap_copy
	.text
# -- Begin  uint128_shr
# mark_begin;
       .align    16,0x90
uint128_shr:
# parameter 1(self): %rdi
# parameter 2(i): %esi
..B25.1:                        # Preds ..B25.0
..___tag_value_uint128_shr.872:                                 #262.1
..LN2805:
   .loc    2  262
        pushq     %rbp                                          #262.1
        movq      %rsp, %rbp                                    #262.1
..___tag_value_uint128_shr.873:                                 #
        subq      $16, %rsp                                     #262.1
        movq      %r13, -16(%rbp)                               #262.1
..___tag_value_uint128_shr.875:                                 #
        movq      %r12, -8(%rbp)                                #262.1
..___tag_value_uint128_shr.876:                                 #
        movl      %esi, %r13d                                   #262.1
        movq      %rdi, %r12                                    #262.1
        lea       _gprof_pack24(%rip), %rdx                     #262.1
        call      mcount@PLT                                    #262.1
                                # LOE rbx r12 r14 r15 r13d
..B25.4:                        # Preds ..B25.1
..LN2807:
   .loc    2  263
        movq      %r12, %rdi                                    #263.0
        movl      %r13d, %esi                                   #263.0
        movl      %esi, %ecx                                    #263.0
        movq      8(%rdi), %rax                                 #263.0
        shrdq     %cl, %rax, (%rdi)                             #263.0
        shrq      %cl, %rax                                     #263.0
        movq      %rax, 8(%rdi)                                 #263.0
..LN2809:
   .loc    2  274
        movq      -8(%rbp), %r12                                #274.1
..___tag_value_uint128_shr.877:                                 #
        movq      -16(%rbp), %r13                               #274.1
..___tag_value_uint128_shr.878:                                 #
        movq      %rbp, %rsp                                    #274.1
        popq      %rbp                                          #274.1
..___tag_value_uint128_shr.879:                                 #
        ret                                                     #274.1
        .align    16,0x90
..___tag_value_uint128_shr.880:                                 #
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
       .align    16,0x90
uint128_bswap:
# parameter 1(self): %rdi
..B26.1:                        # Preds ..B26.0
..___tag_value_uint128_bswap.881:                               #294.1
..LN2811:
   .loc    2  294
        pushq     %rbp                                          #294.1
        movq      %rsp, %rbp                                    #294.1
..___tag_value_uint128_bswap.882:                               #
        subq      $16, %rsp                                     #294.1
        movq      %r13, -16(%rbp)                               #294.1
..___tag_value_uint128_bswap.884:                               #
        movq      %rdi, %r13                                    #294.1
        lea       _gprof_pack25(%rip), %rdx                     #294.1
        call      mcount@PLT                                    #294.1
                                # LOE rbx r12 r13 r14 r15
..B26.4:                        # Preds ..B26.1
..LN2813:
   .loc    2  295
        movq      %r13, %rdi                                    #295.0
        movq      (%rdi), %rax                                  #295.0
        movq      8(%rdi), %rcx                                 #295.0
        bswap     %rax                                          #295.0
        bswap     %rcx                                          #295.0
        movq      %rax, 8(%rdi)                                 #295.0
        movq      %rcx, (%rdi)                                  #295.0
..LN2815:
   .loc    2  307
        movq      -16(%rbp), %r13                               #307.1
..___tag_value_uint128_bswap.885:                               #
        movq      %rbp, %rsp                                    #307.1
        popq      %rbp                                          #307.1
..___tag_value_uint128_bswap.886:                               #
        ret                                                     #307.1
        .align    16,0x90
..___tag_value_uint128_bswap.887:                               #
                                # LOE
# mark_end;
	.type	uint128_bswap,@function
	.size	uint128_bswap,.-uint128_bswap
.LNuint128_bswap:
	.data
# -- End  uint128_bswap
	.text
# -- Begin  NucStrFastaExprAlloc
# mark_begin;
       .align    16,0x90
NucStrFastaExprAlloc:
# parameter 1(sz): %rdi
..B27.1:                        # Preds ..B27.0
..___tag_value_NucStrFastaExprAlloc.888:                        #790.1
..LN2817:
   .loc    1  790
        pushq     %rbp                                          #790.1
        movq      %rsp, %rbp                                    #790.1
..___tag_value_NucStrFastaExprAlloc.889:                        #
        lea       _gprof_pack26(%rip), %rdx                     #790.1
        call      mcount@PLT                                    #790.1
                                # LOE rbx r12 r13 r14 r15
..B27.7:                        # Preds ..B27.1
..LN2819:
   .loc    1  791
        movl      $160, %edi                                    #791.19
        call      malloc@PLT                                    #791.19
                                # LOE rax rbx r12 r13 r14 r15
..B27.2:                        # Preds ..B27.7
..LN2821:
   .loc    1  792
        testq     %rax, %rax                                    #792.19
        je        ..B27.4       # Prob 12%                      #792.19
                                # LOE rax rbx r12 r13 r14 r15
..B27.3:                        # Preds ..B27.2
..LN2823:
   .loc    1  795
        lea       15(%rax), %rdx                                #795.36
..LN2825:
        andq      $-16, %rdx                                    #795.43
..LN2827:
   .loc    1  796
        movq      %rax, 8(%rdx)                                 #796.9
..LN2829:
   .loc    1  799
        movq      %rdx, %rax                                    #799.16
        movq      %rbp, %rsp                                    #799.16
        popq      %rbp                                          #799.16
..___tag_value_NucStrFastaExprAlloc.891:                        #
        ret                                                     #799.16
..___tag_value_NucStrFastaExprAlloc.892:                        #
                                # LOE
..B27.4:                        # Preds ..B27.2
..LN2831:
   .loc    1  801
        xorl      %eax, %eax                                    #801.12
        movq      %rbp, %rsp                                    #801.12
        popq      %rbp                                          #801.12
..___tag_value_NucStrFastaExprAlloc.893:                        #
        ret                                                     #801.12
        .align    16,0x90
..___tag_value_NucStrFastaExprAlloc.894:                        #
                                # LOE
# mark_end;
	.type	NucStrFastaExprAlloc,@function
	.size	NucStrFastaExprAlloc,.-NucStrFastaExprAlloc
.LNNucStrFastaExprAlloc:
	.data
# -- End  NucStrFastaExprAlloc
	.text
# -- Begin  NucStrFastaExprMake4
# mark_begin;
       .align    16,0x90
NucStrFastaExprMake4:
# parameter 1(expr): %rdi
# parameter 2(positional): %esi
# parameter 3(fasta): %rdx
# parameter 4(size): %rcx
..B28.1:                        # Preds ..B28.0
..___tag_value_NucStrFastaExprMake4.895:                        #1017.1
..LN2833:
   .loc    1  1017
        pushq     %rbp                                          #1017.1
        movq      %rsp, %rbp                                    #1017.1
..___tag_value_NucStrFastaExprMake4.896:                        #
        subq      $80, %rsp                                     #1017.1
        movq      %r15, -24(%rbp)                               #1017.1
..___tag_value_NucStrFastaExprMake4.898:                        #
        movq      %r14, -16(%rbp)                               #1017.1
..___tag_value_NucStrFastaExprMake4.899:                        #
        movq      %r13, -40(%rbp)                               #1017.1
..___tag_value_NucStrFastaExprMake4.900:                        #
        movq      %r12, -48(%rbp)                               #1017.1
..___tag_value_NucStrFastaExprMake4.901:                        #
        movq      %rbx, -32(%rbp)                               #1017.1
..___tag_value_NucStrFastaExprMake4.902:                        #
        movq      %rcx, %r13                                    #1017.1
        movq      %rdx, %r14                                    #1017.1
        movl      %esi, %r12d                                   #1017.1
        movq      %rdi, %r15                                    #1017.1
        lea       _gprof_pack27(%rip), %rdx                     #1017.1
        call      mcount@PLT                                    #1017.1
                                # LOE r13 r14 r15 r12d
..B28.58:                       # Preds ..B28.1
..LN2835:
   .loc    1  1023
        cmpq      $29, %r13                                     #1023.17
        jbe       ..B28.3       # Prob 43%                      #1023.17
                                # LOE r13 r14 r15 r12d
..B28.2:                        # Preds ..B28.58
..LN2837:
   .loc    1  1024
        movq      -32(%rbp), %rbx                               #1024.16
..___tag_value_NucStrFastaExprMake4.903:                        #
        movq      -48(%rbp), %r12                               #1024.16
..___tag_value_NucStrFastaExprMake4.904:                        #
        movq      -40(%rbp), %r13                               #1024.16
..___tag_value_NucStrFastaExprMake4.905:                        #
        movq      -16(%rbp), %r14                               #1024.16
..___tag_value_NucStrFastaExprMake4.906:                        #
        movl      $7, %eax                                      #1024.16
        movq      -24(%rbp), %r15                               #1024.16
..___tag_value_NucStrFastaExprMake4.907:                        #
        movq      %rbp, %rsp                                    #1024.16
        popq      %rbp                                          #1024.16
..___tag_value_NucStrFastaExprMake4.908:                        #
        ret                                                     #1024.16
..___tag_value_NucStrFastaExprMake4.909:                        #
                                # LOE
..B28.3:                        # Preds ..B28.58
..LN2839:
   .loc    1  1026
        movl      $144, %edi                                    #1026.9
        call      NucStrFastaExprAlloc@PLT                      #1026.9
                                # LOE rax r13 r14 r15 r12d
..B28.59:                       # Preds ..B28.3
        movq      %rax, %rbx                                    #1026.9
                                # LOE rbx r13 r14 r15 r12d
..B28.4:                        # Preds ..B28.59
..LN2841:
   .loc    1  1027
        testq     %rbx, %rbx                                    #1027.15
        je        ..B28.55      # Prob 5%                       #1027.15
                                # LOE rbx r13 r14 r15 r12d
..B28.5:                        # Preds ..B28.4
..LN2843:
   .loc    1  1030
        movq      %rbx, (%r15)                                  #1030.5
..LN2845:
   .loc    1  1031
        movl      %r13d, 4(%rbx)                                #1031.5
..LN2847:
   .loc    1  1035
        xorl      %edx, %edx                                    #1035.11
..LN2849:
        testq     %r13, %r13                                    #1035.22
        jbe       ..B28.16      # Prob 16%                      #1035.22
                                # LOE rdx rbx r13 r14 r12d
..B28.7:                        # Preds ..B28.5 ..B28.14
..LN2851:
   .loc    1  1037
        movsbq    (%rdx,%r14), %rcx                             #1037.49
..LN2853:
        lea       fasta_4na_map.0(%rip), %rax                   #1037.25
        movsbl    (%rax,%rcx), %esi                             #1037.25
..LN2855:
   .loc    1  1038
        movq      %rdx, %rax                                    #1038.22
        andq      $3, %rax                                      #1038.22
        je        ..B28.54      # Prob 20%                      #1038.22
                                # LOE rax rdx rbx r13 r14 esi r12d
..B28.8:                        # Preds ..B28.7
        cmpq      $1, %rax                                      #1038.22
        je        ..B28.13      # Prob 25%                      #1038.22
                                # LOE rax rdx rbx r13 r14 esi r12d
..B28.9:                        # Preds ..B28.8
        cmpq      $2, %rax                                      #1038.22
        jne       ..B28.11      # Prob 67%                      #1038.22
                                # LOE rax rdx rbx r13 r14 esi r12d
..B28.10:                       # Preds ..B28.9
..LN2857:
   .loc    1  1049
        movq      %rdx, %rcx                                    #1049.32
        shrq      $2, %rcx                                      #1049.32
..LN2859:
   .loc    1  1050
        movzwl    -64(%rbp,%rcx,2), %eax                        #1050.13
..LN2861:
   .loc    1  1049
        shll      $12, %esi                                     #1049.47
..LN2863:
   .loc    1  1050
        orl       $-4096, %eax                                  #1050.13
..LN2865:
   .loc    1  1049
        orw       %si, -80(%rbp,%rcx,2)                         #1049.13
..LN2867:
   .loc    1  1050
        movw      %ax, -64(%rbp,%rcx,2)                         #1050.13
        jmp       ..B28.14      # Prob 100%                     #1050.13
                                # LOE rdx rbx r13 r14 r12d
..B28.11:                       # Preds ..B28.9
..LN2869:
   .loc    1  1038
        cmpq      $3, %rax                                      #1038.22
        jne       ..B28.14      # Prob 50%                      #1038.22
                                # LOE rdx rbx r13 r14 esi r12d
..B28.12:                       # Preds ..B28.11
..LN2871:
   .loc    1  1053
        movq      %rdx, %rcx                                    #1053.32
        shrq      $2, %rcx                                      #1053.32
..LN2873:
   .loc    1  1054
        movzwl    -64(%rbp,%rcx,2), %eax                        #1054.13
..LN2875:
   .loc    1  1053
        shll      $8, %esi                                      #1053.47
..LN2877:
   .loc    1  1054
        orl       $-61696, %eax                                 #1054.13
..LN2879:
   .loc    1  1053
        orw       %si, -80(%rbp,%rcx,2)                         #1053.13
..LN2881:
   .loc    1  1054
        movw      %ax, -64(%rbp,%rcx,2)                         #1054.13
        jmp       ..B28.14      # Prob 100%                     #1054.13
                                # LOE rdx rbx r13 r14 r12d
..B28.13:                       # Preds ..B28.8
..LN2883:
   .loc    1  1045
        movq      %rdx, %rcx                                    #1045.32
        shrq      $2, %rcx                                      #1045.32
..LN2885:
   .loc    1  1046
        movzwl    -64(%rbp,%rcx,2), %eax                        #1046.13
        orl       $15, %eax                                     #1046.13
..LN2887:
   .loc    1  1045
        orw       %si, -80(%rbp,%rcx,2)                         #1045.13
..LN2889:
   .loc    1  1046
        movw      %ax, -64(%rbp,%rcx,2)                         #1046.13
                                # LOE rdx rbx r13 r14 r12d
..B28.14:                       # Preds ..B28.11 ..B28.54 ..B28.13 ..B28.10 ..B28.12
                                #      
..LN2891:
   .loc    1  1035
        incq      %rdx                                          #1035.31
..LN2893:
        cmpq      %r13, %rdx                                    #1035.22
        jb        ..B28.7       # Prob 82%                      #1035.22
                                # LOE rdx rbx r13 r14 r12d
..B28.16:                       # Preds ..B28.14 ..B28.5
..LN2895:
   .loc    1  1060
        addq      $3, %rdx                                      #1060.21
..LN2897:
        shrq      $2, %rdx                                      #1060.28
..LN2899:
        cmpq      $8, %rdx                                      #1060.35
        jae       ..B28.29      # Prob 50%                      #1060.35
                                # LOE rdx rbx r13 r12d
..B28.17:                       # Preds ..B28.16
..LN2901:
        lea       -80(%rbp,%rdx,2), %rsi                        #1060.5
        movq      %rdx, %rdi                                    #1060.5
        negq      %rdi                                          #1060.5
        addq      $8, %rdi                                      #1060.5
        movq      %rsi, %rax                                    #1060.5
        andq      $15, %rax                                     #1060.5
        movq      %rax, %rcx                                    #1060.5
        negq      %rcx                                          #1060.5
        addq      $16, %rcx                                     #1060.5
        shrq      $1, %rcx                                      #1060.5
        cmpq      $0, %rax                                      #1060.5
        cmovne    %rcx, %rax                                    #1060.5
        lea       8(%rax), %r8                                  #1060.5
        cmpq      %r8, %rdi                                     #1060.5
        jl        ..B28.53      # Prob 10%                      #1060.5
                                # LOE rax rdx rbx rsi rdi r13 r12d
..B28.18:                       # Preds ..B28.17
        movq      %rdi, %rcx                                    #1060.5
        subq      %rax, %rcx                                    #1060.5
        andq      $7, %rcx                                      #1060.5
        negq      %rcx                                          #1060.5
        addq      %rdi, %rcx                                    #1060.5
        xorl      %r9d, %r9d                                    #1060.5
        testq     %rax, %rax                                    #1060.5
        jbe       ..B28.22      # Prob 10%                      #1060.5
                                # LOE rax rdx rcx rbx rsi rdi r9 r13 r12d
..B28.19:                       # Preds ..B28.18
        lea       -64(%rbp,%rdx,2), %r8                         #
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r13 r12d
..B28.20:                       # Preds ..B28.20 ..B28.19
..LN2903:
   .loc    1  1062
        xorl      %r10d, %r10d                                  #1062.9
        movw      %r10w, (%rsi,%r9,2)                           #1062.9
..LN2905:
   .loc    1  1063
        movw      %r10w, (%r8,%r9,2)                            #1063.9
..LN2907:
   .loc    1  1060
        incq      %r9                                           #1060.5
        cmpq      %rax, %r9                                     #1060.5
        jb        ..B28.20      # Prob 82%                      #1060.5
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r13 r12d
..B28.22:                       # Preds ..B28.20 ..B28.18
        lea       -64(%rbp,%rdx,2), %r9                         #
        lea       -80(%rbp,%rdx,2), %r8                         #
..LN2909:
   .loc    1  1062
        pxor      %xmm0, %xmm0                                  #1062.29
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r13 r12d xmm0
..B28.23:                       # Preds ..B28.23 ..B28.22
..LN2911:
        movdqa    %xmm0, (%r8,%rax,2)                           #1062.9
..LN2913:
   .loc    1  1063
        movdqa    %xmm0, (%r9,%rax,2)                           #1063.9
..LN2915:
   .loc    1  1060
        addq      $8, %rax                                      #1060.5
        cmpq      %rcx, %rax                                    #1060.5
        jb        ..B28.23      # Prob 82%                      #1060.5
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r13 r12d xmm0
..B28.25:                       # Preds ..B28.23 ..B28.53
        cmpq      %rdi, %rcx                                    #1060.5
        jae       ..B28.29      # Prob 10%                      #1060.5
                                # LOE rdx rcx rbx rsi rdi r13 r12d
..B28.26:                       # Preds ..B28.25
        lea       -64(%rbp,%rdx,2), %rax                        #
                                # LOE rax rcx rbx rsi rdi r13 r12d
..B28.27:                       # Preds ..B28.27 ..B28.26
..LN2917:
   .loc    1  1062
        xorl      %edx, %edx                                    #1062.9
        movw      %dx, (%rsi,%rcx,2)                            #1062.9
..LN2919:
   .loc    1  1063
        movw      %dx, (%rax,%rcx,2)                            #1063.9
..LN2921:
   .loc    1  1060
        incq      %rcx                                          #1060.5
        cmpq      %rdi, %rcx                                    #1060.5
        jb        ..B28.27      # Prob 82%                      #1060.5
                                # LOE rax rcx rbx rsi rdi r13 r12d
..B28.29:                       # Preds ..B28.27 ..B28.25 ..B28.16
..LN2923:
   .loc    1  1068
        testl     %r12d, %r12d                                  #1068.10
        je        ..B28.31      # Prob 50%                      #1068.10
                                # LOE rbx r13
..B28.30:                       # Preds ..B28.29
..LN2925:
   .loc    1  1070
        movl      $10, (%rbx)                                   #1070.9
        jmp       ..B28.38      # Prob 100%                     #1070.9
                                # LOE rbx
..B28.31:                       # Preds ..B28.29
..LN2927:
   .loc    1  1072
        cmpq      $2, %r13                                      #1072.22
        jae       ..B28.33      # Prob 50%                      #1072.22
                                # LOE rbx r13
..B28.32:                       # Preds ..B28.31
..LN2929:
   .loc    1  1074
        movzwl    -80(%rbp), %eax                               #1074.29
..LN2931:
   .loc    1  1078
        movzwl    -64(%rbp), %esi                               #1078.26
..LN2933:
   .loc    1  1082
        movl      $6, (%rbx)                                    #1082.9
..LN2935:
   .loc    1  1074
        movw      %ax, -78(%rbp)                                #1074.9
..LN2937:
   .loc    1  1075
        movl      -80(%rbp), %edx                               #1075.29
..LN2939:
   .loc    1  1076
        movl      -80(%rbp), %ecx                               #1076.29
..LN2941:
   .loc    1  1075
        movl      %edx, -76(%rbp)                               #1075.9
..LN2943:
   .loc    1  1078
        movw      %si, -62(%rbp)                                #1078.9
..LN2945:
   .loc    1  1079
        movl      -64(%rbp), %edi                               #1079.26
..LN2947:
   .loc    1  1080
        movl      -64(%rbp), %r8d                               #1080.26
..LN2949:
   .loc    1  1079
        movl      %edi, -60(%rbp)                               #1079.9
..LN2951:
   .loc    1  1075
        movl      %edx, %edx                                    #1075.9
..LN2953:
   .loc    1  1076
        shlq      $32, %rdx                                     #1076.29
..LN2955:
   .loc    1  1079
        movl      %edi, %edi                                    #1079.9
..LN2957:
   .loc    1  1080
        shlq      $32, %rdi                                     #1080.26
..LN2959:
   .loc    1  1076
        orq       %rcx, %rdx                                    #1076.29
..LN2961:
        movq      %rdx, -72(%rbp)                               #1076.9
..LN2963:
   .loc    1  1080
        orq       %r8, %rdi                                     #1080.26
..LN2965:
        movq      %rdi, -56(%rbp)                               #1080.9
        jmp       ..B28.38      # Prob 100%                     #1080.9
                                # LOE rbx
..B28.33:                       # Preds ..B28.31
..LN2967:
   .loc    1  1084
        cmpq      $6, %r13                                      #1084.22
        jae       ..B28.35      # Prob 50%                      #1084.22
                                # LOE rbx r13
..B28.34:                       # Preds ..B28.33
..LN2969:
   .loc    1  1086
        movl      -80(%rbp), %eax                               #1086.29
..LN2971:
   .loc    1  1087
        movl      -80(%rbp), %edx                               #1087.29
..LN2973:
   .loc    1  1089
        movl      -64(%rbp), %ecx                               #1089.26
..LN2975:
   .loc    1  1090
        movl      -64(%rbp), %esi                               #1090.26
..LN2977:
   .loc    1  1086
        movl      %eax, -76(%rbp)                               #1086.9
..LN2979:
   .loc    1  1092
        movl      $7, (%rbx)                                    #1092.9
..LN2981:
   .loc    1  1086
        movl      %eax, %eax                                    #1086.9
..LN2983:
   .loc    1  1087
        shlq      $32, %rax                                     #1087.29
..LN2985:
   .loc    1  1089
        movl      %ecx, -60(%rbp)                               #1089.9
        movl      %ecx, %ecx                                    #1089.9
..LN2987:
   .loc    1  1090
        shlq      $32, %rcx                                     #1090.26
..LN2989:
   .loc    1  1087
        orq       %rdx, %rax                                    #1087.29
..LN2991:
        movq      %rax, -72(%rbp)                               #1087.9
..LN2993:
   .loc    1  1090
        orq       %rsi, %rcx                                    #1090.26
..LN2995:
        movq      %rcx, -56(%rbp)                               #1090.9
        jmp       ..B28.38      # Prob 100%                     #1090.9
                                # LOE rbx
..B28.35:                       # Preds ..B28.33
..LN2997:
   .loc    1  1094
        cmpq      $14, %r13                                     #1094.22
        jae       ..B28.37      # Prob 50%                      #1094.22
                                # LOE rbx
..B28.36:                       # Preds ..B28.35
..LN2999:
   .loc    1  1096
        movq      -80(%rbp), %rax                               #1096.29
..LN3001:
   .loc    1  1097
        movq      -64(%rbp), %rdx                               #1097.26
..LN3003:
   .loc    1  1099
        movl      $1, (%rbx)                                    #1099.9
..LN3005:
   .loc    1  1096
        movq      %rax, -72(%rbp)                               #1096.9
..LN3007:
   .loc    1  1097
        movq      %rdx, -56(%rbp)                               #1097.9
        jmp       ..B28.38      # Prob 100%                     #1097.9
                                # LOE rbx
..B28.37:                       # Preds ..B28.35
..LN3009:
   .loc    1  1103
        movl      $8, (%rbx)                                    #1103.9
                                # LOE rbx
..B28.38:                       # Preds ..B28.30 ..B28.32 ..B28.34 ..B28.36 ..B28.37
                                #      
..LN3011:
   .loc    1  1106
        movq      -80(%rbp), %rax                               #1106.42
..LN3013:
   .loc    1  1110
        lea       -80(%rbp), %rdi                               #1110.5
..LN3015:
   .loc    1  1106
        movq      %rax, 16(%rbx)                                #1106.42
        movq      -72(%rbp), %rdx                               #1106.42
        movq      %rdx, 24(%rbx)                                #1106.42
..LN3017:
   .loc    1  1107
        movq      -64(%rbp), %rcx                               #1107.39
        movq      %rcx, 32(%rbx)                                #1107.39
        movq      -56(%rbp), %rsi                               #1107.39
        movq      %rsi, 40(%rbx)                                #1107.39
..LN3019:
   .loc    1  1110
        call      uint128_bswap@PLT                             #1110.5
                                # LOE rbx
..B28.39:                       # Preds ..B28.38
..LN3021:
   .loc    1  1111
        lea       -64(%rbp), %rdi                               #1111.5
        call      uint128_bswap@PLT                             #1111.5
                                # LOE rbx
..B28.40:                       # Preds ..B28.39
..LN3023:
   .loc    1  1114
        lea       -80(%rbp), %rdi                               #1114.5
        movl      $4, %esi                                      #1114.5
        call      uint128_shr@PLT                               #1114.5
                                # LOE rbx
..B28.41:                       # Preds ..B28.40
..LN3025:
   .loc    1  1115
        lea       -64(%rbp), %rdi                               #1115.5
        movl      $4, %esi                                      #1115.5
        call      uint128_shr@PLT                               #1115.5
                                # LOE rbx
..B28.42:                       # Preds ..B28.41
..LN3027:
   .loc    1  1118
        lea       -80(%rbp), %rsi                               #1118.5
        lea       48(%rbx), %rdi                                #1118.5
        call      uint128_bswap_copy@PLT                        #1118.5
                                # LOE rbx
..B28.43:                       # Preds ..B28.42
..LN3029:
   .loc    1  1119
        lea       -64(%rbp), %rsi                               #1119.5
        lea       64(%rbx), %rdi                                #1119.5
        call      uint128_bswap_copy@PLT                        #1119.5
                                # LOE rbx
..B28.44:                       # Preds ..B28.43
..LN3031:
   .loc    1  1121
        lea       -80(%rbp), %rdi                               #1121.5
        movl      $4, %esi                                      #1121.5
        call      uint128_shr@PLT                               #1121.5
                                # LOE rbx
..B28.45:                       # Preds ..B28.44
..LN3033:
   .loc    1  1122
        lea       -64(%rbp), %rdi                               #1122.5
        movl      $4, %esi                                      #1122.5
        call      uint128_shr@PLT                               #1122.5
                                # LOE rbx
..B28.46:                       # Preds ..B28.45
..LN3035:
   .loc    1  1124
        lea       -80(%rbp), %rsi                               #1124.5
        lea       80(%rbx), %rdi                                #1124.5
        call      uint128_bswap_copy@PLT                        #1124.5
                                # LOE rbx
..B28.47:                       # Preds ..B28.46
..LN3037:
   .loc    1  1125
        lea       -64(%rbp), %rsi                               #1125.5
        lea       96(%rbx), %rdi                                #1125.5
        call      uint128_bswap_copy@PLT                        #1125.5
                                # LOE rbx
..B28.48:                       # Preds ..B28.47
..LN3039:
   .loc    1  1127
        lea       -80(%rbp), %rdi                               #1127.5
        movl      $4, %esi                                      #1127.5
        call      uint128_shr@PLT                               #1127.5
                                # LOE rbx
..B28.49:                       # Preds ..B28.48
..LN3041:
   .loc    1  1128
        lea       -64(%rbp), %rdi                               #1128.5
        movl      $4, %esi                                      #1128.5
        call      uint128_shr@PLT                               #1128.5
                                # LOE rbx
..B28.50:                       # Preds ..B28.49
..LN3043:
   .loc    1  1130
        lea       -80(%rbp), %rsi                               #1130.5
        lea       112(%rbx), %rdi                               #1130.5
        call      uint128_bswap_copy@PLT                        #1130.5
                                # LOE rbx
..B28.51:                       # Preds ..B28.50
..LN3045:
   .loc    1  1131
        lea       -64(%rbp), %rsi                               #1131.5
        addq      $128, %rbx                                    #1131.5
        movq      %rbx, %rdi                                    #1131.5
        call      uint128_bswap_copy@PLT                        #1131.5
                                # LOE
..B28.52:                       # Preds ..B28.51
..LN3047:
   .loc    1  1173
        movq      -32(%rbp), %rbx                               #1173.12
..___tag_value_NucStrFastaExprMake4.915:                        #
        movq      -48(%rbp), %r12                               #1173.12
..___tag_value_NucStrFastaExprMake4.916:                        #
        movq      -40(%rbp), %r13                               #1173.12
..___tag_value_NucStrFastaExprMake4.917:                        #
        movq      -16(%rbp), %r14                               #1173.12
..___tag_value_NucStrFastaExprMake4.918:                        #
        xorl      %eax, %eax                                    #1173.12
        movq      -24(%rbp), %r15                               #1173.12
..___tag_value_NucStrFastaExprMake4.919:                        #
        movq      %rbp, %rsp                                    #1173.12
        popq      %rbp                                          #1173.12
..___tag_value_NucStrFastaExprMake4.920:                        #
        ret                                                     #1173.12
..___tag_value_NucStrFastaExprMake4.921:                        #
                                # LOE
..B28.53:                       # Preds ..B28.17                # Infreq
..LN3049:
   .loc    1  1060
        xorl      %ecx, %ecx                                    #1060.5
        jmp       ..B28.25      # Prob 100%                     #1060.5
                                # LOE rdx rcx rbx rsi rdi r13 r12d
..B28.54:                       # Preds ..B28.7                 # Infreq
..LN3051:
   .loc    1  1041
        movq      %rdx, %rax                                    #1041.32
        shrq      $2, %rax                                      #1041.32
..LN3053:
        shll      $4, %esi                                      #1041.46
..LN3055:
        movw      %si, -80(%rbp,%rax,2)                         #1041.13
..LN3057:
   .loc    1  1042
        movl      $240, %esi                                    #1042.13
        movw      %si, -64(%rbp,%rax,2)                         #1042.13
        jmp       ..B28.14      # Prob 100%                     #1042.13
                                # LOE rdx rbx r13 r14 r12d
..B28.55:                       # Preds ..B28.4                 # Infreq
..LN3059:
   .loc    1  1028
        call      __errno_location@PLT                          #1028.16
                                # LOE rax
..B28.60:                       # Preds ..B28.55                # Infreq
        movq      -32(%rbp), %rbx                               #1028.16
..___tag_value_NucStrFastaExprMake4.927:                        #
        movq      -48(%rbp), %r12                               #1028.16
..___tag_value_NucStrFastaExprMake4.928:                        #
        movq      -40(%rbp), %r13                               #1028.16
..___tag_value_NucStrFastaExprMake4.929:                        #
        movq      -16(%rbp), %r14                               #1028.16
..___tag_value_NucStrFastaExprMake4.930:                        #
        movl      (%rax), %eax                                  #1028.16
        movq      -24(%rbp), %r15                               #1028.16
..___tag_value_NucStrFastaExprMake4.931:                        #
        movq      %rbp, %rsp                                    #1028.16
        popq      %rbp                                          #1028.16
..___tag_value_NucStrFastaExprMake4.932:                        #
        ret                                                     #1028.16
        .align    16,0x90
..___tag_value_NucStrFastaExprMake4.933:                        #
                                # LOE
# mark_end;
	.type	NucStrFastaExprMake4,@function
	.size	NucStrFastaExprMake4,.-NucStrFastaExprMake4
.LNNucStrFastaExprMake4:
	.data
# -- End  NucStrFastaExprMake4
	.text
# -- Begin  NucStrstrWhack
# mark_begin;
       .align    16,0x90
	.globl NucStrstrWhack
NucStrstrWhack:
# parameter 1(self): %rdi
..B29.1:                        # Preds ..B29.0
..___tag_value_NucStrstrWhack.934:                              #1574.1
..LN3061:
   .loc    1  1574
        pushq     %rbp                                          #1574.1
        movq      %rsp, %rbp                                    #1574.1
..___tag_value_NucStrstrWhack.935:                              #
        subq      $16, %rsp                                     #1574.1
        movq      %r13, -16(%rbp)                               #1574.1
..___tag_value_NucStrstrWhack.937:                              #
        movq      %rdi, %r13                                    #1574.1
        lea       _gprof_pack28(%rip), %rdx                     #1574.1
        call      mcount@PLT                                    #1574.1
                                # LOE rbx r12 r13 r14 r15
..B29.25:                       # Preds ..B29.1
..LN3063:
   .loc    1  1575
        testq     %r13, %r13                                    #1575.18
        je        ..B29.22      # Prob 12%                      #1575.18
                                # LOE rbx r12 r13 r14 r15
..B29.2:                        # Preds ..B29.25
..LN3065:
   .loc    1  1577
        movl      (%r13), %eax                                  #1577.18
..LN3067:
        cmpl      $12, %eax                                     #1577.9
        ja        ..B29.21      # Prob 50%                      #1577.9
                                # LOE rbx r12 r13 r14 r15 eax
..B29.3:                        # Preds ..B29.2
        movl      %eax, %eax                                    #1577.9
        lea       ..1..TPKT.18_0.0.13(%rip), %rdx               #1577.9
        movq      (%rdx,%rax,8), %rcx                           #1577.9
        jmp       *%rcx                                         #1577.9
                                # LOE rbx r12 r13 r14 r15
..1.18_0.TAG.0c.0.13:
..B29.5:                        # Preds ..B29.3
..LN3069:
   .loc    1  1601
        movq      8(%r13), %rdi                                 #1601.13
        call      NucStrstrWhack@PLT                            #1601.13
        jmp       ..B29.21      # Prob 100%                     #1601.13
                                # LOE rbx r12 r13 r14 r15
..1.18_0.TAG.0b.0.13:
..B29.7:                        # Preds ..B29.3
..LN3071:
   .loc    1  1597
        movq      8(%r13), %rdi                                 #1597.13
        call      NucStrstrWhack@PLT                            #1597.13
                                # LOE rbx r12 r13 r14 r15
..B29.8:                        # Preds ..B29.7
..LN3073:
   .loc    1  1598
        movq      16(%r13), %rdi                                #1598.13
        call      NucStrstrWhack@PLT                            #1598.13
        jmp       ..B29.21      # Prob 100%                     #1598.13
                                # LOE rbx r12 r13 r14 r15
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
..B29.20:                       # Preds ..B29.3 ..B29.3 ..B29.3 ..B29.3 ..B29.3
                                #       ..B29.3 ..B29.3 ..B29.3 ..B29.3 ..B29.3
                                #       ..B29.3
..LN3075:
   .loc    1  1593
        movq      8(%r13), %r13                                 #1593.20
                                # LOE rbx r12 r13 r14 r15
..B29.21:                       # Preds ..B29.5 ..B29.8 ..B29.20 ..B29.2
..LN3077:
   .loc    1  1604
        movq      %r13, %rdi                                    #1604.9
        xorl      %eax, %eax                                    #1604.9
        call      free@PLT                                      #1604.9
                                # LOE rbx r12 r14 r15
..B29.22:                       # Preds ..B29.21 ..B29.25
..LN3079:
   .loc    1  1606
        movq      -16(%rbp), %r13                               #1606.1
..___tag_value_NucStrstrWhack.938:                              #
        movq      %rbp, %rsp                                    #1606.1
        popq      %rbp                                          #1606.1
..___tag_value_NucStrstrWhack.939:                              #
        ret                                                     #1606.1
        .align    16,0x90
..___tag_value_NucStrstrWhack.940:                              #
                                # LOE
# mark_end;
	.type	NucStrstrWhack,@function
	.size	NucStrstrWhack,.-NucStrstrWhack
.LNNucStrstrWhack:
	.section .data1, "wa"
	.space 24	# pad
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
# -- Begin  NucStrstrInit
# mark_begin;
       .align    16,0x90
NucStrstrInit:
..B30.1:                        # Preds ..B30.0
..___tag_value_NucStrstrInit.941:                               #1484.1
..LN3081:
   .loc    1  1484
        pushq     %rbp                                          #1484.1
        movq      %rsp, %rbp                                    #1484.1
..___tag_value_NucStrstrInit.942:                               #
        subq      $32, %rsp                                     #1484.1
        movq      %r15, -32(%rbp)                               #1484.1
..___tag_value_NucStrstrInit.944:                               #
        movq      %r12, -16(%rbp)                               #1484.1
..___tag_value_NucStrstrInit.945:                               #
        movq      %rbx, -24(%rbp)                               #1484.1
..___tag_value_NucStrstrInit.946:                               #
        lea       _gprof_pack29(%rip), %rdx                     #1484.1
        call      mcount@PLT                                    #1484.1
                                # LOE r13 r14
..B30.27:                       # Preds ..B30.1
..LN3083:
   .loc    1  1491
        lea       fasta_2na_map.0(%rip), %rdx                   #1491.14
..LN3085:
        movl      $-1, %eax                                     #1491.5
        movd      %eax, %xmm0                                   #1491.5
        pshufd    $0, %xmm0, %xmm1                              #1491.5
        movaps    %xmm1, (%rdx)                                 #1491.5
        movaps    %xmm1, 16(%rdx)                               #1491.5
        movaps    %xmm1, 32(%rdx)                               #1491.5
        movaps    %xmm1, 48(%rdx)                               #1491.5
        movaps    %xmm1, 64(%rdx)                               #1491.5
        movaps    %xmm1, 80(%rdx)                               #1491.5
        movaps    %xmm1, 96(%rdx)                               #1491.5
        movaps    %xmm1, 112(%rdx)                              #1491.5
                                # LOE r13 r14
..B30.2:                        # Preds ..B30.27
..LN3087:
   .loc    1  1492
        lea       fasta_4na_map.0(%rip), %rdx                   #1492.14
..LN3089:
        movl      $-1, %eax                                     #1492.5
        movd      %eax, %xmm0                                   #1492.5
        pshufd    $0, %xmm0, %xmm1                              #1492.5
        movaps    %xmm1, (%rdx)                                 #1492.5
        movaps    %xmm1, 16(%rdx)                               #1492.5
        movaps    %xmm1, 32(%rdx)                               #1492.5
        movaps    %xmm1, 48(%rdx)                               #1492.5
        movaps    %xmm1, 64(%rdx)                               #1492.5
        movaps    %xmm1, 80(%rdx)                               #1492.5
        movaps    %xmm1, 96(%rdx)                               #1492.5
        movaps    %xmm1, 112(%rdx)                              #1492.5
                                # LOE r13 r14
..B30.3:                        # Preds ..B30.2
..LN3091:
   .loc    1  1495
        xorl      %r15d, %r15d                                  #1495.11
..LN3093:
   .loc    1  1487
        lea       _2__STRING.0.0(%rip), %rax                    #1487.29
..LN3095:
   .loc    1  1497
        movsbl    (%rax), %ebx                                  #1497.14
        lea       1+_2__STRING.0.0(%rip), %r12                  #
..LN3097:
   .loc    1  1498
        call      __ctype_tolower_loc@PLT                       #1498.48
                                # LOE r12 r13 r14 ebx r15d
..B30.9:                        # Preds ..B30.3 ..B30.10
        movl      %ebx, %edi                                    #1498.48
        call      tolower@PLT                                   #1498.48
                                # LOE r12 r13 r14 eax ebx r15d
..B30.28:                       # Preds ..B30.9
        movslq    %eax, %rdx                                    #1498.48
                                # LOE rdx r12 r13 r14 ebx r15d
..B30.10:                       # Preds ..B30.28
..LN3099:
        lea       fasta_2na_map.0(%rip), %rax                   #1498.32
        movb      %r15b, (%rax,%rdx)                            #1498.32
..LN3101:
        movslq    %ebx, %rdx                                    #1498.9
        movb      %r15b, (%rax,%rdx)                            #1498.9
..LN3103:
   .loc    1  1495
        movsbl    (%r12), %ebx                                  #1495.31
..LN3105:
        incl      %r15d                                         #1495.48
..LN3107:
        incq      %r12                                          #1495.54
..LN3109:
        testl     %ebx, %ebx                                    #1495.42
        jne       ..B30.9       # Prob 82%                      #1495.42
                                # LOE r12 r13 r14 ebx r15d
..B30.11:                       # Preds ..B30.10
..LN3111:
   .loc    1  1502
        xorl      %r15d, %r15d                                  #1502.11
..LN3113:
   .loc    1  1488
        lea       _2__STRING.1.0(%rip), %rax                    #1488.25
..LN3115:
   .loc    1  1504
        movsbl    (%rax), %ebx                                  #1504.14
        lea       1+_2__STRING.1.0(%rip), %r12                  #
                                # LOE r12 r13 r14 ebx r15d
..B30.17:                       # Preds ..B30.11 ..B30.18
..LN3117:
   .loc    1  1505
        movl      %ebx, %edi                                    #1505.48
        call      tolower@PLT                                   #1505.48
                                # LOE r12 r13 r14 eax ebx r15d
..B30.29:                       # Preds ..B30.17
        movslq    %eax, %rdx                                    #1505.48
                                # LOE rdx r12 r13 r14 ebx r15d
..B30.18:                       # Preds ..B30.29
..LN3119:
        lea       fasta_4na_map.0(%rip), %rax                   #1505.32
        movb      %r15b, (%rax,%rdx)                            #1505.32
..LN3121:
        movslq    %ebx, %rdx                                    #1505.9
        movb      %r15b, (%rax,%rdx)                            #1505.9
..LN3123:
   .loc    1  1502
        movsbl    (%r12), %ebx                                  #1502.31
..LN3125:
        incl      %r15d                                         #1502.48
..LN3127:
        incq      %r12                                          #1502.54
..LN3129:
        testl     %ebx, %ebx                                    #1502.42
        jne       ..B30.17      # Prob 82%                      #1502.42
                                # LOE r12 r13 r14 ebx r15d
..B30.19:                       # Preds ..B30.18
..LN3131:
   .loc    1  1510
        xorl      %edx, %edx                                    #1510.11
..LN3133:
        xorl      %eax, %eax                                    #1510.30
                                # LOE rdx r13 r14 eax
..B30.20:                       # Preds ..B30.23 ..B30.19
..LN3135:
   .loc    1  1511
        lea       expand_2na.0(%rip), %rcx                      #1511.28
        movzwl    (%rcx,%rdx,2), %ecx                           #1511.28
                                # LOE rdx r13 r14 eax ecx
..B30.22:                       # Preds ..B30.20
..LN3137:
        rorw      $8, %cx                                       #1511.0
                                # LOE rdx r13 r14 eax ecx
..B30.23:                       # Preds ..B30.22
..LN3139:
        lea       expand_2na.0(%rip), %rbx                      #1511.9
        movw      %cx, (%rbx,%rdx,2)                            #1511.9
..LN3141:
   .loc    1  1510
        incl      %eax                                          #1510.30
        movl      %eax, %edx                                    #1510.30
..LN3143:
        movl      %edx, %eax                                    #1510.5
..LN3145:
        cmpl      $256, %eax                                    #1510.22
        jb        ..B30.20      # Prob 99%                      #1510.22
                                # LOE rdx r13 r14 eax
..B30.24:                       # Preds ..B30.23
..LN3147:
   .loc    1  1513
        movq      -24(%rbp), %rbx                               #1513.1
..___tag_value_NucStrstrInit.947:                               #
        movq      -16(%rbp), %r12                               #1513.1
..___tag_value_NucStrstrInit.948:                               #
        movq      -32(%rbp), %r15                               #1513.1
..___tag_value_NucStrstrInit.949:                               #
        movq      %rbp, %rsp                                    #1513.1
        popq      %rbp                                          #1513.1
..___tag_value_NucStrstrInit.950:                               #
        ret                                                     #1513.1
        .align    16,0x90
..___tag_value_NucStrstrInit.951:                               #
                                # LOE
# mark_end;
	.type	NucStrstrInit,@function
	.size	NucStrstrInit,.-NucStrstrInit
.LNNucStrstrInit:
	.data
# -- End  NucStrstrInit
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
_gprof_pack0:
	.long	0
	.type	_gprof_pack0,@object
	.size	_gprof_pack0,4
	.align 4
_gprof_pack1:
	.long	0
	.type	_gprof_pack1,@object
	.size	_gprof_pack1,4
	.align 4
_gprof_pack2:
	.long	0
	.type	_gprof_pack2,@object
	.size	_gprof_pack2,4
	.align 4
_gprof_pack3:
	.long	0
	.type	_gprof_pack3,@object
	.size	_gprof_pack3,4
	.align 4
_gprof_pack4:
	.long	0
	.type	_gprof_pack4,@object
	.size	_gprof_pack4,4
	.align 4
_gprof_pack5:
	.long	0
	.type	_gprof_pack5,@object
	.size	_gprof_pack5,4
	.align 4
_gprof_pack6:
	.long	0
	.type	_gprof_pack6,@object
	.size	_gprof_pack6,4
	.align 4
_gprof_pack7:
	.long	0
	.type	_gprof_pack7,@object
	.size	_gprof_pack7,4
	.align 4
_gprof_pack8:
	.long	0
	.type	_gprof_pack8,@object
	.size	_gprof_pack8,4
	.align 4
_gprof_pack9:
	.long	0
	.type	_gprof_pack9,@object
	.size	_gprof_pack9,4
	.align 4
_gprof_pack10:
	.long	0
	.type	_gprof_pack10,@object
	.size	_gprof_pack10,4
	.align 4
_gprof_pack11:
	.long	0
	.type	_gprof_pack11,@object
	.size	_gprof_pack11,4
	.align 4
_gprof_pack12:
	.long	0
	.type	_gprof_pack12,@object
	.size	_gprof_pack12,4
	.align 4
_gprof_pack13:
	.long	0
	.type	_gprof_pack13,@object
	.size	_gprof_pack13,4
	.align 4
_gprof_pack14:
	.long	0
	.type	_gprof_pack14,@object
	.size	_gprof_pack14,4
	.align 4
_gprof_pack15:
	.long	0
	.type	_gprof_pack15,@object
	.size	_gprof_pack15,4
	.align 4
_gprof_pack16:
	.long	0
	.type	_gprof_pack16,@object
	.size	_gprof_pack16,4
	.align 4
_gprof_pack17:
	.long	0
	.type	_gprof_pack17,@object
	.size	_gprof_pack17,4
	.align 4
_gprof_pack18:
	.long	0
	.type	_gprof_pack18,@object
	.size	_gprof_pack18,4
	.align 4
_gprof_pack19:
	.long	0
	.type	_gprof_pack19,@object
	.size	_gprof_pack19,4
	.align 4
_gprof_pack20:
	.long	0
	.type	_gprof_pack20,@object
	.size	_gprof_pack20,4
	.align 4
_gprof_pack21:
	.long	0
	.type	_gprof_pack21,@object
	.size	_gprof_pack21,4
	.align 4
_gprof_pack22:
	.long	0
	.type	_gprof_pack22,@object
	.size	_gprof_pack22,4
	.align 4
_gprof_pack23:
	.long	0
	.type	_gprof_pack23,@object
	.size	_gprof_pack23,4
	.align 4
_gprof_pack24:
	.long	0
	.type	_gprof_pack24,@object
	.size	_gprof_pack24,4
	.align 4
_gprof_pack25:
	.long	0
	.type	_gprof_pack25,@object
	.size	_gprof_pack25,4
	.align 4
_gprof_pack26:
	.long	0
	.type	_gprof_pack26,@object
	.size	_gprof_pack26,4
	.align 4
_gprof_pack27:
	.long	0
	.type	_gprof_pack27,@object
	.size	_gprof_pack27,4
	.align 4
_gprof_pack28:
	.long	0
	.type	_gprof_pack28,@object
	.size	_gprof_pack28,4
	.align 4
_gprof_pack29:
	.long	0
	.type	_gprof_pack29,@object
	.size	_gprof_pack29,4
	.section .rodata.str1.4, "aMS",@progbits,1
	.align 4
	.align 4
_2__STRING.0.0:
	.byte	65
	.byte	67
	.byte	71
	.byte	84
	.byte	0
	.type	_2__STRING.0.0,@object
	.size	_2__STRING.0.0,5
	.space 3	# pad
	.align 4
_2__STRING.1.0:
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
	.type	_2__STRING.1.0,@object
	.size	_2__STRING.1.0,17
	.data
	.section .note.GNU-stack, ""
// -- Begin DWARF2 SEGMENT .debug_info
	.section .debug_info
.debug_info_seg:
	.align 1
// -- Begin DWARF2 SEGMENT .debug_line
	.section .debug_line
.debug_line_seg:
	.align 1
// -- Begin DWARF2 SEGMENT .debug_abbrev
	.section .debug_abbrev
.debug_abbrev_seg:
	.align 1
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
	.4byte 0x0000037c
	.4byte 0x0000001c
	.4byte ..___tag_value_NucStrstrSearch.1-.
	.4byte ..___tag_value_NucStrstrSearch.159-..___tag_value_NucStrstrSearch.1
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.2-..___tag_value_NucStrstrSearch.1
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.4-..___tag_value_NucStrstrSearch.2
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.5-..___tag_value_NucStrstrSearch.4
	.byte 0x8e
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.6-..___tag_value_NucStrstrSearch.5
	.byte 0x8d
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.7-..___tag_value_NucStrstrSearch.6
	.byte 0x8c
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.8-..___tag_value_NucStrstrSearch.7
	.byte 0x83
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.9-..___tag_value_NucStrstrSearch.8
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.10-..___tag_value_NucStrstrSearch.9
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.11-..___tag_value_NucStrstrSearch.10
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.12-..___tag_value_NucStrstrSearch.11
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.13-..___tag_value_NucStrstrSearch.12
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.14-..___tag_value_NucStrstrSearch.13
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.15-..___tag_value_NucStrstrSearch.14
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x08
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.21-..___tag_value_NucStrstrSearch.15
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.22-..___tag_value_NucStrstrSearch.21
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.23-..___tag_value_NucStrstrSearch.22
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.24-..___tag_value_NucStrstrSearch.23
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.25-..___tag_value_NucStrstrSearch.24
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.26-..___tag_value_NucStrstrSearch.25
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.27-..___tag_value_NucStrstrSearch.26
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x08
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.33-..___tag_value_NucStrstrSearch.27
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.34-..___tag_value_NucStrstrSearch.33
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.35-..___tag_value_NucStrstrSearch.34
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.36-..___tag_value_NucStrstrSearch.35
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.37-..___tag_value_NucStrstrSearch.36
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.38-..___tag_value_NucStrstrSearch.37
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.39-..___tag_value_NucStrstrSearch.38
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x08
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.45-..___tag_value_NucStrstrSearch.39
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.46-..___tag_value_NucStrstrSearch.45
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.47-..___tag_value_NucStrstrSearch.46
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.48-..___tag_value_NucStrstrSearch.47
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.49-..___tag_value_NucStrstrSearch.48
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.50-..___tag_value_NucStrstrSearch.49
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.51-..___tag_value_NucStrstrSearch.50
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x08
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.57-..___tag_value_NucStrstrSearch.51
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.58-..___tag_value_NucStrstrSearch.57
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.59-..___tag_value_NucStrstrSearch.58
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.60-..___tag_value_NucStrstrSearch.59
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.61-..___tag_value_NucStrstrSearch.60
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.62-..___tag_value_NucStrstrSearch.61
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.63-..___tag_value_NucStrstrSearch.62
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x08
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.69-..___tag_value_NucStrstrSearch.63
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.70-..___tag_value_NucStrstrSearch.69
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.71-..___tag_value_NucStrstrSearch.70
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.72-..___tag_value_NucStrstrSearch.71
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.73-..___tag_value_NucStrstrSearch.72
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.74-..___tag_value_NucStrstrSearch.73
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.75-..___tag_value_NucStrstrSearch.74
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x08
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.81-..___tag_value_NucStrstrSearch.75
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.82-..___tag_value_NucStrstrSearch.81
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.83-..___tag_value_NucStrstrSearch.82
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.84-..___tag_value_NucStrstrSearch.83
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.85-..___tag_value_NucStrstrSearch.84
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.86-..___tag_value_NucStrstrSearch.85
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.87-..___tag_value_NucStrstrSearch.86
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x08
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.93-..___tag_value_NucStrstrSearch.87
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.94-..___tag_value_NucStrstrSearch.93
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.95-..___tag_value_NucStrstrSearch.94
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.96-..___tag_value_NucStrstrSearch.95
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.97-..___tag_value_NucStrstrSearch.96
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.98-..___tag_value_NucStrstrSearch.97
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.99-..___tag_value_NucStrstrSearch.98
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x08
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.105-..___tag_value_NucStrstrSearch.99
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.106-..___tag_value_NucStrstrSearch.105
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.107-..___tag_value_NucStrstrSearch.106
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.108-..___tag_value_NucStrstrSearch.107
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.109-..___tag_value_NucStrstrSearch.108
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.110-..___tag_value_NucStrstrSearch.109
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.111-..___tag_value_NucStrstrSearch.110
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x08
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.117-..___tag_value_NucStrstrSearch.111
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.118-..___tag_value_NucStrstrSearch.117
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.119-..___tag_value_NucStrstrSearch.118
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.120-..___tag_value_NucStrstrSearch.119
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.121-..___tag_value_NucStrstrSearch.120
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.122-..___tag_value_NucStrstrSearch.121
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.123-..___tag_value_NucStrstrSearch.122
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x08
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.129-..___tag_value_NucStrstrSearch.123
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.130-..___tag_value_NucStrstrSearch.129
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.131-..___tag_value_NucStrstrSearch.130
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.132-..___tag_value_NucStrstrSearch.131
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.133-..___tag_value_NucStrstrSearch.132
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.134-..___tag_value_NucStrstrSearch.133
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.135-..___tag_value_NucStrstrSearch.134
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x08
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.141-..___tag_value_NucStrstrSearch.135
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.142-..___tag_value_NucStrstrSearch.141
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.143-..___tag_value_NucStrstrSearch.142
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.144-..___tag_value_NucStrstrSearch.143
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.145-..___tag_value_NucStrstrSearch.144
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.146-..___tag_value_NucStrstrSearch.145
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.147-..___tag_value_NucStrstrSearch.146
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x08
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.153-..___tag_value_NucStrstrSearch.147
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.154-..___tag_value_NucStrstrSearch.153
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.155-..___tag_value_NucStrstrSearch.154
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.156-..___tag_value_NucStrstrSearch.155
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.157-..___tag_value_NucStrstrSearch.156
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.158-..___tag_value_NucStrstrSearch.157
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.4byte 0x00000184
	.4byte 0x0000039c
	.4byte ..___tag_value_eval_4na_pos.160-.
	.4byte ..___tag_value_eval_4na_pos.228-..___tag_value_eval_4na_pos.160
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.161-..___tag_value_eval_4na_pos.160
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.163-..___tag_value_eval_4na_pos.161
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.164-..___tag_value_eval_4na_pos.163
	.byte 0x8e
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.165-..___tag_value_eval_4na_pos.164
	.byte 0x8d
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.166-..___tag_value_eval_4na_pos.165
	.byte 0x8c
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.167-..___tag_value_eval_4na_pos.166
	.byte 0x83
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.168-..___tag_value_eval_4na_pos.167
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.169-..___tag_value_eval_4na_pos.168
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.170-..___tag_value_eval_4na_pos.169
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.171-..___tag_value_eval_4na_pos.170
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.172-..___tag_value_eval_4na_pos.171
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.173-..___tag_value_eval_4na_pos.172
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.174-..___tag_value_eval_4na_pos.173
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x07
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.180-..___tag_value_eval_4na_pos.174
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.181-..___tag_value_eval_4na_pos.180
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.182-..___tag_value_eval_4na_pos.181
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.183-..___tag_value_eval_4na_pos.182
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.184-..___tag_value_eval_4na_pos.183
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.185-..___tag_value_eval_4na_pos.184
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.186-..___tag_value_eval_4na_pos.185
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x07
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.192-..___tag_value_eval_4na_pos.186
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.193-..___tag_value_eval_4na_pos.192
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.194-..___tag_value_eval_4na_pos.193
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.195-..___tag_value_eval_4na_pos.194
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.196-..___tag_value_eval_4na_pos.195
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.197-..___tag_value_eval_4na_pos.196
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.198-..___tag_value_eval_4na_pos.197
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x07
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.204-..___tag_value_eval_4na_pos.198
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.205-..___tag_value_eval_4na_pos.204
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.206-..___tag_value_eval_4na_pos.205
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.207-..___tag_value_eval_4na_pos.206
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.208-..___tag_value_eval_4na_pos.207
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.209-..___tag_value_eval_4na_pos.208
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.210-..___tag_value_eval_4na_pos.209
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x07
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.216-..___tag_value_eval_4na_pos.210
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.217-..___tag_value_eval_4na_pos.216
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.218-..___tag_value_eval_4na_pos.217
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.219-..___tag_value_eval_4na_pos.218
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.220-..___tag_value_eval_4na_pos.219
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.221-..___tag_value_eval_4na_pos.220
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.222-..___tag_value_eval_4na_pos.221
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x07
	.byte 0x8f
	.byte 0x08
	.4byte 0x00000000
	.byte 0x00
	.4byte 0x00000034
	.4byte 0x00000524
	.4byte ..___tag_value_prime_buffer_4na.229-.
	.4byte ..___tag_value_prime_buffer_4na.235-..___tag_value_prime_buffer_4na.229
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_4na.230-..___tag_value_prime_buffer_4na.229
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_4na.232-..___tag_value_prime_buffer_4na.230
	.byte 0x8e
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_4na.233-..___tag_value_prime_buffer_4na.232
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_4na.234-..___tag_value_prime_buffer_4na.233
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.4byte 0x00000184
	.4byte 0x0000055c
	.4byte ..___tag_value_eval_2na_pos.236-.
	.4byte ..___tag_value_eval_2na_pos.304-..___tag_value_eval_2na_pos.236
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.237-..___tag_value_eval_2na_pos.236
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.239-..___tag_value_eval_2na_pos.237
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.240-..___tag_value_eval_2na_pos.239
	.byte 0x8e
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.241-..___tag_value_eval_2na_pos.240
	.byte 0x8d
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.242-..___tag_value_eval_2na_pos.241
	.byte 0x8c
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.243-..___tag_value_eval_2na_pos.242
	.byte 0x83
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.244-..___tag_value_eval_2na_pos.243
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.245-..___tag_value_eval_2na_pos.244
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.246-..___tag_value_eval_2na_pos.245
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.247-..___tag_value_eval_2na_pos.246
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.248-..___tag_value_eval_2na_pos.247
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.249-..___tag_value_eval_2na_pos.248
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.250-..___tag_value_eval_2na_pos.249
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x07
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.256-..___tag_value_eval_2na_pos.250
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.257-..___tag_value_eval_2na_pos.256
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.258-..___tag_value_eval_2na_pos.257
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.259-..___tag_value_eval_2na_pos.258
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.260-..___tag_value_eval_2na_pos.259
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.261-..___tag_value_eval_2na_pos.260
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.262-..___tag_value_eval_2na_pos.261
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x07
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.268-..___tag_value_eval_2na_pos.262
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.269-..___tag_value_eval_2na_pos.268
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.270-..___tag_value_eval_2na_pos.269
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.271-..___tag_value_eval_2na_pos.270
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.272-..___tag_value_eval_2na_pos.271
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.273-..___tag_value_eval_2na_pos.272
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.274-..___tag_value_eval_2na_pos.273
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x07
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.280-..___tag_value_eval_2na_pos.274
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.281-..___tag_value_eval_2na_pos.280
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.282-..___tag_value_eval_2na_pos.281
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.283-..___tag_value_eval_2na_pos.282
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.284-..___tag_value_eval_2na_pos.283
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.285-..___tag_value_eval_2na_pos.284
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.286-..___tag_value_eval_2na_pos.285
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x07
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.292-..___tag_value_eval_2na_pos.286
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.293-..___tag_value_eval_2na_pos.292
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.294-..___tag_value_eval_2na_pos.293
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.295-..___tag_value_eval_2na_pos.294
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.296-..___tag_value_eval_2na_pos.295
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.297-..___tag_value_eval_2na_pos.296
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.298-..___tag_value_eval_2na_pos.297
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x07
	.byte 0x8f
	.byte 0x08
	.4byte 0x00000000
	.byte 0x00
	.4byte 0x00000034
	.4byte 0x000006e4
	.4byte ..___tag_value_prime_buffer_2na.305-.
	.4byte ..___tag_value_prime_buffer_2na.311-..___tag_value_prime_buffer_2na.305
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_2na.306-..___tag_value_prime_buffer_2na.305
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_2na.308-..___tag_value_prime_buffer_2na.306
	.byte 0x8c
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_2na.309-..___tag_value_prime_buffer_2na.308
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_prime_buffer_2na.310-..___tag_value_prime_buffer_2na.309
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.4byte 0x00000104
	.4byte 0x0000071c
	.4byte ..___tag_value_eval_4na_128.312-.
	.4byte ..___tag_value_eval_4na_128.356-..___tag_value_eval_4na_128.312
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.313-..___tag_value_eval_4na_128.312
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.315-..___tag_value_eval_4na_128.313
	.byte 0x8f
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.316-..___tag_value_eval_4na_128.315
	.byte 0x8e
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.317-..___tag_value_eval_4na_128.316
	.byte 0x8d
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.318-..___tag_value_eval_4na_128.317
	.byte 0x8c
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.319-..___tag_value_eval_4na_128.318
	.byte 0x83
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.320-..___tag_value_eval_4na_128.319
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.321-..___tag_value_eval_4na_128.320
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.322-..___tag_value_eval_4na_128.321
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.323-..___tag_value_eval_4na_128.322
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.324-..___tag_value_eval_4na_128.323
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.325-..___tag_value_eval_4na_128.324
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.326-..___tag_value_eval_4na_128.325
	.byte 0x83
	.byte 0x08
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x07
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.332-..___tag_value_eval_4na_128.326
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.333-..___tag_value_eval_4na_128.332
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.334-..___tag_value_eval_4na_128.333
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.335-..___tag_value_eval_4na_128.334
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.336-..___tag_value_eval_4na_128.335
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.337-..___tag_value_eval_4na_128.336
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.338-..___tag_value_eval_4na_128.337
	.byte 0x83
	.byte 0x08
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x07
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.344-..___tag_value_eval_4na_128.338
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.345-..___tag_value_eval_4na_128.344
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.346-..___tag_value_eval_4na_128.345
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.347-..___tag_value_eval_4na_128.346
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.348-..___tag_value_eval_4na_128.347
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.349-..___tag_value_eval_4na_128.348
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.350-..___tag_value_eval_4na_128.349
	.byte 0x83
	.byte 0x08
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x07
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x04
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x000000ac
	.4byte 0x00000824
	.4byte ..___tag_value_eval_4na_32.357-.
	.4byte ..___tag_value_eval_4na_32.383-..___tag_value_eval_4na_32.357
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.358-..___tag_value_eval_4na_32.357
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.360-..___tag_value_eval_4na_32.358
	.byte 0x8f
	.byte 0x19
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.361-..___tag_value_eval_4na_32.360
	.byte 0x8e
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.362-..___tag_value_eval_4na_32.361
	.byte 0x8d
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.363-..___tag_value_eval_4na_32.362
	.byte 0x8c
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.364-..___tag_value_eval_4na_32.363
	.byte 0x83
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.365-..___tag_value_eval_4na_32.364
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.366-..___tag_value_eval_4na_32.365
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.367-..___tag_value_eval_4na_32.366
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.368-..___tag_value_eval_4na_32.367
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.369-..___tag_value_eval_4na_32.368
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.370-..___tag_value_eval_4na_32.369
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.371-..___tag_value_eval_4na_32.370
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x08
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x19
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.377-..___tag_value_eval_4na_32.371
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.378-..___tag_value_eval_4na_32.377
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.379-..___tag_value_eval_4na_32.378
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.380-..___tag_value_eval_4na_32.379
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.381-..___tag_value_eval_4na_32.380
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.382-..___tag_value_eval_4na_32.381
	.byte 0x09
	.2byte 0x0606
	.byte 0x00
	.4byte 0x00000034
	.4byte 0x000008d4
	.4byte ..___tag_value_uint16_lsbit.384-.
	.4byte ..___tag_value_uint16_lsbit.390-..___tag_value_uint16_lsbit.384
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_uint16_lsbit.385-..___tag_value_uint16_lsbit.384
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_uint16_lsbit.387-..___tag_value_uint16_lsbit.385
	.byte 0x83
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_uint16_lsbit.388-..___tag_value_uint16_lsbit.387
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_uint16_lsbit.389-..___tag_value_uint16_lsbit.388
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.4byte 0x000000ac
	.4byte 0x0000090c
	.4byte ..___tag_value_eval_4na_16.391-.
	.4byte ..___tag_value_eval_4na_16.417-..___tag_value_eval_4na_16.391
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.392-..___tag_value_eval_4na_16.391
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.394-..___tag_value_eval_4na_16.392
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.395-..___tag_value_eval_4na_16.394
	.byte 0x8e
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.396-..___tag_value_eval_4na_16.395
	.byte 0x8d
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.397-..___tag_value_eval_4na_16.396
	.byte 0x8c
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.398-..___tag_value_eval_4na_16.397
	.byte 0x83
	.byte 0x09
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.399-..___tag_value_eval_4na_16.398
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.400-..___tag_value_eval_4na_16.399
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.401-..___tag_value_eval_4na_16.400
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.402-..___tag_value_eval_4na_16.401
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.403-..___tag_value_eval_4na_16.402
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.404-..___tag_value_eval_4na_16.403
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.405-..___tag_value_eval_4na_16.404
	.byte 0x83
	.byte 0x09
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x07
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.411-..___tag_value_eval_4na_16.405
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.412-..___tag_value_eval_4na_16.411
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.413-..___tag_value_eval_4na_16.412
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.414-..___tag_value_eval_4na_16.413
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.415-..___tag_value_eval_4na_16.414
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.416-..___tag_value_eval_4na_16.415
	.byte 0x09
	.2byte 0x0606
	.byte 0x00
	.4byte 0x00000104
	.4byte 0x000009bc
	.4byte ..___tag_value_eval_2na_128.418-.
	.4byte ..___tag_value_eval_2na_128.462-..___tag_value_eval_2na_128.418
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.419-..___tag_value_eval_2na_128.418
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.421-..___tag_value_eval_2na_128.419
	.byte 0x8f
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.422-..___tag_value_eval_2na_128.421
	.byte 0x8e
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.423-..___tag_value_eval_2na_128.422
	.byte 0x8d
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.424-..___tag_value_eval_2na_128.423
	.byte 0x8c
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.425-..___tag_value_eval_2na_128.424
	.byte 0x83
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.426-..___tag_value_eval_2na_128.425
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.427-..___tag_value_eval_2na_128.426
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.428-..___tag_value_eval_2na_128.427
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.429-..___tag_value_eval_2na_128.428
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.430-..___tag_value_eval_2na_128.429
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.431-..___tag_value_eval_2na_128.430
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.432-..___tag_value_eval_2na_128.431
	.byte 0x83
	.byte 0x08
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x04
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.438-..___tag_value_eval_2na_128.432
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.439-..___tag_value_eval_2na_128.438
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.440-..___tag_value_eval_2na_128.439
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.441-..___tag_value_eval_2na_128.440
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.442-..___tag_value_eval_2na_128.441
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.443-..___tag_value_eval_2na_128.442
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.444-..___tag_value_eval_2na_128.443
	.byte 0x83
	.byte 0x08
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x04
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.450-..___tag_value_eval_2na_128.444
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.451-..___tag_value_eval_2na_128.450
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.452-..___tag_value_eval_2na_128.451
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.453-..___tag_value_eval_2na_128.452
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.454-..___tag_value_eval_2na_128.453
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.455-..___tag_value_eval_2na_128.454
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.456-..___tag_value_eval_2na_128.455
	.byte 0x83
	.byte 0x08
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x04
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x07
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x000000ac
	.4byte 0x00000ac4
	.4byte ..___tag_value_eval_2na_32.463-.
	.4byte ..___tag_value_eval_2na_32.489-..___tag_value_eval_2na_32.463
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.464-..___tag_value_eval_2na_32.463
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.466-..___tag_value_eval_2na_32.464
	.byte 0x8f
	.byte 0x09
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.467-..___tag_value_eval_2na_32.466
	.byte 0x8e
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.468-..___tag_value_eval_2na_32.467
	.byte 0x8d
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.469-..___tag_value_eval_2na_32.468
	.byte 0x8c
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.470-..___tag_value_eval_2na_32.469
	.byte 0x83
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.471-..___tag_value_eval_2na_32.470
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.472-..___tag_value_eval_2na_32.471
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.473-..___tag_value_eval_2na_32.472
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.474-..___tag_value_eval_2na_32.473
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.475-..___tag_value_eval_2na_32.474
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.476-..___tag_value_eval_2na_32.475
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.477-..___tag_value_eval_2na_32.476
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x07
	.byte 0x8d
	.byte 0x05
	.byte 0x8e
	.byte 0x08
	.byte 0x8f
	.byte 0x09
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.483-..___tag_value_eval_2na_32.477
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.484-..___tag_value_eval_2na_32.483
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.485-..___tag_value_eval_2na_32.484
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.486-..___tag_value_eval_2na_32.485
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.487-..___tag_value_eval_2na_32.486
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.488-..___tag_value_eval_2na_32.487
	.byte 0x09
	.2byte 0x0606
	.byte 0x00
	.4byte 0x000000ac
	.4byte 0x00000b74
	.4byte ..___tag_value_eval_2na_16.490-.
	.4byte ..___tag_value_eval_2na_16.516-..___tag_value_eval_2na_16.490
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.491-..___tag_value_eval_2na_16.490
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.493-..___tag_value_eval_2na_16.491
	.byte 0x8f
	.byte 0x09
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.494-..___tag_value_eval_2na_16.493
	.byte 0x8e
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.495-..___tag_value_eval_2na_16.494
	.byte 0x8d
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.496-..___tag_value_eval_2na_16.495
	.byte 0x8c
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.497-..___tag_value_eval_2na_16.496
	.byte 0x83
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.498-..___tag_value_eval_2na_16.497
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.499-..___tag_value_eval_2na_16.498
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.500-..___tag_value_eval_2na_16.499
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.501-..___tag_value_eval_2na_16.500
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.502-..___tag_value_eval_2na_16.501
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.503-..___tag_value_eval_2na_16.502
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.504-..___tag_value_eval_2na_16.503
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x07
	.byte 0x8d
	.byte 0x05
	.byte 0x8e
	.byte 0x08
	.byte 0x8f
	.byte 0x09
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.510-..___tag_value_eval_2na_16.504
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.511-..___tag_value_eval_2na_16.510
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.512-..___tag_value_eval_2na_16.511
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.513-..___tag_value_eval_2na_16.512
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.514-..___tag_value_eval_2na_16.513
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.515-..___tag_value_eval_2na_16.514
	.byte 0x09
	.2byte 0x0606
	.byte 0x00
	.4byte 0x000000ac
	.4byte 0x00000c24
	.4byte ..___tag_value_eval_2na_8.517-.
	.4byte ..___tag_value_eval_2na_8.543-..___tag_value_eval_2na_8.517
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.518-..___tag_value_eval_2na_8.517
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.520-..___tag_value_eval_2na_8.518
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.521-..___tag_value_eval_2na_8.520
	.byte 0x8e
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.522-..___tag_value_eval_2na_8.521
	.byte 0x8d
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.523-..___tag_value_eval_2na_8.522
	.byte 0x8c
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.524-..___tag_value_eval_2na_8.523
	.byte 0x83
	.byte 0x09
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.525-..___tag_value_eval_2na_8.524
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.526-..___tag_value_eval_2na_8.525
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.527-..___tag_value_eval_2na_8.526
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.528-..___tag_value_eval_2na_8.527
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.529-..___tag_value_eval_2na_8.528
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.530-..___tag_value_eval_2na_8.529
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.531-..___tag_value_eval_2na_8.530
	.byte 0x83
	.byte 0x09
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x07
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.537-..___tag_value_eval_2na_8.531
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.538-..___tag_value_eval_2na_8.537
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.539-..___tag_value_eval_2na_8.538
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.540-..___tag_value_eval_2na_8.539
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.541-..___tag_value_eval_2na_8.540
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.542-..___tag_value_eval_2na_8.541
	.byte 0x09
	.2byte 0x0606
	.byte 0x00
	.4byte 0x000000ac
	.4byte 0x00000cd4
	.4byte ..___tag_value_eval_4na_64.544-.
	.4byte ..___tag_value_eval_4na_64.570-..___tag_value_eval_4na_64.544
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.545-..___tag_value_eval_4na_64.544
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.547-..___tag_value_eval_4na_64.545
	.byte 0x8f
	.byte 0x19
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.548-..___tag_value_eval_4na_64.547
	.byte 0x8e
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.549-..___tag_value_eval_4na_64.548
	.byte 0x8d
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.550-..___tag_value_eval_4na_64.549
	.byte 0x8c
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.551-..___tag_value_eval_4na_64.550
	.byte 0x83
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.552-..___tag_value_eval_4na_64.551
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.553-..___tag_value_eval_4na_64.552
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.554-..___tag_value_eval_4na_64.553
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.555-..___tag_value_eval_4na_64.554
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.556-..___tag_value_eval_4na_64.555
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.557-..___tag_value_eval_4na_64.556
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.558-..___tag_value_eval_4na_64.557
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x08
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x19
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.564-..___tag_value_eval_4na_64.558
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.565-..___tag_value_eval_4na_64.564
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.566-..___tag_value_eval_4na_64.565
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.567-..___tag_value_eval_4na_64.566
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.568-..___tag_value_eval_4na_64.567
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.569-..___tag_value_eval_4na_64.568
	.byte 0x09
	.2byte 0x0606
	.byte 0x00
	.4byte 0x000000ac
	.4byte 0x00000d84
	.4byte ..___tag_value_eval_2na_64.571-.
	.4byte ..___tag_value_eval_2na_64.597-..___tag_value_eval_2na_64.571
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.572-..___tag_value_eval_2na_64.571
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.574-..___tag_value_eval_2na_64.572
	.byte 0x8f
	.byte 0x09
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.575-..___tag_value_eval_2na_64.574
	.byte 0x8e
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.576-..___tag_value_eval_2na_64.575
	.byte 0x8d
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.577-..___tag_value_eval_2na_64.576
	.byte 0x8c
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.578-..___tag_value_eval_2na_64.577
	.byte 0x83
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.579-..___tag_value_eval_2na_64.578
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.580-..___tag_value_eval_2na_64.579
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.581-..___tag_value_eval_2na_64.580
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.582-..___tag_value_eval_2na_64.581
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.583-..___tag_value_eval_2na_64.582
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.584-..___tag_value_eval_2na_64.583
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.585-..___tag_value_eval_2na_64.584
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x07
	.byte 0x8d
	.byte 0x05
	.byte 0x8e
	.byte 0x08
	.byte 0x8f
	.byte 0x09
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.591-..___tag_value_eval_2na_64.585
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.592-..___tag_value_eval_2na_64.591
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.593-..___tag_value_eval_2na_64.592
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.594-..___tag_value_eval_2na_64.593
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.595-..___tag_value_eval_2na_64.594
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.596-..___tag_value_eval_2na_64.595
	.byte 0x09
	.2byte 0x0606
	.byte 0x00
	.4byte 0x000000cc
	.4byte 0x00000e34
	.4byte ..___tag_value_NucStrstrMake.598-.
	.4byte ..___tag_value_NucStrstrMake.630-..___tag_value_NucStrstrMake.598
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.599-..___tag_value_NucStrstrMake.598
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.601-..___tag_value_NucStrstrMake.599
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.602-..___tag_value_NucStrstrMake.601
	.byte 0x8e
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.603-..___tag_value_NucStrstrMake.602
	.byte 0x8d
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.604-..___tag_value_NucStrstrMake.603
	.byte 0x8c
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.605-..___tag_value_NucStrstrMake.604
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.606-..___tag_value_NucStrstrMake.605
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.607-..___tag_value_NucStrstrMake.606
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.608-..___tag_value_NucStrstrMake.607
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.609-..___tag_value_NucStrstrMake.608
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.610-..___tag_value_NucStrstrMake.609
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x07
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.615-..___tag_value_NucStrstrMake.610
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.616-..___tag_value_NucStrstrMake.615
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.617-..___tag_value_NucStrstrMake.616
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.618-..___tag_value_NucStrstrMake.617
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.619-..___tag_value_NucStrstrMake.618
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.620-..___tag_value_NucStrstrMake.619
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x07
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.625-..___tag_value_NucStrstrMake.620
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.626-..___tag_value_NucStrstrMake.625
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.627-..___tag_value_NucStrstrMake.626
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.628-..___tag_value_NucStrstrMake.627
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.629-..___tag_value_NucStrstrMake.628
	.byte 0x09
	.2byte 0x0606
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000144
	.4byte 0x00000f04
	.4byte ..___tag_value_nss_expr.631-.
	.4byte ..___tag_value_nss_expr.684-..___tag_value_nss_expr.631
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_nss_expr.632-..___tag_value_nss_expr.631
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_expr.634-..___tag_value_nss_expr.632
	.byte 0x8e
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_nss_expr.635-..___tag_value_nss_expr.634
	.byte 0x8d
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_nss_expr.636-..___tag_value_nss_expr.635
	.byte 0x8c
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_nss_expr.637-..___tag_value_nss_expr.636
	.byte 0x83
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_nss_expr.638-..___tag_value_nss_expr.637
	.byte 0x8f
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_nss_expr.639-..___tag_value_nss_expr.638
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_nss_expr.640-..___tag_value_nss_expr.639
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_expr.641-..___tag_value_nss_expr.640
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_expr.642-..___tag_value_nss_expr.641
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_nss_expr.643-..___tag_value_nss_expr.642
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_expr.644-..___tag_value_nss_expr.643
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_expr.645-..___tag_value_nss_expr.644
	.byte 0x83
	.byte 0x08
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x07
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_nss_expr.651-..___tag_value_nss_expr.645
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_nss_expr.652-..___tag_value_nss_expr.651
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_expr.653-..___tag_value_nss_expr.652
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_expr.654-..___tag_value_nss_expr.653
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_nss_expr.655-..___tag_value_nss_expr.654
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_expr.656-..___tag_value_nss_expr.655
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_expr.657-..___tag_value_nss_expr.656
	.byte 0x83
	.byte 0x08
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x07
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_nss_expr.663-..___tag_value_nss_expr.657
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x8f
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_nss_expr.665-..___tag_value_nss_expr.663
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_expr.666-..___tag_value_nss_expr.665
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_expr.667-..___tag_value_nss_expr.666
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_nss_expr.668-..___tag_value_nss_expr.667
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_expr.669-..___tag_value_nss_expr.668
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_expr.670-..___tag_value_nss_expr.669
	.byte 0x83
	.byte 0x08
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x07
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_nss_expr.675-..___tag_value_nss_expr.670
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_nss_expr.676-..___tag_value_nss_expr.675
	.byte 0x8f
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_nss_expr.677-..___tag_value_nss_expr.676
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_nss_expr.678-..___tag_value_nss_expr.677
	.byte 0x8f
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_nss_expr.679-..___tag_value_nss_expr.678
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_expr.680-..___tag_value_nss_expr.679
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_nss_expr.681-..___tag_value_nss_expr.680
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_expr.682-..___tag_value_nss_expr.681
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_expr.683-..___tag_value_nss_expr.682
	.byte 0x09
	.2byte 0x0606
	.byte 0x00
	.4byte 0x000000bc
	.4byte 0x0000104c
	.4byte ..___tag_value_nss_unary_expr.685-.
	.4byte ..___tag_value_nss_unary_expr.717-..___tag_value_nss_unary_expr.685
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.686-..___tag_value_nss_unary_expr.685
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.688-..___tag_value_nss_unary_expr.686
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.689-..___tag_value_nss_unary_expr.688
	.byte 0x8e
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.690-..___tag_value_nss_unary_expr.689
	.byte 0x8d
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.691-..___tag_value_nss_unary_expr.690
	.byte 0x8c
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.692-..___tag_value_nss_unary_expr.691
	.byte 0x83
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.693-..___tag_value_nss_unary_expr.692
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.694-..___tag_value_nss_unary_expr.693
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.695-..___tag_value_nss_unary_expr.694
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.696-..___tag_value_nss_unary_expr.695
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.697-..___tag_value_nss_unary_expr.696
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.698-..___tag_value_nss_unary_expr.697
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.699-..___tag_value_nss_unary_expr.698
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x07
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.705-..___tag_value_nss_unary_expr.699
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.706-..___tag_value_nss_unary_expr.705
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.707-..___tag_value_nss_unary_expr.706
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.708-..___tag_value_nss_unary_expr.707
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.709-..___tag_value_nss_unary_expr.708
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.710-..___tag_value_nss_unary_expr.709
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.711-..___tag_value_nss_unary_expr.710
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x07
	.byte 0x8f
	.byte 0x08
	.4byte 0x00000104
	.4byte 0x0000110c
	.4byte ..___tag_value_nss_primary_expr.718-.
	.4byte ..___tag_value_nss_primary_expr.762-..___tag_value_nss_primary_expr.718
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.719-..___tag_value_nss_primary_expr.718
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.721-..___tag_value_nss_primary_expr.719
	.byte 0x8f
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.722-..___tag_value_nss_primary_expr.721
	.byte 0x8e
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.723-..___tag_value_nss_primary_expr.722
	.byte 0x8d
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.724-..___tag_value_nss_primary_expr.723
	.byte 0x8c
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.725-..___tag_value_nss_primary_expr.724
	.byte 0x83
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.726-..___tag_value_nss_primary_expr.725
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.727-..___tag_value_nss_primary_expr.726
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.728-..___tag_value_nss_primary_expr.727
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.729-..___tag_value_nss_primary_expr.728
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.730-..___tag_value_nss_primary_expr.729
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.731-..___tag_value_nss_primary_expr.730
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.732-..___tag_value_nss_primary_expr.731
	.byte 0x83
	.byte 0x03
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x04
	.byte 0x8d
	.byte 0x05
	.byte 0x8e
	.byte 0x06
	.byte 0x8f
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.738-..___tag_value_nss_primary_expr.732
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.739-..___tag_value_nss_primary_expr.738
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.740-..___tag_value_nss_primary_expr.739
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.741-..___tag_value_nss_primary_expr.740
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.742-..___tag_value_nss_primary_expr.741
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.743-..___tag_value_nss_primary_expr.742
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.744-..___tag_value_nss_primary_expr.743
	.byte 0x83
	.byte 0x03
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x04
	.byte 0x8d
	.byte 0x05
	.byte 0x8e
	.byte 0x06
	.byte 0x8f
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.750-..___tag_value_nss_primary_expr.744
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.751-..___tag_value_nss_primary_expr.750
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.752-..___tag_value_nss_primary_expr.751
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.753-..___tag_value_nss_primary_expr.752
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.754-..___tag_value_nss_primary_expr.753
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.755-..___tag_value_nss_primary_expr.754
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_primary_expr.756-..___tag_value_nss_primary_expr.755
	.byte 0x83
	.byte 0x03
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x04
	.byte 0x8d
	.byte 0x05
	.byte 0x8e
	.byte 0x06
	.byte 0x8f
	.byte 0x07
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x0000005c
	.4byte 0x00001214
	.4byte ..___tag_value_nss_sob.763-.
	.4byte ..___tag_value_nss_sob.775-..___tag_value_nss_sob.763
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_nss_sob.764-..___tag_value_nss_sob.763
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_sob.766-..___tag_value_nss_sob.764
	.byte 0x8f
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_nss_sob.767-..___tag_value_nss_sob.766
	.byte 0x8e
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_nss_sob.768-..___tag_value_nss_sob.767
	.byte 0x8d
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_nss_sob.769-..___tag_value_nss_sob.768
	.byte 0x83
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_nss_sob.770-..___tag_value_nss_sob.769
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_sob.771-..___tag_value_nss_sob.770
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_nss_sob.772-..___tag_value_nss_sob.771
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_sob.773-..___tag_value_nss_sob.772
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_nss_sob.774-..___tag_value_nss_sob.773
	.byte 0x09
	.2byte 0x0606
	.byte 0x00
	.4byte 0x000000ac
	.4byte 0x00001274
	.4byte ..___tag_value_nss_fasta_expr.776-.
	.4byte ..___tag_value_nss_fasta_expr.802-..___tag_value_nss_fasta_expr.776
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.777-..___tag_value_nss_fasta_expr.776
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.779-..___tag_value_nss_fasta_expr.777
	.byte 0x8f
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.780-..___tag_value_nss_fasta_expr.779
	.byte 0x8e
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.781-..___tag_value_nss_fasta_expr.780
	.byte 0x8d
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.782-..___tag_value_nss_fasta_expr.781
	.byte 0x8c
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.783-..___tag_value_nss_fasta_expr.782
	.byte 0x83
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.784-..___tag_value_nss_fasta_expr.783
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.785-..___tag_value_nss_fasta_expr.784
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.786-..___tag_value_nss_fasta_expr.785
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.787-..___tag_value_nss_fasta_expr.786
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.788-..___tag_value_nss_fasta_expr.787
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.789-..___tag_value_nss_fasta_expr.788
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.790-..___tag_value_nss_fasta_expr.789
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x07
	.byte 0x8d
	.byte 0x08
	.byte 0x8e
	.byte 0x04
	.byte 0x8f
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.796-..___tag_value_nss_fasta_expr.790
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.797-..___tag_value_nss_fasta_expr.796
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.798-..___tag_value_nss_fasta_expr.797
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.799-..___tag_value_nss_fasta_expr.798
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.800-..___tag_value_nss_fasta_expr.799
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.801-..___tag_value_nss_fasta_expr.800
	.byte 0x09
	.2byte 0x0606
	.byte 0x00
	.4byte 0x0000007c
	.4byte 0x00001324
	.4byte ..___tag_value_nss_FASTA_expr.803-.
	.4byte ..___tag_value_nss_FASTA_expr.823-..___tag_value_nss_FASTA_expr.803
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.804-..___tag_value_nss_FASTA_expr.803
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.806-..___tag_value_nss_FASTA_expr.804
	.byte 0x8f
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.807-..___tag_value_nss_FASTA_expr.806
	.byte 0x8e
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.808-..___tag_value_nss_FASTA_expr.807
	.byte 0x8d
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.809-..___tag_value_nss_FASTA_expr.808
	.byte 0x8c
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.810-..___tag_value_nss_FASTA_expr.809
	.byte 0x83
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.811-..___tag_value_nss_FASTA_expr.810
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.812-..___tag_value_nss_FASTA_expr.811
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.813-..___tag_value_nss_FASTA_expr.812
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.814-..___tag_value_nss_FASTA_expr.813
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.815-..___tag_value_nss_FASTA_expr.814
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.816-..___tag_value_nss_FASTA_expr.815
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_FASTA_expr.817-..___tag_value_nss_FASTA_expr.816
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x06
	.byte 0x8e
	.byte 0x07
	.byte 0x8f
	.byte 0x08
	.byte 0x00
	.4byte 0x000000ec
	.4byte 0x000013a4
	.4byte ..___tag_value_NucStrFastaExprMake2.824-.
	.4byte ..___tag_value_NucStrFastaExprMake2.862-..___tag_value_NucStrFastaExprMake2.824
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.825-..___tag_value_NucStrFastaExprMake2.824
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.827-..___tag_value_NucStrFastaExprMake2.825
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.828-..___tag_value_NucStrFastaExprMake2.827
	.byte 0x8e
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.829-..___tag_value_NucStrFastaExprMake2.828
	.byte 0x8d
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.830-..___tag_value_NucStrFastaExprMake2.829
	.byte 0x8c
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.831-..___tag_value_NucStrFastaExprMake2.830
	.byte 0x83
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.832-..___tag_value_NucStrFastaExprMake2.831
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.833-..___tag_value_NucStrFastaExprMake2.832
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.834-..___tag_value_NucStrFastaExprMake2.833
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.835-..___tag_value_NucStrFastaExprMake2.834
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.836-..___tag_value_NucStrFastaExprMake2.835
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.837-..___tag_value_NucStrFastaExprMake2.836
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.838-..___tag_value_NucStrFastaExprMake2.837
	.byte 0x83
	.byte 0x05
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x04
	.byte 0x8d
	.byte 0x07
	.byte 0x8e
	.byte 0x08
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.844-..___tag_value_NucStrFastaExprMake2.838
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.845-..___tag_value_NucStrFastaExprMake2.844
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.846-..___tag_value_NucStrFastaExprMake2.845
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.847-..___tag_value_NucStrFastaExprMake2.846
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.848-..___tag_value_NucStrFastaExprMake2.847
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.849-..___tag_value_NucStrFastaExprMake2.848
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.850-..___tag_value_NucStrFastaExprMake2.849
	.byte 0x83
	.byte 0x05
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x04
	.byte 0x8d
	.byte 0x07
	.byte 0x8e
	.byte 0x08
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.856-..___tag_value_NucStrFastaExprMake2.850
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.857-..___tag_value_NucStrFastaExprMake2.856
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.858-..___tag_value_NucStrFastaExprMake2.857
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.859-..___tag_value_NucStrFastaExprMake2.858
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.860-..___tag_value_NucStrFastaExprMake2.859
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.861-..___tag_value_NucStrFastaExprMake2.860
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000044
	.4byte 0x00001494
	.4byte ..___tag_value_uint128_bswap_copy.863-.
	.4byte ..___tag_value_uint128_bswap_copy.871-..___tag_value_uint128_bswap_copy.863
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap_copy.864-..___tag_value_uint128_bswap_copy.863
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap_copy.866-..___tag_value_uint128_bswap_copy.864
	.byte 0x8d
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap_copy.867-..___tag_value_uint128_bswap_copy.866
	.byte 0x8c
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap_copy.868-..___tag_value_uint128_bswap_copy.867
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap_copy.869-..___tag_value_uint128_bswap_copy.868
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap_copy.870-..___tag_value_uint128_bswap_copy.869
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000044
	.4byte 0x000014dc
	.4byte ..___tag_value_uint128_shr.872-.
	.4byte ..___tag_value_uint128_shr.880-..___tag_value_uint128_shr.872
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_uint128_shr.873-..___tag_value_uint128_shr.872
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_uint128_shr.875-..___tag_value_uint128_shr.873
	.byte 0x8d
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_uint128_shr.876-..___tag_value_uint128_shr.875
	.byte 0x8c
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_uint128_shr.877-..___tag_value_uint128_shr.876
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_uint128_shr.878-..___tag_value_uint128_shr.877
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_uint128_shr.879-..___tag_value_uint128_shr.878
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000034
	.4byte 0x00001524
	.4byte ..___tag_value_uint128_bswap.881-.
	.4byte ..___tag_value_uint128_bswap.887-..___tag_value_uint128_bswap.881
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap.882-..___tag_value_uint128_bswap.881
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap.884-..___tag_value_uint128_bswap.882
	.byte 0x8d
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap.885-..___tag_value_uint128_bswap.884
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_uint128_bswap.886-..___tag_value_uint128_bswap.885
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.4byte 0x00000034
	.4byte 0x0000155c
	.4byte ..___tag_value_NucStrFastaExprAlloc.888-.
	.4byte ..___tag_value_NucStrFastaExprAlloc.894-..___tag_value_NucStrFastaExprAlloc.888
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprAlloc.889-..___tag_value_NucStrFastaExprAlloc.888
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprAlloc.891-..___tag_value_NucStrFastaExprAlloc.889
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprAlloc.892-..___tag_value_NucStrFastaExprAlloc.891
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprAlloc.893-..___tag_value_NucStrFastaExprAlloc.892
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.4byte 0x000000ec
	.4byte 0x00001594
	.4byte ..___tag_value_NucStrFastaExprMake4.895-.
	.4byte ..___tag_value_NucStrFastaExprMake4.933-..___tag_value_NucStrFastaExprMake4.895
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.896-..___tag_value_NucStrFastaExprMake4.895
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.898-..___tag_value_NucStrFastaExprMake4.896
	.byte 0x8f
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.899-..___tag_value_NucStrFastaExprMake4.898
	.byte 0x8e
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.900-..___tag_value_NucStrFastaExprMake4.899
	.byte 0x8d
	.byte 0x07
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.901-..___tag_value_NucStrFastaExprMake4.900
	.byte 0x8c
	.byte 0x08
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.902-..___tag_value_NucStrFastaExprMake4.901
	.byte 0x83
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.903-..___tag_value_NucStrFastaExprMake4.902
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.904-..___tag_value_NucStrFastaExprMake4.903
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.905-..___tag_value_NucStrFastaExprMake4.904
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.906-..___tag_value_NucStrFastaExprMake4.905
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.907-..___tag_value_NucStrFastaExprMake4.906
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.908-..___tag_value_NucStrFastaExprMake4.907
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.909-..___tag_value_NucStrFastaExprMake4.908
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x08
	.byte 0x8d
	.byte 0x07
	.byte 0x8e
	.byte 0x04
	.byte 0x8f
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.915-..___tag_value_NucStrFastaExprMake4.909
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.916-..___tag_value_NucStrFastaExprMake4.915
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.917-..___tag_value_NucStrFastaExprMake4.916
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.918-..___tag_value_NucStrFastaExprMake4.917
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.919-..___tag_value_NucStrFastaExprMake4.918
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.920-..___tag_value_NucStrFastaExprMake4.919
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.921-..___tag_value_NucStrFastaExprMake4.920
	.byte 0x83
	.byte 0x06
	.byte 0x86
	.byte 0x02
	.byte 0x8c
	.byte 0x08
	.byte 0x8d
	.byte 0x07
	.byte 0x8e
	.byte 0x04
	.byte 0x8f
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.927-..___tag_value_NucStrFastaExprMake4.921
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.928-..___tag_value_NucStrFastaExprMake4.927
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.929-..___tag_value_NucStrFastaExprMake4.928
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.930-..___tag_value_NucStrFastaExprMake4.929
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.931-..___tag_value_NucStrFastaExprMake4.930
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.932-..___tag_value_NucStrFastaExprMake4.931
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000034
	.4byte 0x00001684
	.4byte ..___tag_value_NucStrstrWhack.934-.
	.4byte ..___tag_value_NucStrstrWhack.940-..___tag_value_NucStrstrWhack.934
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_NucStrstrWhack.935-..___tag_value_NucStrstrWhack.934
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrWhack.937-..___tag_value_NucStrstrWhack.935
	.byte 0x8d
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_NucStrstrWhack.938-..___tag_value_NucStrstrWhack.937
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrWhack.939-..___tag_value_NucStrstrWhack.938
	.byte 0x09
	.2byte 0x0606
	.4byte 0x00000000
	.2byte 0x0000
	.4byte 0x0000004c
	.4byte 0x000016bc
	.4byte ..___tag_value_NucStrstrInit.941-.
	.4byte ..___tag_value_NucStrstrInit.951-..___tag_value_NucStrstrInit.941
	.byte 0x00
	.byte 0x04
	.4byte ..___tag_value_NucStrstrInit.942-..___tag_value_NucStrstrInit.941
	.byte 0x0c
	.2byte 0x1006
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrInit.944-..___tag_value_NucStrstrInit.942
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrInit.945-..___tag_value_NucStrstrInit.944
	.byte 0x8c
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_NucStrstrInit.946-..___tag_value_NucStrstrInit.945
	.byte 0x83
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_NucStrstrInit.947-..___tag_value_NucStrstrInit.946
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrInit.948-..___tag_value_NucStrstrInit.947
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrInit.949-..___tag_value_NucStrstrInit.948
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrInit.950-..___tag_value_NucStrstrInit.949
	.byte 0x09
	.2byte 0x0606
	.section .text
.LNDBG_TXe:
# End
