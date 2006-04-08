Summary: system tray FTP and webdav server
Name: netek
Version: 0.8.1
Release: 1
License: GPL
Group: Applications/Internet
Source: netek-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

%description
neteK is a user-friendly personal FTP and WebDAV server. The program stays in the system tray without using too many system resources. Target platforms are Linux, Windows, and Mac OS.

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
install -m 644 -D COPYING %{buildroot}/usr/share/doc/netek/COPYING
install -m 644 -D netek.desktop %{buildroot}/usr/share/applications/netek.desktop
install -m 644 -D icons/netek.png %{buildroot}/usr/share/pixmaps/netek.png
install -m 644 -D netek_konqueror.desktop %{buildroot}/opt/kde3/share/apps/konqueror/servicemenus/netek_konqueror.desktop

%clean
rm -fr %{buildroot}

%files
%defattr(-,root,root)
/
