%define _usrdir /usr
%define _ugdir  %{_usrdir}/apps/ug-setting-mobileap-efl

Name:		ug-setting-mobileap-efl
Summary:	Tethering UI Gadget Library
Version:	1.0.95
Release:	1
Group:		App/Network
License:	Flora-1.0
Source0:	%{name}-%{version}.tar.gz
BuildRequires:	pkgconfig(evas)
BuildRequires:	pkgconfig(utilX)
BuildRequires:	pkgconfig(elementary)
BuildRequires:	pkgconfig(efl-assist)
BuildRequires:	pkgconfig(ui-gadget-1)
BuildRequires:	pkgconfig(capi-network-wifi)
BuildRequires:	pkgconfig(capi-network-tethering)
BuildRequires:	pkgconfig(capi-network-connection)
BuildRequires:	pkgconfig(notification)
BuildRequires:	cmake
BuildRequires:	edje-bin
BuildRequires:	gettext-tools
BuildRequires:	model-build-features
Requires(post):	/usr/bin/vconftool

%description
Tethering UI Gadget Library

%prep
%setup -q

%build
cmake -DCMAKE_INSTALL_PREFIX="%{_ugdir}"  \
%if 0%{?model_build_feature_network_dsds} == 1
	-DTIZEN_DUALSIM_ENABLE=1 \
%endif
	.

make %{?_smp_mflags}


%install
rm -rf %{buildroot}
%make_install

#License
mkdir -p %{buildroot}%{_datadir}/license
cp LICENSE %{buildroot}%{_datadir}/license/%{name}

%post
/usr/bin/vconftool set -t bool db/private/libug-setting-mobileap-efl/prev_wifi_status 0 -u 5000 -f -s ug-setting-mobileap-efl
/usr/bin/vconftool set -t int file/private/libug-setting-mobileap-efl/wifi_popup_check_box_status 0 -u 5000 -i -f -s ug-setting-mobileap-efl
/usr/bin/vconftool set -t int file/private/libug-setting-mobileap-efl/bt_popup_check_box_status 0 -u 5000 -i -f -s ug-setting-mobileap-efl
/usr/bin/vconftool set -t int file/private/libug-setting-mobileap-efl/usb_popup_check_box_status 0 -u 5000 -i -f -s ug-setting-mobileap-efl
/usr/bin/vconftool set -t int file/private/libug-setting-mobileap-efl/is_device_rename_local 0 -u 5000 -i -f -s ug-setting-mobileap-efl
/usr/bin/vconftool set -t int memory/private/libug-setting-mobileap-efl/trying_usb_tethering 0 -u 5000 -i -f -s ug-setting-mobileap-efl

mkdir -p /usr/apps/ug-setting-mobileap-efl/bin/ -m 777
chown -R 5000:5000 /usr/apps/ug-setting-mobileap-efl/bin/
chsmack -a "_" /usr/apps/ug-setting-mobileap-efl/bin/

%files
%manifest ug-setting-mobileap-efl.manifest
/etc/smack/accesses.d/ug-setting-mobileap-efl.efl
%defattr(-,root,root,-)
/usr/ug/res/locale/*/LC_MESSAGES/ug-setting-mobileap-efl*
/usr/ug/res/images/ug-setting-mobileap-efl/*.png
%{_ugdir}/res/edje/ug-setting-mobileap-efl/*.edj
%{_ugdir}/res/help/ug-setting-mobileap-efl/help_setting_tethering.xml
%{_ugdir}/lib/ug/libug-setting-mobileap-efl.so
/usr/share/packages/ug-setting-mobileap-efl.xml
/usr/apps/ug-setting-mobileap-efl/shared/res/tables/ug-setting-mobileap-efl_ChangeableColorInfo.xml
/usr/apps/ug-setting-mobileap-efl/shared/res/tables/ug-setting-mobileap-efl_fontInfoTable.xml
%{_datadir}/license/%{name}
