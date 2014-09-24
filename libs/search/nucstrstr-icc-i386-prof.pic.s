	.section .text
.LNDBG_TX:
# -- Machine type PW
# mark_description "Intel(R) C++ Compiler for applications running on IA-32, Version 10.1    Build 20081024 %s";
# mark_description "-I/home/yaschenk/devel/internal/asm-trace/sra/search/linux -I/home/yaschenk/devel/internal/asm-trace/sra/sea";
# mark_description "rch/unix -I/home/yaschenk/devel/internal/asm-trace/sra/search -I/home/yaschenk/devel/internal/asm-trace/itf ";
# mark_description "-I/home/yaschenk/devel/internal/asm-trace/inc/icc/i386 -I/home/yaschenk/devel/internal/asm-trace/inc/icc -I/";
# mark_description "home/yaschenk/devel/internal/asm-trace/inc/gcc/i386 -I/home/yaschenk/devel/internal/asm-trace/inc/gcc -I/hom";
# mark_description "e/yaschenk/devel/internal/asm-trace/inc/linux -I/home/yaschenk/devel/internal/asm-trace/inc/unix -I/home/rod";
# mark_description "armer/devel/internal/asm-trace/inc -c -S -o nucstrstr.pic.s -fPIC -O3 -unroll -xW -vec_report5 -DNDEBUG -D_P";
# mark_description "ROFILING -p -DLINUX -DUNIX -D_GNU_SOURCE -D_REENTRANT -D_FILE_OFFSET_BITS=64 -MD";
	.file "nucstrstr.c"
	.text
..TXTST0:
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
..B1.1:                         # Preds ..B1.0
        pushl     %ebp                                          #5217.1
        movl      %esp, %ebp                                    #5217.1
        subl      $16, %esp                                     #5217.1
        movl      %edi, -8(%ebp)                                #5217.1
        movl      %esi, -12(%ebp)                               #5217.1
        movl      %ebx, -16(%ebp)                               #5217.1
        movl      8(%ebp), %edi                                 #5215.5
        movl      16(%ebp), %eax                                #5215.5
        movl      20(%ebp), %esi                                #5215.5
        movl      %eax, -4(%ebp)                                #5215.5
        call      ..L1          # Prob 100%                     #5217.1
..L1:                                                           #
        popl      %ebx                                          #5217.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L1], %ebx       #5217.1
        lea       _gprof_pack0@GOTOFF(%ebx), %edx               #5217.1
        call      mcount@PLT                                    #5217.1
                                # LOE ebx esi edi
..B1.2:                         # Preds ..B1.1
        testl     %edi, %edi                                    #5218.18
        je        ..B1.108      # Prob 6%                       #5218.18
                                # LOE ebx esi edi
..B1.3:                         # Preds ..B1.2
        movl      12(%ebp), %eax                                #5218.37
        testl     %eax, %eax                                    #5218.37
        je        ..B1.108      # Prob 6%                       #5218.37
                                # LOE ebx esi edi
..B1.4:                         # Preds ..B1.3
        movl      -4(%ebp), %edx                                #
                                # LOE edx ebx esi edi
..B1.5:                         # Preds ..B1.15 ..B1.18 ..B1.4
        testl     %esi, %esi                                    #5218.52
        je        ..B1.108      # Prob 28%                      #5218.52
                                # LOE edx ebx esi edi
..B1.8:                         # Preds ..B1.5 ..B1.32
        movl      (%edi), %eax                                  #5223.18
        cmpl      $12, %eax                                     #5223.9
        ja        ..B1.108      # Prob 50%                      #5223.9
                                # LOE eax edx ebx esi edi
..B1.9:                         # Preds ..B1.8
        movl      ..1..TPKT.30_0.0.0@GOTOFF(%ebx,%eax,4), %eax  #5223.9
        jmp       *%eax                                         #5223.9
                                # LOE edx ebx esi edi
..1.30_0.TAG.0c.0.0:
..B1.11:                        # Preds ..B1.9
        movl      4(%edi), %eax                                 #5286.22
        testl     %eax, %eax                                    #5286.22
        je        ..B1.20       # Prob 20%                      #5286.22
                                # LOE eax edx ebx esi edi
..B1.12:                        # Preds ..B1.11
        cmpl      $13, %eax                                     #5286.22
        je        ..B1.20       # Prob 25%                      #5286.22
                                # LOE eax edx ebx esi edi
..B1.13:                        # Preds ..B1.12
        cmpl      $14, %eax                                     #5286.22
        jne       ..B1.16       # Prob 67%                      #5286.22
                                # LOE eax edx ebx esi edi
..B1.14:                        # Preds ..B1.13
        movl      8(%edi), %edi                                 #5297.29
        movl      4(%edi), %eax                                 #5297.29
        cmpl      %esi, %eax                                    #5298.34
        ja        ..B1.108      # Prob 5%                       #5298.34
                                # LOE eax edx ebx edi
..B1.15:                        # Preds ..B1.14
        movl      %eax, %esi                                    #5297.17
        testl     %edi, %edi                                    #5218.18
        jne       ..B1.5        # Prob 94%                      #5218.18
        jmp       ..B1.108      # Prob 100%                     #5218.18
                                # LOE edx ebx esi edi
..B1.16:                        # Preds ..B1.13
        cmpl      $15, %eax                                     #5286.22
        jne       ..B1.108      # Prob 50%                      #5286.22
                                # LOE edx ebx esi edi
..B1.17:                        # Preds ..B1.16
        movl      8(%edi), %edi                                 #5302.29
        movl      4(%edi), %eax                                 #5302.29
        cmpl      %esi, %eax                                    #5303.34
        ja        ..B1.108      # Prob 5%                       #5303.34
                                # LOE eax edx ebx esi edi
..B1.18:                        # Preds ..B1.17
        addl      %esi, %edx                                    #5306.27
        subl      %eax, %edx                                    #5306.33
        movl      %eax, %esi                                    #5302.17
        testl     %edi, %edi                                    #5218.18
        jne       ..B1.5        # Prob 94%                      #5218.18
        jmp       ..B1.108      # Prob 100%                     #5218.18
                                # LOE edx ebx esi edi
..B1.20:                        # Preds ..B1.11 ..B1.12
        movl      %edx, -4(%ebp)                                #
                                # LOE edx ebx esi edi dl dh
..B1.21:                        # Preds ..B1.20
        movl      24(%ebp), %eax                                #5217.1
        movl      12(%ebp), %ecx                                #5217.1
        pushl     %eax                                          #5217.1
        pushl     %esi                                          #5217.1
        pushl     %edx                                          #5217.1
        pushl     %ecx                                          #5217.1
        pushl     8(%edi)                                       #5217.1
        call      NucStrstrSearch@PLT                           #5290.25
                                # LOE eax edi
..B1.118:                       # Preds ..B1.21
        addl      $20, %esp                                     #5290.25
                                # LOE eax edi
..B1.22:                        # Preds ..B1.118
        movl      4(%edi), %edx                                 #5291.22
        testl     %edx, %edx                                    #5291.42
        je        ..B1.57       # Prob 28%                      #5291.42
                                # LOE eax
..B1.23:                        # Preds ..B1.22
        testl     %eax, %eax                                    #5293.31
        jne       ..B1.108      # Prob 72%                      #5293.31
                                # LOE
..B1.24:                        # Preds ..B1.23
        movl      -16(%ebp), %ebx                               #5294.28
        movl      -12(%ebp), %esi                               #5294.28
        movl      -8(%ebp), %edi                                #5294.28
        movl      $1, %eax                                      #5294.28
        movl      %ebp, %esp                                    #5294.28
        popl      %ebp                                          #5294.28
        ret                                                     #5294.28
                                # LOE
..1.30_0.TAG.0b.0.0:
..B1.27:                        # Preds ..B1.9
        movl      24(%ebp), %eax                                #5217.1
        movl      12(%ebp), %ecx                                #5217.1
        movl      %edx, -4(%ebp)                                #5272.21
        pushl     %eax                                          #5217.1
        pushl     %esi                                          #5217.1
        pushl     %edx                                          #5217.1
        pushl     %ecx                                          #5217.1
        pushl     8(%edi)                                       #5217.1
        call      NucStrstrSearch@PLT                           #5272.21
                                # LOE eax ebx esi edi
..B1.119:                       # Preds ..B1.27
        movl      -4(%ebp), %edx                                #
        addl      $20, %esp                                     #5272.21
                                # LOE eax edx ebx esi edi dl dh
..B1.28:                        # Preds ..B1.119
        movl      4(%edi), %ecx                                 #5273.22
        cmpl      $17, %ecx                                     #5273.22
        jne       ..B1.30       # Prob 67%                      #5273.22
                                # LOE eax edx ecx ebx esi edi dl dh
..B1.29:                        # Preds ..B1.28
        testl     %eax, %eax                                    #5276.31
        jne       ..B1.57       # Prob 5%                       #5276.31
        jmp       ..B1.32       # Prob 100%                     #5276.31
                                # LOE eax edx ebx esi edi dl dh
..B1.30:                        # Preds ..B1.28
        cmpl      $16, %ecx                                     #5273.22
        jne       ..B1.32       # Prob 50%                      #5273.22
                                # LOE eax edx ebx esi edi dl dh
..B1.31:                        # Preds ..B1.30
        testl     %eax, %eax                                    #5280.31
        je        ..B1.57       # Prob 5%                       #5280.31
                                # LOE eax edx ebx esi edi dl dh
..B1.32:                        # Preds ..B1.29 ..B1.31 ..B1.30
        movl      12(%edi), %edi                                #5284.38
        testl     %edi, %edi                                    #5218.18
        jne       ..B1.8        # Prob 94%                      #5218.18
        jmp       ..B1.108      # Prob 100%                     #5218.18
                                # LOE edx ebx esi edi
..1.30_0.TAG.0a.0.0:
..B1.34:                        # Preds ..B1.9
        movl      %edx, -4(%ebp)                                #
                                # LOE esi edi
..B1.35:                        # Preds ..B1.34
        movl      4(%edi), %edx                                 #5268.24
        cmpl      %edx, %esi                                    #5268.24
        jb        ..B1.108      # Prob 28%                      #5268.24
                                # LOE edx esi edi
..B1.36:                        # Preds ..B1.35
        movl      24(%ebp), %eax                                #5269.9
        testl     %eax, %eax                                    #5269.9
        je        ..B1.38       # Prob 12%                      #5269.9
                                # LOE edx esi edi
..B1.37:                        # Preds ..B1.36
        movl      24(%ebp), %eax                                #5269.18
        movl      %edx, (%eax)                                  #5269.18
                                # LOE esi edi
..B1.38:                        # Preds ..B1.37 ..B1.36
        movl      -4(%ebp), %eax                                #5218.5
        movl      12(%ebp), %edx                                #5218.5
        pushl     %esi                                          #5218.5
        pushl     %eax                                          #5218.5
        pushl     %edx                                          #5218.5
        pushl     %edi                                          #5218.5
        call      eval_4na_pos                                  #5270.20
                                # LOE eax
..B1.120:                       # Preds ..B1.38
        movl      -16(%ebp), %ebx                               #5270.20
        movl      -12(%ebp), %esi                               #5270.20
        movl      -8(%ebp), %edi                                #5270.20
        addl      $16, %esp                                     #5270.20
        movl      %ebp, %esp                                    #5270.20
        popl      %ebp                                          #5270.20
        ret                                                     #5270.20
                                # LOE
..1.30_0.TAG.09.0.0:
..B1.40:                        # Preds ..B1.9
        movl      %edx, -4(%ebp)                                #
                                # LOE esi edi
..B1.41:                        # Preds ..B1.40
        movl      4(%edi), %edx                                 #5264.24
        cmpl      %edx, %esi                                    #5264.24
        jb        ..B1.108      # Prob 28%                      #5264.24
                                # LOE edx esi edi
..B1.42:                        # Preds ..B1.41
        movl      24(%ebp), %eax                                #5265.9
        testl     %eax, %eax                                    #5265.9
        je        ..B1.44       # Prob 12%                      #5265.9
                                # LOE edx esi edi
..B1.43:                        # Preds ..B1.42
        movl      24(%ebp), %eax                                #5265.18
        movl      %edx, (%eax)                                  #5265.18
                                # LOE esi edi
..B1.44:                        # Preds ..B1.43 ..B1.42
        movl      -4(%ebp), %eax                                #5218.5
        movl      12(%ebp), %edx                                #5218.5
        pushl     %esi                                          #5218.5
        pushl     %eax                                          #5218.5
        pushl     %edx                                          #5218.5
        pushl     %edi                                          #5218.5
        call      eval_2na_pos                                  #5266.20
                                # LOE eax
..B1.121:                       # Preds ..B1.44
        movl      -16(%ebp), %ebx                               #5266.20
        movl      -12(%ebp), %esi                               #5266.20
        movl      -8(%ebp), %edi                                #5266.20
        addl      $16, %esp                                     #5266.20
        movl      %ebp, %esp                                    #5266.20
        popl      %ebp                                          #5266.20
        ret                                                     #5266.20
                                # LOE
..1.30_0.TAG.08.0.0:
..B1.46:                        # Preds ..B1.9
        movl      %edx, -4(%ebp)                                #
                                # LOE esi edi
..B1.47:                        # Preds ..B1.46
        movl      4(%edi), %edx                                 #5259.24
        cmpl      %edx, %esi                                    #5259.24
        jb        ..B1.108      # Prob 28%                      #5259.24
                                # LOE edx esi edi
..B1.48:                        # Preds ..B1.47
        movl      24(%ebp), %eax                                #5260.9
        testl     %eax, %eax                                    #5260.9
        je        ..B1.50       # Prob 12%                      #5260.9
                                # LOE edx esi edi
..B1.49:                        # Preds ..B1.48
        movl      24(%ebp), %eax                                #5260.18
        movl      %edx, (%eax)                                  #5260.18
                                # LOE esi edi
..B1.50:                        # Preds ..B1.49 ..B1.48
        movl      -4(%ebp), %eax                                #5218.5
        movl      12(%ebp), %edx                                #5218.5
        pushl     %esi                                          #5218.5
        pushl     %eax                                          #5218.5
        pushl     %edx                                          #5218.5
        pushl     %edi                                          #5218.5
        call      eval_4na_128                                  #5261.20
                                # LOE eax
..B1.122:                       # Preds ..B1.50
        movl      -16(%ebp), %ebx                               #5261.20
        movl      -12(%ebp), %esi                               #5261.20
        movl      -8(%ebp), %edi                                #5261.20
        addl      $16, %esp                                     #5261.20
        movl      %ebp, %esp                                    #5261.20
        popl      %ebp                                          #5261.20
        ret                                                     #5261.20
                                # LOE
..1.30_0.TAG.07.0.0:
..B1.52:                        # Preds ..B1.9
        movl      %edx, -4(%ebp)                                #
                                # LOE esi edi
..B1.53:                        # Preds ..B1.52
        movl      4(%edi), %edx                                 #5255.24
        cmpl      %edx, %esi                                    #5255.24
        jb        ..B1.108      # Prob 28%                      #5255.24
                                # LOE edx esi edi
..B1.54:                        # Preds ..B1.53
        movl      24(%ebp), %eax                                #5256.9
        testl     %eax, %eax                                    #5256.9
        je        ..B1.56       # Prob 12%                      #5256.9
                                # LOE edx esi edi
..B1.55:                        # Preds ..B1.54
        movl      24(%ebp), %eax                                #5256.18
        movl      %edx, (%eax)                                  #5256.18
                                # LOE esi edi
..B1.56:                        # Preds ..B1.55 ..B1.54
        movl      -4(%ebp), %eax                                #5218.5
        movl      12(%ebp), %edx                                #5218.5
        pushl     %esi                                          #5218.5
        pushl     %eax                                          #5218.5
        pushl     %edx                                          #5218.5
        pushl     %edi                                          #5218.5
        call      eval_4na_32                                   #5257.20
                                # LOE eax
..B1.123:                       # Preds ..B1.56
        addl      $16, %esp                                     #5257.20
                                # LOE eax
..B1.57:                        # Preds ..B1.29 ..B1.31 ..B1.22 ..B1.123
        movl      -16(%ebp), %ebx                               #5257.20
        movl      -12(%ebp), %esi                               #5257.20
        movl      -8(%ebp), %edi                                #5257.20
        movl      %ebp, %esp                                    #5257.20
        popl      %ebp                                          #5257.20
        ret                                                     #5257.20
                                # LOE
..1.30_0.TAG.06.0.0:
..B1.59:                        # Preds ..B1.9
        movl      %edx, -4(%ebp)                                #
                                # LOE esi edi
..B1.60:                        # Preds ..B1.59
        movl      4(%edi), %edx                                 #5251.24
        cmpl      %edx, %esi                                    #5251.24
        jb        ..B1.108      # Prob 28%                      #5251.24
                                # LOE edx esi edi
..B1.61:                        # Preds ..B1.60
        movl      24(%ebp), %eax                                #5252.9
        testl     %eax, %eax                                    #5252.9
        je        ..B1.63       # Prob 12%                      #5252.9
                                # LOE edx esi edi
..B1.62:                        # Preds ..B1.61
        movl      24(%ebp), %eax                                #5252.18
        movl      %edx, (%eax)                                  #5252.18
                                # LOE esi edi
..B1.63:                        # Preds ..B1.62 ..B1.61
        movl      -4(%ebp), %eax                                #5218.5
        movl      12(%ebp), %edx                                #5218.5
        pushl     %esi                                          #5218.5
        pushl     %eax                                          #5218.5
        pushl     %edx                                          #5218.5
        pushl     %edi                                          #5218.5
        call      eval_4na_16                                   #5253.20
                                # LOE eax
..B1.124:                       # Preds ..B1.63
        addl      $16, %esp                                     #5253.20
                                # LOE eax
..B1.64:                        # Preds ..B1.124
        movl      -16(%ebp), %ebx                               #5253.20
        movl      -12(%ebp), %esi                               #5253.20
        movl      -8(%ebp), %edi                                #5253.20
        movl      %ebp, %esp                                    #5253.20
        popl      %ebp                                          #5253.20
        ret                                                     #5253.20
                                # LOE
..1.30_0.TAG.05.0.0:
..B1.66:                        # Preds ..B1.9
        movl      %edx, -4(%ebp)                                #
                                # LOE esi edi
..B1.67:                        # Preds ..B1.66
        movl      4(%edi), %edx                                 #5247.24
        cmpl      %edx, %esi                                    #5247.24
        jb        ..B1.108      # Prob 28%                      #5247.24
                                # LOE edx esi edi
..B1.68:                        # Preds ..B1.67
        movl      24(%ebp), %eax                                #5248.9
        testl     %eax, %eax                                    #5248.9
        je        ..B1.70       # Prob 12%                      #5248.9
                                # LOE edx esi edi
..B1.69:                        # Preds ..B1.68
        movl      24(%ebp), %eax                                #5248.18
        movl      %edx, (%eax)                                  #5248.18
                                # LOE esi edi
..B1.70:                        # Preds ..B1.69 ..B1.68
        movl      -4(%ebp), %eax                                #5218.5
        movl      12(%ebp), %edx                                #5218.5
        pushl     %esi                                          #5218.5
        pushl     %eax                                          #5218.5
        pushl     %edx                                          #5218.5
        pushl     %edi                                          #5218.5
        call      eval_2na_128                                  #5249.20
                                # LOE eax
..B1.125:                       # Preds ..B1.70
        movl      -16(%ebp), %ebx                               #5249.20
        movl      -12(%ebp), %esi                               #5249.20
        movl      -8(%ebp), %edi                                #5249.20
        addl      $16, %esp                                     #5249.20
        movl      %ebp, %esp                                    #5249.20
        popl      %ebp                                          #5249.20
        ret                                                     #5249.20
                                # LOE
..1.30_0.TAG.04.0.0:
..B1.72:                        # Preds ..B1.9
        movl      %edx, -4(%ebp)                                #
                                # LOE esi edi
..B1.73:                        # Preds ..B1.72
        movl      4(%edi), %edx                                 #5243.24
        cmpl      %edx, %esi                                    #5243.24
        jb        ..B1.108      # Prob 28%                      #5243.24
                                # LOE edx esi edi
..B1.74:                        # Preds ..B1.73
        movl      24(%ebp), %eax                                #5244.9
        testl     %eax, %eax                                    #5244.9
        je        ..B1.76       # Prob 12%                      #5244.9
                                # LOE edx esi edi
..B1.75:                        # Preds ..B1.74
        movl      24(%ebp), %eax                                #5244.18
        movl      %edx, (%eax)                                  #5244.18
                                # LOE esi edi
..B1.76:                        # Preds ..B1.75 ..B1.74
        movl      -4(%ebp), %eax                                #5218.5
        movl      12(%ebp), %edx                                #5218.5
        pushl     %esi                                          #5218.5
        pushl     %eax                                          #5218.5
        pushl     %edx                                          #5218.5
        pushl     %edi                                          #5218.5
        call      eval_2na_32                                   #5245.20
                                # LOE eax
..B1.126:                       # Preds ..B1.76
        addl      $16, %esp                                     #5245.20
                                # LOE eax
..B1.77:                        # Preds ..B1.126
        movl      -16(%ebp), %ebx                               #5245.20
        movl      -12(%ebp), %esi                               #5245.20
        movl      -8(%ebp), %edi                                #5245.20
        movl      %ebp, %esp                                    #5245.20
        popl      %ebp                                          #5245.20
        ret                                                     #5245.20
                                # LOE
..1.30_0.TAG.03.0.0:
..B1.79:                        # Preds ..B1.9
        movl      %edx, -4(%ebp)                                #
                                # LOE esi edi
..B1.80:                        # Preds ..B1.79
        movl      4(%edi), %edx                                 #5239.24
        cmpl      %edx, %esi                                    #5239.24
        jb        ..B1.108      # Prob 28%                      #5239.24
                                # LOE edx esi edi
..B1.81:                        # Preds ..B1.80
        movl      24(%ebp), %eax                                #5240.9
        testl     %eax, %eax                                    #5240.9
        je        ..B1.83       # Prob 12%                      #5240.9
                                # LOE edx esi edi
..B1.82:                        # Preds ..B1.81
        movl      24(%ebp), %eax                                #5240.18
        movl      %edx, (%eax)                                  #5240.18
                                # LOE esi edi
..B1.83:                        # Preds ..B1.82 ..B1.81
        movl      -4(%ebp), %eax                                #5218.5
        movl      12(%ebp), %edx                                #5218.5
        pushl     %esi                                          #5218.5
        pushl     %eax                                          #5218.5
        pushl     %edx                                          #5218.5
        pushl     %edi                                          #5218.5
        call      eval_2na_16                                   #5241.20
                                # LOE eax
..B1.127:                       # Preds ..B1.83
        addl      $16, %esp                                     #5241.20
                                # LOE eax
..B1.84:                        # Preds ..B1.127
        movl      -16(%ebp), %ebx                               #5241.20
        movl      -12(%ebp), %esi                               #5241.20
        movl      -8(%ebp), %edi                                #5241.20
        movl      %ebp, %esp                                    #5241.20
        popl      %ebp                                          #5241.20
        ret                                                     #5241.20
                                # LOE
..1.30_0.TAG.02.0.0:
..B1.86:                        # Preds ..B1.9
        movl      %edx, -4(%ebp)                                #
                                # LOE esi edi
..B1.87:                        # Preds ..B1.86
        movl      4(%edi), %edx                                 #5235.24
        cmpl      %edx, %esi                                    #5235.24
        jb        ..B1.108      # Prob 28%                      #5235.24
                                # LOE edx esi edi
..B1.88:                        # Preds ..B1.87
        movl      24(%ebp), %eax                                #5236.9
        testl     %eax, %eax                                    #5236.9
        je        ..B1.90       # Prob 12%                      #5236.9
                                # LOE edx esi edi
..B1.89:                        # Preds ..B1.88
        movl      24(%ebp), %eax                                #5236.18
        movl      %edx, (%eax)                                  #5236.18
                                # LOE esi edi
..B1.90:                        # Preds ..B1.89 ..B1.88
        movl      -4(%ebp), %eax                                #5218.5
        movl      12(%ebp), %edx                                #5218.5
        pushl     %esi                                          #5218.5
        pushl     %eax                                          #5218.5
        pushl     %edx                                          #5218.5
        pushl     %edi                                          #5218.5
        call      eval_2na_8                                    #5237.20
                                # LOE eax
..B1.128:                       # Preds ..B1.90
        addl      $16, %esp                                     #5237.20
                                # LOE eax
..B1.91:                        # Preds ..B1.128
        movl      -16(%ebp), %ebx                               #5237.20
        movl      -12(%ebp), %esi                               #5237.20
        movl      -8(%ebp), %edi                                #5237.20
        movl      %ebp, %esp                                    #5237.20
        popl      %ebp                                          #5237.20
        ret                                                     #5237.20
                                # LOE
..1.30_0.TAG.01.0.0:
..B1.93:                        # Preds ..B1.9
        movl      %edx, -4(%ebp)                                #
                                # LOE esi edi
..B1.94:                        # Preds ..B1.93
        movl      4(%edi), %edx                                 #5230.24
        cmpl      %edx, %esi                                    #5230.24
        jb        ..B1.108      # Prob 28%                      #5230.24
                                # LOE edx esi edi
..B1.95:                        # Preds ..B1.94
        movl      24(%ebp), %eax                                #5231.9
        testl     %eax, %eax                                    #5231.9
        je        ..B1.97       # Prob 12%                      #5231.9
                                # LOE edx esi edi
..B1.96:                        # Preds ..B1.95
        movl      24(%ebp), %eax                                #5231.18
        movl      %edx, (%eax)                                  #5231.18
                                # LOE esi edi
..B1.97:                        # Preds ..B1.96 ..B1.95
        movl      -4(%ebp), %eax                                #5218.5
        movl      12(%ebp), %edx                                #5218.5
        pushl     %esi                                          #5218.5
        pushl     %eax                                          #5218.5
        pushl     %edx                                          #5218.5
        pushl     %edi                                          #5218.5
        call      eval_4na_64                                   #5232.20
                                # LOE eax
..B1.129:                       # Preds ..B1.97
        addl      $16, %esp                                     #5232.20
                                # LOE eax
..B1.98:                        # Preds ..B1.129
        movl      -16(%ebp), %ebx                               #5232.20
        movl      -12(%ebp), %esi                               #5232.20
        movl      -8(%ebp), %edi                                #5232.20
        movl      %ebp, %esp                                    #5232.20
        popl      %ebp                                          #5232.20
        ret                                                     #5232.20
                                # LOE
..1.30_0.TAG.00.0.0:
..B1.100:                       # Preds ..B1.9
        movl      %edx, -4(%ebp)                                #
                                # LOE esi edi
..B1.101:                       # Preds ..B1.100
        movl      4(%edi), %edx                                 #5226.24
        cmpl      %edx, %esi                                    #5226.24
        jb        ..B1.108      # Prob 28%                      #5226.24
                                # LOE edx esi edi
..B1.102:                       # Preds ..B1.101
        movl      24(%ebp), %eax                                #5227.9
        testl     %eax, %eax                                    #5227.9
        je        ..B1.104      # Prob 12%                      #5227.9
                                # LOE edx esi edi
..B1.103:                       # Preds ..B1.102
        movl      24(%ebp), %eax                                #5227.18
        movl      %edx, (%eax)                                  #5227.18
                                # LOE esi edi
..B1.104:                       # Preds ..B1.103 ..B1.102
        movl      -4(%ebp), %eax                                #5218.5
        movl      12(%ebp), %edx                                #5218.5
        pushl     %esi                                          #5218.5
        pushl     %eax                                          #5218.5
        pushl     %edx                                          #5218.5
        pushl     %edi                                          #5218.5
        call      eval_2na_64                                   #5228.20
                                # LOE eax
..B1.130:                       # Preds ..B1.104
        addl      $16, %esp                                     #5228.20
                                # LOE eax
..B1.105:                       # Preds ..B1.130
        movl      -16(%ebp), %ebx                               #5228.20
        movl      -12(%ebp), %esi                               #5228.20
        movl      -8(%ebp), %edi                                #5228.20
        movl      %ebp, %esp                                    #5228.20
        popl      %ebp                                          #5228.20
        ret                                                     #5228.20
                                # LOE
..B1.108:                       # Preds ..B1.14 ..B1.17 ..B1.18 ..B1.15 ..B1.8
                                #       ..B1.32 ..B1.16 ..B1.5 ..B1.101 ..B1.94
                                #       ..B1.87 ..B1.80 ..B1.73 ..B1.67 ..B1.60
                                #       ..B1.53 ..B1.47 ..B1.41 ..B1.35 ..B1.23
                                #       ..B1.3 ..B1.2
        movl      -16(%ebp), %ebx                               #5311.12
        movl      -12(%ebp), %esi                               #5311.12
        movl      -8(%ebp), %edi                                #5311.12
        xorl      %eax, %eax                                    #5311.12
        movl      %ebp, %esp                                    #5311.12
        popl      %ebp                                          #5311.12
        ret                                                     #5311.12
        .align    2,0x90
                                # LOE
# mark_end;
	.type	NucStrstrSearch,@function
	.size	NucStrstrSearch,.-NucStrstrSearch
.LNNucStrstrSearch:
	.section .data1, "wa"
	.align 4
	.align 4
..1..TPKT.30_0.0.0:
	.long	..1.30_0.TAG.00.0.0
	.long	..1.30_0.TAG.01.0.0
	.long	..1.30_0.TAG.02.0.0
	.long	..1.30_0.TAG.03.0.0
	.long	..1.30_0.TAG.04.0.0
	.long	..1.30_0.TAG.05.0.0
	.long	..1.30_0.TAG.06.0.0
	.long	..1.30_0.TAG.07.0.0
	.long	..1.30_0.TAG.08.0.0
	.long	..1.30_0.TAG.09.0.0
	.long	..1.30_0.TAG.0a.0.0
	.long	..1.30_0.TAG.0b.0.0
	.long	..1.30_0.TAG.0c.0.0
	.data
# -- End  NucStrstrSearch
	.text
# -- Begin  eval_2na_64
# mark_begin;
       .align    2,0x90
eval_2na_64:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B2.1:                         # Preds ..B2.0
        pushl     %ebx                                          #3075.1
        movl      %esp, %ebx                                    #3075.1
        andl      $-16, %esp                                    #3075.1
        pushl     %ebp                                          #3075.1
        pushl     %ebp                                          #3075.1
        movl      4(%ebx), %ebp                                 #3075.1
        movl      %ebp, 4(%esp)                                 #3075.1
        movl      %esp, %ebp                                    #3075.1
        subl      $120, %esp                                    #3075.1
        movl      %ebx, -120(%ebp)                              #3075.1
        movl      %edi, -40(%ebp)                               #3075.1
        movl      %esi, -36(%ebp)                               #3075.1
        movl      16(%ebx), %eax                                #3073.5
        movl      %eax, -24(%ebp)                               #3073.5
        call      ..L2          # Prob 100%                     #3075.1
..L2:                                                           #
        popl      %ecx                                          #3075.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L2], %ecx       #3075.1
        lea       _gprof_pack1@GOTOFF(%ecx), %edx               #3075.1
        movl      %ecx, %ebx                                    #3075.1
        call      mcount@PLT                                    #3075.1
        movl      -120(%ebp), %ebx                              #3075.1
                                # LOE
..B2.51:                        # Preds ..B2.1
        movl      20(%ebx), %edx                                #3113.5
        movl      -24(%ebp), %eax                               #3113.5
        movl      12(%ebx), %ecx                                #3116.30
        lea       (%eax,%edx), %esi                             #3113.5
        movl      %eax, %edi                                    #3116.49
        shrl      $2, %edi                                      #3116.49
        movl      %edi, -16(%ebp)                               #3116.49
        lea       (%ecx,%edi), %edi                             #3116.30
        movl      %edi, -20(%ebp)                               #3116.30
        movl      8(%ebx), %edi                                 #3119.12
        subl      4(%edi), %esi                                 #3119.12
        movl      %esi, -28(%ebp)                               #3119.12
        lea       3(%eax,%edx), %esi                            #3122.50
        movl      -20(%ebp), %eax                               #3125.14
        movl      -16(%ebp), %edx                               #3125.14
        shrl      $2, %esi                                      #3122.57
        addl      %ecx, %esi                                    #3122.30
        testb     $15, %al                                      #3125.14
        jne       ..B2.3        # Prob 50%                      #3125.14
                                # LOE eax edx esi al dl ah dh
..B2.2:                         # Preds ..B2.51
        movdqa    (%eax), %xmm0                                 #3125.14
        movdqa    %xmm0, -104(%ebp)                             #3125.14
        jmp       ..B2.4        # Prob 100%                     #3125.14
                                # LOE edx esi dl dh
..B2.3:                         # Preds ..B2.51
        movdqu    (%eax), %xmm0                                 #3125.14
        movdqa    %xmm0, -104(%ebp)                             #3125.14
                                # LOE edx esi dl dh
..B2.4:                         # Preds ..B2.2 ..B2.3
        movl      12(%ebx), %eax                                #3126.5
        lea       16(%eax,%edx), %edx                           #3126.5
        movl      %edx, -32(%ebp)                               #3126.5
        cmpl      %esi, %edx                                    #3131.16
        jae       ..B2.6        # Prob 12%                      #3131.16
                                # LOE edx esi dl dh
..B2.5:                         # Preds ..B2.4
        movzbl    -1(%edx), %eax                                #3132.24
        shll      $8, %eax                                      #3132.38
        jmp       ..B2.7        # Prob 100%                     #3132.38
                                # LOE eax edx esi dl dh
..B2.6:                         # Preds ..B2.4
        xorl      %eax, %eax                                    #
                                # LOE eax edx esi dl dh
..B2.7:                         # Preds ..B2.5 ..B2.6
        movl      8(%ebx), %ecx                                 #3136.5
        movl      -24(%ebp), %edi                               #3148.20
        movdqa    16(%ecx), %xmm6                               #3136.5
        movdqa    32(%ecx), %xmm5                               #3136.5
        movdqa    48(%ecx), %xmm7                               #3136.5
        movdqa    64(%ecx), %xmm4                               #3136.5
        movdqa    80(%ecx), %xmm3                               #3136.5
        movdqa    96(%ecx), %xmm2                               #3136.5
        movdqa    112(%ecx), %xmm0                              #3136.5
        movdqa    128(%ecx), %xmm1                              #3136.5
        andl      $3, %edi                                      #3148.20
        je        ..B2.43       # Prob 20%                      #3148.20
                                # LOE eax edx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B2.8:                         # Preds ..B2.7
        cmpl      $1, %edi                                      #3148.20
        je        ..B2.14       # Prob 25%                      #3148.20
                                # LOE eax edx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B2.9:                         # Preds ..B2.8
        cmpl      $2, %edi                                      #3148.20
        je        ..B2.13       # Prob 33%                      #3148.20
                                # LOE eax edx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B2.10:                        # Preds ..B2.9
        cmpl      $3, %edi                                      #3148.20
        je        ..B2.12       # Prob 50%                      #3148.20
                                # LOE eax edx esi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B2.11:                        # Preds ..B2.10
        movdqa    %xmm0, -88(%ebp)                              #
        movdqa    -104(%ebp), %xmm0                             #
        movdqa    %xmm7, -72(%ebp)                              #
        movl      %esi, -48(%ebp)                               #
        jmp       ..B2.35       # Prob 100%                     #
                                # LOE eax edx dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.12:                        # Preds ..B2.10
        movdqa    %xmm0, -88(%ebp)                              #
        movdqa    -104(%ebp), %xmm0                             #
        movdqa    %xmm7, -72(%ebp)                              #
        movl      %esi, -48(%ebp)                               #
        xorl      %ecx, %ecx                                    #
        movl      %ecx, -56(%ebp)                               #
        movl      %ecx, -52(%ebp)                               #
        movl      %ecx, -44(%ebp)                               #
        movl      $8, %ecx                                      #
        jmp       ..B2.17       # Prob 100%                     #
                                # LOE eax edx ecx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.13:                        # Preds ..B2.9
        movdqa    %xmm0, -88(%ebp)                              #
        movdqa    -104(%ebp), %xmm0                             #
        movdqa    %xmm7, -72(%ebp)                              #
        movl      %esi, -48(%ebp)                               #
        xorl      %ecx, %ecx                                    #
        movl      %ecx, -52(%ebp)                               #
        movl      %ecx, -44(%ebp)                               #
        movl      $8, %ecx                                      #
        jmp       ..B2.16       # Prob 100%                     #
                                # LOE eax edx ecx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.14:                        # Preds ..B2.8
        movdqa    %xmm0, -88(%ebp)                              #
        movdqa    -104(%ebp), %xmm0                             #
        movdqa    %xmm7, -72(%ebp)                              #
        movl      %esi, -48(%ebp)                               #
        xorl      %edi, %edi                                    #
        movl      %edi, -44(%ebp)                               #
        movl      $8, %ecx                                      #
                                # LOE eax edx ecx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.15:                        # Preds ..B2.14 ..B2.39
        movdqa    %xmm0, %xmm7                                  #3171.22
        pand      %xmm4, %xmm7                                  #3171.22
        pcmpeqd   -72(%ebp), %xmm7                              #3172.22
        pmovmskb  %xmm7, %esi                                   #3173.22
        movl      %esi, %edi                                    #3174.17
        andl      $3855, %edi                                   #3174.17
        shll      $4, %edi                                      #3174.17
        andl      %edi, %esi                                    #3174.17
        movl      %esi, %edi                                    #3174.17
        sarl      $4, %edi                                      #3174.17
        orl       %edi, %esi                                    #3174.17
        movl      %esi, -52(%ebp)                               #3174.17
                                # LOE eax edx ecx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.16:                        # Preds ..B2.13 ..B2.15
        movdqa    %xmm0, %xmm7                                  #3177.22
        pand      %xmm2, %xmm7                                  #3177.22
        pcmpeqd   %xmm3, %xmm7                                  #3178.22
        pmovmskb  %xmm7, %esi                                   #3179.22
        movl      %esi, %edi                                    #3180.17
        andl      $3855, %edi                                   #3180.17
        shll      $4, %edi                                      #3180.17
        andl      %edi, %esi                                    #3180.17
        movl      %esi, %edi                                    #3180.17
        sarl      $4, %edi                                      #3180.17
        orl       %edi, %esi                                    #3180.17
        movl      %esi, -56(%ebp)                               #3180.17
                                # LOE eax edx ecx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.17:                        # Preds ..B2.12 ..B2.16
        movdqa    %xmm0, %xmm7                                  #3183.22
        pand      %xmm1, %xmm7                                  #3183.22
        pcmpeqd   -88(%ebp), %xmm7                              #3184.22
        pmovmskb  %xmm7, %esi                                   #3185.22
        movl      %esi, %edi                                    #3186.17
        andl      $3855, %edi                                   #3186.17
        shll      $4, %edi                                      #3186.17
        andl      %edi, %esi                                    #3186.17
        movl      %esi, %edi                                    #3186.17
        sarl      $4, %edi                                      #3186.17
        orl       %edi, %esi                                    #3186.17
        movl      -24(%ebp), %edi                               #3190.17
        movl      %esi, -20(%ebp)                               #3186.17
        andl      $-4, %edi                                     #3190.17
        movl      %edi, -16(%ebp)                               #3190.17
        movl      -44(%ebp), %edi                               #3193.29
        orl       -52(%ebp), %edi                               #3193.29
        orl       -56(%ebp), %edi                               #3193.34
        orl       %esi, %edi                                    #3193.39
        je        ..B2.26       # Prob 78%                      #3193.47
                                # LOE eax edx ecx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.18:                        # Preds ..B2.17
        movl      -44(%ebp), %edi                               #3211.58
        movl      %ecx, -104(%ebp)                              #
        movl      %edx, -100(%ebp)                              #
        movl      %eax, -96(%ebp)                               #
        movzwl    %di, %eax                                     #3211.58
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #3211.30
        movswl    %ax, %eax                                     #3211.30
        movl      %eax, -24(%ebp)                               #3211.30
        movl      -52(%ebp), %eax                               #3212.58
        movzwl    %ax, %eax                                     #3212.58
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #3212.30
        movswl    %ax, %esi                                     #3212.30
        movl      -56(%ebp), %eax                               #3213.58
        movzwl    %ax, %eax                                     #3213.58
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #3213.30
        movswl    %ax, %ecx                                     #3213.30
        movl      -20(%ebp), %eax                               #3214.58
        movzwl    %ax, %eax                                     #3214.58
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #3214.30
        movswl    %ax, %edx                                     #3214.30
        lea       1(,%esi,4), %eax                              #3220.40
        movl      %eax, -116(%ebp)                              #3220.40
        lea       2(,%ecx,4), %eax                              #3221.40
        movl      -104(%ebp), %ecx                              #3226.32
        testl     %edi, %edi                                    #3226.32
        movl      %eax, -112(%ebp)                              #3221.40
        lea       3(,%edx,4), %eax                              #3222.40
        movl      -100(%ebp), %edx                              #3226.32
        movl      %eax, -108(%ebp)                              #3222.40
        movl      -96(%ebp), %eax                               #3226.32
        je        ..B2.20       # Prob 50%                      #3226.32
                                # LOE eax edx ecx al dl cl ah dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.19:                        # Preds ..B2.18
        movl      -24(%ebp), %edi                               #3226.43
        movl      -16(%ebp), %esi                               #3226.43
        lea       (%esi,%edi,4), %edi                           #3226.43
        movl      -28(%ebp), %esi                               #3226.49
        cmpl      %edi, %esi                                    #3226.49
        jae       ..B2.44       # Prob 1%                       #3226.49
                                # LOE eax edx ecx al dl cl ah dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.20:                        # Preds ..B2.19 ..B2.18
        movl      -52(%ebp), %esi                               #3227.32
        testl     %esi, %esi                                    #3227.32
        je        ..B2.22       # Prob 50%                      #3227.32
                                # LOE eax edx ecx al dl cl ah dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.21:                        # Preds ..B2.20
        movl      -116(%ebp), %edi                              #3227.43
        addl      -16(%ebp), %edi                               #3227.43
        movl      -28(%ebp), %esi                               #3227.49
        cmpl      %edi, %esi                                    #3227.49
        jae       ..B2.44       # Prob 1%                       #3227.49
                                # LOE eax edx ecx al dl cl ah dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.22:                        # Preds ..B2.21 ..B2.20
        movl      -56(%ebp), %esi                               #3228.32
        testl     %esi, %esi                                    #3228.32
        je        ..B2.24       # Prob 50%                      #3228.32
                                # LOE eax edx ecx al dl cl ah dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.23:                        # Preds ..B2.22
        movl      -112(%ebp), %edi                              #3228.43
        addl      -16(%ebp), %edi                               #3228.43
        movl      -28(%ebp), %esi                               #3228.49
        cmpl      %edi, %esi                                    #3228.49
        jae       ..B2.44       # Prob 1%                       #3228.49
                                # LOE eax edx ecx al dl cl ah dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.24:                        # Preds ..B2.23 ..B2.22
        movl      -20(%ebp), %esi                               #3229.32
        testl     %esi, %esi                                    #3229.32
        je        ..B2.26       # Prob 50%                      #3229.32
                                # LOE eax edx ecx al dl cl ah dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.25:                        # Preds ..B2.24
        movl      -108(%ebp), %edi                              #3229.43
        addl      -16(%ebp), %edi                               #3229.43
        movl      -28(%ebp), %esi                               #3229.49
        cmpl      %edi, %esi                                    #3229.49
        jae       ..B2.44       # Prob 1%                       #3229.49
                                # LOE eax edx ecx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.26:                        # Preds ..B2.25 ..B2.24 ..B2.17
        movl      -16(%ebp), %esi                               #3234.17
        lea       4(%esi), %edi                                 #3234.17
        movl      -28(%ebp), %esi                               #3237.28
        movl      %edi, -24(%ebp)                               #3234.17
        cmpl      %esi, %edi                                    #3237.28
        ja        ..B2.41       # Prob 1%                       #3237.28
                                # LOE eax edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.27:                        # Preds ..B2.26
        addl      $-1, %ecx                                     #3241.25
        jne       ..B2.36       # Prob 50%                      #3241.39
                                # LOE eax edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.28:                        # Preds ..B2.27
        movl      -48(%ebp), %ecx                               #3262.25
        cmpl      %ecx, %edx                                    #3262.25
        jae       ..B2.41       # Prob 1%                       #3262.25
                                # LOE eax edx esi xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.29:                        # Preds ..B2.28
        movl      -16(%ebp), %ecx                               #3266.13
        movl      %esi, %edi                                    #3267.24
        lea       36(%ecx), %esi                                #3266.13
        movl      %esi, -24(%ebp)                               #3266.13
        cmpl      %edi, %esi                                    #3267.24
        ja        ..B2.41       # Prob 1%                       #3267.24
                                # LOE eax edx xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.30:                        # Preds ..B2.29
        testb     $15, %dl                                      #3272.22
        jne       ..B2.32       # Prob 50%                      #3272.22
                                # LOE eax edx xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.31:                        # Preds ..B2.30
        movdqa    (%edx), %xmm0                                 #3272.22
        jmp       ..B2.33       # Prob 100%                     #3272.22
                                # LOE eax edx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.32:                        # Preds ..B2.30
        movdqu    (%edx), %xmm0                                 #3272.22
                                # LOE eax edx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.33:                        # Preds ..B2.31 ..B2.32
        movl      -48(%ebp), %ecx                               #3353.24
        addl      $16, %edx                                     #3350.13
        movl      %edx, -32(%ebp)                               #3350.13
        cmpl      %ecx, %edx                                    #3353.24
        jae       ..B2.35       # Prob 12%                      #3353.24
                                # LOE eax edx dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.34:                        # Preds ..B2.33
        movzbl    -1(%edx), %eax                                #3354.32
        shll      $8, %eax                                      #3354.46
                                # LOE eax edx dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.35:                        # Preds ..B2.33 ..B2.11 ..B2.34
        movl      $8, %ecx                                      #3155.13
        jmp       ..B2.39       # Prob 100%                     #3155.13
                                # LOE eax edx ecx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.36:                        # Preds ..B2.27
        movl      -32(%ebp), %esi                               #3249.26
        movl      -48(%ebp), %edi                               #3249.26
        psrldq    $1, %xmm0                                     #3246.26
        cmpl      %edi, %esi                                    #3249.26
        jae       ..B2.38       # Prob 19%                      #3249.26
                                # LOE eax edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.37:                        # Preds ..B2.36
        movzbl    (%esi), %edi                                  #3252.37
        sarl      $8, %eax                                      #3251.21
        shll      $8, %edi                                      #3252.48
        orl       %edi, %eax                                    #3252.21
        pinsrw    $7, %eax, %xmm0                               #3253.30
                                # LOE eax edx ecx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.38:                        # Preds ..B2.37 ..B2.36
        addl      $1, -32(%ebp)                                 #3257.20
                                # LOE eax edx ecx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.39:                        # Preds ..B2.35 ..B2.38 ..B2.43
        movdqa    %xmm0, %xmm7                                  #3165.22
        pand      %xmm5, %xmm7                                  #3165.22
        pcmpeqd   %xmm6, %xmm7                                  #3166.22
        pmovmskb  %xmm7, %esi                                   #3167.22
        movl      %esi, %edi                                    #3168.17
        andl      $3855, %edi                                   #3168.17
        shll      $4, %edi                                      #3168.17
        andl      %edi, %esi                                    #3168.17
        movl      %esi, %edi                                    #3168.17
        sarl      $4, %edi                                      #3168.17
        orl       %edi, %esi                                    #3168.17
        movl      %esi, -44(%ebp)                               #3168.17
        jmp       ..B2.15       # Prob 100%                     #3168.17
                                # LOE eax edx ecx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.41:                        # Preds ..B2.26 ..B2.28 ..B2.29 # Infreq
        movl      -36(%ebp), %esi                               #3363.12
        movl      -40(%ebp), %edi                               #3363.12
        xorl      %eax, %eax                                    #3363.12
        movl      %ebp, %esp                                    #3363.12
        popl      %ebp                                          #3363.12
        movl      %ebx, %esp                                    #3363.12
        popl      %ebx                                          #3363.12
        ret                                                     #3363.12
                                # LOE
..B2.43:                        # Preds ..B2.7                  # Infreq
        movdqa    %xmm0, -88(%ebp)                              #
        movdqa    -104(%ebp), %xmm0                             #
        movdqa    %xmm7, -72(%ebp)                              #
        movl      %esi, -48(%ebp)                               #
        movl      $8, %ecx                                      #
        jmp       ..B2.39       # Prob 100%                     #
                                # LOE eax edx ecx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.44:                        # Preds ..B2.21 ..B2.19 ..B2.25 ..B2.23 # Infreq
        movl      -36(%ebp), %esi                               #3228.63
        movl      -40(%ebp), %edi                               #3228.63
        movl      $1, %eax                                      #3228.63
        movl      %ebp, %esp                                    #3228.63
        popl      %ebp                                          #3228.63
        movl      %ebx, %esp                                    #3228.63
        popl      %ebx                                          #3228.63
        ret                                                     #3228.63
        .align    2,0x90
                                # LOE
# mark_end;
	.type	eval_2na_64,@function
	.size	eval_2na_64,.-eval_2na_64
.LNeval_2na_64:
	.data
# -- End  eval_2na_64
	.text
# -- Begin  eval_4na_64
# mark_begin;
       .align    2,0x90
eval_4na_64:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B3.1:                         # Preds ..B3.0
        pushl     %ebx                                          #4501.1
        movl      %esp, %ebx                                    #4501.1
        andl      $-16, %esp                                    #4501.1
        pushl     %ebp                                          #4501.1
        pushl     %ebp                                          #4501.1
        movl      4(%ebx), %ebp                                 #4501.1
        movl      %ebp, 4(%esp)                                 #4501.1
        movl      %esp, %ebp                                    #4501.1
        subl      $152, %esp                                    #4501.1
        movl      %ebx, -152(%ebp)                              #4501.1
        movl      %edi, -44(%ebp)                               #4501.1
        movl      %esi, -40(%ebp)                               #4501.1
        movl      12(%ebx), %ecx                                #4499.5
        movl      16(%ebx), %esi                                #4499.5
        call      ..L3          # Prob 100%                     #4501.1
..L3:                                                           #
        popl      %edi                                          #4501.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L3], %edi       #4501.1
        lea       _gprof_pack2@GOTOFF(%edi), %edx               #4501.1
        movl      %edi, %ebx                                    #4501.1
        call      mcount@PLT                                    #4501.1
        movl      -152(%ebp), %ebx                              #4501.1
                                # LOE esi edi
..B3.41:                        # Preds ..B3.1
        movl      12(%ebx), %ecx                                #
        movl      %esi, -24(%ebp)                               #
        movl      %esi, %eax                                    #4538.49
        movl      %edi, -16(%ebp)                               #
        shrl      $2, %eax                                      #4538.49
        movl      20(%ebx), %edi                                #4535.5
        movl      %eax, -20(%ebp)                               #4538.49
        movl      8(%ebx), %eax                                 #4541.12
        movdqa    16(%eax), %xmm5                               #4554.5
        movdqa    32(%eax), %xmm4                               #4554.5
        movdqa    48(%eax), %xmm3                               #4554.5
        movdqa    64(%eax), %xmm6                               #4554.5
        movdqa    80(%eax), %xmm2                               #4554.5
        movdqa    96(%eax), %xmm1                               #4554.5
        movdqa    128(%eax), %xmm7                              #4554.5
        lea       (%esi,%edi), %edx                             #4535.5
        subl      4(%eax), %edx                                 #4541.12
        movl      %edx, -32(%ebp)                               #4541.12
        lea       3(%esi,%edi), %edx                            #4544.50
        shrl      $2, %edx                                      #4544.57
        movl      -20(%ebp), %esi                               #4547.14
        movl      -16(%ebp), %edi                               #4547.14
        addl      %ecx, %edx                                    #4544.30
        movl      %edx, -36(%ebp)                               #4544.30
        movzbl    (%esi,%ecx), %edx                             #4547.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4547.14
        movw      %dx, -148(%ebp)                               #4547.14
        movzbl    1(%esi,%ecx), %edx                            #4547.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4547.14
        movw      %dx, -146(%ebp)                               #4547.14
        movzbl    2(%esi,%ecx), %edx                            #4547.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4547.14
        movw      %dx, -144(%ebp)                               #4547.14
        movzbl    3(%esi,%ecx), %edx                            #4547.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4547.14
        movw      %dx, -142(%ebp)                               #4547.14
        movzbl    4(%esi,%ecx), %edx                            #4547.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4547.14
        movw      %dx, -140(%ebp)                               #4547.14
        movzbl    5(%esi,%ecx), %edx                            #4547.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4547.14
        movw      %dx, -138(%ebp)                               #4547.14
        movzbl    6(%esi,%ecx), %edx                            #4547.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4547.14
        movw      %dx, -136(%ebp)                               #4547.14
        movzbl    7(%esi,%ecx), %edx                            #4547.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4547.14
        movw      %dx, -134(%ebp)                               #4547.14
        movdqu    -148(%ebp), %xmm0                             #4547.14
        lea       8(%ecx,%esi), %edx                            #4548.5
        movl      -24(%ebp), %esi                               #4568.20
        movdqa    %xmm0, -120(%ebp)                             #4547.14
        movdqa    112(%eax), %xmm0                              #4554.5
        movl      %esi, %eax                                    #4568.20
        movl      %edx, -28(%ebp)                               #4548.5
        andl      $3, %eax                                      #4568.20
        je        ..B3.33       # Prob 20%                      #4568.20
                                # LOE eax edx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B3.2:                         # Preds ..B3.41
        cmpl      $1, %eax                                      #4568.20
        je        ..B3.8        # Prob 25%                      #4568.20
                                # LOE eax edx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B3.3:                         # Preds ..B3.2
        cmpl      $2, %eax                                      #4568.20
        je        ..B3.7        # Prob 33%                      #4568.20
                                # LOE eax edx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B3.4:                         # Preds ..B3.3
        cmpl      $3, %eax                                      #4568.20
        je        ..B3.6        # Prob 50%                      #4568.20
                                # LOE edx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B3.5:                         # Preds ..B3.4
        movdqa    %xmm7, -104(%ebp)                             #
        movdqa    %xmm0, -72(%ebp)                              #
        movdqa    -120(%ebp), %xmm0                             #
        movdqa    %xmm6, -88(%ebp)                              #
        movl      %edi, -16(%ebp)                               #
        jmp       ..B3.25       # Prob 100%                     #
                                # LOE edx esi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.6:                         # Preds ..B3.4
        movdqa    %xmm7, -104(%ebp)                             #
        movdqa    %xmm0, -72(%ebp)                              #
        movdqa    -120(%ebp), %xmm0                             #
        movdqa    %xmm6, -88(%ebp)                              #
        xorl      %eax, %eax                                    #
        movl      %eax, -56(%ebp)                               #
        movl      %edi, -16(%ebp)                               #
        xorl      %ecx, %ecx                                    #
        movl      $4, %eax                                      #
        movl      %ecx, -52(%ebp)                               #
        movl      %ecx, -48(%ebp)                               #
        jmp       ..B3.11       # Prob 100%                     #
                                # LOE eax edx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.7:                         # Preds ..B3.3
        movdqa    %xmm7, -104(%ebp)                             #
        movdqa    %xmm0, -72(%ebp)                              #
        movdqa    -120(%ebp), %xmm0                             #
        movdqa    %xmm6, -88(%ebp)                              #
        xorl      %eax, %eax                                    #
        movl      %eax, -52(%ebp)                               #
        movl      %edi, -16(%ebp)                               #
        xorl      %ecx, %ecx                                    #
        movl      $4, %eax                                      #
        movl      %ecx, -48(%ebp)                               #
        jmp       ..B3.10       # Prob 100%                     #
                                # LOE eax edx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.8:                         # Preds ..B3.2
        movdqa    %xmm7, -104(%ebp)                             #
        movdqa    %xmm0, -72(%ebp)                              #
        movdqa    -120(%ebp), %xmm0                             #
        movdqa    %xmm6, -88(%ebp)                              #
        xorl      %ecx, %ecx                                    #
        movl      %ecx, -48(%ebp)                               #
        movl      %edi, -16(%ebp)                               #
        movl      $4, %eax                                      #
                                # LOE eax edx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.9:                         # Preds ..B3.8 ..B3.29
        movdqa    %xmm0, %xmm7                                  #4592.22
        movdqa    %xmm0, %xmm6                                  #4593.22
        pand      -88(%ebp), %xmm6                              #4593.22
        pand      %xmm3, %xmm7                                  #4592.22
        pcmpeqd   %xmm6, %xmm7                                  #4594.22
        pmovmskb  %xmm7, %ecx                                   #4595.22
        movl      %ecx, %edi                                    #4596.17
        andl      $3855, %edi                                   #4596.17
        shll      $4, %edi                                      #4596.17
        andl      %edi, %ecx                                    #4596.17
        movl      %ecx, %edi                                    #4596.17
        sarl      $4, %edi                                      #4596.17
        orl       %edi, %ecx                                    #4596.17
        movl      %ecx, -52(%ebp)                               #4596.17
                                # LOE eax edx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.10:                        # Preds ..B3.7 ..B3.9
        movdqa    %xmm0, %xmm7                                  #4599.22
        movdqa    %xmm0, %xmm6                                  #4600.22
        pand      %xmm2, %xmm7                                  #4599.22
        pand      %xmm1, %xmm6                                  #4600.22
        pcmpeqd   %xmm6, %xmm7                                  #4601.22
        pmovmskb  %xmm7, %ecx                                   #4602.22
        movl      %ecx, %edi                                    #4603.17
        andl      $3855, %edi                                   #4603.17
        shll      $4, %edi                                      #4603.17
        andl      %edi, %ecx                                    #4603.17
        movl      %ecx, %edi                                    #4603.17
        sarl      $4, %edi                                      #4603.17
        orl       %edi, %ecx                                    #4603.17
        movl      %ecx, -56(%ebp)                               #4603.17
                                # LOE eax edx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.11:                        # Preds ..B3.6 ..B3.10
        movdqa    %xmm0, %xmm7                                  #4606.22
        movdqa    %xmm0, %xmm6                                  #4607.22
        pand      -72(%ebp), %xmm7                              #4606.22
        pand      -104(%ebp), %xmm6                             #4607.22
        pcmpeqd   %xmm6, %xmm7                                  #4608.22
        pmovmskb  %xmm7, %edi                                   #4609.22
        movl      %edi, %ecx                                    #4610.17
        andl      $3855, %ecx                                   #4610.17
        shll      $4, %ecx                                      #4610.17
        andl      %ecx, %edi                                    #4610.17
        movl      %edi, %ecx                                    #4610.17
        sarl      $4, %ecx                                      #4610.17
        orl       %ecx, %edi                                    #4610.17
        movl      %edi, -20(%ebp)                               #4610.17
        movl      %esi, %ecx                                    #4614.17
        andl      $-4, %ecx                                     #4614.17
        movl      -48(%ebp), %esi                               #4617.29
        orl       -52(%ebp), %esi                               #4617.29
        orl       -56(%ebp), %esi                               #4617.34
        orl       %edi, %esi                                    #4617.39
        je        ..B3.20       # Prob 78%                      #4617.47
                                # LOE eax edx ecx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.12:                        # Preds ..B3.11
        movl      -48(%ebp), %edi                               #4635.58
        movl      %ecx, -120(%ebp)                              #
        movl      %eax, -116(%ebp)                              #
        movzwl    %di, %eax                                     #4635.58
        movl      %edx, -112(%ebp)                              #
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4635.30
        movswl    %ax, %eax                                     #4635.30
        movl      %eax, -132(%ebp)                              #4635.30
        movl      -52(%ebp), %eax                               #4636.58
        movzwl    %ax, %eax                                     #4636.58
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4636.30
        movswl    %ax, %esi                                     #4636.30
        movl      -56(%ebp), %eax                               #4637.58
        movzwl    %ax, %eax                                     #4637.58
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4637.30
        movswl    %ax, %ecx                                     #4637.30
        movl      -20(%ebp), %eax                               #4638.58
        movzwl    %ax, %eax                                     #4638.58
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4638.30
        movswl    %ax, %edx                                     #4638.30
        lea       1(%esi,%esi), %eax                            #4644.40
        movl      %eax, -128(%ebp)                              #4644.40
        lea       2(%ecx,%ecx), %eax                            #4645.40
        movl      -120(%ebp), %ecx                              #4650.32
        testl     %edi, %edi                                    #4650.32
        movl      %eax, -124(%ebp)                              #4645.40
        lea       3(%edx,%edx), %eax                            #4646.40
        movl      -112(%ebp), %edx                              #4650.32
        movl      %eax, -24(%ebp)                               #4646.40
        movl      -116(%ebp), %eax                              #4650.32
        je        ..B3.14       # Prob 50%                      #4650.32
                                # LOE eax edx ecx al dl cl ah dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.13:                        # Preds ..B3.12
        movl      -132(%ebp), %esi                              #4650.43
        movl      -32(%ebp), %edi                               #4650.49
        lea       (%ecx,%esi,2), %esi                           #4650.43
        cmpl      %esi, %edi                                    #4650.49
        jae       ..B3.34       # Prob 1%                       #4650.49
                                # LOE eax edx ecx al dl cl ah dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.14:                        # Preds ..B3.13 ..B3.12
        movl      -52(%ebp), %esi                               #4651.32
        testl     %esi, %esi                                    #4651.32
        je        ..B3.16       # Prob 50%                      #4651.32
                                # LOE eax edx ecx al dl cl ah dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.15:                        # Preds ..B3.14
        movl      -128(%ebp), %edi                              #4651.43
        movl      -32(%ebp), %esi                               #4651.49
        addl      %ecx, %edi                                    #4651.43
        cmpl      %edi, %esi                                    #4651.49
        jae       ..B3.34       # Prob 1%                       #4651.49
                                # LOE eax edx ecx al dl cl ah dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.16:                        # Preds ..B3.15 ..B3.14
        movl      -56(%ebp), %esi                               #4652.32
        testl     %esi, %esi                                    #4652.32
        je        ..B3.18       # Prob 50%                      #4652.32
                                # LOE eax edx ecx al dl cl ah dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.17:                        # Preds ..B3.16
        movl      -124(%ebp), %edi                              #4652.43
        movl      -32(%ebp), %esi                               #4652.49
        addl      %ecx, %edi                                    #4652.43
        cmpl      %edi, %esi                                    #4652.49
        jae       ..B3.34       # Prob 1%                       #4652.49
                                # LOE eax edx ecx al dl cl ah dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.18:                        # Preds ..B3.17 ..B3.16
        movl      -20(%ebp), %esi                               #4653.32
        testl     %esi, %esi                                    #4653.32
        je        ..B3.20       # Prob 50%                      #4653.32
                                # LOE eax edx ecx al dl cl ah dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.19:                        # Preds ..B3.18
        movl      -24(%ebp), %edi                               #4653.43
        movl      -32(%ebp), %esi                               #4653.49
        addl      %ecx, %edi                                    #4653.43
        cmpl      %edi, %esi                                    #4653.49
        jae       ..B3.34       # Prob 1%                       #4653.49
                                # LOE eax edx ecx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.20:                        # Preds ..B3.19 ..B3.18 ..B3.11
        movl      -32(%ebp), %edi                               #4661.28
        lea       4(%ecx), %esi                                 #4658.17
        cmpl      %edi, %esi                                    #4661.28
        ja        ..B3.31       # Prob 1%                       #4661.28
                                # LOE eax edx ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.21:                        # Preds ..B3.20
        addl      $-1, %eax                                     #4665.25
        jne       ..B3.26       # Prob 50%                      #4665.39
                                # LOE eax edx ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.22:                        # Preds ..B3.21
        movl      -36(%ebp), %eax                               #4682.25
        cmpl      %eax, %edx                                    #4682.25
        jae       ..B3.31       # Prob 1%                       #4682.25
                                # LOE edx ecx edi xmm1 xmm2 xmm3 xmm4 xmm5
..B3.23:                        # Preds ..B3.22
        movl      %edi, %eax                                    #4687.24
        lea       20(%ecx), %esi                                #4686.13
        cmpl      %eax, %esi                                    #4687.24
        ja        ..B3.31       # Prob 1%                       #4687.24
                                # LOE edx esi xmm1 xmm2 xmm3 xmm4 xmm5
..B3.24:                        # Preds ..B3.23
        movl      -16(%ebp), %eax                               #4692.22
        movzbl    (%edx), %ecx                                  #4692.22
        movzwl    expand_2na.0@GOTOFF(%eax,%ecx,2), %edi        #4692.22
        movzbl    1(%edx), %ecx                                 #4692.22
        movzwl    expand_2na.0@GOTOFF(%eax,%ecx,2), %ecx        #4692.22
        movw      %di, -148(%ebp)                               #4692.22
        movw      %cx, -146(%ebp)                               #4692.22
        movzbl    2(%edx), %ecx                                 #4692.22
        movzwl    expand_2na.0@GOTOFF(%eax,%ecx,2), %ecx        #4692.22
        movw      %cx, -144(%ebp)                               #4692.22
        movzbl    3(%edx), %ecx                                 #4692.22
        movzwl    expand_2na.0@GOTOFF(%eax,%ecx,2), %ecx        #4692.22
        movw      %cx, -142(%ebp)                               #4692.22
        movzbl    4(%edx), %ecx                                 #4692.22
        movzwl    expand_2na.0@GOTOFF(%eax,%ecx,2), %ecx        #4692.22
        movw      %cx, -140(%ebp)                               #4692.22
        movzbl    5(%edx), %ecx                                 #4692.22
        movzwl    expand_2na.0@GOTOFF(%eax,%ecx,2), %ecx        #4692.22
        movw      %cx, -138(%ebp)                               #4692.22
        movzbl    6(%edx), %ecx                                 #4692.22
        movzwl    expand_2na.0@GOTOFF(%eax,%ecx,2), %ecx        #4692.22
        movw      %cx, -136(%ebp)                               #4692.22
        movzbl    7(%edx), %ecx                                 #4692.22
        addl      $8, %edx                                      #4707.13
        movzwl    expand_2na.0@GOTOFF(%eax,%ecx,2), %eax        #4692.22
        movw      %ax, -134(%ebp)                               #4692.22
        movdqu    -148(%ebp), %xmm0                             #4692.22
        movl      %edx, -28(%ebp)                               #4707.13
                                # LOE edx esi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.25:                        # Preds ..B3.5 ..B3.24
        movl      $4, %eax                                      #4575.13
        jmp       ..B3.29       # Prob 100%                     #4575.13
                                # LOE eax edx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.26:                        # Preds ..B3.21
        movl      -36(%ebp), %edi                               #4673.26
        movl      -28(%ebp), %ecx                               #4673.26
        psrldq    $2, %xmm0                                     #4670.26
        cmpl      %edi, %ecx                                    #4673.26
        jae       ..B3.28       # Prob 19%                      #4673.26
                                # LOE eax edx ecx esi cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.27:                        # Preds ..B3.26
        movl      -16(%ebp), %edi                               #4674.57
        movzbl    (%ecx), %ecx                                  #4674.72
        movzwl    expand_2na.0@GOTOFF(%edi,%ecx,2), %ecx        #4674.57
        pinsrw    $7, %ecx, %xmm0                               #4674.30
                                # LOE eax edx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.28:                        # Preds ..B3.27 ..B3.26
        addl      $1, -28(%ebp)                                 #4677.20
                                # LOE eax edx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.29:                        # Preds ..B3.25 ..B3.28 ..B3.33
        movdqa    %xmm0, %xmm7                                  #4585.22
        movdqa    %xmm0, %xmm6                                  #4586.22
        pand      %xmm5, %xmm7                                  #4585.22
        pand      %xmm4, %xmm6                                  #4586.22
        pcmpeqd   %xmm6, %xmm7                                  #4587.22
        pmovmskb  %xmm7, %ecx                                   #4588.22
        movl      %ecx, %edi                                    #4589.17
        andl      $3855, %edi                                   #4589.17
        shll      $4, %edi                                      #4589.17
        andl      %edi, %ecx                                    #4589.17
        movl      %ecx, %edi                                    #4589.17
        sarl      $4, %edi                                      #4589.17
        orl       %edi, %ecx                                    #4589.17
        movl      %ecx, -48(%ebp)                               #4589.17
        jmp       ..B3.9        # Prob 100%                     #4589.17
                                # LOE eax edx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.31:                        # Preds ..B3.20 ..B3.22 ..B3.23 # Infreq
        movl      -40(%ebp), %esi                               #4718.12
        movl      -44(%ebp), %edi                               #4718.12
        xorl      %eax, %eax                                    #4718.12
        movl      %ebp, %esp                                    #4718.12
        popl      %ebp                                          #4718.12
        movl      %ebx, %esp                                    #4718.12
        popl      %ebx                                          #4718.12
        ret                                                     #4718.12
                                # LOE
..B3.33:                        # Preds ..B3.41                 # Infreq
        movdqa    %xmm7, -104(%ebp)                             #
        movdqa    %xmm0, -72(%ebp)                              #
        movdqa    -120(%ebp), %xmm0                             #
        movdqa    %xmm6, -88(%ebp)                              #
        movl      $4, %eax                                      #
        movl      %edi, -16(%ebp)                               #
        jmp       ..B3.29       # Prob 100%                     #
                                # LOE eax edx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.34:                        # Preds ..B3.13 ..B3.19 ..B3.17 ..B3.15 # Infreq
        movl      -40(%ebp), %esi                               #4651.63
        movl      -44(%ebp), %edi                               #4651.63
        movl      $1, %eax                                      #4651.63
        movl      %ebp, %esp                                    #4651.63
        popl      %ebp                                          #4651.63
        movl      %ebx, %esp                                    #4651.63
        popl      %ebx                                          #4651.63
        ret                                                     #4651.63
        .align    2,0x90
                                # LOE
# mark_end;
	.type	eval_4na_64,@function
	.size	eval_4na_64,.-eval_4na_64
.LNeval_4na_64:
	.data
# -- End  eval_4na_64
	.text
# -- Begin  eval_2na_8
# mark_begin;
       .align    2,0x90
eval_2na_8:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B4.1:                         # Preds ..B4.0
        pushl     %ebx                                          #2196.1
        movl      %esp, %ebx                                    #2196.1
        andl      $-16, %esp                                    #2196.1
        pushl     %ebp                                          #2196.1
        pushl     %ebp                                          #2196.1
        movl      4(%ebx), %ebp                                 #2196.1
        movl      %ebp, 4(%esp)                                 #2196.1
        movl      %esp, %ebp                                    #2196.1
        subl      $88, %esp                                     #2196.1
        movl      %ebx, -88(%ebp)                               #2196.1
        movl      %edi, -16(%ebp)                               #2196.1
        movl      %esi, -12(%ebp)                               #2196.1
        movl      16(%ebx), %esi                                #2194.5
        call      ..L4          # Prob 100%                     #2196.1
..L4:                                                           #
        popl      %eax                                          #2196.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L4], %eax       #2196.1
        lea       _gprof_pack3@GOTOFF(%eax), %edx               #2196.1
        movl      %eax, %ebx                                    #2196.1
        call      mcount@PLT                                    #2196.1
        movl      -88(%ebp), %ebx                               #2196.1
                                # LOE esi
..B4.41:                        # Preds ..B4.1
        movl      20(%ebx), %eax                                #2233.5
        movl      12(%ebx), %ecx                                #2236.30
        lea       (%esi,%eax), %edx                             #2233.5
        movl      %esi, %edi                                    #2236.49
        shrl      $2, %edi                                      #2236.49
        lea       3(%esi,%eax), %eax                            #2242.50
        movl      %edi, -4(%ebp)                                #2236.49
        lea       (%ecx,%edi), %edi                             #2236.30
        movl      %edi, -8(%ebp)                                #2236.30
        shrl      $2, %eax                                      #2242.57
        movl      8(%ebx), %edi                                 #2239.12
        subl      4(%edi), %edx                                 #2239.12
        movl      -8(%ebp), %edi                                #2245.14
        addl      %ecx, %eax                                    #2242.30
        testl     $15, %edi                                     #2245.14
        movl      -4(%ebp), %ecx                                #2245.14
        jne       ..B4.3        # Prob 50%                      #2245.14
                                # LOE eax edx ecx esi edi cl ch
..B4.2:                         # Preds ..B4.41
        movdqa    (%edi), %xmm6                                 #2245.14
        jmp       ..B4.4        # Prob 100%                     #2245.14
                                # LOE eax edx ecx esi cl ch xmm6
..B4.3:                         # Preds ..B4.41
        movdqu    (%edi), %xmm6                                 #2245.14
                                # LOE eax edx ecx esi cl ch xmm6
..B4.4:                         # Preds ..B4.2 ..B4.3
        movl      12(%ebx), %edi                                #2246.5
        lea       16(%edi,%ecx), %ecx                           #2246.5
        movl      8(%ebx), %edi                                 #2256.5
        movdqa    16(%edi), %xmm0                               #2256.5
        movdqa    32(%edi), %xmm5                               #2256.5
        movdqa    48(%edi), %xmm4                               #2256.5
        movdqa    64(%edi), %xmm3                               #2256.5
        movdqa    80(%edi), %xmm2                               #2256.5
        movdqa    96(%edi), %xmm1                               #2256.5
        movdqa    128(%edi), %xmm7                              #2256.5
        movdqa    %xmm0, -40(%ebp)                              #2256.5
        movdqa    112(%edi), %xmm0                              #2256.5
        movdqa    %xmm7, -56(%ebp)                              #2256.5
        movl      %esi, %edi                                    #2268.20
        andl      $3, %edi                                      #2268.20
        je        ..B4.8        # Prob 20%                      #2268.20
                                # LOE eax edx ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.5:                         # Preds ..B4.4
        cmpl      $1, %edi                                      #2268.20
        je        ..B4.11       # Prob 25%                      #2268.20
                                # LOE eax edx ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.6:                         # Preds ..B4.5
        cmpl      $2, %edi                                      #2268.20
        je        ..B4.10       # Prob 33%                      #2268.20
                                # LOE eax edx ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.7:                         # Preds ..B4.6
        cmpl      $3, %edi                                      #2268.20
        je        ..B4.9        # Prob 50%                      #2268.20
                                # LOE eax edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.8:                         # Preds ..B4.4 ..B4.7
        movl      %eax, -20(%ebp)                               #
        jmp       ..B4.30       # Prob 100%                     #
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.9:                         # Preds ..B4.7
        movl      %eax, -20(%ebp)                               #
        xorl      %edi, %edi                                    #
        movl      %edi, -60(%ebp)                               #
        movl      %edi, -24(%ebp)                               #
        movl      %edi, -4(%ebp)                                #
        jmp       ..B4.14       # Prob 100%                     #
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.10:                        # Preds ..B4.6
        movl      %eax, -20(%ebp)                               #
        xorl      %edi, %edi                                    #
        movl      %edi, -24(%ebp)                               #
        movl      %edi, -4(%ebp)                                #
        jmp       ..B4.13       # Prob 100%                     #
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.11:                        # Preds ..B4.5
        movl      %eax, -20(%ebp)                               #
        xorl      %edi, %edi                                    #
        movl      %edi, -4(%ebp)                                #
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.12:                        # Preds ..B4.11 ..B4.30
        movdqa    %xmm6, %xmm7                                  #2291.22
        pand      %xmm3, %xmm7                                  #2291.22
        pcmpeqb   %xmm4, %xmm7                                  #2292.22
        pmovmskb  %xmm7, %eax                                   #2293.22
        movl      %eax, -24(%ebp)                               #2293.22
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.13:                        # Preds ..B4.10 ..B4.12
        movdqa    %xmm6, %xmm7                                  #2297.22
        pand      %xmm1, %xmm7                                  #2297.22
        pcmpeqb   %xmm2, %xmm7                                  #2298.22
        pmovmskb  %xmm7, %eax                                   #2299.22
        movl      %eax, -60(%ebp)                               #2299.22
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.14:                        # Preds ..B4.9 ..B4.13
        pand      -56(%ebp), %xmm6                              #2303.22
        movl      -4(%ebp), %edi                                #2313.29
        pcmpeqb   %xmm0, %xmm6                                  #2304.22
        pmovmskb  %xmm6, %eax                                   #2305.22
        movl      %eax, -8(%ebp)                                #2305.22
        andl      $-4, %esi                                     #2310.17
        orl       -24(%ebp), %edi                               #2313.29
        orl       -60(%ebp), %edi                               #2313.34
        orl       %eax, %edi                                    #2313.39
        je        ..B4.23       # Prob 78%                      #2313.47
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B4.15:                        # Preds ..B4.14
        movl      -4(%ebp), %edi                                #2331.30
        movl      %esi, -68(%ebp)                               #
        movl      %edi, %eax                                    #2331.30
        movl      %ecx, -64(%ebp)                               #
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2331.30
        movswl    %ax, %ecx                                     #2331.30
        movl      -24(%ebp), %eax                               #2332.30
        movl      %ecx, -84(%ebp)                               #2331.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2332.30
        movswl    %ax, %ecx                                     #2332.30
        movl      -60(%ebp), %eax                               #2333.30
        lea       1(,%ecx,4), %ecx                              #2340.40
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2333.30
        movswl    %ax, %esi                                     #2333.30
        movl      -8(%ebp), %eax                                #2334.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2334.30
        movl      %ecx, -80(%ebp)                               #2340.40
        movswl    %ax, %eax                                     #2334.30
        lea       2(,%esi,4), %ecx                              #2341.40
        movl      -68(%ebp), %esi                               #2346.32
        movl      %ecx, -76(%ebp)                               #2341.40
        lea       3(,%eax,4), %ecx                              #2342.40
        movl      %ecx, -72(%ebp)                               #2342.40
        movl      -64(%ebp), %ecx                               #2346.32
        testl     %edi, %edi                                    #2346.32
        je        ..B4.17       # Prob 50%                      #2346.32
                                # LOE edx ecx esi cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B4.16:                        # Preds ..B4.15
        movl      -84(%ebp), %eax                               #2346.43
        lea       (%esi,%eax,4), %edi                           #2346.43
        cmpl      %edi, %edx                                    #2346.49
        jae       ..B4.34       # Prob 1%                       #2346.49
                                # LOE edx ecx esi cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B4.17:                        # Preds ..B4.16 ..B4.15
        movl      -24(%ebp), %eax                               #2347.32
        testl     %eax, %eax                                    #2347.32
        je        ..B4.19       # Prob 50%                      #2347.32
                                # LOE edx ecx esi cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B4.18:                        # Preds ..B4.17
        movl      -80(%ebp), %eax                               #2347.43
        addl      %esi, %eax                                    #2347.43
        cmpl      %eax, %edx                                    #2347.49
        jae       ..B4.35       # Prob 1%                       #2347.49
                                # LOE edx ecx esi cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B4.19:                        # Preds ..B4.18 ..B4.17
        movl      -60(%ebp), %eax                               #2348.32
        testl     %eax, %eax                                    #2348.32
        je        ..B4.21       # Prob 50%                      #2348.32
                                # LOE edx ecx esi cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B4.20:                        # Preds ..B4.19
        movl      -76(%ebp), %eax                               #2348.43
        addl      %esi, %eax                                    #2348.43
        cmpl      %eax, %edx                                    #2348.49
        jae       ..B4.35       # Prob 1%                       #2348.49
                                # LOE edx ecx esi cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B4.21:                        # Preds ..B4.20 ..B4.19
        movl      -8(%ebp), %eax                                #2349.32
        testl     %eax, %eax                                    #2349.32
        je        ..B4.23       # Prob 50%                      #2349.32
                                # LOE edx ecx esi cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B4.22:                        # Preds ..B4.21
        movl      -72(%ebp), %eax                               #2349.43
        addl      %esi, %eax                                    #2349.43
        cmpl      %eax, %edx                                    #2349.49
        jae       ..B4.35       # Prob 1%                       #2349.49
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B4.23:                        # Preds ..B4.22 ..B4.21 ..B4.14
        lea       4(%esi), %eax                                 #2386.13
        cmpl      %eax, %edx                                    #2357.28
        jb        ..B4.32       # Prob 1%                       #2357.28
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B4.24:                        # Preds ..B4.23
        movl      -20(%ebp), %eax                               #2382.25
        cmpl      %eax, %ecx                                    #2382.25
        jae       ..B4.32       # Prob 1%                       #2382.25
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B4.25:                        # Preds ..B4.24
        addl      $64, %esi                                     #2386.13
        cmpl      %edx, %esi                                    #2387.24
        ja        ..B4.32       # Prob 1%                       #2387.24
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B4.26:                        # Preds ..B4.25
        testb     $15, %cl                                      #2392.22
        jne       ..B4.28       # Prob 50%                      #2392.22
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B4.27:                        # Preds ..B4.26
        movdqa    (%ecx), %xmm6                                 #2392.22
        jmp       ..B4.29       # Prob 100%                     #2392.22
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.28:                        # Preds ..B4.26
        movdqu    (%ecx), %xmm6                                 #2392.22
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.29:                        # Preds ..B4.27 ..B4.28
        addl      $16, %ecx                                     #2470.13
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.30:                        # Preds ..B4.29 ..B4.8
        movdqa    %xmm6, %xmm7                                  #2285.22
        pand      %xmm5, %xmm7                                  #2285.22
        pcmpeqb   -40(%ebp), %xmm7                              #2286.22
        pmovmskb  %xmm7, %eax                                   #2287.22
        movl      %eax, -4(%ebp)                                #2287.22
        jmp       ..B4.12       # Prob 100%                     #2287.22
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.32:                        # Preds ..B4.23 ..B4.24 ..B4.25 # Infreq
        movl      -12(%ebp), %esi                               #2483.12
        movl      -16(%ebp), %edi                               #2483.12
        xorl      %eax, %eax                                    #2483.12
        movl      %ebp, %esp                                    #2483.12
        popl      %ebp                                          #2483.12
        movl      %ebx, %esp                                    #2483.12
        popl      %ebx                                          #2483.12
        ret                                                     #2483.12
                                # LOE
..B4.34:                        # Preds ..B4.16                 # Infreq
        movl      -12(%ebp), %esi                               #2346.63
        movl      -16(%ebp), %edi                               #2346.63
        movl      $1, %eax                                      #2346.63
        movl      %ebp, %esp                                    #2346.63
        popl      %ebp                                          #2346.63
        movl      %ebx, %esp                                    #2346.63
        popl      %ebx                                          #2346.63
        ret                                                     #2346.63
                                # LOE
..B4.35:                        # Preds ..B4.22 ..B4.20 ..B4.18 # Infreq
        movl      -12(%ebp), %esi                               #2347.63
        movl      -16(%ebp), %edi                               #2347.63
        movl      $1, %eax                                      #2347.63
        movl      %ebp, %esp                                    #2347.63
        popl      %ebp                                          #2347.63
        movl      %ebx, %esp                                    #2347.63
        popl      %ebx                                          #2347.63
        ret                                                     #2347.63
        .align    2,0x90
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
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B5.1:                         # Preds ..B5.0
        pushl     %ebx                                          #2489.1
        movl      %esp, %ebx                                    #2489.1
        andl      $-16, %esp                                    #2489.1
        pushl     %ebp                                          #2489.1
        pushl     %ebp                                          #2489.1
        movl      4(%ebx), %ebp                                 #2489.1
        movl      %ebp, 4(%esp)                                 #2489.1
        movl      %esp, %ebp                                    #2489.1
        subl      $104, %esp                                    #2489.1
        movl      %ebx, -104(%ebp)                              #2489.1
        movl      %edi, -28(%ebp)                               #2489.1
        movl      %esi, -24(%ebp)                               #2489.1
        movl      16(%ebx), %eax                                #2487.5
        movl      %eax, -12(%ebp)                               #2487.5
        call      ..L5          # Prob 100%                     #2489.1
..L5:                                                           #
        popl      %ecx                                          #2489.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L5], %ecx       #2489.1
        lea       _gprof_pack4@GOTOFF(%ecx), %edx               #2489.1
        movl      %ecx, %ebx                                    #2489.1
        call      mcount@PLT                                    #2489.1
        movl      -104(%ebp), %ebx                              #2489.1
                                # LOE
..B5.51:                        # Preds ..B5.1
        movl      20(%ebx), %ecx                                #2526.5
        movl      -12(%ebp), %edx                               #2526.5
        movl      12(%ebx), %esi                                #2529.30
        lea       (%edx,%ecx), %eax                             #2526.5
        movl      %edx, %edi                                    #2529.49
        shrl      $2, %edi                                      #2529.49
        movl      %edi, -4(%ebp)                                #2529.49
        lea       (%esi,%edi), %edi                             #2529.30
        movl      %edi, -8(%ebp)                                #2529.30
        movl      8(%ebx), %edi                                 #2532.12
        subl      4(%edi), %eax                                 #2532.12
        movl      %eax, -16(%ebp)                               #2532.12
        lea       3(%edx,%ecx), %eax                            #2535.50
        movl      -8(%ebp), %edx                                #2538.14
        movl      -4(%ebp), %ecx                                #2538.14
        shrl      $2, %eax                                      #2535.57
        addl      %esi, %eax                                    #2535.30
        testb     $15, %dl                                      #2538.14
        jne       ..B5.3        # Prob 50%                      #2538.14
                                # LOE eax edx ecx dl cl dh ch
..B5.2:                         # Preds ..B5.51
        movdqa    (%edx), %xmm0                                 #2538.14
        movdqa    %xmm0, -88(%ebp)                              #2538.14
        jmp       ..B5.4        # Prob 100%                     #2538.14
                                # LOE eax ecx cl ch
..B5.3:                         # Preds ..B5.51
        movdqu    (%edx), %xmm0                                 #2538.14
        movdqa    %xmm0, -88(%ebp)                              #2538.14
                                # LOE eax ecx cl ch
..B5.4:                         # Preds ..B5.2 ..B5.3
        movl      12(%ebx), %edx                                #2539.5
        lea       16(%edx,%ecx), %esi                           #2539.5
        movl      %esi, -20(%ebp)                               #2539.5
        cmpl      %eax, %esi                                    #2544.16
        jae       ..B5.6        # Prob 12%                      #2544.16
                                # LOE eax esi
..B5.5:                         # Preds ..B5.4
        movzbl    -1(%esi), %ecx                                #2545.24
        shll      $8, %ecx                                      #2545.38
        jmp       ..B5.7        # Prob 100%                     #2545.38
                                # LOE eax ecx esi
..B5.6:                         # Preds ..B5.4
        xorl      %ecx, %ecx                                    #
                                # LOE eax ecx esi
..B5.7:                         # Preds ..B5.5 ..B5.6
        movl      8(%ebx), %edx                                 #2549.5
        movl      -12(%ebp), %edi                               #2561.20
        movdqa    16(%edx), %xmm7                               #2549.5
        movdqa    32(%edx), %xmm6                               #2549.5
        movdqa    48(%edx), %xmm5                               #2549.5
        movdqa    64(%edx), %xmm4                               #2549.5
        movdqa    80(%edx), %xmm3                               #2549.5
        movdqa    96(%edx), %xmm2                               #2549.5
        movdqa    112(%edx), %xmm0                              #2549.5
        movdqa    128(%edx), %xmm1                              #2549.5
        andl      $3, %edi                                      #2561.20
        je        ..B5.43       # Prob 20%                      #2561.20
                                # LOE eax ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B5.8:                         # Preds ..B5.7
        cmpl      $1, %edi                                      #2561.20
        je        ..B5.14       # Prob 25%                      #2561.20
                                # LOE eax ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B5.9:                         # Preds ..B5.8
        cmpl      $2, %edi                                      #2561.20
        je        ..B5.13       # Prob 33%                      #2561.20
                                # LOE eax ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B5.10:                        # Preds ..B5.9
        cmpl      $3, %edi                                      #2561.20
        je        ..B5.12       # Prob 50%                      #2561.20
                                # LOE eax ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B5.11:                        # Preds ..B5.10
        movdqa    %xmm0, -56(%ebp)                              #
        movdqa    -88(%ebp), %xmm0                              #
        movdqa    %xmm7, -72(%ebp)                              #
        movl      %eax, -32(%ebp)                               #
        jmp       ..B5.35       # Prob 100%                     #
                                # LOE ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.12:                        # Preds ..B5.10
        movdqa    %xmm0, -56(%ebp)                              #
        movdqa    -88(%ebp), %xmm0                              #
        movdqa    %xmm7, -72(%ebp)                              #
        movl      %eax, -32(%ebp)                               #
        xorl      %edx, %edx                                    #
        movl      %edx, -40(%ebp)                               #
        movl      %edx, -36(%ebp)                               #
        movl      %edx, -8(%ebp)                                #
        movl      $2, %edx                                      #
        jmp       ..B5.17       # Prob 100%                     #
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.13:                        # Preds ..B5.9
        movdqa    %xmm0, -56(%ebp)                              #
        movdqa    -88(%ebp), %xmm0                              #
        movdqa    %xmm7, -72(%ebp)                              #
        movl      %eax, -32(%ebp)                               #
        xorl      %edx, %edx                                    #
        movl      %edx, -36(%ebp)                               #
        movl      %edx, -8(%ebp)                                #
        movl      $2, %edx                                      #
        jmp       ..B5.16       # Prob 100%                     #
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.14:                        # Preds ..B5.8
        movdqa    %xmm0, -56(%ebp)                              #
        movdqa    -88(%ebp), %xmm0                              #
        movdqa    %xmm7, -72(%ebp)                              #
        movl      %eax, -32(%ebp)                               #
        xorl      %edi, %edi                                    #
        movl      %edi, -8(%ebp)                                #
        movl      $2, %edx                                      #
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.15:                        # Preds ..B5.14 ..B5.39
        movdqa    %xmm0, %xmm7                                  #2584.22
        pand      %xmm4, %xmm7                                  #2584.22
        pcmpeqw   %xmm5, %xmm7                                  #2585.22
        pmovmskb  %xmm7, %eax                                   #2586.22
        movl      %eax, -36(%ebp)                               #2586.22
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.16:                        # Preds ..B5.13 ..B5.15
        movdqa    %xmm0, %xmm7                                  #2590.22
        pand      %xmm2, %xmm7                                  #2590.22
        pcmpeqw   %xmm3, %xmm7                                  #2591.22
        pmovmskb  %xmm7, %eax                                   #2592.22
        movl      %eax, -40(%ebp)                               #2592.22
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.17:                        # Preds ..B5.12 ..B5.16
        movdqa    %xmm0, %xmm7                                  #2596.22
        movl      -12(%ebp), %edi                               #2603.17
        pand      %xmm1, %xmm7                                  #2596.22
        andl      $-4, %edi                                     #2603.17
        pcmpeqw   -56(%ebp), %xmm7                              #2597.22
        pmovmskb  %xmm7, %eax                                   #2598.22
        movl      %edi, -4(%ebp)                                #2603.17
        movl      -8(%ebp), %edi                                #2606.29
        movl      %eax, -76(%ebp)                               #2598.22
        orl       -36(%ebp), %edi                               #2606.29
        orl       -40(%ebp), %edi                               #2606.34
        orl       %eax, %edi                                    #2606.39
        je        ..B5.26       # Prob 78%                      #2606.47
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.18:                        # Preds ..B5.17
        movl      -8(%ebp), %edi                                #2624.30
        movl      %edx, -88(%ebp)                               #
        movl      %edi, %eax                                    #2624.30
        movl      %esi, -84(%ebp)                               #
        movl      %ecx, -80(%ebp)                               #
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2624.30
        movswl    %ax, %edx                                     #2624.30
        movl      -36(%ebp), %eax                               #2625.30
        movl      %edx, -12(%ebp)                               #2624.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2625.30
        movswl    %ax, %esi                                     #2625.30
        movl      -40(%ebp), %eax                               #2626.30
        lea       1(,%esi,4), %esi                              #2633.40
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2626.30
        movswl    %ax, %ecx                                     #2626.30
        lea       2(,%ecx,4), %ecx                              #2634.40
        movl      -76(%ebp), %eax                               #2627.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2627.30
        movl      %esi, -100(%ebp)                              #2633.40
        movl      -84(%ebp), %esi                               #2639.32
        movl      %ecx, -96(%ebp)                               #2634.40
        movl      -80(%ebp), %ecx                               #2639.32
        movswl    %ax, %edx                                     #2627.30
        lea       3(,%edx,4), %edx                              #2635.40
        movl      %edx, -92(%ebp)                               #2635.40
        movl      -88(%ebp), %edx                               #2639.32
        testl     %edi, %edi                                    #2639.32
        je        ..B5.20       # Prob 50%                      #2639.32
                                # LOE edx ecx esi dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.19:                        # Preds ..B5.18
        movl      -12(%ebp), %edi                               #2639.43
        movl      -4(%ebp), %eax                                #2639.43
        lea       (%eax,%edi,4), %edi                           #2639.43
        movl      -16(%ebp), %eax                               #2639.49
        cmpl      %edi, %eax                                    #2639.49
        jae       ..B5.44       # Prob 1%                       #2639.49
                                # LOE edx ecx esi dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.20:                        # Preds ..B5.19 ..B5.18
        movl      -36(%ebp), %eax                               #2640.32
        testl     %eax, %eax                                    #2640.32
        je        ..B5.22       # Prob 50%                      #2640.32
                                # LOE edx ecx esi dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.21:                        # Preds ..B5.20
        movl      -100(%ebp), %edi                              #2640.43
        addl      -4(%ebp), %edi                                #2640.43
        movl      -16(%ebp), %eax                               #2640.49
        cmpl      %edi, %eax                                    #2640.49
        jae       ..B5.44       # Prob 1%                       #2640.49
                                # LOE edx ecx esi dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.22:                        # Preds ..B5.21 ..B5.20
        movl      -40(%ebp), %eax                               #2641.32
        testl     %eax, %eax                                    #2641.32
        je        ..B5.24       # Prob 50%                      #2641.32
                                # LOE edx ecx esi dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.23:                        # Preds ..B5.22
        movl      -96(%ebp), %edi                               #2641.43
        addl      -4(%ebp), %edi                                #2641.43
        movl      -16(%ebp), %eax                               #2641.49
        cmpl      %edi, %eax                                    #2641.49
        jae       ..B5.44       # Prob 1%                       #2641.49
                                # LOE edx ecx esi dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.24:                        # Preds ..B5.23 ..B5.22
        movl      -76(%ebp), %eax                               #2642.32
        testl     %eax, %eax                                    #2642.32
        je        ..B5.26       # Prob 50%                      #2642.32
                                # LOE edx ecx esi dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.25:                        # Preds ..B5.24
        movl      -92(%ebp), %edi                               #2642.43
        addl      -4(%ebp), %edi                                #2642.43
        movl      -16(%ebp), %eax                               #2642.49
        cmpl      %edi, %eax                                    #2642.49
        jae       ..B5.44       # Prob 1%                       #2642.49
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.26:                        # Preds ..B5.25 ..B5.24 ..B5.17
        movl      -4(%ebp), %eax                                #2647.17
        lea       4(%eax), %edi                                 #2647.17
        movl      -16(%ebp), %eax                               #2650.28
        movl      %edi, -12(%ebp)                               #2647.17
        cmpl      %eax, %edi                                    #2650.28
        ja        ..B5.41       # Prob 1%                       #2650.28
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.27:                        # Preds ..B5.26
        addl      $-1, %edx                                     #2654.25
        jne       ..B5.36       # Prob 50%                      #2654.39
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.28:                        # Preds ..B5.27
        movl      -32(%ebp), %eax                               #2675.25
        cmpl      %eax, %esi                                    #2675.25
        jae       ..B5.41       # Prob 1%                       #2675.25
                                # LOE ecx esi xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.29:                        # Preds ..B5.28
        movl      -4(%ebp), %eax                                #2679.13
        movl      -16(%ebp), %edi                               #2680.24
        lea       60(%eax), %edx                                #2679.13
        movl      %edx, -12(%ebp)                               #2679.13
        cmpl      %edi, %edx                                    #2680.24
        ja        ..B5.41       # Prob 1%                       #2680.24
                                # LOE ecx esi xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.30:                        # Preds ..B5.29
        testl     $15, %esi                                     #2685.22
        jne       ..B5.32       # Prob 50%                      #2685.22
                                # LOE ecx esi xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.31:                        # Preds ..B5.30
        movdqa    (%esi), %xmm0                                 #2685.22
        jmp       ..B5.33       # Prob 100%                     #2685.22
                                # LOE ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.32:                        # Preds ..B5.30
        movdqu    (%esi), %xmm0                                 #2685.22
                                # LOE ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.33:                        # Preds ..B5.31 ..B5.32
        movl      -32(%ebp), %eax                               #2766.24
        addl      $16, %esi                                     #2763.13
        movl      %esi, -20(%ebp)                               #2763.13
        cmpl      %eax, %esi                                    #2766.24
        jae       ..B5.35       # Prob 12%                      #2766.24
                                # LOE ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.34:                        # Preds ..B5.33
        movzbl    -1(%esi), %ecx                                #2767.32
        shll      $8, %ecx                                      #2767.46
                                # LOE ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.35:                        # Preds ..B5.33 ..B5.11 ..B5.34
        movl      $2, %edx                                      #2568.13
        jmp       ..B5.39       # Prob 100%                     #2568.13
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.36:                        # Preds ..B5.27
        movl      -20(%ebp), %eax                               #2662.26
        movl      -32(%ebp), %edi                               #2662.26
        psrldq    $1, %xmm0                                     #2659.26
        cmpl      %edi, %eax                                    #2662.26
        jae       ..B5.38       # Prob 19%                      #2662.26
                                # LOE eax edx ecx esi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.37:                        # Preds ..B5.36
        movzbl    (%eax), %edi                                  #2665.37
        sarl      $8, %ecx                                      #2664.21
        shll      $8, %edi                                      #2665.48
        orl       %edi, %ecx                                    #2665.21
        pinsrw    $7, %ecx, %xmm0                               #2666.30
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.38:                        # Preds ..B5.37 ..B5.36
        addl      $1, -20(%ebp)                                 #2670.20
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.39:                        # Preds ..B5.35 ..B5.38 ..B5.43
        movdqa    %xmm0, %xmm7                                  #2578.22
        pand      %xmm6, %xmm7                                  #2578.22
        pcmpeqw   -72(%ebp), %xmm7                              #2579.22
        pmovmskb  %xmm7, %eax                                   #2580.22
        movl      %eax, -8(%ebp)                                #2580.22
        jmp       ..B5.15       # Prob 100%                     #2580.22
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.41:                        # Preds ..B5.26 ..B5.28 ..B5.29 # Infreq
        movl      -24(%ebp), %esi                               #2776.12
        movl      -28(%ebp), %edi                               #2776.12
        xorl      %eax, %eax                                    #2776.12
        movl      %ebp, %esp                                    #2776.12
        popl      %ebp                                          #2776.12
        movl      %ebx, %esp                                    #2776.12
        popl      %ebx                                          #2776.12
        ret                                                     #2776.12
                                # LOE
..B5.43:                        # Preds ..B5.7                  # Infreq
        movdqa    %xmm0, -56(%ebp)                              #
        movdqa    -88(%ebp), %xmm0                              #
        movdqa    %xmm7, -72(%ebp)                              #
        movl      %eax, -32(%ebp)                               #
        movl      $2, %edx                                      #
        jmp       ..B5.39       # Prob 100%                     #
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.44:                        # Preds ..B5.21 ..B5.19 ..B5.25 ..B5.23 # Infreq
        movl      -24(%ebp), %esi                               #2641.63
        movl      -28(%ebp), %edi                               #2641.63
        movl      $1, %eax                                      #2641.63
        movl      %ebp, %esp                                    #2641.63
        popl      %ebp                                          #2641.63
        movl      %ebx, %esp                                    #2641.63
        popl      %ebx                                          #2641.63
        ret                                                     #2641.63
        .align    2,0x90
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
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B6.1:                         # Preds ..B6.0
        pushl     %ebx                                          #2782.1
        movl      %esp, %ebx                                    #2782.1
        andl      $-16, %esp                                    #2782.1
        pushl     %ebp                                          #2782.1
        pushl     %ebp                                          #2782.1
        movl      4(%ebx), %ebp                                 #2782.1
        movl      %ebp, 4(%esp)                                 #2782.1
        movl      %esp, %ebp                                    #2782.1
        subl      $104, %esp                                    #2782.1
        movl      %ebx, -104(%ebp)                              #2782.1
        movl      %edi, -28(%ebp)                               #2782.1
        movl      %esi, -24(%ebp)                               #2782.1
        movl      16(%ebx), %eax                                #2780.5
        movl      %eax, -12(%ebp)                               #2780.5
        call      ..L6          # Prob 100%                     #2782.1
..L6:                                                           #
        popl      %ecx                                          #2782.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L6], %ecx       #2782.1
        lea       _gprof_pack5@GOTOFF(%ecx), %edx               #2782.1
        movl      %ecx, %ebx                                    #2782.1
        call      mcount@PLT                                    #2782.1
        movl      -104(%ebp), %ebx                              #2782.1
                                # LOE
..B6.51:                        # Preds ..B6.1
        movl      20(%ebx), %ecx                                #2819.5
        movl      -12(%ebp), %edx                               #2819.5
        movl      12(%ebx), %esi                                #2822.30
        lea       (%edx,%ecx), %eax                             #2819.5
        movl      %edx, %edi                                    #2822.49
        shrl      $2, %edi                                      #2822.49
        movl      %edi, -4(%ebp)                                #2822.49
        lea       (%esi,%edi), %edi                             #2822.30
        movl      %edi, -8(%ebp)                                #2822.30
        movl      8(%ebx), %edi                                 #2825.12
        subl      4(%edi), %eax                                 #2825.12
        movl      %eax, -16(%ebp)                               #2825.12
        lea       3(%edx,%ecx), %eax                            #2828.50
        movl      -8(%ebp), %edx                                #2831.14
        movl      -4(%ebp), %ecx                                #2831.14
        shrl      $2, %eax                                      #2828.57
        addl      %esi, %eax                                    #2828.30
        testb     $15, %dl                                      #2831.14
        jne       ..B6.3        # Prob 50%                      #2831.14
                                # LOE eax edx ecx dl cl dh ch
..B6.2:                         # Preds ..B6.51
        movdqa    (%edx), %xmm0                                 #2831.14
        movdqa    %xmm0, -88(%ebp)                              #2831.14
        jmp       ..B6.4        # Prob 100%                     #2831.14
                                # LOE eax ecx cl ch
..B6.3:                         # Preds ..B6.51
        movdqu    (%edx), %xmm0                                 #2831.14
        movdqa    %xmm0, -88(%ebp)                              #2831.14
                                # LOE eax ecx cl ch
..B6.4:                         # Preds ..B6.2 ..B6.3
        movl      12(%ebx), %edx                                #2832.5
        lea       16(%edx,%ecx), %esi                           #2832.5
        movl      %esi, -20(%ebp)                               #2832.5
        cmpl      %eax, %esi                                    #2837.16
        jae       ..B6.6        # Prob 12%                      #2837.16
                                # LOE eax esi
..B6.5:                         # Preds ..B6.4
        movzbl    -1(%esi), %ecx                                #2838.24
        shll      $8, %ecx                                      #2838.38
        jmp       ..B6.7        # Prob 100%                     #2838.38
                                # LOE eax ecx esi
..B6.6:                         # Preds ..B6.4
        xorl      %ecx, %ecx                                    #
                                # LOE eax ecx esi
..B6.7:                         # Preds ..B6.5 ..B6.6
        movl      8(%ebx), %edx                                 #2842.5
        movl      -12(%ebp), %edi                               #2854.20
        movdqa    16(%edx), %xmm7                               #2842.5
        movdqa    32(%edx), %xmm6                               #2842.5
        movdqa    48(%edx), %xmm5                               #2842.5
        movdqa    64(%edx), %xmm4                               #2842.5
        movdqa    80(%edx), %xmm3                               #2842.5
        movdqa    96(%edx), %xmm2                               #2842.5
        movdqa    112(%edx), %xmm0                              #2842.5
        movdqa    128(%edx), %xmm1                              #2842.5
        andl      $3, %edi                                      #2854.20
        je        ..B6.43       # Prob 20%                      #2854.20
                                # LOE eax ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B6.8:                         # Preds ..B6.7
        cmpl      $1, %edi                                      #2854.20
        je        ..B6.14       # Prob 25%                      #2854.20
                                # LOE eax ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B6.9:                         # Preds ..B6.8
        cmpl      $2, %edi                                      #2854.20
        je        ..B6.13       # Prob 33%                      #2854.20
                                # LOE eax ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B6.10:                        # Preds ..B6.9
        cmpl      $3, %edi                                      #2854.20
        je        ..B6.12       # Prob 50%                      #2854.20
                                # LOE eax ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B6.11:                        # Preds ..B6.10
        movdqa    %xmm0, -56(%ebp)                              #
        movdqa    -88(%ebp), %xmm0                              #
        movdqa    %xmm7, -72(%ebp)                              #
        movl      %eax, -32(%ebp)                               #
        jmp       ..B6.35       # Prob 100%                     #
                                # LOE ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.12:                        # Preds ..B6.10
        movdqa    %xmm0, -56(%ebp)                              #
        movdqa    -88(%ebp), %xmm0                              #
        movdqa    %xmm7, -72(%ebp)                              #
        movl      %eax, -32(%ebp)                               #
        xorl      %edx, %edx                                    #
        movl      %edx, -40(%ebp)                               #
        movl      %edx, -36(%ebp)                               #
        movl      %edx, -8(%ebp)                                #
        movl      $4, %edx                                      #
        jmp       ..B6.17       # Prob 100%                     #
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.13:                        # Preds ..B6.9
        movdqa    %xmm0, -56(%ebp)                              #
        movdqa    -88(%ebp), %xmm0                              #
        movdqa    %xmm7, -72(%ebp)                              #
        movl      %eax, -32(%ebp)                               #
        xorl      %edx, %edx                                    #
        movl      %edx, -36(%ebp)                               #
        movl      %edx, -8(%ebp)                                #
        movl      $4, %edx                                      #
        jmp       ..B6.16       # Prob 100%                     #
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.14:                        # Preds ..B6.8
        movdqa    %xmm0, -56(%ebp)                              #
        movdqa    -88(%ebp), %xmm0                              #
        movdqa    %xmm7, -72(%ebp)                              #
        movl      %eax, -32(%ebp)                               #
        xorl      %edi, %edi                                    #
        movl      %edi, -8(%ebp)                                #
        movl      $4, %edx                                      #
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.15:                        # Preds ..B6.14 ..B6.39
        movdqa    %xmm0, %xmm7                                  #2877.22
        pand      %xmm4, %xmm7                                  #2877.22
        pcmpeqd   %xmm5, %xmm7                                  #2878.22
        pmovmskb  %xmm7, %eax                                   #2879.22
        movl      %eax, -36(%ebp)                               #2879.22
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.16:                        # Preds ..B6.13 ..B6.15
        movdqa    %xmm0, %xmm7                                  #2883.22
        pand      %xmm2, %xmm7                                  #2883.22
        pcmpeqd   %xmm3, %xmm7                                  #2884.22
        pmovmskb  %xmm7, %eax                                   #2885.22
        movl      %eax, -40(%ebp)                               #2885.22
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.17:                        # Preds ..B6.12 ..B6.16
        movdqa    %xmm0, %xmm7                                  #2889.22
        movl      -12(%ebp), %edi                               #2896.17
        pand      %xmm1, %xmm7                                  #2889.22
        andl      $-4, %edi                                     #2896.17
        pcmpeqd   -56(%ebp), %xmm7                              #2890.22
        pmovmskb  %xmm7, %eax                                   #2891.22
        movl      %edi, -4(%ebp)                                #2896.17
        movl      -8(%ebp), %edi                                #2899.29
        movl      %eax, -76(%ebp)                               #2891.22
        orl       -36(%ebp), %edi                               #2899.29
        orl       -40(%ebp), %edi                               #2899.34
        orl       %eax, %edi                                    #2899.39
        je        ..B6.26       # Prob 78%                      #2899.47
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.18:                        # Preds ..B6.17
        movl      -8(%ebp), %edi                                #2917.30
        movl      %edx, -88(%ebp)                               #
        movl      %edi, %eax                                    #2917.30
        movl      %esi, -84(%ebp)                               #
        movl      %ecx, -80(%ebp)                               #
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2917.30
        movswl    %ax, %edx                                     #2917.30
        movl      -36(%ebp), %eax                               #2918.30
        movl      %edx, -12(%ebp)                               #2917.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2918.30
        movswl    %ax, %esi                                     #2918.30
        movl      -40(%ebp), %eax                               #2919.30
        lea       1(,%esi,4), %esi                              #2926.40
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2919.30
        movswl    %ax, %ecx                                     #2919.30
        lea       2(,%ecx,4), %ecx                              #2927.40
        movl      -76(%ebp), %eax                               #2920.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2920.30
        movl      %esi, -100(%ebp)                              #2926.40
        movl      -84(%ebp), %esi                               #2932.32
        movl      %ecx, -96(%ebp)                               #2927.40
        movl      -80(%ebp), %ecx                               #2932.32
        movswl    %ax, %edx                                     #2920.30
        lea       3(,%edx,4), %edx                              #2928.40
        movl      %edx, -92(%ebp)                               #2928.40
        movl      -88(%ebp), %edx                               #2932.32
        testl     %edi, %edi                                    #2932.32
        je        ..B6.20       # Prob 50%                      #2932.32
                                # LOE edx ecx esi dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.19:                        # Preds ..B6.18
        movl      -12(%ebp), %edi                               #2932.43
        movl      -4(%ebp), %eax                                #2932.43
        lea       (%eax,%edi,4), %edi                           #2932.43
        movl      -16(%ebp), %eax                               #2932.49
        cmpl      %edi, %eax                                    #2932.49
        jae       ..B6.44       # Prob 1%                       #2932.49
                                # LOE edx ecx esi dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.20:                        # Preds ..B6.19 ..B6.18
        movl      -36(%ebp), %eax                               #2933.32
        testl     %eax, %eax                                    #2933.32
        je        ..B6.22       # Prob 50%                      #2933.32
                                # LOE edx ecx esi dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.21:                        # Preds ..B6.20
        movl      -100(%ebp), %edi                              #2933.43
        addl      -4(%ebp), %edi                                #2933.43
        movl      -16(%ebp), %eax                               #2933.49
        cmpl      %edi, %eax                                    #2933.49
        jae       ..B6.44       # Prob 1%                       #2933.49
                                # LOE edx ecx esi dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.22:                        # Preds ..B6.21 ..B6.20
        movl      -40(%ebp), %eax                               #2934.32
        testl     %eax, %eax                                    #2934.32
        je        ..B6.24       # Prob 50%                      #2934.32
                                # LOE edx ecx esi dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.23:                        # Preds ..B6.22
        movl      -96(%ebp), %edi                               #2934.43
        addl      -4(%ebp), %edi                                #2934.43
        movl      -16(%ebp), %eax                               #2934.49
        cmpl      %edi, %eax                                    #2934.49
        jae       ..B6.44       # Prob 1%                       #2934.49
                                # LOE edx ecx esi dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.24:                        # Preds ..B6.23 ..B6.22
        movl      -76(%ebp), %eax                               #2935.32
        testl     %eax, %eax                                    #2935.32
        je        ..B6.26       # Prob 50%                      #2935.32
                                # LOE edx ecx esi dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.25:                        # Preds ..B6.24
        movl      -92(%ebp), %edi                               #2935.43
        addl      -4(%ebp), %edi                                #2935.43
        movl      -16(%ebp), %eax                               #2935.49
        cmpl      %edi, %eax                                    #2935.49
        jae       ..B6.44       # Prob 1%                       #2935.49
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.26:                        # Preds ..B6.25 ..B6.24 ..B6.17
        movl      -4(%ebp), %eax                                #2940.17
        lea       4(%eax), %edi                                 #2940.17
        movl      -16(%ebp), %eax                               #2943.28
        movl      %edi, -12(%ebp)                               #2940.17
        cmpl      %eax, %edi                                    #2943.28
        ja        ..B6.41       # Prob 1%                       #2943.28
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.27:                        # Preds ..B6.26
        addl      $-1, %edx                                     #2947.25
        jne       ..B6.36       # Prob 50%                      #2947.39
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.28:                        # Preds ..B6.27
        movl      -32(%ebp), %eax                               #2968.25
        cmpl      %eax, %esi                                    #2968.25
        jae       ..B6.41       # Prob 1%                       #2968.25
                                # LOE ecx esi xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.29:                        # Preds ..B6.28
        movl      -4(%ebp), %eax                                #2972.13
        movl      -16(%ebp), %edi                               #2973.24
        lea       52(%eax), %edx                                #2972.13
        movl      %edx, -12(%ebp)                               #2972.13
        cmpl      %edi, %edx                                    #2973.24
        ja        ..B6.41       # Prob 1%                       #2973.24
                                # LOE ecx esi xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.30:                        # Preds ..B6.29
        testl     $15, %esi                                     #2978.22
        jne       ..B6.32       # Prob 50%                      #2978.22
                                # LOE ecx esi xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.31:                        # Preds ..B6.30
        movdqa    (%esi), %xmm0                                 #2978.22
        jmp       ..B6.33       # Prob 100%                     #2978.22
                                # LOE ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.32:                        # Preds ..B6.30
        movdqu    (%esi), %xmm0                                 #2978.22
                                # LOE ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.33:                        # Preds ..B6.31 ..B6.32
        movl      -32(%ebp), %eax                               #3059.24
        addl      $16, %esi                                     #3056.13
        movl      %esi, -20(%ebp)                               #3056.13
        cmpl      %eax, %esi                                    #3059.24
        jae       ..B6.35       # Prob 12%                      #3059.24
                                # LOE ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.34:                        # Preds ..B6.33
        movzbl    -1(%esi), %ecx                                #3060.32
        shll      $8, %ecx                                      #3060.46
                                # LOE ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.35:                        # Preds ..B6.33 ..B6.11 ..B6.34
        movl      $4, %edx                                      #2861.13
        jmp       ..B6.39       # Prob 100%                     #2861.13
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.36:                        # Preds ..B6.27
        movl      -20(%ebp), %eax                               #2955.26
        movl      -32(%ebp), %edi                               #2955.26
        psrldq    $1, %xmm0                                     #2952.26
        cmpl      %edi, %eax                                    #2955.26
        jae       ..B6.38       # Prob 19%                      #2955.26
                                # LOE eax edx ecx esi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.37:                        # Preds ..B6.36
        movzbl    (%eax), %edi                                  #2958.37
        sarl      $8, %ecx                                      #2957.21
        shll      $8, %edi                                      #2958.48
        orl       %edi, %ecx                                    #2958.21
        pinsrw    $7, %ecx, %xmm0                               #2959.30
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.38:                        # Preds ..B6.37 ..B6.36
        addl      $1, -20(%ebp)                                 #2963.20
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.39:                        # Preds ..B6.35 ..B6.38 ..B6.43
        movdqa    %xmm0, %xmm7                                  #2871.22
        pand      %xmm6, %xmm7                                  #2871.22
        pcmpeqd   -72(%ebp), %xmm7                              #2872.22
        pmovmskb  %xmm7, %eax                                   #2873.22
        movl      %eax, -8(%ebp)                                #2873.22
        jmp       ..B6.15       # Prob 100%                     #2873.22
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.41:                        # Preds ..B6.26 ..B6.28 ..B6.29 # Infreq
        movl      -24(%ebp), %esi                               #3069.12
        movl      -28(%ebp), %edi                               #3069.12
        xorl      %eax, %eax                                    #3069.12
        movl      %ebp, %esp                                    #3069.12
        popl      %ebp                                          #3069.12
        movl      %ebx, %esp                                    #3069.12
        popl      %ebx                                          #3069.12
        ret                                                     #3069.12
                                # LOE
..B6.43:                        # Preds ..B6.7                  # Infreq
        movdqa    %xmm0, -56(%ebp)                              #
        movdqa    -88(%ebp), %xmm0                              #
        movdqa    %xmm7, -72(%ebp)                              #
        movl      %eax, -32(%ebp)                               #
        movl      $4, %edx                                      #
        jmp       ..B6.39       # Prob 100%                     #
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.44:                        # Preds ..B6.21 ..B6.19 ..B6.25 ..B6.23 # Infreq
        movl      -24(%ebp), %esi                               #2934.63
        movl      -28(%ebp), %edi                               #2934.63
        movl      $1, %eax                                      #2934.63
        movl      %ebp, %esp                                    #2934.63
        popl      %ebp                                          #2934.63
        movl      %ebx, %esp                                    #2934.63
        popl      %ebx                                          #2934.63
        ret                                                     #2934.63
        .align    2,0x90
                                # LOE
# mark_end;
	.type	eval_2na_32,@function
	.size	eval_2na_32,.-eval_2na_32
.LNeval_2na_32:
	.data
# -- End  eval_2na_32
	.text
# -- Begin  eval_2na_128
# mark_begin;
       .align    2,0x90
eval_2na_128:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B7.1:                         # Preds ..B7.0
        pushl     %ebx                                          #3369.1
        movl      %esp, %ebx                                    #3369.1
        andl      $-16, %esp                                    #3369.1
        pushl     %ebp                                          #3369.1
        pushl     %ebp                                          #3369.1
        movl      4(%ebx), %ebp                                 #3369.1
        movl      %ebp, 4(%esp)                                 #3369.1
        movl      %esp, %ebp                                    #3369.1
        subl      $88, %esp                                     #3369.1
        movl      %ebx, -88(%ebp)                               #3369.1
        movl      %edi, -24(%ebp)                               #3369.1
        movl      %esi, -20(%ebp)                               #3369.1
        movl      16(%ebx), %esi                                #3367.5
        call      ..L7          # Prob 100%                     #3369.1
..L7:                                                           #
        popl      %eax                                          #3369.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L7], %eax       #3369.1
        lea       _gprof_pack6@GOTOFF(%eax), %edx               #3369.1
        movl      %eax, %ebx                                    #3369.1
        call      mcount@PLT                                    #3369.1
        movl      -88(%ebp), %ebx                               #3369.1
                                # LOE esi
..B7.47:                        # Preds ..B7.1
        movl      20(%ebx), %ecx                                #3406.5
        movl      12(%ebx), %edx                                #3409.30
        lea       (%esi,%ecx), %eax                             #3406.5
        movl      %esi, %edi                                    #3409.49
        shrl      $2, %edi                                      #3409.49
        lea       3(%esi,%ecx), %ecx                            #3415.50
        movl      %edi, -4(%ebp)                                #3409.49
        lea       (%edx,%edi), %edi                             #3409.30
        movl      %edi, -12(%ebp)                               #3409.30
        shrl      $2, %ecx                                      #3415.57
        movl      8(%ebx), %edi                                 #3412.12
        subl      4(%edi), %eax                                 #3412.12
        addl      %edx, %ecx                                    #3415.30
        movl      -12(%ebp), %edx                               #3418.14
        movl      %ecx, -8(%ebp)                                #3415.30
        movl      -4(%ebp), %ecx                                #3418.14
        testb     $15, %dl                                      #3418.14
        jne       ..B7.3        # Prob 50%                      #3418.14
                                # LOE eax edx ecx esi dl cl dh ch
..B7.2:                         # Preds ..B7.47
        movdqa    (%edx), %xmm0                                 #3418.14
        movdqa    %xmm0, -40(%ebp)                              #3418.14
        jmp       ..B7.4        # Prob 100%                     #3418.14
                                # LOE eax ecx esi cl ch
..B7.3:                         # Preds ..B7.47
        movdqu    (%edx), %xmm0                                 #3418.14
        movdqa    %xmm0, -40(%ebp)                              #3418.14
                                # LOE eax ecx esi cl ch
..B7.4:                         # Preds ..B7.2 ..B7.3
        movl      12(%ebx), %edi                                #3419.5
        movl      -8(%ebp), %edx                                #3424.16
        lea       16(%edi,%ecx), %edi                           #3419.5
        movl      %edi, -16(%ebp)                               #3419.5
        cmpl      %edx, %edi                                    #3424.16
        jae       ..B7.6        # Prob 12%                      #3424.16
                                # LOE eax esi edi
..B7.5:                         # Preds ..B7.4
        movzbl    -1(%edi), %ecx                                #3425.24
        shll      $8, %ecx                                      #3425.38
        jmp       ..B7.7        # Prob 100%                     #3425.38
                                # LOE eax ecx esi edi
..B7.6:                         # Preds ..B7.4
        xorl      %ecx, %ecx                                    #
                                # LOE eax ecx esi edi
..B7.7:                         # Preds ..B7.5 ..B7.6
        movl      8(%ebx), %edx                                 #3429.5
        movdqa    16(%edx), %xmm6                               #3429.5
        movdqa    32(%edx), %xmm5                               #3429.5
        movdqa    48(%edx), %xmm4                               #3429.5
        movdqa    64(%edx), %xmm3                               #3429.5
        movdqa    80(%edx), %xmm2                               #3429.5
        movdqa    96(%edx), %xmm1                               #3429.5
        movdqa    112(%edx), %xmm0                              #3429.5
        movdqa    128(%edx), %xmm7                              #3429.5
        movl      %eax, %edx                                    #3434.20
        subl      %esi, %edx                                    #3434.20
        addl      $7, %edx                                      #3434.33
        shrl      $2, %edx                                      #3434.40
        movl      %edx, -12(%ebp)                               #3434.40
        movl      %esi, %edx                                    #3441.20
        andl      $3, %edx                                      #3441.20
        je        ..B7.42       # Prob 20%                      #3441.20
                                # LOE eax edx ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B7.8:                         # Preds ..B7.7
        cmpl      $1, %edx                                      #3441.20
        je        ..B7.14       # Prob 25%                      #3441.20
                                # LOE eax edx ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B7.9:                         # Preds ..B7.8
        cmpl      $2, %edx                                      #3441.20
        je        ..B7.13       # Prob 33%                      #3441.20
                                # LOE eax edx ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B7.10:                        # Preds ..B7.9
        cmpl      $3, %edx                                      #3441.20
        je        ..B7.12       # Prob 50%                      #3441.20
                                # LOE eax ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B7.11:                        # Preds ..B7.10
        movdqa    %xmm7, -72(%ebp)                              #
        movdqa    %xmm0, -56(%ebp)                              #
        movdqa    -40(%ebp), %xmm0                              #
        movl      %eax, -76(%ebp)                               #
        movl      %ecx, -4(%ebp)                                #
        jmp       ..B7.26       # Prob 100%                     #
                                # LOE esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.12:                        # Preds ..B7.10
        movdqa    %xmm7, -72(%ebp)                              #
        movdqa    %xmm0, -56(%ebp)                              #
        movdqa    -40(%ebp), %xmm0                              #
        movl      %eax, -76(%ebp)                               #
        movl      %ecx, -4(%ebp)                                #
        xorl      %edx, %edx                                    #
        movl      %edx, -84(%ebp)                               #
        movl      %edx, -80(%ebp)                               #
        jmp       ..B7.17       # Prob 100%                     #
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.13:                        # Preds ..B7.9
        movdqa    %xmm7, -72(%ebp)                              #
        movdqa    %xmm0, -56(%ebp)                              #
        movdqa    -40(%ebp), %xmm0                              #
        movl      %eax, -76(%ebp)                               #
        movl      %ecx, -4(%ebp)                                #
        xorl      %edx, %edx                                    #
        movl      %edx, -80(%ebp)                               #
        jmp       ..B7.16       # Prob 100%                     #
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.14:                        # Preds ..B7.8
        movdqa    %xmm7, -72(%ebp)                              #
        movdqa    %xmm0, -56(%ebp)                              #
        movdqa    -40(%ebp), %xmm0                              #
        movl      %eax, -76(%ebp)                               #
        movl      %ecx, -4(%ebp)                                #
        xorl      %edx, %edx                                    #
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.15:                        # Preds ..B7.14 ..B7.30
        movdqa    %xmm0, %xmm7                                  #3464.22
        pand      %xmm3, %xmm7                                  #3464.22
        pcmpeqd   %xmm4, %xmm7                                  #3465.22
        pmovmskb  %xmm7, %eax                                   #3466.22
        addl      $1, %eax                                      #3467.17
        shrl      $16, %eax                                     #3467.17
        negl      %eax                                          #3467.17
        movl      %eax, -80(%ebp)                               #3467.17
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.16:                        # Preds ..B7.13 ..B7.15
        movdqa    %xmm0, %xmm7                                  #3470.22
        pand      %xmm1, %xmm7                                  #3470.22
        pcmpeqd   %xmm2, %xmm7                                  #3471.22
        pmovmskb  %xmm7, %eax                                   #3472.22
        addl      $1, %eax                                      #3473.17
        shrl      $16, %eax                                     #3473.17
        negl      %eax                                          #3473.17
        movl      %eax, -84(%ebp)                               #3473.17
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.17:                        # Preds ..B7.12 ..B7.16
        movdqa    %xmm0, %xmm7                                  #3476.22
        pand      -72(%ebp), %xmm7                              #3476.22
        pcmpeqd   -56(%ebp), %xmm7                              #3477.22
        pmovmskb  %xmm7, %eax                                   #3478.22
        andl      $-4, %esi                                     #3483.17
        movl      %edx, %ecx                                    #3486.29
        orl       -80(%ebp), %ecx                               #3486.29
        addl      $1, %eax                                      #3479.17
        shrl      $16, %eax                                     #3479.17
        orl       -84(%ebp), %ecx                               #3486.34
        negl      %eax                                          #3479.17
        orl       %eax, %ecx                                    #3486.39
        jne       ..B7.31       # Prob 1%                       #3486.47
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.18:                        # Preds ..B7.17
        movl      -76(%ebp), %eax                               #3530.28
        addl      $4, %esi                                      #3527.17
        cmpl      %eax, %esi                                    #3530.28
        ja        ..B7.38       # Prob 1%                       #3530.28
                                # LOE esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.19:                        # Preds ..B7.18
        movl      -12(%ebp), %eax                               #3534.25
        addl      $-1, %eax                                     #3534.25
        movl      %eax, -12(%ebp)                               #3534.25
        jne       ..B7.27       # Prob 50%                      #3534.39
                                # LOE esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.20:                        # Preds ..B7.19
        movl      -8(%ebp), %eax                                #3555.25
        cmpl      %eax, %edi                                    #3555.25
        jae       ..B7.38       # Prob 1%                       #3555.25
                                # LOE eax esi edi al ah xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.21:                        # Preds ..B7.20
        testl     $15, %edi                                     #3565.22
        jne       ..B7.23       # Prob 50%                      #3565.22
                                # LOE eax esi edi al ah xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.22:                        # Preds ..B7.21
        movdqa    (%edi), %xmm0                                 #3565.22
        jmp       ..B7.24       # Prob 100%                     #3565.22
                                # LOE eax esi edi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.23:                        # Preds ..B7.21
        movdqu    (%edi), %xmm0                                 #3565.22
                                # LOE eax esi edi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.24:                        # Preds ..B7.22 ..B7.23
        addl      $16, %edi                                     #3643.13
        movl      %edi, -16(%ebp)                               #3643.13
        cmpl      %eax, %edi                                    #3646.24
        jae       ..B7.26       # Prob 12%                      #3646.24
                                # LOE esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.25:                        # Preds ..B7.24
        movzbl    -1(%edi), %eax                                #3647.32
        shll      $8, %eax                                      #3647.46
        movl      %eax, -4(%ebp)                                #3647.46
                                # LOE esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.26:                        # Preds ..B7.24 ..B7.11 ..B7.25
        movl      $16, %eax                                     #3448.13
        movl      %eax, -12(%ebp)                               #3448.13
        jmp       ..B7.30       # Prob 100%                     #3448.13
                                # LOE esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.27:                        # Preds ..B7.19
        movl      -8(%ebp), %edx                                #3542.26
        movl      -16(%ebp), %eax                               #3542.26
        psrldq    $1, %xmm0                                     #3539.26
        cmpl      %edx, %eax                                    #3542.26
        jae       ..B7.29       # Prob 19%                      #3542.26
                                # LOE eax esi edi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.28:                        # Preds ..B7.27
        movl      -4(%ebp), %ecx                                #3544.21
        movzbl    (%eax), %edx                                  #3545.37
        sarl      $8, %ecx                                      #3544.21
        shll      $8, %edx                                      #3545.48
        orl       %edx, %ecx                                    #3545.21
        pinsrw    $7, %ecx, %xmm0                               #3546.30
        movl      %ecx, -4(%ebp)                                #3545.21
                                # LOE esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.29:                        # Preds ..B7.28 ..B7.27
        addl      $1, -16(%ebp)                                 #3550.20
                                # LOE esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.30:                        # Preds ..B7.26 ..B7.29 ..B7.42
        movdqa    %xmm0, %xmm7                                  #3458.22
        pand      %xmm5, %xmm7                                  #3458.22
        pcmpeqd   %xmm6, %xmm7                                  #3459.22
        pmovmskb  %xmm7, %edx                                   #3460.22
        addl      $1, %edx                                      #3461.17
        shrl      $16, %edx                                     #3461.17
        negl      %edx                                          #3461.17
        jmp       ..B7.15       # Prob 100%                     #3461.17
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.31:                        # Preds ..B7.17                 # Infreq
        movl      -76(%ebp), %eax                               #
        subl      %esi, %eax                                    #3489.30
        cmpl      $2, %eax                                      #3489.37
        je        ..B7.35       # Prob 25%                      #3489.37
                                # LOE eax edx
..B7.32:                        # Preds ..B7.31                 # Infreq
        cmpl      $1, %eax                                      #3489.37
        je        ..B7.36       # Prob 33%                      #3489.37
                                # LOE eax edx
..B7.33:                        # Preds ..B7.32                 # Infreq
        testl     %eax, %eax                                    #3489.37
        je        ..B7.37       # Prob 50%                      #3489.37
                                # LOE edx
..B7.34:                        # Preds ..B7.35 ..B7.36 ..B7.37 ..B7.33 # Infreq
        movl      -20(%ebp), %esi                               #3492.32
        movl      -24(%ebp), %edi                               #3492.32
        movl      $1, %eax                                      #3492.32
        movl      %ebp, %esp                                    #3492.32
        popl      %ebp                                          #3492.32
        movl      %ebx, %esp                                    #3492.32
        popl      %ebx                                          #3492.32
        ret                                                     #3492.32
                                # LOE
..B7.35:                        # Preds ..B7.31                 # Infreq
        movl      -84(%ebp), %eax                               #3494.36
        testl     %eax, %eax                                    #3494.36
        jne       ..B7.34       # Prob 28%                      #3494.36
                                # LOE edx
..B7.36:                        # Preds ..B7.35 ..B7.32         # Infreq
        movl      -80(%ebp), %eax                               #3496.36
        testl     %eax, %eax                                    #3496.36
        jne       ..B7.34       # Prob 28%                      #3496.36
                                # LOE edx
..B7.37:                        # Preds ..B7.33 ..B7.36         # Infreq
        testl     %edx, %edx                                    #3498.36
        jne       ..B7.34       # Prob 28%                      #3498.36
                                # LOE
..B7.38:                        # Preds ..B7.20 ..B7.18 ..B7.37 # Infreq
        movl      -20(%ebp), %esi                               #3500.28
        movl      -24(%ebp), %edi                               #3500.28
        xorl      %eax, %eax                                    #3500.28
        movl      %ebp, %esp                                    #3500.28
        popl      %ebp                                          #3500.28
        movl      %ebx, %esp                                    #3500.28
        popl      %ebx                                          #3500.28
        ret                                                     #3500.28
                                # LOE
..B7.42:                        # Preds ..B7.7                  # Infreq
        movdqa    %xmm7, -72(%ebp)                              #
        movdqa    %xmm0, -56(%ebp)                              #
        movdqa    -40(%ebp), %xmm0                              #
        movl      %eax, -76(%ebp)                               #
        movl      %ecx, -4(%ebp)                                #
        jmp       ..B7.30       # Prob 100%                     #
        .align    2,0x90
                                # LOE esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
# mark_end;
	.type	eval_2na_128,@function
	.size	eval_2na_128,.-eval_2na_128
.LNeval_2na_128:
	.data
# -- End  eval_2na_128
	.text
# -- Begin  eval_4na_16
# mark_begin;
       .align    2,0x90
eval_4na_16:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B8.1:                         # Preds ..B8.0
        pushl     %ebx                                          #4057.1
        movl      %esp, %ebx                                    #4057.1
        andl      $-16, %esp                                    #4057.1
        pushl     %ebp                                          #4057.1
        pushl     %ebp                                          #4057.1
        movl      4(%ebx), %ebp                                 #4057.1
        movl      %ebp, 4(%esp)                                 #4057.1
        movl      %esp, %ebp                                    #4057.1
        subl      $136, %esp                                    #4057.1
        movl      %ebx, -136(%ebp)                              #4057.1
        movl      %edi, -36(%ebp)                               #4057.1
        movl      %esi, -32(%ebp)                               #4057.1
        movl      12(%ebx), %ecx                                #4055.5
        movl      16(%ebx), %esi                                #4055.5
        call      ..L8          # Prob 100%                     #4057.1
..L8:                                                           #
        popl      %edi                                          #4057.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L8], %edi       #4057.1
        lea       _gprof_pack7@GOTOFF(%edi), %edx               #4057.1
        movl      %edi, %ebx                                    #4057.1
        call      mcount@PLT                                    #4057.1
        movl      -136(%ebp), %ebx                              #4057.1
                                # LOE esi edi
..B8.35:                        # Preds ..B8.1
        movl      12(%ebx), %ecx                                #
        movl      %esi, -16(%ebp)                               #
        movl      %esi, %eax                                    #4093.49
        movl      %edi, -116(%ebp)                              #
        shrl      $2, %eax                                      #4093.49
        movl      20(%ebx), %edi                                #4090.5
        movl      %eax, -20(%ebp)                               #4093.49
        movl      8(%ebx), %eax                                 #4096.12
        movdqa    16(%eax), %xmm5                               #4109.5
        movdqa    32(%eax), %xmm4                               #4109.5
        movdqa    48(%eax), %xmm3                               #4109.5
        movdqa    64(%eax), %xmm0                               #4109.5
        movdqa    80(%eax), %xmm7                               #4109.5
        movdqa    96(%eax), %xmm1                               #4109.5
        movdqa    112(%eax), %xmm6                              #4109.5
        movdqa    %xmm0, -56(%ebp)                              #4109.5
        movdqa    128(%eax), %xmm0                              #4109.5
        lea       (%esi,%edi), %edx                             #4090.5
        subl      4(%eax), %edx                                 #4096.12
        movl      %edx, -24(%ebp)                               #4096.12
        lea       3(%esi,%edi), %edx                            #4099.50
        shrl      $2, %edx                                      #4099.57
        movl      -20(%ebp), %esi                               #4102.14
        movl      -116(%ebp), %edi                              #4102.14
        addl      %ecx, %edx                                    #4099.30
        movl      %edx, -28(%ebp)                               #4099.30
        movzbl    (%esi,%ecx), %edx                             #4102.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4102.14
        movw      %dx, -132(%ebp)                               #4102.14
        movzbl    1(%esi,%ecx), %edx                            #4102.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4102.14
        movw      %dx, -130(%ebp)                               #4102.14
        movzbl    2(%esi,%ecx), %edx                            #4102.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4102.14
        movw      %dx, -128(%ebp)                               #4102.14
        movzbl    3(%esi,%ecx), %edx                            #4102.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4102.14
        movw      %dx, -126(%ebp)                               #4102.14
        movzbl    4(%esi,%ecx), %edx                            #4102.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4102.14
        movw      %dx, -124(%ebp)                               #4102.14
        movzbl    5(%esi,%ecx), %edx                            #4102.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4102.14
        movw      %dx, -122(%ebp)                               #4102.14
        movzbl    6(%esi,%ecx), %edx                            #4102.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4102.14
        movw      %dx, -120(%ebp)                               #4102.14
        movzbl    7(%esi,%ecx), %edx                            #4102.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4102.14
        movw      %dx, -118(%ebp)                               #4102.14
        movdqu    -132(%ebp), %xmm2                             #4102.14
        lea       8(%ecx,%esi), %edx                            #4103.5
        movl      -16(%ebp), %esi                               #4123.20
        movl      %esi, %eax                                    #4123.20
        andl      $3, %eax                                      #4123.20
        je        ..B8.5        # Prob 20%                      #4123.20
                                # LOE eax edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B8.2:                         # Preds ..B8.35
        cmpl      $1, %eax                                      #4123.20
        je        ..B8.8        # Prob 25%                      #4123.20
                                # LOE eax edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B8.3:                         # Preds ..B8.2
        cmpl      $2, %eax                                      #4123.20
        je        ..B8.7        # Prob 33%                      #4123.20
                                # LOE eax edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B8.4:                         # Preds ..B8.3
        cmpl      $3, %eax                                      #4123.20
        je        ..B8.6        # Prob 50%                      #4123.20
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B8.5:                         # Preds ..B8.35 ..B8.4
        movdqa    %xmm6, -72(%ebp)                              #
        movdqa    %xmm7, -88(%ebp)                              #
        jmp       ..B8.24       # Prob 100%                     #
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B8.6:                         # Preds ..B8.4
        movdqa    %xmm6, -72(%ebp)                              #
        movdqa    %xmm7, -88(%ebp)                              #
        xorl      %eax, %eax                                    #
        movl      %eax, -96(%ebp)                               #
        movl      %eax, -20(%ebp)                               #
        movl      %eax, -40(%ebp)                               #
        jmp       ..B8.11       # Prob 100%                     #
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B8.7:                         # Preds ..B8.3
        movdqa    %xmm6, -72(%ebp)                              #
        movdqa    %xmm7, -88(%ebp)                              #
        xorl      %eax, %eax                                    #
        movl      %eax, -40(%ebp)                               #
        movl      %eax, -20(%ebp)                               #
        jmp       ..B8.10       # Prob 100%                     #
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B8.8:                         # Preds ..B8.2
        movdqa    %xmm6, -72(%ebp)                              #
        movdqa    %xmm7, -88(%ebp)                              #
        xorl      %ecx, %ecx                                    #
        movl      %ecx, -20(%ebp)                               #
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B8.9:                         # Preds ..B8.8 ..B8.24
        movdqa    %xmm2, %xmm7                                  #4147.22
        movdqa    %xmm2, %xmm6                                  #4148.22
        pand      -56(%ebp), %xmm6                              #4148.22
        pand      %xmm3, %xmm7                                  #4147.22
        pcmpeqw   %xmm6, %xmm7                                  #4149.22
        pmovmskb  %xmm7, %eax                                   #4150.22
        movl      %eax, -40(%ebp)                               #4150.22
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B8.10:                        # Preds ..B8.7 ..B8.9
        movdqa    %xmm2, %xmm7                                  #4154.22
        movdqa    %xmm2, %xmm6                                  #4155.22
        pand      -88(%ebp), %xmm7                              #4154.22
        pand      %xmm1, %xmm6                                  #4155.22
        pcmpeqw   %xmm6, %xmm7                                  #4156.22
        pmovmskb  %xmm7, %eax                                   #4157.22
        movl      %eax, -96(%ebp)                               #4157.22
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B8.11:                        # Preds ..B8.6 ..B8.10
        movdqa    %xmm2, %xmm6                                  #4161.22
        pand      -72(%ebp), %xmm6                              #4161.22
        movl      -20(%ebp), %ecx                               #4172.29
        pand      %xmm0, %xmm2                                  #4162.22
        pcmpeqw   %xmm2, %xmm6                                  #4163.22
        pmovmskb  %xmm6, %eax                                   #4164.22
        movl      %eax, -92(%ebp)                               #4164.22
        andl      $-4, %esi                                     #4169.17
        orl       -40(%ebp), %ecx                               #4172.29
        orl       -96(%ebp), %ecx                               #4172.34
        orl       %eax, %ecx                                    #4172.39
        je        ..B8.20       # Prob 78%                      #4172.47
                                # LOE edx esi edi xmm0 xmm1 xmm3 xmm4 xmm5
..B8.12:                        # Preds ..B8.11
        movl      %esi, -16(%ebp)                               #
        movl      -20(%ebp), %esi                               #4190.30
        movl      %edx, -100(%ebp)                              #
        movl      %esi, %eax                                    #4190.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4190.30
        movswl    %ax, %edx                                     #4190.30
        movl      -40(%ebp), %eax                               #4191.30
        movl      %edx, -116(%ebp)                              #4190.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4191.30
        movswl    %ax, %edx                                     #4191.30
        movl      -96(%ebp), %eax                               #4192.30
        lea       1(%edx,%edx), %edx                            #4199.40
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4192.30
        movswl    %ax, %ecx                                     #4192.30
        movl      -92(%ebp), %eax                               #4193.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4193.30
        movl      %edx, -112(%ebp)                              #4199.40
        movswl    %ax, %eax                                     #4193.30
        lea       2(%ecx,%ecx), %edx                            #4200.40
        movl      %edx, -108(%ebp)                              #4200.40
        lea       3(%eax,%eax), %edx                            #4201.40
        testl     %esi, %esi                                    #4205.32
        movl      -16(%ebp), %esi                               #4205.32
        movl      %edx, -104(%ebp)                              #4201.40
        movl      -100(%ebp), %edx                              #4205.32
        je        ..B8.14       # Prob 50%                      #4205.32
                                # LOE edx esi edi dl dh xmm0 xmm1 xmm3 xmm4 xmm5
..B8.13:                        # Preds ..B8.12
        movl      -116(%ebp), %eax                              #4205.43
        movl      -24(%ebp), %ecx                               #4205.49
        lea       (%esi,%eax,2), %eax                           #4205.43
        cmpl      %eax, %ecx                                    #4205.49
        jae       ..B8.28       # Prob 1%                       #4205.49
                                # LOE edx esi edi dl dh xmm0 xmm1 xmm3 xmm4 xmm5
..B8.14:                        # Preds ..B8.13 ..B8.12
        movl      -40(%ebp), %eax                               #4206.32
        testl     %eax, %eax                                    #4206.32
        je        ..B8.16       # Prob 50%                      #4206.32
                                # LOE edx esi edi dl dh xmm0 xmm1 xmm3 xmm4 xmm5
..B8.15:                        # Preds ..B8.14
        movl      -112(%ebp), %ecx                              #4206.43
        movl      -24(%ebp), %eax                               #4206.49
        addl      %esi, %ecx                                    #4206.43
        cmpl      %ecx, %eax                                    #4206.49
        jae       ..B8.28       # Prob 1%                       #4206.49
                                # LOE edx esi edi dl dh xmm0 xmm1 xmm3 xmm4 xmm5
..B8.16:                        # Preds ..B8.15 ..B8.14
        movl      -96(%ebp), %eax                               #4207.32
        testl     %eax, %eax                                    #4207.32
        je        ..B8.18       # Prob 50%                      #4207.32
                                # LOE edx esi edi dl dh xmm0 xmm1 xmm3 xmm4 xmm5
..B8.17:                        # Preds ..B8.16
        movl      -108(%ebp), %ecx                              #4207.43
        movl      -24(%ebp), %eax                               #4207.49
        addl      %esi, %ecx                                    #4207.43
        cmpl      %ecx, %eax                                    #4207.49
        jae       ..B8.28       # Prob 1%                       #4207.49
                                # LOE edx esi edi dl dh xmm0 xmm1 xmm3 xmm4 xmm5
..B8.18:                        # Preds ..B8.17 ..B8.16
        movl      -92(%ebp), %eax                               #4208.32
        testl     %eax, %eax                                    #4208.32
        je        ..B8.20       # Prob 50%                      #4208.32
                                # LOE edx esi edi dl dh xmm0 xmm1 xmm3 xmm4 xmm5
..B8.19:                        # Preds ..B8.18
        movl      -104(%ebp), %ecx                              #4208.43
        movl      -24(%ebp), %eax                               #4208.49
        addl      %esi, %ecx                                    #4208.43
        cmpl      %ecx, %eax                                    #4208.49
        jae       ..B8.28       # Prob 1%                       #4208.49
                                # LOE edx esi edi xmm0 xmm1 xmm3 xmm4 xmm5
..B8.20:                        # Preds ..B8.19 ..B8.18 ..B8.11
        movl      -24(%ebp), %eax                               #4216.28
        lea       4(%esi), %ecx                                 #4241.13
        cmpl      %ecx, %eax                                    #4216.28
        jb        ..B8.26       # Prob 1%                       #4216.28
                                # LOE edx esi edi xmm0 xmm1 xmm3 xmm4 xmm5
..B8.21:                        # Preds ..B8.20
        movl      -28(%ebp), %eax                               #4237.25
        cmpl      %eax, %edx                                    #4237.25
        jae       ..B8.26       # Prob 1%                       #4237.25
                                # LOE edx esi edi xmm0 xmm1 xmm3 xmm4 xmm5
..B8.22:                        # Preds ..B8.21
        movl      -24(%ebp), %eax                               #4242.24
        addl      $32, %esi                                     #4241.13
        cmpl      %eax, %esi                                    #4242.24
        ja        ..B8.26       # Prob 1%                       #4242.24
                                # LOE edx esi edi xmm0 xmm1 xmm3 xmm4 xmm5
..B8.23:                        # Preds ..B8.22
        movzbl    (%edx), %eax                                  #4247.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %ecx        #4247.22
        movzbl    1(%edx), %eax                                 #4247.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #4247.22
        movw      %cx, -132(%ebp)                               #4247.22
        movw      %ax, -130(%ebp)                               #4247.22
        movzbl    2(%edx), %eax                                 #4247.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #4247.22
        movw      %ax, -128(%ebp)                               #4247.22
        movzbl    3(%edx), %eax                                 #4247.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #4247.22
        movw      %ax, -126(%ebp)                               #4247.22
        movzbl    4(%edx), %eax                                 #4247.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #4247.22
        movw      %ax, -124(%ebp)                               #4247.22
        movzbl    5(%edx), %eax                                 #4247.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #4247.22
        movw      %ax, -122(%ebp)                               #4247.22
        movzbl    6(%edx), %eax                                 #4247.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #4247.22
        movw      %ax, -120(%ebp)                               #4247.22
        movzbl    7(%edx), %eax                                 #4247.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #4247.22
        addl      $8, %edx                                      #4262.13
        movw      %ax, -118(%ebp)                               #4247.22
        movdqu    -132(%ebp), %xmm2                             #4247.22
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B8.24:                        # Preds ..B8.23 ..B8.5
        movdqa    %xmm2, %xmm7                                  #4140.22
        movdqa    %xmm2, %xmm6                                  #4141.22
        pand      %xmm5, %xmm7                                  #4140.22
        pand      %xmm4, %xmm6                                  #4141.22
        pcmpeqw   %xmm6, %xmm7                                  #4142.22
        pmovmskb  %xmm7, %eax                                   #4143.22
        movl      %eax, -20(%ebp)                               #4143.22
        jmp       ..B8.9        # Prob 100%                     #4143.22
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B8.26:                        # Preds ..B8.20 ..B8.21 ..B8.22 # Infreq
        movl      -32(%ebp), %esi                               #4273.12
        movl      -36(%ebp), %edi                               #4273.12
        xorl      %eax, %eax                                    #4273.12
        movl      %ebp, %esp                                    #4273.12
        popl      %ebp                                          #4273.12
        movl      %ebx, %esp                                    #4273.12
        popl      %ebx                                          #4273.12
        ret                                                     #4273.12
                                # LOE
..B8.28:                        # Preds ..B8.19 ..B8.17 ..B8.15 ..B8.13 # Infreq
        movl      -32(%ebp), %esi                               #4205.63
        movl      -36(%ebp), %edi                               #4205.63
        movl      $1, %eax                                      #4205.63
        movl      %ebp, %esp                                    #4205.63
        popl      %ebp                                          #4205.63
        movl      %ebx, %esp                                    #4205.63
        popl      %ebx                                          #4205.63
        ret                                                     #4205.63
        .align    2,0x90
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
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B9.1:                         # Preds ..B9.0
        pushl     %ebx                                          #4279.1
        movl      %esp, %ebx                                    #4279.1
        andl      $-16, %esp                                    #4279.1
        pushl     %ebp                                          #4279.1
        pushl     %ebp                                          #4279.1
        movl      4(%ebx), %ebp                                 #4279.1
        movl      %ebp, 4(%esp)                                 #4279.1
        movl      %esp, %ebp                                    #4279.1
        subl      $152, %esp                                    #4279.1
        movl      %ebx, -152(%ebp)                              #4279.1
        movl      %edi, -44(%ebp)                               #4279.1
        movl      %esi, -40(%ebp)                               #4279.1
        movl      12(%ebx), %ecx                                #4277.5
        movl      16(%ebx), %esi                                #4277.5
        call      ..L9          # Prob 100%                     #4279.1
..L9:                                                           #
        popl      %edi                                          #4279.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L9], %edi       #4279.1
        lea       _gprof_pack8@GOTOFF(%edi), %edx               #4279.1
        movl      %edi, %ebx                                    #4279.1
        call      mcount@PLT                                    #4279.1
        movl      -152(%ebp), %ebx                              #4279.1
                                # LOE esi edi
..B9.41:                        # Preds ..B9.1
        movl      12(%ebx), %ecx                                #
        movl      %esi, -24(%ebp)                               #
        movl      %esi, %eax                                    #4315.49
        movl      %edi, -16(%ebp)                               #
        shrl      $2, %eax                                      #4315.49
        movl      20(%ebx), %edi                                #4312.5
        movl      %eax, -20(%ebp)                               #4315.49
        movl      8(%ebx), %eax                                 #4318.12
        movdqa    16(%eax), %xmm5                               #4331.5
        movdqa    32(%eax), %xmm4                               #4331.5
        movdqa    48(%eax), %xmm3                               #4331.5
        movdqa    64(%eax), %xmm6                               #4331.5
        movdqa    80(%eax), %xmm2                               #4331.5
        movdqa    96(%eax), %xmm1                               #4331.5
        movdqa    128(%eax), %xmm7                              #4331.5
        lea       (%esi,%edi), %edx                             #4312.5
        subl      4(%eax), %edx                                 #4318.12
        movl      %edx, -32(%ebp)                               #4318.12
        lea       3(%esi,%edi), %edx                            #4321.50
        shrl      $2, %edx                                      #4321.57
        movl      -20(%ebp), %esi                               #4324.14
        movl      -16(%ebp), %edi                               #4324.14
        addl      %ecx, %edx                                    #4321.30
        movl      %edx, -36(%ebp)                               #4321.30
        movzbl    (%esi,%ecx), %edx                             #4324.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4324.14
        movw      %dx, -148(%ebp)                               #4324.14
        movzbl    1(%esi,%ecx), %edx                            #4324.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4324.14
        movw      %dx, -146(%ebp)                               #4324.14
        movzbl    2(%esi,%ecx), %edx                            #4324.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4324.14
        movw      %dx, -144(%ebp)                               #4324.14
        movzbl    3(%esi,%ecx), %edx                            #4324.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4324.14
        movw      %dx, -142(%ebp)                               #4324.14
        movzbl    4(%esi,%ecx), %edx                            #4324.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4324.14
        movw      %dx, -140(%ebp)                               #4324.14
        movzbl    5(%esi,%ecx), %edx                            #4324.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4324.14
        movw      %dx, -138(%ebp)                               #4324.14
        movzbl    6(%esi,%ecx), %edx                            #4324.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4324.14
        movw      %dx, -136(%ebp)                               #4324.14
        movzbl    7(%esi,%ecx), %edx                            #4324.14
        movzwl    expand_2na.0@GOTOFF(%edi,%edx,2), %edx        #4324.14
        movw      %dx, -134(%ebp)                               #4324.14
        movdqu    -148(%ebp), %xmm0                             #4324.14
        lea       8(%ecx,%esi), %edx                            #4325.5
        movl      -24(%ebp), %esi                               #4345.20
        movdqa    %xmm0, -120(%ebp)                             #4324.14
        movdqa    112(%eax), %xmm0                              #4331.5
        movl      %esi, %eax                                    #4345.20
        movl      %edx, -28(%ebp)                               #4325.5
        andl      $3, %eax                                      #4345.20
        je        ..B9.33       # Prob 20%                      #4345.20
                                # LOE eax edx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B9.2:                         # Preds ..B9.41
        cmpl      $1, %eax                                      #4345.20
        je        ..B9.8        # Prob 25%                      #4345.20
                                # LOE eax edx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B9.3:                         # Preds ..B9.2
        cmpl      $2, %eax                                      #4345.20
        je        ..B9.7        # Prob 33%                      #4345.20
                                # LOE eax edx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B9.4:                         # Preds ..B9.3
        cmpl      $3, %eax                                      #4345.20
        je        ..B9.6        # Prob 50%                      #4345.20
                                # LOE edx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B9.5:                         # Preds ..B9.4
        movdqa    %xmm7, -72(%ebp)                              #
        movdqa    %xmm0, -88(%ebp)                              #
        movdqa    -120(%ebp), %xmm0                             #
        movdqa    %xmm6, -104(%ebp)                             #
        movl      %edi, -16(%ebp)                               #
        jmp       ..B9.25       # Prob 100%                     #
                                # LOE edx esi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.6:                         # Preds ..B9.4
        movdqa    %xmm7, -72(%ebp)                              #
        movdqa    %xmm0, -88(%ebp)                              #
        movdqa    -120(%ebp), %xmm0                             #
        movdqa    %xmm6, -104(%ebp)                             #
        xorl      %eax, %eax                                    #
        movl      %eax, -56(%ebp)                               #
        movl      %edi, -16(%ebp)                               #
        xorl      %ecx, %ecx                                    #
        movl      %ecx, -52(%ebp)                               #
        movl      %eax, -48(%ebp)                               #
        movl      $2, %ecx                                      #
        jmp       ..B9.11       # Prob 100%                     #
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.7:                         # Preds ..B9.3
        movdqa    %xmm7, -72(%ebp)                              #
        movdqa    %xmm0, -88(%ebp)                              #
        movdqa    -120(%ebp), %xmm0                             #
        movdqa    %xmm6, -104(%ebp)                             #
        xorl      %eax, %eax                                    #
        movl      %eax, -52(%ebp)                               #
        movl      %edi, -16(%ebp)                               #
        movl      $2, %ecx                                      #
        movl      %eax, -48(%ebp)                               #
        jmp       ..B9.10       # Prob 100%                     #
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.8:                         # Preds ..B9.2
        movdqa    %xmm7, -72(%ebp)                              #
        movdqa    %xmm0, -88(%ebp)                              #
        movdqa    -120(%ebp), %xmm0                             #
        movdqa    %xmm6, -104(%ebp)                             #
        xorl      %eax, %eax                                    #
        movl      %eax, -48(%ebp)                               #
        movl      %edi, -16(%ebp)                               #
        movl      $2, %ecx                                      #
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.9:                         # Preds ..B9.8 ..B9.29
        movdqa    %xmm0, %xmm7                                  #4369.22
        movdqa    %xmm0, %xmm6                                  #4370.22
        pand      -104(%ebp), %xmm6                             #4370.22
        pand      %xmm3, %xmm7                                  #4369.22
        pcmpeqd   %xmm6, %xmm7                                  #4371.22
        pmovmskb  %xmm7, %eax                                   #4372.22
        movl      %eax, -52(%ebp)                               #4372.22
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.10:                        # Preds ..B9.7 ..B9.9
        movdqa    %xmm0, %xmm7                                  #4376.22
        movdqa    %xmm0, %xmm6                                  #4377.22
        pand      %xmm2, %xmm7                                  #4376.22
        pand      %xmm1, %xmm6                                  #4377.22
        pcmpeqd   %xmm6, %xmm7                                  #4378.22
        pmovmskb  %xmm7, %eax                                   #4379.22
        movl      %eax, -56(%ebp)                               #4379.22
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.11:                        # Preds ..B9.6 ..B9.10
        movdqa    %xmm0, %xmm7                                  #4383.22
        movdqa    %xmm0, %xmm6                                  #4384.22
        pand      -88(%ebp), %xmm7                              #4383.22
        pand      -72(%ebp), %xmm6                              #4384.22
        pcmpeqd   %xmm6, %xmm7                                  #4385.22
        pmovmskb  %xmm7, %eax                                   #4386.22
        movl      -48(%ebp), %edi                               #4394.29
        movl      %eax, -112(%ebp)                              #4386.22
        andl      $-4, %esi                                     #4391.17
        movl      %esi, -20(%ebp)                               #4391.17
        orl       -52(%ebp), %edi                               #4394.29
        orl       -56(%ebp), %edi                               #4394.34
        orl       %eax, %edi                                    #4394.39
        je        ..B9.20       # Prob 78%                      #4394.47
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.12:                        # Preds ..B9.11
        movl      %esi, %eax                                    #
        movl      -48(%ebp), %edi                               #4412.30
        movl      %eax, -20(%ebp)                               #
        movl      %edi, %eax                                    #4412.30
        movl      %ecx, -120(%ebp)                              #
        movl      %edx, -116(%ebp)                              #
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4412.30
        movswl    %ax, %edx                                     #4412.30
        movl      -52(%ebp), %eax                               #4413.30
        movl      %edx, -132(%ebp)                              #4412.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4413.30
        movswl    %ax, %esi                                     #4413.30
        lea       1(%esi,%esi), %esi                            #4421.40
        movl      -56(%ebp), %eax                               #4414.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4414.30
        movswl    %ax, %ecx                                     #4414.30
        movl      -112(%ebp), %eax                              #4415.30
        lea       2(%ecx,%ecx), %ecx                            #4422.40
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4415.30
        movl      %esi, -128(%ebp)                              #4421.40
        movl      %ecx, -124(%ebp)                              #4422.40
        movl      -120(%ebp), %ecx                              #4427.32
        movswl    %ax, %edx                                     #4415.30
        lea       3(%edx,%edx), %edx                            #4423.40
        testl     %edi, %edi                                    #4427.32
        movl      %edx, -24(%ebp)                               #4423.40
        movl      -116(%ebp), %edx                              #4427.32
        je        ..B9.14       # Prob 50%                      #4427.32
                                # LOE edx ecx dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.13:                        # Preds ..B9.12
        movl      -132(%ebp), %esi                              #4427.43
        movl      -20(%ebp), %eax                               #4427.43
        movl      -32(%ebp), %edi                               #4427.49
        lea       (%eax,%esi,2), %eax                           #4427.43
        cmpl      %eax, %edi                                    #4427.49
        jae       ..B9.34       # Prob 1%                       #4427.49
                                # LOE edx ecx dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.14:                        # Preds ..B9.13 ..B9.12
        movl      -52(%ebp), %eax                               #4428.32
        testl     %eax, %eax                                    #4428.32
        je        ..B9.16       # Prob 50%                      #4428.32
                                # LOE edx ecx dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.15:                        # Preds ..B9.14
        movl      -128(%ebp), %esi                              #4428.43
        addl      -20(%ebp), %esi                               #4428.43
        movl      -32(%ebp), %eax                               #4428.49
        cmpl      %esi, %eax                                    #4428.49
        jae       ..B9.34       # Prob 1%                       #4428.49
                                # LOE edx ecx dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.16:                        # Preds ..B9.15 ..B9.14
        movl      -56(%ebp), %eax                               #4429.32
        testl     %eax, %eax                                    #4429.32
        je        ..B9.18       # Prob 50%                      #4429.32
                                # LOE edx ecx dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.17:                        # Preds ..B9.16
        movl      -124(%ebp), %esi                              #4429.43
        addl      -20(%ebp), %esi                               #4429.43
        movl      -32(%ebp), %eax                               #4429.49
        cmpl      %esi, %eax                                    #4429.49
        jae       ..B9.34       # Prob 1%                       #4429.49
                                # LOE edx ecx dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.18:                        # Preds ..B9.17 ..B9.16
        movl      -112(%ebp), %eax                              #4430.32
        testl     %eax, %eax                                    #4430.32
        je        ..B9.20       # Prob 50%                      #4430.32
                                # LOE edx ecx dl cl dh ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.19:                        # Preds ..B9.18
        movl      -24(%ebp), %esi                               #4430.43
        addl      -20(%ebp), %esi                               #4430.43
        movl      -32(%ebp), %eax                               #4430.49
        cmpl      %esi, %eax                                    #4430.49
        jae       ..B9.34       # Prob 1%                       #4430.49
                                # LOE edx ecx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.20:                        # Preds ..B9.19 ..B9.18 ..B9.11
        movl      -20(%ebp), %esi                               #4435.17
        movl      -32(%ebp), %eax                               #4438.28
        lea       4(%esi), %esi                                 #4435.17
        cmpl      %eax, %esi                                    #4438.28
        ja        ..B9.31       # Prob 1%                       #4438.28
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.21:                        # Preds ..B9.20
        addl      $-1, %ecx                                     #4442.25
        jne       ..B9.26       # Prob 50%                      #4442.39
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.22:                        # Preds ..B9.21
        movl      -36(%ebp), %eax                               #4459.25
        cmpl      %eax, %edx                                    #4459.25
        jae       ..B9.31       # Prob 1%                       #4459.25
                                # LOE edx xmm1 xmm2 xmm3 xmm4 xmm5
..B9.23:                        # Preds ..B9.22
        movl      -20(%ebp), %esi                               #4463.13
        movl      -32(%ebp), %eax                               #4464.24
        lea       28(%esi), %esi                                #4463.13
        cmpl      %eax, %esi                                    #4464.24
        ja        ..B9.31       # Prob 1%                       #4464.24
                                # LOE edx esi xmm1 xmm2 xmm3 xmm4 xmm5
..B9.24:                        # Preds ..B9.23
        movl      -16(%ebp), %eax                               #4469.22
        movzbl    (%edx), %ecx                                  #4469.22
        movzwl    expand_2na.0@GOTOFF(%eax,%ecx,2), %edi        #4469.22
        movzbl    1(%edx), %ecx                                 #4469.22
        movzwl    expand_2na.0@GOTOFF(%eax,%ecx,2), %ecx        #4469.22
        movw      %di, -148(%ebp)                               #4469.22
        movw      %cx, -146(%ebp)                               #4469.22
        movzbl    2(%edx), %ecx                                 #4469.22
        movzwl    expand_2na.0@GOTOFF(%eax,%ecx,2), %ecx        #4469.22
        movw      %cx, -144(%ebp)                               #4469.22
        movzbl    3(%edx), %ecx                                 #4469.22
        movzwl    expand_2na.0@GOTOFF(%eax,%ecx,2), %ecx        #4469.22
        movw      %cx, -142(%ebp)                               #4469.22
        movzbl    4(%edx), %ecx                                 #4469.22
        movzwl    expand_2na.0@GOTOFF(%eax,%ecx,2), %ecx        #4469.22
        movw      %cx, -140(%ebp)                               #4469.22
        movzbl    5(%edx), %ecx                                 #4469.22
        movzwl    expand_2na.0@GOTOFF(%eax,%ecx,2), %ecx        #4469.22
        movw      %cx, -138(%ebp)                               #4469.22
        movzbl    6(%edx), %ecx                                 #4469.22
        movzwl    expand_2na.0@GOTOFF(%eax,%ecx,2), %ecx        #4469.22
        movw      %cx, -136(%ebp)                               #4469.22
        movzbl    7(%edx), %ecx                                 #4469.22
        addl      $8, %edx                                      #4484.13
        movzwl    expand_2na.0@GOTOFF(%eax,%ecx,2), %eax        #4469.22
        movw      %ax, -134(%ebp)                               #4469.22
        movdqu    -148(%ebp), %xmm0                             #4469.22
        movl      %edx, -28(%ebp)                               #4484.13
                                # LOE edx esi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.25:                        # Preds ..B9.5 ..B9.24
        movl      $2, %ecx                                      #4352.13
        jmp       ..B9.29       # Prob 100%                     #4352.13
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.26:                        # Preds ..B9.21
        movl      -36(%ebp), %edi                               #4450.26
        movl      -28(%ebp), %eax                               #4450.26
        psrldq    $2, %xmm0                                     #4447.26
        cmpl      %edi, %eax                                    #4450.26
        jae       ..B9.28       # Prob 19%                      #4450.26
                                # LOE eax edx ecx esi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.27:                        # Preds ..B9.26
        movl      -16(%ebp), %edi                               #4451.57
        movzbl    (%eax), %eax                                  #4451.72
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #4451.57
        pinsrw    $7, %eax, %xmm0                               #4451.30
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.28:                        # Preds ..B9.27 ..B9.26
        addl      $1, -28(%ebp)                                 #4454.20
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.29:                        # Preds ..B9.25 ..B9.28 ..B9.33
        movdqa    %xmm0, %xmm7                                  #4362.22
        movdqa    %xmm0, %xmm6                                  #4363.22
        pand      %xmm5, %xmm7                                  #4362.22
        pand      %xmm4, %xmm6                                  #4363.22
        pcmpeqd   %xmm6, %xmm7                                  #4364.22
        pmovmskb  %xmm7, %eax                                   #4365.22
        movl      %eax, -48(%ebp)                               #4365.22
        jmp       ..B9.9        # Prob 100%                     #4365.22
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.31:                        # Preds ..B9.20 ..B9.22 ..B9.23 # Infreq
        movl      -40(%ebp), %esi                               #4495.12
        movl      -44(%ebp), %edi                               #4495.12
        xorl      %eax, %eax                                    #4495.12
        movl      %ebp, %esp                                    #4495.12
        popl      %ebp                                          #4495.12
        movl      %ebx, %esp                                    #4495.12
        popl      %ebx                                          #4495.12
        ret                                                     #4495.12
                                # LOE
..B9.33:                        # Preds ..B9.41                 # Infreq
        movdqa    %xmm7, -72(%ebp)                              #
        movdqa    %xmm0, -88(%ebp)                              #
        movdqa    -120(%ebp), %xmm0                             #
        movdqa    %xmm6, -104(%ebp)                             #
        movl      $2, %ecx                                      #
        movl      %edi, -16(%ebp)                               #
        jmp       ..B9.29       # Prob 100%                     #
                                # LOE edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.34:                        # Preds ..B9.13 ..B9.19 ..B9.17 ..B9.15 # Infreq
        movl      -40(%ebp), %esi                               #4428.63
        movl      -44(%ebp), %edi                               #4428.63
        movl      $1, %eax                                      #4428.63
        movl      %ebp, %esp                                    #4428.63
        popl      %ebp                                          #4428.63
        movl      %ebx, %esp                                    #4428.63
        popl      %ebx                                          #4428.63
        ret                                                     #4428.63
        .align    2,0x90
                                # LOE
# mark_end;
	.type	eval_4na_32,@function
	.size	eval_4na_32,.-eval_4na_32
.LNeval_4na_32:
	.data
# -- End  eval_4na_32
	.text
# -- Begin  eval_4na_128
# mark_begin;
       .align    2,0x90
eval_4na_128:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B10.1:                        # Preds ..B10.0
        pushl     %ebx                                          #4724.1
        movl      %esp, %ebx                                    #4724.1
        andl      $-16, %esp                                    #4724.1
        pushl     %ebp                                          #4724.1
        pushl     %ebp                                          #4724.1
        movl      4(%ebx), %ebp                                 #4724.1
        movl      %ebp, 4(%esp)                                 #4724.1
        movl      %esp, %ebp                                    #4724.1
        subl      $120, %esp                                    #4724.1
        movl      %ebx, -120(%ebp)                              #4724.1
        movl      %edi, -24(%ebp)                               #4724.1
        movl      %esi, -20(%ebp)                               #4724.1
        movl      12(%ebx), %ecx                                #4722.5
        movl      16(%ebx), %esi                                #4722.5
        call      ..L10         # Prob 100%                     #4724.1
..L10:                                                          #
        popl      %edi                                          #4724.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L10], %edi      #4724.1
        lea       _gprof_pack9@GOTOFF(%edi), %edx               #4724.1
        movl      %edi, %ebx                                    #4724.1
        call      mcount@PLT                                    #4724.1
        movl      -120(%ebp), %ebx                              #4724.1
                                # LOE esi edi
..B10.37:                       # Preds ..B10.1
        movl      12(%ebx), %ecx                                #
        movl      %esi, -100(%ebp)                              #
        movl      %esi, %eax                                    #4760.49
        movl      %edi, -96(%ebp)                               #
        shrl      $2, %eax                                      #4760.49
        movl      20(%ebx), %edi                                #4757.5
        movl      %eax, -4(%ebp)                                #4760.49
        movl      8(%ebx), %eax                                 #4763.12
        movdqa    16(%eax), %xmm5                               #4776.5
        movdqa    32(%eax), %xmm4                               #4776.5
        movdqa    48(%eax), %xmm3                               #4776.5
        movdqa    64(%eax), %xmm2                               #4776.5
        movdqa    80(%eax), %xmm1                               #4776.5
        movdqa    96(%eax), %xmm6                               #4776.5
        movdqa    128(%eax), %xmm7                              #4776.5
        lea       (%esi,%edi), %edx                             #4757.5
        subl      4(%eax), %edx                                 #4763.12
        movl      %edx, -12(%ebp)                               #4763.12
        movl      -4(%ebp), %edx                                #4769.14
        lea       3(%esi,%edi), %esi                            #4766.50
        movl      -96(%ebp), %edi                               #4769.14
        shrl      $2, %esi                                      #4766.57
        addl      %ecx, %esi                                    #4766.30
        movl      %esi, -16(%ebp)                               #4766.30
        movzbl    (%edx,%ecx), %esi                             #4769.14
        movzwl    expand_2na.0@GOTOFF(%edi,%esi,2), %esi        #4769.14
        movw      %si, -116(%ebp)                               #4769.14
        movzbl    1(%edx,%ecx), %esi                            #4769.14
        movzwl    expand_2na.0@GOTOFF(%edi,%esi,2), %esi        #4769.14
        movw      %si, -114(%ebp)                               #4769.14
        movzbl    2(%edx,%ecx), %esi                            #4769.14
        movzwl    expand_2na.0@GOTOFF(%edi,%esi,2), %esi        #4769.14
        movw      %si, -112(%ebp)                               #4769.14
        movzbl    3(%edx,%ecx), %esi                            #4769.14
        movzwl    expand_2na.0@GOTOFF(%edi,%esi,2), %esi        #4769.14
        movw      %si, -110(%ebp)                               #4769.14
        movzbl    4(%edx,%ecx), %esi                            #4769.14
        movzwl    expand_2na.0@GOTOFF(%edi,%esi,2), %esi        #4769.14
        movw      %si, -108(%ebp)                               #4769.14
        movzbl    5(%edx,%ecx), %esi                            #4769.14
        movzwl    expand_2na.0@GOTOFF(%edi,%esi,2), %esi        #4769.14
        movw      %si, -106(%ebp)                               #4769.14
        movzbl    6(%edx,%ecx), %esi                            #4769.14
        movzwl    expand_2na.0@GOTOFF(%edi,%esi,2), %esi        #4769.14
        movw      %si, -104(%ebp)                               #4769.14
        movzbl    7(%edx,%ecx), %esi                            #4769.14
        movzwl    expand_2na.0@GOTOFF(%edi,%esi,2), %esi        #4769.14
        lea       8(%ecx,%edx), %edx                            #4770.5
        movl      -12(%ebp), %ecx                               #4781.20
        movw      %si, -102(%ebp)                               #4769.14
        movdqu    -116(%ebp), %xmm0                             #4769.14
        movl      -100(%ebp), %esi                              #4781.20
        movdqa    %xmm0, -40(%ebp)                              #4769.14
        movdqa    112(%eax), %xmm0                              #4776.5
        movl      %edx, -8(%ebp)                                #4770.5
        subl      %esi, %ecx                                    #4781.20
        addl      $7, %ecx                                      #4781.33
        shrl      $2, %ecx                                      #4781.40
        movl      %esi, %eax                                    #4790.20
        andl      $3, %eax                                      #4790.20
        je        ..B10.32      # Prob 20%                      #4790.20
                                # LOE eax edx ecx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B10.2:                        # Preds ..B10.37
        cmpl      $1, %eax                                      #4790.20
        je        ..B10.8       # Prob 25%                      #4790.20
                                # LOE eax edx ecx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B10.3:                        # Preds ..B10.2
        cmpl      $2, %eax                                      #4790.20
        je        ..B10.7       # Prob 33%                      #4790.20
                                # LOE eax edx ecx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B10.4:                        # Preds ..B10.3
        cmpl      $3, %eax                                      #4790.20
        je        ..B10.6       # Prob 50%                      #4790.20
                                # LOE edx ecx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B10.5:                        # Preds ..B10.4
        movdqa    %xmm7, -88(%ebp)                              #
        movdqa    %xmm0, -72(%ebp)                              #
        movdqa    -40(%ebp), %xmm0                              #
        movdqa    %xmm6, -56(%ebp)                              #
        jmp       ..B10.16      # Prob 100%                     #
                                # LOE edx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.6:                        # Preds ..B10.4
        movdqa    %xmm7, -88(%ebp)                              #
        movdqa    %xmm0, -72(%ebp)                              #
        movdqa    -40(%ebp), %xmm0                              #
        movdqa    %xmm6, -56(%ebp)                              #
        xorl      %eax, %eax                                    #
        movl      %eax, -100(%ebp)                              #
        movl      %ecx, -4(%ebp)                                #
        movl      %eax, -96(%ebp)                               #
        movl      %eax, -92(%ebp)                               #
        jmp       ..B10.11      # Prob 100%                     #
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.7:                        # Preds ..B10.3
        movdqa    %xmm7, -88(%ebp)                              #
        movdqa    %xmm0, -72(%ebp)                              #
        movdqa    -40(%ebp), %xmm0                              #
        movdqa    %xmm6, -56(%ebp)                              #
        xorl      %eax, %eax                                    #
        movl      %eax, -96(%ebp)                               #
        movl      %ecx, -4(%ebp)                                #
        movl      %eax, -92(%ebp)                               #
        jmp       ..B10.10      # Prob 100%                     #
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.8:                        # Preds ..B10.2
        movdqa    %xmm7, -88(%ebp)                              #
        movdqa    %xmm0, -72(%ebp)                              #
        movdqa    -40(%ebp), %xmm0                              #
        movdqa    %xmm6, -56(%ebp)                              #
        xorl      %eax, %eax                                    #
        movl      %ecx, -4(%ebp)                                #
        movl      %eax, -92(%ebp)                               #
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.9:                        # Preds ..B10.8 ..B10.20
        movdqa    %xmm0, %xmm7                                  #4814.22
        movdqa    %xmm0, %xmm6                                  #4815.22
        pand      %xmm3, %xmm7                                  #4814.22
        pand      %xmm2, %xmm6                                  #4815.22
        pcmpeqd   %xmm6, %xmm7                                  #4816.22
        pmovmskb  %xmm7, %eax                                   #4817.22
        addl      $1, %eax                                      #4818.17
        shrl      $16, %eax                                     #4818.17
        negl      %eax                                          #4818.17
        movl      %eax, -96(%ebp)                               #4818.17
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.10:                       # Preds ..B10.7 ..B10.9
        movdqa    %xmm0, %xmm7                                  #4821.22
        movdqa    %xmm0, %xmm6                                  #4822.22
        pand      -56(%ebp), %xmm6                              #4822.22
        pand      %xmm1, %xmm7                                  #4821.22
        pcmpeqd   %xmm6, %xmm7                                  #4823.22
        pmovmskb  %xmm7, %eax                                   #4824.22
        addl      $1, %eax                                      #4825.17
        shrl      $16, %eax                                     #4825.17
        negl      %eax                                          #4825.17
        movl      %eax, -100(%ebp)                              #4825.17
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.11:                       # Preds ..B10.6 ..B10.10
        movdqa    %xmm0, %xmm7                                  #4828.22
        movdqa    %xmm0, %xmm6                                  #4829.22
        movl      -92(%ebp), %ecx                               #4839.29
        pand      -72(%ebp), %xmm7                              #4828.22
        pand      -88(%ebp), %xmm6                              #4829.22
        pcmpeqd   %xmm6, %xmm7                                  #4830.22
        pmovmskb  %xmm7, %eax                                   #4831.22
        andl      $-4, %esi                                     #4836.17
        addl      $1, %eax                                      #4832.17
        orl       -96(%ebp), %ecx                               #4839.29
        orl       -100(%ebp), %ecx                              #4839.34
        shrl      $16, %eax                                     #4832.17
        negl      %eax                                          #4832.17
        orl       %eax, %ecx                                    #4839.39
        jne       ..B10.21      # Prob 1%                       #4839.47
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.12:                       # Preds ..B10.11
        movl      -12(%ebp), %eax                               #4883.28
        addl      $4, %esi                                      #4880.17
        cmpl      %eax, %esi                                    #4883.28
        ja        ..B10.28      # Prob 1%                       #4883.28
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.13:                       # Preds ..B10.12
        movl      -4(%ebp), %eax                                #4887.25
        addl      $-1, %eax                                     #4887.25
        movl      %eax, -4(%ebp)                                #4887.25
        jne       ..B10.17      # Prob 50%                      #4887.39
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.14:                       # Preds ..B10.13
        movl      -16(%ebp), %eax                               #4904.25
        cmpl      %eax, %edx                                    #4904.25
        jae       ..B10.28      # Prob 1%                       #4904.25
                                # LOE edx esi edi xmm1 xmm2 xmm3 xmm4 xmm5
..B10.15:                       # Preds ..B10.14
        movzbl    (%edx), %eax                                  #4914.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %ecx        #4914.22
        movzbl    1(%edx), %eax                                 #4914.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #4914.22
        movw      %cx, -116(%ebp)                               #4914.22
        movw      %ax, -114(%ebp)                               #4914.22
        movzbl    2(%edx), %eax                                 #4914.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #4914.22
        movw      %ax, -112(%ebp)                               #4914.22
        movzbl    3(%edx), %eax                                 #4914.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #4914.22
        movw      %ax, -110(%ebp)                               #4914.22
        movzbl    4(%edx), %eax                                 #4914.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #4914.22
        movw      %ax, -108(%ebp)                               #4914.22
        movzbl    5(%edx), %eax                                 #4914.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #4914.22
        movw      %ax, -106(%ebp)                               #4914.22
        movzbl    6(%edx), %eax                                 #4914.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #4914.22
        movw      %ax, -104(%ebp)                               #4914.22
        movzbl    7(%edx), %eax                                 #4914.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #4914.22
        addl      $8, %edx                                      #4929.13
        movw      %ax, -102(%ebp)                               #4914.22
        movdqu    -116(%ebp), %xmm0                             #4914.22
        movl      %edx, -8(%ebp)                                #4929.13
                                # LOE edx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.16:                       # Preds ..B10.5 ..B10.15
        movl      $8, %eax                                      #4797.13
        movl      %eax, -4(%ebp)                                #4797.13
        jmp       ..B10.20      # Prob 100%                     #4797.13
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.17:                       # Preds ..B10.13
        movl      -16(%ebp), %ecx                               #4895.26
        movl      -8(%ebp), %eax                                #4895.26
        psrldq    $2, %xmm0                                     #4892.26
        cmpl      %ecx, %eax                                    #4895.26
        jae       ..B10.19      # Prob 19%                      #4895.26
                                # LOE eax edx esi edi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.18:                       # Preds ..B10.17
        movzbl    (%eax), %ecx                                  #4896.72
        movzwl    expand_2na.0@GOTOFF(%edi,%ecx,2), %eax        #4896.57
        pinsrw    $7, %eax, %xmm0                               #4896.30
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.19:                       # Preds ..B10.18 ..B10.17
        addl      $1, -8(%ebp)                                  #4899.20
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.20:                       # Preds ..B10.16 ..B10.19 ..B10.32
        movdqa    %xmm0, %xmm7                                  #4807.22
        movdqa    %xmm0, %xmm6                                  #4808.22
        pand      %xmm5, %xmm7                                  #4807.22
        pand      %xmm4, %xmm6                                  #4808.22
        pcmpeqd   %xmm6, %xmm7                                  #4809.22
        pmovmskb  %xmm7, %eax                                   #4810.22
        addl      $1, %eax                                      #4811.17
        shrl      $16, %eax                                     #4811.17
        negl      %eax                                          #4811.17
        movl      %eax, -92(%ebp)                               #4811.17
        jmp       ..B10.9       # Prob 100%                     #4811.17
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.21:                       # Preds ..B10.11                # Infreq
        movl      -92(%ebp), %eax                               #
        movl      -12(%ebp), %edx                               #4842.30
        subl      %esi, %edx                                    #4842.30
        movl      %edx, -12(%ebp)                               #4842.30
        cmpl      $2, %edx                                      #4842.37
        je        ..B10.25      # Prob 25%                      #4842.37
                                # LOE eax edx al dl ah dh
..B10.22:                       # Preds ..B10.21                # Infreq
        cmpl      $1, %edx                                      #4842.37
        je        ..B10.26      # Prob 33%                      #4842.37
                                # LOE eax edx al dl ah dh
..B10.23:                       # Preds ..B10.22                # Infreq
        testl     %edx, %edx                                    #4842.37
        je        ..B10.27      # Prob 50%                      #4842.37
                                # LOE eax al ah
..B10.24:                       # Preds ..B10.25 ..B10.26 ..B10.27 ..B10.23 # Infreq
        movl      -20(%ebp), %esi                               #4845.32
        movl      -24(%ebp), %edi                               #4845.32
        movl      $1, %eax                                      #4845.32
        movl      %ebp, %esp                                    #4845.32
        popl      %ebp                                          #4845.32
        movl      %ebx, %esp                                    #4845.32
        popl      %ebx                                          #4845.32
        ret                                                     #4845.32
                                # LOE
..B10.25:                       # Preds ..B10.21                # Infreq
        movl      -100(%ebp), %edx                              #4847.36
        testl     %edx, %edx                                    #4847.36
        jne       ..B10.24      # Prob 28%                      #4847.36
                                # LOE eax al ah
..B10.26:                       # Preds ..B10.25 ..B10.22       # Infreq
        movl      -96(%ebp), %edx                               #4849.36
        testl     %edx, %edx                                    #4849.36
        jne       ..B10.24      # Prob 28%                      #4849.36
                                # LOE eax al ah
..B10.27:                       # Preds ..B10.23 ..B10.26       # Infreq
        testl     %eax, %eax                                    #4851.36
        jne       ..B10.24      # Prob 28%                      #4851.36
                                # LOE
..B10.28:                       # Preds ..B10.14 ..B10.12 ..B10.27 # Infreq
        movl      -20(%ebp), %esi                               #4853.28
        movl      -24(%ebp), %edi                               #4853.28
        xorl      %eax, %eax                                    #4853.28
        movl      %ebp, %esp                                    #4853.28
        popl      %ebp                                          #4853.28
        movl      %ebx, %esp                                    #4853.28
        popl      %ebx                                          #4853.28
        ret                                                     #4853.28
                                # LOE
..B10.32:                       # Preds ..B10.37                # Infreq
        movdqa    %xmm7, -88(%ebp)                              #
        movdqa    %xmm0, -72(%ebp)                              #
        movdqa    -40(%ebp), %xmm0                              #
        movdqa    %xmm6, -56(%ebp)                              #
        movl      %ecx, -4(%ebp)                                #
        jmp       ..B10.20      # Prob 100%                     #
        .align    2,0x90
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
# mark_end;
	.type	eval_4na_128,@function
	.size	eval_4na_128,.-eval_4na_128
.LNeval_4na_128:
	.data
# -- End  eval_4na_128
	.text
# -- Begin  eval_2na_pos
# mark_begin;
       .align    2,0x90
eval_2na_pos:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B11.1:                        # Preds ..B11.0
        pushl     %ebx                                          #3662.1
        movl      %esp, %ebx                                    #3662.1
        andl      $-16, %esp                                    #3662.1
        pushl     %ebp                                          #3662.1
        pushl     %ebp                                          #3662.1
        movl      4(%ebx), %ebp                                 #3662.1
        movl      %ebp, 4(%esp)                                 #3662.1
        movl      %esp, %ebp                                    #3662.1
        subl      $104, %esp                                    #3662.1
        movl      %ebx, -104(%ebp)                              #3662.1
        movl      %edi, -36(%ebp)                               #3662.1
        movl      %esi, -40(%ebp)                               #3662.1
        movl      16(%ebx), %esi                                #3660.5
        call      ..L11         # Prob 100%                     #3662.1
..L11:                                                          #
        popl      %eax                                          #3662.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L11], %eax      #3662.1
        lea       _gprof_pack10@GOTOFF(%eax), %edx              #3662.1
        movl      %eax, %ebx                                    #3662.1
        call      mcount@PLT                                    #3662.1
        movl      -104(%ebp), %ebx                              #3662.1
                                # LOE esi
..B11.60:                       # Preds ..B11.1
        movl      20(%ebx), %ecx                                #3705.5
        movl      12(%ebx), %eax                                #3713.30
        movl      %esi, -32(%ebp)                               #3662.1
        lea       (%esi,%ecx), %edx                             #3705.5
        movl      %esi, %edi                                    #3713.49
        shrl      $2, %edi                                      #3713.49
        movl      %edi, -12(%ebp)                               #3713.49
        lea       (%eax,%edi), %edi                             #3713.30
        movl      %edi, -16(%ebp)                               #3713.30
        lea       3(%esi,%ecx), %ecx                            #3719.50
        shrl      $2, %ecx                                      #3719.57
        movl      8(%ebx), %edi                                 #3716.12
        subl      4(%edi), %edx                                 #3716.12
        addl      %eax, %ecx                                    #3719.30
        movl      -16(%ebp), %eax                               #3722.14
        movl      %ecx, -20(%ebp)                               #3719.30
        movl      -12(%ebp), %ecx                               #3722.14
        testb     $15, %al                                      #3722.14
        jne       ..B11.3       # Prob 50%                      #3722.14
                                # LOE eax edx ecx esi al cl ah ch
..B11.2:                        # Preds ..B11.60
        movdqa    (%eax), %xmm0                                 #3722.14
        movdqa    %xmm0, -56(%ebp)                              #3722.14
        jmp       ..B11.4       # Prob 100%                     #3722.14
                                # LOE edx ecx esi cl ch
..B11.3:                        # Preds ..B11.60
        movdqu    (%eax), %xmm0                                 #3722.14
        movdqa    %xmm0, -56(%ebp)                              #3722.14
                                # LOE edx ecx esi cl ch
..B11.4:                        # Preds ..B11.2 ..B11.3
        movl      12(%ebx), %edi                                #3723.5
        movl      -20(%ebp), %eax                               #3728.16
        lea       16(%edi,%ecx), %edi                           #3723.5
        movl      %edi, -28(%ebp)                               #3723.5
        cmpl      %eax, %edi                                    #3728.16
        jae       ..B11.6       # Prob 12%                      #3728.16
                                # LOE edx esi edi
..B11.5:                        # Preds ..B11.4
        movzbl    -1(%edi), %ecx                                #3729.24
        shll      $8, %ecx                                      #3729.38
        jmp       ..B11.7       # Prob 100%                     #3729.38
                                # LOE edx ecx esi edi
..B11.6:                        # Preds ..B11.4
        xorl      %ecx, %ecx                                    #
                                # LOE edx ecx esi edi
..B11.7:                        # Preds ..B11.5 ..B11.6
        movl      8(%ebx), %eax                                 #3733.5
        movdqa    16(%eax), %xmm6                               #3733.5
        movdqa    32(%eax), %xmm5                               #3733.5
        movdqa    48(%eax), %xmm4                               #3733.5
        movdqa    64(%eax), %xmm3                               #3733.5
        movdqa    80(%eax), %xmm2                               #3733.5
        movdqa    96(%eax), %xmm1                               #3733.5
        movdqa    112(%eax), %xmm0                              #3733.5
        movdqa    128(%eax), %xmm7                              #3733.5
        movl      %edx, %eax                                    #3738.20
        subl      %esi, %eax                                    #3738.20
        addl      $7, %eax                                      #3738.33
        shrl      $2, %eax                                      #3738.40
        movl      %eax, -24(%ebp)                               #3738.40
        movl      %esi, %eax                                    #3745.20
        andl      $3, %eax                                      #3745.20
        je        ..B11.55      # Prob 20%                      #3745.20
                                # LOE eax edx ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B11.8:                        # Preds ..B11.7
        cmpl      $1, %eax                                      #3745.20
        je        ..B11.14      # Prob 25%                      #3745.20
                                # LOE eax edx ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B11.9:                        # Preds ..B11.8
        cmpl      $2, %eax                                      #3745.20
        je        ..B11.13      # Prob 33%                      #3745.20
                                # LOE eax edx ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B11.10:                       # Preds ..B11.9
        cmpl      $3, %eax                                      #3745.20
        je        ..B11.12      # Prob 50%                      #3745.20
                                # LOE edx ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B11.11:                       # Preds ..B11.10
        movdqa    %xmm7, -88(%ebp)                              #
        movdqa    %xmm0, -72(%ebp)                              #
        movdqa    -56(%ebp), %xmm0                              #
        movl      %edx, -92(%ebp)                               #
        jmp       ..B11.26      # Prob 100%                     #
                                # LOE ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.12:                       # Preds ..B11.10
        movdqa    %xmm7, -88(%ebp)                              #
        movdqa    %xmm0, -72(%ebp)                              #
        movdqa    -56(%ebp), %xmm0                              #
        movl      %edx, -92(%ebp)                               #
        xorl      %eax, %eax                                    #
        movl      %eax, -100(%ebp)                              #
        movl      %eax, -96(%ebp)                               #
        movl      %eax, -12(%ebp)                               #
        jmp       ..B11.17      # Prob 100%                     #
                                # LOE ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.13:                       # Preds ..B11.9
        movdqa    %xmm7, -88(%ebp)                              #
        movdqa    %xmm0, -72(%ebp)                              #
        movdqa    -56(%ebp), %xmm0                              #
        movl      %edx, -92(%ebp)                               #
        xorl      %eax, %eax                                    #
        movl      %eax, -96(%ebp)                               #
        movl      %eax, -12(%ebp)                               #
        jmp       ..B11.16      # Prob 100%                     #
                                # LOE ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.14:                       # Preds ..B11.8
        movdqa    %xmm7, -88(%ebp)                              #
        movdqa    %xmm0, -72(%ebp)                              #
        movdqa    -56(%ebp), %xmm0                              #
        movl      %edx, -92(%ebp)                               #
        xorl      %eax, %eax                                    #
        movl      %eax, -12(%ebp)                               #
                                # LOE ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.15:                       # Preds ..B11.14 ..B11.30
        movdqa    %xmm0, %xmm7                                  #3768.22
        pand      %xmm3, %xmm7                                  #3768.22
        pcmpeqd   %xmm4, %xmm7                                  #3769.22
        pmovmskb  %xmm7, %eax                                   #3770.22
        addl      $1, %eax                                      #3771.17
        shrl      $16, %eax                                     #3771.17
        negl      %eax                                          #3771.17
        movl      %eax, -96(%ebp)                               #3771.17
                                # LOE ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.16:                       # Preds ..B11.13 ..B11.15
        movdqa    %xmm0, %xmm7                                  #3774.22
        pand      %xmm1, %xmm7                                  #3774.22
        pcmpeqd   %xmm2, %xmm7                                  #3775.22
        pmovmskb  %xmm7, %eax                                   #3776.22
        addl      $1, %eax                                      #3777.17
        shrl      $16, %eax                                     #3777.17
        negl      %eax                                          #3777.17
        movl      %eax, -100(%ebp)                              #3777.17
                                # LOE ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.17:                       # Preds ..B11.12 ..B11.16
        movdqa    %xmm0, %xmm7                                  #3780.22
        movl      -12(%ebp), %edx                               #3790.29
        pand      -88(%ebp), %xmm7                              #3780.22
        pcmpeqd   -72(%ebp), %xmm7                              #3781.22
        pmovmskb  %xmm7, %eax                                   #3782.22
        andl      $-4, %esi                                     #3787.17
        orl       -96(%ebp), %edx                               #3790.29
        addl      $1, %eax                                      #3783.17
        orl       -100(%ebp), %edx                              #3790.34
        shrl      $16, %eax                                     #3783.17
        negl      %eax                                          #3783.17
        orl       %eax, %edx                                    #3790.39
        movl      %eax, -16(%ebp)                               #3783.17
        jne       ..B11.31      # Prob 1%                       #3790.47
                                # LOE eax ecx esi edi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.18:                       # Preds ..B11.17
        movl      -92(%ebp), %eax                               #3856.28
        addl      $4, %esi                                      #3853.17
        cmpl      %eax, %esi                                    #3856.28
        ja        ..B11.53      # Prob 1%                       #3856.28
                                # LOE ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.19:                       # Preds ..B11.18
        movl      -24(%ebp), %eax                               #3860.25
        addl      $-1, %eax                                     #3860.25
        movl      %eax, -24(%ebp)                               #3860.25
        jne       ..B11.27      # Prob 50%                      #3860.39
                                # LOE ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.20:                       # Preds ..B11.19
        movl      -20(%ebp), %eax                               #3881.25
        cmpl      %eax, %edi                                    #3881.25
        jae       ..B11.53      # Prob 1%                       #3881.25
                                # LOE eax ecx esi edi al ah xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.21:                       # Preds ..B11.20
        testl     $15, %edi                                     #3891.22
        jne       ..B11.23      # Prob 50%                      #3891.22
                                # LOE eax ecx esi edi al ah xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.22:                       # Preds ..B11.21
        movdqa    (%edi), %xmm0                                 #3891.22
        jmp       ..B11.24      # Prob 100%                     #3891.22
                                # LOE eax ecx esi edi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.23:                       # Preds ..B11.21
        movdqu    (%edi), %xmm0                                 #3891.22
                                # LOE eax ecx esi edi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.24:                       # Preds ..B11.22 ..B11.23
        addl      $16, %edi                                     #3969.13
        movl      %edi, -28(%ebp)                               #3969.13
        cmpl      %eax, %edi                                    #3972.24
        jae       ..B11.26      # Prob 12%                      #3972.24
                                # LOE ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.25:                       # Preds ..B11.24
        movzbl    -1(%edi), %ecx                                #3973.32
        shll      $8, %ecx                                      #3973.46
                                # LOE ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.26:                       # Preds ..B11.24 ..B11.11 ..B11.25
        movl      $16, %eax                                     #3752.13
        movl      %eax, -24(%ebp)                               #3752.13
        jmp       ..B11.30      # Prob 100%                     #3752.13
                                # LOE ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.27:                       # Preds ..B11.19
        movl      -20(%ebp), %edx                               #3868.26
        movl      -28(%ebp), %eax                               #3868.26
        psrldq    $1, %xmm0                                     #3865.26
        cmpl      %edx, %eax                                    #3868.26
        jae       ..B11.29      # Prob 19%                      #3868.26
                                # LOE eax ecx esi edi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.28:                       # Preds ..B11.27
        movzbl    (%eax), %edx                                  #3871.37
        sarl      $8, %ecx                                      #3870.21
        shll      $8, %edx                                      #3871.48
        orl       %edx, %ecx                                    #3871.21
        pinsrw    $7, %ecx, %xmm0                               #3872.30
                                # LOE ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.29:                       # Preds ..B11.28 ..B11.27
        addl      $1, -28(%ebp)                                 #3876.20
                                # LOE ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.30:                       # Preds ..B11.26 ..B11.29 ..B11.55
        movdqa    %xmm0, %xmm7                                  #3762.22
        pand      %xmm5, %xmm7                                  #3762.22
        pcmpeqd   %xmm6, %xmm7                                  #3763.22
        pmovmskb  %xmm7, %eax                                   #3764.22
        addl      $1, %eax                                      #3765.17
        shrl      $16, %eax                                     #3765.17
        negl      %eax                                          #3765.17
        movl      %eax, -12(%ebp)                               #3765.17
        jmp       ..B11.15      # Prob 100%                     #3765.17
                                # LOE ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.31:                       # Preds ..B11.17                # Infreq
        movl      -92(%ebp), %edx                               #
        movl      %eax, %ecx                                    #
        movl      -12(%ebp), %eax                               #
        subl      %esi, %edx                                    #3793.30
        je        ..B11.52      # Prob 25%                      #3793.37
                                # LOE eax edx ecx esi al cl ah ch
..B11.32:                       # Preds ..B11.31                # Infreq
        cmpl      $1, %edx                                      #3793.37
        jne       ..B11.37      # Prob 67%                      #3793.37
                                # LOE eax edx ecx esi al cl ah ch
..B11.33:                       # Preds ..B11.32                # Infreq
        testl     %eax, %eax                                    #3799.36
        jne       ..B11.36      # Prob 28%                      #3799.36
                                # LOE esi
..B11.34:                       # Preds ..B11.33                # Infreq
        movl      -96(%ebp), %eax                               #3800.36
        testl     %eax, %eax                                    #3800.36
        je        ..B11.53      # Prob 72%                      #3800.36
                                # LOE esi
..B11.35:                       # Preds ..B11.45 ..B11.39 ..B11.34 # Infreq
        subl      -32(%ebp), %esi                               #3800.47
        movl      -36(%ebp), %edi                               #3800.61
        addl      $2, %esi                                      #3800.61
        movl      %esi, %eax                                    #3800.61
        movl      -40(%ebp), %esi                               #3800.61
        movl      %ebp, %esp                                    #3800.61
        popl      %ebp                                          #3800.61
        movl      %ebx, %esp                                    #3800.61
        popl      %ebx                                          #3800.61
        ret                                                     #3800.61
                                # LOE
..B11.36:                       # Preds ..B11.52 ..B11.44 ..B11.38 ..B11.33 # Infreq
        subl      -32(%ebp), %esi                               #3799.47
        movl      -36(%ebp), %edi                               #3799.61
        addl      $1, %esi                                      #3799.61
        movl      %esi, %eax                                    #3799.61
        movl      -40(%ebp), %esi                               #3799.61
        movl      %ebp, %esp                                    #3799.61
        popl      %ebp                                          #3799.61
        movl      %ebx, %esp                                    #3799.61
        popl      %ebx                                          #3799.61
        ret                                                     #3799.61
                                # LOE
..B11.37:                       # Preds ..B11.32                # Infreq
        cmpl      $2, %edx                                      #3793.37
        jne       ..B11.44      # Prob 50%                      #3793.37
                                # LOE eax ecx esi al cl ah ch
..B11.38:                       # Preds ..B11.37                # Infreq
        testl     %eax, %eax                                    #3803.36
        jne       ..B11.36      # Prob 28%                      #3803.36
                                # LOE esi
..B11.39:                       # Preds ..B11.38                # Infreq
        movl      -96(%ebp), %eax                               #3804.36
        testl     %eax, %eax                                    #3804.36
        jne       ..B11.35      # Prob 28%                      #3804.36
                                # LOE esi
..B11.40:                       # Preds ..B11.39                # Infreq
        movl      -100(%ebp), %eax                              #3805.36
        testl     %eax, %eax                                    #3805.36
        je        ..B11.53      # Prob 72%                      #3805.36
                                # LOE esi
..B11.41:                       # Preds ..B11.46 ..B11.40       # Infreq
        subl      -32(%ebp), %esi                               #3805.47
        movl      -36(%ebp), %edi                               #3805.61
        addl      $3, %esi                                      #3805.61
        movl      %esi, %eax                                    #3805.61
        movl      -40(%ebp), %esi                               #3805.61
        movl      %ebp, %esp                                    #3805.61
        popl      %ebp                                          #3805.61
        movl      %ebx, %esp                                    #3805.61
        popl      %ebx                                          #3805.61
        ret                                                     #3805.61
                                # LOE
..B11.44:                       # Preds ..B11.37                # Infreq
        testl     %eax, %eax                                    #3808.36
        jne       ..B11.36      # Prob 28%                      #3808.36
                                # LOE ecx esi cl ch
..B11.45:                       # Preds ..B11.44                # Infreq
        movl      -96(%ebp), %eax                               #3809.36
        testl     %eax, %eax                                    #3809.36
        jne       ..B11.35      # Prob 28%                      #3809.36
                                # LOE ecx esi cl ch
..B11.46:                       # Preds ..B11.45                # Infreq
        movl      -100(%ebp), %eax                              #3810.36
        testl     %eax, %eax                                    #3810.36
        jne       ..B11.41      # Prob 28%                      #3810.36
                                # LOE ecx esi cl ch
..B11.47:                       # Preds ..B11.46                # Infreq
        testl     %ecx, %ecx                                    #3811.36
        je        ..B11.53      # Prob 72%                      #3811.36
                                # LOE esi
..B11.48:                       # Preds ..B11.47                # Infreq
        subl      -32(%ebp), %esi                               #3811.47
        movl      -36(%ebp), %edi                               #3811.61
        addl      $4, %esi                                      #3811.61
        movl      %esi, %eax                                    #3811.61
        movl      -40(%ebp), %esi                               #3811.61
        movl      %ebp, %esp                                    #3811.61
        popl      %ebp                                          #3811.61
        movl      %ebx, %esp                                    #3811.61
        popl      %ebx                                          #3811.61
        ret                                                     #3811.61
                                # LOE
..B11.52:                       # Preds ..B11.31                # Infreq
        testl     %eax, %eax                                    #3796.36
        jne       ..B11.36      # Prob 28%                      #3796.36
                                # LOE esi
..B11.53:                       # Preds ..B11.20 ..B11.18 ..B11.52 ..B11.34 ..B11.40
                                #       ..B11.47                # Infreq
        movl      -40(%ebp), %esi                               #3813.28
        movl      -36(%ebp), %edi                               #3813.28
        xorl      %eax, %eax                                    #3813.28
        movl      %ebp, %esp                                    #3813.28
        popl      %ebp                                          #3813.28
        movl      %ebx, %esp                                    #3813.28
        popl      %ebx                                          #3813.28
        ret                                                     #3813.28
                                # LOE
..B11.55:                       # Preds ..B11.7                 # Infreq
        movdqa    %xmm7, -88(%ebp)                              #
        movdqa    %xmm0, -72(%ebp)                              #
        movdqa    -56(%ebp), %xmm0                              #
        movl      %edx, -92(%ebp)                               #
        jmp       ..B11.30      # Prob 100%                     #
        .align    2,0x90
                                # LOE ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
# mark_end;
	.type	eval_2na_pos,@function
	.size	eval_2na_pos,.-eval_2na_pos
.LNeval_2na_pos:
	.data
# -- End  eval_2na_pos
	.text
# -- Begin  eval_4na_pos
# mark_begin;
       .align    2,0x90
eval_4na_pos:
# parameter 1(self): 8 + %ebx
# parameter 2(ncbi2na): 12 + %ebx
# parameter 3(pos): 16 + %ebx
# parameter 4(len): 20 + %ebx
..B12.1:                        # Preds ..B12.0
        pushl     %ebx                                          #4946.1
        movl      %esp, %ebx                                    #4946.1
        andl      $-16, %esp                                    #4946.1
        pushl     %ebp                                          #4946.1
        pushl     %ebp                                          #4946.1
        movl      4(%ebx), %ebp                                 #4946.1
        movl      %ebp, 4(%esp)                                 #4946.1
        movl      %esp, %ebp                                    #4946.1
        subl      $136, %esp                                    #4946.1
        movl      %ebx, -136(%ebp)                              #4946.1
        movl      %edi, -32(%ebp)                               #4946.1
        movl      %esi, -36(%ebp)                               #4946.1
        movl      12(%ebx), %ecx                                #4944.5
        movl      16(%ebx), %esi                                #4944.5
        call      ..L12         # Prob 100%                     #4946.1
..L12:                                                          #
        popl      %edi                                          #4946.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L12], %edi      #4946.1
        lea       _gprof_pack11@GOTOFF(%edi), %edx              #4946.1
        movl      %edi, %ebx                                    #4946.1
        call      mcount@PLT                                    #4946.1
        movl      -136(%ebp), %ebx                              #4946.1
                                # LOE esi edi
..B12.50:                       # Preds ..B12.1
        movl      12(%ebx), %ecx                                #
        movl      %esi, -116(%ebp)                              #
        movl      %esi, %eax                                    #4993.49
        movl      %edi, -112(%ebp)                              #
        movl      20(%ebx), %edi                                #4985.5
        movl      %esi, -28(%ebp)                               #4946.1
        lea       (%esi,%edi), %edx                             #4985.5
        shrl      $2, %eax                                      #4993.49
        lea       3(%esi,%edi), %esi                            #4999.50
        movl      -112(%ebp), %edi                              #5002.14
        movl      %eax, -12(%ebp)                               #4993.49
        movl      8(%ebx), %eax                                 #4996.12
        movdqa    16(%eax), %xmm5                               #5009.5
        movdqa    32(%eax), %xmm4                               #5009.5
        subl      4(%eax), %edx                                 #4996.12
        movdqa    48(%eax), %xmm3                               #5009.5
        movdqa    64(%eax), %xmm2                               #5009.5
        movdqa    80(%eax), %xmm1                               #5009.5
        movdqa    96(%eax), %xmm6                               #5009.5
        movdqa    128(%eax), %xmm7                              #5009.5
        movl      %edx, -20(%ebp)                               #4996.12
        movl      -12(%ebp), %edx                               #5002.14
        shrl      $2, %esi                                      #4999.57
        addl      %ecx, %esi                                    #4999.30
        movl      %esi, -24(%ebp)                               #4999.30
        movzbl    (%edx,%ecx), %esi                             #5002.14
        movzwl    expand_2na.0@GOTOFF(%edi,%esi,2), %esi        #5002.14
        movw      %si, -132(%ebp)                               #5002.14
        movzbl    1(%edx,%ecx), %esi                            #5002.14
        movzwl    expand_2na.0@GOTOFF(%edi,%esi,2), %esi        #5002.14
        movw      %si, -130(%ebp)                               #5002.14
        movzbl    2(%edx,%ecx), %esi                            #5002.14
        movzwl    expand_2na.0@GOTOFF(%edi,%esi,2), %esi        #5002.14
        movw      %si, -128(%ebp)                               #5002.14
        movzbl    3(%edx,%ecx), %esi                            #5002.14
        movzwl    expand_2na.0@GOTOFF(%edi,%esi,2), %esi        #5002.14
        movw      %si, -126(%ebp)                               #5002.14
        movzbl    4(%edx,%ecx), %esi                            #5002.14
        movzwl    expand_2na.0@GOTOFF(%edi,%esi,2), %esi        #5002.14
        movw      %si, -124(%ebp)                               #5002.14
        movzbl    5(%edx,%ecx), %esi                            #5002.14
        movzwl    expand_2na.0@GOTOFF(%edi,%esi,2), %esi        #5002.14
        movw      %si, -122(%ebp)                               #5002.14
        movzbl    6(%edx,%ecx), %esi                            #5002.14
        movzwl    expand_2na.0@GOTOFF(%edi,%esi,2), %esi        #5002.14
        movw      %si, -120(%ebp)                               #5002.14
        movzbl    7(%edx,%ecx), %esi                            #5002.14
        movzwl    expand_2na.0@GOTOFF(%edi,%esi,2), %esi        #5002.14
        lea       8(%ecx,%edx), %edx                            #5003.5
        movl      -20(%ebp), %ecx                               #5014.20
        movw      %si, -118(%ebp)                               #5002.14
        movdqu    -132(%ebp), %xmm0                             #5002.14
        movl      -116(%ebp), %esi                              #5014.20
        movdqa    %xmm0, -56(%ebp)                              #5002.14
        movdqa    112(%eax), %xmm0                              #5009.5
        movl      %edx, -16(%ebp)                               #5003.5
        subl      %esi, %ecx                                    #5014.20
        addl      $7, %ecx                                      #5014.33
        shrl      $2, %ecx                                      #5014.40
        movl      %esi, %eax                                    #5023.20
        andl      $3, %eax                                      #5023.20
        je        ..B12.45      # Prob 20%                      #5023.20
                                # LOE eax edx ecx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B12.2:                        # Preds ..B12.50
        cmpl      $1, %eax                                      #5023.20
        je        ..B12.8       # Prob 25%                      #5023.20
                                # LOE eax edx ecx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B12.3:                        # Preds ..B12.2
        cmpl      $2, %eax                                      #5023.20
        je        ..B12.7       # Prob 33%                      #5023.20
                                # LOE eax edx ecx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B12.4:                        # Preds ..B12.3
        cmpl      $3, %eax                                      #5023.20
        je        ..B12.6       # Prob 50%                      #5023.20
                                # LOE edx ecx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B12.5:                        # Preds ..B12.4
        movdqa    %xmm7, -104(%ebp)                             #
        movdqa    %xmm0, -88(%ebp)                              #
        movdqa    -56(%ebp), %xmm0                              #
        movdqa    %xmm6, -72(%ebp)                              #
        jmp       ..B12.16      # Prob 100%                     #
                                # LOE edx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.6:                        # Preds ..B12.4
        movdqa    %xmm7, -104(%ebp)                             #
        movdqa    %xmm0, -88(%ebp)                              #
        movdqa    -56(%ebp), %xmm0                              #
        movdqa    %xmm6, -72(%ebp)                              #
        xorl      %eax, %eax                                    #
        movl      %eax, -116(%ebp)                              #
        movl      %ecx, -12(%ebp)                               #
        movl      %eax, -112(%ebp)                              #
        movl      %eax, -40(%ebp)                               #
        jmp       ..B12.11      # Prob 100%                     #
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.7:                        # Preds ..B12.3
        movdqa    %xmm7, -104(%ebp)                             #
        movdqa    %xmm0, -88(%ebp)                              #
        movdqa    -56(%ebp), %xmm0                              #
        movdqa    %xmm6, -72(%ebp)                              #
        xorl      %eax, %eax                                    #
        movl      %eax, -112(%ebp)                              #
        movl      %ecx, -12(%ebp)                               #
        movl      %eax, -40(%ebp)                               #
        jmp       ..B12.10      # Prob 100%                     #
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.8:                        # Preds ..B12.2
        movdqa    %xmm7, -104(%ebp)                             #
        movdqa    %xmm0, -88(%ebp)                              #
        movdqa    -56(%ebp), %xmm0                              #
        movdqa    %xmm6, -72(%ebp)                              #
        xorl      %eax, %eax                                    #
        movl      %ecx, -12(%ebp)                               #
        movl      %eax, -40(%ebp)                               #
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.9:                        # Preds ..B12.8 ..B12.20
        movdqa    %xmm0, %xmm7                                  #5047.22
        movdqa    %xmm0, %xmm6                                  #5048.22
        pand      %xmm3, %xmm7                                  #5047.22
        pand      %xmm2, %xmm6                                  #5048.22
        pcmpeqd   %xmm6, %xmm7                                  #5049.22
        pmovmskb  %xmm7, %eax                                   #5050.22
        addl      $1, %eax                                      #5051.17
        shrl      $16, %eax                                     #5051.17
        negl      %eax                                          #5051.17
        movl      %eax, -112(%ebp)                              #5051.17
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.10:                       # Preds ..B12.7 ..B12.9
        movdqa    %xmm0, %xmm7                                  #5054.22
        movdqa    %xmm0, %xmm6                                  #5055.22
        pand      -72(%ebp), %xmm6                              #5055.22
        pand      %xmm1, %xmm7                                  #5054.22
        pcmpeqd   %xmm6, %xmm7                                  #5056.22
        pmovmskb  %xmm7, %eax                                   #5057.22
        addl      $1, %eax                                      #5058.17
        shrl      $16, %eax                                     #5058.17
        negl      %eax                                          #5058.17
        movl      %eax, -116(%ebp)                              #5058.17
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.11:                       # Preds ..B12.6 ..B12.10
        movdqa    %xmm0, %xmm7                                  #5061.22
        movdqa    %xmm0, %xmm6                                  #5062.22
        movl      -40(%ebp), %ecx                               #5072.29
        pand      -88(%ebp), %xmm7                              #5061.22
        pand      -104(%ebp), %xmm6                             #5062.22
        pcmpeqd   %xmm6, %xmm7                                  #5063.22
        pmovmskb  %xmm7, %eax                                   #5064.22
        andl      $-4, %esi                                     #5069.17
        addl      $1, %eax                                      #5065.17
        orl       -112(%ebp), %ecx                              #5072.29
        orl       -116(%ebp), %ecx                              #5072.34
        shrl      $16, %eax                                     #5065.17
        negl      %eax                                          #5065.17
        movl      %eax, -108(%ebp)                              #5065.17
        orl       %eax, %ecx                                    #5072.39
        jne       ..B12.21      # Prob 1%                       #5072.47
                                # LOE eax edx esi edi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.12:                       # Preds ..B12.11
        movl      -20(%ebp), %eax                               #5138.28
        addl      $4, %esi                                      #5135.17
        cmpl      %eax, %esi                                    #5138.28
        ja        ..B12.43      # Prob 1%                       #5138.28
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.13:                       # Preds ..B12.12
        movl      -12(%ebp), %eax                               #5142.25
        addl      $-1, %eax                                     #5142.25
        movl      %eax, -12(%ebp)                               #5142.25
        jne       ..B12.17      # Prob 50%                      #5142.39
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.14:                       # Preds ..B12.13
        movl      -24(%ebp), %eax                               #5159.25
        cmpl      %eax, %edx                                    #5159.25
        jae       ..B12.43      # Prob 1%                       #5159.25
                                # LOE edx esi edi xmm1 xmm2 xmm3 xmm4 xmm5
..B12.15:                       # Preds ..B12.14
        movzbl    (%edx), %eax                                  #5169.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %ecx        #5169.22
        movzbl    1(%edx), %eax                                 #5169.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #5169.22
        movw      %cx, -132(%ebp)                               #5169.22
        movw      %ax, -130(%ebp)                               #5169.22
        movzbl    2(%edx), %eax                                 #5169.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #5169.22
        movw      %ax, -128(%ebp)                               #5169.22
        movzbl    3(%edx), %eax                                 #5169.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #5169.22
        movw      %ax, -126(%ebp)                               #5169.22
        movzbl    4(%edx), %eax                                 #5169.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #5169.22
        movw      %ax, -124(%ebp)                               #5169.22
        movzbl    5(%edx), %eax                                 #5169.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #5169.22
        movw      %ax, -122(%ebp)                               #5169.22
        movzbl    6(%edx), %eax                                 #5169.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #5169.22
        movw      %ax, -120(%ebp)                               #5169.22
        movzbl    7(%edx), %eax                                 #5169.22
        movzwl    expand_2na.0@GOTOFF(%edi,%eax,2), %eax        #5169.22
        addl      $8, %edx                                      #5184.13
        movw      %ax, -118(%ebp)                               #5169.22
        movdqu    -132(%ebp), %xmm0                             #5169.22
        movl      %edx, -16(%ebp)                               #5184.13
                                # LOE edx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.16:                       # Preds ..B12.5 ..B12.15
        movl      $8, %eax                                      #5030.13
        movl      %eax, -12(%ebp)                               #5030.13
        jmp       ..B12.20      # Prob 100%                     #5030.13
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.17:                       # Preds ..B12.13
        movl      -24(%ebp), %ecx                               #5150.26
        movl      -16(%ebp), %eax                               #5150.26
        psrldq    $2, %xmm0                                     #5147.26
        cmpl      %ecx, %eax                                    #5150.26
        jae       ..B12.19      # Prob 19%                      #5150.26
                                # LOE eax edx esi edi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.18:                       # Preds ..B12.17
        movzbl    (%eax), %ecx                                  #5151.72
        movzwl    expand_2na.0@GOTOFF(%edi,%ecx,2), %eax        #5151.57
        pinsrw    $7, %eax, %xmm0                               #5151.30
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.19:                       # Preds ..B12.18 ..B12.17
        addl      $1, -16(%ebp)                                 #5154.20
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.20:                       # Preds ..B12.16 ..B12.19 ..B12.45
        movdqa    %xmm0, %xmm7                                  #5040.22
        movdqa    %xmm0, %xmm6                                  #5041.22
        pand      %xmm5, %xmm7                                  #5040.22
        pand      %xmm4, %xmm6                                  #5041.22
        pcmpeqd   %xmm6, %xmm7                                  #5042.22
        pmovmskb  %xmm7, %eax                                   #5043.22
        addl      $1, %eax                                      #5044.17
        shrl      $16, %eax                                     #5044.17
        negl      %eax                                          #5044.17
        movl      %eax, -40(%ebp)                               #5044.17
        jmp       ..B12.9       # Prob 100%                     #5044.17
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.21:                       # Preds ..B12.11                # Infreq
        movl      %eax, %edx                                    #
        movl      -40(%ebp), %eax                               #
        movl      -20(%ebp), %ecx                               #5075.30
        subl      %esi, %ecx                                    #5075.30
        movl      %ecx, -20(%ebp)                               #5075.30
        je        ..B12.42      # Prob 25%                      #5075.37
                                # LOE eax edx ecx esi al dl cl ah dh ch
..B12.22:                       # Preds ..B12.21                # Infreq
        cmpl      $1, %ecx                                      #5075.37
        jne       ..B12.27      # Prob 67%                      #5075.37
                                # LOE eax edx ecx esi al dl cl ah dh ch
..B12.23:                       # Preds ..B12.22                # Infreq
        testl     %eax, %eax                                    #5081.36
        jne       ..B12.26      # Prob 28%                      #5081.36
                                # LOE esi
..B12.24:                       # Preds ..B12.23                # Infreq
        movl      -112(%ebp), %eax                              #5082.36
        testl     %eax, %eax                                    #5082.36
        je        ..B12.43      # Prob 72%                      #5082.36
                                # LOE esi
..B12.25:                       # Preds ..B12.35 ..B12.29 ..B12.24 # Infreq
        subl      -28(%ebp), %esi                               #5082.47
        movl      -32(%ebp), %edi                               #5082.61
        addl      $2, %esi                                      #5082.61
        movl      %esi, %eax                                    #5082.61
        movl      -36(%ebp), %esi                               #5082.61
        movl      %ebp, %esp                                    #5082.61
        popl      %ebp                                          #5082.61
        movl      %ebx, %esp                                    #5082.61
        popl      %ebx                                          #5082.61
        ret                                                     #5082.61
                                # LOE
..B12.26:                       # Preds ..B12.42 ..B12.34 ..B12.28 ..B12.23 # Infreq
        subl      -28(%ebp), %esi                               #5081.47
        movl      -32(%ebp), %edi                               #5081.61
        addl      $1, %esi                                      #5081.61
        movl      %esi, %eax                                    #5081.61
        movl      -36(%ebp), %esi                               #5081.61
        movl      %ebp, %esp                                    #5081.61
        popl      %ebp                                          #5081.61
        movl      %ebx, %esp                                    #5081.61
        popl      %ebx                                          #5081.61
        ret                                                     #5081.61
                                # LOE
..B12.27:                       # Preds ..B12.22                # Infreq
        cmpl      $2, %ecx                                      #5075.37
        jne       ..B12.34      # Prob 50%                      #5075.37
                                # LOE eax edx esi al dl ah dh
..B12.28:                       # Preds ..B12.27                # Infreq
        testl     %eax, %eax                                    #5085.36
        jne       ..B12.26      # Prob 28%                      #5085.36
                                # LOE esi
..B12.29:                       # Preds ..B12.28                # Infreq
        movl      -112(%ebp), %eax                              #5086.36
        testl     %eax, %eax                                    #5086.36
        jne       ..B12.25      # Prob 28%                      #5086.36
                                # LOE esi
..B12.30:                       # Preds ..B12.29                # Infreq
        movl      -116(%ebp), %eax                              #5087.36
        testl     %eax, %eax                                    #5087.36
        je        ..B12.43      # Prob 72%                      #5087.36
                                # LOE esi
..B12.31:                       # Preds ..B12.36 ..B12.30       # Infreq
        subl      -28(%ebp), %esi                               #5087.47
        movl      -32(%ebp), %edi                               #5087.61
        addl      $3, %esi                                      #5087.61
        movl      %esi, %eax                                    #5087.61
        movl      -36(%ebp), %esi                               #5087.61
        movl      %ebp, %esp                                    #5087.61
        popl      %ebp                                          #5087.61
        movl      %ebx, %esp                                    #5087.61
        popl      %ebx                                          #5087.61
        ret                                                     #5087.61
                                # LOE
..B12.34:                       # Preds ..B12.27                # Infreq
        testl     %eax, %eax                                    #5090.36
        jne       ..B12.26      # Prob 28%                      #5090.36
                                # LOE edx esi dl dh
..B12.35:                       # Preds ..B12.34                # Infreq
        movl      -112(%ebp), %eax                              #5091.36
        testl     %eax, %eax                                    #5091.36
        jne       ..B12.25      # Prob 28%                      #5091.36
                                # LOE edx esi dl dh
..B12.36:                       # Preds ..B12.35                # Infreq
        movl      -116(%ebp), %eax                              #5092.36
        testl     %eax, %eax                                    #5092.36
        jne       ..B12.31      # Prob 28%                      #5092.36
                                # LOE edx esi dl dh
..B12.37:                       # Preds ..B12.36                # Infreq
        testl     %edx, %edx                                    #5093.36
        je        ..B12.43      # Prob 72%                      #5093.36
                                # LOE esi
..B12.38:                       # Preds ..B12.37                # Infreq
        subl      -28(%ebp), %esi                               #5093.47
        movl      -32(%ebp), %edi                               #5093.61
        addl      $4, %esi                                      #5093.61
        movl      %esi, %eax                                    #5093.61
        movl      -36(%ebp), %esi                               #5093.61
        movl      %ebp, %esp                                    #5093.61
        popl      %ebp                                          #5093.61
        movl      %ebx, %esp                                    #5093.61
        popl      %ebx                                          #5093.61
        ret                                                     #5093.61
                                # LOE
..B12.42:                       # Preds ..B12.21                # Infreq
        testl     %eax, %eax                                    #5078.36
        jne       ..B12.26      # Prob 28%                      #5078.36
                                # LOE esi
..B12.43:                       # Preds ..B12.14 ..B12.12 ..B12.42 ..B12.24 ..B12.30
                                #       ..B12.37                # Infreq
        movl      -36(%ebp), %esi                               #5095.28
        movl      -32(%ebp), %edi                               #5095.28
        xorl      %eax, %eax                                    #5095.28
        movl      %ebp, %esp                                    #5095.28
        popl      %ebp                                          #5095.28
        movl      %ebx, %esp                                    #5095.28
        popl      %ebx                                          #5095.28
        ret                                                     #5095.28
                                # LOE
..B12.45:                       # Preds ..B12.50                # Infreq
        movdqa    %xmm7, -104(%ebp)                             #
        movdqa    %xmm0, -88(%ebp)                              #
        movdqa    -56(%ebp), %xmm0                              #
        movdqa    %xmm6, -72(%ebp)                              #
        movl      %ecx, -12(%ebp)                               #
        jmp       ..B12.20      # Prob 100%                     #
        .align    2,0x90
                                # LOE edx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
# mark_end;
	.type	eval_4na_pos,@function
	.size	eval_4na_pos,.-eval_4na_pos
.LNeval_4na_pos:
	.data
# -- End  eval_4na_pos
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
..B13.1:                        # Preds ..B13.0
        pushl     %ebp                                          #1539.1
        movl      %esp, %ebp                                    #1539.1
        subl      $16, %esp                                     #1539.1
        movl      %edi, -8(%ebp)                                #1539.1
        movl      %esi, -4(%ebp)                                #1539.1
        movl      %ebx, -12(%ebp)                               #1539.1
        movl      8(%ebp), %esi                                 #1537.5
        call      ..L13         # Prob 100%                     #1539.1
..L13:                                                          #
        popl      %ebx                                          #1539.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L13], %ebx      #1539.1
        lea       _gprof_pack12@GOTOFF(%ebx), %edx              #1539.1
        call      mcount@PLT                                    #1539.1
                                # LOE ebx esi
..B13.17:                       # Preds ..B13.1
        testl     %esi, %esi                                    #1540.17
        je        ..B13.12      # Prob 6%                       #1540.17
                                # LOE ebx esi
..B13.2:                        # Preds ..B13.17
        movl      16(%ebp), %eax                                #1542.23
        testl     %eax, %eax                                    #1542.23
        je        ..B13.11      # Prob 12%                      #1542.23
                                # LOE ebx esi
..B13.3:                        # Preds ..B13.2
        movl      20(%ebp), %eax                                #1542.38
        testl     %eax, %eax                                    #1542.38
        je        ..B13.11      # Prob 50%                      #1542.38
                                # LOE ebx esi
..B13.4:                        # Preds ..B13.3
        movsbl    fasta_2na_map.0@GOTOFF(%ebx), %eax            #1547.18
        movl      $0, -16(%ebp)                                 #1544.24
        testl     %eax, %eax                                    #1547.41
        je        ..B13.13      # Prob 5%                       #1547.41
                                # LOE ebx esi
..B13.5:                        # Preds ..B13.13 ..B13.4
        pushl     12(%ebp)                                      #1539.1
        movl      20(%ebp), %edi                                #1550.19
        movl      16(%ebp), %edx                                #1550.19
        lea       -16(%ebp), %eax                               #1551.49
        pushl     %eax                                          #1539.1
        pushl     %esi                                          #1539.1
        lea       (%edi,%edx), %edi                             #1550.19
        pushl     %edi                                          #1539.1
        pushl     %edx                                          #1539.1
        call      nss_expr                                      #1551.21
                                # LOE eax ebx esi edi
..B13.18:                       # Preds ..B13.5
        addl      $20, %esp                                     #1551.21
                                # LOE eax ebx esi edi
..B13.6:                        # Preds ..B13.18
        movl      -16(%ebp), %edx                               #1552.18
        testl     %edx, %edx                                    #1552.28
        jne       ..B13.9       # Prob 22%                      #1552.28
                                # LOE eax ebx esi edi
..B13.7:                        # Preds ..B13.6
        cmpl      %edi, %eax                                    #1554.31
        je        ..B13.14      # Prob 5%                       #1554.31
                                # LOE ebx esi
..B13.8:                        # Preds ..B13.7
        movl      $22, -16(%ebp)                                #1557.17
                                # LOE ebx esi
..B13.9:                        # Preds ..B13.8 ..B13.6
        pushl     (%esi)                                        #1560.32
        call      NucStrstrWhack@PLT                            #1560.13
                                # LOE esi
..B13.19:                       # Preds ..B13.9
        popl      %ecx                                          #1560.13
                                # LOE esi
..B13.10:                       # Preds ..B13.19
        movl      -16(%ebp), %eax                               #1562.20
        movl      -12(%ebp), %ebx                               #1562.20
        movl      -8(%ebp), %edi                                #1562.20
        movl      $0, (%esi)                                    #1561.13
        movl      -4(%ebp), %esi                                #1562.20
        movl      %ebp, %esp                                    #1562.20
        popl      %ebp                                          #1562.20
        ret                                                     #1562.20
                                # LOE
..B13.11:                       # Preds ..B13.3 ..B13.2
        movl      $0, (%esi)                                    #1565.9
                                # LOE
..B13.12:                       # Preds ..B13.17 ..B13.11
        movl      -12(%ebp), %ebx                               #1567.12
        movl      -4(%ebp), %esi                                #1567.12
        movl      -8(%ebp), %edi                                #1567.12
        movl      $22, %eax                                     #1567.12
        movl      %ebp, %esp                                    #1567.12
        popl      %ebp                                          #1567.12
        ret                                                     #1567.12
                                # LOE
..B13.13:                       # Preds ..B13.4                 # Infreq
        call      NucStrstrInit                                 #1548.17
        jmp       ..B13.5       # Prob 100%                     #1548.17
                                # LOE ebx esi
..B13.14:                       # Preds ..B13.7                 # Infreq
        movl      -12(%ebp), %ebx                               #1555.28
        movl      -4(%ebp), %esi                                #1555.28
        movl      -8(%ebp), %edi                                #1555.28
        xorl      %eax, %eax                                    #1555.28
        movl      %ebp, %esp                                    #1555.28
        popl      %ebp                                          #1555.28
        ret                                                     #1555.28
        .align    2,0x90
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
..B14.1:                        # Preds ..B14.0
        pushl     %ebp                                          #1574.1
        movl      %esp, %ebp                                    #1574.1
        subl      $8, %esp                                      #1574.1
        movl      %edi, -4(%ebp)                                #1574.1
        movl      %ebx, -8(%ebp)                                #1574.1
        movl      8(%ebp), %edi                                 #1573.6
        call      ..L14         # Prob 100%                     #1574.1
..L14:                                                          #
        popl      %ebx                                          #1574.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L14], %ebx      #1574.1
        lea       _gprof_pack13@GOTOFF(%ebx), %edx              #1574.1
        call      mcount@PLT                                    #1574.1
                                # LOE ebx esi edi
..B14.25:                       # Preds ..B14.1
        testl     %edi, %edi                                    #1575.18
        je        ..B14.22      # Prob 12%                      #1575.18
                                # LOE ebx esi edi
..B14.2:                        # Preds ..B14.25
        movl      (%edi), %eax                                  #1577.18
        cmpl      $12, %eax                                     #1577.9
        ja        ..B14.21      # Prob 50%                      #1577.9
                                # LOE eax ebx esi edi
..B14.3:                        # Preds ..B14.2
        movl      ..1..TPKT.18_0.0.15@GOTOFF(%ebx,%eax,4), %eax #1577.9
        jmp       *%eax                                         #1577.9
                                # LOE ebx esi edi
..1.18_0.TAG.0c.0.15:
..B14.5:                        # Preds ..B14.3
        pushl     8(%edi)                                       #1601.30
        call      NucStrstrWhack@PLT                            #1601.13
                                # LOE ebx esi edi
..B14.26:                       # Preds ..B14.5
        popl      %ecx                                          #1601.13
        jmp       ..B14.21      # Prob 100%                     #1601.13
                                # LOE ebx esi edi
..1.18_0.TAG.0b.0.15:
..B14.7:                        # Preds ..B14.3
        pushl     8(%edi)                                       #1597.30
        call      NucStrstrWhack@PLT                            #1597.13
                                # LOE ebx esi edi
..B14.8:                        # Preds ..B14.7
        pushl     12(%edi)                                      #1598.30
        call      NucStrstrWhack@PLT                            #1598.13
                                # LOE ebx esi edi
..B14.27:                       # Preds ..B14.8
        addl      $8, %esp                                      #1598.13
        jmp       ..B14.21      # Prob 100%                     #1598.13
                                # LOE ebx esi edi
..1.18_0.TAG.0a.0.15:
..1.18_0.TAG.09.0.15:
..1.18_0.TAG.08.0.15:
..1.18_0.TAG.07.0.15:
..1.18_0.TAG.06.0.15:
..1.18_0.TAG.05.0.15:
..1.18_0.TAG.04.0.15:
..1.18_0.TAG.03.0.15:
..1.18_0.TAG.02.0.15:
..1.18_0.TAG.01.0.15:
..1.18_0.TAG.00.0.15:
..B14.20:                       # Preds ..B14.3 ..B14.3 ..B14.3 ..B14.3 ..B14.3
                                #       ..B14.3 ..B14.3 ..B14.3 ..B14.3 ..B14.3
                                #       ..B14.3
        movl      8(%edi), %edi                                 #1593.20
                                # LOE ebx esi edi
..B14.21:                       # Preds ..B14.27 ..B14.26 ..B14.20 ..B14.2
        pushl     %edi                                          #1604.9
        call      free@PLT                                      #1604.9
                                # LOE esi
..B14.28:                       # Preds ..B14.21
        popl      %ecx                                          #1604.9
                                # LOE esi
..B14.22:                       # Preds ..B14.28 ..B14.25
        movl      -8(%ebp), %ebx                                #1606.1
        movl      -4(%ebp), %edi                                #1606.1
        movl      %ebp, %esp                                    #1606.1
        popl      %ebp                                          #1606.1
        ret                                                     #1606.1
        .align    2,0x90
                                # LOE
# mark_end;
	.type	NucStrstrWhack,@function
	.size	NucStrstrWhack,.-NucStrstrWhack
.LNNucStrstrWhack:
	.section .data1, "wa"
	.align 4
..1..TPKT.18_0.0.15:
	.long	..1.18_0.TAG.00.0.15
	.long	..1.18_0.TAG.01.0.15
	.long	..1.18_0.TAG.02.0.15
	.long	..1.18_0.TAG.03.0.15
	.long	..1.18_0.TAG.04.0.15
	.long	..1.18_0.TAG.05.0.15
	.long	..1.18_0.TAG.06.0.15
	.long	..1.18_0.TAG.07.0.15
	.long	..1.18_0.TAG.08.0.15
	.long	..1.18_0.TAG.09.0.15
	.long	..1.18_0.TAG.0a.0.15
	.long	..1.18_0.TAG.0b.0.15
	.long	..1.18_0.TAG.0c.0.15
	.data
# -- End  NucStrstrWhack
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
..B15.1:                        # Preds ..B15.0
        pushl     %ebp                                          #1361.1
        movl      %esp, %ebp                                    #1361.1
        subl      $16, %esp                                     #1361.1
        movl      %edi, -12(%ebp)                               #1361.1
        movl      %esi, -8(%ebp)                                #1361.1
        movl      %ebx, -4(%ebp)                                #1361.1
        movl      8(%ebp), %esi                                 #1359.13
        movl      16(%ebp), %edi                                #1359.13
        call      ..L15         # Prob 100%                     #1361.1
..L15:                                                          #
        popl      %ebx                                          #1361.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L15], %ebx      #1361.1
        lea       _gprof_pack14@GOTOFF(%ebx), %edx              #1361.1
        call      mcount@PLT                                    #1361.1
        jmp       ..B15.2       # Prob 100%                     #1361.1
                                # LOE ebx esi edi
..B15.43:                       # Preds ..B15.42
        movl      $12, (%eax)                                   #1384.13
        movl      $13, 4(%eax)                                  #1385.13
        movl      $0, 8(%eax)                                   #1386.13
        movl      %eax, (%edi)                                  #1387.13
        lea       8(%eax), %edi                                 #1389.44
                                # LOE ebx esi edi
..B15.2:                        # Preds ..B15.43 ..B15.1
        movsbl    (%esi), %eax                                  #1363.12
        cmpl      $33, %eax                                     #1363.17
        je        ..B15.32      # Prob 16%                      #1363.17
                                # LOE eax ebx esi edi
..B15.3:                        # Preds ..B15.2
        cmpl      $94, %eax                                     #1364.16
        jne       ..B15.10      # Prob 67%                      #1364.16
                                # LOE eax ebx esi edi
..B15.4:                        # Preds ..B15.3
        pushl     $12                                           #1364.16
        call      malloc@PLT                                    #1364.16
                                # LOE eax ebx esi edi
..B15.48:                       # Preds ..B15.4
        popl      %ecx                                          #1364.16
        movl      %eax, %edx                                    #1364.16
                                # LOE edx ebx esi edi
..B15.5:                        # Preds ..B15.48
        testl     %edx, %edx                                    #1364.16
        jne       ..B15.7       # Prob 68%                      #1364.16
                                # LOE edx ebx esi edi
..B15.6:                        # Preds ..B15.5
        call      __errno_location@PLT                          #1364.16
                                # LOE eax esi
..B15.49:                       # Preds ..B15.6
        movl      20(%ebp), %ecx                                #1364.16
        movl      (%eax), %edx                                  #1364.16
        movl      %edx, (%ecx)                                  #1364.16
        jmp       ..B15.27      # Prob 100%                     #1364.16
                                # LOE esi
..B15.7:                        # Preds ..B15.5
        movl      12(%ebp), %ecx                                #1361.1
        movl      %edx, -16(%ebp)                               #1364.16
        movl      $12, (%edx)                                   #1364.16
        movl      $14, 4(%edx)                                  #1364.16
        pushl     %ecx                                          #1361.1
        movl      %edx, (%edi)                                  #1364.16
        movl      $0, 8(%edx)                                   #1364.16
        addl      $1, %esi                                      #1364.16
        pushl     %esi                                          #1361.1
        call      nss_sob                                       #1364.16
                                # LOE eax
..B15.50:                       # Preds ..B15.7
        movl      -16(%ebp), %edx                               #
        movl      24(%ebp), %ecx                                #1361.1
        movl      20(%ebp), %ebx                                #1361.1
        pushl     %ecx                                          #1361.1
        pushl     %ebx                                          #1361.1
        addl      $8, %edx                                      #1364.16
        pushl     %edx                                          #1361.1
        movl      12(%ebp), %edx                                #1361.1
        pushl     %edx                                          #1361.1
        pushl     %eax                                          #1361.1
        call      nss_fasta_expr                                #1364.16
                                # LOE eax
..B15.51:                       # Preds ..B15.50
        addl      $28, %esp                                     #1364.16
        movl      %eax, %esi                                    #1364.16
        jmp       ..B15.27      # Prob 100%                     #1364.16
                                # LOE esi
..B15.10:                       # Preds ..B15.3
        cmpl      $40, %eax                                     #1364.16
        jne       ..B15.21      # Prob 50%                      #1364.16
                                # LOE ebx esi edi
..B15.11:                       # Preds ..B15.10
        pushl     $12                                           #1364.16
        call      malloc@PLT                                    #1364.16
                                # LOE eax ebx esi edi
..B15.52:                       # Preds ..B15.11
        popl      %ecx                                          #1364.16
        movl      %eax, %edx                                    #1364.16
                                # LOE edx ebx esi edi
..B15.12:                       # Preds ..B15.52
        testl     %edx, %edx                                    #1364.16
        jne       ..B15.14      # Prob 68%                      #1364.16
                                # LOE edx ebx esi edi
..B15.13:                       # Preds ..B15.12
        call      __errno_location@PLT                          #1364.16
                                # LOE eax esi
..B15.53:                       # Preds ..B15.13
        movl      20(%ebp), %ecx                                #1364.16
        movl      (%eax), %edx                                  #1364.16
        movl      %edx, (%ecx)                                  #1364.16
        jmp       ..B15.27      # Prob 100%                     #1364.16
                                # LOE esi
..B15.14:                       # Preds ..B15.12
        movl      24(%ebp), %ecx                                #1361.1
        movl      20(%ebp), %ebx                                #1361.1
        movl      %edx, -16(%ebp)                               #1364.16
        pushl     %ecx                                          #1361.1
        movl      12(%ebp), %ecx                                #1361.1
        movl      $12, (%edx)                                   #1364.16
        movl      %edx, (%edi)                                  #1364.16
        addl      $1, %esi                                      #1364.16
        movl      $0, 4(%edx)                                   #1364.16
        pushl     %ebx                                          #1361.1
        lea       8(%edx), %edi                                 #1364.16
        pushl     %edi                                          #1361.1
        pushl     %ecx                                          #1361.1
        pushl     %esi                                          #1361.1
        call      nss_expr                                      #1364.16
                                # LOE eax
..B15.54:                       # Preds ..B15.14
        movl      -16(%ebp), %edx                               #
        addl      $20, %esp                                     #1364.16
        movl      %eax, %esi                                    #1364.16
                                # LOE edx esi dl dh
..B15.15:                       # Preds ..B15.54
        movl      20(%ebp), %ecx                                #1364.16
        movl      (%ecx), %ebx                                  #1364.16
        testl     %ebx, %ebx                                    #1364.16
        jne       ..B15.27      # Prob 50%                      #1364.16
                                # LOE edx esi dl dh
..B15.16:                       # Preds ..B15.15
        movl      8(%edx), %eax                                 #1364.16
        testl     %eax, %eax                                    #1364.16
        je        ..B15.19      # Prob 12%                      #1364.16
                                # LOE esi
..B15.17:                       # Preds ..B15.16
        movl      12(%ebp), %eax                                #1364.16
        cmpl      %eax, %esi                                    #1364.16
        je        ..B15.19      # Prob 12%                      #1364.16
                                # LOE esi
..B15.18:                       # Preds ..B15.17
        movsbl    (%esi), %eax                                  #1364.16
        addl      $1, %esi                                      #1364.16
        cmpl      $41, %eax                                     #1364.16
        je        ..B15.27      # Prob 50%                      #1364.16
                                # LOE esi
..B15.19:                       # Preds ..B15.18 ..B15.17 ..B15.16
        movl      20(%ebp), %eax                                #1364.16
        movl      $22, (%eax)                                   #1364.16
        jmp       ..B15.27      # Prob 100%                     #1364.16
                                # LOE esi
..B15.21:                       # Preds ..B15.10
        movl      24(%ebp), %eax                                #1361.1
        movl      20(%ebp), %edx                                #1361.1
        movl      12(%ebp), %ecx                                #1361.1
        pushl     %eax                                          #1361.1
        pushl     %edx                                          #1361.1
        pushl     %edi                                          #1361.1
        pushl     %ecx                                          #1361.1
        pushl     %esi                                          #1361.1
        call      nss_fasta_expr                                #1364.16
                                # LOE eax ebx edi
..B15.55:                       # Preds ..B15.21
        addl      $20, %esp                                     #1364.16
        movl      %eax, %esi                                    #1364.16
                                # LOE ebx esi edi
..B15.22:                       # Preds ..B15.55
        movl      20(%ebp), %eax                                #1364.16
        movl      (%eax), %edx                                  #1364.16
        testl     %edx, %edx                                    #1364.16
        jne       ..B15.27      # Prob 50%                      #1364.16
                                # LOE ebx esi edi
..B15.23:                       # Preds ..B15.22
        movl      12(%ebp), %eax                                #1364.16
        cmpl      %eax, %esi                                    #1364.16
        jae       ..B15.27      # Prob 33%                      #1364.16
                                # LOE ebx esi edi
..B15.24:                       # Preds ..B15.23
        movl      12(%ebp), %eax                                #1361.1
        pushl     %eax                                          #1361.1
        pushl     %esi                                          #1361.1
        call      nss_sob                                       #1364.16
                                # LOE eax ebx edi
..B15.56:                       # Preds ..B15.24
        addl      $8, %esp                                      #1364.16
        movl      %eax, %esi                                    #1364.16
        movl      12(%ebp), %eax                                #1364.16
        cmpl      %eax, %esi                                    #1364.16
        jae       ..B15.27      # Prob 12%                      #1364.16
                                # LOE ebx esi edi
..B15.25:                       # Preds ..B15.56
        movsbl    (%esi), %eax                                  #1364.16
        cmpl      $36, %eax                                     #1364.16
        je        ..B15.28      # Prob 5%                       #1364.16
                                # LOE ebx esi edi
..B15.27:                       # Preds ..B15.22 ..B15.23 ..B15.25 ..B15.30 ..B15.56
                                #       ..B15.58 ..B15.15 ..B15.19 ..B15.18 ..B15.53
                                #       ..B15.49 ..B15.51
        movl      -4(%ebp), %ebx                                #1364.16
        movl      -12(%ebp), %edi                               #1364.16
        movl      %esi, %eax                                    #1364.16
        movl      -8(%ebp), %esi                                #1364.16
        movl      %ebp, %esp                                    #1364.16
        popl      %ebp                                          #1364.16
        ret                                                     #1364.16
                                # LOE
..B15.28:                       # Preds ..B15.25                # Infreq
        pushl     $12                                           #1364.16
        addl      $1, %esi                                      #1364.16
        call      malloc@PLT                                    #1364.16
                                # LOE eax ebx esi edi
..B15.57:                       # Preds ..B15.28                # Infreq
        popl      %ecx                                          #1364.16
                                # LOE eax ebx esi edi
..B15.29:                       # Preds ..B15.57                # Infreq
        testl     %eax, %eax                                    #1364.16
        je        ..B15.31      # Prob 12%                      #1364.16
                                # LOE eax ebx esi edi
..B15.30:                       # Preds ..B15.29                # Infreq
        movl      (%edi), %edx                                  #1364.16
        movl      $12, (%eax)                                   #1364.16
        movl      %edx, 8(%eax)                                 #1364.16
        movl      %eax, (%edi)                                  #1364.16
        movl      $15, 4(%eax)                                  #1364.16
        jmp       ..B15.27      # Prob 100%                     #1364.16
                                # LOE esi
..B15.31:                       # Preds ..B15.29                # Infreq
        call      __errno_location@PLT                          #1364.16
                                # LOE eax esi
..B15.58:                       # Preds ..B15.31                # Infreq
        movl      20(%ebp), %ecx                                #1364.16
        movl      (%eax), %edx                                  #1364.16
        movl      %edx, (%ecx)                                  #1364.16
        jmp       ..B15.27      # Prob 100%                     #1364.16
                                # LOE esi
..B15.32:                       # Preds ..B15.2                 # Infreq
        movl      12(%ebp), %eax                                #1374.9
        addl      $1, %esi                                      #1374.19
        cmpl      %eax, %esi                                    #1374.9
        jae       ..B15.38      # Prob 12%                      #1374.9
                                # LOE ebx esi edi
..B15.33:                       # Preds ..B15.32                # Infreq
        call      __ctype_b_loc@PLT                             #1374.9
                                # LOE eax ebx esi edi
..B15.59:                       # Preds ..B15.33                # Infreq
        movzbl    (%esi), %ecx                                  #1374.9
        movl      (%eax), %edx                                  #1374.9
        movzwl    (%edx,%ecx,2), %eax                           #1374.9
        testl     $8192, %eax                                   #1374.9
        je        ..B15.38      # Prob 15%                      #1374.9
                                # LOE edx ebx esi edi
..B15.34:                       # Preds ..B15.59                # Infreq
        movl      12(%ebp), %eax                                #
                                # LOE eax edx ebx esi edi
..B15.35:                       # Preds ..B15.36 ..B15.34       # Infreq
        addl      $1, %esi                                      #1374.9
        cmpl      %eax, %esi                                    #1374.9
        jae       ..B15.38      # Prob 1%                       #1374.9
                                # LOE eax edx ebx esi edi
..B15.36:                       # Preds ..B15.35                # Infreq
        movzbl    (%esi), %ecx                                  #1374.9
        movzwl    (%edx,%ecx,2), %ecx                           #1374.9
        testl     $8192, %ecx                                   #1374.9
        jne       ..B15.35      # Prob 82%                      #1374.9
                                # LOE eax edx ebx esi edi
..B15.38:                       # Preds ..B15.35 ..B15.36 ..B15.59 ..B15.32 # Infreq
        movl      12(%ebp), %eax                                #1375.15
        cmpl      %eax, %esi                                    #1375.15
        jne       ..B15.41      # Prob 68%                      #1375.15
                                # LOE ebx esi edi
..B15.39:                       # Preds ..B15.38                # Infreq
        movl      20(%ebp), %eax                                #1376.9
        movl      $22, (%eax)                                   #1376.9
                                # LOE esi
..B15.40:                       # Preds ..B15.61 ..B15.39       # Infreq
        movl      -4(%ebp), %ebx                                #1393.12
        movl      -12(%ebp), %edi                               #1393.12
        movl      %esi, %eax                                    #1375.5
        movl      -8(%ebp), %esi                                #1393.12
        movl      %ebp, %esp                                    #1393.12
        popl      %ebp                                          #1393.12
        ret                                                     #1393.12
                                # LOE
..B15.41:                       # Preds ..B15.38                # Infreq
        pushl     $12                                           #1379.34
        call      malloc@PLT                                    #1379.25
                                # LOE eax ebx esi edi
..B15.60:                       # Preds ..B15.41                # Infreq
        popl      %ecx                                          #1379.25
                                # LOE eax ebx esi edi
..B15.42:                       # Preds ..B15.60                # Infreq
        testl     %eax, %eax                                    #1380.19
        jne       ..B15.43      # Prob 99%                      #1380.19
                                # LOE eax ebx esi edi
..B15.44:                       # Preds ..B15.42                # Infreq
        call      __errno_location@PLT                          #1381.24
                                # LOE eax esi
..B15.61:                       # Preds ..B15.44                # Infreq
        movl      20(%ebp), %ecx                                #1381.13
        movl      (%eax), %edx                                  #1381.24
        movl      %edx, (%ecx)                                  #1381.13
        jmp       ..B15.40      # Prob 100%                     #1381.13
        .align    2,0x90
                                # LOE esi
# mark_end;
	.type	nss_unary_expr,@function
	.size	nss_unary_expr,.-nss_unary_expr
.LNnss_unary_expr:
	.data
# -- End  nss_unary_expr
	.text
# -- Begin  nss_sob
# mark_begin;
       .align    2,0x90
nss_sob:
# parameter 1(p): 8 + %ebp
# parameter 2(end): 12 + %ebp
..B16.1:                        # Preds ..B16.0
        pushl     %ebp                                          #1181.1
        movl      %esp, %ebp                                    #1181.1
        subl      $12, %esp                                     #1181.1
        movl      %edi, -4(%ebp)                                #1181.1
        movl      %esi, -8(%ebp)                                #1181.1
        movl      %ebx, -12(%ebp)                               #1181.1
        movl      8(%ebp), %esi                                 #1180.13
        movl      12(%ebp), %edi                                #1180.13
        call      ..L16         # Prob 100%                     #1181.1
..L16:                                                          #
        popl      %ebx                                          #1181.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L16], %ebx      #1181.1
        lea       _gprof_pack15@GOTOFF(%ebx), %edx              #1181.1
        call      mcount@PLT                                    #1181.1
                                # LOE ebx esi edi
..B16.11:                       # Preds ..B16.1
        cmpl      %edi, %esi                                    #1182.17
        jae       ..B16.7       # Prob 12%                      #1182.17
                                # LOE ebx esi edi
..B16.2:                        # Preds ..B16.11
        call      __ctype_b_loc@PLT                             #1182.24
                                # LOE eax esi edi
..B16.12:                       # Preds ..B16.2
        movzbl    (%esi), %edx                                  #1182.24
        movl      (%eax), %ecx                                  #1182.24
        movzwl    (%ecx,%edx,2), %ebx                           #1182.24
        testl     $8192, %ebx                                   #1182.24
        je        ..B16.7       # Prob 15%                      #1182.24
                                # LOE ecx esi edi
..B16.3:                        # Preds ..B16.12
        xorl      %edx, %edx                                    #1182.24
        movl      %esi, %eax                                    #1182.24
                                # LOE eax edx ecx esi edi
..B16.4:                        # Preds ..B16.5 ..B16.3
        addl      $1, %eax                                      #1182.24
        addl      $1, %edx                                      #1182.24
        cmpl      %eax, %edi                                    #1182.17
        jbe       ..B16.6       # Prob 1%                       #1182.17
                                # LOE eax edx ecx esi edi
..B16.5:                        # Preds ..B16.4
        movzbl    (%esi,%edx), %ebx                             #1182.24
        movzwl    (%ecx,%ebx,2), %ebx                           #1182.24
        testl     $8192, %ebx                                   #1182.24
        jne       ..B16.4       # Prob 82%                      #1182.24
                                # LOE eax edx ecx esi edi
..B16.6:                        # Preds ..B16.4 ..B16.5
        addl      %edx, %esi                                    #1183.12
                                # LOE esi
..B16.7:                        # Preds ..B16.12 ..B16.11 ..B16.6
        movl      -12(%ebp), %ebx                               #1184.12
        movl      -4(%ebp), %edi                                #1184.12
        movl      %esi, %eax                                    #1184.12
        movl      -8(%ebp), %esi                                #1184.12
        movl      %ebp, %esp                                    #1184.12
        popl      %ebp                                          #1184.12
        ret                                                     #1184.12
        .align    2,0x90
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
       .align    2,0x90
nss_fasta_expr:
# parameter 1(p): 8 + %ebp
# parameter 2(end): 12 + %ebp
# parameter 3(expr): 16 + %ebp
# parameter 4(status): 20 + %ebp
# parameter 5(positional): 24 + %ebp
..B17.1:                        # Preds ..B17.0
        pushl     %ebp                                          #1234.1
        movl      %esp, %ebp                                    #1234.1
        subl      $16, %esp                                     #1234.1
        movl      %edi, -4(%ebp)                                #1234.1
        movl      %esi, -8(%ebp)                                #1234.1
        movl      %ebx, -12(%ebp)                               #1234.1
        movl      8(%ebp), %esi                                 #1232.13
        call      ..L17         # Prob 100%                     #1234.1
..L17:                                                          #
        popl      %ebx                                          #1234.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L17], %ebx      #1234.1
        lea       _gprof_pack16@GOTOFF(%ebx), %edx              #1234.1
        call      mcount@PLT                                    #1234.1
                                # LOE ebx esi
..B17.77:                       # Preds ..B17.1
        movsbl    (%esi), %eax                                  #1236.16
        cmpl      $39, %eax                                     #1236.16
        jne       ..B17.16      # Prob 67%                      #1236.16
                                # LOE eax ebx esi
..B17.2:                        # Preds ..B17.77
        movl      12(%ebp), %eax                                #1239.13
        lea       1(%esi), %ecx                                 #1239.30
        movl      %ecx, %esi                                    #1239.30
        cmpl      %eax, %ecx                                    #1239.13
        jae       ..B17.10      # Prob 44%                      #1239.13
                                # LOE ecx ebx esi
..B17.4:                        # Preds ..B17.2
        lea       fasta_2na_map.0@GOTOFF(%ebx), %edx            #1239.13
        movl      %ecx, -16(%ebp)                               #
        xorl      %eax, %eax                                    #
                                # LOE eax edx ebx esi
..B17.5:                        # Preds ..B17.7 ..B17.4
        movsbl    (%esi), %ecx                                  #1239.13
        testl     %ecx, %ecx                                    #1239.13
        jl        ..B17.92      # Prob 1%                       #1239.13
                                # LOE eax edx ebx esi
..B17.6:                        # Preds ..B17.5
        movzbl    (%esi), %edi                                  #1239.13
        movsbl    (%edi,%edx), %ecx                             #1239.13
        testl     %ecx, %ecx                                    #1239.13
        jl        ..B17.47      # Prob 16%                      #1239.13
                                # LOE eax edx ebx esi edi
..B17.7:                        # Preds ..B17.49 ..B17.6
        movl      12(%ebp), %ecx                                #1239.13
        addl      $1, %esi                                      #1239.13
        cmpl      %ecx, %esi                                    #1239.13
        jb        ..B17.5       # Prob 82%                      #1239.13
                                # LOE eax edx ebx esi
..B17.8:                        # Preds ..B17.7
        movl      -16(%ebp), %ecx                               #
                                # LOE eax ecx esi cl ch
..B17.9:                        # Preds ..B17.92 ..B17.8
        cmpl      %ecx, %esi                                    #1239.13
        ja        ..B17.42      # Prob 12%                      #1239.13
                                # LOE eax ecx esi cl ch
..B17.10:                       # Preds ..B17.2 ..B17.9
        movl      $22, %edx                                     #1239.13
        movl      $22, %eax                                     #
                                # LOE eax edx esi
..B17.11:                       # Preds ..B17.10 ..B17.44 ..B17.46
        testl     %eax, %eax                                    #1240.26
        jne       ..B17.30      # Prob 50%                      #1240.26
                                # LOE edx esi
..B17.12:                       # Preds ..B17.11
        movl      12(%ebp), %eax                                #1240.38
        cmpl      %eax, %esi                                    #1240.38
        je        ..B17.14      # Prob 12%                      #1240.38
                                # LOE edx esi
..B17.13:                       # Preds ..B17.12
        movl      20(%ebp), %eax                                #1241.13
        movl      %edx, (%eax)                                  #1241.13
        movsbl    (%esi), %edx                                  #1240.47
        addl      $1, %esi                                      #1240.47
        cmpl      $39, %edx                                     #1240.55
        je        ..B17.31      # Prob 50%                      #1240.55
                                # LOE esi
..B17.14:                       # Preds ..B17.13 ..B17.12
        movl      20(%ebp), %eax                                #1241.13
        movl      $22, (%eax)                                   #1241.13
        jmp       ..B17.31      # Prob 100%                     #1241.13
                                # LOE esi
..B17.16:                       # Preds ..B17.77
        cmpl      $34, %eax                                     #1236.16
        jne       ..B17.32      # Prob 50%                      #1236.16
                                # LOE ebx esi
..B17.17:                       # Preds ..B17.16
        movl      12(%ebp), %eax                                #1244.13
        lea       1(%esi), %ecx                                 #1244.30
        movl      %ecx, %esi                                    #1244.30
        cmpl      %eax, %ecx                                    #1244.13
        jae       ..B17.25      # Prob 44%                      #1244.13
                                # LOE ecx ebx esi
..B17.19:                       # Preds ..B17.17
        lea       fasta_2na_map.0@GOTOFF(%ebx), %edx            #1244.13
        movl      %ecx, -16(%ebp)                               #
        xorl      %eax, %eax                                    #
                                # LOE eax edx ebx esi
..B17.20:                       # Preds ..B17.22 ..B17.19
        movsbl    (%esi), %ecx                                  #1244.13
        testl     %ecx, %ecx                                    #1244.13
        jl        ..B17.91      # Prob 1%                       #1244.13
                                # LOE eax edx ebx esi
..B17.21:                       # Preds ..B17.20
        movzbl    (%esi), %edi                                  #1244.13
        movsbl    (%edi,%edx), %ecx                             #1244.13
        testl     %ecx, %ecx                                    #1244.13
        jl        ..B17.58      # Prob 16%                      #1244.13
                                # LOE eax edx ebx esi edi
..B17.22:                       # Preds ..B17.60 ..B17.21
        movl      12(%ebp), %ecx                                #1244.13
        addl      $1, %esi                                      #1244.13
        cmpl      %ecx, %esi                                    #1244.13
        jb        ..B17.20      # Prob 82%                      #1244.13
                                # LOE eax edx ebx esi
..B17.23:                       # Preds ..B17.22
        movl      -16(%ebp), %ecx                               #
                                # LOE eax ecx esi cl ch
..B17.24:                       # Preds ..B17.91 ..B17.23
        cmpl      %ecx, %esi                                    #1244.13
        ja        ..B17.53      # Prob 12%                      #1244.13
                                # LOE eax ecx esi cl ch
..B17.25:                       # Preds ..B17.17 ..B17.24
        movl      $22, %edx                                     #1244.13
        movl      $22, %eax                                     #
                                # LOE eax edx esi
..B17.26:                       # Preds ..B17.25 ..B17.55 ..B17.57
        testl     %eax, %eax                                    #1245.26
        jne       ..B17.30      # Prob 50%                      #1245.26
                                # LOE edx esi
..B17.27:                       # Preds ..B17.26
        movl      12(%ebp), %eax                                #1245.38
        cmpl      %eax, %esi                                    #1245.38
        je        ..B17.29      # Prob 12%                      #1245.38
                                # LOE edx esi
..B17.28:                       # Preds ..B17.27
        movl      20(%ebp), %eax                                #1241.13
        movl      %edx, (%eax)                                  #1241.13
        movsbl    (%esi), %edx                                  #1245.47
        addl      $1, %esi                                      #1245.47
        cmpl      $34, %edx                                     #1245.55
        je        ..B17.31      # Prob 50%                      #1245.55
                                # LOE esi
..B17.29:                       # Preds ..B17.28 ..B17.27
        movl      20(%ebp), %eax                                #1246.13
        movl      $22, (%eax)                                   #1246.13
        jmp       ..B17.31      # Prob 100%                     #1246.13
                                # LOE esi
..B17.30:                       # Preds ..B17.11 ..B17.26
        movl      20(%ebp), %eax                                #1241.13
        movl      %edx, (%eax)                                  #1241.13
                                # LOE esi
..B17.31:                       # Preds ..B17.13 ..B17.28 ..B17.30 ..B17.14 ..B17.29
                                #      
        movl      -12(%ebp), %ebx                               #1252.12
        movl      -4(%ebp), %edi                                #1252.12
        movl      %esi, %eax                                    #1252.12
        movl      -8(%ebp), %esi                                #1252.12
        movl      %ebp, %esp                                    #1252.12
        popl      %ebp                                          #1252.12
        ret                                                     #1252.12
                                # LOE
..B17.32:                       # Preds ..B17.16
        movl      12(%ebp), %eax                                #1249.16
        movl      %esi, %edi                                    #1234.1
        cmpl      %eax, %esi                                    #1249.16
        jae       ..B17.40      # Prob 44%                      #1249.16
                                # LOE ebx esi edi
..B17.34:                       # Preds ..B17.32
        lea       fasta_2na_map.0@GOTOFF(%ebx), %edx            #1249.16
        xorl      %eax, %eax                                    #
                                # LOE eax edx ebx esi edi
..B17.35:                       # Preds ..B17.37 ..B17.34
        movsbl    (%edi), %ecx                                  #1249.16
        testl     %ecx, %ecx                                    #1249.16
        jl        ..B17.90      # Prob 1%                       #1249.16
                                # LOE eax edx ebx edi
..B17.36:                       # Preds ..B17.35
        movzbl    (%edi), %esi                                  #1249.16
        movsbl    (%esi,%edx), %ecx                             #1249.16
        testl     %ecx, %ecx                                    #1249.16
        jl        ..B17.69      # Prob 16%                      #1249.16
                                # LOE eax edx ebx esi edi
..B17.37:                       # Preds ..B17.71 ..B17.36
        movl      12(%ebp), %ecx                                #1249.16
        addl      $1, %edi                                      #1249.16
        cmpl      %ecx, %edi                                    #1249.16
        jb        ..B17.35      # Prob 82%                      #1249.16
                                # LOE eax edx ebx edi
..B17.38:                       # Preds ..B17.37
        movl      8(%ebp), %esi                                 #
                                # LOE eax esi edi
..B17.39:                       # Preds ..B17.90 ..B17.38
        cmpl      %esi, %edi                                    #1249.16
        ja        ..B17.64      # Prob 12%                      #1249.16
                                # LOE eax esi edi
..B17.40:                       # Preds ..B17.32 ..B17.39
        movl      20(%ebp), %eax                                #1249.16
        movl      $22, (%eax)                                   #1249.16
                                # LOE edi
..B17.41:                       # Preds ..B17.40 ..B17.66 ..B17.68
        movl      -12(%ebp), %ebx                               #1249.16
        movl      -8(%ebp), %esi                                #1249.16
        movl      %edi, %eax                                    #1249.16
        movl      -4(%ebp), %edi                                #1249.16
        movl      %ebp, %esp                                    #1249.16
        popl      %ebp                                          #1249.16
        ret                                                     #1249.16
                                # LOE
..B17.42:                       # Preds ..B17.9                 # Infreq
        testl     %eax, %eax                                    #1239.13
        jne       ..B17.45      # Prob 50%                      #1239.13
                                # LOE ecx esi cl ch
..B17.43:                       # Preds ..B17.42                # Infreq
        movl      24(%ebp), %edx                                #1239.13
        movl      %esi, %eax                                    #1239.13
        subl      %ecx, %eax                                    #1239.13
        pushl     %eax                                          #1239.13
        pushl     %ecx                                          #1239.13
        movl      16(%ebp), %ecx                                #1239.13
        pushl     %edx                                          #1239.13
        pushl     %ecx                                          #1239.13
        call      NucStrFastaExprMake2                          #1239.13
                                # LOE eax esi
..B17.78:                       # Preds ..B17.43                # Infreq
        addl      $16, %esp                                     #1239.13
                                # LOE eax esi
..B17.44:                       # Preds ..B17.78                # Infreq
        movl      %eax, %edx                                    #1239.13
        jmp       ..B17.11      # Prob 100%                     #1239.13
                                # LOE eax edx esi
..B17.45:                       # Preds ..B17.42                # Infreq
        movl      24(%ebp), %edx                                #1239.13
        movl      %esi, %eax                                    #1239.13
        subl      %ecx, %eax                                    #1239.13
        pushl     %eax                                          #1239.13
        pushl     %ecx                                          #1239.13
        movl      16(%ebp), %ecx                                #1239.13
        pushl     %edx                                          #1239.13
        pushl     %ecx                                          #1239.13
        call      NucStrFastaExprMake4                          #1239.13
                                # LOE eax esi
..B17.79:                       # Preds ..B17.45                # Infreq
        addl      $16, %esp                                     #1239.13
                                # LOE eax esi
..B17.46:                       # Preds ..B17.79                # Infreq
        movl      %eax, %edx                                    #1239.13
        jmp       ..B17.11      # Prob 100%                     #1239.13
                                # LOE eax edx esi
..B17.47:                       # Preds ..B17.6                 # Infreq
        lea       fasta_4na_map.0@GOTOFF(%ebx), %ecx            #1239.13
        cmpl      %ecx, %edx                                    #1239.13
        je        ..B17.92      # Prob 1%                       #1239.13
                                # LOE eax ecx ebx esi edi
..B17.48:                       # Preds ..B17.47                # Infreq
        movsbl    (%edi,%ecx), %edx                             #1239.13
        testl     %edx, %edx                                    #1239.13
        jl        ..B17.92      # Prob 1%                       #1239.13
                                # LOE eax ecx ebx esi
..B17.49:                       # Preds ..B17.48                # Infreq
        movl      %ecx, %edx                                    #1239.13
        movl      $1, %eax                                      #
        jmp       ..B17.7       # Prob 100%                     #
                                # LOE eax edx ebx esi
..B17.92:                       # Preds ..B17.47 ..B17.48 ..B17.5 # Infreq
        movl      -16(%ebp), %ecx                               #
        jmp       ..B17.9       # Prob 100%                     #
                                # LOE eax ecx esi cl ch
..B17.53:                       # Preds ..B17.24                # Infreq
        testl     %eax, %eax                                    #1244.13
        jne       ..B17.56      # Prob 50%                      #1244.13
                                # LOE ecx esi cl ch
..B17.54:                       # Preds ..B17.53                # Infreq
        movl      24(%ebp), %edx                                #1244.13
        movl      %esi, %eax                                    #1244.13
        subl      %ecx, %eax                                    #1244.13
        pushl     %eax                                          #1244.13
        pushl     %ecx                                          #1244.13
        movl      16(%ebp), %ecx                                #1244.13
        pushl     %edx                                          #1244.13
        pushl     %ecx                                          #1244.13
        call      NucStrFastaExprMake2                          #1244.13
                                # LOE eax esi
..B17.80:                       # Preds ..B17.54                # Infreq
        addl      $16, %esp                                     #1244.13
                                # LOE eax esi
..B17.55:                       # Preds ..B17.80                # Infreq
        movl      %eax, %edx                                    #1244.13
        jmp       ..B17.26      # Prob 100%                     #1244.13
                                # LOE eax edx esi
..B17.56:                       # Preds ..B17.53                # Infreq
        movl      24(%ebp), %edx                                #1244.13
        movl      %esi, %eax                                    #1244.13
        subl      %ecx, %eax                                    #1244.13
        pushl     %eax                                          #1244.13
        pushl     %ecx                                          #1244.13
        movl      16(%ebp), %ecx                                #1244.13
        pushl     %edx                                          #1244.13
        pushl     %ecx                                          #1244.13
        call      NucStrFastaExprMake4                          #1244.13
                                # LOE eax esi
..B17.81:                       # Preds ..B17.56                # Infreq
        addl      $16, %esp                                     #1244.13
                                # LOE eax esi
..B17.57:                       # Preds ..B17.81                # Infreq
        movl      %eax, %edx                                    #1244.13
        jmp       ..B17.26      # Prob 100%                     #1244.13
                                # LOE eax edx esi
..B17.58:                       # Preds ..B17.21                # Infreq
        lea       fasta_4na_map.0@GOTOFF(%ebx), %ecx            #1244.13
        cmpl      %ecx, %edx                                    #1244.13
        je        ..B17.91      # Prob 1%                       #1244.13
                                # LOE eax ecx ebx esi edi
..B17.59:                       # Preds ..B17.58                # Infreq
        movsbl    (%edi,%ecx), %edx                             #1244.13
        testl     %edx, %edx                                    #1244.13
        jl        ..B17.91      # Prob 1%                       #1244.13
                                # LOE eax ecx ebx esi
..B17.60:                       # Preds ..B17.59                # Infreq
        movl      %ecx, %edx                                    #1244.13
        movl      $1, %eax                                      #
        jmp       ..B17.22      # Prob 100%                     #
                                # LOE eax edx ebx esi
..B17.91:                       # Preds ..B17.58 ..B17.59 ..B17.20 # Infreq
        movl      -16(%ebp), %ecx                               #
        jmp       ..B17.24      # Prob 100%                     #
                                # LOE eax ecx esi cl ch
..B17.64:                       # Preds ..B17.39                # Infreq
        testl     %eax, %eax                                    #1249.16
        jne       ..B17.67      # Prob 50%                      #1249.16
                                # LOE esi edi
..B17.65:                       # Preds ..B17.64                # Infreq
        movl      24(%ebp), %edx                                #1249.16
        movl      16(%ebp), %ecx                                #1249.16
        movl      %edi, %eax                                    #1249.16
        subl      %esi, %eax                                    #1249.16
        pushl     %eax                                          #1249.16
        pushl     %esi                                          #1249.16
        pushl     %edx                                          #1249.16
        pushl     %ecx                                          #1249.16
        call      NucStrFastaExprMake2                          #1249.16
                                # LOE eax edi
..B17.82:                       # Preds ..B17.65                # Infreq
        addl      $16, %esp                                     #1249.16
                                # LOE eax edi
..B17.66:                       # Preds ..B17.82                # Infreq
        movl      20(%ebp), %edx                                #1249.16
        movl      %eax, (%edx)                                  #1249.16
        jmp       ..B17.41      # Prob 100%                     #1249.16
                                # LOE edi
..B17.67:                       # Preds ..B17.64                # Infreq
        movl      24(%ebp), %edx                                #1249.16
        movl      16(%ebp), %ecx                                #1249.16
        movl      %edi, %eax                                    #1249.16
        subl      %esi, %eax                                    #1249.16
        pushl     %eax                                          #1249.16
        pushl     %esi                                          #1249.16
        pushl     %edx                                          #1249.16
        pushl     %ecx                                          #1249.16
        call      NucStrFastaExprMake4                          #1249.16
                                # LOE eax edi
..B17.83:                       # Preds ..B17.67                # Infreq
        addl      $16, %esp                                     #1249.16
                                # LOE eax edi
..B17.68:                       # Preds ..B17.83                # Infreq
        movl      20(%ebp), %edx                                #1249.16
        movl      %eax, (%edx)                                  #1249.16
        jmp       ..B17.41      # Prob 100%                     #1249.16
                                # LOE edi
..B17.69:                       # Preds ..B17.36                # Infreq
        lea       fasta_4na_map.0@GOTOFF(%ebx), %ecx            #1249.16
        cmpl      %ecx, %edx                                    #1249.16
        je        ..B17.90      # Prob 1%                       #1249.16
                                # LOE eax ecx ebx esi edi
..B17.70:                       # Preds ..B17.69                # Infreq
        movsbl    (%ecx,%esi), %edx                             #1249.16
        testl     %edx, %edx                                    #1249.16
        jl        ..B17.90      # Prob 1%                       #1249.16
                                # LOE eax ecx ebx edi
..B17.71:                       # Preds ..B17.70                # Infreq
        movl      %ecx, %edx                                    #1249.16
        movl      $1, %eax                                      #
        jmp       ..B17.37      # Prob 100%                     #
                                # LOE eax edx ebx edi
..B17.90:                       # Preds ..B17.69 ..B17.70 ..B17.35 # Infreq
        movl      8(%ebp), %esi                                 #
        jmp       ..B17.39      # Prob 100%                     #
        .align    2,0x90
                                # LOE eax esi edi
# mark_end;
	.type	nss_fasta_expr,@function
	.size	nss_fasta_expr,.-nss_fasta_expr
.LNnss_fasta_expr:
	.data
# -- End  nss_fasta_expr
	.text
# -- Begin  NucStrFastaExprMake4
# mark_begin;
       .align    2,0x90
NucStrFastaExprMake4:
# parameter 1(expr): 8 + %ebx
# parameter 2(positional): 12 + %ebx
# parameter 3(fasta): 16 + %ebx
# parameter 4(size): 20 + %ebx
..B18.1:                        # Preds ..B18.0
        pushl     %ebx                                          #1017.1
        movl      %esp, %ebx                                    #1017.1
        andl      $-16, %esp                                    #1017.1
        pushl     %ebp                                          #1017.1
        pushl     %ebp                                          #1017.1
        movl      4(%ebx), %ebp                                 #1017.1
        movl      %ebp, 4(%esp)                                 #1017.1
        movl      %esp, %ebp                                    #1017.1
        subl      $72, %esp                                     #1017.1
        movl      %ebx, -72(%ebp)                               #1017.1
        movl      %edi, -44(%ebp)                               #1017.1
        movl      %esi, -48(%ebp)                               #1017.1
        call      ..L18         # Prob 100%                     #1017.1
..L18:                                                          #
        popl      %edi                                          #1017.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L18], %edi      #1017.1
        lea       _gprof_pack17@GOTOFF(%edi), %edx              #1017.1
        movl      %edi, %ebx                                    #1017.1
        call      mcount@PLT                                    #1017.1
        movl      -72(%ebp), %ebx                               #1017.1
                                # LOE edi
..B18.46:                       # Preds ..B18.1
        movl      20(%ebx), %eax                                #1023.17
        cmpl      $29, %eax                                     #1023.17
        jbe       ..B18.3       # Prob 43%                      #1023.17
                                # LOE edi
..B18.2:                        # Preds ..B18.46
        movl      -48(%ebp), %esi                               #1024.16
        movl      -44(%ebp), %edi                               #1024.16
        movl      $7, %eax                                      #1024.16
        movl      %ebp, %esp                                    #1024.16
        popl      %ebp                                          #1024.16
        movl      %ebx, %esp                                    #1024.16
        popl      %ebx                                          #1024.16
        ret                                                     #1024.16
                                # LOE
..B18.3:                        # Preds ..B18.46
        pushl     $160                                          #1026.32
        movl      %edi, %ebx                                    #1026.9
        call      malloc@PLT                                    #1026.9
        movl      -72(%ebp), %ebx                               #1026.9
                                # LOE eax edi
..B18.47:                       # Preds ..B18.3
        addl      $4, %esp                                      #1026.9
                                # LOE eax edi
..B18.4:                        # Preds ..B18.47
        testl     %eax, %eax                                    #1026.9
        je        ..B18.43      # Prob 12%                      #1026.9
                                # LOE eax edi
..B18.5:                        # Preds ..B18.4
        lea       15(%eax), %esi                                #1026.9
        andl      $-16, %esi                                    #1026.9
        movl      %eax, 8(%esi)                                 #1026.9
        je        ..B18.43      # Prob 5%                       #1027.15
                                # LOE esi edi
..B18.6:                        # Preds ..B18.5
        movl      8(%ebx), %eax                                 #1030.5
        movl      20(%ebx), %edx                                #1031.5
        movl      %esi, (%eax)                                  #1030.5
        movl      %edx, 4(%esi)                                 #1031.5
        testl     %edx, %edx                                    #1035.22
        jbe       ..B18.42      # Prob 16%                      #1035.22
                                # LOE esi edi
..B18.7:                        # Preds ..B18.6
        movl      16(%ebx), %ecx                                #
        movl      %esi, -60(%ebp)                               #
        xorl      %eax, %eax                                    #
                                # LOE eax ecx edi
..B18.8:                        # Preds ..B18.15 ..B18.7
        movsbl    (%ecx,%eax), %edx                             #1037.49
        movsbl    fasta_4na_map.0@GOTOFF(%edi,%edx), %edx       #1037.25
        movl      %eax, %esi                                    #1038.22
        andl      $3, %esi                                      #1038.22
        je        ..B18.41      # Prob 20%                      #1038.22
                                # LOE eax edx ecx esi edi
..B18.9:                        # Preds ..B18.8
        cmpl      $1, %esi                                      #1038.22
        je        ..B18.14      # Prob 25%                      #1038.22
                                # LOE eax edx ecx esi edi
..B18.10:                       # Preds ..B18.9
        cmpl      $2, %esi                                      #1038.22
        jne       ..B18.12      # Prob 67%                      #1038.22
                                # LOE eax edx ecx esi edi
..B18.11:                       # Preds ..B18.10
        movl      %eax, %esi                                    #1049.32
        shrl      $2, %esi                                      #1049.32
        shll      $12, %edx                                     #1049.47
        orw       %dx, -40(%ebp,%esi,2)                         #1049.13
        movzwl    -24(%ebp,%esi,2), %edx                        #1050.13
        orl       $-4096, %edx                                  #1050.13
        movw      %dx, -24(%ebp,%esi,2)                         #1050.13
        jmp       ..B18.15      # Prob 100%                     #1050.13
                                # LOE eax ecx edi
..B18.12:                       # Preds ..B18.10
        cmpl      $3, %esi                                      #1038.22
        jne       ..B18.15      # Prob 50%                      #1038.22
                                # LOE eax edx ecx edi
..B18.13:                       # Preds ..B18.12
        movl      %eax, %esi                                    #1053.32
        shrl      $2, %esi                                      #1053.32
        shll      $8, %edx                                      #1053.47
        orw       %dx, -40(%ebp,%esi,2)                         #1053.13
        movzwl    -24(%ebp,%esi,2), %edx                        #1054.13
        orl       $-61696, %edx                                 #1054.13
        movw      %dx, -24(%ebp,%esi,2)                         #1054.13
        jmp       ..B18.15      # Prob 100%                     #1054.13
                                # LOE eax ecx edi
..B18.14:                       # Preds ..B18.9
        movl      %eax, %esi                                    #1045.32
        shrl      $2, %esi                                      #1045.32
        orw       %dx, -40(%ebp,%esi,2)                         #1045.13
        movzwl    -24(%ebp,%esi,2), %edx                        #1046.13
        orl       $15, %edx                                     #1046.13
        movw      %dx, -24(%ebp,%esi,2)                         #1046.13
                                # LOE eax ecx edi
..B18.15:                       # Preds ..B18.41 ..B18.14 ..B18.11 ..B18.13 ..B18.12
                                #      
        movl      20(%ebx), %edx                                #1035.22
        addl      $1, %eax                                      #1035.31
        cmpl      %edx, %eax                                    #1035.22
        jb        ..B18.8       # Prob 82%                      #1035.22
                                # LOE eax ecx edi
..B18.16:                       # Preds ..B18.15
        movl      -60(%ebp), %esi                               #
                                # LOE eax esi
..B18.17:                       # Preds ..B18.16 ..B18.42
        addl      $3, %eax                                      #1060.21
        shrl      $2, %eax                                      #1060.28
        cmpl      $8, %eax                                      #1060.35
        jae       ..B18.30      # Prob 50%                      #1060.35
                                # LOE eax esi
..B18.18:                       # Preds ..B18.17
        lea       -40(%ebp,%eax,2), %ecx                        #1060.5
        movl      %eax, %edi                                    #1060.5
        negl      %edi                                          #1060.5
        addl      $8, %edi                                      #1060.5
        lea       (%eax,%eax), %edx                             #
        andl      $15, %ecx                                     #1060.5
        movl      %edx, -56(%ebp)                               #
        movl      %ecx, %edx                                    #1060.5
        negl      %edx                                          #1060.5
        addl      $16, %edx                                     #1060.5
        shrl      $1, %edx                                      #1060.5
        cmpl      $0, %ecx                                      #1060.5
        cmovne    %edx, %ecx                                    #1060.5
        movl      %ecx, -52(%ebp)                               #1060.5
        lea       8(%ecx), %edx                                 #1060.5
        cmpl      %edx, %edi                                    #1060.5
        jb        ..B18.40      # Prob 10%                      #1060.5
                                # LOE eax ecx esi edi cl ch
..B18.19:                       # Preds ..B18.18
        movl      %edi, %edx                                    #1060.5
        subl      %ecx, %edx                                    #1060.5
        andl      $7, %edx                                      #1060.5
        negl      %edx                                          #1060.5
        addl      %edi, %edx                                    #1060.5
        testl     %ecx, %ecx                                    #1060.5
        jbe       ..B18.23      # Prob 1%                       #1060.5
                                # LOE eax edx ecx esi edi cl ch
..B18.20:                       # Preds ..B18.19
        movl      %edi, -68(%ebp)                               #
        movl      %ecx, %edi                                    #
        movl      %eax, -64(%ebp)                               #
        xorl      %ecx, %ecx                                    #
        movl      %esi, -60(%ebp)                               #
        movl      -56(%ebp), %esi                               #
                                # LOE edx ecx esi edi
..B18.21:                       # Preds ..B18.21 ..B18.20
        xorl      %eax, %eax                                    #1062.9
        movw      %ax, -40(%ebp,%esi)                           #1062.9
        addl      $1, %ecx                                      #1060.5
        movw      %ax, -24(%ebp,%esi)                           #1063.9
        addl      $2, %esi                                      #1060.5
        cmpl      %edi, %ecx                                    #1060.5
        jb        ..B18.21      # Prob 99%                      #1060.5
                                # LOE edx ecx esi edi
..B18.22:                       # Preds ..B18.21
        movl      -64(%ebp), %eax                               #
        movl      -60(%ebp), %esi                               #
        movl      %edi, -52(%ebp)                               #
        movl      -68(%ebp), %edi                               #
                                # LOE eax edx esi edi
..B18.23:                       # Preds ..B18.22 ..B18.19
        movl      -52(%ebp), %ecx                               #1062.29
        movl      %esi, -60(%ebp)                               #1062.29
        movl      %ecx, %esi                                    #1062.29
        lea       (%ecx,%ecx), %ecx                             #1062.29
        pxor      %xmm0, %xmm0                                  #1062.29
        lea       (%ecx,%eax,2), %ecx                           #1062.29
                                # LOE eax edx ecx esi edi xmm0
..B18.24:                       # Preds ..B18.24 ..B18.23
        movdqa    %xmm0, -40(%ebp,%ecx)                         #1062.9
        movdqa    %xmm0, -24(%ebp,%ecx)                         #1063.9
        addl      $16, %ecx                                     #1060.5
        addl      $8, %esi                                      #1060.5
        cmpl      %edx, %esi                                    #1060.5
        jb        ..B18.24      # Prob 99%                      #1060.5
                                # LOE eax edx ecx esi edi xmm0
..B18.25:                       # Preds ..B18.24
        movl      -60(%ebp), %esi                               #
                                # LOE eax edx esi edi
..B18.26:                       # Preds ..B18.25 ..B18.40
        cmpl      %edi, %edx                                    #1060.5
        jae       ..B18.30      # Prob 1%                       #1060.5
                                # LOE eax edx esi edi
..B18.27:                       # Preds ..B18.26
        lea       (%edx,%edx), %ecx                             #
        lea       (%ecx,%eax,2), %eax                           #
                                # LOE eax edx esi edi
..B18.28:                       # Preds ..B18.28 ..B18.27
        xorl      %ecx, %ecx                                    #1062.9
        movw      %cx, -40(%ebp,%eax)                           #1062.9
        addl      $1, %edx                                      #1060.5
        movw      %cx, -24(%ebp,%eax)                           #1063.9
        addl      $2, %eax                                      #1060.5
        cmpl      %edi, %edx                                    #1060.5
        jb        ..B18.28      # Prob 99%                      #1060.5
                                # LOE eax edx esi edi
..B18.30:                       # Preds ..B18.28 ..B18.26 ..B18.17
        movl      12(%ebx), %eax                                #1068.10
        testl     %eax, %eax                                    #1068.10
        je        ..B18.32      # Prob 50%                      #1068.10
                                # LOE esi
..B18.31:                       # Preds ..B18.30
        movl      $10, (%esi)                                   #1070.9
        jmp       ..B18.39      # Prob 100%                     #1070.9
                                # LOE esi
..B18.32:                       # Preds ..B18.30
        movl      20(%ebx), %eax                                #1072.22
        cmpl      $2, %eax                                      #1072.22
        jae       ..B18.34      # Prob 50%                      #1072.22
                                # LOE esi
..B18.33:                       # Preds ..B18.32
        movzwl    -40(%ebp), %eax                               #1074.29
        movl      $6, (%esi)                                    #1082.9
        movw      %ax, -38(%ebp)                                #1074.9
        movl      -40(%ebp), %edi                               #1075.29
        movzwl    -24(%ebp), %eax                               #1078.26
        movl      %edi, -36(%ebp)                               #1075.9
        movl      -40(%ebp), %edx                               #1076.29
        movl      %edx, -32(%ebp)                               #1076.9
        movl      %edi, -28(%ebp)                               #1076.9
        movw      %ax, -22(%ebp)                                #1078.9
        movl      -24(%ebp), %edx                               #1079.26
        movl      %edx, -20(%ebp)                               #1079.9
        movl      -24(%ebp), %eax                               #1080.26
        movl      %eax, -16(%ebp)                               #1080.9
        movl      %edx, -12(%ebp)                               #1080.9
        jmp       ..B18.39      # Prob 100%                     #1080.9
                                # LOE esi
..B18.34:                       # Preds ..B18.32
        movl      20(%ebx), %eax                                #1084.22
        cmpl      $6, %eax                                      #1084.22
        jae       ..B18.36      # Prob 50%                      #1084.22
                                # LOE esi
..B18.35:                       # Preds ..B18.34
        movl      -40(%ebp), %edx                               #1086.29
        movl      $7, (%esi)                                    #1092.9
        movl      %edx, -36(%ebp)                               #1086.9
        movl      -40(%ebp), %eax                               #1087.29
        movl      %eax, -32(%ebp)                               #1087.9
        movl      -24(%ebp), %eax                               #1089.26
        movl      %edx, -28(%ebp)                               #1087.9
        movl      %eax, -20(%ebp)                               #1089.9
        movl      -24(%ebp), %edi                               #1090.26
        movl      %edi, -16(%ebp)                               #1090.9
        movl      %eax, -12(%ebp)                               #1090.9
        jmp       ..B18.39      # Prob 100%                     #1090.9
                                # LOE esi
..B18.36:                       # Preds ..B18.34
        movl      20(%ebx), %eax                                #1094.22
        cmpl      $14, %eax                                     #1094.22
        jae       ..B18.38      # Prob 50%                      #1094.22
                                # LOE esi
..B18.37:                       # Preds ..B18.36
        movl      -40(%ebp), %eax                               #1096.29
        movl      -36(%ebp), %edx                               #1096.29
        movl      -24(%ebp), %edi                               #1097.26
        movl      %eax, -32(%ebp)                               #1096.9
        movl      -20(%ebp), %eax                               #1097.26
        movl      $1, (%esi)                                    #1099.9
        movl      %edx, -28(%ebp)                               #1096.9
        movl      %edi, -16(%ebp)                               #1097.9
        movl      %eax, -12(%ebp)                               #1097.9
        jmp       ..B18.39      # Prob 100%                     #1097.9
                                # LOE esi
..B18.38:                       # Preds ..B18.36
        movl      $8, (%esi)                                    #1103.9
                                # LOE esi
..B18.39:                       # Preds ..B18.31 ..B18.33 ..B18.35 ..B18.37 ..B18.38
                                #      
        movl      -40(%ebp), %eax                               #1106.42
        movl      -36(%ebp), %edi                               #1106.42
        lea       -40(%ebp), %ecx                               #1110.5
        movl      %eax, 16(%esi)                                #1106.42
        movl      -32(%ebp), %eax                               #1106.42
        movl      %edi, 20(%esi)                                #1106.42
        movl      %eax, 24(%esi)                                #1106.42
        movl      -28(%ebp), %eax                               #1106.42
        movl      %eax, 28(%esi)                                #1106.42
        movl      -24(%ebp), %eax                               #1107.39
        movl      %eax, 32(%esi)                                #1107.39
        movl      -20(%ebp), %eax                               #1107.39
        movl      %eax, 36(%esi)                                #1107.39
        movl      -16(%ebp), %eax                               #1107.39
        movl      %eax, 40(%esi)                                #1107.39
        movl      -12(%ebp), %eax                               #1107.39
        movl      %eax, 44(%esi)                                #1107.39
# Begin ASM
        mov (%ecx), %eax;mov 12(%ecx), %edx;bswap %eax;bswap %edx;mov %eax, 12(%ecx);mov %edx, (%ecx);mov 4(%ecx), %eax;mov 8(%ecx), %edx;bswap %eax;bswap %edx;mov %eax, 8(%ecx);mov %edx, 4(%ecx);
# End ASM                                                       #1110.5
        lea       -24(%ebp), %ecx                               #1111.5
# Begin ASM
        mov (%ecx), %eax;mov 12(%ecx), %edx;bswap %eax;bswap %edx;mov %eax, 12(%ecx);mov %edx, (%ecx);mov 4(%ecx), %eax;mov 8(%ecx), %edx;bswap %eax;bswap %edx;mov %eax, 8(%ecx);mov %edx, 4(%ecx);
# End ASM                                                       #1111.5
        movl      $4, %ecx                                      #1114.5
        lea       -40(%ebp), %edx                               #1114.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #1114.5
        lea       -24(%ebp), %edx                               #1115.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #1115.5
        lea       48(%esi), %ecx                                #1118.5
        lea       -40(%ebp), %edx                               #1118.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #1118.5
        lea       64(%esi), %ecx                                #1119.5
        lea       -24(%ebp), %edx                               #1119.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #1119.5
        movl      $4, %ecx                                      #1121.5
        lea       -40(%ebp), %edx                               #1121.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #1121.5
        lea       -24(%ebp), %edx                               #1122.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #1122.5
        lea       80(%esi), %ecx                                #1124.5
        lea       -40(%ebp), %edx                               #1124.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #1124.5
        lea       96(%esi), %ecx                                #1125.5
        lea       -24(%ebp), %edx                               #1125.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #1125.5
        movl      $4, %ecx                                      #1127.5
        lea       -40(%ebp), %edx                               #1127.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #1127.5
        lea       -24(%ebp), %edx                               #1128.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #1128.5
        lea       112(%esi), %ecx                               #1130.5
        lea       -40(%ebp), %edx                               #1130.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #1130.5
        addl      $128, %esi                                    #1131.5
        movl      %esi, %ecx                                    #1131.5
        lea       -24(%ebp), %edx                               #1131.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #1131.5
        xorl      %eax, %eax                                    #1173.12
        movl      -48(%ebp), %esi                               #1173.12
        movl      -44(%ebp), %edi                               #1173.12
        movl      %ebp, %esp                                    #1173.12
        popl      %ebp                                          #1173.12
        movl      %ebx, %esp                                    #1173.12
        popl      %ebx                                          #1173.12
        ret                                                     #1173.12
                                # LOE
..B18.40:                       # Preds ..B18.18                # Infreq
        xorl      %edx, %edx                                    #1060.5
        jmp       ..B18.26      # Prob 100%                     #1060.5
                                # LOE eax edx esi edi
..B18.41:                       # Preds ..B18.8                 # Infreq
        movl      %eax, %esi                                    #1041.32
        shrl      $2, %esi                                      #1041.32
        shll      $4, %edx                                      #1041.46
        movw      %dx, -40(%ebp,%esi,2)                         #1041.13
        movl      $240, %edx                                    #1042.13
        movw      %dx, -24(%ebp,%esi,2)                         #1042.13
        jmp       ..B18.15      # Prob 100%                     #1042.13
                                # LOE eax ecx edi
..B18.42:                       # Preds ..B18.6                 # Infreq
        xorl      %eax, %eax                                    #
        jmp       ..B18.17      # Prob 100%                     #
                                # LOE eax esi
..B18.43:                       # Preds ..B18.4 ..B18.5         # Infreq
        movl      %edi, %ebx                                    #1028.16
        call      __errno_location@PLT                          #1028.16
        movl      -72(%ebp), %ebx                               #1028.16
                                # LOE eax
..B18.48:                       # Preds ..B18.43                # Infreq
        movl      -48(%ebp), %esi                               #1028.16
        movl      -44(%ebp), %edi                               #1028.16
        movl      (%eax), %eax                                  #1028.16
        movl      %ebp, %esp                                    #1028.16
        popl      %ebp                                          #1028.16
        movl      %ebx, %esp                                    #1028.16
        popl      %ebx                                          #1028.16
        ret                                                     #1028.16
        .align    2,0x90
                                # LOE
# mark_end;
	.type	NucStrFastaExprMake4,@function
	.size	NucStrFastaExprMake4,.-NucStrFastaExprMake4
.LNNucStrFastaExprMake4:
	.data
# -- End  NucStrFastaExprMake4
	.text
# -- Begin  NucStrFastaExprMake2
# mark_begin;
       .align    2,0x90
NucStrFastaExprMake2:
# parameter 1(expr): 8 + %ebx
# parameter 2(positional): 12 + %ebx
# parameter 3(fasta): 16 + %ebx
# parameter 4(size): 20 + %ebx
..B19.1:                        # Preds ..B19.0
        pushl     %ebx                                          #838.1
        movl      %esp, %ebx                                    #838.1
        andl      $-16, %esp                                    #838.1
        pushl     %ebp                                          #838.1
        pushl     %ebp                                          #838.1
        movl      4(%ebx), %ebp                                 #838.1
        movl      %ebp, 4(%esp)                                 #838.1
        movl      %esp, %ebp                                    #838.1
        subl      $72, %esp                                     #838.1
        movl      %ebx, -72(%ebp)                               #838.1
        movl      %edi, -48(%ebp)                               #838.1
        movl      %esi, -52(%ebp)                               #838.1
        call      ..L19         # Prob 100%                     #838.1
..L19:                                                          #
        popl      %edi                                          #838.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L19], %edi      #838.1
        lea       _gprof_pack18@GOTOFF(%edi), %edx              #838.1
        movl      %edi, %ebx                                    #838.1
        call      mcount@PLT                                    #838.1
        movl      -72(%ebp), %ebx                               #838.1
                                # LOE edi
..B19.48:                       # Preds ..B19.1
        movl      20(%ebx), %eax                                #844.17
        cmpl      $61, %eax                                     #844.17
        jbe       ..B19.3       # Prob 43%                      #844.17
                                # LOE edi
..B19.2:                        # Preds ..B19.48
        movl      -52(%ebp), %esi                               #845.16
        movl      -48(%ebp), %edi                               #845.16
        movl      $7, %eax                                      #845.16
        movl      %ebp, %esp                                    #845.16
        popl      %ebp                                          #845.16
        movl      %ebx, %esp                                    #845.16
        popl      %ebx                                          #845.16
        ret                                                     #845.16
                                # LOE
..B19.3:                        # Preds ..B19.48
        pushl     $160                                          #847.32
        movl      %edi, %ebx                                    #847.9
        call      malloc@PLT                                    #847.9
        movl      -72(%ebp), %ebx                               #847.9
                                # LOE eax edi
..B19.49:                       # Preds ..B19.3
        addl      $4, %esp                                      #847.9
                                # LOE eax edi
..B19.4:                        # Preds ..B19.49
        testl     %eax, %eax                                    #847.9
        je        ..B19.45      # Prob 12%                      #847.9
                                # LOE eax edi
..B19.5:                        # Preds ..B19.4
        lea       15(%eax), %esi                                #847.9
        andl      $-16, %esi                                    #847.9
        movl      %eax, 8(%esi)                                 #847.9
        je        ..B19.45      # Prob 5%                       #848.15
                                # LOE esi edi
..B19.6:                        # Preds ..B19.5
        movl      8(%ebx), %eax                                 #851.5
        movl      20(%ebx), %edx                                #852.5
        movl      %esi, (%eax)                                  #851.5
        movl      %edx, 4(%esi)                                 #852.5
        testl     %edx, %edx                                    #856.22
        jbe       ..B19.44      # Prob 16%                      #856.22
                                # LOE esi edi
..B19.7:                        # Preds ..B19.6
        movl      16(%ebx), %edx                                #
        movl      %esi, -60(%ebp)                               #
        movl      %edi, -44(%ebp)                               #
        xorl      %eax, %eax                                    #
                                # LOE eax edx
..B19.8:                        # Preds ..B19.15 ..B19.7
        movl      -44(%ebp), %ecx                               #858.24
        movsbl    (%edx,%eax), %esi                             #858.48
        movzbl    fasta_2na_map.0@GOTOFF(%ecx,%esi), %ecx       #858.24
        movl      %eax, %esi                                    #859.22
        andl      $3, %esi                                      #859.22
        je        ..B19.43      # Prob 20%                      #859.22
                                # LOE eax edx ecx esi
..B19.9:                        # Preds ..B19.8
        cmpl      $1, %esi                                      #859.22
        je        ..B19.14      # Prob 25%                      #859.22
                                # LOE eax edx ecx esi
..B19.10:                       # Preds ..B19.9
        cmpl      $2, %esi                                      #859.22
        jne       ..B19.12      # Prob 67%                      #859.22
                                # LOE eax edx ecx esi
..B19.11:                       # Preds ..B19.10
        movl      %eax, %esi                                    #870.32
        shrl      $2, %esi                                      #870.32
        orb       $12, -24(%ebp,%esi)                           #871.13
        shll      $2, %ecx                                      #870.47
        orb       %cl, -40(%ebp,%esi)                           #870.13
        jmp       ..B19.15      # Prob 100%                     #870.13
                                # LOE eax edx
..B19.12:                       # Preds ..B19.10
        cmpl      $3, %esi                                      #859.22
        jne       ..B19.15      # Prob 50%                      #859.22
                                # LOE eax edx ecx
..B19.13:                       # Preds ..B19.12
        movl      %eax, %esi                                    #874.32
        shrl      $2, %esi                                      #874.32
        orb       $3, -24(%ebp,%esi)                            #875.13
        orb       %cl, -40(%ebp,%esi)                           #874.13
        jmp       ..B19.15      # Prob 100%                     #874.13
                                # LOE eax edx
..B19.14:                       # Preds ..B19.9
        movl      %eax, %esi                                    #866.32
        shrl      $2, %esi                                      #866.32
        orb       $48, -24(%ebp,%esi)                           #867.13
        shll      $4, %ecx                                      #866.47
        orb       %cl, -40(%ebp,%esi)                           #866.13
                                # LOE eax edx
..B19.15:                       # Preds ..B19.43 ..B19.14 ..B19.11 ..B19.13 ..B19.12
                                #      
        movl      20(%ebx), %ecx                                #856.22
        addl      $1, %eax                                      #856.31
        cmpl      %ecx, %eax                                    #856.22
        jb        ..B19.8       # Prob 82%                      #856.22
                                # LOE eax edx
..B19.16:                       # Preds ..B19.15
        movl      -60(%ebp), %esi                               #
                                # LOE eax esi
..B19.17:                       # Preds ..B19.16 ..B19.44
        addl      $3, %eax                                      #881.21
        shrl      $2, %eax                                      #881.28
        cmpl      $16, %eax                                     #881.35
        jae       ..B19.30      # Prob 50%                      #881.35
                                # LOE eax esi
..B19.18:                       # Preds ..B19.17
        lea       -40(%ebp), %edx                               #883.9
        movl      %eax, %ecx                                    #881.5
        negl      %ecx                                          #881.5
        addl      $16, %ecx                                     #881.5
        lea       (%edx,%eax), %edx                             #881.5
        andl      $15, %edx                                     #881.5
        movl      %edx, %edi                                    #881.5
        negl      %edi                                          #881.5
        addl      $16, %edi                                     #881.5
        cmpl      $0, %edx                                      #881.5
        cmovne    %edi, %edx                                    #881.5
        movl      %edx, -56(%ebp)                               #881.5
        lea       16(%edx), %edx                                #881.5
        cmpl      %edx, %ecx                                    #881.5
        jb        ..B19.42      # Prob 10%                      #881.5
                                # LOE eax ecx esi
..B19.19:                       # Preds ..B19.18
        movl      -56(%ebp), %edx                               #881.5
        movl      %ecx, %edi                                    #881.5
        subl      %edx, %edi                                    #881.5
        andl      $15, %edi                                     #881.5
        negl      %edi                                          #881.5
        addl      %ecx, %edi                                    #881.5
        testl     %edx, %edx                                    #881.5
        jbe       ..B19.23      # Prob 1%                       #881.5
                                # LOE eax edx ecx esi edi dl dh
..B19.20:                       # Preds ..B19.19
        movl      %eax, -64(%ebp)                               #
        movl      %ecx, -68(%ebp)                               #
        movl      %eax, %ecx                                    #
        movl      %esi, -60(%ebp)                               #
        lea       (%edx,%eax), %edx                             #
        movl      %edx, -44(%ebp)                               #
        movl      %edx, %esi                                    #
                                # LOE eax ecx esi edi
..B19.21:                       # Preds ..B19.21 ..B19.20
        xorl      %edx, %edx                                    #883.9
        movb      %dl, -40(%ebp,%ecx)                           #883.9
        movb      %dl, -24(%ebp,%ecx)                           #884.9
        addl      $1, %ecx                                      #881.5
        cmpl      %esi, %ecx                                    #881.5
        jb        ..B19.21      # Prob 99%                      #881.5
                                # LOE eax ecx esi edi
..B19.22:                       # Preds ..B19.21
        movl      -68(%ebp), %ecx                               #
        movl      -60(%ebp), %esi                               #
                                # LOE eax ecx esi edi
..B19.23:                       # Preds ..B19.22 ..B19.19
        movl      -56(%ebp), %edx                               #883.29
        movl      %esi, -60(%ebp)                               #883.29
        movl      %edx, %esi                                    #883.29
        lea       (%edx,%eax), %edx                             #883.29
        pxor      %xmm0, %xmm0                                  #883.29
        movl      %edx, -44(%ebp)                               #883.29
                                # LOE eax edx ecx esi edi xmm0
..B19.24:                       # Preds ..B19.24 ..B19.23
        movdqa    %xmm0, -40(%ebp,%edx)                         #883.9
        movdqa    %xmm0, -24(%ebp,%edx)                         #884.9
        addl      $16, %edx                                     #881.5
        addl      $16, %esi                                     #881.5
        cmpl      %edi, %esi                                    #881.5
        jb        ..B19.24      # Prob 99%                      #881.5
                                # LOE eax edx ecx esi edi xmm0
..B19.25:                       # Preds ..B19.24
        movl      -60(%ebp), %esi                               #
                                # LOE eax ecx esi edi
..B19.26:                       # Preds ..B19.25 ..B19.42
        cmpl      %ecx, %edi                                    #881.5
        jae       ..B19.30      # Prob 1%                       #881.5
                                # LOE eax ecx esi edi
..B19.27:                       # Preds ..B19.26
        addl      %edi, %eax                                    #
                                # LOE eax ecx esi edi
..B19.28:                       # Preds ..B19.28 ..B19.27
        xorl      %edx, %edx                                    #883.9
        movb      %dl, -40(%ebp,%eax)                           #883.9
        addl      $1, %edi                                      #881.5
        movb      %dl, -24(%ebp,%eax)                           #884.9
        addl      $1, %eax                                      #881.5
        cmpl      %ecx, %edi                                    #881.5
        jb        ..B19.28      # Prob 99%                      #881.5
                                # LOE eax ecx esi edi
..B19.30:                       # Preds ..B19.28 ..B19.26 ..B19.17
        movl      12(%ebx), %eax                                #890.10
        testl     %eax, %eax                                    #890.10
        je        ..B19.32      # Prob 50%                      #890.10
                                # LOE esi
..B19.31:                       # Preds ..B19.30
        movl      $9, (%esi)                                    #892.9
        jmp       ..B19.41      # Prob 100%                     #892.9
                                # LOE esi
..B19.32:                       # Preds ..B19.30
        movl      20(%ebx), %eax                                #896.22
        cmpl      $2, %eax                                      #896.22
        jae       ..B19.34      # Prob 50%                      #896.22
                                # LOE esi
..B19.33:                       # Preds ..B19.32
        movzbl    -40(%ebp), %eax                               #898.29
        movl      $2, (%esi)                                    #908.9
        movb      %al, -39(%ebp)                                #898.9
        movzwl    -40(%ebp), %edi                               #899.29
        movw      %di, -38(%ebp)                                #899.9
        movl      -40(%ebp), %eax                               #900.29
        movl      %eax, -36(%ebp)                               #900.9
        movl      -40(%ebp), %edx                               #901.29
        movl      %edx, -32(%ebp)                               #901.9
        movl      %eax, -28(%ebp)                               #901.9
        movzbl    -24(%ebp), %eax                               #903.26
        movb      %al, -23(%ebp)                                #903.9
        movzwl    -24(%ebp), %eax                               #904.26
        movw      %ax, -22(%ebp)                                #904.9
        movl      -24(%ebp), %edx                               #905.26
        movl      %edx, -20(%ebp)                               #905.9
        movl      -24(%ebp), %eax                               #906.26
        movl      %eax, -16(%ebp)                               #906.9
        movl      %edx, -12(%ebp)                               #906.9
        jmp       ..B19.41      # Prob 100%                     #906.9
                                # LOE esi
..B19.34:                       # Preds ..B19.32
        movl      20(%ebx), %eax                                #910.22
        cmpl      $6, %eax                                      #910.22
        jae       ..B19.36      # Prob 50%                      #910.22
                                # LOE esi
..B19.35:                       # Preds ..B19.34
        movzwl    -40(%ebp), %eax                               #912.29
        movl      $3, (%esi)                                    #920.9
        movw      %ax, -38(%ebp)                                #912.9
        movl      -40(%ebp), %edi                               #913.29
        movzwl    -24(%ebp), %eax                               #916.26
        movl      %edi, -36(%ebp)                               #913.9
        movl      -40(%ebp), %edx                               #914.29
        movl      %edx, -32(%ebp)                               #914.9
        movl      %edi, -28(%ebp)                               #914.9
        movw      %ax, -22(%ebp)                                #916.9
        movl      -24(%ebp), %edx                               #917.26
        movl      %edx, -20(%ebp)                               #917.9
        movl      -24(%ebp), %eax                               #918.26
        movl      %eax, -16(%ebp)                               #918.9
        movl      %edx, -12(%ebp)                               #918.9
        jmp       ..B19.41      # Prob 100%                     #918.9
                                # LOE esi
..B19.36:                       # Preds ..B19.34
        movl      20(%ebx), %eax                                #922.22
        cmpl      $14, %eax                                     #922.22
        jae       ..B19.38      # Prob 50%                      #922.22
                                # LOE esi
..B19.37:                       # Preds ..B19.36
        movl      -40(%ebp), %edx                               #924.29
        movl      $4, (%esi)                                    #930.9
        movl      %edx, -36(%ebp)                               #924.9
        movl      -40(%ebp), %eax                               #925.29
        movl      %eax, -32(%ebp)                               #925.9
        movl      -24(%ebp), %eax                               #927.26
        movl      %edx, -28(%ebp)                               #925.9
        movl      %eax, -20(%ebp)                               #927.9
        movl      -24(%ebp), %edi                               #928.26
        movl      %edi, -16(%ebp)                               #928.9
        movl      %eax, -12(%ebp)                               #928.9
        jmp       ..B19.41      # Prob 100%                     #928.9
                                # LOE esi
..B19.38:                       # Preds ..B19.36
        movl      20(%ebx), %eax                                #932.22
        cmpl      $30, %eax                                     #932.22
        jae       ..B19.40      # Prob 50%                      #932.22
                                # LOE esi
..B19.39:                       # Preds ..B19.38
        movl      -40(%ebp), %eax                               #934.29
        movl      -36(%ebp), %edx                               #934.29
        movl      -24(%ebp), %edi                               #935.26
        movl      %eax, -32(%ebp)                               #934.9
        movl      -20(%ebp), %eax                               #935.26
        movl      $0, (%esi)                                    #937.9
        movl      %edx, -28(%ebp)                               #934.9
        movl      %edi, -16(%ebp)                               #935.9
        movl      %eax, -12(%ebp)                               #935.9
        jmp       ..B19.41      # Prob 100%                     #935.9
                                # LOE esi
..B19.40:                       # Preds ..B19.38
        movl      $5, (%esi)                                    #941.9
                                # LOE esi
..B19.41:                       # Preds ..B19.31 ..B19.33 ..B19.35 ..B19.37 ..B19.39
                                #       ..B19.40
        movl      -40(%ebp), %eax                               #944.42
        movl      -36(%ebp), %edi                               #944.42
        lea       -40(%ebp), %ecx                               #948.5
        movl      %eax, 16(%esi)                                #944.42
        movl      -32(%ebp), %eax                               #944.42
        movl      %edi, 20(%esi)                                #944.42
        movl      %eax, 24(%esi)                                #944.42
        movl      -28(%ebp), %eax                               #944.42
        movl      %eax, 28(%esi)                                #944.42
        movl      -24(%ebp), %eax                               #945.39
        movl      %eax, 32(%esi)                                #945.39
        movl      -20(%ebp), %eax                               #945.39
        movl      %eax, 36(%esi)                                #945.39
        movl      -16(%ebp), %eax                               #945.39
        movl      %eax, 40(%esi)                                #945.39
        movl      -12(%ebp), %eax                               #945.39
        movl      %eax, 44(%esi)                                #945.39
# Begin ASM
        mov (%ecx), %eax;mov 12(%ecx), %edx;bswap %eax;bswap %edx;mov %eax, 12(%ecx);mov %edx, (%ecx);mov 4(%ecx), %eax;mov 8(%ecx), %edx;bswap %eax;bswap %edx;mov %eax, 8(%ecx);mov %edx, 4(%ecx);
# End ASM                                                       #948.5
        lea       -24(%ebp), %ecx                               #949.5
# Begin ASM
        mov (%ecx), %eax;mov 12(%ecx), %edx;bswap %eax;bswap %edx;mov %eax, 12(%ecx);mov %edx, (%ecx);mov 4(%ecx), %eax;mov 8(%ecx), %edx;bswap %eax;bswap %edx;mov %eax, 8(%ecx);mov %edx, 4(%ecx);
# End ASM                                                       #949.5
        movl      $2, %ecx                                      #952.5
        lea       -40(%ebp), %edx                               #952.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #952.5
        lea       -24(%ebp), %edx                               #953.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #953.5
        lea       48(%esi), %ecx                                #956.5
        lea       -40(%ebp), %edx                               #956.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #956.5
        lea       64(%esi), %ecx                                #957.5
        lea       -24(%ebp), %edx                               #957.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #957.5
        movl      $2, %ecx                                      #959.5
        lea       -40(%ebp), %edx                               #959.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #959.5
        lea       -24(%ebp), %edx                               #960.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #960.5
        lea       80(%esi), %ecx                                #962.5
        lea       -40(%ebp), %edx                               #962.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #962.5
        lea       96(%esi), %ecx                                #963.5
        lea       -24(%ebp), %edx                               #963.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #963.5
        movl      $2, %ecx                                      #965.5
        lea       -40(%ebp), %edx                               #965.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #965.5
        lea       -24(%ebp), %edx                               #966.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #966.5
        lea       112(%esi), %ecx                               #968.5
        lea       -40(%ebp), %edx                               #968.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #968.5
        addl      $128, %esi                                    #969.5
        movl      %esi, %ecx                                    #969.5
        lea       -24(%ebp), %edx                               #969.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #969.5
        xorl      %eax, %eax                                    #1011.12
        movl      -52(%ebp), %esi                               #1011.12
        movl      -48(%ebp), %edi                               #1011.12
        movl      %ebp, %esp                                    #1011.12
        popl      %ebp                                          #1011.12
        movl      %ebx, %esp                                    #1011.12
        popl      %ebx                                          #1011.12
        ret                                                     #1011.12
                                # LOE
..B19.42:                       # Preds ..B19.18                # Infreq
        xorl      %edi, %edi                                    #881.5
        jmp       ..B19.26      # Prob 100%                     #881.5
                                # LOE eax ecx esi edi
..B19.43:                       # Preds ..B19.8                 # Infreq
        movl      %eax, %esi                                    #862.32
        shrl      $2, %esi                                      #862.32
        movb      $192, -24(%ebp,%esi)                          #863.13
        shll      $6, %ecx                                      #862.46
        movb      %cl, -40(%ebp,%esi)                           #862.13
        jmp       ..B19.15      # Prob 100%                     #862.13
                                # LOE eax edx
..B19.44:                       # Preds ..B19.6                 # Infreq
        xorl      %eax, %eax                                    #
        jmp       ..B19.17      # Prob 100%                     #
                                # LOE eax esi
..B19.45:                       # Preds ..B19.4 ..B19.5         # Infreq
        movl      %edi, %ebx                                    #849.16
        call      __errno_location@PLT                          #849.16
        movl      -72(%ebp), %ebx                               #849.16
                                # LOE eax
..B19.50:                       # Preds ..B19.45                # Infreq
        movl      -52(%ebp), %esi                               #849.16
        movl      -48(%ebp), %edi                               #849.16
        movl      (%eax), %eax                                  #849.16
        movl      %ebp, %esp                                    #849.16
        popl      %ebp                                          #849.16
        movl      %ebx, %esp                                    #849.16
        popl      %ebx                                          #849.16
        ret                                                     #849.16
        .align    2,0x90
                                # LOE
# mark_end;
	.type	NucStrFastaExprMake2,@function
	.size	NucStrFastaExprMake2,.-NucStrFastaExprMake2
.LNNucStrFastaExprMake2:
	.data
# -- End  NucStrFastaExprMake2
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
..B20.1:                        # Preds ..B20.0
        pushl     %ebp                                          #1406.1
        movl      %esp, %ebp                                    #1406.1
        subl      $28, %esp                                     #1406.1
        movl      %edi, -16(%ebp)                               #1406.1
        movl      %esi, -20(%ebp)                               #1406.1
        movl      %ebx, -12(%ebp)                               #1406.1
        movl      8(%ebp), %esi                                 #1404.13
        movl      16(%ebp), %edi                                #1404.13
        call      ..L20         # Prob 100%                     #1406.1
..L20:                                                          #
        popl      %ebx                                          #1406.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L20], %ebx      #1406.1
        lea       _gprof_pack19@GOTOFF(%ebx), %edx              #1406.1
        call      mcount@PLT                                    #1406.1
                                # LOE ebx esi edi
..B20.85:                       # Preds ..B20.1
        call      __ctype_b_loc@PLT                             #1409.9
                                # LOE eax ebx esi edi
..B20.86:                       # Preds ..B20.85
        movl      20(%ebp), %edx                                #1409.9
        movl      12(%ebp), %ecx                                #1409.9
        movl      %ebx, -8(%ebp)                                #1409.9
        movl      %eax, -4(%ebp)                                #1409.9
        jmp       ..B20.2       # Prob 100%                     #1409.9
                                # LOE edx ecx esi edi
..B20.54:                       # Preds ..B20.53
        movl      -24(%ebp), %ebx                               #1459.17
        movl      $11, (%eax)                                   #1458.17
        movl      %ebx, 4(%eax)                                 #1459.17
        movl      (%edi), %ebx                                  #1460.41
        movl      %ebx, 8(%eax)                                 #1460.17
        movl      %eax, (%edi)                                  #1461.17
        lea       12(%eax), %edi                                #1464.42
                                # LOE edx ecx esi edi
..B20.2:                        # Preds ..B20.86 ..B20.54
        movl      $0, (%edi)                                    #1407.5
        cmpl      %ecx, %esi                                    #1409.9
        jae       ..B20.8       # Prob 12%                      #1409.9
                                # LOE edx ecx esi edi
..B20.3:                        # Preds ..B20.2
        movl      -4(%ebp), %eax                                #1409.9
        movl      (%eax), %eax                                  #1409.9
        movzbl    (%esi), %ebx                                  #1409.9
        movzwl    (%eax,%ebx,2), %ebx                           #1409.9
        testl     $8192, %ebx                                   #1409.9
        je        ..B20.8       # Prob 15%                      #1409.9
                                # LOE eax edx ecx esi edi
..B20.5:                        # Preds ..B20.3 ..B20.6
        addl      $1, %esi                                      #1409.9
        cmpl      %ecx, %esi                                    #1409.9
        jae       ..B20.8       # Prob 1%                       #1409.9
                                # LOE eax edx ecx esi edi
..B20.6:                        # Preds ..B20.5
        movzbl    (%esi), %ebx                                  #1409.9
        movzwl    (%eax,%ebx,2), %ebx                           #1409.9
        testl     $8192, %ebx                                   #1409.9
        jne       ..B20.5       # Prob 82%                      #1409.9
                                # LOE eax edx ecx esi edi
..B20.8:                        # Preds ..B20.5 ..B20.6 ..B20.3 ..B20.2
        cmpl      %ecx, %esi                                    #1410.15
        je        ..B20.58      # Prob 16%                      #1410.15
                                # LOE edx ecx esi edi
..B20.9:                        # Preds ..B20.8
        movsbl    (%esi), %eax                                  #1412.13
        cmpl      $33, %eax                                     #1412.13
        je        ..B20.65      # Prob 16%                      #1412.13
                                # LOE eax edx ecx esi edi
..B20.10:                       # Preds ..B20.9
        cmpl      $94, %eax                                     #1412.13
        jne       ..B20.17      # Prob 67%                      #1412.13
                                # LOE eax edx ecx esi edi
..B20.11:                       # Preds ..B20.10
        movl      -8(%ebp), %ebx                                #1412.13
        pushl     $12                                           #1412.13
        call      malloc@PLT                                    #1412.13
                                # LOE eax esi edi
..B20.87:                       # Preds ..B20.11
        movl      20(%ebp), %edx                                #
        movl      12(%ebp), %ecx                                #
        addl      $4, %esp                                      #1412.13
        movl      %eax, %ebx                                    #1412.13
                                # LOE edx ecx ebx esi edi
..B20.12:                       # Preds ..B20.87
        testl     %ebx, %ebx                                    #1412.13
        jne       ..B20.14      # Prob 68%                      #1412.13
                                # LOE edx ecx ebx esi edi
..B20.13:                       # Preds ..B20.12
        movl      -8(%ebp), %ebx                                #1412.13
        call      __errno_location@PLT                          #1412.13
                                # LOE eax esi edi
..B20.88:                       # Preds ..B20.13
        movl      20(%ebp), %edx                                #
        movl      12(%ebp), %ecx                                #
        movl      (%eax), %ebx                                  #1412.13
        movl      %ebx, (%edx)                                  #1412.13
        jmp       ..B20.35      # Prob 100%                     #1412.13
                                # LOE edx ecx ebx esi edi
..B20.14:                       # Preds ..B20.12
        movl      %ebx, -24(%ebp)                               #1412.13
        movl      $12, (%ebx)                                   #1412.13
        movl      %ebx, (%edi)                                  #1412.13
        movl      $14, 4(%ebx)                                  #1412.13
        pushl     %ecx                                          #1406.1
        addl      $1, %esi                                      #1412.13
        movl      $0, 8(%ebx)                                   #1412.13
        pushl     %esi                                          #1406.1
        call      nss_sob                                       #1412.13
                                # LOE eax ebx edi bl bh
..B20.89:                       # Preds ..B20.14
        movl      20(%ebp), %edx                                #
        movl      12(%ebp), %ecx                                #
        movl      24(%ebp), %esi                                #1406.1
        addl      $8, %ebx                                      #1412.13
        pushl     %esi                                          #1406.1
        pushl     %edx                                          #1406.1
        pushl     %ebx                                          #1406.1
        pushl     %ecx                                          #1406.1
        pushl     %eax                                          #1406.1
        call      nss_fasta_expr                                #1412.13
                                # LOE eax edi
..B20.90:                       # Preds ..B20.89
        movl      20(%ebp), %edx                                #
        movl      12(%ebp), %ecx                                #
        addl      $28, %esp                                     #1412.13
        movl      %eax, %esi                                    #1412.13
                                # LOE edx ecx esi edi
..B20.15:                       # Preds ..B20.90
        movl      (%edx), %ebx                                  #1413.16
        jmp       ..B20.35      # Prob 100%                     #1413.16
                                # LOE edx ecx ebx esi edi
..B20.17:                       # Preds ..B20.10
        cmpl      $40, %eax                                     #1412.13
        jne       ..B20.28      # Prob 50%                      #1412.13
                                # LOE edx ecx esi edi
..B20.18:                       # Preds ..B20.17
        movl      -8(%ebp), %ebx                                #1412.13
        pushl     $12                                           #1412.13
        call      malloc@PLT                                    #1412.13
                                # LOE eax esi edi
..B20.91:                       # Preds ..B20.18
        movl      20(%ebp), %edx                                #
        movl      12(%ebp), %ecx                                #
        movl      %eax, -24(%ebp)                               #1412.13
        addl      $4, %esp                                      #1412.13
                                # LOE eax edx ecx esi edi al ah
..B20.19:                       # Preds ..B20.91
        testl     %eax, %eax                                    #1412.13
        jne       ..B20.21      # Prob 68%                      #1412.13
                                # LOE eax edx ecx esi edi al ah
..B20.20:                       # Preds ..B20.19
        movl      -8(%ebp), %ebx                                #1412.13
        call      __errno_location@PLT                          #1412.13
                                # LOE eax esi edi
..B20.92:                       # Preds ..B20.20
        movl      20(%ebp), %edx                                #
        movl      12(%ebp), %ecx                                #
        movl      (%eax), %ebx                                  #1412.13
        movl      %ebx, (%edx)                                  #1412.13
        jmp       ..B20.35      # Prob 100%                     #1412.13
                                # LOE edx ecx ebx esi edi
..B20.21:                       # Preds ..B20.19
        movl      24(%ebp), %ebx                                #1406.1
        movl      $12, (%eax)                                   #1412.13
        movl      %eax, (%edi)                                  #1412.13
        movl      $0, 4(%eax)                                   #1412.13
        pushl     %ebx                                          #1406.1
        addl      $1, %esi                                      #1412.13
        pushl     %edx                                          #1406.1
        lea       8(%eax), %eax                                 #1412.13
        pushl     %eax                                          #1406.1
        pushl     %ecx                                          #1406.1
        pushl     %esi                                          #1406.1
        call      nss_expr                                      #1412.13
                                # LOE eax edi
..B20.93:                       # Preds ..B20.21
        movl      20(%ebp), %edx                                #
        movl      12(%ebp), %ecx                                #
        addl      $20, %esp                                     #1412.13
        movl      %eax, %esi                                    #1412.13
                                # LOE edx ecx esi edi
..B20.22:                       # Preds ..B20.93
        movl      (%edx), %ebx                                  #1412.13
        testl     %ebx, %ebx                                    #1412.13
        jne       ..B20.35      # Prob 50%                      #1412.13
                                # LOE edx ecx ebx esi edi
..B20.23:                       # Preds ..B20.22
        movl      -24(%ebp), %eax                               #1412.13
        movl      8(%eax), %eax                                 #1412.13
        testl     %eax, %eax                                    #1412.13
        je        ..B20.26      # Prob 12%                      #1412.13
                                # LOE edx ecx ebx esi edi
..B20.24:                       # Preds ..B20.23
        cmpl      %ecx, %esi                                    #1412.13
        je        ..B20.26      # Prob 12%                      #1412.13
                                # LOE edx ecx ebx esi edi
..B20.25:                       # Preds ..B20.24
        movsbl    (%esi), %eax                                  #1412.13
        addl      $1, %esi                                      #1412.13
        cmpl      $41, %eax                                     #1412.13
        je        ..B20.35      # Prob 50%                      #1412.13
                                # LOE edx ecx ebx esi edi
..B20.26:                       # Preds ..B20.25 ..B20.24 ..B20.23
        movl      $22, (%edx)                                   #1412.13
        movl      $22, %ebx                                     #
        jmp       ..B20.35      # Prob 100%                     #
                                # LOE edx ecx ebx esi edi
..B20.28:                       # Preds ..B20.17
        movl      24(%ebp), %eax                                #1406.1
        pushl     %eax                                          #1406.1
        pushl     %edx                                          #1406.1
        pushl     %edi                                          #1406.1
        pushl     %ecx                                          #1406.1
        pushl     %esi                                          #1406.1
        call      nss_fasta_expr                                #1412.13
                                # LOE eax edi
..B20.94:                       # Preds ..B20.28
        movl      20(%ebp), %edx                                #
        movl      12(%ebp), %ecx                                #
        addl      $20, %esp                                     #1412.13
        movl      %eax, %esi                                    #1412.13
                                # LOE edx ecx esi edi
..B20.29:                       # Preds ..B20.94
        movl      (%edx), %ebx                                  #1412.13
        testl     %ebx, %ebx                                    #1412.13
        jne       ..B20.35      # Prob 50%                      #1412.13
                                # LOE edx ecx ebx esi edi
..B20.30:                       # Preds ..B20.29
        cmpl      %ecx, %esi                                    #1412.13
        jae       ..B20.35      # Prob 33%                      #1412.13
                                # LOE edx ecx ebx esi edi
..B20.31:                       # Preds ..B20.30
        movl      %ebx, -28(%ebp)                               #1412.13
        pushl     %ecx                                          #1406.1
        pushl     %esi                                          #1406.1
        call      nss_sob                                       #1412.13
                                # LOE eax ebx edi bl bh
..B20.95:                       # Preds ..B20.31
        movl      20(%ebp), %edx                                #
        movl      12(%ebp), %ecx                                #
        addl      $8, %esp                                      #1412.13
        movl      %eax, %esi                                    #1412.13
        cmpl      %ecx, %esi                                    #1412.13
        jae       ..B20.35      # Prob 12%                      #1412.13
                                # LOE edx ecx ebx esi edi bl bh
..B20.32:                       # Preds ..B20.95
        movsbl    (%esi), %eax                                  #1412.13
        cmpl      $36, %eax                                     #1412.13
        je        ..B20.59      # Prob 5%                       #1412.13
                                # LOE edx ecx ebx esi edi
..B20.35:                       # Preds ..B20.76 ..B20.72 ..B20.101 ..B20.88 ..B20.15
                                #       ..B20.92 ..B20.25 ..B20.26 ..B20.22 ..B20.98
                                #       ..B20.95 ..B20.61 ..B20.32 ..B20.30 ..B20.29
                                #      
        testl     %ebx, %ebx                                    #1413.26
        jne       ..B20.58      # Prob 58%                      #1413.26
                                # LOE edx ecx esi edi
..B20.36:                       # Preds ..B20.35
        cmpl      %ecx, %esi                                    #1415.17
        jae       ..B20.42      # Prob 12%                      #1415.17
                                # LOE edx ecx esi edi
..B20.37:                       # Preds ..B20.36
        movl      -4(%ebp), %eax                                #1415.17
        movl      (%eax), %eax                                  #1415.17
        movzbl    (%esi), %ebx                                  #1415.17
        movzwl    (%eax,%ebx,2), %ebx                           #1415.17
        testl     $8192, %ebx                                   #1415.17
        je        ..B20.42      # Prob 15%                      #1415.17
                                # LOE eax edx ecx esi edi
..B20.39:                       # Preds ..B20.37 ..B20.40
        addl      $1, %esi                                      #1415.17
        cmpl      %ecx, %esi                                    #1415.17
        jae       ..B20.42      # Prob 1%                       #1415.17
                                # LOE eax edx ecx esi edi
..B20.40:                       # Preds ..B20.39
        movzbl    (%esi), %ebx                                  #1415.17
        movzwl    (%eax,%ebx,2), %ebx                           #1415.17
        testl     $8192, %ebx                                   #1415.17
        jne       ..B20.39      # Prob 82%                      #1415.17
                                # LOE eax edx ecx esi edi
..B20.42:                       # Preds ..B20.39 ..B20.40 ..B20.37 ..B20.36
        cmpl      %ecx, %esi                                    #1416.23
        je        ..B20.58      # Prob 4%                       #1416.23
                                # LOE edx ecx esi edi
..B20.43:                       # Preds ..B20.42
        movsbl    (%esi), %eax                                  #1422.28
        addl      $1, %esi                                      #1422.28
        cmpl      $41, %eax                                     #1422.28
        je        ..B20.56      # Prob 25%                      #1422.28
                                # LOE eax edx ecx esi edi
..B20.44:                       # Preds ..B20.43
        cmpl      $38, %eax                                     #1422.28
        jne       ..B20.48      # Prob 67%                      #1422.28
                                # LOE eax edx ecx esi edi
..B20.45:                       # Preds ..B20.44
        cmpl      %ecx, %esi                                    #1427.30
        jae       ..B20.47      # Prob 12%                      #1427.30
                                # LOE edx ecx esi edi
..B20.46:                       # Preds ..B20.45
        movsbl    (%esi), %eax                                  #1427.39
        lea       1(%esi), %ebx                                 #1428.28
        cmpl      $38, %eax                                     #1428.28
        cmove     %ebx, %esi                                    #1428.28
                                # LOE edx ecx esi edi
..B20.47:                       # Preds ..B20.46 ..B20.45
        movl      $16, %eax                                     #1429.21
        movl      %eax, -24(%ebp)                               #1429.21
        jmp       ..B20.52      # Prob 100%                     #1429.21
                                # LOE edx ecx esi edi
..B20.48:                       # Preds ..B20.44
        cmpl      $124, %eax                                    #1422.28
        jne       ..B20.55      # Prob 50%                      #1422.28
                                # LOE edx ecx esi edi
..B20.49:                       # Preds ..B20.48
        cmpl      %ecx, %esi                                    #1432.30
        jae       ..B20.51      # Prob 12%                      #1432.30
                                # LOE edx ecx esi edi
..B20.50:                       # Preds ..B20.49
        movsbl    (%esi), %eax                                  #1432.39
        lea       1(%esi), %ebx                                 #1433.28
        cmpl      $124, %eax                                    #1433.28
        cmove     %ebx, %esi                                    #1433.28
                                # LOE edx ecx esi edi
..B20.51:                       # Preds ..B20.50 ..B20.49
        movl      $17, %eax                                     #1434.21
        movl      %eax, -24(%ebp)                               #1434.21
                                # LOE edx ecx esi edi
..B20.52:                       # Preds ..B20.47 ..B20.51
        movl      -8(%ebp), %ebx                                #1451.21
        pushl     $16                                           #1451.30
        call      malloc@PLT                                    #1451.21
                                # LOE eax esi edi
..B20.96:                       # Preds ..B20.52
        movl      20(%ebp), %edx                                #
        movl      12(%ebp), %ecx                                #
        addl      $4, %esp                                      #1451.21
                                # LOE eax edx ecx esi edi
..B20.53:                       # Preds ..B20.96
        testl     %eax, %eax                                    #1452.27
        je        ..B20.82      # Prob 1%                       #1452.27
        jmp       ..B20.54      # Prob 100%                     #1452.27
                                # LOE eax edx ecx esi edi
..B20.55:                       # Preds ..B20.48
        movl      20(%ebp), %edx                                #1438.21
        movl      -12(%ebp), %ebx                               #1439.32
        movl      -16(%ebp), %edi                               #1439.32
        addl      $-1, %esi                                     #1439.32
        movl      %esi, %eax                                    #1439.32
        movl      -20(%ebp), %esi                               #1439.32
        movl      $22, (%edx)                                   #1438.21
        movl      %ebp, %esp                                    #1439.32
        popl      %ebp                                          #1439.32
        ret                                                     #1439.32
                                # LOE
..B20.56:                       # Preds ..B20.43
        movl      -12(%ebp), %ebx                               #1425.32
        movl      -16(%ebp), %edi                               #1425.32
        addl      $-1, %esi                                     #1425.32
        movl      %esi, %eax                                    #1425.32
        movl      -20(%ebp), %esi                               #1425.32
        movl      %ebp, %esp                                    #1425.32
        popl      %ebp                                          #1425.32
        ret                                                     #1425.32
                                # LOE
..B20.58:                       # Preds ..B20.8 ..B20.42 ..B20.35
        movl      -12(%ebp), %ebx                               #1470.12
        movl      -16(%ebp), %edi                               #1470.12
        movl      %esi, %eax                                    #1470.12
        movl      -20(%ebp), %esi                               #1470.12
        movl      %ebp, %esp                                    #1470.12
        popl      %ebp                                          #1470.12
        ret                                                     #1470.12
                                # LOE
..B20.59:                       # Preds ..B20.32                # Infreq
        movl      -8(%ebp), %ebx                                #1412.13
        pushl     $12                                           #1412.13
        addl      $1, %esi                                      #1412.13
        call      malloc@PLT                                    #1412.13
                                # LOE eax esi edi
..B20.97:                       # Preds ..B20.59                # Infreq
        movl      20(%ebp), %edx                                #
        movl      12(%ebp), %ecx                                #
        addl      $4, %esp                                      #1412.13
                                # LOE eax edx ecx esi edi
..B20.60:                       # Preds ..B20.97                # Infreq
        testl     %eax, %eax                                    #1412.13
        je        ..B20.62      # Prob 12%                      #1412.13
                                # LOE eax edx ecx esi edi
..B20.61:                       # Preds ..B20.60                # Infreq
        movl      (%edi), %ebx                                  #1412.13
        movl      $12, (%eax)                                   #1412.13
        movl      %ebx, 8(%eax)                                 #1412.13
        movl      %eax, (%edi)                                  #1412.13
        movl      (%edx), %ebx                                  #1413.16
        movl      $15, 4(%eax)                                  #1412.13
        jmp       ..B20.35      # Prob 100%                     #1412.13
                                # LOE edx ecx ebx esi edi
..B20.62:                       # Preds ..B20.60                # Infreq
        movl      -8(%ebp), %ebx                                #1412.13
        call      __errno_location@PLT                          #1412.13
                                # LOE eax esi edi
..B20.98:                       # Preds ..B20.62                # Infreq
        movl      20(%ebp), %edx                                #
        movl      12(%ebp), %ecx                                #
        movl      (%eax), %ebx                                  #1412.13
        movl      %ebx, (%edx)                                  #1412.13
        jmp       ..B20.35      # Prob 100%                     #1412.13
                                # LOE edx ecx ebx esi edi
..B20.65:                       # Preds ..B20.9                 # Infreq
        addl      $1, %esi                                      #1412.13
        cmpl      %ecx, %esi                                    #1412.13
        jae       ..B20.71      # Prob 12%                      #1412.13
                                # LOE edx ecx esi edi
..B20.66:                       # Preds ..B20.65                # Infreq
        movl      -4(%ebp), %eax                                #1412.13
        movl      (%eax), %eax                                  #1412.13
        movzbl    (%esi), %ebx                                  #1412.13
        movzwl    (%eax,%ebx,2), %ebx                           #1412.13
        testl     $8192, %ebx                                   #1412.13
        je        ..B20.71      # Prob 15%                      #1412.13
                                # LOE eax edx ecx esi edi
..B20.68:                       # Preds ..B20.66 ..B20.69       # Infreq
        addl      $1, %esi                                      #1412.13
        cmpl      %ecx, %esi                                    #1412.13
        jae       ..B20.71      # Prob 1%                       #1412.13
                                # LOE eax edx ecx esi edi
..B20.69:                       # Preds ..B20.68                # Infreq
        movzbl    (%esi), %ebx                                  #1412.13
        movzwl    (%eax,%ebx,2), %ebx                           #1412.13
        testl     $8192, %ebx                                   #1412.13
        jne       ..B20.68      # Prob 82%                      #1412.13
                                # LOE eax edx ecx esi edi
..B20.71:                       # Preds ..B20.68 ..B20.69 ..B20.66 ..B20.65 # Infreq
        cmpl      %ecx, %esi                                    #1412.13
        jne       ..B20.73      # Prob 68%                      #1412.13
                                # LOE edx ecx esi edi
..B20.72:                       # Preds ..B20.71                # Infreq
        movl      $22, (%edx)                                   #1412.13
        movl      $22, %ebx                                     #
        jmp       ..B20.35      # Prob 100%                     #
                                # LOE edx ecx ebx esi edi
..B20.73:                       # Preds ..B20.71                # Infreq
        movl      -8(%ebp), %ebx                                #1412.13
        pushl     $12                                           #1412.13
        call      malloc@PLT                                    #1412.13
                                # LOE eax esi edi
..B20.99:                       # Preds ..B20.73                # Infreq
        movl      20(%ebp), %edx                                #
        movl      12(%ebp), %ecx                                #
        addl      $4, %esp                                      #1412.13
                                # LOE eax edx ecx esi edi
..B20.74:                       # Preds ..B20.99                # Infreq
        testl     %eax, %eax                                    #1412.13
        je        ..B20.78      # Prob 1%                       #1412.13
                                # LOE eax edx ecx esi edi
..B20.75:                       # Preds ..B20.74                # Infreq
        movl      24(%ebp), %ebx                                #1406.1
        movl      $12, (%eax)                                   #1412.13
        movl      %eax, (%edi)                                  #1412.13
        movl      $13, 4(%eax)                                  #1412.13
        pushl     %ebx                                          #1406.1
        movl      $0, 8(%eax)                                   #1412.13
        pushl     %edx                                          #1406.1
        addl      $8, %eax                                      #1412.13
        pushl     %eax                                          #1406.1
        pushl     %ecx                                          #1406.1
        pushl     %esi                                          #1406.1
        call      nss_unary_expr                                #1412.13
                                # LOE eax edi
..B20.100:                      # Preds ..B20.75                # Infreq
        movl      20(%ebp), %edx                                #
        movl      12(%ebp), %ecx                                #
        addl      $20, %esp                                     #1412.13
        movl      %eax, %esi                                    #1412.13
                                # LOE edx ecx esi edi
..B20.76:                       # Preds ..B20.100               # Infreq
        movl      (%edx), %ebx                                  #1413.16
        jmp       ..B20.35      # Prob 100%                     #1413.16
                                # LOE edx ecx ebx esi edi
..B20.78:                       # Preds ..B20.74                # Infreq
        movl      -8(%ebp), %ebx                                #1412.13
        call      __errno_location@PLT                          #1412.13
                                # LOE eax esi edi
..B20.101:                      # Preds ..B20.78                # Infreq
        movl      20(%ebp), %edx                                #
        movl      12(%ebp), %ecx                                #
        movl      (%eax), %ebx                                  #1412.13
        movl      %ebx, (%edx)                                  #1412.13
        jmp       ..B20.35      # Prob 100%                     #1412.13
                                # LOE edx ecx ebx esi edi
..B20.82:                       # Preds ..B20.53                # Infreq
        movl      -8(%ebp), %ebx                                #
        call      __errno_location@PLT                          #1454.32
                                # LOE eax esi
..B20.102:                      # Preds ..B20.82                # Infreq
        movl      20(%ebp), %ecx                                #1454.21
        movl      -12(%ebp), %ebx                               #1455.28
        movl      -16(%ebp), %edi                               #1455.28
        movl      (%eax), %edx                                  #1454.32
        movl      %edx, (%ecx)                                  #1454.21
        movl      %esi, %eax                                    #1451.21
        movl      -20(%ebp), %esi                               #1455.28
        movl      %ebp, %esp                                    #1455.28
        popl      %ebp                                          #1455.28
        ret                                                     #1455.28
        .align    2,0x90
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
..B21.1:                        # Preds ..B21.0
        pushl     %ebp                                          #1484.1
        movl      %esp, %ebp                                    #1484.1
        subl      $32, %esp                                     #1484.1
        movl      %edi, -24(%ebp)                               #1484.1
        movl      %esi, -28(%ebp)                               #1484.1
        movl      %ebx, -32(%ebp)                               #1484.1
        call      ..L21         # Prob 100%                     #1484.1
..L21:                                                          #
        popl      %ebx                                          #1484.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L21], %ebx      #1484.1
        lea       _gprof_pack20@GOTOFF(%ebx), %edx              #1484.1
        call      mcount@PLT                                    #1484.1
                                # LOE ebx
..B21.29:                       # Preds ..B21.1
        lea       fasta_2na_map.0@GOTOFF(%ebx), %edi            #1491.14
        movl      $-1, %eax                                     #1491.5
        movd      %eax, %xmm0                                   #1491.5
        pshufd    $0, %xmm0, %xmm1                              #1491.5
        movq      %xmm1, (%edi)                                 #1491.5
        movq      %xmm1, 8(%edi)                                #1491.5
        movq      %xmm1, 16(%edi)                               #1491.5
        movq      %xmm1, 24(%edi)                               #1491.5
        movq      %xmm1, 32(%edi)                               #1491.5
        movq      %xmm1, 40(%edi)                               #1491.5
        movq      %xmm1, 48(%edi)                               #1491.5
        movq      %xmm1, 56(%edi)                               #1491.5
        movq      %xmm1, 64(%edi)                               #1491.5
        movq      %xmm1, 72(%edi)                               #1491.5
        movq      %xmm1, 80(%edi)                               #1491.5
        movq      %xmm1, 88(%edi)                               #1491.5
        movq      %xmm1, 96(%edi)                               #1491.5
        movq      %xmm1, 104(%edi)                              #1491.5
        movq      %xmm1, 112(%edi)                              #1491.5
        movq      %xmm1, 120(%edi)                              #1491.5
                                # LOE ebx edi
..B21.2:                        # Preds ..B21.29
        lea       fasta_4na_map.0@GOTOFF(%ebx), %eax            #1492.14
        movl      $-1, %edx                                     #1492.5
        movd      %edx, %xmm0                                   #1492.5
        pshufd    $0, %xmm0, %xmm1                              #1492.5
        movq      %xmm1, (%eax)                                 #1492.5
        movq      %xmm1, 8(%eax)                                #1492.5
        movq      %xmm1, 16(%eax)                               #1492.5
        movq      %xmm1, 24(%eax)                               #1492.5
        movq      %xmm1, 32(%eax)                               #1492.5
        movq      %xmm1, 40(%eax)                               #1492.5
        movq      %xmm1, 48(%eax)                               #1492.5
        movq      %xmm1, 56(%eax)                               #1492.5
        movq      %xmm1, 64(%eax)                               #1492.5
        movq      %xmm1, 72(%eax)                               #1492.5
        movq      %xmm1, 80(%eax)                               #1492.5
        movq      %xmm1, 88(%eax)                               #1492.5
        movq      %xmm1, 96(%eax)                               #1492.5
        movq      %xmm1, 104(%eax)                              #1492.5
        movq      %xmm1, 112(%eax)                              #1492.5
        movq      %xmm1, 120(%eax)                              #1492.5
                                # LOE eax ebx edi
..B21.3:                        # Preds ..B21.2
        lea       _2__STRING.0.0@GOTOFF(%ebx), %ecx             #1487.29
        movsbl    (%ecx), %esi                                  #1497.14
        movl      %eax, -8(%ebp)                                #
        movl      %edi, -4(%ebp)                                #
        movl      %ebx, -20(%ebp)                               #
        xorl      %edx, %edx                                    #
                                # LOE edx ecx esi
..B21.10:                       # Preds ..B21.3 ..B21.11
        movl      -20(%ebp), %ebx                               #1498.48
        movl      %edx, -16(%ebp)                               #1498.48
        pushl     %esi                                          #1497.9
        movl      %ecx, -12(%ebp)                               #1498.48
        call      tolower@PLT                                   #1498.48
                                # LOE eax esi
..B21.30:                       # Preds ..B21.10
        movl      -12(%ebp), %ecx                               #
        movl      -16(%ebp), %edx                               #
        addl      $4, %esp                                      #1498.48
                                # LOE eax edx ecx esi dl cl dh ch
..B21.11:                       # Preds ..B21.30
        movl      -4(%ebp), %ebx                                #1498.32
        movb      %dl, (%eax,%ebx)                              #1498.32
        movb      %dl, (%ebx,%esi)                              #1498.9
        movsbl    1(%ecx), %esi                                 #1495.31
        addl      $1, %edx                                      #1495.48
        addl      $1, %ecx                                      #1495.54
        testl     %esi, %esi                                    #1495.42
        jne       ..B21.10      # Prob 82%                      #1495.42
                                # LOE edx ecx esi
..B21.12:                       # Preds ..B21.11
        movl      -8(%ebp), %eax                                #
        movl      -20(%ebp), %ebx                               #
        lea       _2__STRING.1.0@GOTOFF(%ebx), %ecx             #1488.25
        movsbl    (%ecx), %esi                                  #1504.14
        xorl      %edx, %edx                                    #
        movl      %eax, -8(%ebp)                                #
        movl      %ebx, -20(%ebp)                               #
                                # LOE edx ecx esi
..B21.19:                       # Preds ..B21.12 ..B21.20
        movl      -20(%ebp), %ebx                               #1505.48
        movl      %edx, -12(%ebp)                               #1505.48
        pushl     %esi                                          #1504.9
        movl      %ecx, -16(%ebp)                               #1505.48
        call      tolower@PLT                                   #1505.48
                                # LOE eax esi
..B21.31:                       # Preds ..B21.19
        movl      -16(%ebp), %ecx                               #
        movl      -12(%ebp), %edx                               #
        addl      $4, %esp                                      #1505.48
                                # LOE eax edx ecx esi dl cl dh ch
..B21.20:                       # Preds ..B21.31
        movl      -8(%ebp), %ebx                                #1505.32
        movb      %dl, (%eax,%ebx)                              #1505.32
        movb      %dl, (%ebx,%esi)                              #1505.9
        movsbl    1(%ecx), %esi                                 #1502.31
        addl      $1, %edx                                      #1502.48
        addl      $1, %ecx                                      #1502.54
        testl     %esi, %esi                                    #1502.42
        jne       ..B21.19      # Prob 82%                      #1502.42
                                # LOE edx ecx esi
..B21.21:                       # Preds ..B21.20
        movl      -20(%ebp), %ebx                               #
        lea       512(%ebx), %eax                               #1510.11
                                # LOE eax ebx
..B21.22:                       # Preds ..B21.25 ..B21.21
        movzwl    expand_2na.0@GOTOFF(%ebx), %edx               #1511.28
                                # LOE eax edx ebx
..B21.24:                       # Preds ..B21.22
# Begin ASM
        rorw $8, %dx
# End ASM                                                       #1511.0
                                # LOE eax edx ebx
..B21.25:                       # Preds ..B21.24
        movw      %dx, expand_2na.0@GOTOFF(%ebx)                #1511.9
        addl      $2, %ebx                                      #1510.30
        cmpl      %ebx, %eax                                    #1510.22
        ja        ..B21.22      # Prob 99%                      #1510.22
                                # LOE eax ebx
..B21.26:                       # Preds ..B21.25
        movl      -32(%ebp), %ebx                               #1513.1
        movl      -28(%ebp), %esi                               #1513.1
        movl      -24(%ebp), %edi                               #1513.1
        movl      %ebp, %esp                                    #1513.1
        popl      %ebp                                          #1513.1
        ret                                                     #1513.1
        .align    2,0x90
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
_gprof_pack1:
	.long	0
	.type	_gprof_pack1,@object
	.size	_gprof_pack1,4
_gprof_pack2:
	.long	0
	.type	_gprof_pack2,@object
	.size	_gprof_pack2,4
_gprof_pack3:
	.long	0
	.type	_gprof_pack3,@object
	.size	_gprof_pack3,4
_gprof_pack4:
	.long	0
	.type	_gprof_pack4,@object
	.size	_gprof_pack4,4
_gprof_pack5:
	.long	0
	.type	_gprof_pack5,@object
	.size	_gprof_pack5,4
_gprof_pack6:
	.long	0
	.type	_gprof_pack6,@object
	.size	_gprof_pack6,4
_gprof_pack7:
	.long	0
	.type	_gprof_pack7,@object
	.size	_gprof_pack7,4
_gprof_pack8:
	.long	0
	.type	_gprof_pack8,@object
	.size	_gprof_pack8,4
_gprof_pack9:
	.long	0
	.type	_gprof_pack9,@object
	.size	_gprof_pack9,4
_gprof_pack10:
	.long	0
	.type	_gprof_pack10,@object
	.size	_gprof_pack10,4
_gprof_pack11:
	.long	0
	.type	_gprof_pack11,@object
	.size	_gprof_pack11,4
_gprof_pack12:
	.long	0
	.type	_gprof_pack12,@object
	.size	_gprof_pack12,4
_gprof_pack13:
	.long	0
	.type	_gprof_pack13,@object
	.size	_gprof_pack13,4
_gprof_pack14:
	.long	0
	.type	_gprof_pack14,@object
	.size	_gprof_pack14,4
_gprof_pack15:
	.long	0
	.type	_gprof_pack15,@object
	.size	_gprof_pack15,4
_gprof_pack16:
	.long	0
	.type	_gprof_pack16,@object
	.size	_gprof_pack16,4
_gprof_pack17:
	.long	0
	.type	_gprof_pack17,@object
	.size	_gprof_pack17,4
_gprof_pack18:
	.long	0
	.type	_gprof_pack18,@object
	.size	_gprof_pack18,4
_gprof_pack19:
	.long	0
	.type	_gprof_pack19,@object
	.size	_gprof_pack19,4
_gprof_pack20:
	.long	0
	.type	_gprof_pack20,@object
	.size	_gprof_pack20,4
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
// -- Begin DWARF2 SEGMENT .debug_abbrev
	.section .debug_abbrev
.debug_abbrev_seg:
	.align 1
	.section .text
.LNDBG_TXe:
# End
