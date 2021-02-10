node('windows') {
	  deleteDir()

    stage('Checkout') {
        checkout scm
        sh 'git submodule update --init --recursive'
    }
}
