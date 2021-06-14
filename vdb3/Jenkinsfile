pipeline
{
    agent
    {
        docker
        {
            image 'ncbi-vdb3.ubuntu.ci'
            args '-u root -v /var/run/docker.sock:/var/run/docker.sock' // not sure if -u root is a good idea
        }
    }
    stages {
        stage('build') {
            steps {
                sh 'rm -f Makefile.config;make DEFAULT_OUTDIR=outdir DEFAULT_BUILD=dbg test'
            }
        }
        stage('docs') {
            steps {
                sh 'make docs'
            }
        }
        stage('package') {
            steps {
                sh 'make package'
            }
        }
    }
}
