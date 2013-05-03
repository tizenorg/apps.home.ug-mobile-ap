%define _usrdir /usr
%define _ugdir  %{_usrdir}/ug

Name:       ug-setting-mobileap-efl
Summary:    Tethering UI Gadget Library
Version:    0.1.165
Release:    1
Group:      TO_BE/FILLED_IN
License:    Flora License Version 1.0
Source0:    %{name}-%{version}.tar.gz
Requires(post): /usr/bin/vconftool
BuildRequires: cmake
BuildRequires: edje-bin
BuildRequires: gettext-tools
BuildRequires: pkgconfig(elementary)
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
%{_ugdir}/lib/libug-setting-mobileap-efl.so
/usr/share/packages/ug-setting-mobileap-efl.xml

%changelog
* Tue Apr 09 2013 Seungyoun Ju <sy39.ju@samsung.com> 0.1.162-1
- Maximum connection information is displayed
- Implement connection timer
- UX Change : Data usage and Connected device
- Change ug name & Support appsvc
- Translation data is updated

* Fri Mar 29 2013 Seungyoun Ju <sy39.ju@samsung.com> 0.1.161-2
- Fix : The grouping of genlist item is wrong
- Back button position and Naviframe title visibility are changed
- Make grouping for genlist expanded item
- Editfield change is applied
- Ticker notification is implemented for passphrase entry
- Implement title slide
- Pop-up parent and entry's context menu allowance are changed

* Mon Jan 28 2013 Seungyoun Ju <sy39.ju@samsung.com> 0.1.160-1
- Remove unrequired log

* Tue Jan 15 2013 Seungyoun Ju <sy39.ju@samsung.com> 0.1.159-1
- Fix : When device is rotated, guide text is displayed short time
- Fix : Security / Hide items are disabled sometimes
- Translation data is updated
- Pop-up scenario for USB tethering is implemented
- IMF is hided by focus out
- Wi-Fi tethering is enabled without checking enable state

* Fri Dec 07 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.158-1
- New winset is applied and some bugs are fixed

* Mon Nov 12 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.157-1
- Cellular network state is separated to CONNECTION_CELLULAR_STATE_AVAILABLE and CONNECTION_CELLULAR_STATE_CONNECTED

* Wed Oct 31 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.156-2
- Initial package upload

* Mon Oct 29 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.156-1
- Install path is changed from /opt to /usr

* Thu Oct 22 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.155-1
- License is added

* Thu Oct 04 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.154-1
- Connection / Wi-Fi's changed CAPIs are applied

* Fri Sep 21 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.153-1
- Pop-up button's style is changed

* Fri Sep 21 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.152-1
- Add manifest file for SMACK

* Wed Aug 29 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.151-1
- When client's connection is changed, old handles are freed

* Wed Aug 17 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.150-1
- Managed APIs are used to get Network connection state and control Wi-Fi
- Crash issue is fixed because of "Unable to use tethering" pop-up
- End separator is added to each genlist
- Wi-Fi direct is considered when Wi-Fi tethering is enabled

* Wed Aug 01 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.149-1
- Managed APIs are used to get Wi-Fi tethering settings
- Password entry issue("Enter" can be inserted) is fixed

* Thu Jul 26 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.148-2
- Unused resource is removed

* Thu Jul 26 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.148-1
- Apply White theme

* Wed Jul 18 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.147-2
- Upload pkg for ug library change

* Thu Jul 12 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.147-1
- ui-gadget-1 is applied
- typedef struct rule is fixed
- unnecessary dependency and header include are removed

* Thu Jul 12 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.146-1
- library is stripped

* Thu Jul 12 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.145-1
- "Wi-Fi tethering is not enabled" issue is fixed

* Fri Jun 29 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.144-1
- libmobile-ap is replaced with capi-network-tethering

* Mon Jun 25 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.143-1
- Data usage is got from agent

* Thu Jun 04 2012 Injun Yang <injun.yang@samsung.com> 0.1.142-1
- Initialize vconf key
- Remove unused vconf key

* Thu May 31 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.141-1
- Enabled indicatons are handled
- Wi-Fi vconf is used to know when Wi-Fi is disabled
- Code clean-up

* Thu May 24 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.140-1
- hide option vconfkey is exported to internal vconfkeys

* Fri May 18 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.139-1
- "Tethering" translation data is applied

* Thu May 17 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.138-1
- Private vconfkey is changed following new naming rule

* Mon Mar 26 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.137-1
- Tethering and portable hotspot is changed to Tethering

* Tue Mar 14 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.135-1
- libmobile-ap API's and EFL are changed

* Wed Feb 06 2012 Seungyoun Ju <sy39.ju@samsung.com> 0.1.130-1
- Deprecated dependencies are removed
