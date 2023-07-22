#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/ilavaleev/Dev/SFND_2D_Feature_Tracking/build
  make -f /Users/ilavaleev/Dev/SFND_2D_Feature_Tracking/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/ilavaleev/Dev/SFND_2D_Feature_Tracking/build
  make -f /Users/ilavaleev/Dev/SFND_2D_Feature_Tracking/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/ilavaleev/Dev/SFND_2D_Feature_Tracking/build
  make -f /Users/ilavaleev/Dev/SFND_2D_Feature_Tracking/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/ilavaleev/Dev/SFND_2D_Feature_Tracking/build
  make -f /Users/ilavaleev/Dev/SFND_2D_Feature_Tracking/build/CMakeScripts/ReRunCMake.make
fi

