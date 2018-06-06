#!/usr/bin/env groovy
// Helps IDE github format, not an actual Groovy script
pipeline
{
    // "$BUILD_ID"
    agent any
        options
        {
            timestamps()
                //                timeout(time: 30, unit: 'MINUTES')
                //                buildDiscarder(logRotator(numtoKeepStr:'10'))
        } // options
    triggers
    {
        pollSCM("H/5 * * * 1-5") // H means hash?
    } // triggers
    stages
    {
        stage('Checkout')
        {
            agent any
                steps
                {
                    echo "Starting checkout"
                    sh "id"
                    sh "pwd"
                    sh "echo $WORKSPACE"
                    sh "head /etc/*release*"
                }
        }

        stage('Debug Compile')
        {
            agent any
                steps
                {
                    echo "Debug compile"
                    sh '''
                    rm -rf ngs ngs-tools ncbi-vdb sra-tools

                    git clone https://github.com/ncbi/ngs.git
                    git clone https://github.com/ncbi/ngs-tools.git
                    git clone https://github.com/ncbi/ncbi-vdb.git

                    for repo in "ngs" "ngs-tools" "ncbi-vdb"
                    do
                        cd $repo
                        # ./configure --without-debug
                        # make
                        cd ..
                    done
                    echo "Debug compile"
                    '''
                    echo "Debug compile completed"
                }
        }

        stage('Create build containers')
        {
            parallel
            {
                stage('CentOS 7')
                {
                    agent
                    {
                        dockerfile
                        {
                            filename 'Dockerfile.centos7'
                            dir 'build'
                            additionalBuildArgs '-t centos7'
                        }
                    }
                    steps
                    {
                        echo "On CentOS7"
                        sh "df -HT"
                        sh "head /etc/*release*"
                    }
                }

                stage('Debian 9')
                {
                    agent
                    {
                        dockerfile
                        {
                            filename 'Dockerfile.debian9'
                            dir 'build'
                            additionalBuildArgs '-t debian9'
                        }
                    }
                    steps
                    {
                        echo "On Debian9"
                        sh "df -HT"
                        sh "head /etc/*release*"
                    }
                }
            }
        }

        stage('Source Packaging')
        {
            parallel
            {
                stage('Tarball')
                {
                    agent { docker { image 'centos7' } }
                    steps
                    {
                        sh "pwd"
                        sh "ls"
                        sh "tar -caf /tmp/vdb.tar.gz *"
                        sh "mv /tmp/vdb.tar.gz ."
                        sh "ls -l vdb.tar.gz"
                    }
                    post
                    {
                        success
                        {
                            archiveArtifacts(artifacts: "vdb.tar.gz",
                                             allowEmptyArchive:
                                             false)
                        }
                    }
                }

                stage('CentOS 7 Source RPM')
                {
                    agent { docker { image 'centos7' } }
                    steps
                    {
                        sh 'build/makerpm.sh'
                    }
                    post
                    {
                        success
                        {
                            sh 'echo artifcat'
                            //archiveArtifacts(artifacts: "**/*.srpm", allowEmptyArchive: false)
                        }
                    }
                }
            }
        }

        stage('Release Compile')
        {
            parallel
            {
                stage('Static Analysis')
                {
                    agent { docker { image 'debian9' } }
                    steps
                    {
                        sh "./configure --with-debug "
                        sh "#make"
                    }
                }
                stage('Linux Compile')
                {
                    agent { docker { image 'centos7' } }
                    steps
                    {
                        sh "./configure --with-debug "
                        sh "#make"
                    }
                }
                stage('macOS Compile')
                {
                    agent { docker { image 'centos7' } }
                    steps
                    {
                        sh "echo talk to xcode here"
                    }
                }
                stage('Windows Compile')
                {
                    agent { docker { image 'centos7' } }
                    steps
                    {
                        sh "echo talk to MSVC here"
                    }
                }
            }
        }

        stage('Binary Packaging')
        {
            parallel
            {
                stage('RPM Package')
                {
                    agent { docker { image 'centos7' } }
                    steps
                    {
                        sh 'echo "Hello World"'
                    }
                }

                stage('Debian Package')
                {
                    agent { docker { image 'debian9' } }
                    steps
                    {
                        sh 'echo "Hello World"'
                    }
                }
            }
        }

        stage('Container Packaging')
        {
            parallel
            {
                stage('CentOS Container')
                {
                    agent { docker { image 'centos7' } }
                    steps
                    {
                        sh 'echo "Hello World"'
                    }
                }

                stage('Debian Container')
                {
                    agent { docker { image 'debian9' } }
                    steps
                    {
                        sh 'echo "Hello World"'
                    }
                }
            }
        }

        stage('Testing in Containers')
        {
            parallel
            {
                stage('CentOS Testing')
                {
                    agent { docker { image 'centos7' } }
                    steps
                    {
                        sh 'echo "Hello World"'
                    }
                }

                stage('Debian Testing')
                {
                    agent { docker { image 'debian9' } }
                    steps
                    {
                        sh 'echo "Hello World"'
                    }
                }

                stage('Debian Code Coverage')
                {
                    agent { docker { image 'debian9' } }
                    steps
                    {
                        sh 'echo "Hello World"'
                    }
                }
            }
        }

        stage('Deploy Artifacts')
        {
            agent { docker { image 'centos7' } }
            when
            {
                expression
                {
                    currentBuild.result == null || currentBuild.result ==
                        'SUCCESS'
                }
            }
            steps
            {
                sh 'echo "Hello World"'
            }
        }

        // TODO: Valgrind, Fuzz, deep testing on nightly engineering branch
        // Amazon EC2 slaves? Windows, Mac
        // AWS Steps (s3Upload)
    } // stages
    post
    {
        always
        {
            sh 'echo'
        }
        success
        {
            sh ''
        }
        failure
        {
            sh ''
            /*
            mail to: 'mike.vartanian@nih.gov',
                 subject: 'build failed',
                 body: 'something happened'
             */
        }
    } // post
} // pipeline

