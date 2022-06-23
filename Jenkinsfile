def SW_NODE = "sw"

node(SW_NODE) {
	  deleteDir()

    stage('Checkout') {
        checkout scm
		bat '''
			git submodule update --init --recursive
		'''
    }

	stage('Build externals') {
		bat '''
			cmake -Hexternal/libxml2 -Bexternal/libxml2/_build -DCMAKE_INSTALL_PREFIX=_install
			cmake --build external/libxml2/_build --target install

			cmake -Hexternal/SOEM/ -B external/SOEM/_build -DCMAKE_INSTALL_PREFIX=_install -DCMAKE_POSITION_INDEPENDENT_CODE=ON
			cmake --build external/SOEM/_build --target install
		'''
	}

	stage('Build libraries') {
		bat '''
			cmake -H. -B_build -DCMAKE_INSTALL_PREFIX=_install -DWITH_PROT_ETH=ON -DWITH_PROT_ECAT=ON -DWITH_EXAMPLES=ON
			cmake --build _build
		'''
	}
}
