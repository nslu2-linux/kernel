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
BASEVER  = 2.6.19
PATCHVER = 2.6.20
REVISION := $(shell sed -e 's/-git.*//' patches/${PATCHVER}/KERNEL)
SNAPSHOT := $(shell cat patches/${PATCHVER}/KERNEL)

APEX_REVISION = 1.4.11
APEX_TARGET = nslu2
APEX_CONFIG = slugos

MADWIFIVER = r1503-20060415

DEFCONFIG=defconfig

ifeq (${ENDIAN},b)
DEBIAN_ARCH = armeb
else
DEBIAN_ARCH = arm
endif

KERNEL_SOURCE   = http://www2.kernel.org/pub/linux/kernel/v${MAJORVER}/linux-${BASEVER}.tar.bz2
KERNEL_PATCH    = http://www2.kernel.org/pub/linux/kernel/v${MAJORVER}/testing/patch-${REVISION}.bz2
KERNEL_SNAPSHOT = http://www2.kernel.org/pub/linux/kernel/v${MAJORVER}/snapshots/patch-${SNAPSHOT}.bz2

APEX_SOURCE	= ftp://ftp.buici.com/pub/apex/apex-${APEX_REVISION}.tar.gz

MADWIFI_SOURCE   = http://snapshots.madwifi.org/madwifi-ng/madwifi-ng-${MADWIFIVER}.tar.gz

#CROSS_COMPILE = ${DEBIAN_ARCH}-linux-gnu-
CROSS_COMPILE ?= ${DEBIAN_ARCH}-linux-

ifdef CROSS_COMPILE
CROSS_COMPILE_FLAGS = CROSS_COMPILE=${CROSS_COMPILE}
else
CROSS_COMPILE_FLAGS = 
endif

all: kernel modules apex

kernel: vmlinuz-nslu2-${SNAPSHOT} vmlinuz-nas100d-${SNAPSHOT} vmlinuz-ixp4xx-${SNAPSHOT} vmlinuz-dsmg600-${SNAPSHOT} vmlinuz-fsg3-${SNAPSHOT}
madwifi: lib/modules/${SNAPSHOT}/net/ath_hal.ko
modules: modules-${SNAPSHOT}.tar.gz
patched: linux-${SNAPSHOT}/.config 
apex: apex-${APEX_CONFIG}-${APEX_TARGET}-${DEBIAN_ARCH}-${APEX_REVISION}.bin

# ifeq (${APEX_TARGET},fsg3)
# 	sed -i -e '/CONFIG_MACH_FSG/d' apex-${APEX_REVISION}/.config
# 	echo 'CONFIG_MACH_FSG=y' >> apex-${APEX_REVISION}/.config
# 	echo 'CONFIG_ARCH_NUMBER=1091' >> apex-${APEX_REVISION}/.config
# endif

apex-${APEX_CONFIG}-${APEX_TARGET}-${DEBIAN_ARCH}-${APEX_REVISION}.bin: apex-${APEX_REVISION}/.config
	( cd apex-${APEX_REVISION} ; \
	  ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=arm all )
ifeq (${ENDIAN},b)
	devio '<<'apex-${APEX_REVISION}/apex.bin >$@ 'cp$$'
else
	devio '<<'apex-${APEX_REVISION}/apex.bin >$@ 'xp $$,4'
endif

apex-${APEX_REVISION}/.config: apex-${APEX_REVISION}/src/mach-ixp42x/${APEX_CONFIG}-${APEX_TARGET}-${DEBIAN_ARCH}_config
	( cd apex-${APEX_REVISION} ; \
	  ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=arm clean ; \
	  ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=arm ${APEX_CONFIG}-${APEX_TARGET}-${DEBIAN_ARCH}_config )

apex-${APEX_REVISION}/src/mach-ixp42x/${APEX_CONFIG}-${APEX_TARGET}-${DEBIAN_ARCH}_config: \
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

usr: usr-${SNAPSHOT}.tar.gz

usr-${SNAPSHOT}.tar.gz: vmlinuz-${SNAPSHOT} \
	 usr/local/bin/wlanconfig
	tar zcf usr-${SNAPSHOT}.tar.gz usr/local

# Add the following depedency if you want madwifi built.
#	 lib/modules/${SNAPSHOT}/net/ath_hal.ko

modules-${SNAPSHOT}.tar.gz: vmlinuz-${SNAPSHOT}
	tar zcf modules-${SNAPSHOT}.tar.gz lib/modules/${SNAPSHOT}

lib/modules/${SNAPSHOT}/net/ath_hal.ko usr/local/bin/wlanconfig: \
		madwifi-ng/Makefile \
		vmlinuz-${SNAPSHOT}
	rm -rf lib/modules/${SNAPSHOT}/net
	( cd madwifi-ng ; \
	  ${MAKE} CROSS_COMPILE=${CROSS_COMPILE} TOOLPATH=${CROSS_COMPILE} \
		TARGET=xscale-${ENDIAN}e-elf \
		KERNELPATH=`cd .. ; pwd`/linux-${SNAPSHOT} \
		DESTDIR=`cd .. ; pwd` \
		all install )

madwifi-ng/Makefile: \
		downloads/madwifi-ng-${MADWIFIVER}.tar.gz
	[ -e madwifi-ng ] || \
	( tar zxf downloads/madwifi-ng-${MADWIFIVER}.tar.gz ; \
	  mv madwifi-ng-${MADWIFIVER} madwifi-ng ; \
	  ( cd patches/madwifi ; \
	    grep -v \# series | \
	    while read f; do case "$$f" in http:*) wget -O - "$$f";; *) cat "$$f";; esac; done \
	  ) | \
	  patch -d madwifi-ng -p1 )
	touch $@

downloads/madwifi-ng-${MADWIFIVER}.tar.gz :
	[ -e downloads/madwifi-ng-${MADWIFIVER}.tar.gz ] || \
	( mkdir -p downloads ; cd downloads ; \
	  wget ${MADWIFI_SOURCE} )

vmlinuz-ixp4xx-${SNAPSHOT}: vmlinuz-${SNAPSHOT}
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

vmlinuz-nas100d-${SNAPSHOT}: vmlinuz-${SNAPSHOT}
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

vmlinuz-nslu2-${SNAPSHOT}: vmlinuz-${SNAPSHOT}
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

vmlinuz-dsmg600-${SNAPSHOT}: vmlinuz-${SNAPSHOT}
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


vmlinuz-fsg3-${SNAPSHOT}: vmlinuz-${SNAPSHOT}
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


vmlinuz-${SNAPSHOT}: linux-${SNAPSHOT}/.config
	( cd linux-${SNAPSHOT} ; \
	  ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=arm bzImage modules )
	( cd linux-${SNAPSHOT} ; \
	  INSTALL_MOD_PATH=".." ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=arm modules_install )
	rm -f lib/modules/${SNAPSHOT}/build lib/modules/${SNAPSHOT}/source
	cp linux-${SNAPSHOT}/arch/arm/boot/zImage vmlinuz-${SNAPSHOT}

menuconfig: linux-${SNAPSHOT}/.config
	${MAKE} -C linux-${SNAPSHOT} ARCH=arm CROSS_COMPILE=${CROSS_COMPILE} menuconfig

ifeq (${SNAPSHOT},${BASEVER})
linux-${SNAPSHOT}/.config: \
		downloads/linux-${BASEVER}.tar.bz2 \
		patches/${PATCHVER}/series patches/${PATCHVER}/??-*.patch patches/${PATCHVER}/$(DEFCONFIG)
	[ -e linux-${REVISION} ] || \
	( tar xjf downloads/linux-${BASEVER}.tar.bz2 ; \
	  cd linux-${SNAPSHOT} ; \
	  ln -s ../patches/${PATCHVER} patches ; \
	  quilt push -a )
else
ifeq (${REVISION},${SNAPSHOT})
linux-${SNAPSHOT}/.config: \
		downloads/linux-${BASEVER}.tar.bz2 \
		downloads/patch-${REVISION}.bz2 \
		patches/${PATCHVER}/series patches/${PATCHVER}/??-*.patch patches/${PATCHVER}/$(DEFCONFIG)
	[ -e linux-${REVISION} ] || \
	( tar xjf downloads/linux-${BASEVER}.tar.bz2 ; \
	  mv linux-${BASEVER} linux-${SNAPSHOT} ; \
	  bzcat downloads/patch-${REVISION}.bz2 | \
	  patch -d linux-${SNAPSHOT} -p1 ; \
	  cd linux-${SNAPSHOT} ; \
	  ln -s ../patches/${PATCHVER} patches ; \
	  quilt push -a )
else
linux-${SNAPSHOT}/.config: \
		downloads/linux-${BASEVER}.tar.bz2 \
		downloads/patch-${REVISION}.bz2 \
		downloads/patch-${SNAPSHOT}.bz2 \
		patches/${PATCHVER}/series patches/${PATCHVER}/??-*.patch patches/${PATCHVER}/$(DEFCONFIG)
	[ -e linux-${REVISION} ] || \
	( tar xjf downloads/linux-${BASEVER}.tar.bz2 ; \
	  mv linux-${BASEVER} linux-${SNAPSHOT} ; \
	  bzcat downloads/patch-${REVISION}.bz2 downloads/patch-${SNAPSHOT}.bz2 | \
	  patch -d linux-${SNAPSHOT} -p1 ; \
	  cd linux-${SNAPSHOT} ; \
	  ln -s ../patches/${PATCHVER} patches ; \
	  quilt push -a )
endif
endif
ifeq (${ENDIAN},b)
	sed -e 's/.*CONFIG_CPU_BIG_ENDIAN.*/CONFIG_CPU_BIG_ENDIAN=y/' \
		< patches/${PATCHVER}/$(DEFCONFIG) > linux-${SNAPSHOT}/.config
else
	sed -e 's/.*CONFIG_CPU_BIG_ENDIAN.*/\# CONFIG_CPU_BIG_ENDIAN is not set/' \
		< patches/${PATCHVER}/$(DEFCONFIG) > linux-${SNAPSHOT}/.config
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

clobber: clobber-apex
	rm -rf vmlinuz-* modules-*.tar.gz usr-*.tar.gz
	rm -rf linux-*
	rm -rf madwifi-ng
	rm -rf lib usr

clobber-apex:
	rm -rf apex-*

.PHONY: all kernel menuconfig modules clobber apex clobber-apex

# End of Makefile
