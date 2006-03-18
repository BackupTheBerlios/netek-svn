Summary: system tray application for creating FTP and webdav shares
Name: netek
Version: 0.8.1
Release: 1
License: GPL
Group: Applications/Internet
Source: netek-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

%description
neteK is an user-friendly application for creating network shares (drives) with FTP and webdav. The program stays in the system tray without using too much system resources. Target platforms are Linux, Windows, and Mac OS.

%prep
%setup -q

%build
if [ -x "$QMAKE" ]; then
	"$QMAKE"
else
	qmake
fi

make

%install
rm -fr %{buildroot}
install -s -m 755 -D netek %{buildroot}/usr/bin/netek
install -m 644 -D netek.desktop %{buildroot}/usr/share/applications/netek.desktop
install -m 644 -D icons/netek.png %{buildroot}/usr/share/pixmaps/netek.png

%clean
rm -fr %{buildroot}

%files
%defattr(-,root,root)
/
