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
BASEVER  = 2.6.18
MINORVER = 2.6.19
PATCHVER = 2.6.19
REVISION := $(shell sed -e 's/-git.*//' patches/${PATCHVER}/KERNEL)
SNAPSHOT := $(shell cat patches/${PATCHVER}/KERNEL)

APEX_REVISION = 1.4.7
APEX_TARGET = nslu2

MADWIFIVER = r1503-20060415

DEFCONFIG=defconfig

ifeq (${ENDIAN},b)
DEBIAN_ARCH = armeb
else
DEBIAN_ARCH = arm
endif

KERNEL_SOURCE   = http://kernel.org/pub/linux/kernel/v${MAJORVER}/linux-${BASEVER}.tar.bz2
KERNEL_PATCH    = http://kernel.org/pub/linux/kernel/v${MAJORVER}/testing/patch-${REVISION}.bz2
KERNEL_SNAPSHOT = http://kernel.org/pub/linux/kernel/v${MAJORVER}/snapshots/patch-${SNAPSHOT}.bz2

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

kernel: vmlinuz-nslu2-${REVISION} vmlinuz-nas100d-${REVISION} vmlinuz-loft-${REVISION} vmlinuz-dsmg600-${REVISION} vmlinuz-fsg3-${REVISION}
madwifi: lib/modules/${REVISION}/net/ath_hal.ko
modules: modules-${REVISION}.tar.gz
patched: linux-${REVISION}/.config 
apex: apex-${APEX_TARGET}-${APEX_REVISION}.bin

apex-${APEX_TARGET}-${APEX_REVISION}.bin: apex-${APEX_REVISION}/Makefile patches/apex/defconfig
ifeq (${ENDIAN},b)
	sed -e 's/.*CONFIG_CPU_BIG_ENDIAN.*/CONFIG_CPU_BIG_ENDIAN=y/' \
		-e '/CONFIG_ARCH_NUMBER/d' \
		< patches/apex/defconfig > apex-${APEX_REVISION}/.config
else
	sed -e 's/.*CONFIG_CPU_BIG_ENDIAN.*/\# CONFIG_CPU_BIG_ENDIAN is not set/' \
		-e '/CONFIG_ARCH_NUMBER/d' \
		< patches/apex/defconfig > apex-${APEX_REVISION}/.config
endif
ifeq (${APEX_TARGET},nslu2)
	sed -i -e '/CONFIG_MACH_NSLU2/d' apex-${APEX_REVISION}/.config
	echo 'CONFIG_MACH_NSLU2=y' >> apex-${APEX_REVISION}/.config
endif
ifeq (${APEX_TARGET},loft)
	sed -i -e '/CONFIG_MACH_AVILA2347/d' apex-${APEX_REVISION}/.config
	echo 'CONFIG_MACH_AVILA2347=y' >> apex-${APEX_REVISION}/.config
	echo 'CONFIG_ARCH_NUMBER=849' >> apex-${APEX_REVISION}/.config
endif
ifeq (${APEX_TARGET},nas100d)
	sed -i -e '/CONFIG_MACH_NAS100D/d' apex-${APEX_REVISION}/.config
	echo 'CONFIG_MACH_NAS100D=y' >> apex-${APEX_REVISION}/.config
endif
ifeq (${APEX_TARGET},dsmg600)
	sed -i -e '/CONFIG_MACH_DSMG600/d' apex-${APEX_REVISION}/.config
	echo 'CONFIG_MACH_DSMG600=y' >> apex-${APEX_REVISION}/.config
	echo 'CONFIG_ARCH_NUMBER=964' >> apex-${APEX_REVISION}/.config
endif
ifeq (${APEX_TARGET},fsg3)
	sed -i -e '/CONFIG_MACH_FSG/d' apex-${APEX_REVISION}/.config
	echo 'CONFIG_MACH_FSG=y' >> apex-${APEX_REVISION}/.config
	echo 'CONFIG_ARCH_NUMBER=1091' >> apex-${APEX_REVISION}/.config
endif
	( cd apex-${APEX_REVISION} ; \
	  ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=arm clean all )
	cp apex-${APEX_REVISION}/apex apex-${APEX_TARGET}-${APEX_REVISION}.bin

apex-${APEX_REVISION}/Makefile: \
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

usr: usr-${REVISION}.tar.gz

usr-${REVISION}.tar.gz: vmlinuz-${REVISION} \
	 usr/local/bin/wlanconfig
	tar zcf usr-${REVISION}.tar.gz usr/local

# Add the following depedency if you want madwifi built.
#	 lib/modules/${REVISION}/net/ath_hal.ko

modules-${REVISION}.tar.gz: vmlinuz-${REVISION}
	tar zcf modules-${REVISION}.tar.gz lib/modules/${REVISION}

lib/modules/${REVISION}/net/ath_hal.ko usr/local/bin/wlanconfig: \
		madwifi-ng/Makefile \
		vmlinuz-${REVISION}
	rm -rf lib/modules/${REVISION}/net
	( cd madwifi-ng ; \
	  ${MAKE} CROSS_COMPILE=${CROSS_COMPILE} TOOLPATH=${CROSS_COMPILE} \
		TARGET=xscale-${ENDIAN}e-elf \
		KERNELPATH=`cd .. ; pwd`/linux-${REVISION} \
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

vmlinuz-loft-${REVISION}: vmlinuz-${REVISION}
ifeq (${ENDIAN},b)
	devio '<<'$< >$@ \
		'wb 0xe3a01c03,4' 'wb 0xe3811051,4' \
		'cp$$'
else
	devio '<<'$< >$@ \
		'wb 0xe3a01c03,4' 'wb 0xe3811051,4' \
		'wb 0xee110f10,4' \
		'wb 0xe3c00080,4' \
		'wb 0xee010f10,4' \
		'xp $$,4'
endif

vmlinuz-nas100d-${REVISION}: vmlinuz-${REVISION}
ifeq (${ENDIAN},b)
	cp $< $@
else
	devio '<<'$< >$@ \
		'wb 0xee110f10,4' \
		'wb 0xe3c00080,4' \
		'wb 0xee010f10,4' \
		'xp $$,4'
endif

vmlinuz-nslu2-${REVISION}: vmlinuz-${REVISION}
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

vmlinuz-dsmg600-${REVISION}: vmlinuz-${REVISION}
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


vmlinuz-fsg3-${REVISION}: vmlinuz-${REVISION}
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


vmlinuz-${REVISION}: linux-${REVISION}/.config
	( cd linux-${REVISION} ; \
	  ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=arm bzImage modules )
	( cd linux-${REVISION} ; \
	  INSTALL_MOD_PATH=".." ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=arm modules_install )
	rm -f lib/modules/${REVISION}/build lib/modules/${REVISION}/source
	cp linux-${REVISION}/arch/arm/boot/zImage vmlinuz-${REVISION}

menuconfig: linux-${REVISION}/.config
	${MAKE} -C linux-${REVISION} ARCH=arm CROSS_COMPILE=${CROSS_COMPILE} menuconfig

ifeq (${REVISION},${BASEVER})
linux-${REVISION}/.config: \
		downloads/linux-${BASEVER}.tar.bz2 \
		patches/${PATCHVER}/series patches/${PATCHVER}/??-*.patch patches/${PATCHVER}/$(DEFCONFIG)
	[ -e linux-${REVISION} ] || \
	( tar xjf downloads/linux-${BASEVER}.tar.bz2 ; \
	  cd linux-${REVISION} ; \
	  ln -s ../patches/${PATCHVER} patches ; \
	  quilt push -a )
else
ifeq (${REVISION},${MINORVER})
linux-${REVISION}/.config: \
		downloads/linux-${MINORVER}.tar.bz2 \
		patches/${PATCHVER}/series patches/${PATCHVER}/??-*.patch patches/${PATCHVER}/$(DEFCONFIG)
	[ -e linux-${REVISION} ] || \
	( tar xjf downloads/linux-${MINORVER}.tar.bz2 ; \
	  cd linux-${REVISION} ; \
	  ln -s ../patches/${PATCHVER} patches ; \
	  quilt push -a )
else
ifeq (${REVISION},${SNAPSHOT})
linux-${REVISION}/.config: \
		downloads/linux-${BASEVER}.tar.bz2 \
		downloads/patch-${REVISION}.bz2 \
		patches/${PATCHVER}/series patches/${PATCHVER}/??-*.patch patches/${PATCHVER}/$(DEFCONFIG)
	[ -e linux-${REVISION} ] || \
	( tar xjf downloads/linux-${BASEVER}.tar.bz2 ; \
	  mv linux-${BASEVER} linux-${REVISION} ; \
	  bzcat downloads/patch-${REVISION}.bz2 | \
	  patch -d linux-${REVISION} -p1 ; \
	  cd linux-${REVISION} ; \
	  ln -s ../patches/${PATCHVER} patches ; \
	  quilt push -a )
else
linux-${REVISION}/.config: \
		downloads/linux-${BASEVER}.tar.bz2 \
		downloads/patch-${REVISION}.bz2 \
		downloads/patch-${SNAPSHOT}.bz2 \
		patches/${PATCHVER}/series patches/${PATCHVER}/??-*.patch patches/${PATCHVER}/$(DEFCONFIG)
	[ -e linux-${REVISION} ] || \
	( tar xjf downloads/linux-${BASEVER}.tar.bz2 ; \
	  mv linux-${BASEVER} linux-${REVISION} ; \
	  bzcat downloads/patch-${REVISION}.bz2 downloads/patch-${SNAPSHOT}.bz2 | \
	  patch -d linux-${REVISION} -p1 ; \
	  cd linux-${REVISION} ; \
	  ln -s ../patches/${PATCHVER} patches ; \
	  quilt push -a )
endif
endif
endif
ifeq (${ENDIAN},b)
	sed -e 's/.*CONFIG_CPU_BIG_ENDIAN.*/CONFIG_CPU_BIG_ENDIAN=y/' \
		-e '/CONFIG_JFFS2_NATIVE_ENDIAN/d' \
		-e '/CONFIG_JFFS2_BIG_ENDIAN/d' \
		-e '/CONFIG_JFFS2_LITTLE_ENDIAN/d' \
		< patches/${PATCHVER}/$(DEFCONFIG) > linux-${REVISION}/.config
	echo '# CONFIG_JFFS2_NATIVE_ENDIAN is not set' >> linux-${REVISION}/.config
	echo 'CONFIG_JFFS2_BIG_ENDIAN=y' >> linux-${REVISION}/.config
	echo '# CONFIG_JFFS2_LITTLE_ENDIAN is not set' >> linux-${REVISION}/.config
else
	sed -e 's/.*CONFIG_CPU_BIG_ENDIAN.*/\# CONFIG_CPU_BIG_ENDIAN is not set/' \
		-e '/CONFIG_JFFS2_NATIVE_ENDIAN/d' \
		-e '/CONFIG_JFFS2_BIG_ENDIAN/d' \
		-e '/CONFIG_JFFS2_LITTLE_ENDIAN/d' \
		< patches/${PATCHVER}/$(DEFCONFIG) > linux-${REVISION}/.config
	echo '# CONFIG_JFFS2_NATIVE_ENDIAN is not set' >> linux-${REVISION}/.config
	echo '# CONFIG_JFFS2_BIG_ENDIAN is not set' >> linux-${REVISION}/.config
	echo 'CONFIG_JFFS2_LITTLE_ENDIAN=y' >> linux-${REVISION}/.config
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

clobber:
	rm -rf vmlinuz-* modules-*.tar.gz usr-*.tar.gz
	rm -rf linux-*
	rm -rf madwifi-ng
	rm -rf apex-*
	rm -rf lib usr

.PHONY: all kernel menuconfig modules clobber apex

# End of Makefile
