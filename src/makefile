include prorab.mk
include prorab-test.mk
include prorab-clang-format.mk
include prorab-license.mk

# =====================================
# ============== library ==============

this_name := calslog

$(eval $(call prorab-config, ../config))

this__root_dir_rel := ../

this__version_cpp := $(this_out_dir)version.cpp

this_srcs += $(this__version_cpp) $(filter-out %main.cpp,$(call prorab-src-dir, $(this_name)))

this_no_install := true
this_static_lib_only := true

$(eval $(prorab-build-lib))

define this__rules
$(abspath $(d)$(this__version_cpp)): $(abspath $(d)$(this__root_dir_rel)build/debian/changelog)
$(.RECIPEPREFIX)$(a)mkdir -p $$(dir $$@)
$(.RECIPEPREFIX)$(a)(cd $(d)$(this__root_dir_rel); echo "extern const char* const program_version = \"$$$$(myci-deb-version.sh)\";" > $$@)
endef
$(eval $(this__rules))

# ================================================
# ============== format and license ==============
this_src_dir := $(this_name)
$(eval $(prorab-clang-format))
this_license_file := ../LICENSE
$(eval $(prorab-license))

# ===============================================
# ============== install resources ==============
define this__rules
install::
$(.RECIPEPREFIX)$(a)for i in $(patsubst $(d)../res/%,/%,$(shell find $(d)../res -type f -name "*")); do \
		install -d $(DESTDIR)$(PREFIX)/share/$(this_name)$$$${i%/*}; \
		install -m 644 $(d)../res$$$$i $(DESTDIR)$(PREFIX)/share/$(this_name)$$$$i; \
	done
endef
$(eval $(this__rules))

# =========================================
# ============== application ==============
define calslog_rules
    $$(eval $$(prorab-clear-this-vars))

    this_name := calslog-$1-$2

    $$(eval $$(call prorab-config, ../config))

    this_srcs += calslog/main.cpp

    this_ldlibs += -pthread
    this_ldflags += -rdynamic

    this_ldlibs += $$(this_out_dir)libcalslog.a

    this_ldlibs += -l fsif
    this_ldlibs += -l clargs
    this_ldlibs += -l nitki
    this_ldlibs += -l opros
    this_ldlibs += -l tml
    this_ldlibs += -l ruis
    this_ldlibs += -l utki
    this_ldlibs += -l m

    this_ldlibs += -l ruisapp-$1-$2

    $$(eval $$(prorab-build-app))

    this_run_name := app-$1-$2
    this_test_cmd := $$(prorab_this_name) --window --res-path=../res
    this_test_deps := $$(prorab_this_name)
    this_test_ld_path := $$(prorab_space)
    $$(eval $$(prorab-run))

endef

# $(eval $(call calslog_rules,opengl,xorg))
$(eval $(call calslog_rules,opengles,xorg))
# $(eval $(call calslog_rules,opengl,wayland))
$(eval $(call calslog_rules,opengles,wayland))

# clean up
calslog_rules :=
