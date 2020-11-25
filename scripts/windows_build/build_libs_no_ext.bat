cmake -H../../ -B../../_build -DCMAKE_INSTALL_PREFIX=../../_install -DWITH_PROT_MCB=ON -DWITH_PROT_ETH=ON -DWITH_PROT_ECAT=ON
cmake --build ../../_build
cmake --build ../../_build --target install

copy ..\..\_install\bin\ingenialink.dll C:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\bin\ingenialink.dll C:\GitProjects\ConsoleApplication2\Debug
copy ..\..\_install\lib\ingenialink.lib C:\GitProjects\ConsoleApplication2
copy ..\..\_install\include\ingenialink\common.h C:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\config.h C:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\const.h C:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\dict.h C:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\dict_labels.h C:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\err.h C:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\monitor.h C:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\net.h C:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\poller.h C:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\registers.h C:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\servo.h C:\GitProjects\ConsoleApplication2\ConsoleApplication2
copy ..\..\_install\include\ingenialink\version.h C:\GitProjects\ConsoleApplication2\ConsoleApplication2