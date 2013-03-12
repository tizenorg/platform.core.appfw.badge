%define DBDIR "/opt/dbspace"
Name:       badge
Summary:    badge library
Version:    0.0.2
Release:    2
Group:      TBD
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(dbus-glib-1)
BuildRequires: pkgconfig(dlog)
BuildRequires: cmake
Requires(post): /sbin/ldconfig
requires(postun): /sbin/ldconfig
%description
Badge library.

%prep
%setup -q

%package devel
Summary:    Badge library (devel)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
Badge library (devel).

%build
export LDFLAGS+="-Wl,--rpath=%{_prefix}/lib -Wl,--as-needed"
LDFLAGS="$LDFLAGS" cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}
make %{?jobs:-j%jobs}

%install
%make_install

mkdir -p %{buildroot}/usr/share/license
cp -f LICENSE.APLv2.0 %{buildroot}/usr/share/license/%{name}


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
chmod 660 %{DBDIR}/.%{name}.db
chmod 660 %{DBDIR}/.%{name}.db-journal

%postun -p /sbin/ldconfig

%files
%manifest badge.manifest
%defattr(-,root,root,-)
%{_libdir}/lib%{name}.so*
/usr/share/license/%{name}

%files devel
%defattr(-,root,root,-)
%{_includedir}/%{name}/*.h
%{_libdir}/pkgconfig/%{name}.pc

