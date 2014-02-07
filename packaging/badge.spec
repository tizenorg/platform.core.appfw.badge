Name:       badge
Summary:    Badge library
Version:    0.0.5
Release:    0
Group:      Application Framework/Libraries
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001: badge.manifest
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

%package -n libbadge
Summary:    Badge Library

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
cp %{SOURCE1001} .

%build
%cmake . 
make %{?jobs:-j%jobs}

%install
%make_install
install -D -m 0640 %{name}.sql %{buildroot}%{TZ_SYS_SHARE}/%{name}/ressources/%{name}.sql
install -D -m 0750 %{SOURCE1002} %{buildroot}%{TZ_SYS_SHARE}/%{name}/ressources/init_db.sh

%post  -p /sbin/ldconfig -n libbadge

%postun -p /sbin/ldconfig -n libbadge

%files -n libbadge
%manifest %{name}.manifest
%license LICENSE.APLv2.0
%manifest badge.manifest
%defattr(-,root,root,-)
%{_libdir}/libbadge.so.*
%{TZ_SYS_SHARE}/%{name}
%attr(640,root,%{TZ_SYS_USER_GROUP}) %{TZ_SYS_SHARE}/%{name}/ressources/%{name}.sql
%attr(750,root,%{TZ_SYS_USER_GROUP}) %{TZ_SYS_SHARE}/%{name}/ressources/init_db.sh

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/%{name}/*.h
%{_libdir}/libbadge.so
%{_libdir}/pkgconfig/%{name}.pc

