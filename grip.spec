%define name grip
%define version 2.94
%define release 1

Summary: Grip, a CD player and ripper/MP3-encoder front-end
Name: %{name}
Version: %{version}
Release: %{release}
Copyright: GPL
Group: Applications/Sound
Source: http://www.nostatic/grip/grip-%{version}.tgz
URL: http://www.nostatic.org/grip
Packager: Mike Oliphant <oliphant@gtk.org>
Icon: gripicon.gif
Buildroot: /tmp/%{name}-root

%description
Grip is a gtk-based cd-player and cd-ripper. It has the ripping capabilities
of cdparanoia builtin, but can also use external rippers (such as
cdda2wav). It also provides an automated frontend for MP3 encoders, letting
you take a disc and transform it easily straight into MP3s. The CDDB
protocol is supported for retrieving track information from disc database
servers. Grip works with DigitalDJ to provide a unified "computerized"
version of your music collection.

%package -n gcd
Summary: GCD, a cd-player with a gtk+ interface
Group: Applications/Sound
Icon: gcdicon.gif

%description -n gcd
GCD is a cd-player with a gtk+ interface. It supports the CDDB protocol for
lookup of track information over the net, caching the information locally
once it has been retrieved. HTTP proxies are supported for those behind
firewalls. It is fully compatible with Grip for its config file and storage
of disc track information.


%prep
rm -rf $RPM_BUILD_ROOT
%setup

%build
cd cdparanoia
./configure --prefix==$RPM_BUILD_ROOT/usr
make lib
cd ..
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT

make PREFIX=$RPM_BUILD_ROOT/usr install gcdinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc README CHANGES CREDITS LICENSE TODO gripicon.tif
/usr/bin/grip
/usr/man/man1/grip.1

%files -n gcd
%doc README CHANGES CREDITS LICENSE TODO gcdicon.tif
/usr/bin/gcd
/usr/man/man1/gcd.1
