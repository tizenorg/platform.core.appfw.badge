Name:       badge
Summary:    badge library
Version:    0.0.5
Release:    1
Group:      Application Framework/Libraries
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(dbus-glib-1)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(com-core)
BuildRequires: cmake
%description
Badge library.

%prep
%setup -q

%package devel
Summary:    Badge library (devel)
Group:      Application Framework/Development
Requires:   %{name} = %{version}-%{release}

%description devel
Badge library (devel).

%build
%cmake . 
make %{?jobs:-j%jobs}

%install
%make_install

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
if [ -f /usr/lib/rpm-plugins/msm.so ]
then
    chsmack -a 'badge::db' %{DBDIR}/.%{name}.db*
fi

%postun -p /sbin/ldconfig

%files
%license LICENSE.APLv2.0
%manifest badge.manifest
%defattr(-,root,root,-)
%{_libdir}/lib%{name}.so*

%files devel
%defattr(-,root,root,-)
%{_includedir}/%{name}/*.h
%{_libdir}/pkgconfig/%{name}.pc

