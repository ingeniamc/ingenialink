cmake -H../../ -B ../../external/sercomm/_build -DCMAKE_INSTALL_PREFIX=../../_install
cmake --build ../../external/sercomm/_build --target install

cmake -H../../ -B ../../external/libxml2/_build -DCMAKE_INSTALL_PREFIX=../../_install
cmake --build ../../external/libxml2/_build --target install

cmake -H../../ -B../../_build -DCMAKE_INSTALL_PREFIX=../../_install -DWITH_PROT_MCB=ON
cmake --build ../../_build
cmake --build ../../_build --target install


copy ..\..\_install\bin\ingenialink.dll D:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\bin\sercomm.dll D:\GitProjects\ConsoleApplication2\ConsoleApplication2

copy ..\..\_install\bin\ingenialink.dll D:\GitProjects\ConsoleApplication2\Debug
copy ..\..\_install\bin\sercomm.dll D:\GitProjects\ConsoleApplication2\Debug

copy ..\..\_install\lib\ingenialink.lib D:\GitProjects\ConsoleApplication2
copy ..\..\_install\include\ingenialink\common.h D:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\config.h D:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\const.h D:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\dict.h D:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\dict_labels.h D:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\err.h D:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\monitor.h D:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\net.h D:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\poller.h D:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\registers.h D:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\servo.h D:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\version.h D:\GitProjects\ConsoleApplication2\ConsoleApplication2