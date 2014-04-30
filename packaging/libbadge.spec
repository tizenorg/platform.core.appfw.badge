Name:       libbadge
Summary:    Badge library
Version:    0.0.5
Release:    0
Group:      Application Framework/Libraries
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001: %{name}.manifest
Source1002: init_db.sh
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(dbus-glib-1)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(com-core)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(libtzplatform-config)
BuildRequires: cmake

%description
Badge library.

%package devel
Summary: Badge library (devel)
Group: Application Framework/Development
Requires: %{name} = %{version}-%{release}

%description devel
Development files needed to build software that needs to system a system badge.

%package service-devel
Summary: Badge service library (service-devel)
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description service-devel
Development files needed to build badge service

%prep
%setup -q
cp %{SOURCE1001} .

%build
%cmake .
make %{?jobs:-j%jobs}

%install
%make_install
install -D -m 0640 badge.sql %{buildroot}%{TZ_SYS_SHARE}/badge/ressources/badge.sql
install -D -m 0750 %{SOURCE1002} %{buildroot}%{TZ_SYS_SHARE}/badge/ressources/init_db.sh

%post  -p /sbin/ldconfig -n libbadge

%postun -p /sbin/ldconfig -n libbadge

%files -n libbadge
%manifest %{name}.manifest
%license LICENSE.APLv2.0
%defattr(-,root,root,-)
%{_libdir}/libbadge.so.*
%{TZ_SYS_SHARE}/badge
%attr(640,root,%{TZ_SYS_USER_GROUP}) %{TZ_SYS_SHARE}/badge/ressources/badge.sql
%attr(750,root,%{TZ_SYS_USER_GROUP}) %{TZ_SYS_SHARE}/badge/ressources/init_db.sh

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/badge/badge.h
%{_includedir}/badge/badge_error.h
%{_includedir}/badge/badge_setting.h
%{_libdir}/libbadge.so
%{_libdir}/pkgconfig/badge.pc

%files service-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/badge/service/badge_db.h
%{_includedir}/badge/service/badge_setting_service.h
%{_libdir}/pkgconfig/badge-service.pc
