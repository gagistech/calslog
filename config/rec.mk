include $(config_dir)base/base.mk

this_cxxflags += -DDEBUG
this_cxxflags += -O0

this_cxxflags += -D BEDSIDEMON_RECORD_SPO2_MEASUREMENTS
