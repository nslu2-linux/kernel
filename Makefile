# Makefile for building the IXP4xx kernel.
#
#  Based on the build script for the Loft from Giant Shoulder, Inc.
#    written by Tommy B.
#  Based on the great work done in OpenEmbedded for the Linksys NSLU2
#    written by the nslu2-linux development community.
#
#  This builds the kernel in the linux-${REVISION} directory.
#  It then installs all the kernel modules in lib
#  and creates a tarball there containing all the modules 
#  under the lib/modules/${REVISION} directory.
#
#  N.B. You may need to change your crosstool path
#


# ENDIAN = l
ENDIAN = b
MAJORVER = 2.6

# Previous Stable
# BASEVER  = 2.6.23.14
# PATCHVER = 2.6.23

# Latest Stable
BASEVER  = 2.6.24.4
PATCHVER = 2.6.24

# Latest Development
# BASEVER  = 2.6.24
# PATCHVER = 2.6.25

# CROSS_COMPILE = /home/slug/angstrom/tmp/cross/bin/${ARCH}-angstrom-linux-gnueabi-

REVISION := $(shell sed -e 's/-\(git\|v\).*//' patches/${PATCHVER}/KERNEL)
SNAPSHOT := $(shell sed -e 's/-v.*//' patches/${PATCHVER}/KERNEL)
COMMITID := $(shell sed -e 's/.*-\(v[0-9.]*.*\)/\1/' patches/${PATCHVER}/KERNEL)

U-BOOT_COMMIT = v1.3.2

APEX_REVISION = 1.5.13
APEX_CONFIG = slugos

ARM_KERNEL_SHIM_REVISION = 1.5

DEFCONFIG=defconfig

ifeq (${ENDIAN},b)
ARCH = armeb
else
ARCH = arm
endif

KERNEL_SOURCE   = http://www2.kernel.org/pub/linux/kernel/v${MAJORVER}/linux-${BASEVER}.tar.bz2
KERNEL_PATCH    = http://www2.kernel.org/pub/linux/kernel/v${MAJORVER}/testing/patch-${REVISION}.bz2
KERNEL_SNAPSHOT = http://www2.kernel.org/pub/linux/kernel/v${MAJORVER}/snapshots/patch-${SNAPSHOT}.bz2

APEX_SOURCE	= ftp://ftp.buici.com/pub/apex/apex-${APEX_REVISION}.tar.gz

ARM_KERNEL_SHIM_SOURCE = ftp://ftp.buici.com/pub/arm/arm-kernel-shim/arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION}.tar.gz

U-BOOT_SITE	= http://www.denx.de/cgi-bin/gitweb.cgi?p=u-boot.git;a=snapshot;sf=tgz
U-BOOT_SNAPSHOT	= ${U-BOOT_SITE};h=${U-BOOT_COMMIT}
U-BOOT_DIR	= u-boot

# CROSS_COMPILE ?= ${ARCH}-linux-gnu-
CROSS_COMPILE ?= ${ARCH}-linux-

ifdef CROSS_COMPILE
CROSS_COMPILE_FLAGS = CROSS_COMPILE=${CROSS_COMPILE}
else
CROSS_COMPILE_FLAGS = 
endif

all: kernel modules arm-kernel-shim apex u-boot

kernel: vmlinuz-nslu2-${SNAPSHOT}-${ARCH} vmlinuz-nas100d-${SNAPSHOT}-${ARCH} vmlinuz-ixp4xx-${SNAPSHOT}-${ARCH} vmlinuz-dsmg600-${SNAPSHOT}-${ARCH} vmlinuz-fsg3-${SNAPSHOT}-${ARCH}
modules: modules-${SNAPSHOT}-${ARCH}.tar.gz
patched: linux-${SNAPSHOT}-${ARCH}/.config 
u-boot uboot: u-boot-nslu2.bin
apex:   apex-${APEX_CONFIG}-nslu2-${ARCH}-${APEX_REVISION}.bin \
	apex-${APEX_CONFIG}-nslu2-16mb-${ARCH}-${APEX_REVISION}.bin \
	apex-${APEX_CONFIG}-nas100d-${ARCH}-${APEX_REVISION}.bin \
	apex-${APEX_CONFIG}-dsmg600-${ARCH}-${APEX_REVISION}.bin \
	apex-${APEX_CONFIG}-fsg3-${ARCH}-${APEX_REVISION}.bin
arm-kernel-shim: \
	arm-kernel-shim-nslu2${ENDIAN}e.bin \
	arm-kernel-shim-nas100d${ENDIAN}e.bin \
	arm-kernel-shim-dsmg600${ENDIAN}e.bin \
	arm-kernel-shim-fsg3${ENDIAN}e.bin

u-boot-nslu2.bin: \
		${U-BOOT_DIR}/include/configs/nslu2.h \
		${U-BOOT_DIR}/cpu/ixp/npe/IxNpeMicrocode.c
	${MAKE} -C ${U-BOOT_DIR} distclean
	${MAKE} -C ${U-BOOT_DIR} nslu2_config
	${MAKE} -C ${U-BOOT_DIR} all
ifeq (${ENDIAN},b)
	devio '<<'${U-BOOT_DIR}/u-boot.bin >$@ 'cp$$'
else
	devio '<<'${U-BOOT_DIR}/u-boot.bin >$@ 'xp $$,4'
endif

.PRECIOUS: ${U-BOOT_DIR}/cpu/ixp/npe/IxNpeMicrocode.c
${U-BOOT_DIR}/cpu/ixp/npe/IxNpeMicrocode.c: \
		downloads/IPL_ixp400NpeLibrary-2_1.zip
	unzip -p -j downloads/IPL_ixp400NpeLibrary-2_1.zip \
		ixp400_xscale_sw/src/npeDl/IxNpeMicrocode.c \
		> ${U-BOOT_DIR}/cpu/ixp/npe/IxNpeMicrocode.c

.PRECIOUS: ${U-BOOT_DIR}/include/configs/nslu2.h

${U-BOOT_DIR}/include/configs/nslu2.h: \
		downloads/u-boot-${U-BOOT_COMMIT}.tar.gz
	[ -e ${U-BOOT_DIR} ] || \
	( tar zxf downloads/u-boot-${U-BOOT_COMMIT}.tar.gz ; \
	  cd ${U-BOOT_DIR} ; \
	  ln -s ../patches/u-boot patches ; \
	  [ ! -e patches/series ] || quilt push -a )
	touch ${U-BOOT_DIR}/include/configs/nslu2.h

downloads/u-boot-${U-BOOT_COMMIT}.tar.gz:
	[ -e downloads/u-boot-${U-BOOT_COMMIT}.tar.gz ] || \
	( mkdir -p downloads ; cd downloads ; \
	  wget -O u-boot-${U-BOOT_COMMIT}.tar.gz '${U-BOOT_SNAPSHOT}' )

apex-${APEX_CONFIG}-%-${ARCH}-${APEX_REVISION}.bin: apex-${APEX_REVISION}/src/mach-ixp42x/${APEX_CONFIG}-%-${ARCH}_config
	( cd apex-${APEX_REVISION} ; \
	  ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=arm clean ; \
	  ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=arm ${APEX_CONFIG}-$*-${ARCH}_config )
	( cd apex-${APEX_REVISION} ; \
	  ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=arm all )
ifeq (${ENDIAN},b)
	devio '<<'apex-${APEX_REVISION}/apex.bin >$@ 'cp$$'
else
	devio '<<'apex-${APEX_REVISION}/apex.bin >$@ 'xp $$,4'
endif

.PRECIOUS: apex-${APEX_REVISION}/src/mach-ixp42x/${APEX_CONFIG}-%-${ARCH}_config

apex-${APEX_REVISION}/src/mach-ixp42x/${APEX_CONFIG}-%-${ARCH}_config: \
		downloads/apex-${APEX_REVISION}.tar.gz
	[ -e apex-${APEX_REVISION} ] || \
	( tar zxf downloads/apex-${APEX_REVISION}.tar.gz ; \
	  cd apex-${APEX_REVISION} ; \
	  ln -s ../patches/apex patches ; \
	  [ ! -e patches/series ] || quilt push -a )
	touch apex-${APEX_REVISION}/Makefile

downloads/apex-${APEX_REVISION}.tar.gz :
	[ -e downloads/apex-${APEX_REVISION}.tar.gz ] || \
	( mkdir -p downloads ; cd downloads ; \
	  wget ${APEX_SOURCE} )

arm-kernel-shim-%${ENDIAN}e.bin: arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION}/config-%${ENDIAN}e.h
	( cd arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION} ; \
	  mv config.h config.h.orig ; cp config-$*${ENDIAN}e.h config.h ; \
	  ${MAKE} ${CROSS_COMPILE_FLAGS} clean arm-kernel-shim.bin )
	cp arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION}/arm-kernel-shim.bin \
		arm-kernel-shim-$*${ENDIAN}e.bin

arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION}/config-%${ENDIAN}e.h: \
		downloads/arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION}.tar.gz
	[ -e arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION} ] || \
	( tar zxf downloads/arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION}.tar.gz ; \
	  cd arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION} ; \
	  ln -s ../patches/arm-kernel-shim patches ; \
	  [ ! -e patches/series ] || quilt push -a )
ifeq (${ENDIAN},b)
	sed -e 's|//#define FORCE_BIGENDIAN|#define FORCE_BIGENDIAN|' patches/arm-kernel-shim/config-$*.h \
		> arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION}/config-$*${ENDIAN}e.h
else
	sed -e 's|//#define FORCE_LITTLEENDIAN|#define FORCE_LITTLEENDIAN|' patches/arm-kernel-shim/config-$*.h \
		> arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION}/config-$*${ENDIAN}e.h
endif

downloads/arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION}.tar.gz :
	[ -e downloads/arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION}.tar.gz ] || \
	( mkdir -p downloads ; cd downloads ; \
	  wget ${ARM_KERNEL_SHIM_SOURCE} )

modules-${SNAPSHOT}-${ARCH}.tar.gz: vmlinuz-${SNAPSHOT}-${ARCH}
	( cd linux-${SNAPSHOT}-${ARCH} ; \
	  INSTALL_MOD_PATH="../modules-${SNAPSHOT}-${ARCH}" ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=arm modules modules_install ) || true
	rm -f modules-${SNAPSHOT}-${ARCH}/lib/modules/${SNAPSHOT}/build modules-${SNAPSHOT}-${ARCH}/lib/modules/${SNAPSHOT}/source
ifeq (${SNAPSHOT},${BASEVER})
	tar -C modules-${SNAPSHOT}-${ARCH} -zcf modules-${SNAPSHOT}-${ARCH}.tar.gz .
else
	tar -C modules-${SNAPSHOT}-${ARCH} -zcf modules-${SNAPSHOT}-${ARCH}.tar.gz .
endif

vmlinuz-ixp4xx-${SNAPSHOT}-${ARCH}: vmlinuz-${SNAPSHOT}-${ARCH}
ifeq (${ENDIAN},b)
	devio '<<'$< >$@ \
		'cp$$'
else
	devio '<<'$< >$@ \
		'wb 0xee110f10,4' \
		'wb 0xe3c00080,4' \
		'wb 0xee010f10,4' \
		'xp $$,4'
endif

vmlinuz-%-${SNAPSHOT}-${ARCH}: vmlinuz-${SNAPSHOT}-${ARCH} arm-kernel-shim-%${ENDIAN}e.bin
ifeq (${ENDIAN},b)
	cat arm-kernel-shim-$*${ENDIAN}e.bin vmlinuz-${SNAPSHOT}-${ARCH} > $@
else
	( cat arm-kernel-shim-$*${ENDIAN}e.bin vmlinuz-${SNAPSHOT}-${ARCH} > $$$$ ; \
	  devio '<<'$$$$ >$@ 'xp $$,4' ; \
	  rm -f $$$$ )
endif

vmlinuz-${SNAPSHOT}-${ARCH}: linux-${SNAPSHOT}-${ARCH}/.config
	( cd linux-${SNAPSHOT}-${ARCH} ; \
	  ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=arm bzImage )
	cp linux-${SNAPSHOT}-${ARCH}/arch/arm/boot/zImage vmlinuz-${SNAPSHOT}-${ARCH}

menuconfig: linux-${SNAPSHOT}-${ARCH}/.config
	${MAKE} -C linux-${SNAPSHOT}-${ARCH} ARCH=arm CROSS_COMPILE=${CROSS_COMPILE} menuconfig

ifneq (${SNAPSHOT},${COMMITID})
linux-${SNAPSHOT}-${ARCH}/.config: \
		patches/${PATCHVER}/$(DEFCONFIG)
	[ -e linux-${SNAPSHOT}-${ARCH} ] || \
	( git clone -q \
	  git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux-2.6.git \
	  linux-${SNAPSHOT}-${ARCH} )
	( cd linux-${SNAPSHOT}-${ARCH} ; \
	  git checkout master ; \
	  git reset --hard ${COMMITID} ; \
	  git clean -d -x )
	( cd linux-${SNAPSHOT}-${ARCH} ; \
	  ln -s ../patches/${PATCHVER} patches ; \
	  quilt push -a )
else
ifeq (${SNAPSHOT},${BASEVER})
linux-${SNAPSHOT}-${ARCH}/.config: \
		downloads/linux-${BASEVER}.tar.bz2 \
		patches/${PATCHVER}/$(DEFCONFIG)
	[ -e linux-${SNAPSHOT}-${ARCH} ] || \
	( tar xjf downloads/linux-${BASEVER}.tar.bz2 ; \
	  mv linux-${BASEVER} linux-${SNAPSHOT}-${ARCH} ; \
	  cd linux-${SNAPSHOT}-${ARCH} ; \
	  ln -s ../patches/${PATCHVER} patches ; \
	  quilt push -a )
else
ifeq (${REVISION},${SNAPSHOT})
linux-${SNAPSHOT}-${ARCH}/.config: \
		downloads/linux-${BASEVER}.tar.bz2 \
		downloads/patch-${REVISION}.bz2 \
		patches/${PATCHVER}/$(DEFCONFIG)
	[ -e linux-${SNAPSHOT}-${ARCH} ] || \
	( tar xjf downloads/linux-${BASEVER}.tar.bz2 ; \
	  mv linux-${BASEVER} linux-${SNAPSHOT}-${ARCH} ; \
	  bzcat downloads/patch-${REVISION}.bz2 | \
	  patch -d linux-${SNAPSHOT}-${ARCH} -p1 ; \
	  cd linux-${SNAPSHOT}-${ARCH} ; \
	  ln -s ../patches/${PATCHVER} patches ; \
	  quilt push -a )
else
ifeq (${REVISION},${BASEVER})
linux-${SNAPSHOT}-${ARCH}/.config: \
		downloads/linux-${BASEVER}.tar.bz2 \
		downloads/patch-${SNAPSHOT}.bz2 \
		patches/${PATCHVER}/$(DEFCONFIG)
	[ -e linux-${SNAPSHOT}-${ARCH} ] || \
	( tar xjf downloads/linux-${BASEVER}.tar.bz2 ; \
	  mv linux-${BASEVER} linux-${SNAPSHOT}-${ARCH} ; \
	  bzcat downloads/patch-${SNAPSHOT}.bz2 | \
	  patch -d linux-${SNAPSHOT}-${ARCH} -p1 ; \
	  cd linux-${SNAPSHOT}-${ARCH} ; \
	  ln -s ../patches/${PATCHVER} patches ; \
	  quilt push -a )
else
linux-${SNAPSHOT}-${ARCH}/.config: \
		downloads/linux-${BASEVER}.tar.bz2 \
		downloads/patch-${REVISION}.bz2 \
		downloads/patch-${SNAPSHOT}.bz2 \
		patches/${PATCHVER}/$(DEFCONFIG)
	[ -e linux-${SNAPSHOT}-${ARCH} ] || \
	( tar xjf downloads/linux-${BASEVER}.tar.bz2 ; \
	  mv linux-${BASEVER} linux-${SNAPSHOT}-${ARCH} ; \
	  bzcat downloads/patch-${REVISION}.bz2 downloads/patch-${SNAPSHOT}.bz2 | \
	  patch -d linux-${SNAPSHOT}-${ARCH} -p1 ; \
	  cd linux-${SNAPSHOT}-${ARCH} ; \
	  ln -s ../patches/${PATCHVER} patches ; \
	  quilt push -a )
endif
endif
endif
endif
ifeq (${ENDIAN},b)
	sed -e 's/.*CONFIG_CPU_BIG_ENDIAN.*/CONFIG_CPU_BIG_ENDIAN=y/' \
		< patches/${PATCHVER}/$(DEFCONFIG) > linux-${SNAPSHOT}-${ARCH}/.config
else
	sed -e 's/.*CONFIG_CPU_BIG_ENDIAN.*/\# CONFIG_CPU_BIG_ENDIAN is not set/' \
		< patches/${PATCHVER}/$(DEFCONFIG) > linux-${SNAPSHOT}-${ARCH}/.config
endif

downloads/linux-${BASEVER}.tar.bz2 :
	[ -e downloads/linux-${BASEVER}.tar.bz2 ] || \
	( mkdir -p downloads ; cd downloads ; \
	  wget ${KERNEL_SOURCE} )

downloads/patch-${REVISION}.bz2 :
	[ -e downloads/patch-${REVISION}.bz2 ] || \
	( mkdir -p downloads ; cd downloads ; \
	  wget ${KERNEL_PATCH} )

ifneq (${REVISION},${SNAPSHOT})
downloads/patch-${SNAPSHOT}.bz2 :
	[ -e downloads/patch-${SNAPSHOT}.bz2 ] || \
	( mkdir -p downloads ; cd downloads ; \
	  wget ${KERNEL_SNAPSHOT} )
endif

downloads:
	mkdir -p downloads

clobber: clobber-apex clobber-arm-kernel-shim clobber-u-boot
	rm -rf vmlinuz-* modules-*.tar.gz
	rm -rf linux-* modules-*
	rm -rf *~

clobber-apex:
	rm -rf apex-*

clobber-u-boot clobber-uboot:
	rm -rf u-boot-* ${U-BOOT_DIR}

clobber-arm-kernel-shim:
	rm -rf arm-kernel-shim-*

.PHONY: all kernel menuconfig modules clobber apex clobber-apex clobber-arm-kernel-shim

# End of Makefile

