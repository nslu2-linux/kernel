# Makefile for building the IXP4xx kernel.
#
#  Based on the build script for the Loft from Giant Shoulder, Inc.
#    written by Tommy B.
#  Based on the great work done in OpenEmbedded for the Linksys NSLU2
#    written by the nslu2-linux development community.
#
#  This builds the kernel in the linux-${REVISION} directory
#  and builds the ixp400 and ixp425-eth kernel modules.
#  It then installs all the kernel modules in lib
#  and creates a tarball there containing all the modules 
#  under the lib/modules/${REVISION} directory.
#
#  N.B. No downloads are provided with this script
#  The Intel files need to be obtained from Intel in
#  the proper manner (including agreeing to the licensing).
#
#  N.B. You may need to change your crosstool path
#


ENDIAN = l
#ENDIAN = b
MAJORVER = 2.6
BASEVER  = 2.6.16
MINORVER = 2.6.16
PATCHVER = 2.6.16
REVISION := $(shell sed -e 's/-git.*//' patches/${PATCHVER}/KERNEL)
SNAPSHOT := $(shell cat patches/${PATCHVER}/KERNEL)

MADWIFIVER = r1503-20060415

ifeq (${ENDIAN},b)
DEBIAN_ARCH = armeb
else
DEBIAN_ARCH = arm
endif

KERNEL_SOURCE   = http://kernel.org/pub/linux/kernel/v${MAJORVER}/linux-${BASEVER}.tar.bz2
KERNEL_PATCH    = http://kernel.org/pub/linux/kernel/v${MAJORVER}/testing/patch-${REVISION}.bz2
KERNEL_SNAPSHOT = http://kernel.org/pub/linux/kernel/v${MAJORVER}/snapshots/patch-${SNAPSHOT}.bz2

MADWIFI_SOURCE   = http://snapshots.madwifi.org/madwifi-ng/madwifi-ng-${MADWIFIVER}.tar.gz

#CROSS_COMPILE = ${DEBIAN_ARCH}-linux-gnu-
CROSS_COMPILE ?= ${DEBIAN_ARCH}-linux-

ifdef CROSS_COMPILE
CROSS_COMPILE_FLAGS = CROSS_COMPILE=${CROSS_COMPILE}
else
CROSS_COMPILE_FLAGS = 
endif

all: kernel modules usr

kernel: vmlinuz-nslu2-${REVISION} vmlinuz-nas100d-${REVISION} vmlinuz-loft-${REVISION} vmlinuz-ds101-${REVISION} vmlinuz-dsmg600-${REVISION}
ixp400: lib/modules/${REVISION}/kernel/drivers/ixp400/ixp400.ko
ixp400-eth: lib/modules/${REVISION}/kernel/drivers/net/ixp400_eth.ko
madwifi: lib/modules/${REVISION}/net/ath_hal.ko
modules: modules-${REVISION}.tar.gz
usr: usr-${REVISION}.tar.gz

usr-${REVISION}.tar.gz: vmlinuz-${REVISION} \
	 usr/local/bin/wlanconfig
	tar zcf usr-${REVISION}.tar.gz usr/local

modules-${REVISION}.tar.gz: vmlinuz-${REVISION} \
	 lib/modules/${REVISION}/kernel/drivers/ixp400/ixp400.ko \
	 lib/modules/${REVISION}/kernel/drivers/net/ixp400_eth.ko \
	 lib/modules/${REVISION}/net/ath_hal.ko
	tar zcf modules-${REVISION}.tar.gz lib/modules/${REVISION}

lib/modules/${REVISION}/net/ath_hal.ko usr/local/bin/wlanconfig: \
		madwifi-ng/Makefile \
		vmlinuz-${REVISION}
	rm -rf lib/modules/${REVISION}/net
	( cd madwifi-ng ; \
	  make CROSS_COMPILE=${CROSS_COMPILE} TOOLPATH=${CROSS_COMPILE} \
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

lib/modules/${REVISION}/kernel/drivers/net/ixp400_eth.ko: \
		ixp400_eth/Kbuild ixp400_xscale_sw/Makefile \
		vmlinuz-${REVISION}
	( cd ixp400_eth ; \
	  make -C `cd .. ; pwd`/linux-${REVISION} ARCH=arm \
		CROSS_COMPILE=${CROSS_COMPILE} M=`pwd` )
	mkdir -p lib/modules/${REVISION}/kernel/drivers/net
	cp ixp400_eth/ixp400_eth.ko lib/modules/${REVISION}/kernel/drivers/net
	chmod 644 lib/modules/${REVISION}/kernel/drivers/net/ixp400_eth.ko

ixp400_eth/Kbuild:
	( mkdir ixp400_eth ; cd ixp400_eth ; \
	  unzip ../downloads/GPL_ixp400LinuxEthernetDriverPatch-1_5.zip )
	( cd patches/ixp400-eth ; \
	  grep -v \# series | \
	  while read f; do case "$$f" in http:*) wget -O - "$$f";; *) cat "$$f";; esac; done \
	) | \
	patch -d ixp400_eth -p1

lib/modules/${REVISION}/kernel/drivers/ixp400/ixp400.ko: \
		ixp400_xscale_sw/Makefile vmlinuz-${REVISION}
	( . ixp400_xscale_sw/buildUtils/environment.linux.sh ; \
	  export KERNEL_DIR=`pwd`/linux-${REVISION} ; \
	  export IX_XSCALE_SW=`pwd`/ixp400_xscale_sw ; \
	  export IX_TARGET=linux${ENDIAN}e ; \
	  make -C ixp400_xscale_sw ARCH=arm \
	    CROSS_COMPILE=${CROSS_COMPILE} ixp400.ko )
	mkdir -p lib/modules/${REVISION}/kernel/drivers/ixp400
	cp ixp400_xscale_sw/lib/linux${ENDIAN}e/ixp400.ko \
		lib/modules/${REVISION}/kernel/drivers/ixp400
	chmod 644 lib/modules/${REVISION}/kernel/drivers/ixp400/ixp400.ko

ixp400_xscale_sw/Makefile:
	unzip downloads/IPL_ixp400AccessLibrary-2_1.zip
	unzip downloads/IPL_ixp400NpeLibrary-2_1.zip
	( cd patches/ixp400 ; \
	  grep -v \# series | \
	  while read f; do case "$$f" in http:*) wget -O - "$$f";; *) cat "$$f";; esac; done \
	) | \
	patch -d ixp400_xscale_sw -p1
	( cd patches/ixp-osal ; \
	  grep -v \# series | \
	  while read f; do case "$$f" in http:*) wget -O - "$$f";; *) cat "$$f";; esac; done \
	) | \
	patch -d ixp_osal -p1

vmlinuz-loft-${REVISION}: vmlinuz-${REVISION}
ifeq (${ENDIAN},b)
	cp $< $@
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

vmlinuz-ds101-${REVISION}: vmlinuz-${REVISION}
ifeq (${ENDIAN},b)
	devio '<<'$< >$@ \
		'wb 0xe3a01c03,4' 'wb 0xe3811041,4' \
		'cp$$'
else
		devio '<<'$< >$@ \
		'wb 0xe3a01c03,4' 'wb 0xe3811041,4' \
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


vmlinuz-${REVISION}: linux-${REVISION}/.config
	( cd linux-${REVISION} ; \
	  make ${CROSS_COMPILE_FLAGS} ARCH=arm bzImage modules )
	( cd linux-${REVISION} ; \
	  INSTALL_MOD_PATH=".." make ${CROSS_COMPILE_FLAGS} ARCH=arm modules_install )
	rm -f lib/modules/${REVISION}/build lib/modules/${REVISION}/source
	cp linux-${REVISION}/arch/arm/boot/zImage vmlinuz-${REVISION}

ifeq (${REVISION},${BASEVER})
linux-${REVISION}/.config: \
		downloads/linux-${BASEVER}.tar.bz2 \
		patches/${PATCHVER}/series patches/${PATCHVER}/??-*.patch patches/${PATCHVER}/defconfig
	[ -e linux-${REVISION} ] || \
	( tar xjf downloads/linux-${BASEVER}.tar.bz2 ; \
	  cd linux-${REVISION} ; \
	  ln -s ../patches/${PATCHVER} patches ; \
	  quilt push -a )
else
ifeq (${REVISION},${MINORVER})
linux-${REVISION}/.config: \
		downloads/linux-${MINORVER}.tar.bz2 \
		patches/${PATCHVER}/series patches/${PATCHVER}/??-*.patch patches/${PATCHVER}/defconfig
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
		patches/${PATCHVER}/series patches/${PATCHVER}/??-*.patch patches/${PATCHVER}/defconfig
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
		patches/${PATCHVER}/series patches/${PATCHVER}/??-*.patch patches/${PATCHVER}/defconfig
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
		< patches/${PATCHVER}/defconfig > linux-${REVISION}/.config
else
	sed -e 's/.*CONFIG_CPU_BIG_ENDIAN.*/\# CONFIG_CPU_BIG_ENDIAN is not set/' \
		< patches/${PATCHVER}/defconfig > linux-${REVISION}/.config
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
	rm -rf ixp400_xscale_sw ixp_osal ixp400_eth
	rm -rf madwifi-ng
	rm -rf lib usr

.PHONY: all kernel modules clobber

# End of Makefile
