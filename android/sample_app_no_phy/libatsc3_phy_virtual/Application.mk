#
# Application.mk
#
# note that APP_ABI is overridden by build.gradle setting.
#
APP_DEBUG := true
APP_STRIP_MODE := "none"

#APP_STL := gnustl_shared
APP_STL := c++_shared

APP_ABI := all
# APP_ABI := arm64-v8a
# -D_ANDROID
APP_CPPFLAGS += -std=c++11  -fexceptions

#jjustman-2020-08-11 - workaround for srt:
# In file included from /Users/jjustman/Desktop/libatsc3/src/phy/virtual/srt/transmitmedia.cpp:31:
# /Users/jjustman/Desktop/libatsc3/src/phy/virtual/srt/socketoptions.hpp:98:5: error: cannot use 'try' with exceptions disabled

# APP_CPPFLAGS :=
