#!/usr/bin/env groovy
// Helps IDE github format, not an actual Groovy script
pipeline {
    // "$BUILD_ID"
    agent none
        options {
            timestamps()
//                timeout(time: 30, unit: 'MINUTES')
//                buildDiscarder(logRotator(numtoKeepStr:'10'))
        }
    triggers {
        pollSCM("H/5 * * * 1-5") // H means hash?
    }
    stages {
        stage('Checkout') {
            agent { label 'centos' }
            steps {
                echo "Starting checkout"

                    sh "mkdir $WORKSPACE/devel"
//                    git clone https://github.com/ncbi/ncbi-vdb.git

//                    checkout("https://github.com/ncbi/ncbi-vdb.git")
            }
        }

        stage('Tarball') {
            agent { label 'centos' }
            steps {
                sh "tar -caf vdb.tar.gz ./*"
            }
            post { success {
                archiveArtifacts(artifacts: "**/target/*.jar",
                                 allowEmptyArchive:
                                 true)
            }
            }
        }

        stage('Source RPM') {
            agent { label 'centos' }
            steps {
                sh 'echo "Hello World"'
            }
        }

        stage('Compile') {
            agent { label 'centos' }
            steps {
                sh "./configure --with-debug "
                    sh "make"
            }
        }

        stage('SRPM Package') {
            agent { label 'centos' }
            steps {
                sh 'echo "Hello World"'
            }
        }

        stage('RPM Package') {
            agent { label 'centos' }
            steps {
                sh 'echo "Hello World"'
            }
        }

        stage('Debian Package') {
            agent { label 'debian' }
            steps {
                sh 'echo "Hello World"'
            }
        }

        stage('CentOS Container') {
            agent { label 'centos' }
            steps {
                sh 'echo "Hello World"'
            }
        }

        stage('Debian Container') {
            agent { label 'centos' }
            steps {
                sh 'echo "Hello World"'
            }
        }

        stage('CentOS Testing') {
            agent { label 'centos' }
            steps {
                sh 'echo "Hello World"'
            }
        }

        stage('Debian Testing') {
            agent { label 'debian' }
            steps {
                sh 'echo "Hello World"'
            }
        }

        stage('Deploy Artifcats') {
            agent { label 'centos' }
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
            mail to: 'mike.vartanian@nih.gov',
                 subject: 'build failed',
                 body: 'something happened'
        }
    } // post


}

