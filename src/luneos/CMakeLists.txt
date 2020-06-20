set(EXTRA_SOURCE_FILES
  luneos/compass.cpp
  luneos/devicekeys.cpp
  luneos/fsmonitor.cpp
  luneos/orientation.cpp
  luneos/deviceinfo.cpp
  luneos/displaystate.cpp
  luneos/geocode.cpp
  luneos/soundvolumecontrol.cpp
  )

set(EXTRA_HEADER_FILES
  luneos/compass.h
  luneos/devicekeys.h
  luneos/fsmonitor.h
  luneos/orientation.h
  luneos/deviceinfo.h
  luneos/displaystate.h
  luneos/geocode.h
  luneos/soundvolumecontrol.h
  )

pkg_check_modules(EXTRA REQUIRED
  Qt5SystemInfo
  Qt5Sparql
  libresourceqt5
  Qt5Sensors
  )

set(EXTRA_RESOURCES "../qml/luneos/luneos.qrc")
