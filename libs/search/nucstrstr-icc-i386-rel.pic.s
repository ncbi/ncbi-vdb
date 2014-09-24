# -- Machine type PW
# mark_description "Intel(R) C++ Compiler for applications running on IA-32, Version 10.1    Build 20081024 %s";
# mark_description "-I/home/yaschenk/devel/internal/asm-trace/sra/search/linux -I/home/yaschenk/devel/internal/asm-trace/sra/sea";
# mark_description "rch/unix -I/home/yaschenk/devel/internal/asm-trace/sra/search -I/home/yaschenk/devel/internal/asm-trace/itf ";
# mark_description "-I/home/yaschenk/devel/internal/asm-trace/inc/icc/i386 -I/home/yaschenk/devel/internal/asm-trace/inc/icc -I/";
# mark_description "home/yaschenk/devel/internal/asm-trace/inc/gcc/i386 -I/home/yaschenk/devel/internal/asm-trace/inc/gcc -I/hom";
# mark_description "e/yaschenk/devel/internal/asm-trace/inc/linux -I/home/yaschenk/devel/internal/asm-trace/inc/unix -I/home/rod";
# mark_description "armer/devel/internal/asm-trace/inc -c -S -o nucstrstr.pic.s -fPIC -O3 -unroll -xW -DNDEBUG -DLINUX -DUNIX -D";
# mark_description "_GNU_SOURCE -D_REENTRANT -D_FILE_OFFSET_BITS=64 -MD";
	.file "nucstrstr.c"
	.text
..TXTST0:
# -- Begin  NucStrstrSearch
# mark_begin;
       .align    2,0x90
	.globl NucStrstrSearch
NucStrstrSearch:
# parameter 1: 36 + %esp
# parameter 2: 40 + %esp
# parameter 3: 44 + %esp
# parameter 4: 48 + %esp
# parameter 5: 52 + %esp
..B1.1:                         # Preds ..B1.0
        pushl     %edi                                          #5217.1
        pushl     %esi                                          #5217.1
        pushl     %ebx                                          #5217.1
        subl      $20, %esp                                     #5217.1
        movl      36(%esp), %esi                                #5215.5
        movl      40(%esp), %edx                                #5215.5
        movl      44(%esp), %ecx                                #5215.5
        movl      48(%esp), %edi                                #5215.5
                                # LOE edx ecx ebp esi edi
..B1.2:                         # Preds ..B1.1
        testl     %esi, %esi                                    #5218.18
        je        ..B1.108      # Prob 6%                       #5218.18
                                # LOE edx ecx ebp esi edi
..B1.3:                         # Preds ..B1.2
        testl     %edx, %edx                                    #5218.37
        je        ..B1.108      # Prob 6%                       #5218.37
                                # LOE edx ecx ebp esi edi
..B1.4:                         # Preds ..B1.3
        call      ..L1          # Prob 100%                     #
..L1:                                                           #
        popl      %ebx                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L1], %ebx       #
                                # LOE edx ecx ebx ebp esi edi
..B1.5:                         # Preds ..B1.15 ..B1.18 ..B1.4
        testl     %edi, %edi                                    #5218.52
        je        ..B1.108      # Prob 28%                      #5218.52
                                # LOE edx ecx ebx ebp esi edi
..B1.7:                         # Preds ..B1.5
        movl      %ebp, 16(%esp)                                #
        movl      %ecx, %ebp                                    #
                                # LOE edx ebx ebp esi edi
..B1.8:                         # Preds ..B1.32 ..B1.7
        movl      (%esi), %eax                                  #5223.18
        cmpl      $12, %eax                                     #5223.9
        ja        ..B1.33       # Prob 50%                      #5223.9
                                # LOE eax ebx ebp esi edi
..B1.9:                         # Preds ..B1.8
        movl      ..1..TPKT.30_0.0.0@GOTOFF(%ebx,%eax,4), %eax  #5223.9
        jmp       *%eax                                         #5223.9
                                # LOE ebx ebp esi edi
..1.30_0.TAG.0c.0.0:
..B1.10:                        # Preds ..B1.9
        movl      40(%esp), %edx                                #
        movl      %ebp, %ecx                                    #
        movl      16(%esp), %ebp                                #
                                # LOE edx ecx ebx ebp esi edi
..B1.11:                        # Preds ..B1.10
        movl      4(%esi), %eax                                 #5286.22
        testl     %eax, %eax                                    #5286.22
        je        ..B1.21       # Prob 20%                      #5286.22
                                # LOE eax edx ecx ebx ebp esi edi
..B1.12:                        # Preds ..B1.11
        cmpl      $13, %eax                                     #5286.22
        je        ..B1.21       # Prob 25%                      #5286.22
                                # LOE eax edx ecx ebx ebp esi edi
..B1.13:                        # Preds ..B1.12
        cmpl      $14, %eax                                     #5286.22
        jne       ..B1.16       # Prob 67%                      #5286.22
                                # LOE eax edx ecx ebx ebp esi edi
..B1.14:                        # Preds ..B1.13
        movl      8(%esi), %esi                                 #5297.29
        movl      4(%esi), %eax                                 #5297.29
        cmpl      %edi, %eax                                    #5298.34
        ja        ..B1.108      # Prob 5%                       #5298.34
                                # LOE eax edx ecx ebx ebp esi
..B1.15:                        # Preds ..B1.14
        movl      %eax, %edi                                    #5297.17
        testl     %esi, %esi                                    #5218.18
        jne       ..B1.5        # Prob 94%                      #5218.18
        jmp       ..B1.108      # Prob 100%                     #5218.18
                                # LOE edx ecx ebx ebp esi edi
..B1.16:                        # Preds ..B1.13
        cmpl      $15, %eax                                     #5286.22
        jne       ..B1.108      # Prob 50%                      #5286.22
                                # LOE edx ecx ebx ebp esi edi
..B1.17:                        # Preds ..B1.16
        movl      8(%esi), %esi                                 #5302.29
        movl      4(%esi), %eax                                 #5302.29
        cmpl      %edi, %eax                                    #5303.34
        ja        ..B1.108      # Prob 5%                       #5303.34
                                # LOE eax edx ecx ebx ebp esi edi
..B1.18:                        # Preds ..B1.17
        addl      %edi, %ecx                                    #5306.27
        subl      %eax, %ecx                                    #5306.33
        movl      %eax, %edi                                    #5302.17
        testl     %esi, %esi                                    #5218.18
        jne       ..B1.5        # Prob 94%                      #5218.18
        jmp       ..B1.108      # Prob 100%                     #5218.18
                                # LOE edx ecx ebx ebp esi edi
..B1.21:                        # Preds ..B1.11 ..B1.12
        movl      52(%esp), %eax                                #5217.1
        pushl     %eax                                          #5217.1
        pushl     %edi                                          #5217.1
        pushl     %ecx                                          #5217.1
        pushl     %edx                                          #5217.1
        pushl     8(%esi)                                       #5217.1
        call      NucStrstrSearch@PLT                           #5290.25
                                # LOE eax ebp esi
..B1.118:                       # Preds ..B1.21
        addl      $20, %esp                                     #5290.25
                                # LOE eax ebp esi
..B1.22:                        # Preds ..B1.118
        movl      4(%esi), %edx                                 #5291.22
        testl     %edx, %edx                                    #5291.42
        je        ..B1.57       # Prob 28%                      #5291.42
                                # LOE eax ebp
..B1.23:                        # Preds ..B1.22
        testl     %eax, %eax                                    #5293.31
        jne       ..B1.108      # Prob 72%                      #5293.31
                                # LOE ebp
..B1.24:                        # Preds ..B1.23
        movl      $1, %eax                                      #5294.28
        addl      $20, %esp                                     #5294.28
        popl      %ebx                                          #5294.28
        popl      %esi                                          #5294.28
        popl      %edi                                          #5294.28
        ret                                                     #5294.28
                                # LOE
..1.30_0.TAG.0b.0.0:
..B1.27:                        # Preds ..B1.9
        movl      52(%esp), %eax                                #5217.1
        pushl     %eax                                          #5217.1
        pushl     %edi                                          #5217.1
        pushl     %ebp                                          #5217.1
        movl      52(%esp), %edx                                #5217.1
        pushl     %edx                                          #5217.1
        pushl     8(%esi)                                       #5217.1
        call      NucStrstrSearch@PLT                           #5272.21
                                # LOE eax ebx ebp esi edi
..B1.119:                       # Preds ..B1.27
        addl      $20, %esp                                     #5272.21
                                # LOE eax ebx ebp esi edi
..B1.28:                        # Preds ..B1.119
        movl      4(%esi), %edx                                 #5273.22
        cmpl      $17, %edx                                     #5273.22
        jne       ..B1.30       # Prob 67%                      #5273.22
                                # LOE eax edx ebx ebp esi edi
..B1.29:                        # Preds ..B1.28
        testl     %eax, %eax                                    #5276.31
        jne       ..B1.112      # Prob 5%                       #5276.31
        jmp       ..B1.32       # Prob 100%                     #5276.31
                                # LOE eax ebx ebp esi edi
..B1.30:                        # Preds ..B1.28
        cmpl      $16, %edx                                     #5273.22
        jne       ..B1.32       # Prob 50%                      #5273.22
                                # LOE eax ebx ebp esi edi
..B1.31:                        # Preds ..B1.30
        testl     %eax, %eax                                    #5280.31
        je        ..B1.112      # Prob 5%                       #5280.31
                                # LOE eax ebx ebp esi edi
..B1.32:                        # Preds ..B1.29 ..B1.31 ..B1.30
        movl      12(%esi), %esi                                #5284.38
        testl     %esi, %esi                                    #5218.18
        jne       ..B1.8        # Prob 94%                      #5218.18
                                # LOE ebx ebp esi edi
..B1.33:                        # Preds ..B1.8 ..B1.32
        movl      16(%esp), %ebp                                #
        jmp       ..B1.108      # Prob 100%                     #
                                # LOE ebp
..1.30_0.TAG.0a.0.0:
..B1.34:                        # Preds ..B1.9
        movl      40(%esp), %edx                                #
        movl      %ebp, %ecx                                    #
        movl      16(%esp), %ebp                                #
                                # LOE edx ecx ebp esi edi
..B1.35:                        # Preds ..B1.34
        movl      4(%esi), %eax                                 #5268.24
        cmpl      %eax, %edi                                    #5268.24
        jb        ..B1.108      # Prob 28%                      #5268.24
                                # LOE eax edx ecx ebp esi edi
..B1.36:                        # Preds ..B1.35
        movl      52(%esp), %ebx                                #5269.9
        testl     %ebx, %ebx                                    #5269.9
        je        ..B1.38       # Prob 12%                      #5269.9
                                # LOE eax edx ecx ebp esi edi
..B1.37:                        # Preds ..B1.36
        movl      52(%esp), %ebx                                #5269.18
        movl      %eax, (%ebx)                                  #5269.18
                                # LOE edx ecx ebp esi edi
..B1.38:                        # Preds ..B1.37 ..B1.36
        movl      %edi, 12(%esp)                                #5218.5
        movl      %esi, %eax                                    #5270.20
        call      eval_4na_pos                                  #5270.20
                                # LOE eax ebp
..B1.120:                       # Preds ..B1.38
        addl      $20, %esp                                     #5270.20
        popl      %ebx                                          #5270.20
        popl      %esi                                          #5270.20
        popl      %edi                                          #5270.20
        ret                                                     #5270.20
                                # LOE
..1.30_0.TAG.09.0.0:
..B1.40:                        # Preds ..B1.9
        movl      40(%esp), %edx                                #
        movl      %ebp, %ecx                                    #
        movl      16(%esp), %ebp                                #
                                # LOE edx ecx ebp esi edi
..B1.41:                        # Preds ..B1.40
        movl      4(%esi), %eax                                 #5264.24
        cmpl      %eax, %edi                                    #5264.24
        jb        ..B1.108      # Prob 28%                      #5264.24
                                # LOE eax edx ecx ebp esi edi
..B1.42:                        # Preds ..B1.41
        movl      52(%esp), %ebx                                #5265.9
        testl     %ebx, %ebx                                    #5265.9
        je        ..B1.44       # Prob 12%                      #5265.9
                                # LOE eax edx ecx ebp esi edi
..B1.43:                        # Preds ..B1.42
        movl      52(%esp), %ebx                                #5265.18
        movl      %eax, (%ebx)                                  #5265.18
                                # LOE edx ecx ebp esi edi
..B1.44:                        # Preds ..B1.43 ..B1.42
        movl      %edi, 12(%esp)                                #5218.5
        movl      %esi, %eax                                    #5266.20
        call      eval_2na_pos                                  #5266.20
                                # LOE eax ebp
..B1.121:                       # Preds ..B1.44
        addl      $20, %esp                                     #5266.20
        popl      %ebx                                          #5266.20
        popl      %esi                                          #5266.20
        popl      %edi                                          #5266.20
        ret                                                     #5266.20
                                # LOE
..1.30_0.TAG.08.0.0:
..B1.46:                        # Preds ..B1.9
        movl      40(%esp), %edx                                #
        movl      %ebp, %ecx                                    #
        movl      16(%esp), %ebp                                #
                                # LOE edx ecx ebp esi edi
..B1.47:                        # Preds ..B1.46
        movl      4(%esi), %eax                                 #5259.24
        cmpl      %eax, %edi                                    #5259.24
        jb        ..B1.108      # Prob 28%                      #5259.24
                                # LOE eax edx ecx ebp esi edi
..B1.48:                        # Preds ..B1.47
        movl      52(%esp), %ebx                                #5260.9
        testl     %ebx, %ebx                                    #5260.9
        je        ..B1.50       # Prob 12%                      #5260.9
                                # LOE eax edx ecx ebp esi edi
..B1.49:                        # Preds ..B1.48
        movl      52(%esp), %ebx                                #5260.18
        movl      %eax, (%ebx)                                  #5260.18
                                # LOE edx ecx ebp esi edi
..B1.50:                        # Preds ..B1.49 ..B1.48
        movl      %edi, 12(%esp)                                #5218.5
        movl      %esi, %eax                                    #5261.20
        call      eval_4na_128                                  #5261.20
                                # LOE eax ebp
..B1.122:                       # Preds ..B1.50
        addl      $20, %esp                                     #5261.20
        popl      %ebx                                          #5261.20
        popl      %esi                                          #5261.20
        popl      %edi                                          #5261.20
        ret                                                     #5261.20
                                # LOE
..1.30_0.TAG.07.0.0:
..B1.52:                        # Preds ..B1.9
        movl      40(%esp), %edx                                #
        movl      %ebp, %ecx                                    #
        movl      16(%esp), %ebp                                #
                                # LOE edx ecx ebp esi edi
..B1.53:                        # Preds ..B1.52
        movl      4(%esi), %eax                                 #5255.24
        cmpl      %eax, %edi                                    #5255.24
        jb        ..B1.108      # Prob 28%                      #5255.24
                                # LOE eax edx ecx ebp esi edi
..B1.54:                        # Preds ..B1.53
        movl      52(%esp), %ebx                                #5256.9
        testl     %ebx, %ebx                                    #5256.9
        je        ..B1.56       # Prob 12%                      #5256.9
                                # LOE eax edx ecx ebp esi edi
..B1.55:                        # Preds ..B1.54
        movl      52(%esp), %ebx                                #5256.18
        movl      %eax, (%ebx)                                  #5256.18
                                # LOE edx ecx ebp esi edi
..B1.56:                        # Preds ..B1.55 ..B1.54
        movl      %edi, 12(%esp)                                #5218.5
        movl      %esi, %eax                                    #5257.20
        call      eval_4na_32                                   #5257.20
                                # LOE eax ebp
..B1.57:                        # Preds ..B1.22 ..B1.56
        addl      $20, %esp                                     #5257.20
        popl      %ebx                                          #5257.20
        popl      %esi                                          #5257.20
        popl      %edi                                          #5257.20
        ret                                                     #5257.20
                                # LOE
..1.30_0.TAG.06.0.0:
..B1.59:                        # Preds ..B1.9
        movl      40(%esp), %edx                                #
        movl      %ebp, %ecx                                    #
        movl      16(%esp), %ebp                                #
                                # LOE edx ecx ebp esi edi
..B1.60:                        # Preds ..B1.59
        movl      4(%esi), %eax                                 #5251.24
        cmpl      %eax, %edi                                    #5251.24
        jb        ..B1.108      # Prob 28%                      #5251.24
                                # LOE eax edx ecx ebp esi edi
..B1.61:                        # Preds ..B1.60
        movl      52(%esp), %ebx                                #5252.9
        testl     %ebx, %ebx                                    #5252.9
        je        ..B1.63       # Prob 12%                      #5252.9
                                # LOE eax edx ecx ebp esi edi
..B1.62:                        # Preds ..B1.61
        movl      52(%esp), %ebx                                #5252.18
        movl      %eax, (%ebx)                                  #5252.18
                                # LOE edx ecx ebp esi edi
..B1.63:                        # Preds ..B1.62 ..B1.61
        movl      %edi, 12(%esp)                                #5218.5
        movl      %esi, %eax                                    #5253.20
        call      eval_4na_16                                   #5253.20
                                # LOE eax ebp
..B1.64:                        # Preds ..B1.63
        addl      $20, %esp                                     #5253.20
        popl      %ebx                                          #5253.20
        popl      %esi                                          #5253.20
        popl      %edi                                          #5253.20
        ret                                                     #5253.20
                                # LOE
..1.30_0.TAG.05.0.0:
..B1.66:                        # Preds ..B1.9
        movl      40(%esp), %edx                                #
        movl      %ebp, %ecx                                    #
        movl      16(%esp), %ebp                                #
                                # LOE edx ecx ebp esi edi
..B1.67:                        # Preds ..B1.66
        movl      4(%esi), %eax                                 #5247.24
        cmpl      %eax, %edi                                    #5247.24
        jb        ..B1.108      # Prob 28%                      #5247.24
                                # LOE eax edx ecx ebp esi edi
..B1.68:                        # Preds ..B1.67
        movl      52(%esp), %ebx                                #5248.9
        testl     %ebx, %ebx                                    #5248.9
        je        ..B1.70       # Prob 12%                      #5248.9
                                # LOE eax edx ecx ebp esi edi
..B1.69:                        # Preds ..B1.68
        movl      52(%esp), %ebx                                #5248.18
        movl      %eax, (%ebx)                                  #5248.18
                                # LOE edx ecx ebp esi edi
..B1.70:                        # Preds ..B1.69 ..B1.68
        movl      %edi, 12(%esp)                                #5218.5
        movl      %esi, %eax                                    #5249.20
        call      eval_2na_128                                  #5249.20
                                # LOE eax ebp
..B1.125:                       # Preds ..B1.70
        addl      $20, %esp                                     #5249.20
        popl      %ebx                                          #5249.20
        popl      %esi                                          #5249.20
        popl      %edi                                          #5249.20
        ret                                                     #5249.20
                                # LOE
..1.30_0.TAG.04.0.0:
..B1.72:                        # Preds ..B1.9
        movl      40(%esp), %edx                                #
        movl      %ebp, %ecx                                    #
        movl      16(%esp), %ebp                                #
                                # LOE edx ecx ebp esi edi
..B1.73:                        # Preds ..B1.72
        movl      4(%esi), %eax                                 #5243.24
        cmpl      %eax, %edi                                    #5243.24
        jb        ..B1.108      # Prob 28%                      #5243.24
                                # LOE eax edx ecx ebp esi edi
..B1.74:                        # Preds ..B1.73
        movl      52(%esp), %ebx                                #5244.9
        testl     %ebx, %ebx                                    #5244.9
        je        ..B1.76       # Prob 12%                      #5244.9
                                # LOE eax edx ecx ebp esi edi
..B1.75:                        # Preds ..B1.74
        movl      52(%esp), %ebx                                #5244.18
        movl      %eax, (%ebx)                                  #5244.18
                                # LOE edx ecx ebp esi edi
..B1.76:                        # Preds ..B1.75 ..B1.74
        movl      %edi, 12(%esp)                                #5218.5
        movl      %esi, %eax                                    #5245.20
        call      eval_2na_32                                   #5245.20
                                # LOE eax ebp
..B1.77:                        # Preds ..B1.76
        addl      $20, %esp                                     #5245.20
        popl      %ebx                                          #5245.20
        popl      %esi                                          #5245.20
        popl      %edi                                          #5245.20
        ret                                                     #5245.20
                                # LOE
..1.30_0.TAG.03.0.0:
..B1.79:                        # Preds ..B1.9
        movl      40(%esp), %edx                                #
        movl      %ebp, %ecx                                    #
        movl      16(%esp), %ebp                                #
                                # LOE edx ecx ebp esi edi
..B1.80:                        # Preds ..B1.79
        movl      4(%esi), %eax                                 #5239.24
        cmpl      %eax, %edi                                    #5239.24
        jb        ..B1.108      # Prob 28%                      #5239.24
                                # LOE eax edx ecx ebp esi edi
..B1.81:                        # Preds ..B1.80
        movl      52(%esp), %ebx                                #5240.9
        testl     %ebx, %ebx                                    #5240.9
        je        ..B1.83       # Prob 12%                      #5240.9
                                # LOE eax edx ecx ebp esi edi
..B1.82:                        # Preds ..B1.81
        movl      52(%esp), %ebx                                #5240.18
        movl      %eax, (%ebx)                                  #5240.18
                                # LOE edx ecx ebp esi edi
..B1.83:                        # Preds ..B1.82 ..B1.81
        movl      %edi, 12(%esp)                                #5218.5
        movl      %esi, %eax                                    #5241.20
        call      eval_2na_16                                   #5241.20
                                # LOE eax ebp
..B1.84:                        # Preds ..B1.83
        addl      $20, %esp                                     #5241.20
        popl      %ebx                                          #5241.20
        popl      %esi                                          #5241.20
        popl      %edi                                          #5241.20
        ret                                                     #5241.20
                                # LOE
..1.30_0.TAG.02.0.0:
..B1.86:                        # Preds ..B1.9
        movl      40(%esp), %edx                                #
        movl      %ebp, %ecx                                    #
        movl      16(%esp), %ebp                                #
                                # LOE edx ecx ebp esi edi
..B1.87:                        # Preds ..B1.86
        movl      4(%esi), %eax                                 #5235.24
        cmpl      %eax, %edi                                    #5235.24
        jb        ..B1.108      # Prob 28%                      #5235.24
                                # LOE eax edx ecx ebp esi edi
..B1.88:                        # Preds ..B1.87
        movl      52(%esp), %ebx                                #5236.9
        testl     %ebx, %ebx                                    #5236.9
        je        ..B1.90       # Prob 12%                      #5236.9
                                # LOE eax edx ecx ebp esi edi
..B1.89:                        # Preds ..B1.88
        movl      52(%esp), %ebx                                #5236.18
        movl      %eax, (%ebx)                                  #5236.18
                                # LOE edx ecx ebp esi edi
..B1.90:                        # Preds ..B1.89 ..B1.88
        movl      %edi, 12(%esp)                                #5218.5
        movl      %esi, %eax                                    #5237.20
        call      eval_2na_8                                    #5237.20
                                # LOE eax ebp
..B1.91:                        # Preds ..B1.90
        addl      $20, %esp                                     #5237.20
        popl      %ebx                                          #5237.20
        popl      %esi                                          #5237.20
        popl      %edi                                          #5237.20
        ret                                                     #5237.20
                                # LOE
..1.30_0.TAG.01.0.0:
..B1.93:                        # Preds ..B1.9
        movl      40(%esp), %edx                                #
        movl      %ebp, %ecx                                    #
        movl      16(%esp), %ebp                                #
                                # LOE edx ecx ebp esi edi
..B1.94:                        # Preds ..B1.93
        movl      4(%esi), %eax                                 #5230.24
        cmpl      %eax, %edi                                    #5230.24
        jb        ..B1.108      # Prob 28%                      #5230.24
                                # LOE eax edx ecx ebp esi edi
..B1.95:                        # Preds ..B1.94
        movl      52(%esp), %ebx                                #5231.9
        testl     %ebx, %ebx                                    #5231.9
        je        ..B1.97       # Prob 12%                      #5231.9
                                # LOE eax edx ecx ebp esi edi
..B1.96:                        # Preds ..B1.95
        movl      52(%esp), %ebx                                #5231.18
        movl      %eax, (%ebx)                                  #5231.18
                                # LOE edx ecx ebp esi edi
..B1.97:                        # Preds ..B1.96 ..B1.95
        movl      %edi, 12(%esp)                                #5218.5
        movl      %esi, %eax                                    #5232.20
        call      eval_4na_64                                   #5232.20
                                # LOE eax ebp
..B1.98:                        # Preds ..B1.97
        addl      $20, %esp                                     #5232.20
        popl      %ebx                                          #5232.20
        popl      %esi                                          #5232.20
        popl      %edi                                          #5232.20
        ret                                                     #5232.20
                                # LOE
..1.30_0.TAG.00.0.0:
..B1.100:                       # Preds ..B1.9
        movl      40(%esp), %edx                                #
        movl      %ebp, %ecx                                    #
        movl      16(%esp), %ebp                                #
                                # LOE edx ecx ebp esi edi
..B1.101:                       # Preds ..B1.100
        movl      4(%esi), %eax                                 #5226.24
        cmpl      %eax, %edi                                    #5226.24
        jb        ..B1.108      # Prob 28%                      #5226.24
                                # LOE eax edx ecx ebp esi edi
..B1.102:                       # Preds ..B1.101
        movl      52(%esp), %ebx                                #5227.9
        testl     %ebx, %ebx                                    #5227.9
        je        ..B1.104      # Prob 12%                      #5227.9
                                # LOE eax edx ecx ebp esi edi
..B1.103:                       # Preds ..B1.102
        movl      52(%esp), %ebx                                #5227.18
        movl      %eax, (%ebx)                                  #5227.18
                                # LOE edx ecx ebp esi edi
..B1.104:                       # Preds ..B1.103 ..B1.102
        movl      %edi, 12(%esp)                                #5218.5
        movl      %esi, %eax                                    #5228.20
        call      eval_2na_64                                   #5228.20
                                # LOE eax ebp
..B1.105:                       # Preds ..B1.104
        addl      $20, %esp                                     #5228.20
        popl      %ebx                                          #5228.20
        popl      %esi                                          #5228.20
        popl      %edi                                          #5228.20
        ret                                                     #5228.20
                                # LOE
..B1.108:                       # Preds ..B1.14 ..B1.17 ..B1.18 ..B1.15 ..B1.16
                                #       ..B1.5 ..B1.101 ..B1.94 ..B1.87 ..B1.80
                                #       ..B1.73 ..B1.67 ..B1.60 ..B1.53 ..B1.47
                                #       ..B1.41 ..B1.35 ..B1.33 ..B1.23 ..B1.3
                                #       ..B1.2
        xorl      %eax, %eax                                    #5311.12
        addl      $20, %esp                                     #5311.12
        popl      %ebx                                          #5311.12
        popl      %esi                                          #5311.12
        popl      %edi                                          #5311.12
        ret                                                     #5311.12
                                # LOE
..B1.112:                       # Preds ..B1.29 ..B1.31         # Infreq
        movl      16(%esp), %ebp                                #
        addl      $20, %esp                                     #5281.28
        popl      %ebx                                          #5281.28
        popl      %esi                                          #5281.28
        popl      %edi                                          #5281.28
        ret                                                     #5281.28
        .align    2,0x90
                                # LOE
# mark_end;
	.type	NucStrstrSearch,@function
	.size	NucStrstrSearch,.-NucStrstrSearch
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
# parameter 1: %eax
# parameter 2: %edx
# parameter 3: %ecx
# parameter 4: 20 + %ebp
..B2.1:                         # Preds ..B2.0
        pushl     %ebp                                          #3075.1
        movl      %esp, %ebp                                    #3075.1
        andl      $-16, %esp                                    #3075.1
        pushl     %edi                                          #3075.1
        pushl     %esi                                          #3075.1
        pushl     %ebx                                          #3075.1
        subl      $84, %esp                                     #3075.1
        movl      %ecx, %ebx                                    #3116.49
        shrl      $2, %ebx                                      #3116.49
        movl      20(%ebp), %edi                                #3073.5
        movl      %ebx, 80(%esp)                                #3116.49
        lea       (%ecx,%edi), %esi                             #3113.5
        lea       (%edx,%ebx), %ebx                             #3116.30
        lea       3(%ecx,%edi), %edi                            #3122.50
        subl      4(%eax), %esi                                 #3119.12
        movl      %esi, 72(%esp)                                #3119.12
        movl      80(%esp), %esi                                #3125.14
        shrl      $2, %edi                                      #3122.57
        addl      %edx, %edi                                    #3122.30
        testb     $15, %bl                                      #3125.14
        jne       ..B2.3        # Prob 50%                      #3125.14
                                # LOE eax edx ecx ebx esi edi
..B2.2:                         # Preds ..B2.1
        movdqa    (%ebx), %xmm0                                 #3125.14
        movdqa    %xmm0, (%esp)                                 #3125.14
        jmp       ..B2.4        # Prob 100%                     #3125.14
                                # LOE eax edx ecx esi edi
..B2.3:                         # Preds ..B2.1
        movdqu    (%ebx), %xmm0                                 #3125.14
        movdqa    %xmm0, (%esp)                                 #3125.14
                                # LOE eax edx ecx esi edi
..B2.4:                         # Preds ..B2.2 ..B2.3
        lea       16(%edx,%esi), %ebx                           #3126.5
        movl      %ebx, 76(%esp)                                #3126.5
        cmpl      %edi, %ebx                                    #3131.16
        jae       ..B2.6        # Prob 12%                      #3131.16
                                # LOE eax ecx ebx edi bl bh
..B2.5:                         # Preds ..B2.4
        movzbl    -1(%ebx), %edx                                #3132.24
        shll      $8, %edx                                      #3132.38
        jmp       ..B2.7        # Prob 100%                     #3132.38
                                # LOE eax edx ecx ebx edi bl bh
..B2.6:                         # Preds ..B2.4
        xorl      %edx, %edx                                    #
                                # LOE eax edx ecx ebx edi bl bh
..B2.7:                         # Preds ..B2.5 ..B2.6
        movdqa    16(%eax), %xmm7                               #3136.5
        movdqa    32(%eax), %xmm6                               #3136.5
        movdqa    48(%eax), %xmm5                               #3136.5
        movdqa    64(%eax), %xmm4                               #3136.5
        movdqa    80(%eax), %xmm3                               #3136.5
        movdqa    96(%eax), %xmm0                               #3136.5
        movdqa    112(%eax), %xmm2                              #3136.5
        movdqa    128(%eax), %xmm1                              #3136.5
        movl      %ecx, %eax                                    #3148.20
        andl      $3, %eax                                      #3148.20
        je        ..B2.43       # Prob 20%                      #3148.20
                                # LOE eax edx ecx ebx edi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B2.8:                         # Preds ..B2.7
        cmpl      $1, %eax                                      #3148.20
        je        ..B2.14       # Prob 25%                      #3148.20
                                # LOE eax edx ecx ebx edi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B2.9:                         # Preds ..B2.8
        cmpl      $2, %eax                                      #3148.20
        je        ..B2.13       # Prob 33%                      #3148.20
                                # LOE eax edx ecx ebx edi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B2.10:                        # Preds ..B2.9
        cmpl      $3, %eax                                      #3148.20
        je        ..B2.12       # Prob 50%                      #3148.20
                                # LOE edx ecx ebx edi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B2.11:                        # Preds ..B2.10
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    (%esp), %xmm0                                 #
        movdqa    %xmm7, 48(%esp)                               #
        movl      %edi, 64(%esp)                                #
        jmp       ..B2.35       # Prob 100%                     #
                                # LOE edx ecx ebx bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.12:                        # Preds ..B2.10
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    (%esp), %xmm0                                 #
        movdqa    %xmm7, 48(%esp)                               #
        movl      %edi, 64(%esp)                                #
        xorl      %eax, %eax                                    #
        movl      %eax, 24(%esp)                                #
        xorl      %esi, %esi                                    #
        movl      %esi, 28(%esp)                                #
        movl      %eax, 80(%esp)                                #
        movl      $8, %esi                                      #
        jmp       ..B2.17       # Prob 100%                     #
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.13:                        # Preds ..B2.9
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    (%esp), %xmm0                                 #
        movdqa    %xmm7, 48(%esp)                               #
        movl      %edi, 64(%esp)                                #
        xorl      %eax, %eax                                    #
        movl      %eax, 28(%esp)                                #
        movl      %eax, 80(%esp)                                #
        movl      $8, %esi                                      #
        jmp       ..B2.16       # Prob 100%                     #
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.14:                        # Preds ..B2.8
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    (%esp), %xmm0                                 #
        movdqa    %xmm7, 48(%esp)                               #
        movl      %edi, 64(%esp)                                #
        xorl      %eax, %eax                                    #
        movl      %eax, 80(%esp)                                #
        movl      $8, %esi                                      #
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.15:                        # Preds ..B2.14 ..B2.39
        movdqa    %xmm0, %xmm7                                  #3171.22
        pand      %xmm4, %xmm7                                  #3171.22
        pcmpeqd   %xmm5, %xmm7                                  #3172.22
        pmovmskb  %xmm7, %eax                                   #3173.22
        movl      %eax, %edi                                    #3174.17
        andl      $3855, %edi                                   #3174.17
        shll      $4, %edi                                      #3174.17
        andl      %edi, %eax                                    #3174.17
        movl      %eax, %edi                                    #3174.17
        sarl      $4, %edi                                      #3174.17
        orl       %edi, %eax                                    #3174.17
        movl      %eax, 28(%esp)                                #3174.17
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.16:                        # Preds ..B2.13 ..B2.15
        movdqa    %xmm0, %xmm7                                  #3177.22
        pand      32(%esp), %xmm7                               #3177.22
        pcmpeqd   %xmm3, %xmm7                                  #3178.22
        pmovmskb  %xmm7, %eax                                   #3179.22
        movl      %eax, %edi                                    #3180.17
        andl      $3855, %edi                                   #3180.17
        shll      $4, %edi                                      #3180.17
        andl      %edi, %eax                                    #3180.17
        movl      %eax, %edi                                    #3180.17
        sarl      $4, %edi                                      #3180.17
        orl       %edi, %eax                                    #3180.17
        movl      %eax, 24(%esp)                                #3180.17
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.17:                        # Preds ..B2.12 ..B2.16
        movdqa    %xmm0, %xmm7                                  #3183.22
        pand      %xmm1, %xmm7                                  #3183.22
        pcmpeqd   %xmm2, %xmm7                                  #3184.22
        pmovmskb  %xmm7, %eax                                   #3185.22
        movl      %eax, %edi                                    #3186.17
        andl      $3855, %edi                                   #3186.17
        shll      $4, %edi                                      #3186.17
        andl      %edi, %eax                                    #3186.17
        movl      %eax, %edi                                    #3186.17
        sarl      $4, %edi                                      #3186.17
        orl       %edi, %eax                                    #3186.17
        movl      %eax, 68(%esp)                                #3186.17
        movl      %ecx, %edi                                    #3190.17
        movl      80(%esp), %ecx                                #3193.29
        andl      $-4, %edi                                     #3190.17
        orl       28(%esp), %ecx                                #3193.29
        orl       24(%esp), %ecx                                #3193.34
        orl       %eax, %ecx                                    #3193.39
        je        ..B2.26       # Prob 78%                      #3193.47
                                # LOE edx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.18:                        # Preds ..B2.17
        movl      %ebx, 16(%esp)                                #
        movl      80(%esp), %ebx                                #3211.58
        movl      %edx, 20(%esp)                                #
        movzwl    %bx, %eax                                     #3211.58
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #3211.30
        movswl    %ax, %eax                                     #3211.30
        movl      %eax, (%esp)                                  #3211.30
        movl      28(%esp), %eax                                #3212.58
        movzwl    %ax, %eax                                     #3212.58
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #3212.30
        movswl    %ax, %edx                                     #3212.30
        lea       1(,%edx,4), %edx                              #3220.40
        movl      24(%esp), %eax                                #3213.58
        movzwl    %ax, %eax                                     #3213.58
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #3213.30
        movswl    %ax, %ecx                                     #3213.30
        movl      68(%esp), %eax                                #3214.58
        movzwl    %ax, %eax                                     #3214.58
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #3214.30
        movl      %edx, 4(%esp)                                 #3220.40
        movswl    %ax, %eax                                     #3214.30
        lea       2(,%ecx,4), %edx                              #3221.40
        movl      %edx, 8(%esp)                                 #3221.40
        movl      20(%esp), %edx                                #3226.32
        lea       3(,%eax,4), %eax                              #3222.40
        testl     %ebx, %ebx                                    #3226.32
        movl      16(%esp), %ebx                                #3226.32
        movl      %eax, 12(%esp)                                #3222.40
        je        ..B2.20       # Prob 50%                      #3226.32
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.19:                        # Preds ..B2.18
        movl      (%esp), %eax                                  #3226.43
        movl      72(%esp), %ecx                                #3226.49
        lea       (%edi,%eax,4), %eax                           #3226.43
        cmpl      %eax, %ecx                                    #3226.49
        jae       ..B2.44       # Prob 1%                       #3226.49
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.20:                        # Preds ..B2.19 ..B2.18
        movl      28(%esp), %eax                                #3227.32
        testl     %eax, %eax                                    #3227.32
        je        ..B2.22       # Prob 50%                      #3227.32
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.21:                        # Preds ..B2.20
        movl      4(%esp), %ecx                                 #3227.43
        movl      72(%esp), %eax                                #3227.49
        addl      %edi, %ecx                                    #3227.43
        cmpl      %ecx, %eax                                    #3227.49
        jae       ..B2.44       # Prob 1%                       #3227.49
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.22:                        # Preds ..B2.21 ..B2.20
        movl      24(%esp), %eax                                #3228.32
        testl     %eax, %eax                                    #3228.32
        je        ..B2.24       # Prob 50%                      #3228.32
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.23:                        # Preds ..B2.22
        movl      8(%esp), %ecx                                 #3228.43
        movl      72(%esp), %eax                                #3228.49
        addl      %edi, %ecx                                    #3228.43
        cmpl      %ecx, %eax                                    #3228.49
        jae       ..B2.44       # Prob 1%                       #3228.49
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.24:                        # Preds ..B2.23 ..B2.22
        movl      68(%esp), %eax                                #3229.32
        testl     %eax, %eax                                    #3229.32
        je        ..B2.26       # Prob 50%                      #3229.32
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.25:                        # Preds ..B2.24
        movl      12(%esp), %ecx                                #3229.43
        movl      72(%esp), %eax                                #3229.49
        addl      %edi, %ecx                                    #3229.43
        cmpl      %ecx, %eax                                    #3229.49
        jae       ..B2.44       # Prob 1%                       #3229.49
                                # LOE edx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.26:                        # Preds ..B2.25 ..B2.24 ..B2.17
        movl      72(%esp), %eax                                #3237.28
        lea       4(%edi), %ecx                                 #3234.17
        cmpl      %eax, %ecx                                    #3237.28
        ja        ..B2.41       # Prob 1%                       #3237.28
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.27:                        # Preds ..B2.26
        addl      $-1, %esi                                     #3241.25
        jne       ..B2.36       # Prob 50%                      #3241.39
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.28:                        # Preds ..B2.27
        movl      64(%esp), %eax                                #3262.25
        cmpl      %eax, %ebx                                    #3262.25
        jae       ..B2.41       # Prob 1%                       #3262.25
                                # LOE edx ebx edi xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.29:                        # Preds ..B2.28
        movl      72(%esp), %eax                                #3267.24
        lea       36(%edi), %ecx                                #3266.13
        cmpl      %eax, %ecx                                    #3267.24
        ja        ..B2.41       # Prob 1%                       #3267.24
                                # LOE edx ecx ebx xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.30:                        # Preds ..B2.29
        testb     $15, %bl                                      #3272.22
        jne       ..B2.32       # Prob 50%                      #3272.22
                                # LOE edx ecx ebx xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.31:                        # Preds ..B2.30
        movdqa    (%ebx), %xmm0                                 #3272.22
        jmp       ..B2.33       # Prob 100%                     #3272.22
                                # LOE edx ecx ebx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.32:                        # Preds ..B2.30
        movdqu    (%ebx), %xmm0                                 #3272.22
                                # LOE edx ecx ebx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.33:                        # Preds ..B2.31 ..B2.32
        movl      64(%esp), %eax                                #3353.24
        addl      $16, %ebx                                     #3350.13
        movl      %ebx, 76(%esp)                                #3350.13
        cmpl      %eax, %ebx                                    #3353.24
        jae       ..B2.35       # Prob 12%                      #3353.24
                                # LOE edx ecx ebx bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.34:                        # Preds ..B2.33
        movzbl    -1(%ebx), %edx                                #3354.32
        shll      $8, %edx                                      #3354.46
                                # LOE edx ecx ebx bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.35:                        # Preds ..B2.33 ..B2.11 ..B2.34
        movl      $8, %esi                                      #3155.13
        jmp       ..B2.39       # Prob 100%                     #3155.13
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.36:                        # Preds ..B2.27
        movl      76(%esp), %eax                                #3249.26
        movl      64(%esp), %edi                                #3249.26
        psrldq    $1, %xmm0                                     #3246.26
        cmpl      %edi, %eax                                    #3249.26
        jae       ..B2.38       # Prob 19%                      #3249.26
                                # LOE eax edx ecx ebx esi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.37:                        # Preds ..B2.36
        movzbl    (%eax), %edi                                  #3252.37
        sarl      $8, %edx                                      #3251.21
        shll      $8, %edi                                      #3252.48
        orl       %edi, %edx                                    #3252.21
        pinsrw    $7, %edx, %xmm0                               #3253.30
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.38:                        # Preds ..B2.37 ..B2.36
        addl      $1, 76(%esp)                                  #3257.20
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.39:                        # Preds ..B2.35 ..B2.38 ..B2.43
        movdqa    %xmm0, %xmm7                                  #3165.22
        pand      %xmm6, %xmm7                                  #3165.22
        pcmpeqd   48(%esp), %xmm7                               #3166.22
        pmovmskb  %xmm7, %eax                                   #3167.22
        movl      %eax, %edi                                    #3168.17
        andl      $3855, %edi                                   #3168.17
        shll      $4, %edi                                      #3168.17
        andl      %edi, %eax                                    #3168.17
        movl      %eax, %edi                                    #3168.17
        sarl      $4, %edi                                      #3168.17
        orl       %edi, %eax                                    #3168.17
        movl      %eax, 80(%esp)                                #3168.17
        jmp       ..B2.15       # Prob 100%                     #3168.17
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.41:                        # Preds ..B2.26 ..B2.28 ..B2.29 # Infreq
        xorl      %eax, %eax                                    #3363.12
        addl      $84, %esp                                     #3363.12
        popl      %ebx                                          #3363.12
        popl      %esi                                          #3363.12
        popl      %edi                                          #3363.12
        movl      %ebp, %esp                                    #3363.12
        popl      %ebp                                          #3363.12
        ret                                                     #3363.12
                                # LOE
..B2.43:                        # Preds ..B2.7                  # Infreq
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    (%esp), %xmm0                                 #
        movdqa    %xmm7, 48(%esp)                               #
        movl      %edi, 64(%esp)                                #
        movl      $8, %esi                                      #
        jmp       ..B2.39       # Prob 100%                     #
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B2.44:                        # Preds ..B2.21 ..B2.19 ..B2.25 ..B2.23 # Infreq
        movl      $1, %eax                                      #3228.63
        addl      $84, %esp                                     #3228.63
        popl      %ebx                                          #3228.63
        popl      %esi                                          #3228.63
        popl      %edi                                          #3228.63
        movl      %ebp, %esp                                    #3228.63
        popl      %ebp                                          #3228.63
        ret                                                     #3228.63
        .align    2,0x90
                                # LOE
# mark_end;
	.type	eval_2na_64,@function
	.size	eval_2na_64,.-eval_2na_64
	.data
# -- End  eval_2na_64
	.text
# -- Begin  eval_4na_64
# mark_begin;
       .align    2,0x90
eval_4na_64:
# parameter 1: %eax
# parameter 2: %edx
# parameter 3: %ecx
# parameter 4: 20 + %ebp
..B3.1:                         # Preds ..B3.0
        pushl     %ebp                                          #4501.1
        movl      %esp, %ebp                                    #4501.1
        andl      $-16, %esp                                    #4501.1
        pushl     %edi                                          #4501.1
        pushl     %esi                                          #4501.1
        pushl     %ebx                                          #4501.1
        subl      $132, %esp                                    #4501.1
        movdqa    16(%eax), %xmm5                               #4554.5
        movdqa    32(%eax), %xmm6                               #4554.5
        movl      20(%ebp), %ebx                                #4499.5
        movdqa    48(%eax), %xmm4                               #4554.5
        movdqa    64(%eax), %xmm3                               #4554.5
        movdqa    80(%eax), %xmm2                               #4554.5
        movdqa    96(%eax), %xmm1                               #4554.5
        movdqa    128(%eax), %xmm7                              #4554.5
        movl      %ecx, %edi                                    #4538.49
        shrl      $2, %edi                                      #4538.49
        call      ..L2          # Prob 100%                     #4501.1
..L2:                                                           #
        popl      %esi                                          #4501.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L2], %esi       #4501.1
        movl      %esi, 32(%esp)                                #4501.1
        lea       (%ecx,%ebx), %esi                             #4535.5
        lea       3(%ecx,%ebx), %ebx                            #4544.50
        shrl      $2, %ebx                                      #4544.57
        subl      4(%eax), %esi                                 #4541.12
        movl      %esi, 112(%esp)                               #4541.12
        movzbl    (%edi,%edx), %esi                             #4547.14
        addl      %edx, %ebx                                    #4544.30
        movl      %ebx, 116(%esp)                               #4544.30
        movl      32(%esp), %ebx                                #4547.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4547.14
        movw      %si, (%esp)                                   #4547.14
        movzbl    1(%edi,%edx), %esi                            #4547.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4547.14
        movw      %si, 2(%esp)                                  #4547.14
        movzbl    2(%edi,%edx), %esi                            #4547.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4547.14
        movw      %si, 4(%esp)                                  #4547.14
        movzbl    3(%edi,%edx), %esi                            #4547.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4547.14
        movw      %si, 6(%esp)                                  #4547.14
        movzbl    4(%edi,%edx), %esi                            #4547.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4547.14
        movw      %si, 8(%esp)                                  #4547.14
        movzbl    5(%edi,%edx), %esi                            #4547.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4547.14
        movw      %si, 10(%esp)                                 #4547.14
        movzbl    6(%edi,%edx), %esi                            #4547.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4547.14
        movw      %si, 12(%esp)                                 #4547.14
        movzbl    7(%edi,%edx), %esi                            #4547.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4547.14
        movw      %si, 14(%esp)                                 #4547.14
        movdqu    (%esp), %xmm0                                 #4547.14
        lea       8(%edx,%edi), %esi                            #4548.5
        movdqa    %xmm0, 16(%esp)                               #4547.14
        movdqa    112(%eax), %xmm0                              #4554.5
        movl      %esi, %edx                                    #4548.5
        movl      %ecx, %eax                                    #4568.20
        andl      $3, %eax                                      #4568.20
        je        ..B3.33       # Prob 20%                      #4568.20
                                # LOE eax edx ecx ebx esi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B3.2:                         # Preds ..B3.1
        cmpl      $1, %eax                                      #4568.20
        je        ..B3.8        # Prob 25%                      #4568.20
                                # LOE eax edx ecx ebx esi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B3.3:                         # Preds ..B3.2
        cmpl      $2, %eax                                      #4568.20
        je        ..B3.7        # Prob 33%                      #4568.20
                                # LOE eax edx ecx ebx esi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B3.4:                         # Preds ..B3.3
        cmpl      $3, %eax                                      #4568.20
        je        ..B3.6        # Prob 50%                      #4568.20
                                # LOE edx ecx ebx esi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B3.5:                         # Preds ..B3.4
        movdqa    %xmm7, 80(%esp)                               #
        movdqa    %xmm0, 48(%esp)                               #
        movdqa    16(%esp), %xmm0                               #
        movdqa    %xmm6, 64(%esp)                               #
        movl      %ecx, 96(%esp)                                #
        jmp       ..B3.25       # Prob 100%                     #
                                # LOE edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.6:                         # Preds ..B3.4
        movdqa    %xmm7, 80(%esp)                               #
        movdqa    %xmm0, 48(%esp)                               #
        movdqa    16(%esp), %xmm0                               #
        movdqa    %xmm6, 64(%esp)                               #
        xorl      %eax, %eax                                    #
        movl      %eax, 40(%esp)                                #
        movl      %ecx, 96(%esp)                                #
        xorl      %edi, %edi                                    #
        movl      $4, %eax                                      #
        movl      %edi, 44(%esp)                                #
        movl      %edi, 100(%esp)                               #
        jmp       ..B3.11       # Prob 100%                     #
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.7:                         # Preds ..B3.3
        movdqa    %xmm7, 80(%esp)                               #
        movdqa    %xmm0, 48(%esp)                               #
        movdqa    16(%esp), %xmm0                               #
        movdqa    %xmm6, 64(%esp)                               #
        xorl      %eax, %eax                                    #
        movl      %eax, 44(%esp)                                #
        movl      %ecx, 96(%esp)                                #
        xorl      %edi, %edi                                    #
        movl      $4, %eax                                      #
        movl      %edi, 100(%esp)                               #
        jmp       ..B3.10       # Prob 100%                     #
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.8:                         # Preds ..B3.2
        movdqa    %xmm7, 80(%esp)                               #
        movdqa    %xmm0, 48(%esp)                               #
        movdqa    16(%esp), %xmm0                               #
        movdqa    %xmm6, 64(%esp)                               #
        xorl      %edi, %edi                                    #
        movl      %ecx, 96(%esp)                                #
        movl      %edi, 100(%esp)                               #
        movl      $4, %eax                                      #
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.9:                         # Preds ..B3.8 ..B3.29
        movdqa    %xmm0, %xmm7                                  #4592.22
        movdqa    %xmm0, %xmm6                                  #4593.22
        pand      %xmm4, %xmm7                                  #4592.22
        pand      %xmm3, %xmm6                                  #4593.22
        pcmpeqd   %xmm6, %xmm7                                  #4594.22
        pmovmskb  %xmm7, %ecx                                   #4595.22
        movl      %ecx, %edi                                    #4596.17
        andl      $3855, %edi                                   #4596.17
        shll      $4, %edi                                      #4596.17
        andl      %edi, %ecx                                    #4596.17
        movl      %ecx, %edi                                    #4596.17
        sarl      $4, %edi                                      #4596.17
        orl       %edi, %ecx                                    #4596.17
        movl      %ecx, 44(%esp)                                #4596.17
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
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
        movl      %ecx, 40(%esp)                                #4603.17
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.11:                        # Preds ..B3.6 ..B3.10
        movdqa    %xmm0, %xmm7                                  #4606.22
        movdqa    %xmm0, %xmm6                                  #4607.22
        pand      48(%esp), %xmm7                               #4606.22
        pand      80(%esp), %xmm6                               #4607.22
        pcmpeqd   %xmm6, %xmm7                                  #4608.22
        pmovmskb  %xmm7, %ecx                                   #4609.22
        movl      %ecx, %edi                                    #4610.17
        andl      $3855, %edi                                   #4610.17
        shll      $4, %edi                                      #4610.17
        andl      %edi, %ecx                                    #4610.17
        movl      %ecx, %edi                                    #4610.17
        sarl      $4, %edi                                      #4610.17
        orl       %edi, %ecx                                    #4610.17
        movl      96(%esp), %edi                                #4614.17
        movl      %ecx, 104(%esp)                               #4610.17
        andl      $-4, %edi                                     #4614.17
        movl      %edi, 108(%esp)                               #4614.17
        movl      100(%esp), %edi                               #4617.29
        orl       44(%esp), %edi                                #4617.29
        orl       40(%esp), %edi                                #4617.34
        orl       %ecx, %edi                                    #4617.39
        je        ..B3.20       # Prob 78%                      #4617.47
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.12:                        # Preds ..B3.11
        movl      100(%esp), %edi                               #4635.58
        movl      %eax, 28(%esp)                                #
        movl      %esi, 32(%esp)                                #
        movzwl    %di, %eax                                     #4635.58
        movl      %edx, 36(%esp)                                #
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4635.30
        movswl    %ax, %eax                                     #4635.30
        movl      %eax, 16(%esp)                                #4635.30
        movl      44(%esp), %eax                                #4636.58
        movzwl    %ax, %eax                                     #4636.58
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4636.30
        movswl    %ax, %esi                                     #4636.30
        movl      40(%esp), %eax                                #4637.58
        movzwl    %ax, %eax                                     #4637.58
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4637.30
        movswl    %ax, %ecx                                     #4637.30
        movl      104(%esp), %eax                               #4638.58
        movzwl    %ax, %eax                                     #4638.58
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4638.30
        movswl    %ax, %edx                                     #4638.30
        lea       1(%esi,%esi), %eax                            #4644.40
        movl      32(%esp), %esi                                #4650.32
        movl      %eax, 20(%esp)                                #4644.40
        lea       2(%ecx,%ecx), %eax                            #4645.40
        testl     %edi, %edi                                    #4650.32
        movl      %eax, 24(%esp)                                #4645.40
        lea       3(%edx,%edx), %eax                            #4646.40
        movl      36(%esp), %edx                                #4650.32
        movl      %eax, 96(%esp)                                #4646.40
        movl      28(%esp), %eax                                #4650.32
        je        ..B3.14       # Prob 50%                      #4650.32
                                # LOE eax edx ebx esi al dl ah dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.13:                        # Preds ..B3.12
        movl      16(%esp), %edi                                #4650.43
        movl      108(%esp), %ecx                               #4650.43
        lea       (%ecx,%edi,2), %edi                           #4650.43
        movl      112(%esp), %ecx                               #4650.49
        cmpl      %edi, %ecx                                    #4650.49
        jae       ..B3.34       # Prob 1%                       #4650.49
                                # LOE eax edx ebx esi al dl ah dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.14:                        # Preds ..B3.13 ..B3.12
        movl      44(%esp), %ecx                                #4651.32
        testl     %ecx, %ecx                                    #4651.32
        je        ..B3.16       # Prob 50%                      #4651.32
                                # LOE eax edx ebx esi al dl ah dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.15:                        # Preds ..B3.14
        movl      20(%esp), %edi                                #4651.43
        addl      108(%esp), %edi                               #4651.43
        movl      112(%esp), %ecx                               #4651.49
        cmpl      %edi, %ecx                                    #4651.49
        jae       ..B3.34       # Prob 1%                       #4651.49
                                # LOE eax edx ebx esi al dl ah dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.16:                        # Preds ..B3.15 ..B3.14
        movl      40(%esp), %ecx                                #4652.32
        testl     %ecx, %ecx                                    #4652.32
        je        ..B3.18       # Prob 50%                      #4652.32
                                # LOE eax edx ebx esi al dl ah dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.17:                        # Preds ..B3.16
        movl      24(%esp), %edi                                #4652.43
        addl      108(%esp), %edi                               #4652.43
        movl      112(%esp), %ecx                               #4652.49
        cmpl      %edi, %ecx                                    #4652.49
        jae       ..B3.34       # Prob 1%                       #4652.49
                                # LOE eax edx ebx esi al dl ah dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.18:                        # Preds ..B3.17 ..B3.16
        movl      104(%esp), %ecx                               #4653.32
        testl     %ecx, %ecx                                    #4653.32
        je        ..B3.20       # Prob 50%                      #4653.32
                                # LOE eax edx ebx esi al dl ah dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.19:                        # Preds ..B3.18
        movl      96(%esp), %edi                                #4653.43
        addl      108(%esp), %edi                               #4653.43
        movl      112(%esp), %ecx                               #4653.49
        cmpl      %edi, %ecx                                    #4653.49
        jae       ..B3.34       # Prob 1%                       #4653.49
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.20:                        # Preds ..B3.19 ..B3.18 ..B3.11
        movl      108(%esp), %ecx                               #4658.17
        lea       4(%ecx), %edi                                 #4658.17
        movl      112(%esp), %ecx                               #4661.28
        movl      %edi, 96(%esp)                                #4658.17
        cmpl      %ecx, %edi                                    #4661.28
        ja        ..B3.31       # Prob 1%                       #4661.28
                                # LOE eax edx ecx ebx esi cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.21:                        # Preds ..B3.20
        addl      $-1, %eax                                     #4665.25
        jne       ..B3.26       # Prob 50%                      #4665.39
                                # LOE eax edx ecx ebx esi cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.22:                        # Preds ..B3.21
        movl      116(%esp), %eax                               #4682.25
        cmpl      %eax, %esi                                    #4682.25
        jae       ..B3.31       # Prob 1%                       #4682.25
                                # LOE ecx ebx esi cl ch xmm1 xmm2 xmm3 xmm4 xmm5
..B3.23:                        # Preds ..B3.22
        movl      108(%esp), %eax                               #4686.13
        lea       20(%eax), %edx                                #4686.13
        movl      %edx, 96(%esp)                                #4686.13
        cmpl      %ecx, %edx                                    #4687.24
        ja        ..B3.31       # Prob 1%                       #4687.24
                                # LOE ebx esi xmm1 xmm2 xmm3 xmm4 xmm5
..B3.24:                        # Preds ..B3.23
        movzbl    (%esi), %eax                                  #4692.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %edx        #4692.22
        movzbl    1(%esi), %ecx                                 #4692.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%ecx,2), %edi        #4692.22
        movzbl    2(%esi), %eax                                 #4692.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4692.22
        movw      %dx, (%esp)                                   #4692.22
        movw      %di, 2(%esp)                                  #4692.22
        movw      %ax, 4(%esp)                                  #4692.22
        movzbl    3(%esi), %eax                                 #4692.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4692.22
        movw      %ax, 6(%esp)                                  #4692.22
        movzbl    4(%esi), %eax                                 #4692.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4692.22
        movw      %ax, 8(%esp)                                  #4692.22
        movzbl    5(%esi), %eax                                 #4692.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4692.22
        movw      %ax, 10(%esp)                                 #4692.22
        movzbl    6(%esi), %eax                                 #4692.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4692.22
        movw      %ax, 12(%esp)                                 #4692.22
        movzbl    7(%esi), %eax                                 #4692.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4692.22
        addl      $8, %esi                                      #4707.13
        movl      %esi, %edx                                    #4707.13
        movw      %ax, 14(%esp)                                 #4692.22
        movdqu    (%esp), %xmm0                                 #4692.22
                                # LOE edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.25:                        # Preds ..B3.5 ..B3.24
        movl      $4, %eax                                      #4575.13
        jmp       ..B3.29       # Prob 100%                     #4575.13
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.26:                        # Preds ..B3.21
        movl      116(%esp), %ecx                               #4673.26
        psrldq    $2, %xmm0                                     #4670.26
        cmpl      %ecx, %edx                                    #4673.26
        jae       ..B3.28       # Prob 19%                      #4673.26
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.27:                        # Preds ..B3.26
        movzbl    (%edx), %ecx                                  #4674.72
        movzwl    expand_2na.0@GOTOFF(%ebx,%ecx,2), %edi        #4674.57
        pinsrw    $7, %edi, %xmm0                               #4674.30
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.28:                        # Preds ..B3.27 ..B3.26
        addl      $1, %edx                                      #4677.20
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.29:                        # Preds ..B3.25 ..B3.28 ..B3.33
        movdqa    %xmm0, %xmm7                                  #4585.22
        movdqa    %xmm0, %xmm6                                  #4586.22
        pand      64(%esp), %xmm6                               #4586.22
        pand      %xmm5, %xmm7                                  #4585.22
        pcmpeqd   %xmm6, %xmm7                                  #4587.22
        pmovmskb  %xmm7, %ecx                                   #4588.22
        movl      %ecx, %edi                                    #4589.17
        andl      $3855, %edi                                   #4589.17
        shll      $4, %edi                                      #4589.17
        andl      %edi, %ecx                                    #4589.17
        movl      %ecx, %edi                                    #4589.17
        sarl      $4, %edi                                      #4589.17
        orl       %edi, %ecx                                    #4589.17
        movl      %ecx, 100(%esp)                               #4589.17
        jmp       ..B3.9        # Prob 100%                     #4589.17
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.31:                        # Preds ..B3.20 ..B3.22 ..B3.23 # Infreq
        xorl      %eax, %eax                                    #4718.12
        addl      $132, %esp                                    #4718.12
        popl      %ebx                                          #4718.12
        popl      %esi                                          #4718.12
        popl      %edi                                          #4718.12
        movl      %ebp, %esp                                    #4718.12
        popl      %ebp                                          #4718.12
        ret                                                     #4718.12
                                # LOE
..B3.33:                        # Preds ..B3.1                  # Infreq
        movdqa    %xmm7, 80(%esp)                               #
        movdqa    %xmm0, 48(%esp)                               #
        movdqa    16(%esp), %xmm0                               #
        movdqa    %xmm6, 64(%esp)                               #
        movl      $4, %eax                                      #
        movl      %ecx, 96(%esp)                                #
        jmp       ..B3.29       # Prob 100%                     #
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B3.34:                        # Preds ..B3.13 ..B3.19 ..B3.17 ..B3.15 # Infreq
        movl      $1, %eax                                      #4651.63
        addl      $132, %esp                                    #4651.63
        popl      %ebx                                          #4651.63
        popl      %esi                                          #4651.63
        popl      %edi                                          #4651.63
        movl      %ebp, %esp                                    #4651.63
        popl      %ebp                                          #4651.63
        ret                                                     #4651.63
        .align    2,0x90
                                # LOE
# mark_end;
	.type	eval_4na_64,@function
	.size	eval_4na_64,.-eval_4na_64
	.data
# -- End  eval_4na_64
	.text
# -- Begin  eval_2na_8
# mark_begin;
       .align    2,0x90
eval_2na_8:
# parameter 1: %eax
# parameter 2: %edx
# parameter 3: %ecx
# parameter 4: 20 + %ebp
..B4.1:                         # Preds ..B4.0
        pushl     %ebp                                          #2196.1
        movl      %esp, %ebp                                    #2196.1
        andl      $-16, %esp                                    #2196.1
        pushl     %edi                                          #2196.1
        pushl     %esi                                          #2196.1
        pushl     %ebx                                          #2196.1
        subl      $84, %esp                                     #2196.1
        movl      %ecx, %esi                                    #2236.49
        shrl      $2, %esi                                      #2236.49
        movl      20(%ebp), %ebx                                #2194.5
        movl      %esi, 68(%esp)                                #2236.49
        lea       (%ecx,%ebx), %edi                             #2233.5
        lea       (%edx,%esi), %esi                             #2236.30
        lea       3(%ecx,%ebx), %ebx                            #2242.50
        subl      4(%eax), %edi                                 #2239.12
        movl      %edi, 64(%esp)                                #2239.12
        movl      68(%esp), %edi                                #2245.14
        shrl      $2, %ebx                                      #2242.57
        addl      %edx, %ebx                                    #2242.30
        testl     $15, %esi                                     #2245.14
        jne       ..B4.3        # Prob 50%                      #2245.14
                                # LOE eax edx ecx ebx esi edi
..B4.2:                         # Preds ..B4.1
        movdqa    (%esi), %xmm0                                 #2245.14
        movdqa    %xmm0, 48(%esp)                               #2245.14
        jmp       ..B4.4        # Prob 100%                     #2245.14
                                # LOE eax edx ecx ebx edi
..B4.3:                         # Preds ..B4.1
        movdqu    (%esi), %xmm0                                 #2245.14
        movdqa    %xmm0, 48(%esp)                               #2245.14
                                # LOE eax edx ecx ebx edi
..B4.4:                         # Preds ..B4.2 ..B4.3
        movdqa    16(%eax), %xmm7                               #2256.5
        movdqa    32(%eax), %xmm6                               #2256.5
        movdqa    48(%eax), %xmm5                               #2256.5
        movdqa    64(%eax), %xmm4                               #2256.5
        movdqa    80(%eax), %xmm3                               #2256.5
        movdqa    96(%eax), %xmm2                               #2256.5
        movdqa    112(%eax), %xmm1                              #2256.5
        movdqa    128(%eax), %xmm0                              #2256.5
        lea       16(%edx,%edi), %edi                           #2246.5
        movl      %ecx, %eax                                    #2268.20
        andl      $3, %eax                                      #2268.20
        je        ..B4.8        # Prob 20%                      #2268.20
                                # LOE eax ecx ebx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B4.5:                         # Preds ..B4.4
        cmpl      $1, %eax                                      #2268.20
        je        ..B4.11       # Prob 25%                      #2268.20
                                # LOE eax ecx ebx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B4.6:                         # Preds ..B4.5
        cmpl      $2, %eax                                      #2268.20
        je        ..B4.10       # Prob 33%                      #2268.20
                                # LOE eax ecx ebx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B4.7:                         # Preds ..B4.6
        cmpl      $3, %eax                                      #2268.20
        je        ..B4.9        # Prob 50%                      #2268.20
                                # LOE ecx ebx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B4.8:                         # Preds ..B4.4 ..B4.7
        movdqa    %xmm7, 32(%esp)                               #
        movl      %ebx, 28(%esp)                                #
        jmp       ..B4.30       # Prob 100%                     #
                                # LOE ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.9:                         # Preds ..B4.7
        movdqa    %xmm7, 32(%esp)                               #
        movl      %ebx, 28(%esp)                                #
        xorl      %eax, %eax                                    #
        movl      %eax, 68(%esp)                                #
        xorl      %edx, %edx                                    #
        xorl      %esi, %esi                                    #
        jmp       ..B4.14       # Prob 100%                     #
                                # LOE edx ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.10:                        # Preds ..B4.6
        movdqa    %xmm7, 32(%esp)                               #
        movl      %ebx, 28(%esp)                                #
        xorl      %edx, %edx                                    #
        xorl      %esi, %esi                                    #
        jmp       ..B4.13       # Prob 100%                     #
                                # LOE edx ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.11:                        # Preds ..B4.5
        movdqa    %xmm7, 32(%esp)                               #
        movl      %ebx, 28(%esp)                                #
        xorl      %esi, %esi                                    #
                                # LOE ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.12:                        # Preds ..B4.11 ..B4.30
        movdqa    48(%esp), %xmm7                               #2291.22
        pand      %xmm4, %xmm7                                  #2291.22
        pcmpeqb   %xmm5, %xmm7                                  #2292.22
        pmovmskb  %xmm7, %edx                                   #2293.22
                                # LOE edx ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.13:                        # Preds ..B4.10 ..B4.12
        movdqa    48(%esp), %xmm7                               #2297.22
        pand      %xmm2, %xmm7                                  #2297.22
        pcmpeqb   %xmm3, %xmm7                                  #2298.22
        pmovmskb  %xmm7, %ebx                                   #2299.22
        movl      %ebx, 68(%esp)                                #2299.22
                                # LOE edx ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.14:                        # Preds ..B4.9 ..B4.13
        movdqa    48(%esp), %xmm7                               #2303.22
        pand      %xmm0, %xmm7                                  #2303.22
        pcmpeqb   %xmm1, %xmm7                                  #2304.22
        pmovmskb  %xmm7, %ebx                                   #2305.22
        andl      $-4, %ecx                                     #2310.17
        movl      %ebx, 24(%esp)                                #2305.22
        movl      %esi, %eax                                    #2313.29
        orl       %edx, %eax                                    #2313.29
        orl       68(%esp), %eax                                #2313.34
        orl       %ebx, %eax                                    #2313.39
        je        ..B4.23       # Prob 78%                      #2313.47
                                # LOE edx ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.15:                        # Preds ..B4.14
        movl      %ecx, 16(%esp)                                #
        movl      %esi, %eax                                    #2331.30
        movl      %edi, 20(%esp)                                #
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2331.30
        movswl    %ax, %ecx                                     #2331.30
        movl      %ecx, (%esp)                                  #2331.30
        movl      %edx, %eax                                    #2332.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2332.30
        movswl    %ax, %edi                                     #2332.30
        lea       1(,%edi,4), %edi                              #2340.40
        movl      68(%esp), %eax                                #2333.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2333.30
        movswl    %ax, %ebx                                     #2333.30
        movl      24(%esp), %eax                                #2334.30
        lea       2(,%ebx,4), %ebx                              #2341.40
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2334.30
        movl      %edi, 4(%esp)                                 #2340.40
        movl      20(%esp), %edi                                #2346.32
        movl      %ebx, 8(%esp)                                 #2341.40
        movswl    %ax, %ecx                                     #2334.30
        lea       3(,%ecx,4), %ecx                              #2342.40
        movl      %ecx, 12(%esp)                                #2342.40
        movl      16(%esp), %ecx                                #2346.32
        testl     %esi, %esi                                    #2346.32
        je        ..B4.17       # Prob 50%                      #2346.32
                                # LOE edx ecx edi cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.16:                        # Preds ..B4.15
        movl      (%esp), %ebx                                  #2346.43
        movl      64(%esp), %esi                                #2346.49
        lea       (%ecx,%ebx,4), %ebx                           #2346.43
        cmpl      %ebx, %esi                                    #2346.49
        jae       ..B4.34       # Prob 1%                       #2346.49
                                # LOE edx ecx edi cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.17:                        # Preds ..B4.16 ..B4.15
        testl     %edx, %edx                                    #2347.32
        je        ..B4.19       # Prob 50%                      #2347.32
                                # LOE ecx edi cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.18:                        # Preds ..B4.17
        movl      4(%esp), %edx                                 #2347.43
        movl      64(%esp), %eax                                #2347.49
        addl      %ecx, %edx                                    #2347.43
        cmpl      %edx, %eax                                    #2347.49
        jae       ..B4.34       # Prob 1%                       #2347.49
                                # LOE ecx edi cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.19:                        # Preds ..B4.18 ..B4.17
        movl      68(%esp), %eax                                #2348.32
        testl     %eax, %eax                                    #2348.32
        je        ..B4.21       # Prob 50%                      #2348.32
                                # LOE ecx edi cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.20:                        # Preds ..B4.19
        movl      8(%esp), %edx                                 #2348.43
        movl      64(%esp), %eax                                #2348.49
        addl      %ecx, %edx                                    #2348.43
        cmpl      %edx, %eax                                    #2348.49
        jae       ..B4.34       # Prob 1%                       #2348.49
                                # LOE ecx edi cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.21:                        # Preds ..B4.20 ..B4.19
        movl      24(%esp), %eax                                #2349.32
        testl     %eax, %eax                                    #2349.32
        je        ..B4.23       # Prob 50%                      #2349.32
                                # LOE ecx edi cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.22:                        # Preds ..B4.21
        movl      12(%esp), %edx                                #2349.43
        movl      64(%esp), %eax                                #2349.49
        addl      %ecx, %edx                                    #2349.43
        cmpl      %edx, %eax                                    #2349.49
        jae       ..B4.34       # Prob 1%                       #2349.49
                                # LOE ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.23:                        # Preds ..B4.22 ..B4.21 ..B4.14
        movl      64(%esp), %eax                                #2357.28
        lea       4(%ecx), %edx                                 #2386.13
        cmpl      %edx, %eax                                    #2357.28
        jb        ..B4.32       # Prob 1%                       #2357.28
                                # LOE ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.24:                        # Preds ..B4.23
        movl      28(%esp), %eax                                #2382.25
        cmpl      %eax, %edi                                    #2382.25
        jae       ..B4.32       # Prob 1%                       #2382.25
                                # LOE ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.25:                        # Preds ..B4.24
        movl      64(%esp), %eax                                #2387.24
        addl      $64, %ecx                                     #2386.13
        cmpl      %eax, %ecx                                    #2387.24
        ja        ..B4.32       # Prob 1%                       #2387.24
                                # LOE ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.26:                        # Preds ..B4.25
        testl     $15, %edi                                     #2392.22
        jne       ..B4.28       # Prob 50%                      #2392.22
                                # LOE ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.27:                        # Preds ..B4.26
        movdqa    (%edi), %xmm7                                 #2392.22
        movdqa    %xmm7, 48(%esp)                               #2392.22
        jmp       ..B4.29       # Prob 100%                     #2392.22
                                # LOE ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.28:                        # Preds ..B4.26
        movdqu    (%edi), %xmm7                                 #2392.22
        movdqa    %xmm7, 48(%esp)                               #2392.22
                                # LOE ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.29:                        # Preds ..B4.27 ..B4.28
        addl      $16, %edi                                     #2470.13
                                # LOE ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.30:                        # Preds ..B4.29 ..B4.8
        movdqa    48(%esp), %xmm7                               #2285.22
        pand      %xmm6, %xmm7                                  #2285.22
        pcmpeqb   32(%esp), %xmm7                               #2286.22
        pmovmskb  %xmm7, %esi                                   #2287.22
        jmp       ..B4.12       # Prob 100%                     #2287.22
                                # LOE ecx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B4.32:                        # Preds ..B4.23 ..B4.24 ..B4.25 # Infreq
        xorl      %eax, %eax                                    #2483.12
        addl      $84, %esp                                     #2483.12
        popl      %ebx                                          #2483.12
        popl      %esi                                          #2483.12
        popl      %edi                                          #2483.12
        movl      %ebp, %esp                                    #2483.12
        popl      %ebp                                          #2483.12
        ret                                                     #2483.12
                                # LOE
..B4.34:                        # Preds ..B4.22 ..B4.20 ..B4.18 ..B4.16 # Infreq
        movl      $1, %eax                                      #2346.63
        addl      $84, %esp                                     #2346.63
        popl      %ebx                                          #2346.63
        popl      %esi                                          #2346.63
        popl      %edi                                          #2346.63
        movl      %ebp, %esp                                    #2346.63
        popl      %ebp                                          #2346.63
        ret                                                     #2346.63
        .align    2,0x90
                                # LOE
# mark_end;
	.type	eval_2na_8,@function
	.size	eval_2na_8,.-eval_2na_8
	.data
# -- End  eval_2na_8
	.text
# -- Begin  eval_2na_16
# mark_begin;
       .align    2,0x90
eval_2na_16:
# parameter 1: %eax
# parameter 2: %edx
# parameter 3: %ecx
# parameter 4: 20 + %ebp
..B5.1:                         # Preds ..B5.0
        pushl     %ebp                                          #2489.1
        movl      %esp, %ebp                                    #2489.1
        andl      $-16, %esp                                    #2489.1
        pushl     %edi                                          #2489.1
        pushl     %esi                                          #2489.1
        pushl     %ebx                                          #2489.1
        subl      $84, %esp                                     #2489.1
        movl      %ecx, %ebx                                    #2529.49
        shrl      $2, %ebx                                      #2529.49
        movl      20(%ebp), %edi                                #2487.5
        movl      %ebx, 80(%esp)                                #2529.49
        lea       (%ecx,%edi), %esi                             #2526.5
        lea       (%edx,%ebx), %ebx                             #2529.30
        lea       3(%ecx,%edi), %edi                            #2535.50
        subl      4(%eax), %esi                                 #2532.12
        movl      %esi, 72(%esp)                                #2532.12
        movl      80(%esp), %esi                                #2538.14
        shrl      $2, %edi                                      #2535.57
        addl      %edx, %edi                                    #2535.30
        testb     $15, %bl                                      #2538.14
        jne       ..B5.3        # Prob 50%                      #2538.14
                                # LOE eax edx ecx ebx esi edi
..B5.2:                         # Preds ..B5.1
        movdqa    (%ebx), %xmm0                                 #2538.14
        movdqa    %xmm0, (%esp)                                 #2538.14
        jmp       ..B5.4        # Prob 100%                     #2538.14
                                # LOE eax edx ecx esi edi
..B5.3:                         # Preds ..B5.1
        movdqu    (%ebx), %xmm0                                 #2538.14
        movdqa    %xmm0, (%esp)                                 #2538.14
                                # LOE eax edx ecx esi edi
..B5.4:                         # Preds ..B5.2 ..B5.3
        lea       16(%edx,%esi), %ebx                           #2539.5
        movl      %ebx, 76(%esp)                                #2539.5
        cmpl      %edi, %ebx                                    #2544.16
        jae       ..B5.6        # Prob 12%                      #2544.16
                                # LOE eax ecx ebx edi bl bh
..B5.5:                         # Preds ..B5.4
        movzbl    -1(%ebx), %edx                                #2545.24
        shll      $8, %edx                                      #2545.38
        jmp       ..B5.7        # Prob 100%                     #2545.38
                                # LOE eax edx ecx ebx edi bl bh
..B5.6:                         # Preds ..B5.4
        xorl      %edx, %edx                                    #
                                # LOE eax edx ecx ebx edi bl bh
..B5.7:                         # Preds ..B5.5 ..B5.6
        movdqa    16(%eax), %xmm0                               #2549.5
        movdqa    32(%eax), %xmm6                               #2549.5
        movdqa    48(%eax), %xmm5                               #2549.5
        movdqa    64(%eax), %xmm4                               #2549.5
        movdqa    80(%eax), %xmm3                               #2549.5
        movdqa    96(%eax), %xmm2                               #2549.5
        movdqa    112(%eax), %xmm1                              #2549.5
        movdqa    128(%eax), %xmm7                              #2549.5
        movl      %ecx, %eax                                    #2561.20
        andl      $3, %eax                                      #2561.20
        je        ..B5.43       # Prob 20%                      #2561.20
                                # LOE eax edx ecx ebx edi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B5.8:                         # Preds ..B5.7
        cmpl      $1, %eax                                      #2561.20
        je        ..B5.14       # Prob 25%                      #2561.20
                                # LOE eax edx ecx ebx edi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B5.9:                         # Preds ..B5.8
        cmpl      $2, %eax                                      #2561.20
        je        ..B5.13       # Prob 33%                      #2561.20
                                # LOE eax edx ecx ebx edi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B5.10:                        # Preds ..B5.9
        cmpl      $3, %eax                                      #2561.20
        je        ..B5.12       # Prob 50%                      #2561.20
                                # LOE edx ecx ebx edi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B5.11:                        # Preds ..B5.10
        movdqa    %xmm7, 48(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    (%esp), %xmm0                                 #
        movl      %edi, 64(%esp)                                #
        jmp       ..B5.35       # Prob 100%                     #
                                # LOE edx ecx ebx bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.12:                        # Preds ..B5.10
        movdqa    %xmm7, 48(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    (%esp), %xmm0                                 #
        movl      %edi, 64(%esp)                                #
        xorl      %eax, %eax                                    #
        xorl      %esi, %esi                                    #
        movl      %eax, 24(%esp)                                #
        movl      %esi, 28(%esp)                                #
        movl      $2, %esi                                      #
        movl      %eax, 80(%esp)                                #
        jmp       ..B5.17       # Prob 100%                     #
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.13:                        # Preds ..B5.9
        movdqa    %xmm7, 48(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    (%esp), %xmm0                                 #
        movl      %edi, 64(%esp)                                #
        xorl      %eax, %eax                                    #
        movl      $2, %esi                                      #
        movl      %eax, 28(%esp)                                #
        movl      %eax, 80(%esp)                                #
        jmp       ..B5.16       # Prob 100%                     #
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.14:                        # Preds ..B5.8
        movdqa    %xmm7, 48(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    (%esp), %xmm0                                 #
        movl      %edi, 64(%esp)                                #
        xorl      %eax, %eax                                    #
        movl      $2, %esi                                      #
        movl      %eax, 80(%esp)                                #
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.15:                        # Preds ..B5.14 ..B5.39
        movdqa    %xmm0, %xmm7                                  #2584.22
        pand      %xmm4, %xmm7                                  #2584.22
        pcmpeqw   %xmm5, %xmm7                                  #2585.22
        pmovmskb  %xmm7, %eax                                   #2586.22
        movl      %eax, 28(%esp)                                #2586.22
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.16:                        # Preds ..B5.13 ..B5.15
        movdqa    %xmm0, %xmm7                                  #2590.22
        pand      %xmm2, %xmm7                                  #2590.22
        pcmpeqw   %xmm3, %xmm7                                  #2591.22
        pmovmskb  %xmm7, %eax                                   #2592.22
        movl      %eax, 24(%esp)                                #2592.22
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.17:                        # Preds ..B5.12 ..B5.16
        movdqa    %xmm0, %xmm7                                  #2596.22
        pand      48(%esp), %xmm7                               #2596.22
        pcmpeqw   %xmm1, %xmm7                                  #2597.22
        pmovmskb  %xmm7, %eax                                   #2598.22
        movl      %ecx, %edi                                    #2603.17
        movl      80(%esp), %ecx                                #2606.29
        movl      %eax, 68(%esp)                                #2598.22
        andl      $-4, %edi                                     #2603.17
        orl       28(%esp), %ecx                                #2606.29
        orl       24(%esp), %ecx                                #2606.34
        orl       %eax, %ecx                                    #2606.39
        je        ..B5.26       # Prob 78%                      #2606.47
                                # LOE edx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.18:                        # Preds ..B5.17
        movl      %ebx, 16(%esp)                                #
        movl      80(%esp), %ebx                                #2624.30
        movl      %edx, 20(%esp)                                #
        movl      %ebx, %eax                                    #2624.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2624.30
        movswl    %ax, %edx                                     #2624.30
        movl      28(%esp), %eax                                #2625.30
        movl      %edx, (%esp)                                  #2624.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2625.30
        movswl    %ax, %edx                                     #2625.30
        movl      24(%esp), %eax                                #2626.30
        lea       1(,%edx,4), %edx                              #2633.40
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2626.30
        movswl    %ax, %ecx                                     #2626.30
        movl      68(%esp), %eax                                #2627.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2627.30
        movl      %edx, 4(%esp)                                 #2633.40
        movswl    %ax, %eax                                     #2627.30
        lea       2(,%ecx,4), %edx                              #2634.40
        movl      %edx, 8(%esp)                                 #2634.40
        lea       3(,%eax,4), %edx                              #2635.40
        testl     %ebx, %ebx                                    #2639.32
        movl      16(%esp), %ebx                                #2639.32
        movl      %edx, 12(%esp)                                #2635.40
        movl      20(%esp), %edx                                #2639.32
        je        ..B5.20       # Prob 50%                      #2639.32
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.19:                        # Preds ..B5.18
        movl      (%esp), %eax                                  #2639.43
        movl      72(%esp), %ecx                                #2639.49
        lea       (%edi,%eax,4), %eax                           #2639.43
        cmpl      %eax, %ecx                                    #2639.49
        jae       ..B5.44       # Prob 1%                       #2639.49
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.20:                        # Preds ..B5.19 ..B5.18
        movl      28(%esp), %eax                                #2640.32
        testl     %eax, %eax                                    #2640.32
        je        ..B5.22       # Prob 50%                      #2640.32
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.21:                        # Preds ..B5.20
        movl      4(%esp), %ecx                                 #2640.43
        movl      72(%esp), %eax                                #2640.49
        addl      %edi, %ecx                                    #2640.43
        cmpl      %ecx, %eax                                    #2640.49
        jae       ..B5.44       # Prob 1%                       #2640.49
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.22:                        # Preds ..B5.21 ..B5.20
        movl      24(%esp), %eax                                #2641.32
        testl     %eax, %eax                                    #2641.32
        je        ..B5.24       # Prob 50%                      #2641.32
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.23:                        # Preds ..B5.22
        movl      8(%esp), %ecx                                 #2641.43
        movl      72(%esp), %eax                                #2641.49
        addl      %edi, %ecx                                    #2641.43
        cmpl      %ecx, %eax                                    #2641.49
        jae       ..B5.44       # Prob 1%                       #2641.49
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.24:                        # Preds ..B5.23 ..B5.22
        movl      68(%esp), %eax                                #2642.32
        testl     %eax, %eax                                    #2642.32
        je        ..B5.26       # Prob 50%                      #2642.32
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.25:                        # Preds ..B5.24
        movl      12(%esp), %ecx                                #2642.43
        movl      72(%esp), %eax                                #2642.49
        addl      %edi, %ecx                                    #2642.43
        cmpl      %ecx, %eax                                    #2642.49
        jae       ..B5.44       # Prob 1%                       #2642.49
                                # LOE edx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.26:                        # Preds ..B5.25 ..B5.24 ..B5.17
        movl      72(%esp), %eax                                #2650.28
        lea       4(%edi), %ecx                                 #2647.17
        cmpl      %eax, %ecx                                    #2650.28
        ja        ..B5.41       # Prob 1%                       #2650.28
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.27:                        # Preds ..B5.26
        addl      $-1, %esi                                     #2654.25
        jne       ..B5.36       # Prob 50%                      #2654.39
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.28:                        # Preds ..B5.27
        movl      64(%esp), %eax                                #2675.25
        cmpl      %eax, %ebx                                    #2675.25
        jae       ..B5.41       # Prob 1%                       #2675.25
                                # LOE edx ebx edi xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.29:                        # Preds ..B5.28
        movl      72(%esp), %eax                                #2680.24
        lea       60(%edi), %ecx                                #2679.13
        cmpl      %eax, %ecx                                    #2680.24
        ja        ..B5.41       # Prob 1%                       #2680.24
                                # LOE edx ecx ebx xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.30:                        # Preds ..B5.29
        testb     $15, %bl                                      #2685.22
        jne       ..B5.32       # Prob 50%                      #2685.22
                                # LOE edx ecx ebx xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.31:                        # Preds ..B5.30
        movdqa    (%ebx), %xmm0                                 #2685.22
        jmp       ..B5.33       # Prob 100%                     #2685.22
                                # LOE edx ecx ebx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.32:                        # Preds ..B5.30
        movdqu    (%ebx), %xmm0                                 #2685.22
                                # LOE edx ecx ebx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.33:                        # Preds ..B5.31 ..B5.32
        movl      64(%esp), %eax                                #2766.24
        addl      $16, %ebx                                     #2763.13
        movl      %ebx, 76(%esp)                                #2763.13
        cmpl      %eax, %ebx                                    #2766.24
        jae       ..B5.35       # Prob 12%                      #2766.24
                                # LOE edx ecx ebx bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.34:                        # Preds ..B5.33
        movzbl    -1(%ebx), %edx                                #2767.32
        shll      $8, %edx                                      #2767.46
                                # LOE edx ecx ebx bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.35:                        # Preds ..B5.33 ..B5.11 ..B5.34
        movl      $2, %esi                                      #2568.13
        jmp       ..B5.39       # Prob 100%                     #2568.13
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.36:                        # Preds ..B5.27
        movl      76(%esp), %eax                                #2662.26
        movl      64(%esp), %edi                                #2662.26
        psrldq    $1, %xmm0                                     #2659.26
        cmpl      %edi, %eax                                    #2662.26
        jae       ..B5.38       # Prob 19%                      #2662.26
                                # LOE eax edx ecx ebx esi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.37:                        # Preds ..B5.36
        movzbl    (%eax), %edi                                  #2665.37
        sarl      $8, %edx                                      #2664.21
        shll      $8, %edi                                      #2665.48
        orl       %edi, %edx                                    #2665.21
        pinsrw    $7, %edx, %xmm0                               #2666.30
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.38:                        # Preds ..B5.37 ..B5.36
        addl      $1, 76(%esp)                                  #2670.20
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.39:                        # Preds ..B5.35 ..B5.38 ..B5.43
        movdqa    %xmm0, %xmm7                                  #2578.22
        pand      %xmm6, %xmm7                                  #2578.22
        pcmpeqw   32(%esp), %xmm7                               #2579.22
        pmovmskb  %xmm7, %eax                                   #2580.22
        movl      %eax, 80(%esp)                                #2580.22
        jmp       ..B5.15       # Prob 100%                     #2580.22
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.41:                        # Preds ..B5.26 ..B5.28 ..B5.29 # Infreq
        xorl      %eax, %eax                                    #2776.12
        addl      $84, %esp                                     #2776.12
        popl      %ebx                                          #2776.12
        popl      %esi                                          #2776.12
        popl      %edi                                          #2776.12
        movl      %ebp, %esp                                    #2776.12
        popl      %ebp                                          #2776.12
        ret                                                     #2776.12
                                # LOE
..B5.43:                        # Preds ..B5.7                  # Infreq
        movdqa    %xmm7, 48(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    (%esp), %xmm0                                 #
        movl      %edi, 64(%esp)                                #
        movl      $2, %esi                                      #
        jmp       ..B5.39       # Prob 100%                     #
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B5.44:                        # Preds ..B5.21 ..B5.19 ..B5.25 ..B5.23 # Infreq
        movl      $1, %eax                                      #2641.63
        addl      $84, %esp                                     #2641.63
        popl      %ebx                                          #2641.63
        popl      %esi                                          #2641.63
        popl      %edi                                          #2641.63
        movl      %ebp, %esp                                    #2641.63
        popl      %ebp                                          #2641.63
        ret                                                     #2641.63
        .align    2,0x90
                                # LOE
# mark_end;
	.type	eval_2na_16,@function
	.size	eval_2na_16,.-eval_2na_16
	.data
# -- End  eval_2na_16
	.text
# -- Begin  eval_2na_32
# mark_begin;
       .align    2,0x90
eval_2na_32:
# parameter 1: %eax
# parameter 2: %edx
# parameter 3: %ecx
# parameter 4: 20 + %ebp
..B6.1:                         # Preds ..B6.0
        pushl     %ebp                                          #2782.1
        movl      %esp, %ebp                                    #2782.1
        andl      $-16, %esp                                    #2782.1
        pushl     %edi                                          #2782.1
        pushl     %esi                                          #2782.1
        pushl     %ebx                                          #2782.1
        subl      $84, %esp                                     #2782.1
        movl      %ecx, %ebx                                    #2822.49
        shrl      $2, %ebx                                      #2822.49
        movl      20(%ebp), %edi                                #2780.5
        movl      %ebx, 80(%esp)                                #2822.49
        lea       (%ecx,%edi), %esi                             #2819.5
        lea       (%edx,%ebx), %ebx                             #2822.30
        lea       3(%ecx,%edi), %edi                            #2828.50
        subl      4(%eax), %esi                                 #2825.12
        movl      %esi, 72(%esp)                                #2825.12
        movl      80(%esp), %esi                                #2831.14
        shrl      $2, %edi                                      #2828.57
        addl      %edx, %edi                                    #2828.30
        testb     $15, %bl                                      #2831.14
        jne       ..B6.3        # Prob 50%                      #2831.14
                                # LOE eax edx ecx ebx esi edi
..B6.2:                         # Preds ..B6.1
        movdqa    (%ebx), %xmm0                                 #2831.14
        movdqa    %xmm0, (%esp)                                 #2831.14
        jmp       ..B6.4        # Prob 100%                     #2831.14
                                # LOE eax edx ecx esi edi
..B6.3:                         # Preds ..B6.1
        movdqu    (%ebx), %xmm0                                 #2831.14
        movdqa    %xmm0, (%esp)                                 #2831.14
                                # LOE eax edx ecx esi edi
..B6.4:                         # Preds ..B6.2 ..B6.3
        lea       16(%edx,%esi), %ebx                           #2832.5
        movl      %ebx, 76(%esp)                                #2832.5
        cmpl      %edi, %ebx                                    #2837.16
        jae       ..B6.6        # Prob 12%                      #2837.16
                                # LOE eax ecx ebx edi bl bh
..B6.5:                         # Preds ..B6.4
        movzbl    -1(%ebx), %edx                                #2838.24
        shll      $8, %edx                                      #2838.38
        jmp       ..B6.7        # Prob 100%                     #2838.38
                                # LOE eax edx ecx ebx edi bl bh
..B6.6:                         # Preds ..B6.4
        xorl      %edx, %edx                                    #
                                # LOE eax edx ecx ebx edi bl bh
..B6.7:                         # Preds ..B6.5 ..B6.6
        movdqa    16(%eax), %xmm0                               #2842.5
        movdqa    32(%eax), %xmm6                               #2842.5
        movdqa    48(%eax), %xmm5                               #2842.5
        movdqa    64(%eax), %xmm4                               #2842.5
        movdqa    80(%eax), %xmm3                               #2842.5
        movdqa    96(%eax), %xmm2                               #2842.5
        movdqa    112(%eax), %xmm1                              #2842.5
        movdqa    128(%eax), %xmm7                              #2842.5
        movl      %ecx, %eax                                    #2854.20
        andl      $3, %eax                                      #2854.20
        je        ..B6.43       # Prob 20%                      #2854.20
                                # LOE eax edx ecx ebx edi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B6.8:                         # Preds ..B6.7
        cmpl      $1, %eax                                      #2854.20
        je        ..B6.14       # Prob 25%                      #2854.20
                                # LOE eax edx ecx ebx edi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B6.9:                         # Preds ..B6.8
        cmpl      $2, %eax                                      #2854.20
        je        ..B6.13       # Prob 33%                      #2854.20
                                # LOE eax edx ecx ebx edi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B6.10:                        # Preds ..B6.9
        cmpl      $3, %eax                                      #2854.20
        je        ..B6.12       # Prob 50%                      #2854.20
                                # LOE edx ecx ebx edi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B6.11:                        # Preds ..B6.10
        movdqa    %xmm7, 48(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    (%esp), %xmm0                                 #
        movl      %edi, 64(%esp)                                #
        jmp       ..B6.35       # Prob 100%                     #
                                # LOE edx ecx ebx bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.12:                        # Preds ..B6.10
        movdqa    %xmm7, 48(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    (%esp), %xmm0                                 #
        movl      %edi, 64(%esp)                                #
        xorl      %eax, %eax                                    #
        xorl      %esi, %esi                                    #
        movl      %eax, 24(%esp)                                #
        movl      %esi, 28(%esp)                                #
        movl      $4, %esi                                      #
        movl      %eax, 80(%esp)                                #
        jmp       ..B6.17       # Prob 100%                     #
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.13:                        # Preds ..B6.9
        movdqa    %xmm7, 48(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    (%esp), %xmm0                                 #
        movl      %edi, 64(%esp)                                #
        xorl      %eax, %eax                                    #
        movl      $4, %esi                                      #
        movl      %eax, 28(%esp)                                #
        movl      %eax, 80(%esp)                                #
        jmp       ..B6.16       # Prob 100%                     #
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.14:                        # Preds ..B6.8
        movdqa    %xmm7, 48(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    (%esp), %xmm0                                 #
        movl      %edi, 64(%esp)                                #
        xorl      %eax, %eax                                    #
        movl      $4, %esi                                      #
        movl      %eax, 80(%esp)                                #
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.15:                        # Preds ..B6.14 ..B6.39
        movdqa    %xmm0, %xmm7                                  #2877.22
        pand      %xmm4, %xmm7                                  #2877.22
        pcmpeqd   %xmm5, %xmm7                                  #2878.22
        pmovmskb  %xmm7, %eax                                   #2879.22
        movl      %eax, 28(%esp)                                #2879.22
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.16:                        # Preds ..B6.13 ..B6.15
        movdqa    %xmm0, %xmm7                                  #2883.22
        pand      %xmm2, %xmm7                                  #2883.22
        pcmpeqd   %xmm3, %xmm7                                  #2884.22
        pmovmskb  %xmm7, %eax                                   #2885.22
        movl      %eax, 24(%esp)                                #2885.22
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.17:                        # Preds ..B6.12 ..B6.16
        movdqa    %xmm0, %xmm7                                  #2889.22
        pand      48(%esp), %xmm7                               #2889.22
        pcmpeqd   %xmm1, %xmm7                                  #2890.22
        pmovmskb  %xmm7, %eax                                   #2891.22
        movl      %ecx, %edi                                    #2896.17
        movl      80(%esp), %ecx                                #2899.29
        movl      %eax, 68(%esp)                                #2891.22
        andl      $-4, %edi                                     #2896.17
        orl       28(%esp), %ecx                                #2899.29
        orl       24(%esp), %ecx                                #2899.34
        orl       %eax, %ecx                                    #2899.39
        je        ..B6.26       # Prob 78%                      #2899.47
                                # LOE edx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.18:                        # Preds ..B6.17
        movl      %ebx, 16(%esp)                                #
        movl      80(%esp), %ebx                                #2917.30
        movl      %edx, 20(%esp)                                #
        movl      %ebx, %eax                                    #2917.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2917.30
        movswl    %ax, %edx                                     #2917.30
        movl      28(%esp), %eax                                #2918.30
        movl      %edx, (%esp)                                  #2917.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2918.30
        movswl    %ax, %edx                                     #2918.30
        movl      24(%esp), %eax                                #2919.30
        lea       1(,%edx,4), %edx                              #2926.40
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2919.30
        movswl    %ax, %ecx                                     #2919.30
        movl      68(%esp), %eax                                #2920.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #2920.30
        movl      %edx, 4(%esp)                                 #2926.40
        movswl    %ax, %eax                                     #2920.30
        lea       2(,%ecx,4), %edx                              #2927.40
        movl      %edx, 8(%esp)                                 #2927.40
        lea       3(,%eax,4), %edx                              #2928.40
        testl     %ebx, %ebx                                    #2932.32
        movl      16(%esp), %ebx                                #2932.32
        movl      %edx, 12(%esp)                                #2928.40
        movl      20(%esp), %edx                                #2932.32
        je        ..B6.20       # Prob 50%                      #2932.32
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.19:                        # Preds ..B6.18
        movl      (%esp), %eax                                  #2932.43
        movl      72(%esp), %ecx                                #2932.49
        lea       (%edi,%eax,4), %eax                           #2932.43
        cmpl      %eax, %ecx                                    #2932.49
        jae       ..B6.44       # Prob 1%                       #2932.49
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.20:                        # Preds ..B6.19 ..B6.18
        movl      28(%esp), %eax                                #2933.32
        testl     %eax, %eax                                    #2933.32
        je        ..B6.22       # Prob 50%                      #2933.32
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.21:                        # Preds ..B6.20
        movl      4(%esp), %ecx                                 #2933.43
        movl      72(%esp), %eax                                #2933.49
        addl      %edi, %ecx                                    #2933.43
        cmpl      %ecx, %eax                                    #2933.49
        jae       ..B6.44       # Prob 1%                       #2933.49
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.22:                        # Preds ..B6.21 ..B6.20
        movl      24(%esp), %eax                                #2934.32
        testl     %eax, %eax                                    #2934.32
        je        ..B6.24       # Prob 50%                      #2934.32
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.23:                        # Preds ..B6.22
        movl      8(%esp), %ecx                                 #2934.43
        movl      72(%esp), %eax                                #2934.49
        addl      %edi, %ecx                                    #2934.43
        cmpl      %ecx, %eax                                    #2934.49
        jae       ..B6.44       # Prob 1%                       #2934.49
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.24:                        # Preds ..B6.23 ..B6.22
        movl      68(%esp), %eax                                #2935.32
        testl     %eax, %eax                                    #2935.32
        je        ..B6.26       # Prob 50%                      #2935.32
                                # LOE edx ebx esi edi dl bl dh bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.25:                        # Preds ..B6.24
        movl      12(%esp), %ecx                                #2935.43
        movl      72(%esp), %eax                                #2935.49
        addl      %edi, %ecx                                    #2935.43
        cmpl      %ecx, %eax                                    #2935.49
        jae       ..B6.44       # Prob 1%                       #2935.49
                                # LOE edx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.26:                        # Preds ..B6.25 ..B6.24 ..B6.17
        movl      72(%esp), %eax                                #2943.28
        lea       4(%edi), %ecx                                 #2940.17
        cmpl      %eax, %ecx                                    #2943.28
        ja        ..B6.41       # Prob 1%                       #2943.28
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.27:                        # Preds ..B6.26
        addl      $-1, %esi                                     #2947.25
        jne       ..B6.36       # Prob 50%                      #2947.39
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.28:                        # Preds ..B6.27
        movl      64(%esp), %eax                                #2968.25
        cmpl      %eax, %ebx                                    #2968.25
        jae       ..B6.41       # Prob 1%                       #2968.25
                                # LOE edx ebx edi xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.29:                        # Preds ..B6.28
        movl      72(%esp), %eax                                #2973.24
        lea       52(%edi), %ecx                                #2972.13
        cmpl      %eax, %ecx                                    #2973.24
        ja        ..B6.41       # Prob 1%                       #2973.24
                                # LOE edx ecx ebx xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.30:                        # Preds ..B6.29
        testb     $15, %bl                                      #2978.22
        jne       ..B6.32       # Prob 50%                      #2978.22
                                # LOE edx ecx ebx xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.31:                        # Preds ..B6.30
        movdqa    (%ebx), %xmm0                                 #2978.22
        jmp       ..B6.33       # Prob 100%                     #2978.22
                                # LOE edx ecx ebx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.32:                        # Preds ..B6.30
        movdqu    (%ebx), %xmm0                                 #2978.22
                                # LOE edx ecx ebx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.33:                        # Preds ..B6.31 ..B6.32
        movl      64(%esp), %eax                                #3059.24
        addl      $16, %ebx                                     #3056.13
        movl      %ebx, 76(%esp)                                #3056.13
        cmpl      %eax, %ebx                                    #3059.24
        jae       ..B6.35       # Prob 12%                      #3059.24
                                # LOE edx ecx ebx bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.34:                        # Preds ..B6.33
        movzbl    -1(%ebx), %edx                                #3060.32
        shll      $8, %edx                                      #3060.46
                                # LOE edx ecx ebx bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.35:                        # Preds ..B6.33 ..B6.11 ..B6.34
        movl      $4, %esi                                      #2861.13
        jmp       ..B6.39       # Prob 100%                     #2861.13
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.36:                        # Preds ..B6.27
        movl      76(%esp), %eax                                #2955.26
        movl      64(%esp), %edi                                #2955.26
        psrldq    $1, %xmm0                                     #2952.26
        cmpl      %edi, %eax                                    #2955.26
        jae       ..B6.38       # Prob 19%                      #2955.26
                                # LOE eax edx ecx ebx esi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.37:                        # Preds ..B6.36
        movzbl    (%eax), %edi                                  #2958.37
        sarl      $8, %edx                                      #2957.21
        shll      $8, %edi                                      #2958.48
        orl       %edi, %edx                                    #2958.21
        pinsrw    $7, %edx, %xmm0                               #2959.30
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.38:                        # Preds ..B6.37 ..B6.36
        addl      $1, 76(%esp)                                  #2963.20
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.39:                        # Preds ..B6.35 ..B6.38 ..B6.43
        movdqa    %xmm0, %xmm7                                  #2871.22
        pand      %xmm6, %xmm7                                  #2871.22
        pcmpeqd   32(%esp), %xmm7                               #2872.22
        pmovmskb  %xmm7, %eax                                   #2873.22
        movl      %eax, 80(%esp)                                #2873.22
        jmp       ..B6.15       # Prob 100%                     #2873.22
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.41:                        # Preds ..B6.26 ..B6.28 ..B6.29 # Infreq
        xorl      %eax, %eax                                    #3069.12
        addl      $84, %esp                                     #3069.12
        popl      %ebx                                          #3069.12
        popl      %esi                                          #3069.12
        popl      %edi                                          #3069.12
        movl      %ebp, %esp                                    #3069.12
        popl      %ebp                                          #3069.12
        ret                                                     #3069.12
                                # LOE
..B6.43:                        # Preds ..B6.7                  # Infreq
        movdqa    %xmm7, 48(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    (%esp), %xmm0                                 #
        movl      %edi, 64(%esp)                                #
        movl      $4, %esi                                      #
        jmp       ..B6.39       # Prob 100%                     #
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B6.44:                        # Preds ..B6.21 ..B6.19 ..B6.25 ..B6.23 # Infreq
        movl      $1, %eax                                      #2934.63
        addl      $84, %esp                                     #2934.63
        popl      %ebx                                          #2934.63
        popl      %esi                                          #2934.63
        popl      %edi                                          #2934.63
        movl      %ebp, %esp                                    #2934.63
        popl      %ebp                                          #2934.63
        ret                                                     #2934.63
        .align    2,0x90
                                # LOE
# mark_end;
	.type	eval_2na_32,@function
	.size	eval_2na_32,.-eval_2na_32
	.data
# -- End  eval_2na_32
	.text
# -- Begin  eval_2na_128
# mark_begin;
       .align    2,0x90
eval_2na_128:
# parameter 1: %eax
# parameter 2: %edx
# parameter 3: %ecx
# parameter 4: 20 + %ebp
..B7.1:                         # Preds ..B7.0
        pushl     %ebp                                          #3369.1
        movl      %esp, %ebp                                    #3369.1
        andl      $-16, %esp                                    #3369.1
        pushl     %edi                                          #3369.1
        pushl     %esi                                          #3369.1
        pushl     %ebx                                          #3369.1
        subl      $84, %esp                                     #3369.1
        movl      %ecx, %esi                                    #3409.49
        shrl      $2, %esi                                      #3409.49
        movl      20(%ebp), %ebx                                #3367.5
        movl      %esi, 68(%esp)                                #3409.49
        lea       (%ecx,%ebx), %edi                             #3406.5
        lea       (%edx,%esi), %esi                             #3409.30
        lea       3(%ecx,%ebx), %ebx                            #3415.50
        subl      4(%eax), %edi                                 #3412.12
        movl      %edi, 60(%esp)                                #3412.12
        movl      68(%esp), %edi                                #3418.14
        shrl      $2, %ebx                                      #3415.57
        addl      %edx, %ebx                                    #3415.30
        testl     $15, %esi                                     #3418.14
        jne       ..B7.3        # Prob 50%                      #3418.14
                                # LOE eax edx ecx ebx esi edi
..B7.2:                         # Preds ..B7.1
        movdqa    (%esi), %xmm0                                 #3418.14
        movdqa    %xmm0, 32(%esp)                               #3418.14
        jmp       ..B7.4        # Prob 100%                     #3418.14
                                # LOE eax edx ecx ebx edi
..B7.3:                         # Preds ..B7.1
        movdqu    (%esi), %xmm0                                 #3418.14
        movdqa    %xmm0, 32(%esp)                               #3418.14
                                # LOE eax edx ecx ebx edi
..B7.4:                         # Preds ..B7.2 ..B7.3
        lea       16(%edx,%edi), %edx                           #3419.5
        movl      %edx, 64(%esp)                                #3419.5
        cmpl      %ebx, %edx                                    #3424.16
        jae       ..B7.6        # Prob 12%                      #3424.16
                                # LOE eax edx ecx ebx dl dh
..B7.5:                         # Preds ..B7.4
        movzbl    -1(%edx), %esi                                #3425.24
        shll      $8, %esi                                      #3425.38
        jmp       ..B7.7        # Prob 100%                     #3425.38
                                # LOE eax edx ecx ebx esi dl dh
..B7.6:                         # Preds ..B7.4
        xorl      %esi, %esi                                    #
                                # LOE eax edx ecx ebx esi dl dh
..B7.7:                         # Preds ..B7.5 ..B7.6
        movl      60(%esp), %edi                                #3434.20
        movdqa    16(%eax), %xmm6                               #3429.5
        movdqa    32(%eax), %xmm5                               #3429.5
        movdqa    48(%eax), %xmm4                               #3429.5
        movdqa    64(%eax), %xmm3                               #3429.5
        movdqa    80(%eax), %xmm2                               #3429.5
        movdqa    96(%eax), %xmm1                               #3429.5
        movdqa    112(%eax), %xmm0                              #3429.5
        movdqa    128(%eax), %xmm7                              #3429.5
        subl      %ecx, %edi                                    #3434.20
        addl      $7, %edi                                      #3434.33
        shrl      $2, %edi                                      #3434.40
        movl      %ecx, %eax                                    #3441.20
        andl      $3, %eax                                      #3441.20
        je        ..B7.42       # Prob 20%                      #3441.20
                                # LOE eax edx ecx ebx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B7.8:                         # Preds ..B7.7
        cmpl      $1, %eax                                      #3441.20
        je        ..B7.14       # Prob 25%                      #3441.20
                                # LOE eax edx ecx ebx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B7.9:                         # Preds ..B7.8
        cmpl      $2, %eax                                      #3441.20
        je        ..B7.13       # Prob 33%                      #3441.20
                                # LOE eax edx ecx ebx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B7.10:                        # Preds ..B7.9
        cmpl      $3, %eax                                      #3441.20
        je        ..B7.12       # Prob 50%                      #3441.20
                                # LOE edx ecx ebx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B7.11:                        # Preds ..B7.10
        movdqa    %xmm7, (%esp)                                 #
        movdqa    %xmm0, 16(%esp)                               #
        movdqa    32(%esp), %xmm0                               #
        movl      %esi, 68(%esp)                                #
        movl      %ebx, 56(%esp)                                #
        jmp       ..B7.26       # Prob 100%                     #
                                # LOE edx ecx dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.12:                        # Preds ..B7.10
        movdqa    %xmm7, (%esp)                                 #
        movdqa    %xmm0, 16(%esp)                               #
        movdqa    32(%esp), %xmm0                               #
        movl      %esi, 68(%esp)                                #
        movl      %ebx, 56(%esp)                                #
        xorl      %eax, %eax                                    #
        movl      %eax, 48(%esp)                                #
        movl      %eax, 52(%esp)                                #
        jmp       ..B7.17       # Prob 100%                     #
                                # LOE eax edx ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.13:                        # Preds ..B7.9
        movdqa    %xmm7, (%esp)                                 #
        movdqa    %xmm0, 16(%esp)                               #
        movdqa    32(%esp), %xmm0                               #
        movl      %esi, 68(%esp)                                #
        movl      %ebx, 56(%esp)                                #
        xorl      %eax, %eax                                    #
        movl      %eax, 52(%esp)                                #
        jmp       ..B7.16       # Prob 100%                     #
                                # LOE eax edx ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.14:                        # Preds ..B7.8
        movdqa    %xmm7, (%esp)                                 #
        movdqa    %xmm0, 16(%esp)                               #
        movdqa    32(%esp), %xmm0                               #
        movl      %esi, 68(%esp)                                #
        movl      %ebx, 56(%esp)                                #
        xorl      %eax, %eax                                    #
                                # LOE eax edx ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.15:                        # Preds ..B7.14 ..B7.30
        movdqa    %xmm0, %xmm7                                  #3464.22
        pand      %xmm3, %xmm7                                  #3464.22
        pcmpeqd   %xmm4, %xmm7                                  #3465.22
        pmovmskb  %xmm7, %ebx                                   #3466.22
        addl      $1, %ebx                                      #3467.17
        shrl      $16, %ebx                                     #3467.17
        negl      %ebx                                          #3467.17
        movl      %ebx, 52(%esp)                                #3467.17
                                # LOE eax edx ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.16:                        # Preds ..B7.13 ..B7.15
        movdqa    %xmm0, %xmm7                                  #3470.22
        pand      %xmm1, %xmm7                                  #3470.22
        pcmpeqd   %xmm2, %xmm7                                  #3471.22
        pmovmskb  %xmm7, %ebx                                   #3472.22
        addl      $1, %ebx                                      #3473.17
        shrl      $16, %ebx                                     #3473.17
        negl      %ebx                                          #3473.17
        movl      %ebx, 48(%esp)                                #3473.17
                                # LOE eax edx ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.17:                        # Preds ..B7.12 ..B7.16
        movdqa    %xmm0, %xmm7                                  #3476.22
        pand      (%esp), %xmm7                                 #3476.22
        pcmpeqd   16(%esp), %xmm7                               #3477.22
        pmovmskb  %xmm7, %ebx                                   #3478.22
        andl      $-4, %ecx                                     #3483.17
        movl      %eax, %esi                                    #3486.29
        orl       52(%esp), %esi                                #3486.29
        addl      $1, %ebx                                      #3479.17
        shrl      $16, %ebx                                     #3479.17
        orl       48(%esp), %esi                                #3486.34
        negl      %ebx                                          #3479.17
        orl       %ebx, %esi                                    #3486.39
        jne       ..B7.31       # Prob 1%                       #3486.47
                                # LOE eax edx ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.18:                        # Preds ..B7.17
        movl      60(%esp), %eax                                #3530.28
        addl      $4, %ecx                                      #3527.17
        cmpl      %eax, %ecx                                    #3530.28
        ja        ..B7.38       # Prob 1%                       #3530.28
                                # LOE edx ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.19:                        # Preds ..B7.18
        addl      $-1, %edi                                     #3534.25
        jne       ..B7.27       # Prob 50%                      #3534.39
                                # LOE edx ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.20:                        # Preds ..B7.19
        movl      56(%esp), %eax                                #3555.25
        cmpl      %eax, %edx                                    #3555.25
        jae       ..B7.38       # Prob 1%                       #3555.25
                                # LOE eax edx ecx al ah xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.21:                        # Preds ..B7.20
        testb     $15, %dl                                      #3565.22
        jne       ..B7.23       # Prob 50%                      #3565.22
                                # LOE eax edx ecx al ah xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.22:                        # Preds ..B7.21
        movdqa    (%edx), %xmm0                                 #3565.22
        jmp       ..B7.24       # Prob 100%                     #3565.22
                                # LOE eax edx ecx al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.23:                        # Preds ..B7.21
        movdqu    (%edx), %xmm0                                 #3565.22
                                # LOE eax edx ecx al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.24:                        # Preds ..B7.22 ..B7.23
        addl      $16, %edx                                     #3643.13
        movl      %edx, 64(%esp)                                #3643.13
        cmpl      %eax, %edx                                    #3646.24
        jae       ..B7.26       # Prob 12%                      #3646.24
                                # LOE edx ecx dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.25:                        # Preds ..B7.24
        movzbl    -1(%edx), %eax                                #3647.32
        shll      $8, %eax                                      #3647.46
        movl      %eax, 68(%esp)                                #3647.46
                                # LOE edx ecx dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.26:                        # Preds ..B7.24 ..B7.11 ..B7.25
        movl      $16, %edi                                     #3448.13
        jmp       ..B7.30       # Prob 100%                     #3448.13
                                # LOE edx ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.27:                        # Preds ..B7.19
        movl      64(%esp), %eax                                #3542.26
        movl      56(%esp), %ebx                                #3542.26
        psrldq    $1, %xmm0                                     #3539.26
        cmpl      %ebx, %eax                                    #3542.26
        jae       ..B7.29       # Prob 19%                      #3542.26
                                # LOE eax edx ecx edi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.28:                        # Preds ..B7.27
        movl      68(%esp), %esi                                #3544.21
        movzbl    (%eax), %ebx                                  #3545.37
        sarl      $8, %esi                                      #3544.21
        shll      $8, %ebx                                      #3545.48
        orl       %ebx, %esi                                    #3545.21
        pinsrw    $7, %esi, %xmm0                               #3546.30
        movl      %esi, 68(%esp)                                #3545.21
                                # LOE edx ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.29:                        # Preds ..B7.28 ..B7.27
        addl      $1, 64(%esp)                                  #3550.20
                                # LOE edx ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.30:                        # Preds ..B7.26 ..B7.29 ..B7.42
        movdqa    %xmm0, %xmm7                                  #3458.22
        pand      %xmm5, %xmm7                                  #3458.22
        pcmpeqd   %xmm6, %xmm7                                  #3459.22
        pmovmskb  %xmm7, %eax                                   #3460.22
        addl      $1, %eax                                      #3461.17
        shrl      $16, %eax                                     #3461.17
        negl      %eax                                          #3461.17
        jmp       ..B7.15       # Prob 100%                     #3461.17
                                # LOE eax edx ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B7.31:                        # Preds ..B7.17                 # Infreq
        movl      60(%esp), %edx                                #3489.30
        subl      %ecx, %edx                                    #3489.30
        movl      %edx, 60(%esp)                                #3489.30
        cmpl      $2, %edx                                      #3489.37
        je        ..B7.35       # Prob 25%                      #3489.37
                                # LOE eax edx dl dh
..B7.32:                        # Preds ..B7.31                 # Infreq
        cmpl      $1, %edx                                      #3489.37
        je        ..B7.36       # Prob 33%                      #3489.37
                                # LOE eax edx dl dh
..B7.33:                        # Preds ..B7.32                 # Infreq
        testl     %edx, %edx                                    #3489.37
        je        ..B7.37       # Prob 50%                      #3489.37
                                # LOE eax
..B7.34:                        # Preds ..B7.35 ..B7.36 ..B7.37 ..B7.33 # Infreq
        movl      $1, %eax                                      #3492.32
        addl      $84, %esp                                     #3492.32
        popl      %ebx                                          #3492.32
        popl      %esi                                          #3492.32
        popl      %edi                                          #3492.32
        movl      %ebp, %esp                                    #3492.32
        popl      %ebp                                          #3492.32
        ret                                                     #3492.32
                                # LOE
..B7.35:                        # Preds ..B7.31                 # Infreq
        movl      48(%esp), %edx                                #3494.36
        testl     %edx, %edx                                    #3494.36
        jne       ..B7.34       # Prob 28%                      #3494.36
                                # LOE eax
..B7.36:                        # Preds ..B7.35 ..B7.32         # Infreq
        movl      52(%esp), %edx                                #3496.36
        testl     %edx, %edx                                    #3496.36
        jne       ..B7.34       # Prob 28%                      #3496.36
                                # LOE eax
..B7.37:                        # Preds ..B7.33 ..B7.36         # Infreq
        testl     %eax, %eax                                    #3498.36
        jne       ..B7.34       # Prob 28%                      #3498.36
                                # LOE
..B7.38:                        # Preds ..B7.20 ..B7.18 ..B7.37 # Infreq
        xorl      %eax, %eax                                    #3500.28
        addl      $84, %esp                                     #3500.28
        popl      %ebx                                          #3500.28
        popl      %esi                                          #3500.28
        popl      %edi                                          #3500.28
        movl      %ebp, %esp                                    #3500.28
        popl      %ebp                                          #3500.28
        ret                                                     #3500.28
                                # LOE
..B7.42:                        # Preds ..B7.7                  # Infreq
        movdqa    %xmm7, (%esp)                                 #
        movdqa    %xmm0, 16(%esp)                               #
        movdqa    32(%esp), %xmm0                               #
        movl      %esi, 68(%esp)                                #
        movl      %ebx, 56(%esp)                                #
        jmp       ..B7.30       # Prob 100%                     #
        .align    2,0x90
                                # LOE edx ecx edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
# mark_end;
	.type	eval_2na_128,@function
	.size	eval_2na_128,.-eval_2na_128
	.data
# -- End  eval_2na_128
	.text
# -- Begin  eval_4na_16
# mark_begin;
       .align    2,0x90
eval_4na_16:
# parameter 1: %eax
# parameter 2: %edx
# parameter 3: %ecx
# parameter 4: 20 + %ebp
..B8.1:                         # Preds ..B8.0
        pushl     %ebp                                          #4057.1
        movl      %esp, %ebp                                    #4057.1
        andl      $-16, %esp                                    #4057.1
        pushl     %edi                                          #4057.1
        pushl     %esi                                          #4057.1
        pushl     %ebx                                          #4057.1
        subl      $116, %esp                                    #4057.1
        movdqa    16(%eax), %xmm5                               #4109.5
        movdqa    32(%eax), %xmm4                               #4109.5
        movl      20(%ebp), %ebx                                #4055.5
        movdqa    48(%eax), %xmm6                               #4109.5
        movdqa    64(%eax), %xmm3                               #4109.5
        movdqa    80(%eax), %xmm2                               #4109.5
        movdqa    96(%eax), %xmm0                               #4109.5
        movdqa    128(%eax), %xmm7                              #4109.5
        movdqa    %xmm0, 80(%esp)                               #4109.5
        movdqa    112(%eax), %xmm0                              #4109.5
        movl      %ecx, %edi                                    #4093.49
        call      ..L3          # Prob 100%                     #4057.1
..L3:                                                           #
        popl      %esi                                          #4057.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L3], %esi       #4057.1
        shrl      $2, %edi                                      #4093.49
        movl      %esi, 16(%esp)                                #4057.1
        lea       (%ecx,%ebx), %esi                             #4090.5
        subl      4(%eax), %esi                                 #4096.12
        lea       3(%ecx,%ebx), %ebx                            #4099.50
        movl      %esi, 100(%esp)                               #4096.12
        movzbl    (%edi,%edx), %esi                             #4102.14
        shrl      $2, %ebx                                      #4099.57
        addl      %edx, %ebx                                    #4099.30
        movl      %ecx, %eax                                    #4123.20
        movl      %ebx, 104(%esp)                               #4099.30
        movl      16(%esp), %ebx                                #4102.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4102.14
        movw      %si, (%esp)                                   #4102.14
        movzbl    1(%edi,%edx), %esi                            #4102.14
        andl      $3, %eax                                      #4123.20
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4102.14
        movw      %si, 2(%esp)                                  #4102.14
        movzbl    2(%edi,%edx), %esi                            #4102.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4102.14
        movw      %si, 4(%esp)                                  #4102.14
        movzbl    3(%edi,%edx), %esi                            #4102.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4102.14
        movw      %si, 6(%esp)                                  #4102.14
        movzbl    4(%edi,%edx), %esi                            #4102.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4102.14
        movw      %si, 8(%esp)                                  #4102.14
        movzbl    5(%edi,%edx), %esi                            #4102.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4102.14
        movw      %si, 10(%esp)                                 #4102.14
        movzbl    6(%edi,%edx), %esi                            #4102.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4102.14
        movw      %si, 12(%esp)                                 #4102.14
        movzbl    7(%edi,%edx), %esi                            #4102.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4102.14
        movw      %si, 14(%esp)                                 #4102.14
        movdqu    (%esp), %xmm1                                 #4102.14
        lea       8(%edx,%edi), %esi                            #4103.5
        je        ..B8.5        # Prob 20%                      #4123.20
                                # LOE eax ecx ebx esi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B8.2:                         # Preds ..B8.1
        cmpl      $1, %eax                                      #4123.20
        je        ..B8.8        # Prob 25%                      #4123.20
                                # LOE eax ecx ebx esi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B8.3:                         # Preds ..B8.2
        cmpl      $2, %eax                                      #4123.20
        je        ..B8.7        # Prob 33%                      #4123.20
                                # LOE eax ecx ebx esi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B8.4:                         # Preds ..B8.3
        cmpl      $3, %eax                                      #4123.20
        je        ..B8.6        # Prob 50%                      #4123.20
                                # LOE ecx ebx esi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B8.5:                         # Preds ..B8.1 ..B8.4
        movdqa    %xmm7, 64(%esp)                               #
        movdqa    %xmm6, 48(%esp)                               #
        jmp       ..B8.24       # Prob 100%                     #
                                # LOE ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B8.6:                         # Preds ..B8.4
        movdqa    %xmm7, 64(%esp)                               #
        movdqa    %xmm6, 48(%esp)                               #
        xorl      %edi, %edi                                    #
        movl      %edi, 44(%esp)                                #
        xorl      %edx, %edx                                    #
        movl      %edi, 96(%esp)                                #
        jmp       ..B8.11       # Prob 100%                     #
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B8.7:                         # Preds ..B8.3
        movdqa    %xmm7, 64(%esp)                               #
        movdqa    %xmm6, 48(%esp)                               #
        xorl      %eax, %eax                                    #
        movl      %eax, 96(%esp)                                #
        xorl      %edx, %edx                                    #
        jmp       ..B8.10       # Prob 100%                     #
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B8.8:                         # Preds ..B8.2
        movdqa    %xmm7, 64(%esp)                               #
        movdqa    %xmm6, 48(%esp)                               #
        xorl      %edx, %edx                                    #
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B8.9:                         # Preds ..B8.8 ..B8.24
        movdqa    %xmm1, %xmm7                                  #4147.22
        movdqa    %xmm1, %xmm6                                  #4148.22
        pand      48(%esp), %xmm7                               #4147.22
        pand      %xmm3, %xmm6                                  #4148.22
        pcmpeqw   %xmm6, %xmm7                                  #4149.22
        pmovmskb  %xmm7, %edi                                   #4150.22
        movl      %edi, 96(%esp)                                #4150.22
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B8.10:                        # Preds ..B8.7 ..B8.9
        movdqa    %xmm1, %xmm7                                  #4154.22
        movdqa    %xmm1, %xmm6                                  #4155.22
        pand      80(%esp), %xmm6                               #4155.22
        pand      %xmm2, %xmm7                                  #4154.22
        pcmpeqw   %xmm6, %xmm7                                  #4156.22
        pmovmskb  %xmm7, %edi                                   #4157.22
        movl      %edi, 44(%esp)                                #4157.22
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B8.11:                        # Preds ..B8.6 ..B8.10
        movdqa    %xmm1, %xmm6                                  #4161.22
        pand      64(%esp), %xmm1                               #4162.22
        andl      $-4, %ecx                                     #4169.17
        pand      %xmm0, %xmm6                                  #4161.22
        pcmpeqw   %xmm1, %xmm6                                  #4163.22
        pmovmskb  %xmm6, %edi                                   #4164.22
        movl      %edi, 40(%esp)                                #4164.22
        movl      %edx, %eax                                    #4172.29
        orl       96(%esp), %eax                                #4172.29
        orl       44(%esp), %eax                                #4172.34
        orl       %edi, %eax                                    #4172.39
        je        ..B8.20       # Prob 78%                      #4172.47
                                # LOE edx ecx ebx esi xmm0 xmm2 xmm3 xmm4 xmm5
..B8.12:                        # Preds ..B8.11
        movl      %ecx, 32(%esp)                                #
        movl      %edx, %eax                                    #4190.30
        movl      %esi, 36(%esp)                                #
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4190.30
        movswl    %ax, %ecx                                     #4190.30
        movl      96(%esp), %eax                                #4191.30
        movl      %ecx, 16(%esp)                                #4190.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4191.30
        movswl    %ax, %edi                                     #4191.30
        lea       1(%edi,%edi), %edi                            #4199.40
        movl      44(%esp), %eax                                #4192.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4192.30
        movswl    %ax, %esi                                     #4192.30
        movl      40(%esp), %eax                                #4193.30
        lea       2(%esi,%esi), %esi                            #4200.40
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4193.30
        movl      %edi, 20(%esp)                                #4199.40
        movl      %esi, 24(%esp)                                #4200.40
        movl      36(%esp), %esi                                #4205.32
        movswl    %ax, %ecx                                     #4193.30
        lea       3(%ecx,%ecx), %ecx                            #4201.40
        testl     %edx, %edx                                    #4205.32
        movl      %ecx, 28(%esp)                                #4201.40
        movl      32(%esp), %ecx                                #4205.32
        je        ..B8.14       # Prob 50%                      #4205.32
                                # LOE ecx ebx esi cl ch xmm0 xmm2 xmm3 xmm4 xmm5
..B8.13:                        # Preds ..B8.12
        movl      16(%esp), %eax                                #4205.43
        movl      100(%esp), %edx                               #4205.49
        lea       (%ecx,%eax,2), %edi                           #4205.43
        cmpl      %edi, %edx                                    #4205.49
        jae       ..B8.28       # Prob 1%                       #4205.49
                                # LOE ecx ebx esi cl ch xmm0 xmm2 xmm3 xmm4 xmm5
..B8.14:                        # Preds ..B8.13 ..B8.12
        movl      96(%esp), %eax                                #4206.32
        testl     %eax, %eax                                    #4206.32
        je        ..B8.16       # Prob 50%                      #4206.32
                                # LOE ecx ebx esi cl ch xmm0 xmm2 xmm3 xmm4 xmm5
..B8.15:                        # Preds ..B8.14
        movl      20(%esp), %edx                                #4206.43
        movl      100(%esp), %eax                               #4206.49
        addl      %ecx, %edx                                    #4206.43
        cmpl      %edx, %eax                                    #4206.49
        jae       ..B8.28       # Prob 1%                       #4206.49
                                # LOE ecx ebx esi cl ch xmm0 xmm2 xmm3 xmm4 xmm5
..B8.16:                        # Preds ..B8.15 ..B8.14
        movl      44(%esp), %eax                                #4207.32
        testl     %eax, %eax                                    #4207.32
        je        ..B8.18       # Prob 50%                      #4207.32
                                # LOE ecx ebx esi cl ch xmm0 xmm2 xmm3 xmm4 xmm5
..B8.17:                        # Preds ..B8.16
        movl      24(%esp), %edx                                #4207.43
        movl      100(%esp), %eax                               #4207.49
        addl      %ecx, %edx                                    #4207.43
        cmpl      %edx, %eax                                    #4207.49
        jae       ..B8.28       # Prob 1%                       #4207.49
                                # LOE ecx ebx esi cl ch xmm0 xmm2 xmm3 xmm4 xmm5
..B8.18:                        # Preds ..B8.17 ..B8.16
        movl      40(%esp), %eax                                #4208.32
        testl     %eax, %eax                                    #4208.32
        je        ..B8.20       # Prob 50%                      #4208.32
                                # LOE ecx ebx esi cl ch xmm0 xmm2 xmm3 xmm4 xmm5
..B8.19:                        # Preds ..B8.18
        movl      28(%esp), %edx                                #4208.43
        movl      100(%esp), %eax                               #4208.49
        addl      %ecx, %edx                                    #4208.43
        cmpl      %edx, %eax                                    #4208.49
        jae       ..B8.28       # Prob 1%                       #4208.49
                                # LOE ecx ebx esi xmm0 xmm2 xmm3 xmm4 xmm5
..B8.20:                        # Preds ..B8.19 ..B8.18 ..B8.11
        movl      100(%esp), %eax                               #4216.28
        lea       4(%ecx), %edx                                 #4241.13
        cmpl      %edx, %eax                                    #4216.28
        jb        ..B8.26       # Prob 1%                       #4216.28
                                # LOE ecx ebx esi xmm0 xmm2 xmm3 xmm4 xmm5
..B8.21:                        # Preds ..B8.20
        movl      104(%esp), %eax                               #4237.25
        cmpl      %eax, %esi                                    #4237.25
        jae       ..B8.26       # Prob 1%                       #4237.25
                                # LOE ecx ebx esi xmm0 xmm2 xmm3 xmm4 xmm5
..B8.22:                        # Preds ..B8.21
        movl      100(%esp), %eax                               #4242.24
        addl      $32, %ecx                                     #4241.13
        cmpl      %eax, %ecx                                    #4242.24
        ja        ..B8.26       # Prob 1%                       #4242.24
                                # LOE ecx ebx esi xmm0 xmm2 xmm3 xmm4 xmm5
..B8.23:                        # Preds ..B8.22
        movzbl    (%esi), %eax                                  #4247.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %edx        #4247.22
        movzbl    1(%esi), %edi                                 #4247.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%edi,2), %eax        #4247.22
        movw      %dx, (%esp)                                   #4247.22
        movw      %ax, 2(%esp)                                  #4247.22
        movzbl    2(%esi), %eax                                 #4247.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4247.22
        movw      %ax, 4(%esp)                                  #4247.22
        movzbl    3(%esi), %eax                                 #4247.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4247.22
        movw      %ax, 6(%esp)                                  #4247.22
        movzbl    4(%esi), %eax                                 #4247.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4247.22
        movw      %ax, 8(%esp)                                  #4247.22
        movzbl    5(%esi), %eax                                 #4247.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4247.22
        movw      %ax, 10(%esp)                                 #4247.22
        movzbl    6(%esi), %eax                                 #4247.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4247.22
        movw      %ax, 12(%esp)                                 #4247.22
        movzbl    7(%esi), %eax                                 #4247.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4247.22
        addl      $8, %esi                                      #4262.13
        movw      %ax, 14(%esp)                                 #4247.22
        movdqu    (%esp), %xmm1                                 #4247.22
                                # LOE ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B8.24:                        # Preds ..B8.23 ..B8.5
        movdqa    %xmm1, %xmm7                                  #4140.22
        movdqa    %xmm1, %xmm6                                  #4141.22
        pand      %xmm5, %xmm7                                  #4140.22
        pand      %xmm4, %xmm6                                  #4141.22
        pcmpeqw   %xmm6, %xmm7                                  #4142.22
        pmovmskb  %xmm7, %edx                                   #4143.22
        jmp       ..B8.9        # Prob 100%                     #4143.22
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B8.26:                        # Preds ..B8.20 ..B8.21 ..B8.22 # Infreq
        xorl      %eax, %eax                                    #4273.12
        addl      $116, %esp                                    #4273.12
        popl      %ebx                                          #4273.12
        popl      %esi                                          #4273.12
        popl      %edi                                          #4273.12
        movl      %ebp, %esp                                    #4273.12
        popl      %ebp                                          #4273.12
        ret                                                     #4273.12
                                # LOE
..B8.28:                        # Preds ..B8.19 ..B8.17 ..B8.15 ..B8.13 # Infreq
        movl      $1, %eax                                      #4205.63
        addl      $116, %esp                                    #4205.63
        popl      %ebx                                          #4205.63
        popl      %esi                                          #4205.63
        popl      %edi                                          #4205.63
        movl      %ebp, %esp                                    #4205.63
        popl      %ebp                                          #4205.63
        ret                                                     #4205.63
        .align    2,0x90
                                # LOE
# mark_end;
	.type	eval_4na_16,@function
	.size	eval_4na_16,.-eval_4na_16
	.data
# -- End  eval_4na_16
	.text
# -- Begin  eval_4na_32
# mark_begin;
       .align    2,0x90
eval_4na_32:
# parameter 1: %eax
# parameter 2: %edx
# parameter 3: %ecx
# parameter 4: 20 + %ebp
..B9.1:                         # Preds ..B9.0
        pushl     %ebp                                          #4279.1
        movl      %esp, %ebp                                    #4279.1
        andl      $-16, %esp                                    #4279.1
        pushl     %edi                                          #4279.1
        pushl     %esi                                          #4279.1
        pushl     %ebx                                          #4279.1
        subl      $132, %esp                                    #4279.1
        movdqa    16(%eax), %xmm7                               #4331.5
        movdqa    32(%eax), %xmm5                               #4331.5
        movl      20(%ebp), %ebx                                #4277.5
        movdqa    48(%eax), %xmm4                               #4331.5
        movdqa    64(%eax), %xmm3                               #4331.5
        movdqa    80(%eax), %xmm6                               #4331.5
        movdqa    112(%eax), %xmm2                              #4331.5
        movdqa    128(%eax), %xmm1                              #4331.5
        movl      %ecx, %edi                                    #4315.49
        shrl      $2, %edi                                      #4315.49
        call      ..L4          # Prob 100%                     #4279.1
..L4:                                                           #
        popl      %esi                                          #4279.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L4], %esi       #4279.1
        movl      %esi, 32(%esp)                                #4279.1
        lea       (%ecx,%ebx), %esi                             #4312.5
        lea       3(%ecx,%ebx), %ebx                            #4321.50
        shrl      $2, %ebx                                      #4321.57
        subl      4(%eax), %esi                                 #4318.12
        movl      %esi, 112(%esp)                               #4318.12
        movzbl    (%edi,%edx), %esi                             #4324.14
        addl      %edx, %ebx                                    #4321.30
        movl      %ebx, 116(%esp)                               #4321.30
        movl      32(%esp), %ebx                                #4324.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4324.14
        movw      %si, (%esp)                                   #4324.14
        movzbl    1(%edi,%edx), %esi                            #4324.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4324.14
        movw      %si, 2(%esp)                                  #4324.14
        movzbl    2(%edi,%edx), %esi                            #4324.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4324.14
        movw      %si, 4(%esp)                                  #4324.14
        movzbl    3(%edi,%edx), %esi                            #4324.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4324.14
        movw      %si, 6(%esp)                                  #4324.14
        movzbl    4(%edi,%edx), %esi                            #4324.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4324.14
        movw      %si, 8(%esp)                                  #4324.14
        movzbl    5(%edi,%edx), %esi                            #4324.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4324.14
        movw      %si, 10(%esp)                                 #4324.14
        movzbl    6(%edi,%edx), %esi                            #4324.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4324.14
        movw      %si, 12(%esp)                                 #4324.14
        movzbl    7(%edi,%edx), %esi                            #4324.14
        movzwl    expand_2na.0@GOTOFF(%ebx,%esi,2), %esi        #4324.14
        movw      %si, 14(%esp)                                 #4324.14
        movdqu    (%esp), %xmm0                                 #4324.14
        lea       8(%edx,%edi), %esi                            #4325.5
        movdqa    %xmm0, 16(%esp)                               #4324.14
        movdqa    96(%eax), %xmm0                               #4331.5
        movl      %esi, %edx                                    #4325.5
        movl      %ecx, %eax                                    #4345.20
        andl      $3, %eax                                      #4345.20
        je        ..B9.33       # Prob 20%                      #4345.20
                                # LOE eax edx ecx ebx esi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B9.2:                         # Preds ..B9.1
        cmpl      $1, %eax                                      #4345.20
        je        ..B9.8        # Prob 25%                      #4345.20
                                # LOE eax edx ecx ebx esi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B9.3:                         # Preds ..B9.2
        cmpl      $2, %eax                                      #4345.20
        je        ..B9.7        # Prob 33%                      #4345.20
                                # LOE eax edx ecx ebx esi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B9.4:                         # Preds ..B9.3
        cmpl      $3, %eax                                      #4345.20
        je        ..B9.6        # Prob 50%                      #4345.20
                                # LOE edx ecx ebx esi bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B9.5:                         # Preds ..B9.4
        movdqa    %xmm0, 80(%esp)                               #
        movdqa    16(%esp), %xmm0                               #
        movdqa    %xmm6, 64(%esp)                               #
        movdqa    %xmm7, 48(%esp)                               #
        jmp       ..B9.25       # Prob 100%                     #
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.6:                         # Preds ..B9.4
        movdqa    %xmm0, 80(%esp)                               #
        movdqa    16(%esp), %xmm0                               #
        movdqa    %xmm6, 64(%esp)                               #
        movdqa    %xmm7, 48(%esp)                               #
        xorl      %eax, %eax                                    #
        movl      %eax, 44(%esp)                                #
        xorl      %edi, %edi                                    #
        movl      %edi, 96(%esp)                                #
        movl      %eax, 104(%esp)                               #
        movl      $2, %edi                                      #
        jmp       ..B9.11       # Prob 100%                     #
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.7:                         # Preds ..B9.3
        movdqa    %xmm0, 80(%esp)                               #
        movdqa    16(%esp), %xmm0                               #
        movdqa    %xmm6, 64(%esp)                               #
        movdqa    %xmm7, 48(%esp)                               #
        xorl      %eax, %eax                                    #
        movl      %eax, 96(%esp)                                #
        movl      $2, %edi                                      #
        movl      %eax, 104(%esp)                               #
        jmp       ..B9.10       # Prob 100%                     #
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.8:                         # Preds ..B9.2
        movdqa    %xmm0, 80(%esp)                               #
        movdqa    16(%esp), %xmm0                               #
        movdqa    %xmm6, 64(%esp)                               #
        movdqa    %xmm7, 48(%esp)                               #
        xorl      %eax, %eax                                    #
        movl      %eax, 104(%esp)                               #
        movl      $2, %edi                                      #
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.9:                         # Preds ..B9.8 ..B9.29
        movdqa    %xmm0, %xmm7                                  #4369.22
        movdqa    %xmm0, %xmm6                                  #4370.22
        pand      %xmm4, %xmm7                                  #4369.22
        pand      %xmm3, %xmm6                                  #4370.22
        pcmpeqd   %xmm6, %xmm7                                  #4371.22
        pmovmskb  %xmm7, %eax                                   #4372.22
        movl      %eax, 96(%esp)                                #4372.22
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.10:                        # Preds ..B9.7 ..B9.9
        movdqa    %xmm0, %xmm7                                  #4376.22
        movdqa    %xmm0, %xmm6                                  #4377.22
        pand      64(%esp), %xmm7                               #4376.22
        pand      80(%esp), %xmm6                               #4377.22
        pcmpeqd   %xmm6, %xmm7                                  #4378.22
        pmovmskb  %xmm7, %eax                                   #4379.22
        movl      %eax, 44(%esp)                                #4379.22
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.11:                        # Preds ..B9.6 ..B9.10
        movdqa    %xmm0, %xmm7                                  #4383.22
        movdqa    %xmm0, %xmm6                                  #4384.22
        pand      %xmm2, %xmm7                                  #4383.22
        pand      %xmm1, %xmm6                                  #4384.22
        pcmpeqd   %xmm6, %xmm7                                  #4385.22
        pmovmskb  %xmm7, %eax                                   #4386.22
        movl      %eax, 100(%esp)                               #4386.22
        andl      $-4, %ecx                                     #4391.17
        movl      %ecx, 108(%esp)                               #4391.17
        movl      104(%esp), %ecx                               #4394.29
        orl       96(%esp), %ecx                                #4394.29
        orl       44(%esp), %ecx                                #4394.34
        orl       %eax, %ecx                                    #4394.39
        je        ..B9.20       # Prob 78%                      #4394.47
                                # LOE edx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.12:                        # Preds ..B9.11
        movl      %edi, 32(%esp)                                #
        movl      104(%esp), %edi                               #4412.30
        movl      %esi, 36(%esp)                                #
        movl      %edi, %eax                                    #4412.30
        movl      %edx, 40(%esp)                                #
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4412.30
        movswl    %ax, %edx                                     #4412.30
        movl      96(%esp), %eax                                #4413.30
        movl      %edx, 16(%esp)                                #4412.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4413.30
        movswl    %ax, %esi                                     #4413.30
        lea       1(%esi,%esi), %esi                            #4421.40
        movl      44(%esp), %eax                                #4414.30
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4414.30
        movswl    %ax, %ecx                                     #4414.30
        movl      100(%esp), %eax                               #4415.30
        lea       2(%ecx,%ecx), %ecx                            #4422.40
# Begin ASM
        bsf %ax, %ax;jnz .+5;xor %eax, %eax;dec %eax;
# End ASM                                                       #4415.30
        movl      %esi, 20(%esp)                                #4421.40
        movl      36(%esp), %esi                                #4427.32
        movl      %ecx, 24(%esp)                                #4422.40
        movswl    %ax, %edx                                     #4415.30
        lea       3(%edx,%edx), %edx                            #4423.40
        movl      %edx, 28(%esp)                                #4423.40
        movl      40(%esp), %edx                                #4427.32
        testl     %edi, %edi                                    #4427.32
        movl      32(%esp), %edi                                #4427.32
        je        ..B9.14       # Prob 50%                      #4427.32
                                # LOE edx ebx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.13:                        # Preds ..B9.12
        movl      16(%esp), %ecx                                #4427.43
        movl      108(%esp), %eax                               #4427.43
        lea       (%eax,%ecx,2), %ecx                           #4427.43
        movl      112(%esp), %eax                               #4427.49
        cmpl      %ecx, %eax                                    #4427.49
        jae       ..B9.34       # Prob 1%                       #4427.49
                                # LOE edx ebx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.14:                        # Preds ..B9.13 ..B9.12
        movl      96(%esp), %eax                                #4428.32
        testl     %eax, %eax                                    #4428.32
        je        ..B9.16       # Prob 50%                      #4428.32
                                # LOE edx ebx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.15:                        # Preds ..B9.14
        movl      20(%esp), %ecx                                #4428.43
        addl      108(%esp), %ecx                               #4428.43
        movl      112(%esp), %eax                               #4428.49
        cmpl      %ecx, %eax                                    #4428.49
        jae       ..B9.34       # Prob 1%                       #4428.49
                                # LOE edx ebx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.16:                        # Preds ..B9.15 ..B9.14
        movl      44(%esp), %eax                                #4429.32
        testl     %eax, %eax                                    #4429.32
        je        ..B9.18       # Prob 50%                      #4429.32
                                # LOE edx ebx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.17:                        # Preds ..B9.16
        movl      24(%esp), %ecx                                #4429.43
        addl      108(%esp), %ecx                               #4429.43
        movl      112(%esp), %eax                               #4429.49
        cmpl      %ecx, %eax                                    #4429.49
        jae       ..B9.34       # Prob 1%                       #4429.49
                                # LOE edx ebx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.18:                        # Preds ..B9.17 ..B9.16
        movl      100(%esp), %eax                               #4430.32
        testl     %eax, %eax                                    #4430.32
        je        ..B9.20       # Prob 50%                      #4430.32
                                # LOE edx ebx esi edi dl dh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.19:                        # Preds ..B9.18
        movl      28(%esp), %ecx                                #4430.43
        addl      108(%esp), %ecx                               #4430.43
        movl      112(%esp), %eax                               #4430.49
        cmpl      %ecx, %eax                                    #4430.49
        jae       ..B9.34       # Prob 1%                       #4430.49
                                # LOE edx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.20:                        # Preds ..B9.19 ..B9.18 ..B9.11
        movl      108(%esp), %ecx                               #4435.17
        movl      112(%esp), %eax                               #4438.28
        lea       4(%ecx), %ecx                                 #4435.17
        cmpl      %eax, %ecx                                    #4438.28
        ja        ..B9.31       # Prob 1%                       #4438.28
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.21:                        # Preds ..B9.20
        addl      $-1, %edi                                     #4442.25
        jne       ..B9.26       # Prob 50%                      #4442.39
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.22:                        # Preds ..B9.21
        movl      116(%esp), %eax                               #4459.25
        cmpl      %eax, %esi                                    #4459.25
        jae       ..B9.31       # Prob 1%                       #4459.25
                                # LOE ebx esi xmm1 xmm2 xmm3 xmm4 xmm5
..B9.23:                        # Preds ..B9.22
        movl      108(%esp), %ecx                               #4463.13
        movl      112(%esp), %eax                               #4464.24
        lea       28(%ecx), %ecx                                #4463.13
        cmpl      %eax, %ecx                                    #4464.24
        ja        ..B9.31       # Prob 1%                       #4464.24
                                # LOE ecx ebx esi xmm1 xmm2 xmm3 xmm4 xmm5
..B9.24:                        # Preds ..B9.23
        movzbl    (%esi), %eax                                  #4469.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %edx        #4469.22
        movzbl    1(%esi), %edi                                 #4469.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%edi,2), %eax        #4469.22
        movw      %dx, (%esp)                                   #4469.22
        movw      %ax, 2(%esp)                                  #4469.22
        movzbl    2(%esi), %eax                                 #4469.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4469.22
        movw      %ax, 4(%esp)                                  #4469.22
        movzbl    3(%esi), %eax                                 #4469.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4469.22
        movw      %ax, 6(%esp)                                  #4469.22
        movzbl    4(%esi), %eax                                 #4469.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4469.22
        movw      %ax, 8(%esp)                                  #4469.22
        movzbl    5(%esi), %eax                                 #4469.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4469.22
        movw      %ax, 10(%esp)                                 #4469.22
        movzbl    6(%esi), %eax                                 #4469.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4469.22
        movw      %ax, 12(%esp)                                 #4469.22
        movzbl    7(%esi), %eax                                 #4469.22
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4469.22
        addl      $8, %esi                                      #4484.13
        movl      %esi, %edx                                    #4484.13
        movw      %ax, 14(%esp)                                 #4469.22
        movdqu    (%esp), %xmm0                                 #4469.22
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.25:                        # Preds ..B9.5 ..B9.24
        movl      $2, %edi                                      #4352.13
        jmp       ..B9.29       # Prob 100%                     #4352.13
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.26:                        # Preds ..B9.21
        movl      116(%esp), %eax                               #4450.26
        psrldq    $2, %xmm0                                     #4447.26
        cmpl      %eax, %edx                                    #4450.26
        jae       ..B9.28       # Prob 19%                      #4450.26
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.27:                        # Preds ..B9.26
        movzbl    (%edx), %eax                                  #4451.72
        movzwl    expand_2na.0@GOTOFF(%ebx,%eax,2), %eax        #4451.57
        pinsrw    $7, %eax, %xmm0                               #4451.30
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.28:                        # Preds ..B9.27 ..B9.26
        addl      $1, %edx                                      #4454.20
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.29:                        # Preds ..B9.25 ..B9.28 ..B9.33
        movdqa    %xmm0, %xmm7                                  #4362.22
        movdqa    %xmm0, %xmm6                                  #4363.22
        pand      48(%esp), %xmm7                               #4362.22
        pand      %xmm5, %xmm6                                  #4363.22
        pcmpeqd   %xmm6, %xmm7                                  #4364.22
        pmovmskb  %xmm7, %eax                                   #4365.22
        movl      %eax, 104(%esp)                               #4365.22
        jmp       ..B9.9        # Prob 100%                     #4365.22
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.31:                        # Preds ..B9.20 ..B9.22 ..B9.23 # Infreq
        xorl      %eax, %eax                                    #4495.12
        addl      $132, %esp                                    #4495.12
        popl      %ebx                                          #4495.12
        popl      %esi                                          #4495.12
        popl      %edi                                          #4495.12
        movl      %ebp, %esp                                    #4495.12
        popl      %ebp                                          #4495.12
        ret                                                     #4495.12
                                # LOE
..B9.33:                        # Preds ..B9.1                  # Infreq
        movdqa    %xmm0, 80(%esp)                               #
        movdqa    16(%esp), %xmm0                               #
        movdqa    %xmm6, 64(%esp)                               #
        movdqa    %xmm7, 48(%esp)                               #
        movl      $2, %edi                                      #
        jmp       ..B9.29       # Prob 100%                     #
                                # LOE edx ecx ebx esi edi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B9.34:                        # Preds ..B9.13 ..B9.19 ..B9.17 ..B9.15 # Infreq
        movl      $1, %eax                                      #4428.63
        addl      $132, %esp                                    #4428.63
        popl      %ebx                                          #4428.63
        popl      %esi                                          #4428.63
        popl      %edi                                          #4428.63
        movl      %ebp, %esp                                    #4428.63
        popl      %ebp                                          #4428.63
        ret                                                     #4428.63
        .align    2,0x90
                                # LOE
# mark_end;
	.type	eval_4na_32,@function
	.size	eval_4na_32,.-eval_4na_32
	.data
# -- End  eval_4na_32
	.text
# -- Begin  eval_4na_128
# mark_begin;
       .align    2,0x90
eval_4na_128:
# parameter 1: %eax
# parameter 2: %edx
# parameter 3: %ecx
# parameter 4: 20 + %ebp
..B10.1:                        # Preds ..B10.0
        pushl     %ebp                                          #4724.1
        movl      %esp, %ebp                                    #4724.1
        andl      $-16, %esp                                    #4724.1
        pushl     %edi                                          #4724.1
        pushl     %esi                                          #4724.1
        pushl     %ebx                                          #4724.1
        subl      $116, %esp                                    #4724.1
        movl      20(%ebp), %esi                                #4722.5
        movl      %ecx, 16(%esp)                                #4724.1
        movdqa    16(%eax), %xmm5                               #4776.5
        movdqa    32(%eax), %xmm4                               #4776.5
        movdqa    48(%eax), %xmm3                               #4776.5
        movdqa    64(%eax), %xmm2                               #4776.5
        movdqa    80(%eax), %xmm1                               #4776.5
        movdqa    96(%eax), %xmm6                               #4776.5
        movdqa    128(%eax), %xmm7                              #4776.5
        movl      %ecx, %edi                                    #4760.49
        shrl      $2, %edi                                      #4760.49
        call      ..L5          # Prob 100%                     #4724.1
..L5:                                                           #
        popl      %ebx                                          #4724.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L5], %ebx       #4724.1
        movl      %ebx, 20(%esp)                                #4724.1
        lea       (%ecx,%esi), %ebx                             #4757.5
        lea       3(%ecx,%esi), %ecx                            #4766.50
        subl      4(%eax), %ebx                                 #4763.12
        movl      20(%esp), %esi                                #4769.14
        movl      %ebx, 100(%esp)                               #4763.12
        shrl      $2, %ecx                                      #4766.57
        addl      %edx, %ecx                                    #4766.30
        movl      %ecx, 96(%esp)                                #4766.30
        movzbl    (%edi,%edx), %ecx                             #4769.14
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #4769.14
        movw      %cx, (%esp)                                   #4769.14
        movzbl    1(%edi,%edx), %ecx                            #4769.14
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #4769.14
        movw      %cx, 2(%esp)                                  #4769.14
        movzbl    2(%edi,%edx), %ecx                            #4769.14
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #4769.14
        movw      %cx, 4(%esp)                                  #4769.14
        movzbl    3(%edi,%edx), %ecx                            #4769.14
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #4769.14
        movw      %cx, 6(%esp)                                  #4769.14
        movzbl    4(%edi,%edx), %ecx                            #4769.14
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #4769.14
        movw      %cx, 8(%esp)                                  #4769.14
        movzbl    5(%edi,%edx), %ecx                            #4769.14
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #4769.14
        movw      %cx, 10(%esp)                                 #4769.14
        movzbl    6(%edi,%edx), %ecx                            #4769.14
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #4769.14
        movw      %cx, 12(%esp)                                 #4769.14
        movzbl    7(%edi,%edx), %ecx                            #4769.14
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #4769.14
        movw      %cx, 14(%esp)                                 #4769.14
        movdqu    (%esp), %xmm0                                 #4769.14
        lea       8(%edx,%edi), %edx                            #4770.5
        movdqa    %xmm0, 64(%esp)                               #4769.14
        movdqa    112(%eax), %xmm0                              #4776.5
        movl      %edx, %ecx                                    #4770.5
        movl      16(%esp), %eax                                #4781.20
        subl      %eax, %ebx                                    #4781.20
        addl      $7, %ebx                                      #4781.33
        shrl      $2, %ebx                                      #4781.40
        movl      %eax, %edi                                    #4790.20
        andl      $3, %edi                                      #4790.20
        je        ..B10.32      # Prob 20%                      #4790.20
                                # LOE eax edx ecx ebx esi edi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B10.2:                        # Preds ..B10.1
        cmpl      $1, %edi                                      #4790.20
        je        ..B10.8       # Prob 25%                      #4790.20
                                # LOE eax edx ecx ebx esi edi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B10.3:                        # Preds ..B10.2
        cmpl      $2, %edi                                      #4790.20
        je        ..B10.7       # Prob 33%                      #4790.20
                                # LOE eax edx ecx ebx esi edi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B10.4:                        # Preds ..B10.3
        cmpl      $3, %edi                                      #4790.20
        je        ..B10.6       # Prob 50%                      #4790.20
                                # LOE eax edx ecx ebx esi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B10.5:                        # Preds ..B10.4
        movdqa    %xmm7, 16(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    64(%esp), %xmm0                               #
        movdqa    %xmm6, 48(%esp)                               #
        movl      %ecx, 92(%esp)                                #
        jmp       ..B10.16      # Prob 100%                     #
                                # LOE eax edx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.6:                        # Preds ..B10.4
        movdqa    %xmm7, 16(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    64(%esp), %xmm0                               #
        movdqa    %xmm6, 48(%esp)                               #
        xorl      %edi, %edi                                    #
        movl      %edi, 80(%esp)                                #
        movl      %ecx, 92(%esp)                                #
        movl      %edi, 84(%esp)                                #
        movl      %edi, 88(%esp)                                #
        jmp       ..B10.11      # Prob 100%                     #
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.7:                        # Preds ..B10.3
        movdqa    %xmm7, 16(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    64(%esp), %xmm0                               #
        movdqa    %xmm6, 48(%esp)                               #
        xorl      %edi, %edi                                    #
        movl      %edi, 84(%esp)                                #
        movl      %ecx, 92(%esp)                                #
        movl      %edi, 88(%esp)                                #
        jmp       ..B10.10      # Prob 100%                     #
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.8:                        # Preds ..B10.2
        movdqa    %xmm7, 16(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    64(%esp), %xmm0                               #
        movdqa    %xmm6, 48(%esp)                               #
        xorl      %edi, %edi                                    #
        movl      %ecx, 92(%esp)                                #
        movl      %edi, 88(%esp)                                #
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.9:                        # Preds ..B10.8 ..B10.20
        movdqa    %xmm0, %xmm7                                  #4814.22
        movdqa    %xmm0, %xmm6                                  #4815.22
        pand      %xmm3, %xmm7                                  #4814.22
        pand      %xmm2, %xmm6                                  #4815.22
        pcmpeqd   %xmm6, %xmm7                                  #4816.22
        pmovmskb  %xmm7, %ecx                                   #4817.22
        addl      $1, %ecx                                      #4818.17
        shrl      $16, %ecx                                     #4818.17
        negl      %ecx                                          #4818.17
        movl      %ecx, 84(%esp)                                #4818.17
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.10:                       # Preds ..B10.7 ..B10.9
        movdqa    %xmm0, %xmm7                                  #4821.22
        movdqa    %xmm0, %xmm6                                  #4822.22
        pand      48(%esp), %xmm6                               #4822.22
        pand      %xmm1, %xmm7                                  #4821.22
        pcmpeqd   %xmm6, %xmm7                                  #4823.22
        pmovmskb  %xmm7, %ecx                                   #4824.22
        addl      $1, %ecx                                      #4825.17
        shrl      $16, %ecx                                     #4825.17
        negl      %ecx                                          #4825.17
        movl      %ecx, 80(%esp)                                #4825.17
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.11:                       # Preds ..B10.6 ..B10.10
        movdqa    %xmm0, %xmm7                                  #4828.22
        movdqa    %xmm0, %xmm6                                  #4829.22
        movl      88(%esp), %edi                                #4839.29
        pand      32(%esp), %xmm7                               #4828.22
        pand      16(%esp), %xmm6                               #4829.22
        pcmpeqd   %xmm6, %xmm7                                  #4830.22
        pmovmskb  %xmm7, %ecx                                   #4831.22
        andl      $-4, %eax                                     #4836.17
        addl      $1, %ecx                                      #4832.17
        orl       84(%esp), %edi                                #4839.29
        orl       80(%esp), %edi                                #4839.34
        shrl      $16, %ecx                                     #4832.17
        negl      %ecx                                          #4832.17
        orl       %ecx, %edi                                    #4839.39
        jne       ..B10.21      # Prob 1%                       #4839.47
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.12:                       # Preds ..B10.11
        movl      100(%esp), %ecx                               #4883.28
        addl      $4, %eax                                      #4880.17
        cmpl      %ecx, %eax                                    #4883.28
        ja        ..B10.28      # Prob 1%                       #4883.28
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.13:                       # Preds ..B10.12
        addl      $-1, %ebx                                     #4887.25
        jne       ..B10.17      # Prob 50%                      #4887.39
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.14:                       # Preds ..B10.13
        movl      96(%esp), %ecx                                #4904.25
        cmpl      %ecx, %edx                                    #4904.25
        jae       ..B10.28      # Prob 1%                       #4904.25
                                # LOE eax edx esi xmm1 xmm2 xmm3 xmm4 xmm5
..B10.15:                       # Preds ..B10.14
        movzbl    (%edx), %ecx                                  #4914.22
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ebx        #4914.22
        movzbl    1(%edx), %edi                                 #4914.22
        movzwl    expand_2na.0@GOTOFF(%esi,%edi,2), %ecx        #4914.22
        movw      %bx, (%esp)                                   #4914.22
        movw      %cx, 2(%esp)                                  #4914.22
        movzbl    2(%edx), %ecx                                 #4914.22
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #4914.22
        movw      %cx, 4(%esp)                                  #4914.22
        movzbl    3(%edx), %ecx                                 #4914.22
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #4914.22
        movw      %cx, 6(%esp)                                  #4914.22
        movzbl    4(%edx), %ecx                                 #4914.22
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #4914.22
        movw      %cx, 8(%esp)                                  #4914.22
        movzbl    5(%edx), %ecx                                 #4914.22
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #4914.22
        movw      %cx, 10(%esp)                                 #4914.22
        movzbl    6(%edx), %ecx                                 #4914.22
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #4914.22
        movw      %cx, 12(%esp)                                 #4914.22
        movzbl    7(%edx), %ecx                                 #4914.22
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #4914.22
        addl      $8, %edx                                      #4929.13
        movw      %cx, 14(%esp)                                 #4914.22
        movdqu    (%esp), %xmm0                                 #4914.22
        movl      %edx, 92(%esp)                                #4929.13
                                # LOE eax edx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.16:                       # Preds ..B10.5 ..B10.15
        movl      $8, %ebx                                      #4797.13
        jmp       ..B10.20      # Prob 100%                     #4797.13
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.17:                       # Preds ..B10.13
        movl      96(%esp), %edi                                #4895.26
        movl      92(%esp), %ecx                                #4895.26
        psrldq    $2, %xmm0                                     #4892.26
        cmpl      %edi, %ecx                                    #4895.26
        jae       ..B10.19      # Prob 19%                      #4895.26
                                # LOE eax edx ecx ebx esi cl ch xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.18:                       # Preds ..B10.17
        movzbl    (%ecx), %edi                                  #4896.72
        movzwl    expand_2na.0@GOTOFF(%esi,%edi,2), %ecx        #4896.57
        pinsrw    $7, %ecx, %xmm0                               #4896.30
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.19:                       # Preds ..B10.18 ..B10.17
        addl      $1, 92(%esp)                                  #4899.20
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.20:                       # Preds ..B10.16 ..B10.19 ..B10.32
        movdqa    %xmm0, %xmm7                                  #4807.22
        movdqa    %xmm0, %xmm6                                  #4808.22
        pand      %xmm5, %xmm7                                  #4807.22
        pand      %xmm4, %xmm6                                  #4808.22
        pcmpeqd   %xmm6, %xmm7                                  #4809.22
        pmovmskb  %xmm7, %ecx                                   #4810.22
        addl      $1, %ecx                                      #4811.17
        shrl      $16, %ecx                                     #4811.17
        negl      %ecx                                          #4811.17
        movl      %ecx, 88(%esp)                                #4811.17
        jmp       ..B10.9       # Prob 100%                     #4811.17
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B10.21:                       # Preds ..B10.11                # Infreq
        movl      88(%esp), %edi                                #
        movl      100(%esp), %edx                               #4842.30
        subl      %eax, %edx                                    #4842.30
        movl      %edx, 100(%esp)                               #4842.30
        cmpl      $2, %edx                                      #4842.37
        je        ..B10.25      # Prob 25%                      #4842.37
                                # LOE edx edi dl dh
..B10.22:                       # Preds ..B10.21                # Infreq
        movl      %edx, %eax                                    #4842.37
        cmpl      $1, %eax                                      #4842.37
        je        ..B10.26      # Prob 33%                      #4842.37
                                # LOE eax edi al ah
..B10.23:                       # Preds ..B10.22                # Infreq
        testl     %eax, %eax                                    #4842.37
        je        ..B10.27      # Prob 50%                      #4842.37
                                # LOE edi
..B10.24:                       # Preds ..B10.25 ..B10.26 ..B10.27 ..B10.23 # Infreq
        movl      $1, %eax                                      #4845.32
        addl      $116, %esp                                    #4845.32
        popl      %ebx                                          #4845.32
        popl      %esi                                          #4845.32
        popl      %edi                                          #4845.32
        movl      %ebp, %esp                                    #4845.32
        popl      %ebp                                          #4845.32
        ret                                                     #4845.32
                                # LOE
..B10.25:                       # Preds ..B10.21                # Infreq
        movl      80(%esp), %eax                                #4847.36
        testl     %eax, %eax                                    #4847.36
        jne       ..B10.24      # Prob 28%                      #4847.36
                                # LOE edi
..B10.26:                       # Preds ..B10.25 ..B10.22       # Infreq
        movl      84(%esp), %eax                                #4849.36
        testl     %eax, %eax                                    #4849.36
        jne       ..B10.24      # Prob 28%                      #4849.36
                                # LOE edi
..B10.27:                       # Preds ..B10.23 ..B10.26       # Infreq
        testl     %edi, %edi                                    #4851.36
        jne       ..B10.24      # Prob 28%                      #4851.36
                                # LOE
..B10.28:                       # Preds ..B10.14 ..B10.12 ..B10.27 # Infreq
        xorl      %eax, %eax                                    #4853.28
        addl      $116, %esp                                    #4853.28
        popl      %ebx                                          #4853.28
        popl      %esi                                          #4853.28
        popl      %edi                                          #4853.28
        movl      %ebp, %esp                                    #4853.28
        popl      %ebp                                          #4853.28
        ret                                                     #4853.28
                                # LOE
..B10.32:                       # Preds ..B10.1                 # Infreq
        movdqa    %xmm7, 16(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    64(%esp), %xmm0                               #
        movdqa    %xmm6, 48(%esp)                               #
        movl      %ecx, 92(%esp)                                #
        jmp       ..B10.20      # Prob 100%                     #
        .align    2,0x90
                                # LOE eax edx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
# mark_end;
	.type	eval_4na_128,@function
	.size	eval_4na_128,.-eval_4na_128
	.data
# -- End  eval_4na_128
	.text
# -- Begin  eval_2na_pos
# mark_begin;
       .align    2,0x90
eval_2na_pos:
# parameter 1: %eax
# parameter 2: %edx
# parameter 3: %ecx
# parameter 4: 20 + %ebp
..B11.1:                        # Preds ..B11.0
        pushl     %ebp                                          #3662.1
        movl      %esp, %ebp                                    #3662.1
        andl      $-16, %esp                                    #3662.1
        pushl     %edi                                          #3662.1
        pushl     %esi                                          #3662.1
        pushl     %ebx                                          #3662.1
        subl      $84, %esp                                     #3662.1
        movl      20(%ebp), %edi                                #3660.5
        movl      %ecx, 60(%esp)                                #3662.1
        lea       (%ecx,%edi), %esi                             #3705.5
        subl      4(%eax), %esi                                 #3716.12
        movl      %ecx, %ebx                                    #3713.49
        movl      %esi, 64(%esp)                                #3716.12
        shrl      $2, %ebx                                      #3713.49
        movl      %ebx, 72(%esp)                                #3713.49
        lea       (%edx,%ebx), %ebx                             #3713.30
        lea       3(%ecx,%edi), %esi                            #3719.50
        shrl      $2, %esi                                      #3719.57
        movl      72(%esp), %edi                                #3722.14
        addl      %edx, %esi                                    #3719.30
        testb     $15, %bl                                      #3722.14
        jne       ..B11.3       # Prob 50%                      #3722.14
                                # LOE eax edx ecx ebx esi edi cl ch
..B11.2:                        # Preds ..B11.1
        movdqa    (%ebx), %xmm0                                 #3722.14
        movdqa    %xmm0, 32(%esp)                               #3722.14
        jmp       ..B11.4       # Prob 100%                     #3722.14
                                # LOE eax edx ecx esi edi cl ch
..B11.3:                        # Preds ..B11.1
        movdqu    (%ebx), %xmm0                                 #3722.14
        movdqa    %xmm0, 32(%esp)                               #3722.14
                                # LOE eax edx ecx esi edi cl ch
..B11.4:                        # Preds ..B11.2 ..B11.3
        lea       16(%edx,%edi), %ebx                           #3723.5
        movl      %ebx, 68(%esp)                                #3723.5
        cmpl      %esi, %ebx                                    #3728.16
        jae       ..B11.6       # Prob 12%                      #3728.16
                                # LOE eax ecx ebx esi cl bl ch bh
..B11.5:                        # Preds ..B11.4
        movzbl    -1(%ebx), %edx                                #3729.24
        shll      $8, %edx                                      #3729.38
        jmp       ..B11.7       # Prob 100%                     #3729.38
                                # LOE eax edx ecx ebx esi cl bl ch bh
..B11.6:                        # Preds ..B11.4
        xorl      %edx, %edx                                    #
                                # LOE eax edx ecx ebx esi cl bl ch bh
..B11.7:                        # Preds ..B11.5 ..B11.6
        movdqa    16(%eax), %xmm6                               #3733.5
        movdqa    32(%eax), %xmm5                               #3733.5
        movdqa    48(%eax), %xmm4                               #3733.5
        movdqa    64(%eax), %xmm3                               #3733.5
        movdqa    80(%eax), %xmm2                               #3733.5
        movdqa    96(%eax), %xmm1                               #3733.5
        movdqa    112(%eax), %xmm0                              #3733.5
        movdqa    128(%eax), %xmm7                              #3733.5
        movl      64(%esp), %eax                                #3738.20
        subl      %ecx, %eax                                    #3738.20
        addl      $7, %eax                                      #3738.33
        shrl      $2, %eax                                      #3738.40
        movl      %ecx, %edi                                    #3745.20
        andl      $3, %edi                                      #3745.20
        je        ..B11.55      # Prob 20%                      #3745.20
                                # LOE eax edx ecx ebx esi edi cl bl ch bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B11.8:                        # Preds ..B11.7
        cmpl      $1, %edi                                      #3745.20
        je        ..B11.14      # Prob 25%                      #3745.20
                                # LOE eax edx ecx ebx esi edi cl bl ch bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B11.9:                        # Preds ..B11.8
        cmpl      $2, %edi                                      #3745.20
        je        ..B11.13      # Prob 33%                      #3745.20
                                # LOE eax edx ecx ebx esi edi cl bl ch bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B11.10:                       # Preds ..B11.9
        cmpl      $3, %edi                                      #3745.20
        je        ..B11.12      # Prob 50%                      #3745.20
                                # LOE eax edx ecx ebx esi cl bl ch bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B11.11:                       # Preds ..B11.10
        movdqa    %xmm7, (%esp)                                 #
        movdqa    %xmm0, 16(%esp)                               #
        movdqa    32(%esp), %xmm0                               #
        movl      %esi, 56(%esp)                                #
        jmp       ..B11.26      # Prob 100%                     #
                                # LOE edx ecx ebx bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.12:                       # Preds ..B11.10
        movdqa    %xmm7, (%esp)                                 #
        movdqa    %xmm0, 16(%esp)                               #
        movdqa    32(%esp), %xmm0                               #
        movl      %esi, 56(%esp)                                #
        xorl      %edi, %edi                                    #
        movl      %edi, 48(%esp)                                #
        movl      %edi, 52(%esp)                                #
        movl      %edi, 72(%esp)                                #
        jmp       ..B11.17      # Prob 100%                     #
                                # LOE eax edx ecx ebx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.13:                       # Preds ..B11.9
        movdqa    %xmm7, (%esp)                                 #
        movdqa    %xmm0, 16(%esp)                               #
        movdqa    32(%esp), %xmm0                               #
        movl      %esi, 56(%esp)                                #
        xorl      %edi, %edi                                    #
        movl      %edi, 52(%esp)                                #
        movl      %edi, 72(%esp)                                #
        jmp       ..B11.16      # Prob 100%                     #
                                # LOE eax edx ecx ebx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.14:                       # Preds ..B11.8
        movdqa    %xmm7, (%esp)                                 #
        movdqa    %xmm0, 16(%esp)                               #
        movdqa    32(%esp), %xmm0                               #
        movl      %esi, 56(%esp)                                #
        xorl      %edi, %edi                                    #
        movl      %edi, 72(%esp)                                #
                                # LOE eax edx ecx ebx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.15:                       # Preds ..B11.14 ..B11.30
        movdqa    %xmm0, %xmm7                                  #3768.22
        pand      %xmm3, %xmm7                                  #3768.22
        pcmpeqd   %xmm4, %xmm7                                  #3769.22
        pmovmskb  %xmm7, %esi                                   #3770.22
        addl      $1, %esi                                      #3771.17
        shrl      $16, %esi                                     #3771.17
        negl      %esi                                          #3771.17
        movl      %esi, 52(%esp)                                #3771.17
                                # LOE eax edx ecx ebx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.16:                       # Preds ..B11.13 ..B11.15
        movdqa    %xmm0, %xmm7                                  #3774.22
        pand      %xmm1, %xmm7                                  #3774.22
        pcmpeqd   %xmm2, %xmm7                                  #3775.22
        pmovmskb  %xmm7, %esi                                   #3776.22
        addl      $1, %esi                                      #3777.17
        shrl      $16, %esi                                     #3777.17
        negl      %esi                                          #3777.17
        movl      %esi, 48(%esp)                                #3777.17
                                # LOE eax edx ecx ebx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.17:                       # Preds ..B11.12 ..B11.16
        movdqa    %xmm0, %xmm7                                  #3780.22
        movl      72(%esp), %edi                                #3790.29
        pand      (%esp), %xmm7                                 #3780.22
        pcmpeqd   16(%esp), %xmm7                               #3781.22
        pmovmskb  %xmm7, %esi                                   #3782.22
        andl      $-4, %ecx                                     #3787.17
        orl       52(%esp), %edi                                #3790.29
        addl      $1, %esi                                      #3783.17
        orl       48(%esp), %edi                                #3790.34
        shrl      $16, %esi                                     #3783.17
        negl      %esi                                          #3783.17
        orl       %esi, %edi                                    #3790.39
        movl      %esi, 32(%esp)                                #3783.17
        jne       ..B11.31      # Prob 1%                       #3790.47
                                # LOE eax edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.18:                       # Preds ..B11.17
        movl      64(%esp), %esi                                #3856.28
        addl      $4, %ecx                                      #3853.17
        cmpl      %esi, %ecx                                    #3856.28
        ja        ..B11.53      # Prob 1%                       #3856.28
                                # LOE eax edx ecx ebx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.19:                       # Preds ..B11.18
        addl      $-1, %eax                                     #3860.25
        jne       ..B11.27      # Prob 50%                      #3860.39
                                # LOE eax edx ecx ebx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.20:                       # Preds ..B11.19
        movl      56(%esp), %esi                                #3881.25
        cmpl      %esi, %ebx                                    #3881.25
        jae       ..B11.53      # Prob 1%                       #3881.25
                                # LOE edx ecx ebx esi xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.21:                       # Preds ..B11.20
        testb     $15, %bl                                      #3891.22
        jne       ..B11.23      # Prob 50%                      #3891.22
                                # LOE edx ecx ebx esi xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.22:                       # Preds ..B11.21
        movdqa    (%ebx), %xmm0                                 #3891.22
        jmp       ..B11.24      # Prob 100%                     #3891.22
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.23:                       # Preds ..B11.21
        movdqu    (%ebx), %xmm0                                 #3891.22
                                # LOE edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.24:                       # Preds ..B11.22 ..B11.23
        addl      $16, %ebx                                     #3969.13
        movl      %ebx, 68(%esp)                                #3969.13
        cmpl      %esi, %ebx                                    #3972.24
        jae       ..B11.26      # Prob 12%                      #3972.24
                                # LOE edx ecx ebx bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.25:                       # Preds ..B11.24
        movzbl    -1(%ebx), %edx                                #3973.32
        shll      $8, %edx                                      #3973.46
                                # LOE edx ecx ebx bl bh xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.26:                       # Preds ..B11.24 ..B11.11 ..B11.25
        movl      $16, %eax                                     #3752.13
        jmp       ..B11.30      # Prob 100%                     #3752.13
                                # LOE eax edx ecx ebx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.27:                       # Preds ..B11.19
        movl      68(%esp), %esi                                #3868.26
        movl      56(%esp), %edi                                #3868.26
        psrldq    $1, %xmm0                                     #3865.26
        cmpl      %edi, %esi                                    #3868.26
        jae       ..B11.29      # Prob 19%                      #3868.26
                                # LOE eax edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.28:                       # Preds ..B11.27
        movzbl    (%esi), %edi                                  #3871.37
        sarl      $8, %edx                                      #3870.21
        shll      $8, %edi                                      #3871.48
        orl       %edi, %edx                                    #3871.21
        pinsrw    $7, %edx, %xmm0                               #3872.30
                                # LOE eax edx ecx ebx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.29:                       # Preds ..B11.28 ..B11.27
        addl      $1, 68(%esp)                                  #3876.20
                                # LOE eax edx ecx ebx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.30:                       # Preds ..B11.26 ..B11.29 ..B11.55
        movdqa    %xmm0, %xmm7                                  #3762.22
        pand      %xmm5, %xmm7                                  #3762.22
        pcmpeqd   %xmm6, %xmm7                                  #3763.22
        pmovmskb  %xmm7, %esi                                   #3764.22
        addl      $1, %esi                                      #3765.17
        shrl      $16, %esi                                     #3765.17
        negl      %esi                                          #3765.17
        movl      %esi, 72(%esp)                                #3765.17
        jmp       ..B11.15      # Prob 100%                     #3765.17
                                # LOE eax edx ecx ebx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
..B11.31:                       # Preds ..B11.17                # Infreq
        movl      %esi, %edx                                    #
        movl      72(%esp), %edi                                #
        movl      64(%esp), %ebx                                #3793.30
        subl      %ecx, %ebx                                    #3793.30
        movl      %ebx, 64(%esp)                                #3793.30
        je        ..B11.52      # Prob 25%                      #3793.37
                                # LOE edx ecx ebx edi dl bl dh bh
..B11.32:                       # Preds ..B11.31                # Infreq
        cmpl      $1, %ebx                                      #3793.37
        jne       ..B11.37      # Prob 67%                      #3793.37
                                # LOE edx ecx ebx edi dl bl dh bh
..B11.33:                       # Preds ..B11.32                # Infreq
        testl     %edi, %edi                                    #3799.36
        jne       ..B11.36      # Prob 28%                      #3799.36
                                # LOE ecx
..B11.34:                       # Preds ..B11.33                # Infreq
        movl      52(%esp), %edx                                #3800.36
        testl     %edx, %edx                                    #3800.36
        je        ..B11.53      # Prob 72%                      #3800.36
                                # LOE ecx
..B11.35:                       # Preds ..B11.45 ..B11.39 ..B11.34 # Infreq
        subl      60(%esp), %ecx                                #3800.47
        addl      $2, %ecx                                      #3800.61
        movl      %ecx, %eax                                    #3800.61
        addl      $84, %esp                                     #3800.61
        popl      %ebx                                          #3800.61
        popl      %esi                                          #3800.61
        popl      %edi                                          #3800.61
        movl      %ebp, %esp                                    #3800.61
        popl      %ebp                                          #3800.61
        ret                                                     #3800.61
                                # LOE
..B11.36:                       # Preds ..B11.52 ..B11.44 ..B11.38 ..B11.33 # Infreq
        subl      60(%esp), %ecx                                #3799.47
        addl      $1, %ecx                                      #3799.61
        movl      %ecx, %eax                                    #3799.61
        addl      $84, %esp                                     #3799.61
        popl      %ebx                                          #3799.61
        popl      %esi                                          #3799.61
        popl      %edi                                          #3799.61
        movl      %ebp, %esp                                    #3799.61
        popl      %ebp                                          #3799.61
        ret                                                     #3799.61
                                # LOE
..B11.37:                       # Preds ..B11.32                # Infreq
        cmpl      $2, %ebx                                      #3793.37
        jne       ..B11.44      # Prob 50%                      #3793.37
                                # LOE edx ecx edi dl dh
..B11.38:                       # Preds ..B11.37                # Infreq
        testl     %edi, %edi                                    #3803.36
        jne       ..B11.36      # Prob 28%                      #3803.36
                                # LOE ecx
..B11.39:                       # Preds ..B11.38                # Infreq
        movl      52(%esp), %edx                                #3804.36
        testl     %edx, %edx                                    #3804.36
        jne       ..B11.35      # Prob 28%                      #3804.36
                                # LOE ecx
..B11.40:                       # Preds ..B11.39                # Infreq
        movl      48(%esp), %edx                                #3805.36
        testl     %edx, %edx                                    #3805.36
        je        ..B11.53      # Prob 72%                      #3805.36
                                # LOE ecx
..B11.41:                       # Preds ..B11.46 ..B11.40       # Infreq
        subl      60(%esp), %ecx                                #3805.47
        addl      $3, %ecx                                      #3805.61
        movl      %ecx, %eax                                    #3805.61
        addl      $84, %esp                                     #3805.61
        popl      %ebx                                          #3805.61
        popl      %esi                                          #3805.61
        popl      %edi                                          #3805.61
        movl      %ebp, %esp                                    #3805.61
        popl      %ebp                                          #3805.61
        ret                                                     #3805.61
                                # LOE
..B11.44:                       # Preds ..B11.37                # Infreq
        testl     %edi, %edi                                    #3808.36
        jne       ..B11.36      # Prob 28%                      #3808.36
                                # LOE edx ecx dl dh
..B11.45:                       # Preds ..B11.44                # Infreq
        movl      52(%esp), %ebx                                #3809.36
        testl     %ebx, %ebx                                    #3809.36
        jne       ..B11.35      # Prob 28%                      #3809.36
                                # LOE edx ecx dl dh
..B11.46:                       # Preds ..B11.45                # Infreq
        movl      48(%esp), %ebx                                #3810.36
        testl     %ebx, %ebx                                    #3810.36
        jne       ..B11.41      # Prob 28%                      #3810.36
                                # LOE edx ecx dl dh
..B11.47:                       # Preds ..B11.46                # Infreq
        testl     %edx, %edx                                    #3811.36
        je        ..B11.53      # Prob 72%                      #3811.36
                                # LOE ecx
..B11.48:                       # Preds ..B11.47                # Infreq
        subl      60(%esp), %ecx                                #3811.47
        addl      $4, %ecx                                      #3811.61
        movl      %ecx, %eax                                    #3811.61
        addl      $84, %esp                                     #3811.61
        popl      %ebx                                          #3811.61
        popl      %esi                                          #3811.61
        popl      %edi                                          #3811.61
        movl      %ebp, %esp                                    #3811.61
        popl      %ebp                                          #3811.61
        ret                                                     #3811.61
                                # LOE
..B11.52:                       # Preds ..B11.31                # Infreq
        testl     %edi, %edi                                    #3796.36
        jne       ..B11.36      # Prob 28%                      #3796.36
                                # LOE ecx
..B11.53:                       # Preds ..B11.20 ..B11.18 ..B11.52 ..B11.34 ..B11.40
                                #       ..B11.47                # Infreq
        xorl      %eax, %eax                                    #3813.28
        addl      $84, %esp                                     #3813.28
        popl      %ebx                                          #3813.28
        popl      %esi                                          #3813.28
        popl      %edi                                          #3813.28
        movl      %ebp, %esp                                    #3813.28
        popl      %ebp                                          #3813.28
        ret                                                     #3813.28
                                # LOE
..B11.55:                       # Preds ..B11.7                 # Infreq
        movdqa    %xmm7, (%esp)                                 #
        movdqa    %xmm0, 16(%esp)                               #
        movdqa    32(%esp), %xmm0                               #
        movl      %esi, 56(%esp)                                #
        jmp       ..B11.30      # Prob 100%                     #
        .align    2,0x90
                                # LOE eax edx ecx ebx xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6
# mark_end;
	.type	eval_2na_pos,@function
	.size	eval_2na_pos,.-eval_2na_pos
	.data
# -- End  eval_2na_pos
	.text
# -- Begin  eval_4na_pos
# mark_begin;
       .align    2,0x90
eval_4na_pos:
# parameter 1: %eax
# parameter 2: %edx
# parameter 3: %ecx
# parameter 4: 20 + %ebp
..B12.1:                        # Preds ..B12.0
        pushl     %ebp                                          #4946.1
        movl      %esp, %ebp                                    #4946.1
        andl      $-16, %esp                                    #4946.1
        pushl     %edi                                          #4946.1
        pushl     %esi                                          #4946.1
        pushl     %ebx                                          #4946.1
        subl      $116, %esp                                    #4946.1
        movl      20(%ebp), %esi                                #4944.5
        movl      %ecx, 104(%esp)                               #4946.1
        movl      %ecx, 92(%esp)                                #4946.1
        movdqa    16(%eax), %xmm5                               #5009.5
        movdqa    32(%eax), %xmm4                               #5009.5
        movdqa    48(%eax), %xmm3                               #5009.5
        movdqa    64(%eax), %xmm2                               #5009.5
        movdqa    80(%eax), %xmm1                               #5009.5
        movdqa    96(%eax), %xmm6                               #5009.5
        movdqa    128(%eax), %xmm7                              #5009.5
        movl      %ecx, %edi                                    #4993.49
        shrl      $2, %edi                                      #4993.49
        call      ..L6          # Prob 100%                     #4946.1
..L6:                                                           #
        popl      %ebx                                          #4946.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L6], %ebx       #4946.1
        movl      %ebx, 16(%esp)                                #4946.1
        lea       (%ecx,%esi), %ebx                             #4985.5
        subl      4(%eax), %ebx                                 #4996.12
        lea       3(%ecx,%esi), %ecx                            #4999.50
        movl      16(%esp), %esi                                #5002.14
        movl      %ebx, 100(%esp)                               #4996.12
        shrl      $2, %ecx                                      #4999.57
        addl      %edx, %ecx                                    #4999.30
        movl      %ecx, 96(%esp)                                #4999.30
        movzbl    (%edi,%edx), %ecx                             #5002.14
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #5002.14
        movw      %cx, (%esp)                                   #5002.14
        movzbl    1(%edi,%edx), %ecx                            #5002.14
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #5002.14
        movw      %cx, 2(%esp)                                  #5002.14
        movzbl    2(%edi,%edx), %ecx                            #5002.14
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #5002.14
        movw      %cx, 4(%esp)                                  #5002.14
        movzbl    3(%edi,%edx), %ecx                            #5002.14
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #5002.14
        movw      %cx, 6(%esp)                                  #5002.14
        movzbl    4(%edi,%edx), %ecx                            #5002.14
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #5002.14
        movw      %cx, 8(%esp)                                  #5002.14
        movzbl    5(%edi,%edx), %ecx                            #5002.14
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #5002.14
        movw      %cx, 10(%esp)                                 #5002.14
        movzbl    6(%edi,%edx), %ecx                            #5002.14
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #5002.14
        movw      %cx, 12(%esp)                                 #5002.14
        movzbl    7(%edi,%edx), %ecx                            #5002.14
        movzwl    expand_2na.0@GOTOFF(%esi,%ecx,2), %ecx        #5002.14
        movw      %cx, 14(%esp)                                 #5002.14
        movdqu    (%esp), %xmm0                                 #5002.14
        lea       8(%edx,%edi), %ecx                            #5003.5
        movdqa    %xmm0, 64(%esp)                               #5002.14
        movdqa    112(%eax), %xmm0                              #5009.5
        movl      %ecx, %edx                                    #5003.5
        movl      104(%esp), %eax                               #5014.20
        subl      %eax, %ebx                                    #5014.20
        addl      $7, %ebx                                      #5014.33
        shrl      $2, %ebx                                      #5014.40
        movl      %eax, %edi                                    #5023.20
        andl      $3, %edi                                      #5023.20
        je        ..B12.45      # Prob 20%                      #5023.20
                                # LOE eax edx ecx ebx esi edi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B12.2:                        # Preds ..B12.1
        cmpl      $1, %edi                                      #5023.20
        je        ..B12.8       # Prob 25%                      #5023.20
                                # LOE eax edx ecx ebx esi edi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B12.3:                        # Preds ..B12.2
        cmpl      $2, %edi                                      #5023.20
        je        ..B12.7       # Prob 33%                      #5023.20
                                # LOE eax edx ecx ebx esi edi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B12.4:                        # Preds ..B12.3
        cmpl      $3, %edi                                      #5023.20
        je        ..B12.6       # Prob 50%                      #5023.20
                                # LOE eax edx ecx ebx esi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7
..B12.5:                        # Preds ..B12.4
        movdqa    %xmm7, 16(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    64(%esp), %xmm0                               #
        movdqa    %xmm6, 48(%esp)                               #
        jmp       ..B12.16      # Prob 100%                     #
                                # LOE eax edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.6:                        # Preds ..B12.4
        movdqa    %xmm7, 16(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    64(%esp), %xmm0                               #
        movdqa    %xmm6, 48(%esp)                               #
        xorl      %edi, %edi                                    #
        movl      %edi, 80(%esp)                                #
        movl      %edi, 84(%esp)                                #
        movl      %edi, 88(%esp)                                #
        jmp       ..B12.11      # Prob 100%                     #
                                # LOE eax edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.7:                        # Preds ..B12.3
        movdqa    %xmm7, 16(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    64(%esp), %xmm0                               #
        movdqa    %xmm6, 48(%esp)                               #
        xorl      %edi, %edi                                    #
        movl      %edi, 84(%esp)                                #
        movl      %edi, 88(%esp)                                #
        jmp       ..B12.10      # Prob 100%                     #
                                # LOE eax edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.8:                        # Preds ..B12.2
        movdqa    %xmm7, 16(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    64(%esp), %xmm0                               #
        movdqa    %xmm6, 48(%esp)                               #
        xorl      %edi, %edi                                    #
        movl      %edi, 88(%esp)                                #
                                # LOE eax edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.9:                        # Preds ..B12.8 ..B12.20
        movdqa    %xmm0, %xmm7                                  #5047.22
        movdqa    %xmm0, %xmm6                                  #5048.22
        pand      %xmm3, %xmm7                                  #5047.22
        pand      %xmm2, %xmm6                                  #5048.22
        pcmpeqd   %xmm6, %xmm7                                  #5049.22
        pmovmskb  %xmm7, %edi                                   #5050.22
        addl      $1, %edi                                      #5051.17
        shrl      $16, %edi                                     #5051.17
        negl      %edi                                          #5051.17
        movl      %edi, 84(%esp)                                #5051.17
                                # LOE eax edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.10:                       # Preds ..B12.7 ..B12.9
        movdqa    %xmm0, %xmm7                                  #5054.22
        movdqa    %xmm0, %xmm6                                  #5055.22
        pand      48(%esp), %xmm6                               #5055.22
        pand      %xmm1, %xmm7                                  #5054.22
        pcmpeqd   %xmm6, %xmm7                                  #5056.22
        pmovmskb  %xmm7, %edi                                   #5057.22
        addl      $1, %edi                                      #5058.17
        shrl      $16, %edi                                     #5058.17
        negl      %edi                                          #5058.17
        movl      %edi, 80(%esp)                                #5058.17
                                # LOE eax edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.11:                       # Preds ..B12.6 ..B12.10
        movdqa    %xmm0, %xmm7                                  #5061.22
        movdqa    %xmm0, %xmm6                                  #5062.22
        pand      32(%esp), %xmm7                               #5061.22
        pand      16(%esp), %xmm6                               #5062.22
        pcmpeqd   %xmm6, %xmm7                                  #5063.22
        pmovmskb  %xmm7, %edi                                   #5064.22
        andl      $-4, %eax                                     #5069.17
        movl      %eax, 104(%esp)                               #5069.17
        addl      $1, %edi                                      #5065.17
        shrl      $16, %edi                                     #5065.17
        movl      88(%esp), %eax                                #5072.29
        negl      %edi                                          #5065.17
        movl      %edi, 64(%esp)                                #5065.17
        orl       84(%esp), %eax                                #5072.29
        orl       80(%esp), %eax                                #5072.34
        orl       %edi, %eax                                    #5072.39
        movl      104(%esp), %eax                               #5072.47
        jne       ..B12.21      # Prob 1%                       #5072.47
                                # LOE eax edx ecx ebx esi edi al ah xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.12:                       # Preds ..B12.11
        movl      100(%esp), %edi                               #5138.28
        addl      $4, %eax                                      #5135.17
        cmpl      %edi, %eax                                    #5138.28
        ja        ..B12.43      # Prob 1%                       #5138.28
                                # LOE eax edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.13:                       # Preds ..B12.12
        addl      $-1, %ebx                                     #5142.25
        jne       ..B12.17      # Prob 50%                      #5142.39
                                # LOE eax edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.14:                       # Preds ..B12.13
        movl      96(%esp), %edx                                #5159.25
        cmpl      %edx, %ecx                                    #5159.25
        jae       ..B12.43      # Prob 1%                       #5159.25
                                # LOE eax ecx esi xmm1 xmm2 xmm3 xmm4 xmm5
..B12.15:                       # Preds ..B12.14
        movzbl    (%ecx), %edx                                  #5169.22
        movzwl    expand_2na.0@GOTOFF(%esi,%edx,2), %ebx        #5169.22
        movzbl    1(%ecx), %edi                                 #5169.22
        movzwl    expand_2na.0@GOTOFF(%esi,%edi,2), %edx        #5169.22
        movw      %bx, (%esp)                                   #5169.22
        movw      %dx, 2(%esp)                                  #5169.22
        movzbl    2(%ecx), %edx                                 #5169.22
        movzwl    expand_2na.0@GOTOFF(%esi,%edx,2), %edx        #5169.22
        movw      %dx, 4(%esp)                                  #5169.22
        movzbl    3(%ecx), %edx                                 #5169.22
        movzwl    expand_2na.0@GOTOFF(%esi,%edx,2), %edx        #5169.22
        movw      %dx, 6(%esp)                                  #5169.22
        movzbl    4(%ecx), %edx                                 #5169.22
        movzwl    expand_2na.0@GOTOFF(%esi,%edx,2), %edx        #5169.22
        movw      %dx, 8(%esp)                                  #5169.22
        movzbl    5(%ecx), %edx                                 #5169.22
        movzwl    expand_2na.0@GOTOFF(%esi,%edx,2), %edx        #5169.22
        movw      %dx, 10(%esp)                                 #5169.22
        movzbl    6(%ecx), %edx                                 #5169.22
        movzwl    expand_2na.0@GOTOFF(%esi,%edx,2), %edx        #5169.22
        movw      %dx, 12(%esp)                                 #5169.22
        movzbl    7(%ecx), %edx                                 #5169.22
        movzwl    expand_2na.0@GOTOFF(%esi,%edx,2), %edx        #5169.22
        addl      $8, %ecx                                      #5184.13
        movw      %dx, 14(%esp)                                 #5169.22
        movdqu    (%esp), %xmm0                                 #5169.22
        movl      %ecx, %edx                                    #5184.13
                                # LOE eax edx ecx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.16:                       # Preds ..B12.5 ..B12.15
        movl      $8, %ebx                                      #5030.13
        jmp       ..B12.20      # Prob 100%                     #5030.13
                                # LOE eax edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.17:                       # Preds ..B12.13
        movl      96(%esp), %edi                                #5150.26
        psrldq    $2, %xmm0                                     #5147.26
        cmpl      %edi, %edx                                    #5150.26
        jae       ..B12.19      # Prob 19%                      #5150.26
                                # LOE eax edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.18:                       # Preds ..B12.17
        movzbl    (%edx), %edi                                  #5151.72
        movzwl    expand_2na.0@GOTOFF(%esi,%edi,2), %edi        #5151.57
        pinsrw    $7, %edi, %xmm0                               #5151.30
                                # LOE eax edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.19:                       # Preds ..B12.18 ..B12.17
        addl      $1, %edx                                      #5154.20
                                # LOE eax edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.20:                       # Preds ..B12.16 ..B12.19 ..B12.45
        movdqa    %xmm0, %xmm7                                  #5040.22
        movdqa    %xmm0, %xmm6                                  #5041.22
        pand      %xmm5, %xmm7                                  #5040.22
        pand      %xmm4, %xmm6                                  #5041.22
        pcmpeqd   %xmm6, %xmm7                                  #5042.22
        pmovmskb  %xmm7, %edi                                   #5043.22
        addl      $1, %edi                                      #5044.17
        shrl      $16, %edi                                     #5044.17
        negl      %edi                                          #5044.17
        movl      %edi, 88(%esp)                                #5044.17
        jmp       ..B12.9       # Prob 100%                     #5044.17
                                # LOE eax edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
..B12.21:                       # Preds ..B12.11                # Infreq
        movl      %edi, %edx                                    #
        movl      88(%esp), %edi                                #
        movl      100(%esp), %ecx                               #5075.30
        subl      %eax, %ecx                                    #5075.30
        movl      %ecx, 100(%esp)                               #5075.30
        je        ..B12.42      # Prob 25%                      #5075.37
                                # LOE eax edx ecx edi al dl cl ah dh ch
..B12.22:                       # Preds ..B12.21                # Infreq
        cmpl      $1, %ecx                                      #5075.37
        jne       ..B12.27      # Prob 67%                      #5075.37
                                # LOE eax edx ecx edi al dl cl ah dh ch
..B12.23:                       # Preds ..B12.22                # Infreq
        testl     %edi, %edi                                    #5081.36
        jne       ..B12.26      # Prob 28%                      #5081.36
                                # LOE eax al ah
..B12.24:                       # Preds ..B12.23                # Infreq
        movl      84(%esp), %edx                                #5082.36
        testl     %edx, %edx                                    #5082.36
        je        ..B12.43      # Prob 72%                      #5082.36
                                # LOE eax al ah
..B12.25:                       # Preds ..B12.35 ..B12.29 ..B12.24 # Infreq
        subl      92(%esp), %eax                                #5082.47
        addl      $2, %eax                                      #5082.61
        addl      $116, %esp                                    #5082.61
        popl      %ebx                                          #5082.61
        popl      %esi                                          #5082.61
        popl      %edi                                          #5082.61
        movl      %ebp, %esp                                    #5082.61
        popl      %ebp                                          #5082.61
        ret                                                     #5082.61
                                # LOE
..B12.26:                       # Preds ..B12.42 ..B12.34 ..B12.28 ..B12.23 # Infreq
        subl      92(%esp), %eax                                #5081.47
        addl      $1, %eax                                      #5081.61
        addl      $116, %esp                                    #5081.61
        popl      %ebx                                          #5081.61
        popl      %esi                                          #5081.61
        popl      %edi                                          #5081.61
        movl      %ebp, %esp                                    #5081.61
        popl      %ebp                                          #5081.61
        ret                                                     #5081.61
                                # LOE
..B12.27:                       # Preds ..B12.22                # Infreq
        cmpl      $2, %ecx                                      #5075.37
        jne       ..B12.34      # Prob 50%                      #5075.37
                                # LOE eax edx edi al dl ah dh
..B12.28:                       # Preds ..B12.27                # Infreq
        testl     %edi, %edi                                    #5085.36
        jne       ..B12.26      # Prob 28%                      #5085.36
                                # LOE eax al ah
..B12.29:                       # Preds ..B12.28                # Infreq
        movl      84(%esp), %edx                                #5086.36
        testl     %edx, %edx                                    #5086.36
        jne       ..B12.25      # Prob 28%                      #5086.36
                                # LOE eax al ah
..B12.30:                       # Preds ..B12.29                # Infreq
        movl      80(%esp), %edx                                #5087.36
        testl     %edx, %edx                                    #5087.36
        je        ..B12.43      # Prob 72%                      #5087.36
                                # LOE eax al ah
..B12.31:                       # Preds ..B12.36 ..B12.30       # Infreq
        subl      92(%esp), %eax                                #5087.47
        addl      $3, %eax                                      #5087.61
        addl      $116, %esp                                    #5087.61
        popl      %ebx                                          #5087.61
        popl      %esi                                          #5087.61
        popl      %edi                                          #5087.61
        movl      %ebp, %esp                                    #5087.61
        popl      %ebp                                          #5087.61
        ret                                                     #5087.61
                                # LOE
..B12.34:                       # Preds ..B12.27                # Infreq
        testl     %edi, %edi                                    #5090.36
        jne       ..B12.26      # Prob 28%                      #5090.36
                                # LOE eax edx al dl ah dh
..B12.35:                       # Preds ..B12.34                # Infreq
        movl      84(%esp), %ecx                                #5091.36
        testl     %ecx, %ecx                                    #5091.36
        jne       ..B12.25      # Prob 28%                      #5091.36
                                # LOE eax edx al dl ah dh
..B12.36:                       # Preds ..B12.35                # Infreq
        movl      80(%esp), %ecx                                #5092.36
        testl     %ecx, %ecx                                    #5092.36
        jne       ..B12.31      # Prob 28%                      #5092.36
                                # LOE eax edx al dl ah dh
..B12.37:                       # Preds ..B12.36                # Infreq
        testl     %edx, %edx                                    #5093.36
        je        ..B12.43      # Prob 72%                      #5093.36
                                # LOE eax al ah
..B12.38:                       # Preds ..B12.37                # Infreq
        subl      92(%esp), %eax                                #5093.47
        addl      $4, %eax                                      #5093.61
        addl      $116, %esp                                    #5093.61
        popl      %ebx                                          #5093.61
        popl      %esi                                          #5093.61
        popl      %edi                                          #5093.61
        movl      %ebp, %esp                                    #5093.61
        popl      %ebp                                          #5093.61
        ret                                                     #5093.61
                                # LOE
..B12.42:                       # Preds ..B12.21                # Infreq
        testl     %edi, %edi                                    #5078.36
        jne       ..B12.26      # Prob 28%                      #5078.36
                                # LOE eax al ah
..B12.43:                       # Preds ..B12.14 ..B12.12 ..B12.42 ..B12.24 ..B12.30
                                #       ..B12.37                # Infreq
        xorl      %eax, %eax                                    #5095.28
        addl      $116, %esp                                    #5095.28
        popl      %ebx                                          #5095.28
        popl      %esi                                          #5095.28
        popl      %edi                                          #5095.28
        movl      %ebp, %esp                                    #5095.28
        popl      %ebp                                          #5095.28
        ret                                                     #5095.28
                                # LOE
..B12.45:                       # Preds ..B12.1                 # Infreq
        movdqa    %xmm7, 16(%esp)                               #
        movdqa    %xmm0, 32(%esp)                               #
        movdqa    64(%esp), %xmm0                               #
        movdqa    %xmm6, 48(%esp)                               #
        jmp       ..B12.20      # Prob 100%                     #
        .align    2,0x90
                                # LOE eax edx ecx ebx esi xmm0 xmm1 xmm2 xmm3 xmm4 xmm5
# mark_end;
	.type	eval_4na_pos,@function
	.size	eval_4na_pos,.-eval_4na_pos
	.data
# -- End  eval_4na_pos
	.text
# -- Begin  NucStrstrMake
# mark_begin;
       .align    2,0x90
	.globl NucStrstrMake
NucStrstrMake:
# parameter 1: 44 + %esp
# parameter 2: 48 + %esp
# parameter 3: 52 + %esp
# parameter 4: 56 + %esp
..B13.1:                        # Preds ..B13.0
        pushl     %edi                                          #1539.1
        pushl     %esi                                          #1539.1
        pushl     %ebp                                          #1539.1
        pushl     %ebx                                          #1539.1
        subl      $24, %esp                                     #1539.1
        movl      44(%esp), %esi                                #1537.5
        testl     %esi, %esi                                    #1540.17
        movl      52(%esp), %ebp                                #1537.5
        movl      56(%esp), %edi                                #1537.5
        je        ..B13.12      # Prob 6%                       #1540.17
                                # LOE ebp esi edi
..B13.2:                        # Preds ..B13.1
        testl     %ebp, %ebp                                    #1542.23
        je        ..B13.11      # Prob 12%                      #1542.23
                                # LOE ebp esi edi
..B13.3:                        # Preds ..B13.2
        testl     %edi, %edi                                    #1542.38
        je        ..B13.11      # Prob 50%                      #1542.38
                                # LOE ebp esi edi
..B13.4:                        # Preds ..B13.3
        movl      $0, 20(%esp)                                  #1544.24
        call      ..L7          # Prob 100%                     #
..L7:                                                           #
        popl      %ebx                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L7], %ebx       #
        movsbl    fasta_2na_map.0@GOTOFF(%ebx), %eax            #1547.18
        testl     %eax, %eax                                    #1547.41
        je        ..B13.13      # Prob 5%                       #1547.41
                                # LOE ebx ebp esi edi
..B13.5:                        # Preds ..B13.13 ..B13.4
        lea       20(%esp), %eax                                #1551.49
        movl      %eax, 12(%esp)                                #1551.49
        movl      48(%esp), %eax                                #1539.1
        lea       (%edi,%ebp), %edi                             #1550.19
        movl      %edi, %edx                                    #1551.21
        movl      %eax, 16(%esp)                                #1539.1
        movl      %ebp, %eax                                    #1551.21
        movl      %esi, %ecx                                    #1551.21
        call      nss_expr                                      #1551.21
                                # LOE eax ebx esi edi
..B13.6:                        # Preds ..B13.5
        movl      20(%esp), %edx                                #1552.18
        testl     %edx, %edx                                    #1552.28
        jne       ..B13.9       # Prob 22%                      #1552.28
                                # LOE eax ebx esi edi
..B13.7:                        # Preds ..B13.6
        cmpl      %edi, %eax                                    #1554.31
        je        ..B13.14      # Prob 5%                       #1554.31
                                # LOE ebx esi
..B13.8:                        # Preds ..B13.7
        movl      $22, 20(%esp)                                 #1557.17
                                # LOE ebx esi
..B13.9:                        # Preds ..B13.8 ..B13.6
        pushl     (%esi)                                        #1560.32
        call      NucStrstrWhack@PLT                            #1560.13
                                # LOE esi
..B13.10:                       # Preds ..B13.9
        movl      24(%esp), %eax                                #1562.20
        movl      $0, (%esi)                                    #1561.13
        addl      $28, %esp                                     #1562.20
        popl      %ebx                                          #1562.20
        popl      %ebp                                          #1562.20
        popl      %esi                                          #1562.20
        popl      %edi                                          #1562.20
        ret                                                     #1562.20
                                # LOE
..B13.11:                       # Preds ..B13.3 ..B13.2
        movl      $0, (%esi)                                    #1565.9
                                # LOE
..B13.12:                       # Preds ..B13.11 ..B13.1
        movl      $22, %eax                                     #1567.12
        addl      $24, %esp                                     #1567.12
        popl      %ebx                                          #1567.12
        popl      %ebp                                          #1567.12
        popl      %esi                                          #1567.12
        popl      %edi                                          #1567.12
        ret                                                     #1567.12
                                # LOE
..B13.13:                       # Preds ..B13.4                 # Infreq
        call      NucStrstrInit                                 #1548.17
        jmp       ..B13.5       # Prob 100%                     #1548.17
                                # LOE ebx ebp esi edi
..B13.14:                       # Preds ..B13.7                 # Infreq
        xorl      %eax, %eax                                    #1555.28
        addl      $24, %esp                                     #1555.28
        popl      %ebx                                          #1555.28
        popl      %ebp                                          #1555.28
        popl      %esi                                          #1555.28
        popl      %edi                                          #1555.28
        ret                                                     #1555.28
        .align    2,0x90
                                # LOE
# mark_end;
	.type	NucStrstrMake,@function
	.size	NucStrstrMake,.-NucStrstrMake
	.data
# -- End  NucStrstrMake
	.text
# -- Begin  nss_expr
# mark_begin;
       .align    2,0x90
nss_expr:
# parameter 1: %eax
# parameter 2: %edx
# parameter 3: %ecx
# parameter 4: 68 + %esp
# parameter 5: 72 + %esp
..B14.1:                        # Preds ..B14.0
        pushl     %edi                                          #1406.1
        pushl     %esi                                          #1406.1
        pushl     %ebp                                          #1406.1
        pushl     %ebx                                          #1406.1
        subl      $36, %esp                                     #1406.1
        movl      %ecx, %edi                                    #1406.1
        movl      %edx, %esi                                    #1406.1
        movl      %eax, %ebp                                    #1406.1
        call      ..L8          # Prob 100%                     #1406.1
..L8:                                                           #
        popl      %ebx                                          #1406.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L8], %ebx       #1406.1
        call      __ctype_b_loc@PLT                             #1409.9
                                # LOE eax ebx ebp esi edi
..B14.41:                       # Preds ..B14.1
        movl      %ebx, 28(%esp)                                #1409.9
        movl      %eax, 32(%esp)                                #1409.9
        movl      %ebp, %edx                                    #1409.9
        movl      68(%esp), %ebp                                #1409.9
        jmp       ..B14.2       # Prob 100%                     #1409.9
                                # LOE edx ebp esi edi
..B14.29:                       # Preds ..B14.28
        movl      20(%esp), %ecx                                #1459.17
        movl      $11, (%eax)                                   #1458.17
        movl      %ecx, 4(%eax)                                 #1459.17
        movl      (%edi), %ebx                                  #1460.41
        movl      %ebx, 8(%eax)                                 #1460.17
        movl      %eax, (%edi)                                  #1461.17
        lea       12(%eax), %edi                                #1464.42
                                # LOE edx ebp esi edi
..B14.2:                        # Preds ..B14.41 ..B14.29
        movl      $0, (%edi)                                    #1407.5
        cmpl      %esi, %edx                                    #1409.9
        jae       ..B14.8       # Prob 12%                      #1409.9
                                # LOE edx ebp esi edi
..B14.3:                        # Preds ..B14.2
        movl      32(%esp), %ecx                                #1409.9
        movl      (%ecx), %ecx                                  #1409.9
        movzbl    (%edx), %ebx                                  #1409.9
        movzwl    (%ecx,%ebx,2), %ebx                           #1409.9
        testl     $8192, %ebx                                   #1409.9
        je        ..B14.8       # Prob 15%                      #1409.9
                                # LOE edx ecx ebp esi edi
..B14.5:                        # Preds ..B14.3 ..B14.6
        addl      $1, %edx                                      #1409.9
        cmpl      %esi, %edx                                    #1409.9
        jae       ..B14.8       # Prob 1%                       #1409.9
                                # LOE edx ecx ebp esi edi
..B14.6:                        # Preds ..B14.5
        movzbl    (%edx), %ebx                                  #1409.9
        movzwl    (%ecx,%ebx,2), %ebx                           #1409.9
        testl     $8192, %ebx                                   #1409.9
        jne       ..B14.5       # Prob 82%                      #1409.9
                                # LOE edx ecx ebp esi edi
..B14.8:                        # Preds ..B14.5 ..B14.6 ..B14.3 ..B14.2
        cmpl      %esi, %edx                                    #1410.15
        je        ..B14.32      # Prob 16%                      #1410.15
                                # LOE edx ebp esi edi
..B14.9:                        # Preds ..B14.8
        movl      72(%esp), %ebx                                #1406.1
        movl      %ebp, 12(%esp)                                #1406.1
        movl      %ebx, 16(%esp)                                #1406.1
        movl      %edx, %eax                                    #1412.13
        movl      %esi, %edx                                    #1412.13
        movl      %edi, %ecx                                    #1412.13
        call      nss_unary_expr                                #1412.13
                                # LOE eax ebp esi edi
..B14.42:                       # Preds ..B14.9
        movl      %eax, %edx                                    #1412.13
                                # LOE edx ebp esi edi
..B14.10:                       # Preds ..B14.42
        movl      (%ebp), %ecx                                  #1413.16
        testl     %ecx, %ecx                                    #1413.26
        jne       ..B14.32      # Prob 58%                      #1413.26
                                # LOE edx ebp esi edi
..B14.11:                       # Preds ..B14.10
        cmpl      %esi, %edx                                    #1415.17
        jae       ..B14.17      # Prob 12%                      #1415.17
                                # LOE edx ebp esi edi
..B14.12:                       # Preds ..B14.11
        movl      32(%esp), %ecx                                #1415.17
        movl      (%ecx), %ecx                                  #1415.17
        movzbl    (%edx), %ebx                                  #1415.17
        movzwl    (%ecx,%ebx,2), %ebx                           #1415.17
        testl     $8192, %ebx                                   #1415.17
        je        ..B14.17      # Prob 15%                      #1415.17
                                # LOE edx ecx ebp esi edi
..B14.14:                       # Preds ..B14.12 ..B14.15
        addl      $1, %edx                                      #1415.17
        cmpl      %esi, %edx                                    #1415.17
        jae       ..B14.17      # Prob 1%                       #1415.17
                                # LOE edx ecx ebp esi edi
..B14.15:                       # Preds ..B14.14
        movzbl    (%edx), %ebx                                  #1415.17
        movzwl    (%ecx,%ebx,2), %ebx                           #1415.17
        testl     $8192, %ebx                                   #1415.17
        jne       ..B14.14      # Prob 82%                      #1415.17
                                # LOE edx ecx ebp esi edi
..B14.17:                       # Preds ..B14.14 ..B14.15 ..B14.12 ..B14.11
        cmpl      %esi, %edx                                    #1416.23
        je        ..B14.32      # Prob 4%                       #1416.23
                                # LOE edx ebp esi edi
..B14.18:                       # Preds ..B14.17
        movsbl    (%edx), %ecx                                  #1422.28
        addl      $1, %edx                                      #1422.28
        cmpl      $41, %ecx                                     #1422.28
        je        ..B14.31      # Prob 25%                      #1422.28
                                # LOE edx ecx ebp esi edi
..B14.19:                       # Preds ..B14.18
        cmpl      $38, %ecx                                     #1422.28
        jne       ..B14.23      # Prob 67%                      #1422.28
                                # LOE edx ecx ebp esi edi
..B14.20:                       # Preds ..B14.19
        cmpl      %esi, %edx                                    #1427.30
        jae       ..B14.22      # Prob 12%                      #1427.30
                                # LOE edx ebp esi edi
..B14.21:                       # Preds ..B14.20
        movsbl    (%edx), %ecx                                  #1427.39
        lea       1(%edx), %ebx                                 #1428.28
        cmpl      $38, %ecx                                     #1428.28
        cmove     %ebx, %edx                                    #1428.28
                                # LOE edx ebp esi edi
..B14.22:                       # Preds ..B14.21 ..B14.20
        movl      $16, %ecx                                     #1429.21
        movl      %ecx, 20(%esp)                                #1429.21
        jmp       ..B14.27      # Prob 100%                     #1429.21
                                # LOE edx ebp esi edi
..B14.23:                       # Preds ..B14.19
        cmpl      $124, %ecx                                    #1422.28
        jne       ..B14.30      # Prob 50%                      #1422.28
                                # LOE edx ebp esi edi
..B14.24:                       # Preds ..B14.23
        cmpl      %esi, %edx                                    #1432.30
        jae       ..B14.26      # Prob 12%                      #1432.30
                                # LOE edx ebp esi edi
..B14.25:                       # Preds ..B14.24
        movsbl    (%edx), %ecx                                  #1432.39
        lea       1(%edx), %ebx                                 #1433.28
        cmpl      $124, %ecx                                    #1433.28
        cmove     %ebx, %edx                                    #1433.28
                                # LOE edx ebp esi edi
..B14.26:                       # Preds ..B14.25 ..B14.24
        movl      $17, %ecx                                     #1434.21
        movl      %ecx, 20(%esp)                                #1434.21
                                # LOE edx ebp esi edi
..B14.27:                       # Preds ..B14.22 ..B14.26
        pushl     $16                                           #1451.30
        movl      32(%esp), %ebx                                #1451.21
        movl      %edx, 28(%esp)                                #1451.21
        call      malloc@PLT                                    #1451.21
                                # LOE eax ebp esi edi
..B14.43:                       # Preds ..B14.27
        movl      28(%esp), %edx                                #
        addl      $4, %esp                                      #1451.21
                                # LOE eax edx ebp esi edi dl dh
..B14.28:                       # Preds ..B14.43
        testl     %eax, %eax                                    #1452.27
        je        ..B14.38      # Prob 1%                       #1452.27
        jmp       ..B14.29      # Prob 100%                     #1452.27
                                # LOE eax edx ebp esi edi dl dh
..B14.30:                       # Preds ..B14.23
        movl      68(%esp), %ecx                                #1438.21
        movl      $22, (%ecx)                                   #1438.21
        movl      %edx, %ebp                                    #
        addl      $-1, %ebp                                     #1439.32
        movl      %ebp, %eax                                    #1439.32
        addl      $36, %esp                                     #1439.32
        popl      %ebx                                          #1439.32
        popl      %ebp                                          #1439.32
        popl      %esi                                          #1439.32
        popl      %edi                                          #1439.32
        ret                                                     #1439.32
                                # LOE
..B14.31:                       # Preds ..B14.18
        movl      %edx, %ebp                                    #
        addl      $-1, %ebp                                     #1425.32
        movl      %ebp, %eax                                    #1425.32
        addl      $36, %esp                                     #1425.32
        popl      %ebx                                          #1425.32
        popl      %ebp                                          #1425.32
        popl      %esi                                          #1425.32
        popl      %edi                                          #1425.32
        ret                                                     #1425.32
                                # LOE
..B14.32:                       # Preds ..B14.8 ..B14.17 ..B14.10
        movl      %edx, %ebp                                    #
                                # LOE ebp
..B14.33:                       # Preds ..B14.32
        movl      %ebp, %eax                                    #1470.12
        addl      $36, %esp                                     #1470.12
        popl      %ebx                                          #1470.12
        popl      %ebp                                          #1470.12
        popl      %esi                                          #1470.12
        popl      %edi                                          #1470.12
        ret                                                     #1470.12
                                # LOE
..B14.38:                       # Preds ..B14.28                # Infreq
        movl      28(%esp), %ebx                                #
        movl      %edx, %ebp                                    #
        call      __errno_location@PLT                          #1454.32
                                # LOE eax ebp
..B14.44:                       # Preds ..B14.38                # Infreq
        movl      68(%esp), %ebx                                #1454.21
        movl      (%eax), %ecx                                  #1454.32
        movl      %ecx, (%ebx)                                  #1454.21
        movl      %ebp, %eax                                    #1451.21
        addl      $36, %esp                                     #1455.28
        popl      %ebx                                          #1455.28
        popl      %ebp                                          #1455.28
        popl      %esi                                          #1455.28
        popl      %edi                                          #1455.28
        ret                                                     #1455.28
        .align    2,0x90
                                # LOE
# mark_end;
	.type	nss_expr,@function
	.size	nss_expr,.-nss_expr
	.data
# -- End  nss_expr
	.text
# -- Begin  nss_unary_expr
# mark_begin;
       .align    2,0x90
nss_unary_expr:
# parameter 1: %eax
# parameter 2: %edx
# parameter 3: %ecx
# parameter 4: 56 + %esp
# parameter 5: 60 + %esp
..B15.1:                        # Preds ..B15.0
        pushl     %edi                                          #1361.1
        pushl     %esi                                          #1361.1
        pushl     %ebp                                          #1361.1
        pushl     %ebx                                          #1361.1
        subl      $24, %esp                                     #1361.1
        movl      %ecx, %edi                                    #1361.1
        movl      %edx, %esi                                    #1361.1
        movl      %eax, %ebp                                    #1361.1
        call      ..L9          # Prob 100%                     #1361.1
..L9:                                                           #
        popl      %ebx                                          #1361.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L9], %ebx       #1361.1
        jmp       ..B15.2       # Prob 100%                     #1361.1
                                # LOE ebx ebp esi edi
..B15.43:                       # Preds ..B15.42
        movl      $12, (%eax)                                   #1384.13
        movl      $13, 4(%eax)                                  #1385.13
        movl      $0, 8(%eax)                                   #1386.13
        movl      %eax, (%edi)                                  #1387.13
        lea       8(%eax), %edi                                 #1389.44
                                # LOE ebx ebp esi edi
..B15.2:                        # Preds ..B15.43 ..B15.1
        movsbl    (%ebp), %eax                                  #1363.12
        cmpl      $33, %eax                                     #1363.17
        je        ..B15.32      # Prob 16%                      #1363.17
                                # LOE eax ebx ebp esi edi
..B15.3:                        # Preds ..B15.2
        cmpl      $94, %eax                                     #1364.16
        jne       ..B15.10      # Prob 67%                      #1364.16
                                # LOE eax ebx ebp esi edi
..B15.4:                        # Preds ..B15.3
        pushl     $12                                           #1364.16
        call      malloc@PLT                                    #1364.16
                                # LOE eax ebx ebp esi edi
..B15.48:                       # Preds ..B15.4
        popl      %ecx                                          #1364.16
        movl      %eax, %ecx                                    #1364.16
                                # LOE ecx ebx ebp esi edi
..B15.5:                        # Preds ..B15.48
        testl     %ecx, %ecx                                    #1364.16
        jne       ..B15.7       # Prob 68%                      #1364.16
                                # LOE ecx ebx ebp esi edi
..B15.6:                        # Preds ..B15.5
        call      __errno_location@PLT                          #1364.16
                                # LOE eax ebp
..B15.49:                       # Preds ..B15.6
        movl      56(%esp), %ecx                                #1364.16
        movl      (%eax), %edx                                  #1364.16
        movl      %edx, (%ecx)                                  #1364.16
        jmp       ..B15.27      # Prob 100%                     #1364.16
                                # LOE ebp
..B15.7:                        # Preds ..B15.5
        movl      $12, (%ecx)                                   #1364.16
        movl      %ecx, 20(%esp)                                #1364.16
        movl      %ecx, (%edi)                                  #1364.16
        addl      $1, %ebp                                      #1364.16
        movl      $14, 4(%ecx)                                  #1364.16
        movl      %ebp, %eax                                    #1364.16
        movl      %esi, %edx                                    #1364.16
        movl      $0, 8(%ecx)                                   #1364.16
        call      nss_sob                                       #1364.16
                                # LOE eax esi
..B15.50:                       # Preds ..B15.7
        movl      20(%esp), %ecx                                #
        movl      56(%esp), %ebx                                #1361.1
        movl      60(%esp), %ebp                                #1361.1
        movl      %ebx, 12(%esp)                                #1361.1
        movl      %ebp, 16(%esp)                                #1361.1
        addl      $8, %ecx                                      #1364.16
        movl      %esi, %edx                                    #1364.16
        call      nss_fasta_expr                                #1364.16
                                # LOE eax
..B15.51:                       # Preds ..B15.50
        movl      %eax, %ebp                                    #1364.16
        jmp       ..B15.27      # Prob 100%                     #1364.16
                                # LOE ebp
..B15.10:                       # Preds ..B15.3
        cmpl      $40, %eax                                     #1364.16
        jne       ..B15.21      # Prob 50%                      #1364.16
                                # LOE ebx ebp esi edi
..B15.11:                       # Preds ..B15.10
        pushl     $12                                           #1364.16
        call      malloc@PLT                                    #1364.16
                                # LOE eax ebx ebp esi edi
..B15.52:                       # Preds ..B15.11
        popl      %ecx                                          #1364.16
        movl      %eax, 20(%esp)                                #1364.16
                                # LOE eax ebx ebp esi edi al ah
..B15.12:                       # Preds ..B15.52
        testl     %eax, %eax                                    #1364.16
        jne       ..B15.14      # Prob 68%                      #1364.16
                                # LOE eax ebx ebp esi edi al ah
..B15.13:                       # Preds ..B15.12
        call      __errno_location@PLT                          #1364.16
                                # LOE eax ebp
..B15.53:                       # Preds ..B15.13
        movl      56(%esp), %ecx                                #1364.16
        movl      (%eax), %edx                                  #1364.16
        movl      %edx, (%ecx)                                  #1364.16
        jmp       ..B15.27      # Prob 100%                     #1364.16
                                # LOE ebp
..B15.14:                       # Preds ..B15.12
        movl      %eax, %ebx                                    #1364.16
        movl      60(%esp), %eax                                #1361.1
        movl      $12, (%ebx)                                   #1364.16
        movl      %ebx, (%edi)                                  #1364.16
        movl      56(%esp), %edi                                #1361.1
        movl      $0, 4(%ebx)                                   #1364.16
        addl      $1, %ebp                                      #1364.16
        lea       8(%ebx), %ecx                                 #1364.16
        movl      %edi, 12(%esp)                                #1361.1
        movl      %eax, 16(%esp)                                #1361.1
        movl      %ebp, %eax                                    #1364.16
        movl      %esi, %edx                                    #1364.16
        call      nss_expr                                      #1364.16
                                # LOE eax ebx esi bl bh
..B15.54:                       # Preds ..B15.14
        movl      %eax, %ebp                                    #1364.16
                                # LOE ebx ebp esi bl bh
..B15.15:                       # Preds ..B15.54
        movl      56(%esp), %eax                                #1364.16
        movl      (%eax), %edx                                  #1364.16
        testl     %edx, %edx                                    #1364.16
        jne       ..B15.27      # Prob 50%                      #1364.16
                                # LOE ebx ebp esi bl bh
..B15.16:                       # Preds ..B15.15
        movl      %ebx, %eax                                    #1364.16
        movl      8(%eax), %edx                                 #1364.16
        testl     %edx, %edx                                    #1364.16
        je        ..B15.19      # Prob 12%                      #1364.16
                                # LOE ebp esi
..B15.17:                       # Preds ..B15.16
        cmpl      %esi, %ebp                                    #1364.16
        je        ..B15.19      # Prob 12%                      #1364.16
                                # LOE ebp
..B15.18:                       # Preds ..B15.17
        movsbl    (%ebp), %eax                                  #1364.16
        addl      $1, %ebp                                      #1364.16
        cmpl      $41, %eax                                     #1364.16
        je        ..B15.27      # Prob 50%                      #1364.16
                                # LOE ebp
..B15.19:                       # Preds ..B15.18 ..B15.17 ..B15.16
        movl      56(%esp), %eax                                #1364.16
        movl      $22, (%eax)                                   #1364.16
        jmp       ..B15.27      # Prob 100%                     #1364.16
                                # LOE ebp
..B15.21:                       # Preds ..B15.10
        movl      56(%esp), %eax                                #1361.1
        movl      %eax, 12(%esp)                                #1361.1
        movl      60(%esp), %eax                                #1361.1
        movl      %esi, %edx                                    #1364.16
        movl      %edi, %ecx                                    #1364.16
        movl      %eax, 16(%esp)                                #1361.1
        movl      %ebp, %eax                                    #1364.16
        call      nss_fasta_expr                                #1364.16
                                # LOE eax ebx esi edi
..B15.55:                       # Preds ..B15.21
        movl      %eax, %ebp                                    #1364.16
                                # LOE ebx ebp esi edi
..B15.22:                       # Preds ..B15.55
        movl      56(%esp), %eax                                #1364.16
        movl      (%eax), %edx                                  #1364.16
        testl     %edx, %edx                                    #1364.16
        jne       ..B15.27      # Prob 50%                      #1364.16
                                # LOE ebx ebp esi edi
..B15.23:                       # Preds ..B15.22
        cmpl      %esi, %ebp                                    #1364.16
        jae       ..B15.27      # Prob 33%                      #1364.16
                                # LOE ebx ebp esi edi
..B15.24:                       # Preds ..B15.23
        movl      %ebp, %eax                                    #1364.16
        movl      %esi, %edx                                    #1364.16
        call      nss_sob                                       #1364.16
                                # LOE eax ebx esi edi
..B15.56:                       # Preds ..B15.24
        movl      %eax, %ebp                                    #1364.16
        cmpl      %esi, %ebp                                    #1364.16
        jae       ..B15.27      # Prob 12%                      #1364.16
                                # LOE ebx ebp edi
..B15.25:                       # Preds ..B15.56
        movsbl    (%ebp), %eax                                  #1364.16
        cmpl      $36, %eax                                     #1364.16
        je        ..B15.28      # Prob 5%                       #1364.16
                                # LOE ebx ebp edi
..B15.27:                       # Preds ..B15.22 ..B15.23 ..B15.25 ..B15.30 ..B15.56
                                #       ..B15.58 ..B15.15 ..B15.19 ..B15.18 ..B15.53
                                #       ..B15.49 ..B15.51
        movl      %ebp, %eax                                    #1364.16
        addl      $24, %esp                                     #1364.16
        popl      %ebx                                          #1364.16
        popl      %ebp                                          #1364.16
        popl      %esi                                          #1364.16
        popl      %edi                                          #1364.16
        ret                                                     #1364.16
                                # LOE
..B15.28:                       # Preds ..B15.25                # Infreq
        pushl     $12                                           #1364.16
        addl      $1, %ebp                                      #1364.16
        call      malloc@PLT                                    #1364.16
                                # LOE eax ebx ebp edi
..B15.57:                       # Preds ..B15.28                # Infreq
        popl      %ecx                                          #1364.16
                                # LOE eax ebx ebp edi
..B15.29:                       # Preds ..B15.57                # Infreq
        testl     %eax, %eax                                    #1364.16
        je        ..B15.31      # Prob 12%                      #1364.16
                                # LOE eax ebx ebp edi
..B15.30:                       # Preds ..B15.29                # Infreq
        movl      (%edi), %edx                                  #1364.16
        movl      $12, (%eax)                                   #1364.16
        movl      %edx, 8(%eax)                                 #1364.16
        movl      %eax, (%edi)                                  #1364.16
        movl      $15, 4(%eax)                                  #1364.16
        jmp       ..B15.27      # Prob 100%                     #1364.16
                                # LOE ebp
..B15.31:                       # Preds ..B15.29                # Infreq
        call      __errno_location@PLT                          #1364.16
                                # LOE eax ebp
..B15.58:                       # Preds ..B15.31                # Infreq
        movl      56(%esp), %ecx                                #1364.16
        movl      (%eax), %edx                                  #1364.16
        movl      %edx, (%ecx)                                  #1364.16
        jmp       ..B15.27      # Prob 100%                     #1364.16
                                # LOE ebp
..B15.32:                       # Preds ..B15.2                 # Infreq
        addl      $1, %ebp                                      #1374.19
        cmpl      %esi, %ebp                                    #1374.9
        jae       ..B15.38      # Prob 12%                      #1374.9
                                # LOE ebx ebp esi edi
..B15.33:                       # Preds ..B15.32                # Infreq
        call      __ctype_b_loc@PLT                             #1374.9
                                # LOE eax ebx ebp esi edi
..B15.59:                       # Preds ..B15.33                # Infreq
        movzbl    (%ebp), %edx                                  #1374.9
        movl      (%eax), %eax                                  #1374.9
        movzwl    (%eax,%edx,2), %ecx                           #1374.9
        testl     $8192, %ecx                                   #1374.9
        je        ..B15.38      # Prob 15%                      #1374.9
                                # LOE eax ebx ebp esi edi
..B15.35:                       # Preds ..B15.59 ..B15.36       # Infreq
        addl      $1, %ebp                                      #1374.9
        cmpl      %esi, %ebp                                    #1374.9
        jae       ..B15.38      # Prob 1%                       #1374.9
                                # LOE eax ebx ebp esi edi
..B15.36:                       # Preds ..B15.35                # Infreq
        movzbl    (%ebp), %edx                                  #1374.9
        movzwl    (%eax,%edx,2), %ecx                           #1374.9
        testl     $8192, %ecx                                   #1374.9
        jne       ..B15.35      # Prob 82%                      #1374.9
                                # LOE eax ebx ebp esi edi
..B15.38:                       # Preds ..B15.35 ..B15.36 ..B15.59 ..B15.32 # Infreq
        cmpl      %esi, %ebp                                    #1375.15
        jne       ..B15.41      # Prob 68%                      #1375.15
                                # LOE ebx ebp esi edi
..B15.39:                       # Preds ..B15.38                # Infreq
        movl      56(%esp), %eax                                #1376.9
        movl      $22, (%eax)                                   #1376.9
                                # LOE ebp
..B15.40:                       # Preds ..B15.61 ..B15.39       # Infreq
        movl      %ebp, %eax                                    #1375.5
        addl      $24, %esp                                     #1393.12
        popl      %ebx                                          #1393.12
        popl      %ebp                                          #1393.12
        popl      %esi                                          #1393.12
        popl      %edi                                          #1393.12
        ret                                                     #1393.12
                                # LOE
..B15.41:                       # Preds ..B15.38                # Infreq
        pushl     $12                                           #1379.34
        call      malloc@PLT                                    #1379.25
                                # LOE eax ebx ebp esi edi
..B15.60:                       # Preds ..B15.41                # Infreq
        popl      %ecx                                          #1379.25
                                # LOE eax ebx ebp esi edi
..B15.42:                       # Preds ..B15.60                # Infreq
        testl     %eax, %eax                                    #1380.19
        jne       ..B15.43      # Prob 99%                      #1380.19
                                # LOE eax ebx ebp esi edi
..B15.44:                       # Preds ..B15.42                # Infreq
        call      __errno_location@PLT                          #1381.24
                                # LOE eax ebp
..B15.61:                       # Preds ..B15.44                # Infreq
        movl      56(%esp), %ecx                                #1381.13
        movl      (%eax), %edx                                  #1381.24
        movl      %edx, (%ecx)                                  #1381.13
        jmp       ..B15.40      # Prob 100%                     #1381.13
        .align    2,0x90
                                # LOE ebp
# mark_end;
	.type	nss_unary_expr,@function
	.size	nss_unary_expr,.-nss_unary_expr
	.data
# -- End  nss_unary_expr
	.text
# -- Begin  nss_sob
# mark_begin;
       .align    2,0x90
nss_sob:
# parameter 1: %eax
# parameter 2: %edx
..B16.1:                        # Preds ..B16.0
        pushl     %edi                                          #1181.1
        pushl     %esi                                          #1181.1
        pushl     %ebx                                          #1181.1
        movl      %edx, %edi                                    #1181.1
        movl      %eax, %esi                                    #1181.1
        cmpl      %edi, %esi                                    #1182.17
        jae       ..B16.7       # Prob 12%                      #1182.17
                                # LOE ebp esi edi
..B16.2:                        # Preds ..B16.1
        call      ..L10         # Prob 100%                     #
..L10:                                                          #
        popl      %ebx                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L10], %ebx      #
        call      __ctype_b_loc@PLT                             #1182.24
                                # LOE eax ebp esi edi
..B16.11:                       # Preds ..B16.2
        movzbl    (%esi), %edx                                  #1182.24
        movl      (%eax), %ecx                                  #1182.24
        movzwl    (%ecx,%edx,2), %ebx                           #1182.24
        testl     $8192, %ebx                                   #1182.24
        je        ..B16.7       # Prob 15%                      #1182.24
                                # LOE ecx ebp esi edi
..B16.3:                        # Preds ..B16.11
        xorl      %edx, %edx                                    #1182.24
        movl      %esi, %eax                                    #1182.24
                                # LOE eax edx ecx ebp esi edi
..B16.4:                        # Preds ..B16.5 ..B16.3
        addl      $1, %eax                                      #1182.24
        addl      $1, %edx                                      #1182.24
        cmpl      %eax, %edi                                    #1182.17
        jbe       ..B16.6       # Prob 1%                       #1182.17
                                # LOE eax edx ecx ebp esi edi
..B16.5:                        # Preds ..B16.4
        movzbl    (%esi,%edx), %ebx                             #1182.24
        movzwl    (%ecx,%ebx,2), %ebx                           #1182.24
        testl     $8192, %ebx                                   #1182.24
        jne       ..B16.4       # Prob 82%                      #1182.24
                                # LOE eax edx ecx ebp esi edi
..B16.6:                        # Preds ..B16.4 ..B16.5
        addl      %edx, %esi                                    #1183.12
                                # LOE ebp esi
..B16.7:                        # Preds ..B16.11 ..B16.6 ..B16.1
        movl      %esi, %eax                                    #1184.12
        popl      %ebx                                          #1184.12
        popl      %esi                                          #1184.12
        popl      %edi                                          #1184.12
        ret                                                     #1184.12
        .align    2,0x90
                                # LOE
# mark_end;
	.type	nss_sob,@function
	.size	nss_sob,.-nss_sob
	.data
# -- End  nss_sob
	.text
# -- Begin  nss_fasta_expr
# mark_begin;
       .align    2,0x90
nss_fasta_expr:
# parameter 1: %eax
# parameter 2: %edx
# parameter 3: %ecx
# parameter 4: 56 + %esp
# parameter 5: 60 + %esp
..B17.1:                        # Preds ..B17.0
        pushl     %edi                                          #1234.1
        pushl     %esi                                          #1234.1
        pushl     %ebp                                          #1234.1
        pushl     %ebx                                          #1234.1
        subl      $24, %esp                                     #1234.1
        movl      %edx, %ebx                                    #1234.1
        movl      %eax, %esi                                    #1234.1
        movsbl    (%esi), %ebp                                  #1236.16
        call      ..L11         # Prob 100%                     #1234.1
..L11:                                                          #
        popl      %edx                                          #1234.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L11], %edx      #1234.1
        cmpl      $39, %ebp                                     #1236.16
        jne       ..B17.16      # Prob 67%                      #1236.16
                                # LOE edx ecx ebx ebp esi
..B17.2:                        # Preds ..B17.1
        addl      $1, %esi                                      #1239.30
        movl      %esi, %ebp                                    #1239.30
        cmpl      %ebx, %esi                                    #1239.13
        jae       ..B17.10      # Prob 44%                      #1239.13
                                # LOE edx ecx ebx ebp esi
..B17.4:                        # Preds ..B17.2
        lea       fasta_2na_map.0@GOTOFF(%edx), %eax            #1239.13
        movl      %esi, 16(%esp)                                #
        movl      %ecx, 20(%esp)                                #
        xorl      %edi, %edi                                    #
                                # LOE eax edx ebx ebp edi
..B17.5:                        # Preds ..B17.7 ..B17.4
        movsbl    (%ebp), %ecx                                  #1239.13
        testl     %ecx, %ecx                                    #1239.13
        jl        ..B17.91      # Prob 1%                       #1239.13
                                # LOE eax edx ebx ebp edi
..B17.6:                        # Preds ..B17.5
        movzbl    (%ebp), %esi                                  #1239.13
        movsbl    (%esi,%eax), %ecx                             #1239.13
        testl     %ecx, %ecx                                    #1239.13
        jl        ..B17.47      # Prob 16%                      #1239.13
                                # LOE eax edx ebx ebp esi edi
..B17.7:                        # Preds ..B17.49 ..B17.6
        addl      $1, %ebp                                      #1239.13
        cmpl      %ebx, %ebp                                    #1239.13
        jb        ..B17.5       # Prob 82%                      #1239.13
                                # LOE eax edx ebx ebp edi
..B17.8:                        # Preds ..B17.7
        movl      16(%esp), %esi                                #
        movl      20(%esp), %ecx                                #
                                # LOE ecx ebx ebp esi edi cl ch
..B17.9:                        # Preds ..B17.91 ..B17.8
        cmpl      %esi, %ebp                                    #1239.13
        ja        ..B17.42      # Prob 12%                      #1239.13
                                # LOE ecx ebx ebp esi edi cl ch
..B17.10:                       # Preds ..B17.2 ..B17.9
        movl      $22, %edx                                     #1239.13
        movl      $22, %eax                                     #
                                # LOE eax edx ebx ebp
..B17.11:                       # Preds ..B17.10 ..B17.44 ..B17.46
        testl     %eax, %eax                                    #1240.26
        jne       ..B17.30      # Prob 50%                      #1240.26
                                # LOE edx ebx ebp
..B17.12:                       # Preds ..B17.11
        cmpl      %ebx, %ebp                                    #1240.38
        je        ..B17.14      # Prob 12%                      #1240.38
                                # LOE edx ebp
..B17.13:                       # Preds ..B17.12
        movl      56(%esp), %eax                                #1241.13
        movl      %edx, (%eax)                                  #1241.13
        movsbl    (%ebp), %edx                                  #1240.47
        addl      $1, %ebp                                      #1240.47
        cmpl      $39, %edx                                     #1240.55
        je        ..B17.31      # Prob 50%                      #1240.55
                                # LOE ebp
..B17.14:                       # Preds ..B17.13 ..B17.12
        movl      56(%esp), %eax                                #1241.13
        movl      $22, (%eax)                                   #1241.13
        jmp       ..B17.31      # Prob 100%                     #1241.13
                                # LOE ebp
..B17.16:                       # Preds ..B17.1
        cmpl      $34, %ebp                                     #1236.16
        jne       ..B17.32      # Prob 50%                      #1236.16
                                # LOE edx ecx ebx esi
..B17.17:                       # Preds ..B17.16
        addl      $1, %esi                                      #1244.30
        movl      %esi, %ebp                                    #1244.30
        cmpl      %ebx, %esi                                    #1244.13
        jae       ..B17.25      # Prob 44%                      #1244.13
                                # LOE edx ecx ebx ebp esi
..B17.19:                       # Preds ..B17.17
        lea       fasta_2na_map.0@GOTOFF(%edx), %eax            #1244.13
        movl      %esi, 16(%esp)                                #
        movl      %ecx, 20(%esp)                                #
        xorl      %edi, %edi                                    #
                                # LOE eax edx ebx ebp edi
..B17.20:                       # Preds ..B17.22 ..B17.19
        movsbl    (%ebp), %ecx                                  #1244.13
        testl     %ecx, %ecx                                    #1244.13
        jl        ..B17.90      # Prob 1%                       #1244.13
                                # LOE eax edx ebx ebp edi
..B17.21:                       # Preds ..B17.20
        movzbl    (%ebp), %esi                                  #1244.13
        movsbl    (%esi,%eax), %ecx                             #1244.13
        testl     %ecx, %ecx                                    #1244.13
        jl        ..B17.58      # Prob 16%                      #1244.13
                                # LOE eax edx ebx ebp esi edi
..B17.22:                       # Preds ..B17.60 ..B17.21
        addl      $1, %ebp                                      #1244.13
        cmpl      %ebx, %ebp                                    #1244.13
        jb        ..B17.20      # Prob 82%                      #1244.13
                                # LOE eax edx ebx ebp edi
..B17.23:                       # Preds ..B17.22
        movl      16(%esp), %esi                                #
        movl      20(%esp), %ecx                                #
                                # LOE ecx ebx ebp esi edi cl ch
..B17.24:                       # Preds ..B17.90 ..B17.23
        cmpl      %esi, %ebp                                    #1244.13
        ja        ..B17.53      # Prob 12%                      #1244.13
                                # LOE ecx ebx ebp esi edi cl ch
..B17.25:                       # Preds ..B17.17 ..B17.24
        movl      $22, %edx                                     #1244.13
        movl      $22, %eax                                     #
                                # LOE eax edx ebx ebp
..B17.26:                       # Preds ..B17.25 ..B17.55 ..B17.57
        testl     %eax, %eax                                    #1245.26
        jne       ..B17.30      # Prob 50%                      #1245.26
                                # LOE edx ebx ebp
..B17.27:                       # Preds ..B17.26
        cmpl      %ebx, %ebp                                    #1245.38
        je        ..B17.29      # Prob 12%                      #1245.38
                                # LOE edx ebp
..B17.28:                       # Preds ..B17.27
        movl      56(%esp), %eax                                #1241.13
        movl      %edx, (%eax)                                  #1241.13
        movsbl    (%ebp), %edx                                  #1245.47
        addl      $1, %ebp                                      #1245.47
        cmpl      $34, %edx                                     #1245.55
        je        ..B17.31      # Prob 50%                      #1245.55
                                # LOE ebp
..B17.29:                       # Preds ..B17.28 ..B17.27
        movl      56(%esp), %eax                                #1246.13
        movl      $22, (%eax)                                   #1246.13
        jmp       ..B17.31      # Prob 100%                     #1246.13
                                # LOE ebp
..B17.30:                       # Preds ..B17.11 ..B17.26
        movl      56(%esp), %eax                                #1241.13
        movl      %edx, (%eax)                                  #1241.13
                                # LOE ebp
..B17.31:                       # Preds ..B17.13 ..B17.28 ..B17.30 ..B17.14 ..B17.29
                                #      
        movl      %ebp, %eax                                    #1252.12
        addl      $24, %esp                                     #1252.12
        popl      %ebx                                          #1252.12
        popl      %ebp                                          #1252.12
        popl      %esi                                          #1252.12
        popl      %edi                                          #1252.12
        ret                                                     #1252.12
                                # LOE
..B17.32:                       # Preds ..B17.16
        movl      %esi, %ebp                                    #1234.1
        cmpl      %ebx, %esi                                    #1249.16
        jae       ..B17.40      # Prob 44%                      #1249.16
                                # LOE edx ecx ebx ebp esi
..B17.34:                       # Preds ..B17.32
        lea       fasta_2na_map.0@GOTOFF(%edx), %eax            #1249.16
        movl      %esi, 16(%esp)                                #
        movl      %ecx, 20(%esp)                                #
        xorl      %edi, %edi                                    #
                                # LOE eax edx ebx ebp edi
..B17.35:                       # Preds ..B17.37 ..B17.34
        movsbl    (%ebp), %ecx                                  #1249.16
        testl     %ecx, %ecx                                    #1249.16
        jl        ..B17.89      # Prob 1%                       #1249.16
                                # LOE eax edx ebx ebp edi
..B17.36:                       # Preds ..B17.35
        movzbl    (%ebp), %esi                                  #1249.16
        movsbl    (%esi,%eax), %ecx                             #1249.16
        testl     %ecx, %ecx                                    #1249.16
        jl        ..B17.69      # Prob 16%                      #1249.16
                                # LOE eax edx ebx ebp esi edi
..B17.37:                       # Preds ..B17.71 ..B17.36
        addl      $1, %ebp                                      #1249.16
        cmpl      %ebx, %ebp                                    #1249.16
        jb        ..B17.35      # Prob 82%                      #1249.16
                                # LOE eax edx ebx ebp edi
..B17.38:                       # Preds ..B17.37
        movl      16(%esp), %esi                                #
        movl      20(%esp), %ecx                                #
                                # LOE ecx ebp esi edi cl ch
..B17.39:                       # Preds ..B17.89 ..B17.38
        cmpl      %esi, %ebp                                    #1249.16
        ja        ..B17.64      # Prob 12%                      #1249.16
                                # LOE ecx ebp esi edi cl ch
..B17.40:                       # Preds ..B17.32 ..B17.39
        movl      56(%esp), %eax                                #1249.16
        movl      $22, (%eax)                                   #1249.16
                                # LOE ebp
..B17.41:                       # Preds ..B17.40 ..B17.66 ..B17.68
        movl      %ebp, %eax                                    #1249.16
        addl      $24, %esp                                     #1249.16
        popl      %ebx                                          #1249.16
        popl      %ebp                                          #1249.16
        popl      %esi                                          #1249.16
        popl      %edi                                          #1249.16
        ret                                                     #1249.16
                                # LOE
..B17.42:                       # Preds ..B17.9                 # Infreq
        testl     %edi, %edi                                    #1239.13
        jne       ..B17.45      # Prob 50%                      #1239.13
                                # LOE ecx ebx ebp esi cl ch
..B17.43:                       # Preds ..B17.42                # Infreq
        movl      60(%esp), %edx                                #1239.13
        movl      %ebp, %edi                                    #1239.13
        subl      %esi, %edi                                    #1239.13
        movl      %edi, 12(%esp)                                #1239.13
        movl      %ecx, %eax                                    #1239.13
        movl      %esi, %ecx                                    #1239.13
        call      NucStrFastaExprMake2                          #1239.13
                                # LOE eax ebx ebp
..B17.44:                       # Preds ..B17.43                # Infreq
        movl      %eax, %edx                                    #1239.13
        jmp       ..B17.11      # Prob 100%                     #1239.13
                                # LOE eax edx ebx ebp
..B17.45:                       # Preds ..B17.42                # Infreq
        movl      60(%esp), %edx                                #1239.13
        movl      %ebp, %edi                                    #1239.13
        subl      %esi, %edi                                    #1239.13
        movl      %edi, 12(%esp)                                #1239.13
        movl      %ecx, %eax                                    #1239.13
        movl      %esi, %ecx                                    #1239.13
        call      NucStrFastaExprMake4                          #1239.13
                                # LOE eax ebx ebp
..B17.46:                       # Preds ..B17.45                # Infreq
        movl      %eax, %edx                                    #1239.13
        jmp       ..B17.11      # Prob 100%                     #1239.13
                                # LOE eax edx ebx ebp
..B17.47:                       # Preds ..B17.6                 # Infreq
        lea       fasta_4na_map.0@GOTOFF(%edx), %ecx            #1239.13
        cmpl      %ecx, %eax                                    #1239.13
        je        ..B17.91      # Prob 1%                       #1239.13
                                # LOE edx ecx ebx ebp esi edi
..B17.48:                       # Preds ..B17.47                # Infreq
        movsbl    (%esi,%ecx), %eax                             #1239.13
        testl     %eax, %eax                                    #1239.13
        jl        ..B17.91      # Prob 1%                       #1239.13
                                # LOE edx ecx ebx ebp edi
..B17.49:                       # Preds ..B17.48                # Infreq
        movl      %ecx, %eax                                    #1239.13
        movl      $1, %edi                                      #
        jmp       ..B17.7       # Prob 100%                     #
                                # LOE eax edx ebx ebp edi
..B17.91:                       # Preds ..B17.47 ..B17.48 ..B17.5 # Infreq
        movl      16(%esp), %esi                                #
        movl      20(%esp), %ecx                                #
        jmp       ..B17.9       # Prob 100%                     #
                                # LOE ecx ebx ebp esi edi cl ch
..B17.53:                       # Preds ..B17.24                # Infreq
        testl     %edi, %edi                                    #1244.13
        jne       ..B17.56      # Prob 50%                      #1244.13
                                # LOE ecx ebx ebp esi cl ch
..B17.54:                       # Preds ..B17.53                # Infreq
        movl      60(%esp), %edx                                #1244.13
        movl      %ebp, %edi                                    #1244.13
        subl      %esi, %edi                                    #1244.13
        movl      %edi, 12(%esp)                                #1244.13
        movl      %ecx, %eax                                    #1244.13
        movl      %esi, %ecx                                    #1244.13
        call      NucStrFastaExprMake2                          #1244.13
                                # LOE eax ebx ebp
..B17.55:                       # Preds ..B17.54                # Infreq
        movl      %eax, %edx                                    #1244.13
        jmp       ..B17.26      # Prob 100%                     #1244.13
                                # LOE eax edx ebx ebp
..B17.56:                       # Preds ..B17.53                # Infreq
        movl      60(%esp), %edx                                #1244.13
        movl      %ebp, %edi                                    #1244.13
        subl      %esi, %edi                                    #1244.13
        movl      %edi, 12(%esp)                                #1244.13
        movl      %ecx, %eax                                    #1244.13
        movl      %esi, %ecx                                    #1244.13
        call      NucStrFastaExprMake4                          #1244.13
                                # LOE eax ebx ebp
..B17.57:                       # Preds ..B17.56                # Infreq
        movl      %eax, %edx                                    #1244.13
        jmp       ..B17.26      # Prob 100%                     #1244.13
                                # LOE eax edx ebx ebp
..B17.58:                       # Preds ..B17.21                # Infreq
        lea       fasta_4na_map.0@GOTOFF(%edx), %ecx            #1244.13
        cmpl      %ecx, %eax                                    #1244.13
        je        ..B17.90      # Prob 1%                       #1244.13
                                # LOE edx ecx ebx ebp esi edi
..B17.59:                       # Preds ..B17.58                # Infreq
        movsbl    (%esi,%ecx), %eax                             #1244.13
        testl     %eax, %eax                                    #1244.13
        jl        ..B17.90      # Prob 1%                       #1244.13
                                # LOE edx ecx ebx ebp edi
..B17.60:                       # Preds ..B17.59                # Infreq
        movl      %ecx, %eax                                    #1244.13
        movl      $1, %edi                                      #
        jmp       ..B17.22      # Prob 100%                     #
                                # LOE eax edx ebx ebp edi
..B17.90:                       # Preds ..B17.58 ..B17.59 ..B17.20 # Infreq
        movl      16(%esp), %esi                                #
        movl      20(%esp), %ecx                                #
        jmp       ..B17.24      # Prob 100%                     #
                                # LOE ecx ebx ebp esi edi cl ch
..B17.64:                       # Preds ..B17.39                # Infreq
        testl     %edi, %edi                                    #1249.16
        jne       ..B17.67      # Prob 50%                      #1249.16
                                # LOE ecx ebp esi cl ch
..B17.65:                       # Preds ..B17.64                # Infreq
        movl      60(%esp), %edx                                #1249.16
        movl      %ebp, %ebx                                    #1249.16
        subl      %esi, %ebx                                    #1249.16
        movl      %ebx, 12(%esp)                                #1249.16
        movl      %ecx, %eax                                    #1249.16
        movl      %esi, %ecx                                    #1249.16
        call      NucStrFastaExprMake2                          #1249.16
                                # LOE eax ebp
..B17.66:                       # Preds ..B17.65                # Infreq
        movl      56(%esp), %edx                                #1249.16
        movl      %eax, (%edx)                                  #1249.16
        jmp       ..B17.41      # Prob 100%                     #1249.16
                                # LOE ebp
..B17.67:                       # Preds ..B17.64                # Infreq
        movl      60(%esp), %edx                                #1249.16
        movl      %ebp, %ebx                                    #1249.16
        subl      %esi, %ebx                                    #1249.16
        movl      %ebx, 12(%esp)                                #1249.16
        movl      %ecx, %eax                                    #1249.16
        movl      %esi, %ecx                                    #1249.16
        call      NucStrFastaExprMake4                          #1249.16
                                # LOE eax ebp
..B17.68:                       # Preds ..B17.67                # Infreq
        movl      56(%esp), %edx                                #1249.16
        movl      %eax, (%edx)                                  #1249.16
        jmp       ..B17.41      # Prob 100%                     #1249.16
                                # LOE ebp
..B17.69:                       # Preds ..B17.36                # Infreq
        lea       fasta_4na_map.0@GOTOFF(%edx), %ecx            #1249.16
        cmpl      %ecx, %eax                                    #1249.16
        je        ..B17.89      # Prob 1%                       #1249.16
                                # LOE edx ecx ebx ebp esi edi
..B17.70:                       # Preds ..B17.69                # Infreq
        movsbl    (%ecx,%esi), %eax                             #1249.16
        testl     %eax, %eax                                    #1249.16
        jl        ..B17.89      # Prob 1%                       #1249.16
                                # LOE edx ecx ebx ebp edi
..B17.71:                       # Preds ..B17.70                # Infreq
        movl      %ecx, %eax                                    #1249.16
        movl      $1, %edi                                      #
        jmp       ..B17.37      # Prob 100%                     #
                                # LOE eax edx ebx ebp edi
..B17.89:                       # Preds ..B17.69 ..B17.70 ..B17.35 # Infreq
        movl      16(%esp), %esi                                #
        movl      20(%esp), %ecx                                #
        jmp       ..B17.39      # Prob 100%                     #
        .align    2,0x90
                                # LOE ecx ebp esi edi cl ch
# mark_end;
	.type	nss_fasta_expr,@function
	.size	nss_fasta_expr,.-nss_fasta_expr
	.data
# -- End  nss_fasta_expr
	.text
# -- Begin  NucStrFastaExprMake4
# mark_begin;
       .align    2,0x90
NucStrFastaExprMake4:
# parameter 1: %eax
# parameter 2: %edx
# parameter 3: %ecx
# parameter 4: 20 + %ebp
..B18.1:                        # Preds ..B18.0
        pushl     %ebp                                          #1017.1
        movl      %esp, %ebp                                    #1017.1
        andl      $-16, %esp                                    #1017.1
        pushl     %edi                                          #1017.1
        pushl     %esi                                          #1017.1
        pushl     %ebx                                          #1017.1
        subl      $52, %esp                                     #1017.1
        movl      %ecx, %esi                                    #1017.1
        movl      20(%ebp), %ecx                                #1015.5
        movl      %edx, 12(%esp)                                #1017.1
        movl      %eax, %edx                                    #1017.1
        cmpl      $29, %ecx                                     #1023.17
        jbe       ..B18.3       # Prob 43%                      #1023.17
                                # LOE edx esi
..B18.2:                        # Preds ..B18.1
        movl      $7, %eax                                      #1024.16
        addl      $52, %esp                                     #1024.16
        popl      %ebx                                          #1024.16
        popl      %esi                                          #1024.16
        popl      %edi                                          #1024.16
        movl      %ebp, %esp                                    #1024.16
        popl      %ebp                                          #1024.16
        ret                                                     #1024.16
                                # LOE
..B18.3:                        # Preds ..B18.1
        pushl     $160                                          #1026.32
        movl      %edx, 12(%esp)                                #1026.9
        call      ..L12         # Prob 100%                     #
..L12:                                                          #
        popl      %ebx                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L12], %ebx      #
        call      malloc@PLT                                    #1026.9
                                # LOE eax ebx esi
..B18.46:                       # Preds ..B18.3
        movl      12(%esp), %edx                                #
        addl      $4, %esp                                      #1026.9
                                # LOE eax edx ebx esi dl dh
..B18.4:                        # Preds ..B18.46
        testl     %eax, %eax                                    #1026.9
        je        ..B18.43      # Prob 12%                      #1026.9
                                # LOE eax edx ebx esi dl dh
..B18.5:                        # Preds ..B18.4
        lea       15(%eax), %edi                                #1026.9
        andl      $-16, %edi                                    #1026.9
        movl      %eax, 8(%edi)                                 #1026.9
        je        ..B18.43      # Prob 5%                       #1027.15
                                # LOE edx ebx esi edi dl dh
..B18.6:                        # Preds ..B18.5
        movl      20(%ebp), %eax                                #1031.5
        movl      %edi, (%edx)                                  #1030.5
        movl      %eax, 4(%edi)                                 #1031.5
        testl     %eax, %eax                                    #1035.22
        jbe       ..B18.42      # Prob 16%                      #1035.22
                                # LOE ebx esi edi
..B18.7:                        # Preds ..B18.6
        movl      20(%ebp), %edx                                #
        movl      %edi, 4(%esp)                                 #
        xorl      %ecx, %ecx                                    #
                                # LOE edx ecx ebx esi
..B18.8:                        # Preds ..B18.15 ..B18.7
        movsbl    (%esi,%ecx), %eax                             #1037.49
        movsbl    fasta_4na_map.0@GOTOFF(%ebx,%eax), %eax       #1037.25
        movl      %ecx, %edi                                    #1038.22
        andl      $3, %edi                                      #1038.22
        je        ..B18.41      # Prob 20%                      #1038.22
                                # LOE eax edx ecx ebx esi edi
..B18.9:                        # Preds ..B18.8
        cmpl      $1, %edi                                      #1038.22
        je        ..B18.14      # Prob 25%                      #1038.22
                                # LOE eax edx ecx ebx esi edi
..B18.10:                       # Preds ..B18.9
        cmpl      $2, %edi                                      #1038.22
        jne       ..B18.12      # Prob 67%                      #1038.22
                                # LOE eax edx ecx ebx esi edi
..B18.11:                       # Preds ..B18.10
        movl      %ecx, %edi                                    #1049.32
        shrl      $2, %edi                                      #1049.32
        shll      $12, %eax                                     #1049.47
        orw       %ax, 16(%esp,%edi,2)                          #1049.13
        movzwl    32(%esp,%edi,2), %eax                         #1050.13
        orl       $-4096, %eax                                  #1050.13
        movw      %ax, 32(%esp,%edi,2)                          #1050.13
        jmp       ..B18.15      # Prob 100%                     #1050.13
                                # LOE edx ecx ebx esi
..B18.12:                       # Preds ..B18.10
        cmpl      $3, %edi                                      #1038.22
        jne       ..B18.15      # Prob 50%                      #1038.22
                                # LOE eax edx ecx ebx esi
..B18.13:                       # Preds ..B18.12
        movl      %ecx, %edi                                    #1053.32
        shrl      $2, %edi                                      #1053.32
        shll      $8, %eax                                      #1053.47
        orw       %ax, 16(%esp,%edi,2)                          #1053.13
        movzwl    32(%esp,%edi,2), %eax                         #1054.13
        orl       $-61696, %eax                                 #1054.13
        movw      %ax, 32(%esp,%edi,2)                          #1054.13
        jmp       ..B18.15      # Prob 100%                     #1054.13
                                # LOE edx ecx ebx esi
..B18.14:                       # Preds ..B18.9
        movl      %ecx, %edi                                    #1045.32
        shrl      $2, %edi                                      #1045.32
        orw       %ax, 16(%esp,%edi,2)                          #1045.13
        movzwl    32(%esp,%edi,2), %eax                         #1046.13
        orl       $15, %eax                                     #1046.13
        movw      %ax, 32(%esp,%edi,2)                          #1046.13
                                # LOE edx ecx ebx esi
..B18.15:                       # Preds ..B18.41 ..B18.14 ..B18.11 ..B18.13 ..B18.12
                                #      
        addl      $1, %ecx                                      #1035.31
        cmpl      %edx, %ecx                                    #1035.22
        jb        ..B18.8       # Prob 82%                      #1035.22
                                # LOE edx ecx ebx esi
..B18.16:                       # Preds ..B18.15
        movl      4(%esp), %edi                                 #
                                # LOE ecx edi
..B18.17:                       # Preds ..B18.16 ..B18.42
        addl      $3, %ecx                                      #1060.21
        shrl      $2, %ecx                                      #1060.28
        cmpl      $8, %ecx                                      #1060.35
        jae       ..B18.30      # Prob 50%                      #1060.35
                                # LOE ecx edi
..B18.18:                       # Preds ..B18.17
        movl      %ecx, %ebx                                    #1060.5
        negl      %ebx                                          #1060.5
        addl      $8, %ebx                                      #1060.5
        lea       (%ecx,%ecx), %esi                             #
        movl      %esi, 8(%esp)                                 #
        lea       16(%esp,%ecx,2), %esi                         #1060.5
        andl      $15, %esi                                     #1060.5
        movl      %esi, %eax                                    #1060.5
        negl      %eax                                          #1060.5
        addl      $16, %eax                                     #1060.5
        shrl      $1, %eax                                      #1060.5
        cmpl      $0, %esi                                      #1060.5
        cmovne    %eax, %esi                                    #1060.5
        lea       8(%esi), %edx                                 #1060.5
        cmpl      %edx, %ebx                                    #1060.5
        jb        ..B18.40      # Prob 10%                      #1060.5
                                # LOE ecx ebx esi edi
..B18.19:                       # Preds ..B18.18
        movl      %ebx, %edx                                    #1060.5
        subl      %esi, %edx                                    #1060.5
        andl      $7, %edx                                      #1060.5
        negl      %edx                                          #1060.5
        addl      %ebx, %edx                                    #1060.5
        testl     %esi, %esi                                    #1060.5
        jbe       ..B18.23      # Prob 1%                       #1060.5
                                # LOE edx ecx ebx esi edi
..B18.20:                       # Preds ..B18.19
        movl      %ecx, (%esp)                                  #
        movl      %edi, 4(%esp)                                 #
        movl      8(%esp), %edi                                 #
        xorl      %eax, %eax                                    #
                                # LOE eax edx ebx esi edi
..B18.21:                       # Preds ..B18.21 ..B18.20
        xorl      %ecx, %ecx                                    #1062.9
        movw      %cx, 16(%esp,%edi)                            #1062.9
        addl      $1, %eax                                      #1060.5
        movw      %cx, 32(%esp,%edi)                            #1063.9
        addl      $2, %edi                                      #1060.5
        cmpl      %esi, %eax                                    #1060.5
        jb        ..B18.21      # Prob 99%                      #1060.5
                                # LOE eax edx ebx esi edi
..B18.22:                       # Preds ..B18.21
        movl      (%esp), %ecx                                  #
        movl      4(%esp), %edi                                 #
                                # LOE edx ecx ebx esi edi
..B18.23:                       # Preds ..B18.22 ..B18.19
        lea       (%esi,%esi), %eax                             #1062.29
        lea       (%eax,%ecx,2), %eax                           #1062.29
        pxor      %xmm0, %xmm0                                  #1062.29
                                # LOE eax edx ecx ebx esi edi xmm0
..B18.24:                       # Preds ..B18.24 ..B18.23
        movdqa    %xmm0, 16(%esp,%eax)                          #1062.9
        movdqa    %xmm0, 32(%esp,%eax)                          #1063.9
        addl      $16, %eax                                     #1060.5
        addl      $8, %esi                                      #1060.5
        cmpl      %edx, %esi                                    #1060.5
        jb        ..B18.24      # Prob 99%                      #1060.5
                                # LOE eax edx ecx ebx esi edi xmm0
..B18.26:                       # Preds ..B18.24 ..B18.40
        cmpl      %ebx, %edx                                    #1060.5
        jae       ..B18.30      # Prob 1%                       #1060.5
                                # LOE edx ecx ebx edi
..B18.27:                       # Preds ..B18.26
        lea       (%edx,%edx), %eax                             #
        lea       (%eax,%ecx,2), %eax                           #
                                # LOE eax edx ebx edi
..B18.28:                       # Preds ..B18.28 ..B18.27
        xorl      %esi, %esi                                    #1062.9
        movw      %si, 16(%esp,%eax)                            #1062.9
        addl      $1, %edx                                      #1060.5
        movw      %si, 32(%esp,%eax)                            #1063.9
        addl      $2, %eax                                      #1060.5
        cmpl      %ebx, %edx                                    #1060.5
        jb        ..B18.28      # Prob 99%                      #1060.5
                                # LOE eax edx ebx edi
..B18.30:                       # Preds ..B18.28 ..B18.26 ..B18.17
        movl      12(%esp), %eax                                #1068.10
        testl     %eax, %eax                                    #1068.10
        je        ..B18.32      # Prob 50%                      #1068.10
                                # LOE edi
..B18.31:                       # Preds ..B18.30
        movl      $10, (%edi)                                   #1070.9
        jmp       ..B18.39      # Prob 100%                     #1070.9
                                # LOE edi
..B18.32:                       # Preds ..B18.30
        movl      20(%ebp), %eax                                #1072.22
        cmpl      $2, %eax                                      #1072.22
        jae       ..B18.34      # Prob 50%                      #1072.22
                                # LOE edi
..B18.33:                       # Preds ..B18.32
        movzwl    16(%esp), %eax                                #1074.29
        movzwl    32(%esp), %esi                                #1078.26
        movl      $6, (%edi)                                    #1082.9
        movw      %ax, 18(%esp)                                 #1074.9
        movl      16(%esp), %ebx                                #1075.29
        movl      %ebx, 20(%esp)                                #1075.9
        movl      16(%esp), %edx                                #1076.29
        movl      %edx, 24(%esp)                                #1076.9
        movl      %ebx, 28(%esp)                                #1076.9
        movw      %si, 34(%esp)                                 #1078.9
        movl      32(%esp), %edx                                #1079.26
        movl      %edx, 36(%esp)                                #1079.9
        movl      32(%esp), %eax                                #1080.26
        movl      %eax, 40(%esp)                                #1080.9
        movl      %edx, 44(%esp)                                #1080.9
        jmp       ..B18.39      # Prob 100%                     #1080.9
                                # LOE edi
..B18.34:                       # Preds ..B18.32
        movl      20(%ebp), %eax                                #1084.22
        cmpl      $6, %eax                                      #1084.22
        jae       ..B18.36      # Prob 50%                      #1084.22
                                # LOE edi
..B18.35:                       # Preds ..B18.34
        movl      16(%esp), %edx                                #1086.29
        movl      32(%esp), %esi                                #1089.26
        movl      $7, (%edi)                                    #1092.9
        movl      %edx, 20(%esp)                                #1086.9
        movl      16(%esp), %eax                                #1087.29
        movl      %eax, 24(%esp)                                #1087.9
        movl      %edx, 28(%esp)                                #1087.9
        movl      %esi, 36(%esp)                                #1089.9
        movl      32(%esp), %ebx                                #1090.26
        movl      %ebx, 40(%esp)                                #1090.9
        movl      %esi, 44(%esp)                                #1090.9
        jmp       ..B18.39      # Prob 100%                     #1090.9
                                # LOE edi
..B18.36:                       # Preds ..B18.34
        movl      20(%ebp), %eax                                #1094.22
        cmpl      $14, %eax                                     #1094.22
        jae       ..B18.38      # Prob 50%                      #1094.22
                                # LOE edi
..B18.37:                       # Preds ..B18.36
        movl      16(%esp), %eax                                #1096.29
        movl      20(%esp), %edx                                #1096.29
        movl      32(%esp), %ebx                                #1097.26
        movl      36(%esp), %esi                                #1097.26
        movl      %eax, 24(%esp)                                #1096.9
        movl      $1, (%edi)                                    #1099.9
        movl      %edx, 28(%esp)                                #1096.9
        movl      %ebx, 40(%esp)                                #1097.9
        movl      %esi, 44(%esp)                                #1097.9
        jmp       ..B18.39      # Prob 100%                     #1097.9
                                # LOE edi
..B18.38:                       # Preds ..B18.36
        movl      $8, (%edi)                                    #1103.9
                                # LOE edi
..B18.39:                       # Preds ..B18.31 ..B18.33 ..B18.35 ..B18.37 ..B18.38
                                #      
        movl      16(%esp), %eax                                #1106.42
        movl      20(%esp), %ebx                                #1106.42
        movl      24(%esp), %esi                                #1106.42
        lea       16(%esp), %ecx                                #1110.5
        movl      %eax, 16(%edi)                                #1106.42
        movl      28(%esp), %eax                                #1106.42
        movl      %ebx, 20(%edi)                                #1106.42
        movl      %esi, 24(%edi)                                #1106.42
        movl      %eax, 28(%edi)                                #1106.42
        movl      32(%esp), %eax                                #1107.39
        movl      %eax, 32(%edi)                                #1107.39
        movl      36(%esp), %eax                                #1107.39
        movl      %eax, 36(%edi)                                #1107.39
        movl      40(%esp), %eax                                #1107.39
        movl      %eax, 40(%edi)                                #1107.39
        movl      44(%esp), %eax                                #1107.39
        movl      %eax, 44(%edi)                                #1107.39
# Begin ASM
        mov (%ecx), %eax;mov 12(%ecx), %edx;bswap %eax;bswap %edx;mov %eax, 12(%ecx);mov %edx, (%ecx);mov 4(%ecx), %eax;mov 8(%ecx), %edx;bswap %eax;bswap %edx;mov %eax, 8(%ecx);mov %edx, 4(%ecx);
# End ASM                                                       #1110.5
        lea       32(%esp), %ecx                                #1111.5
# Begin ASM
        mov (%ecx), %eax;mov 12(%ecx), %edx;bswap %eax;bswap %edx;mov %eax, 12(%ecx);mov %edx, (%ecx);mov 4(%ecx), %eax;mov 8(%ecx), %edx;bswap %eax;bswap %edx;mov %eax, 8(%ecx);mov %edx, 4(%ecx);
# End ASM                                                       #1111.5
        movl      $4, %ecx                                      #1114.5
        lea       16(%esp), %edx                                #1114.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #1114.5
        lea       32(%esp), %edx                                #1115.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #1115.5
        lea       48(%edi), %ecx                                #1118.5
        lea       16(%esp), %edx                                #1118.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #1118.5
        lea       64(%edi), %ecx                                #1119.5
        lea       32(%esp), %edx                                #1119.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #1119.5
        movl      $4, %ecx                                      #1121.5
        lea       16(%esp), %edx                                #1121.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #1121.5
        lea       32(%esp), %edx                                #1122.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #1122.5
        lea       80(%edi), %ecx                                #1124.5
        lea       16(%esp), %edx                                #1124.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #1124.5
        lea       96(%edi), %ecx                                #1125.5
        lea       32(%esp), %edx                                #1125.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #1125.5
        movl      $4, %ecx                                      #1127.5
        lea       16(%esp), %edx                                #1127.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #1127.5
        lea       32(%esp), %edx                                #1128.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #1128.5
        lea       112(%edi), %ecx                               #1130.5
        lea       16(%esp), %edx                                #1130.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #1130.5
        addl      $128, %edi                                    #1131.5
        movl      %edi, %ecx                                    #1131.5
        lea       32(%esp), %edx                                #1131.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #1131.5
        xorl      %eax, %eax                                    #1173.12
        addl      $52, %esp                                     #1173.12
        popl      %ebx                                          #1173.12
        popl      %esi                                          #1173.12
        popl      %edi                                          #1173.12
        movl      %ebp, %esp                                    #1173.12
        popl      %ebp                                          #1173.12
        ret                                                     #1173.12
                                # LOE
..B18.40:                       # Preds ..B18.18                # Infreq
        xorl      %edx, %edx                                    #1060.5
        jmp       ..B18.26      # Prob 100%                     #1060.5
                                # LOE edx ecx ebx edi
..B18.41:                       # Preds ..B18.8                 # Infreq
        movl      %ecx, %edi                                    #1041.32
        shrl      $2, %edi                                      #1041.32
        shll      $4, %eax                                      #1041.46
        movw      %ax, 16(%esp,%edi,2)                          #1041.13
        movl      $240, %eax                                    #1042.13
        movw      %ax, 32(%esp,%edi,2)                          #1042.13
        jmp       ..B18.15      # Prob 100%                     #1042.13
                                # LOE edx ecx ebx esi
..B18.42:                       # Preds ..B18.6                 # Infreq
        xorl      %ecx, %ecx                                    #
        jmp       ..B18.17      # Prob 100%                     #
                                # LOE ecx edi
..B18.43:                       # Preds ..B18.4 ..B18.5         # Infreq
        call      __errno_location@PLT                          #1028.16
                                # LOE eax
..B18.47:                       # Preds ..B18.43                # Infreq
        movl      (%eax), %eax                                  #1028.16
        addl      $52, %esp                                     #1028.16
        popl      %ebx                                          #1028.16
        popl      %esi                                          #1028.16
        popl      %edi                                          #1028.16
        movl      %ebp, %esp                                    #1028.16
        popl      %ebp                                          #1028.16
        ret                                                     #1028.16
        .align    2,0x90
                                # LOE
# mark_end;
	.type	NucStrFastaExprMake4,@function
	.size	NucStrFastaExprMake4,.-NucStrFastaExprMake4
	.data
# -- End  NucStrFastaExprMake4
	.text
# -- Begin  NucStrFastaExprMake2
# mark_begin;
       .align    2,0x90
NucStrFastaExprMake2:
# parameter 1: %eax
# parameter 2: %edx
# parameter 3: %ecx
# parameter 4: 20 + %ebp
..B19.1:                        # Preds ..B19.0
        pushl     %ebp                                          #838.1
        movl      %esp, %ebp                                    #838.1
        andl      $-16, %esp                                    #838.1
        pushl     %edi                                          #838.1
        pushl     %esi                                          #838.1
        pushl     %ebx                                          #838.1
        subl      $52, %esp                                     #838.1
        movl      %ecx, %esi                                    #838.1
        movl      %edx, 48(%esp)                                #838.1
        movl      %eax, %edi                                    #838.1
        movl      20(%ebp), %eax                                #836.5
        cmpl      $61, %eax                                     #844.17
        jbe       ..B19.3       # Prob 43%                      #844.17
                                # LOE esi edi
..B19.2:                        # Preds ..B19.1
        movl      $7, %eax                                      #845.16
        addl      $52, %esp                                     #845.16
        popl      %ebx                                          #845.16
        popl      %esi                                          #845.16
        popl      %edi                                          #845.16
        movl      %ebp, %esp                                    #845.16
        popl      %ebp                                          #845.16
        ret                                                     #845.16
                                # LOE
..B19.3:                        # Preds ..B19.1
        call      ..L13         # Prob 100%                     #
..L13:                                                          #
        popl      %ebx                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L13], %ebx      #
        movl      %ebx, 12(%esp)                                #
        pushl     $160                                          #847.32
        call      malloc@PLT                                    #847.9
                                # LOE eax esi edi
..B19.48:                       # Preds ..B19.3
        popl      %ecx                                          #847.9
                                # LOE eax esi edi
..B19.4:                        # Preds ..B19.48
        testl     %eax, %eax                                    #847.9
        je        ..B19.45      # Prob 12%                      #847.9
                                # LOE eax esi edi
..B19.5:                        # Preds ..B19.4
        lea       15(%eax), %ebx                                #847.9
        andl      $-16, %ebx                                    #847.9
        movl      %eax, 8(%ebx)                                 #847.9
        je        ..B19.45      # Prob 5%                       #848.15
                                # LOE ebx esi edi
..B19.6:                        # Preds ..B19.5
        movl      20(%ebp), %eax                                #852.5
        movl      %ebx, (%edi)                                  #851.5
        movl      %eax, 4(%ebx)                                 #852.5
        testl     %eax, %eax                                    #856.22
        jbe       ..B19.44      # Prob 16%                      #856.22
                                # LOE ebx esi
..B19.7:                        # Preds ..B19.6
        movl      12(%esp), %eax                                #
        movl      %ebx, 8(%esp)                                 #
        xorl      %edi, %edi                                    #
                                # LOE eax esi edi
..B19.8:                        # Preds ..B19.15 ..B19.7
        movsbl    (%esi,%edi), %edx                             #858.48
        movzbl    fasta_2na_map.0@GOTOFF(%eax,%edx), %edx       #858.24
        movl      %edi, %ecx                                    #859.22
        andl      $3, %ecx                                      #859.22
        je        ..B19.43      # Prob 20%                      #859.22
                                # LOE eax edx ecx esi edi
..B19.9:                        # Preds ..B19.8
        cmpl      $1, %ecx                                      #859.22
        je        ..B19.14      # Prob 25%                      #859.22
                                # LOE eax edx ecx esi edi
..B19.10:                       # Preds ..B19.9
        cmpl      $2, %ecx                                      #859.22
        jne       ..B19.12      # Prob 67%                      #859.22
                                # LOE eax edx ecx esi edi
..B19.11:                       # Preds ..B19.10
        movl      %edi, %ecx                                    #870.32
        shrl      $2, %ecx                                      #870.32
        orb       $12, 32(%esp,%ecx)                            #871.13
        shll      $2, %edx                                      #870.47
        orb       %dl, 16(%esp,%ecx)                            #870.13
        jmp       ..B19.15      # Prob 100%                     #870.13
                                # LOE eax esi edi
..B19.12:                       # Preds ..B19.10
        cmpl      $3, %ecx                                      #859.22
        jne       ..B19.15      # Prob 50%                      #859.22
                                # LOE eax edx esi edi
..B19.13:                       # Preds ..B19.12
        movl      %edi, %ecx                                    #874.32
        shrl      $2, %ecx                                      #874.32
        orb       $3, 32(%esp,%ecx)                             #875.13
        orb       %dl, 16(%esp,%ecx)                            #874.13
        jmp       ..B19.15      # Prob 100%                     #874.13
                                # LOE eax esi edi
..B19.14:                       # Preds ..B19.9
        movl      %edi, %ecx                                    #866.32
        shrl      $2, %ecx                                      #866.32
        orb       $48, 32(%esp,%ecx)                            #867.13
        shll      $4, %edx                                      #866.47
        orb       %dl, 16(%esp,%ecx)                            #866.13
                                # LOE eax esi edi
..B19.15:                       # Preds ..B19.43 ..B19.14 ..B19.11 ..B19.13 ..B19.12
                                #      
        movl      20(%ebp), %edx                                #856.22
        addl      $1, %edi                                      #856.31
        cmpl      %edx, %edi                                    #856.22
        jb        ..B19.8       # Prob 82%                      #856.22
                                # LOE eax esi edi
..B19.16:                       # Preds ..B19.15
        movl      8(%esp), %ebx                                 #
                                # LOE ebx edi
..B19.17:                       # Preds ..B19.16 ..B19.44
        addl      $3, %edi                                      #881.21
        shrl      $2, %edi                                      #881.28
        cmpl      $16, %edi                                     #881.35
        jae       ..B19.30      # Prob 50%                      #881.35
                                # LOE ebx edi
..B19.18:                       # Preds ..B19.17
        lea       16(%esp), %edx                                #883.9
        movl      %edi, %esi                                    #881.5
        negl      %esi                                          #881.5
        addl      $16, %esi                                     #881.5
        lea       (%edx,%edi), %edx                             #881.5
        andl      $15, %edx                                     #881.5
        movl      %edx, %eax                                    #881.5
        negl      %eax                                          #881.5
        addl      $16, %eax                                     #881.5
        cmpl      $0, %edx                                      #881.5
        cmovne    %eax, %edx                                    #881.5
        lea       16(%edx), %eax                                #881.5
        cmpl      %eax, %esi                                    #881.5
        jb        ..B19.42      # Prob 10%                      #881.5
                                # LOE edx ebx esi edi
..B19.19:                       # Preds ..B19.18
        movl      %esi, %eax                                    #881.5
        subl      %edx, %eax                                    #881.5
        andl      $15, %eax                                     #881.5
        negl      %eax                                          #881.5
        addl      %esi, %eax                                    #881.5
        testl     %edx, %edx                                    #881.5
        jbe       ..B19.23      # Prob 1%                       #881.5
                                # LOE eax edx ebx esi edi
..B19.20:                       # Preds ..B19.19
        movl      %edi, 4(%esp)                                 #
        movl      %esi, (%esp)                                  #
        movl      %ebx, 8(%esp)                                 #
        movl      %edi, %ebx                                    #
        lea       (%edx,%edi), %ecx                             #
        movl      %ecx, 12(%esp)                                #
        movl      %ecx, %esi                                    #
                                # LOE eax edx ebx esi edi
..B19.21:                       # Preds ..B19.21 ..B19.20
        xorl      %ecx, %ecx                                    #883.9
        movb      %cl, 16(%esp,%ebx)                            #883.9
        movb      %cl, 32(%esp,%ebx)                            #884.9
        addl      $1, %ebx                                      #881.5
        cmpl      %esi, %ebx                                    #881.5
        jb        ..B19.21      # Prob 99%                      #881.5
                                # LOE eax edx ebx esi edi
..B19.22:                       # Preds ..B19.21
        movl      (%esp), %esi                                  #
        movl      8(%esp), %ebx                                 #
                                # LOE eax edx ebx esi edi
..B19.23:                       # Preds ..B19.22 ..B19.19
        lea       (%edx,%edi), %ecx                             #883.29
        movl      %ecx, 12(%esp)                                #883.29
        pxor      %xmm0, %xmm0                                  #883.29
                                # LOE eax edx ecx ebx esi edi xmm0
..B19.24:                       # Preds ..B19.24 ..B19.23
        movdqa    %xmm0, 16(%esp,%ecx)                          #883.9
        movdqa    %xmm0, 32(%esp,%ecx)                          #884.9
        addl      $16, %ecx                                     #881.5
        addl      $16, %edx                                     #881.5
        cmpl      %eax, %edx                                    #881.5
        jb        ..B19.24      # Prob 99%                      #881.5
                                # LOE eax edx ecx ebx esi edi xmm0
..B19.26:                       # Preds ..B19.24 ..B19.42
        cmpl      %esi, %eax                                    #881.5
        jae       ..B19.30      # Prob 1%                       #881.5
                                # LOE eax ebx esi edi
..B19.27:                       # Preds ..B19.26
        addl      %eax, %edi                                    #
                                # LOE eax ebx esi edi
..B19.28:                       # Preds ..B19.28 ..B19.27
        xorl      %edx, %edx                                    #883.9
        movb      %dl, 16(%esp,%edi)                            #883.9
        addl      $1, %eax                                      #881.5
        movb      %dl, 32(%esp,%edi)                            #884.9
        addl      $1, %edi                                      #881.5
        cmpl      %esi, %eax                                    #881.5
        jb        ..B19.28      # Prob 99%                      #881.5
                                # LOE eax ebx esi edi
..B19.30:                       # Preds ..B19.28 ..B19.26 ..B19.17
        movl      48(%esp), %eax                                #890.10
        testl     %eax, %eax                                    #890.10
        je        ..B19.32      # Prob 50%                      #890.10
                                # LOE ebx
..B19.31:                       # Preds ..B19.30
        movl      $9, (%ebx)                                    #892.9
        jmp       ..B19.41      # Prob 100%                     #892.9
                                # LOE ebx
..B19.32:                       # Preds ..B19.30
        movl      20(%ebp), %eax                                #896.22
        cmpl      $2, %eax                                      #896.22
        jae       ..B19.34      # Prob 50%                      #896.22
                                # LOE ebx
..B19.33:                       # Preds ..B19.32
        movzbl    16(%esp), %eax                                #898.29
        movl      $2, (%ebx)                                    #908.9
        movb      %al, 17(%esp)                                 #898.9
        movzwl    16(%esp), %esi                                #899.29
        movzbl    32(%esp), %eax                                #903.26
        movw      %si, 18(%esp)                                 #899.9
        movl      16(%esp), %edx                                #900.29
        movl      %edx, 20(%esp)                                #900.9
        movl      16(%esp), %edi                                #901.29
        movl      %edi, 24(%esp)                                #901.9
        movl      %edx, 28(%esp)                                #901.9
        movb      %al, 33(%esp)                                 #903.9
        movzwl    32(%esp), %eax                                #904.26
        movw      %ax, 34(%esp)                                 #904.9
        movl      32(%esp), %edx                                #905.26
        movl      %edx, 36(%esp)                                #905.9
        movl      32(%esp), %eax                                #906.26
        movl      %eax, 40(%esp)                                #906.9
        movl      %edx, 44(%esp)                                #906.9
        jmp       ..B19.41      # Prob 100%                     #906.9
                                # LOE ebx
..B19.34:                       # Preds ..B19.32
        movl      20(%ebp), %eax                                #910.22
        cmpl      $6, %eax                                      #910.22
        jae       ..B19.36      # Prob 50%                      #910.22
                                # LOE ebx
..B19.35:                       # Preds ..B19.34
        movzwl    16(%esp), %eax                                #912.29
        movzwl    32(%esp), %edi                                #916.26
        movl      $3, (%ebx)                                    #920.9
        movw      %ax, 18(%esp)                                 #912.9
        movl      16(%esp), %esi                                #913.29
        movl      %esi, 20(%esp)                                #913.9
        movl      16(%esp), %edx                                #914.29
        movl      %edx, 24(%esp)                                #914.9
        movl      %esi, 28(%esp)                                #914.9
        movw      %di, 34(%esp)                                 #916.9
        movl      32(%esp), %edx                                #917.26
        movl      %edx, 36(%esp)                                #917.9
        movl      32(%esp), %eax                                #918.26
        movl      %eax, 40(%esp)                                #918.9
        movl      %edx, 44(%esp)                                #918.9
        jmp       ..B19.41      # Prob 100%                     #918.9
                                # LOE ebx
..B19.36:                       # Preds ..B19.34
        movl      20(%ebp), %eax                                #922.22
        cmpl      $14, %eax                                     #922.22
        jae       ..B19.38      # Prob 50%                      #922.22
                                # LOE ebx
..B19.37:                       # Preds ..B19.36
        movl      16(%esp), %edx                                #924.29
        movl      32(%esp), %edi                                #927.26
        movl      $4, (%ebx)                                    #930.9
        movl      %edx, 20(%esp)                                #924.9
        movl      16(%esp), %eax                                #925.29
        movl      %eax, 24(%esp)                                #925.9
        movl      %edx, 28(%esp)                                #925.9
        movl      %edi, 36(%esp)                                #927.9
        movl      32(%esp), %esi                                #928.26
        movl      %esi, 40(%esp)                                #928.9
        movl      %edi, 44(%esp)                                #928.9
        jmp       ..B19.41      # Prob 100%                     #928.9
                                # LOE ebx
..B19.38:                       # Preds ..B19.36
        movl      20(%ebp), %eax                                #932.22
        cmpl      $30, %eax                                     #932.22
        jae       ..B19.40      # Prob 50%                      #932.22
                                # LOE ebx
..B19.39:                       # Preds ..B19.38
        movl      16(%esp), %eax                                #934.29
        movl      20(%esp), %edx                                #934.29
        movl      32(%esp), %esi                                #935.26
        movl      36(%esp), %edi                                #935.26
        movl      %eax, 24(%esp)                                #934.9
        movl      $0, (%ebx)                                    #937.9
        movl      %edx, 28(%esp)                                #934.9
        movl      %esi, 40(%esp)                                #935.9
        movl      %edi, 44(%esp)                                #935.9
        jmp       ..B19.41      # Prob 100%                     #935.9
                                # LOE ebx
..B19.40:                       # Preds ..B19.38
        movl      $5, (%ebx)                                    #941.9
                                # LOE ebx
..B19.41:                       # Preds ..B19.31 ..B19.33 ..B19.35 ..B19.37 ..B19.39
                                #       ..B19.40
        movl      16(%esp), %eax                                #944.42
        movl      20(%esp), %esi                                #944.42
        movl      24(%esp), %edi                                #944.42
        lea       16(%esp), %ecx                                #948.5
        movl      %eax, 16(%ebx)                                #944.42
        movl      28(%esp), %eax                                #944.42
        movl      %esi, 20(%ebx)                                #944.42
        movl      %edi, 24(%ebx)                                #944.42
        movl      %eax, 28(%ebx)                                #944.42
        movl      32(%esp), %eax                                #945.39
        movl      %eax, 32(%ebx)                                #945.39
        movl      36(%esp), %eax                                #945.39
        movl      %eax, 36(%ebx)                                #945.39
        movl      40(%esp), %eax                                #945.39
        movl      %eax, 40(%ebx)                                #945.39
        movl      44(%esp), %eax                                #945.39
        movl      %eax, 44(%ebx)                                #945.39
# Begin ASM
        mov (%ecx), %eax;mov 12(%ecx), %edx;bswap %eax;bswap %edx;mov %eax, 12(%ecx);mov %edx, (%ecx);mov 4(%ecx), %eax;mov 8(%ecx), %edx;bswap %eax;bswap %edx;mov %eax, 8(%ecx);mov %edx, 4(%ecx);
# End ASM                                                       #948.5
        lea       32(%esp), %ecx                                #949.5
# Begin ASM
        mov (%ecx), %eax;mov 12(%ecx), %edx;bswap %eax;bswap %edx;mov %eax, 12(%ecx);mov %edx, (%ecx);mov 4(%ecx), %eax;mov 8(%ecx), %edx;bswap %eax;bswap %edx;mov %eax, 8(%ecx);mov %edx, 4(%ecx);
# End ASM                                                       #949.5
        movl      $2, %ecx                                      #952.5
        lea       16(%esp), %edx                                #952.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #952.5
        lea       32(%esp), %edx                                #953.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #953.5
        lea       48(%ebx), %ecx                                #956.5
        lea       16(%esp), %edx                                #956.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #956.5
        lea       64(%ebx), %ecx                                #957.5
        lea       32(%esp), %edx                                #957.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #957.5
        movl      $2, %ecx                                      #959.5
        lea       16(%esp), %edx                                #959.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #959.5
        lea       32(%esp), %edx                                #960.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #960.5
        lea       80(%ebx), %ecx                                #962.5
        lea       16(%esp), %edx                                #962.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #962.5
        lea       96(%ebx), %ecx                                #963.5
        lea       32(%esp), %edx                                #963.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #963.5
        movl      $2, %ecx                                      #965.5
        lea       16(%esp), %edx                                #965.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #965.5
        lea       32(%esp), %edx                                #966.5
# Begin ASM
        mov 4(%edx), %eax;shrd %cl, %eax, (%edx);mov 8(%edx), %eax;shrd %cl, %eax, 4(%edx);mov 12(%edx), %eax;shrd %cl, %eax, 8(%edx);shr %cl, %eax;mov %eax, 12(%edx);
# End ASM                                                       #966.5
        lea       112(%ebx), %ecx                               #968.5
        lea       16(%esp), %edx                                #968.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #968.5
        addl      $128, %ebx                                    #969.5
        movl      %ebx, %ecx                                    #969.5
        lea       32(%esp), %edx                                #969.5
# Begin ASM
        push %ebx;mov (%edx), %eax;mov 12(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 12(%ecx);mov %ebx, (%ecx);mov 4(%edx), %eax;mov 8(%edx), %ebx;bswap %eax;bswap %ebx;mov %eax, 8(%ecx);mov %ebx, 4(%ecx);pop %ebx;
# End ASM                                                       #969.5
        xorl      %eax, %eax                                    #1011.12
        addl      $52, %esp                                     #1011.12
        popl      %ebx                                          #1011.12
        popl      %esi                                          #1011.12
        popl      %edi                                          #1011.12
        movl      %ebp, %esp                                    #1011.12
        popl      %ebp                                          #1011.12
        ret                                                     #1011.12
                                # LOE
..B19.42:                       # Preds ..B19.18                # Infreq
        xorl      %eax, %eax                                    #881.5
        jmp       ..B19.26      # Prob 100%                     #881.5
                                # LOE eax ebx esi edi
..B19.43:                       # Preds ..B19.8                 # Infreq
        movl      %edi, %ecx                                    #862.32
        shrl      $2, %ecx                                      #862.32
        movb      $192, 32(%esp,%ecx)                           #863.13
        shll      $6, %edx                                      #862.46
        movb      %dl, 16(%esp,%ecx)                            #862.13
        jmp       ..B19.15      # Prob 100%                     #862.13
                                # LOE eax esi edi
..B19.44:                       # Preds ..B19.6                 # Infreq
        xorl      %edi, %edi                                    #
        jmp       ..B19.17      # Prob 100%                     #
                                # LOE ebx edi
..B19.45:                       # Preds ..B19.4 ..B19.5         # Infreq
        movl      12(%esp), %ebx                                #849.16
        call      __errno_location@PLT                          #849.16
                                # LOE eax
..B19.49:                       # Preds ..B19.45                # Infreq
        movl      (%eax), %eax                                  #849.16
        addl      $52, %esp                                     #849.16
        popl      %ebx                                          #849.16
        popl      %esi                                          #849.16
        popl      %edi                                          #849.16
        movl      %ebp, %esp                                    #849.16
        popl      %ebp                                          #849.16
        ret                                                     #849.16
        .align    2,0x90
                                # LOE
# mark_end;
	.type	NucStrFastaExprMake2,@function
	.size	NucStrFastaExprMake2,.-NucStrFastaExprMake2
	.data
# -- End  NucStrFastaExprMake2
	.text
# -- Begin  NucStrstrWhack
# mark_begin;
       .align    2,0x90
	.globl NucStrstrWhack
NucStrstrWhack:
# parameter 1: 12 + %esp
..B20.1:                        # Preds ..B20.0
        pushl     %ebp                                          #1574.1
        pushl     %ebx                                          #1574.1
        movl      12(%esp), %ebp                                #1573.6
        testl     %ebp, %ebp                                    #1575.18
        je        ..B20.22      # Prob 12%                      #1575.18
                                # LOE ebp esi edi
..B20.2:                        # Preds ..B20.1
        movl      (%ebp), %eax                                  #1577.18
        call      ..L14         # Prob 100%                     #
..L14:                                                          #
        popl      %ebx                                          #
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L14], %ebx      #
        cmpl      $12, %eax                                     #1577.9
        ja        ..B20.21      # Prob 50%                      #1577.9
                                # LOE eax ebx ebp esi edi
..B20.3:                        # Preds ..B20.2
        movl      ..1..TPKT.18_0.0.15@GOTOFF(%ebx,%eax,4), %eax #1577.9
        jmp       *%eax                                         #1577.9
                                # LOE ebx ebp esi edi
..1.18_0.TAG.0c.0.15:
..B20.5:                        # Preds ..B20.3
        pushl     8(%ebp)                                       #1601.30
        call      NucStrstrWhack@PLT                            #1601.13
                                # LOE ebx ebp esi edi
..B20.25:                       # Preds ..B20.5
        popl      %ecx                                          #1601.13
        jmp       ..B20.21      # Prob 100%                     #1601.13
                                # LOE ebx ebp esi edi
..1.18_0.TAG.0b.0.15:
..B20.7:                        # Preds ..B20.3
        pushl     8(%ebp)                                       #1597.30
        call      NucStrstrWhack@PLT                            #1597.13
                                # LOE ebx ebp esi edi
..B20.8:                        # Preds ..B20.7
        pushl     12(%ebp)                                      #1598.30
        call      NucStrstrWhack@PLT                            #1598.13
                                # LOE ebx ebp esi edi
..B20.26:                       # Preds ..B20.8
        addl      $8, %esp                                      #1598.13
        jmp       ..B20.21      # Prob 100%                     #1598.13
                                # LOE ebx ebp esi edi
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
..B20.20:                       # Preds ..B20.3 ..B20.3 ..B20.3 ..B20.3 ..B20.3
                                #       ..B20.3 ..B20.3 ..B20.3 ..B20.3 ..B20.3
                                #       ..B20.3
        movl      8(%ebp), %ebp                                 #1593.20
                                # LOE ebx ebp esi edi
..B20.21:                       # Preds ..B20.26 ..B20.25 ..B20.20 ..B20.2
        pushl     %ebp                                          #1604.9
        call      free@PLT                                      #1604.9
                                # LOE esi edi
..B20.27:                       # Preds ..B20.21
        popl      %ecx                                          #1604.9
                                # LOE esi edi
..B20.22:                       # Preds ..B20.27 ..B20.1
        popl      %ebx                                          #1606.1
        popl      %ebp                                          #1606.1
        ret                                                     #1606.1
        .align    2,0x90
                                # LOE
# mark_end;
	.type	NucStrstrWhack,@function
	.size	NucStrstrWhack,.-NucStrstrWhack
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
# -- Begin  NucStrstrInit
# mark_begin;
       .align    2,0x90
NucStrstrInit:
..B21.1:                        # Preds ..B21.0
        pushl     %edi                                          #1484.1
        pushl     %esi                                          #1484.1
        pushl     %ebp                                          #1484.1
        pushl     %ebx                                          #1484.1
        subl      $12, %esp                                     #1484.1
        movl      $-1, %edx                                     #1491.5
        call      ..L15         # Prob 100%                     #1484.1
..L15:                                                          #
        popl      %ebx                                          #1484.1
        addl      $_GLOBAL_OFFSET_TABLE_+[. - ..L15], %ebx      #1484.1
        lea       fasta_2na_map.0@GOTOFF(%ebx), %eax            #1491.14
        movd      %edx, %xmm0                                   #1491.5
        pshufd    $0, %xmm0, %xmm1                              #1491.5
        movq      %xmm1, (%eax)                                 #1491.5
        movq      %xmm1, 8(%eax)                                #1491.5
        movq      %xmm1, 16(%eax)                               #1491.5
        movq      %xmm1, 24(%eax)                               #1491.5
        movq      %xmm1, 32(%eax)                               #1491.5
        movq      %xmm1, 40(%eax)                               #1491.5
        movq      %xmm1, 48(%eax)                               #1491.5
        movq      %xmm1, 56(%eax)                               #1491.5
        movq      %xmm1, 64(%eax)                               #1491.5
        movq      %xmm1, 72(%eax)                               #1491.5
        movq      %xmm1, 80(%eax)                               #1491.5
        movq      %xmm1, 88(%eax)                               #1491.5
        movq      %xmm1, 96(%eax)                               #1491.5
        movq      %xmm1, 104(%eax)                              #1491.5
        movq      %xmm1, 112(%eax)                              #1491.5
        movq      %xmm1, 120(%eax)                              #1491.5
                                # LOE eax ebx edi
..B21.2:                        # Preds ..B21.1
        lea       fasta_4na_map.0@GOTOFF(%ebx), %ecx            #1492.14
        movl      $-1, %edx                                     #1492.5
        movd      %edx, %xmm0                                   #1492.5
        pshufd    $0, %xmm0, %xmm1                              #1492.5
        movq      %xmm1, (%ecx)                                 #1492.5
        movq      %xmm1, 8(%ecx)                                #1492.5
        movq      %xmm1, 16(%ecx)                               #1492.5
        movq      %xmm1, 24(%ecx)                               #1492.5
        movq      %xmm1, 32(%ecx)                               #1492.5
        movq      %xmm1, 40(%ecx)                               #1492.5
        movq      %xmm1, 48(%ecx)                               #1492.5
        movq      %xmm1, 56(%ecx)                               #1492.5
        movq      %xmm1, 64(%ecx)                               #1492.5
        movq      %xmm1, 72(%ecx)                               #1492.5
        movq      %xmm1, 80(%ecx)                               #1492.5
        movq      %xmm1, 88(%ecx)                               #1492.5
        movq      %xmm1, 96(%ecx)                               #1492.5
        movq      %xmm1, 104(%ecx)                              #1492.5
        movq      %xmm1, 112(%ecx)                              #1492.5
        movq      %xmm1, 120(%ecx)                              #1492.5
                                # LOE eax ecx ebx edi
..B21.3:                        # Preds ..B21.2
        lea       _2__STRING.0.0@GOTOFF(%ebx), %esi             #1487.29
        movsbl    (%esi), %ebp                                  #1497.14
        movl      %ecx, 4(%esp)                                 #
        movl      %eax, 8(%esp)                                 #
        xorl      %edx, %edx                                    #
                                # LOE edx ebx ebp esi
..B21.10:                       # Preds ..B21.3 ..B21.11
        pushl     %ebp                                          #1497.9
        movl      %edx, 4(%esp)                                 #1498.48
        call      tolower@PLT                                   #1498.48
                                # LOE eax ebx ebp esi
..B21.29:                       # Preds ..B21.10
        movl      4(%esp), %edx                                 #
        addl      $4, %esp                                      #1498.48
                                # LOE eax edx ebx ebp esi dl dh
..B21.11:                       # Preds ..B21.29
        movl      8(%esp), %ecx                                 #1498.32
        movb      %dl, (%eax,%ecx)                              #1498.32
        movb      %dl, (%ecx,%ebp)                              #1498.9
        movsbl    1(%esi), %ebp                                 #1495.31
        addl      $1, %edx                                      #1495.48
        addl      $1, %esi                                      #1495.54
        testl     %ebp, %ebp                                    #1495.42
        jne       ..B21.10      # Prob 82%                      #1495.42
                                # LOE edx ebx ebp esi
..B21.12:                       # Preds ..B21.11
        movl      4(%esp), %ecx                                 #
        lea       _2__STRING.1.0@GOTOFF(%ebx), %esi             #1488.25
        movsbl    (%esi), %ebp                                  #1504.14
        movl      %ecx, 4(%esp)                                 #
        xorl      %edx, %edx                                    #
                                # LOE edx ebx ebp esi
..B21.19:                       # Preds ..B21.12 ..B21.20
        pushl     %ebp                                          #1504.9
        movl      %edx, 4(%esp)                                 #1505.48
        call      tolower@PLT                                   #1505.48
                                # LOE eax ebx ebp esi
..B21.30:                       # Preds ..B21.19
        movl      4(%esp), %edx                                 #
        addl      $4, %esp                                      #1505.48
                                # LOE eax edx ebx ebp esi dl dh
..B21.20:                       # Preds ..B21.30
        movl      4(%esp), %ecx                                 #1505.32
        movb      %dl, (%eax,%ecx)                              #1505.32
        movb      %dl, (%ecx,%ebp)                              #1505.9
        movsbl    1(%esi), %ebp                                 #1502.31
        addl      $1, %edx                                      #1502.48
        addl      $1, %esi                                      #1502.54
        testl     %ebp, %ebp                                    #1502.42
        jne       ..B21.19      # Prob 82%                      #1502.42
                                # LOE edx ebx ebp esi
..B21.21:                       # Preds ..B21.20
        lea       512(%ebx), %eax                               #1510.11
                                # LOE eax ebx edi
..B21.22:                       # Preds ..B21.25 ..B21.21
        movzwl    expand_2na.0@GOTOFF(%ebx), %edx               #1511.28
                                # LOE eax edx ebx edi
..B21.24:                       # Preds ..B21.22
# Begin ASM
        rorw $8, %dx
# End ASM                                                       #1511.0
                                # LOE eax edx ebx edi
..B21.25:                       # Preds ..B21.24
        movw      %dx, expand_2na.0@GOTOFF(%ebx)                #1511.9
        addl      $2, %ebx                                      #1510.30
        cmpl      %ebx, %eax                                    #1510.22
        ja        ..B21.22      # Prob 99%                      #1510.22
                                # LOE eax ebx edi
..B21.26:                       # Preds ..B21.25
        addl      $12, %esp                                     #1513.1
        popl      %ebx                                          #1513.1
        popl      %ebp                                          #1513.1
        popl      %esi                                          #1513.1
        popl      %edi                                          #1513.1
        ret                                                     #1513.1
        .align    2,0x90
                                # LOE
# mark_end;
	.type	NucStrstrInit,@function
	.size	NucStrstrInit,.-NucStrstrInit
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
# End
