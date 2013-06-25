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
BuildRequires: sqlite3

%description
Badge library.

%package -n libbadge
Summary:    Badge Library
Requires:   %{name} = %{version}-%{release}

%description -n libbadge
Badge library.


%package devel
Summary:    Badge library (devel)
Group:      Application Framework/Development
Requires:   libbadge = %{version}-%{release}

%description devel
Badge library (devel).

%prep
%setup -q

%build
%cmake . 
make %{?jobs:-j%jobs}

%install
%make_install
mkdir -p %{buildroot}/opt/dbspace
sqlite3 %{buildroot}/opt/dbspace/.%{name}.db < %{name}.sql

%post  -p /sbin/ldconfig -n libbadge

%postun -p /sbin/ldconfig -n libbadge

%files -n libbadge
%license LICENSE.APLv2.0
%manifest badge.manifest
%defattr(-,root,root,-)
%{_libdir}/libbadge.so.*

%files 
%verify(not md5 size mtime) %config(noreplace) %attr(660,root,app) /opt/dbspace/.%{name}.db-journal
%verify(not md5 size mtime) %config(noreplace) %attr(660,root,app) /opt/dbspace/.%{name}.db

%files devel
%defattr(-,root,root,-)
%{_includedir}/%{name}/*.h
%{_libdir}/libbadge.so
%{_libdir}/pkgconfig/%{name}.pc

