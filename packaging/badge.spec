%define DBDIR "/opt/dbspace"
Name:       badge
Summary:    badge library
Version:    0.0.16
Release:    1
Group:      TBD
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(dbus-glib-1)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(com-core)
BuildRequires: pkgconfig(capi-appfw-package-manager)
BuildRequires: pkgconfig(db-util)
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
LDFLAGS="$LDFLAGS" cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}
make %{?jobs:-j%jobs}

%install
%make_install

mkdir -p %{buildroot}/usr/share/license
cp -f LICENSE %{buildroot}/usr/share/license/%{name}


%post
/sbin/ldconfig

if [ ! -d %{DBDIR} ]
then
	mkdir -p %{DBDIR}
fi

if [ ! -f %{DBDIR}/.%{name}.db ]
then
	sqlite3 %{DBDIR}/.%{name}.db 'PRAGMA journal_mode = PERSIST;
		create table if not exists badge_data (
			pkgname TEXT NOT NULL,
			writable_pkgs TEXT,
			badge INTEGER default 0,
			rowid INTEGER PRIMARY KEY AUTOINCREMENT,
			UNIQUE (pkgname)
		);
		create table if not exists badge_option (
			pkgname TEXT NOT NULL,
			display INTEGER default 1,
			UNIQUE (pkgname)
		);
	'
fi

chown :5000 %{DBDIR}/.%{name}.db
chown :5000 %{DBDIR}/.%{name}.db-journal
chmod 644 %{DBDIR}/.%{name}.db
chmod 644 %{DBDIR}/.%{name}.db-journal
if [ -f /usr/lib/rpm-plugins/msm.so ]
then
    chsmack -a 'badge::db' %{DBDIR}/.%{name}.db*
fi

%postun -p /sbin/ldconfig

%files
%manifest badge.manifest
%defattr(-,root,root,-)
%{_libdir}/libbadge.so*
/usr/share/license/%{name}

%files devel
%defattr(-,root,root,-)
%{_includedir}/badge/badge.h
%{_includedir}/badge/badge_error.h
%{_includedir}/badge/badge_internal.h
%{_libdir}/pkgconfig/%{name}.pc

%files service-devel
%defattr(-,root,root,-)
%{_includedir}/badge/service/badge_db.h
%{_includedir}/badge/service/badge_setting_service.h
%{_libdir}/pkgconfig/badge-service.pc
