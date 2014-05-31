%define _usrdir /usr
%define _ugdir  %{_usrdir}/ug
%define SHARE /usr/share

Name:       ug-setting-mobileap-efl
Summary:    Tethering UI Gadget Library
Version:    0.1.170
Release:    1
Group:      TO_BE/FILLED_IN
License:    Flora License Version 1.0
Source0:    %{name}-%{version}.tar.gz
Requires(post): /usr/bin/vconftool
BuildRequires: cmake
BuildRequires: edje-bin
BuildRequires: gettext-tools
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(efl-assist)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(ui-gadget-1)
BuildRequires: pkgconfig(wifi-direct)
BuildRequires: pkgconfig(capi-network-tethering)
BuildRequires: pkgconfig(capi-network-connection)
BuildRequires: pkgconfig(capi-network-wifi)
BuildRequires: pkgconfig(capi-telephony-sim)
BuildRequires: pkgconfig(notification)

%description
Tethering UI Gadget Library

%prep
%setup -q

%build
cmake . -DCMAKE_INSTALL_PREFIX="%{_ugdir}"
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

#License
mkdir -p %{buildroot}%{SHARE}/license
cp LICENSE.Flora %{buildroot}%{SHARE}/license/ug-setting-mobileap-efl

%post
/usr/bin/vconftool set -t bool db/private/libug-setting-mobileap-efl/prev_wifi_status 0 -u 5000 -f
mkdir -p /usr/ug/bin/
ln -sf /usr/bin/ug-client /usr/ug/bin/setting-mobileap-efl

%files
%manifest ug-setting-mobileap-efl.manifest
%defattr(-,root,root,-)
%{_ugdir}/res/edje/ug-setting-mobileap-efl/*.edj
%{_ugdir}/res/locale/*/LC_MESSAGES/*
%{_ugdir}/res/images/ug-setting-mobileap-efl/*.png
%{SHARE}/license/ug-setting-mobileap-efl
%{_ugdir}/lib/libug-setting-mobileap-efl.so
/usr/share/packages/ug-setting-mobileap-efl.xml
