MESON_FLAGS :=  --cross-file arctan_crossfile.txt --prefix=$(ARCTAN_HOME)/files \
				--sysconfdir=/etc --localstatedir=/var --libdir=lib --sbindir=bin --buildtype=release -Ddefault_library=shared \
				--buildtype=debugoptimized -Dmlibc_no_headers=true -Ddefault_library=both -Ddisable_crypt_option=true -Ddisable_iconv_option=true \
				-Ddisable_intl_option=true

MESON_BUILD_DIR := build

.PHONY: all
all: clean
	git clone https://github.com/managarm/mlibc
	cp arctan_crossfile.txt mlibc/
	cp -r sysdeps/* mlibc/sysdeps
	cd mlibc/ && git apply ../meson.build.patch
	cd mlibc/ && meson setup $(MESON_BUILD_DIR) $(MESON_FLAGS)
	meson install -C $(shell pwd)/mlibc/$(MESON_BUILD_DIR)
	rm -rf mlibc/

.PHONY: clean
clean:
	rm -rf mlibc/
