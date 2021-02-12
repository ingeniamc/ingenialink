node('windows') {
	  deleteDir()

    stage('Checkout') {
        checkout scm
		bat '''
			git submodule update --init --recursive
		'''
    }

	stage('Build externals') {
		bat '''
			cmake -Hexternal/sercomm -Bexternal/sercomm/_build -DCMAKE_INSTALL_PREFIX=_install
			cmake --build external/sercomm/_build --target install

			cmake -Hexternal/libxml2 -Bexternal/libxml2/_build -DCMAKE_INSTALL_PREFIX=_install
			cmake --build external/libxml2/_build --target install

			cmake -Hexternal/SOEM -Bexternal/SOEM/_build -DCMAKE_INSTALL_PREFIX=_install
			cmake --build external/SOEM/_build --target install

		'''
	}

	stage('Build libraries') {
		bat '''
			cmake -H. -B_build -DCMAKE_INSTALL_PREFIX=_install
			cmake --build _build
		'''
	}
}
