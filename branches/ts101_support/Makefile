# Makefile for building the TS101 kernel.
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

MAJORVER = 2.6
BASEVER  = 2.6.20
PATCHVER = 2.6.20
REVISION := $(shell sed -e 's/-git.*//' patches/${PATCHVER}/KERNEL)
SNAPSHOT := $(shell cat patches/${PATCHVER}/KERNEL)

ARCH=ppc
DEFCONFIG=defconfig

KERNEL_SOURCE   = http://www2.kernel.org/pub/linux/kernel/v${MAJORVER}/linux-${BASEVER}.tar.bz2
KERNEL_PATCH    = http://www2.kernel.org/pub/linux/kernel/v${MAJORVER}/testing/patch-${REVISION}.bz2
KERNEL_SNAPSHOT = http://www2.kernel.org/pub/linux/kernel/v${MAJORVER}/snapshots/patch-${SNAPSHOT}.bz2

CROSS_COMPILE = powerpc-linux-uclibc-

ifdef CROSS_COMPILE
CROSS_COMPILE_FLAGS = CROSS_COMPILE=${CROSS_COMPILE}
else
CROSS_COMPILE_FLAGS = 
endif

all: kernel modules

kernel: uImage-${SNAPSHOT}-${ARCH}
modules: modules-${SNAPSHOT}-${ARCH}.tar.gz
patched: linux-${SNAPSHOT}-${ARCH}/.config 

modules-${SNAPSHOT}-${ARCH}.tar.gz: uImage-${SNAPSHOT}-${ARCH}
	tar -C modules-${SNAPSHOT}-${ARCH} -zcf modules-${SNAPSHOT}-${ARCH}.tar.gz lib/modules/${SNAPSHOT}

uImage-${SNAPSHOT}-${ARCH}: linux-${SNAPSHOT}-${ARCH}/.config
	( cd linux-${SNAPSHOT}-${ARCH} ; \
	  ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=${ARCH} uImage )
	( cd linux-${SNAPSHOT}-${ARCH} ; \
	  INSTALL_MOD_PATH="../modules-${SNAPSHOT}-${ARCH}" ${MAKE} ${CROSS_COMPILE_FLAGS} ARCH=${ARCH} modules modules_install ) || true
	rm -f modules-${SNAPSHOT}-${ARCH}/lib/modules/${SNAPSHOT}/build modules-${SNAPSHOT}-${ARCH}/lib/modules/${SNAPSHOT}/source
	cp linux-${SNAPSHOT}-${ARCH}/arch/${ARCH}/boot/uImage uImage-${SNAPSHOT}-${ARCH}

menuconfig: linux-${SNAPSHOT}-${ARCH}/.config
	${MAKE} -C linux-${SNAPSHOT}-${ARCH} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} menuconfig

ifeq (${SNAPSHOT},${BASEVER})
linux-${SNAPSHOT}-${ARCH}/.config: \
		downloads/linux-${BASEVER}.tar.bz2 \
		patches/${PATCHVER}/series patches/${PATCHVER}/???-*.patch \
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
		patches/${PATCHVER}/series patches/${PATCHVER}/???-*.patch \
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
		patches/${PATCHVER}/series patches/${PATCHVER}/???-*.patch \
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
	cp patches/${PATCHVER}/$(DEFCONFIG) linux-${SNAPSHOT}-${ARCH}/.config	

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
	rm -rf uImage-* modules-*.tar.gz
	rm -rf linux-* modules-*
	rm -rf *~

.PHONY: all kernel menuconfig modules clobber

# End of Makefile
