#Valgrind���
Valgrind��һ��Linux�£�����Դ���루GPL V2���ķ�����Թ��ߵļ��ϣ�
����Linux������ڴ���Ժʹ���������Valgrind���ںˣ�core���Լ���
���ں˵��������Թ�����ɡ��ں�������һ����ܣ�framework������ģ
����һ��CPU���������ṩ������������ߣ������������������ڲ�� (plug-in)��
�����ں��ṩ�ķ�����ɸ����ض����ڴ������������֧�ֵ�ƽ̨�кܶ࣬
��X86/Linux��AMD64/Linux��ARM/Linux��PPC32/Linux��PPC64/Linux��S390X/Linux�ȡ�
Valgrind����Ƴɷ�����ʽ�ģ���ֱ�ӹ����ڿ�ִ���ļ��ϣ�����ڼ��ǰ����Ҫ����
���롢���Ӻ��޸���ĳ���
��Ŀ�ٷ���ַ��http://valgrind.org

-----------------------------------------------------------------------------
#Valgrind-extend�������
valgrind-extend����valgrind��Ϊ��̬��׮��ܣ������������Բ�ͬ�����������
������ɼ�⣺
1�������������ͨ��������α���������Ϣ��Ϊÿ����������һ��max����Ϊ
�������ܱ�ʾ�����ֵ��ͨ���Բ������ķ�������������һ����������������
�Ƚϣ��õ��ñ��ʽ�Ƿ�����������������+��-��*��/���������ܻ������
����������Χ�������������ȱ�ݣ�

2����������ת������;�Զ����Ʋ�׮���ValgrindΪ���������������ƶϷ���ʶ
�����ͱ����ķ���������Ϣ���õ��ڴ���ؿ⺯����Ϊ��ͻ���Ͳ����ļ��ϣ���
������ΪǱ�ڵ����ͷ���ת��ȱ�ݺ�ѡ����Ȼ�����м�����������������
����ʱ��⣬����ȷ�����������ͷ���ת��ȱ�ݡ�

����������λ����Ŀ¼��integerĿ¼�£����η���ת�����λ��sconvcheckĿ¼��
valgrind-extend���Ŀǰ֧�ֻ���X86��arm��linuxϵͳ�������ܹ���ϵͳ�ݲ�֧�֡�

#Valgrind-extendʹ�÷���
##��װ��
1.���Խ�������Ŀclone�£�Ȼ����valgrind�ٷ�make��make install���뷽������

2.������Ѿ���װ�ٷ�valgrind��ֻ�����ض�Ӧ����ļ��У�Ȼ���չٷ�ָ���޸���Ŀ��Ŀ¼Makefile.am���ļ������������integer/sconvcheck������TOOLS��EXP_TOOLS���������޸�configure.in�ļ�����/sconvcheck/Makefile��sconvcheck/tests/Makefile��AC_OUTPUT�б�integer���ͬ����

Ȼ������Ŀ¼�����������У�
autogen.sh
./configure --prefix=��pwd��/inst
make
make install
��װ�޴�����ʾ���ɹ���

##ʹ�ã�
��Valgrind�������ܲ��һ����ʹ��������valgrind --tool='toolname' ./'binary' �������ж�Ӧ���ܲ���

����'toolname'Ϊ�����������Ŀ�ж�ӦΪinteger��sconvcheck��'binary'Ϊ�����������Ķ������ļ�

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
