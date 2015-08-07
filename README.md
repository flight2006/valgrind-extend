#Valgrind简介
Valgrind是一套Linux下，开放源代码（GPL V2）的仿真调试工具的集合，
用于Linux程序的内存调试和代码剖析。Valgrind由内核（core）以及基
于内核的其他调试工具组成。内核类似于一个框架（framework），它模
拟了一个CPU环境，并提供服务给其他工具；而其他工具则类似于插件 (plug-in)，
利用内核提供的服务完成各种特定的内存调试任务。它所支持的平台有很多，
如X86/Linux，AMD64/Linux，ARM/Linux，PPC32/Linux，PPC64/Linux，S390X/Linux等。
Valgrind被设计成非侵入式的，它直接工作于可执行文件上，因此在检查前不需要重新
编译、连接和修改你的程序。
项目官方地址：http://valgrind.org

-----------------------------------------------------------------------------
#Valgrind-extend插件介绍
valgrind-extend利用valgrind作为动态插桩框架，设计了两个针对不同整数错误检测的
插件，可检测：
1）整数溢出错误；通过获得整形变量类型信息，为每个类型设置一个max常量为
该类型能表示的最大值，通过对操作数的方向运算与另外一个或多个操作数进行
比较，得到该表达式是否发生整数溢出。运算符+、-、*、/计算结果可能会产生的
超出声明范围整数的整数溢出缺陷；

2）整数符号转换错误;以二进制插桩框架Valgrind为基础，利用类型推断方法识
别整型变量的符号类型信息，得到内存相关库函数中为冲突类型参数的集合，并
将其作为潜在的整型符号转换缺陷候选集。然后在中间代码层面插入检测代码做
运行时检测，最终确定真正的整型符号转换缺陷。

整数溢出插件位于主目录下integer目录下，整形符号转换插件位于sconvcheck目录。
valgrind-extend插件目前支持基于X86和arm的linux系统，其他架构或系统暂不支持。

#Valgrind-extend使用方法
##安装：
1.可以将整个项目clone下，然后按照valgrind官方make，make install编译方法编译

2.如果你已经安装官方valgrind，只需下载对应插件文件夹，然后按照官方指南修改项目主目录Makefile.am中文件，将插件名（integer/sconvcheck）加入TOOLS或EXP_TOOLS变量，再修改configure.in文件加入/sconvcheck/Makefile和sconvcheck/tests/Makefile到AC_OUTPUT列表（integer插件同理），

然后在主目录下运行命令行：
autogen.sh
./configure --prefix=‘pwd‘/inst
make
make install
安装无错误提示即成功。

##使用：
像Valgrind其他功能插件一样，使用命令行valgrind --tool='toolname' ./'binary' 即可运行对应功能部件

其中'toolname'为插件名，本项目中对应为integer和sconvcheck，'binary'为待测程序编译后的二进制文件

------------------------------------------------------------------------------

#Release notes for Valgrind
~~~~~~~~~~~~~~~~~~~~~~~~~~
If you are building a binary package of Valgrind for distribution,
please read README_PACKAGERS.  It contains some important information.

If you are developing Valgrind, please read README_DEVELOPERS.  It contains
some useful information.

For instructions on how to build/install, see the end of this file.

If you have problems, consult the FAQ to see if there are workarounds.


Executive Summary
~~~~~~~~~~~~~~~~~
Valgrind is a framework for building dynamic analysis tools. There are
Valgrind tools that can automatically detect many memory management
and threading bugs, and profile your programs in detail. You can also
use Valgrind to build new tools.

The Valgrind distribution currently includes six production-quality
tools: a memory error detector, two thread error detectors, a cache
and branch-prediction profiler, a call-graph generating cache abd
branch-prediction profiler, and a heap profiler. It also includes
three experimental tools: a heap/stack/global array overrun detector,
a different kind of heap profiler, and a SimPoint basic block vector
generator.

Valgrind is closely tied to details of the CPU, operating system and to
a lesser extent, compiler and basic C libraries. This makes it difficult
to make it portable.  Nonetheless, it is available for the following
platforms: 

- X86/Linux
- AMD64/Linux
- PPC32/Linux
- PPC64/Linux
- ARM/Linux
- x86/MacOSX
- AMD64/MacOSX
- S390X/Linux
- MIPS32/Linux

Note that AMD64 is just another name for x86_64, and Valgrind runs fine
on Intel processors.  Also note that the core of MacOSX is called
"Darwin" and this name is used sometimes.

Valgrind is licensed under the GNU General Public License, version 2. 
Read the file COPYING in the source distribution for details.

However: if you contribute code, you need to make it available as GPL
version 2 or later, and not 2-only.


Documentation
~~~~~~~~~~~~~
A comprehensive user guide is supplied.  Point your browser at
$PREFIX/share/doc/valgrind/manual.html, where $PREFIX is whatever you
specified with --prefix= when building.


Building and installing it
~~~~~~~~~~~~~~~~~~~~~~~~~~
To install from the Subversion repository :

  0. Check out the code from SVN, following the instructions at
     http://www.valgrind.org/downloads/repository.html.

  1. cd into the source directory.

  2. Run ./autogen.sh to setup the environment (you need the standard
     autoconf tools to do so).

  3. Continue with the following instructions...

To install from a tar.bz2 distribution:

  4. Run ./configure, with some options if you wish.  The only interesting
     one is the usual --prefix=/where/you/want/it/installed.

  5. Run "make".

  6. Run "make install", possibly as root if the destination permissions
     require that.

  7. See if it works.  Try "valgrind ls -l".  Either this works, or it
     bombs out with some complaint.  In that case, please let us know
     (see www.valgrind.org).

Important!  Do not move the valgrind installation into a place
different from that specified by --prefix at build time.  This will
cause things to break in subtle ways, mostly when Valgrind handles
fork/exec calls.


The Valgrind Developers
