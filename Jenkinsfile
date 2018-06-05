#!/usr/bin/env groovy
// Helps IDE github format, not an actual Groovy script
pipeline {
    // "$BUILD_ID"
    agent any
        options {
            timestamps()
                //                timeout(time: 30, unit: 'MINUTES')
                //                buildDiscarder(logRotator(numtoKeepStr:'10'))
        } // options
    triggers {
        pollSCM("H/5 * * * 1-5") // H means hash?
    } // triggers
    stages {
        stage('Checkout') {
            agent any
                steps {
                    echo "Starting checkout"
                        sh "id"
                        sh "pwd"
                        sh "echo $WORKSPACE"
                        //sh "mkdir -p $WORKSPACE/devel"
                        //                    git clone https://github.com/ncbi/ncbi-vdb.git

                        //                    checkout("https://github.com/ncbi/ncbi-vdb.git")
                }
        }

        stage('Create build containers') {
            parallel {
                stage('CentOS7')
                {
                    agent {
                        dockerfile {
                            filename 'Dockerfile'
                            dir '$WORKSPACE/build'
                            additionalBuildArgs '-t centos7'
                        }
                    }
                    steps { sh "df -HT" }
                }

                stage('Debian9')
                {
                    agent { dockerfile { filename 'Dockerfile.debian9' dir '$WORKSPACE/build' additionalBuildArgs '-t debian9' } }
                    steps { sh "df -HT" }
                }
            }
        }

        stage('Source Packaging') {
            parallel {
                stage('Tarball') {
                    agent { docker { image 'centos7' } }
                    steps {
                        sh "tar -caf vdb.tar.gz ./*"
                    }
                    post { success {
                        archiveArtifacts(artifacts: "**/target/*.jar",
                                         allowEmptyArchive:
                                         false)
                    }
                    }
                }

                stage('Source RPM') {
                    agent { docker { image 'centos7' } }
                    steps {
                        sh 'echo "Hello World"'
                    }
                    post { success {
                        archiveArtifacts(artifacts: "**/*.srpm",
                                         allowEmptyArchive:
                                         false)
                    }
                    }
                }
            }
        }

        stage('Compilation')
        {
            parallel {
                stage('Static Analysis') {
                    agent { docker { image 'debian9' } }
                    steps {
                        sh "./configure --with-debug "
                            sh "make"
                    }
                }
                stage('Compile') {
                    agent { docker { image 'centos7' } }
                    steps {
                        sh "./configure --with-debug "
                            sh "make"
                    }
                }
            }
        }

        stage('Packaging')
        {
            parallel
            {

                stage('RPM Package') {
                    agent { docker { image 'centos7' } }
                    steps {
                        sh 'echo "Hello World"'
                    }
                }

                stage('Debian Package') {
                    agent { docker { image 'debian9' } }
                    steps {
                        sh 'echo "Hello World"'
                    }
                }
            }
        }

        stage('Containerization')
        {
            parallel
            {
                stage('CentOS Container') {
                    agent { docker { image 'centos7' } }
                    steps {
                        sh 'echo "Hello World"'
                    }
                }

                stage('Debian Container') {
                    agent { docker { image 'debian9' } }
                    steps {
                        sh 'echo "Hello World"'
                    }
                }
            }
        }

        stage('Container Testing')
        {
            parallel {

                stage('CentOS Testing') {
                    agent { docker { image 'centos7' } }
                    steps {
                        sh 'echo "Hello World"'
                    }
                }

                stage('Debian Testing') {
                    agent { docker { image 'debian9' } }
                    steps {
                        sh 'echo "Hello World"'
                    }
                }

                stage('Debian Code Coverage') {
                    agent { docker { image 'debian9' } }
                    steps {
                        sh 'echo "Hello World"'
                    }
                }
            }
        }

        stage('Deploy Artifacts') {
            agent { docker { image 'centos7' } }
            when {
                expression {
                    currentBuild.result == null || currentBuild.result ==
                        'SUCCESS'
                }
            }
            steps {
                sh 'echo "Hello World"'
            }
        }

        // TODO: Valgrind, Fuzz, Nightly Deep Test
        // Amazon EC2 slave
        // AWS Steps (s3Upload)
    } // stages
    post {
        always {
            sh 'echo'
        }
        success {
            sh ''
        }
        failure {
            mail to: 'mike.vartanian@nih.gov',
                 subject: 'build failed',
                 body: 'something happened'
        }
    } // post
} // pipeline

