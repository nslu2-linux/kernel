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


ENDIAN = l
#ENDIAN = b
MAJORVER = 2.6
BASEVER  = 2.6.20
PATCHVER = 2.6.20
REVISION := $(shell sed -e 's/-git.*//' patches/${PATCHVER}/KERNEL)
SNAPSHOT := $(shell cat patches/${PATCHVER}/KERNEL)

MACHINE = nslu2

APEX_REVISION = 1.4.11
APEX_CONFIG = slugos

ARM_KERNEL_SHIM_REVISION = 1.3

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

#CROSS_COMPILE = ${ARCH}-linux-gnu-
CROSS_COMPILE ?= ${ARCH}-linux-

ifdef CROSS_COMPILE
CROSS_COMPILE_FLAGS = CROSS_COMPILE=${CROSS_COMPILE}
else
CROSS_COMPILE_FLAGS = 
endif

all: kernel modules arm-kernel-shim

kernel: vmlinuz-nslu2-${SNAPSHOT}-${ARCH} vmlinuz-nas100d-${SNAPSHOT}-${ARCH} vmlinuz-ixp4xx-${SNAPSHOT}-${ARCH} vmlinuz-dsmg600-${SNAPSHOT}-${ARCH} vmlinuz-fsg3-${SNAPSHOT}-${ARCH}
modules: modules-${SNAPSHOT}-${ARCH}.tar.gz
patched: linux-${SNAPSHOT}-${ARCH}/.config 
apex: apex-${APEX_CONFIG}-${MACHINE}-${ARCH}-${APEX_REVISION}.bin
arm-kernel-shim: arm-kernel-shim-${MACHINE}${ENDIAN}e.bin

apex-${APEX_CONFIG}-${MACHINE}-${ARCH}-${APEX_REVISION}.bin: apex-${APEX_REVISION}/.config
	( cd apex-${APEX_REVISION} ; \
	  ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=arm all )
ifeq (${ENDIAN},b)
	devio '<<'apex-${APEX_REVISION}/apex.bin >$@ 'cp$$'
else
	devio '<<'apex-${APEX_REVISION}/apex.bin >$@ 'xp $$,4'
endif

apex-${APEX_REVISION}/.config: apex-${APEX_REVISION}/src/mach-ixp42x/${APEX_CONFIG}-${MACHINE}-${ARCH}_config
	( cd apex-${APEX_REVISION} ; \
	  ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=arm clean ; \
	  ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=arm ${APEX_CONFIG}-${MACHINE}-${ARCH}_config )

apex-${APEX_REVISION}/src/mach-ixp42x/${APEX_CONFIG}-${MACHINE}-${ARCH}_config: \
		downloads/apex-${APEX_REVISION}.tar.gz
	[ -e apex-${APEX_REVISION} ] || \
	( tar zxf downloads/apex-${APEX_REVISION}.tar.gz ; \
	  cd apex-${APEX_REVISION} ; \
	  ln -s ../patches/apex patches ; \
	  quilt push -a )
	touch apex-${APEX_REVISION}/Makefile

downloads/apex-${APEX_REVISION}.tar.gz :
	[ -e downloads/apex-${APEX_REVISION}.tar.gz ] || \
	( mkdir -p downloads ; cd downloads ; \
	  wget ${APEX_SOURCE} )

arm-kernel-shim-${MACHINE}${ENDIAN}e.bin: arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION}/config-${MACHINE}.h
	( cd arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION} ; \
	  rm -f config.h ; cp config-${MACHINE}.h config.h ; \
	  ${MAKE} ${CROSS_COMPILE_FLAGS} clean arm-kernel-shim.bin )
	cp arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION}/arm-kernel-shim.bin \
		arm-kernel-shim-${MACHINE}${ENDIAN}e.bin

arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION}/config-${MACHINE}.h: \
		downloads/arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION}.tar.gz
	[ -e arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION} ] || \
	( tar zxf downloads/arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION}.tar.gz \
		--transform='s|/${ARM_KERNEL_SHIM_REVISION}||' ; \
	  cd arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION} ; \
	  ln -s ../patches/arm-kernel-shim patches ; \
	  [ ! -e patches/series ] || quilt push -a )
	cp patches/arm-kernel-shim/config-${MACHINE}.h \
		arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION}/config-${MACHINE}.h

downloads/arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION}.tar.gz :
	[ -e downloads/arm-kernel-shim-${ARM_KERNEL_SHIM_REVISION}.tar.gz ] || \
	( mkdir -p downloads ; cd downloads ; \
	  wget ${ARM_KERNEL_SHIM_SOURCE} )

modules-${SNAPSHOT}-${ARCH}.tar.gz: vmlinuz-${SNAPSHOT}-${ARCH}
	tar -C modules-${SNAPSHOT}-${ARCH} -zcf modules-${SNAPSHOT}-${ARCH}.tar.gz lib/modules/${SNAPSHOT}

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

vmlinuz-nas100d-${SNAPSHOT}-${ARCH}: vmlinuz-${SNAPSHOT}-${ARCH}
ifeq (${ENDIAN},b)
	devio '<<'$< >$@ \
		'wb 0xe3a01c03,4' 'wb 0xe3811061,4' \
		'cp$$'
else
	devio '<<'$< >$@ \
		'wb 0xe3a01c03,4' 'wb 0xe3811061,4' \
		'wb 0xee110f10,4' \
		'wb 0xe3c00080,4' \
		'wb 0xee010f10,4' \
		'xp $$,4'
endif

vmlinuz-nslu2-${SNAPSHOT}-${ARCH}: vmlinuz-${SNAPSHOT}-${ARCH}
ifeq (${ENDIAN},b)
	devio '<<'$< >$@ \
		'wb 0xe3a01c02,4' 'wb 0xe3811055,4' \
		'cp$$'
else
	devio '<<'$< >$@ \
		'wb 0xe3a01c02,4' 'wb 0xe3811055,4' \
		'wb 0xee110f10,4' \
		'wb 0xe3c00080,4' \
		'wb 0xee010f10,4' \
		'xp $$,4'
endif

vmlinuz-dsmg600-${SNAPSHOT}-${ARCH}: vmlinuz-${SNAPSHOT}-${ARCH}
ifeq (${ENDIAN},b)
	devio '<<'$< >$@ \
		'wb 0xe3a01c03,4' 'wb 0xe38110c4,4' \
		'cp$$'
else
		devio '<<'$< >$@ \
		'wb 0xe3a01c03,4' 'wb 0xe38110c4,4' \
		'wb 0xee110f10,4' \
		'wb 0xe3c00080,4' \
		'wb 0xee010f10,4' \
		'xp $$,4'
endif


vmlinuz-fsg3-${SNAPSHOT}-${ARCH}: vmlinuz-${SNAPSHOT}-${ARCH}
ifeq (${ENDIAN},b)
	devio '<<'$< >$@ \
		'wb 0xe3a01c04,4' 'wb 0xe3811043,4' \
		'cp$$'
else
		devio '<<'$< >$@ \
		'wb 0xe3a01c04,4' 'wb 0xe3811043,4' \
		'wb 0xee110f10,4' \
		'wb 0xe3c00080,4' \
		'wb 0xee010f10,4' \
		'xp $$,4'
endif


vmlinuz-${SNAPSHOT}-${ARCH}: linux-${SNAPSHOT}-${ARCH}/.config
	( cd linux-${SNAPSHOT}-${ARCH} ; \
	  ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=arm bzImage modules )
	( cd linux-${SNAPSHOT}-${ARCH} ; \
	  INSTALL_MOD_PATH="../modules-${SNAPSHOT}-${ARCH}" ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=arm modules_install )
	rm -f modules-${SNAPSHOT}-${ARCH}/lib/modules/${SNAPSHOT}/build modules-${SNAPSHOT}-${ARCH}/lib/modules/${SNAPSHOT}/source
	cp linux-${SNAPSHOT}-${ARCH}/arch/arm/boot/zImage vmlinuz-${SNAPSHOT}-${ARCH}

menuconfig: linux-${SNAPSHOT}-${ARCH}/.config
	${MAKE} -C linux-${SNAPSHOT}-${ARCH} ARCH=arm CROSS_COMPILE=${CROSS_COMPILE} menuconfig

ifeq (${SNAPSHOT},${BASEVER})
linux-${SNAPSHOT}-${ARCH}/.config: \
		downloads/linux-${BASEVER}.tar.bz2 \
		patches/${PATCHVER}/series patches/${PATCHVER}/??-*.patch \
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
		patches/${PATCHVER}/series patches/${PATCHVER}/??-*.patch \
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
linux-${SNAPSHOT}-${ARCH}/.config: \
		downloads/linux-${BASEVER}.tar.bz2 \
		downloads/patch-${REVISION}.bz2 \
		downloads/patch-${SNAPSHOT}.bz2 \
		patches/${PATCHVER}/series patches/${PATCHVER}/??-*.patch \
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

clobber: clobber-apex clobber-arm-kernel-shim
	rm -rf vmlinuz-* modules-*.tar.gz
	rm -rf linux-* modules-*
	rm -rf *~

clobber-apex:
	rm -rf apex-*

clobber-arm-kernel-shim:
	rm -rf arm-kernel-shim-*

.PHONY: all kernel menuconfig modules clobber apex clobber-apex clobber-arm-kernel-shim

# End of Makefile
