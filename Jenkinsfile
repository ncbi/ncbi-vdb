#!/usr/bin/env groovy
// Helps IDE github format, not an actual Groovy script
pipeline {
    // "$BUILD_ID"
    agent none
        options {
            timeout(time: 30, unit: 'MINUTES')
                buildDiscarder(logRotator(numtoKeepStr:'10'))
        }
    triggers {
        pollSCM("H 4/* 0 0 1-5")
    }
    stages {
        stage('Checkout') {
            agent any
                steps {
                    sh "mkdir $WORKSPACE/devel;"
                        dir() {
                            checkout()
                        }
                }
        }

        stage('Tarball') {
            agent any
                steps {
                    sh "tar -caf vdb.tar.gz ./*"
                }
            post { success {
                archiveArtifacts(artifacts: '**/target/*.jar',
                                                        allowEmptyArchive:
                                                        true)
            }
            }
        }

        stage('Source RPM') {
            agent any
                steps {
                    sh 'echo "Hello World"'
                }
        }

        stage('Compile') {
            agent any
                steps {
                    sh "./configure --with-debug && make"
                }
        }

        stage('SRPM Package') {
            agent any
                steps {
                    sh 'echo "Hello World"'
                }
        }

        stage('RPM Package') {
            agent any
                steps {
                    sh 'echo "Hello World"'
                }
        }

        stage('Debian Package') {
            agent any // { label 'debian' }
        steps {
            sh 'echo "Hello World"'
        }
        }

        stage('CentOS Container') {
            agent any
                steps {
                    sh 'echo "Hello World"'
                }
        }

        stage('Debian Container') {
            agent any
                steps {
                    sh 'echo "Hello World"'
                }
        }

        stage('CentOS Testing') {
            agent any
                steps {
                    sh 'echo "Hello World"'
                }
        }

        stage('Debian Testing') {
            agent any
                steps {
                    sh 'echo "Hello World"'
                }
        }

        stage('Deploy Artifcats') {
            agent any
                when {
                    expression {
                        currentBuild.result == null || currentBuild.result ==
                            'SUCCESS'
                    }
                }
            steps {
                sh 'echo "Hello World"'
                    sh '''
                    echo "Multiline shell steps works too"
                    ls -lah
                    '''
            }
        }
        // TODO: Valgrind, Fuzz, Nightly Deep Test
        // Debian Pbuilder
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
            sh ''
        }
    } // post


}
