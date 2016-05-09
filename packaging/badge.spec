Name:       badge
Summary:    Badge library
Version:    0.0.16
Release:    1
Group:      Applications/Core Applications
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(dbus-glib-1)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(capi-appfw-package-manager)
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(libtzplatform-config)
BuildRequires: cmake
Requires(post): /sbin/ldconfig
requires(postun): /sbin/ldconfig
%description
Client/Server library for updating badge information

%prep
%setup -q

%package devel
Summary:    Badge library (devel)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
Development files needed to build software that needs to system a system badge.

%package service-devel
Summary:    Badge library (service-devel)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description service-devel
Development files needed to build badge service

%build
%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS="$CFLAGS -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_ENGINEER_MODE"
export FFLAGS="$FFLAGS -DTIZEN_ENGINEER_MODE"
%endif
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif
export LDFLAGS+="-Wl,--rpath=%{_prefix}/lib -Wl,--as-needed"
LDFLAGS="$LDFLAGS" %cmake .
make %{?jobs:-j%jobs}

%install
%make_install

mkdir -p %{buildroot}/usr/share/license
cp -f LICENSE %{buildroot}/usr/share/license/%{name}


%post
/sbin/ldconfig

if [ ! -d %{TZ_SYS_DB} ]
then
	mkdir -p %{TZ_SYS_DB}
fi

%postun -p /sbin/ldconfig

%files
%manifest badge.manifest
%defattr(-,root,root,-)
%{_libdir}/libbadge.so*
%{_datarootdir}/license/%{name}
%{_includedir}/badge/badge.h
%{_includedir}/badge/badge_error.h
%{_includedir}/badge/badge_internal.h

%files devel
%defattr(-,root,root,-)
%{_includedir}/badge/badge.h
%{_includedir}/badge/badge_error.h
%{_includedir}/badge/badge_internal.h
%{_includedir}/badge/badge_db.h
%{_includedir}/badge/badge_setting_service.h
%{_libdir}/pkgconfig/%{name}.pc
