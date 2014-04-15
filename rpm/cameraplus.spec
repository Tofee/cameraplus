Name:           cameraplus
Summary:        Camera+
Version:        0
Release:        1
Group:          Applications/Multimedia
License:        LGPL v2.1+
URL:            http://gitorious.org/cameraplus
Source0:        %{name}-%{version}.tar.gz
Source1:        cameraplus.desktop
Source2:        cameraplus-logo.png
BuildRequires:  pkgconfig(gstreamer-1.0)
BuildRequires:  pkgconfig(gstreamer-plugins-base-1.0)
BuildRequires:  pkgconfig(gstreamer-video-1.0)
BuildRequires:  pkgconfig(gstreamer-plugins-bad-1.0)
BuildRequires:  pkgconfig(gstreamer-pbutils-1.0)
BuildRequires:  pkgconfig(nemo-gstreamer-interfaces-1.0)
BuildRequires:  pkgconfig(nemo-gstreamer-meta-1.0)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(qdeclarative5-boostable)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Location)
BuildRequires:  pkgconfig(Qt5OpenGL)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(libcanberra)
BuildRequires:  pkgconfig(Qt5Sparql)
BuildRequires:  pkgconfig(qmsystem2-qt5)
BuildRequires:  pkgconfig(libresourceqt5)
BuildRequires:  pkgconfig(Qt5SystemInfo)
BuildRequires:  pkgconfig(Qt5OpenGLExtensions)
BuildRequires:  desktop-file-utils
Requires:       qt5-qtdeclarative-import-location
Requires:       qt5-qtlocation-plugin-geoservices-nokia
Requires:       qt5-qtlocation-plugin-geoservices-osm

%description
Camera+

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%prep
%setup -q

%build
%qmake5

make %{?jobs:-j%jobs}

%install
%qmake5_install

mkdir -p $RPM_BUILD_ROOT/usr/share/themes/jolla-ambient/meegotouch/icons/
cp icons/*.png $RPM_BUILD_ROOT/usr/share/themes/jolla-ambient/meegotouch/icons/
cp %SOURCE2 $RPM_BUILD_ROOT/usr/share/themes/jolla-ambient/meegotouch/icons/

mkdir -p $RPM_BUILD_ROOT/usr/share/qtcamera/config/
cp data/sailfish/qtcamera.ini $RPM_BUILD_ROOT/usr/share/qtcamera/config/
cp data/sailfish/properties.ini $RPM_BUILD_ROOT/usr/share/qtcamera/config/
cp data/sailfish/image.gep $RPM_BUILD_ROOT/usr/share/qtcamera/config/
cp data/sailfish/video.gep $RPM_BUILD_ROOT/usr/share/qtcamera/config/

mkdir -p $RPM_BUILD_ROOT/usr/share/cameraplus/config/
cp data/sailfish/cameraplus.ini $RPM_BUILD_ROOT/usr/share/cameraplus/config/

mkdir -p $RPM_BUILD_ROOT/usr/share/applications/
cp %SOURCE1 $RPM_BUILD_ROOT/usr/share/applications/

desktop-file-install --delete-original                   \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

mv $RPM_BUILD_ROOT/usr/lib/libqtcamera.so.1.0.0 $RPM_BUILD_ROOT/usr/lib/libqtcamera.so.1
rm $RPM_BUILD_ROOT/usr/lib/libqtcamera.so.1.0
rm $RPM_BUILD_ROOT/usr/lib/libqtcamera.so

mkdir -p $RPM_BUILD_ROOT/usr/lib/qt5/qml
mv $RPM_BUILD_ROOT/usr/lib/qt5/imports/QtCamera $RPM_BUILD_ROOT/usr/lib/qt5/qml/
rm -rf $RPM_BUILD_ROOT/usr/lib/qt5/imports

%files
%defattr(-,root,root,-)
%{_bindir}/cameraplus
%{_libdir}/libqtcamera.so.1
%{_datadir}/cameraplus/*
%{_datadir}/qtcamera/*
%{_datadir}/themes/jolla-ambient/meegotouch/icons/*
%{_libdir}/qt5/qml/QtCamera/*
%{_datadir}/applications/cameraplus.desktop
%{_datadir}/themes/jolla-ambient/meegotouch/icons/
