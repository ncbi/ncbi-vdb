# -- Machine type EFI2
# mark_description "Intel(R) C++ Compiler Professional for applications running on Intel(R) 64, Version 11.0    Build 20090131 %";
# mark_description "s";
# mark_description "-I/home/yaschenk/devel/internal/asm-trace/sra/search/linux -I/home/yaschenk/devel/internal/asm-trace/sra/sea";
# mark_description "rch/unix -I/home/yaschenk/devel/internal/asm-trace/sra/search -I/home/yaschenk/devel/internal/asm-trace/itf ";
# mark_description "-I/home/yaschenk/devel/internal/asm-trace/inc/icc/x86_64 -I/home/yaschenk/devel/internal/asm-trace/inc/icc -";
# mark_description "I/home/yaschenk/devel/internal/asm-trace/inc/gcc/x86_64 -I/home/yaschenk/devel/internal/asm-trace/inc/gcc -I";
# mark_description "/home/yaschenk/devel/internal/asm-trace/inc/linux -I/home/yaschenk/devel/internal/asm-trace/inc/unix -I/home";
# mark_description "/yaschenk/devel/internal/asm-trace/inc -c -S -o nucstrstr.pic.s -fPIC -O3 -unroll -xW -DNDEBUG -DLINUX -DUNI";
# mark_description "X -D_GNU_SOURCE -D_REENTRANT -D_FILE_OFFSET_BITS=64 -MD";
	.file "nucstrstr.c"
	.text
..TXTST0:
# -- Begin  NucStrstrSearch
# mark_begin;
       .align    16,0x90
	.globl NucStrstrSearch
NucStrstrSearch:
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %edx
# parameter 4: %ecx
# parameter 5: %r8
..B1.1:                         # Preds ..B1.0
..___tag_value_NucStrstrSearch.1:                               #5217.1
        pushq     %rbx                                          #5217.1
..___tag_value_NucStrstrSearch.3:                               #
        subq      $32, %rsp                                     #5217.1
..___tag_value_NucStrstrSearch.5:                               #
        movq      %rdi, %rbx                                    #5217.1
        testq     %rbx, %rbx                                    #5218.18
        je        ..B1.107      # Prob 6%                       #5218.18
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 edx ecx
..B1.2:                         # Preds ..B1.1
        testq     %rsi, %rsi                                    #5218.37
        je        ..B1.107      # Prob 6%                       #5218.37
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 edx ecx
..B1.4:                         # Preds ..B1.2 ..B1.14 ..B1.17
        testl     %ecx, %ecx                                    #5218.52
        je        ..B1.107      # Prob 28%                      #5218.52
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 edx ecx
..B1.6:                         # Preds ..B1.4
        movq      %r12, 24(%rsp)                                #
..___tag_value_NucStrstrSearch.6:                               #
        movq      %rsi, %r12                                    #
        movq      %r13, 16(%rsp)                                #
..___tag_value_NucStrstrSearch.7:                               #
        movq      %r14, 8(%rsp)                                 #
..___tag_value_NucStrstrSearch.8:                               #
        movq      %r15, (%rsp)                                  #
..___tag_value_NucStrstrSearch.9:                               #
        movl      %edx, %r13d                                   #
        movl      %ecx, %r14d                                   #
        movq      %r8, %r15                                     #
                                # LOE rbx rbp r12 r15 r13d r14d
..B1.7:                         # Preds ..B1.31 ..B1.6
        movl      (%rbx), %eax                                  #5223.18
        cmpl      $12, %eax                                     #5223.9
        ja        ..B1.32       # Prob 28%                      #5223.9
                                # LOE rbx rbp r12 r15 eax r13d r14d
..B1.8:                         # Preds ..B1.7
        movl      %eax, %eax                                    #5223.9
        lea       ..1..TPKT.30_0.0.1(%rip), %rdi                #5223.9
        movq      (%rdi,%rax,8), %r9                            #5223.9
        jmp       *%r9                                          #5223.9
                                # LOE rbx rbp r12 r15 r13d r14d
..1.30_0.TAG.0c.0.1:
..B1.9:                         # Preds ..B1.8
        movq      %r12, %rsi                                    #
        movq      24(%rsp), %r12                                #
..___tag_value_NucStrstrSearch.10:                              #
        movl      %r13d, %edx                                   #
        movq      16(%rsp), %r13                                #
..___tag_value_NucStrstrSearch.11:                              #
        movl      %r14d, %ecx                                   #
        movq      %r15, %r8                                     #
        movq      8(%rsp), %r14                                 #
..___tag_value_NucStrstrSearch.12:                              #
        movq      (%rsp), %r15                                  #
..___tag_value_NucStrstrSearch.13:                              #
..___tag_value_NucStrstrSearch.14:                              #
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 edx ecx
..B1.10:                        # Preds ..B1.9
        movl      4(%rbx), %eax                                 #5286.22
        testl     %eax, %eax                                    #5286.22
        je        ..B1.20       # Prob 20%                      #5286.22
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.11:                        # Preds ..B1.10
        cmpl      $13, %eax                                     #5286.22
        je        ..B1.20       # Prob 25%                      #5286.22
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.12:                        # Preds ..B1.11
        cmpl      $14, %eax                                     #5286.22
        jne       ..B1.15       # Prob 67%                      #5286.22
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.13:                        # Preds ..B1.12
        movq      8(%rbx), %rbx                                 #5297.29
        movl      4(%rbx), %eax                                 #5297.29
        cmpl      %ecx, %eax                                    #5298.34
        ja        ..B1.107      # Prob 12%                      #5298.34
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx
..B1.14:                        # Preds ..B1.13
        movl      %eax, %ecx                                    #5297.17
        testq     %rbx, %rbx                                    #5218.18
        jne       ..B1.4        # Prob 94%                      #5218.18
        jmp       ..B1.107      # Prob 100%                     #5218.18
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 edx ecx
..B1.15:                        # Preds ..B1.12
        cmpl      $15, %eax                                     #5286.22
        jne       ..B1.107      # Prob 50%                      #5286.22
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 edx ecx
..B1.16:                        # Preds ..B1.15
        movq      8(%rbx), %rbx                                 #5302.29
        movl      4(%rbx), %eax                                 #5302.29
        cmpl      %ecx, %eax                                    #5303.34
        ja        ..B1.107      # Prob 12%                      #5303.34
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.17:                        # Preds ..B1.16
        addl      %ecx, %edx                                    #5306.27
        subl      %eax, %edx                                    #5306.33
        movl      %eax, %ecx                                    #5302.17
        testq     %rbx, %rbx                                    #5218.18
        jne       ..B1.4        # Prob 94%                      #5218.18
        jmp       ..B1.107      # Prob 100%                     #5218.18
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 edx ecx
..B1.20:                        # Preds ..B1.10 ..B1.11
        movq      8(%rbx), %rdi                                 #5290.25
        call      NucStrstrSearch@PLT                           #5290.25
                                # LOE rbx rbp r12 r13 r14 r15 eax
..B1.21:                        # Preds ..B1.20
        movl      4(%rbx), %edx                                 #5291.22
        testl     %edx, %edx                                    #5291.42
        je        ..B1.56       # Prob 28%                      #5291.42
                                # LOE rbp r12 r13 r14 r15 eax
..B1.22:                        # Preds ..B1.21
        testl     %eax, %eax                                    #5293.31
        jne       ..B1.107      # Prob 50%                      #5293.31
                                # LOE rbp r12 r13 r14 r15
..B1.23:                        # Preds ..B1.22
        movl      $1, %eax                                      #5294.28
        addq      $32, %rsp                                     #5294.28
..___tag_value_NucStrstrSearch.18:                              #
        popq      %rbx                                          #5294.28
..___tag_value_NucStrstrSearch.20:                              #
        ret                                                     #5294.28
..___tag_value_NucStrstrSearch.21:                              #
                                # LOE
..1.30_0.TAG.0b.0.1:
..B1.26:                        # Preds ..B1.8
        movq      8(%rbx), %rdi                                 #5272.21
        movq      %r12, %rsi                                    #5272.21
        movl      %r13d, %edx                                   #5272.21
        movl      %r14d, %ecx                                   #5272.21
        movq      %r15, %r8                                     #5272.21
        call      NucStrstrSearch@PLT                           #5272.21
                                # LOE rbx rbp r12 r15 eax r13d r14d
..B1.27:                        # Preds ..B1.26
        movl      4(%rbx), %edi                                 #5273.22
        cmpl      $17, %edi                                     #5273.22
        jne       ..B1.29       # Prob 67%                      #5273.22
                                # LOE rbx rbp r12 r15 eax edi r13d r14d
..B1.28:                        # Preds ..B1.27
        testl     %eax, %eax                                    #5276.31
        jne       ..B1.111      # Prob 12%                      #5276.31
        jmp       ..B1.31       # Prob 100%                     #5276.31
                                # LOE rbx rbp r12 r15 eax r13d r14d
..B1.29:                        # Preds ..B1.27
        cmpl      $16, %edi                                     #5273.22
        jne       ..B1.31       # Prob 50%                      #5273.22
                                # LOE rbx rbp r12 r15 eax r13d r14d
..B1.30:                        # Preds ..B1.29
        testl     %eax, %eax                                    #5280.31
        je        ..B1.111      # Prob 12%                      #5280.31
                                # LOE rbx rbp r12 r15 eax r13d r14d
..B1.31:                        # Preds ..B1.28 ..B1.30 ..B1.29
        movq      16(%rbx), %rbx                                #5284.38
        testq     %rbx, %rbx                                    #5218.18
        jne       ..B1.7        # Prob 94%                      #5218.18
                                # LOE rbx rbp r12 r15 r13d r14d
..B1.32:                        # Preds ..B1.7 ..B1.31
        movq      24(%rsp), %r12                                #
..___tag_value_NucStrstrSearch.23:                              #
        movq      16(%rsp), %r13                                #
..___tag_value_NucStrstrSearch.24:                              #
        movq      8(%rsp), %r14                                 #
..___tag_value_NucStrstrSearch.25:                              #
        movq      (%rsp), %r15                                  #
..___tag_value_NucStrstrSearch.26:                              #
        jmp       ..B1.107      # Prob 100%                     #
..___tag_value_NucStrstrSearch.27:                              #
                                # LOE rbp r12 r13 r14 r15
..1.30_0.TAG.0a.0.1:
..B1.33:                        # Preds ..B1.8
        movq      %r12, %rsi                                    #
        movq      24(%rsp), %r12                                #
..___tag_value_NucStrstrSearch.31:                              #
        movl      %r13d, %edx                                   #
        movq      16(%rsp), %r13                                #
..___tag_value_NucStrstrSearch.32:                              #
        movl      %r14d, %ecx                                   #
        movq      %r15, %r8                                     #
        movq      8(%rsp), %r14                                 #
..___tag_value_NucStrstrSearch.33:                              #
        movq      (%rsp), %r15                                  #
..___tag_value_NucStrstrSearch.34:                              #
..___tag_value_NucStrstrSearch.35:                              #
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 edx ecx
..B1.34:                        # Preds ..B1.33
        movl      4(%rbx), %eax                                 #5268.24
        cmpl      %eax, %ecx                                    #5268.24
        jb        ..B1.107      # Prob 28%                      #5268.24
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.35:                        # Preds ..B1.34
        testq     %r8, %r8                                      #5269.9
        je        ..B1.37       # Prob 12%                      #5269.9
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.36:                        # Preds ..B1.35
        movl      %eax, (%r8)                                   #5269.18
                                # LOE rbx rbp rsi r12 r13 r14 r15 edx ecx
..B1.37:                        # Preds ..B1.35 ..B1.36
        movq      %rbx, %rdi                                    #5270.20
        call      eval_4na_pos@PLT                              #5270.20
                                # LOE rbp r12 r13 r14 r15 eax
..B1.119:                       # Preds ..B1.37
        addq      $32, %rsp                                     #5270.20
..___tag_value_NucStrstrSearch.39:                              #
        popq      %rbx                                          #5270.20
..___tag_value_NucStrstrSearch.41:                              #
        ret                                                     #5270.20
..___tag_value_NucStrstrSearch.42:                              #
                                # LOE
..1.30_0.TAG.09.0.1:
..B1.39:                        # Preds ..B1.8
        movq      %r12, %rsi                                    #
        movq      24(%rsp), %r12                                #
..___tag_value_NucStrstrSearch.44:                              #
        movl      %r13d, %edx                                   #
        movq      16(%rsp), %r13                                #
..___tag_value_NucStrstrSearch.45:                              #
        movl      %r14d, %ecx                                   #
        movq      %r15, %r8                                     #
        movq      8(%rsp), %r14                                 #
..___tag_value_NucStrstrSearch.46:                              #
        movq      (%rsp), %r15                                  #
..___tag_value_NucStrstrSearch.47:                              #
..___tag_value_NucStrstrSearch.48:                              #
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 edx ecx
..B1.40:                        # Preds ..B1.39
        movl      4(%rbx), %eax                                 #5264.24
        cmpl      %eax, %ecx                                    #5264.24
        jb        ..B1.107      # Prob 28%                      #5264.24
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.41:                        # Preds ..B1.40
        testq     %r8, %r8                                      #5265.9
        je        ..B1.43       # Prob 12%                      #5265.9
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.42:                        # Preds ..B1.41
        movl      %eax, (%r8)                                   #5265.18
                                # LOE rbx rbp rsi r12 r13 r14 r15 edx ecx
..B1.43:                        # Preds ..B1.41 ..B1.42
        movq      %rbx, %rdi                                    #5266.20
        call      eval_2na_pos@PLT                              #5266.20
                                # LOE rbp r12 r13 r14 r15 eax
..B1.120:                       # Preds ..B1.43
        addq      $32, %rsp                                     #5266.20
..___tag_value_NucStrstrSearch.52:                              #
        popq      %rbx                                          #5266.20
..___tag_value_NucStrstrSearch.54:                              #
        ret                                                     #5266.20
..___tag_value_NucStrstrSearch.55:                              #
                                # LOE
..1.30_0.TAG.08.0.1:
..B1.45:                        # Preds ..B1.8
        movq      %r12, %rsi                                    #
        movq      24(%rsp), %r12                                #
..___tag_value_NucStrstrSearch.57:                              #
        movl      %r13d, %edx                                   #
        movq      16(%rsp), %r13                                #
..___tag_value_NucStrstrSearch.58:                              #
        movl      %r14d, %ecx                                   #
        movq      %r15, %r8                                     #
        movq      8(%rsp), %r14                                 #
..___tag_value_NucStrstrSearch.59:                              #
        movq      (%rsp), %r15                                  #
..___tag_value_NucStrstrSearch.60:                              #
..___tag_value_NucStrstrSearch.61:                              #
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 edx ecx
..B1.46:                        # Preds ..B1.45
        movl      4(%rbx), %eax                                 #5259.24
        cmpl      %eax, %ecx                                    #5259.24
        jb        ..B1.107      # Prob 28%                      #5259.24
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.47:                        # Preds ..B1.46
        testq     %r8, %r8                                      #5260.9
        je        ..B1.49       # Prob 12%                      #5260.9
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.48:                        # Preds ..B1.47
        movl      %eax, (%r8)                                   #5260.18
                                # LOE rbx rbp rsi r12 r13 r14 r15 edx ecx
..B1.49:                        # Preds ..B1.47 ..B1.48
        movq      %rbx, %rdi                                    #5261.20
        call      eval_4na_128@PLT                              #5261.20
                                # LOE rbp r12 r13 r14 r15 eax
..B1.121:                       # Preds ..B1.49
        addq      $32, %rsp                                     #5261.20
..___tag_value_NucStrstrSearch.65:                              #
        popq      %rbx                                          #5261.20
..___tag_value_NucStrstrSearch.67:                              #
        ret                                                     #5261.20
..___tag_value_NucStrstrSearch.68:                              #
                                # LOE
..1.30_0.TAG.07.0.1:
..B1.51:                        # Preds ..B1.8
        movq      %r12, %rsi                                    #
        movq      24(%rsp), %r12                                #
..___tag_value_NucStrstrSearch.70:                              #
        movl      %r13d, %edx                                   #
        movq      16(%rsp), %r13                                #
..___tag_value_NucStrstrSearch.71:                              #
        movl      %r14d, %ecx                                   #
        movq      %r15, %r8                                     #
        movq      8(%rsp), %r14                                 #
..___tag_value_NucStrstrSearch.72:                              #
        movq      (%rsp), %r15                                  #
..___tag_value_NucStrstrSearch.73:                              #
..___tag_value_NucStrstrSearch.74:                              #
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 edx ecx
..B1.52:                        # Preds ..B1.51
        movl      4(%rbx), %eax                                 #5255.24
        cmpl      %eax, %ecx                                    #5255.24
        jb        ..B1.107      # Prob 28%                      #5255.24
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.53:                        # Preds ..B1.52
        testq     %r8, %r8                                      #5256.9
        je        ..B1.55       # Prob 12%                      #5256.9
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.54:                        # Preds ..B1.53
        movl      %eax, (%r8)                                   #5256.18
                                # LOE rbx rbp rsi r12 r13 r14 r15 edx ecx
..B1.55:                        # Preds ..B1.53 ..B1.54
        movq      %rbx, %rdi                                    #5257.20
        call      eval_4na_32@PLT                               #5257.20
                                # LOE rbp r12 r13 r14 r15 eax
..B1.56:                        # Preds ..B1.21 ..B1.55
        addq      $32, %rsp                                     #5257.20
..___tag_value_NucStrstrSearch.78:                              #
        popq      %rbx                                          #5257.20
..___tag_value_NucStrstrSearch.80:                              #
        ret                                                     #5257.20
..___tag_value_NucStrstrSearch.81:                              #
                                # LOE
..1.30_0.TAG.06.0.1:
..B1.58:                        # Preds ..B1.8
        movq      %r12, %rsi                                    #
        movq      24(%rsp), %r12                                #
..___tag_value_NucStrstrSearch.83:                              #
        movl      %r13d, %edx                                   #
        movq      16(%rsp), %r13                                #
..___tag_value_NucStrstrSearch.84:                              #
        movl      %r14d, %ecx                                   #
        movq      %r15, %r8                                     #
        movq      8(%rsp), %r14                                 #
..___tag_value_NucStrstrSearch.85:                              #
        movq      (%rsp), %r15                                  #
..___tag_value_NucStrstrSearch.86:                              #
..___tag_value_NucStrstrSearch.87:                              #
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 edx ecx
..B1.59:                        # Preds ..B1.58
        movl      4(%rbx), %eax                                 #5251.24
        cmpl      %eax, %ecx                                    #5251.24
        jb        ..B1.107      # Prob 28%                      #5251.24
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.60:                        # Preds ..B1.59
        testq     %r8, %r8                                      #5252.9
        je        ..B1.62       # Prob 12%                      #5252.9
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.61:                        # Preds ..B1.60
        movl      %eax, (%r8)                                   #5252.18
                                # LOE rbx rbp rsi r12 r13 r14 r15 edx ecx
..B1.62:                        # Preds ..B1.60 ..B1.61
        movq      %rbx, %rdi                                    #5253.20
        call      eval_4na_16@PLT                               #5253.20
                                # LOE rbp r12 r13 r14 r15 eax
..B1.63:                        # Preds ..B1.62
        addq      $32, %rsp                                     #5253.20
..___tag_value_NucStrstrSearch.91:                              #
        popq      %rbx                                          #5253.20
..___tag_value_NucStrstrSearch.93:                              #
        ret                                                     #5253.20
..___tag_value_NucStrstrSearch.94:                              #
                                # LOE
..1.30_0.TAG.05.0.1:
..B1.65:                        # Preds ..B1.8
        movq      %r12, %rsi                                    #
        movq      24(%rsp), %r12                                #
..___tag_value_NucStrstrSearch.96:                              #
        movl      %r13d, %edx                                   #
        movq      16(%rsp), %r13                                #
..___tag_value_NucStrstrSearch.97:                              #
        movl      %r14d, %ecx                                   #
        movq      %r15, %r8                                     #
        movq      8(%rsp), %r14                                 #
..___tag_value_NucStrstrSearch.98:                              #
        movq      (%rsp), %r15                                  #
..___tag_value_NucStrstrSearch.99:                              #
..___tag_value_NucStrstrSearch.100:                             #
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 edx ecx
..B1.66:                        # Preds ..B1.65
        movl      4(%rbx), %eax                                 #5247.24
        cmpl      %eax, %ecx                                    #5247.24
        jb        ..B1.107      # Prob 28%                      #5247.24
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.67:                        # Preds ..B1.66
        testq     %r8, %r8                                      #5248.9
        je        ..B1.69       # Prob 12%                      #5248.9
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.68:                        # Preds ..B1.67
        movl      %eax, (%r8)                                   #5248.18
                                # LOE rbx rbp rsi r12 r13 r14 r15 edx ecx
..B1.69:                        # Preds ..B1.67 ..B1.68
        movq      %rbx, %rdi                                    #5249.20
        call      eval_2na_128@PLT                              #5249.20
                                # LOE rbp r12 r13 r14 r15 eax
..B1.124:                       # Preds ..B1.69
        addq      $32, %rsp                                     #5249.20
..___tag_value_NucStrstrSearch.104:                             #
        popq      %rbx                                          #5249.20
..___tag_value_NucStrstrSearch.106:                             #
        ret                                                     #5249.20
..___tag_value_NucStrstrSearch.107:                             #
                                # LOE
..1.30_0.TAG.04.0.1:
..B1.71:                        # Preds ..B1.8
        movq      %r12, %rsi                                    #
        movq      24(%rsp), %r12                                #
..___tag_value_NucStrstrSearch.109:                             #
        movl      %r13d, %edx                                   #
        movq      16(%rsp), %r13                                #
..___tag_value_NucStrstrSearch.110:                             #
        movl      %r14d, %ecx                                   #
        movq      %r15, %r8                                     #
        movq      8(%rsp), %r14                                 #
..___tag_value_NucStrstrSearch.111:                             #
        movq      (%rsp), %r15                                  #
..___tag_value_NucStrstrSearch.112:                             #
..___tag_value_NucStrstrSearch.113:                             #
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 edx ecx
..B1.72:                        # Preds ..B1.71
        movl      4(%rbx), %eax                                 #5243.24
        cmpl      %eax, %ecx                                    #5243.24
        jb        ..B1.107      # Prob 28%                      #5243.24
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.73:                        # Preds ..B1.72
        testq     %r8, %r8                                      #5244.9
        je        ..B1.75       # Prob 12%                      #5244.9
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.74:                        # Preds ..B1.73
        movl      %eax, (%r8)                                   #5244.18
                                # LOE rbx rbp rsi r12 r13 r14 r15 edx ecx
..B1.75:                        # Preds ..B1.73 ..B1.74
        movq      %rbx, %rdi                                    #5245.20
        call      eval_2na_32@PLT                               #5245.20
                                # LOE rbp r12 r13 r14 r15 eax
..B1.76:                        # Preds ..B1.75
        addq      $32, %rsp                                     #5245.20
..___tag_value_NucStrstrSearch.117:                             #
        popq      %rbx                                          #5245.20
..___tag_value_NucStrstrSearch.119:                             #
        ret                                                     #5245.20
..___tag_value_NucStrstrSearch.120:                             #
                                # LOE
..1.30_0.TAG.03.0.1:
..B1.78:                        # Preds ..B1.8
        movq      %r12, %rsi                                    #
        movq      24(%rsp), %r12                                #
..___tag_value_NucStrstrSearch.122:                             #
        movl      %r13d, %edx                                   #
        movq      16(%rsp), %r13                                #
..___tag_value_NucStrstrSearch.123:                             #
        movl      %r14d, %ecx                                   #
        movq      %r15, %r8                                     #
        movq      8(%rsp), %r14                                 #
..___tag_value_NucStrstrSearch.124:                             #
        movq      (%rsp), %r15                                  #
..___tag_value_NucStrstrSearch.125:                             #
..___tag_value_NucStrstrSearch.126:                             #
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 edx ecx
..B1.79:                        # Preds ..B1.78
        movl      4(%rbx), %eax                                 #5239.24
        cmpl      %eax, %ecx                                    #5239.24
        jb        ..B1.107      # Prob 28%                      #5239.24
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.80:                        # Preds ..B1.79
        testq     %r8, %r8                                      #5240.9
        je        ..B1.82       # Prob 12%                      #5240.9
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.81:                        # Preds ..B1.80
        movl      %eax, (%r8)                                   #5240.18
                                # LOE rbx rbp rsi r12 r13 r14 r15 edx ecx
..B1.82:                        # Preds ..B1.80 ..B1.81
        movq      %rbx, %rdi                                    #5241.20
        call      eval_2na_16@PLT                               #5241.20
                                # LOE rbp r12 r13 r14 r15 eax
..B1.83:                        # Preds ..B1.82
        addq      $32, %rsp                                     #5241.20
..___tag_value_NucStrstrSearch.130:                             #
        popq      %rbx                                          #5241.20
..___tag_value_NucStrstrSearch.132:                             #
        ret                                                     #5241.20
..___tag_value_NucStrstrSearch.133:                             #
                                # LOE
..1.30_0.TAG.02.0.1:
..B1.85:                        # Preds ..B1.8
        movq      %r12, %rsi                                    #
        movq      24(%rsp), %r12                                #
..___tag_value_NucStrstrSearch.135:                             #
        movl      %r13d, %edx                                   #
        movq      16(%rsp), %r13                                #
..___tag_value_NucStrstrSearch.136:                             #
        movl      %r14d, %ecx                                   #
        movq      %r15, %r8                                     #
        movq      8(%rsp), %r14                                 #
..___tag_value_NucStrstrSearch.137:                             #
        movq      (%rsp), %r15                                  #
..___tag_value_NucStrstrSearch.138:                             #
..___tag_value_NucStrstrSearch.139:                             #
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 edx ecx
..B1.86:                        # Preds ..B1.85
        movl      4(%rbx), %eax                                 #5235.24
        cmpl      %eax, %ecx                                    #5235.24
        jb        ..B1.107      # Prob 28%                      #5235.24
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.87:                        # Preds ..B1.86
        testq     %r8, %r8                                      #5236.9
        je        ..B1.89       # Prob 12%                      #5236.9
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.88:                        # Preds ..B1.87
        movl      %eax, (%r8)                                   #5236.18
                                # LOE rbx rbp rsi r12 r13 r14 r15 edx ecx
..B1.89:                        # Preds ..B1.87 ..B1.88
        movq      %rbx, %rdi                                    #5237.20
        call      eval_2na_8@PLT                                #5237.20
                                # LOE rbp r12 r13 r14 r15 eax
..B1.90:                        # Preds ..B1.89
        addq      $32, %rsp                                     #5237.20
..___tag_value_NucStrstrSearch.143:                             #
        popq      %rbx                                          #5237.20
..___tag_value_NucStrstrSearch.145:                             #
        ret                                                     #5237.20
..___tag_value_NucStrstrSearch.146:                             #
                                # LOE
..1.30_0.TAG.01.0.1:
..B1.92:                        # Preds ..B1.8
        movq      %r12, %rsi                                    #
        movq      24(%rsp), %r12                                #
..___tag_value_NucStrstrSearch.148:                             #
        movl      %r13d, %edx                                   #
        movq      16(%rsp), %r13                                #
..___tag_value_NucStrstrSearch.149:                             #
        movl      %r14d, %ecx                                   #
        movq      %r15, %r8                                     #
        movq      8(%rsp), %r14                                 #
..___tag_value_NucStrstrSearch.150:                             #
        movq      (%rsp), %r15                                  #
..___tag_value_NucStrstrSearch.151:                             #
..___tag_value_NucStrstrSearch.152:                             #
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 edx ecx
..B1.93:                        # Preds ..B1.92
        movl      4(%rbx), %eax                                 #5230.24
        cmpl      %eax, %ecx                                    #5230.24
        jb        ..B1.107      # Prob 28%                      #5230.24
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.94:                        # Preds ..B1.93
        testq     %r8, %r8                                      #5231.9
        je        ..B1.96       # Prob 12%                      #5231.9
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.95:                        # Preds ..B1.94
        movl      %eax, (%r8)                                   #5231.18
                                # LOE rbx rbp rsi r12 r13 r14 r15 edx ecx
..B1.96:                        # Preds ..B1.94 ..B1.95
        movq      %rbx, %rdi                                    #5232.20
        call      eval_4na_64@PLT                               #5232.20
                                # LOE rbp r12 r13 r14 r15 eax
..B1.97:                        # Preds ..B1.96
        addq      $32, %rsp                                     #5232.20
..___tag_value_NucStrstrSearch.156:                             #
        popq      %rbx                                          #5232.20
..___tag_value_NucStrstrSearch.158:                             #
        ret                                                     #5232.20
..___tag_value_NucStrstrSearch.159:                             #
                                # LOE
..1.30_0.TAG.00.0.1:
..B1.99:                        # Preds ..B1.8
        movq      %r12, %rsi                                    #
        movq      24(%rsp), %r12                                #
..___tag_value_NucStrstrSearch.161:                             #
        movl      %r13d, %edx                                   #
        movq      16(%rsp), %r13                                #
..___tag_value_NucStrstrSearch.162:                             #
        movl      %r14d, %ecx                                   #
        movq      %r15, %r8                                     #
        movq      8(%rsp), %r14                                 #
..___tag_value_NucStrstrSearch.163:                             #
        movq      (%rsp), %r15                                  #
..___tag_value_NucStrstrSearch.164:                             #
..___tag_value_NucStrstrSearch.165:                             #
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 edx ecx
..B1.100:                       # Preds ..B1.99
        movl      4(%rbx), %eax                                 #5226.24
        cmpl      %eax, %ecx                                    #5226.24
        jb        ..B1.107      # Prob 28%                      #5226.24
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.101:                       # Preds ..B1.100
        testq     %r8, %r8                                      #5227.9
        je        ..B1.103      # Prob 12%                      #5227.9
                                # LOE rbx rbp rsi r8 r12 r13 r14 r15 eax edx ecx
..B1.102:                       # Preds ..B1.101
        movl      %eax, (%r8)                                   #5227.18
                                # LOE rbx rbp rsi r12 r13 r14 r15 edx ecx
..B1.103:                       # Preds ..B1.101 ..B1.102
        movq      %rbx, %rdi                                    #5228.20
        call      eval_2na_64@PLT                               #5228.20
                                # LOE rbp r12 r13 r14 r15 eax
..B1.104:                       # Preds ..B1.103
        addq      $32, %rsp                                     #5228.20
..___tag_value_NucStrstrSearch.169:                             #
        popq      %rbx                                          #5228.20
..___tag_value_NucStrstrSearch.171:                             #
        ret                                                     #5228.20
..___tag_value_NucStrstrSearch.172:                             #
                                # LOE
..B1.107:                       # Preds ..B1.13 ..B1.16 ..B1.17 ..B1.14 ..B1.15
                                #       ..B1.4 ..B1.100 ..B1.93 ..B1.86 ..B1.79
                                #       ..B1.72 ..B1.66 ..B1.59 ..B1.52 ..B1.46
                                #       ..B1.40 ..B1.34 ..B1.32 ..B1.22 ..B1.2
                                #       ..B1.1
        xorl      %eax, %eax                                    #5311.12
        addq      $32, %rsp                                     #5311.12
..___tag_value_NucStrstrSearch.178:                             #
        popq      %rbx                                          #5311.12
..___tag_value_NucStrstrSearch.180:                             #
        ret                                                     #5311.12
..___tag_value_NucStrstrSearch.181:                             #
                                # LOE
..B1.111:                       # Preds ..B1.28 ..B1.30         # Infreq
        movq      24(%rsp), %r12                                #
..___tag_value_NucStrstrSearch.187:                             #
        movq      16(%rsp), %r13                                #
..___tag_value_NucStrstrSearch.188:                             #
        movq      8(%rsp), %r14                                 #
..___tag_value_NucStrstrSearch.189:                             #
        movq      (%rsp), %r15                                  #
..___tag_value_NucStrstrSearch.190:                             #
        addq      $32, %rsp                                     #5281.28
..___tag_value_NucStrstrSearch.191:                             #
        popq      %rbx                                          #5281.28
..___tag_value_NucStrstrSearch.193:                             #
        ret                                                     #5281.28
        .align    16,0x90
..___tag_value_NucStrstrSearch.194:                             #
                                # LOE
# mark_end;
	.type	NucStrstrSearch,@function
	.size	NucStrstrSearch,.-NucStrstrSearch
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
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %edx
# parameter 4: %ecx
..B2.1:                         # Preds ..B2.0
..___tag_value_eval_4na_pos.195:                                #4946.1
        pushq     %r12                                          #4946.1
..___tag_value_eval_4na_pos.197:                                #
        pushq     %rbp                                          #4946.1
..___tag_value_eval_4na_pos.199:                                #
        pushq     %rbx                                          #4946.1
..___tag_value_eval_4na_pos.201:                                #
        movdqa    16(%rdi), %xmm8                               #5009.5
        movdqa    32(%rdi), %xmm7                               #5009.5
        movdqa    48(%rdi), %xmm6                               #5009.5
        movdqa    64(%rdi), %xmm5                               #5009.5
        movdqa    80(%rdi), %xmm4                               #5009.5
        movdqa    96(%rdi), %xmm3                               #5009.5
        movdqa    112(%rdi), %xmm2                              #5009.5
        movdqa    128(%rdi), %xmm1                              #5009.5
        lea       (%rdx,%rcx), %ebx                             #4985.5
        subl      4(%rdi), %ebx                                 #4996.12
        movl      %edx, %r9d                                    #4946.1
        movl      %edx, %ebp                                    #4993.49
        shrl      $2, %ebp                                      #4993.49
        movzbl    (%rbp,%rsi), %r10d                            #5002.14
        lea       3(%rdx,%rcx), %r8d                            #4999.50
        shrl      $2, %r8d                                      #4999.57
        addq      %rsi, %r8                                     #4999.30
        lea       expand_2na.0(%rip), %rcx                      #5002.14
        movzwl    (%rcx,%r10,2), %r11d                          #5002.14
        movzbl    1(%rbp,%rsi), %r10d                           #5002.14
        movzwl    (%rcx,%r10,2), %r10d                          #5002.14
        movw      %r11w, -16(%rsp)                              #5002.14
        movl      %ebx, %edi                                    #5014.20
        xorl      %r11d, %r11d                                  #5012.15
        movw      %r10w, -14(%rsp)                              #5002.14
        movzbl    2(%rbp,%rsi), %r10d                           #5002.14
        movzwl    (%rcx,%r10,2), %r10d                          #5002.14
        xorl      %eax, %eax                                    #5012.10
        subl      %edx, %edi                                    #5014.20
        movw      %r10w, -12(%rsp)                              #5002.14
        movzbl    3(%rbp,%rsi), %r10d                           #5002.14
        movzwl    (%rcx,%r10,2), %r10d                          #5002.14
        movw      %r10w, -10(%rsp)                              #5002.14
        movzbl    4(%rbp,%rsi), %r10d                           #5002.14
        movzwl    (%rcx,%r10,2), %r10d                          #5002.14
        movw      %r10w, -8(%rsp)                               #5002.14
        movzbl    5(%rbp,%rsi), %r10d                           #5002.14
        movzwl    (%rcx,%r10,2), %r10d                          #5002.14
        addl      $7, %edi                                      #5014.33
        shrl      $2, %edi                                      #5014.40
        movw      %r10w, -6(%rsp)                               #5002.14
        movzbl    6(%rbp,%rsi), %r10d                           #5002.14
        movzwl    (%rcx,%r10,2), %r10d                          #5002.14
        movw      %r10w, -4(%rsp)                               #5002.14
        movzbl    7(%rbp,%rsi), %r10d                           #5002.14
        movzwl    (%rcx,%r10,2), %ecx                           #5002.14
        movw      %cx, -2(%rsp)                                 #5002.14
        movdqu    -16(%rsp), %xmm0                              #5002.14
        lea       8(%rsi,%rbp), %rbp                            #5003.5
        movq      %rbp, %rcx                                    #5003.5
        xorl      %r10d, %r10d                                  #5012.5
        movl      %edx, %esi                                    #5023.20
        andl      $3, %esi                                      #5023.20
        je        ..B2.20       # Prob 20%                      #5023.20
                                # LOE rcx rbp r8 r12 r13 r14 r15 eax edx ebx esi edi r9d r10d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.2:                         # Preds ..B2.1
        cmpl      $1, %esi                                      #5023.20
        je        ..B2.9        # Prob 25%                      #5023.20
                                # LOE rcx rbp r8 r12 r13 r14 r15 eax edx ebx esi edi r9d r10d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.3:                         # Preds ..B2.2
        cmpl      $2, %esi                                      #5023.20
        je        ..B2.10       # Prob 33%                      #5023.20
                                # LOE rcx rbp r8 r12 r13 r14 r15 eax edx ebx esi edi r9d r10d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.4:                         # Preds ..B2.3
        cmpl      $3, %esi                                      #5023.20
        je        ..B2.11       # Prob 50%                      #5023.20
        jmp       ..B2.16       # Prob 100%                     #5023.20
                                # LOE rcx rbp r8 r12 r13 r14 r15 eax edx ebx edi r9d r10d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.9:                         # Preds ..B2.2 ..B2.20
        movdqa    %xmm0, %xmm10                                 #5047.22
        movdqa    %xmm0, %xmm9                                  #5048.22
        pand      %xmm6, %xmm10                                 #5047.22
        pand      %xmm5, %xmm9                                  #5048.22
        pcmpeqd   %xmm9, %xmm10                                 #5049.22
        pmovmskb  %xmm10, %eax                                  #5050.22
        incl      %eax                                          #5051.17
        shrl      $16, %eax                                     #5051.17
        negl      %eax                                          #5051.17
                                # LOE rcx rbp r8 r13 r14 r15 eax edx ebx edi r9d r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.10:                        # Preds ..B2.3 ..B2.9
        movdqa    %xmm0, %xmm10                                 #5054.22
        movdqa    %xmm0, %xmm9                                  #5055.22
        pand      %xmm4, %xmm10                                 #5054.22
        pand      %xmm3, %xmm9                                  #5055.22
        pcmpeqd   %xmm9, %xmm10                                 #5056.22
        pmovmskb  %xmm10, %r11d                                 #5057.22
        incl      %r11d                                         #5058.17
        shrl      $16, %r11d                                    #5058.17
        negl      %r11d                                         #5058.17
                                # LOE rcx rbp r8 r13 r14 r15 eax edx ebx edi r9d r10d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.11:                        # Preds ..B2.4 ..B2.10
        movdqa    %xmm0, %xmm10                                 #5061.22
        movdqa    %xmm0, %xmm9                                  #5062.22
        andl      $-4, %edx                                     #5069.17
        pand      %xmm2, %xmm10                                 #5061.22
        pand      %xmm1, %xmm9                                  #5062.22
        pcmpeqd   %xmm9, %xmm10                                 #5063.22
        pmovmskb  %xmm10, %esi                                  #5064.22
        movl      %r10d, %r12d                                  #5072.29
        orl       %eax, %r12d                                   #5072.29
        incl      %esi                                          #5065.17
        shrl      $16, %esi                                     #5065.17
        negl      %esi                                          #5065.17
        orl       %r11d, %r12d                                  #5072.34
        orl       %esi, %r12d                                   #5072.39
        jne       ..B2.21       # Prob 20%                      #5072.47
                                # LOE rcx rbp r8 r13 r14 r15 eax edx ebx esi edi r9d r10d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.12:                        # Preds ..B2.11
        addl      $4, %edx                                      #5135.17
        cmpl      %ebx, %edx                                    #5138.28
        ja        ..B2.44       # Prob 20%                      #5138.28
                                # LOE rcx rbp r8 r13 r14 r15 edx ebx edi r9d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.13:                        # Preds ..B2.12
        decl      %edi                                          #5142.25
        jne       ..B2.17       # Prob 50%                      #5142.39
                                # LOE rcx rbp r8 r13 r14 r15 edx ebx edi r9d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.14:                        # Preds ..B2.13
        cmpq      %r8, %rbp                                     #5159.25
        jae       ..B2.44       # Prob 4%                       #5159.25
                                # LOE rbp r8 r13 r14 r15 edx ebx r9d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.15:                        # Preds ..B2.14
        movzbl    (%rbp), %esi                                  #5169.22
        movzbl    1(%rbp), %r10d                                #5169.22
        movzbl    2(%rbp), %r12d                                #5169.22
        lea       expand_2na.0(%rip), %rcx                      #5169.22
        movzwl    (%rcx,%rsi,2), %edi                           #5169.22
        movzwl    (%rcx,%r10,2), %r11d                          #5169.22
        movzwl    (%rcx,%r12,2), %esi                           #5169.22
        movw      %di, -16(%rsp)                                #5169.22
        movw      %r11w, -14(%rsp)                              #5169.22
        movw      %si, -12(%rsp)                                #5169.22
        movzbl    3(%rbp), %esi                                 #5169.22
        movzwl    (%rcx,%rsi,2), %esi                           #5169.22
        movw      %si, -10(%rsp)                                #5169.22
        movzbl    4(%rbp), %esi                                 #5169.22
        movzwl    (%rcx,%rsi,2), %esi                           #5169.22
        movw      %si, -8(%rsp)                                 #5169.22
        movzbl    5(%rbp), %esi                                 #5169.22
        movzwl    (%rcx,%rsi,2), %esi                           #5169.22
        movw      %si, -6(%rsp)                                 #5169.22
        movzbl    6(%rbp), %esi                                 #5169.22
        movzwl    (%rcx,%rsi,2), %esi                           #5169.22
        movw      %si, -4(%rsp)                                 #5169.22
        movzbl    7(%rbp), %esi                                 #5169.22
        movzwl    (%rcx,%rsi,2), %ecx                           #5169.22
        addq      $8, %rbp                                      #5184.13
        movw      %cx, -2(%rsp)                                 #5169.22
        movdqu    -16(%rsp), %xmm0                              #5169.22
        movq      %rbp, %rcx                                    #5184.13
                                # LOE rcx rbp r8 r13 r14 r15 edx ebx r9d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.16:                        # Preds ..B2.4 ..B2.15
        movl      $8, %edi                                      #5030.13
        jmp       ..B2.20       # Prob 100%                     #5030.13
                                # LOE rcx rbp r8 r13 r14 r15 edx ebx edi r9d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.17:                        # Preds ..B2.13
        psrldq    $2, %xmm0                                     #5147.26
        cmpq      %r8, %rcx                                     #5150.26
        jae       ..B2.19       # Prob 19%                      #5150.26
                                # LOE rcx rbp r8 r13 r14 r15 edx ebx edi r9d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.18:                        # Preds ..B2.17
        movzbl    (%rcx), %r10d                                 #5151.72
        lea       expand_2na.0(%rip), %rsi                      #5151.57
        movzwl    (%rsi,%r10,2), %r11d                          #5151.57
        pinsrw    $7, %r11d, %xmm0                              #5151.30
                                # LOE rcx rbp r8 r13 r14 r15 edx ebx edi r9d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.19:                        # Preds ..B2.18 ..B2.17
        incq      %rcx                                          #5154.20
                                # LOE rcx rbp r8 r13 r14 r15 edx ebx edi r9d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.20:                        # Preds ..B2.1 ..B2.16 ..B2.19
        movdqa    %xmm0, %xmm10                                 #5040.22
        movdqa    %xmm0, %xmm9                                  #5041.22
        pand      %xmm8, %xmm10                                 #5040.22
        pand      %xmm7, %xmm9                                  #5041.22
        pcmpeqd   %xmm9, %xmm10                                 #5042.22
        pmovmskb  %xmm10, %r10d                                 #5043.22
        incl      %r10d                                         #5044.17
        shrl      $16, %r10d                                    #5044.17
        negl      %r10d                                         #5044.17
        jmp       ..B2.9        # Prob 100%                     #5044.17
                                # LOE rcx rbp r8 r13 r14 r15 edx ebx edi r9d r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B2.21:                        # Preds ..B2.11                 # Infreq
        subl      %edx, %ebx                                    #5075.30
        je        ..B2.42       # Prob 25%                      #5075.37
                                # LOE r12 r13 r14 r15 eax edx ebx esi r9d r10d r11d
..B2.22:                        # Preds ..B2.21                 # Infreq
        cmpl      $1, %ebx                                      #5075.37
        jne       ..B2.27       # Prob 67%                      #5075.37
                                # LOE r12 r13 r14 r15 eax edx ebx esi r9d r10d r11d
..B2.23:                        # Preds ..B2.22                 # Infreq
        testl     %r10d, %r10d                                  #5081.36
        jne       ..B2.43       # Prob 28%                      #5081.36
                                # LOE r12 r13 r14 r15 eax edx r9d
..B2.24:                        # Preds ..B2.23                 # Infreq
        testl     %eax, %eax                                    #5082.36
        je        ..B2.44       # Prob 50%                      #5082.36
                                # LOE r12 r13 r14 r15 edx r9d
..B2.25:                        # Preds ..B2.35 ..B2.29 ..B2.24 # Infreq
        subl      %r9d, %edx                                    #5082.47
        addl      $2, %edx                                      #5082.61
        movl      %edx, %eax                                    #5082.61
..___tag_value_eval_4na_pos.204:                                #5082.61
        popq      %rbx                                          #5082.61
..___tag_value_eval_4na_pos.206:                                #
        popq      %rbp                                          #5082.61
..___tag_value_eval_4na_pos.208:                                #
        popq      %r12                                          #5082.61
..___tag_value_eval_4na_pos.210:                                #
        ret                                                     #5082.61
..___tag_value_eval_4na_pos.211:                                #
                                # LOE
..B2.27:                        # Preds ..B2.22                 # Infreq
        cmpl      $2, %ebx                                      #5075.37
        jne       ..B2.34       # Prob 50%                      #5075.37
                                # LOE r12 r13 r14 r15 eax edx esi r9d r10d r11d
..B2.28:                        # Preds ..B2.27                 # Infreq
        testl     %r10d, %r10d                                  #5085.36
        jne       ..B2.43       # Prob 28%                      #5085.36
                                # LOE r12 r13 r14 r15 eax edx r9d r11d
..B2.29:                        # Preds ..B2.28                 # Infreq
        testl     %eax, %eax                                    #5086.36
        jne       ..B2.25       # Prob 28%                      #5086.36
                                # LOE r12 r13 r14 r15 edx r9d r11d
..B2.30:                        # Preds ..B2.29                 # Infreq
        testl     %r11d, %r11d                                  #5087.36
        je        ..B2.44       # Prob 50%                      #5087.36
                                # LOE r12 r13 r14 r15 edx r9d
..B2.31:                        # Preds ..B2.36 ..B2.30         # Infreq
        subl      %r9d, %edx                                    #5087.47
        addl      $3, %edx                                      #5087.61
        movl      %edx, %eax                                    #5087.61
..___tag_value_eval_4na_pos.215:                                #5087.61
        popq      %rbx                                          #5087.61
..___tag_value_eval_4na_pos.217:                                #
        popq      %rbp                                          #5087.61
..___tag_value_eval_4na_pos.219:                                #
        popq      %r12                                          #5087.61
..___tag_value_eval_4na_pos.221:                                #
        ret                                                     #5087.61
..___tag_value_eval_4na_pos.222:                                #
                                # LOE
..B2.34:                        # Preds ..B2.27                 # Infreq
        testl     %r10d, %r10d                                  #5090.36
        jne       ..B2.43       # Prob 28%                      #5090.36
                                # LOE r12 r13 r14 r15 eax edx esi r9d r11d
..B2.35:                        # Preds ..B2.34                 # Infreq
        testl     %eax, %eax                                    #5091.36
        jne       ..B2.25       # Prob 28%                      #5091.36
                                # LOE r12 r13 r14 r15 edx esi r9d r11d
..B2.36:                        # Preds ..B2.35                 # Infreq
        testl     %r11d, %r11d                                  #5092.36
        jne       ..B2.31       # Prob 28%                      #5092.36
                                # LOE r12 r13 r14 r15 edx esi r9d
..B2.37:                        # Preds ..B2.36                 # Infreq
        testl     %esi, %esi                                    #5093.36
        je        ..B2.44       # Prob 50%                      #5093.36
                                # LOE r12 r13 r14 r15 edx r9d
..B2.38:                        # Preds ..B2.37                 # Infreq
        subl      %r9d, %edx                                    #5093.47
        addl      $4, %edx                                      #5093.61
        movl      %edx, %eax                                    #5093.61
..___tag_value_eval_4na_pos.226:                                #5093.61
        popq      %rbx                                          #5093.61
..___tag_value_eval_4na_pos.228:                                #
        popq      %rbp                                          #5093.61
..___tag_value_eval_4na_pos.230:                                #
        popq      %r12                                          #5093.61
..___tag_value_eval_4na_pos.232:                                #
        ret                                                     #5093.61
..___tag_value_eval_4na_pos.233:                                #
                                # LOE
..B2.42:                        # Preds ..B2.21                 # Infreq
        testl     %r10d, %r10d                                  #5078.36
        je        ..B2.44       # Prob 50%                      #5078.36
                                # LOE r12 r13 r14 r15 edx r9d
..B2.43:                        # Preds ..B2.34 ..B2.28 ..B2.23 ..B2.42 # Infreq
        subl      %r9d, %edx                                    #5078.47
        incl      %edx                                          #5078.61
        movl      %edx, %eax                                    #5078.61
..___tag_value_eval_4na_pos.237:                                #5078.61
        popq      %rbx                                          #5078.61
..___tag_value_eval_4na_pos.239:                                #
        popq      %rbp                                          #5078.61
..___tag_value_eval_4na_pos.241:                                #
        popq      %r12                                          #5078.61
..___tag_value_eval_4na_pos.243:                                #
        ret                                                     #5078.61
..___tag_value_eval_4na_pos.244:                                #
                                # LOE
..B2.44:                        # Preds ..B2.14 ..B2.12 ..B2.37 ..B2.30 ..B2.24
                                #       ..B2.42                 # Infreq
        xorl      %eax, %eax                                    #5095.28
..___tag_value_eval_4na_pos.248:                                #5095.28
        popq      %rbx                                          #5095.28
..___tag_value_eval_4na_pos.250:                                #
        popq      %rbp                                          #5095.28
..___tag_value_eval_4na_pos.252:                                #
        popq      %r12                                          #5095.28
..___tag_value_eval_4na_pos.254:                                #
        ret                                                     #5095.28
        .align    16,0x90
..___tag_value_eval_4na_pos.255:                                #
                                # LOE
# mark_end;
	.type	eval_4na_pos,@function
	.size	eval_4na_pos,.-eval_4na_pos
	.data
# -- End  eval_4na_pos
	.text
# -- Begin  eval_2na_pos
# mark_begin;
       .align    16,0x90
eval_2na_pos:
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %edx
# parameter 4: %ecx
..B3.1:                         # Preds ..B3.0
..___tag_value_eval_2na_pos.256:                                #3662.1
        pushq     %r15                                          #3662.1
..___tag_value_eval_2na_pos.258:                                #
        pushq     %r12                                          #3662.1
..___tag_value_eval_2na_pos.260:                                #
        pushq     %rbp                                          #3662.1
..___tag_value_eval_2na_pos.262:                                #
        pushq     %rbx                                          #3662.1
..___tag_value_eval_2na_pos.264:                                #
        lea       (%rdx,%rcx), %ebx                             #3705.5
        subl      4(%rdi), %ebx                                 #3716.12
        movl      %edx, %r9d                                    #3662.1
        movl      %edx, %r10d                                   #3713.49
        shrl      $2, %r10d                                     #3713.49
        lea       (%rsi,%r10), %rbp                             #3713.30
        lea       3(%rdx,%rcx), %r8d                            #3719.50
        shrl      $2, %r8d                                      #3719.57
        addq      %rsi, %r8                                     #3719.30
        testq     $15, %rbp                                     #3722.14
        jne       ..B3.3        # Prob 50%                      #3722.14
                                # LOE rbp rsi rdi r8 r10 r12 r13 r14 edx ebx r9d
..B3.2:                         # Preds ..B3.1
        movdqa    (%rbp), %xmm0                                 #3722.14
        jmp       ..B3.4        # Prob 100%                     #3722.14
                                # LOE rsi rdi r8 r10 r12 r13 r14 edx ebx r9d xmm0
..B3.3:                         # Preds ..B3.1
        movdqu    (%rbp), %xmm0                                 #3722.14
                                # LOE rsi rdi r8 r10 r12 r13 r14 edx ebx r9d xmm0
..B3.4:                         # Preds ..B3.2 ..B3.3
        lea       16(%rsi,%r10), %rsi                           #3723.5
        movq      %rsi, %rbp                                    #3723.5
        cmpq      %r8, %rsi                                     #3728.16
        jae       ..B3.6        # Prob 12%                      #3728.16
                                # LOE rbp rsi rdi r8 r12 r13 r14 edx ecx ebx r9d xmm0
..B3.5:                         # Preds ..B3.4
        movzbl    -1(%rsi), %ecx                                #3729.24
        shll      $8, %ecx                                      #3729.38
                                # LOE rbp rsi rdi r8 r12 r13 r14 edx ecx ebx r9d xmm0
..B3.6:                         # Preds ..B3.5 ..B3.4
        movdqa    16(%rdi), %xmm8                               #3733.5
        movdqa    32(%rdi), %xmm7                               #3733.5
        movdqa    48(%rdi), %xmm6                               #3733.5
        movdqa    64(%rdi), %xmm5                               #3733.5
        movdqa    80(%rdi), %xmm4                               #3733.5
        movdqa    96(%rdi), %xmm3                               #3733.5
        movdqa    112(%rdi), %xmm2                              #3733.5
        movdqa    128(%rdi), %xmm1                              #3733.5
        xorl      %r15d, %r15d                                  #3736.15
        xorl      %eax, %eax                                    #3736.10
        xorl      %r11d, %r11d                                  #3736.5
        movl      %ebx, %r10d                                   #3738.20
        subl      %edx, %r10d                                   #3738.20
        addl      $7, %r10d                                     #3738.33
        shrl      $2, %r10d                                     #3738.40
        movl      %edx, %edi                                    #3745.20
        andl      $3, %edi                                      #3745.20
        je        ..B3.29       # Prob 20%                      #3745.20
                                # LOE rbp rsi r8 r12 r13 r14 eax edx ecx ebx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.7:                         # Preds ..B3.6
        cmpl      $1, %edi                                      #3745.20
        je        ..B3.14       # Prob 25%                      #3745.20
                                # LOE rbp rsi r8 r12 r13 r14 eax edx ecx ebx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.8:                         # Preds ..B3.7
        cmpl      $2, %edi                                      #3745.20
        je        ..B3.15       # Prob 33%                      #3745.20
                                # LOE rbp rsi r8 r12 r13 r14 eax edx ecx ebx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.9:                         # Preds ..B3.8
        cmpl      $3, %edi                                      #3745.20
        je        ..B3.16       # Prob 50%                      #3745.20
        jmp       ..B3.25       # Prob 100%                     #3745.20
                                # LOE rbp rsi r8 r12 r13 r14 eax edx ecx ebx r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.14:                        # Preds ..B3.7 ..B3.29
        movdqa    %xmm0, %xmm9                                  #3768.22
        pand      %xmm5, %xmm9                                  #3768.22
        pcmpeqd   %xmm6, %xmm9                                  #3769.22
        pmovmskb  %xmm9, %eax                                   #3770.22
        incl      %eax                                          #3771.17
        shrl      $16, %eax                                     #3771.17
        negl      %eax                                          #3771.17
                                # LOE rbp rsi r8 r13 r14 eax edx ecx ebx r9d r10d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.15:                        # Preds ..B3.8 ..B3.14
        movdqa    %xmm0, %xmm9                                  #3774.22
        pand      %xmm3, %xmm9                                  #3774.22
        pcmpeqd   %xmm4, %xmm9                                  #3775.22
        pmovmskb  %xmm9, %r15d                                  #3776.22
        incl      %r15d                                         #3777.17
        shrl      $16, %r15d                                    #3777.17
        negl      %r15d                                         #3777.17
                                # LOE rbp rsi r8 r13 r14 eax edx ecx ebx r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.16:                        # Preds ..B3.9 ..B3.15
        movdqa    %xmm0, %xmm9                                  #3780.22
        andl      $-4, %edx                                     #3787.17
        movl      %r11d, %r12d                                  #3790.29
        pand      %xmm1, %xmm9                                  #3780.22
        pcmpeqd   %xmm2, %xmm9                                  #3781.22
        pmovmskb  %xmm9, %edi                                   #3782.22
        orl       %eax, %r12d                                   #3790.29
        orl       %r15d, %r12d                                  #3790.34
        incl      %edi                                          #3783.17
        shrl      $16, %edi                                     #3783.17
        negl      %edi                                          #3783.17
        orl       %edi, %r12d                                   #3790.39
        jne       ..B3.30       # Prob 20%                      #3790.47
                                # LOE rbp rsi r8 r13 r14 eax edx ecx ebx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.17:                        # Preds ..B3.16
        addl      $4, %edx                                      #3853.17
        cmpl      %ebx, %edx                                    #3856.28
        ja        ..B3.53       # Prob 20%                      #3856.28
                                # LOE rbp rsi r8 r13 r14 edx ecx ebx r9d r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.18:                        # Preds ..B3.17
        decl      %r10d                                         #3860.25
        jne       ..B3.26       # Prob 50%                      #3860.39
                                # LOE rbp rsi r8 r13 r14 edx ecx ebx r9d r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.19:                        # Preds ..B3.18
        cmpq      %r8, %rsi                                     #3881.25
        jae       ..B3.53       # Prob 4%                       #3881.25
                                # LOE rsi r8 r13 r14 edx ecx ebx r9d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.20:                        # Preds ..B3.19
        testq     $15, %rsi                                     #3891.22
        jne       ..B3.22       # Prob 50%                      #3891.22
                                # LOE rsi r8 r13 r14 edx ecx ebx r9d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.21:                        # Preds ..B3.20
        movdqa    (%rsi), %xmm0                                 #3891.22
        jmp       ..B3.23       # Prob 100%                     #3891.22
                                # LOE rsi r8 r13 r14 edx ecx ebx r9d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.22:                        # Preds ..B3.20
        movdqu    (%rsi), %xmm0                                 #3891.22
                                # LOE rsi r8 r13 r14 edx ecx ebx r9d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.23:                        # Preds ..B3.21 ..B3.22
        addq      $16, %rsi                                     #3969.13
        movq      %rsi, %rbp                                    #3969.13
        cmpq      %r8, %rsi                                     #3972.24
        jae       ..B3.25       # Prob 12%                      #3972.24
                                # LOE rbp rsi r8 r13 r14 edx ecx ebx r9d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.24:                        # Preds ..B3.23
        movzbl    -1(%rsi), %ecx                                #3973.32
        shll      $8, %ecx                                      #3973.46
                                # LOE rbp rsi r8 r13 r14 edx ecx ebx r9d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.25:                        # Preds ..B3.9 ..B3.23 ..B3.24
        movl      $16, %r10d                                    #3752.13
        jmp       ..B3.29       # Prob 100%                     #3752.13
                                # LOE rbp rsi r8 r13 r14 edx ecx ebx r9d r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.26:                        # Preds ..B3.18
        psrldq    $1, %xmm0                                     #3865.26
        cmpq      %r8, %rbp                                     #3868.26
        jae       ..B3.28       # Prob 19%                      #3868.26
                                # LOE rbp rsi r8 r13 r14 edx ecx ebx r9d r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.27:                        # Preds ..B3.26
        movzbl    (%rbp), %edi                                  #3871.37
        sarl      $8, %ecx                                      #3870.21
        shll      $8, %edi                                      #3871.48
        orl       %edi, %ecx                                    #3871.21
        pinsrw    $7, %ecx, %xmm0                               #3872.30
                                # LOE rbp rsi r8 r13 r14 edx ecx ebx r9d r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.28:                        # Preds ..B3.27 ..B3.26
        incq      %rbp                                          #3876.20
                                # LOE rbp rsi r8 r13 r14 edx ecx ebx r9d r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.29:                        # Preds ..B3.6 ..B3.25 ..B3.28
        movdqa    %xmm0, %xmm9                                  #3762.22
        pand      %xmm7, %xmm9                                  #3762.22
        pcmpeqd   %xmm8, %xmm9                                  #3763.22
        pmovmskb  %xmm9, %r11d                                  #3764.22
        incl      %r11d                                         #3765.17
        shrl      $16, %r11d                                    #3765.17
        negl      %r11d                                         #3765.17
        jmp       ..B3.14       # Prob 100%                     #3765.17
                                # LOE rbp rsi r8 r13 r14 edx ecx ebx r9d r10d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B3.30:                        # Preds ..B3.16                 # Infreq
        subl      %edx, %ebx                                    #3793.30
        je        ..B3.51       # Prob 25%                      #3793.37
                                # LOE r12 r13 r14 eax edx ebx edi r9d r11d r15d
..B3.31:                        # Preds ..B3.30                 # Infreq
        cmpl      $1, %ebx                                      #3793.37
        jne       ..B3.36       # Prob 67%                      #3793.37
                                # LOE r12 r13 r14 eax edx ebx edi r9d r11d r15d
..B3.32:                        # Preds ..B3.31                 # Infreq
        testl     %r11d, %r11d                                  #3799.36
        jne       ..B3.52       # Prob 28%                      #3799.36
                                # LOE r12 r13 r14 eax edx r9d
..B3.33:                        # Preds ..B3.32                 # Infreq
        testl     %eax, %eax                                    #3800.36
        je        ..B3.53       # Prob 50%                      #3800.36
                                # LOE r12 r13 r14 edx r9d
..B3.34:                        # Preds ..B3.44 ..B3.38 ..B3.33 # Infreq
        subl      %r9d, %edx                                    #3800.47
        addl      $2, %edx                                      #3800.61
        movl      %edx, %eax                                    #3800.61
..___tag_value_eval_2na_pos.267:                                #3800.61
        popq      %rbx                                          #3800.61
..___tag_value_eval_2na_pos.269:                                #
        popq      %rbp                                          #3800.61
..___tag_value_eval_2na_pos.271:                                #
        popq      %r12                                          #3800.61
..___tag_value_eval_2na_pos.273:                                #
        popq      %r15                                          #3800.61
..___tag_value_eval_2na_pos.275:                                #
        ret                                                     #3800.61
..___tag_value_eval_2na_pos.276:                                #
                                # LOE
..B3.36:                        # Preds ..B3.31                 # Infreq
        cmpl      $2, %ebx                                      #3793.37
        jne       ..B3.43       # Prob 50%                      #3793.37
                                # LOE r12 r13 r14 eax edx edi r9d r11d r15d
..B3.37:                        # Preds ..B3.36                 # Infreq
        testl     %r11d, %r11d                                  #3803.36
        jne       ..B3.52       # Prob 28%                      #3803.36
                                # LOE r12 r13 r14 eax edx r9d r15d
..B3.38:                        # Preds ..B3.37                 # Infreq
        testl     %eax, %eax                                    #3804.36
        jne       ..B3.34       # Prob 28%                      #3804.36
                                # LOE r12 r13 r14 edx r9d r15d
..B3.39:                        # Preds ..B3.38                 # Infreq
        testl     %r15d, %r15d                                  #3805.36
        je        ..B3.53       # Prob 50%                      #3805.36
                                # LOE r12 r13 r14 edx r9d
..B3.40:                        # Preds ..B3.45 ..B3.39         # Infreq
        subl      %r9d, %edx                                    #3805.47
        addl      $3, %edx                                      #3805.61
        movl      %edx, %eax                                    #3805.61
..___tag_value_eval_2na_pos.281:                                #3805.61
        popq      %rbx                                          #3805.61
..___tag_value_eval_2na_pos.283:                                #
        popq      %rbp                                          #3805.61
..___tag_value_eval_2na_pos.285:                                #
        popq      %r12                                          #3805.61
..___tag_value_eval_2na_pos.287:                                #
        popq      %r15                                          #3805.61
..___tag_value_eval_2na_pos.289:                                #
        ret                                                     #3805.61
..___tag_value_eval_2na_pos.290:                                #
                                # LOE
..B3.43:                        # Preds ..B3.36                 # Infreq
        testl     %r11d, %r11d                                  #3808.36
        jne       ..B3.52       # Prob 28%                      #3808.36
                                # LOE r12 r13 r14 eax edx edi r9d r15d
..B3.44:                        # Preds ..B3.43                 # Infreq
        testl     %eax, %eax                                    #3809.36
        jne       ..B3.34       # Prob 28%                      #3809.36
                                # LOE r12 r13 r14 edx edi r9d r15d
..B3.45:                        # Preds ..B3.44                 # Infreq
        testl     %r15d, %r15d                                  #3810.36
        jne       ..B3.40       # Prob 28%                      #3810.36
                                # LOE r12 r13 r14 edx edi r9d
..B3.46:                        # Preds ..B3.45                 # Infreq
        testl     %edi, %edi                                    #3811.36
        je        ..B3.53       # Prob 50%                      #3811.36
                                # LOE r12 r13 r14 edx r9d
..B3.47:                        # Preds ..B3.46                 # Infreq
        subl      %r9d, %edx                                    #3811.47
        addl      $4, %edx                                      #3811.61
        movl      %edx, %eax                                    #3811.61
..___tag_value_eval_2na_pos.295:                                #3811.61
        popq      %rbx                                          #3811.61
..___tag_value_eval_2na_pos.297:                                #
        popq      %rbp                                          #3811.61
..___tag_value_eval_2na_pos.299:                                #
        popq      %r12                                          #3811.61
..___tag_value_eval_2na_pos.301:                                #
        popq      %r15                                          #3811.61
..___tag_value_eval_2na_pos.303:                                #
        ret                                                     #3811.61
..___tag_value_eval_2na_pos.304:                                #
                                # LOE
..B3.51:                        # Preds ..B3.30                 # Infreq
        testl     %r11d, %r11d                                  #3796.36
        je        ..B3.53       # Prob 50%                      #3796.36
                                # LOE r12 r13 r14 edx r9d
..B3.52:                        # Preds ..B3.43 ..B3.37 ..B3.32 ..B3.51 # Infreq
        subl      %r9d, %edx                                    #3796.47
        incl      %edx                                          #3796.61
        movl      %edx, %eax                                    #3796.61
..___tag_value_eval_2na_pos.309:                                #3796.61
        popq      %rbx                                          #3796.61
..___tag_value_eval_2na_pos.311:                                #
        popq      %rbp                                          #3796.61
..___tag_value_eval_2na_pos.313:                                #
        popq      %r12                                          #3796.61
..___tag_value_eval_2na_pos.315:                                #
        popq      %r15                                          #3796.61
..___tag_value_eval_2na_pos.317:                                #
        ret                                                     #3796.61
..___tag_value_eval_2na_pos.318:                                #
                                # LOE
..B3.53:                        # Preds ..B3.17 ..B3.19 ..B3.46 ..B3.39 ..B3.33
                                #       ..B3.51                 # Infreq
        xorl      %eax, %eax                                    #3813.28
..___tag_value_eval_2na_pos.323:                                #3813.28
        popq      %rbx                                          #3813.28
..___tag_value_eval_2na_pos.325:                                #
        popq      %rbp                                          #3813.28
..___tag_value_eval_2na_pos.327:                                #
        popq      %r12                                          #3813.28
..___tag_value_eval_2na_pos.329:                                #
        popq      %r15                                          #3813.28
..___tag_value_eval_2na_pos.331:                                #
        ret                                                     #3813.28
        .align    16,0x90
..___tag_value_eval_2na_pos.332:                                #
                                # LOE
# mark_end;
	.type	eval_2na_pos,@function
	.size	eval_2na_pos,.-eval_2na_pos
	.data
# -- End  eval_2na_pos
	.text
# -- Begin  eval_4na_128
# mark_begin;
       .align    16,0x90
eval_4na_128:
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %edx
# parameter 4: %ecx
..B4.1:                         # Preds ..B4.0
..___tag_value_eval_4na_128.333:                                #4724.1
        pushq     %rbp                                          #4724.1
..___tag_value_eval_4na_128.335:                                #
        pushq     %rbx                                          #4724.1
..___tag_value_eval_4na_128.337:                                #
        movdqa    16(%rdi), %xmm8                               #4776.5
        movdqa    32(%rdi), %xmm7                               #4776.5
        movdqa    48(%rdi), %xmm6                               #4776.5
        movdqa    64(%rdi), %xmm5                               #4776.5
        movdqa    80(%rdi), %xmm4                               #4776.5
        movdqa    96(%rdi), %xmm3                               #4776.5
        movdqa    112(%rdi), %xmm2                              #4776.5
        movdqa    128(%rdi), %xmm1                              #4776.5
        lea       (%rdx,%rcx), %ebp                             #4757.5
        movl      %edx, %r10d                                   #4760.49
        shrl      $2, %r10d                                     #4760.49
        subl      4(%rdi), %ebp                                 #4763.12
        movzbl    (%r10,%rsi), %r8d                             #4769.14
        movzbl    1(%r10,%rsi), %r11d                           #4769.14
        lea       3(%rdx,%rcx), %ecx                            #4766.50
        shrl      $2, %ecx                                      #4766.57
        addq      %rsi, %rcx                                    #4766.30
        lea       expand_2na.0(%rip), %rax                      #4769.14
        movzwl    (%rax,%r8,2), %r9d                            #4769.14
        movzwl    (%rax,%r11,2), %r8d                           #4769.14
        movw      %r9w, -24(%rsp)                               #4769.14
        movzbl    2(%r10,%rsi), %r9d                            #4769.14
        movzwl    (%rax,%r9,2), %r11d                           #4769.14
        movw      %r8w, -22(%rsp)                               #4769.14
        movzbl    3(%r10,%rsi), %r8d                            #4769.14
        movzwl    (%rax,%r8,2), %r9d                            #4769.14
        movw      %r11w, -20(%rsp)                              #4769.14
        movzbl    4(%r10,%rsi), %r11d                           #4769.14
        movzwl    (%rax,%r11,2), %r8d                           #4769.14
        movw      %r9w, -18(%rsp)                               #4769.14
        movzbl    5(%r10,%rsi), %r9d                            #4769.14
        movzwl    (%rax,%r9,2), %r11d                           #4769.14
        movw      %r8w, -16(%rsp)                               #4769.14
        movzbl    6(%r10,%rsi), %r8d                            #4769.14
        movzwl    (%rax,%r8,2), %r9d                            #4769.14
        xorl      %r8d, %r8d                                    #4779.15
        movw      %r11w, -14(%rsp)                              #4769.14
        movzbl    7(%r10,%rsi), %r11d                           #4769.14
        movzwl    (%rax,%r11,2), %eax                           #4769.14
        movw      %r9w, -12(%rsp)                               #4769.14
        xorl      %r9d, %r9d                                    #4779.10
        movw      %ax, -10(%rsp)                                #4769.14
        movdqu    -24(%rsp), %xmm0                              #4769.14
        lea       8(%rsi,%r10), %rax                            #4770.5
        movq      %rax, %r10                                    #4770.5
        xorl      %edi, %edi                                    #4779.5
        movl      %ebp, %esi                                    #4781.20
        subl      %edx, %esi                                    #4781.20
        addl      $7, %esi                                      #4781.33
        shrl      $2, %esi                                      #4781.40
        movl      %edx, %r11d                                   #4790.20
        andl      $3, %r11d                                     #4790.20
        je        ..B4.20       # Prob 20%                      #4790.20
                                # LOE rax rcx rbx r10 r12 r13 r14 r15 edx ebp esi edi r8d r9d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.2:                         # Preds ..B4.1
        cmpl      $1, %r11d                                     #4790.20
        je        ..B4.9        # Prob 25%                      #4790.20
                                # LOE rax rcx rbx r10 r12 r13 r14 r15 edx ebp esi edi r8d r9d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.3:                         # Preds ..B4.2
        cmpl      $2, %r11d                                     #4790.20
        je        ..B4.10       # Prob 33%                      #4790.20
                                # LOE rax rcx rbx r10 r12 r13 r14 r15 edx ebp esi edi r8d r9d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.4:                         # Preds ..B4.3
        cmpl      $3, %r11d                                     #4790.20
        je        ..B4.11       # Prob 50%                      #4790.20
        jmp       ..B4.16       # Prob 100%                     #4790.20
                                # LOE rax rcx rbx r10 r12 r13 r14 r15 edx ebp esi edi r8d r9d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.9:                         # Preds ..B4.2 ..B4.20
        movdqa    %xmm0, %xmm10                                 #4814.22
        movdqa    %xmm0, %xmm9                                  #4815.22
        pand      %xmm6, %xmm10                                 #4814.22
        pand      %xmm5, %xmm9                                  #4815.22
        pcmpeqd   %xmm9, %xmm10                                 #4816.22
        pmovmskb  %xmm10, %r9d                                  #4817.22
        incl      %r9d                                          #4818.17
        shrl      $16, %r9d                                     #4818.17
        negl      %r9d                                          #4818.17
                                # LOE rax rcx r10 r12 r13 r14 r15 edx ebp esi edi r9d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.10:                        # Preds ..B4.3 ..B4.9
        movdqa    %xmm0, %xmm10                                 #4821.22
        movdqa    %xmm0, %xmm9                                  #4822.22
        pand      %xmm4, %xmm10                                 #4821.22
        pand      %xmm3, %xmm9                                  #4822.22
        pcmpeqd   %xmm9, %xmm10                                 #4823.22
        pmovmskb  %xmm10, %r8d                                  #4824.22
        incl      %r8d                                          #4825.17
        shrl      $16, %r8d                                     #4825.17
        negl      %r8d                                          #4825.17
                                # LOE rax rcx r10 r12 r13 r14 r15 edx ebp esi edi r8d r9d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.11:                        # Preds ..B4.4 ..B4.10
        movdqa    %xmm0, %xmm10                                 #4828.22
        movdqa    %xmm0, %xmm9                                  #4829.22
        andl      $-4, %edx                                     #4836.17
        pand      %xmm2, %xmm10                                 #4828.22
        pand      %xmm1, %xmm9                                  #4829.22
        pcmpeqd   %xmm9, %xmm10                                 #4830.22
        pmovmskb  %xmm10, %ebx                                  #4831.22
        movl      %edi, %r11d                                   #4839.29
        orl       %r9d, %r11d                                   #4839.29
        incl      %ebx                                          #4832.17
        shrl      $16, %ebx                                     #4832.17
        negl      %ebx                                          #4832.17
        orl       %r8d, %r11d                                   #4839.34
        orl       %ebx, %r11d                                   #4839.39
        jne       ..B4.21       # Prob 20%                      #4839.47
                                # LOE rax rcx r10 r12 r13 r14 r15 edx ebp esi edi r8d r9d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.12:                        # Preds ..B4.11
        addl      $4, %edx                                      #4880.17
        cmpl      %ebp, %edx                                    #4883.28
        ja        ..B4.29       # Prob 20%                      #4883.28
                                # LOE rax rcx r10 r12 r13 r14 r15 edx ebp esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.13:                        # Preds ..B4.12
        decl      %esi                                          #4887.25
        jne       ..B4.17       # Prob 50%                      #4887.39
                                # LOE rax rcx r10 r12 r13 r14 r15 edx ebp esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.14:                        # Preds ..B4.13
        cmpq      %rcx, %rax                                    #4904.25
        jae       ..B4.29       # Prob 4%                       #4904.25
                                # LOE rax rcx r12 r13 r14 r15 edx ebp xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.15:                        # Preds ..B4.14
        movzbl    (%rax), %esi                                  #4914.22
        movzbl    1(%rax), %r8d                                 #4914.22
        movzbl    2(%rax), %r10d                                #4914.22
        lea       expand_2na.0(%rip), %rbx                      #4914.22
        movzwl    (%rbx,%rsi,2), %edi                           #4914.22
        movzwl    (%rbx,%r8,2), %r9d                            #4914.22
        movzwl    (%rbx,%r10,2), %r11d                          #4914.22
        movzbl    3(%rax), %esi                                 #4914.22
        movzbl    4(%rax), %r8d                                 #4914.22
        movzbl    5(%rax), %r10d                                #4914.22
        movw      %di, -24(%rsp)                                #4914.22
        movzwl    (%rbx,%rsi,2), %edi                           #4914.22
        movzbl    6(%rax), %esi                                 #4914.22
        movw      %r9w, -22(%rsp)                               #4914.22
        movzwl    (%rbx,%r8,2), %r9d                            #4914.22
        movzbl    7(%rax), %r8d                                 #4914.22
        movw      %r11w, -20(%rsp)                              #4914.22
        movzwl    (%rbx,%r10,2), %r11d                          #4914.22
        addq      $8, %rax                                      #4929.13
        movq      %rax, %r10                                    #4929.13
        movw      %di, -18(%rsp)                                #4914.22
        movzwl    (%rbx,%rsi,2), %edi                           #4914.22
        movzwl    (%rbx,%r8,2), %ebx                            #4914.22
        movw      %r9w, -16(%rsp)                               #4914.22
        movw      %r11w, -14(%rsp)                              #4914.22
        movw      %di, -12(%rsp)                                #4914.22
        movw      %bx, -10(%rsp)                                #4914.22
        movdqu    -24(%rsp), %xmm0                              #4914.22
                                # LOE rax rcx r10 r12 r13 r14 r15 edx ebp xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.16:                        # Preds ..B4.4 ..B4.15
        movl      $8, %esi                                      #4797.13
        jmp       ..B4.20       # Prob 100%                     #4797.13
                                # LOE rax rcx r10 r12 r13 r14 r15 edx ebp esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.17:                        # Preds ..B4.13
        psrldq    $2, %xmm0                                     #4892.26
        cmpq      %rcx, %r10                                    #4895.26
        jae       ..B4.19       # Prob 19%                      #4895.26
                                # LOE rax rcx r10 r12 r13 r14 r15 edx ebp esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.18:                        # Preds ..B4.17
        movzbl    (%r10), %edi                                  #4896.72
        lea       expand_2na.0(%rip), %rbx                      #4896.57
        movzwl    (%rbx,%rdi,2), %r8d                           #4896.57
        pinsrw    $7, %r8d, %xmm0                               #4896.30
                                # LOE rax rcx r10 r12 r13 r14 r15 edx ebp esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.19:                        # Preds ..B4.18 ..B4.17
        incq      %r10                                          #4899.20
                                # LOE rax rcx r10 r12 r13 r14 r15 edx ebp esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.20:                        # Preds ..B4.1 ..B4.16 ..B4.19
        movdqa    %xmm0, %xmm10                                 #4807.22
        movdqa    %xmm0, %xmm9                                  #4808.22
        pand      %xmm8, %xmm10                                 #4807.22
        pand      %xmm7, %xmm9                                  #4808.22
        pcmpeqd   %xmm9, %xmm10                                 #4809.22
        pmovmskb  %xmm10, %edi                                  #4810.22
        incl      %edi                                          #4811.17
        shrl      $16, %edi                                     #4811.17
        negl      %edi                                          #4811.17
        jmp       ..B4.9        # Prob 100%                     #4811.17
                                # LOE rax rcx r10 r12 r13 r14 r15 edx ebp esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B4.21:                        # Preds ..B4.11                 # Infreq
        subl      %edx, %ebp                                    #4842.30
        cmpl      $2, %ebp                                      #4842.37
        je        ..B4.25       # Prob 25%                      #4842.37
                                # LOE rbx r12 r13 r14 r15 ebp edi r8d r9d
..B4.22:                        # Preds ..B4.21                 # Infreq
        cmpl      $1, %ebp                                      #4842.37
        je        ..B4.26       # Prob 33%                      #4842.37
                                # LOE rbx r12 r13 r14 r15 ebp edi r9d
..B4.23:                        # Preds ..B4.22                 # Infreq
        testl     %ebp, %ebp                                    #4842.37
        je        ..B4.27       # Prob 50%                      #4842.37
                                # LOE rbx r12 r13 r14 r15 edi
..B4.24:                        # Preds ..B4.23                 # Infreq
        movl      $1, %eax                                      #4845.32
..___tag_value_eval_4na_128.340:                                #4845.32
        popq      %rbx                                          #4845.32
..___tag_value_eval_4na_128.342:                                #
        popq      %rbp                                          #4845.32
..___tag_value_eval_4na_128.344:                                #
        ret                                                     #4845.32
..___tag_value_eval_4na_128.345:                                #
                                # LOE
..B4.25:                        # Preds ..B4.21                 # Infreq
        testl     %r8d, %r8d                                    #4847.36
        jne       ..B4.28       # Prob 28%                      #4847.36
                                # LOE rbx r12 r13 r14 r15 edi r9d
..B4.26:                        # Preds ..B4.22 ..B4.25         # Infreq
        testl     %r9d, %r9d                                    #4849.36
        jne       ..B4.28       # Prob 28%                      #4849.36
                                # LOE rbx r12 r13 r14 r15 edi
..B4.27:                        # Preds ..B4.26 ..B4.23         # Infreq
        testl     %edi, %edi                                    #4851.36
        je        ..B4.29       # Prob 50%                      #4851.36
                                # LOE rbx r12 r13 r14 r15
..B4.28:                        # Preds ..B4.25 ..B4.26 ..B4.27 # Infreq
        movl      $1, %eax                                      #4851.47
..___tag_value_eval_4na_128.348:                                #4851.47
        popq      %rbx                                          #4851.47
..___tag_value_eval_4na_128.350:                                #
        popq      %rbp                                          #4851.47
..___tag_value_eval_4na_128.352:                                #
        ret                                                     #4851.47
..___tag_value_eval_4na_128.353:                                #
                                # LOE
..B4.29:                        # Preds ..B4.14 ..B4.12 ..B4.27 # Infreq
        xorl      %eax, %eax                                    #4853.28
..___tag_value_eval_4na_128.356:                                #4853.28
        popq      %rbx                                          #4853.28
..___tag_value_eval_4na_128.358:                                #
        popq      %rbp                                          #4853.28
..___tag_value_eval_4na_128.360:                                #
        ret                                                     #4853.28
        .align    16,0x90
..___tag_value_eval_4na_128.361:                                #
                                # LOE
# mark_end;
	.type	eval_4na_128,@function
	.size	eval_4na_128,.-eval_4na_128
	.data
# -- End  eval_4na_128
	.text
# -- Begin  eval_4na_32
# mark_begin;
       .align    16,0x90
eval_4na_32:
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %edx
# parameter 4: %ecx
..B5.1:                         # Preds ..B5.0
..___tag_value_eval_4na_32.362:                                 #4279.1
        pushq     %r15                                          #4279.1
..___tag_value_eval_4na_32.364:                                 #
        pushq     %r14                                          #4279.1
..___tag_value_eval_4na_32.366:                                 #
        pushq     %r13                                          #4279.1
..___tag_value_eval_4na_32.368:                                 #
        pushq     %r12                                          #4279.1
..___tag_value_eval_4na_32.370:                                 #
        pushq     %rbx                                          #4279.1
..___tag_value_eval_4na_32.372:                                 #
        movdqa    16(%rdi), %xmm8                               #4331.5
        movdqa    32(%rdi), %xmm7                               #4331.5
        movdqa    48(%rdi), %xmm6                               #4331.5
        movdqa    64(%rdi), %xmm5                               #4331.5
        movdqa    80(%rdi), %xmm4                               #4331.5
        movdqa    96(%rdi), %xmm3                               #4331.5
        movdqa    112(%rdi), %xmm2                              #4331.5
        movdqa    128(%rdi), %xmm1                              #4331.5
        lea       (%rdx,%rcx), %r11d                            #4312.5
        movl      %edx, %ebx                                    #4315.49
        shrl      $2, %ebx                                      #4315.49
        subl      4(%rdi), %r11d                                #4318.12
        movzbl    (%rbx,%rsi), %eax                             #4324.14
        movzbl    1(%rbx,%rsi), %r9d                            #4324.14
        lea       3(%rdx,%rcx), %r10d                           #4321.50
        shrl      $2, %r10d                                     #4321.57
        addq      %rsi, %r10                                    #4321.30
        lea       expand_2na.0(%rip), %rcx                      #4324.14
        movzwl    (%rcx,%rax,2), %r8d                           #4324.14
        movzwl    (%rcx,%r9,2), %eax                            #4324.14
        movw      %r8w, -16(%rsp)                               #4324.14
        movzbl    2(%rbx,%rsi), %r8d                            #4324.14
        movzwl    (%rcx,%r8,2), %r9d                            #4324.14
        movw      %ax, -14(%rsp)                                #4324.14
        movzbl    3(%rbx,%rsi), %eax                            #4324.14
        movzwl    (%rcx,%rax,2), %r8d                           #4324.14
        movw      %r9w, -12(%rsp)                               #4324.14
        movzbl    4(%rbx,%rsi), %r9d                            #4324.14
        movzwl    (%rcx,%r9,2), %eax                            #4324.14
        movw      %r8w, -10(%rsp)                               #4324.14
        movzbl    5(%rbx,%rsi), %r8d                            #4324.14
        movzwl    (%rcx,%r8,2), %r9d                            #4324.14
        movw      %ax, -8(%rsp)                                 #4324.14
        movzbl    6(%rbx,%rsi), %eax                            #4324.14
        movzwl    (%rcx,%rax,2), %r8d                           #4324.14
        movw      %r9w, -6(%rsp)                                #4324.14
        movzbl    7(%rbx,%rsi), %r9d                            #4324.14
        movzwl    (%rcx,%r9,2), %ecx                            #4324.14
        movw      %r8w, -4(%rsp)                                #4324.14
        movl      %edx, %eax                                    #4345.20
        lea       8(%rsi,%rbx), %rbx                            #4325.5
        movw      %cx, -2(%rsp)                                 #4324.14
        movdqu    -16(%rsp), %xmm0                              #4324.14
        movq      %rbx, %rcx                                    #4325.5
        xorl      %r8d, %r8d                                    #4334.15
        xorl      %r9d, %r9d                                    #4334.10
        xorl      %edi, %edi                                    #4334.5
        movl      $2, %esi                                      #4340.5
        andl      $3, %eax                                      #4345.20
        je        ..B5.29       # Prob 20%                      #4345.20
                                # LOE rcx rbx rbp r10 r12 r13 r14 r15 eax edx esi edi r8d r9d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.2:                         # Preds ..B5.1
        cmpl      $1, %eax                                      #4345.20
        je        ..B5.9        # Prob 25%                      #4345.20
                                # LOE rcx rbx rbp r10 r12 r13 r14 r15 eax edx esi edi r8d r9d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.3:                         # Preds ..B5.2
        cmpl      $2, %eax                                      #4345.20
        je        ..B5.10       # Prob 33%                      #4345.20
                                # LOE rcx rbx rbp r10 r12 r13 r14 r15 eax edx esi edi r8d r9d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.4:                         # Preds ..B5.3
        cmpl      $3, %eax                                      #4345.20
        je        ..B5.11       # Prob 50%                      #4345.20
        jmp       ..B5.25       # Prob 100%                     #4345.20
                                # LOE rcx rbx rbp r10 r12 r13 r14 r15 edx esi edi r8d r9d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.9:                         # Preds ..B5.2 ..B5.29
        movdqa    %xmm0, %xmm10                                 #4369.22
        movdqa    %xmm0, %xmm9                                  #4370.22
        pand      %xmm6, %xmm10                                 #4369.22
        pand      %xmm5, %xmm9                                  #4370.22
        pcmpeqd   %xmm9, %xmm10                                 #4371.22
        pmovmskb  %xmm10, %r9d                                  #4372.22
                                # LOE rcx rbx rbp r10 edx esi edi r9d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.10:                        # Preds ..B5.3 ..B5.9
        movdqa    %xmm0, %xmm10                                 #4376.22
        movdqa    %xmm0, %xmm9                                  #4377.22
        pand      %xmm4, %xmm10                                 #4376.22
        pand      %xmm3, %xmm9                                  #4377.22
        pcmpeqd   %xmm9, %xmm10                                 #4378.22
        pmovmskb  %xmm10, %r8d                                  #4379.22
                                # LOE rcx rbx rbp r10 edx esi edi r8d r9d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.11:                        # Preds ..B5.4 ..B5.10
        movdqa    %xmm0, %xmm10                                 #4383.22
        movdqa    %xmm0, %xmm9                                  #4384.22
        movl      %edx, %r13d                                   #4391.17
        pand      %xmm2, %xmm10                                 #4383.22
        pand      %xmm1, %xmm9                                  #4384.22
        pcmpeqd   %xmm9, %xmm10                                 #4385.22
        pmovmskb  %xmm10, %r12d                                 #4386.22
        andl      $-4, %r13d                                    #4391.17
        movl      %edi, %edx                                    #4394.29
        orl       %r9d, %edx                                    #4394.29
        orl       %r8d, %edx                                    #4394.34
        orl       %r12d, %edx                                   #4394.39
        je        ..B5.20       # Prob 78%                      #4394.47
                                # LOE rcx rbx rbp r10 esi edi r8d r9d r11d r12d r13d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.12:                        # Preds ..B5.11
        movl      %edi, %eax                                    #4412.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #4412.30
        movswq    %ax, %r15                                     #4412.30
        movl      %r9d, %eax                                    #4413.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #4413.30
        movswq    %ax, %rdx                                     #4413.30
        movl      %r8d, %eax                                    #4414.30
        lea       1(%rdx,%rdx), %edx                            #4421.40
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #4414.30
        movswq    %ax, %r14                                     #4414.30
        movl      %r12d, %eax                                   #4415.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #4415.30
        movswq    %ax, %rax                                     #4415.30
        lea       2(%r14,%r14), %r14d                           #4422.40
        lea       3(%rax,%rax), %eax                            #4423.40
        testl     %edi, %edi                                    #4427.32
        je        ..B5.14       # Prob 50%                      #4427.32
                                # LOE rcx rbx rbp r10 eax edx esi r8d r9d r11d r12d r13d r14d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.13:                        # Preds ..B5.12
        lea       (%r13,%r15,2), %edi                           #4427.43
        cmpl      %edi, %r11d                                   #4427.49
        jae       ..B5.34       # Prob 20%                      #4427.49
                                # LOE rcx rbx rbp r10 eax edx esi r8d r9d r11d r12d r13d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.14:                        # Preds ..B5.13 ..B5.12
        testl     %r9d, %r9d                                    #4428.32
        je        ..B5.16       # Prob 50%                      #4428.32
                                # LOE rcx rbx rbp r10 eax edx esi r8d r11d r12d r13d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.15:                        # Preds ..B5.14
        addl      %r13d, %edx                                   #4428.43
        cmpl      %edx, %r11d                                   #4428.49
        jae       ..B5.34       # Prob 20%                      #4428.49
                                # LOE rcx rbx rbp r10 eax esi r8d r11d r12d r13d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.16:                        # Preds ..B5.15 ..B5.14
        testl     %r8d, %r8d                                    #4429.32
        je        ..B5.18       # Prob 50%                      #4429.32
                                # LOE rcx rbx rbp r10 eax esi r11d r12d r13d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.17:                        # Preds ..B5.16
        addl      %r13d, %r14d                                  #4429.43
        cmpl      %r14d, %r11d                                  #4429.49
        jae       ..B5.34       # Prob 20%                      #4429.49
                                # LOE rcx rbx rbp r10 eax esi r11d r12d r13d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.18:                        # Preds ..B5.17 ..B5.16
        testl     %r12d, %r12d                                  #4430.32
        je        ..B5.20       # Prob 50%                      #4430.32
                                # LOE rcx rbx rbp r10 eax esi r11d r13d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.19:                        # Preds ..B5.18
        addl      %r13d, %eax                                   #4430.43
        cmpl      %eax, %r11d                                   #4430.49
        jae       ..B5.34       # Prob 20%                      #4430.49
                                # LOE rcx rbx rbp r10 esi r11d r13d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.20:                        # Preds ..B5.19 ..B5.18 ..B5.11
        lea       4(%r13), %edx                                 #4435.17
        cmpl      %r11d, %edx                                   #4438.28
        ja        ..B5.31       # Prob 20%                      #4438.28
                                # LOE rcx rbx rbp r10 edx esi r11d r13d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.21:                        # Preds ..B5.20
        decl      %esi                                          #4442.25
        jne       ..B5.26       # Prob 50%                      #4442.39
                                # LOE rcx rbx rbp r10 edx esi r11d r13d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.22:                        # Preds ..B5.21
        cmpq      %r10, %rbx                                    #4459.25
        jae       ..B5.31       # Prob 4%                       #4459.25
                                # LOE rbx rbp r10 r11d r13d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.23:                        # Preds ..B5.22
        lea       28(%r13), %edx                                #4463.13
        cmpl      %r11d, %edx                                   #4464.24
        ja        ..B5.31       # Prob 4%                       #4464.24
                                # LOE rbx rbp r10 edx r11d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.24:                        # Preds ..B5.23
        movzbl    (%rbx), %ecx                                  #4469.22
        movzbl    1(%rbx), %edi                                 #4469.22
        movzbl    2(%rbx), %r9d                                 #4469.22
        movzbl    3(%rbx), %r13d                                #4469.22
        movzbl    4(%rbx), %r15d                                #4469.22
        lea       expand_2na.0(%rip), %rax                      #4469.22
        movzwl    (%rax,%rcx,2), %esi                           #4469.22
        movzwl    (%rax,%rdi,2), %r8d                           #4469.22
        movzwl    (%rax,%r9,2), %r12d                           #4469.22
        movzwl    (%rax,%r13,2), %r14d                          #4469.22
        movzwl    (%rax,%r15,2), %ecx                           #4469.22
        movw      %si, -16(%rsp)                                #4469.22
        movzbl    5(%rbx), %esi                                 #4469.22
        movzwl    (%rax,%rsi,2), %edi                           #4469.22
        movw      %r8w, -14(%rsp)                               #4469.22
        movzbl    6(%rbx), %r8d                                 #4469.22
        movzwl    (%rax,%r8,2), %r9d                            #4469.22
        movw      %r12w, -12(%rsp)                              #4469.22
        movzbl    7(%rbx), %r12d                                #4469.22
        movzwl    (%rax,%r12,2), %eax                           #4469.22
        movw      %r14w, -10(%rsp)                              #4469.22
        movw      %cx, -8(%rsp)                                 #4469.22
        addq      $8, %rbx                                      #4484.13
        movw      %di, -6(%rsp)                                 #4469.22
        movq      %rbx, %rcx                                    #4484.13
        movw      %r9w, -4(%rsp)                                #4469.22
        movw      %ax, -2(%rsp)                                 #4469.22
        movdqu    -16(%rsp), %xmm0                              #4469.22
                                # LOE rcx rbx rbp r10 edx r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.25:                        # Preds ..B5.4 ..B5.24
        movl      $2, %esi                                      #4352.13
        jmp       ..B5.29       # Prob 100%                     #4352.13
                                # LOE rcx rbx rbp r10 edx esi r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.26:                        # Preds ..B5.21
        psrldq    $2, %xmm0                                     #4447.26
        cmpq      %r10, %rcx                                    #4450.26
        jae       ..B5.28       # Prob 19%                      #4450.26
                                # LOE rcx rbx rbp r10 edx esi r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.27:                        # Preds ..B5.26
        movzbl    (%rcx), %edi                                  #4451.72
        lea       expand_2na.0(%rip), %rax                      #4451.57
        movzwl    (%rax,%rdi,2), %r8d                           #4451.57
        pinsrw    $7, %r8d, %xmm0                               #4451.30
                                # LOE rcx rbx rbp r10 edx esi r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.28:                        # Preds ..B5.27 ..B5.26
        incq      %rcx                                          #4454.20
                                # LOE rcx rbx rbp r10 edx esi r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.29:                        # Preds ..B5.1 ..B5.25 ..B5.28
        movdqa    %xmm0, %xmm10                                 #4362.22
        movdqa    %xmm0, %xmm9                                  #4363.22
        pand      %xmm8, %xmm10                                 #4362.22
        pand      %xmm7, %xmm9                                  #4363.22
        pcmpeqd   %xmm9, %xmm10                                 #4364.22
        pmovmskb  %xmm10, %edi                                  #4365.22
        jmp       ..B5.9        # Prob 100%                     #4365.22
                                # LOE rcx rbx rbp r10 edx esi edi r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B5.31:                        # Preds ..B5.20 ..B5.22 ..B5.23 # Infreq
        xorl      %eax, %eax                                    #4495.12
..___tag_value_eval_4na_32.375:                                 #4495.12
        popq      %rbx                                          #4495.12
..___tag_value_eval_4na_32.377:                                 #
        popq      %r12                                          #4495.12
..___tag_value_eval_4na_32.379:                                 #
        popq      %r13                                          #4495.12
..___tag_value_eval_4na_32.381:                                 #
        popq      %r14                                          #4495.12
..___tag_value_eval_4na_32.383:                                 #
        popq      %r15                                          #4495.12
..___tag_value_eval_4na_32.385:                                 #
        ret                                                     #4495.12
..___tag_value_eval_4na_32.386:                                 #
                                # LOE
..B5.34:                        # Preds ..B5.13 ..B5.19 ..B5.17 ..B5.15 # Infreq
        movl      $1, %eax                                      #4428.63
..___tag_value_eval_4na_32.392:                                 #4428.63
        popq      %rbx                                          #4428.63
..___tag_value_eval_4na_32.394:                                 #
        popq      %r12                                          #4428.63
..___tag_value_eval_4na_32.396:                                 #
        popq      %r13                                          #4428.63
..___tag_value_eval_4na_32.398:                                 #
        popq      %r14                                          #4428.63
..___tag_value_eval_4na_32.400:                                 #
        popq      %r15                                          #4428.63
..___tag_value_eval_4na_32.402:                                 #
        ret                                                     #4428.63
        .align    16,0x90
..___tag_value_eval_4na_32.403:                                 #
                                # LOE
# mark_end;
	.type	eval_4na_32,@function
	.size	eval_4na_32,.-eval_4na_32
	.data
# -- End  eval_4na_32
	.text
# -- Begin  eval_4na_16
# mark_begin;
       .align    16,0x90
eval_4na_16:
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %edx
# parameter 4: %ecx
..B6.1:                         # Preds ..B6.0
..___tag_value_eval_4na_16.404:                                 #4057.1
        pushq     %r13                                          #4057.1
..___tag_value_eval_4na_16.406:                                 #
        pushq     %r12                                          #4057.1
..___tag_value_eval_4na_16.408:                                 #
        pushq     %rbx                                          #4057.1
..___tag_value_eval_4na_16.410:                                 #
        movdqa    16(%rdi), %xmm8                               #4109.5
        movdqa    32(%rdi), %xmm7                               #4109.5
        movdqa    48(%rdi), %xmm6                               #4109.5
        movdqa    64(%rdi), %xmm5                               #4109.5
        movdqa    80(%rdi), %xmm4                               #4109.5
        movdqa    96(%rdi), %xmm3                               #4109.5
        movdqa    112(%rdi), %xmm2                              #4109.5
        movdqa    128(%rdi), %xmm1                              #4109.5
        lea       (%rdx,%rcx), %r10d                            #4090.5
        subl      4(%rdi), %r10d                                #4096.12
        movl      %edx, %eax                                    #4093.49
        shrl      $2, %eax                                      #4093.49
        movzbl    (%rax,%rsi), %r8d                             #4102.14
        lea       3(%rdx,%rcx), %r9d                            #4099.50
        shrl      $2, %r9d                                      #4099.57
        addq      %rsi, %r9                                     #4099.30
        lea       expand_2na.0(%rip), %rcx                      #4102.14
        movzwl    (%rcx,%r8,2), %r11d                           #4102.14
        movzbl    1(%rax,%rsi), %r8d                            #4102.14
        movw      %r11w, -16(%rsp)                              #4102.14
        movzwl    (%rcx,%r8,2), %r11d                           #4102.14
        movzbl    2(%rax,%rsi), %r8d                            #4102.14
        xorl      %edi, %edi                                    #4112.15
        movw      %r11w, -14(%rsp)                              #4102.14
        movzwl    (%rcx,%r8,2), %r11d                           #4102.14
        movzbl    3(%rax,%rsi), %r8d                            #4102.14
        movw      %r11w, -12(%rsp)                              #4102.14
        movzwl    (%rcx,%r8,2), %r11d                           #4102.14
        movzbl    4(%rax,%rsi), %r8d                            #4102.14
        movw      %r11w, -10(%rsp)                              #4102.14
        movzwl    (%rcx,%r8,2), %r11d                           #4102.14
        movzbl    5(%rax,%rsi), %r8d                            #4102.14
        movw      %r11w, -8(%rsp)                               #4102.14
        movzwl    (%rcx,%r8,2), %r11d                           #4102.14
        movzbl    6(%rax,%rsi), %r8d                            #4102.14
        movw      %r11w, -6(%rsp)                               #4102.14
        movzwl    (%rcx,%r8,2), %r11d                           #4102.14
        movzbl    7(%rax,%rsi), %r8d                            #4102.14
        movzwl    (%rcx,%r8,2), %ecx                            #4102.14
        xorl      %r8d, %r8d                                    #4112.10
        movw      %r11w, -4(%rsp)                               #4102.14
        movw      %cx, -2(%rsp)                                 #4102.14
        movdqu    -16(%rsp), %xmm0                              #4102.14
        lea       8(%rsi,%rax), %rcx                            #4103.5
        xorl      %esi, %esi                                    #4112.5
        movl      %edx, %r11d                                   #4123.20
        andl      $3, %r11d                                     #4123.20
        je        ..B6.24       # Prob 20%                      #4123.20
                                # LOE rcx rbx rbp r9 r12 r13 r14 r15 edx esi edi r8d r10d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.2:                         # Preds ..B6.1
        cmpl      $1, %r11d                                     #4123.20
        je        ..B6.9        # Prob 25%                      #4123.20
                                # LOE rcx rbx rbp r9 r12 r13 r14 r15 edx esi edi r8d r10d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.3:                         # Preds ..B6.2
        cmpl      $2, %r11d                                     #4123.20
        je        ..B6.10       # Prob 33%                      #4123.20
                                # LOE rcx rbx rbp r9 r12 r13 r14 r15 edx esi edi r8d r10d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.4:                         # Preds ..B6.3
        cmpl      $3, %r11d                                     #4123.20
        je        ..B6.11       # Prob 50%                      #4123.20
        jmp       ..B6.24       # Prob 100%                     #4123.20
                                # LOE rcx rbx rbp r9 r12 r13 r14 r15 edx esi edi r8d r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.9:                         # Preds ..B6.2 ..B6.24
        movdqa    %xmm0, %xmm10                                 #4147.22
        movdqa    %xmm0, %xmm9                                  #4148.22
        pand      %xmm6, %xmm10                                 #4147.22
        pand      %xmm5, %xmm9                                  #4148.22
        pcmpeqw   %xmm9, %xmm10                                 #4149.22
        pmovmskb  %xmm10, %r8d                                  #4150.22
                                # LOE rcx rbp r9 r14 r15 edx esi r8d r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.10:                        # Preds ..B6.3 ..B6.9
        movdqa    %xmm0, %xmm10                                 #4154.22
        movdqa    %xmm0, %xmm9                                  #4155.22
        pand      %xmm4, %xmm10                                 #4154.22
        pand      %xmm3, %xmm9                                  #4155.22
        pcmpeqw   %xmm9, %xmm10                                 #4156.22
        pmovmskb  %xmm10, %edi                                  #4157.22
                                # LOE rcx rbp r9 r14 r15 edx esi edi r8d r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.11:                        # Preds ..B6.4 ..B6.10
        movdqa    %xmm0, %xmm9                                  #4161.22
        pand      %xmm1, %xmm0                                  #4162.22
        andl      $-4, %edx                                     #4169.17
        pand      %xmm2, %xmm9                                  #4161.22
        pcmpeqw   %xmm0, %xmm9                                  #4163.22
        pmovmskb  %xmm9, %r13d                                  #4164.22
        movl      %esi, %ebx                                    #4172.29
        orl       %r8d, %ebx                                    #4172.29
        orl       %edi, %ebx                                    #4172.34
        orl       %r13d, %ebx                                   #4172.39
        je        ..B6.20       # Prob 78%                      #4172.47
                                # LOE rcx rbp r9 r14 r15 edx esi edi r8d r10d r13d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.12:                        # Preds ..B6.11
        movl      %esi, %eax                                    #4190.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #4190.30
        movswq    %ax, %r12                                     #4190.30
        movl      %r8d, %eax                                    #4191.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #4191.30
        movswq    %ax, %rbx                                     #4191.30
        movl      %edi, %eax                                    #4192.30
        lea       1(%rbx,%rbx), %ebx                            #4199.40
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #4192.30
        movswq    %ax, %r11                                     #4192.30
        movl      %r13d, %eax                                   #4193.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #4193.30
        movswq    %ax, %rax                                     #4193.30
        lea       2(%r11,%r11), %r11d                           #4200.40
        lea       3(%rax,%rax), %eax                            #4201.40
        testl     %esi, %esi                                    #4205.32
        je        ..B6.14       # Prob 50%                      #4205.32
                                # LOE rcx rbp r9 r14 r15 eax edx ebx edi r8d r10d r11d r12d r13d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.13:                        # Preds ..B6.12
        lea       (%rdx,%r12,2), %esi                           #4205.43
        cmpl      %esi, %r10d                                   #4205.49
        jae       ..B6.29       # Prob 20%                      #4205.49
                                # LOE rcx rbp r9 r14 r15 eax edx ebx edi r8d r10d r11d r13d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.14:                        # Preds ..B6.13 ..B6.12
        testl     %r8d, %r8d                                    #4206.32
        je        ..B6.16       # Prob 50%                      #4206.32
                                # LOE rcx rbp r9 r14 r15 eax edx ebx edi r10d r11d r13d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.15:                        # Preds ..B6.14
        addl      %edx, %ebx                                    #4206.43
        cmpl      %ebx, %r10d                                   #4206.49
        jae       ..B6.29       # Prob 20%                      #4206.49
                                # LOE rcx rbp r9 r14 r15 eax edx edi r10d r11d r13d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.16:                        # Preds ..B6.15 ..B6.14
        testl     %edi, %edi                                    #4207.32
        je        ..B6.18       # Prob 50%                      #4207.32
                                # LOE rcx rbp r9 r14 r15 eax edx r10d r11d r13d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.17:                        # Preds ..B6.16
        addl      %edx, %r11d                                   #4207.43
        cmpl      %r11d, %r10d                                  #4207.49
        jae       ..B6.29       # Prob 20%                      #4207.49
                                # LOE rcx rbp r9 r14 r15 eax edx r10d r13d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.18:                        # Preds ..B6.17 ..B6.16
        testl     %r13d, %r13d                                  #4208.32
        je        ..B6.20       # Prob 50%                      #4208.32
                                # LOE rcx rbp r9 r14 r15 eax edx r10d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.19:                        # Preds ..B6.18
        addl      %edx, %eax                                    #4208.43
        cmpl      %eax, %r10d                                   #4208.49
        jae       ..B6.29       # Prob 20%                      #4208.49
                                # LOE rcx rbp r9 r14 r15 edx r10d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.20:                        # Preds ..B6.19 ..B6.18 ..B6.11
        lea       4(%rdx), %eax                                 #4241.13
        cmpl      %eax, %r10d                                   #4216.28
        jb        ..B6.26       # Prob 20%                      #4216.28
                                # LOE rcx rbp r9 r14 r15 edx r10d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.21:                        # Preds ..B6.20
        cmpq      %r9, %rcx                                     #4237.25
        jae       ..B6.26       # Prob 4%                       #4237.25
                                # LOE rcx rbp r9 r14 r15 edx r10d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.22:                        # Preds ..B6.21
        addl      $32, %edx                                     #4241.13
        cmpl      %r10d, %edx                                   #4242.24
        ja        ..B6.26       # Prob 4%                       #4242.24
                                # LOE rcx rbp r9 r14 r15 edx r10d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.23:                        # Preds ..B6.22
        movzbl    (%rcx), %ebx                                  #4247.22
        movzbl    1(%rcx), %edi                                 #4247.22
        movzbl    2(%rcx), %r11d                                #4247.22
        movzbl    3(%rcx), %r13d                                #4247.22
        lea       expand_2na.0(%rip), %rax                      #4247.22
        movzwl    (%rax,%rbx,2), %esi                           #4247.22
        movzwl    (%rax,%rdi,2), %r8d                           #4247.22
        movzwl    (%rax,%r11,2), %r12d                          #4247.22
        movzwl    (%rax,%r13,2), %ebx                           #4247.22
        movw      %si, -16(%rsp)                                #4247.22
        movzbl    4(%rcx), %esi                                 #4247.22
        movzwl    (%rax,%rsi,2), %edi                           #4247.22
        movw      %r8w, -14(%rsp)                               #4247.22
        movzbl    5(%rcx), %r8d                                 #4247.22
        movzwl    (%rax,%r8,2), %r11d                           #4247.22
        movw      %r12w, -12(%rsp)                              #4247.22
        movzbl    6(%rcx), %r12d                                #4247.22
        movzwl    (%rax,%r12,2), %r13d                          #4247.22
        movw      %bx, -10(%rsp)                                #4247.22
        movzbl    7(%rcx), %ebx                                 #4247.22
        movzwl    (%rax,%rbx,2), %eax                           #4247.22
        movw      %di, -8(%rsp)                                 #4247.22
        addq      $8, %rcx                                      #4262.13
        movw      %r11w, -6(%rsp)                               #4247.22
        movw      %r13w, -4(%rsp)                               #4247.22
        movw      %ax, -2(%rsp)                                 #4247.22
        movdqu    -16(%rsp), %xmm0                              #4247.22
                                # LOE rcx rbp r9 r14 r15 edx r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.24:                        # Preds ..B6.1 ..B6.4 ..B6.23
        movdqa    %xmm0, %xmm10                                 #4140.22
        movdqa    %xmm0, %xmm9                                  #4141.22
        pand      %xmm8, %xmm10                                 #4140.22
        pand      %xmm7, %xmm9                                  #4141.22
        pcmpeqw   %xmm9, %xmm10                                 #4142.22
        pmovmskb  %xmm10, %esi                                  #4143.22
        jmp       ..B6.9        # Prob 100%                     #4143.22
                                # LOE rcx rbp r9 r14 r15 edx esi r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B6.26:                        # Preds ..B6.20 ..B6.21 ..B6.22 # Infreq
        xorl      %eax, %eax                                    #4273.12
..___tag_value_eval_4na_16.413:                                 #4273.12
        popq      %rbx                                          #4273.12
..___tag_value_eval_4na_16.415:                                 #
        popq      %r12                                          #4273.12
..___tag_value_eval_4na_16.417:                                 #
        popq      %r13                                          #4273.12
..___tag_value_eval_4na_16.419:                                 #
        ret                                                     #4273.12
..___tag_value_eval_4na_16.420:                                 #
                                # LOE
..B6.29:                        # Preds ..B6.19 ..B6.17 ..B6.15 ..B6.13 # Infreq
        movl      $1, %eax                                      #4205.63
..___tag_value_eval_4na_16.424:                                 #4205.63
        popq      %rbx                                          #4205.63
..___tag_value_eval_4na_16.426:                                 #
        popq      %r12                                          #4205.63
..___tag_value_eval_4na_16.428:                                 #
        popq      %r13                                          #4205.63
..___tag_value_eval_4na_16.430:                                 #
        ret                                                     #4205.63
        .align    16,0x90
..___tag_value_eval_4na_16.431:                                 #
                                # LOE
# mark_end;
	.type	eval_4na_16,@function
	.size	eval_4na_16,.-eval_4na_16
	.data
# -- End  eval_4na_16
	.text
# -- Begin  eval_2na_128
# mark_begin;
       .align    16,0x90
eval_2na_128:
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %edx
# parameter 4: %ecx
..B7.1:                         # Preds ..B7.0
..___tag_value_eval_2na_128.432:                                #3369.1
        pushq     %r15                                          #3369.1
..___tag_value_eval_2na_128.434:                                #
        pushq     %r12                                          #3369.1
..___tag_value_eval_2na_128.436:                                #
        pushq     %rbx                                          #3369.1
..___tag_value_eval_2na_128.438:                                #
        lea       (%rdx,%rcx), %r15d                            #3406.5
        movl      %edx, %eax                                    #3409.49
        shrl      $2, %eax                                      #3409.49
        subl      4(%rdi), %r15d                                #3412.12
        lea       (%rsi,%rax), %rbx                             #3409.30
        lea       3(%rdx,%rcx), %r11d                           #3415.50
        shrl      $2, %r11d                                     #3415.57
        addq      %rsi, %r11                                    #3415.30
        testq     $15, %rbx                                     #3418.14
        jne       ..B7.3        # Prob 50%                      #3418.14
                                # LOE rax rbx rbp rsi rdi r11 r12 r13 r14 edx r15d
..B7.2:                         # Preds ..B7.1
        movdqa    (%rbx), %xmm0                                 #3418.14
        jmp       ..B7.4        # Prob 100%                     #3418.14
                                # LOE rax rbp rsi rdi r11 r12 r13 r14 edx r15d xmm0
..B7.3:                         # Preds ..B7.1
        movdqu    (%rbx), %xmm0                                 #3418.14
                                # LOE rax rbp rsi rdi r11 r12 r13 r14 edx r15d xmm0
..B7.4:                         # Preds ..B7.2 ..B7.3
        lea       16(%rsi,%rax), %r10                           #3419.5
        movq      %r10, %r9                                     #3419.5
        cmpq      %r11, %r10                                    #3424.16
        jae       ..B7.6        # Prob 12%                      #3424.16
                                # LOE rbp rdi r9 r10 r11 r12 r13 r14 edx r8d r15d xmm0
..B7.5:                         # Preds ..B7.4
        movzbl    -1(%r10), %r8d                                #3425.24
        shll      $8, %r8d                                      #3425.38
                                # LOE rbp rdi r9 r10 r11 r12 r13 r14 edx r8d r15d xmm0
..B7.6:                         # Preds ..B7.5 ..B7.4
        movdqa    16(%rdi), %xmm8                               #3429.5
        movdqa    32(%rdi), %xmm7                               #3429.5
        movdqa    48(%rdi), %xmm6                               #3429.5
        movdqa    64(%rdi), %xmm5                               #3429.5
        movdqa    80(%rdi), %xmm4                               #3429.5
        movdqa    96(%rdi), %xmm3                               #3429.5
        movdqa    112(%rdi), %xmm2                              #3429.5
        movdqa    128(%rdi), %xmm1                              #3429.5
        xorl      %ebx, %ebx                                    #3432.15
        xorl      %esi, %esi                                    #3432.10
        xorl      %ecx, %ecx                                    #3432.5
        movl      %r15d, %eax                                   #3434.20
        subl      %edx, %eax                                    #3434.20
        addl      $7, %eax                                      #3434.33
        shrl      $2, %eax                                      #3434.40
        movl      %edx, %edi                                    #3441.20
        andl      $3, %edi                                      #3441.20
        je        ..B7.29       # Prob 20%                      #3441.20
                                # LOE rbp r9 r10 r11 r12 r13 r14 eax edx ecx ebx esi edi r8d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.7:                         # Preds ..B7.6
        cmpl      $1, %edi                                      #3441.20
        je        ..B7.14       # Prob 25%                      #3441.20
                                # LOE rbp r9 r10 r11 r12 r13 r14 eax edx ecx ebx esi edi r8d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.8:                         # Preds ..B7.7
        cmpl      $2, %edi                                      #3441.20
        je        ..B7.15       # Prob 33%                      #3441.20
                                # LOE rbp r9 r10 r11 r12 r13 r14 eax edx ecx ebx esi edi r8d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.9:                         # Preds ..B7.8
        cmpl      $3, %edi                                      #3441.20
        je        ..B7.16       # Prob 50%                      #3441.20
        jmp       ..B7.25       # Prob 100%                     #3441.20
                                # LOE rbp r9 r10 r11 r12 r13 r14 eax edx ecx ebx esi r8d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.14:                        # Preds ..B7.7 ..B7.29
        movdqa    %xmm0, %xmm9                                  #3464.22
        pand      %xmm5, %xmm9                                  #3464.22
        pcmpeqd   %xmm6, %xmm9                                  #3465.22
        pmovmskb  %xmm9, %esi                                   #3466.22
        incl      %esi                                          #3467.17
        shrl      $16, %esi                                     #3467.17
        negl      %esi                                          #3467.17
                                # LOE rbp r9 r10 r11 r13 r14 eax edx ecx esi r8d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.15:                        # Preds ..B7.8 ..B7.14
        movdqa    %xmm0, %xmm9                                  #3470.22
        pand      %xmm3, %xmm9                                  #3470.22
        pcmpeqd   %xmm4, %xmm9                                  #3471.22
        pmovmskb  %xmm9, %ebx                                   #3472.22
        incl      %ebx                                          #3473.17
        shrl      $16, %ebx                                     #3473.17
        negl      %ebx                                          #3473.17
                                # LOE rbp r9 r10 r11 r13 r14 eax edx ecx ebx esi r8d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.16:                        # Preds ..B7.9 ..B7.15
        movdqa    %xmm0, %xmm9                                  #3476.22
        andl      $-4, %edx                                     #3483.17
        movl      %ecx, %r12d                                   #3486.29
        pand      %xmm1, %xmm9                                  #3476.22
        pcmpeqd   %xmm2, %xmm9                                  #3477.22
        pmovmskb  %xmm9, %edi                                   #3478.22
        orl       %esi, %r12d                                   #3486.29
        orl       %ebx, %r12d                                   #3486.34
        incl      %edi                                          #3479.17
        shrl      $16, %edi                                     #3479.17
        negl      %edi                                          #3479.17
        orl       %edi, %r12d                                   #3486.39
        jne       ..B7.30       # Prob 20%                      #3486.47
                                # LOE rbp r9 r10 r11 r13 r14 eax edx ecx ebx esi r8d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.17:                        # Preds ..B7.16
        addl      $4, %edx                                      #3527.17
        cmpl      %r15d, %edx                                   #3530.28
        ja        ..B7.38       # Prob 20%                      #3530.28
                                # LOE rbp r9 r10 r11 r13 r14 eax edx r8d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.18:                        # Preds ..B7.17
        decl      %eax                                          #3534.25
        jne       ..B7.26       # Prob 50%                      #3534.39
                                # LOE rbp r9 r10 r11 r13 r14 eax edx r8d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.19:                        # Preds ..B7.18
        cmpq      %r11, %r10                                    #3555.25
        jae       ..B7.38       # Prob 4%                       #3555.25
                                # LOE rbp r10 r11 r13 r14 edx r8d r15d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.20:                        # Preds ..B7.19
        testq     $15, %r10                                     #3565.22
        jne       ..B7.22       # Prob 50%                      #3565.22
                                # LOE rbp r10 r11 r13 r14 edx r8d r15d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.21:                        # Preds ..B7.20
        movdqa    (%r10), %xmm0                                 #3565.22
        jmp       ..B7.23       # Prob 100%                     #3565.22
                                # LOE rbp r10 r11 r13 r14 edx r8d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.22:                        # Preds ..B7.20
        movdqu    (%r10), %xmm0                                 #3565.22
                                # LOE rbp r10 r11 r13 r14 edx r8d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.23:                        # Preds ..B7.21 ..B7.22
        addq      $16, %r10                                     #3643.13
        movq      %r10, %r9                                     #3643.13
        cmpq      %r11, %r10                                    #3646.24
        jae       ..B7.25       # Prob 12%                      #3646.24
                                # LOE rbp r9 r10 r11 r13 r14 edx r8d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.24:                        # Preds ..B7.23
        movzbl    -1(%r10), %r8d                                #3647.32
        shll      $8, %r8d                                      #3647.46
                                # LOE rbp r9 r10 r11 r13 r14 edx r8d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.25:                        # Preds ..B7.9 ..B7.23 ..B7.24
        movl      $16, %eax                                     #3448.13
        jmp       ..B7.29       # Prob 100%                     #3448.13
                                # LOE rbp r9 r10 r11 r13 r14 eax edx r8d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.26:                        # Preds ..B7.18
        psrldq    $1, %xmm0                                     #3539.26
        cmpq      %r11, %r9                                     #3542.26
        jae       ..B7.28       # Prob 19%                      #3542.26
                                # LOE rbp r9 r10 r11 r13 r14 eax edx r8d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.27:                        # Preds ..B7.26
        movzbl    (%r9), %ecx                                   #3545.37
        sarl      $8, %r8d                                      #3544.21
        shll      $8, %ecx                                      #3545.48
        orl       %ecx, %r8d                                    #3545.21
        pinsrw    $7, %r8d, %xmm0                               #3546.30
                                # LOE rbp r9 r10 r11 r13 r14 eax edx r8d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.28:                        # Preds ..B7.27 ..B7.26
        incq      %r9                                           #3550.20
                                # LOE rbp r9 r10 r11 r13 r14 eax edx r8d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.29:                        # Preds ..B7.6 ..B7.25 ..B7.28
        movdqa    %xmm0, %xmm9                                  #3458.22
        pand      %xmm7, %xmm9                                  #3458.22
        pcmpeqd   %xmm8, %xmm9                                  #3459.22
        pmovmskb  %xmm9, %ecx                                   #3460.22
        incl      %ecx                                          #3461.17
        shrl      $16, %ecx                                     #3461.17
        negl      %ecx                                          #3461.17
        jmp       ..B7.14       # Prob 100%                     #3461.17
                                # LOE rbp r9 r10 r11 r13 r14 eax edx ecx r8d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B7.30:                        # Preds ..B7.16                 # Infreq
        subl      %edx, %r15d                                   #3489.30
        cmpl      $2, %r15d                                     #3489.37
        je        ..B7.34       # Prob 25%                      #3489.37
                                # LOE rbp r12 r13 r14 ecx ebx esi r15d
..B7.31:                        # Preds ..B7.30                 # Infreq
        cmpl      $1, %r15d                                     #3489.37
        je        ..B7.35       # Prob 33%                      #3489.37
                                # LOE rbp r12 r13 r14 ecx esi r15d
..B7.32:                        # Preds ..B7.31                 # Infreq
        testl     %r15d, %r15d                                  #3489.37
        je        ..B7.36       # Prob 50%                      #3489.37
                                # LOE rbp r12 r13 r14 ecx
..B7.33:                        # Preds ..B7.32                 # Infreq
        movl      $1, %eax                                      #3492.32
..___tag_value_eval_2na_128.441:                                #3492.32
        popq      %rbx                                          #3492.32
..___tag_value_eval_2na_128.442:                                #
        popq      %r12                                          #3492.32
..___tag_value_eval_2na_128.444:                                #
        popq      %r15                                          #3492.32
..___tag_value_eval_2na_128.446:                                #
        ret                                                     #3492.32
..___tag_value_eval_2na_128.447:                                #
                                # LOE
..B7.34:                        # Preds ..B7.30                 # Infreq
        testl     %ebx, %ebx                                    #3494.36
        jne       ..B7.37       # Prob 28%                      #3494.36
                                # LOE rbp r12 r13 r14 ecx esi
..B7.35:                        # Preds ..B7.34 ..B7.31         # Infreq
        testl     %esi, %esi                                    #3496.36
        jne       ..B7.37       # Prob 28%                      #3496.36
                                # LOE rbp r12 r13 r14 ecx
..B7.36:                        # Preds ..B7.35 ..B7.32         # Infreq
        testl     %ecx, %ecx                                    #3498.36
        je        ..B7.38       # Prob 50%                      #3498.36
                                # LOE rbp r12 r13 r14
..B7.37:                        # Preds ..B7.34 ..B7.35 ..B7.36 # Infreq
        movl      $1, %eax                                      #3498.47
..___tag_value_eval_2na_128.451:                                #3498.47
        popq      %rbx                                          #3498.47
..___tag_value_eval_2na_128.452:                                #
        popq      %r12                                          #3498.47
..___tag_value_eval_2na_128.454:                                #
        popq      %r15                                          #3498.47
..___tag_value_eval_2na_128.456:                                #
        ret                                                     #3498.47
..___tag_value_eval_2na_128.457:                                #
                                # LOE
..B7.38:                        # Preds ..B7.17 ..B7.19 ..B7.36 # Infreq
        xorl      %eax, %eax                                    #3500.28
..___tag_value_eval_2na_128.461:                                #3500.28
        popq      %rbx                                          #3500.28
..___tag_value_eval_2na_128.462:                                #
        popq      %r12                                          #3500.28
..___tag_value_eval_2na_128.464:                                #
        popq      %r15                                          #3500.28
..___tag_value_eval_2na_128.466:                                #
        ret                                                     #3500.28
        .align    16,0x90
..___tag_value_eval_2na_128.467:                                #
                                # LOE
# mark_end;
	.type	eval_2na_128,@function
	.size	eval_2na_128,.-eval_2na_128
	.data
# -- End  eval_2na_128
	.text
# -- Begin  eval_2na_32
# mark_begin;
       .align    16,0x90
eval_2na_32:
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %edx
# parameter 4: %ecx
..B8.1:                         # Preds ..B8.0
..___tag_value_eval_2na_32.468:                                 #2782.1
        pushq     %r15                                          #2782.1
..___tag_value_eval_2na_32.470:                                 #
        pushq     %r14                                          #2782.1
..___tag_value_eval_2na_32.472:                                 #
        pushq     %r13                                          #2782.1
..___tag_value_eval_2na_32.474:                                 #
        pushq     %r12                                          #2782.1
..___tag_value_eval_2na_32.476:                                 #
        pushq     %rbp                                          #2782.1
..___tag_value_eval_2na_32.478:                                 #
        pushq     %rbx                                          #2782.1
..___tag_value_eval_2na_32.480:                                 #
        lea       (%rdx,%rcx), %r15d                            #2819.5
        movl      %edx, %eax                                    #2822.49
        shrl      $2, %eax                                      #2822.49
        subl      4(%rdi), %r15d                                #2825.12
        lea       (%rsi,%rax), %r8                              #2822.30
        lea       3(%rdx,%rcx), %r14d                           #2828.50
        shrl      $2, %r14d                                     #2828.57
        addq      %rsi, %r14                                    #2828.30
        testq     $15, %r8                                      #2831.14
        jne       ..B8.3        # Prob 50%                      #2831.14
                                # LOE rax rbx rbp rsi rdi r8 r12 r13 r14 edx r15d
..B8.2:                         # Preds ..B8.1
        movdqa    (%r8), %xmm0                                  #2831.14
        jmp       ..B8.4        # Prob 100%                     #2831.14
                                # LOE rax rbx rbp rsi rdi r12 r13 r14 edx r15d xmm0
..B8.3:                         # Preds ..B8.1
        movdqu    (%r8), %xmm0                                  #2831.14
                                # LOE rax rbx rbp rsi rdi r12 r13 r14 edx r15d xmm0
..B8.4:                         # Preds ..B8.2 ..B8.3
        lea       16(%rsi,%rax), %r8                            #2832.5
        movq      %r8, %rsi                                     #2832.5
        cmpq      %r14, %r8                                     #2837.16
        jae       ..B8.6        # Prob 12%                      #2837.16
                                # LOE rbx rbp rsi rdi r8 r12 r13 r14 edx ecx r15d xmm0
..B8.5:                         # Preds ..B8.4
        movzbl    -1(%r8), %ecx                                 #2838.24
        shll      $8, %ecx                                      #2838.38
                                # LOE rbx rbp rsi rdi r8 r12 r13 r14 edx ecx r15d xmm0
..B8.6:                         # Preds ..B8.5 ..B8.4
        movdqa    16(%rdi), %xmm8                               #2842.5
        movdqa    32(%rdi), %xmm7                               #2842.5
        movdqa    48(%rdi), %xmm6                               #2842.5
        movdqa    64(%rdi), %xmm5                               #2842.5
        movdqa    80(%rdi), %xmm4                               #2842.5
        movdqa    96(%rdi), %xmm3                               #2842.5
        movdqa    112(%rdi), %xmm2                              #2842.5
        movdqa    128(%rdi), %xmm1                              #2842.5
        xorl      %r10d, %r10d                                  #2845.15
        xorl      %r11d, %r11d                                  #2845.10
        xorl      %r9d, %r9d                                    #2845.5
        movl      $4, %edi                                      #2849.5
        movl      %edx, %eax                                    #2854.20
        andl      $3, %eax                                      #2854.20
        je        ..B8.38       # Prob 20%                      #2854.20
                                # LOE rbx rbp rsi r8 r12 r13 r14 eax edx ecx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.7:                         # Preds ..B8.6
        cmpl      $1, %eax                                      #2854.20
        je        ..B8.14       # Prob 25%                      #2854.20
                                # LOE rbx rbp rsi r8 r12 r13 r14 eax edx ecx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.8:                         # Preds ..B8.7
        cmpl      $2, %eax                                      #2854.20
        je        ..B8.15       # Prob 33%                      #2854.20
                                # LOE rbx rbp rsi r8 r12 r13 r14 eax edx ecx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.9:                         # Preds ..B8.8
        cmpl      $3, %eax                                      #2854.20
        je        ..B8.16       # Prob 50%                      #2854.20
        jmp       ..B8.34       # Prob 100%                     #2854.20
                                # LOE rbx rbp rsi r8 r12 r13 r14 edx ecx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.14:                        # Preds ..B8.7 ..B8.38
        movdqa    %xmm0, %xmm9                                  #2877.22
        pand      %xmm5, %xmm9                                  #2877.22
        pcmpeqd   %xmm6, %xmm9                                  #2878.22
        pmovmskb  %xmm9, %r11d                                  #2879.22
                                # LOE rsi r8 r14 edx ecx edi r9d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.15:                        # Preds ..B8.8 ..B8.14
        movdqa    %xmm0, %xmm9                                  #2883.22
        pand      %xmm3, %xmm9                                  #2883.22
        pcmpeqd   %xmm4, %xmm9                                  #2884.22
        pmovmskb  %xmm9, %r10d                                  #2885.22
                                # LOE rsi r8 r14 edx ecx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.16:                        # Preds ..B8.9 ..B8.15
        movdqa    %xmm0, %xmm9                                  #2889.22
        movl      %edx, %ebx                                    #2896.17
        andl      $-4, %ebx                                     #2896.17
        pand      %xmm1, %xmm9                                  #2889.22
        pcmpeqd   %xmm2, %xmm9                                  #2890.22
        pmovmskb  %xmm9, %ebp                                   #2891.22
        movl      %r9d, %edx                                    #2899.29
        orl       %r11d, %edx                                   #2899.29
        orl       %r10d, %edx                                   #2899.34
        orl       %ebp, %edx                                    #2899.39
        je        ..B8.25       # Prob 78%                      #2899.47
                                # LOE rsi r8 r14 ecx ebx ebp edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.17:                        # Preds ..B8.16
        movl      %r9d, %eax                                    #2917.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #2917.30
        movswq    %ax, %r13                                     #2917.30
        movl      %r11d, %eax                                   #2918.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #2918.30
        movswq    %ax, %r12                                     #2918.30
        movl      %r10d, %eax                                   #2919.30
        shll      $2, %r13d                                     #2925.34
        shll      $2, %r12d                                     #2926.34
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #2919.30
        movswq    %ax, %rdx                                     #2919.30
        movl      %ebp, %eax                                    #2920.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #2920.30
        movswq    %ax, %rax                                     #2920.30
        shll      $2, %edx                                      #2927.34
        shll      $2, %eax                                      #2928.34
        testl     %r9d, %r9d                                    #2932.32
        je        ..B8.19       # Prob 50%                      #2932.32
                                # LOE rsi r8 r14 eax edx ecx ebx ebp edi r10d r11d r12d r13d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.18:                        # Preds ..B8.17
        addl      %ebx, %r13d                                   #2932.43
        cmpl      %r13d, %r15d                                  #2932.49
        jae       ..B8.43       # Prob 20%                      #2932.49
                                # LOE rsi r8 r14 eax edx ecx ebx ebp edi r10d r11d r12d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.19:                        # Preds ..B8.18 ..B8.17
        testl     %r11d, %r11d                                  #2933.32
        je        ..B8.21       # Prob 50%                      #2933.32
                                # LOE rsi r8 r14 eax edx ecx ebx ebp edi r10d r12d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.20:                        # Preds ..B8.19
        lea       1(%rbx,%r12), %r9d                            #2933.43
        cmpl      %r9d, %r15d                                   #2933.49
        jae       ..B8.43       # Prob 20%                      #2933.49
                                # LOE rsi r8 r14 eax edx ecx ebx ebp edi r10d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.21:                        # Preds ..B8.20 ..B8.19
        testl     %r10d, %r10d                                  #2934.32
        je        ..B8.23       # Prob 50%                      #2934.32
                                # LOE rsi r8 r14 eax edx ecx ebx ebp edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.22:                        # Preds ..B8.21
        lea       2(%rbx,%rdx), %edx                            #2934.43
        cmpl      %edx, %r15d                                   #2934.49
        jae       ..B8.43       # Prob 20%                      #2934.49
                                # LOE rsi r8 r14 eax ecx ebx ebp edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.23:                        # Preds ..B8.22 ..B8.21
        testl     %ebp, %ebp                                    #2935.32
        je        ..B8.25       # Prob 50%                      #2935.32
                                # LOE rsi r8 r14 eax ecx ebx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.24:                        # Preds ..B8.23
        lea       3(%rbx,%rax), %eax                            #2935.43
        cmpl      %eax, %r15d                                   #2935.49
        jae       ..B8.43       # Prob 20%                      #2935.49
                                # LOE rsi r8 r14 ecx ebx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.25:                        # Preds ..B8.24 ..B8.23 ..B8.16
        lea       4(%rbx), %edx                                 #2940.17
        cmpl      %r15d, %edx                                   #2943.28
        ja        ..B8.40       # Prob 20%                      #2943.28
                                # LOE rsi r8 r14 edx ecx ebx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.26:                        # Preds ..B8.25
        decl      %edi                                          #2947.25
        jne       ..B8.35       # Prob 50%                      #2947.39
                                # LOE rsi r8 r14 edx ecx ebx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.27:                        # Preds ..B8.26
        cmpq      %r14, %r8                                     #2968.25
        jae       ..B8.40       # Prob 4%                       #2968.25
                                # LOE r8 r14 ecx ebx r15d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.28:                        # Preds ..B8.27
        lea       52(%rbx), %edx                                #2972.13
        cmpl      %r15d, %edx                                   #2973.24
        ja        ..B8.40       # Prob 4%                       #2973.24
                                # LOE r8 r14 edx ecx r15d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.29:                        # Preds ..B8.28
        testq     $15, %r8                                      #2978.22
        jne       ..B8.31       # Prob 50%                      #2978.22
                                # LOE r8 r14 edx ecx r15d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.30:                        # Preds ..B8.29
        movdqa    (%r8), %xmm0                                  #2978.22
        jmp       ..B8.32       # Prob 100%                     #2978.22
                                # LOE r8 r14 edx ecx r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.31:                        # Preds ..B8.29
        movdqu    (%r8), %xmm0                                  #2978.22
                                # LOE r8 r14 edx ecx r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.32:                        # Preds ..B8.30 ..B8.31
        addq      $16, %r8                                      #3056.13
        movq      %r8, %rsi                                     #3056.13
        cmpq      %r14, %r8                                     #3059.24
        jae       ..B8.34       # Prob 12%                      #3059.24
                                # LOE rsi r8 r14 edx ecx r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.33:                        # Preds ..B8.32
        movzbl    -1(%r8), %ecx                                 #3060.32
        shll      $8, %ecx                                      #3060.46
                                # LOE rsi r8 r14 edx ecx r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.34:                        # Preds ..B8.9 ..B8.32 ..B8.33
        movl      $4, %edi                                      #2861.13
        jmp       ..B8.38       # Prob 100%                     #2861.13
                                # LOE rsi r8 r14 edx ecx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.35:                        # Preds ..B8.26
        psrldq    $1, %xmm0                                     #2952.26
        cmpq      %r14, %rsi                                    #2955.26
        jae       ..B8.37       # Prob 19%                      #2955.26
                                # LOE rsi r8 r14 edx ecx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.36:                        # Preds ..B8.35
        movzbl    (%rsi), %eax                                  #2958.37
        sarl      $8, %ecx                                      #2957.21
        shll      $8, %eax                                      #2958.48
        orl       %eax, %ecx                                    #2958.21
        pinsrw    $7, %ecx, %xmm0                               #2959.30
                                # LOE rsi r8 r14 edx ecx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.37:                        # Preds ..B8.36 ..B8.35
        incq      %rsi                                          #2963.20
                                # LOE rsi r8 r14 edx ecx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.38:                        # Preds ..B8.6 ..B8.34 ..B8.37
        movdqa    %xmm0, %xmm9                                  #2871.22
        pand      %xmm7, %xmm9                                  #2871.22
        pcmpeqd   %xmm8, %xmm9                                  #2872.22
        pmovmskb  %xmm9, %r9d                                   #2873.22
        jmp       ..B8.14       # Prob 100%                     #2873.22
                                # LOE rsi r8 r14 edx ecx edi r9d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B8.40:                        # Preds ..B8.25 ..B8.27 ..B8.28 # Infreq
        xorl      %eax, %eax                                    #3069.12
..___tag_value_eval_2na_32.483:                                 #3069.12
        popq      %rbx                                          #3069.12
..___tag_value_eval_2na_32.485:                                 #
        popq      %rbp                                          #3069.12
..___tag_value_eval_2na_32.487:                                 #
        popq      %r12                                          #3069.12
..___tag_value_eval_2na_32.489:                                 #
        popq      %r13                                          #3069.12
..___tag_value_eval_2na_32.491:                                 #
        popq      %r14                                          #3069.12
..___tag_value_eval_2na_32.493:                                 #
        popq      %r15                                          #3069.12
..___tag_value_eval_2na_32.495:                                 #
        ret                                                     #3069.12
..___tag_value_eval_2na_32.496:                                 #
                                # LOE
..B8.43:                        # Preds ..B8.22 ..B8.20 ..B8.18 ..B8.24 # Infreq
        movl      $1, %eax                                      #2935.63
..___tag_value_eval_2na_32.503:                                 #2935.63
        popq      %rbx                                          #2935.63
..___tag_value_eval_2na_32.505:                                 #
        popq      %rbp                                          #2935.63
..___tag_value_eval_2na_32.507:                                 #
        popq      %r12                                          #2935.63
..___tag_value_eval_2na_32.509:                                 #
        popq      %r13                                          #2935.63
..___tag_value_eval_2na_32.511:                                 #
        popq      %r14                                          #2935.63
..___tag_value_eval_2na_32.513:                                 #
        popq      %r15                                          #2935.63
..___tag_value_eval_2na_32.515:                                 #
        ret                                                     #2935.63
        .align    16,0x90
..___tag_value_eval_2na_32.516:                                 #
                                # LOE
# mark_end;
	.type	eval_2na_32,@function
	.size	eval_2na_32,.-eval_2na_32
	.data
# -- End  eval_2na_32
	.text
# -- Begin  eval_2na_16
# mark_begin;
       .align    16,0x90
eval_2na_16:
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %edx
# parameter 4: %ecx
..B9.1:                         # Preds ..B9.0
..___tag_value_eval_2na_16.517:                                 #2489.1
        pushq     %r15                                          #2489.1
..___tag_value_eval_2na_16.519:                                 #
        pushq     %r14                                          #2489.1
..___tag_value_eval_2na_16.521:                                 #
        pushq     %r13                                          #2489.1
..___tag_value_eval_2na_16.523:                                 #
        pushq     %r12                                          #2489.1
..___tag_value_eval_2na_16.525:                                 #
        pushq     %rbp                                          #2489.1
..___tag_value_eval_2na_16.527:                                 #
        pushq     %rbx                                          #2489.1
..___tag_value_eval_2na_16.529:                                 #
        lea       (%rdx,%rcx), %r15d                            #2526.5
        movl      %edx, %eax                                    #2529.49
        shrl      $2, %eax                                      #2529.49
        subl      4(%rdi), %r15d                                #2532.12
        lea       (%rsi,%rax), %r8                              #2529.30
        lea       3(%rdx,%rcx), %r14d                           #2535.50
        shrl      $2, %r14d                                     #2535.57
        addq      %rsi, %r14                                    #2535.30
        testq     $15, %r8                                      #2538.14
        jne       ..B9.3        # Prob 50%                      #2538.14
                                # LOE rax rbx rbp rsi rdi r8 r12 r13 r14 edx r15d
..B9.2:                         # Preds ..B9.1
        movdqa    (%r8), %xmm0                                  #2538.14
        jmp       ..B9.4        # Prob 100%                     #2538.14
                                # LOE rax rbx rbp rsi rdi r12 r13 r14 edx r15d xmm0
..B9.3:                         # Preds ..B9.1
        movdqu    (%r8), %xmm0                                  #2538.14
                                # LOE rax rbx rbp rsi rdi r12 r13 r14 edx r15d xmm0
..B9.4:                         # Preds ..B9.2 ..B9.3
        lea       16(%rsi,%rax), %r8                            #2539.5
        movq      %r8, %rsi                                     #2539.5
        cmpq      %r14, %r8                                     #2544.16
        jae       ..B9.6        # Prob 12%                      #2544.16
                                # LOE rbx rbp rsi rdi r8 r12 r13 r14 edx ecx r15d xmm0
..B9.5:                         # Preds ..B9.4
        movzbl    -1(%r8), %ecx                                 #2545.24
        shll      $8, %ecx                                      #2545.38
                                # LOE rbx rbp rsi rdi r8 r12 r13 r14 edx ecx r15d xmm0
..B9.6:                         # Preds ..B9.5 ..B9.4
        movdqa    16(%rdi), %xmm8                               #2549.5
        movdqa    32(%rdi), %xmm7                               #2549.5
        movdqa    48(%rdi), %xmm6                               #2549.5
        movdqa    64(%rdi), %xmm5                               #2549.5
        movdqa    80(%rdi), %xmm4                               #2549.5
        movdqa    96(%rdi), %xmm3                               #2549.5
        movdqa    112(%rdi), %xmm2                              #2549.5
        movdqa    128(%rdi), %xmm1                              #2549.5
        xorl      %r10d, %r10d                                  #2552.15
        xorl      %r11d, %r11d                                  #2552.10
        xorl      %r9d, %r9d                                    #2552.5
        movl      $2, %edi                                      #2556.5
        movl      %edx, %eax                                    #2561.20
        andl      $3, %eax                                      #2561.20
        je        ..B9.38       # Prob 20%                      #2561.20
                                # LOE rbx rbp rsi r8 r12 r13 r14 eax edx ecx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.7:                         # Preds ..B9.6
        cmpl      $1, %eax                                      #2561.20
        je        ..B9.14       # Prob 25%                      #2561.20
                                # LOE rbx rbp rsi r8 r12 r13 r14 eax edx ecx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.8:                         # Preds ..B9.7
        cmpl      $2, %eax                                      #2561.20
        je        ..B9.15       # Prob 33%                      #2561.20
                                # LOE rbx rbp rsi r8 r12 r13 r14 eax edx ecx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.9:                         # Preds ..B9.8
        cmpl      $3, %eax                                      #2561.20
        je        ..B9.16       # Prob 50%                      #2561.20
        jmp       ..B9.34       # Prob 100%                     #2561.20
                                # LOE rbx rbp rsi r8 r12 r13 r14 edx ecx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.14:                        # Preds ..B9.7 ..B9.38
        movdqa    %xmm0, %xmm9                                  #2584.22
        pand      %xmm5, %xmm9                                  #2584.22
        pcmpeqw   %xmm6, %xmm9                                  #2585.22
        pmovmskb  %xmm9, %r11d                                  #2586.22
                                # LOE rsi r8 r14 edx ecx edi r9d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.15:                        # Preds ..B9.8 ..B9.14
        movdqa    %xmm0, %xmm9                                  #2590.22
        pand      %xmm3, %xmm9                                  #2590.22
        pcmpeqw   %xmm4, %xmm9                                  #2591.22
        pmovmskb  %xmm9, %r10d                                  #2592.22
                                # LOE rsi r8 r14 edx ecx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.16:                        # Preds ..B9.9 ..B9.15
        movdqa    %xmm0, %xmm9                                  #2596.22
        movl      %edx, %ebx                                    #2603.17
        andl      $-4, %ebx                                     #2603.17
        pand      %xmm1, %xmm9                                  #2596.22
        pcmpeqw   %xmm2, %xmm9                                  #2597.22
        pmovmskb  %xmm9, %ebp                                   #2598.22
        movl      %r9d, %edx                                    #2606.29
        orl       %r11d, %edx                                   #2606.29
        orl       %r10d, %edx                                   #2606.34
        orl       %ebp, %edx                                    #2606.39
        je        ..B9.25       # Prob 78%                      #2606.47
                                # LOE rsi r8 r14 ecx ebx ebp edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.17:                        # Preds ..B9.16
        movl      %r9d, %eax                                    #2624.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #2624.30
        movswq    %ax, %r13                                     #2624.30
        movl      %r11d, %eax                                   #2625.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #2625.30
        movswq    %ax, %r12                                     #2625.30
        movl      %r10d, %eax                                   #2626.30
        shll      $2, %r13d                                     #2632.34
        shll      $2, %r12d                                     #2633.34
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #2626.30
        movswq    %ax, %rdx                                     #2626.30
        movl      %ebp, %eax                                    #2627.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #2627.30
        movswq    %ax, %rax                                     #2627.30
        shll      $2, %edx                                      #2634.34
        shll      $2, %eax                                      #2635.34
        testl     %r9d, %r9d                                    #2639.32
        je        ..B9.19       # Prob 50%                      #2639.32
                                # LOE rsi r8 r14 eax edx ecx ebx ebp edi r10d r11d r12d r13d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.18:                        # Preds ..B9.17
        addl      %ebx, %r13d                                   #2639.43
        cmpl      %r13d, %r15d                                  #2639.49
        jae       ..B9.43       # Prob 20%                      #2639.49
                                # LOE rsi r8 r14 eax edx ecx ebx ebp edi r10d r11d r12d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.19:                        # Preds ..B9.18 ..B9.17
        testl     %r11d, %r11d                                  #2640.32
        je        ..B9.21       # Prob 50%                      #2640.32
                                # LOE rsi r8 r14 eax edx ecx ebx ebp edi r10d r12d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.20:                        # Preds ..B9.19
        lea       1(%rbx,%r12), %r9d                            #2640.43
        cmpl      %r9d, %r15d                                   #2640.49
        jae       ..B9.43       # Prob 20%                      #2640.49
                                # LOE rsi r8 r14 eax edx ecx ebx ebp edi r10d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.21:                        # Preds ..B9.20 ..B9.19
        testl     %r10d, %r10d                                  #2641.32
        je        ..B9.23       # Prob 50%                      #2641.32
                                # LOE rsi r8 r14 eax edx ecx ebx ebp edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.22:                        # Preds ..B9.21
        lea       2(%rbx,%rdx), %edx                            #2641.43
        cmpl      %edx, %r15d                                   #2641.49
        jae       ..B9.43       # Prob 20%                      #2641.49
                                # LOE rsi r8 r14 eax ecx ebx ebp edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.23:                        # Preds ..B9.22 ..B9.21
        testl     %ebp, %ebp                                    #2642.32
        je        ..B9.25       # Prob 50%                      #2642.32
                                # LOE rsi r8 r14 eax ecx ebx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.24:                        # Preds ..B9.23
        lea       3(%rbx,%rax), %eax                            #2642.43
        cmpl      %eax, %r15d                                   #2642.49
        jae       ..B9.43       # Prob 20%                      #2642.49
                                # LOE rsi r8 r14 ecx ebx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.25:                        # Preds ..B9.24 ..B9.23 ..B9.16
        lea       4(%rbx), %edx                                 #2647.17
        cmpl      %r15d, %edx                                   #2650.28
        ja        ..B9.40       # Prob 20%                      #2650.28
                                # LOE rsi r8 r14 edx ecx ebx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.26:                        # Preds ..B9.25
        decl      %edi                                          #2654.25
        jne       ..B9.35       # Prob 50%                      #2654.39
                                # LOE rsi r8 r14 edx ecx ebx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.27:                        # Preds ..B9.26
        cmpq      %r14, %r8                                     #2675.25
        jae       ..B9.40       # Prob 4%                       #2675.25
                                # LOE r8 r14 ecx ebx r15d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.28:                        # Preds ..B9.27
        lea       60(%rbx), %edx                                #2679.13
        cmpl      %r15d, %edx                                   #2680.24
        ja        ..B9.40       # Prob 4%                       #2680.24
                                # LOE r8 r14 edx ecx r15d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.29:                        # Preds ..B9.28
        testq     $15, %r8                                      #2685.22
        jne       ..B9.31       # Prob 50%                      #2685.22
                                # LOE r8 r14 edx ecx r15d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.30:                        # Preds ..B9.29
        movdqa    (%r8), %xmm0                                  #2685.22
        jmp       ..B9.32       # Prob 100%                     #2685.22
                                # LOE r8 r14 edx ecx r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.31:                        # Preds ..B9.29
        movdqu    (%r8), %xmm0                                  #2685.22
                                # LOE r8 r14 edx ecx r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.32:                        # Preds ..B9.30 ..B9.31
        addq      $16, %r8                                      #2763.13
        movq      %r8, %rsi                                     #2763.13
        cmpq      %r14, %r8                                     #2766.24
        jae       ..B9.34       # Prob 12%                      #2766.24
                                # LOE rsi r8 r14 edx ecx r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.33:                        # Preds ..B9.32
        movzbl    -1(%r8), %ecx                                 #2767.32
        shll      $8, %ecx                                      #2767.46
                                # LOE rsi r8 r14 edx ecx r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.34:                        # Preds ..B9.9 ..B9.32 ..B9.33
        movl      $2, %edi                                      #2568.13
        jmp       ..B9.38       # Prob 100%                     #2568.13
                                # LOE rsi r8 r14 edx ecx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.35:                        # Preds ..B9.26
        psrldq    $1, %xmm0                                     #2659.26
        cmpq      %r14, %rsi                                    #2662.26
        jae       ..B9.37       # Prob 19%                      #2662.26
                                # LOE rsi r8 r14 edx ecx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.36:                        # Preds ..B9.35
        movzbl    (%rsi), %eax                                  #2665.37
        sarl      $8, %ecx                                      #2664.21
        shll      $8, %eax                                      #2665.48
        orl       %eax, %ecx                                    #2665.21
        pinsrw    $7, %ecx, %xmm0                               #2666.30
                                # LOE rsi r8 r14 edx ecx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.37:                        # Preds ..B9.36 ..B9.35
        incq      %rsi                                          #2670.20
                                # LOE rsi r8 r14 edx ecx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.38:                        # Preds ..B9.6 ..B9.34 ..B9.37
        movdqa    %xmm0, %xmm9                                  #2578.22
        pand      %xmm7, %xmm9                                  #2578.22
        pcmpeqw   %xmm8, %xmm9                                  #2579.22
        pmovmskb  %xmm9, %r9d                                   #2580.22
        jmp       ..B9.14       # Prob 100%                     #2580.22
                                # LOE rsi r8 r14 edx ecx edi r9d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B9.40:                        # Preds ..B9.25 ..B9.27 ..B9.28 # Infreq
        xorl      %eax, %eax                                    #2776.12
..___tag_value_eval_2na_16.532:                                 #2776.12
        popq      %rbx                                          #2776.12
..___tag_value_eval_2na_16.534:                                 #
        popq      %rbp                                          #2776.12
..___tag_value_eval_2na_16.536:                                 #
        popq      %r12                                          #2776.12
..___tag_value_eval_2na_16.538:                                 #
        popq      %r13                                          #2776.12
..___tag_value_eval_2na_16.540:                                 #
        popq      %r14                                          #2776.12
..___tag_value_eval_2na_16.542:                                 #
        popq      %r15                                          #2776.12
..___tag_value_eval_2na_16.544:                                 #
        ret                                                     #2776.12
..___tag_value_eval_2na_16.545:                                 #
                                # LOE
..B9.43:                        # Preds ..B9.22 ..B9.20 ..B9.18 ..B9.24 # Infreq
        movl      $1, %eax                                      #2642.63
..___tag_value_eval_2na_16.552:                                 #2642.63
        popq      %rbx                                          #2642.63
..___tag_value_eval_2na_16.554:                                 #
        popq      %rbp                                          #2642.63
..___tag_value_eval_2na_16.556:                                 #
        popq      %r12                                          #2642.63
..___tag_value_eval_2na_16.558:                                 #
        popq      %r13                                          #2642.63
..___tag_value_eval_2na_16.560:                                 #
        popq      %r14                                          #2642.63
..___tag_value_eval_2na_16.562:                                 #
        popq      %r15                                          #2642.63
..___tag_value_eval_2na_16.564:                                 #
        ret                                                     #2642.63
        .align    16,0x90
..___tag_value_eval_2na_16.565:                                 #
                                # LOE
# mark_end;
	.type	eval_2na_16,@function
	.size	eval_2na_16,.-eval_2na_16
	.data
# -- End  eval_2na_16
	.text
# -- Begin  eval_2na_8
# mark_begin;
       .align    16,0x90
eval_2na_8:
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %edx
# parameter 4: %ecx
..B10.1:                        # Preds ..B10.0
..___tag_value_eval_2na_8.566:                                  #2196.1
        pushq     %r13                                          #2196.1
..___tag_value_eval_2na_8.568:                                  #
        pushq     %r12                                          #2196.1
..___tag_value_eval_2na_8.570:                                  #
        pushq     %rbx                                          #2196.1
..___tag_value_eval_2na_8.572:                                  #
        lea       (%rdx,%rcx), %r10d                            #2233.5
        movl      %edx, %eax                                    #2236.49
        shrl      $2, %eax                                      #2236.49
        subl      4(%rdi), %r10d                                #2239.12
        lea       (%rsi,%rax), %r8                              #2236.30
        lea       3(%rdx,%rcx), %r9d                            #2242.50
        shrl      $2, %r9d                                      #2242.57
        addq      %rsi, %r9                                     #2242.30
        testq     $15, %r8                                      #2245.14
        jne       ..B10.3       # Prob 50%                      #2245.14
                                # LOE rax rbx rbp rsi rdi r8 r9 r12 r13 r14 r15 edx r10d
..B10.2:                        # Preds ..B10.1
        movdqa    (%r8), %xmm0                                  #2245.14
        jmp       ..B10.4       # Prob 100%                     #2245.14
                                # LOE rax rbx rbp rsi rdi r9 r12 r13 r14 r15 edx r10d xmm0
..B10.3:                        # Preds ..B10.1
        movdqu    (%r8), %xmm0                                  #2245.14
                                # LOE rax rbx rbp rsi rdi r9 r12 r13 r14 r15 edx r10d xmm0
..B10.4:                        # Preds ..B10.2 ..B10.3
        movdqa    16(%rdi), %xmm8                               #2256.5
        movdqa    32(%rdi), %xmm7                               #2256.5
        movdqa    48(%rdi), %xmm6                               #2256.5
        movdqa    64(%rdi), %xmm5                               #2256.5
        movdqa    80(%rdi), %xmm4                               #2256.5
        movdqa    96(%rdi), %xmm3                               #2256.5
        movdqa    112(%rdi), %xmm2                              #2256.5
        movdqa    128(%rdi), %xmm1                              #2256.5
        lea       16(%rsi,%rax), %rcx                           #2246.5
        xorl      %edi, %edi                                    #2259.15
        xorl      %r8d, %r8d                                    #2259.10
        xorl      %esi, %esi                                    #2259.5
        movl      %edx, %r11d                                   #2268.20
        andl      $3, %r11d                                     #2268.20
        je        ..B10.30      # Prob 20%                      #2268.20
                                # LOE rcx rbx rbp r9 r12 r13 r14 r15 edx esi edi r8d r10d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.5:                        # Preds ..B10.4
        cmpl      $1, %r11d                                     #2268.20
        je        ..B10.12      # Prob 25%                      #2268.20
                                # LOE rcx rbx rbp r9 r12 r13 r14 r15 edx esi edi r8d r10d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.6:                        # Preds ..B10.5
        cmpl      $2, %r11d                                     #2268.20
        je        ..B10.13      # Prob 33%                      #2268.20
                                # LOE rcx rbx rbp r9 r12 r13 r14 r15 edx esi edi r8d r10d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.7:                        # Preds ..B10.6
        cmpl      $3, %r11d                                     #2268.20
        je        ..B10.14      # Prob 50%                      #2268.20
        jmp       ..B10.30      # Prob 100%                     #2268.20
                                # LOE rcx rbx rbp r9 r12 r13 r14 r15 edx esi edi r8d r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.12:                       # Preds ..B10.5 ..B10.30
        movdqa    %xmm0, %xmm9                                  #2291.22
        pand      %xmm5, %xmm9                                  #2291.22
        pcmpeqb   %xmm6, %xmm9                                  #2292.22
        pmovmskb  %xmm9, %r8d                                   #2293.22
                                # LOE rcx rbp r9 r14 r15 edx esi r8d r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.13:                       # Preds ..B10.6 ..B10.12
        movdqa    %xmm0, %xmm9                                  #2297.22
        pand      %xmm3, %xmm9                                  #2297.22
        pcmpeqb   %xmm4, %xmm9                                  #2298.22
        pmovmskb  %xmm9, %edi                                   #2299.22
                                # LOE rcx rbp r9 r14 r15 edx esi edi r8d r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.14:                       # Preds ..B10.7 ..B10.13
        pand      %xmm1, %xmm0                                  #2303.22
        pcmpeqb   %xmm2, %xmm0                                  #2304.22
        pmovmskb  %xmm0, %r12d                                  #2305.22
        andl      $-4, %edx                                     #2310.17
        movl      %esi, %ebx                                    #2313.29
        orl       %r8d, %ebx                                    #2313.29
        orl       %edi, %ebx                                    #2313.34
        orl       %r12d, %ebx                                   #2313.39
        je        ..B10.23      # Prob 78%                      #2313.47
                                # LOE rcx rbp r9 r14 r15 edx esi edi r8d r10d r12d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.15:                       # Preds ..B10.14
        movl      %esi, %eax                                    #2331.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #2331.30
        movswq    %ax, %r13                                     #2331.30
        movl      %r8d, %eax                                    #2332.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #2332.30
        movswq    %ax, %r11                                     #2332.30
        movl      %edi, %eax                                    #2333.30
        shll      $2, %r13d                                     #2339.34
        shll      $2, %r11d                                     #2340.34
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #2333.30
        movswq    %ax, %rbx                                     #2333.30
        movl      %r12d, %eax                                   #2334.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #2334.30
        movswq    %ax, %rax                                     #2334.30
        shll      $2, %ebx                                      #2341.34
        shll      $2, %eax                                      #2342.34
        testl     %esi, %esi                                    #2346.32
        je        ..B10.17      # Prob 50%                      #2346.32
                                # LOE rcx rbp r9 r14 r15 eax edx ebx edi r8d r10d r11d r12d r13d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.16:                       # Preds ..B10.15
        addl      %edx, %r13d                                   #2346.43
        cmpl      %r13d, %r10d                                  #2346.49
        jae       ..B10.35      # Prob 20%                      #2346.49
                                # LOE rcx rbp r9 r14 r15 eax edx ebx edi r8d r10d r11d r12d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.17:                       # Preds ..B10.16 ..B10.15
        testl     %r8d, %r8d                                    #2347.32
        je        ..B10.19      # Prob 50%                      #2347.32
                                # LOE rcx rbp r9 r14 r15 eax edx ebx edi r10d r11d r12d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.18:                       # Preds ..B10.17
        lea       1(%rdx,%r11), %esi                            #2347.43
        cmpl      %esi, %r10d                                   #2347.49
        jae       ..B10.35      # Prob 20%                      #2347.49
                                # LOE rcx rbp r9 r14 r15 eax edx ebx edi r10d r12d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.19:                       # Preds ..B10.18 ..B10.17
        testl     %edi, %edi                                    #2348.32
        je        ..B10.21      # Prob 50%                      #2348.32
                                # LOE rcx rbp r9 r14 r15 eax edx ebx r10d r12d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.20:                       # Preds ..B10.19
        lea       2(%rdx,%rbx), %ebx                            #2348.43
        cmpl      %ebx, %r10d                                   #2348.49
        jae       ..B10.35      # Prob 20%                      #2348.49
                                # LOE rcx rbp r9 r14 r15 eax edx r10d r12d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.21:                       # Preds ..B10.20 ..B10.19
        testl     %r12d, %r12d                                  #2349.32
        je        ..B10.23      # Prob 50%                      #2349.32
                                # LOE rcx rbp r9 r14 r15 eax edx r10d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.22:                       # Preds ..B10.21
        lea       3(%rdx,%rax), %eax                            #2349.43
        cmpl      %eax, %r10d                                   #2349.49
        jae       ..B10.35      # Prob 20%                      #2349.49
                                # LOE rcx rbp r9 r14 r15 edx r10d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.23:                       # Preds ..B10.22 ..B10.21 ..B10.14
        lea       4(%rdx), %eax                                 #2386.13
        cmpl      %eax, %r10d                                   #2357.28
        jb        ..B10.32      # Prob 20%                      #2357.28
                                # LOE rcx rbp r9 r14 r15 edx r10d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.24:                       # Preds ..B10.23
        cmpq      %r9, %rcx                                     #2382.25
        jae       ..B10.32      # Prob 4%                       #2382.25
                                # LOE rcx rbp r9 r14 r15 edx r10d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.25:                       # Preds ..B10.24
        addl      $64, %edx                                     #2386.13
        cmpl      %r10d, %edx                                   #2387.24
        ja        ..B10.32      # Prob 4%                       #2387.24
                                # LOE rcx rbp r9 r14 r15 edx r10d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.26:                       # Preds ..B10.25
        testq     $15, %rcx                                     #2392.22
        jne       ..B10.28      # Prob 50%                      #2392.22
                                # LOE rcx rbp r9 r14 r15 edx r10d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.27:                       # Preds ..B10.26
        movdqa    (%rcx), %xmm0                                 #2392.22
        jmp       ..B10.29      # Prob 100%                     #2392.22
                                # LOE rcx rbp r9 r14 r15 edx r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.28:                       # Preds ..B10.26
        movdqu    (%rcx), %xmm0                                 #2392.22
                                # LOE rcx rbp r9 r14 r15 edx r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.29:                       # Preds ..B10.27 ..B10.28
        addq      $16, %rcx                                     #2470.13
                                # LOE rcx rbp r9 r14 r15 edx r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.30:                       # Preds ..B10.4 ..B10.7 ..B10.29
        movdqa    %xmm0, %xmm9                                  #2285.22
        pand      %xmm7, %xmm9                                  #2285.22
        pcmpeqb   %xmm8, %xmm9                                  #2286.22
        pmovmskb  %xmm9, %esi                                   #2287.22
        jmp       ..B10.12      # Prob 100%                     #2287.22
                                # LOE rcx rbp r9 r14 r15 edx esi r10d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B10.32:                       # Preds ..B10.23 ..B10.24 ..B10.25 # Infreq
        xorl      %eax, %eax                                    #2483.12
..___tag_value_eval_2na_8.575:                                  #2483.12
        popq      %rbx                                          #2483.12
..___tag_value_eval_2na_8.576:                                  #
        popq      %r12                                          #2483.12
..___tag_value_eval_2na_8.578:                                  #
        popq      %r13                                          #2483.12
..___tag_value_eval_2na_8.580:                                  #
        ret                                                     #2483.12
..___tag_value_eval_2na_8.581:                                  #
                                # LOE
..B10.35:                       # Preds ..B10.22 ..B10.20 ..B10.18 ..B10.16 # Infreq
        movl      $1, %eax                                      #2346.63
..___tag_value_eval_2na_8.585:                                  #2346.63
        popq      %rbx                                          #2346.63
..___tag_value_eval_2na_8.586:                                  #
        popq      %r12                                          #2346.63
..___tag_value_eval_2na_8.588:                                  #
        popq      %r13                                          #2346.63
..___tag_value_eval_2na_8.590:                                  #
        ret                                                     #2346.63
        .align    16,0x90
..___tag_value_eval_2na_8.591:                                  #
                                # LOE
# mark_end;
	.type	eval_2na_8,@function
	.size	eval_2na_8,.-eval_2na_8
	.data
# -- End  eval_2na_8
	.text
# -- Begin  eval_4na_64
# mark_begin;
       .align    16,0x90
eval_4na_64:
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %edx
# parameter 4: %ecx
..B11.1:                        # Preds ..B11.0
..___tag_value_eval_4na_64.592:                                 #4501.1
        pushq     %r15                                          #4501.1
..___tag_value_eval_4na_64.594:                                 #
        pushq     %r14                                          #4501.1
..___tag_value_eval_4na_64.596:                                 #
        pushq     %r13                                          #4501.1
..___tag_value_eval_4na_64.598:                                 #
        pushq     %r12                                          #4501.1
..___tag_value_eval_4na_64.600:                                 #
        pushq     %rbx                                          #4501.1
..___tag_value_eval_4na_64.602:                                 #
        movdqa    16(%rdi), %xmm8                               #4554.5
        movdqa    32(%rdi), %xmm7                               #4554.5
        movdqa    48(%rdi), %xmm6                               #4554.5
        movdqa    64(%rdi), %xmm5                               #4554.5
        movdqa    80(%rdi), %xmm4                               #4554.5
        movdqa    96(%rdi), %xmm3                               #4554.5
        movdqa    112(%rdi), %xmm2                              #4554.5
        movdqa    128(%rdi), %xmm1                              #4554.5
        lea       (%rdx,%rcx), %r11d                            #4535.5
        movl      %edx, %ebx                                    #4538.49
        shrl      $2, %ebx                                      #4538.49
        subl      4(%rdi), %r11d                                #4541.12
        movzbl    (%rbx,%rsi), %eax                             #4547.14
        movzbl    1(%rbx,%rsi), %r9d                            #4547.14
        lea       3(%rdx,%rcx), %r10d                           #4544.50
        shrl      $2, %r10d                                     #4544.57
        addq      %rsi, %r10                                    #4544.30
        lea       expand_2na.0(%rip), %rcx                      #4547.14
        movzwl    (%rcx,%rax,2), %r8d                           #4547.14
        movzwl    (%rcx,%r9,2), %eax                            #4547.14
        movw      %r8w, -16(%rsp)                               #4547.14
        movzbl    2(%rbx,%rsi), %r8d                            #4547.14
        movzwl    (%rcx,%r8,2), %r9d                            #4547.14
        movw      %ax, -14(%rsp)                                #4547.14
        movzbl    3(%rbx,%rsi), %eax                            #4547.14
        movzwl    (%rcx,%rax,2), %r8d                           #4547.14
        movw      %r9w, -12(%rsp)                               #4547.14
        movzbl    4(%rbx,%rsi), %r9d                            #4547.14
        movzwl    (%rcx,%r9,2), %eax                            #4547.14
        movw      %r8w, -10(%rsp)                               #4547.14
        movzbl    5(%rbx,%rsi), %r8d                            #4547.14
        movzwl    (%rcx,%r8,2), %r9d                            #4547.14
        movw      %ax, -8(%rsp)                                 #4547.14
        movzbl    6(%rbx,%rsi), %eax                            #4547.14
        movzwl    (%rcx,%rax,2), %r8d                           #4547.14
        movw      %r9w, -6(%rsp)                                #4547.14
        movzbl    7(%rbx,%rsi), %r9d                            #4547.14
        movzwl    (%rcx,%r9,2), %ecx                            #4547.14
        movw      %r8w, -4(%rsp)                                #4547.14
        movl      %edx, %eax                                    #4568.20
        lea       8(%rsi,%rbx), %rbx                            #4548.5
        movw      %cx, -2(%rsp)                                 #4547.14
        movdqu    -16(%rsp), %xmm0                              #4547.14
        movq      %rbx, %rcx                                    #4548.5
        xorl      %r8d, %r8d                                    #4557.15
        xorl      %r9d, %r9d                                    #4557.10
        xorl      %edi, %edi                                    #4557.5
        movl      $4, %esi                                      #4563.5
        andl      $3, %eax                                      #4568.20
        je        ..B11.29      # Prob 20%                      #4568.20
                                # LOE rcx rbx rbp r10 r12 r13 r14 r15 eax edx esi edi r8d r9d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.2:                        # Preds ..B11.1
        cmpl      $1, %eax                                      #4568.20
        je        ..B11.9       # Prob 25%                      #4568.20
                                # LOE rcx rbx rbp r10 r12 r13 r14 r15 eax edx esi edi r8d r9d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.3:                        # Preds ..B11.2
        cmpl      $2, %eax                                      #4568.20
        je        ..B11.10      # Prob 33%                      #4568.20
                                # LOE rcx rbx rbp r10 r12 r13 r14 r15 eax edx esi edi r8d r9d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.4:                        # Preds ..B11.3
        cmpl      $3, %eax                                      #4568.20
        je        ..B11.11      # Prob 50%                      #4568.20
        jmp       ..B11.25      # Prob 100%                     #4568.20
                                # LOE rcx rbx rbp r10 r12 r13 r14 r15 edx esi edi r8d r9d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.9:                        # Preds ..B11.2 ..B11.29
        movdqa    %xmm0, %xmm10                                 #4592.22
        movdqa    %xmm0, %xmm9                                  #4593.22
        pand      %xmm6, %xmm10                                 #4592.22
        pand      %xmm5, %xmm9                                  #4593.22
        pcmpeqd   %xmm9, %xmm10                                 #4594.22
        pmovmskb  %xmm10, %r9d                                  #4595.22
        movl      %r9d, %r8d                                    #4596.17
        andl      $3855, %r8d                                   #4596.17
        shll      $4, %r8d                                      #4596.17
        andl      %r8d, %r9d                                    #4596.17
        movl      %r9d, %r12d                                   #4596.17
        shrl      $4, %r12d                                     #4596.17
        orl       %r12d, %r9d                                   #4596.17
                                # LOE rcx rbx rbp r10 edx esi edi r9d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.10:                       # Preds ..B11.3 ..B11.9
        movdqa    %xmm0, %xmm10                                 #4599.22
        movdqa    %xmm0, %xmm9                                  #4600.22
        pand      %xmm4, %xmm10                                 #4599.22
        pand      %xmm3, %xmm9                                  #4600.22
        pcmpeqd   %xmm9, %xmm10                                 #4601.22
        pmovmskb  %xmm10, %r8d                                  #4602.22
        movl      %r8d, %r12d                                   #4603.17
        andl      $3855, %r12d                                  #4603.17
        shll      $4, %r12d                                     #4603.17
        andl      %r12d, %r8d                                   #4603.17
        movl      %r8d, %r13d                                   #4603.17
        shrl      $4, %r13d                                     #4603.17
        orl       %r13d, %r8d                                   #4603.17
                                # LOE rcx rbx rbp r10 edx esi edi r8d r9d r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.11:                       # Preds ..B11.4 ..B11.10
        movdqa    %xmm0, %xmm10                                 #4606.22
        movdqa    %xmm0, %xmm9                                  #4607.22
        pand      %xmm2, %xmm10                                 #4606.22
        pand      %xmm1, %xmm9                                  #4607.22
        pcmpeqd   %xmm9, %xmm10                                 #4608.22
        pmovmskb  %xmm10, %r12d                                 #4609.22
        movl      %r12d, %r13d                                  #4610.17
        andl      $3855, %r13d                                  #4610.17
        shll      $4, %r13d                                     #4610.17
        andl      %r13d, %r12d                                  #4610.17
        movl      %r12d, %r14d                                  #4610.17
        shrl      $4, %r14d                                     #4610.17
        orl       %r14d, %r12d                                  #4610.17
        movl      %edx, %r13d                                   #4614.17
        andl      $-4, %r13d                                    #4614.17
        movl      %edi, %edx                                    #4617.29
        orl       %r9d, %edx                                    #4617.29
        orl       %r8d, %edx                                    #4617.34
        orl       %r12d, %edx                                   #4617.39
        je        ..B11.20      # Prob 78%                      #4617.47
                                # LOE rcx rbx rbp r10 esi edi r8d r9d r11d r12d r13d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.12:                       # Preds ..B11.11
        movl      %edi, %eax                                    #4635.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #4635.30
        movswq    %ax, %r15                                     #4635.30
        movl      %r9d, %eax                                    #4636.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #4636.30
        movswq    %ax, %rdx                                     #4636.30
        movl      %r8d, %eax                                    #4637.30
        lea       1(%rdx,%rdx), %edx                            #4644.40
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #4637.30
        movswq    %ax, %r14                                     #4637.30
        movl      %r12d, %eax                                   #4638.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #4638.30
        movswq    %ax, %rax                                     #4638.30
        lea       2(%r14,%r14), %r14d                           #4645.40
        lea       3(%rax,%rax), %eax                            #4646.40
        testl     %edi, %edi                                    #4650.32
        je        ..B11.14      # Prob 50%                      #4650.32
                                # LOE rcx rbx rbp r10 eax edx esi r8d r9d r11d r12d r13d r14d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.13:                       # Preds ..B11.12
        lea       (%r13,%r15,2), %edi                           #4650.43
        cmpl      %edi, %r11d                                   #4650.49
        jae       ..B11.34      # Prob 20%                      #4650.49
                                # LOE rcx rbx rbp r10 eax edx esi r8d r9d r11d r12d r13d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.14:                       # Preds ..B11.13 ..B11.12
        testl     %r9d, %r9d                                    #4651.32
        je        ..B11.16      # Prob 50%                      #4651.32
                                # LOE rcx rbx rbp r10 eax edx esi r8d r11d r12d r13d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.15:                       # Preds ..B11.14
        addl      %r13d, %edx                                   #4651.43
        cmpl      %edx, %r11d                                   #4651.49
        jae       ..B11.34      # Prob 20%                      #4651.49
                                # LOE rcx rbx rbp r10 eax esi r8d r11d r12d r13d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.16:                       # Preds ..B11.15 ..B11.14
        testl     %r8d, %r8d                                    #4652.32
        je        ..B11.18      # Prob 50%                      #4652.32
                                # LOE rcx rbx rbp r10 eax esi r11d r12d r13d r14d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.17:                       # Preds ..B11.16
        addl      %r13d, %r14d                                  #4652.43
        cmpl      %r14d, %r11d                                  #4652.49
        jae       ..B11.34      # Prob 20%                      #4652.49
                                # LOE rcx rbx rbp r10 eax esi r11d r12d r13d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.18:                       # Preds ..B11.17 ..B11.16
        testl     %r12d, %r12d                                  #4653.32
        je        ..B11.20      # Prob 50%                      #4653.32
                                # LOE rcx rbx rbp r10 eax esi r11d r13d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.19:                       # Preds ..B11.18
        addl      %r13d, %eax                                   #4653.43
        cmpl      %eax, %r11d                                   #4653.49
        jae       ..B11.34      # Prob 20%                      #4653.49
                                # LOE rcx rbx rbp r10 esi r11d r13d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.20:                       # Preds ..B11.19 ..B11.18 ..B11.11
        lea       4(%r13), %edx                                 #4658.17
        cmpl      %r11d, %edx                                   #4661.28
        ja        ..B11.31      # Prob 20%                      #4661.28
                                # LOE rcx rbx rbp r10 edx esi r11d r13d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.21:                       # Preds ..B11.20
        decl      %esi                                          #4665.25
        jne       ..B11.26      # Prob 50%                      #4665.39
                                # LOE rcx rbx rbp r10 edx esi r11d r13d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.22:                       # Preds ..B11.21
        cmpq      %r10, %rbx                                    #4682.25
        jae       ..B11.31      # Prob 4%                       #4682.25
                                # LOE rbx rbp r10 r11d r13d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.23:                       # Preds ..B11.22
        lea       20(%r13), %edx                                #4686.13
        cmpl      %r11d, %edx                                   #4687.24
        ja        ..B11.31      # Prob 4%                       #4687.24
                                # LOE rbx rbp r10 edx r11d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.24:                       # Preds ..B11.23
        movzbl    (%rbx), %ecx                                  #4692.22
        movzbl    1(%rbx), %edi                                 #4692.22
        movzbl    2(%rbx), %r9d                                 #4692.22
        movzbl    3(%rbx), %r13d                                #4692.22
        movzbl    4(%rbx), %r15d                                #4692.22
        lea       expand_2na.0(%rip), %rax                      #4692.22
        movzwl    (%rax,%rcx,2), %esi                           #4692.22
        movzwl    (%rax,%rdi,2), %r8d                           #4692.22
        movzwl    (%rax,%r9,2), %r12d                           #4692.22
        movzwl    (%rax,%r13,2), %r14d                          #4692.22
        movzwl    (%rax,%r15,2), %ecx                           #4692.22
        movw      %si, -16(%rsp)                                #4692.22
        movzbl    5(%rbx), %esi                                 #4692.22
        movzwl    (%rax,%rsi,2), %edi                           #4692.22
        movw      %r8w, -14(%rsp)                               #4692.22
        movzbl    6(%rbx), %r8d                                 #4692.22
        movzwl    (%rax,%r8,2), %r9d                            #4692.22
        movw      %r12w, -12(%rsp)                              #4692.22
        movzbl    7(%rbx), %r12d                                #4692.22
        movzwl    (%rax,%r12,2), %eax                           #4692.22
        movw      %r14w, -10(%rsp)                              #4692.22
        movw      %cx, -8(%rsp)                                 #4692.22
        addq      $8, %rbx                                      #4707.13
        movw      %di, -6(%rsp)                                 #4692.22
        movq      %rbx, %rcx                                    #4707.13
        movw      %r9w, -4(%rsp)                                #4692.22
        movw      %ax, -2(%rsp)                                 #4692.22
        movdqu    -16(%rsp), %xmm0                              #4692.22
                                # LOE rcx rbx rbp r10 edx r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.25:                       # Preds ..B11.4 ..B11.24
        movl      $4, %esi                                      #4575.13
        jmp       ..B11.29      # Prob 100%                     #4575.13
                                # LOE rcx rbx rbp r10 edx esi r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.26:                       # Preds ..B11.21
        psrldq    $2, %xmm0                                     #4670.26
        cmpq      %r10, %rcx                                    #4673.26
        jae       ..B11.28      # Prob 19%                      #4673.26
                                # LOE rcx rbx rbp r10 edx esi r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.27:                       # Preds ..B11.26
        movzbl    (%rcx), %edi                                  #4674.72
        lea       expand_2na.0(%rip), %rax                      #4674.57
        movzwl    (%rax,%rdi,2), %r8d                           #4674.57
        pinsrw    $7, %r8d, %xmm0                               #4674.30
                                # LOE rcx rbx rbp r10 edx esi r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.28:                       # Preds ..B11.27 ..B11.26
        incq      %rcx                                          #4677.20
                                # LOE rcx rbx rbp r10 edx esi r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.29:                       # Preds ..B11.1 ..B11.25 ..B11.28
        movdqa    %xmm0, %xmm10                                 #4585.22
        movdqa    %xmm0, %xmm9                                  #4586.22
        pand      %xmm8, %xmm10                                 #4585.22
        pand      %xmm7, %xmm9                                  #4586.22
        pcmpeqd   %xmm9, %xmm10                                 #4587.22
        pmovmskb  %xmm10, %edi                                  #4588.22
        movl      %edi, %r8d                                    #4589.17
        andl      $3855, %r8d                                   #4589.17
        shll      $4, %r8d                                      #4589.17
        andl      %r8d, %edi                                    #4589.17
        movl      %edi, %r9d                                    #4589.17
        shrl      $4, %r9d                                      #4589.17
        orl       %r9d, %edi                                    #4589.17
        jmp       ..B11.9       # Prob 100%                     #4589.17
                                # LOE rcx rbx rbp r10 edx esi edi r11d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B11.31:                       # Preds ..B11.20 ..B11.22 ..B11.23 # Infreq
        xorl      %eax, %eax                                    #4718.12
..___tag_value_eval_4na_64.605:                                 #4718.12
        popq      %rbx                                          #4718.12
..___tag_value_eval_4na_64.607:                                 #
        popq      %r12                                          #4718.12
..___tag_value_eval_4na_64.609:                                 #
        popq      %r13                                          #4718.12
..___tag_value_eval_4na_64.611:                                 #
        popq      %r14                                          #4718.12
..___tag_value_eval_4na_64.613:                                 #
        popq      %r15                                          #4718.12
..___tag_value_eval_4na_64.615:                                 #
        ret                                                     #4718.12
..___tag_value_eval_4na_64.616:                                 #
                                # LOE
..B11.34:                       # Preds ..B11.13 ..B11.19 ..B11.17 ..B11.15 # Infreq
        movl      $1, %eax                                      #4651.63
..___tag_value_eval_4na_64.622:                                 #4651.63
        popq      %rbx                                          #4651.63
..___tag_value_eval_4na_64.624:                                 #
        popq      %r12                                          #4651.63
..___tag_value_eval_4na_64.626:                                 #
        popq      %r13                                          #4651.63
..___tag_value_eval_4na_64.628:                                 #
        popq      %r14                                          #4651.63
..___tag_value_eval_4na_64.630:                                 #
        popq      %r15                                          #4651.63
..___tag_value_eval_4na_64.632:                                 #
        ret                                                     #4651.63
        .align    16,0x90
..___tag_value_eval_4na_64.633:                                 #
                                # LOE
# mark_end;
	.type	eval_4na_64,@function
	.size	eval_4na_64,.-eval_4na_64
	.data
# -- End  eval_4na_64
	.text
# -- Begin  eval_2na_64
# mark_begin;
       .align    16,0x90
eval_2na_64:
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %edx
# parameter 4: %ecx
..B12.1:                        # Preds ..B12.0
..___tag_value_eval_2na_64.634:                                 #3075.1
        pushq     %r15                                          #3075.1
..___tag_value_eval_2na_64.636:                                 #
        pushq     %r14                                          #3075.1
..___tag_value_eval_2na_64.638:                                 #
        pushq     %r13                                          #3075.1
..___tag_value_eval_2na_64.640:                                 #
        pushq     %r12                                          #3075.1
..___tag_value_eval_2na_64.642:                                 #
        pushq     %rbp                                          #3075.1
..___tag_value_eval_2na_64.644:                                 #
        pushq     %rbx                                          #3075.1
..___tag_value_eval_2na_64.646:                                 #
        lea       (%rdx,%rcx), %r15d                            #3113.5
        movl      %edx, %eax                                    #3116.49
        shrl      $2, %eax                                      #3116.49
        subl      4(%rdi), %r15d                                #3119.12
        lea       (%rsi,%rax), %r8                              #3116.30
        lea       3(%rdx,%rcx), %r14d                           #3122.50
        shrl      $2, %r14d                                     #3122.57
        addq      %rsi, %r14                                    #3122.30
        testq     $15, %r8                                      #3125.14
        jne       ..B12.3       # Prob 50%                      #3125.14
                                # LOE rax rbx rbp rsi rdi r8 r12 r13 r14 edx r15d
..B12.2:                        # Preds ..B12.1
        movdqa    (%r8), %xmm0                                  #3125.14
        jmp       ..B12.4       # Prob 100%                     #3125.14
                                # LOE rax rbx rbp rsi rdi r12 r13 r14 edx r15d xmm0
..B12.3:                        # Preds ..B12.1
        movdqu    (%r8), %xmm0                                  #3125.14
                                # LOE rax rbx rbp rsi rdi r12 r13 r14 edx r15d xmm0
..B12.4:                        # Preds ..B12.2 ..B12.3
        lea       16(%rsi,%rax), %r8                            #3126.5
        movq      %r8, %rsi                                     #3126.5
        cmpq      %r14, %r8                                     #3131.16
        jae       ..B12.6       # Prob 12%                      #3131.16
                                # LOE rbx rbp rsi rdi r8 r12 r13 r14 edx ecx r15d xmm0
..B12.5:                        # Preds ..B12.4
        movzbl    -1(%r8), %ecx                                 #3132.24
        shll      $8, %ecx                                      #3132.38
                                # LOE rbx rbp rsi rdi r8 r12 r13 r14 edx ecx r15d xmm0
..B12.6:                        # Preds ..B12.5 ..B12.4
        movdqa    16(%rdi), %xmm8                               #3136.5
        movdqa    32(%rdi), %xmm7                               #3136.5
        movdqa    48(%rdi), %xmm6                               #3136.5
        movdqa    64(%rdi), %xmm5                               #3136.5
        movdqa    80(%rdi), %xmm4                               #3136.5
        movdqa    96(%rdi), %xmm3                               #3136.5
        movdqa    112(%rdi), %xmm2                              #3136.5
        movdqa    128(%rdi), %xmm1                              #3136.5
        xorl      %r10d, %r10d                                  #3139.15
        xorl      %r11d, %r11d                                  #3139.10
        xorl      %r9d, %r9d                                    #3139.5
        movl      $8, %edi                                      #3143.5
        movl      %edx, %eax                                    #3148.20
        andl      $3, %eax                                      #3148.20
        je        ..B12.38      # Prob 20%                      #3148.20
                                # LOE rbx rbp rsi r8 r12 r13 r14 eax edx ecx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.7:                        # Preds ..B12.6
        cmpl      $1, %eax                                      #3148.20
        je        ..B12.14      # Prob 25%                      #3148.20
                                # LOE rbx rbp rsi r8 r12 r13 r14 eax edx ecx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.8:                        # Preds ..B12.7
        cmpl      $2, %eax                                      #3148.20
        je        ..B12.15      # Prob 33%                      #3148.20
                                # LOE rbx rbp rsi r8 r12 r13 r14 eax edx ecx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.9:                        # Preds ..B12.8
        cmpl      $3, %eax                                      #3148.20
        je        ..B12.16      # Prob 50%                      #3148.20
        jmp       ..B12.34      # Prob 100%                     #3148.20
                                # LOE rbx rbp rsi r8 r12 r13 r14 edx ecx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.14:                       # Preds ..B12.7 ..B12.38
        movdqa    %xmm0, %xmm9                                  #3171.22
        pand      %xmm5, %xmm9                                  #3171.22
        pcmpeqd   %xmm6, %xmm9                                  #3172.22
        pmovmskb  %xmm9, %r11d                                  #3173.22
        movl      %r11d, %ebx                                   #3174.17
        andl      $3855, %ebx                                   #3174.17
        shll      $4, %ebx                                      #3174.17
        andl      %ebx, %r11d                                   #3174.17
        movl      %r11d, %ebp                                   #3174.17
        shrl      $4, %ebp                                      #3174.17
        orl       %ebp, %r11d                                   #3174.17
                                # LOE rsi r8 r14 edx ecx edi r9d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.15:                       # Preds ..B12.8 ..B12.14
        movdqa    %xmm0, %xmm9                                  #3177.22
        pand      %xmm3, %xmm9                                  #3177.22
        pcmpeqd   %xmm4, %xmm9                                  #3178.22
        pmovmskb  %xmm9, %r10d                                  #3179.22
        movl      %r10d, %ebx                                   #3180.17
        andl      $3855, %ebx                                   #3180.17
        shll      $4, %ebx                                      #3180.17
        andl      %ebx, %r10d                                   #3180.17
        movl      %r10d, %ebp                                   #3180.17
        shrl      $4, %ebp                                      #3180.17
        orl       %ebp, %r10d                                   #3180.17
                                # LOE rsi r8 r14 edx ecx edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.16:                       # Preds ..B12.9 ..B12.15
        movdqa    %xmm0, %xmm9                                  #3183.22
        pand      %xmm1, %xmm9                                  #3183.22
        pcmpeqd   %xmm2, %xmm9                                  #3184.22
        pmovmskb  %xmm9, %ebp                                   #3185.22
        movl      %ebp, %ebx                                    #3186.17
        andl      $3855, %ebx                                   #3186.17
        shll      $4, %ebx                                      #3186.17
        andl      %ebx, %ebp                                    #3186.17
        movl      %ebp, %r12d                                   #3186.17
        shrl      $4, %r12d                                     #3186.17
        orl       %r12d, %ebp                                   #3186.17
        movl      %edx, %ebx                                    #3190.17
        andl      $-4, %ebx                                     #3190.17
        movl      %r9d, %edx                                    #3193.29
        orl       %r11d, %edx                                   #3193.29
        orl       %r10d, %edx                                   #3193.34
        orl       %ebp, %edx                                    #3193.39
        je        ..B12.25      # Prob 78%                      #3193.47
                                # LOE rsi r8 r14 ecx ebx ebp edi r9d r10d r11d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.17:                       # Preds ..B12.16
        movl      %r9d, %eax                                    #3211.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #3211.30
        movswq    %ax, %r13                                     #3211.30
        movl      %r11d, %eax                                   #3212.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #3212.30
        movswq    %ax, %r12                                     #3212.30
        movl      %r10d, %eax                                   #3213.30
        shll      $2, %r13d                                     #3219.34
        shll      $2, %r12d                                     #3220.34
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #3213.30
        movswq    %ax, %rdx                                     #3213.30
        movl      %ebp, %eax                                    #3214.30
# Begin ASM
        bsf %ax, %ax;jnz .+6;xor %eax, %eax;dec %eax;
# End ASM                                                       #3214.30
        movswq    %ax, %rax                                     #3214.30
        shll      $2, %edx                                      #3221.34
        shll      $2, %eax                                      #3222.34
        testl     %r9d, %r9d                                    #3226.32
        je        ..B12.19      # Prob 50%                      #3226.32
                                # LOE rsi r8 r14 eax edx ecx ebx ebp edi r10d r11d r12d r13d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.18:                       # Preds ..B12.17
        addl      %ebx, %r13d                                   #3226.43
        cmpl      %r13d, %r15d                                  #3226.49
        jae       ..B12.43      # Prob 20%                      #3226.49
                                # LOE rsi r8 r14 eax edx ecx ebx ebp edi r10d r11d r12d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.19:                       # Preds ..B12.18 ..B12.17
        testl     %r11d, %r11d                                  #3227.32
        je        ..B12.21      # Prob 50%                      #3227.32
                                # LOE rsi r8 r14 eax edx ecx ebx ebp edi r10d r12d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.20:                       # Preds ..B12.19
        lea       1(%rbx,%r12), %r9d                            #3227.43
        cmpl      %r9d, %r15d                                   #3227.49
        jae       ..B12.43      # Prob 20%                      #3227.49
                                # LOE rsi r8 r14 eax edx ecx ebx ebp edi r10d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.21:                       # Preds ..B12.20 ..B12.19
        testl     %r10d, %r10d                                  #3228.32
        je        ..B12.23      # Prob 50%                      #3228.32
                                # LOE rsi r8 r14 eax edx ecx ebx ebp edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.22:                       # Preds ..B12.21
        lea       2(%rbx,%rdx), %edx                            #3228.43
        cmpl      %edx, %r15d                                   #3228.49
        jae       ..B12.43      # Prob 20%                      #3228.49
                                # LOE rsi r8 r14 eax ecx ebx ebp edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.23:                       # Preds ..B12.22 ..B12.21
        testl     %ebp, %ebp                                    #3229.32
        je        ..B12.25      # Prob 50%                      #3229.32
                                # LOE rsi r8 r14 eax ecx ebx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.24:                       # Preds ..B12.23
        lea       3(%rbx,%rax), %eax                            #3229.43
        cmpl      %eax, %r15d                                   #3229.49
        jae       ..B12.43      # Prob 20%                      #3229.49
                                # LOE rsi r8 r14 ecx ebx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.25:                       # Preds ..B12.24 ..B12.23 ..B12.16
        lea       4(%rbx), %edx                                 #3234.17
        cmpl      %r15d, %edx                                   #3237.28
        ja        ..B12.40      # Prob 20%                      #3237.28
                                # LOE rsi r8 r14 edx ecx ebx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.26:                       # Preds ..B12.25
        decl      %edi                                          #3241.25
        jne       ..B12.35      # Prob 50%                      #3241.39
                                # LOE rsi r8 r14 edx ecx ebx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.27:                       # Preds ..B12.26
        cmpq      %r14, %r8                                     #3262.25
        jae       ..B12.40      # Prob 4%                       #3262.25
                                # LOE r8 r14 ecx ebx r15d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.28:                       # Preds ..B12.27
        lea       36(%rbx), %edx                                #3266.13
        cmpl      %r15d, %edx                                   #3267.24
        ja        ..B12.40      # Prob 4%                       #3267.24
                                # LOE r8 r14 edx ecx r15d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.29:                       # Preds ..B12.28
        testq     $15, %r8                                      #3272.22
        jne       ..B12.31      # Prob 50%                      #3272.22
                                # LOE r8 r14 edx ecx r15d xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.30:                       # Preds ..B12.29
        movdqa    (%r8), %xmm0                                  #3272.22
        jmp       ..B12.32      # Prob 100%                     #3272.22
                                # LOE r8 r14 edx ecx r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.31:                       # Preds ..B12.29
        movdqu    (%r8), %xmm0                                  #3272.22
                                # LOE r8 r14 edx ecx r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.32:                       # Preds ..B12.30 ..B12.31
        addq      $16, %r8                                      #3350.13
        movq      %r8, %rsi                                     #3350.13
        cmpq      %r14, %r8                                     #3353.24
        jae       ..B12.34      # Prob 12%                      #3353.24
                                # LOE rsi r8 r14 edx ecx r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.33:                       # Preds ..B12.32
        movzbl    -1(%r8), %ecx                                 #3354.32
        shll      $8, %ecx                                      #3354.46
                                # LOE rsi r8 r14 edx ecx r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.34:                       # Preds ..B12.9 ..B12.32 ..B12.33
        movl      $8, %edi                                      #3155.13
        jmp       ..B12.38      # Prob 100%                     #3155.13
                                # LOE rsi r8 r14 edx ecx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.35:                       # Preds ..B12.26
        psrldq    $1, %xmm0                                     #3246.26
        cmpq      %r14, %rsi                                    #3249.26
        jae       ..B12.37      # Prob 19%                      #3249.26
                                # LOE rsi r8 r14 edx ecx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.36:                       # Preds ..B12.35
        movzbl    (%rsi), %eax                                  #3252.37
        sarl      $8, %ecx                                      #3251.21
        shll      $8, %eax                                      #3252.48
        orl       %eax, %ecx                                    #3252.21
        pinsrw    $7, %ecx, %xmm0                               #3253.30
                                # LOE rsi r8 r14 edx ecx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.37:                       # Preds ..B12.36 ..B12.35
        incq      %rsi                                          #3257.20
                                # LOE rsi r8 r14 edx ecx edi r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.38:                       # Preds ..B12.6 ..B12.34 ..B12.37
        movdqa    %xmm0, %xmm9                                  #3165.22
        pand      %xmm7, %xmm9                                  #3165.22
        pcmpeqd   %xmm8, %xmm9                                  #3166.22
        pmovmskb  %xmm9, %r9d                                   #3167.22
        movl      %r9d, %ebx                                    #3168.17
        andl      $3855, %ebx                                   #3168.17
        shll      $4, %ebx                                      #3168.17
        andl      %ebx, %r9d                                    #3168.17
        movl      %r9d, %ebp                                    #3168.17
        shrl      $4, %ebp                                      #3168.17
        orl       %ebp, %r9d                                    #3168.17
        jmp       ..B12.14      # Prob 100%                     #3168.17
                                # LOE rsi r8 r14 edx ecx edi r9d r15d xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8
..B12.40:                       # Preds ..B12.25 ..B12.27 ..B12.28 # Infreq
        xorl      %eax, %eax                                    #3363.12
..___tag_value_eval_2na_64.649:                                 #3363.12
        popq      %rbx                                          #3363.12
..___tag_value_eval_2na_64.651:                                 #
        popq      %rbp                                          #3363.12
..___tag_value_eval_2na_64.653:                                 #
        popq      %r12                                          #3363.12
..___tag_value_eval_2na_64.655:                                 #
        popq      %r13                                          #3363.12
..___tag_value_eval_2na_64.657:                                 #
        popq      %r14                                          #3363.12
..___tag_value_eval_2na_64.659:                                 #
        popq      %r15                                          #3363.12
..___tag_value_eval_2na_64.661:                                 #
        ret                                                     #3363.12
..___tag_value_eval_2na_64.662:                                 #
                                # LOE
..B12.43:                       # Preds ..B12.22 ..B12.20 ..B12.18 ..B12.24 # Infreq
        movl      $1, %eax                                      #3229.63
..___tag_value_eval_2na_64.669:                                 #3229.63
        popq      %rbx                                          #3229.63
..___tag_value_eval_2na_64.671:                                 #
        popq      %rbp                                          #3229.63
..___tag_value_eval_2na_64.673:                                 #
        popq      %r12                                          #3229.63
..___tag_value_eval_2na_64.675:                                 #
        popq      %r13                                          #3229.63
..___tag_value_eval_2na_64.677:                                 #
        popq      %r14                                          #3229.63
..___tag_value_eval_2na_64.679:                                 #
        popq      %r15                                          #3229.63
..___tag_value_eval_2na_64.681:                                 #
        ret                                                     #3229.63
        .align    16,0x90
..___tag_value_eval_2na_64.682:                                 #
                                # LOE
# mark_end;
	.type	eval_2na_64,@function
	.size	eval_2na_64,.-eval_2na_64
	.data
# -- End  eval_2na_64
	.text
# -- Begin  NucStrstrMake
# mark_begin;
       .align    16,0x90
	.globl NucStrstrMake
NucStrstrMake:
# parameter 1: %rdi
# parameter 2: %esi
# parameter 3: %rdx
# parameter 4: %ecx
..B13.1:                        # Preds ..B13.0
..___tag_value_NucStrstrMake.683:                               #1539.1
        pushq     %r15                                          #1539.1
..___tag_value_NucStrstrMake.685:                               #
        pushq     %r14                                          #1539.1
..___tag_value_NucStrstrMake.687:                               #
        pushq     %r13                                          #1539.1
..___tag_value_NucStrstrMake.689:                               #
        pushq     %r12                                          #1539.1
..___tag_value_NucStrstrMake.691:                               #
        pushq     %rbp                                          #1539.1
..___tag_value_NucStrstrMake.693:                               #
        pushq     %rbx                                          #1539.1
..___tag_value_NucStrstrMake.695:                               #
        subq      $24, %rsp                                     #1539.1
..___tag_value_NucStrstrMake.697:                               #
        movl      %ecx, %r13d                                   #1539.1
        movq      %rdx, %r12                                    #1539.1
        movl      %esi, %ebp                                    #1539.1
        movq      %rdi, %rbx                                    #1539.1
        testq     %rbx, %rbx                                    #1540.17
        je        ..B13.12      # Prob 6%                       #1540.17
                                # LOE rbx r12 ebp r13d
..B13.2:                        # Preds ..B13.1
        testq     %r12, %r12                                    #1542.23
        je        ..B13.11      # Prob 6%                       #1542.23
                                # LOE rbx r12 ebp r13d
..B13.3:                        # Preds ..B13.2
        testl     %r13d, %r13d                                  #1542.38
        je        ..B13.11      # Prob 28%                      #1542.38
                                # LOE rbx r12 ebp r13d
..B13.4:                        # Preds ..B13.3
        movl      $0, 16(%rsp)                                  #1544.24
        lea       fasta_2na_map.0(%rip), %rax                   #1547.18
        movsbl    (%rax), %edx                                  #1547.18
        testl     %edx, %edx                                    #1547.41
        je        ..B13.13      # Prob 5%                       #1547.41
                                # LOE rbx r12 ebp r13d
..B13.5:                        # Preds ..B13.35 ..B13.4
        lea       16(%rsp), %rcx                                #1551.21
        movl      %r13d, %r13d                                  #1550.19
        addq      %r12, %r13                                    #1550.19
        movq      %r12, %rdi                                    #1551.21
        movq      %r13, %rsi                                    #1551.21
        movq      %rbx, %rdx                                    #1551.21
        movl      %ebp, %r8d                                    #1551.21
        call      nss_expr@PLT                                  #1551.21
                                # LOE rax rbx r13
..B13.6:                        # Preds ..B13.5
        movl      16(%rsp), %ebp                                #1552.18
        testl     %ebp, %ebp                                    #1552.28
        jne       ..B13.9       # Prob 10%                      #1552.28
                                # LOE rax rbx r13 ebp
..B13.7:                        # Preds ..B13.6
        cmpq      %r13, %rax                                    #1554.31
        je        ..B13.37      # Prob 12%                      #1554.31
                                # LOE rbx
..B13.8:                        # Preds ..B13.7
        movl      $22, 16(%rsp)                                 #1557.17
        movl      $22, %ebp                                     #1557.17
                                # LOE rbx ebp
..B13.9:                        # Preds ..B13.6 ..B13.8
        movq      (%rbx), %rdi                                  #1560.13
        call      NucStrstrWhack@PLT                            #1560.13
                                # LOE rbx ebp
..B13.10:                       # Preds ..B13.9
        movq      $0, (%rbx)                                    #1561.13
        movl      %ebp, %eax                                    #1562.20
        addq      $24, %rsp                                     #1562.20
..___tag_value_NucStrstrMake.698:                               #
        popq      %rbx                                          #1562.20
..___tag_value_NucStrstrMake.700:                               #
        popq      %rbp                                          #1562.20
..___tag_value_NucStrstrMake.702:                               #
        popq      %r12                                          #1562.20
..___tag_value_NucStrstrMake.704:                               #
        popq      %r13                                          #1562.20
..___tag_value_NucStrstrMake.706:                               #
        popq      %r14                                          #1562.20
..___tag_value_NucStrstrMake.708:                               #
        popq      %r15                                          #1562.20
..___tag_value_NucStrstrMake.710:                               #
        ret                                                     #1562.20
..___tag_value_NucStrstrMake.711:                               #
                                # LOE
..B13.11:                       # Preds ..B13.3 ..B13.2
        movq      $0, (%rbx)                                    #1565.9
                                # LOE
..B13.12:                       # Preds ..B13.1 ..B13.11
        movl      $22, %eax                                     #1567.12
        addq      $24, %rsp                                     #1567.12
..___tag_value_NucStrstrMake.718:                               #
        popq      %rbx                                          #1567.12
..___tag_value_NucStrstrMake.720:                               #
        popq      %rbp                                          #1567.12
..___tag_value_NucStrstrMake.722:                               #
        popq      %r12                                          #1567.12
..___tag_value_NucStrstrMake.724:                               #
        popq      %r13                                          #1567.12
..___tag_value_NucStrstrMake.726:                               #
        popq      %r14                                          #1567.12
..___tag_value_NucStrstrMake.728:                               #
        popq      %r15                                          #1567.12
..___tag_value_NucStrstrMake.730:                               #
        ret                                                     #1567.12
..___tag_value_NucStrstrMake.731:                               #
                                # LOE
..B13.13:                       # Preds ..B13.4                 # Infreq
        lea       fasta_2na_map.0(%rip), %rdx                   #1548.17
        movl      $-1, %eax                                     #1548.17
        movd      %eax, %xmm0                                   #1548.17
        pshufd    $0, %xmm0, %xmm1                              #1548.17
        movaps    %xmm1, (%rdx)                                 #1548.17
        movaps    %xmm1, 16(%rdx)                               #1548.17
        movaps    %xmm1, 32(%rdx)                               #1548.17
        movaps    %xmm1, 48(%rdx)                               #1548.17
        movaps    %xmm1, 64(%rdx)                               #1548.17
        movaps    %xmm1, 80(%rdx)                               #1548.17
        movaps    %xmm1, 96(%rdx)                               #1548.17
        movaps    %xmm1, 112(%rdx)                              #1548.17
                                # LOE rbx r12 ebp r13d
..B13.14:                       # Preds ..B13.13                # Infreq
        lea       fasta_4na_map.0(%rip), %rdx                   #1548.17
        movl      $-1, %eax                                     #1548.17
        movd      %eax, %xmm0                                   #1548.17
        pshufd    $0, %xmm0, %xmm1                              #1548.17
        movaps    %xmm1, (%rdx)                                 #1548.17
        movaps    %xmm1, 16(%rdx)                               #1548.17
        movaps    %xmm1, 32(%rdx)                               #1548.17
        movaps    %xmm1, 48(%rdx)                               #1548.17
        movaps    %xmm1, 64(%rdx)                               #1548.17
        movaps    %xmm1, 80(%rdx)                               #1548.17
        movaps    %xmm1, 96(%rdx)                               #1548.17
        movaps    %xmm1, 112(%rdx)                              #1548.17
                                # LOE rbx r12 ebp r13d
..B13.15:                       # Preds ..B13.14                # Infreq
        xorl      %r15d, %r15d                                  #1548.17
        lea       _2__STRING.0.0(%rip), %rax                    #1548.17
        movsbl    (%rax), %edx                                  #1548.17
        movl      %edx, (%rsp)                                  #1548.17
        lea       1+_2__STRING.0.0(%rip), %r14                  #
        call      __ctype_tolower_loc@PLT                       #1548.17
                                # LOE rbx r12 r14 ebp r13d r15d
..B13.43:                       # Preds ..B13.15                # Infreq
        movl      %r13d, 8(%rsp)                                #
        movl      (%rsp), %r13d                                 #
                                # LOE rbx r12 r14 ebp r13d r15d
..B13.21:                       # Preds ..B13.43 ..B13.22       # Infreq
        movl      %r13d, %edi                                   #1548.17
        call      tolower@PLT                                   #1548.17
                                # LOE rbx r12 r14 eax ebp r13d r15d
..B13.41:                       # Preds ..B13.21                # Infreq
        movslq    %eax, %rdx                                    #1548.17
                                # LOE rdx rbx r12 r14 ebp r13d r15d
..B13.22:                       # Preds ..B13.41                # Infreq
        lea       fasta_2na_map.0(%rip), %rax                   #1548.17
        movb      %r15b, (%rax,%rdx)                            #1548.17
        movslq    %r13d, %rdx                                   #1548.17
        movb      %r15b, (%rax,%rdx)                            #1548.17
        movsbl    (%r14), %r13d                                 #1548.17
        incl      %r15d                                         #1548.17
        incq      %r14                                          #1548.17
        testl     %r13d, %r13d                                  #1548.17
        jne       ..B13.21      # Prob 82%                      #1548.17
                                # LOE rbx r12 r14 ebp r13d r15d
..B13.23:                       # Preds ..B13.22                # Infreq
        xorl      %eax, %eax                                    #1548.17
        lea       _2__STRING.1.0(%rip), %rdx                    #1548.17
        movsbl    (%rdx), %r14d                                 #1548.17
        lea       1+_2__STRING.1.0(%rip), %r15                  #
        movl      %eax, %r13d                                   #1548.17
                                # LOE rbx r12 r15 ebp r13d r14d
..B13.29:                       # Preds ..B13.23 ..B13.30       # Infreq
        movl      %r14d, %edi                                   #1548.17
        call      tolower@PLT                                   #1548.17
                                # LOE rbx r12 r15 eax ebp r13d r14d
..B13.42:                       # Preds ..B13.29                # Infreq
        movslq    %eax, %rcx                                    #1548.17
                                # LOE rcx rbx r12 r15 ebp r13d r14d
..B13.30:                       # Preds ..B13.42                # Infreq
        lea       fasta_4na_map.0(%rip), %rdx                   #1548.17
        movb      %r13b, (%rdx,%rcx)                            #1548.17
        movslq    %r14d, %rcx                                   #1548.17
        movb      %r13b, (%rdx,%rcx)                            #1548.17
        movsbl    (%r15), %r14d                                 #1548.17
        incl      %r13d                                         #1548.17
        incq      %r15                                          #1548.17
        testl     %r14d, %r14d                                  #1548.17
        jne       ..B13.29      # Prob 82%                      #1548.17
                                # LOE rbx r12 r15 ebp r13d r14d
..B13.31:                       # Preds ..B13.30                # Infreq
        movl      8(%rsp), %r13d                                #
        xorl      %edx, %edx                                    #1548.17
        xorl      %eax, %eax                                    #1548.17
                                # LOE rdx rbx r12 eax ebp r13d
..B13.32:                       # Preds ..B13.35 ..B13.31       # Infreq
        lea       expand_2na.0(%rip), %rcx                      #1548.17
        movzwl    (%rcx,%rdx,2), %ecx                           #1548.17
                                # LOE rdx rbx r12 eax ecx ebp r13d
..B13.34:                       # Preds ..B13.32                # Infreq
        rorw      $8, %cx                                       #1548.17
                                # LOE rdx rbx r12 eax ecx ebp r13d
..B13.35:                       # Preds ..B13.34                # Infreq
        lea       expand_2na.0(%rip), %rsi                      #1548.17
        movw      %cx, (%rsi,%rdx,2)                            #1548.17
        incl      %eax                                          #1548.17
        movl      %eax, %edx                                    #1548.17
        movl      %edx, %eax                                    #1548.17
        cmpl      $256, %eax                                    #1548.17
        jb        ..B13.32      # Prob 99%                      #1548.17
        jmp       ..B13.5       # Prob 100%                     #1548.17
                                # LOE rdx rbx r12 eax ebp r13d
..B13.37:                       # Preds ..B13.7                 # Infreq
        xorl      %eax, %eax                                    #1555.28
        addq      $24, %rsp                                     #1555.28
..___tag_value_NucStrstrMake.738:                               #
        popq      %rbx                                          #1555.28
..___tag_value_NucStrstrMake.740:                               #
        popq      %rbp                                          #1555.28
..___tag_value_NucStrstrMake.742:                               #
        popq      %r12                                          #1555.28
..___tag_value_NucStrstrMake.744:                               #
        popq      %r13                                          #1555.28
..___tag_value_NucStrstrMake.746:                               #
        popq      %r14                                          #1555.28
..___tag_value_NucStrstrMake.748:                               #
        popq      %r15                                          #1555.28
..___tag_value_NucStrstrMake.750:                               #
        ret                                                     #1555.28
        .align    16,0x90
..___tag_value_NucStrstrMake.751:                               #
                                # LOE
# mark_end;
	.type	NucStrstrMake,@function
	.size	NucStrstrMake,.-NucStrstrMake
	.data
# -- End  NucStrstrMake
	.text
# -- Begin  NucStrstrWhack
# mark_begin;
       .align    16,0x90
	.globl NucStrstrWhack
NucStrstrWhack:
# parameter 1: %rdi
..B14.1:                        # Preds ..B14.0
..___tag_value_NucStrstrWhack.752:                              #1574.1
        pushq     %r14                                          #1574.1
..___tag_value_NucStrstrWhack.754:                              #
        movq      %rdi, %r14                                    #1574.1
        testq     %r14, %r14                                    #1575.18
        je        ..B14.22      # Prob 12%                      #1575.18
                                # LOE rbx rbp r12 r13 r14 r15
..B14.2:                        # Preds ..B14.1
        movl      (%r14), %eax                                  #1577.18
        cmpl      $12, %eax                                     #1577.9
        ja        ..B14.21      # Prob 50%                      #1577.9
                                # LOE rbx rbp r12 r13 r14 r15 eax
..B14.3:                        # Preds ..B14.2
        movl      %eax, %eax                                    #1577.9
        lea       ..1..TPKT.18_0.0.16(%rip), %rdx               #1577.9
        movq      (%rdx,%rax,8), %rcx                           #1577.9
        jmp       *%rcx                                         #1577.9
                                # LOE rbx rbp r12 r13 r14 r15
..1.18_0.TAG.0c.0.16:
..B14.5:                        # Preds ..B14.3
        movq      8(%r14), %rdi                                 #1601.13
        call      NucStrstrWhack@PLT                            #1601.13
        jmp       ..B14.21      # Prob 100%                     #1601.13
                                # LOE rbx rbp r12 r13 r14 r15
..1.18_0.TAG.0b.0.16:
..B14.7:                        # Preds ..B14.3
        movq      8(%r14), %rdi                                 #1597.13
        call      NucStrstrWhack@PLT                            #1597.13
                                # LOE rbx rbp r12 r13 r14 r15
..B14.8:                        # Preds ..B14.7
        movq      16(%r14), %rdi                                #1598.13
        call      NucStrstrWhack@PLT                            #1598.13
        jmp       ..B14.21      # Prob 100%                     #1598.13
                                # LOE rbx rbp r12 r13 r14 r15
..1.18_0.TAG.0a.0.16:
..1.18_0.TAG.09.0.16:
..1.18_0.TAG.08.0.16:
..1.18_0.TAG.07.0.16:
..1.18_0.TAG.06.0.16:
..1.18_0.TAG.05.0.16:
..1.18_0.TAG.04.0.16:
..1.18_0.TAG.03.0.16:
..1.18_0.TAG.02.0.16:
..1.18_0.TAG.01.0.16:
..1.18_0.TAG.00.0.16:
..B14.20:                       # Preds ..B14.3 ..B14.3 ..B14.3 ..B14.3 ..B14.3
                                #       ..B14.3 ..B14.3 ..B14.3 ..B14.3 ..B14.3
                                #       ..B14.3
        movq      8(%r14), %r14                                 #1593.20
                                # LOE rbx rbp r12 r13 r14 r15
..B14.21:                       # Preds ..B14.5 ..B14.8 ..B14.20 ..B14.2
        movq      %r14, %rdi                                    #1604.9
        xorl      %eax, %eax                                    #1604.9
        call      free@PLT                                      #1604.9
                                # LOE rbx rbp r12 r13 r15
..B14.22:                       # Preds ..B14.21 ..B14.1
..___tag_value_NucStrstrWhack.757:                              #1606.1
        popq      %r14                                          #1606.1
..___tag_value_NucStrstrWhack.758:                              #
        ret                                                     #1606.1
        .align    16,0x90
..___tag_value_NucStrstrWhack.759:                              #
                                # LOE
# mark_end;
	.type	NucStrstrWhack,@function
	.size	NucStrstrWhack,.-NucStrstrWhack
	.section .data1, "wa"
	.space 24	# pad
	.align 32
..1..TPKT.18_0.0.16:
	.quad	..1.18_0.TAG.00.0.16
	.quad	..1.18_0.TAG.01.0.16
	.quad	..1.18_0.TAG.02.0.16
	.quad	..1.18_0.TAG.03.0.16
	.quad	..1.18_0.TAG.04.0.16
	.quad	..1.18_0.TAG.05.0.16
	.quad	..1.18_0.TAG.06.0.16
	.quad	..1.18_0.TAG.07.0.16
	.quad	..1.18_0.TAG.08.0.16
	.quad	..1.18_0.TAG.09.0.16
	.quad	..1.18_0.TAG.0a.0.16
	.quad	..1.18_0.TAG.0b.0.16
	.quad	..1.18_0.TAG.0c.0.16
	.data
# -- End  NucStrstrWhack
	.text
# -- Begin  nss_unary_expr
# mark_begin;
       .align    16,0x90
nss_unary_expr:
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %rdx
# parameter 4: %rcx
# parameter 5: %r8d
..B15.1:                        # Preds ..B15.0
..___tag_value_nss_unary_expr.760:                              #1361.1
        pushq     %r15                                          #1361.1
..___tag_value_nss_unary_expr.762:                              #
        pushq     %r14                                          #1361.1
..___tag_value_nss_unary_expr.764:                              #
        pushq     %r13                                          #1361.1
..___tag_value_nss_unary_expr.766:                              #
        pushq     %r12                                          #1361.1
..___tag_value_nss_unary_expr.768:                              #
        pushq     %rbp                                          #1361.1
..___tag_value_nss_unary_expr.770:                              #
        pushq     %rbx                                          #1361.1
..___tag_value_nss_unary_expr.772:                              #
        pushq     %rsi                                          #1361.1
..___tag_value_nss_unary_expr.774:                              #
        movl      %r8d, %r14d                                   #1361.1
        movq      %rcx, %r13                                    #1361.1
        movq      %rdx, %r12                                    #1361.1
        movq      %rsi, %rbp                                    #1361.1
        movq      %rdi, %rbx                                    #1361.1
        call      __ctype_b_loc@PLT                             #1364.16
                                # LOE rax rbx rbp r12 r13 r14d
..B15.60:                       # Preds ..B15.1
        movq      %rax, %r15                                    #1364.16
        jmp       ..B15.2       # Prob 100%                     #1364.16
                                # LOE rbx rbp r12 r13 r15 r14d
..B15.48:                       # Preds ..B15.47
        movl      $12, (%rax)                                   #1384.13
        movl      $13, 4(%rax)                                  #1385.13
        movq      $0, 8(%rax)                                   #1386.13
        movq      %rax, (%r12)                                  #1387.13
        lea       8(%rax), %r12                                 #1389.44
                                # LOE rbx rbp r12 r13 r15 r14d
..B15.2:                        # Preds ..B15.48 ..B15.60
        movsbl    (%rbx), %eax                                  #1363.12
        cmpl      $33, %eax                                     #1363.17
        je        ..B15.38      # Prob 33%                      #1363.17
                                # LOE rbx rbp r12 r13 r15 eax r14d
..B15.3:                        # Preds ..B15.2
        cmpl      $94, %eax                                     #1364.16
        jne       ..B15.15      # Prob 67%                      #1364.16
                                # LOE rbx rbp r12 r13 r15 eax r14d
..B15.4:                        # Preds ..B15.3
        movl      $16, %edi                                     #1364.16
        call      malloc@PLT                                    #1364.16
                                # LOE rax rbx rbp r12 r13 r15 r14d
..B15.5:                        # Preds ..B15.4
        testq     %rax, %rax                                    #1364.16
        jne       ..B15.7       # Prob 68%                      #1364.16
                                # LOE rax rbx rbp r12 r13 r15 r14d
..B15.6:                        # Preds ..B15.5
        call      __errno_location@PLT                          #1364.16
                                # LOE rax rbx r13
..B15.62:                       # Preds ..B15.6
        movl      (%rax), %edx                                  #1364.16
        movl      %edx, (%r13)                                  #1364.16
        jmp       ..B15.37      # Prob 100%                     #1364.16
                                # LOE rbx
..B15.7:                        # Preds ..B15.5
        movl      $12, (%rax)                                   #1364.16
        lea       1(%rbx), %rdi                                 #1364.16
        xorl      %r8d, %r8d                                    #1364.16
        cmpq      %rbp, %rdi                                    #1364.16
        movl      $14, 4(%rax)                                  #1364.16
        movq      $0, 8(%rax)                                   #1364.16
        movq      %rax, (%r12)                                  #1364.16
        jae       ..B15.12      # Prob 2%                       #1364.16
                                # LOE rax rbx rbp rdi r8 r13 r15 r14d
..B15.8:                        # Preds ..B15.7
        movq      (%r15), %rsi                                  #1364.16
        movq      %rbp, %rcx                                    #1361.1
        subq      %rbx, %rcx                                    #1361.1
        decq      %rcx                                          #1361.1
                                # LOE rax rcx rbx rbp rsi rdi r8 r13 r14d
..B15.9:                        # Preds ..B15.10 ..B15.8
        movzbl    1(%r8,%rbx), %r9d                             #1364.16
        movzwl    (%rsi,%r9,2), %r10d                           #1364.16
        testl     $8192, %r10d                                  #1364.16
        je        ..B15.12      # Prob 20%                      #1364.16
                                # LOE rax rcx rbx rbp rsi rdi r8 r13 r14d
..B15.10:                       # Preds ..B15.9
        lea       2(%r8,%rbx), %rdi                             #1364.16
        incq      %r8                                           #1364.16
        cmpq      %rcx, %r8                                     #1364.16
        jb        ..B15.9       # Prob 1%                       #1364.16
                                # LOE rax rcx rbx rbp rsi rdi r8 r13 r14d
..B15.12:                       # Preds ..B15.9 ..B15.10 ..B15.7
        addq      $8, %rax                                      #1364.16
        movq      %rbp, %rsi                                    #1364.16
        movq      %rax, %rdx                                    #1364.16
        movq      %r13, %rcx                                    #1364.16
        movl      %r14d, %r8d                                   #1364.16
        call      nss_fasta_expr@PLT                            #1364.16
                                # LOE rax
..B15.63:                       # Preds ..B15.12
        movq      %rax, %rbx                                    #1364.16
        jmp       ..B15.37      # Prob 100%                     #1364.16
                                # LOE rbx
..B15.15:                       # Preds ..B15.3
        cmpl      $40, %eax                                     #1364.16
        jne       ..B15.27      # Prob 50%                      #1364.16
                                # LOE rbx rbp r12 r13 r15 r14d
..B15.16:                       # Preds ..B15.15
        movl      $16, %edi                                     #1364.16
        call      malloc@PLT                                    #1364.16
                                # LOE rax rbx rbp r12 r13 r14d
..B15.64:                       # Preds ..B15.16
        movq      %rax, %r15                                    #1364.16
                                # LOE rbx rbp r12 r13 r15 r14d
..B15.17:                       # Preds ..B15.64
        testq     %r15, %r15                                    #1364.16
        jne       ..B15.19      # Prob 68%                      #1364.16
                                # LOE rbx rbp r12 r13 r15 r14d
..B15.18:                       # Preds ..B15.17
        call      __errno_location@PLT                          #1364.16
                                # LOE rax rbx r13
..B15.65:                       # Preds ..B15.18
        movl      (%rax), %edx                                  #1364.16
        movl      %edx, (%r13)                                  #1364.16
        jmp       ..B15.37      # Prob 100%                     #1364.16
                                # LOE rbx
..B15.19:                       # Preds ..B15.17
        movl      $12, (%r15)                                   #1364.16
        incq      %rbx                                          #1364.16
        lea       8(%r15), %rdx                                 #1364.16
        movq      %rbx, %rdi                                    #1364.16
        movl      $0, 4(%r15)                                   #1364.16
        movq      %r15, (%r12)                                  #1364.16
        movq      %rbp, %rsi                                    #1364.16
        movq      %r13, %rcx                                    #1364.16
        movl      %r14d, %r8d                                   #1364.16
        call      nss_expr@PLT                                  #1364.16
                                # LOE rax rbp r13 r15
..B15.66:                       # Preds ..B15.19
        movq      %rax, %rbx                                    #1364.16
                                # LOE rbx rbp r13 r15
..B15.20:                       # Preds ..B15.66
        movl      (%r13), %eax                                  #1364.16
        testl     %eax, %eax                                    #1364.16
        jne       ..B15.37      # Prob 50%                      #1364.16
                                # LOE rbx rbp r13 r15
..B15.21:                       # Preds ..B15.20
        movq      8(%r15), %rax                                 #1364.16
        testq     %rax, %rax                                    #1364.16
        je        ..B15.50      # Prob 12%                      #1364.16
                                # LOE rbx rbp r13
..B15.22:                       # Preds ..B15.21
        cmpq      %rbp, %rbx                                    #1364.16
        je        ..B15.50      # Prob 12%                      #1364.16
                                # LOE rbx r13
..B15.23:                       # Preds ..B15.22
        movsbl    (%rbx), %eax                                  #1364.16
        cmpl      $41, %eax                                     #1364.16
        je        ..B15.25      # Prob 16%                      #1364.16
                                # LOE rbx r13
..B15.24:                       # Preds ..B15.23
        movl      $22, (%r13)                                   #1364.16
        incq      %rbx                                          #1364.16
        jmp       ..B15.37      # Prob 100%                     #1364.16
                                # LOE rbx
..B15.25:                       # Preds ..B15.23
        incq      %rbx                                          #1364.16
        jmp       ..B15.37      # Prob 100%                     #1364.16
                                # LOE rbx
..B15.27:                       # Preds ..B15.15
        movq      %rbx, %rdi                                    #1364.16
        movq      %rbp, %rsi                                    #1364.16
        movq      %r12, %rdx                                    #1364.16
        movq      %r13, %rcx                                    #1364.16
        movl      %r14d, %r8d                                   #1364.16
        call      nss_fasta_expr@PLT                            #1364.16
                                # LOE rax rbp r12 r13 r15
..B15.28:                       # Preds ..B15.27
        movl      (%r13), %edx                                  #1364.16
        movq      %rax, %rbx                                    #1364.16
        testl     %edx, %edx                                    #1364.16
        jne       ..B15.37      # Prob 50%                      #1364.16
                                # LOE rax rbx rbp r12 r13 r15
..B15.29:                       # Preds ..B15.28
        cmpq      %rbp, %rax                                    #1364.16
        jae       ..B15.37      # Prob 33%                      #1364.16
                                # LOE rax rbx rbp r12 r13 r15
..B15.30:                       # Preds ..B15.29
        movq      (%r15), %rsi                                  #1364.16
        movq      %rax, %rbx                                    #1364.16
        xorl      %edx, %edx                                    #1364.16
        movq      %rbp, %rcx                                    #1361.1
        subq      %rax, %rcx                                    #1361.1
                                # LOE rax rdx rcx rbx rbp rsi r12 r13
..B15.31:                       # Preds ..B15.30 ..B15.32
        movzbl    (%rdx,%rax), %edi                             #1364.16
        movzwl    (%rsi,%rdi,2), %r8d                           #1364.16
        testl     $8192, %r8d                                   #1364.16
        je        ..B15.34      # Prob 20%                      #1364.16
                                # LOE rax rdx rcx rbx rbp rsi r12 r13
..B15.32:                       # Preds ..B15.31
        lea       1(%rdx,%rax), %rbx                            #1364.16
        incq      %rdx                                          #1364.16
        cmpq      %rcx, %rdx                                    #1364.16
        jb        ..B15.31      # Prob 1%                       #1364.16
                                # LOE rax rdx rcx rbx rbp rsi r12 r13
..B15.34:                       # Preds ..B15.31 ..B15.32
        cmpq      %rbp, %rbx                                    #1364.16
        jae       ..B15.37      # Prob 12%                      #1364.16
                                # LOE rbx r12 r13
..B15.35:                       # Preds ..B15.34
        movsbl    (%rbx), %eax                                  #1364.16
        cmpl      $36, %eax                                     #1364.16
        je        ..B15.51      # Prob 5%                       #1364.16
                                # LOE rbx r12 r13
..B15.37:                       # Preds ..B15.28 ..B15.29 ..B15.34 ..B15.35 ..B15.53
                                #       ..B15.70 ..B15.20 ..B15.50 ..B15.65 ..B15.62
                                #       ..B15.63 ..B15.24 ..B15.25
        movq      %rbx, %rax                                    #1364.16
        popq      %rcx                                          #1364.16
..___tag_value_nss_unary_expr.775:                              #
        popq      %rbx                                          #1364.16
..___tag_value_nss_unary_expr.777:                              #
        popq      %rbp                                          #1364.16
..___tag_value_nss_unary_expr.779:                              #
        popq      %r12                                          #1364.16
..___tag_value_nss_unary_expr.781:                              #
        popq      %r13                                          #1364.16
..___tag_value_nss_unary_expr.783:                              #
        popq      %r14                                          #1364.16
..___tag_value_nss_unary_expr.785:                              #
        popq      %r15                                          #1364.16
..___tag_value_nss_unary_expr.787:                              #
        ret                                                     #1364.16
..___tag_value_nss_unary_expr.788:                              #
                                # LOE
..B15.38:                       # Preds ..B15.2
        incq      %rbx                                          #1374.19
        movq      %rbx, %rsi                                    #1374.9
        xorl      %ecx, %ecx                                    #1374.9
        cmpq      %rbp, %rbx                                    #1374.9
        jae       ..B15.43      # Prob 2%                       #1374.9
                                # LOE rcx rbx rbp rsi r12 r13 r15 r14d
..B15.39:                       # Preds ..B15.38
        movq      (%r15), %rdx                                  #1374.9
        movq      %rbp, %rax                                    #1361.1
        subq      %rbx, %rax                                    #1361.1
                                # LOE rax rdx rcx rbx rbp rsi r12 r13 r15 r14d
..B15.40:                       # Preds ..B15.41 ..B15.39
        movzbl    (%rcx,%rsi), %edi                             #1374.9
        movzwl    (%rdx,%rdi,2), %r8d                           #1374.9
        testl     $8192, %r8d                                   #1374.9
        je        ..B15.43      # Prob 20%                      #1374.9
                                # LOE rax rdx rcx rbx rbp rsi r12 r13 r15 r14d
..B15.41:                       # Preds ..B15.40
        lea       1(%rcx,%rsi), %rbx                            #1374.9
        incq      %rcx                                          #1374.9
        cmpq      %rax, %rcx                                    #1374.9
        jb        ..B15.40      # Prob 16%                      #1374.9
                                # LOE rax rdx rcx rbx rbp rsi r12 r13 r15 r14d
..B15.43:                       # Preds ..B15.40 ..B15.41 ..B15.38
        cmpq      %rbp, %rbx                                    #1375.15
        jne       ..B15.46      # Prob 68%                      #1375.15
                                # LOE rbx rbp r12 r13 r15 r14d
..B15.44:                       # Preds ..B15.43
        movl      $22, (%r13)                                   #1376.9
                                # LOE rbx
..B15.45:                       # Preds ..B15.44 ..B15.71
        movq      %rbx, %rax                                    #1393.12
        popq      %rcx                                          #1393.12
..___tag_value_nss_unary_expr.795:                              #
        popq      %rbx                                          #1393.12
..___tag_value_nss_unary_expr.797:                              #
        popq      %rbp                                          #1393.12
..___tag_value_nss_unary_expr.799:                              #
        popq      %r12                                          #1393.12
..___tag_value_nss_unary_expr.801:                              #
        popq      %r13                                          #1393.12
..___tag_value_nss_unary_expr.803:                              #
        popq      %r14                                          #1393.12
..___tag_value_nss_unary_expr.805:                              #
        popq      %r15                                          #1393.12
..___tag_value_nss_unary_expr.807:                              #
        ret                                                     #1393.12
..___tag_value_nss_unary_expr.808:                              #
                                # LOE
..B15.46:                       # Preds ..B15.43
        movl      $16, %edi                                     #1379.25
        call      malloc@PLT                                    #1379.25
                                # LOE rax rbx rbp r12 r13 r15 r14d
..B15.47:                       # Preds ..B15.46
        testq     %rax, %rax                                    #1380.19
        je        ..B15.56      # Prob 1%                       #1380.19
        jmp       ..B15.48      # Prob 100%                     #1380.19
                                # LOE rax rbx rbp r12 r13 r15 r14d
..B15.50:                       # Preds ..B15.21 ..B15.22       # Infreq
        movl      $22, (%r13)                                   #1364.16
        jmp       ..B15.37      # Prob 100%                     #1364.16
                                # LOE rbx
..B15.51:                       # Preds ..B15.35                # Infreq
        incq      %rbx                                          #1364.16
        movl      $16, %edi                                     #1364.16
        call      malloc@PLT                                    #1364.16
                                # LOE rax rbx r12 r13
..B15.52:                       # Preds ..B15.51                # Infreq
        testq     %rax, %rax                                    #1364.16
        je        ..B15.54      # Prob 12%                      #1364.16
                                # LOE rax rbx r12 r13
..B15.53:                       # Preds ..B15.52                # Infreq
        movl      $12, (%rax)                                   #1364.16
        movl      $15, 4(%rax)                                  #1364.16
        movq      (%r12), %rdx                                  #1364.16
        movq      %rdx, 8(%rax)                                 #1364.16
        movq      %rax, (%r12)                                  #1364.16
        jmp       ..B15.37      # Prob 100%                     #1364.16
                                # LOE rbx
..B15.54:                       # Preds ..B15.52                # Infreq
        call      __errno_location@PLT                          #1364.16
                                # LOE rax rbx r13
..B15.70:                       # Preds ..B15.54                # Infreq
        movl      (%rax), %edx                                  #1364.16
        movl      %edx, (%r13)                                  #1364.16
        jmp       ..B15.37      # Prob 100%                     #1364.16
                                # LOE rbx
..B15.56:                       # Preds ..B15.47                # Infreq
        call      __errno_location@PLT                          #1381.24
                                # LOE rax rbx r13
..B15.71:                       # Preds ..B15.56                # Infreq
        movl      (%rax), %edx                                  #1381.24
        movl      %edx, (%r13)                                  #1381.13
        jmp       ..B15.45      # Prob 100%                     #1381.13
        .align    16,0x90
..___tag_value_nss_unary_expr.815:                              #
                                # LOE rbx
# mark_end;
	.type	nss_unary_expr,@function
	.size	nss_unary_expr,.-nss_unary_expr
	.data
# -- End  nss_unary_expr
	.text
# -- Begin  nss_fasta_expr
# mark_begin;
       .align    16,0x90
nss_fasta_expr:
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %rdx
# parameter 4: %rcx
# parameter 5: %r8d
..B16.1:                        # Preds ..B16.0
..___tag_value_nss_fasta_expr.816:                              #1234.1
        pushq     %r14                                          #1234.1
..___tag_value_nss_fasta_expr.818:                              #
        pushq     %r12                                          #1234.1
..___tag_value_nss_fasta_expr.820:                              #
        pushq     %rbx                                          #1234.1
..___tag_value_nss_fasta_expr.822:                              #
        movq      %rcx, %r12                                    #1234.1
        movq      %rsi, %rbx                                    #1234.1
        movq      %rdi, %rax                                    #1234.1
        movsbl    (%rax), %ecx                                  #1236.16
        cmpl      $39, %ecx                                     #1236.16
        jne       ..B16.11      # Prob 67%                      #1236.16
                                # LOE rax rdx rbx rbp r12 r13 r15 ecx r8d
..B16.2:                        # Preds ..B16.1
        incq      %rax                                          #1239.30
        movq      %rax, %r14                                    #1239.30
        cmpq      %rbx, %rax                                    #1239.13
        jb        ..B16.4       # Prob 56%                      #1239.13
                                # LOE rax rdx rbx rbp r12 r13 r14 r15 r8d
..B16.3:                        # Preds ..B16.56 ..B16.38 ..B16.2
        movl      $22, (%r12)                                   #1239.13
..___tag_value_nss_fasta_expr.825:                              #1239.13
        popq      %rbx                                          #1239.13
..___tag_value_nss_fasta_expr.826:                              #
        popq      %r12                                          #1239.13
..___tag_value_nss_fasta_expr.828:                              #
        popq      %r14                                          #1239.13
..___tag_value_nss_fasta_expr.830:                              #
        ret                                                     #1239.13
..___tag_value_nss_fasta_expr.831:                              #
                                # LOE rax rbp r13 r15
..B16.4:                        # Preds ..B16.2
        xorl      %r9d, %r9d                                    #1239.13
        lea       fasta_2na_map.0(%rip), %rcx                   #1239.13
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15 r8d r9d
..B16.5:                        # Preds ..B16.7 ..B16.4
        movsbl    (%r14), %r10d                                 #1239.13
        testl     %r10d, %r10d                                  #1239.13
        jl        ..B16.9       # Prob 20%                      #1239.13
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15 r8d r9d
..B16.6:                        # Preds ..B16.5
        movzbl    (%r14), %r10d                                 #1239.13
        movsbl    (%r10,%rcx), %r11d                            #1239.13
        testl     %r11d, %r11d                                  #1239.13
        jl        ..B16.44      # Prob 16%                      #1239.13
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15 r8d r9d r10d
..B16.7:                        # Preds ..B16.46 ..B16.6
        incq      %r14                                          #1239.13
        cmpq      %rbx, %r14                                    #1239.13
        jb        ..B16.5       # Prob 82%                      #1239.13
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15 r8d r9d
..B16.9:                        # Preds ..B16.45 ..B16.44 ..B16.5 ..B16.7
        cmpq      %rax, %r14                                    #1239.13
        ja        ..B16.32      # Prob 12%                      #1239.13
                                # LOE rax rdx rbx rbp r12 r13 r14 r15 r8d r9d
..B16.10:                       # Preds ..B16.9
        movl      $22, (%r12)                                   #1239.13
        movq      %r14, %rax                                    #1239.13
..___tag_value_nss_fasta_expr.835:                              #1239.13
        popq      %rbx                                          #1239.13
..___tag_value_nss_fasta_expr.836:                              #
        popq      %r12                                          #1239.13
..___tag_value_nss_fasta_expr.838:                              #
        popq      %r14                                          #1239.13
..___tag_value_nss_fasta_expr.840:                              #
        ret                                                     #1239.13
..___tag_value_nss_fasta_expr.841:                              #
                                # LOE rax rbp r13 r15
..B16.11:                       # Preds ..B16.1
        cmpl      $34, %ecx                                     #1236.16
        jne       ..B16.22      # Prob 50%                      #1236.16
                                # LOE rax rdx rbx rbp r12 r13 r15 r8d
..B16.12:                       # Preds ..B16.11
        incq      %rax                                          #1244.30
        movq      %rax, %r14                                    #1244.30
        cmpq      %rbx, %rax                                    #1244.13
        jb        ..B16.14      # Prob 56%                      #1244.13
                                # LOE rax rdx rbx rbp r12 r13 r14 r15 r8d
..B16.13:                       # Preds ..B16.12
        movl      $22, (%r12)                                   #1244.13
..___tag_value_nss_fasta_expr.845:                              #1244.13
        popq      %rbx                                          #1244.13
..___tag_value_nss_fasta_expr.846:                              #
        popq      %r12                                          #1244.13
..___tag_value_nss_fasta_expr.848:                              #
        popq      %r14                                          #1244.13
..___tag_value_nss_fasta_expr.850:                              #
        ret                                                     #1244.13
..___tag_value_nss_fasta_expr.851:                              #
                                # LOE rax rbp r13 r15
..B16.14:                       # Preds ..B16.12
        xorl      %r9d, %r9d                                    #1244.13
        lea       fasta_2na_map.0(%rip), %rcx                   #1244.13
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15 r8d r9d
..B16.15:                       # Preds ..B16.17 ..B16.14
        movsbl    (%r14), %r10d                                 #1244.13
        testl     %r10d, %r10d                                  #1244.13
        jl        ..B16.19      # Prob 20%                      #1244.13
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15 r8d r9d
..B16.16:                       # Preds ..B16.15
        movzbl    (%r14), %r10d                                 #1244.13
        movsbl    (%r10,%rcx), %r11d                            #1244.13
        testl     %r11d, %r11d                                  #1244.13
        jl        ..B16.62      # Prob 16%                      #1244.13
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15 r8d r9d r10d
..B16.17:                       # Preds ..B16.64 ..B16.16
        incq      %r14                                          #1244.13
        cmpq      %rbx, %r14                                    #1244.13
        jb        ..B16.15      # Prob 82%                      #1244.13
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15 r8d r9d
..B16.19:                       # Preds ..B16.63 ..B16.62 ..B16.15 ..B16.17
        cmpq      %rax, %r14                                    #1244.13
        ja        ..B16.50      # Prob 12%                      #1244.13
                                # LOE rax rdx rbx rbp r12 r13 r14 r15 r8d r9d
..B16.20:                       # Preds ..B16.19
        movl      $22, (%r12)                                   #1244.13
        movq      %r14, %rax                                    #1244.13
                                # LOE rax rbp r13 r15
..B16.21:                       # Preds ..B16.20
..___tag_value_nss_fasta_expr.855:                              #1252.12
        popq      %rbx                                          #1252.12
..___tag_value_nss_fasta_expr.856:                              #
        popq      %r12                                          #1252.12
..___tag_value_nss_fasta_expr.858:                              #
        popq      %r14                                          #1252.12
..___tag_value_nss_fasta_expr.860:                              #
        ret                                                     #1252.12
..___tag_value_nss_fasta_expr.861:                              #
                                # LOE
..B16.22:                       # Preds ..B16.11
        movq      %rax, %r14                                    #1234.1
        cmpq      %rbx, %rax                                    #1249.16
        jae       ..B16.30      # Prob 44%                      #1249.16
                                # LOE rax rdx rbx rbp r12 r13 r14 r15 r8d
..B16.24:                       # Preds ..B16.22
        xorl      %r9d, %r9d                                    #1249.16
        lea       fasta_2na_map.0(%rip), %rcx                   #1249.16
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15 r8d r9d
..B16.25:                       # Preds ..B16.27 ..B16.24
        movsbl    (%r14), %r10d                                 #1249.16
        testl     %r10d, %r10d                                  #1249.16
        jl        ..B16.29      # Prob 20%                      #1249.16
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15 r8d r9d
..B16.26:                       # Preds ..B16.25
        movzbl    (%r14), %r10d                                 #1249.16
        movsbl    (%r10,%rcx), %r11d                            #1249.16
        testl     %r11d, %r11d                                  #1249.16
        jl        ..B16.73      # Prob 16%                      #1249.16
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15 r8d r9d r10d
..B16.27:                       # Preds ..B16.75 ..B16.26
        incq      %r14                                          #1249.16
        cmpq      %rbx, %r14                                    #1249.16
        jb        ..B16.25      # Prob 82%                      #1249.16
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15 r8d r9d
..B16.29:                       # Preds ..B16.74 ..B16.73 ..B16.25 ..B16.27
        cmpq      %rax, %r14                                    #1249.16
        ja        ..B16.68      # Prob 12%                      #1249.16
                                # LOE rax rdx rbp r12 r13 r14 r15 r8d r9d
..B16.30:                       # Preds ..B16.22 ..B16.29
        movl      $22, (%r12)                                   #1249.16
                                # LOE rbp r13 r14 r15
..B16.31:                       # Preds ..B16.30 ..B16.70 ..B16.72
        movq      %r14, %rax                                    #1249.16
..___tag_value_nss_fasta_expr.865:                              #1249.16
        popq      %rbx                                          #1249.16
..___tag_value_nss_fasta_expr.866:                              #
        popq      %r12                                          #1249.16
..___tag_value_nss_fasta_expr.868:                              #
        popq      %r14                                          #1249.16
..___tag_value_nss_fasta_expr.870:                              #
        ret                                                     #1249.16
..___tag_value_nss_fasta_expr.871:                              #
                                # LOE
..B16.32:                       # Preds ..B16.9                 # Infreq
        testl     %r9d, %r9d                                    #1239.13
        jne       ..B16.35      # Prob 50%                      #1239.13
                                # LOE rax rdx rbx rbp r12 r13 r14 r15 r8d
..B16.33:                       # Preds ..B16.32                # Infreq
        movq      %r14, %rcx                                    #1239.13
        subq      %rax, %rcx                                    #1239.13
        movq      %rdx, %rdi                                    #1239.13
        movl      %r8d, %esi                                    #1239.13
        movq      %rax, %rdx                                    #1239.13
        call      NucStrFastaExprMake2@PLT                      #1239.13
        jmp       ..B16.82      # Prob 100%                     #1239.13
                                # LOE rbx rbp r12 r13 r14 r15 eax
..B16.35:                       # Preds ..B16.32                # Infreq
        movq      %r14, %rcx                                    #1239.13
        subq      %rax, %rcx                                    #1239.13
        movq      %rdx, %rdi                                    #1239.13
        movl      %r8d, %esi                                    #1239.13
        movq      %rax, %rdx                                    #1239.13
        call      NucStrFastaExprMake4@PLT                      #1239.13
                                # LOE rbx rbp r12 r13 r14 r15 eax
..B16.82:                       # Preds ..B16.33 ..B16.35       # Infreq
        movl      %eax, %esi                                    #1239.13
                                # LOE rbx rbp r12 r13 r14 r15 esi
..B16.37:                       # Preds ..B16.82                # Infreq
        movq      %r14, %rax                                    #1239.13
        testl     %esi, %esi                                    #1240.26
        jne       ..B16.41      # Prob 50%                      #1240.26
                                # LOE rax rbx rbp r12 r13 r14 r15 esi
..B16.38:                       # Preds ..B16.37                # Infreq
        cmpq      %rbx, %r14                                    #1240.38
        je        ..B16.3       # Prob 12%                      #1240.38
                                # LOE rax rbp r12 r13 r14 r15 esi
..B16.39:                       # Preds ..B16.38                # Infreq
        movsbl    (%r14), %ecx                                  #1240.47
        lea       1(%r14), %rax                                 #1240.47
        cmpl      $39, %ecx                                     #1240.55
        je        ..B16.41      # Prob 16%                      #1240.55
                                # LOE rax rbp r12 r13 r15 esi
..B16.40:                       # Preds ..B16.39                # Infreq
        movl      $22, (%r12)                                   #1241.13
..___tag_value_nss_fasta_expr.875:                              #1241.13
        popq      %rbx                                          #1241.13
..___tag_value_nss_fasta_expr.876:                              #
        popq      %r12                                          #1241.13
..___tag_value_nss_fasta_expr.878:                              #
        popq      %r14                                          #1241.13
..___tag_value_nss_fasta_expr.880:                              #
        ret                                                     #1241.13
..___tag_value_nss_fasta_expr.881:                              #
                                # LOE rax rbp r13 r15
..B16.41:                       # Preds ..B16.55 ..B16.57 ..B16.37 ..B16.39 # Infreq
        movl      %esi, (%r12)                                  #1241.13
..___tag_value_nss_fasta_expr.885:                              #1241.13
        popq      %rbx                                          #1241.13
..___tag_value_nss_fasta_expr.886:                              #
        popq      %r12                                          #1241.13
..___tag_value_nss_fasta_expr.888:                              #
        popq      %r14                                          #1241.13
..___tag_value_nss_fasta_expr.890:                              #
        ret                                                     #1241.13
..___tag_value_nss_fasta_expr.891:                              #
                                # LOE rax rbp r13 r15
..B16.44:                       # Preds ..B16.6                 # Infreq
        lea       fasta_4na_map.0(%rip), %r11                   #1239.13
        cmpq      %r11, %rcx                                    #1239.13
        je        ..B16.9       # Prob 20%                      #1239.13
                                # LOE rax rdx rbx rbp r12 r13 r14 r15 r8d r9d r10d
..B16.45:                       # Preds ..B16.44                # Infreq
        movl      %r10d, %r10d                                  #1239.13
        lea       fasta_4na_map.0(%rip), %rcx                   #1239.13
        movsbl    (%rcx,%r10), %r11d                            #1239.13
        testl     %r11d, %r11d                                  #1239.13
        jl        ..B16.9       # Prob 20%                      #1239.13
                                # LOE rax rdx rbx rbp r12 r13 r14 r15 r8d r9d
..B16.46:                       # Preds ..B16.45                # Infreq
        movl      $1, %r9d                                      #1239.13
        lea       fasta_4na_map.0(%rip), %rcx                   #1239.13
        jmp       ..B16.7       # Prob 100%                     #1239.13
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15 r8d r9d
..B16.50:                       # Preds ..B16.19                # Infreq
        testl     %r9d, %r9d                                    #1244.13
        jne       ..B16.53      # Prob 50%                      #1244.13
                                # LOE rax rdx rbx rbp r12 r13 r14 r15 r8d
..B16.51:                       # Preds ..B16.50                # Infreq
        movq      %r14, %rcx                                    #1244.13
        subq      %rax, %rcx                                    #1244.13
        movq      %rdx, %rdi                                    #1244.13
        movl      %r8d, %esi                                    #1244.13
        movq      %rax, %rdx                                    #1244.13
        call      NucStrFastaExprMake2@PLT                      #1244.13
        jmp       ..B16.84      # Prob 100%                     #1244.13
                                # LOE rbx rbp r12 r13 r14 r15 eax
..B16.53:                       # Preds ..B16.50                # Infreq
        movq      %r14, %rcx                                    #1244.13
        subq      %rax, %rcx                                    #1244.13
        movq      %rdx, %rdi                                    #1244.13
        movl      %r8d, %esi                                    #1244.13
        movq      %rax, %rdx                                    #1244.13
        call      NucStrFastaExprMake4@PLT                      #1244.13
                                # LOE rbx rbp r12 r13 r14 r15 eax
..B16.84:                       # Preds ..B16.51 ..B16.53       # Infreq
        movl      %eax, %esi                                    #1244.13
                                # LOE rbx rbp r12 r13 r14 r15 esi
..B16.55:                       # Preds ..B16.84                # Infreq
        movq      %r14, %rax                                    #1244.13
        testl     %esi, %esi                                    #1245.26
        jne       ..B16.41      # Prob 50%                      #1245.26
                                # LOE rax rbx rbp r12 r13 r14 r15 esi
..B16.56:                       # Preds ..B16.55                # Infreq
        cmpq      %rbx, %r14                                    #1245.38
        je        ..B16.3       # Prob 12%                      #1245.38
                                # LOE rax rbp r12 r13 r14 r15 esi
..B16.57:                       # Preds ..B16.56                # Infreq
        movsbl    (%r14), %ecx                                  #1245.47
        lea       1(%r14), %rax                                 #1245.47
        cmpl      $34, %ecx                                     #1245.55
        je        ..B16.41      # Prob 16%                      #1245.55
                                # LOE rax rbp r12 r13 r15 esi
..B16.58:                       # Preds ..B16.57                # Infreq
        movl      $22, (%r12)                                   #1246.13
..___tag_value_nss_fasta_expr.895:                              #1246.13
        popq      %rbx                                          #1246.13
..___tag_value_nss_fasta_expr.896:                              #
        popq      %r12                                          #1246.13
..___tag_value_nss_fasta_expr.898:                              #
        popq      %r14                                          #1246.13
..___tag_value_nss_fasta_expr.900:                              #
        ret                                                     #1246.13
..___tag_value_nss_fasta_expr.901:                              #
                                # LOE rax rbp r13 r15
..B16.62:                       # Preds ..B16.16                # Infreq
        lea       fasta_4na_map.0(%rip), %r11                   #1244.13
        cmpq      %r11, %rcx                                    #1244.13
        je        ..B16.19      # Prob 20%                      #1244.13
                                # LOE rax rdx rbx rbp r12 r13 r14 r15 r8d r9d r10d
..B16.63:                       # Preds ..B16.62                # Infreq
        movl      %r10d, %r10d                                  #1244.13
        lea       fasta_4na_map.0(%rip), %rcx                   #1244.13
        movsbl    (%rcx,%r10), %r11d                            #1244.13
        testl     %r11d, %r11d                                  #1244.13
        jl        ..B16.19      # Prob 20%                      #1244.13
                                # LOE rax rdx rbx rbp r12 r13 r14 r15 r8d r9d
..B16.64:                       # Preds ..B16.63                # Infreq
        movl      $1, %r9d                                      #1244.13
        lea       fasta_4na_map.0(%rip), %rcx                   #1244.13
        jmp       ..B16.17      # Prob 100%                     #1244.13
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15 r8d r9d
..B16.68:                       # Preds ..B16.29                # Infreq
        testl     %r9d, %r9d                                    #1249.16
        jne       ..B16.71      # Prob 50%                      #1249.16
                                # LOE rax rdx rbp r12 r13 r14 r15 r8d
..B16.69:                       # Preds ..B16.68                # Infreq
        movq      %r14, %rcx                                    #1249.16
        subq      %rax, %rcx                                    #1249.16
        movq      %rdx, %rdi                                    #1249.16
        movl      %r8d, %esi                                    #1249.16
        movq      %rax, %rdx                                    #1249.16
        call      NucStrFastaExprMake2@PLT                      #1249.16
                                # LOE rbp r12 r13 r14 r15 eax
..B16.70:                       # Preds ..B16.69                # Infreq
        movl      %eax, (%r12)                                  #1249.16
        jmp       ..B16.31      # Prob 100%                     #1249.16
                                # LOE rbp r13 r14 r15
..B16.71:                       # Preds ..B16.68                # Infreq
        movq      %r14, %rcx                                    #1249.16
        subq      %rax, %rcx                                    #1249.16
        movq      %rdx, %rdi                                    #1249.16
        movl      %r8d, %esi                                    #1249.16
        movq      %rax, %rdx                                    #1249.16
        call      NucStrFastaExprMake4@PLT                      #1249.16
                                # LOE rbp r12 r13 r14 r15 eax
..B16.72:                       # Preds ..B16.71                # Infreq
        movl      %eax, (%r12)                                  #1249.16
        jmp       ..B16.31      # Prob 100%                     #1249.16
                                # LOE rbp r13 r14 r15
..B16.73:                       # Preds ..B16.26                # Infreq
        lea       fasta_4na_map.0(%rip), %r11                   #1249.16
        cmpq      %r11, %rcx                                    #1249.16
        je        ..B16.29      # Prob 20%                      #1249.16
                                # LOE rax rdx rbx rbp r12 r13 r14 r15 r8d r9d r10d
..B16.74:                       # Preds ..B16.73                # Infreq
        movl      %r10d, %r10d                                  #1249.16
        lea       fasta_4na_map.0(%rip), %rcx                   #1249.16
        movsbl    (%rcx,%r10), %r11d                            #1249.16
        testl     %r11d, %r11d                                  #1249.16
        jl        ..B16.29      # Prob 20%                      #1249.16
                                # LOE rax rdx rbx rbp r12 r13 r14 r15 r8d r9d
..B16.75:                       # Preds ..B16.74                # Infreq
        movl      $1, %r9d                                      #1249.16
        lea       fasta_4na_map.0(%rip), %rcx                   #1249.16
        jmp       ..B16.27      # Prob 100%                     #1249.16
        .align    16,0x90
..___tag_value_nss_fasta_expr.905:                              #
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15 r8d r9d
# mark_end;
	.type	nss_fasta_expr,@function
	.size	nss_fasta_expr,.-nss_fasta_expr
	.data
# -- End  nss_fasta_expr
	.text
# -- Begin  NucStrFastaExprMake2
# mark_begin;
       .align    16,0x90
NucStrFastaExprMake2:
# parameter 1: %rdi
# parameter 2: %esi
# parameter 3: %rdx
# parameter 4: %rcx
..B17.1:                        # Preds ..B17.0
..___tag_value_NucStrFastaExprMake2.906:                        #838.1
        pushq     %r13                                          #838.1
..___tag_value_NucStrFastaExprMake2.908:                        #
        pushq     %r12                                          #838.1
..___tag_value_NucStrFastaExprMake2.910:                        #
        pushq     %rbp                                          #838.1
..___tag_value_NucStrFastaExprMake2.912:                        #
        pushq     %rbx                                          #838.1
..___tag_value_NucStrFastaExprMake2.914:                        #
        subq      $40, %rsp                                     #838.1
..___tag_value_NucStrFastaExprMake2.916:                        #
        movq      %rcx, %r12                                    #838.1
        movq      %rdx, %rbp                                    #838.1
        movl      %esi, %r13d                                   #838.1
        movq      %rdi, %rbx                                    #838.1
        cmpq      $61, %r12                                     #844.17
        jbe       ..B17.3       # Prob 43%                      #844.17
                                # LOE rbx rbp r12 r14 r15 r13d
..B17.2:                        # Preds ..B17.1
        movl      $7, %eax                                      #845.16
        addq      $40, %rsp                                     #845.16
..___tag_value_NucStrFastaExprMake2.917:                        #
        popq      %rbx                                          #845.16
..___tag_value_NucStrFastaExprMake2.919:                        #
        popq      %rbp                                          #845.16
..___tag_value_NucStrFastaExprMake2.921:                        #
        popq      %r12                                          #845.16
..___tag_value_NucStrFastaExprMake2.923:                        #
        popq      %r13                                          #845.16
..___tag_value_NucStrFastaExprMake2.925:                        #
        ret                                                     #845.16
..___tag_value_NucStrFastaExprMake2.926:                        #
                                # LOE
..B17.3:                        # Preds ..B17.1
        movl      $160, %edi                                    #847.9
        call      malloc@PLT                                    #847.9
                                # LOE rax rbx rbp r12 r14 r15 r13d
..B17.4:                        # Preds ..B17.3
        testq     %rax, %rax                                    #847.9
        je        ..B17.44      # Prob 12%                      #847.9
                                # LOE rax rbx rbp r12 r14 r15 r13d
..B17.5:                        # Preds ..B17.4
        lea       15(%rax), %rdx                                #847.9
        andq      $-16, %rdx                                    #847.9
        movq      %rax, 8(%rdx)                                 #847.9
        je        ..B17.44      # Prob 5%                       #848.15
                                # LOE rdx rbx rbp r12 r14 r15 r13d
..B17.6:                        # Preds ..B17.5
        movq      %rdx, (%rbx)                                  #851.5
        movl      %r12d, 4(%rdx)                                #852.5
        xorl      %esi, %esi                                    #856.11
        testq     %r12, %r12                                    #856.22
        jbe       ..B17.17      # Prob 16%                      #856.22
                                # LOE rdx rbp rsi r12 r14 r15 r13d
..B17.8:                        # Preds ..B17.6 ..B17.15
        movsbq    (%rsi,%rbp), %rcx                             #858.48
        lea       fasta_2na_map.0(%rip), %rax                   #858.24
        movzbl    (%rax,%rcx), %ecx                             #858.24
        movq      %rsi, %rax                                    #859.22
        andq      $3, %rax                                      #859.22
        je        ..B17.43      # Prob 20%                      #859.22
                                # LOE rax rdx rbp rsi r12 r14 r15 ecx r13d
..B17.9:                        # Preds ..B17.8
        cmpq      $1, %rax                                      #859.22
        je        ..B17.14      # Prob 25%                      #859.22
                                # LOE rax rdx rbp rsi r12 r14 r15 ecx r13d
..B17.10:                       # Preds ..B17.9
        cmpq      $2, %rax                                      #859.22
        jne       ..B17.12      # Prob 67%                      #859.22
                                # LOE rax rdx rbp rsi r12 r14 r15 ecx r13d
..B17.11:                       # Preds ..B17.10
        movq      %rsi, %rax                                    #870.32
        shrq      $2, %rax                                      #870.32
        shll      $2, %ecx                                      #870.47
        orb       %cl, (%rsp,%rax)                              #870.13
        orb       $12, 16(%rsp,%rax)                            #871.13
        jmp       ..B17.15      # Prob 100%                     #871.13
                                # LOE rdx rbp rsi r12 r14 r15 r13d
..B17.12:                       # Preds ..B17.10
        cmpq      $3, %rax                                      #859.22
        jne       ..B17.15      # Prob 50%                      #859.22
                                # LOE rdx rbp rsi r12 r14 r15 ecx r13d
..B17.13:                       # Preds ..B17.12
        movq      %rsi, %rax                                    #874.32
        shrq      $2, %rax                                      #874.32
        orb       %cl, (%rsp,%rax)                              #874.13
        orb       $3, 16(%rsp,%rax)                             #875.13
        jmp       ..B17.15      # Prob 100%                     #875.13
                                # LOE rdx rbp rsi r12 r14 r15 r13d
..B17.14:                       # Preds ..B17.9
        movq      %rsi, %rax                                    #866.32
        shrq      $2, %rax                                      #866.32
        shll      $4, %ecx                                      #866.47
        orb       %cl, (%rsp,%rax)                              #866.13
        orb       $48, 16(%rsp,%rax)                            #867.13
                                # LOE rdx rbp rsi r12 r14 r15 r13d
..B17.15:                       # Preds ..B17.12 ..B17.43 ..B17.14 ..B17.11 ..B17.13
                                #      
        incq      %rsi                                          #856.31
        cmpq      %r12, %rsi                                    #856.22
        jb        ..B17.8       # Prob 82%                      #856.22
                                # LOE rdx rbp rsi r12 r14 r15 r13d
..B17.17:                       # Preds ..B17.15 ..B17.6
        addq      $3, %rsi                                      #881.21
        shrq      $2, %rsi                                      #881.28
        cmpq      $16, %rsi                                     #881.35
        jae       ..B17.30      # Prob 50%                      #881.35
                                # LOE rdx rsi r12 r14 r15 r13d
..B17.18:                       # Preds ..B17.17
        lea       (%rsp), %rax                                  #883.9
        movq      %rsi, %rbp                                    #881.5
        negq      %rbp                                          #881.5
        addq      $16, %rbp                                     #881.5
        lea       (%rax,%rsi), %rbx                             #881.5
        movq      %rbx, %rax                                    #881.5
        andq      $15, %rax                                     #881.5
        movq      %rax, %rcx                                    #881.5
        negq      %rcx                                          #881.5
        addq      $16, %rcx                                     #881.5
        cmpq      $0, %rax                                      #881.5
        cmovne    %rcx, %rax                                    #881.5
        lea       16(%rax), %r8                                 #881.5
        cmpq      %r8, %rbp                                     #881.5
        jl        ..B17.42      # Prob 10%                      #881.5
                                # LOE rax rdx rbx rbp rsi r12 r14 r15 r13d
..B17.19:                       # Preds ..B17.18
        movq      %rbp, %rcx                                    #881.5
        subq      %rax, %rcx                                    #881.5
        andq      $15, %rcx                                     #881.5
        negq      %rcx                                          #881.5
        addq      %rbp, %rcx                                    #881.5
        xorl      %r9d, %r9d                                    #881.5
        testq     %rax, %rax                                    #881.5
        jbe       ..B17.23      # Prob 10%                      #881.5
                                # LOE rax rdx rcx rbx rbp rsi r9 r12 r14 r15 r13d
..B17.20:                       # Preds ..B17.19
        lea       16(%rsp), %r8                                 #884.9
        addq      %rsi, %r8                                     #
                                # LOE rax rdx rcx rbx rbp rsi r8 r9 r12 r14 r15 r13d
..B17.21:                       # Preds ..B17.21 ..B17.20
        xorl      %r10d, %r10d                                  #883.9
        movb      %r10b, (%r9,%rbx)                             #883.9
        movb      %r10b, (%r9,%r8)                              #884.9
        incq      %r9                                           #881.5
        cmpq      %rax, %r9                                     #881.5
        jb        ..B17.21      # Prob 82%                      #881.5
                                # LOE rax rdx rcx rbx rbp rsi r8 r9 r12 r14 r15 r13d
..B17.23:                       # Preds ..B17.21 ..B17.19
        lea       (%rsp), %r9                                   #883.9
        lea       16(%rsp), %r8                                 #884.9
        addq      %rsi, %r9                                     #
        addq      %rsi, %r8                                     #
        pxor      %xmm0, %xmm0                                  #883.29
                                # LOE rax rdx rcx rbx rbp rsi r8 r9 r12 r14 r15 r13d xmm0
..B17.24:                       # Preds ..B17.24 ..B17.23
        movdqa    %xmm0, (%rax,%r9)                             #883.9
        movdqa    %xmm0, (%rax,%r8)                             #884.9
        addq      $16, %rax                                     #881.5
        cmpq      %rcx, %rax                                    #881.5
        jb        ..B17.24      # Prob 82%                      #881.5
                                # LOE rax rdx rcx rbx rbp rsi r8 r9 r12 r14 r15 r13d xmm0
..B17.26:                       # Preds ..B17.24 ..B17.42
        cmpq      %rbp, %rcx                                    #881.5
        jae       ..B17.30      # Prob 10%                      #881.5
                                # LOE rdx rcx rbx rbp rsi r12 r14 r15 r13d
..B17.27:                       # Preds ..B17.26
        lea       16(%rsp), %rax                                #884.9
        addq      %rax, %rsi                                    #
                                # LOE rdx rcx rbx rbp rsi r12 r14 r15 r13d
..B17.28:                       # Preds ..B17.28 ..B17.27
        xorl      %eax, %eax                                    #883.9
        movb      %al, (%rcx,%rbx)                              #883.9
        movb      %al, (%rcx,%rsi)                              #884.9
        incq      %rcx                                          #881.5
        cmpq      %rbp, %rcx                                    #881.5
        jb        ..B17.28      # Prob 82%                      #881.5
                                # LOE rdx rcx rbx rbp rsi r12 r14 r15 r13d
..B17.30:                       # Preds ..B17.28 ..B17.26 ..B17.17
        testl     %r13d, %r13d                                  #890.10
        je        ..B17.32      # Prob 50%                      #890.10
                                # LOE rdx r12 r14 r15
..B17.31:                       # Preds ..B17.30
        movl      $9, (%rdx)                                    #892.9
        jmp       ..B17.41      # Prob 100%                     #892.9
                                # LOE rdx r14 r15
..B17.32:                       # Preds ..B17.30
        cmpq      $2, %r12                                      #896.22
        jae       ..B17.34      # Prob 50%                      #896.22
                                # LOE rdx r12 r14 r15
..B17.33:                       # Preds ..B17.32
        movzbl    (%rsp), %eax                                  #898.29
        movzbl    16(%rsp), %esi                                #903.26
        movl      $2, (%rdx)                                    #908.9
        movb      %al, 1(%rsp)                                  #898.9
        movzwl    (%rsp), %ecx                                  #899.29
        movw      %cx, 2(%rsp)                                  #899.9
        movl      (%rsp), %ebx                                  #900.29
        movl      (%rsp), %ebp                                  #901.29
        movl      %ebx, 4(%rsp)                                 #900.9
        movb      %sil, 17(%rsp)                                #903.9
        movzwl    16(%rsp), %r8d                                #904.26
        movw      %r8w, 18(%rsp)                                #904.9
        movl      16(%rsp), %r9d                                #905.26
        movl      16(%rsp), %r10d                               #906.26
        movl      %r9d, 20(%rsp)                                #905.9
        movl      %ebx, %ebx                                    #900.9
        shlq      $32, %rbx                                     #901.29
        movl      %r9d, %r9d                                    #905.9
        shlq      $32, %r9                                      #906.26
        orq       %rbp, %rbx                                    #901.29
        movq      %rbx, 8(%rsp)                                 #901.9
        orq       %r10, %r9                                     #906.26
        movq      %r9, 24(%rsp)                                 #906.9
        jmp       ..B17.41      # Prob 100%                     #906.9
                                # LOE rdx r14 r15
..B17.34:                       # Preds ..B17.32
        cmpq      $6, %r12                                      #910.22
        jae       ..B17.36      # Prob 50%                      #910.22
                                # LOE rdx r12 r14 r15
..B17.35:                       # Preds ..B17.34
        movzwl    (%rsp), %eax                                  #912.29
        movzwl    16(%rsp), %ebp                                #916.26
        movl      $3, (%rdx)                                    #920.9
        movw      %ax, 2(%rsp)                                  #912.9
        movl      (%rsp), %ecx                                  #913.29
        movl      (%rsp), %ebx                                  #914.29
        movl      %ecx, 4(%rsp)                                 #913.9
        movw      %bp, 18(%rsp)                                 #916.9
        movl      16(%rsp), %esi                                #917.26
        movl      16(%rsp), %r8d                                #918.26
        movl      %esi, 20(%rsp)                                #917.9
        movl      %ecx, %ecx                                    #913.9
        shlq      $32, %rcx                                     #914.29
        movl      %esi, %esi                                    #917.9
        shlq      $32, %rsi                                     #918.26
        orq       %rbx, %rcx                                    #914.29
        movq      %rcx, 8(%rsp)                                 #914.9
        orq       %r8, %rsi                                     #918.26
        movq      %rsi, 24(%rsp)                                #918.9
        jmp       ..B17.41      # Prob 100%                     #918.9
                                # LOE rdx r14 r15
..B17.36:                       # Preds ..B17.34
        cmpq      $14, %r12                                     #922.22
        jae       ..B17.38      # Prob 50%                      #922.22
                                # LOE rdx r12 r14 r15
..B17.37:                       # Preds ..B17.36
        movl      (%rsp), %eax                                  #924.29
        movl      (%rsp), %ecx                                  #925.29
        movl      16(%rsp), %ebx                                #927.26
        movl      16(%rsp), %ebp                                #928.26
        movl      %eax, 4(%rsp)                                 #924.9
        movl      $4, (%rdx)                                    #930.9
        movl      %eax, %eax                                    #924.9
        shlq      $32, %rax                                     #925.29
        movl      %ebx, 20(%rsp)                                #927.9
        movl      %ebx, %ebx                                    #927.9
        shlq      $32, %rbx                                     #928.26
        orq       %rcx, %rax                                    #925.29
        movq      %rax, 8(%rsp)                                 #925.9
        orq       %rbp, %rbx                                    #928.26
        movq      %rbx, 24(%rsp)                                #928.9
        jmp       ..B17.41      # Prob 100%                     #928.9
                                # LOE rdx r14 r15
..B17.38:                       # Preds ..B17.36
        cmpq      $30, %r12                                     #932.22
        jae       ..B17.40      # Prob 50%                      #932.22
                                # LOE rdx r14 r15
..B17.39:                       # Preds ..B17.38
        movq      (%rsp), %rax                                  #934.29
        movq      16(%rsp), %rcx                                #935.26
        movl      $0, (%rdx)                                    #937.9
        movq      %rax, 8(%rsp)                                 #934.9
        movq      %rcx, 24(%rsp)                                #935.9
        jmp       ..B17.41      # Prob 100%                     #935.9
                                # LOE rdx r14 r15
..B17.40:                       # Preds ..B17.38
        movl      $5, (%rdx)                                    #941.9
                                # LOE rdx r14 r15
..B17.41:                       # Preds ..B17.31 ..B17.33 ..B17.35 ..B17.37 ..B17.39
                                #       ..B17.40
        movq      (%rsp), %rcx                                  #944.42
        lea       (%rsp), %rdi                                  #948.5
        movq      %rcx, 16(%rdx)                                #944.42
        movq      8(%rsp), %r8                                  #944.42
        movq      %r8, 24(%rdx)                                 #944.42
        movl      $2, %esi                                      #952.5
        movq      16(%rsp), %r9                                 #945.39
        movq      %r9, 32(%rdx)                                 #945.39
        movq      24(%rsp), %r10                                #945.39
        movq      %r10, 40(%rdx)                                #945.39
        movq      (%rdi), %rax                                  #948.5
        movq      8(%rdi), %rcx                                 #948.5
        bswap     %rax                                          #948.5
        bswap     %rcx                                          #948.5
        movq      %rax, 8(%rdi)                                 #948.5
        movq      %rcx, (%rdi)                                  #948.5
        lea       16(%rsp), %rdi                                #949.5
        movq      (%rdi), %rax                                  #949.5
        movq      8(%rdi), %rcx                                 #949.5
        bswap     %rax                                          #949.5
        bswap     %rcx                                          #949.5
        movq      %rax, 8(%rdi)                                 #949.5
        movq      %rcx, (%rdi)                                  #949.5
        lea       (%rsp), %rdi                                  #952.5
        movl      %esi, %ecx                                    #952.5
        movq      8(%rdi), %rax                                 #952.5
        shrdq     %cl, %rax, (%rdi)                             #952.5
        shrq      %cl, %rax                                     #952.5
        movq      %rax, 8(%rdi)                                 #952.5
        lea       16(%rsp), %rdi                                #953.5
        movl      %esi, %ecx                                    #953.5
        movq      8(%rdi), %rax                                 #953.5
        shrdq     %cl, %rax, (%rdi)                             #953.5
        shrq      %cl, %rax                                     #953.5
        movq      %rax, 8(%rdi)                                 #953.5
        lea       (%rsp), %rsi                                  #956.5
        lea       48(%rdx), %rdi                                #956.5
        movq      (%rsi), %rax                                  #956.5
        movq      8(%rsi), %rcx                                 #956.5
        bswap     %rax                                          #956.5
        bswap     %rcx                                          #956.5
        movq      %rax, 8(%rdi)                                 #956.5
        movq      %rcx, (%rdi)                                  #956.5
        lea       16(%rsp), %rsi                                #957.5
        lea       64(%rdx), %rdi                                #957.5
        movq      (%rsi), %rax                                  #957.5
        movq      8(%rsi), %rcx                                 #957.5
        bswap     %rax                                          #957.5
        bswap     %rcx                                          #957.5
        movq      %rax, 8(%rdi)                                 #957.5
        movq      %rcx, (%rdi)                                  #957.5
        lea       (%rsp), %rdi                                  #959.5
        movl      $2, %esi                                      #959.5
        movl      %esi, %ecx                                    #959.5
        movq      8(%rdi), %rax                                 #959.5
        shrdq     %cl, %rax, (%rdi)                             #959.5
        shrq      %cl, %rax                                     #959.5
        movq      %rax, 8(%rdi)                                 #959.5
        lea       16(%rsp), %rdi                                #960.5
        movl      %esi, %ecx                                    #960.5
        movq      8(%rdi), %rax                                 #960.5
        shrdq     %cl, %rax, (%rdi)                             #960.5
        shrq      %cl, %rax                                     #960.5
        movq      %rax, 8(%rdi)                                 #960.5
        lea       (%rsp), %rsi                                  #962.5
        lea       80(%rdx), %rdi                                #962.5
        movq      (%rsi), %rax                                  #962.5
        movq      8(%rsi), %rcx                                 #962.5
        bswap     %rax                                          #962.5
        bswap     %rcx                                          #962.5
        movq      %rax, 8(%rdi)                                 #962.5
        movq      %rcx, (%rdi)                                  #962.5
        lea       16(%rsp), %rsi                                #963.5
        lea       96(%rdx), %rdi                                #963.5
        movq      (%rsi), %rax                                  #963.5
        movq      8(%rsi), %rcx                                 #963.5
        bswap     %rax                                          #963.5
        bswap     %rcx                                          #963.5
        movq      %rax, 8(%rdi)                                 #963.5
        movq      %rcx, (%rdi)                                  #963.5
        lea       (%rsp), %rdi                                  #965.5
        movl      $2, %esi                                      #965.5
        movl      %esi, %ecx                                    #965.5
        movq      8(%rdi), %rax                                 #965.5
        shrdq     %cl, %rax, (%rdi)                             #965.5
        shrq      %cl, %rax                                     #965.5
        movq      %rax, 8(%rdi)                                 #965.5
        lea       16(%rsp), %rdi                                #966.5
        movl      %esi, %ecx                                    #966.5
        movq      8(%rdi), %rax                                 #966.5
        shrdq     %cl, %rax, (%rdi)                             #966.5
        shrq      %cl, %rax                                     #966.5
        movq      %rax, 8(%rdi)                                 #966.5
        lea       (%rsp), %rsi                                  #968.5
        lea       112(%rdx), %rdi                               #968.5
        addq      $128, %rdx                                    #969.5
        movq      (%rsi), %rax                                  #968.5
        movq      8(%rsi), %rcx                                 #968.5
        bswap     %rax                                          #968.5
        bswap     %rcx                                          #968.5
        movq      %rax, 8(%rdi)                                 #968.5
        movq      %rcx, (%rdi)                                  #968.5
        lea       16(%rsp), %rsi                                #969.5
        movq      %rdx, %rdi                                    #969.5
        movq      (%rsi), %rax                                  #969.5
        movq      8(%rsi), %rcx                                 #969.5
        bswap     %rax                                          #969.5
        bswap     %rcx                                          #969.5
        movq      %rax, 8(%rdi)                                 #969.5
        movq      %rcx, (%rdi)                                  #969.5
        xorl      %eax, %eax                                    #1011.12
        addq      $40, %rsp                                     #1011.12
..___tag_value_NucStrFastaExprMake2.931:                        #
        popq      %rbx                                          #1011.12
..___tag_value_NucStrFastaExprMake2.933:                        #
        popq      %rbp                                          #1011.12
..___tag_value_NucStrFastaExprMake2.935:                        #
        popq      %r12                                          #1011.12
..___tag_value_NucStrFastaExprMake2.937:                        #
        popq      %r13                                          #1011.12
..___tag_value_NucStrFastaExprMake2.939:                        #
        ret                                                     #1011.12
..___tag_value_NucStrFastaExprMake2.940:                        #
                                # LOE
..B17.42:                       # Preds ..B17.18                # Infreq
        xorl      %ecx, %ecx                                    #881.5
        jmp       ..B17.26      # Prob 100%                     #881.5
                                # LOE rdx rcx rbx rbp rsi r12 r14 r15 r13d
..B17.43:                       # Preds ..B17.8                 # Infreq
        movq      %rsi, %rax                                    #862.32
        shrq      $2, %rax                                      #862.32
        shll      $6, %ecx                                      #862.46
        movb      %cl, (%rsp,%rax)                              #862.13
        movb      $192, 16(%rsp,%rax)                           #863.13
        jmp       ..B17.15      # Prob 100%                     #863.13
                                # LOE rdx rbp rsi r12 r14 r15 r13d
..B17.44:                       # Preds ..B17.4 ..B17.5         # Infreq
        call      __errno_location@PLT                          #849.16
                                # LOE rax r14 r15
..B17.48:                       # Preds ..B17.44                # Infreq
        movl      (%rax), %eax                                  #849.16
        addq      $40, %rsp                                     #849.16
..___tag_value_NucStrFastaExprMake2.945:                        #
        popq      %rbx                                          #849.16
..___tag_value_NucStrFastaExprMake2.947:                        #
        popq      %rbp                                          #849.16
..___tag_value_NucStrFastaExprMake2.949:                        #
        popq      %r12                                          #849.16
..___tag_value_NucStrFastaExprMake2.951:                        #
        popq      %r13                                          #849.16
..___tag_value_NucStrFastaExprMake2.953:                        #
        ret                                                     #849.16
        .align    16,0x90
..___tag_value_NucStrFastaExprMake2.954:                        #
                                # LOE
# mark_end;
	.type	NucStrFastaExprMake2,@function
	.size	NucStrFastaExprMake2,.-NucStrFastaExprMake2
	.data
# -- End  NucStrFastaExprMake2
	.text
# -- Begin  NucStrFastaExprMake4
# mark_begin;
       .align    16,0x90
NucStrFastaExprMake4:
# parameter 1: %rdi
# parameter 2: %esi
# parameter 3: %rdx
# parameter 4: %rcx
..B18.1:                        # Preds ..B18.0
..___tag_value_NucStrFastaExprMake4.955:                        #1017.1
        pushq     %r15                                          #1017.1
..___tag_value_NucStrFastaExprMake4.957:                        #
        pushq     %r14                                          #1017.1
..___tag_value_NucStrFastaExprMake4.959:                        #
        pushq     %r13                                          #1017.1
..___tag_value_NucStrFastaExprMake4.961:                        #
        pushq     %rbx                                          #1017.1
..___tag_value_NucStrFastaExprMake4.963:                        #
        subq      $40, %rsp                                     #1017.1
..___tag_value_NucStrFastaExprMake4.965:                        #
        movq      %rcx, %r14                                    #1017.1
        movq      %rdx, %r13                                    #1017.1
        movl      %esi, %r15d                                   #1017.1
        movq      %rdi, %rbx                                    #1017.1
        cmpq      $29, %r14                                     #1023.17
        jbe       ..B18.3       # Prob 43%                      #1023.17
                                # LOE rbx rbp r12 r13 r14 r15d
..B18.2:                        # Preds ..B18.1
        movl      $7, %eax                                      #1024.16
        addq      $40, %rsp                                     #1024.16
..___tag_value_NucStrFastaExprMake4.966:                        #
        popq      %rbx                                          #1024.16
..___tag_value_NucStrFastaExprMake4.968:                        #
        popq      %r13                                          #1024.16
..___tag_value_NucStrFastaExprMake4.970:                        #
        popq      %r14                                          #1024.16
..___tag_value_NucStrFastaExprMake4.972:                        #
        popq      %r15                                          #1024.16
..___tag_value_NucStrFastaExprMake4.974:                        #
        ret                                                     #1024.16
..___tag_value_NucStrFastaExprMake4.975:                        #
                                # LOE
..B18.3:                        # Preds ..B18.1
        movl      $160, %edi                                    #1026.9
        call      malloc@PLT                                    #1026.9
                                # LOE rax rbx rbp r12 r13 r14 r15d
..B18.4:                        # Preds ..B18.3
        testq     %rax, %rax                                    #1026.9
        je        ..B18.42      # Prob 12%                      #1026.9
                                # LOE rax rbx rbp r12 r13 r14 r15d
..B18.5:                        # Preds ..B18.4
        lea       15(%rax), %rdx                                #1026.9
        andq      $-16, %rdx                                    #1026.9
        movq      %rax, 8(%rdx)                                 #1026.9
        je        ..B18.42      # Prob 5%                       #1027.15
                                # LOE rdx rbx rbp r12 r13 r14 r15d
..B18.6:                        # Preds ..B18.5
        movq      %rdx, (%rbx)                                  #1030.5
        movl      %r14d, 4(%rdx)                                #1031.5
        xorl      %r8d, %r8d                                    #1035.11
        testq     %r14, %r14                                    #1035.22
        jbe       ..B18.17      # Prob 16%                      #1035.22
                                # LOE rdx rbp r8 r12 r13 r14 r15d
..B18.8:                        # Preds ..B18.6 ..B18.15
        movsbq    (%r8,%r13), %rcx                              #1037.49
        lea       fasta_4na_map.0(%rip), %rax                   #1037.25
        movsbl    (%rax,%rcx), %ebx                             #1037.25
        movq      %r8, %rax                                     #1038.22
        andq      $3, %rax                                      #1038.22
        je        ..B18.41      # Prob 20%                      #1038.22
                                # LOE rax rdx rbp r8 r12 r13 r14 ebx r15d
..B18.9:                        # Preds ..B18.8
        cmpq      $1, %rax                                      #1038.22
        je        ..B18.14      # Prob 25%                      #1038.22
                                # LOE rax rdx rbp r8 r12 r13 r14 ebx r15d
..B18.10:                       # Preds ..B18.9
        cmpq      $2, %rax                                      #1038.22
        jne       ..B18.12      # Prob 67%                      #1038.22
                                # LOE rax rdx rbp r8 r12 r13 r14 ebx r15d
..B18.11:                       # Preds ..B18.10
        movq      %r8, %rcx                                     #1049.32
        shrq      $2, %rcx                                      #1049.32
        movzwl    16(%rsp,%rcx,2), %eax                         #1050.13
        shll      $12, %ebx                                     #1049.47
        orl       $-4096, %eax                                  #1050.13
        orw       %bx, (%rsp,%rcx,2)                            #1049.13
        movw      %ax, 16(%rsp,%rcx,2)                          #1050.13
        jmp       ..B18.15      # Prob 100%                     #1050.13
                                # LOE rdx rbp r8 r12 r13 r14 r15d
..B18.12:                       # Preds ..B18.10
        cmpq      $3, %rax                                      #1038.22
        jne       ..B18.15      # Prob 50%                      #1038.22
                                # LOE rdx rbp r8 r12 r13 r14 ebx r15d
..B18.13:                       # Preds ..B18.12
        movq      %r8, %rcx                                     #1053.32
        shrq      $2, %rcx                                      #1053.32
        movzwl    16(%rsp,%rcx,2), %eax                         #1054.13
        shll      $8, %ebx                                      #1053.47
        orl       $-61696, %eax                                 #1054.13
        orw       %bx, (%rsp,%rcx,2)                            #1053.13
        movw      %ax, 16(%rsp,%rcx,2)                          #1054.13
        jmp       ..B18.15      # Prob 100%                     #1054.13
                                # LOE rdx rbp r8 r12 r13 r14 r15d
..B18.14:                       # Preds ..B18.9
        movq      %r8, %rcx                                     #1045.32
        shrq      $2, %rcx                                      #1045.32
        movzwl    16(%rsp,%rcx,2), %eax                         #1046.13
        orl       $15, %eax                                     #1046.13
        orw       %bx, (%rsp,%rcx,2)                            #1045.13
        movw      %ax, 16(%rsp,%rcx,2)                          #1046.13
                                # LOE rdx rbp r8 r12 r13 r14 r15d
..B18.15:                       # Preds ..B18.12 ..B18.41 ..B18.14 ..B18.11 ..B18.13
                                #      
        incq      %r8                                           #1035.31
        cmpq      %r14, %r8                                     #1035.22
        jb        ..B18.8       # Prob 82%                      #1035.22
                                # LOE rdx rbp r8 r12 r13 r14 r15d
..B18.17:                       # Preds ..B18.15 ..B18.6
        addq      $3, %r8                                       #1060.21
        shrq      $2, %r8                                       #1060.28
        cmpq      $8, %r8                                       #1060.35
        jae       ..B18.30      # Prob 50%                      #1060.35
                                # LOE rdx rbp r8 r12 r14 r15d
..B18.18:                       # Preds ..B18.17
        lea       (%rsp,%r8,2), %rbx                            #1060.5
        movq      %r8, %rsi                                     #1060.5
        negq      %rsi                                          #1060.5
        addq      $8, %rsi                                      #1060.5
        movq      %rbx, %rax                                    #1060.5
        andq      $15, %rax                                     #1060.5
        movq      %rax, %rcx                                    #1060.5
        negq      %rcx                                          #1060.5
        addq      $16, %rcx                                     #1060.5
        shrq      $1, %rcx                                      #1060.5
        cmpq      $0, %rax                                      #1060.5
        cmovne    %rcx, %rax                                    #1060.5
        lea       8(%rax), %r9                                  #1060.5
        cmpq      %r9, %rsi                                     #1060.5
        jl        ..B18.40      # Prob 10%                      #1060.5
                                # LOE rax rdx rbx rbp rsi r8 r12 r14 r15d
..B18.19:                       # Preds ..B18.18
        movq      %rsi, %rcx                                    #1060.5
        subq      %rax, %rcx                                    #1060.5
        andq      $7, %rcx                                      #1060.5
        negq      %rcx                                          #1060.5
        addq      %rsi, %rcx                                    #1060.5
        xorl      %r10d, %r10d                                  #1060.5
        testq     %rax, %rax                                    #1060.5
        jbe       ..B18.23      # Prob 10%                      #1060.5
                                # LOE rax rdx rcx rbx rbp rsi r8 r10 r12 r14 r15d
..B18.20:                       # Preds ..B18.19
        lea       16(%rsp,%r8,2), %r9                           #
                                # LOE rax rdx rcx rbx rbp rsi r8 r9 r10 r12 r14 r15d
..B18.21:                       # Preds ..B18.21 ..B18.20
        xorl      %r11d, %r11d                                  #1062.9
        movw      %r11w, (%rbx,%r10,2)                          #1062.9
        movw      %r11w, (%r9,%r10,2)                           #1063.9
        incq      %r10                                          #1060.5
        cmpq      %rax, %r10                                    #1060.5
        jb        ..B18.21      # Prob 82%                      #1060.5
                                # LOE rax rdx rcx rbx rbp rsi r8 r9 r10 r12 r14 r15d
..B18.23:                       # Preds ..B18.21 ..B18.19
        lea       16(%rsp,%r8,2), %r10                          #
        lea       (%rsp,%r8,2), %r9                             #
        pxor      %xmm0, %xmm0                                  #1062.29
                                # LOE rax rdx rcx rbx rbp rsi r8 r9 r10 r12 r14 r15d xmm0
..B18.24:                       # Preds ..B18.24 ..B18.23
        movdqa    %xmm0, (%r9,%rax,2)                           #1062.9
        movdqa    %xmm0, (%r10,%rax,2)                          #1063.9
        addq      $8, %rax                                      #1060.5
        cmpq      %rcx, %rax                                    #1060.5
        jb        ..B18.24      # Prob 82%                      #1060.5
                                # LOE rax rdx rcx rbx rbp rsi r8 r9 r10 r12 r14 r15d xmm0
..B18.26:                       # Preds ..B18.24 ..B18.40
        cmpq      %rsi, %rcx                                    #1060.5
        jae       ..B18.30      # Prob 10%                      #1060.5
                                # LOE rdx rcx rbx rbp rsi r8 r12 r14 r15d
..B18.27:                       # Preds ..B18.26
        lea       16(%rsp,%r8,2), %rax                          #
                                # LOE rax rdx rcx rbx rbp rsi r12 r14 r15d
..B18.28:                       # Preds ..B18.28 ..B18.27
        xorl      %r8d, %r8d                                    #1062.9
        movw      %r8w, (%rbx,%rcx,2)                           #1062.9
        movw      %r8w, (%rax,%rcx,2)                           #1063.9
        incq      %rcx                                          #1060.5
        cmpq      %rsi, %rcx                                    #1060.5
        jb        ..B18.28      # Prob 82%                      #1060.5
                                # LOE rax rdx rcx rbx rbp rsi r12 r14 r15d
..B18.30:                       # Preds ..B18.28 ..B18.26 ..B18.17
        testl     %r15d, %r15d                                  #1068.10
        je        ..B18.32      # Prob 50%                      #1068.10
                                # LOE rdx rbp r12 r14
..B18.31:                       # Preds ..B18.30
        movl      $10, (%rdx)                                   #1070.9
        jmp       ..B18.39      # Prob 100%                     #1070.9
                                # LOE rdx rbp r12
..B18.32:                       # Preds ..B18.30
        cmpq      $2, %r14                                      #1072.22
        jae       ..B18.34      # Prob 50%                      #1072.22
                                # LOE rdx rbp r12 r14
..B18.33:                       # Preds ..B18.32
        movzwl    (%rsp), %eax                                  #1074.29
        movzwl    16(%rsp), %esi                                #1078.26
        movl      $6, (%rdx)                                    #1082.9
        movw      %ax, 2(%rsp)                                  #1074.9
        movl      (%rsp), %ecx                                  #1075.29
        movl      (%rsp), %ebx                                  #1076.29
        movl      %ecx, 4(%rsp)                                 #1075.9
        movw      %si, 18(%rsp)                                 #1078.9
        movl      16(%rsp), %r8d                                #1079.26
        movl      16(%rsp), %r9d                                #1080.26
        movl      %r8d, 20(%rsp)                                #1079.9
        movl      %ecx, %ecx                                    #1075.9
        shlq      $32, %rcx                                     #1076.29
        movl      %r8d, %r8d                                    #1079.9
        shlq      $32, %r8                                      #1080.26
        orq       %rbx, %rcx                                    #1076.29
        movq      %rcx, 8(%rsp)                                 #1076.9
        orq       %r9, %r8                                      #1080.26
        movq      %r8, 24(%rsp)                                 #1080.9
        jmp       ..B18.39      # Prob 100%                     #1080.9
                                # LOE rdx rbp r12
..B18.34:                       # Preds ..B18.32
        cmpq      $6, %r14                                      #1084.22
        jae       ..B18.36      # Prob 50%                      #1084.22
                                # LOE rdx rbp r12 r14
..B18.35:                       # Preds ..B18.34
        movl      (%rsp), %eax                                  #1086.29
        movl      (%rsp), %ecx                                  #1087.29
        movl      16(%rsp), %ebx                                #1089.26
        movl      16(%rsp), %esi                                #1090.26
        movl      %eax, 4(%rsp)                                 #1086.9
        movl      $7, (%rdx)                                    #1092.9
        movl      %eax, %eax                                    #1086.9
        shlq      $32, %rax                                     #1087.29
        movl      %ebx, 20(%rsp)                                #1089.9
        movl      %ebx, %ebx                                    #1089.9
        shlq      $32, %rbx                                     #1090.26
        orq       %rcx, %rax                                    #1087.29
        movq      %rax, 8(%rsp)                                 #1087.9
        orq       %rsi, %rbx                                    #1090.26
        movq      %rbx, 24(%rsp)                                #1090.9
        jmp       ..B18.39      # Prob 100%                     #1090.9
                                # LOE rdx rbp r12
..B18.36:                       # Preds ..B18.34
        cmpq      $14, %r14                                     #1094.22
        jae       ..B18.38      # Prob 50%                      #1094.22
                                # LOE rdx rbp r12
..B18.37:                       # Preds ..B18.36
        movq      (%rsp), %rax                                  #1096.29
        movq      16(%rsp), %rcx                                #1097.26
        movl      $1, (%rdx)                                    #1099.9
        movq      %rax, 8(%rsp)                                 #1096.9
        movq      %rcx, 24(%rsp)                                #1097.9
        jmp       ..B18.39      # Prob 100%                     #1097.9
                                # LOE rdx rbp r12
..B18.38:                       # Preds ..B18.36
        movl      $8, (%rdx)                                    #1103.9
                                # LOE rdx rbp r12
..B18.39:                       # Preds ..B18.31 ..B18.33 ..B18.35 ..B18.37 ..B18.38
                                #      
        movq      (%rsp), %rcx                                  #1106.42
        lea       (%rsp), %rdi                                  #1110.5
        movq      %rcx, 16(%rdx)                                #1106.42
        movq      8(%rsp), %r8                                  #1106.42
        movq      %r8, 24(%rdx)                                 #1106.42
        movl      $4, %esi                                      #1114.5
        movq      16(%rsp), %r9                                 #1107.39
        movq      %r9, 32(%rdx)                                 #1107.39
        movq      24(%rsp), %r10                                #1107.39
        movq      %r10, 40(%rdx)                                #1107.39
        movq      (%rdi), %rax                                  #1110.5
        movq      8(%rdi), %rcx                                 #1110.5
        bswap     %rax                                          #1110.5
        bswap     %rcx                                          #1110.5
        movq      %rax, 8(%rdi)                                 #1110.5
        movq      %rcx, (%rdi)                                  #1110.5
        lea       16(%rsp), %rdi                                #1111.5
        movq      (%rdi), %rax                                  #1111.5
        movq      8(%rdi), %rcx                                 #1111.5
        bswap     %rax                                          #1111.5
        bswap     %rcx                                          #1111.5
        movq      %rax, 8(%rdi)                                 #1111.5
        movq      %rcx, (%rdi)                                  #1111.5
        lea       (%rsp), %rdi                                  #1114.5
        movl      %esi, %ecx                                    #1114.5
        movq      8(%rdi), %rax                                 #1114.5
        shrdq     %cl, %rax, (%rdi)                             #1114.5
        shrq      %cl, %rax                                     #1114.5
        movq      %rax, 8(%rdi)                                 #1114.5
        lea       16(%rsp), %rdi                                #1115.5
        movl      %esi, %ecx                                    #1115.5
        movq      8(%rdi), %rax                                 #1115.5
        shrdq     %cl, %rax, (%rdi)                             #1115.5
        shrq      %cl, %rax                                     #1115.5
        movq      %rax, 8(%rdi)                                 #1115.5
        lea       (%rsp), %rsi                                  #1118.5
        lea       48(%rdx), %rdi                                #1118.5
        movq      (%rsi), %rax                                  #1118.5
        movq      8(%rsi), %rcx                                 #1118.5
        bswap     %rax                                          #1118.5
        bswap     %rcx                                          #1118.5
        movq      %rax, 8(%rdi)                                 #1118.5
        movq      %rcx, (%rdi)                                  #1118.5
        lea       16(%rsp), %rsi                                #1119.5
        lea       64(%rdx), %rdi                                #1119.5
        movq      (%rsi), %rax                                  #1119.5
        movq      8(%rsi), %rcx                                 #1119.5
        bswap     %rax                                          #1119.5
        bswap     %rcx                                          #1119.5
        movq      %rax, 8(%rdi)                                 #1119.5
        movq      %rcx, (%rdi)                                  #1119.5
        lea       (%rsp), %rdi                                  #1121.5
        movl      $4, %esi                                      #1121.5
        movl      %esi, %ecx                                    #1121.5
        movq      8(%rdi), %rax                                 #1121.5
        shrdq     %cl, %rax, (%rdi)                             #1121.5
        shrq      %cl, %rax                                     #1121.5
        movq      %rax, 8(%rdi)                                 #1121.5
        lea       16(%rsp), %rdi                                #1122.5
        movl      %esi, %ecx                                    #1122.5
        movq      8(%rdi), %rax                                 #1122.5
        shrdq     %cl, %rax, (%rdi)                             #1122.5
        shrq      %cl, %rax                                     #1122.5
        movq      %rax, 8(%rdi)                                 #1122.5
        lea       (%rsp), %rsi                                  #1124.5
        lea       80(%rdx), %rdi                                #1124.5
        movq      (%rsi), %rax                                  #1124.5
        movq      8(%rsi), %rcx                                 #1124.5
        bswap     %rax                                          #1124.5
        bswap     %rcx                                          #1124.5
        movq      %rax, 8(%rdi)                                 #1124.5
        movq      %rcx, (%rdi)                                  #1124.5
        lea       16(%rsp), %rsi                                #1125.5
        lea       96(%rdx), %rdi                                #1125.5
        movq      (%rsi), %rax                                  #1125.5
        movq      8(%rsi), %rcx                                 #1125.5
        bswap     %rax                                          #1125.5
        bswap     %rcx                                          #1125.5
        movq      %rax, 8(%rdi)                                 #1125.5
        movq      %rcx, (%rdi)                                  #1125.5
        lea       (%rsp), %rdi                                  #1127.5
        movl      $4, %esi                                      #1127.5
        movl      %esi, %ecx                                    #1127.5
        movq      8(%rdi), %rax                                 #1127.5
        shrdq     %cl, %rax, (%rdi)                             #1127.5
        shrq      %cl, %rax                                     #1127.5
        movq      %rax, 8(%rdi)                                 #1127.5
        lea       16(%rsp), %rdi                                #1128.5
        movl      %esi, %ecx                                    #1128.5
        movq      8(%rdi), %rax                                 #1128.5
        shrdq     %cl, %rax, (%rdi)                             #1128.5
        shrq      %cl, %rax                                     #1128.5
        movq      %rax, 8(%rdi)                                 #1128.5
        lea       (%rsp), %rsi                                  #1130.5
        lea       112(%rdx), %rdi                               #1130.5
        addq      $128, %rdx                                    #1131.5
        movq      (%rsi), %rax                                  #1130.5
        movq      8(%rsi), %rcx                                 #1130.5
        bswap     %rax                                          #1130.5
        bswap     %rcx                                          #1130.5
        movq      %rax, 8(%rdi)                                 #1130.5
        movq      %rcx, (%rdi)                                  #1130.5
        lea       16(%rsp), %rsi                                #1131.5
        movq      %rdx, %rdi                                    #1131.5
        movq      (%rsi), %rax                                  #1131.5
        movq      8(%rsi), %rcx                                 #1131.5
        bswap     %rax                                          #1131.5
        bswap     %rcx                                          #1131.5
        movq      %rax, 8(%rdi)                                 #1131.5
        movq      %rcx, (%rdi)                                  #1131.5
        xorl      %eax, %eax                                    #1173.12
        addq      $40, %rsp                                     #1173.12
..___tag_value_NucStrFastaExprMake4.980:                        #
        popq      %rbx                                          #1173.12
..___tag_value_NucStrFastaExprMake4.982:                        #
        popq      %r13                                          #1173.12
..___tag_value_NucStrFastaExprMake4.984:                        #
        popq      %r14                                          #1173.12
..___tag_value_NucStrFastaExprMake4.986:                        #
        popq      %r15                                          #1173.12
..___tag_value_NucStrFastaExprMake4.988:                        #
        ret                                                     #1173.12
..___tag_value_NucStrFastaExprMake4.989:                        #
                                # LOE
..B18.40:                       # Preds ..B18.18                # Infreq
        xorl      %ecx, %ecx                                    #1060.5
        jmp       ..B18.26      # Prob 100%                     #1060.5
                                # LOE rdx rcx rbx rbp rsi r8 r12 r14 r15d
..B18.41:                       # Preds ..B18.8                 # Infreq
        movq      %r8, %rax                                     #1041.32
        shrq      $2, %rax                                      #1041.32
        shll      $4, %ebx                                      #1041.46
        movw      %bx, (%rsp,%rax,2)                            #1041.13
        movl      $240, %ebx                                    #1042.13
        movw      %bx, 16(%rsp,%rax,2)                          #1042.13
        jmp       ..B18.15      # Prob 100%                     #1042.13
                                # LOE rdx rbp r8 r12 r13 r14 r15d
..B18.42:                       # Preds ..B18.4 ..B18.5         # Infreq
        call      __errno_location@PLT                          #1028.16
                                # LOE rax rbp r12
..B18.46:                       # Preds ..B18.42                # Infreq
        movl      (%rax), %eax                                  #1028.16
        addq      $40, %rsp                                     #1028.16
..___tag_value_NucStrFastaExprMake4.994:                        #
        popq      %rbx                                          #1028.16
..___tag_value_NucStrFastaExprMake4.996:                        #
        popq      %r13                                          #1028.16
..___tag_value_NucStrFastaExprMake4.998:                        #
        popq      %r14                                          #1028.16
..___tag_value_NucStrFastaExprMake4.1000:                       #
        popq      %r15                                          #1028.16
..___tag_value_NucStrFastaExprMake4.1002:                       #
        ret                                                     #1028.16
        .align    16,0x90
..___tag_value_NucStrFastaExprMake4.1003:                       #
                                # LOE
# mark_end;
	.type	NucStrFastaExprMake4,@function
	.size	NucStrFastaExprMake4,.-NucStrFastaExprMake4
	.data
# -- End  NucStrFastaExprMake4
	.text
# -- Begin  nss_expr
# mark_begin;
       .align    16,0x90
nss_expr:
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %rdx
# parameter 4: %rcx
# parameter 5: %r8d
..B19.1:                        # Preds ..B19.0
..___tag_value_nss_expr.1004:                                   #1406.1
        pushq     %r15                                          #1406.1
..___tag_value_nss_expr.1006:                                   #
        pushq     %r14                                          #1406.1
..___tag_value_nss_expr.1008:                                   #
        pushq     %r13                                          #1406.1
..___tag_value_nss_expr.1010:                                   #
        pushq     %r12                                          #1406.1
..___tag_value_nss_expr.1012:                                   #
        pushq     %rbp                                          #1406.1
..___tag_value_nss_expr.1014:                                   #
        pushq     %rbx                                          #1406.1
..___tag_value_nss_expr.1016:                                   #
        subq      $24, %rsp                                     #1406.1
..___tag_value_nss_expr.1018:                                   #
        movl      %r8d, (%rsp)                                  #1406.1
        movq      %rcx, %rbx                                    #1406.1
        movq      %rdx, %r14                                    #1406.1
        movq      %rsi, %r15                                    #1406.1
        movq      %rdi, %r12                                    #1406.1
        call      __errno_location@PLT                          #1454.32
                                # LOE rax rbx r12 r13 r14 r15
..B19.95:                       # Preds ..B19.1
        movq      %rax, %rbp                                    #1454.32
        call      __ctype_b_loc@PLT                             #1409.9
                                # LOE rax rbx rbp r12 r13 r14 r15
..B19.96:                       # Preds ..B19.95
        movq      %rbp, 8(%rsp)                                 #1409.9
        movq      %r14, %r13                                    #1409.9
        movq      %rax, %r14                                    #1409.9
        jmp       ..B19.2       # Prob 100%                     #1409.9
                                # LOE rbx r12 r13 r14 r15
..B19.74:                       # Preds ..B19.73
        movl      $11, (%rax)                                   #1458.17
        movl      %ebp, 4(%rax)                                 #1459.17
        movq      (%r13), %rdx                                  #1460.41
        movq      %rdx, 8(%rax)                                 #1460.17
        movq      %rax, (%r13)                                  #1461.17
        lea       16(%rax), %r13                                #1464.42
                                # LOE rbx r12 r13 r14 r15
..B19.2:                        # Preds ..B19.74 ..B19.96
        movq      $0, (%r13)                                    #1407.5
        movq      %r12, %rbp                                    #1407.5
        xorl      %esi, %esi                                    #1409.9
        cmpq      %r15, %r12                                    #1409.9
        jae       ..B19.7       # Prob 2%                       #1409.9
                                # LOE rbx rbp rsi r12 r13 r14 r15
..B19.3:                        # Preds ..B19.2
        movq      (%r14), %rcx                                  #1409.9
        movq      %r15, %rdx                                    #1406.1
        subq      %r12, %rdx                                    #1406.1
                                # LOE rdx rcx rbx rbp rsi r12 r13 r14 r15
..B19.4:                        # Preds ..B19.5 ..B19.3
        movzbl    (%rsi,%r12), %edi                             #1409.9
        movzwl    (%rcx,%rdi,2), %r8d                           #1409.9
        testl     $8192, %r8d                                   #1409.9
        je        ..B19.7       # Prob 20%                      #1409.9
                                # LOE rdx rcx rbx rbp rsi r12 r13 r14 r15
..B19.5:                        # Preds ..B19.4
        lea       1(%rsi,%r12), %rbp                            #1409.9
        incq      %rsi                                          #1409.9
        cmpq      %rdx, %rsi                                    #1409.9
        jb        ..B19.4       # Prob 82%                      #1409.9
                                # LOE rdx rcx rbx rbp rsi r12 r13 r14 r15
..B19.7:                        # Preds ..B19.4 ..B19.5 ..B19.2
        cmpq      %r15, %rbp                                    #1410.15
        je        ..B19.78      # Prob 16%                      #1410.15
                                # LOE rbx rbp r13 r14 r15
..B19.8:                        # Preds ..B19.7
        movsbl    (%rbp), %edx                                  #1412.13
        cmpl      $33, %edx                                     #1412.13
        je        ..B19.44      # Prob 33%                      #1412.13
                                # LOE rbx rbp r13 r14 r15 edx
..B19.9:                        # Preds ..B19.8
        cmpl      $94, %edx                                     #1412.13
        jne       ..B19.21      # Prob 67%                      #1412.13
                                # LOE rbx rbp r13 r14 r15 edx
..B19.10:                       # Preds ..B19.9
        movl      $16, %edi                                     #1412.13
        call      malloc@PLT                                    #1412.13
                                # LOE rax rbx rbp r13 r14 r15
..B19.11:                       # Preds ..B19.10
        testq     %rax, %rax                                    #1412.13
        jne       ..B19.13      # Prob 68%                      #1412.13
                                # LOE rax rbx rbp r13 r14 r15
..B19.12:                       # Preds ..B19.82 ..B19.11
        movq      8(%rsp), %rdx                                 #1412.13
        movl      (%rdx), %edx                                  #1412.13
        movl      %edx, (%rbx)                                  #1412.13
        jmp       ..B19.43      # Prob 100%                     #1412.13
                                # LOE rbx rbp r13 r14 r15 edx
..B19.13:                       # Preds ..B19.11
        movl      $12, (%rax)                                   #1412.13
        incq      %rbp                                          #1412.13
        movq      %rbp, %r8                                     #1412.13
        xorl      %edi, %edi                                    #1412.13
        movl      $14, 4(%rax)                                  #1412.13
        cmpq      %r15, %r8                                     #1412.13
        movq      $0, 8(%rax)                                   #1412.13
        movq      %rax, (%r13)                                  #1412.13
        jae       ..B19.18      # Prob 2%                       #1412.13
                                # LOE rax rbx rbp rdi r8 r13 r14 r15
..B19.14:                       # Preds ..B19.13
        movq      (%r14), %rsi                                  #1412.13
        movq      %r15, %rcx                                    #1406.1
        subq      %rbp, %rcx                                    #1406.1
                                # LOE rax rcx rbx rbp rsi rdi r8 r13 r14 r15
..B19.15:                       # Preds ..B19.16 ..B19.14
        movzbl    (%rdi,%r8), %r9d                              #1412.13
        movzwl    (%rsi,%r9,2), %r10d                           #1412.13
        testl     $8192, %r10d                                  #1412.13
        je        ..B19.18      # Prob 20%                      #1412.13
                                # LOE rax rcx rbx rbp rsi rdi r8 r13 r14 r15
..B19.16:                       # Preds ..B19.15
        lea       1(%rdi,%r8), %rbp                             #1412.13
        incq      %rdi                                          #1412.13
        cmpq      %rcx, %rdi                                    #1412.13
        jb        ..B19.15      # Prob 1%                       #1412.13
                                # LOE rax rcx rbx rbp rsi rdi r8 r13 r14 r15
..B19.18:                       # Preds ..B19.15 ..B19.16 ..B19.13
        movl      (%rsp), %r8d                                  #1412.13
        addq      $8, %rax                                      #1412.13
        movq      %rbp, %rdi                                    #1412.13
        movq      %r15, %rsi                                    #1412.13
        movq      %rax, %rdx                                    #1412.13
        movq      %rbx, %rcx                                    #1412.13
        call      nss_fasta_expr@PLT                            #1412.13
                                # LOE rax rbx r13 r14 r15
..B19.98:                       # Preds ..B19.18
        movq      %rax, %rbp                                    #1412.13
                                # LOE rbx rbp r13 r14 r15
..B19.19:                       # Preds ..B19.98
        movl      (%rbx), %edx                                  #1413.16
        jmp       ..B19.43      # Prob 100%                     #1413.16
                                # LOE rbx rbp r13 r14 r15 edx
..B19.21:                       # Preds ..B19.9
        cmpl      $40, %edx                                     #1412.13
        jne       ..B19.33      # Prob 50%                      #1412.13
                                # LOE rbx rbp r13 r14 r15
..B19.22:                       # Preds ..B19.21
        movl      $16, %edi                                     #1412.13
        call      malloc@PLT                                    #1412.13
                                # LOE rax rbx rbp r13 r14 r15
..B19.99:                       # Preds ..B19.22
        movq      %rax, %r12                                    #1412.13
                                # LOE rbx rbp r12 r13 r14 r15
..B19.23:                       # Preds ..B19.99
        testq     %r12, %r12                                    #1412.13
        jne       ..B19.25      # Prob 68%                      #1412.13
                                # LOE rbx rbp r12 r13 r14 r15
..B19.24:                       # Preds ..B19.23
        movq      8(%rsp), %rdx                                 #1412.13
        movl      (%rdx), %edx                                  #1412.13
        movl      %edx, (%rbx)                                  #1412.13
        jmp       ..B19.43      # Prob 100%                     #1412.13
                                # LOE rbx rbp r13 r14 r15 edx
..B19.25:                       # Preds ..B19.23
        movl      (%rsp), %r8d                                  #1412.13
        movl      $12, (%r12)                                   #1412.13
        incq      %rbp                                          #1412.13
        lea       8(%r12), %rdx                                 #1412.13
        movl      $0, 4(%r12)                                   #1412.13
        movq      %r12, (%r13)                                  #1412.13
        movq      %rbp, %rdi                                    #1412.13
        movq      %r15, %rsi                                    #1412.13
        movq      %rbx, %rcx                                    #1412.13
        call      nss_expr@PLT                                  #1412.13
                                # LOE rax rbx r12 r13 r14 r15
..B19.100:                      # Preds ..B19.25
        movq      %rax, %rbp                                    #1412.13
                                # LOE rbx rbp r12 r13 r14 r15
..B19.26:                       # Preds ..B19.100
        movl      (%rbx), %edx                                  #1412.13
        testl     %edx, %edx                                    #1412.13
        jne       ..B19.43      # Prob 50%                      #1412.13
                                # LOE rbx rbp r12 r13 r14 r15 edx
..B19.27:                       # Preds ..B19.26
        movq      8(%r12), %rcx                                 #1412.13
        testq     %rcx, %rcx                                    #1412.13
        je        ..B19.31      # Prob 12%                      #1412.13
                                # LOE rbx rbp r13 r14 r15 edx
..B19.28:                       # Preds ..B19.27
        cmpq      %r15, %rbp                                    #1412.13
        je        ..B19.31      # Prob 12%                      #1412.13
                                # LOE rbx rbp r13 r14 r15 edx
..B19.29:                       # Preds ..B19.28
        movsbl    (%rbp), %ecx                                  #1412.13
        cmpl      $41, %ecx                                     #1412.13
        je        ..B19.80      # Prob 16%                      #1412.13
                                # LOE rbx rbp r13 r14 r15 edx
..B19.30:                       # Preds ..B19.29
        incq      %rbp                                          #1412.13
                                # LOE rbx rbp r13 r14 r15
..B19.31:                       # Preds ..B19.30 ..B19.28 ..B19.27
        movl      $22, (%rbx)                                   #1412.13
        movl      $22, %edx                                     #1412.13
        jmp       ..B19.43      # Prob 100%                     #1412.13
                                # LOE rbx rbp r13 r14 r15 edx
..B19.33:                       # Preds ..B19.21
        movl      (%rsp), %r8d                                  #1412.13
        movq      %rbp, %rdi                                    #1412.13
        movq      %r15, %rsi                                    #1412.13
        movq      %r13, %rdx                                    #1412.13
        movq      %rbx, %rcx                                    #1412.13
        call      nss_fasta_expr@PLT                            #1412.13
                                # LOE rax rbx r13 r14 r15
..B19.34:                       # Preds ..B19.33
        movl      (%rbx), %edx                                  #1412.13
        movq      %rax, %rbp                                    #1412.13
        testl     %edx, %edx                                    #1412.13
        jne       ..B19.43      # Prob 50%                      #1412.13
                                # LOE rax rbx rbp r13 r14 r15 edx
..B19.35:                       # Preds ..B19.34
        cmpq      %r15, %rax                                    #1412.13
        jae       ..B19.43      # Prob 33%                      #1412.13
                                # LOE rax rbx rbp r13 r14 r15 edx
..B19.36:                       # Preds ..B19.35
        movq      (%r14), %rdi                                  #1412.13
        movq      %rax, %rbp                                    #1412.13
        xorl      %ecx, %ecx                                    #1412.13
        movq      %r15, %rsi                                    #1406.1
        subq      %rax, %rsi                                    #1406.1
                                # LOE rax rcx rbx rbp rsi rdi r13 r14 r15 edx
..B19.37:                       # Preds ..B19.36 ..B19.38
        movzbl    (%rcx,%rax), %r8d                             #1412.13
        movzwl    (%rdi,%r8,2), %r9d                            #1412.13
        testl     $8192, %r9d                                   #1412.13
        je        ..B19.40      # Prob 20%                      #1412.13
                                # LOE rax rcx rbx rbp rsi rdi r13 r14 r15 edx
..B19.38:                       # Preds ..B19.37
        lea       1(%rcx,%rax), %rbp                            #1412.13
        incq      %rcx                                          #1412.13
        cmpq      %rsi, %rcx                                    #1412.13
        jb        ..B19.37      # Prob 1%                       #1412.13
                                # LOE rax rcx rbx rbp rsi rdi r13 r14 r15 edx
..B19.40:                       # Preds ..B19.37 ..B19.38
        cmpq      %r15, %rbp                                    #1412.13
        jae       ..B19.43      # Prob 12%                      #1412.13
                                # LOE rbx rbp r13 r14 r15 edx
..B19.41:                       # Preds ..B19.40
        movsbl    (%rbp), %ecx                                  #1412.13
        cmpl      $36, %ecx                                     #1412.13
        je        ..B19.81      # Prob 5%                       #1412.13
                                # LOE rbx rbp r13 r14 r15 edx
..B19.43:                       # Preds ..B19.34 ..B19.35 ..B19.40 ..B19.41 ..B19.83
                                #       ..B19.26 ..B19.31 ..B19.24 ..B19.19 ..B19.12
                                #       ..B19.80
        movq      %rbp, %r12                                    #1412.13
        jmp       ..B19.56      # Prob 100%                     #1412.13
                                # LOE rbx r12 r13 r14 r15 edx
..B19.44:                       # Preds ..B19.8
        lea       1(%rbp), %r12                                 #1412.13
        xorl      %ecx, %ecx                                    #1412.13
        cmpq      %r15, %r12                                    #1412.13
        jae       ..B19.49      # Prob 2%                       #1412.13
                                # LOE rcx rbx rbp r12 r13 r14 r15
..B19.45:                       # Preds ..B19.44
        movq      (%r14), %rdx                                  #1412.13
        movq      %r15, %rax                                    #1406.1
        subq      %rbp, %rax                                    #1406.1
        decq      %rax                                          #1406.1
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15
..B19.46:                       # Preds ..B19.47 ..B19.45
        movzbl    1(%rcx,%rbp), %esi                            #1412.13
        movzwl    (%rdx,%rsi,2), %edi                           #1412.13
        testl     $8192, %edi                                   #1412.13
        je        ..B19.49      # Prob 20%                      #1412.13
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15
..B19.47:                       # Preds ..B19.46
        lea       2(%rcx,%rbp), %r12                            #1412.13
        incq      %rcx                                          #1412.13
        cmpq      %rax, %rcx                                    #1412.13
        jb        ..B19.46      # Prob 1%                       #1412.13
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15
..B19.49:                       # Preds ..B19.46 ..B19.47 ..B19.44
        cmpq      %r15, %r12                                    #1412.13
        jne       ..B19.51      # Prob 68%                      #1412.13
                                # LOE rbx r12 r13 r14 r15
..B19.50:                       # Preds ..B19.49
        movl      $22, (%rbx)                                   #1412.13
        movl      $22, %edx                                     #1412.13
        jmp       ..B19.56      # Prob 100%                     #1412.13
                                # LOE rbx r12 r13 r14 r15 edx
..B19.51:                       # Preds ..B19.49
        movl      $16, %edi                                     #1412.13
        call      malloc@PLT                                    #1412.13
                                # LOE rax rbx r12 r13 r14 r15
..B19.52:                       # Preds ..B19.51
        testq     %rax, %rax                                    #1412.13
        je        ..B19.88      # Prob 1%                       #1412.13
                                # LOE rax rbx r12 r13 r14 r15
..B19.53:                       # Preds ..B19.52
        movl      (%rsp), %r8d                                  #1412.13
        movl      $12, (%rax)                                   #1412.13
        movq      %r12, %rdi                                    #1412.13
        movq      %r15, %rsi                                    #1412.13
        movl      $13, 4(%rax)                                  #1412.13
        movq      %rbx, %rcx                                    #1412.13
        movq      $0, 8(%rax)                                   #1412.13
        movq      %rax, (%r13)                                  #1412.13
        addq      $8, %rax                                      #1412.13
        movq      %rax, %rdx                                    #1412.13
        call      nss_unary_expr@PLT                            #1412.13
                                # LOE rax rbx r13 r14 r15
..B19.103:                      # Preds ..B19.53
        movq      %rax, %r12                                    #1412.13
                                # LOE rbx r12 r13 r14 r15
..B19.54:                       # Preds ..B19.103
        movl      (%rbx), %edx                                  #1413.16
                                # LOE rbx r12 r13 r14 r15 edx
..B19.56:                       # Preds ..B19.54 ..B19.88 ..B19.50 ..B19.43
        testl     %edx, %edx                                    #1413.26
        jne       ..B19.77      # Prob 58%                      #1413.26
                                # LOE rbx r12 r13 r14 r15
..B19.57:                       # Preds ..B19.56
        movq      %r12, %rbp                                    #1413.9
        xorl      %ecx, %ecx                                    #1415.17
        cmpq      %r15, %r12                                    #1415.17
        jae       ..B19.62      # Prob 2%                       #1415.17
                                # LOE rcx rbx rbp r12 r13 r14 r15
..B19.58:                       # Preds ..B19.57
        movq      (%r14), %rdx                                  #1415.17
        movq      %r15, %rax                                    #1406.1
        subq      %r12, %rax                                    #1406.1
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15
..B19.59:                       # Preds ..B19.60 ..B19.58
        movzbl    (%rcx,%r12), %esi                             #1415.17
        movzwl    (%rdx,%rsi,2), %edi                           #1415.17
        testl     $8192, %edi                                   #1415.17
        je        ..B19.62      # Prob 20%                      #1415.17
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15
..B19.60:                       # Preds ..B19.59
        lea       1(%rcx,%r12), %rbp                            #1415.17
        incq      %rcx                                          #1415.17
        cmpq      %rax, %rcx                                    #1415.17
        jb        ..B19.59      # Prob 17%                      #1415.17
                                # LOE rax rdx rcx rbx rbp r12 r13 r14 r15
..B19.62:                       # Preds ..B19.59 ..B19.60 ..B19.57
        cmpq      %r15, %rbp                                    #1416.23
        je        ..B19.78      # Prob 4%                       #1416.23
                                # LOE rbx rbp r13 r14 r15
..B19.63:                       # Preds ..B19.62
        movsbl    (%rbp), %edx                                  #1422.28
        lea       1(%rbp), %r12                                 #1422.28
        cmpl      $41, %edx                                     #1422.28
        je        ..B19.78      # Prob 25%                      #1422.28
                                # LOE rbx rbp r12 r13 r14 r15 edx
..B19.64:                       # Preds ..B19.63
        cmpl      $38, %edx                                     #1422.28
        jne       ..B19.68      # Prob 67%                      #1422.28
                                # LOE rbx rbp r12 r13 r14 r15 edx
..B19.65:                       # Preds ..B19.64
        cmpq      %r15, %r12                                    #1427.30
        jae       ..B19.67      # Prob 12%                      #1427.30
                                # LOE rbx rbp r12 r13 r14 r15
..B19.66:                       # Preds ..B19.65
        movsbl    (%r12), %edx                                  #1427.39
        addq      $2, %rbp                                      #1428.28
        cmpl      $38, %edx                                     #1428.28
        cmove     %rbp, %r12                                    #1428.28
                                # LOE rbx r12 r13 r14 r15
..B19.67:                       # Preds ..B19.66 ..B19.65
        movl      $16, %ebp                                     #1429.21
        jmp       ..B19.72      # Prob 100%                     #1429.21
                                # LOE rbx r12 r13 r14 r15 ebp
..B19.68:                       # Preds ..B19.64
        cmpl      $124, %edx                                    #1422.28
        jne       ..B19.75      # Prob 50%                      #1422.28
                                # LOE rbx rbp r12 r13 r14 r15
..B19.69:                       # Preds ..B19.68
        cmpq      %r15, %r12                                    #1432.30
        jae       ..B19.71      # Prob 12%                      #1432.30
                                # LOE rbx rbp r12 r13 r14 r15
..B19.70:                       # Preds ..B19.69
        movsbl    (%r12), %edx                                  #1432.39
        addq      $2, %rbp                                      #1433.28
        cmpl      $124, %edx                                    #1433.28
        cmove     %rbp, %r12                                    #1433.28
                                # LOE rbx r12 r13 r14 r15
..B19.71:                       # Preds ..B19.70 ..B19.69
        movl      $17, %ebp                                     #1434.21
                                # LOE rbx r12 r13 r14 r15 ebp
..B19.72:                       # Preds ..B19.67 ..B19.71
        movl      $24, %edi                                     #1451.21
        call      malloc@PLT                                    #1451.21
                                # LOE rax rbx r12 r13 r14 r15 ebp
..B19.73:                       # Preds ..B19.72
        testq     %rax, %rax                                    #1452.27
        je        ..B19.92      # Prob 1%                       #1452.27
        jmp       ..B19.74      # Prob 100%                     #1452.27
                                # LOE rax rbx r12 r13 r14 r15 ebp
..B19.75:                       # Preds ..B19.68
        movl      $22, (%rbx)                                   #1438.21
        movq      %rbp, %rax                                    #1439.32
        addq      $24, %rsp                                     #1439.32
..___tag_value_nss_expr.1019:                                   #
        popq      %rbx                                          #1439.32
..___tag_value_nss_expr.1021:                                   #
        popq      %rbp                                          #1439.32
..___tag_value_nss_expr.1023:                                   #
        popq      %r12                                          #1439.32
..___tag_value_nss_expr.1025:                                   #
        popq      %r13                                          #1439.32
..___tag_value_nss_expr.1027:                                   #
        popq      %r14                                          #1439.32
..___tag_value_nss_expr.1029:                                   #
        popq      %r15                                          #1439.32
..___tag_value_nss_expr.1031:                                   #
        ret                                                     #1439.32
..___tag_value_nss_expr.1032:                                   #
                                # LOE
..B19.77:                       # Preds ..B19.56
        movq      %r12, %rbp                                    #1413.9
                                # LOE rbp r13
..B19.78:                       # Preds ..B19.63 ..B19.7 ..B19.62 ..B19.77
        movq      %rbp, %rax                                    #1470.12
        addq      $24, %rsp                                     #1470.12
..___tag_value_nss_expr.1039:                                   #
        popq      %rbx                                          #1470.12
..___tag_value_nss_expr.1041:                                   #
        popq      %rbp                                          #1470.12
..___tag_value_nss_expr.1043:                                   #
        popq      %r12                                          #1470.12
..___tag_value_nss_expr.1045:                                   #
        popq      %r13                                          #1470.12
..___tag_value_nss_expr.1047:                                   #
        popq      %r14                                          #1470.12
..___tag_value_nss_expr.1049:                                   #
        popq      %r15                                          #1470.12
..___tag_value_nss_expr.1051:                                   #
        ret                                                     #1470.12
..___tag_value_nss_expr.1052:                                   #
                                # LOE
..B19.80:                       # Preds ..B19.29                # Infreq
        incq      %rbp                                          #1412.13
        jmp       ..B19.43      # Prob 100%                     #1412.13
                                # LOE rbx rbp r13 r14 r15 edx
..B19.81:                       # Preds ..B19.41                # Infreq
        incq      %rbp                                          #1412.13
        movl      $16, %edi                                     #1412.13
        call      malloc@PLT                                    #1412.13
                                # LOE rax rbx rbp r13 r14 r15
..B19.82:                       # Preds ..B19.81                # Infreq
        testq     %rax, %rax                                    #1412.13
        je        ..B19.12      # Prob 12%                      #1412.13
                                # LOE rax rbx rbp r13 r14 r15
..B19.83:                       # Preds ..B19.82                # Infreq
        movl      $12, (%rax)                                   #1412.13
        movl      $15, 4(%rax)                                  #1412.13
        movq      (%r13), %rdx                                  #1412.13
        movq      %rdx, 8(%rax)                                 #1412.13
        movl      (%rbx), %edx                                  #1413.16
        movq      %rax, (%r13)                                  #1412.13
        jmp       ..B19.43      # Prob 100%                     #1412.13
                                # LOE rbx rbp r13 r14 r15 edx
..B19.88:                       # Preds ..B19.52                # Infreq
        movq      8(%rsp), %rdx                                 #1412.13
        movl      (%rdx), %edx                                  #1412.13
        movl      %edx, (%rbx)                                  #1412.13
        jmp       ..B19.56      # Prob 100%                     #1412.13
                                # LOE rbx r12 r13 r14 r15 edx
..B19.92:                       # Preds ..B19.73                # Infreq
        movq      8(%rsp), %rbp                                 #
        movl      (%rbp), %edx                                  #1454.32
        movl      %edx, (%rbx)                                  #1454.21
        movq      %r12, %rax                                    #1455.28
        addq      $24, %rsp                                     #1455.28
..___tag_value_nss_expr.1059:                                   #
        popq      %rbx                                          #1455.28
..___tag_value_nss_expr.1061:                                   #
        popq      %rbp                                          #1455.28
..___tag_value_nss_expr.1063:                                   #
        popq      %r12                                          #1455.28
..___tag_value_nss_expr.1065:                                   #
        popq      %r13                                          #1455.28
..___tag_value_nss_expr.1067:                                   #
        popq      %r14                                          #1455.28
..___tag_value_nss_expr.1069:                                   #
        popq      %r15                                          #1455.28
..___tag_value_nss_expr.1071:                                   #
        ret                                                     #1455.28
        .align    16,0x90
..___tag_value_nss_expr.1072:                                   #
                                # LOE
# mark_end;
	.type	nss_expr,@function
	.size	nss_expr,.-nss_expr
	.data
# -- End  nss_expr
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
	.4byte 0x0000045c
	.4byte 0x0000001c
	.4byte ..___tag_value_NucStrstrSearch.1-.
	.4byte ..___tag_value_NucStrstrSearch.194-..___tag_value_NucStrstrSearch.1
	.byte 0x00
	.byte 0x0c
	.8byte 0xffffffffffffe007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.3-..___tag_value_NucStrstrSearch.1
	.byte 0x83
	.byte 0x02
	.byte 0x0c
	.8byte 0xffffffffffffe807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.5-..___tag_value_NucStrstrSearch.3
	.byte 0x0c
	.2byte 0x3007
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.6-..___tag_value_NucStrstrSearch.5
	.byte 0x8c
	.byte 0x03
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.7-..___tag_value_NucStrstrSearch.6
	.byte 0x8d
	.byte 0x04
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.8-..___tag_value_NucStrstrSearch.7
	.byte 0x8e
	.byte 0x05
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.9-..___tag_value_NucStrstrSearch.8
	.byte 0x8f
	.byte 0x06
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
	.byte 0x8c
	.byte 0x03
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.18-..___tag_value_NucStrstrSearch.14
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.20-..___tag_value_NucStrstrSearch.18
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.21-..___tag_value_NucStrstrSearch.20
	.byte 0x0c
	.2byte 0x3007
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.23-..___tag_value_NucStrstrSearch.21
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.24-..___tag_value_NucStrstrSearch.23
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.25-..___tag_value_NucStrstrSearch.24
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.26-..___tag_value_NucStrstrSearch.25
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.27-..___tag_value_NucStrstrSearch.26
	.byte 0x8c
	.byte 0x03
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.31-..___tag_value_NucStrstrSearch.27
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.32-..___tag_value_NucStrstrSearch.31
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.33-..___tag_value_NucStrstrSearch.32
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.34-..___tag_value_NucStrstrSearch.33
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.35-..___tag_value_NucStrstrSearch.34
	.byte 0x8c
	.byte 0x03
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.39-..___tag_value_NucStrstrSearch.35
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.41-..___tag_value_NucStrstrSearch.39
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.42-..___tag_value_NucStrstrSearch.41
	.byte 0x0c
	.2byte 0x3007
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.44-..___tag_value_NucStrstrSearch.42
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.45-..___tag_value_NucStrstrSearch.44
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.46-..___tag_value_NucStrstrSearch.45
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.47-..___tag_value_NucStrstrSearch.46
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.48-..___tag_value_NucStrstrSearch.47
	.byte 0x8c
	.byte 0x03
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.52-..___tag_value_NucStrstrSearch.48
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.54-..___tag_value_NucStrstrSearch.52
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.55-..___tag_value_NucStrstrSearch.54
	.byte 0x0c
	.2byte 0x3007
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.57-..___tag_value_NucStrstrSearch.55
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.58-..___tag_value_NucStrstrSearch.57
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.59-..___tag_value_NucStrstrSearch.58
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.60-..___tag_value_NucStrstrSearch.59
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.61-..___tag_value_NucStrstrSearch.60
	.byte 0x8c
	.byte 0x03
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.65-..___tag_value_NucStrstrSearch.61
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.67-..___tag_value_NucStrstrSearch.65
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.68-..___tag_value_NucStrstrSearch.67
	.byte 0x0c
	.2byte 0x3007
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.70-..___tag_value_NucStrstrSearch.68
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
	.byte 0x8c
	.byte 0x03
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.78-..___tag_value_NucStrstrSearch.74
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.80-..___tag_value_NucStrstrSearch.78
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.81-..___tag_value_NucStrstrSearch.80
	.byte 0x0c
	.2byte 0x3007
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.83-..___tag_value_NucStrstrSearch.81
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.84-..___tag_value_NucStrstrSearch.83
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.85-..___tag_value_NucStrstrSearch.84
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.86-..___tag_value_NucStrstrSearch.85
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.87-..___tag_value_NucStrstrSearch.86
	.byte 0x8c
	.byte 0x03
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.91-..___tag_value_NucStrstrSearch.87
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.93-..___tag_value_NucStrstrSearch.91
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.94-..___tag_value_NucStrstrSearch.93
	.byte 0x0c
	.2byte 0x3007
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.96-..___tag_value_NucStrstrSearch.94
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.97-..___tag_value_NucStrstrSearch.96
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.98-..___tag_value_NucStrstrSearch.97
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.99-..___tag_value_NucStrstrSearch.98
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.100-..___tag_value_NucStrstrSearch.99
	.byte 0x8c
	.byte 0x03
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.104-..___tag_value_NucStrstrSearch.100
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.106-..___tag_value_NucStrstrSearch.104
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.107-..___tag_value_NucStrstrSearch.106
	.byte 0x0c
	.2byte 0x3007
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.109-..___tag_value_NucStrstrSearch.107
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.110-..___tag_value_NucStrstrSearch.109
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.111-..___tag_value_NucStrstrSearch.110
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.112-..___tag_value_NucStrstrSearch.111
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.113-..___tag_value_NucStrstrSearch.112
	.byte 0x8c
	.byte 0x03
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.117-..___tag_value_NucStrstrSearch.113
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.119-..___tag_value_NucStrstrSearch.117
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.120-..___tag_value_NucStrstrSearch.119
	.byte 0x0c
	.2byte 0x3007
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.122-..___tag_value_NucStrstrSearch.120
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.123-..___tag_value_NucStrstrSearch.122
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.124-..___tag_value_NucStrstrSearch.123
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.125-..___tag_value_NucStrstrSearch.124
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.126-..___tag_value_NucStrstrSearch.125
	.byte 0x8c
	.byte 0x03
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.130-..___tag_value_NucStrstrSearch.126
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.132-..___tag_value_NucStrstrSearch.130
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.133-..___tag_value_NucStrstrSearch.132
	.byte 0x0c
	.2byte 0x3007
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.135-..___tag_value_NucStrstrSearch.133
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.136-..___tag_value_NucStrstrSearch.135
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.137-..___tag_value_NucStrstrSearch.136
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.138-..___tag_value_NucStrstrSearch.137
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.139-..___tag_value_NucStrstrSearch.138
	.byte 0x8c
	.byte 0x03
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.143-..___tag_value_NucStrstrSearch.139
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.145-..___tag_value_NucStrstrSearch.143
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.146-..___tag_value_NucStrstrSearch.145
	.byte 0x0c
	.2byte 0x3007
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.148-..___tag_value_NucStrstrSearch.146
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.149-..___tag_value_NucStrstrSearch.148
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.150-..___tag_value_NucStrstrSearch.149
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.151-..___tag_value_NucStrstrSearch.150
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.152-..___tag_value_NucStrstrSearch.151
	.byte 0x8c
	.byte 0x03
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.156-..___tag_value_NucStrstrSearch.152
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.158-..___tag_value_NucStrstrSearch.156
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.159-..___tag_value_NucStrstrSearch.158
	.byte 0x0c
	.2byte 0x3007
	.byte 0x83
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.161-..___tag_value_NucStrstrSearch.159
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.162-..___tag_value_NucStrstrSearch.161
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.163-..___tag_value_NucStrstrSearch.162
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.164-..___tag_value_NucStrstrSearch.163
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.165-..___tag_value_NucStrstrSearch.164
	.byte 0x8c
	.byte 0x03
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.169-..___tag_value_NucStrstrSearch.165
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.171-..___tag_value_NucStrstrSearch.169
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.172-..___tag_value_NucStrstrSearch.171
	.byte 0x0c
	.2byte 0x3007
	.byte 0x83
	.byte 0x02
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.178-..___tag_value_NucStrstrSearch.172
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.180-..___tag_value_NucStrstrSearch.178
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.181-..___tag_value_NucStrstrSearch.180
	.byte 0x0c
	.2byte 0x3007
	.byte 0x83
	.byte 0x02
	.byte 0x8c
	.byte 0x03
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x05
	.byte 0x8f
	.byte 0x06
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.187-..___tag_value_NucStrstrSearch.181
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.188-..___tag_value_NucStrstrSearch.187
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.189-..___tag_value_NucStrstrSearch.188
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.190-..___tag_value_NucStrstrSearch.189
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.191-..___tag_value_NucStrstrSearch.190
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrSearch.193-..___tag_value_NucStrstrSearch.191
	.byte 0x0c
	.2byte 0x0807
	.2byte 0x0000
	.byte 0x00
	.4byte 0x0000018c
	.4byte 0x0000047c
	.4byte ..___tag_value_eval_4na_pos.195-.
	.4byte ..___tag_value_eval_4na_pos.255-..___tag_value_eval_4na_pos.195
	.byte 0x00
	.byte 0x0c
	.8byte 0xffffffffffffe007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.197-..___tag_value_eval_4na_pos.195
	.byte 0x8c
	.byte 0x02
	.byte 0x0c
	.8byte 0xffffffffffffe807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.199-..___tag_value_eval_4na_pos.197
	.byte 0x86
	.byte 0x03
	.byte 0x0c
	.8byte 0xfffffffffffff007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.201-..___tag_value_eval_4na_pos.199
	.byte 0x83
	.byte 0x04
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x0c
	.2byte 0x2007
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.204-..___tag_value_eval_4na_pos.201
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.206-..___tag_value_eval_4na_pos.204
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.208-..___tag_value_eval_4na_pos.206
	.byte 0x0c
	.2byte 0x0007
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.210-..___tag_value_eval_4na_pos.208
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.211-..___tag_value_eval_4na_pos.210
	.byte 0x0c
	.2byte 0x2007
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x03
	.byte 0x8c
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.215-..___tag_value_eval_4na_pos.211
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.217-..___tag_value_eval_4na_pos.215
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.219-..___tag_value_eval_4na_pos.217
	.byte 0x0c
	.2byte 0x0007
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.221-..___tag_value_eval_4na_pos.219
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.222-..___tag_value_eval_4na_pos.221
	.byte 0x0c
	.2byte 0x2007
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x03
	.byte 0x8c
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.226-..___tag_value_eval_4na_pos.222
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.228-..___tag_value_eval_4na_pos.226
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.230-..___tag_value_eval_4na_pos.228
	.byte 0x0c
	.2byte 0x0007
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.232-..___tag_value_eval_4na_pos.230
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.233-..___tag_value_eval_4na_pos.232
	.byte 0x0c
	.2byte 0x2007
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x03
	.byte 0x8c
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.237-..___tag_value_eval_4na_pos.233
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.239-..___tag_value_eval_4na_pos.237
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.241-..___tag_value_eval_4na_pos.239
	.byte 0x0c
	.2byte 0x0007
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.243-..___tag_value_eval_4na_pos.241
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.244-..___tag_value_eval_4na_pos.243
	.byte 0x0c
	.2byte 0x2007
	.byte 0x83
	.byte 0x04
	.byte 0x86
	.byte 0x03
	.byte 0x8c
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.248-..___tag_value_eval_4na_pos.244
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.250-..___tag_value_eval_4na_pos.248
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.252-..___tag_value_eval_4na_pos.250
	.byte 0x0c
	.2byte 0x0007
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_pos.254-..___tag_value_eval_4na_pos.252
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.4byte 0x00000000
	.byte 0x00
	.4byte 0x000001a4
	.4byte 0x0000060c
	.4byte ..___tag_value_eval_2na_pos.256-.
	.4byte ..___tag_value_eval_2na_pos.332-..___tag_value_eval_2na_pos.256
	.byte 0x00
	.byte 0x0c
	.8byte 0xffffffffffffe007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.258-..___tag_value_eval_2na_pos.256
	.byte 0x8f
	.byte 0x02
	.byte 0x0c
	.8byte 0xffffffffffffe807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.260-..___tag_value_eval_2na_pos.258
	.byte 0x8c
	.byte 0x03
	.byte 0x0c
	.8byte 0xfffffffffffff007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.262-..___tag_value_eval_2na_pos.260
	.byte 0x86
	.byte 0x04
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.264-..___tag_value_eval_2na_pos.262
	.byte 0x83
	.byte 0x05
	.byte 0x0c
	.2byte 0x0007
	.byte 0x0c
	.2byte 0x2807
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.267-..___tag_value_eval_2na_pos.264
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.269-..___tag_value_eval_2na_pos.267
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.271-..___tag_value_eval_2na_pos.269
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.273-..___tag_value_eval_2na_pos.271
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.275-..___tag_value_eval_2na_pos.273
	.byte 0x0c
	.2byte 0x0007
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.276-..___tag_value_eval_2na_pos.275
	.byte 0x0c
	.2byte 0x2807
	.byte 0x83
	.byte 0x05
	.byte 0x86
	.byte 0x04
	.byte 0x8c
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.281-..___tag_value_eval_2na_pos.276
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.283-..___tag_value_eval_2na_pos.281
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.285-..___tag_value_eval_2na_pos.283
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.287-..___tag_value_eval_2na_pos.285
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.289-..___tag_value_eval_2na_pos.287
	.byte 0x0c
	.2byte 0x0007
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.290-..___tag_value_eval_2na_pos.289
	.byte 0x0c
	.2byte 0x2807
	.byte 0x83
	.byte 0x05
	.byte 0x86
	.byte 0x04
	.byte 0x8c
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.295-..___tag_value_eval_2na_pos.290
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.297-..___tag_value_eval_2na_pos.295
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.299-..___tag_value_eval_2na_pos.297
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.301-..___tag_value_eval_2na_pos.299
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.303-..___tag_value_eval_2na_pos.301
	.byte 0x0c
	.2byte 0x0007
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.304-..___tag_value_eval_2na_pos.303
	.byte 0x0c
	.2byte 0x2807
	.byte 0x83
	.byte 0x05
	.byte 0x86
	.byte 0x04
	.byte 0x8c
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.309-..___tag_value_eval_2na_pos.304
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.311-..___tag_value_eval_2na_pos.309
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.313-..___tag_value_eval_2na_pos.311
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.315-..___tag_value_eval_2na_pos.313
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.317-..___tag_value_eval_2na_pos.315
	.byte 0x0c
	.2byte 0x0007
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.318-..___tag_value_eval_2na_pos.317
	.byte 0x0c
	.2byte 0x2807
	.byte 0x83
	.byte 0x05
	.byte 0x86
	.byte 0x04
	.byte 0x8c
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.323-..___tag_value_eval_2na_pos.318
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.325-..___tag_value_eval_2na_pos.323
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.327-..___tag_value_eval_2na_pos.325
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.329-..___tag_value_eval_2na_pos.327
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_pos.331-..___tag_value_eval_2na_pos.329
	.byte 0x0c
	.2byte 0x0007
	.byte 0x00
	.4byte 0x000000ec
	.4byte 0x000007b4
	.4byte ..___tag_value_eval_4na_128.333-.
	.4byte ..___tag_value_eval_4na_128.361-..___tag_value_eval_4na_128.333
	.byte 0x00
	.byte 0x0c
	.8byte 0xffffffffffffe007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.335-..___tag_value_eval_4na_128.333
	.byte 0x86
	.byte 0x02
	.byte 0x0c
	.8byte 0xffffffffffffe807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.337-..___tag_value_eval_4na_128.335
	.byte 0x83
	.byte 0x03
	.byte 0x0c
	.8byte 0xfffffffffffff007
	.2byte 0xffff
	.byte 0x01
	.byte 0x0c
	.2byte 0x1807
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.340-..___tag_value_eval_4na_128.337
	.byte 0x0c
	.2byte 0x0007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.342-..___tag_value_eval_4na_128.340
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.344-..___tag_value_eval_4na_128.342
	.byte 0x0c
	.8byte 0xfffffffffffff007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.345-..___tag_value_eval_4na_128.344
	.byte 0x0c
	.2byte 0x1807
	.byte 0x83
	.byte 0x03
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.348-..___tag_value_eval_4na_128.345
	.byte 0x0c
	.2byte 0x0007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.350-..___tag_value_eval_4na_128.348
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.352-..___tag_value_eval_4na_128.350
	.byte 0x0c
	.8byte 0xfffffffffffff007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.353-..___tag_value_eval_4na_128.352
	.byte 0x0c
	.2byte 0x1807
	.byte 0x83
	.byte 0x03
	.byte 0x86
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.356-..___tag_value_eval_4na_128.353
	.byte 0x0c
	.2byte 0x0007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.358-..___tag_value_eval_4na_128.356
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_4na_128.360-..___tag_value_eval_4na_128.358
	.byte 0x0c
	.8byte 0xfffffffffffff007
	.2byte 0xffff
	.byte 0x01
	.2byte 0x0000
	.4byte 0x00000124
	.4byte 0x000008a4
	.4byte ..___tag_value_eval_4na_32.362-.
	.4byte ..___tag_value_eval_4na_32.403-..___tag_value_eval_4na_32.362
	.byte 0x00
	.byte 0x0c
	.8byte 0xffffffffffffd007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.364-..___tag_value_eval_4na_32.362
	.byte 0x8f
	.byte 0x02
	.byte 0x0c
	.8byte 0xffffffffffffd807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.366-..___tag_value_eval_4na_32.364
	.byte 0x8e
	.byte 0x03
	.byte 0x0c
	.8byte 0xffffffffffffe007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.368-..___tag_value_eval_4na_32.366
	.byte 0x8d
	.byte 0x04
	.byte 0x0c
	.8byte 0xffffffffffffe807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.370-..___tag_value_eval_4na_32.368
	.byte 0x8c
	.byte 0x05
	.byte 0x0c
	.8byte 0xfffffffffffff007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.372-..___tag_value_eval_4na_32.370
	.byte 0x83
	.byte 0x06
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x0c
	.2byte 0x3007
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.375-..___tag_value_eval_4na_32.372
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.377-..___tag_value_eval_4na_32.375
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.379-..___tag_value_eval_4na_32.377
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.381-..___tag_value_eval_4na_32.379
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.383-..___tag_value_eval_4na_32.381
	.byte 0x0c
	.2byte 0x0007
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.385-..___tag_value_eval_4na_32.383
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.386-..___tag_value_eval_4na_32.385
	.byte 0x0c
	.2byte 0x3007
	.byte 0x83
	.byte 0x06
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.392-..___tag_value_eval_4na_32.386
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.394-..___tag_value_eval_4na_32.392
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.396-..___tag_value_eval_4na_32.394
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.398-..___tag_value_eval_4na_32.396
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.400-..___tag_value_eval_4na_32.398
	.byte 0x0c
	.2byte 0x0007
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_4na_32.402-..___tag_value_eval_4na_32.400
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x000000cc
	.4byte 0x000009cc
	.4byte ..___tag_value_eval_4na_16.404-.
	.4byte ..___tag_value_eval_4na_16.431-..___tag_value_eval_4na_16.404
	.byte 0x00
	.byte 0x0c
	.8byte 0xffffffffffffe007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.406-..___tag_value_eval_4na_16.404
	.byte 0x8d
	.byte 0x02
	.byte 0x0c
	.8byte 0xffffffffffffe807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.408-..___tag_value_eval_4na_16.406
	.byte 0x8c
	.byte 0x03
	.byte 0x0c
	.8byte 0xfffffffffffff007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.410-..___tag_value_eval_4na_16.408
	.byte 0x83
	.byte 0x04
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x0c
	.2byte 0x2007
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.413-..___tag_value_eval_4na_16.410
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.415-..___tag_value_eval_4na_16.413
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.417-..___tag_value_eval_4na_16.415
	.byte 0x0c
	.2byte 0x0007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.419-..___tag_value_eval_4na_16.417
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.420-..___tag_value_eval_4na_16.419
	.byte 0x0c
	.2byte 0x2007
	.byte 0x83
	.byte 0x04
	.byte 0x8c
	.byte 0x03
	.byte 0x8d
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.424-..___tag_value_eval_4na_16.420
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.426-..___tag_value_eval_4na_16.424
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.428-..___tag_value_eval_4na_16.426
	.byte 0x0c
	.2byte 0x0007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_16.430-..___tag_value_eval_4na_16.428
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.4byte 0x00000000
	.byte 0x00
	.4byte 0x000000d4
	.4byte 0x00000a9c
	.4byte ..___tag_value_eval_2na_128.432-.
	.4byte ..___tag_value_eval_2na_128.467-..___tag_value_eval_2na_128.432
	.byte 0x00
	.byte 0x0c
	.8byte 0xfffffffffffff007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.434-..___tag_value_eval_2na_128.432
	.byte 0x8f
	.byte 0x02
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.436-..___tag_value_eval_2na_128.434
	.byte 0x8c
	.byte 0x03
	.byte 0x0c
	.2byte 0x0007
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.438-..___tag_value_eval_2na_128.436
	.byte 0x83
	.byte 0x04
	.byte 0x0c
	.2byte 0x0807
	.byte 0x0c
	.2byte 0x2007
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.441-..___tag_value_eval_2na_128.438
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.442-..___tag_value_eval_2na_128.441
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.444-..___tag_value_eval_2na_128.442
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.446-..___tag_value_eval_2na_128.444
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.447-..___tag_value_eval_2na_128.446
	.byte 0x0c
	.2byte 0x2007
	.byte 0x83
	.byte 0x04
	.byte 0x8c
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.451-..___tag_value_eval_2na_128.447
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.452-..___tag_value_eval_2na_128.451
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.454-..___tag_value_eval_2na_128.452
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.456-..___tag_value_eval_2na_128.454
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.457-..___tag_value_eval_2na_128.456
	.byte 0x0c
	.2byte 0x2007
	.byte 0x83
	.byte 0x04
	.byte 0x8c
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.461-..___tag_value_eval_2na_128.457
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.462-..___tag_value_eval_2na_128.461
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.464-..___tag_value_eval_2na_128.462
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_128.466-..___tag_value_eval_2na_128.464
	.byte 0x0c
	.2byte 0x0807
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000134
	.4byte 0x00000b74
	.4byte ..___tag_value_eval_2na_32.468-.
	.4byte ..___tag_value_eval_2na_32.516-..___tag_value_eval_2na_32.468
	.byte 0x00
	.byte 0x0c
	.8byte 0xffffffffffffd007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.470-..___tag_value_eval_2na_32.468
	.byte 0x8f
	.byte 0x02
	.byte 0x0c
	.8byte 0xffffffffffffd807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.472-..___tag_value_eval_2na_32.470
	.byte 0x8e
	.byte 0x03
	.byte 0x0c
	.8byte 0xffffffffffffe007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.474-..___tag_value_eval_2na_32.472
	.byte 0x8d
	.byte 0x04
	.byte 0x0c
	.8byte 0xffffffffffffe807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.476-..___tag_value_eval_2na_32.474
	.byte 0x8c
	.byte 0x05
	.byte 0x0c
	.8byte 0xfffffffffffff007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.478-..___tag_value_eval_2na_32.476
	.byte 0x86
	.byte 0x06
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.480-..___tag_value_eval_2na_32.478
	.byte 0x83
	.byte 0x07
	.byte 0x0c
	.2byte 0x0007
	.byte 0x0c
	.2byte 0x3807
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.483-..___tag_value_eval_2na_32.480
	.byte 0x0c
	.2byte 0x3007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.485-..___tag_value_eval_2na_32.483
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.487-..___tag_value_eval_2na_32.485
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.489-..___tag_value_eval_2na_32.487
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.491-..___tag_value_eval_2na_32.489
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.493-..___tag_value_eval_2na_32.491
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.495-..___tag_value_eval_2na_32.493
	.byte 0x0c
	.2byte 0x0007
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.496-..___tag_value_eval_2na_32.495
	.byte 0x0c
	.2byte 0x3807
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x06
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.503-..___tag_value_eval_2na_32.496
	.byte 0x0c
	.2byte 0x3007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.505-..___tag_value_eval_2na_32.503
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.507-..___tag_value_eval_2na_32.505
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.509-..___tag_value_eval_2na_32.507
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.511-..___tag_value_eval_2na_32.509
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.513-..___tag_value_eval_2na_32.511
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_32.515-..___tag_value_eval_2na_32.513
	.byte 0x0c
	.2byte 0x0007
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000134
	.4byte 0x00000cac
	.4byte ..___tag_value_eval_2na_16.517-.
	.4byte ..___tag_value_eval_2na_16.565-..___tag_value_eval_2na_16.517
	.byte 0x00
	.byte 0x0c
	.8byte 0xffffffffffffd007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.519-..___tag_value_eval_2na_16.517
	.byte 0x8f
	.byte 0x02
	.byte 0x0c
	.8byte 0xffffffffffffd807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.521-..___tag_value_eval_2na_16.519
	.byte 0x8e
	.byte 0x03
	.byte 0x0c
	.8byte 0xffffffffffffe007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.523-..___tag_value_eval_2na_16.521
	.byte 0x8d
	.byte 0x04
	.byte 0x0c
	.8byte 0xffffffffffffe807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.525-..___tag_value_eval_2na_16.523
	.byte 0x8c
	.byte 0x05
	.byte 0x0c
	.8byte 0xfffffffffffff007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.527-..___tag_value_eval_2na_16.525
	.byte 0x86
	.byte 0x06
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.529-..___tag_value_eval_2na_16.527
	.byte 0x83
	.byte 0x07
	.byte 0x0c
	.2byte 0x0007
	.byte 0x0c
	.2byte 0x3807
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.532-..___tag_value_eval_2na_16.529
	.byte 0x0c
	.2byte 0x3007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.534-..___tag_value_eval_2na_16.532
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.536-..___tag_value_eval_2na_16.534
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.538-..___tag_value_eval_2na_16.536
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.540-..___tag_value_eval_2na_16.538
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.542-..___tag_value_eval_2na_16.540
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.544-..___tag_value_eval_2na_16.542
	.byte 0x0c
	.2byte 0x0007
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.545-..___tag_value_eval_2na_16.544
	.byte 0x0c
	.2byte 0x3807
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x06
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.552-..___tag_value_eval_2na_16.545
	.byte 0x0c
	.2byte 0x3007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.554-..___tag_value_eval_2na_16.552
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.556-..___tag_value_eval_2na_16.554
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.558-..___tag_value_eval_2na_16.556
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.560-..___tag_value_eval_2na_16.558
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.562-..___tag_value_eval_2na_16.560
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_16.564-..___tag_value_eval_2na_16.562
	.byte 0x0c
	.2byte 0x0007
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x000000a4
	.4byte 0x00000de4
	.4byte ..___tag_value_eval_2na_8.566-.
	.4byte ..___tag_value_eval_2na_8.591-..___tag_value_eval_2na_8.566
	.byte 0x00
	.byte 0x0c
	.8byte 0xfffffffffffff007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.568-..___tag_value_eval_2na_8.566
	.byte 0x8d
	.byte 0x02
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.570-..___tag_value_eval_2na_8.568
	.byte 0x8c
	.byte 0x03
	.byte 0x0c
	.2byte 0x0007
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.572-..___tag_value_eval_2na_8.570
	.byte 0x83
	.byte 0x04
	.byte 0x0c
	.2byte 0x0807
	.byte 0x0c
	.2byte 0x2007
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.575-..___tag_value_eval_2na_8.572
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.576-..___tag_value_eval_2na_8.575
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.578-..___tag_value_eval_2na_8.576
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.580-..___tag_value_eval_2na_8.578
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.581-..___tag_value_eval_2na_8.580
	.byte 0x0c
	.2byte 0x2007
	.byte 0x83
	.byte 0x04
	.byte 0x8c
	.byte 0x03
	.byte 0x8d
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.585-..___tag_value_eval_2na_8.581
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.586-..___tag_value_eval_2na_8.585
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.588-..___tag_value_eval_2na_8.586
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_8.590-..___tag_value_eval_2na_8.588
	.byte 0x0c
	.2byte 0x0807
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000124
	.4byte 0x00000e8c
	.4byte ..___tag_value_eval_4na_64.592-.
	.4byte ..___tag_value_eval_4na_64.633-..___tag_value_eval_4na_64.592
	.byte 0x00
	.byte 0x0c
	.8byte 0xffffffffffffd007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.594-..___tag_value_eval_4na_64.592
	.byte 0x8f
	.byte 0x02
	.byte 0x0c
	.8byte 0xffffffffffffd807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.596-..___tag_value_eval_4na_64.594
	.byte 0x8e
	.byte 0x03
	.byte 0x0c
	.8byte 0xffffffffffffe007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.598-..___tag_value_eval_4na_64.596
	.byte 0x8d
	.byte 0x04
	.byte 0x0c
	.8byte 0xffffffffffffe807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.600-..___tag_value_eval_4na_64.598
	.byte 0x8c
	.byte 0x05
	.byte 0x0c
	.8byte 0xfffffffffffff007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.602-..___tag_value_eval_4na_64.600
	.byte 0x83
	.byte 0x06
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x0c
	.2byte 0x3007
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.605-..___tag_value_eval_4na_64.602
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.607-..___tag_value_eval_4na_64.605
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.609-..___tag_value_eval_4na_64.607
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.611-..___tag_value_eval_4na_64.609
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.613-..___tag_value_eval_4na_64.611
	.byte 0x0c
	.2byte 0x0007
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.615-..___tag_value_eval_4na_64.613
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.616-..___tag_value_eval_4na_64.615
	.byte 0x0c
	.2byte 0x3007
	.byte 0x83
	.byte 0x06
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.622-..___tag_value_eval_4na_64.616
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.624-..___tag_value_eval_4na_64.622
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.626-..___tag_value_eval_4na_64.624
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.628-..___tag_value_eval_4na_64.626
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.630-..___tag_value_eval_4na_64.628
	.byte 0x0c
	.2byte 0x0007
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_4na_64.632-..___tag_value_eval_4na_64.630
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000134
	.4byte 0x00000fb4
	.4byte ..___tag_value_eval_2na_64.634-.
	.4byte ..___tag_value_eval_2na_64.682-..___tag_value_eval_2na_64.634
	.byte 0x00
	.byte 0x0c
	.8byte 0xffffffffffffd007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.636-..___tag_value_eval_2na_64.634
	.byte 0x8f
	.byte 0x02
	.byte 0x0c
	.8byte 0xffffffffffffd807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.638-..___tag_value_eval_2na_64.636
	.byte 0x8e
	.byte 0x03
	.byte 0x0c
	.8byte 0xffffffffffffe007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.640-..___tag_value_eval_2na_64.638
	.byte 0x8d
	.byte 0x04
	.byte 0x0c
	.8byte 0xffffffffffffe807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.642-..___tag_value_eval_2na_64.640
	.byte 0x8c
	.byte 0x05
	.byte 0x0c
	.8byte 0xfffffffffffff007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.644-..___tag_value_eval_2na_64.642
	.byte 0x86
	.byte 0x06
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.646-..___tag_value_eval_2na_64.644
	.byte 0x83
	.byte 0x07
	.byte 0x0c
	.2byte 0x0007
	.byte 0x0c
	.2byte 0x3807
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.649-..___tag_value_eval_2na_64.646
	.byte 0x0c
	.2byte 0x3007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.651-..___tag_value_eval_2na_64.649
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.653-..___tag_value_eval_2na_64.651
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.655-..___tag_value_eval_2na_64.653
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.657-..___tag_value_eval_2na_64.655
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.659-..___tag_value_eval_2na_64.657
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.661-..___tag_value_eval_2na_64.659
	.byte 0x0c
	.2byte 0x0007
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.662-..___tag_value_eval_2na_64.661
	.byte 0x0c
	.2byte 0x3807
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x06
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.669-..___tag_value_eval_2na_64.662
	.byte 0x0c
	.2byte 0x3007
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.671-..___tag_value_eval_2na_64.669
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.673-..___tag_value_eval_2na_64.671
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.675-..___tag_value_eval_2na_64.673
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.677-..___tag_value_eval_2na_64.675
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.679-..___tag_value_eval_2na_64.677
	.byte 0x0c
	.2byte 0x0807
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_eval_2na_64.681-..___tag_value_eval_2na_64.679
	.byte 0x0c
	.2byte 0x0007
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x0000019c
	.4byte 0x000010ec
	.4byte ..___tag_value_NucStrstrMake.683-.
	.4byte ..___tag_value_NucStrstrMake.751-..___tag_value_NucStrstrMake.683
	.byte 0x00
	.byte 0x0c
	.8byte 0xffffffffffffc007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.685-..___tag_value_NucStrstrMake.683
	.byte 0x8f
	.byte 0x02
	.byte 0x0c
	.8byte 0xffffffffffffc807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.687-..___tag_value_NucStrstrMake.685
	.byte 0x8e
	.byte 0x03
	.byte 0x0c
	.8byte 0xffffffffffffd007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.689-..___tag_value_NucStrstrMake.687
	.byte 0x8d
	.byte 0x04
	.byte 0x0c
	.8byte 0xffffffffffffd807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.691-..___tag_value_NucStrstrMake.689
	.byte 0x8c
	.byte 0x05
	.byte 0x0c
	.8byte 0xffffffffffffe007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.693-..___tag_value_NucStrstrMake.691
	.byte 0x86
	.byte 0x06
	.byte 0x0c
	.8byte 0xffffffffffffe807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.695-..___tag_value_NucStrstrMake.693
	.byte 0x83
	.byte 0x07
	.byte 0x0c
	.8byte 0xfffffffffffff007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.697-..___tag_value_NucStrstrMake.695
	.byte 0x0c
	.2byte 0x5007
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.698-..___tag_value_NucStrstrMake.697
	.byte 0x0c
	.2byte 0x3807
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.700-..___tag_value_NucStrstrMake.698
	.byte 0x0c
	.2byte 0x3007
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.702-..___tag_value_NucStrstrMake.700
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.704-..___tag_value_NucStrstrMake.702
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.706-..___tag_value_NucStrstrMake.704
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.708-..___tag_value_NucStrstrMake.706
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.710-..___tag_value_NucStrstrMake.708
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.711-..___tag_value_NucStrstrMake.710
	.byte 0x0c
	.2byte 0x5007
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x06
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.718-..___tag_value_NucStrstrMake.711
	.byte 0x0c
	.2byte 0x3807
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.720-..___tag_value_NucStrstrMake.718
	.byte 0x0c
	.2byte 0x3007
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.722-..___tag_value_NucStrstrMake.720
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.724-..___tag_value_NucStrstrMake.722
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.726-..___tag_value_NucStrstrMake.724
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.728-..___tag_value_NucStrstrMake.726
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.730-..___tag_value_NucStrstrMake.728
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.731-..___tag_value_NucStrstrMake.730
	.byte 0x0c
	.2byte 0x5007
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x06
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.738-..___tag_value_NucStrstrMake.731
	.byte 0x0c
	.2byte 0x3807
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.740-..___tag_value_NucStrstrMake.738
	.byte 0x0c
	.2byte 0x3007
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.742-..___tag_value_NucStrstrMake.740
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.744-..___tag_value_NucStrstrMake.742
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.746-..___tag_value_NucStrstrMake.744
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.748-..___tag_value_NucStrstrMake.746
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrstrMake.750-..___tag_value_NucStrstrMake.748
	.byte 0x0c
	.2byte 0x0807
	.2byte 0x0000
	.byte 0x00
	.4byte 0x00000034
	.4byte 0x0000128c
	.4byte ..___tag_value_NucStrstrWhack.752-.
	.4byte ..___tag_value_NucStrstrWhack.759-..___tag_value_NucStrstrWhack.752
	.byte 0x00
	.byte 0x0c
	.2byte 0x0007
	.byte 0x04
	.4byte ..___tag_value_NucStrstrWhack.754-..___tag_value_NucStrstrWhack.752
	.byte 0x8e
	.byte 0x02
	.byte 0x0c
	.2byte 0x0807
	.byte 0x0c
	.2byte 0x1007
	.byte 0x04
	.4byte ..___tag_value_NucStrstrWhack.757-..___tag_value_NucStrstrWhack.754
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrstrWhack.758-..___tag_value_NucStrstrWhack.757
	.byte 0x0c
	.2byte 0x0807
	.4byte 0x00000000
	.2byte 0x0000
	.byte 0x00
	.4byte 0x0000014c
	.4byte 0x000012c4
	.4byte ..___tag_value_nss_unary_expr.760-.
	.4byte ..___tag_value_nss_unary_expr.815-..___tag_value_nss_unary_expr.760
	.byte 0x00
	.byte 0x0c
	.8byte 0xffffffffffffd007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.762-..___tag_value_nss_unary_expr.760
	.byte 0x8f
	.byte 0x02
	.byte 0x0c
	.8byte 0xffffffffffffd807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.764-..___tag_value_nss_unary_expr.762
	.byte 0x8e
	.byte 0x03
	.byte 0x0c
	.8byte 0xffffffffffffe007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.766-..___tag_value_nss_unary_expr.764
	.byte 0x8d
	.byte 0x04
	.byte 0x0c
	.8byte 0xffffffffffffe807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.768-..___tag_value_nss_unary_expr.766
	.byte 0x8c
	.byte 0x05
	.byte 0x0c
	.8byte 0xfffffffffffff007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.770-..___tag_value_nss_unary_expr.768
	.byte 0x86
	.byte 0x06
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.772-..___tag_value_nss_unary_expr.770
	.byte 0x83
	.byte 0x07
	.byte 0x0c
	.2byte 0x0007
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.774-..___tag_value_nss_unary_expr.772
	.byte 0x0c
	.2byte 0x4007
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.775-..___tag_value_nss_unary_expr.774
	.byte 0x0c
	.2byte 0x3807
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.777-..___tag_value_nss_unary_expr.775
	.byte 0x0c
	.2byte 0x3007
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.779-..___tag_value_nss_unary_expr.777
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.781-..___tag_value_nss_unary_expr.779
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.783-..___tag_value_nss_unary_expr.781
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.785-..___tag_value_nss_unary_expr.783
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.787-..___tag_value_nss_unary_expr.785
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.788-..___tag_value_nss_unary_expr.787
	.byte 0x0c
	.2byte 0x4007
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x06
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.795-..___tag_value_nss_unary_expr.788
	.byte 0x0c
	.2byte 0x3807
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.797-..___tag_value_nss_unary_expr.795
	.byte 0x0c
	.2byte 0x3007
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.799-..___tag_value_nss_unary_expr.797
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.801-..___tag_value_nss_unary_expr.799
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.803-..___tag_value_nss_unary_expr.801
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.805-..___tag_value_nss_unary_expr.803
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.807-..___tag_value_nss_unary_expr.805
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_nss_unary_expr.808-..___tag_value_nss_unary_expr.807
	.byte 0x0c
	.2byte 0x4007
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x06
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.4byte 0x00000000
	.2byte 0x0000
	.4byte 0x000001e4
	.4byte 0x00001414
	.4byte ..___tag_value_nss_fasta_expr.816-.
	.4byte ..___tag_value_nss_fasta_expr.905-..___tag_value_nss_fasta_expr.816
	.byte 0x00
	.byte 0x0c
	.8byte 0xfffffffffffff007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.818-..___tag_value_nss_fasta_expr.816
	.byte 0x8e
	.byte 0x02
	.byte 0x0c
	.8byte 0xfffffffffffff807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.820-..___tag_value_nss_fasta_expr.818
	.byte 0x8c
	.byte 0x03
	.byte 0x0c
	.2byte 0x0007
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.822-..___tag_value_nss_fasta_expr.820
	.byte 0x83
	.byte 0x04
	.byte 0x0c
	.2byte 0x0807
	.byte 0x0c
	.2byte 0x2007
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.825-..___tag_value_nss_fasta_expr.822
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.826-..___tag_value_nss_fasta_expr.825
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.828-..___tag_value_nss_fasta_expr.826
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.830-..___tag_value_nss_fasta_expr.828
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.831-..___tag_value_nss_fasta_expr.830
	.byte 0x0c
	.2byte 0x2007
	.byte 0x83
	.byte 0x04
	.byte 0x8c
	.byte 0x03
	.byte 0x8e
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.835-..___tag_value_nss_fasta_expr.831
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.836-..___tag_value_nss_fasta_expr.835
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.838-..___tag_value_nss_fasta_expr.836
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.840-..___tag_value_nss_fasta_expr.838
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.841-..___tag_value_nss_fasta_expr.840
	.byte 0x0c
	.2byte 0x2007
	.byte 0x83
	.byte 0x04
	.byte 0x8c
	.byte 0x03
	.byte 0x8e
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.845-..___tag_value_nss_fasta_expr.841
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.846-..___tag_value_nss_fasta_expr.845
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.848-..___tag_value_nss_fasta_expr.846
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.850-..___tag_value_nss_fasta_expr.848
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.851-..___tag_value_nss_fasta_expr.850
	.byte 0x0c
	.2byte 0x2007
	.byte 0x83
	.byte 0x04
	.byte 0x8c
	.byte 0x03
	.byte 0x8e
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.855-..___tag_value_nss_fasta_expr.851
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.856-..___tag_value_nss_fasta_expr.855
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.858-..___tag_value_nss_fasta_expr.856
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.860-..___tag_value_nss_fasta_expr.858
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.861-..___tag_value_nss_fasta_expr.860
	.byte 0x0c
	.2byte 0x2007
	.byte 0x83
	.byte 0x04
	.byte 0x8c
	.byte 0x03
	.byte 0x8e
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.865-..___tag_value_nss_fasta_expr.861
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.866-..___tag_value_nss_fasta_expr.865
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.868-..___tag_value_nss_fasta_expr.866
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.870-..___tag_value_nss_fasta_expr.868
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.871-..___tag_value_nss_fasta_expr.870
	.byte 0x0c
	.2byte 0x2007
	.byte 0x83
	.byte 0x04
	.byte 0x8c
	.byte 0x03
	.byte 0x8e
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.875-..___tag_value_nss_fasta_expr.871
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.876-..___tag_value_nss_fasta_expr.875
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.878-..___tag_value_nss_fasta_expr.876
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.880-..___tag_value_nss_fasta_expr.878
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.881-..___tag_value_nss_fasta_expr.880
	.byte 0x0c
	.2byte 0x2007
	.byte 0x83
	.byte 0x04
	.byte 0x8c
	.byte 0x03
	.byte 0x8e
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.885-..___tag_value_nss_fasta_expr.881
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.886-..___tag_value_nss_fasta_expr.885
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.888-..___tag_value_nss_fasta_expr.886
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.890-..___tag_value_nss_fasta_expr.888
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.891-..___tag_value_nss_fasta_expr.890
	.byte 0x0c
	.2byte 0x2007
	.byte 0x83
	.byte 0x04
	.byte 0x8c
	.byte 0x03
	.byte 0x8e
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.895-..___tag_value_nss_fasta_expr.891
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.896-..___tag_value_nss_fasta_expr.895
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.898-..___tag_value_nss_fasta_expr.896
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.900-..___tag_value_nss_fasta_expr.898
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_nss_fasta_expr.901-..___tag_value_nss_fasta_expr.900
	.byte 0x0c
	.2byte 0x2007
	.byte 0x83
	.byte 0x04
	.byte 0x8c
	.byte 0x03
	.byte 0x8e
	.byte 0x02
	.byte 0x00
	.4byte 0x0000012c
	.4byte 0x000015fc
	.4byte ..___tag_value_NucStrFastaExprMake2.906-.
	.4byte ..___tag_value_NucStrFastaExprMake2.954-..___tag_value_NucStrFastaExprMake2.906
	.byte 0x00
	.byte 0x0c
	.8byte 0xffffffffffffc007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.908-..___tag_value_NucStrFastaExprMake2.906
	.byte 0x8d
	.byte 0x02
	.byte 0x0c
	.8byte 0xffffffffffffc807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.910-..___tag_value_NucStrFastaExprMake2.908
	.byte 0x8c
	.byte 0x03
	.byte 0x0c
	.8byte 0xffffffffffffd007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.912-..___tag_value_NucStrFastaExprMake2.910
	.byte 0x86
	.byte 0x04
	.byte 0x0c
	.8byte 0xffffffffffffd807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.914-..___tag_value_NucStrFastaExprMake2.912
	.byte 0x83
	.byte 0x05
	.byte 0x0c
	.8byte 0xffffffffffffe007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.916-..___tag_value_NucStrFastaExprMake2.914
	.byte 0x0c
	.2byte 0x5007
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.917-..___tag_value_NucStrFastaExprMake2.916
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.919-..___tag_value_NucStrFastaExprMake2.917
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.921-..___tag_value_NucStrFastaExprMake2.919
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.923-..___tag_value_NucStrFastaExprMake2.921
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.925-..___tag_value_NucStrFastaExprMake2.923
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.926-..___tag_value_NucStrFastaExprMake2.925
	.byte 0x0c
	.2byte 0x5007
	.byte 0x83
	.byte 0x05
	.byte 0x86
	.byte 0x04
	.byte 0x8c
	.byte 0x03
	.byte 0x8d
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.931-..___tag_value_NucStrFastaExprMake2.926
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.933-..___tag_value_NucStrFastaExprMake2.931
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.935-..___tag_value_NucStrFastaExprMake2.933
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.937-..___tag_value_NucStrFastaExprMake2.935
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.939-..___tag_value_NucStrFastaExprMake2.937
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.940-..___tag_value_NucStrFastaExprMake2.939
	.byte 0x0c
	.2byte 0x5007
	.byte 0x83
	.byte 0x05
	.byte 0x86
	.byte 0x04
	.byte 0x8c
	.byte 0x03
	.byte 0x8d
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.945-..___tag_value_NucStrFastaExprMake2.940
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.947-..___tag_value_NucStrFastaExprMake2.945
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.949-..___tag_value_NucStrFastaExprMake2.947
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.951-..___tag_value_NucStrFastaExprMake2.949
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake2.953-..___tag_value_NucStrFastaExprMake2.951
	.byte 0x0c
	.2byte 0x0807
	.2byte 0x0000
	.byte 0x00
	.4byte 0x0000012c
	.4byte 0x0000172c
	.4byte ..___tag_value_NucStrFastaExprMake4.955-.
	.4byte ..___tag_value_NucStrFastaExprMake4.1003-..___tag_value_NucStrFastaExprMake4.955
	.byte 0x00
	.byte 0x0c
	.8byte 0xffffffffffffc007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.957-..___tag_value_NucStrFastaExprMake4.955
	.byte 0x8f
	.byte 0x02
	.byte 0x0c
	.8byte 0xffffffffffffc807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.959-..___tag_value_NucStrFastaExprMake4.957
	.byte 0x8e
	.byte 0x03
	.byte 0x0c
	.8byte 0xffffffffffffd007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.961-..___tag_value_NucStrFastaExprMake4.959
	.byte 0x8d
	.byte 0x04
	.byte 0x0c
	.8byte 0xffffffffffffd807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.963-..___tag_value_NucStrFastaExprMake4.961
	.byte 0x83
	.byte 0x05
	.byte 0x0c
	.8byte 0xffffffffffffe007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.965-..___tag_value_NucStrFastaExprMake4.963
	.byte 0x0c
	.2byte 0x5007
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.966-..___tag_value_NucStrFastaExprMake4.965
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.968-..___tag_value_NucStrFastaExprMake4.966
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.970-..___tag_value_NucStrFastaExprMake4.968
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.972-..___tag_value_NucStrFastaExprMake4.970
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.974-..___tag_value_NucStrFastaExprMake4.972
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.975-..___tag_value_NucStrFastaExprMake4.974
	.byte 0x0c
	.2byte 0x5007
	.byte 0x83
	.byte 0x05
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.980-..___tag_value_NucStrFastaExprMake4.975
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.982-..___tag_value_NucStrFastaExprMake4.980
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.984-..___tag_value_NucStrFastaExprMake4.982
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.986-..___tag_value_NucStrFastaExprMake4.984
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.988-..___tag_value_NucStrFastaExprMake4.986
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.989-..___tag_value_NucStrFastaExprMake4.988
	.byte 0x0c
	.2byte 0x5007
	.byte 0x83
	.byte 0x05
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.994-..___tag_value_NucStrFastaExprMake4.989
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.996-..___tag_value_NucStrFastaExprMake4.994
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.998-..___tag_value_NucStrFastaExprMake4.996
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.1000-..___tag_value_NucStrFastaExprMake4.998
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_NucStrFastaExprMake4.1002-..___tag_value_NucStrFastaExprMake4.1000
	.byte 0x0c
	.2byte 0x0807
	.2byte 0x0000
	.byte 0x00
	.4byte 0x0000019c
	.4byte 0x0000185c
	.4byte ..___tag_value_nss_expr.1004-.
	.4byte ..___tag_value_nss_expr.1072-..___tag_value_nss_expr.1004
	.byte 0x00
	.byte 0x0c
	.8byte 0xffffffffffffc007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1006-..___tag_value_nss_expr.1004
	.byte 0x8f
	.byte 0x02
	.byte 0x0c
	.8byte 0xffffffffffffc807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1008-..___tag_value_nss_expr.1006
	.byte 0x8e
	.byte 0x03
	.byte 0x0c
	.8byte 0xffffffffffffd007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1010-..___tag_value_nss_expr.1008
	.byte 0x8d
	.byte 0x04
	.byte 0x0c
	.8byte 0xffffffffffffd807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1012-..___tag_value_nss_expr.1010
	.byte 0x8c
	.byte 0x05
	.byte 0x0c
	.8byte 0xffffffffffffe007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1014-..___tag_value_nss_expr.1012
	.byte 0x86
	.byte 0x06
	.byte 0x0c
	.8byte 0xffffffffffffe807
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1016-..___tag_value_nss_expr.1014
	.byte 0x83
	.byte 0x07
	.byte 0x0c
	.8byte 0xfffffffffffff007
	.2byte 0xffff
	.byte 0x01
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1018-..___tag_value_nss_expr.1016
	.byte 0x0c
	.2byte 0x5007
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1019-..___tag_value_nss_expr.1018
	.byte 0x0c
	.2byte 0x3807
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1021-..___tag_value_nss_expr.1019
	.byte 0x0c
	.2byte 0x3007
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1023-..___tag_value_nss_expr.1021
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1025-..___tag_value_nss_expr.1023
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1027-..___tag_value_nss_expr.1025
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1029-..___tag_value_nss_expr.1027
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1031-..___tag_value_nss_expr.1029
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1032-..___tag_value_nss_expr.1031
	.byte 0x0c
	.2byte 0x5007
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x06
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1039-..___tag_value_nss_expr.1032
	.byte 0x0c
	.2byte 0x3807
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1041-..___tag_value_nss_expr.1039
	.byte 0x0c
	.2byte 0x3007
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1043-..___tag_value_nss_expr.1041
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1045-..___tag_value_nss_expr.1043
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1047-..___tag_value_nss_expr.1045
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1049-..___tag_value_nss_expr.1047
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1051-..___tag_value_nss_expr.1049
	.byte 0x0c
	.2byte 0x0807
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1052-..___tag_value_nss_expr.1051
	.byte 0x0c
	.2byte 0x5007
	.byte 0x83
	.byte 0x07
	.byte 0x86
	.byte 0x06
	.byte 0x8c
	.byte 0x05
	.byte 0x8d
	.byte 0x04
	.byte 0x8e
	.byte 0x03
	.byte 0x8f
	.byte 0x02
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1059-..___tag_value_nss_expr.1052
	.byte 0x0c
	.2byte 0x3807
	.byte 0x09
	.2byte 0x0303
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1061-..___tag_value_nss_expr.1059
	.byte 0x0c
	.2byte 0x3007
	.byte 0x09
	.2byte 0x0606
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1063-..___tag_value_nss_expr.1061
	.byte 0x0c
	.2byte 0x2807
	.byte 0x09
	.2byte 0x0c0c
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1065-..___tag_value_nss_expr.1063
	.byte 0x0c
	.2byte 0x2007
	.byte 0x09
	.2byte 0x0d0d
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1067-..___tag_value_nss_expr.1065
	.byte 0x0c
	.2byte 0x1807
	.byte 0x09
	.2byte 0x0e0e
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1069-..___tag_value_nss_expr.1067
	.byte 0x0c
	.2byte 0x1007
	.byte 0x09
	.2byte 0x0f0f
	.byte 0x04
	.4byte ..___tag_value_nss_expr.1071-..___tag_value_nss_expr.1069
	.byte 0x0c
	.2byte 0x0807
	.2byte 0x0000
	.byte 0x00
# End
